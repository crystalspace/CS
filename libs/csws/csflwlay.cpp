/////////////////////////////////////////////////////////////////////////////
// Copyright:   (c) Aleksandras Gluchovas
// Licence:     wxWindows license
//
// Ported to Crystal Space by Norman Kraemer <norman@users.sourceforge.net>
/////////////////////////////////////////////////////////////////////////////

#include "cssysdef.h"
#include "csws/csflwlay.h"

/***** Implementation for class csFlowLayout *****/

csFlowLayout::csFlowLayout (csComponent *pParent)
  : csLayout (pParent), mAlign (CENTER), mHgap (5), mVgap (5)
{}

csFlowLayout::csFlowLayout (csComponent *pParent,int align)
  : csLayout (pParent), mAlign (align), mHgap (5), mVgap (5)
{}

csFlowLayout::csFlowLayout (csComponent *pParent, int align, int hgap, int vgap)
  : csLayout (pParent), mAlign (align), mHgap (hgap), mVgap (vgap)
{}

int csFlowLayout::GetAlignment ()
{
  return mAlign;
}

int csFlowLayout::GetHgap ()
{
  return mHgap;
}

int csFlowLayout::GetVgap ()
{
  return mVgap;
}

void csFlowLayout::SetAlignment (int align)
{
  mAlign = align;
}

void csFlowLayout::SetHgap (int hgap)
{
  mHgap = hgap;
}

void csFlowLayout::SetVgap (int vgap)
{
  mVgap = vgap;
}

// impl. of LayoutManager interface

void csFlowLayout::SuggestSize (int &sugw, int& sugh)
{
  if (TwoPhaseLayoutingEnabled () && GetLayoutingPhase () == csLayout::PHASE_1)
  {
    sugw = (int)mPrefDimOfPhase1.x;
    sugh = (int)mPrefDimOfPhase1.y;
    return ; // TBD:: comments...
  }

  int width = 0;
  int maxHeight = 0;

  int i;
  for (i = 0; i < vConstraints.Length (); i++)
  {
    if (i != 0 )
      width += mHgap;

    int w = 0, h = 0;
    vConstraints.Get (i)->comp->SuggestSize (w, h);
    width += w;

    if (h > maxHeight)
      maxHeight = h;
  }

  sugw = width + insets.xmin + insets.xmax;
  sugh = maxHeight + insets.ymin + insets.ymax;
}

void csFlowLayout::LayoutContainer ()
{
  int i, cnt = vConstraints.Length ();
  if (cnt == 0) return;

  int x = 0, y =0;

  int parentWidth = bound.Width (), parentHeight = bound.Height ();

  x += insets.xmin;
  y += insets.ymin;
  parentWidth  -= insets.xmin + insets.xmax;
  parentHeight -= insets.ymin + insets.ymax;

  // used later in 2nd phase of 2-phase layouing

  mPrefDimOfPhase1.Set (0,0);

  int row = 0;

  for (i = 0; i < cnt; i++)
  {
    int rowWidth = 0;
    int col = 0;
    int maxHeight = 0;
    int i1 = i;

    // estimate # of items in current row, and set their sizes

    while (i < cnt && rowWidth < parentWidth)
    {
      if (col != 0) rowWidth += mHgap;

      // FIXME:: getPreferedSize() is called twice for the
      //         compnent which wraps around the current row
      int w = 0, h = 0;
      vConstraints.Get (i)->comp->SuggestSize (w, h);
      if ( rowWidth + w <= parentWidth )
      {
        rowWidth += w;
        if (h > maxHeight) maxHeight = h;
        ++col;
        i++;
      }
      else
        break;
    }

    if (mPrefDimOfPhase1.x < rowWidth)
      mPrefDimOfPhase1.x = rowWidth;
    if (col == 0)
      break; // cannot fit component into the row

    // posion items in current row
    int pos = x; // left-alignend
    if (mAlign == CENTER)
      pos = x + (parentWidth - rowWidth) / 2;
    else
      if (mAlign == RIGHT)
	pos = x + (parentWidth - rowWidth);

    col = 0;
    if (row != 0)
    {
      y += mVgap;
      mPrefDimOfPhase1.y += mVgap;
    }

    while (i1 < i)
    {
      int w = 0, h = 0;
      vConstraints.Get (i1)->comp->SuggestSize (w, h);;
      if (col != 0) pos += mHgap;
      vConstraints.Get (i1)->comp->SetRect (pos, y + (maxHeight - h) / 2,
        pos + w, y + (maxHeight + h) / 2); // centered vertically
      pos += w; ++col;
      i1++;
    }

    if (i<cnt) i--;

    y += maxHeight; ++row;
    mPrefDimOfPhase1.y += maxHeight;
  }

  mPrefDimOfPhase1.x  += insets.xmin + insets.xmax;
  mPrefDimOfPhase1.y  += insets.ymin + insets.ymax;
}
