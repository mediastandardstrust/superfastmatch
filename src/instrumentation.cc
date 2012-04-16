#include <instrumentation.h>

namespace superfastmatch{
  // -----------------------------------
  // InstrumentDefinition implementation
  // -----------------------------------
  InstrumentDefinition::InstrumentDefinition(const string& name,const int32_t sort_order,const headers_t& timers,const headers_t& counters):
  name(name),
  sort_order(sort_order),
  timers(timers),
  counters(counters)
  {}
  
  // -------------------------
  // Instrument implementation
  // -------------------------
  Instrument::Instrument(const InstrumentDefinition* definition):
  definition_(definition)
  {
    timers_.resize(definition_->timers.size());
    counters_.resize(definition_->counters.size());
  }

  const InstrumentDefinition* Instrument::getDefinition() const{
    return definition_;
  }
  
  void Instrument::startTimer(const int32_t timer){
    timers_[timer]=kc::time();
  }

  void Instrument::stopTimer(const int32_t timer){
    timers_[timer]=kc::time()-timers_[timer];
  }

  void Instrument::setCounter(const int32_t counter,const uint64_t value){
    counters_[counter]=value;
  }
  
  void Instrument::incrementCounter(const int32_t counter,const uint64_t value){
    counters_[counter]+=value;
  }
  
  string Instrument::getInstance() const{
    stringstream instance;
    instance << definition_->name << "(" << (void*)this << ")";
    return instance.str();
  }
  
  void Instrument::getHeader(ostream& stream,columns_t& columns) const{
    columns.push_back(getInstance().size()+2);
    stream << setw(columns.back()) << "Instance";
    for (headers_t::const_iterator it=definition_->timers.begin(),ite=definition_->timers.end();it!=ite;++it){
      columns.push_back(max(10UL,it->second.size()+2));
      stream << setw(columns.back()) << it->second;
    }
    for (headers_t::const_iterator it=definition_->counters.begin(),ite=definition_->counters.end();it!=ite;++it){
      columns.push_back(max(10UL,it->second.size()+2));
      stream << setw(columns.back()) << it->second;
    }
    stream << endl;
  }
  
  void Instrument::getRow(ostream& stream,columns_t& columns,const string& name) const{
    size_t column=0;
    stream << setw(columns[column++]) << (name.empty()?getInstance():name);
    stream << setiosflags(ios::fixed) << setprecision(5);
    for (headers_t::const_iterator it=definition_->timers.begin(),ite=definition_->timers.end();it!=ite;++it){
      stream << setw(columns[column++]) << timers_[it->first];
    }
    for (headers_t::const_iterator it=definition_->counters.begin(),ite=definition_->counters.end();it!=ite;++it){
      stream << setw(columns[column++]) << counters_[it->first];
    }
    stream << endl;
  }

  ostream& operator << (ostream& stream, Instrument& instrument){
    columns_t columns;
    instrument.getHeader(stream,columns);
    instrument.getRow(stream,columns);
    return stream;
  }

  Instrument& Instrument::operator+=(const Instrument& rhs){
    assert(definition_==rhs.definition_);
    transform(rhs.timers_.begin(), rhs.timers_.end(),timers_.begin(),timers_.begin(),plus<double>());
    transform(rhs.counters_.begin(), rhs.counters_.end(),counters_.begin(),counters_.begin(),plus<uint64_t>());
    return *this;
  }

  Instrument& Instrument::operator/=(const double rhs){
    assert(rhs!=0.0);
    transform(timers_.begin(),timers_.end(),timers_.begin(),bind2nd(std::divides<double>(),rhs));
    transform(counters_.begin(),counters_.end(),counters_.begin(),bind2nd(std::divides<uint64_t>(),rhs));
    return *this;
  }

  bool Instrument::operator<(const Instrument& rhs){
    return timers_[definition_->sort_order]<rhs.timers_[definition_->sort_order];
  }

  // ------------------------------
  // InstrumentGroup implementation
  // ------------------------------
  InstrumentGroup::InstrumentGroup(const string& name,const size_t recent_size,const size_t slowest_size):
  name_(name),
  recent_size_(recent_size),
  slowest_size_(slowest_size)
  {}

  void InstrumentGroup::add(InstrumentPtr instrument){
    lock_.lock_writer();
    InstrumentHistory* history=&instruments_[instrument->getDefinition()->name];
    history->recent.push_back(instrument);
    if (history->recent.size()>recent_size_){
      history->recent.pop_front();
    }
    history->slowest.insert(instrument);
    if (history->slowest.size()>slowest_size_){
      set<InstrumentPtr>::iterator last=history->slowest.end();
      last--;
      history->slowest.erase(last);
    }
    lock_.unlock();
  }
  
  void InstrumentGroup::merge(InstrumentGroupPtr group){
    lock_.lock_writer();
    group->lock_.lock_reader();
    for(instruments_t::const_iterator it=group->instruments_.begin(),ite=group->instruments_.end();it!=ite;++it){
     InstrumentHistory* history=&instruments_[it->first];
     history->recent.insert(history->recent.end(),it->second.recent.begin(),it->second.recent.end());
     if (history->recent.size()>recent_size_){
       history->recent.erase(history->recent.begin(),history->recent.end()-recent_size_);
     }
     history->slowest.insert(it->second.slowest.begin(),it->second.slowest.end());
     if (history->slowest.size()>slowest_size_){
       set<InstrumentPtr>::iterator start =history->slowest.begin();
       advance(start,slowest_size_);
       history->slowest.erase(start,history->slowest.end()); 
     }
    }
    group->lock_.unlock();
    lock_.unlock();
  }
  
  void InstrumentGroup::clear(){
    lock_.lock_writer();
    instruments_.clear();
    lock_.unlock();
  }
  
  void InstrumentGroup::getHistory(ostream& stream){
    lock_.lock_reader();
    stream << name_ << endl;
    for (instruments_t::iterator it=instruments_.begin(),ite=instruments_.end();it!=ite;++it){
      if(it->second.slowest.size()>0){
        stream << it->first << " Slowest"<< endl;
        columns_t columns;
        (*it->second.slowest.begin())->getHeader(stream,columns);
        Instrument total((*it->second.slowest.begin())->getDefinition());
        for (set<InstrumentPtr>::const_iterator it2=it->second.slowest.begin(),ite2=it->second.slowest.end();it2!=ite2;++it2){
          total+=*(it2->get());
          (*it2)->getRow(stream,columns);
        }
        total.getRow(stream,columns,"Total");
        total/=double(it->second.slowest.size());
        total.getRow(stream,columns,"Average");
      }
      if(it->second.recent.size()>0){
        stream << it->first << " Recent"<< endl;
        columns_t columns;
        it->second.recent.front()->getHeader(stream,columns);
        Instrument total(it->second.recent.front()->getDefinition());
        for (deque<InstrumentPtr>::const_iterator it2=it->second.recent.begin(),ite2=it->second.recent.end();it2!=ite2;++it2){
          total+=*(it2->get());
          (*it2)->getRow(stream,columns);
        }
        total.getRow(stream,columns,"Total");
        total/=double(it->second.recent.size());
        total.getRow(stream,columns,"Average");
      }
    }
    lock_.unlock();
  }
  
  ostream& operator << (ostream& stream, InstrumentGroup& group){
    group.getHistory(stream);
    return stream;
  }
  
}