Introduction
============
SuperFastMatch is a project which began life as a piece of research and development in the production of `Churnalism.com <http://churnalism.com/>`_ in 2010/2011. A means was required of comparing a given text with over 3 million news articles in the `Journalisted.com <http://journalisted.com/>`_ archive. The idea was to find the articles which had the longest sections of common text and to then rank them according to their respective percentages of reuse. 

Initially, we looked at existing search engines like `Xapian <http://xapian.org>`_ and `Lucene <http://lucene.apache.org/>`_ and although being very effective at finding articles containing the same words, they weren't particulary good at finding the articles with the longest common strings. More info on this can be found `here <http://mediastandardstrust.org/blog/the-technology-driving-churnalism-com/>`_.

Besides spotting `churnalism <http://en.wikipedia.org/wiki/Churnalism>`_, the software seemed to have other potential uses and the `Sunlight Foundation <http://sunlightfoundation.com/>`_ believes it could be applied to spotting duplicated federal law. To facilitate this application they have generously offered to fund the open sourcing of this software, and the results should hopefully be soon tested against the `Open States <http://openstates.sunlightlabs.com/>`_ project. Stay tuned!

Contents:
=========

.. toctree::
    :maxdepth: 1
    
    install.rst
    developing.rst
    architecture.rst
    api.rst
    changelog.rst
    license.rst

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

