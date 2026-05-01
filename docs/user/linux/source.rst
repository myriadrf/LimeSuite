Building from Source
####################

Dependencies
************

Before building Lime Suite from source, several dependencies must be installed. However, the dependencies required change based on how much of the software suite will be used. 

Lime Suite will automatically try and build all components that it can satisfy the dependencies for. The following table is a guide to the the requirements for various components. 

.. list-table:: Lime Suite components and dependencies
   :header-rows: 1

   * - Component
     - Dependencies
     - Notes
   * - Build system
     - Git & CMake
     -
   * - Core library
     - sqlite3
     - 
   * - LMS7 GUI
     - wx widgets & OpenGL
     - 
   * - LimeSDR
     - libusb 1.0
     - 
   * - NovenaRF7
     - i2c and spi dev
     - 
   * - SoapyLMS7
     - SoapySDR
     - SDR app ecosystem support

Ubuntu
======

.. code-block:: bash

   #packages for soapysdr available at myriadrf PPA
   sudo add-apt-repository -y ppa:myriadrf/drivers
   sudo apt-get update

   #install core library and build dependencies
   sudo apt-get install git g++ cmake libsqlite3-dev

   #install hardware support dependencies
   sudo apt-get install libsoapysdr-dev libi2c-dev libusb-1.0-0-dev

   #install graphics dependencies
   sudo apt-get install libwxgtk3.0-dev freeglut3-dev

Other Linux Distributions
=========================

Ubuntu derivatives should be able to use the same instructions above.

Other distributions, e.g. RPM-based, will not be able use the myriadrf/drivers PPA, and O/S packaged dependencies may be different versions and use slightly different naming. However, it should be possible to use the above as a guide and it is likely that, at the most, `SoapySDR would have to be built from source`_.

.. _SoapySDR would have to be built from source: https://github.com/pothosware/SoapySDR

Building Lime Suite
*******************

The following commands will clone the Lime Suite repository, configure the project using CMake, build the project, and install it. The output of the cmake command will show the enabled and disabled components. Pay careful attention to this before building with make.

.. code-block:: bash

  git clone https://github.com/myriadrf/LimeSuite.git
  cd LimeSuite
  git checkout stable
  mkdir builddir && cd builddir
  cmake ../
  make -j4
  sudo make install
  sudo ldconfig
  cd ../udev-rules
  sudo ./install.sh

