#pragma once

using namespace iplug;
using namespace igraphics;

namespace NAM_Styling {

  const IColor OFF_WHITE  = IColor(255, 243, 246, 249);
  const IColor NAM_1      = IColor(255, 29, 26, 31);    // Raisin Black
  const IColor NAM_2      = IColor(255, 80, 133, 232);  // Azure
  const IColor NAM_3      = IColor(255, 162, 178, 191); // Cadet Blue Crayola
  const IColor MOUSEOVER  = NAM_3.WithOpacity(0.3);
  const IColor HELP_TEXT  = COLOR_WHITE;

  // Styles
  const IVColorSpec activeColorSpec {
    DEFAULT_BGCOLOR,         // Background
    NAM_1,                   // Foreground
    NAM_2.WithOpacity(0.4f), // Pressed
    NAM_3,                   // Frame
    MOUSEOVER,               // Highlight
    DEFAULT_SHCOLOR,         // Shadow
    NAM_2,                   // Extra 1
    COLOR_RED,               // Extra 2
    DEFAULT_X3COLOR          // Extra 3
  };

  const IVStyle style = IVStyle {
    true, // Show label
    true, // Show value
    activeColorSpec,
    {
      DEFAULT_TEXT_SIZE + 5.f,
      EVAlign::Middle,
      NAM_3
    }, // Knob label text
    {
      DEFAULT_TEXT_SIZE + 5.f,
      EVAlign::Bottom,
      NAM_3
    }, // Knob value text
    DEFAULT_HIDE_CURSOR,
    DEFAULT_DRAW_FRAME,
    false,
    DEFAULT_EMBOSS,
    0.2f,
    2.f,
    DEFAULT_SHADOW_OFFSET,
    DEFAULT_WIDGET_FRAC,
    DEFAULT_WIDGET_ANGLE
  };

}