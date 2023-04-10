#pragma once

#include "IPlugParameter.h"
#include "NAMpanion_Common.h"

using namespace iplug;

struct Smoother {
  int                   m_TotalSteps  = int(0.02 * 48000.0);
  int                   m_StepsLeft   = 0;

  double                m_Value       = 0.0;
  double                m_Target      = 0.0;

  IParam::EDisplayType  m_DisplayType = IParam::EDisplayType::kDisplayLinear;
};

class ParameterSmoother {

private:
  Smoother* m_smoothers = NULL;

public:
  ParameterSmoother(int _numParams) {
    m_smoothers = new Smoother[_numParams];
  };
  ~ParameterSmoother() {
    delete[] m_smoothers;
  };

  inline void reset(Plugin*           _plugin,
                    double            _smoothingTimeMs) {

    const double  sr        = _plugin->GetSampleRate();
    const int     numParams = _plugin->NParams();

    for (auto p = 0; p < numParams; p++) {

      Smoother* smoother = &m_smoothers[p];

      smoother->m_Target      = _plugin->GetParam(p)->Value();
      smoother->m_DisplayType = _plugin->GetParam(p)->DisplayType();

      smoother->m_TotalSteps  = int(sr * _smoothingTimeMs / 1000.0);
      smoother->m_StepsLeft   = 0;
      smoother->m_Value       = smoother->m_Target;

    }
  }

  inline void change(int     _param,
                     double  _newValue) {

    Smoother* smoother = &m_smoothers[_param];

    smoother->m_Target  = _newValue;
    if (smoother->m_Value != smoother->m_Target) {
      smoother->m_StepsLeft = smoother->m_TotalSteps;
    } else {
      smoother->m_StepsLeft = 0;
    }
  }

  inline bool get(int      _param,
                  double&  _value) {

    Smoother* smoother = &m_smoothers[_param];

    bool changed = (smoother->m_StepsLeft > 0);

    if (changed) {

      switch (smoother->m_DisplayType) {

        case IParam::EDisplayType::kDisplayLinear: {
          smoother->m_Value -= (smoother->m_Value - smoother->m_Target) * 2.0 / (1.0 + (smoother->m_StepsLeft--));
          break;
        }

        case IParam::EDisplayType::kDisplayLog: {
          smoother->m_Value /= std::pow((smoother->m_Value / smoother->m_Target), (2.0 / (1.0 + (smoother->m_StepsLeft--))));
          break;
        }

        default: {
          NIY(kDisplayExp kDisplaySquared kDisplaySquareRoot kDisplayCubed kDisplayCubeRoot);
          break;
        }
      }
    } else {
      NOP
    }

    _value = smoother->m_Value;

    return changed;

  }

};
