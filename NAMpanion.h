#pragma once

// TO DO:
// - AGC
// - Web link to Shameless Plugs & NAM
// - mono / stereo / simulated stereo? => Not yet; only useful once NAM itself is stereo
//

#include "IPlug_include_in_plug_hdr.h"
#include "iir1/Iir.h"
#include "Oversampler.h"
#include "NAMpanion_ParamSmoother.h"
#include "NAMpanion_WaveShaper.h"


using namespace iplug;
using namespace igraphics;

const int     kNumPresets       = 1;
const int     kMaxNumChannels   = 2;

const int     kPlotChannel      = kMaxNumChannels;

const double  kSmoothingTimeMs  = 20.0; // Parameter smoothing in milliseconds

enum EParams {
  // Main parameters (the big knobs):
  kParamDrive = 0,
  kParamLow,
  kParamMid,
  kParamHigh,
  kParamOutput,
  kParamActive,
  kParamOversampling,

  // Tweaking knobs (small corner knobs):
  kParamLowFreqMin,
  kParamHighFreqMax,
  kParamLowMaxBoost,
  kParamHighMaxBoost,

  ///////////
  kNumParams
};

char* paramNames[kNumParams] = {
  "Drive",
  "Low",
  "Mid",
  "High",
  "Output",
  "Active",
  "Oversampling",

  "LowFrequencyTweak",
  "HighFrequencyTweak",
  "LowMaxBoost",
  "HighMaxBoost",
};

char* paramLabels[kNumParams] = {
  "Drive",
  "Low",
  "Mid",
  "High",
  "Output",
  "Active",
  "OS",

  "Lo Twk",
  "Hi Twk",
  "Lo Max",
  "Hi Max",
};

char* paramToolTips[kNumParams] = {
  "Drive (dB):\nControls the saturation / overdrive / distortion level",
  "Low:\nControls the overall bass response, by cutting lows before the drive section, or boosting lows after it",
  "Mid (dB):\nControls the overall midrange response, by boosting mids before the drive section, or cuttings mids after it",
  "High:\nControls the overall treble response, by boosting highs before the drive section, or cuttings highs after it",
  "Output (dB):\nControls the final output volume.\nIt can be set higher than unity gain in order to boost whatever comes after it",
  "Active:\nSwitches the plugin on or off",
  "Oversampling:\nSwitches oversampling to 16x, or off.\nLack of oversampling will lead to aliasing, especially at higher Drive settings",
  "Low Frequency Tweak:\nSets the 'bottom' of the EQ's total range",
  "High Frequency Tweak:\nSets the 'top' of the EQ's total range",
  "Low Max Boost:\nSets the maximum boost level of the low shelving filter (after the drive section)",
  "High Max Boost:\nSets the maximum boost level of the high shelving filter (before the drive section)",
};

struct VALUES {
  double default, minimum, maximum, step;
  VALUES(double def, double min, double max, double stp): default(def), minimum(min), maximum(max), step(stp) {};
  VALUES(boolean def): default(def), minimum(false), maximum(true), step(1.0) {};
};

VALUES paramValues[kNumParams] = {  // (default, minimum, maximum, step)

  // Main (big) knobs:
  VALUES(0.0,   0.0,  69.0, 0.01),  // Drive
  VALUES(0.0, -10.0, +10.0, 0.01),  // Low
  VALUES(0.0,  -9.0,  +9.0, 0.01),  // Mid
  VALUES(0.0, -10.0, +10.0, 0.01),  // High
  VALUES(0.0, -51.0, +18.0, 0.01),  // Output
  VALUES(true),                     // Active
  VALUES(true),                     // Oversampling

  // Tweaking (small) knoblets:

  VALUES(sqrt(   8.0*  200.0),    8.0,   200.0, 0.01),   // Low  Frequency Minimum
  VALUES(sqrt(2000.0*20000.0), 2000.0, 20000.0, 0.01),   // High Frequency Maximum

  VALUES( 6.0, 3.0,  9.0, 0.01),  // Low  Max Boost
  VALUES(12.0, 3.0, 21.0, 0.01),  // High Max Boost
};

IRECT controlCoordinates[kNumParams] = {
  IRECT(60 + 0*84, 100, 123 + 0*84, 215), // Drive
  IRECT(60 + 1*84, 100, 123 + 1*84, 215), // Low
  IRECT(60 + 2*84, 100, 123 + 2*84, 215), // Mid
  IRECT(60 + 3*84, 100, 123 + 3*84, 215), // High
  IRECT(60 + 4*84, 100, 123 + 4*84, 215), // Output

  IRECT(60 + 5*84, 100, 123 + 5*84, 150), // Active
  IRECT(60 + 5*84, 165, 123 + 5*84, 215), // Oversampling

  IRECT(              45+0*35, 35,         45+30+0*35, 35+40),  // Low  Frequency Minimum
  IRECT(PLUG_WIDTH-45-30-0*35, 35, PLUG_WIDTH-45-0*35, 35+40),  // high Frequency Maximum

  IRECT(              45+1*35, 35,         45+30+1*35, 35+40),  // Low  Max Boost
  IRECT(PLUG_WIDTH-45-30-1*35, 35, PLUG_WIDTH-45-1*35, 35+40),  // High Max Boost
};

const double  kFreqCentre       = sqrt(paramValues[kParamLowFreqMin ].minimum
                                     * paramValues[kParamHighFreqMax].maximum);

const double  kLowBoostSlope    =  1.0;
const double  kHighBoostSlope   =  1.0;
const double  kDCBlockFreq      = 10.0;

class NAMpanion final: public Plugin {
private:

  // Smoothed parameter values ////////////////////////////////////////////////

