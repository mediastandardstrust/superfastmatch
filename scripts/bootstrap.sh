mkdir external
unamestr=`uname`
if [[ "$unamestr" == 'Linux' ]]; then
	curl -L http://download.savannah.gnu.org/releases/libunwind/libunwind-1.0.1.tar.gz -o external/libunwind-1.0.1.tar.gz
fi
curl http://google-ctemplate.googlecode.com/files/ctemplate-1.0.tar.gz -o external/ctemplate-1.0.tar.gz
curl http://google-perftools.googlecode.com/files/google-perftools-1.8.3.tar.gz -o external/google-perftools-1.8.3.tar.gz
curl http://google-gflags.googlecode.com/files/gflags-1.6.tar.gz -o external/gflags-1.6.tar.gz
curl http://fallabs.com/kyotocabinet/pkg/kyotocabinet-1.2.70.tar.gz -o external/kyotocabinet-1.2.70.tar.gz
curl http://fallabs.com/kyototycoon/pkg/kyototycoon-0.9.51.tar.gz -o external/kyototycoon-0.9.51.tar.gz
curl http://www.oberhumer.com/opensource/lzo/download/lzo-2.06.tar.gz -o external/lzo-2.06.tar.gz
svn checkout http://google-sparsehash.googlecode.com/svn/trunk/ external/google-sparsehash

cd external && for i in *.tar.gz; do tar xzvf $i; done
if [[ "$unamestr" == 'Linux' ]]; then
	cd libunwind* && CFLAGS=-U_FORTIFY_SOURCE ./configure && make && sudo make install && cd ..
fi
cd ctemplate* && ./configure && make && sudo make install && cd ..
cd google-perftools* && ./configure && make && sudo make install  && cd ..
cd gflags* && ./configure && make && sudo make install  && cd ..
cd kyotocabinet* && ./configure && make && sudo make install && cd ..
cd kyototycoon* && ./configure && make && sudo make install && cd ..
cd google-sparsehash* && ./configure && make && sudo make install && cd ..
cd lzo-2.06 && ./configure && make && sudo make install && cd ..
cd ..
