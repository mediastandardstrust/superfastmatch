README
======

This is a new version of Superfastmatch written in C++ to improve matching performance and with an index running totally in memory to improve response times.

The process for installation is as follows:


Dependencies
------------

Superfastmatch depends on these libraries:

[Google gflags](http://code.google.com/p/google-gflags/)

[Google perftools](http://code.google.com/p/google-perftools/)

[Google ctemplate](http://code.google.com/p/google-ctemplate/)

[Google sparsehash](http://code.google.com/p/google-sparsehash/)

[Kyoto Cabinet](http://fallabs.com/kyotocabinet/)

[Kyoto Tycoon](http://fallabs.com/kyototycoon/)

You might be able to get away with installing the .deb packages on the listed project pages, but this is untested. The trunk of sparsehash is required for this [patch](http://code.google.com/p/google-sparsehash/source/detail?r=76);

The easier route is to run:

    ./scripts/bootstrap.sh

and wait for everything to build. The script will ask you for your sudo password, which is required to install the libraries.

Test
----

After the libraries are installed, you can run:

    make check

to run the currently lonely unit test for the index code.

Build
-----

After that you can run:

    make run

to get a superfastmatch instance running. Nothing is currently configurable from the command line yet. Coming soon...

Data
----

Of course you need something to load. Have a look at fixtures.sh and load.sh for some inspiration!