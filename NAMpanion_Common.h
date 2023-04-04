#pragma once

namespace NAMpanion_Common {

#pragma once

#include <tgmath.h>

  const double  _PI             = 3.14159265358979323846264338327950288419716939937510;
  const double  _HALF_PI        = _PI / 2.0;
  const double  _2_OVER_PI      = 2.0 / _PI;
  const double  _DOUBLE_PI      = _PI * 2.0;
  const double  _SQRT_PI        = sqrt(_PI);
  const double  _HALF_SQRT_PI   = _SQRT_PI / 2.0;

  const double  _SQRT_2         = sqrt(2.0);
  const double  _SQRT_1_OVER_2  = sqrt(0.5);
  const double  _SQRT_2_PLUS_1  = _SQRT_2 + 1.0;
  const double  _SQRT_8         = sqrt(8.0);
  const double  _SQRT_8_PLUS_2  = _SQRT_8 + 2.0;
  const double  _SQRT_8_PLUS_3  = _SQRT_8 + 3.0;
  const double  _SQRT_1_OVER_8  = 1.0 / _SQRT_8;

  const double  _632            = sqrt(20.0 * 20000.0);
  const double  _22049          = 22049.0;

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

  inline double lerp(double a, double b, double t) {
    return a + t * (b - a);
  }


};

