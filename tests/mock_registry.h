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
      kc::ForestDB*());
  MOCK_METHOD0(getDocumentDB,
      kc::ForestDB*());
  MOCK_METHOD0(getMetaDB,
      kc::ForestDB*());
  MOCK_METHOD0(getHashesDB,
      kc::ForestDB*());
  MOCK_METHOD0(getAssociationDB,
      kc::ForestDB*());
  MOCK_METHOD0(getMiscDB,
      kc::PolyDB*());
  MOCK_METHOD0(getTemplateCache,
      TemplateCache*());
  MOCK_METHOD0(getLogger,
      Logger*());
  MOCK_METHOD0(getPostings,
      Posting*());
  MOCK_METHOD1(fill_status_dictionary,
      void(TemplateDictionary* dict));
};

}  // namespace superfastmatch