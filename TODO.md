TODO
----
* Accurate total documents per doctype and filter. Will allow clients to accurately page through filtered sets of documents.
* Additional automatic metadata, such as number of associated documents, longest fragment, average/median fragment length. Will allow documents with most significant "churn" to be found more quickly.
* Memory index rotation. If a corpus is bigger than available memory, this will allow a range of doctypes to be loaded into memory selectively. Conversely a delete operation will be avaiable for removing a range of doctypes from memory.
* Improve search speed. A lot of work went into implementing the instrumentation to measure search speed, but was never used for optimisation. There are plenty of things that can be done to speed things thing up, such as caching the hashing results of a document and putting in more shortcut escapes from the inner loops.
* Implement Result caching. A search text that comes up frequently, such as the first article on the NY Times home page, will be queried repeatedly. The Churnalism frontend caches this currently, but it might be better located in SFM itself, using a signature hash as a key to a cache.
* Create a Fragment class. This would effectively invert the index, so that for every common phrase found, you could see in which documents it occurs. Visualisations would then be possible showing, for instance the longest common phrases in Wikipedia, or journalism itself. The API would be extended to allow for paging through the phrases.
* Investigate k-means clustering. There are frequently clusters of phrases in two matching documents that are close together in each. These clusters can be grouped together in a block and give a clearer indication of the shuffling that has occurred between the two documents. This would be added to the results response with a "clusters" JSON object.
* Use ```sparsetable<char[BLOCK]>``` rather than ```sparsetable<char*>```. This would allow for a vast reduction in the memory usage of the index and therefore allow wider hash widths to be used, thereby increasing search speed.
* Full curl test of REST API using python-superfastmatch. Would be a good self-documenting example of how to quickly get up and running with SFM.
* Implement unicode using ICU. Not sure this is necessary currently, but could be useful if non-English users ask for specific features.
* Release Debian packages for easy deployment and stable versioning.
* Improve Web UI:
  * Juxta has a great UI for examining side by side comparisons and is open source. Perhaps this could be adopted for the side-by-side view in SFM.
  * Scatter plots of associations per document (diagonals equals a cluster). 
  * Implement histograms to give a better realtime feel for how full the index is and whether an optimal hash width has been selected.
* Create a walkthrough video of how to use SFM and explain what is being seen in the UI.

DONE
----

* ~~Auto-id per doctype~~
* ~~Cross instance loading~~
* ~~Make association task work for specified doctypes~~
* ~~JSON versions of appropriate templates~~
* ~~Update init.d with correct settings and paths~~
* ~~Make association task multithreaded~~
* ~~Better ordering of search results and fragments within an association~~
* ~~Allow ordering of documents by metadata~~
* ~~Separate queue payload out to speed up queue page and refactor Command to be polymorphic~~
* ~~Implement search page~~
* ~~Implement association task~~
* ~~Create a DocumentSharedPtr and AssociationSharedPtr and pass them around instead~~
* ~~Write tests for each class~~
* ~~Implement gflags parsing~~
* ~~Add separate meta db to speed up document page~~
* ~~Adopt Google C++ Coding standards (eg, getMethods())~~
* ~~Make tests build faster~~


