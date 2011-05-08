kt = __kyototycoon__
db = kt.db
encoding = "IC"

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

function unpack(s)
   if s~=nil and s~='' then
      return kt.unpack(string.rep(encoding,string.len(s)/5),s)
   else
      return {}
   end
end

function pack(t)
   if next(t) ~= nil then
      return kt.pack(string.rep(encoding,(#t/2)),t)
   else
      return ''
   end
end

function get_doc_types(dt)
   doc_types={}
   for _,doc_type in ipairs(kt.split(dt,",")) do
      table.insert(doc_types,tonumber(doc_type))
   end
   return doc_types
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

function update(inmap,outmap)
   local action=inmap.action
   local start_time=kt.time()
   local doc_id=tonumber(inmap.doc_id)
   local doc_type=tonumber(inmap.doc_type)
   local keys,sections,length=hash(inmap)   
   local hash_time=kt.time()
   local new=0
   local deleted=0
   for i,key in ipairs(keys) do
      local value=db:get(key)
      local docs=unpack(value)
      if action=="add" then
         local exists=false
         for j=1,#docs,2 do
            if docs[j]==doc_id and docs[j+1]==doc_type then
               exists=true
            end
         end
         kt.log("debug","Adding: "..sections[i]..":"..(value and #value or 0))
         new=new+1
         table.insert(docs,doc_id)
         table.insert(docs,doc_type)
         if not db:set(key,pack(docs)) then
             kt.log("debug","Insert fail")
         end
      elseif action=="delete" then
         local new_docs={}
         for j=1,#docs,2 do
            if docs[j]~=doc_id and docs[j+1]~=doc_type then
               table.insert(new_docs,doc_id)
               table.insert(new_docs,doc_type)
            end
         end
         if #docs~=#new_docs then
            kt.log("debug","Deleting: "..sections[i]..":"..(value and #value or 0))
            deleted=deleted+1
            if not db:set(key,pack(new_docs)) then
                kt.log("debug","Delete fail")
            end
         end
      end
   end
   outmap['new']=new
   outmap['deleted']=deleted
   outmap['existing']=(#keys-new)
   kt.log("info",string.format("Update Response Time: %.8f secs Hash Time: %.8f secs Text Length: %d Document Id: %d Document Type: %d Added: %d Deleted: %d",kt.time()-start_time,hash_time-start_time,length,doc_id,doc_type,new,deleted))
   return kt.RVSUCCESS
end

function search(inmap,outmap)
   local start_time=kt.time()
   local doc_types=inmap.doc_types and get_doc_types(inmap.doc_types) or {}
   local min_threshold=inmap.min_threshold and tonumber(inmap.min_threshold) or 1
   local max_threshold=inmap.max_threshold and tonumber(inmap.max_threshold) or 999999999
   local num_results=inmap.num_results and tonumber(inmap.num_results) or 20
   local keys,sections,length=hash(inmap)
   local hash_time=kt.time()
   local results={}
   for i,key in ipairs(keys) do 
      local value = db:get(key)
      kt.log("debug","Searching for: "..sections[i]..":"..(value and #value or 0))
      if value and (#value<max_threshold) then -- TODO Need to realise 5 bytes per document
         table.insert(results,value) 
      end
   end
   local query_time=kt.time()
   local result = table.concat(results)
   local concat_time=kt.time()
   local values = unpack(result)
   local unpack_time=kt.time()
   -- Collate scores
   local scores={}
   for i=1,#values,2 do
      local doc_id=values[i]
      local doc_type=values[i+1]
      if scores[doc_type]==nil then
         scores[doc_type]={}
      end
      scores[doc_type][doc_id]=(scores[doc_type][doc_id] or 0)+1
   end
   -- Filter out below min_threshold docs and ignored doc_types
   local threshold_time=kt.time()
   local high_scores={}
   for doc_type,docs in pairs(scores) do
      for doc_id,score in pairs(docs) do
         if score>=min_threshold then
            if (#doc_types==0) or table.contains(doc_types,doc_type) then 
               table.insert(high_scores,{score=score,doc_id=doc_id,doc_type=doc_type})
            end
         end
      end
   end
   -- Sort docs by score
   table.sort(high_scores,function(a,b) return a.score < b.score end)
   -- Build results respecting num_results 
   local result_count={}
   local counter=0
   for _,doc in ipairs(high_scores) do
      result_count[doc.doc_type]=(result_count[doc.doc_type] or 0)+1
      if result_count[doc.doc_type]<=num_results then
         outmap[doc.doc_type..':'..doc.doc_id]=doc.score
         counter=counter+1
      end
   end
   for k,v in pairs(outmap) do
      kt.log("debug",k..":"..v)
   end
   kt.log("debug",string.format("Hash Time: %.8f Query Time: %.8f Concat Time: %.8f Unpack Time: %.8f Threshold Time: %.8f Build Time: %.8f",hash_time-start_time,query_time-hash_time,concat_time-query_time,unpack_time-concat_time,threshold_time-unpack_time,kt.time()-threshold_time))
   kt.log("info",string.format("Search Response Time: %.8f secs Text Length: %d Results: %d Values: %d",kt.time()-start_time,length,counter,#values))
   return kt.RVSUCCESS
end

