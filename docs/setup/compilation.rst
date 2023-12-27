Compilation guide
=================

This page describes how to compile LimeSuiteGUI2 on Linux.

Prerequisites
-------------

Components needed to compile LimeSuiteGUI2:

- `GCC`_
- `CMake`_
- `wxWidgets`_
- `libusb`_

.. note::
    If you are on Ubuntu 22.04, there also exists a script `install_dependencies.sh` to install all the needed dependencies.

Compilation
-----------

In the root directory of the repository run these commands:

.. code-block:: bash

    mkdir build # Create the build directory
    cd build # Change working directory to the build directory
    cmake .. # Generate make file
    make # Compile the program

After a successful compilation the resulting binaries are placed in the `build/bin/` directory
located in the root directory of the repository.

Testing
-------

This project contains unit tests to test functionality of some parts of the system.

Running the tests
^^^^^^^^^^^^^^^^^

To run these tests, in the root directory of the repository run these commands:

.. code-block:: bash

    cd build # Change working directory to the build directory
    cmake .. -DBUILD_SHARED_LIBS=OFF # Generate make file with static libraries
    make LimeSuite2Test # Build the tests binary
    bin/LimeSuite2Test # Run the tests binary

After the tests are ran, you should see the information about which tests were ran,
and whether they were successful or not.

Code coverage
^^^^^^^^^^^^^

To generate a web-based testing code coverage report, in the root directory of the repository run these commands:

.. code-block:: bash

    cd build # Change working directory to the build directory
    cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_SHARED_LIBS=OFF # Generate make file with static libraries and without optimizations
    make LimeSuite2Test_coverage # Generate the coverage report with lcov

After generating the report it can be found in the `build/LimeSuite2Test_coverage`
directory and opening the `index.html` file with a browser.

Docker
^^^^^^

There also exists a Docker file to generate the web-based testing code coverage report.
To generate the testing code coverage report using Docker, in the root folder of the repository run:

.. code-block:: bash

    docker build -o <output_location> -f TestCoverage.Dockerfile .

This will run the whole code coverage report generation script and place the generated HTML in the specified folder.

.. _`GCC`: https://gcc.gnu.org/
.. _`CMake`: https://cmake.org/
.. _`wxWidgets`: https://www.wxwidgets.org/
.. _`libusb`: https://libusb.info/
