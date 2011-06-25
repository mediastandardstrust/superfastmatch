#include "command.h"

namespace superfastmatch{
	const char* Command::key_format = "%d:%020d:%03d:%d:%010d:%010d";	
	
	Command::Command(const Registry& registry,string& key,string& payload):
	registry_(registry),queue_id_(0),priority_(0),type_(Invalid),status_(Queued),doc_type_(0),doc_id_(0),payload_(payload),document_(0){
		if(sscanf(key.c_str(),key_format,&status_,&queue_id_,&priority_,&type_,&doc_type_,&doc_id_)!=6){
			throw "Bad parse of Command key!";
		}
	}
	
	Command::Command(const Registry& registry,const uint64_t queue_id, const uint32_t priority,const CommandType type,const CommandStatus status, const uint32_t doc_type,const uint32_t doc_id,const string& payload):
	registry_(registry),queue_id_(queue_id),priority_(priority),type_(type),status_(status),doc_type_(doc_type),doc_id_(doc_id),payload_(payload),document_(0){
		save();
	}
	
	Command::~Command(){
		if (document_!=0){
			delete document_;
			document_=0;
		}
	}
	
	string Command::command_key(){
		return kc::strprintf(key_format,status_,queue_id_,priority_,type_,doc_type_,doc_id_);
	}
	
	bool Command::save(bool keep_payload){
		return registry_.queueDB->set(command_key(),keep_payload?payload_:"");
	}
	
	CommandType Command::getType(){
		return type_;
	}
	
	CommandStatus Command::getStatus(){
		return status_;
	}
	
	uint32_t Command::getDocType(){
		return doc_type_;
	}
	
	uint32_t Command::getDocId(){
		return doc_id_;
	}
	
	uint64_t Command::getQueueId(){
		return queue_id_;
	}

	bool Command::setActive(){
		return remove() && (status_=Active) && save();
	}
	
	bool Command::setFinished(){
		return remove() && (status_=Finished) && save(false);
	}
	
	bool Command::setFailed(){
		return remove() && (status_=Failed) && save();
	}
	
	bool Command::remove(){
		return registry_.queueDB->remove(command_key());
	}
	
	Document* Command::getDocument(){
		if (document_==0){
			document_=new Document(doc_type_,doc_id_,payload_.c_str(),registry_);
			// In case of drop document!
			document_->load();
		}
		return document_;
	}
	
	std::ostream& operator<< (std::ostream& stream, Command& command) {
		switch (command.getStatus()){
			case Queued:
				stream << "Queued\t";
				break;
			case Active:
				stream << "Active\t";
				break;
			case Finished:
				stream << "Finished";
				break;
			case Failed:
				stream << "Failed";
				break;
		}
		stream << "\t" << command.queue_id_ ;
		stream << "\t" << command.priority_ << "\t\t";
		switch (command.getType()){
			case Invalid:
				stream << "Invalid\t\t";
				break;
			case AddDocument:
				stream << "Add Document\t";
				break;
			case AddAssociation:
				stream << "Add Association";
				break;
			case DropDocument:
				stream << "Drop Document\t";
				break;
			case DropAssociation:
				stream << "Drop Association";
				break;
		}
		stream << "\t" << command.doc_type_ << "\t\t" << command.doc_id_;
		return stream;
	}
	
	
	Command* CommandFactory::createCommand(const Registry& registry, const CommandType commandType,const uint64_t queue_id,const uint32_t doc_type, const uint32_t doc_id,const string& payload){
		switch (commandType){
			case AddDocument:
				return new Command(registry,queue_id,101,commandType,Queued,doc_type,doc_id,payload);
			case AddAssociation:
				return new Command(registry,queue_id,102,commandType,Queued,doc_type,doc_id,payload);
			case DropDocument:
				return new Command(registry,queue_id,2,commandType,Queued,doc_type,doc_id,payload);
			case DropAssociation:
				return new Command(registry,queue_id,1,commandType,Queued,doc_type,doc_id,payload);
			case Invalid:
				break;
		}
		throw "Invalid Command Type!";	
	}
	
	uint64_t CommandFactory::addDocument(const Registry& registry_,const uint32_t doc_type, const uint32_t doc_id,const string& content,const bool associate){
		uint64_t queue_id = registry_.miscDB->increment("QueueCounter",1);
		string empty;
		Command* add_doc = CommandFactory::createCommand(registry_,AddDocument,queue_id,doc_type,doc_id,content);
		delete add_doc;
		if (associate){
			Command* add_ass = CommandFactory::createCommand(registry_,AddAssociation,queue_id,doc_type,doc_id,empty);
			delete add_ass;	
		}
		return queue_id;
	}
		
	uint64_t CommandFactory::dropDocument(const Registry& registry_,const uint32_t doc_type, const uint32_t doc_id){
		uint64_t queue_id = registry_.miscDB->increment("QueueCounter",1);
		string empty;
		Command* drop_doc = CommandFactory::createCommand(registry_,DropDocument,queue_id,doc_type,doc_id,empty);
		Command* drop_ass = CommandFactory::createCommand(registry_,DropAssociation,queue_id,doc_type,doc_id,empty);
		delete drop_doc;
		delete drop_ass;
		return queue_id;
	}
	
	void CommandFactory::insertDropDocument(const Registry& registry_,Command* command){
		string empty;
		Command* drop_doc = CommandFactory::createCommand(registry_,DropDocument,command->getQueueId(),command->getDocType(),command->getDocId(),empty);
		Command* drop_ass = CommandFactory::createCommand(registry_,DropAssociation,command->getQueueId(),command->getDocType(),command->getDocId(),empty);
		delete drop_doc;
		delete drop_ass;
	}
	
	void CommandFactory::getAllCommands(const Registry& registry_,vector<Command*>& commands){
		kc::PolyDB::Cursor* cur = registry_.queueDB->cursor();
		cur->jump();
		string key;
		string value;
		while (cur->get(&key,&value,true)){
			Command* command = new Command(registry_,key,value);
			commands.push_back(command);
		}
		delete cur;
	}
	
	bool CommandFactory::getNextBatch(const Registry& registry_,deque<Command*>& batch,CommandType& batchType){
		string key;
		string value;
		uint32_t batch_count=0;
		kc::PolyDB::Cursor* cur = registry_.queueDB->cursor();
		cur->jump();
		while (batch_count<registry_.max_batch_count && cur->get(&key,&value,true)){
			batch_count++;
			Command* command = new Command(registry_,key,value);
			if (command->getStatus()!=Queued){
				delete command;
				command=0;
				break;
			}
			else if (batch.size()==0){
				batch.push_back(command);
				batchType=command->getType();
			}
			else if (command->getType()==batch.back()->getType()){
				batch.push_back(command);	
			}
			else{
				delete command;
				command=0;
				break;
			}
		}
		delete cur;
		return (batch.size()>0);
	}	
}