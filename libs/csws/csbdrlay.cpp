/////////////////////////////////////////////////////////////////////////////
// Copyright:   (c) Aleksandras Gluchovas
// Licence:     wxWindows license
//
// Ported to Crystal Space by Norman Krämer <norman@users.sourceforge.net>
/////////////////////////////////////////////////////////////////////////////

#include "cssysdef.h"
#include "csws/csbdrlay.h"

csLayoutConstraint *csBorderConstraint::Clone ()
{
  return new csBorderConstraint (*this);
}

/***** Implementation for class BorderLayout *****/

csBorderConstraint csBorderLayout::mCENTER (csBorderLayout::_CENTER);
csBorderConstraint csBorderLayout::mEAST (csBorderLayout::_EAST);
csBorderConstraint csBorderLayout::mNORTH (csBorderLayout::_NORTH);
csBorderConstraint csBorderLayout::mSOUTH (csBorderLayout::_SOUTH);
csBorderConstraint csBorderLayout::mWEST (csBorderLayout::_WEST);

csBorderConstraint csBorderLayout::mAFTER_LAST_LINE (csBorderLayout::_AFTER_LAST_LINE);
csBorderConstraint csBorderLayout::mAFTER_LINE_ENDS (csBorderLayout::_AFTER_LINE_ENDS);
csBorderConstraint csBorderLayout::mBEFORE_FIRST_LINE (csBorderLayout::_BEFORE_FIRST_LINE);
csBorderConstraint csBorderLayout::mBEFORE_LINE_BEGINS (csBorderLayout::_BEFORE_LINE_BEGINS);

csBorderConstraint* csBorderLayout::CENTER = &csBorderLayout::mCENTER;
csBorderConstraint* csBorderLayout::EAST = &csBorderLayout::mEAST;
csBorderConstraint* csBorderLayout::NORTH = &csBorderLayout::mNORTH;
csBorderConstraint* csBorderLayout::SOUTH = &csBorderLayout::mSOUTH;
csBorderConstraint* csBorderLayout::WEST = &csBorderLayout::mWEST;

csBorderConstraint* csBorderLayout::AFTER_LAST_LINE = &csBorderLayout::mAFTER_LAST_LINE;
csBorderConstraint* csBorderLayout::AFTER_LINE_ENDS = &csBorderLayout::mAFTER_LINE_ENDS;
csBorderConstraint* csBorderLayout::BEFORE_FIRST_LINE = &csBorderLayout::mBEFORE_FIRST_LINE;
csBorderConstraint* csBorderLayout::BEFORE_LINE_BEGINS = &csBorderLayout::mBEFORE_LINE_BEGINS;

csBorderLayout::csBorderLayout (csComponent *pParent)
  : csLayout2 (pParent), c (0), mHgap (0), mVgap (0)
{
  lc = &c;
}

csBorderLayout::csBorderLayout (csComponent *pParent, int hgap, int vgap)
  : csLayout2 (pParent), c (0), mHgap (hgap), mVgap (vgap)
{
  lc = &c;
}

csBorderLayout::~csBorderLayout ()
{
}

void csBorderLayout::SuggestSize (int &w, int &h)
{
  int cw, ch, ew, eh, nw, nh, sw, sh, ww, wh;
  cw = ch = ew = eh = nw = nh = sw = sh = ww = wh = 0;
  if (mCENTER.comp) mCENTER.comp->SuggestSize (cw, ch);
  if (mEAST.comp) mEAST.comp->SuggestSize (ew, eh);
  if (mNORTH.comp) mNORTH.comp->SuggestSize (nw, nh);
  if (mSOUTH.comp) mSOUTH.comp->SuggestSize (sw, sh);
  if (mWEST.comp) mWEST.comp->SuggestSize (ww, wh);

  w = GetWidthSum (ww, cw, ew, mHgap,
    mWEST.comp != NULL, mCENTER.comp != NULL, mEAST.comp != NULL);
  //  h = GetWidthSum (nh, ch, sh, mVgap, mNORT.comp != NULL, mCENTER.comp != NULL, mEAST.comp != NULL);

  h = nh + ch + sh;
}

