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

Now that all the open source infrastructure is in place it's time to start learning a bit more about the use-case in hand, US Congress and state legislation. Being based in the UK, it is also interesting to contrast the US system with that of the Houses of Parliament. I am no expert of systems of government, so there is a good chance I might make false assertions!

Legislature
'''''''''''

The US Congress has two chambers, both elected, the House of Representatives and the Senate. Both chambers can introduce bills and the bill can take one of many paths through both chambers. The House represents individual states with elected members in proportion to their population, while two senators are allocated to each state. This contrasts with the House of Lords, which is unelected and unrepresentative of the population, while the House of Commons is larger in number than the House of Representatives and is elected by relatively equally populated constituencies.

The passage of a bill through Congress appears to be quite undefined with a whole series of possible routes and referrals being possible before an Act is signed by the President. In the UK, the system is a configured list of readings and stages before Royal Assent is given.

The end result of acts of Congress is the `US Code <http://www.gpo.gov/fdsys/browse/collectionUScode.action?collectionCode=USCODE>`_ which is an up to date record of all enrolled bills. In the UK, there is a lack of an easily digestible final law, as evidenced `here <http://www.legislation.gov.uk/ukpga/1998/29>`_. The amount of cross-referencing and amending means that to the average member of the public the result is incomprehensible!

States in the US are capable of passing their own local laws. In the UK there is devolution in Scotland, Wales, Northern Ireland and Greater London which permits regional legislation, but leaves the remainder of England with an exceptional democratic deficit. Every part of the UK has representatives in the European Parliament

Data Access
'''''''''''

The US has a well maintained `digital archive <http://www.gpo.gov/fdsys/browse/collection.action?collectionCode=BILLS>`_ of all House of Representative Bills from 2004, and a more recent archive of Senate Bills. The data is accessible in XML form which allows for metadata extraction of Bill information and easy manipulation of the Bill text itself, ideal for the purposes of this project. Text-only bills are available from 1993 onwards.

UK bills are `accessible <http://www.legislation.gov.uk/>`_ in text and HTML form only, which makes them less amenable to analysis without some serious text extraction work. Versions of the bill at various stages are not available and thus it is impossible to track revisions and differences as progress through parliament is made.

What is available in the UK, is effectively a `transcript <http://services.parliament.uk/bills/>`_ of what is said in Parliament, including the reading aloud of the bills. This mixture of debate and legislative text is the data source which drives `TheyWorkForYou.com <http://www.theyworkforyou.com/>`_, the result of some tenacious scraping! Unfortunately, this means that public is forced to focus more on the opinions and personalities of the MP's rather than the substance and progress of the legislation itself.






