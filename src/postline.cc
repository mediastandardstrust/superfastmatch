#include <postline.h>

namespace superfastmatch
{
  // PostLine implementation
  // -----------------------
  
  // This implementation is designed to be fast as it is the 
  // most used code in the application!
  // Both map and unordered_map have been avoided.
  // Cursors and vectors seem to be faster.
  
  PostLine::PostLine(uint32_t max_length):
  codec_(new GroupVarIntCodec()),
  temp_header_(new unsigned char[max_length]),
  temp_sections_(new unsigned char[max_length]),
  old_header_length_(0),
  temp_header_length_(0),
  temp_sections_length_(0),
  updated_section_(0),
  header_(new vector<PostLineHeader>()),
  deltas_(new vector<uint32_t>()),
  docids_(new vector<uint32_t>())
  {
    section_.reserve(max_length);
    header_->reserve(max_length);
    deltas_->reserve(max_length);
    docids_->reserve(max_length);
  }
  
  PostLine::~PostLine(){
    delete[] temp_header_;
    delete[] temp_sections_;
    delete codec_;
    delete header_;
    delete deltas_;
    delete docids_;
  }
  
  vector<PostLineHeader>* PostLine::load(const unsigned char* start){
    start_=const_cast<unsigned char*>(start);
    old_header_length_=codec_->decodeHeader(start_,*header_);
    updated_section_=0;
    temp_header_length_=0;
    temp_sections_length_=0;
    return header_;
  }
  
  bool PostLine::commit(unsigned char* out){
    if (updated_section_==0){
      return false;
    }
    memcpy(out,temp_header_,temp_header_length_);
    memcpy(out+temp_header_length_,temp_sections_,temp_sections_length_);
    old_header_length_=temp_header_length_;
    start_=out;
    updated_section_=0;
    temp_header_length_=0;
    temp_sections_length_=0;
    return true;
  }

  bool PostLine::deleteDocument(const uint32_t doc_type,const uint32_t doc_id){
    if ((doc_type==0)||(doc_id==0)){
      return false;
    }
    vector<PostLineHeader>::iterator header=header_->begin();
    size_t offset=old_header_length_;
    bool noop=true;
    while(header!=header_->end()){
      if (header->doc_type==doc_type){
        noop=false;
        break;
      }
      offset+=header->length;
      header++;
    }
    if (noop){
      return false;
    }
    noop=true;
    codec_->decodeSection(start_+offset,header->length,section_,true);
    uint32_t previous=0;
    vector<uint32_t>::iterator cursor=section_.begin();
    while(cursor!=section_.end()){
      if(((previous+*cursor)==doc_id)){
        cursor=section_.erase(cursor);
        *cursor+=doc_id-previous;
        noop=false;
        break;
      }
      previous+=*cursor;
      cursor++;
    }
    if (noop){
      return false;
    }
    updated_section_=doc_type;
    size_t old_sections_length=old_header_length_;
    for (vector<PostLineHeader>::iterator it=header_->begin(),ite=header_->end();it!=ite;++it){
      if (it->doc_type==updated_section_){
        old_sections_length+=header->length;
        header->length=codec_->encodeSection(section_,temp_sections_+temp_sections_length_);
        temp_sections_length_+=header->length;
        if (header->length==0){
          header_->erase(header);
        }
      }else{
        memcpy(temp_sections_+temp_sections_length_,start_+old_sections_length,it->length);
        temp_sections_length_+=it->length;
        old_sections_length+=it->length;
      }
    }
    temp_header_length_=codec_->encodeHeader(*header_,temp_header_);
    return true;
  }
  
  bool PostLine::addDocument(const uint32_t doc_type,const uint32_t doc_id){
    if ((doc_type==0)||(doc_id==0)){
      return false;
    }
    updated_section_=doc_type;
    vector<PostLineHeader>::iterator header=header_->begin();
    size_t offset=old_header_length_;
    while(true){
      if((header==header_->end())||(header->doc_type>doc_type)){
        header=header_->insert(header,PostLineHeader(doc_type,0));
        break;        
      }else if (header->doc_type==doc_type){
        break;
      }
      offset+=header->length;
      header++;
    }
    codec_->decodeSection(start_+offset,header->length,section_,true);
    uint32_t previous=0;
    vector<uint32_t>::iterator cursor=section_.begin();
    while(true){
      if(cursor==section_.end()){
        section_.push_back(doc_id-previous);
        break;
      }
      else if ((previous+*cursor)>doc_id){
        *cursor+=previous-doc_id;
        section_.insert(cursor,doc_id-previous);
        break;
      }
      else if(((previous+*cursor)==doc_id)){
        break;
      }
      previous+=*cursor;
      cursor++;
    }
    size_t old_sections_length=old_header_length_;
    for (vector<PostLineHeader>::iterator it=header_->begin(),ite=header_->end();it!=ite;++it){
      if (it->doc_type==updated_section_){
        old_sections_length+=header->length;
        header->length=codec_->encodeSection(section_,temp_sections_+temp_sections_length_);
        temp_sections_length_+=header->length;
      }else{
        memcpy(temp_sections_+temp_sections_length_,start_+old_sections_length,it->length);
        temp_sections_length_+=it->length;
        old_sections_length+=it->length;
      }
    }
    temp_header_length_=codec_->encodeHeader(*header_,temp_header_);
    return true;
  }
  
  size_t PostLine::getLength(){ 
    size_t length=(temp_header_length_==0)?old_header_length_:temp_header_length_;
    for (size_t i=0;i<header_->size();i++){
      length+=(*header_)[i].length;
    }
    return length;
  }
  
  size_t PostLine::getLength(const uint32_t doc_type){
    vector<PostLineHeader>::iterator it=header_->begin();
    while(it!=header_->end()){
      if (it->doc_type==doc_type){
       return it->length; 
      }
      it++;
    }
    return 0;
  }
  
  vector<uint32_t>* PostLine::getDocIds(const uint32_t doc_type){
    size_t offset=old_header_length_;
    docids_->resize(0);
    for(vector<PostLineHeader>::iterator it=header_->begin(),ite=header_->end();it!=ite;++it){
      if (it->doc_type==doc_type){
        codec_->decodeSection(start_+offset,it->length,*docids_,false);
        break;
      }
      offset+=it->length;
    }
    return docids_;
  }
  
  vector<uint32_t>* PostLine::getDeltas(const uint32_t doc_type){
    size_t offset=old_header_length_;
    deltas_->resize(0);
    for(vector<PostLineHeader>::iterator it=header_->begin(),ite=header_->end();it!=ite;++it){
      if (it->doc_type==doc_type){
        codec_->decodeSection(start_+offset,it->length,*deltas_,true);
        break;
      }
      offset+=it->length;
    }
    return deltas_;
  }
  
  ostream& operator<< (ostream& stream, PostLine& postline) {
    stream << "Total Bytes: "<< postline.getLength() << endl;
    for(vector<PostLineHeader>::iterator it=postline.header_->begin(),ite=postline.header_->end();it!=ite;++it){
      vector<uint32_t>* docids=postline.getDocIds(it->doc_type);
      stream << "Doc Type: " << it->doc_type << " Bytes: " << it->length << " Doc Ids: {";
      for (vector<uint32_t>::iterator it2=docids->begin(),ite2=docids->end();it2!=ite2;it2++){
        stream << *it2 <<",";
      }
      stream << "}" <<endl; 
    }      
    stream << "-----------------------------------------------"<<endl;
    return stream;
  }
}