########################################################################
# menu launcher and MIME type
########################################################################
file(GLOB lime_suite_png_icons ${CMAKE_CURRENT_SOURCE_DIR}/lime-suite-*.png)

install(
    FILES
        lime-suite.desktop
        ${lime_suite_png_icons}
    DESTINATION share/Lime/Desktop
)

########################################################################
# install/uninstall scripts
########################################################################
install(
    PROGRAMS
        install
        uninstall
    DESTINATION share/Lime/Desktop
)
