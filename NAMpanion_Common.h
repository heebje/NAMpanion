#pragma once

#include <tgmath.h>

namespace NAMpanion_Common {

  const double  _PI       = 3.14159265358979323846264338327950288419716939937510;
  const double  _HALF_PI  = _PI / 2.0;

  #ifdef _DEBUG
    #ifdef _MSC_VER
      #define DEBUG_BREAK __debugbreak()
    #else
      #define DEBUG_BREAK __builtin_trap()
    #endif
    #define NOP       { int dummy = 1; dummy = dummy; }
    #define BRK       { DEBUG_BREAK; }
    #define FAIL(why) { DEBUG_BREAK; exit(-1); }
    #define NIY(what) { DEBUG_BREAK; }
  #else
    #define NOP
    #define BRK
    #define FAIL(why)
    #define NIY(what)
  #endif // _DEBUG

};
