#ifndef _SFMQUEUE_H                       // duplication check
#define _SFMQUEUE_H

#include <map>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <deque>
#include <kcutil.h>
#include <kcpolydb.h>
#include <registry.h>
#include <document.h>


using namespace std;
using namespace kyototycoon;

namespace superfastmatch
{		
	enum CommandType{
		AddDocument,
		AddAssociation,
		DropDocument,
		DropAssociation
	};
	
	enum CommandStatus{
		Queued,
		Active,
		Finished
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
		Command(const Registry& registry,string& key,string& value):
		registry_(registry),payload_(value),document_(0){
			sscanf(key.c_str(),key_format,&status_,&queue_id_,&priority_,&type_,&doc_type_,&doc_id_);
		}
		
		Command(const Registry& registry,const uint64_t queue_id, const uint32_t priority,const CommandType type,const CommandStatus status, const uint32_t doc_type,const uint32_t doc_id,const string& payload):
		registry_(registry),queue_id_(queue_id),priority_(priority),type_(type),status_(status),doc_type_(doc_type),doc_id_(doc_id),payload_(payload),document_(0){
			save();
		}
		
		~Command(){
			if (document_!=0){
				delete document_;
			}
		}
		
	private:
		string command_key(){
			return kc::strprintf(key_format,status_,queue_id_,priority_,type_,doc_type_,doc_id_);
		}
		
		bool save(){
			return registry_.queueDB->set(command_key(),payload_);
		}
	public:
		
		CommandType getType(){
			return type_;
		}
		
		CommandStatus getStatus(){
			return status_;
		}

		bool setActive(){
			return remove() && (status_=Active) && save();
		}
		
		bool setFinished(){
			return remove() && (status_=Finished) && save();
		}
		
		bool remove(){
			return registry_.queueDB->remove(command_key());
		}
		
		bool isSameAction(Command* command){
			return (((command->getType()==AddDocument)||(command->getType()==AddAssociation))&& ((type_==AddDocument)||(type_=AddAssociation)));
		}
		
		Document* getDocument(){
			if (document_==0){
				document_=new Document(doc_type_,doc_id_,payload_.c_str(),registry_);
			}
			return document_;
		}

		void toString(stringstream& s){

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
			}
			throw "Invalid Command Type!";
		}
	public:
		static uint64_t addDocument(const Registry& registry_,const uint32_t doc_type, const uint32_t doc_id,const string& content){
			uint64_t queue_id = registry_.queueDB->increment("QueueCounter",1);
			string empty;
			Command* add_doc = CommandFactory::createCommand(registry_,AddDocument,queue_id,doc_type,doc_id,content);
			Command* add_ass = CommandFactory::createCommand(registry_,AddAssociation,queue_id,doc_type,doc_id,empty);
			delete add_doc;
			delete add_ass;
			return queue_id;
		}
		
		static uint64_t dropDocument(const Registry& registry_,const uint32_t doc_type, const uint32_t doc_id){
			uint64_t queue_id = registry_.queueDB->increment("QueueCounter",1);
			string empty;
			Command* drop_doc = CommandFactory::createCommand(registry_,DropDocument,queue_id,doc_type,doc_id,empty);
			Command* drop_ass = CommandFactory::createCommand(registry_,DropAssociation,queue_id,doc_type,doc_id,empty);
			delete drop_doc;
			delete drop_ass;
			return queue_id;
		}
		
		static bool getNextBatch(const Registry& registry_,deque<Command*>& batch){
			string key;
			string value;
			kc::PolyDB::Cursor* cur = registry_.queueDB->cursor();
			cur->jump();
			while (cur->get(&key,&value,true)){
				Command* command = new Command(registry_,key,value);
				if (not command->getStatus()==Queued){
					break;
				}
				else if (batch.size()==0){
					batch.push_back(command);
				}
				else if (command->getType()==batch.back()->getType()){
					batch.push_back(command);	
				}
				else if(not command->isSameAction(batch.back())){
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
	
		uint64_t add_document(const uint32_t doc_type,const uint32_t doc_id,const string& content){
			return CommandFactory::addDocument(registry_,doc_type,doc_id,content);
		}

		uint64_t delete_document(const uint32_t& doc_type,const uint32_t& doc_id){
			return CommandFactory::dropDocument(registry_,doc_type,doc_id);
		}		
		
		bool process(){
			bool workDone=false;
			deque<Command*> batch;			
			vector<Document*> docs;
			while(CommandFactory::getNextBatch(registry_,batch)){
				workDone=true;
				while(!batch.empty()){
					batch.front()->getDocument();
					batch.front()->setActive();
					batch.pop_front();
				}
				//scan batch for documents with changed content and insert dropdocument in queue at that point
			}
			return workDone;
		}
		
		bool purge(){
			return true;
		}
		
		void toString(stringstream& s){
			
		}
		
		void toString(const uint64_t queue_id,stringstream& s){
			
		}		
	};

}

#endif