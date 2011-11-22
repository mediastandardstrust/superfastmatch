#include <task.h>
#include <posting.h>
#include <document.h>
#include <association.h>

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
  
  // -----------------------
  // AssociationTask members
  // -----------------------
  
  AssociationTask::AssociationTask(const DocPair pair):
  pair_(pair)
  {}
  
  // ----------------------------
  // AssociationTaskQueue members
  // ----------------------------

  AssociationTaskQueue::AssociationTaskQueue(Registry* registry):
  registry_(registry)
  {}

  void AssociationTaskQueue::do_task(Task* task){
    AssociationTask* ptask = (AssociationTask*)task;
    SearchPtr search=Search::createPermanentSearch(registry_,ptask->pair_.doc_type,ptask->pair_.doc_id);
    registry_->getInstrumentGroup(FlagsRegistry::QUEUE)->merge(search->performance);
    delete ptask;
  }

}
