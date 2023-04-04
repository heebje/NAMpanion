#pragma once

#include "IControl.h"
#include "ICornerResizerControl.h"

BEGIN_IPLUG_NAMESPACE
BEGIN_IGRAPHICS_NAMESPACE

class CornerShrinker: public IControl {
public:
  CornerShrinker(float size = 20.0,
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

class CornerResizer: public ICornerResizerControl {
public:
  CornerResizer(const  IRECT&  graphicsBounds,
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

    //ICornerResizerControl(const IRECT& graphicsBounds, float size, const IColor& color = COLOR_TRANSLUCENT, const IColor& mouseOverColour = COLOR_BLACK, const IColor& dragColor = COLOR_BLACK)
    //  : IControl(graphicsBounds.GetFromBRHC(size, size).GetPadded(-1))
  }

  void Draw(IGraphics& g) override {
    const IColor &color = GetUI()->GetResizingInProcess() ? mDragColor : GetMouseIsOver()? mMouseOverColor : mColor;
    g.FillTriangle(color, mRECT.L, mRECT.B, mRECT.R, mRECT.T, mRECT.R, mRECT.B);
  }

  /*void OnMouseDown(float x, float y, const IMouseMod& mod) override {
    GetUI()->StartDragResize();
  }*/

  void OnMouseDblClick(float x, float y, const IMouseMod& mod) override
  {
    IRECT r = GetUI()->GetBounds();
    GetUI()->Resize(r.W(), r.H(), 1.0);
    // GetUI()->Resize(static_cast<int>(mInitialGraphicsBounds.W()), static_cast<int>(mInitialGraphicsBounds.H()), 1.f);
  }

  void OnRescale() override
  {
    /*float size = mSize * (1.f/GetUI()->GetDrawScale());
    IRECT r = GetUI()->GetBounds().GetFromBRHC(size, size);
    SetTargetAndDrawRECTs(r);*/
  }

  void OnMouseOver(float x, float y, const IMouseMod& mod) override
  {
    if (!mMouseOver)
      mPrevCursorType = GetUI()->SetMouseCursor(ECursor::SIZENWSE);
    mMouseOver = true;
    IControl::OnMouseOver(x, y, mod);
  }

  void OnMouseOut() override
  {
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


/*
class SP_CornerResizer: public IControl {
public:
  SP_CornerResizer(const float    graphicsWidth,
                   const float    graphicsHeight,
                   const float    size            = 20.0,
                   const IColor&  color           = COLOR_TRANSLUCENT,
                   const IColor&  mouseOverColour = COLOR_BLACK,
                   const IColor&  dragColor       = COLOR_BLACK): IControl(IRECT(graphicsWidth  - size,
                                                                                 graphicsHeight - size,
                                                                                 graphicsWidth,
                                                                                 graphicsHeight)) {
    mSize                   = size;
    mInitialGraphicsBounds  = IRECT(0, 0, graphicsWidth, graphicsHeight);
    mColor                  = color;
    mMouseOverColor         = mouseOverColour;
    mDragColor              = dragColor;
  }

  void Draw(IGraphics& g) override {
    const IColor &color = mResizingInProcess ? mDragColor : GetMouseIsOver()? mMouseOverColor : mColor;
    g.FillTriangle(color, mRECT.L, mRECT.B, mRECT.R, mRECT.T, mRECT.R, mRECT.B);
  }

  void OnMouseDown(float x, float y, const IMouseMod& mod) override {
    mResizingInProcess = true;
  }

  void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod) {
    if (mResizingInProcess) {

      x = x * GetUI()->GetDrawScale() / mInitialGraphicsBounds.W();
      y = y * GetUI()->GetDrawScale() / mInitialGraphicsBounds.H();

      GetUI()->Resize(mInitialGraphicsBounds.W(),
                      mInitialGraphicsBounds.H(),
                      std::clamp(std::max(x, y), 0.5f, 2.0f));
      NOP
    }
  }

  void OnMouseUp(float x, float y, const IMouseMod& mod) {
    if (mResizingInProcess) {
      NOP;
      mResizingInProcess = false;
    }
  }

  void OnMouseDblClick(float x, float y, const IMouseMod& mod) override
  {
    GetUI()->Resize(static_cast<int>(mInitialGraphicsBounds.W()), static_cast<int>(mInitialGraphicsBounds.H()), 1.f);
  }

  //void OnRescale() override {
  //  float size = mSize * (1.f/GetUI()->GetDrawScale());
  //  IRECT r = mInitialGraphicsBounds.GetFromBRHC(size, size);
  //  SetTargetAndDrawRECTs(r);
  //}

  void OnMouseOver(float x, float y, const IMouseMod& mod) override
  {
    if (!mMouseOver)
      mPrevCursorType = GetUI()->SetMouseCursor(ECursor::SIZENWSE);
    mMouseOver = true;
    IControl::OnMouseOver(x, y, mod);
  }

  void OnMouseOut() override
  {
    if (mMouseOver)
      GetUI()->SetMouseCursor(mPrevCursorType);
    mMouseOver = false;
    IControl::OnMouseOut();
  }
private:
  float   mSize;
  bool    mMouseOver = false;
  ECursor mPrevCursorType = ECursor::ARROW;
  IRECT   mInitialGraphicsBounds;
  IColor  mColor, mMouseOverColor, mDragColor;

  bool    mResizingInProcess = false;

};
*/

END_IGRAPHICS_NAMESPACE
END_IPLUG_NAMESPACE
