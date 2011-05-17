README
======

SuperFastMatch depends on `Paver <http://paver.github.com/paver/#installation>`_ and `VirtualEnv <http://pypi.python.org/pypi/virtualenv>`_  to get bootstrapped. The machine that will serve as the index host also requires `Kyoto Cabinet <http://fallabs.com/kyotocabinet>`_, `Kyoto Tycoon <http://fallabs.com/kyototycoon>`_. and `Lua <http://www.lua.org/>`_.

Example install commands:

.. code-block:: bash

    # Install Lua
    wget http://www.lua.org/ftp/lua-5.1.4.tar.gz
    tar -xvf lua-5.1.4.tar.gz
    cd lua-5.1.4
    make macosx && sudo make install # Change macosx for correct platform
    cd ..
    
    # Install Kyoto Cabinet
    wget http://fallabs.com/kyotocabinet/pkg/kyotocabinet-1.2.52.tar.gz # Version might be updated
    tar -xvf kyotocabinet-1.2.52.tar.gz
    cd kyotocabinet-1.2.52
    ./configure && make && sudo make install
    cd ..
    
    # Install Kyoto Tycoon 
    wget http://fallabs.com/kyototycoon/pkg/kyototycoon-0.9.40.tar.gz # Version might be updated
    tar -xvf kyototycoon-0.9.40.tar.gz
    cd kyototycoon-0.9.40
    ./configure --enable-lua && make && sudo make install
    cd ..
    
    # Install SuperFastMatch
    sudo easy_install paver
    sudo easy_install virtualenv
    git clone git://github.com/mediastandardstrust/superfastmatch.git
    cd superfastmatch
  
After install:

``paver help``

will display options with the custom commands visible last. After that it might be worth having a look at the `Developing <developing.html>`_ page.

After all that, to run the legislation example:

``paver kyototycoon``

in one terminal window and

``paver legislation``

in another.