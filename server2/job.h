#ifndef _SFMJOB_H                       // duplication check
#define _SFMJOB_H

#include <common.h>
#include <registry.h>
#include <document.h>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>

namespace superfastmatch
{
	class Job
	{
	private:
		const Registry& registry_;
		string name_;
	public:
		Job(const Registry& registry,string& name):
		registry_(registry),name_(name)
		{}
		
		Job(const Registry& registry):
		registry_(registry)
		{
			uint64_t counter;
			stringstream s;
			counter=registry_.jobDB->increment("JobCounter",1);
			s << counter;
			name_=s.str();
		}
		
		~Job()
		{
			// delete name_;
		}
		
		bool addItem(string& key){
			if (hasStarted()){
				return false;
			}
			stringstream s;
			s << name_ << "___" << key;
			return registry_.jobDB->add(s.str(),"");
		}
		
		bool log(const char* name, const char* message){
			stringstream s;
			s << name << " : " << message << endl;
			return registry_.jobDB->append(name_,s.str());
		}
		
		bool start(){
			return registry_.jobDB->append(name_,"STARTED\n");
		}
		
		bool finish(){
			stringstream prefix;
			prefix << name_ << "___";
			vector<string> items;
			registry_.jobDB->match_prefix(prefix.str(),&items);
			for (vector<string>::iterator it=items.begin();it<items.end();it++){
				if (not registry_.jobDB->remove(*it)){
					return false;
				}
			}
			return registry_.jobDB->append(name_,"FINISHED\n");			
		}
		
		bool hasStarted(){
			string status;
			bool exists;
			exists = registry_.jobDB->get(name_,&status);
			return exists && status.find("STARTED\n")!=string::npos;	
		}
		
		bool hasFinished(){
			string status;
			registry_.jobDB->get(name_,&status);
			return status.find("FINISHED\n")!=string::npos;
		}
		
		bool isJobItem(const string& key,string& item){
			size_t suffix_pos;
			suffix_pos = key.find("___");
			if((suffix_pos!=string::npos) && (key.substr(0,suffix_pos)==name_)){
				item = key.substr(suffix_pos+3,string::npos);
				return true;
			}
			return false;
		}
	};
}
#endif