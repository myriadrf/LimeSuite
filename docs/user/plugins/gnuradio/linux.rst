Linux
#####

Installing gr-limesder on Linux is straightforward, and can be done via the package manager on many distributions, or by building from source. 

Prerequisites
*************

The following prerequisites should be installed:

* Lime Suite
* `GNU Radio`_

Installation using the Ubuntu PPA
*********************************

The gr-limesdr plugin available to download through the MyriadRF GNU Radio PPA:

.. code-block:: bash

   sudo add-apt-repository ppa:myriadrf/gnuradio
   sudo apt-get update
   sudo apt-get install gr-limesdr

Installing from Source
**********************

Dependencies
============

Enter the following command in terminal to install Boost and SWIG: 

.. code-block:: bash

   sudo apt-get install libboost-all-dev swig

If you installed GNURadio using the package manager also install the gnuradio-dev package: 

.. code-block:: bash

   sudo apt-get install gnuradio-dev

gr-limesdr Source
=================

Clone the gr-limesdr repository:

.. code-block:: bash

   git clone https://github.com/myriadrf/gr-limesdr

Build and Install
=================

Prior to building it is important to checkout the git branch which corresponds to the version of GNU Radio you have installed. For example, if you have GNU Radio 3.8 installed, checkout the gr-3.8 branch.

.. code-block:: bash

   cd gr-limesdr
   git checkout gr-3.8
   mkdir build
   cd build
   cmake ..
   make
   sudo make install
   sudo ldconfig

At this point if GNU Radio Companion was already running, you should reload the blocks by restarting GNU Radio Companion or by selecting the Reload blocks button in top bar.

.. figure:: /images/Reload_Blocks_Button_GnuRadio.png
   :align: center

.. _GNU Radio: https://wiki.gnuradio.org/index.php/InstallingGR