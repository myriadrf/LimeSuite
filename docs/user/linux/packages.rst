Ubuntu Packages
###############

The Ubuntu drivers PPA should be set up and following which packages may be installed.

.. code-block:: bash

   sudo add-apt-repository -y ppa:myriadrf/drivers
   sudo apt-get update
   sudo apt-get install limesuite liblimesuite-dev limesuite-udev limesuite-images
   sudo apt-get install soapysdr-tools soapysdr-module-lms7

We currently package for Ubuntu releases:

* 22.04 LTS