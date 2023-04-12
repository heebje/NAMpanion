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
      case kParamOutput:
      case kParamLowMaxBoost:
      case kParamHighMaxBoost: {
        GetParam(p)->InitGain(paramNames [p],
                              paramValues[p].def,
                              paramValues[p].min,
                              paramValues[p].max,
                              paramValues[p].step);
        break;
      }

      // Double:
      case kParamLow:
      case kParamHigh: {
        GetParam(p)->InitDouble(paramNames [p],
                                paramValues[p].def,
                                paramValues[p].min,
                                paramValues[p].max,
                                paramValues[p].step);
        break;
      }

      // Boolean:
      case kParamActive:
      case kParamOversampling: {
        GetParam(p)->InitBool(paramNames [p],
                              paramValues[p].def);
        break;
      }

      /*case kParamOversampling: {
        GetParam(p)->InitEnum(paramNames[p],
                              paramValues[p].def,
                              { NAM_OVERSAMPLING_FACTORS_VA_LIST });
        break;
      }*/

      // Frequency:
      case kParamLowFreqMin:
      case kParamHighFreqMax: {
        GetParam(p)->InitDouble(paramNames [p],         // InitDouble, to get rid of units showing
                                paramValues[p].def,
                                paramValues[p].min,
                                paramValues[p].max,
                                paramValues[p].step,
                                "", 0, "", IParam::ShapeExp(), IParam::kUnitCustom);
        break;
      }

      default: {
        FAIL("Parameter missing");
        break;
      }

    }

  }

  smoother.reset(this, kSmoothingTimeMs);

  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };

  mLayoutFunc = [&](IGraphics* pGraphics) {

    pGraphics->EnableMouseOver(true);
    pGraphics->EnableTooltips(true);

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
    pGraphics->AttachControl(new IURLControl(titleLabel.GetMidHPadded(160),
                                             "",
                                             PLUG_URL_STR));

    for (int p = 0; p < kNumParams; p++) {

      switch(p) {

        // Big knobs: /////////////////////////////////////////////////////////
        case kParamDrive:
        case kParamLow:
        case kParamMid:
        case kParamHigh:
        case kParamOutput: {
          m_Controls[p] = pGraphics->AttachControl(new IVKnobControl(controlCoordinates[p],
                                                                     p,
                                                                     paramLabels[p],
                                                                     style));
          m_Controls[p]->SetTooltip(paramToolTips[p]);
          break;
        }

        case kParamActive: {
          // On/Off switch:
          pGraphics->AttachControl(new IVToggleControl(controlCoordinates[p],
                                                       p,
                                                       paramLabels[p],
                                                       style.WithShowLabel(false),
                                                       "off",
                                                       "on"))
            ->SetTooltip(paramToolTips[p]);
          // Not to be disabled:
          m_Controls[p] = 0;
          break;
        }

        case kParamOversampling: {
          // Oversampling switch:
          m_Controls[p] = pGraphics->AttachControl(new IVToggleControl(controlCoordinates[p],
                                                                       p,
                                                                       paramLabels[p],
                                                                       style.WithShowLabel(false),
                                                                       "none",
                                                                       "16x"));
          m_Controls[p]->SetTooltip(paramToolTips[p]);
          break;

          //m_Controls[p] = pGraphics->AttachControl(new IVButtonControl(controlCoordinates[p],
          //                                                             [&, pGraphics](IControl* pCaller) {
          //  try {
          //    SplashClickActionFunc(pCaller);
          //    static IPopupMenu menu { "Menu",
          //                            { NAM_OVERSAMPLING_FACTORS_VA_LIST },
          //                            [&](IPopupMenu* pMenu) {
          //                              try {
          //                                auto* itemChosen = pMenu->GetChosenItem();
          //                                if (itemChosen) {
          //                                  pCaller->As<IVButtonControl>()->SetValueStr(itemChosen->GetText());
          //                                  bool found = false;
          //                                  for (int f = 0; f < ENAMpanionFactor::kNumNAMpanionFactors; f++) {
          //                                    if (strcmp(itemChosen->GetText(), OSFactorLabels[f]) == 0) {
          //                                      // Simply set new value here; will get picked up in updateStages()
          //                                      m_Oversampling = ENAMpanionFactor(f);
          //                                      /*SendParameterValueFromUI(kParamOversampling,
          //                                                               GetParam(kParamOversampling)->ToNormalized(m_Oversampling));
          //                                      */
          //                                      found = true;
          //                                      break;
          //                                    }
          //                                  }
          //                                  if (!found) {
          //                                    FAIL(Something wrong with OS options);
          //                                  }

          //                                }
          //                              } catch(...) {
          //                                BRK;
          //                              }
          //                            }
          //                           };
          //    float x, y;
          //    pGraphics->GetMouseDownPoint(x, y);
          //    pGraphics->CreatePopupMenu(*pCaller, menu, x, y);
          //  } catch(...) {
          //    BRK;
          //  }
          //}, "", style.WithValueText(style.labelText)
          //            .WithColor(EVColor::kFG, activeColorSpec.GetColor(EVColor::kPR))));
          //m_Controls[p]->As<IVButtonControl>()->SetValueStr(OSFactorLabels[m_Oversampling]);
          //m_Controls[p]->SetTooltip(paramToolTips[p]);
          //break;
          //
        }

        case kParamLowFreqMin:
        case kParamHighFreqMax:
        case kParamLowMaxBoost:
        case kParamHighMaxBoost: {
          // Little tweak knoblets:
          m_Controls[p] = pGraphics->AttachControl(new IVKnobControl(controlCoordinates[p],
                                                                     p,
                                                                     paramLabels[p],
                                                                     style.WithShowValue(true)
                                                                          .WithLabelText(IText(10,
                                                                                               EVAlign::Top,
                                                                                               NAM_3))
                                                                          .WithValueText(IText(10,
                                                                                               EVAlign::Bottom,
                                                                                               NAM_3))));
          m_Controls[p]->SetTooltip(paramToolTips[p]);
          break;
        }

        default: {
          FAIL("Parameter missing");
          break;
        }
      }

    }

    // EQ Plot:

    auto plotPanel = IRECT(45,230,PLUG_WIDTH-45,PLUG_HEIGHT-45);

    pGraphics->AttachControl(new IPanelControl(plotPanel,
                                                IPattern(NAM_2.WithContrast(-0.75))
                                              ));

    m_Plot = pGraphics->AttachControl(new IVPlotControl(plotPanel,
                                                        {
                                                          { NAM_2,
                                                            [&](double x) -> double {
                                                              return m_PlotValues[int(x * (PLUG_WIDTH-1))];
                                                            }
                                                          }
                                                        },
                                                        pGraphics->Width()));
    m_Plot->SetBlend(IBlend(EBlend::Default, 0.75));

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

  /*if (paramIdx == xkParamOversampling) {
    m_Oversampling = ENAMpanionFactor(GetParam(paramIdx)->Value());

    if (GetUI() && m_Controls[paramIdx]) {
      m_Controls[paramIdx]->As<IVButtonControl>()->SetValueStr(OSFactorLabels[m_Oversampling]);
    }

  } else {*/

  // Smoothed parameters:
  smoother.change(paramIdx, GetParam(paramIdx)->Value());

  if ((paramIdx == kParamActive) && GetUI()) {
    // Reflect in knob appearances:
    updateKnobs();
  }

}

