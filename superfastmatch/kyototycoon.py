"""This module provides a class to aid communication with a Kyoto Tycoon instance over HTTP and provides an interface for running the simple Lua scripts remotely."""

import time
import urllib
import httplib
import cStringIO
from  base64 import b64encode
from collections import defaultdict
from superfastmatch.ordereddict import OrderedDict

class KyotoTycoon:
    """Interface to Kyoto Tycoon"""
    
    def __init__(self,host='127.0.0.1', port = 1978, timeout = 2000, debug=False):
        self.host=host
        self.port=port
        self.timeout=timeout
        self.debug=1 if debug else 0
        
    def open(self):
        """Opens HTTP connection"""
        self.ua = httplib.HTTPConnection(host=self.host, port=self.port, strict=False, timeout=self.timeout)

    def close(self):
        """Closes HTTP connection"""
        self.ua.close()

    def search(self,text,doc_types=[],window_size=15,hash_width=32,min_threshold=1,max_threshold=50000,num_results=20):
        """
        Search for specified text in the document index.
        
        :param text: Text to search the index for.
        :param doc_types: Doc types to search for. Specified as a tuple of integers within a range of 0 and 255. If not specified, all doc types are searched for.
        :param window_size: The length of the window of text from which hashes are created from.
        :param hash_width: The number of bits to use for the hash.
        :param min_threshold: The minimum number of identical hashes for a document to be considered a positive result.
        :param max_threshold: The maximum number of documents to process for a single hash. A hash with a higher number of documents will be ignored.  
        :param num_results: The number of results required for each doc type.
        """
        url="/rpc/play_script"
        body=cStringIO.StringIO()
        self._write(body,"name","search")
        self._write(body,"_debug", self.debug)
        self._write(body,"_window_size",window_size)
        self._write(body,"_hash_width",hash_width)
        self._write(body,"_min_threshold",min_threshold)
        self._write(body,"_max_threshold",max_threshold)
        self._write(body,"_text",text)
        self._write(body,"_doc_types",",".join(str(d) for d in doc_types))
        headers={"Content-Type": "text/tab-separated-values; colenc=B"}
        self.ua.request("POST",url,body.getvalue(),headers)
        res=self.ua.getresponse()
        result = defaultdict(OrderedDict)
        raw_results = self._parse(res.read()).items()
        for key,value in raw_results:
            doc_type,doc_id = (int(k) for k in key.split(":"))
            result[doc_type][doc_id]=value        
        if res.status != 200:
            return None
        return result

    def add(self,doc_type,doc_id,text,window_size=15,hash_width=32,verify_exists=True):
        """
        Add a document to the index

        :param doctype: The document type which must be an integer between 0 and 255
        :param docid: The document id which must be an integer between 0 and 4,294,967,295
        :param text: The text of the document
        :param window_size: The length of the window of text from which hashes are created from.
        :param hash_width: The number of bits to use for the hash.        
        """
        return self._update("add",doc_type,doc_id,text,window_size=15,hash_width=32,verify_exists=verify_exists)

    def delete(self,doc_type,doc_id,text,window_size=15,hash_width=32):
        """
        Delete a document from the index
        
        :param doctype: The document type which must be an integer between 0 and 255
        :param docid: The document id which must be an integer between 0 and 4,294,967,295
        :param text: The text of the document
        :param window_size: The length of the window of text from which hashes are created from.
        :param hash_width: The number of bits to use for the hash. 
        """
        return self._update("delete",doc_type,doc_id,text,window_size=15,hash_width=32)
        
    def _update(self,action,doc_type,doc_id,text,window_size=15,hash_width=32,verify_exists=True):
        url="/rpc/play_script"
        body=cStringIO.StringIO()
        self._write(body,"name","update")
        self._write(body,"_debug", self.debug)
        self._write(body,"_action",action)
        self._write(body,"_window_size",window_size)
        self._write(body,"_hash_width",hash_width)
        self._write(body,"_verify_exists",1 if verify_exists else 0)
        self._write(body,"_doc_id",doc_id)
        self._write(body,"_doc_type",doc_type)
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