void csBorderLayout::LayoutContainer ()
{
  int cw, ch, ew, eh, nw, nh, sw, sh, ww, wh;
  cw = ch = ew = eh = nw = nh = sw = sh = ww = wh = 0;
  if (mCENTER.comp) mCENTER.comp->SuggestSize (cw, ch);
  if (mEAST.comp) mEAST.comp->SuggestSize (ew, eh);
  if (mNORTH.comp) mNORTH.comp->SuggestSize (nw, nh);
  if (mSOUTH.comp) mSOUTH.comp->SuggestSize (sw, sh);
  if (mWEST.comp) mWEST.comp->SuggestSize (ww, wh);

  int x = 0, y = 0;

  int parentWidth = bound.Width ();
  int parentHeight = bound.Height ();

  x += insets.xmin;
  y += insets.ymin;
  parentWidth  -= insets.xmin + insets.xmax;
  parentHeight -= insets.ymin + insets.ymax;

  DistributeSizes (ww, cw, ew, parentWidth, mHgap,
    mWEST.comp != NULL, mCENTER.comp != NULL, mEAST.comp != NULL);
  DistributeSizes (nh, ch, sh, parentHeight, mVgap,
    mNORTH.comp != NULL, mCENTER.comp != NULL, mSOUTH.comp != NULL);

  eh = wh = ch;
  nw = sw = parentWidth;

  if (mCENTER.comp)
  {
    mCENTER.comp->SetPos (x + ww + Hgap (mWEST.comp), y + nh + Vgap (mNORTH.comp));
    mCENTER.comp->SetSize (cw, ch);
  }

  if (mEAST.comp)
  {
    mEAST.comp->SetPos (x + ww + cw + Hgap (mWEST.comp) + Hgap (mCENTER.comp),
      y + nh + Vgap (mNORTH.comp));
    mEAST.comp->SetSize (ew, eh);
  }

  if (mNORTH.comp)
  {
    mNORTH.comp->SetPos (x, y);
    mNORTH.comp->SetSize (nw, nh);
  }

  if (mSOUTH.comp)
  {
    mSOUTH.comp->SetPos (x, y + nh + ch + Vgap (mNORTH.comp) + Vgap (mCENTER.comp));
    mSOUTH.comp->SetSize (sw, sh);
  }

  if (mWEST.comp)
  {
    mWEST.comp->SetPos (x, y + nh + Vgap (mNORTH.comp));
    mWEST.comp->SetSize (ww, wh);
  }
}

csLayoutConstraint *csBorderLayout::AddLayoutComponent (csComponent* comp)
{
  csBorderConstraint *c = (csBorderConstraint*)csLayout2::AddLayoutComponent (comp);

  switch (c->mAlign)
  {
    case _CENTER : mCENTER.comp = comp; break;
    case _EAST   : mEAST.comp   = comp; break;
    case _NORTH  : mNORTH.comp  = comp; break;
    case _SOUTH  : mSOUTH.comp  = comp; break;
    case _WEST   : mWEST.comp   = comp; break;
    default: break;
  }
  return c;
}

void csBorderLayout::RemoveLayoutComponent (csComponent* comp)
{
  int idx = vConstraints.FindKey (comp);
  if (idx != -1)
  {
    csBorderConstraint *c = (csBorderConstraint*)vConstraints.Get (idx);
    switch (c->mAlign)
    {
      case _CENTER : mCENTER.comp = NULL; break;
      case _EAST   : mEAST.comp   = NULL; break;
      case _NORTH  : mNORTH.comp  = NULL; break;
      case _SOUTH  : mSOUTH.comp  = NULL; break;
      case _WEST   : mWEST.comp   = NULL; break;
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
