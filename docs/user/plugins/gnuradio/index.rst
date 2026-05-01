GNU Radio
#########

.. toctree::
   :maxdepth: 3
   :hidden:

   linux
   windows
   configuration
   examples

GNU Radio support is provided via the gr-limesdr plugin, which provides source and sink blocks for use with the GNU Radio API and GNU Radio Companion (GRC). The plugin additionally provides support for LimeRFE, enabling this to be configured from within flowgraphs.

If you have the SoapySDR plugin installed you can also use its source and sink blocks in GNU Radio. However, the generic SoapySDR interface does not provide access to all LimeSDR hardware features, or is as optimal a solution as gr-limesdr, which provides a more direct interface to the hardware. 