void NAMpanion::OnIdle() {
  if (m_PlotNeedsRecalc && m_Plot && GetUI()) {

    if (m_Active) {

      const double sr = GetSampleRate();

      m_LowCut    [kPlotChannel].setup(sr, getLowCutFreq());
      m_LowShelf  [kPlotChannel].setup(sr, getLowShelfFreq(),   getLowShelfBoost(),   kLowBoostSlope);

      m_MidBefore [kPlotChannel].setup(sr, getMidFreq(),        getMidBefore_dB(),    getMidBandwidth());
      m_MidAfter  [kPlotChannel].setup(sr, getMidFreq(),        getMidAfter_dB(),     getMidBandwidth());

      m_HighCut   [kPlotChannel].setup(sr, getHighCutFreq());
      m_HighShelf [kPlotChannel].setup(sr, getHighShelfFreq(),  getHighShelfBoost(),  kHighBoostSlope);

      m_DCBlock   [kPlotChannel].setup(sr, kDCBlockFreq);

      const double ln1000 = std::log(1000.0);

      const double  maxBoost = DBToAmp(std::max({ getLowShelfBoost(),
                                                  getMidBefore_dB(),
                                                  getHighShelfBoost() }));

      for (int s = 0; s < PLUG_WIDTH; s++) {
        double f = 20.0 * exp(ln1000 * s / (PLUG_WIDTH)) / sr;
        m_PlotValues[s] = std::log(abs(m_LowCut   [kPlotChannel].response(f))
                                 * abs(m_LowShelf [kPlotChannel].response(f))

                                 * abs(m_MidBefore[kPlotChannel].response(f))
                                 * abs(m_MidAfter [kPlotChannel].response(f))

                                 * abs(m_HighCut  [kPlotChannel].response(f))
                                 * abs(m_HighShelf[kPlotChannel].response(f))

                                 * abs(m_DCBlock  [kPlotChannel].response(f))

                                 / maxBoost
                                  ) * 0.5 + 0.5;

      }
    } else {
      for (int s = 0; s < PLUG_WIDTH; s++) {
        m_PlotValues[s] = 0.0;
      }
    }

    m_PlotNeedsRecalc = false;
    m_Plot->SetDirty(false);

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

          // Bypassed: ////////////////////////////////////////////////////////

          outputs[ch][s] = inputs[ch][s];

        } else {

          outputs[ch][s] =
            m_Output_Real *
              -m_DCBlock[ch].filter(  // Minus, because this filter erroneously inverts polarity.
                                      // I reported this bug, but the developer denied there was a problem.
                m_HighCut[ch].filter(
                  m_LowShelf[ch].filter(
                    m_MidAfter[ch].filter(

                      m_Oversampler[ch]/*[0]*/.Process(
                        m_Drive_Real *
                          m_MidBefore[ch].filter(
                            m_HighShelf[ch].filter(
                              -m_LowCut[ch].filter( // Minus, because this filter erroneously inverts polarity.
                                                    // I reported this bug, but the developer denied there was a problem.
                                inputs[ch][s]))),
                        /*[&](sample input) {
                          return m_Oversampler[ch][1].Process(
                            input,*/
                        [&](sample input) {
                          return m_Waveshaper[ch].processAudioSample(input);
                        }
                      )
                    )
                  )
                )
              );

          if (m_Active != 1.0) {

            // Transition: ////////////////////////////////////////////////////

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

inline void NAMpanion::AdjustLow() {

  const double  sr            = GetSampleRate();
  const double  lowShelfBoost = getLowShelfBoost();
  double        lowCutFreq    = getLowCutFreq();
  double        lowShelfFreq  = getLowShelfFreq();

  for (int ch = 0; ch < kMaxNumChannels; ch++) {
    m_LowCut  [ch].setup(sr, lowCutFreq);
    m_LowShelf[ch].setup(sr, lowShelfFreq, lowShelfBoost, kLowBoostSlope);
  }

  m_PlotNeedsRecalc = true;

}

inline void NAMpanion::AdjustMid() {

  const double  sr            = GetSampleRate();
  const double  midFreq       = getMidFreq();
  const double  midBefore_dB  = getMidBefore_dB();
  const double  midAfter_dB   = getMidAfter_dB();
  const double  midBandwidth  = getMidBandwidth();

  for (int ch = 0; ch < kMaxNumChannels; ch++) {
    m_MidBefore[ch].setup(sr, midFreq, midBefore_dB, midBandwidth);
    m_MidAfter [ch].setup(sr, midFreq, midAfter_dB,  midBandwidth);
  }

  m_PlotNeedsRecalc = true;

}

inline void NAMpanion::AdjustHigh() {

  const double  sr              = GetSampleRate();
  const double  highShelfBoost  = getHighShelfBoost();
  double        highCutFreq     = getHighCutFreq();
  double        highShelfFreq   = getHighShelfFreq();

  for (int ch = 0; ch < kMaxNumChannels; ch++) {
    m_HighCut  [ch].setup(sr, highCutFreq);
    m_HighShelf[ch].setup(sr, highShelfFreq, highShelfBoost, kHighBoostSlope);
  }

  m_PlotNeedsRecalc = true;

}

inline void NAMpanion::AdjustOversampling() {
  for (int ch = 0; ch < kMaxNumChannels; ch++) {

    if (m_Oversampling >= 0.5) {
      m_Oversampler[ch].SetOverSampling(EFactor::k16x);
    } else {
      m_Oversampler[ch].SetOverSampling(EFactor::kNone);
    }

    /*m_Oversampler[ch][0].SetOverSampling(EFactor(std::min(int(m_Oversampling),
                                                          int(EFactor::k16x))));
    m_Oversampler[ch][1].SetOverSampling(EFactor(std::max(int(m_Oversampling) - int(EFactor::k16x),
                                                          int(EFactor::kNone))));*/
  }
}

inline void NAMpanion::updateStages(bool _resetting) {

  const double sr = GetSampleRate();

  // Non-parameter-related stages:
  if (_resetting) {
    for (int ch = 0; ch < kMaxNumChannels; ch++) {

      // for (int os = 0; os < 2; os++) {
      m_Oversampler[ch]/*[os]*/.Reset(GetBlockSize());
      //}

      m_DCBlock[ch].setup(sr, kDCBlockFreq);

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
            m_Waveshaper[ch].setA((v                  - paramValues[p].min) /
                                  (paramValues[p].max - paramValues[p].min));
          }
        }
        break;
      }

      case kParamLow: {
        double v;
        if (smoother.get(p, v) || _resetting) {
          m_LowPos = v;  // Knob setting
          AdjustLow();
          AdjustMid();
        }
        break;
      }

      case kParamMid: {
        double v;
        if (smoother.get(p, v) || _resetting) {
          m_Mid_dB = v;  // Knob setting == Value in dB
          AdjustMid();
        }
        break;
      }

      case kParamHigh: {
        double v;
        if (smoother.get(p, v) || _resetting) {
          m_HighPos = v;  // Knob setting
          AdjustMid();
          AdjustHigh();
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
          m_PlotNeedsRecalc = true;
        }
        break;
      }

      case kParamOversampling: {
        double v;
        if (smoother.get(p, v) || _resetting) {
          m_Oversampling = v;
          AdjustOversampling();
        }
        break;
        //if (m_Oversampling != m_PrevOversampling) {
        //  m_PrevOversampling = m_Oversampling;
        //  AdjustOversampling();
        //  // SetParameterValue(p, GetParam(p)->ToNormalized(m_Oversampling));

        //  // SendParameterValueFromAPI doesn't work
        //  // SendParameterValueFromDelegate doesn't work
        //  SendParameterValueFromUI(p, m_Oversampling, false);
        //  x;
        //    /*FromUI(kParamOversampling,
        //    GetParam(kParamOversampling)->ToNormalized(m_Oversampling));*/
        //}
        //break;
      }

      // Tweaking knoblets:
      case kParamLowFreqMin: {
        double v;
        if (smoother.get(p, v) || _resetting) {
          m_LowFreqMin = v;
          AdjustLow ();
          AdjustMid ();
          AdjustHigh();
        }
        break;
      }

      case kParamHighFreqMax: {
        double v;
        if (smoother.get(p, v) || _resetting) {
          m_HighFreqMax = v;
          AdjustLow ();
          AdjustMid ();
          AdjustHigh();
        }
        break;
      }

      case kParamLowMaxBoost: {
        double v;
        if (smoother.get(p, v) || _resetting) {
          m_LowMaxBoost = v;
          AdjustLow();
        }
        break;
      }

      case kParamHighMaxBoost: {
        double v;
        if (smoother.get(p, v) || _resetting) {
          m_HighMaxBoost = v;
          AdjustHigh();
        }
        break;
      }

      default: {
        FAIL("Parameter missing");
        break;
      }

    }
  }
}
