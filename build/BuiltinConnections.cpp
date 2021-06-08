/***********************************************************************
 * This is a collection of all built-in connections that are included
 * with the library. Additional connections can be added dynamically.
 **********************************************************************/

#define ENABLE_EVB7COM
#define ENABLE_STREAM
#define ENABLE_NOVENARF7

void __loadConnectionEVB7COMEntry(void);
void __loadConnectionSTREAMEntry(void);
void __loadConnectionNovenaRF7Entry(void);

void __loadAllConnections(void)
{
    #ifdef ENABLE_EVB7COM
    __loadConnectionEVB7COMEntry();
    #endif

    #ifdef ENABLE_STREAM
    __loadConnectionSTREAMEntry();
    #endif

    #ifdef ENABLE_NOVENARF7
    __loadConnectionNovenaRF7Entry();
    #endif
}