  // Main knobs:
  double  m_Drive_Real    = DBToAmp(paramValues[kParamDrive       ].default); // Input gain in real terms, from dB
  double  m_LowPos        =         paramValues[kParamLow         ].default;  // Position of Low knob, -10..+10
  double  m_Mid_dB        =         paramValues[kParamMid         ].default;  // Mid cut/boost in dB
  double  m_HighPos       =         paramValues[kParamHigh        ].default;  // Position of High knob, -10..+10
  double  m_Output_Real   = DBToAmp(paramValues[kParamOutput      ].default); // Output gain in real terms, from dB
  double  m_Active        =         paramValues[kParamActive      ].default;  // 0.0..1.0
  double  m_Oversampling  =         paramValues[kParamOversampling].default;  // 0.0..1.0

    // Little tweak knoblets:
  double  m_LowFreqMin    =         paramValues[kParamLowFreqMin  ].default;  // Lowest  frequency for low  filters
  double  m_HighFreqMax   =         paramValues[kParamHighFreqMax ].default;  // Highest frequency for high filters
  double  m_LowMaxBoost   =         paramValues[kParamLowMaxBoost ].default;  // Maximum boost of low  shelving filter
  double  m_HighMaxBoost  =         paramValues[kParamHighMaxBoost].default;  // Maximum boost of high shelving filter

  // Methods to calculate filter frequencies etc, based on current smoothed parameter values:

  double inline getEQCentre() { // Midpoint between lowest and highest frequencies that EQ can reach;
                                //   determined by tweak knoblets.
                                // This is NOT the same as the currently calculated mid frequency,
                                //   which depends on the low and high knob positions!
    return sqrt(m_LowFreqMin * m_HighFreqMax);
  }

  double inline getLowFreq() {  // Currently active low frequency, which determins either
                                // the low cut filter's or the low shelf filter's frequency,
                                // depending on low knob position.
    return m_LowFreqMin * pow(getEQCentre() / m_LowFreqMin, abs(m_LowPos / paramValues[kParamLow].maximum));
  }

  double inline getMidFreq() {  // Currently active mid frequency, which is the midpoint between the currently
                                // active low and high frequencies.
    return sqrt(getLowFreq() * getHighFreq());
  }

  double inline getMidBefore_dB() {
    return std::max(m_Mid_dB, 0.0);
  }

  double inline getMidAfter_dB() {
    return std::min(m_Mid_dB, 0.0);
  }

  double inline getLowCutFreq() {
    if (m_LowPos < 0.0) {
      return getLowFreq();
    } else {
      return m_LowFreqMin;
    }
  }

  double inline getLowShelfFreq() {
    if (m_LowPos < 0.0) {
      return m_LowFreqMin;
    } else {
      return getLowFreq();
    }
  }

  double inline getHighCutFreq() {
    if (m_HighPos < 0.0) {
      return getHighFreq();
    } else {
      return m_HighFreqMax;
    }
  }

  double inline getHighShelfFreq() {
    if (m_HighPos < 0.0) {
      return m_HighFreqMax;
    } else {
      return getHighFreq();
    }
  }

  double inline getHighFreq() { // Currently active high frequency, which determins either
                                // the high cut filter's or the high shelf filter's frequency,
                                // depending on high knob position.
    return m_HighFreqMax / pow(m_HighFreqMax / getEQCentre(), abs(m_HighPos / paramValues[kParamHigh].maximum));
  }

  double inline getLowShelfBoost() {  // Boost value for the low shelf filter, depending on low knob position
    return (tanh(2.0 * m_LowPos / m_LowMaxBoost) + 1.0) * m_LowMaxBoost / 2.0;
  }

  double inline getHighShelfBoost() { // Boost value for the high shelf filter, depending on high knob position
    return (tanh(2.0 * m_HighPos / m_HighMaxBoost) + 1.0) * m_HighMaxBoost / 2.0;
  }

  double inline getMidBandwidth() { // Boost-/cut-dependent bandwidth for mid peaking filter
    return pow(4.0/3.0, (m_Mid_dB + 27.0) / paramValues[kParamMid].maximum);
  }

  // Knobs (for enabling / disabling):
  IControl*                       m_Controls[kNumParams];

  ParameterSmoother               smoother = ParameterSmoother(kNumParams);

  // Filters etc:

  Iir::Butterworth::HighPass<1>   m_LowCut    [kMaxNumChannels + 1];
  Iir::RBJ::LowShelf              m_LowShelf  [kMaxNumChannels + 1];

  Iir::RBJ::BandShelf             m_MidBefore [kMaxNumChannels + 1];
  Iir::RBJ::BandShelf             m_MidAfter  [kMaxNumChannels + 1];

  Iir::RBJ::LowPass               m_HighCut   [kMaxNumChannels + 1];
  Iir::RBJ::HighShelf             m_HighShelf [kMaxNumChannels + 1];

  Iir::Butterworth::HighPass<1>   m_DCBlock   [kMaxNumChannels + 1];

  OverSampler<sample>             m_Oversampler[kMaxNumChannels] = {
                                    OverSampler(EFactor::k16x, false),  // 16x, no block processing
                                    OverSampler(EFactor::k16x, false)
                                  };
  WaveShaperAsym2                 m_Waveshaper[kMaxNumChannels];

  double                          m_PlotValues[PLUG_WIDTH];
  IControl*                       m_Plot = 0;
  bool                            m_PlotNeedsRecalc = true;

  inline void updateKnobs();
  inline void AdjustLow();
  inline void AdjustMid();
  inline void AdjustHigh();
  inline void AdjustOversampling();
  inline void updateStages(bool _resetting);

public:
  NAMpanion(const InstanceInfo& info);
  void OnReset() override;
  void OnParamChange(int paramIdx) override;
  void OnIdle() override;
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;

};
