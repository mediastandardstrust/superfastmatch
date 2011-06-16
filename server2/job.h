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
		Job(const Registry& registry):
		registry_(registry)
		{
			uint64_t counter;
			stringstream s;
			counter=registry_.jobDB->increment("JobCounter",1);
			s << counter;
			name_=s.str();
		}
		
		~Job(){}
		
		bool log(const char* name, const char* message){
			stringstream s;
			s << name << " : " << message << endl;
			return registry_.jobDB->append(name_,s.str());
		}
		
		bool start(){
			return registry_.jobDB->append(name_,"STARTED\n");
		}
		
		bool finish(){
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
	};
}
#endif