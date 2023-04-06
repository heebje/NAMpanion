#pragma once

#include "NAMpanion_Common.h"

using namespace NAMpanion_Common;

//class WaveshaperAsinh {
//public:
//  double processAudioSample(double x) {
//    return asinh(x);
//  }
//};

class WaveShaperAsym2 {
private:
  double a = 0.0;  // range 0.0 .. 1.0

public:

#ifdef _DEBUG // Sanity check: ////////////////////////////////////////////////
  WaveShaperAsym2() {
    const double saveA = a;
    setA(1.0);
    const double x = abs(processAudioSample(-9.0 / 8.0));
    assert(x < 1.0E-12);
    a = saveA;
  }
#endif // _DEBUG //////////////////////////////////////////////////////////////

  inline double setA(double _a) {
    return a = _a;
  }

  inline double processAudioSample(double _sample) {
    const double a4x3 = (4.0 / 3.0) * a * _sample;
    return tanh(_sample + a4x3 * a4x3 * (1.0 + a4x3 / 3.0));
  }
};
