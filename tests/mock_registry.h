#include <gmock/gmock.h>

namespace superfastmatch {

class MockRegistry : public Registry {
 public:
  MOCK_CONST_METHOD0(getHashWidth,
      uint32_t());
  MOCK_CONST_METHOD0(getHashMask,
      hash_t());
  MOCK_CONST_METHOD0(getWindowSize,
      uint32_t());
  MOCK_CONST_METHOD0(getWhiteSpaceThreshold,
      uint32_t());
  MOCK_CONST_METHOD1(getWhiteSpaceHash,
      hash_t(bool));
  MOCK_CONST_METHOD0(getThreadCount,
      uint32_t());
  MOCK_CONST_METHOD0(getSlotCount,
      uint32_t());
  MOCK_CONST_METHOD0(getPageSize,
      size_t());
  MOCK_CONST_METHOD0(getNumResults,
      size_t());
  MOCK_CONST_METHOD0(getMaxLineLength,
      size_t());
  MOCK_CONST_METHOD0(getMaxHashCount,
      size_t());
  MOCK_CONST_METHOD0(getMaxBatchCount,
      size_t());
  MOCK_CONST_METHOD0(getMaxPostingThreshold,
      size_t());
  MOCK_CONST_METHOD0(getMaxDistance,
      size_t());
  MOCK_CONST_METHOD0(getTimeout,
      double());
  MOCK_CONST_METHOD0(getDataPath,
      string());
  MOCK_CONST_METHOD0(getAddress,
      string());
  MOCK_CONST_METHOD0(getPort,
      uint32_t());
  MOCK_METHOD0(getMode,
      uint32_t());
  MOCK_METHOD0(getQueueDB,
      kc::PolyDB*());
  MOCK_METHOD0(getPayloadDB,
      kc::PolyDB*());
  MOCK_METHOD0(getDocumentDB,
      kc::PolyDB*());
  MOCK_METHOD0(getMetaDB,
      kc::PolyDB*());
  MOCK_METHOD0(getAssociationDB,
      kc::PolyDB*());
  MOCK_METHOD0(getMiscDB,
      kc::PolyDB*());
  MOCK_METHOD0(getTemplateCache,
      TemplateCache*());
  MOCK_METHOD0(getLogger,
      Logger*());
  MOCK_METHOD0(getPostings,
      Posting*());
  MOCK_METHOD0(getDocumentManager,
      DocumentManager*());
  MOCK_METHOD1(fill_status_dictionary,
      void(TemplateDictionary* dict));
};

}  // namespace superfastmatch