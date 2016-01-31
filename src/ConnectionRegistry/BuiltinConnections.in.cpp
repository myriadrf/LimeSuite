/***********************************************************************
 * This is a collection of all built-in connections that are included
 * with the library. Additional connections can be added dynamically.
 **********************************************************************/

#cmakedefine ENABLE_EVB7COM
#cmakedefine ENABLE_STREAM
#cmakedefine ENABLE_NOVENA7

void __loadConnectionEVB7COMEntry(void);
void __loadConnectionSTREAMEntry(void);
void __loadConnectionNovena7Entry(void);

void __loadAllConnections(void)
{
    #ifdef ENABLE_EVB7COM
    __loadConnectionEVB7COMEntry();
    #endif

    #ifdef ENABLE_STREAM
    __loadConnectionSTREAMEntry();
    #endif

    #ifdef ENABLE_NOVENA7
    __loadConnectionNovena7Entry();
    #endif
}
