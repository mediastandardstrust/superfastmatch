Examples
========

Legislation
-----------

This is an exampe Django application that scrapes US Congress Bills puts them in a local postgres database.

The simplest way to use is:

``paver legislation``

which will build everything and start running the webserver. If you need to run some django commands then you might be better off with something like this:

``cd examples/legislation && source ../../.env/examples_env/bin/activate``

in order to activate the examples virtual environment.

To run the scraper and association management commands first start Kyoto Tycoon in a terminal:

``paver kyototycoon``

and in another terminal window:

.. code-block:: bash

    cd examples/legislation 
    source ../../.env/examples_env/bin/activate
    ./manage.py scrape
    
    # Wait until finished
    ./manage.py associate
    
The scraper process takes about 2 hours to cache all the legislation files and then about n hours to index them. 