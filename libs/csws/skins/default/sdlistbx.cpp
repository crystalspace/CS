/*
    Crystal Space Windowing System: Default button skin
    Copyright (C) 1998,1999 by Andrew Zabolotny <bit@eltech.ru>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"
#include "csws/cslistbx.h"
#include "csws/sdefault.h"

/**
 *    The following defines are duplicated from cslistbx.cpp  
 *  Any changes should be made in BOTH places.
 *
 */  

// Amount of space at left and at right of each listbox item
#define LISTBOXITEM_XSPACE              2
// Amount of space at top and at bottom of each listbox item
#define LISTBOXITEM_YSPACE              2

// Mouse scroll time interval in milliseconds
#define MOUSE_SCROLL_INTERVAL           100

// Horizontal large scrolling step
#define LISTBOX_HORIZONTAL_PAGESTEP     8

void csDefaultListBoxItemSkin::Draw (csComponent &This) {
#define This ((csListBoxItem &)This)

 bool enabled = !This.parent->GetState (CSS_DISABLED);
  bool selected = enabled && This.GetState (CSS_LISTBOXITEM_SELECTED);
  if (selected)
  {
    if (This.parent->GetState (CSS_FOCUSED) && enabled)
      This.Clear (CSPAL_LISTBOXITEM_SELECTION);
    else
      This.Rect3D (0, 0, This.bound.Width (), This.bound.Height (),
        CSPAL_LISTBOXITEM_SELECTION, CSPAL_LISTBOXITEM_SELECTION);
  }

  int color;
  if (This.GetState (CSS_SELECTABLE) && enabled)
  {
    if (This.GetItemStyle() == cslisNormal)
      if (selected && This.parent->GetState (CSS_FOCUSED))
        color = CSPAL_LISTBOXITEM_SNTEXT;
      else
        color = CSPAL_LISTBOXITEM_UNTEXT;
    else
      if (selected && This.parent->GetState (CSS_FOCUSED))
        color = CSPAL_LISTBOXITEM_SETEXT;
      else
        color = CSPAL_LISTBOXITEM_UETEXT;
  }
  else
    color = CSPAL_LISTBOXITEM_DTEXT;

  int x = LISTBOXITEM_XSPACE - This.GetDeltaX() + This.GetHOffset();
  csPixmap *ItemBitmap;
  
  if ((ItemBitmap=This.GetItemBitmap()))
  {
    This.Pixmap (ItemBitmap, x, (This.bound.Height () - ItemBitmap->Height ()) / 2);
    x += ItemBitmap->Width () + LISTBOXITEM_XSPACE;
  } /* endif */
  
  char *text;
  if ((text=This.GetText()))
  {
    int fh;
    This.GetTextSize (text, &fh);
    This.Text (x, (This.bound.Height () - fh + 1) / 2, color, -1, text);
  } /* endif */
  
 #undef This
}

void csDefaultListBoxSkin::Draw (csComponent &This)
{
#define This ((csListBox &)This)
  
 int BorderWidth, BorderHeight;
  
 This.GetBorderSize(&BorderWidth, &BorderHeight); 

 if (This.GetPlaceItemsFlag())
    This.PlaceItems ();

  switch (This.GetFrameStyle())
  {
    case cslfsNone:
      break;
    case cslfsThinRect:
      This.Rect3D (0, 0, This.bound.Width (), This.bound.Height (),
          CSPAL_LISTBOX_LIGHT3D, CSPAL_LISTBOX_DARK3D);
      This.Rect3D (1, 1, This.bound.Width () - 1, This.bound.Height () - 1,
          CSPAL_LISTBOX_DARK3D, CSPAL_LISTBOX_LIGHT3D);
      break;
    case cslfsThickRect:
      This.Rect3D (0, 0, This.bound.Width (), This.bound.Height (),
          CSPAL_LISTBOX_LIGHT3D, CSPAL_LISTBOX_DARK3D);
      This.Rect3D (1, 1, This.bound.Width () - 1, This.bound.Height () - 1,
          CSPAL_LISTBOX_2LIGHT3D, CSPAL_LISTBOX_2DARK3D);
      break;
  } /* endswitch */
  This.Box (BorderWidth, BorderHeight, This.bound.Width () - BorderWidth,
    This.bound.Height () - BorderHeight, This.GetFrameStyle() == cslfsThickRect ?
    CSPAL_LISTBOX_BACKGROUND2 : CSPAL_LISTBOX_BACKGROUND);

  #undef This
}