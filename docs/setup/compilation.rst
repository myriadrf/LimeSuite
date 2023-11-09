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

.. _GCC: https://gcc.gnu.org/
.. _CMake: https://cmake.org/
.. _wxWidgets: https://www.wxwidgets.org/
.. _libusb: https://libusb.info/
