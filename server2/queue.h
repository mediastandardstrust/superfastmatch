#ifndef _SFMQUEUE_H                       // duplication check
#define _SFMQUEUE_H

#include <map>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <deque>
#include <tr1/unordered_map>
#include <kcutil.h>
#include <kcpolydb.h>
#include <registry.h>
#include <document.h>
#include <index.h>
#include <google/malloc_extension.h>

using namespace std;
using namespace std::tr1;
using namespace kyototycoon;

namespace superfastmatch
{		
	enum CommandType{
		Invalid,
		AddDocument,
		AddAssociation,
		DropDocument,
		DropAssociation
	};
	
	enum CommandStatus{
		Queued,
		Active,
		Finished,
		Failed
	};
	
	class Command{
	private:
		const Registry& registry_;
		uint64_t queue_id_;
		uint32_t priority_;
		CommandType type_;
		CommandStatus status_;
		uint32_t doc_type_;
		uint32_t doc_id_;
		string payload_;
		Document* document_;

		static const char* key_format;

	public:	
		Command(const Registry& registry,string& key,string& payload):
		registry_(registry),queue_id_(0),priority_(0),type_(Invalid),status_(Queued),doc_type_(0),doc_id_(0),payload_(payload),document_(0){
			if(sscanf(key.c_str(),key_format,&status_,&queue_id_,&priority_,&type_,&doc_type_,&doc_id_)!=6){
				throw "Bad parse of Command key!";
			}
		}
		
		Command(const Registry& registry,const uint64_t queue_id, const uint32_t priority,const CommandType type,const CommandStatus status, const uint32_t doc_type,const uint32_t doc_id,const string& payload):
		registry_(registry),queue_id_(queue_id),priority_(priority),type_(type),status_(status),doc_type_(doc_type),doc_id_(doc_id),payload_(payload),document_(0){
			save();
		}
		
		~Command(){
			if (document_!=0){
				delete document_;
				document_=0;
			}
		}
		
	private:
		string command_key(){
			return kc::strprintf(key_format,status_,queue_id_,priority_,type_,doc_type_,doc_id_);
		}
		
		bool save(bool keep_payload=true){
			return registry_.queueDB->set(command_key(),keep_payload?payload_:"");
		}
	public:
		
		CommandType getType(){
			return type_;
		}
		
		CommandStatus getStatus(){
			return status_;
		}
		
		uint32_t getDocType(){
			return doc_type_;
		}
		
		uint32_t getDocId(){
			return doc_id_;
		}
		
		uint64_t getQueueId(){
			return queue_id_;
		}

		bool setActive(){
			return remove() && (status_=Active) && save();
		}
		
		bool setFinished(){
			return remove() && (status_=Finished) && save(false);
		}
		
		bool setFailed(){
			return remove() && (status_=Failed) && save();
		}
		
		bool remove(){
			return registry_.queueDB->remove(command_key());
		}
		
		Document* getDocument(){
			if (document_==0){
				document_=new Document(doc_type_,doc_id_,payload_.c_str(),registry_);
				// In case of drop document!
				document_->load();
			}
			return document_;
		}
		
		friend std::ostream& operator<< (std::ostream& stream, Command& command) {
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
	};
	
	const char* Command::key_format = "%d:%020d:%03d:%d:%010d:%010d";
	
