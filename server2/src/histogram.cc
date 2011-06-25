#include "histogram.h"

namespace superfastmatch {
	Histogram::Histogram(const Registry& registry)
	:registry_(registry)
	{
		string dummy;
		if (not registry.miscDB->get("histogram:0",&dummy)){
			stringstream s;
			s << (1L<<registry.hash_width)-1;
			registry.miscDB->set("histogram:0",s.str());
		}
		//Cursor here
	}

	Histogram::~Histogram(){
		save();
	}

	bool Histogram::save(){
		return true;
	}

	bool Histogram::swap(uint32_t prev,uint32_t current){
		return true;
	}
}