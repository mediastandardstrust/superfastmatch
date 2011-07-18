README
======

This is a new version of Superfastmatch written in C++ to improve matching performance and with an index running totally in memory to improve response times.

The point of the software is to index large amounts of text in memory. Therefore there isn't much reason to run it on a 32-bit OS with a 4GB cap on memory and a 64-bit OS is assumed

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

On Ubuntu you'll need to do this first:

    sudo apt-get install libunwind7

Which is a dependency for perftools. And you might also need a:

    sudo ldconfig

after the script has finished.

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

Visit http://127.0.0.1:8080 to test the interface.

Data
----

Of course you need something to load. Have a look at fixtures.sh and load.sh for some inspiration!
