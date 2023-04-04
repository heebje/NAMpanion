#pragma once
#pragma once

#include "NAMpanion_Common.h"

// Direct copy from IPlug2.
// Check sanity in SP_SmootherCallback derivative constructor!
enum SP_DisplayType {
  kDisplayLinear,
  kDisplayLog,
  kDisplayExp,
  kDisplaySquared,
  kDisplaySquareRoot,
  kDisplayCubed,
  kDisplayCubeRoot
};

// Abstract class to derive callback class from:
class SP_SmootherCallback {
public:
  virtual void getParamInfo(void*           _plugin,
    int             _paramNumber,
    double&         _sampleRate,
    double&         _value,
    SP_DisplayType& _dt) = 0;
};

struct SP_Smoother {
  int             m_TotalSteps  = int(0.02 * 48000.0);
  int             m_StepsLeft   = 0;

  double          m_Value       = 0.0;
  double          m_Target      = 0.0;

  SP_DisplayType  m_DisplayType = SP_DisplayType::kDisplayLinear;
};

class SP_ParameterSmoother {

private:

  SP_Smoother*  m_smoothers = NULL;

public:

  SP_ParameterSmoother(int _numParams) {
    m_smoothers = new SP_Smoother[_numParams];
  };
  ~SP_ParameterSmoother() {
    delete[] m_smoothers;
  };

  inline void reset(void*                 _plugin,
                    int                   _numParams,
                    double                _smoothingTimeMs,
                    SP_SmootherCallback&  _cb) {

    double sampleRate;

    for (auto p = 0; p < _numParams; p++) {

      SP_Smoother* smoother = &m_smoothers[p];

      _cb.getParamInfo(_plugin,
                       p,
                       sampleRate,
                       smoother->m_Target,
                       smoother->m_DisplayType);

      smoother->m_TotalSteps  = int(sampleRate * _smoothingTimeMs / 1000.0);
      smoother->m_StepsLeft   = 0;
      smoother->m_Value       = smoother->m_Target;

    }
  }

  inline void change(int     _param,
    double  _newValue) {

    SP_Smoother* smoother = &m_smoothers[_param];

    smoother->m_Target  = _newValue;
    if (smoother->m_Value != smoother->m_Target) {
      smoother->m_StepsLeft = smoother->m_TotalSteps;
    } else {
      smoother->m_StepsLeft = 0;
    }
  }

  inline bool SP_ParameterSmoother::get(int _param, double& _value) {

    SP_Smoother* smoother = &m_smoothers[_param];

    bool changed = (smoother->m_StepsLeft > 0);

    if (changed) {

      switch (smoother->m_DisplayType) {

      case SP_DisplayType::kDisplayLinear: {
        smoother->m_Value -= (smoother->m_Value - smoother->m_Target) * 2.0 / (1.0 + (smoother->m_StepsLeft--));
        break;
      }

      case SP_DisplayType::kDisplayLog: {
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
