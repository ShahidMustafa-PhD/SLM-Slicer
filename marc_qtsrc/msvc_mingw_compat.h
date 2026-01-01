#pragma once
// MSVC compatibility shims when mingw headers are included
#ifdef _MSC_VER
  #include <stdarg.h>
  #define __mingw_ovr
  #define __MINGW_ATTRIB_NONNULL(...)
  #define __MINGW_GNU_PRINTF(...)
  #define __MINGW_MS_PRINTF(...)
  #define __MINGW_MS_SCANF(...)
  #define __MINGW_GNU_SCANF(...)
  #define __MINGW_NOTHROW
  #define __MINGW_ATTRIB_DEPRECATED_SEC_WARN
  #define __MINGW_ASM_CALL(x)
  #define __MINGW_UCRT_ASM_CALL(x)
  #define __mingw_bos_extern_ovr
  #define __mingw_bos_ovr
  #define __mingw_ovr
  #define __MINGW_ATTRIB_PURE
  #ifndef __restrict__
    #define __restrict__
  #endif
  #ifndef __builtin_va_list
    #define __builtin_va_list va_list
  #endif
#endif
