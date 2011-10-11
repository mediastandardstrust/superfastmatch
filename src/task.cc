#include <task.h>
#include <posting.h>

namespace superfastmatch
{

  // -------------------
  // TaskPayload members
  // -------------------

  TaskPayload::TaskPayload(DocumentPtr document,TaskOperation operation,uint32_t slots):
  document_(document),operation_(operation),slots_left_(slots)
  {}

  TaskPayload::~TaskPayload(){}
  
  uint64_t TaskPayload::markSlotFinished(){
    uint64_t o_slots=slots_left_;
    do{
      o_slots=slots_left_;
    }while(!slots_left_.cas(o_slots,o_slots-1));
    return o_slots;
  }

  TaskPayload::TaskOperation TaskPayload::getTaskOperation(){
    return operation_;
  }

  DocumentPtr TaskPayload::getDocument(){
    return document_;
  }

  // -------------------
  // PostingTask members
  // -------------------

  PostingTask::PostingTask(PostingSlot* slot,TaskPayload* payload):
  slot_(slot),payload_(payload)
  {}

  PostingTask::~PostingTask(){
    // The task is complete so delete the payload
    if (payload_->markSlotFinished()==1){
      delete payload_;
    }
  }

  PostingSlot* PostingTask::getSlot(){
    return slot_;
  }

  TaskPayload* PostingTask::getPayload(){
    return payload_;
  }

  // ------------------------
  // PostingTaskQueue members
  // ------------------------

  PostingTaskQueue::PostingTaskQueue(){}

  void PostingTaskQueue::do_task(Task* task) {
    PostingTask* ptask = (PostingTask*)task;
    DocumentPtr doc = ptask->getPayload()->getDocument();
    PostingSlot* slot = ptask->getSlot();
    slot->alterIndex(doc,ptask->getPayload()->getTaskOperation());
    delete ptask;
  }
}
