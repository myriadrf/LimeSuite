SoapySDR
########

SoapySDR provides a hardware abstraction layer for SDR devices, allowing them to be used with a variety of software applications that support the SoapySDR API. 

The SoapySDR plugin for Lime Suite enables support for LimeSDR devices within the SoapySDR framework, allowing users to access and control their LimeSDR hardware through compatible software applications.

SoapySDRUtil
************

SoapySDRUtil is particularly useful for testing and debugging SoapySDR support, and can be used to check that your device can be accessed by the SoapySDR plugin.

.. code-block:: bash

   # Make sure that lime is one of the available factories 
   SoapySDRUtil --info

   # Locate connected LimeSDR devices 
   SoapySDRUtil --find="driver=lime"

   # Example output
   Found device 0
     addr = 241:1204
     driver = lime
     media = USB
     module = STREAM
     name = USB 3.0 (LimeSDR-USB)
