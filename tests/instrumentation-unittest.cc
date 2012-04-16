#include <tests.h>

typedef BaseTest InstrumentationTest;

namespace superfastmatch{
  namespace T1{
    enum Timers{
      FIRST
    };
    enum Counters{
      DOC_TYPE,
      DOC_ID,
      COUNT
    }; 
  };

  namespace T2{
    enum Timers{
      FIRST,
      SECOND
    };
    enum Counters{
      COUNT
    };
  };

  class TestClass1 : public Instrumented<TestClass1>{
  public:
    void run(){
      getInstrument()->startTimer(T1::FIRST);
      getInstrument()->setCounter(T1::DOC_TYPE,rand()%10000);
      getInstrument()->setCounter(T1::DOC_ID,rand()%10000);
      for(size_t i=0;i<1000000;i++){
        getInstrument()->incrementCounter(T1::COUNT);
      }
      getInstrument()->stopTimer(T1::FIRST);
    }
  };

  template<> const InstrumentDefinition Instrumented<TestClass1>::getDefinition(){
    return InstrumentDefinition("Test1",T1::FIRST,create_map<int32_t,string>(T1::FIRST,"First"),create_map<int32_t,string>(T1::DOC_TYPE,"Doc Type")(T1::DOC_ID,"Doc Id")(T1::COUNT,"Count"));
  }
  
  class TestClass2 : public Instrumented<TestClass2>{
    public:
      void run(){
        getInstrument()->startTimer(T2::FIRST);
        getInstrument()->startTimer(T2::SECOND);
        for(size_t i=0;i<1000000;i++){
          getInstrument()->incrementCounter(T2::COUNT);
        }
        getInstrument()->stopTimer(T2::SECOND);
        getInstrument()->stopTimer(T2::FIRST);
      }
  };
  
  template<> const InstrumentDefinition Instrumented<TestClass2>::getDefinition(){
    return InstrumentDefinition("Test2",T2::SECOND,create_map<int32_t,string>(T2::FIRST,"First")(T2::SECOND,"Second"),create_map<int32_t,string>(T2::COUNT,"Count"));
  }
}

TEST_F(InstrumentationTest,RegistryTest){
  InstrumentGroup batch("test",20,20);
  for (size_t i=0;i<30;i++){
    TestClass1 test1;
    TestClass2 test2;
    test1.run();
    test2.run();
    batch.add(test1.getInstrument());
    batch.add(test2.getInstrument());
  }
  stringstream s;
  s << batch;
  EXPECT_TRUE(s.str().find("Average"));
  EXPECT_TRUE(s.str().find("Total"));
}

TEST_F(InstrumentationTest,InstrumentedTest){
  InstrumentGroup batch("test",20,20);
  for (size_t i=0;i<30;i++){
    TestClass1 test1;
    TestClass2 test2;
    test1.run();
    test2.run();
    batch.add(test1.getInstrument());
    batch.add(test2.getInstrument());
  }
  stringstream s;
  s << batch;
  EXPECT_TRUE(s.str().find("Average"));
  EXPECT_TRUE(s.str().find("Total"));
}

TEST_F(InstrumentationTest,AssociationTest){
  EXPECT_CALL(registry_,getWindowSize())
    .WillRepeatedly(Return(20));
  EXPECT_CALL(registry_,getWhiteSpaceHash(false))
    .WillRepeatedly(Return(WhiteSpaceHash(20)));
  EXPECT_CALL(registry_,getMaxPostingThreshold())
    .WillRepeatedly(Return(100));  
  TestDocument bible("fixtures/gutenberg/Religious/bible.txt");
  TestDocument koran("fixtures/gutenberg/Religious/koran.txt");
  DocumentPtr doc1 = registry_.getDocumentManager()->createTemporaryDocument(bible.getFormText());
  DocumentPtr doc2 = registry_.getDocumentManager()->createTemporaryDocument(koran.getFormText());
  Association association1(&registry_,doc1,doc2);
  Association association2(&registry_,doc2,doc1);
  association1.match();
  association2.match();
  stringstream s;
  s << *association1.getInstrument();
  EXPECT_TRUE(s.str().find("Average"));
  EXPECT_TRUE(s.str().find("Total"));
  s.clear();
  s << *association2.getInstrument();
  EXPECT_TRUE(s.str().find("Average"));
  EXPECT_TRUE(s.str().find("Total"));
}