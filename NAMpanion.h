#pragma once

// TO DO:
// - Oversampling
// - AGC
// - Web link to Shameless Plugs & NAM
// - mono / stereo / simulated stereo?
//

#include "IPlug_include_in_plug_hdr.h"
#include "iir1/Iir.h"
#include "NAMpanion_ParamSmoother.h"
#include "NAMpanion_WaveShaper.h"


using namespace iplug;
using namespace igraphics;

const int     kNumPresets       = 1;
const int     kMaxNumChannels   =    2;
const double  kSmoothingTimeMs  = 20.0; // Parameter smoothing in milliseconds
const double  kLowFreqMin       =   98.00;
const double  kHighFreqMax      = 5274.04;

const double  kLowFreqMax       = sqrt(kLowFreqMin * kHighFreqMax);
const double  kHighFreqMin      = kLowFreqMax;

const double  kMidFreqDefault   = kLowFreqMax;

const double  kLowBoostMax      =  6.0;
const double  kHighBoostMax     = 18.0;

const double  kDCBlockFreq      = 10.0;

enum EParams {
  kParamDrive = 0,
  kParamLow,
  kParamMid,
  kParamHigh,
  kParamOutput,
  kParamActive,
  //////////
  kNumParams
};

char* paramNames[kNumParams] = {
  "Drive",
  "Low",
  "Mid",
  "High",
  "Output",
  "Active",
};

struct VALUES {
  double default, minimum, maximum, step;
  VALUES(double def, double min, double max, double stp): default(def), minimum(min), maximum(max), step(stp) {};
  VALUES(boolean def): default(def), minimum(false), maximum(true), step(1.0) {};
};

VALUES paramValues[kNumParams] = {
  VALUES(+18.0,   0.0,  69.0, 0.01),  // Drive
  VALUES(  0.0, -10.0, +10.0, 0.01),  // Low
  VALUES(  0.0,  -9.0,  +9.0, 0.01),  // Mid
  VALUES(  0.0, -10.0, +10.0, 0.01),  // High
  VALUES(-18.0, -51.0, +18.0, 0.01),  // Output
  VALUES(true)                        // Active
};

IRECT controlCoordinates[kNumParams] = {
  IRECT(60 + 0*84, 100, 123 + 0*84, 215), // Drive
  IRECT(60 + 1*84, 100, 123 + 1*84, 215), // Low
  IRECT(60 + 2*84, 100, 123 + 2*84, 215), // Mid
  IRECT(60 + 3*84, 100, 123 + 3*84, 215), // High
  IRECT(60 + 4*84, 100, 123 + 4*84, 215), // Output
  IRECT(60 + 5*84, 100, 123 + 5*84, 215), // Active
};

class SmootherCallback final: public SP_SmootherCallback {
public:
  SmootherCallback();
  void getParamInfo(void*           _plugin,
                    int             _paramNumber,
                    double&         _sampleRate,
                    double&         _value,
                    SP_DisplayType& _dt) override;
};

class NAMpanion final: public Plugin {
private:
  //// Parameter smoothing:
  //double                        m_ParamValues  [kNumParams];
  //Iir::Butterworth::LowPass<1>  m_ParamSmoother[kNumParams];

  double inline static getLowFreq(const double lowPos) {
    return kLowFreqMin * pow(kLowFreqMax / kLowFreqMin, abs(lowPos / paramValues[kParamLow].maximum));
  }

  double inline static getLowBoost(const double lowPos) {
    return (tanh(2.0 * lowPos / kLowBoostMax) + 1.0) * kLowBoostMax / 2.0;
  }

  double inline static getMidFreq(const double lowPos, const double highPos) {
    return sqrt(getLowFreq(lowPos) * getHighFreq(highPos));
  }

  double inline static getMidQ(const double mid_dB) {
    return pow(sqrt(0.5), mid_dB / paramValues[kParamMid].maximum); // Make sure Q = 0.71 at maximum mid setting
  }

  double inline static getHighFreq(const double highPos) {
    return kHighFreqMax / pow(kHighFreqMax / kHighFreqMin, abs(highPos / paramValues[kParamHigh].maximum));
  }

  double inline static getHighBoost(const double highPos) {
    return (tanh(2.0 * highPos / kHighBoostMax) + 1.0) * kHighBoostMax / 2.0;
  }

  // Smoothed values:

  double                          m_Drive_dB          = paramValues[kParamDrive].default;
  double                          m_Drive_Real        = DBToAmp(m_Drive_dB);

  double                          m_LowPos            = paramValues[kParamLow].default;   // Position of Low knob, -10..+10
  double                          m_Mid_dB            = paramValues[kParamMid].default;
  double                          m_HighPos           = paramValues[kParamHigh].default;  // Position of High knob, -10..+10

  double                          m_Output_dB         = paramValues[kParamOutput].default;
  double                          m_Output_Real       = DBToAmp(m_Output_dB);

  double                          m_Active            = paramValues[kParamActive].default;

  // Knobs (for enabling / disabling):
  IControl*                       m_Controls[kNumParams];

  SmootherCallback                smootherCallback;
  SP_ParameterSmoother            smoother            = SP_ParameterSmoother(kNumParams);

  Iir::Butterworth::HighPass<1>   lowCut      [kMaxNumChannels];
  Iir::RBJ::LowShelf              lowShelf    [kMaxNumChannels];

  Iir::RBJ::BandShelf             midBefore   [kMaxNumChannels];
  Iir::RBJ::BandShelf             midAfter    [kMaxNumChannels];

  Iir::RBJ::LowPass               highCut     [kMaxNumChannels];
  Iir::RBJ::HighShelf             highShelf   [kMaxNumChannels];

  WaveShaperAsym2              waveshaper  [kMaxNumChannels];
  Iir::Butterworth::HighPass<1>   dcBlock     [kMaxNumChannels];

  inline void updateKnobs();
  inline void updateStages(double _sampleRate, bool _resetting);

public:
  NAMpanion(const InstanceInfo& info);
  void OnReset() override;
  void OnParamChange(int paramIdx) override;
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;

};
