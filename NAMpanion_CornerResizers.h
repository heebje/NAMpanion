#pragma once

#include "IControl.h"
#include "ICornerResizerControl.h"

class NAMCornerShrinker: public IControl {
public:
  NAMCornerShrinker(float size = 20.0,
                    const IColor& color           = COLOR_TRANSLUCENT,
                    const IColor& mouseOverColour = COLOR_BLACK): IControl(IRECT(0, 0, size, size)),
                                                                  mSize(size),
                                                                  mColor(color),
                                                                  mMouseOverColor(mouseOverColour) {
  }

  void Draw(IGraphics& g) override {
    const IColor &color = GetMouseIsOver() ? mMouseOverColor : mColor;
    g.FillTriangle(color, mRECT.L, mRECT.B, mRECT.R, mRECT.T, mRECT.L, mRECT.T);
  }

  void OnMouseDblClick(float x, float y, const IMouseMod& mod) override {
    IRECT r = GetUI()->GetBounds();
    GetUI()->Resize(r.W(), r.H(), 0.5);
  }

  void OnMouseOver(float x, float y, const IMouseMod& mod) override {
    if (!mMouseOver) {
      mPrevCursorType = GetUI()->SetMouseCursor(ECursor::HAND);
    }
    mMouseOver = true;
    IControl::OnMouseOver(x, y, mod);
  }

  void OnMouseOut() override {
    if (mMouseOver) {
      GetUI()->SetMouseCursor(mPrevCursorType);
    }
    mMouseOver = false;
    IControl::OnMouseOut();
  }

private:
  float   mSize;
  bool    mMouseOver = false;
  ECursor mPrevCursorType = ECursor::ARROW;
  IColor  mColor, mMouseOverColor;
};

///////////////////////////////////////////////////////////////////////////////

class NAMCornerResizer: public ICornerResizerControl {

public:
  NAMCornerResizer(const  IRECT&  graphicsBounds,
                   const  float   size            = 20.0,
                   const  IColor& color           = COLOR_BLACK.WithOpacity(0.5),
                   const  IColor& mouseOverColour = COLOR_BLACK,
                   const  IColor& dragColor       = COLOR_BLACK): ICornerResizerControl(graphicsBounds.GetFromBRHC(size, size),
                                                                                        size,
                                                                                        color,
                                                                                        mouseOverColour,
                                                                                        dragColor) {
    mSize                   = size;
    mInitialGraphicsBounds  = graphicsBounds;
    mColor                  = color;
    mMouseOverColor         = mouseOverColour;
    mDragColor              = dragColor;
  }

  void Draw(IGraphics& g) override {
    const IColor &color = GetUI()->GetResizingInProcess() ? mDragColor : GetMouseIsOver()? mMouseOverColor : mColor;
    g.FillTriangle(color, mRECT.L, mRECT.B, mRECT.R, mRECT.T, mRECT.R, mRECT.B);
  }

  void OnMouseDblClick(float x, float y, const IMouseMod& mod) override {
    IRECT r = GetUI()->GetBounds();
    GetUI()->Resize(r.W(), r.H(), 1.0);
  }

  void OnMouseOver(float x, float y, const IMouseMod& mod) override {
    if (!mMouseOver)
      mPrevCursorType = GetUI()->SetMouseCursor(ECursor::SIZENWSE);
    mMouseOver = true;
    IControl::OnMouseOver(x, y, mod);
  }

  void OnMouseOut() override {
    if (mMouseOver)
      GetUI()->SetMouseCursor(mPrevCursorType);
    mMouseOver = false;
    IControl::OnMouseOut();
  }

private:
  float mSize;
  bool mMouseOver = false;
  ECursor mPrevCursorType = ECursor::ARROW;
  IRECT mInitialGraphicsBounds;
  IColor mColor, mMouseOverColor, mDragColor;

};
