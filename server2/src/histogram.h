#ifndef _SFMHISTOGRAM_H                       // duplication check
#define _SFMHISTOGRAM_H

#include <map>
#include <common.h>
#include <registry.h>

namespace superfastmatch
{
	class Registry;
	
	class Histogram{
	private:
		const Registry& registry_;
		map<uint32_t,uint32_t> counts;

	public:
		Histogram(const Registry& registry);
		~Histogram();
		
		bool save();
		bool swap(uint32_t prev,uint32_t current);
	};
}

#endif