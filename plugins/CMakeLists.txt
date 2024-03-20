########################################################################
## Octave plugin integration
########################################################################
add_subdirectory(amarisoft-plugin)
# add_subdirectory(octave)

########################################################################
## freedesktop environment integration
########################################################################
add_subdirectory(Desktop)

########################################################################
## QuickTest software
########################################################################
# add_subdirectory(QuickTest)

########################################################################
## SoapySDR bindings
########################################################################
add_subdirectory(SoapyLMS7)

########################################################################
## GNU Radio plugin
########################################################################
find_package(Gnuradio QUIET)
# requires GNU Radio commit 02da1cb623f77d7d443ddbb291b7fec1eb147b04 (for v3.10) or 4b030830625849df9620e6aac78b7da46c788d26 (v3.11+) to be present
if(Gnuradio_VERSION VERSION_GREATER_EQUAL "3.10.4")
  add_subdirectory(gr-limesdr)
endif()
