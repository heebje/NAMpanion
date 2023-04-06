#include "NAMpanion.h"

#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

#include "NAM_Styling.h"
#include "NAMpanion_CornerResizers.h"
#include "NAMpanion_Common.h"


using namespace NAM_Styling;
using namespace NAMpanion_Common;

NAMpanion::NAMpanion(const InstanceInfo& info): iplug::Plugin(info, MakeConfig(kNumParams, kNumPresets)) {

  for (int p = 0; p < kNumParams; p++) {

    if (paramNames[p]  == NULL) {
      FAIL('Parameter name not definied');
    }

    switch (p) {

      // dBs:
      case kParamDrive:
      case kParamMid:
      case kParamOutput: {
        GetParam(p)->InitGain(paramNames [p],
                              paramValues[p].default,
                              paramValues[p].minimum,
                              paramValues[p].maximum,
                              paramValues[p].step);
        break;
      }

      // Double:
      case kParamLow:
      case kParamHigh: {
        GetParam(p)->InitDouble(paramNames [p],
                                paramValues[p].default,
                                paramValues[p].minimum,
                                paramValues[p].maximum,
                                paramValues[p].step);
        break;
      }

      // Boolean:
      case kParamActive: {
        GetParam(p)->InitBool(paramNames [p],
                              paramValues[p].default);
        break;
      }

      default: {
        FAIL('Parameter missing');
        break;
      }

    }

  }

  smoother.reset(this,
  				       kSmoothingTimeMs);

  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };

  mLayoutFunc = [&](IGraphics* pGraphics) {

    pGraphics->EnableMouseOver(true);

    pGraphics->AttachCornerResizer(new NAMCornerResizer(IRECT(0, 0, PLUG_WIDTH, PLUG_HEIGHT),
                                                        24.0,
                                                        NAM_1,
                                                        NAM_3.WithOpacity(0.75),
                                                        NAM_2));
    pGraphics->AttachControl(new NAMCornerShrinker(24.0,
                                                   NAM_1,
                                                   NAM_3.WithOpacity(0.75)));

    pGraphics->AttachPanelBackground(COLOR_BLACK);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);

    const auto  b           = pGraphics->GetBounds();
    const auto  mainArea    = b.GetPadded(-20);
    const auto  content     = mainArea.GetPadded(-10);
    const auto  titleHeight = 50.0;
    const auto  titleLabel  = content.GetFromTop(titleHeight);

    pGraphics->AttachControl(new IVPanelControl(mainArea,
                                                "",
                                                style.WithColor(kFG,
                                                                NAM_1)));

    auto title = std::string(PLUG_MFR) + " - " + PLUG_NAME;

    pGraphics->AttachControl(new IVLabelControl(titleLabel,
                                                title.c_str(),
                                                style.WithDrawFrame(false).WithValueText({30,
                                                                                          EAlign::Center,
                                                                                          NAM_3})));

    for (int p = 0; p < kNumParams; p++) {
      if (p == kParamActive) {
        pGraphics->AttachControl(new IVToggleControl(controlCoordinates[p],
                                                     p,
                                                     paramNames[p],
                                                     style.WithShowLabel(false),
                                                     "off",
                                                     "on"));
        m_Controls[p] = 0;
      } else {
        m_Controls[p] = pGraphics->AttachControl(new IVKnobControl(controlCoordinates[p],
                                                                   p,
                                                                   paramNames[p],
                                                                   style));
      }
    }

    updateKnobs();

  };

}

void NAMpanion::OnReset() {

  const double sr = GetSampleRate();

  // To prevent useless
  //   "IAudioProcessor::process (..) generates non silent output for silent input for tail above 0 samples"
  // error.
  //
  SetTailSize((int)(sr + 0.5));

  smoother.reset(this,
                 kSmoothingTimeMs);

  updateStages(true);

}

void NAMpanion::OnParamChange(int paramIdx) {
  smoother.change(paramIdx, GetParam(paramIdx)->Value());

  if ((paramIdx == kParamActive) && GetUI()) {
    updateKnobs();
  }
}

void NAMpanion::ProcessBlock(sample** inputs, sample** outputs, int nFrames) {

  // const double  sr        = GetSampleRate();
  const int     nInChans  = NInChansConnected();
  const int     nOutChans = NOutChansConnected();
  const int     nMaxChans = std::max(nInChans, nOutChans);

  for (int s = 0; s < nFrames; s++) {

  	updateStages(false);

    for (int ch = 0; ch < nMaxChans; ch++) {

      if ((nOutChans > nInChans) && (ch == 1)) { // The second output channel in the "1-2" situation
        outputs[1][s] = outputs[0][s]; // Simply copy

      } else {

        if (m_Active == 0.0) {
          // Bypassed:
          outputs[ch][s] = inputs[ch][s];

        } else {

          outputs[ch][s] =
            m_Output_Real *
              -dcBlock[ch].filter(  // Minus, because this filter erroneously inverts polarity.
                                    // I reported this bug, but the developer denied there was a problem.
                highCut[ch].filter(
                  lowShelf[ch].filter(
                    midAfter[ch].filter(
                      waveshaper[ch].processAudioSample(
                        m_Drive_Real *
                          midBefore[ch].filter(
                            highShelf[ch].filter(
                              -lowCut[ch].filter( // Minus, because this filter erroneously inverts polarity.
                                                  // I reported this bug, but the developer denied there was a problem.
                                inputs[ch][s]))))))));

          if (m_Active != 1.0) {
            // Transition: ////////////////////////////////////////////////////////

            outputs[ch][s] =

              ((1.0 - m_Active) * inputs [ch][s]) +
              ((m_Active)       * outputs[ch][s]);

          }
        }

      }

    }
  }


}

