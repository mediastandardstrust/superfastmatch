"""This module provides a class to aid communication with a Kyoto Tycoon instance over HTTP and provides an interface for running the simple Lua scripts remotely."""

import time
import urllib
import httplib
import cStringIO
from  base64 import b64encode
from counter import Counter

class KyotoTycoon:
    """Interface to Kyoto Tycoon"""
    
    def __init__(self,host='127.0.0.1', port = 1977, timeout = 30):
        self.host=host
        self.port=port
        self.timeout=timeout
        
    def open(self):
        """Opens HTTP connection"""
        self.ua = httplib.HTTPConnection(self.host, self.port, False, self.timeout)

    def close(self):
        """Closes HTTP connection"""
        self.ua.close()

    def search(self,text,window_size=15,hash_width=15,min_threshold=1,max_threshold=50000,num_results=20):
        """
        Search for specified text in the document index.
        
        :param text: Text to search the index for.
        :param window_size: The length of the window of text from which hashes are created from.
        :param hash_width: The number of bits to use for the hash.
        :param min_threshold: The minimum number of identical hashes for a document to be considered a positive result.
        :param max_threshold: The maximum number of documents to process for a single hash. A hash with a higher number of documents will be ignored.  
        :param num_results: The number of results required.
        """
        url="/rpc/play_script"
        body=cStringIO.StringIO()
        self._write(body,"name","search")
        self._write(body,"_window_size",window_size)
        self._write(body,"_hash_width",hash_width)
        self._write(body,"_min_threshold",min_threshold)
        self._write(body,"_max_threshold",max_threshold)
        self._write(body,"_text",text)
        headers={"Content-Type": "text/tab-separated-values; colenc=B"}
        self.ua.request("POST",url,body.getvalue(),headers)
        res=self.ua.getresponse()
        result=Counter(self._parse(res.read()))
        if res.status != 200:
            return None
        return result.most_common(num_results)

    def add(self,docid,text,window_size=15,hash_width=15):
        """
        Add a document to the index
        
        :param docid: The document id which must be an integer between 0 and 4,294,967,295
        :param text: The text of the document
        :param window_size: The length of the window of text from which hashes are created from.
        :param hash_width: The number of bits to use for the hash.
        
        """
        url="/rpc/play_script"
        body=cStringIO.StringIO()
        self._write(body,"name","add")
        self._write(body,"_window_size",window_size)
        self._write(body,"_hash_width",hash_width)
        self._write(body,"_docid",docid)
        self._write(body,"_text",text)
        headers={"Content-Type": "text/tab-separated-values; colenc=B"}
        self.ua.request("POST",url,body.getvalue(),headers)
        res=self.ua.getresponse()
        result=self._parse(res.read())
        if res.status != 200:
            return None
        return result

    def _write(self,body,key,value):
        if isinstance(key,int):key=str(key)
        if isinstance(value,int):value=str(value)
        body.write("%s\t%s\n"%(b64encode(key.encode('utf8')),b64encode(value.encode('utf8'))))

    def _parse(self,body):
        result={}
        for line in body.split('\n')[:-1]:
            key,value=line.split('\t')
            if key.startswith("_"):
                result[key[1:]]=value.isdigit() and int(value) or value
        return result
