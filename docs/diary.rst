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






