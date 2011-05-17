Developing
==========

The intention is to make sure that all the development workflows, such as building the docs, running the tests and creating the distribution are covered by `Paver <http://paver.github.com/paver/>`_. The commands are detailed below and can be executed by, for example, typing:

``paver docs`` 

To run the tests, first start up a Kyoto Tycoon instance on port 1977 with an index name like test.kct:

``paver kyototycoon -p 1977 -f test.kct``

and then in another terminal window run:

``paver test``

Commands
--------

.. autofunction:: pavement.test

.. autofunction:: pavement.kyototycoon

.. autofunction:: pavement.sdist

.. autofunction:: pavement.docs

.. autofunction:: pavement.github_docs

.. autofunction:: pavement.examples

.. autofunction:: pavement.legislation