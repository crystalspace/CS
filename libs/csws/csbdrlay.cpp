/////////////////////////////////////////////////////////////////////////////
// Copyright:   (c) Aleksandras Gluchovas
// Licence:     wxWindows license
//
// Ported to Crystal Space by Norman Kraemer <norman@users.sourceforge.net>
/////////////////////////////////////////////////////////////////////////////

#include "cssysdef.h"
#include "csws/csbdrlay.h"

csLayoutConstraint *csBorderConstraint::Clone ()
{
  return new csBorderConstraint (*this);
}

/***** Implementation for class BorderLayout *****/

CS_IMPLEMENT_STATIC_CLASSVAR (csBorderLayout, CENTER, GetCenter, csBorderConstraint, 
                              (csBorderLayout::_CENTER))
CS_IMPLEMENT_STATIC_CLASSVAR (csBorderLayout, EAST, GetEast, csBorderConstraint, 
                              (csBorderLayout::_EAST))
CS_IMPLEMENT_STATIC_CLASSVAR (csBorderLayout, NORTH, GetNorth, csBorderConstraint, 
                              (csBorderLayout::_NORTH))
CS_IMPLEMENT_STATIC_CLASSVAR (csBorderLayout, SOUTH, GetSouth, csBorderConstraint, 
                              (csBorderLayout::_SOUTH))
CS_IMPLEMENT_STATIC_CLASSVAR (csBorderLayout, WEST, GetWest, csBorderConstraint, 
                              (csBorderLayout::_WEST))

CS_IMPLEMENT_STATIC_CLASSVAR (csBorderLayout, AFTER_LAST_LINE, GetAfterLastLine, csBorderConstraint, 
                              (csBorderLayout::_AFTER_LAST_LINE))
CS_IMPLEMENT_STATIC_CLASSVAR (csBorderLayout, AFTER_LINE_ENDS, GetAfterLineEnds, csBorderConstraint, 
                              (csBorderLayout::_AFTER_LINE_ENDS))
CS_IMPLEMENT_STATIC_CLASSVAR (csBorderLayout, BEFORE_FIRST_LINE, GetBeforeFirstLine, csBorderConstraint, 
                              (csBorderLayout::_BEFORE_FIRST_LINE))
CS_IMPLEMENT_STATIC_CLASSVAR (csBorderLayout, BEFORE_LINE_BEGINS, GetBeforeLineBegins, csBorderConstraint, 
                              (csBorderLayout::_BEFORE_LINE_BEGINS))

csBorderLayout::csBorderLayout (csComponent *pParent)
  : csLayout2 (pParent), c (0), mHgap (0), mVgap (0)
{
  lc = &c;
  GetCenter ();
  GetNorth ();
  GetWest ();
  GetSouth ();
  GetEast ();
  GetAfterLastLine ();
  GetAfterLineEnds ();
  GetBeforeFirstLine ();
  GetBeforeLineBegins ();
}

csBorderLayout::csBorderLayout (csComponent *pParent, int hgap, int vgap)
  : csLayout2 (pParent), c (0), mHgap (hgap), mVgap (vgap)
{
  lc = &c;
  GetCenter ();
  GetNorth ();
  GetWest ();
  GetSouth ();
  GetEast ();
  GetAfterLastLine ();
  GetAfterLineEnds ();
  GetBeforeFirstLine ();
  GetBeforeLineBegins ();
}

csBorderLayout::~csBorderLayout ()
{
}

void csBorderLayout::SuggestSize (int &w, int &h)
{
  int cw, ch, ew, eh, nw, nh, sw, sh, ww, wh;
  cw = ch = ew = eh = nw = nh = sw = sh = ww = wh = 0;
  if (CENTER->comp) CENTER->comp->SuggestSize (cw, ch);
  if (EAST->comp) EAST->comp->SuggestSize (ew, eh);
  if (NORTH->comp) NORTH->comp->SuggestSize (nw, nh);
  if (SOUTH->comp) SOUTH->comp->SuggestSize (sw, sh);
  if (WEST->comp) WEST->comp->SuggestSize (ww, wh);

  w = GetWidthSum (ww, cw, ew, mHgap,
    WEST->comp != 0,CENTER->comp != 0, EAST->comp != 0);

  h = nh + ch + sh;
}

