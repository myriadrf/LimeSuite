
#cmakedefine ENABLE_LIMESDR_USB
#cmakedefine ENABLE_LIMESDR_X3
#cmakedefine ENABLE_LIMESDR_XTRX
#cmakedefine ENABLE_LIMESDR_MMX8
#cmakedefine ENABLE_LIMESDR_MINI

void __loadLimeSDR();
void __loadLimeSDR_X3();
void __loadLimeSDR_XTRX();
void __loadLimeSDR_MMX8();
void __loadLimeSDR_Mini();

void __loadBoardSupport()
{
#ifdef ENABLE_LIMESDR_USB
    __loadLimeSDR();
#endif

#ifdef ENABLE_LIMESDR_X3
    __loadLimeSDR_X3();
#endif

#ifdef ENABLE_LIMESDR_XTRX
    __loadLimeSDR_XTRX();
#endif

#ifdef ENABLE_LIMESDR_MMX8
    __loadLimeSDR_MMX8();
#endif

#ifdef ENABLE_LIMESDR_MINI
    __loadLimeSDR_Mini();
#endif
}
