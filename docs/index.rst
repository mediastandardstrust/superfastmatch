Introduction
============
SuperFastMatch is a project which began life as a piece of research and development in the production of `Churnalism.com <http://churnalism.com/>`_ in 2010/2011. A means was required of comparing a given text with over 3 million news articles in the `Journalisted.com <http://journalisted.com/>`_ archive. The idea was to find the articles which had the longest sections of common text and to then rank them according to their respective percentages of reuse. 

Initially, we looked at existing search engines like `Xapian <http://xapian.org>`_ and `Lucene <http://lucene.apache.org/>`_ and although being very effective at finding articles containing the same words, they weren't particulary good at finding the articles with the longest common strings. More info on this can be found `here <http://mediastandardstrust.org/blog/the-technology-driving-churnalism-com/>`_.

Contents:
=========

.. toctree::
    :maxdepth: 2
    
    install.rst
    developing.rst
    architecture.rst
    api.rst

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

