#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <queue>
#include <map>
#include <stdlib.h>

#include "Poco/InflatingStream.h"
#include "Poco/BinaryReader.h"

using Poco::InflatingInputStream; 
using Poco::InflatingStreamBuf;
using Poco::BinaryReader;

using namespace std;

class UInt64File
{
private:
	ifstream stream;
	InflatingInputStream inflater;
	BinaryReader reader;
	Poco::UInt64 _value;
	uint64_t _right_shift;

public:
	UInt64File(const char* file,uint64_t hash_width) :
	stream(file, ios::binary),
	inflater(stream, InflatingStreamBuf::STREAM_GZIP),
	reader(inflater)
	{
		reader >> _value;
		_right_shift=64L-hash_width;
	}

	friend ostream& operator<<(ostream & os, UInt64File & f ) {
		 return os << f.getValue() << " " << f.getHash() << " " << f.getDocument() ;
	}
	
	uint64_t getValue(){
		return _value;
	}
	
	uint32_t getHash(){
		return (uint32_t)(_value>>_right_shift);
	}
	
	uint32_t getDocument(){
		return (uint32_t)(_value&0xFFFFFFFF);
	}
	
	bool next(){
		reader >> _value;
		return reader.good();
	}
};

class UInt64Compare 
{
public:
	const bool operator()( UInt64File *f1, UInt64File *f2) {
		return f1->getValue() > f2->getValue();
	}
};
