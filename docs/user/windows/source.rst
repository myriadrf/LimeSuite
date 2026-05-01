Building from Source
####################

Building Lime Suite on Windows requires the Visual Studio compiler and a number of dependencies.

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
     - Has embedded copy for windows 
   * - LMS7 GUI
     - wx widgets & OpenGL
     - Has embedded GL for windows 
   * - LimeSDR
     - CyAPI 
     - 
   * - NovenaRF7
     - i2c and spi dev
     - **Linux only device**
   * - SoapyLMS7
     - SoapySDR
     - SDR app ecosystem support

The other dependencies can be obtained via the following links: 

* `Git for Windows`_ 
* `CMake`_
* `wxWidgets`_
* `CyUSB`_ 

  * It may be easier to download :code:`cy_ssusbsuite_*.zip` rather than the full SDK.

Getting the Source Code
***********************

After installing the dependencies, open a cmd prompt and run the following command to clone the repository: 

.. code-block:: doscon

   git clone https://github.com/myriadrf/LimeSuite.git

Building Lime Suite
*******************

From the Command Prompt
=======================
  
Complete command line example with cmake for MSVC 2015 64-bit (your dependency directories may vary): 

.. code-block:: doscon

   cd LimeSuite
   git checkout stable
   mkdir builddir
   cd builddir

   cmake ../ -G "Visual Studio 14 2015 Win64" ^
       -DWX_ROOT_DIR=C:/wxWidgets-3.1.0 ^
       -DwxWidgets_ROOT_DIR=C:/wxWidgets-3.1.0 ^
       -DwxWidgets_LIB_DIR=C:/wxWidgets-3.1.0/lib/vc140_x64_lib ^
       -DFX3_SDK_PATH="C:/EZ-USB FX3 SDK/1.3" ^
       -DSoapySDR_DIR=C:/PothosSDR

   cmake --build . --config Release

   #installs files to CMAKE_INSTALL_PREFIX
   cmake --build . --config Release --target install

Using the GUI
=============

Use :code:`cmake-gui`` to configure and generate the Visual Studio project solution. After generating, open the solution file and run the build target. 

.. figure:: /images/Lime_suite_windows_cmake_gui.png
   :align: center


.. _Git for Windows: https://git-for-windows.github.io/
.. _CMake: https://cmake.org/download/
.. _wxWidgets: https://www.wxwidgets.org/downloads/
.. _CyUSB: http://www.cypress.com/documentation/software-and-drivers/ez-usb-fx3-software-development-kit

