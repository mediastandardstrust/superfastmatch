#ifndef _SFMCOMMAND_H
#define _SFMCOMMAND_H

#include <registry.h>
#include <document.h>

namespace superfastmatch{
	enum CommandType{
		Invalid,
		AddDocument,
		AddAssociation,
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
		const Registry& registry_;
		uint64_t queue_id_;
		uint32_t priority_;
		CommandType type_;
		CommandStatus status_;
		uint32_t doc_type_;
		uint32_t doc_id_;
		string payload_;
		Document* document_;

		static const char* key_format;

	public:	
		Command(const Registry& registry,string& key,string& payload);
		Command(const Registry& registry,const uint64_t queue_id, const uint32_t priority,const CommandType type,const CommandStatus status, const uint32_t doc_type,const uint32_t doc_id,const string& payload);
		~Command();
		
	private:
		string command_key();
		bool save(bool keep_payload=true);
		
	public:
		
		CommandType getType();
		CommandStatus getStatus();
		uint32_t getDocType();
		uint32_t getDocId();
		uint64_t getQueueId();
		bool setActive();
		bool setFinished();
		bool setFailed();
		bool remove();
		Document* getDocument();
		
		friend std::ostream& operator<< (std::ostream& stream, Command& command);
	};
	
	class CommandFactory{
	private:
		static Command* createCommand(const Registry& registry, const CommandType commandType,const uint64_t queue_id,const uint32_t doc_type, const uint32_t doc_id,const string& payload);
	public:
		static uint64_t addDocument(const Registry& registry_,const uint32_t doc_type, const uint32_t doc_id,const string& content,const bool associate);
		static uint64_t dropDocument(const Registry& registry_,const uint32_t doc_type, const uint32_t doc_id);
		static void insertDropDocument(const Registry& registry_,Command* command);
		static void getAllCommands(const Registry& registry_,vector<Command*>& commands);
		static bool getNextBatch(const Registry& registry_,deque<Command*>& batch,CommandType& batchType);
	};
}

#endif