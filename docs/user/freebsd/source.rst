Building from Source
####################

A FreeBSD "Port" for Lime Suite exists that automates the build, install, and packaging process. 

.. code-block:: bash

   cd /usr/ports/comms/limesuite
   make config
   make install
   make package ; pkg install work/pkg/<your_package_is_here>
