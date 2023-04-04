#pragma once

//#include <assert.h>
//#include <cmath>

#include "NAMpanion_Common.h"

using namespace NAMpanion_Common;

class AudioProcessorBase {
protected:
  double  m_SampleRate;
public:
  virtual double reset(double _sampleRate) {
    return m_SampleRate = _sampleRate;
  }
  virtual double processAudioSample(double _x) {
    return _x;
  }
};

class WaveShaperBase: public AudioProcessorBase {
protected:
  static inline double dBToAmp(double dB) {
    const double kAmp_dB = 20.0 / log(10);
    return std::exp(kAmp_dB * dB);
  }

#ifdef _DEBUG
public:
  void test(double _input, double _expectedOutput) {
    double v = this->processAudioSample(_input);
    if (abs(v - _expectedOutput) > 0.001) {
      BRK;
    };
  };
#endif
};

// tanh() /////////////////////////////////////////////////////////////////////

class WaveShaperTanh: public WaveShaperBase {
private:
  double m_MaxOut = +1.0;

public:
  WaveShaperTanh(double _MaxOut) {
    setMaxOut(_MaxOut);
  }

  double processAudioSample(double x) override {
    return m_MaxOut * tanh(x / m_MaxOut);
  }

  double setMaxOut(double _MaxOut) {
    assert(_MaxOut > 0.0);
    return m_MaxOut = _MaxOut;
  }

};

// atan() /////////////////////////////////////////////////////////////////////

class WaveShaperAtan: public WaveShaperBase {
public:
  double processAudioSample(double x) override {
    return atan(_HALF_PI * x) / _HALF_PI;
  }
};

// Polynomial /////////////////////////////////////////////////////////////////

class WaveShaperPolynomial: public WaveShaperBase {
private:
  double m_Order = 2.0;
public:
  WaveShaperPolynomial(double _order) {
    m_Order = _order;
  };

  double processAudioSample(double x) override {
    if (m_Order == 2.0) {
      return x / sqrt(x * x + 1);
    } else if (m_Order == 3.0) {
      if (x >= 0) {
        return x / pow(x * x * x + 1.0, 1.0 / 3.0);
      } else {
        return x / pow(-x * x * x + 1.0, 1.0 / 3.0);
      }
    } else {
      if (x >= 0) {
        return x / pow(pow(x, m_Order) + 1.0, 1.0 / 3.0);
      } else {
        return x / pow(pow(-x, m_Order) + 1.0, 1.0 / 3.0);
      }
    }
  }
};

// cosh() /////////////////////////////////////////////////////////////////////

class WaveShaperCosh : public WaveShaperBase {
public:
  double processAudioSample(double x) override {
    return x / (log(cosh(x)) + 1.0);
  }
};

// erf(): https://en.wikipedia.org/wiki/Error_function ////////////////////////

class WaveShaperErf : public WaveShaperBase {
public:
  double processAudioSample(double x) override {
    return erf(x * _HALF_SQRT_PI);
  }
};

// gd(): https://en.wikipedia.org/wiki/Gudermannian_function //////////////////

class WaveShaperGd : public WaveShaperBase {
public:
  double processAudioSample(double x) override {
    return 4 * atan(exp(_HALF_PI * x)) / _PI - 1.0;
  }
};

// sin() //////////////////////////////////////////////////////////////////////

class WaveShaperSin : public WaveShaperBase {
public:
  double processAudioSample(double x) override {
    if (x <= -_HALF_PI) {
      return -1;
    } else if (x >= _HALF_PI) {
      return 1;
    } else {
      return sin(x);
    }
  }
};

// circle arcs ////////////////////////////////////////////////////////////////

class WaveShaperCircle : public WaveShaperBase {
public:
  double processAudioSample(double x) override {
    if (x >= _SQRT_2_PLUS_1) {
      return 1.0;
    } else if (x >= 0) {
      return sqrt(x * (-x + _SQRT_8_PLUS_2) + _SQRT_8_PLUS_3) - _SQRT_2_PLUS_1;
    } else if (x > -_SQRT_2_PLUS_1) {
      return -sqrt(-x * (x + _SQRT_8_PLUS_2) + _SQRT_8_PLUS_3) + _SQRT_2_PLUS_1;
    } else {
      return -1.0;
    }
  }
};

// Smooth Step: https://en.wikipedia.org/wiki/Smoothstep //////////////////////

class WaveShaperSmoothStep : public WaveShaperBase {
public:
  double processAudioSample(double x) override {
    const double CONST_15_8     =  15.0 /   8.0;
    const double CONST_128_675  = 128.0 / 675.0;
    const double CONST_32_375   =  32.0 / 375.0;

    if (x >= CONST_15_8) {
      return 1.0;
    } else if (x <= -CONST_15_8) {
      return -1.0;
    } else {
      return x * (1 - CONST_128_675 * x * x * (1 - CONST_32_375 * x * x));
    }
  }
};

class WaveshaperAsinh: public WaveShaperBase {
public:
  double processAudioSample(double x) override {
    return asinh(x);
  }
};

// Simple hard clipper ////////////////////////////////////////////////////////

