/////////////////////////////////////////////////////////////////////////////
// Copyright:   (c) Aleksandras Gluchovas
// Licence:     wxWindows license
//
// Ported to Crystal Space by Norman Krämer <norman@users.sourceforge.net>
/////////////////////////////////////////////////////////////////////////////

#include "cssysdef.h"
#include "csws/csboxlay.h"

/***** Implementation for class BoxLayout *****/

csBoxLayout::csBoxLayout (csComponent* pParent, int axis): csLayout (pParent), mAxis (axis)
{}

void csBoxLayout::SuggestSize (int &sugw, int& sugh)
{
  int width = 0;
  int maxHeight = 0; 
  int x, y;
  int i, cnt = vConstraints.Length ();

  for (i=0; i < cnt; i++)
  {
    x = y = 0;
    vConstraints.Get (i)->comp->SuggestSize (x, y);
    if (mAxis == X_AXIS) 
    {
      width += x;
      if (y > maxHeight) maxHeight = y;
    }
    else
    {
      width += y;
      if (x > maxHeight) maxHeight = x;
    }
  }

  if (mAxis == X_AXIS)
  {
    sugw = width     + insets.left + insets.right;
    sugh = maxHeight + insets.top  + insets.bottom;
  }
  else
  {
    sugw = maxHeight + insets.left + insets.right, 
    sugh = width     + insets.top  + insets.bottom;
  }
		      

}

void csBoxLayout::LayoutContainer ()
{
  // fragment form preferedLayoutSize()...

  int i, cnt = vConstraints.Length ();

  if (!cnt) return;

  int prefTotalWidth = 0;

  int x, y;

  for (i=0; i<cnt; i++)
  {
    x = y = 0;
    vConstraints.Get (i)->comp->SuggestSize (x, y);
    prefTotalWidth += (mAxis == X_AXIS ? x : y);
  }

  int parentWidth = bound.Width ();
  int parentHeight = bound.Height ();
  x = 0, y =0;
  x += insets.left;
  y += insets.top;
  parentWidth  -= insets.left + insets.right;
  parentHeight -= insets.top  + insets.bottom;


  int widthUsed = 0;

  for (i=0; i<cnt; i++)
  {
    int w=0, h=0;
    vConstraints.Get (i)->comp->SuggestSize (w, h);

    // squeez it if there is less space then preferred

    if (mAxis == X_AXIS)
    {
      if (parentWidth < prefTotalWidth)
      {
	w  = (int)((double)parentWidth * ((double)w / (double)prefTotalWidth));
	if (i+1 == cnt) 
	  w = parentWidth - widthUsed;
      }

      h = parentHeight;
      widthUsed += w;
    }
    else
    {
      if (parentHeight < prefTotalWidth)
      {
	h  = (int)((double)parentHeight * ((double)h / (double)prefTotalWidth));
	if (i+1 == cnt) 
	  h = parentHeight - widthUsed;
      }

      w  = parentWidth;
      widthUsed += h;
    }

    vConstraints.Get (i)->comp->SetRect (x, y, x+w, y+h);
    if (mAxis == X_AXIS) 
      x += w;
    else 
      y += h;

  }
}