void csBorderLayout::LayoutContainer ()
{
  int cw, ch, ew, eh, nw, nh, sw, sh, ww, wh;
  cw = ch = ew = eh = nw = nh = sw = sh = ww = wh = 0;
  if (CENTER->comp) CENTER->comp->SuggestSize (cw, ch);
  if (EAST->comp) EAST->comp->SuggestSize (ew, eh);
  if (NORTH->comp) NORTH->comp->SuggestSize (nw, nh);
  if (SOUTH->comp) SOUTH->comp->SuggestSize (sw, sh);
  if (WEST->comp) WEST->comp->SuggestSize (ww, wh);

  int x = 0, y = 0;

  int parentWidth = bound.Width ();
  int parentHeight = bound.Height ();

  x += insets.xmin;
  y += insets.ymin;
  parentWidth  -= insets.xmin + insets.xmax;
  parentHeight -= insets.ymin + insets.ymax;

  DistributeSizes (ww, cw, ew, parentWidth, mHgap,
    WEST->comp != 0, CENTER->comp != 0, EAST->comp != 0);
  DistributeSizes (nh, ch, sh, parentHeight, mVgap,
    NORTH->comp != 0, CENTER->comp != 0, SOUTH->comp != 0);

  eh = wh = ch;
  nw = sw = parentWidth;

  if (CENTER->comp)
  {
    CENTER->comp->SetPos (x + ww + Hgap (WEST->comp), y + nh + Vgap (NORTH->comp));
    CENTER->comp->SetSize (cw, ch);
  }

  if (EAST->comp)
  {
    EAST->comp->SetPos (x + ww + cw + Hgap (WEST->comp) + Hgap (CENTER->comp),
      y + nh + Vgap (NORTH->comp));
    EAST->comp->SetSize (ew, eh);
  }

  if (NORTH->comp)
  {
    NORTH->comp->SetPos (x, y);
    NORTH->comp->SetSize (nw, nh);
  }

  if (SOUTH->comp)
  {
    SOUTH->comp->SetPos (x, y + nh + ch + Vgap (NORTH->comp) + Vgap (CENTER->comp));
    SOUTH->comp->SetSize (sw, sh);
  }

  if (WEST->comp)
  {
    WEST->comp->SetPos (x, y + nh + Vgap (NORTH->comp));
    WEST->comp->SetSize (ww, wh);
  }
}

csLayoutConstraint *csBorderLayout::AddLayoutComponent (csComponent* comp)
{
  csBorderConstraint *c = (csBorderConstraint*)csLayout2::AddLayoutComponent (comp);

  switch (c->mAlign)
  {
    case _CENTER : CENTER->comp = comp; break;
    case _EAST   : EAST->comp   = comp; break;
    case _NORTH  : NORTH->comp  = comp; break;
    case _SOUTH  : SOUTH->comp  = comp; break;
    case _WEST   : WEST->comp   = comp; break;
    default: break;
  }
  return c;
}

void csBorderLayout::RemoveLayoutComponent (csComponent* comp)
{
  size_t idx = vConstraints.FindKey (vConstraints.KeyCmp(comp));
  if (idx != (size_t)-1)
  {
    csBorderConstraint *c = (csBorderConstraint*)vConstraints.Get (idx);
    switch (c->mAlign)
    {
      case _CENTER : CENTER->comp = 0; break;
      case _EAST   : EAST->comp   = 0; break;
      case _NORTH  : NORTH->comp  = 0; break;
      case _SOUTH  : SOUTH->comp  = 0; break;
      case _WEST   : WEST->comp   = 0; break;
      default: break;
    }
  }
  csLayout2::RemoveLayoutComponent (comp);
}

void csBorderLayout::MaximumLayoutSize (int &w, int &h)
{
  if (parent)
    w = parent->bound.Width (), h = parent->bound.Height ();
  else
    w = h = 1 << 31;
}

float csBorderLayout::GetLayoutAlignmentX ()
{
  return 0;
}

float csBorderLayout::GetLayoutAlignmentY ()
{
  return 0;
}

/*** protected methods ***/

void csBorderLayout::DistributeSizes (int& left, int& center, int& right,
  int totalWidth, int gap, bool hasLeft, bool hasCenter, bool hasRight)
{
  if (hasLeft && hasCenter) totalWidth  -= gap;
  if (hasCenter && hasRight) totalWidth -= gap;
  if (hasRight && hasLeft && !hasCenter) totalWidth -= gap;

  if (left + center + right <= totalWidth)
    center = totalWidth - left - right;
  else
  {
    int sum = left + center + right;

    left   = (totalWidth * left)   / sum;
    center = (totalWidth * center) / sum;

    // compensate round-off errros at the expense of right element
    right  = totalWidth - left - center;
  }
}

int csBorderLayout::GetWidthSum (int left, int center, int right, int gap,
  bool hasLeft, bool hasCenter, bool hasRight)
{
  int sum = left + center + right;

  if (hasLeft && hasCenter) sum += gap;
  if (hasCenter && hasRight) sum += gap;
  if (hasRight && hasLeft && !hasCenter) sum += gap;

  return sum;
}

int csBorderLayout::Hgap (csComponent* pComp)
{
  return (pComp ? mHgap : 0);
}

int csBorderLayout::Vgap (csComponent* pComp)
{
  return (pComp ? mVgap : 0);
}