class WaveShaperHardClip: public WaveShaperBase {
private:
  double  m_MaxOutRaw = 1.0;

public:
  double setMaxOut_dB(double _dB) {
    m_MaxOutRaw = dBToAmp(_dB);
    return _dB;
  }

  double processAudioSample(double _x) override {
    return std::clamp(_x, -m_MaxOutRaw, +m_MaxOutRaw);
  }
};

class WaveShaperAsym: public WaveShaperBase {
private:
  const double  OFFSET          = +0.5085678802207957;
  double        m_DriveRaw      = 1.0;
  double        m_NonLinearity  = 0.0;  // 0.0 to 1.95 max or so
  double        m_Bias          = 0.0;  // Approx -sqrt(2)..+sqrt(2)
  inline double primitive(double x) {
    return x / sqrt(1.0 + m_NonLinearity * x * (_SQRT_2 + x));
  }

public:
  double setDrive(double _drive_dB) {
    m_DriveRaw = iplug::DBToAmp(_drive_dB);
    return _drive_dB;
  }

  double setNonLinearity(double _nonLinearity) {
    return (m_NonLinearity = _nonLinearity);
  }

  double setBias(double _bias) {
    return (m_Bias = _bias);
  }

  double processAudioSample(double _x) override {
    return primitive((m_DriveRaw * _x) - m_Bias - OFFSET)
         - primitive(0.0               - m_Bias - OFFSET);
  }

};

class WaveShaperAsym2: public WaveShaperBase {
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

  inline double processAudioSample(double _sample) override {

    const double a4x3 = (4.0 / 3.0) * a * _sample;

    return tanh(_sample + a4x3 * a4x3 * (1.0 + a4x3 / 3.0));
  }
};

class WaveShaperSqrtSqrt: public WaveShaperBase {
private:

  double qm_Bias       = 0.0;
  double qm_Symmetry   = 0.0;
  double m_g0          = 1.31604325717;

  inline void calculate_g0() {
    m_g0 = g(0);
  }

  inline double f(double _x) {

    static const double a   = +1.7547;
    static const double b   = +0.329;

    _x = a * (_x + b);

    return sqrt(sqrt(_x * _x + 1.0) + _x);
  }

  inline double g(double _x) {
    return (1.0 - qm_Symmetry / 2.0) * f(+_x + qm_Bias)
         - (      qm_Symmetry / 2.0) * f(-_x - qm_Bias);
  }

public:

#ifdef _DEBUG

  double ExpectedOutputs[27] = {
    0.0383285855737,         // -1, -1, -1
    0.0,                     // -1, -1,  0
    0.61960809136250328,     // -1, -1, +1
    -0.198868178976,          // -1,  0, -1
    0.0,                     // -1,  0,  0
    0.709945071876,          // -1,  0, +1
    -0.436064943525,          // -1, +1, -1
    0.0,                     // -1, +1,  0
    0.800282052389,          // -1, +1, +1

    -0.619608091363,          //  0, -1, -1
    0.0,                     //  0, -1,  0
    0.980956013416,          //  0, -1, +1
    -0.709945071876,          //  0,  0, -1
    0.0,                     //  0,  0,  0
    0.890619032902,          //  0,  0, +1
    -0.800282052389,          //  0, +1, -1
    0.0,                     //  0, +1,  0
    0.800282052389,          //  0, +1, +1

    -0.980956013416,          // +1, -1, -1
    0.0,                     // +1, -1,  0
    0.910458472624,          // +1, -1, +1
    -0.890619032902,          // +1,  0, -1
    0.0,                     // +1,  0,  0
    0.673261708074,          // +1,  0, +1
    -0.800282052389,          // +1, +1, -1
    0.0,                     // +1, +1,  0
    0.436064943525,          // +1, +1, +1

  };

  WaveShaperSqrtSqrt() {

    double  tempBias      = qm_Bias;
    double  tempSymmetry  = qm_Symmetry;

    int counter = 0;

    for (double bias : { -1.0, 0.0, 1.0 }) {
      setBias(bias);
      for (double symmetry : { -1.0, 0.0, 1.0 }) {
        setSymmetry(symmetry);
        for (double input: { -1.0, 0.0, 1.0 }) {
          test(input, ExpectedOutputs[counter++]);
          NOP;
        }
      }
    }

    qm_Bias      = tempBias;
    qm_Symmetry  = tempSymmetry;

  }

#endif

  inline double setBias(double _bias) {
    qm_Bias = _bias;
    calculate_g0();
    return (_bias);
  }

  inline double setSymmetry(double _symmetry) {
    qm_Symmetry = _symmetry;
    calculate_g0();
    return (_symmetry);
  }

  inline double processAudioSample(double _x) override {
    return g(_x) - m_g0;
    /*

    double sqrtx2plus1 = sqrt(_x * _x + 1.0);

    double nonInv = sqrt(sqrtx2plus1 + _x) - 1.0 + m_outputBias;

    if (m_Symmetry > 0.0) {

    double halfSymmetry = m_Symmetry / 2.0;

    double inv = sqrt(sqrtx2plus1 - _x) - 1.0 + m_outputBias;

    return ((1.0 - halfSymmetry) * nonInv)
    - ((      halfSymmetry) *    inv);
    } else {

    return nonInv;

    }
    */
  }

};
