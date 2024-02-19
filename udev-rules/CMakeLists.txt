if (NOT UNIX)
    return()
endif()

set(ENABLE_UDEV_RULES ON CACHE BOOL "Install linux udev rules")

include(FeatureSummary)
add_feature_info(UDEV_RULES ENABLE_UDEV_RULES "Install Linux udev rules")

if (NOT ENABLE_UDEV_RULES)
    return()
endif()

set(UDEV_RULES_INSTALL_PATH "/usr/lib/udev/rules.d" CACHE STRING "Path where to install udev rules")

install(
    FILES ${CMAKE_CURRENT_LIST_DIR}/64-limesuite.rules
    DESTINATION ${UDEV_RULES_INSTALL_PATH}
)

install(CODE "
    MESSAGE(\"Reloading udev rules:\")
    execute_process(COMMAND udevadm control --reload-rules COMMAND_ECHO STDERR)
    execute_process(COMMAND udevadm trigger COMMAND_ECHO STDERR)
")
