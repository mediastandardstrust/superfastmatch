#include <postline.h>

namespace superfastmatch
{
  // VarIntCodec implementation
  // --------------------------
  
  size_t VarIntCodec::encodeHeader(const vector<PostLineHeader>& header,unsigned char* start){
    size_t offset=0;
    for (size_t i=0;i<header.size();i++){
      offset+=kc::writevarnum(start+offset,header[i].doc_type);
      offset+=kc::writevarnum(start+offset,header[i].length);
    }
    offset+=kc::writevarnum(start+offset,0);
    return offset;
  }

  size_t VarIntCodec::decodeHeader(const unsigned char* start, vector<PostLineHeader>& header){
    size_t offset=0;
    PostLineHeader item;
    header.resize(0);
    while(start[offset]!=0){
      offset+=kc::readvarnum(start+offset,5,&item.doc_type);
      offset+=kc::readvarnum(start+offset,5,&item.length);
      header.push_back(item);
    };
    return offset+1;
  };

  size_t VarIntCodec::encodeSection(const vector<uint32_t>& section,unsigned char* start){
    size_t offset=0;
    for (size_t i=0;i<section.size();i++){
      offset+=kc::writevarnum(start+offset,section[i]);
    }
    return offset;
  }
  
  size_t VarIntCodec::decodeSection(const unsigned char* start, const size_t length, vector<uint32_t>& section){
    size_t offset=0;
    uint64_t value;
    section.resize(0);
    while (offset!=length){
      offset+=kc::readvarnum(start+offset,5,&value);
      section.push_back(value);
    }
    return offset;
  }
 
  // PostLine implementation
  // -----------------------
  
  // This implementation is designed to be fast as it is the 
  // most used code in the application!
  // Both map and unordered_map have been avoided.
  // Cursors and vectors seem to be faster.
  
  PostLine::PostLine(PostLineCodec* codec,uint32_t max_length):
  codec_(codec),
  temp_header_(new unsigned char[max_length]),
  temp_section_(new unsigned char[max_length])
  {}
  
  PostLine::~PostLine(){
    delete[] temp_header_;
    delete[] temp_section_;
  }
  
  void PostLine::load(const unsigned char* start){
    start_=const_cast<unsigned char*>(start);
    original_offset_=codec_->decodeHeader(start_,header_);
    new_offset_=original_offset_;
  }
  
  void PostLine::commit(unsigned char* out){
    vector<PostLineHeader>::iterator new_section;
    size_t offset=0;
    size_t updated_offset=0;
    size_t updated_start=0;
    for(vector<PostLineHeader>::iterator it=header_.begin(),ite=header_.end();it!=ite;++it){
      if (it->doc_type==updated_){
        new_section=it;
        updated_start=offset;
        updated_offset+=it->length;
        offset+=old_length_;
      }else{
        memmove(out+new_offset_+updated_offset,start_+original_offset_+offset,it->length);
        offset+=it->length;
      }
    }
    memcpy(out+new_offset_+updated_start,temp_section_,new_section->length);
    memcpy(out,temp_header_,new_offset_);
    original_offset_=new_offset_;
    start_=out;
  }
  
  void PostLine::addDocument(const uint32_t doc_type,const uint32_t doc_id){
    updated_=doc_type;
    vector<PostLineHeader>::iterator header=header_.begin();
    size_t offset=0;
    while(true){
      if((header==header_.end())||(header->doc_type>doc_type)){
        header=header_.insert(header,PostLineHeader(doc_type,0));
        break;        
      }else if (header->doc_type==doc_type){
        break;
      }
      offset+=header->length;
      header++;
    }
    old_length_=codec_->decodeSection(start_+original_offset_+offset,header->length,section_);
    uint32_t previous=0;
    vector<uint32_t>::iterator cursor=section_.begin();

    while(true){
      if(cursor==section_.end()){
        section_.push_back(doc_id-previous);
        break;
      }
      else if ((previous+*cursor)>doc_id){
        *cursor+=previous-doc_id;
        cursor=section_.insert(cursor,doc_id-previous);
        cursor++;
        break;
      }
      else if(((previous+*cursor)==doc_id)){
        break;
      }
      previous+=*cursor;
      cursor++;
    }
    header->length=codec_->encodeSection(section_,temp_section_);
    new_offset_=codec_->encodeHeader(header_,temp_header_);
  }
  
  size_t PostLine::getLength(){
    size_t length=new_offset_;
    for (size_t i=0;i<header_.size();i++){
      length+=header_[i].length;
    }
    return length;
  }
  
  size_t PostLine::getLength(const uint32_t doc_type){
    vector<PostLineHeader>::iterator it=header_.begin();
    while(it!=header_.end()){
      if (it->doc_type==doc_type)
        return it->length;
    }
    return 0;
  }
  
  void PostLine::getDocTypes(vector<uint32_t>& doc_types){
    doc_types.resize(0);
    for(vector<PostLineHeader>::iterator it=header_.begin(),ite=header_.end();it!=ite;++it){
      doc_types.push_back(it->doc_type);
    }
  }
  
  void PostLine::getDocIds(const uint32_t doc_type,vector<uint32_t>& doc_ids){
    getDeltas(doc_type,doc_ids);
    uint32_t previous=0;
    for(vector<uint32_t>::iterator it=doc_ids.begin(),ite=doc_ids.end();it!=ite;++it){
      *it+=previous;
      previous=*it;
    }
  }
  
  void PostLine::getDeltas(const uint32_t doc_type,vector<uint32_t>& deltas){
    deltas.resize(0);
    size_t offset=original_offset_;
    for(vector<PostLineHeader>::iterator it=header_.begin(),ite=header_.end();it!=ite;++it){
      if (it->doc_type==doc_type){
        codec_->decodeSection(start_+offset,it->length,deltas);
        break;
      }
      offset+=it->length;
    }
  }
  
  ostream& operator<< (ostream& stream, PostLine& postline) {
    vector<uint32_t> doctypes;
    vector<uint32_t> docids;
    postline.getDocTypes(doctypes);
    for (vector<uint32_t>::iterator it=doctypes.begin(),ite=doctypes.end();it!=ite;++it){
      postline.getDocIds(*it,docids);
      stream << "Doc Type: " << *it << " Doc Ids: {";
      for (vector<uint32_t>::iterator it2=docids.begin(),ite2=docids.end();it2!=ite2;it2++){
        stream << *it2 <<",";
      }
      stream << "}" <<endl; 
    }
    return stream;
  }
}