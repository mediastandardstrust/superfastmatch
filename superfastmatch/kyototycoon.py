import time
import urllib
import httplib
import cStringIO
from  base64 import b64encode
from lib.counter import Counter
from django.conf import settings #TODO work without Django present

class KyotoTycoon:
    def __init__(self,host=settings.INDEX_HOST, port = settings.INDEX_PORT, timeout = 30):
        self.host=host
        self.port=port
        self.timeout=timeout
    
    def open(self):
        self.ua = httplib.HTTPConnection(self.host, self.port, False, self.timeout)

    def close(self):
        self.ua.close()

    def write(self,body,key,value):
        if isinstance(key,int):key=str(key)
        if isinstance(value,int):value=str(value)
        body.write("%s\t%s\n"%(b64encode(key.encode('utf8')),b64encode(value.encode('utf8'))))

    def parse(self,body):
        result={}
        for line in body.split('\n')[:-1]:
            key,value=line.split('\t')
            if key.startswith("_"):
                result[key[1:]]=value.isdigit() and int(value) or value
        return result

    def search(self,text,window_size=15,hash_width=15,min_threshold=1,max_threshold=50000,num_results=20):
        url="/rpc/play_script"
        body=cStringIO.StringIO()
        self.write(body,"name","search")
        self.write(body,"_window_size",window_size)
        self.write(body,"_hash_width",hash_width)
        self.write(body,"_min_threshold",min_threshold)
        self.write(body,"_max_threshold",max_threshold)
        self.write(body,"_text",text)
        headers={"Content-Type": "text/tab-separated-values; colenc=B"}
        self.ua.request("POST",url,body.getvalue(),headers)
        res=self.ua.getresponse()
        result=Counter(self.parse(res.read()))
        if res.status != 200:
            return None
        return result.most_common(num_results)

    def add(self,docid,text,window_size=15,hash_width=15):
        url="/rpc/play_script"
        body=cStringIO.StringIO()
        self.write(body,"name","add")
        self.write(body,"_window_size",window_size)
        self.write(body,"_hash_width",hash_width)
        self.write(body,"_docid",docid)
        self.write(body,"_text",text)
        headers={"Content-Type": "text/tab-separated-values; colenc=B"}
        self.ua.request("POST",url,body.getvalue(),headers)
        res=self.ua.getresponse()
        result=self.parse(res.read())
        if res.status != 200:
            return None
        return result

