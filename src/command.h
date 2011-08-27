#ifndef _SFMCOMMAND_H
#define _SFMCOMMAND_H

#include <registry.h>
#include <document.h>

namespace superfastmatch{
  enum CommandType{
    Invalid,
    AddDocument,
    AddAssociation,
    AddAssociations,
    DropDocument,
    DropAssociation
  };
  
  enum CommandStatus{
    Queued,
    Active,
    Finished,
    Failed
  };
  
  class Command{
  private:
    Registry* registry_;
    uint64_t queue_id_;
    uint32_t priority_;
    CommandType type_;
    CommandStatus status_;
    uint32_t doc_type_;
    uint32_t doc_id_;

    static const char* key_format;

  public: 
    Command(Registry* registry,const string& key);
    Command(Registry* registry,const uint64_t queue_id, const uint32_t priority,const CommandType type,const CommandStatus status, const uint32_t doc_type,const uint32_t doc_id,const string& payload);
    ~Command();
    
  private:
    string command_key();
    bool update();
    bool remove();
    bool getPayload(string* payload);
    
  public:
    
    CommandType getType();
    CommandStatus getStatus();
    uint32_t getDocType();
    uint32_t getDocId();
    uint64_t getQueueId();
    bool setFinished();
    bool setFailed();
    
    DocumentPtr getDocument();
    DocumentPtr createDocument();
    
    void fill_list_dictionary(TemplateDictionary* dict);
  };
  
  class CommandFactory{
  private:
    static Command* createCommand(Registry* registry, const CommandType commandType,const uint64_t queue_id,const uint32_t doc_type, const uint32_t doc_id,const string& payload);
  public:
    static uint64_t addDocument(Registry* registry_,const uint32_t doc_type, const uint32_t doc_id,const string& content,const bool associate);
    static uint64_t dropDocument(Registry* registry_,const uint32_t doc_type, const uint32_t doc_id);
    static void insertDropDocument(Registry* registry_,Command* command);
    static uint64_t addAssociations(Registry* registry_,const uint32_t doc_type);
    static void insertAddAssociation(Registry* registry_,const uint32_t doc_type, const uint32_t doc_id,Command* command);
    static void getAllCommands(Registry* registry_,vector<Command*>& commands);
    static bool getNextBatch(Registry* registry_,deque<Command*>& batch,CommandType& batchType);
  };
}

#endif