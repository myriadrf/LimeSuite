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

Compilation
-----------

In the root folder of the repository run these commands:

.. code-block:: bash

    cd build
    cmake ..
    make

After a successful compilation the resulting binaries are placed in the `build/bin/` folder
located in the root directory of the repository.

.. _`GCC`: https://gcc.gnu.org/
.. _`CMake`: https://cmake.org/
.. _`wxWidgets`: https://www.wxwidgets.org/
.. _`libusb`: https://libusb.info/
