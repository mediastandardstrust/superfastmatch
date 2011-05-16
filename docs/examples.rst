Examples
========

Legislation
-----------

This is an exampe Django application that scrapes US Congress Bills puts them in a local postgres database. The scraper depends on lxml which requires the libxml and libxslt dev libraries to build. On Ubuntu this can be achieved by:

``sudo apt-get install libxml2-dev libxslt1-dev``

The simplest way to use is:

``paver legislation``

which will build everything and start running the webserver. If you need to run some django commands then you might be better off with something like this:

``cd examples/legislation && source ../../.env/examples_env/bin/activate``

in order to activate the examples virtual environment.
