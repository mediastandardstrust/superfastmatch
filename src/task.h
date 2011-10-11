#ifndef _SFMTASK_H                       // duplication check
#define _SFMTASK_H

#include <common.h>
#include <kcthread.h>

namespace superfastmatch
{
  class PostingSlot;
    
  class TaskPayload{
  public:
    enum TaskOperation{
      AddDocument,
      DeleteDocument
    };
  private:
    DocumentPtr document_;
    TaskOperation operation_;
    kc::AtomicInt64 slots_left_;
  public:
    TaskPayload(DocumentPtr document,TaskOperation operation,uint32_t slots);
    ~TaskPayload();

    uint64_t markSlotFinished();
    TaskOperation getTaskOperation();
    DocumentPtr getDocument();
  };

  class PostingTaskQueue : public kc::TaskQueue{
  public:
    explicit PostingTaskQueue();
  private:
    void do_task(Task* task);
  };

  class PostingTask : public kc::TaskQueue::Task{
  private:
    PostingSlot* slot_;
    TaskPayload* payload_;
  public:
    explicit PostingTask(PostingSlot* slot,TaskPayload* payload);
    ~PostingTask();

    PostingSlot* getSlot();
    TaskPayload* getPayload();
  }; 
}
#endif