void NAMpanion::updateKnobs() {
  if (GetUI()) {

    float weight = float((1.0 + GetParam(kParamActive)->Value()) / 2.0f);

    for (int p = 0; p < kNumParams; p++) {
      auto ctrl = m_Controls[p];
      if (ctrl) {
        ctrl->SetBlend(IBlend(EBlend::Default, weight));
        ctrl->SetDirty(false);
      }

    }
  }
}

inline void NAMpanion::AdjustMid(int ch) {

  const double  sr            = GetSampleRate();
  const double  midFreq       = getMidFreq     (m_LowPos, m_HighPos);
  const double  midBandwidth  = getMidBandwidth(m_Mid_dB);

  if (m_Mid_dB > 0.0) {
    // Only the pre-drive mid is active:
    midBefore[ch].setup(sr, midFreq, m_Mid_dB, midBandwidth);
    midAfter [ch].setup(sr, midFreq, 0.0,      midBandwidth);
  } else {
    midBefore[ch].setup(sr, midFreq, 0.0,      midBandwidth);
    midAfter [ch].setup(sr, midFreq, m_Mid_dB, midBandwidth);
  }
}

inline void NAMpanion::updateStages(bool _resetting) {

  const double sr = GetSampleRate();

  // Non-parameter-related stages:
  if (_resetting) {
    for (int ch = 0; ch < kMaxNumChannels; ch++) {
      dcBlock[ch].setup(sr, kDCBlockFreq);
    }
  }

  // Parameter-related stages:
  for (int p = 0; p < kNumParams; p++) {

    switch (p) {

      case kParamDrive: {
        double v;
        if (smoother.get(p, v) || _resetting) {

          m_Drive_Real = DBToAmp(v);

          for (int ch = 0; ch < kMaxNumChannels; ch++) {
            waveshaper[ch].setA((v                      - paramValues[p].minimum) /
                                (paramValues[p].maximum - paramValues[p].minimum));
          }
        }
        break;
      }

      case kParamLow: {
        double v;
        if (smoother.get(p, v) || _resetting) {

          m_LowPos = v;  // Knob setting

          double lowShelfBoost  = getLowBoost(m_LowPos);

          double lowCutFreq;
          double lowShelfFreq;

          if (m_LowPos < 0.0) {
            lowCutFreq   = getLowFreq(m_LowPos);
            lowShelfFreq = kLowFreqMin;
          } else {
            lowCutFreq   = kLowFreqMin;
            lowShelfFreq = getLowFreq(m_LowPos);
          }

          for (int ch = 0; ch < kMaxNumChannels; ch++) {
            lowCut   [ch].setup(sr, lowCutFreq);
            lowShelf [ch].setup(sr, lowShelfFreq, lowShelfBoost, kLowBoostSlope);

            AdjustMid(ch);
          }
        }
        break;
      }

      case kParamMid: {
        double v;
        if (smoother.get(p, v) || _resetting) {

          m_Mid_dB = v;  // Knob setting == Value in dB

          for (int ch = 0; ch < kMaxNumChannels; ch++) {
            AdjustMid(ch);
          }
        }
        break;
      }

      case kParamHigh: {
        double v;
        if (smoother.get(p, v) || _resetting) {

          m_HighPos = v;  // Knob setting

          double highShelfBoost = getHighBoost   (m_HighPos);

          double highCutFreq;
          double highShelfFreq;

          if (m_HighPos < 0.0) {
            highCutFreq   = getHighFreq(m_HighPos);
            highShelfFreq = kHighFreqMax;
          } else {
            highCutFreq   = kHighFreqMax;
            highShelfFreq = getHighFreq(m_HighPos);
          }

          for (int ch = 0; ch < kMaxNumChannels; ch++) {
            highCut  [ch].setup(sr, highCutFreq);
            highShelf[ch].setup(sr, highShelfFreq, highShelfBoost, kHighBoostSlope);

            AdjustMid(ch);
          }
        }
        break;
      }

      case kParamOutput: {
        double v;
        if (smoother.get(p, v) || _resetting) {
          m_Output_Real = DBToAmp(v);
        }
        break;
      }

      case kParamActive: {
        double v;
        if (smoother.get(p, v) || _resetting) {
          m_Active = v;
        }
        break;
      }

      default: {
        FAIL(Wrong number of parameters);
        break;
      }

    }
  }
}
