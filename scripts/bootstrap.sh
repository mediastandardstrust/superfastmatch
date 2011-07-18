curl http://google-ctemplate.googlecode.com/files/ctemplate-0.99.tar.gz -o external/ctemplate-0.99.tar.gz
curl http://google-perftools.googlecode.com/files/google-perftools-1.8.tar.gz -o external/google-perftools-1.8.tar.gz
curl http://fallabs.com/kyotocabinet/pkg/kyotocabinet-1.2.68.tar.gz -o external/kyotocabinet-1.2.68.tar.gz
curl http://fallabs.com/kyototycoon/pkg/kyototycoon-0.9.49.tar.gz -o external/kyototycoon-0.9.49.tar.gz
svn checkout http://google-sparsehash.googlecode.com/svn/trunk/ external/google-sparsehash

cd external && for i in *.tar.gz; do tar xzvf $i; done