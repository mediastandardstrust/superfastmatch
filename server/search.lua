kt = __kyototycoon__
db = kt.db

-- log the start-up message
if kt.thid == 0 then
   kt.log("system", "the search script has been loaded")
end

function table.contains(table,element)
   for _,value in pairs(table) do
      if value==element then
         return true
      end
   end
   return false
end

function hash(inmap)
   local text=inmap.text 
   local window_size=inmap.window_size and tonumber(inmap.window_size) or 15
   local hash_width=inmap.hash_width and tonumber(inmap.hash_width) or 32
   local sections={}
   local keys={}
   for i=1,(string.len(text)-window_size+1) do
      local part=string.sub(string.lower(text),i,i+window_size-1)
      local hash=kt.bit("and",kt.hash_murmur(part), 0xFFFFFFFF)
      table.insert(sections,part)
      table.insert(keys,kt.pack('N',{hash}))
   end
   return keys,sections,#text
end

function add(inmap,outmap)
   local start_time=kt.time()
   local docid=tonumber(inmap.docid)
   local keys,sections,length=hash(inmap)   
   local hash_time=kt.time()
   local new=0
   for _,key in pairs(keys) do
      local value=db:get(key)   
      local docs=kt.unpack('I*',value)      
      if not table.contains(docs,docid) then
         new=new+1
         table.insert(docs,docid)
         if not db:set(key,kt.pack('I*',docs)) then
             kt.log("debug","Insert fail")
         end
      end
   end
   outmap['new']=new
   outmap['existing']=(#keys-new)
   kt.log("info",string.format("Add Response Time: %.8f secs Hash Time: %.8f secs Text Length: %d",kt.time()-start_time,hash_time-start_time,length))
   return kt.RVSUCCESS
end

function search(inmap,outmap)
   local start_time=kt.time()
   local min_threshold=inmap.min_threshold and tonumber(inmap.min_threshold) or 1
   local max_threshold=inmap.max_threshold and tonumber(inmap.max_threshold) or 999999999
   local keys,sections,length=hash(inmap)
   local hash_time=kt.time()
   local results={}
   for i,key in ipairs(keys) do 
      local value = db:get(key)
      -- kt.log("debug",sections[i]..":"..(value and #value or 0))
      if value and #value<max_threshold then
         table.insert(results,value) 
      end
   end
   local query_time=kt.time()
   local result = table.concat(results)
   local concat_time=kt.time()
   local values = kt.unpack('I*',result)
   local unpack_time=kt.time()
   local scores={}
   for i,doc in ipairs(values) do
      scores[doc]=(scores[doc] or 0)+1
   end
   local threshold_time=kt.time()
   local counter=0
   for doc,score in pairs(scores) do
      if score>=min_threshold then
         outmap[doc]=score
         counter=counter+1
      end
   end
   kt.log("debug",string.format("Hash Time: %.8f Query Time: %.8f Concat Time: %.8f Unpack Time: %.8f Threshold Time: %.8f Build Time: %.8f",hash_time-start_time,query_time-hash_time,concat_time-query_time,unpack_time-concat_time,threshold_time-unpack_time,kt.time()-threshold_time))
   kt.log("info",string.format("Search Response Time: %.8f secs Text Length: %d Results: %d Values: %d",kt.time()-start_time,length,counter,#values))
   return kt.RVSUCCESS
end

