#include "queue.h"

namespace superfastmatch{
	
	Queue::Queue(Registry& registry):
	registry_(registry){}

	uint64_t Queue::add_document(const uint32_t doc_type,const uint32_t doc_id,const string& content,bool associate){
		return CommandFactory::addDocument(registry_,doc_type,doc_id,content,associate);
	}

	uint64_t Queue::delete_document(const uint32_t& doc_type,const uint32_t& doc_id){
		return CommandFactory::dropDocument(registry_,doc_type,doc_id);
	}		
	
	bool Queue::process(){
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
			
			// cout << "Releasing Memory" << endl;
			// MallocExtension::instance()->ReleaseFreeMemory();
			//  cout << "Done!" << endl;
			// cout << "End Batch: " <<batch.size() << " Work: " << work.size() << " Docs: " << docs.size() << endl;
		}
		return workDone;
	}
	
	bool Queue::purge(){
		return true;
	}
	
	void Queue::toString(stringstream& s){
		vector<Command*> commands;
		CommandFactory::getAllCommands(registry_,commands);
		s << "Status:\t\tID:\tPriority:\tAction:\t\t\tDocType:\tDocID:"<<endl;
		for (vector<Command*>::iterator it=commands.begin(),ite=commands.end();it!=ite;++it){
			s << **it<< endl;
		}
		FreeClear(commands);
	}
	
	void Queue::toString(const uint64_t queue_id,stringstream& s){

	}
}