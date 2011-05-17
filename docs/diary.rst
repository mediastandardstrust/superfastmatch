Developer Diary
===============

The open-souring of SuperFastMatch has been generously funded by the `Sunlight Foundation <http://sunlightfoundation.com/>`_ and as part of the agreement I will keep a diary of the project's progress here:

Week 1
------

This is my first attempt at a truly open-source project so I've had a lot to learn about making software accessible by more than just the development team I am working with. SuperFastMatch originally started as the algorithm, developed over many sleepless nights, for comparing press releases with newspaper articles used by `Churnalism.com <http://churnalism.com>`_. This was a single use-case, but as the site became known to the public, other use-cases became apparent. One is to track legislation for lobbyist influence. Others might include tracking plagiarism in fiction or making sure academic texts have correct citations.

The point is that when more than one use-case comes up, a piece of code needs to develop into a framework which is easy to understand, is well documented and is easily integrated and installable in concert with other software. Seeing as the majority of the existing code is written in `Python <http://python.org>`_ it made sense to look at how some other Python projects made use of existing infrastructure to meet these ends. I examined `Celery <http://github.com/ask/django-celery>`_, a distributed job engine and tracker, as I was familiar with it from previous work, and saw how it made use of tools to assist its open source development. Below are some ideas and tools that I borrowed from it:

* `Github <http://github.com>`_ has become the de facto platform for sharing and distributing code. It's free for open source projects and is built on top of `git <http://git-scm.com/>`_, which has many advantages over previous generations of source code control software. It also offers static file hosting, from where you are probably reading this documentation!

* `Pypi <http://pypi.python.org/pypi>`_ is a centralised index of Python packages that allows for the versioning and sharing of Python software without the need to remember lots of urls and is reliable enough to be referred to in bootstrap scripts.

* `Sphinx <http://sphinx.pocoo.org/>`_ is a documentation generator that works well with Python packages as well as C++ (and hopefully Lua!). This page and the rest of the docs are written in `reStructured Text <http://docutils.sourceforge.net/rst.html>`_ and then passed throught a batch process which converts them into HTML, after which they are uploaded to the static Github hosting.

* `Paver <http://paver.github.com/paver/>`_ is a combination of tools which allows for the scripting in Python of many development workflows such as bootstrapping, documentation generation, packaging and test execution. I'm used to using `Fabric <http://fabfile.org/>`_ for deployment, and Paver seems to offer a similar experience for development. It will be interesting to see if I integrate some of the C++ build process into the `pavement.py <https://github.com/mediastandardstrust/superfastmatch/blob/master/pavement.py>`_ file.

* `VirtualEnv <http://pypi.python.org/pypi/virtualenv>`_ is an essential tool for isolating Python packages into an application-specific environment. This ensures you always know which versions of software are present and can accurately recreate this when you deploy to either a production or staging server. It comes with `Pip <http://www.pip-installer.org/>`_ installed in the created environment for easy integration with Pypi.

* `Django <http://www.djangoproject.com/>`_ is the platform on which `Churnalism.com <http://churnalism.com>`_ is built and is widely used so it seemed like the natural choice for building an example usage of SuperFastMatch.

Week 2
------

Now that all the open source infrastructure is in place it's time to start learning a bit more about the use-case in hand, US Congress and state legislation. 

Legislature
'''''''''''

The US Congress has two chambers, both elected, the House of Representatives and the Senate. Both chambers can introduce bills and the bill can take one of many paths through both chambers. The passage of a bill through Congress appears to be quite undefined with a whole series of possible routes and referrals being possible before an Act is signed by the President. 

The end result of acts of Congress is the `US Code <http://www.gpo.gov/fdsys/browse/collectionUScode.action?collectionCode=USCODE>`_ which is an up to date record of all enrolled bills. States in the US are also capable of passing their own local laws.

Data Access
'''''''''''

The US has a well maintained `digital archive <http://www.gpo.gov/fdsys/browse/collection.action?collectionCode=BILLS>`_ of all House of Representative Bills from 2004, and a more recent archive of Senate Bills. The data is accessible in XML form which allows for metadata extraction of Bill information and easy manipulation of the Bill text itself, ideal for the purposes of this project. Text-only bills are available from 1993 onwards.

Week 3
------

