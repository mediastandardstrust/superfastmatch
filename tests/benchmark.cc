#include <tests.h>
#include <cstdlib>
#include <document.h>
#include <google/dense_hash_map>

TEST(BenchmarkTest,SearchMapTest){
  search_t results;
  for (size_t i=0;i<10000000;i++){
    DocPair pair(rand()%2+1,rand()%10000+1);
    DocTally* tally=&results[pair];
    tally->count++;
    // if ((i%1000)==0){
    //   cout << "Loop: " << i << " Load Factor: " << results.load_factor() << " Count: " << results.size() <<endl; 
    // }
  }
  // for (search_t::const_iterator it=results.begin(),ite=results.end();it!=ite;++it){
  //   size_t bucket=results.bucket(it->first);
  //   cout << "Bucket: " << bucket << "/" << results.bucket_count() <<" Doctype: " << it->first.doc_type << " Docid: " << it->first.doc_id << " Bucket size: " << results.bucket_size(bucket) << endl;
  // }
}
