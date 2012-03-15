#include "worker.h"

namespace superfastmatch{
  Worker::Worker(Registry* registry):
  registry_(registry),
  api_(registry)
  { }

  void Worker::process_idle(HTTPServer* serv) {
    // serv->log(Logger::INFO,"Idle");
  }
      
  void Worker::process_timer(HTTPServer* serv) {
    if(!registry_->getPostings()->isReady()){
      registry_->getPostings()->init();
    }
    size_t count=registry_->getQueueManager()->processQueue();
    if(count>0){ 
      stringstream s;
      s << "Finished processing "<< count << " items in command queue";
      registry_->getLogger()->log(Logger::INFO,&s);
    };
  }

  int32_t Worker::process(HTTPServer* serv, HTTPServer::Session* sess,
                    const string& path, HTTPClient::Method method,
                    const map<string, string>& reqheads,
                    const string& reqbody,
                    map<string, string>& resheads,
                    string& resbody,
                    const map<string, string>& misc) 
  {
    double start = kyotocabinet::time();
    int32_t code=api_.Invoke(path,method,reqheads,reqbody,resheads,resbody,misc);
    if (code==-1){
      code=process_static(path,method,reqheads,reqbody,resheads,resbody,misc);
    }
    stringstream time;
    time << setiosflags(ios::fixed) << setprecision(4) << kyotocabinet::time()-start << " secs";
    resheads["X-response-time"] = time.str();
    return code;
  }
  
  static const char* media_type(const std::string& url) {
    static const char* types[] = {
      "txt", "text/plain", "text", "text/plain", "asc", "text/plain",
      "js", "application/javascript","css","text/css",
      "html", "text/html", "htm", "text/html",
      "xml", "application/xml",
      "xhtml", "application/xml+xhtml",
      "gz", "application/x-gzip",
      "bz2", "application/x-bzip2",
      "zip", "application/zip",
      "pdf", "application/pdf",
      "png", "image/png",
      "jpg", "image/jpeg", "jpeg", "image/jpeg",
      "gif", "image/gif",
      NULL
    };
    const char* rp = url.c_str();
    const char* pv = std::strrchr(rp, '/');
    if (pv) rp = pv + 1;
    pv = std::strrchr(rp, '.');
    if (pv) {
      rp = pv + 1;
      for (int32_t i = 0; types[i]; i += 2) {
        if (!kc::stricmp(rp, types[i])) return types[i+1];
      }
    }
    return NULL;
  }
  
  // Lifted from ktuilserv.cc in Kyoto Tycoon
  int32_t Worker::process_static(const string& path,
                                 const HTTPClient::Method method,
                                 const map<string,string>& reqheads,
                                 const string& reqbody,
                                 map<string,string>& resheads,
                                 string& resbody,
                                 const map<string,string>& misc){
    int32_t code=200;
    const std::string& lpath = kt::HTTPServer::localize_path(path);
    std::string apath = registry_->getPublicPath() + (lpath.empty() ? "" : kc::File::PATHSTR) + lpath;
    bool dir = kc::strbwm(misc.find("url")->second.c_str(), "/");
    if (method == kt::HTTPClient::MGET) {
      kc::File::Status sbuf;
      if (kc::File::status(apath, &sbuf)) {
        if (dir && sbuf.isdir) {
          const std::string& ipath = apath + kc::File::PATHSTR + "index.html";
          if (kc::File::status(ipath,&sbuf)) {
            apath = ipath;
          }
        }
        if (sbuf.isdir) {
          code = 403;
        } else {
          map<string,string>::const_iterator etag=reqheads.find("if-none-match");
          if ((etag!=reqheads.end())&&(etag->second.compare(toString(sbuf.mtime))==0)){
            code=304;
          }else{
            int64_t size;
            char* buf = kc::File::read_file(apath, &size, 256LL << 20);
            if (buf) {
              code = 200;
              resheads["ETag"]=toString(sbuf.mtime);
              const char* type = media_type(apath);
              if (type) resheads["content-type"] = type;
              resbody.append(buf, size);
              delete[] buf;
            } else {
              code = 403;
            } 
          }
        }
      } else {
        code = 404;
      }
    } else {
      code = 403;
    }
    if (code!=200){
      resheads["content-type"] = "text/plain";
      kc::strprintf(&resbody, "%s\n", kt::HTTPServer::status_name(code));
    }
  return code;
  }
}
