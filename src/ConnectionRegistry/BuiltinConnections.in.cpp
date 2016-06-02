/***********************************************************************
 * This is a collection of all built-in connections that are included
 * with the library. Additional connections can be added dynamically.
 **********************************************************************/

#cmakedefine ENABLE_EVB7COM
#cmakedefine ENABLE_STREAM
#cmakedefine ENABLE_NOVENARF7
#cmakedefine ENABLE_uLimeSDR
#cmakedefine ENABLE_PCIE_XILLYBUS

void __loadConnectionEVB7COMEntry(void);
void __loadConnectionSTREAMEntry(void);
void __loadConnectionNovenaRF7Entry(void);
void __loadConnection_uLimeSDREntry(void);
void __loadConnectionXillybusEntry(void);

void __loadAllConnections(void)
{
    #ifdef ENABLE_EVB7COM
    __loadConnectionEVB7COMEntry();
    #endif

    #ifdef ENABLE_STREAM
    __loadConnectionSTREAMEntry();
    #endif

    #ifdef ENABLE_uLimeSDR
    __loadConnection_uLimeSDREntry();
    #endif

    #ifdef ENABLE_NOVENARF7
    __loadConnectionNovenaRF7Entry();
    #endif

    #ifdef ENABLE_PCIE_XILLYBUS
    __loadConnectionXillybusEntry();
    #endif
}
