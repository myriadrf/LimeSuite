Introduction
############

.. toctree::
   :maxdepth: 2
   :hidden:

   Introduction <self>
   User Guide <user/index>
   Developer Guide <developer/index>

.. figure:: /images/Lime_suite_comps.png
   :align: center

Lime Suite is a collection of software which provides drivers and tools for the LMS7002M transceiver RFIC, along with support for LimeSDR boards which are based on this. Installing Lime Suite enables many SDR applications to be used with supported hardware, through both the native APIs and provided plug-ins, such as for SoapySDR and GNU Radio.

Versions
********

There are two major development branches of Lime Suite:

* :external+suiteng:ref:`Lime Suite NG <index:introduction>`. The Next Generation version.
* **Classic Lime Suite**. Which this documentation is concerned with.

The Lime Suite codebase has grown over the years to incorporate many improvements and become the foundation for various SDR boards and modules that utilise Lime chipsets. The software collection has also grown to include numerous examples, utilities — such as for testing and programming firmware/gateware — and plug-ins for applications such as the SoapySDR framework and GNU Octave.

After many years of development it became clear that Lime Suite would benefit from certain architectural changes, such as improved support for boards with more than one transceiver. Furthermore, these improvements would result in breaking API changes, hence it was decided to implement them via a new project, Lime Suite NG (Next Generation).

The original Lime Suite project is now transitioning to maintenance mode, where it will continue to receive critical fixes, new issues and pull requests are still welcomed, and new releases will be made as and when appropriate. However, all architectural improvements, new features and support for new SDR boards will be implemented via Lime Suite NG.

Given the breaking API changes and a desire to ease transition from Lime Suite to Lime Suite NG, the latter has been developed such that it can coexist alongside the former; both Lime Suite variants may be installed at the same time and applications may make use of either one or both.

In addition, Lime Suite NG has a compatibility wrapper, which provides the legacy LMS API and allows applications to be used with the new version largely unmodified. However, it is recommended that applications eventually migrate to using the new SDRDevice API. 

Which version should I use?
===========================

For guidance on which version of Lime Suite to use, please see the :external+quickstart:ref:`Quick Start Guide <index:introduction>`.

Supported Hardware
******************

Lime Suite supports the following hardware:

* LimeSDR Mini 2.0
* LimeSDR Mini 1.0
* LimeSDR QPCIe
* LimeNET Micro
* LimeSDR PCIe
* LimeSDR USB
* LimeSDR GPIO Board
* STREAM FPGA Development Board with LMS7002M UNITE (EVB7)
* LMS7002M UNITE (EVB7) through COM port
* Novena Laptop with LMS7 RF board

