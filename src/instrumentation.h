#ifndef _SFMINSTRUMENTATION_H                       // duplication check
#define _SFMINSTRUMENTATION_H

#include <common.h>

namespace superfastmatch{
  // Forward Declaration
  class Instrument;
  class InstrumentGroup;
  struct InstrumentHistory;

  // Typedefs
  typedef map<int32_t,string> headers_t;
  typedef vector<double> timers_t;
  typedef vector<uint64_t> counters_t;
  typedef vector<uint32_t> columns_t;
  typedef shared_ptr<Instrument> InstrumentPtr;
  typedef shared_ptr<InstrumentGroup> InstrumentGroupPtr;
  typedef unordered_map<string,InstrumentHistory> instruments_t;

  // --------------------
  // InstrumentDefinition
  // --------------------

  struct InstrumentDefinition{
    const string name;
    const int32_t sort_order;
    const headers_t timers;
    const headers_t counters;
    
    InstrumentDefinition(const string& name,const int32_t sort_order,const headers_t& timers,const headers_t& counters);
  };
  
  // ----------
  // Instrument
  // ----------

  class Instrument{
  private:
    const InstrumentDefinition* definition_;
    timers_t timers_;
    counters_t counters_;
  public:
    Instrument(const InstrumentDefinition* definition);
    const InstrumentDefinition* getDefinition() const;
    void startTimer(const int32_t timer);
    void stopTimer(const int32_t timer);
    void setCounter(const int32_t counter,const uint64_t value);
    void incrementCounter(const int32_t counter,const uint64_t value=1);
    string getInstance() const;
    void getHeader(ostream& stream,columns_t& columns) const;
    void getRow(ostream& stream,columns_t& columns,const string& name="") const;
    Instrument& operator+=(const Instrument& rhs);
    Instrument& operator/=(const double rhs);
    bool operator<(const Instrument& rhs);
    friend ostream& operator<<(ostream& stream, Instrument& instrument);
  private:
    DISALLOW_COPY_AND_ASSIGN(Instrument);
  };

  // -----------------
  // InstrumentHistory
  // -----------------
  struct InstrumentHistoryComp{
    bool operator() (const InstrumentPtr& lhs, const InstrumentPtr& rhs)const{
      return (*rhs)<(*lhs);
    }
  };
  
  struct InstrumentHistory{
    deque<InstrumentPtr> recent;
    set<InstrumentPtr,InstrumentHistoryComp> slowest;
  };

  // ---------------
  // InstrumentGroup
  // ---------------

  class InstrumentGroup{
  private:
    const string name_;
    const size_t recent_size_;
    const size_t slowest_size_;
    instruments_t instruments_;
    kc::RWLock lock_;
  public:
    InstrumentGroup(const string& name,const size_t recent_size,const size_t slowest_size);
    void add(InstrumentPtr instrument);
    void merge(InstrumentGroupPtr group);
    friend std::ostream& operator<< (std::ostream& stream, InstrumentGroup& group);
  private:
    void getHistory(ostream& stream);
    DISALLOW_COPY_AND_ASSIGN(InstrumentGroup);
  };
  
  // ---------------------
  // Instrumented template
  // ---------------------
    
  template <class Derived>
  class Instrumented{
  private:
    const InstrumentPtr perf_;
  public:
    Instrumented():
    perf_(new Instrument(&Derived::definition))
    {}
    
    static const InstrumentDefinition definition;
    static const InstrumentDefinition getDefinition();
    
    static const InstrumentPtr createInstrument(){
      return InstrumentPtr(new Instrument(&Derived::definition));
    }
    
    const InstrumentPtr getInstrument(){
      return perf_;
    }
  };
  
  template <class Derived> const InstrumentDefinition Instrumented<Derived>::definition=Instrumented<Derived>::getDefinition();
}
#endif