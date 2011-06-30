#include "posting.h"

//Is this naughty?
#include "document.h"
#include "registry.h"

namespace superfastmatch
{
	Posting::Posting(const Registry& registry):
	registry_(registry)
	{
		grouper_.resize(registry.max_hash_count);
		
		// Generate masks for each slot
		// Slot 0 is a pass through
		// Slots 1 to slot_count check the MSB of each number
		// Eg. hash width of 16 and slot count of 4:
		// Slot 1: 00 00 00 00 00 00 00 00
		// Slot 2: 01 00 00 00 00 00 00 00
		// Slot 3: 10 00 00 00 00 00 00 00
		// Slot 4: 11 00 00 00 00 00 00 00
		cout << registry.hash_width << ":" << registry.slot_count <<endl;
		// Dummy slot for pass through
		slots_.push_back(0);
		for (uint64_t i=1;i<registry.slot_count;i++){
			slots_.push_back((i<<(registry.hash_width-(registry.slot_count/2))));
		}
		// Stream through the documents with each thread 
		// working with a slot mask
		DocumentCursor* cursor = new DocumentCursor(registry);
		Document* doc;
		while ((doc=cursor->getNext())!=NULL){
			addDocument(doc,1);
			addDocument(doc,2);
			addDocument(doc,3);
			addDocument(doc,4);
			delete doc;
		}
		delete cursor;
	}
	Posting::~Posting(){
		grouper_.clear();
	}

	bool Posting::addDocument(Document* doc, uint32_t slot){
		cout <<"Adding to Posting: " << *doc << " with Slot: " << slot << endl;
		if (histogram_[doc->doctype()].size()==0){
			hist_lock_.lock();
			histogram_[doc->doctype()].resize(registry_.max_line_length);
			histogram_[doc->doctype()][0]=registry_.max_hash_count;
			hist_lock_.unlock();
		}
		doc_count_++;
		hash_t hash;
		uint32_t doc_count;
		for (Document::hashes_vector::const_iterator it=doc->unique_sorted_hashes().begin(),ite=doc->unique_sorted_hashes().end();it!=ite;++it){
			hash = (*it>>registry_.hash_width)^(*it&registry_.hash_mask);
			if ((!slot) || ((hash&slots_[slot])==slots_[slot])){
				// cout << slots_[slot] << ":" << slot << ":" << registry_.hash_width<< ":" << hash << ":"<< grouper_.size() <<endl;
				doc_count=grouper_.mutating_get(hash);
				histogram_[doc->doctype()][doc_count]--;
				histogram_[doc->doctype()][doc_count+1]++;
				grouper_.set(hash,doc_count+1);
				hash_count_++;
			}
		}
		return true;
	}
	
	bool Posting::deleteDocument(Document* doc){
		cout <<"Deleting from Posting: " << *doc << endl;
		return true;
	}
	
	ostream& operator<< (ostream& stream, Posting& posting) {
		posting.hist_lock_.lock();
		// Obviously this will go in ctemplates
	    stream << "<script type=\"text/javascript\">\
		      function drawVisualization() {\
				var data = new google.visualization.DataTable();";
		for (histogram_t::iterator it = posting.histogram_.begin(),ite=posting.histogram_.end();it!=ite;++it){
				stream << "data.addColumn('number', 'Doc type "<< it->first<< "');";
		}
		for (uint32_t i=0;i<500;i++){
			stream << "data.addRow([";
			for (histogram_t::iterator it = posting.histogram_.begin(),ite=posting.histogram_.end();it!=ite;++it){
				stream << it->second[i] <<",";
			}
			stream << " ]);";
		}
		stream << " var chart = new google.visualization.ColumnChart(document.getElementById('visualization'));\
		            chart.draw(data, {width: 1500, height: 400,vAxis: {title: \"Count\",logScale:true},hAxis: {title: \"Docs per hash	\",}});\
 				  	google.visualization.events.addListener(chart, 'onmouseover', barMouseOver);\
				  	google.visualization.events.addListener(chart, 'onmouseout', barMouseOut);\
					function barMouseOver(e) {\
				    	chart.setSelection([e]);\
						document.getElementById(\"chartInfo\").innerHTML=e.row+\" hashes have \"+data.getValue(e.row,e.column)+\" documents\";\
				  	}\
				  	function barMouseOut(e) {\
				    	chart.setSelection([{'row': null, 'column': null}]);\
						document.getElementById(\"chartInfo\").innerHTML=\"\";\
				  	}\
		      }\
		      google.setOnLoadCallback(drawVisualization);\
		    </script>\
			<div id=\"visualization\" style=\"width: 1500px; height: 400px;\"></div>\
			<span id=\"chartInfo\"></span>";
		posting.hist_lock_.unlock();
		return stream;
	}
}