Framework design
''''''''''''''''

The aim of the project is to design a reusable tool for bulk text comparison and analysis. To succeed in becoming reusable, at least two use-cases have to be considered and a clean, easy to understand interface for the user has to be designed. `Django <http://www.djangoproject.com/>`_ offers many ways of designing models for data storage, and this flexibility is useful - but there can be a number of caveats that can obstruct progress further down the line.

`Model Inheritance <http://docs.djangoproject.com/en/dev/topics/db/models/#model-inheritance>`_ allows for the subclassing of a Model which itsef permits further extension. This is an ideal pattern for different types of Document. Say that you have Press Releases and News Articles, or State Law and Congress Law, all with potentially similar content. When you search, you might not know what to expect as results and would like to search all Document types simultaneously. By defining a Document base model, this is possible because it can be assumed that the necessary content in it's indexable form is present. 

With Django, there is a choice of either abstract or multi-table inheritance (and also proxy...). Multi-table inheritance results in an extra table, while abstract inheritance just adds fields to the subclasses' tables. The simplest choice is abstract inheritance and I went for this. However, the disadvantage is that you cannot define a ForeignKey to the abstract base class itself but the related clean indexed content needs to be stored somewhere as a cache so that updates can be detected. To solve this issue I turned to `content types <http://docs.djangoproject.com/en/dev/ref/contrib/contenttypes/>`_, which makes it possible to relate any model instance to any other model instance, regardless of type. This means that the cleaned Content can be related to any of Document's subclasses. At this point it's  probably worth looking at the `model definition <https://github.com/mediastandardstrust/superfastmatch/blob/master/superfastmatch/django/models.py>`_.

Faceting
''''''''

One of the limitations of the original Churnalism code is the inability to search for news articles, only press releases. However, both document types have very similar features, such as date published, an author, a publisher and of course the content itself. Therefore it makes sense to also be able to search for news articles with either a news article, press release or other text as the input text. This would be useful for journalists to check for plagiarism and is useful in lots of other contexts.

The challenge is that the script that builds the search results needs a limit on the number of results to return to make the data and server load manageable. However, imagine searching for both news articles and press releases with a limit of 20 results. It might be the case that 20 news articles have higher ranking than matching press releases. The press releases might be a more interesting result though, so excluding them could omit valuable data. The solution to this is `faceted search <http://en.wikipedia.org/wiki/Faceted_search>`_ where extra data, in this case the document type is stored along with the document id in the index. The number of results returned is per document type, and therefore useful data should not be missed. This has implications for total index disk space usage, but is a truly useful addition to the capabilities of SuperFastMatch.

Other fields, such as published date, could be used as filter, but then we are entering the territory of advanced search engines like `Xapian <http://xapian.org/>`_ and `Solr <http://lucene.apache.org/solr/>`_ which are already good at that type of thing! 

Week 4
------

Kyoto Cabinet
'''''''''''''

Storing and processing data is a challenging task with many, many options! Often the performance characteristics and ceilings of a platform only become apparent after a large amount of implementation for that platform has occurred. These evaluations can take a lot of time and definitely help form an opinion on which platform is good at a particular task. SuperFastMatch works on the simple concept of storing every hash of every window of text in a document with the associated document id (and now document type). This is done for every document and yields an inverted index of hashes to documents. Thus, for a search document or text, hashing its windows allows for a fast search of matching documents. 

The storage requirement for this is very high - typically 5x the original document size. Access speed is very important, given that a search of a document n characters long requires (n-window_size) lookups. Also index creation speed is vital. If it takes 2 days to index 1 days worth of news, Churnalism.com would be behind the current affairs very quickly!

The obvious starting candidate for a platform is SQL, and I experimented with Postgres, testing out such features as the `intarray <http://www.postgresql.org/docs/current/static/intarray.html>`_ data type for storing document ids and `partitioned tables <http://www.postgresql.org/docs/current/static/ddl-partitioning.html>`_ as a means for bulk loading daily data whilst maintaining insert speed. However, even with 24-bit hashing (ie. 16,777,216 possible hashes), lookup and insert speeds proved to be poor.

The next candidate was the sexy new key value store on the block! `Redis <https://github.com/antirez/redis>`_ has lots of nice features ideal for storing the index. High speed inserts and the `sets <http://redis.io/commands#set>`_ command set, great for ensuring no duplicate id is saved for a hash. However, again a performance ceiling reared its skull-damaging surface! Redis operates great when the data set fits totally in memory, but as soon as that limit is surpassed, the paging virtual memory kicks in - but very slowly. The release notes and commit history showed that this was a known issue, but it made Redis impractical for use.

A lateral solution was to consider using `Hadoop <http://hadoop.apache.org/>`_ in the form of `Elastic Map Reduce <http://aws.amazon.com/elasticmapreduce/>`_ as a way of solving the daily index creation problem. It was interesting to stray into the realms of Java land (with its verbosity quite overwhelming at times!). Issues found included the need to code a simple CSVReader - not included in the vanilla distribution. The difficulty of getting something out of Hadoop in binary rather than text file form. The misleading and, in the end, very expensive pricing of both the EC2 service and the S3 costs for downloading the processed data to an external server. I can see that if you are Yahoo and have 1000's of servers, Hadoop is a great way to distribute jobs across many machines. However, if you have a continuous data processing task that could be run on one very powerful machine the economics of it don't quite make sense.

At this point, despair was near! A final gambit seemed to be the service offered by the `search giant <http://www.google.com>`_ themselves. `App Engine <http://code.google.com/appengine/>`_ had a lot of ticks in it's favour. The datastore is capable of massive scaling with no configuration required. API calls to the datastore are very performant (and now can run in `parallel <http://code.google.com/appengine/docs/python/datastore/async.html>`_). The `task queue <http://code.google.com/appengine/docs/python/taskqueue/>`_ is fantastic at allowing extra instances to be pulled up as and when needed for processing the index. All in all, Appengine was the preferred choice and I had a great working prototype. The one major headache was the billing cost for Datastore API writes. Because the data was incoming in unsorted form, each hash for each document required an API write, which added up to about £6,000 just to index 3.5 million news articles! This meant any mistakes in indexing would surely kill the project's budget. A postscript to this might be that the recent addition of the shuffle and reduce phases of the `map-reduce project <http://code.google.com/p/appengine-mapreduce/>`_ might make the insertion costs considerably less. Also the recently `announced <http://www.youtube.com/watch?v=7B7FyU9wW8Y>`_ full-text search API has a numeric data type that could be *misused* to simulate the inverted index.

So after a very large amount of time spent evaluating performance, cost and practical implementation it became clear that was a definite advantage to investing in some serious hardware to negate the higher than expected cost of high performance cloud infrastructure. A server configured by Pete at `Mythic Beasts <http://www.mythic-beasts.com/>`_ with 64GB of RAM and an `Intel X25-M <http://www.intel.com/design/flash/nand/mainstream/index.htm>`_ faciliated the speedy operation of the final solution that we decided upon. 

And that final solution was to use a combination of `Kyoto Cabinet <http://fallabs.com/kyotocabinet/>`_  and `Kyoto Tycoon <http://fallabs.com/kyototycoon/>`_ written by  the talented `Mikio Hirabayashi <http://fallabs.com/mikio/profile.html>`_ (now gainfully employed by Google) who kindly incorporated some feature requests to do with the bit length of `Murmur hashing <http://code.google.com/p/smhasher/>`_ exposed to Lua and gave very useful implementation advice. The pros of Kyoto Cabinet are numerous and include:

* Very fast insertion and lookup speed, whether accessing from disk or memory.
* High tunability of indexes in terms of memory usage and algorithms employed in sorting of keys.
* Embedded Lua scripts can be run in multithreaded HTTP aware server.
* Designed to be used both as a toolkit and a framework for development allowing tighter integration as bottlenecks are encountered, ie.e the library and header files allow for everything to be used in a custom C++ project if desired.
* Great documentation.


Filtering Junk
''''''''''''''

After examining the data for US Congress bills it has become clear that this a far less heterogenous data set than that found with Churnalism.com. For instance phrases like "is amended by adding at the end the following" and "Notwithstanding any other provision of" appear in nearly every bill. These phrases in themselves do not indicate similarity with another bill, but they might if they are part of a longer chunk. So how to ignore the stock phrases when they they are just stock phrases, but include them when they are part of something more unique?

A natural by-product of the index creation, especially as the corpora becomes larger, is that for every window, or in fact hash, there is a sequence of document ids and document types, which are themselves vital for search, but when accumulated together the count per hash gives an indication of the originality or cliché of that window. Say that this sentence is present in both search text and a match candidate:

"The Tobacco Smoker's compensation bill is amended by adding at the end the following"

The last half of the sentence consists of very frequently occurring windows, whilst the start is very likely less common. We want the whole sentence to be given prominence in the fragment results. We can do this by averaging the counts of each hash, perhaps with a high cut-off, say 100, for the high counts. This would allow us to filter out the very common phrases, which would tend to score around 100. This is yet to be implemented, but should be easy to achieve and is scheduled for version 0.4. 

Another possible use of this sort of "heatmap" might be to visualise the originality of texts, with common phrases coloured red perhaps and more original phrases in a cooler tone!
