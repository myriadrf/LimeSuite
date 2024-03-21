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
option(ENABLE_GNURADIO "Enable build library" OFF)
add_feature_info(GNURADIO ENABLE_GNURADIO "GNU Radio 3.9+ Plug-in")
if (ENABLE_GNURADIO)
    add_subdirectory(gr-limesdr)
endif()