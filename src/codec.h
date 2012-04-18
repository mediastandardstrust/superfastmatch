#ifndef _SFMCODEC_H                       // duplication check
#define _SFMCODEC_H

#include <common.h>

using namespace std;

namespace superfastmatch
{
  struct PostLineHeader{
    uint64_t doc_type;
    uint64_t length;
    
    PostLineHeader():
    doc_type(0),length(0){}
    
    PostLineHeader(uint64_t doc_type,uint64_t length):
    doc_type(doc_type),length(length){}
  };

  class PostLineCodec{
  public:
    virtual ~PostLineCodec(){};
    virtual size_t encodeHeader(const vector<PostLineHeader>& header,unsigned char* start)=0;
    virtual size_t decodeHeader(const unsigned char* start, vector<PostLineHeader>& header)=0;
    virtual size_t encodeSection(vector<uint32_t>& section,unsigned char* start)=0;
    virtual size_t decodeSection(const unsigned char* start, const size_t length, vector<uint32_t>& section,bool asDeltas)=0;
  };
  
  class VarIntCodec: public PostLineCodec{
  public:
    virtual ~VarIntCodec();
    size_t encodeHeader(const vector<PostLineHeader>& header,unsigned char* start);
    size_t decodeHeader(const unsigned char* start, vector<PostLineHeader>& header);
    size_t encodeSection(vector<uint32_t>& section,unsigned char* start);
    size_t decodeSection(const unsigned char* start, const size_t length, vector<uint32_t>& section,bool asDeltas=true);
  };
  
  class GroupVarIntCodec: public PostLineCodec{
  public:
    virtual ~GroupVarIntCodec();
    size_t encodeHeader(const vector<PostLineHeader>& header,unsigned char* start);
    size_t decodeHeader(const unsigned char* start, vector<PostLineHeader>& header);
    size_t encodeSection(vector<uint32_t>& section,unsigned char* start);
    size_t decodeSection(const unsigned char* start, const size_t length, vector<uint32_t>& section,bool asDeltas=true);
  };
}
#endif