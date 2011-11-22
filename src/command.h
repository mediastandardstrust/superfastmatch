#ifndef _SFMCOMMAND_H
#define _SFMCOMMAND_H

#include <registry.h>
#include <query.h>
#include <search.h>
#include <document.h>
#include <task.h>

namespace superfastmatch{
  // Forward declaration
  class QueueManager;

  enum CommandAction{
     AddDocument=1,
     AddAssociation,
     AddAssociations,
     DropDocument,
     DropAssociation,
     NullAction
   };

   enum CommandStatus{
     Finished=1,
     Failed,
     Active,
     Queued
   };

   class Command : public std::tr1::enable_shared_from_this<Command>
   {
   friend class QueueManager;
   public:
     typedef bool(Command::*Method)();
     struct ActionDetail{
       uint32_t priority;
       Method method;
       string name;
       ActionDetail(){};
       ActionDetail(const uint32_t priority, const Method method,const string name):
       priority(priority),
       method(method),
       name(name){};
     };
     typedef map<CommandAction,Command::ActionDetail> action_map;
     typedef map<CommandStatus,string> status_map;
   private:
     Registry* registry_;
     uint64_t queue_id_;
     uint64_t payload_id_;
     uint32_t priority_;
     CommandStatus status_;
     CommandAction action_;
     uint32_t doc_type_;
     uint32_t doc_id_;
     string* payload_;

     static const char* key_format;
     static const char* cursor_format;
     static const action_map actions;
     static const status_map statuses;

     // Command methods
     bool addDocument();
     bool addAssociation();
     bool addAssociations();
     bool dropDocument();

     bool save();
     bool changeStatus(CommandStatus status);

   public:
     ~Command();
     bool execute();
     string getKey();
     uint32_t getDocType();
     uint32_t getDocId();
     uint64_t getQueueId();
     uint64_t getPayloadId();
     string& getPayload();
     CommandAction getAction();
     CommandStatus getStatus();
     void fillDictionary(TemplateDictionary* dict);

   private:
     Command(Registry* registry,const CommandAction action,const uint64_t queue_id,const uint64_t payload_id,const uint32_t doc_type,const uint32_t doc_id,const string& payload);
     Command(Registry* registry,const string& key,const string& value);
     DISALLOW_COPY_AND_ASSIGN(Command);
   };
}

#endif