	class CommandFactory{
	private:
		static Command* createCommand(const Registry& registry, const CommandType commandType,const uint64_t queue_id,const uint32_t doc_type, const uint32_t doc_id,const string& payload){
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
	public:
		static uint64_t addDocument(const Registry& registry_,const uint32_t doc_type, const uint32_t doc_id,const string& content,const bool associate){
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
		
		static uint64_t dropDocument(const Registry& registry_,const uint32_t doc_type, const uint32_t doc_id){
			uint64_t queue_id = registry_.miscDB->increment("QueueCounter",1);
			string empty;
			Command* drop_doc = CommandFactory::createCommand(registry_,DropDocument,queue_id,doc_type,doc_id,empty);
			Command* drop_ass = CommandFactory::createCommand(registry_,DropAssociation,queue_id,doc_type,doc_id,empty);
			delete drop_doc;
			delete drop_ass;
			return queue_id;
		}
		
		static void insertDropDocument(const Registry& registry_,Command* command){
			string empty;
			Command* drop_doc = CommandFactory::createCommand(registry_,DropDocument,command->getQueueId(),command->getDocType(),command->getDocId(),empty);
			Command* drop_ass = CommandFactory::createCommand(registry_,DropAssociation,command->getQueueId(),command->getDocType(),command->getDocId(),empty);
			delete drop_doc;
			delete drop_ass;
		}
		
		static void getAllCommands(const Registry& registry_,vector<Command*>& commands){
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
		
		static bool getNextBatch(const Registry& registry_,deque<Command*>& batch,CommandType& batchType){
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
	};
	

	class Queue{
	private:
		const Registry& registry_;
	public:
		Queue(Registry& registry):
		registry_(registry){}
	
		uint64_t add_document(const uint32_t doc_type,const uint32_t doc_id,const string& content,bool associate){
			return CommandFactory::addDocument(registry_,doc_type,doc_id,content,associate);
		}

		uint64_t delete_document(const uint32_t& doc_type,const uint32_t& doc_id){
			return CommandFactory::dropDocument(registry_,doc_type,doc_id);
		}		
		
		bool process(){
			bool workDone=false;
			CommandType batchType=Invalid;			
			deque<Command*> batch;
			deque<Command*> work;
			deque<Document*> docs;
			Index index(registry_);
			while(CommandFactory::getNextBatch(registry_,batch,batchType)){
				workDone=true;
				switch (batchType){
					case AddDocument:
						cout << "Adding Document(s)" <<endl;
						while(!batch.empty()){
							//Check if document exists and insert drop if it does
							if (batch.front()->getDocument()->save()){
								batch.front()->setActive();
								work.push_back(batch.front());
								docs.push_back(batch.front()->getDocument());
								batch.pop_front();	
							}else{
								cout << "Inserting drop for: " << *batch.front()->getDocument() << endl;
								CommandFactory::insertDropDocument(registry_,batch.front());
								break;
							}
						}
						index.batch(docs,false);
						for(deque<Command*>::iterator it=work.begin(),ite=work.end();it!=ite;++it){
							(*it)->setFinished();
						}
						break;	
					case DropDocument:
						cout << "Dropping Document(s)" <<endl;
						while(!batch.empty()){
							batch.front()->setActive();
							work.push_back(batch.front());
							docs.push_back(batch.front()->getDocument());
							batch.pop_front();	
						}
						index.batch(docs,true);
						for (deque<Command*>::iterator it=work.begin(),ite=work.end();it!=ite;++it){
							if (not((*it)->getDocument()->remove())){
								(*it)->setFailed();
							}else{
								(*it)->setFinished();
							}
						}
						break;
					case AddAssociation:
						cout << "Adding Association(s)" <<endl;
						while(!batch.empty()){;
							batch.front()->setActive();
							work.push_back(batch.front());
							docs.push_back(batch.front()->getDocument());
							batch.pop_front();
						}
						//Do Add Association Here
						for(deque<Command*>::iterator it=work.begin(),ite=work.end();it!=ite;++it){
							(*it)->setFinished();
						}
						break;
					case DropAssociation:
						cout << "Dropping Association(s)" <<endl;
						while(!batch.empty()){
							batch.front()->setActive();
							work.push_back(batch.front());
							docs.push_back(batch.front()->getDocument());
							batch.pop_front();
						}
						//Do Drop Association
						for(deque<Command*>::iterator it=work.begin(),ite=work.end();it!=ite;++it){
							(*it)->setFinished();
						}
						break;
					case Invalid:
						throw("Bad batch type");
						break;
				}
				FreeClear(batch);
				FreeClear(work);
				docs.clear();
				cout << "Releasing Memory" << endl;
				MallocExtension::instance()->ReleaseFreeMemory();
				cout << "Done!" << endl;
				// cout << "End Batch: " <<batch.size() << " Work: " << work.size() << " Docs: " << docs.size() << endl;
			}
			return workDone;
		}
		
		bool purge(){
			return true;
		}
		
		void toString(stringstream& s){
			vector<Command*> commands;
			CommandFactory::getAllCommands(registry_,commands);
			s << "Status:\t\tID:\tPriority:\tAction:\t\t\tDocType:\tDocID:"<<endl;
			for (vector<Command*>::iterator it=commands.begin(),ite=commands.end();it!=ite;++it){
				s << **it<< endl;
			}
			FreeClear(commands);
		}
		
		void toString(const uint64_t queue_id,stringstream& s){

		}		
	};

}

#endif