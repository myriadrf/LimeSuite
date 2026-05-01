CLI Tools
#########

Lime Suite includes a number of command-line tools for testing and debugging, as well as performing various tasks such as upgrading firmware and gateware on supported hardware. These tools are available on all platforms.

LimeUtil
********

LimeUtil is a command-line utility that provides various functions for working with LimeSDR devices, such as listing connected devices, upgrading firmware and gateware, and performing basic tests. 

.. code-block:: bash

   # Check to make sure that your device type is one of the available connections
   LimeUtil --info

   # Locate devices on the system
   LimeUtil --find

   # Example output 
     * [module=STREAM, media=USB, name=USB 3.0 (LimeSDR-USB), addr=241:1204]

SoapySDRUtil
************

See also the similar utility provided by :doc:`/user/plugins/soapysdr`, which can be used to check that your device can be accessed by the SoapySDR plugin.

