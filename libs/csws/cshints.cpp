/*
    Crystal Space Windowing System: floating hints class
    Copyright (C) 2000 by Andrew Zabolotny <bit@eltech.ru>

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

#include <stdlib.h>
#include "cssysdef.h"
#include "csws/cshints.h"
#include "csws/csapp.h"
#include "csws/cswspal.h"

csHint::csHint (csComponent *iParent, const char *iText, iFont *Font,
  int FontSize) : csComponent (iParent)
{
  SetPalette (CSPAL_HINT);
  SetFont (Font, FontSize);
  if (app)
    app->InsertClipChild (this);
  SetText (iText);
}

void csHint::SetText (const char *iText)
{
#if 0
  // First of all, decide our width and height
  int h, w = FontSize (iText, &fh);
  while (w > h * 10)
  {
    w >>= 1;
    h = (h + 2) << 1;
  }

  // Split the text (preferably between words) to approximatively given width
  const char *txt = iText;
  const int max_split = 20;
  int split [max_split];
  int splitcount = 0;
  while (splitcount < max_split)
  {
  }

  // Now find our position depending on parent component's position

#endif  
csComponent::SetText (iText);
SetRect (-50,-40,100,40);
}

void csHint::Draw ()
{
  Rect3D (0, 0, bound.Width (), bound.Height (),
    CSPAL_HINT_BORDER, CSPAL_HINT_BORDER);
  Box (1, 1, bound.Width () - 1, bound.Height () - 1, CSPAL_HINT_BACKGROUND);
//@@todo
  Text (2, 2, CSPAL_HINT_TEXT, -1, text);
}

bool csHint::PreHandleEvent (iEvent &Event)
{
  if (IS_MOUSE_EVENT (Event)
   || IS_JOYSTICK_EVENT (Event)
   || IS_KEYBOARD_EVENT (Event))
    Close ();
  return false;
}

//--//--//--//--//--//--//--//--//--//--//--//--//--//--//- Hint manager -//--//

csHintManager::csHintManager (csApp *iApp) : csVector (16, 16)
{
  app = iApp;
  check = false;
  timeout = CSHINT_DEFAULT_TIMEOUT;
  font = NULL;
  fontsize = 0;
}

csHintManager::~csHintManager ()
{
  DeleteAll ();
}

bool csHintManager::FreeItem (csSome Item)
{
  free (Item);
  return true;
}

int csHintManager::Compare (csSome Item1, csSome Item2, int Mode) const
{
  (void)Mode;
  HintStore *ts1 = (HintStore *)Item1;
  HintStore *ts2 = (HintStore *)Item2;
  return (ts1->comp < ts2->comp) ? -1 : (ts1->comp > ts2->comp) ? +1 : 0;
}

int csHintManager::CompareKey (csSome Item, csConstSome Key, int Mode) const
{
  (void)Mode;
  HintStore *ts = (HintStore *)Item;
  csComponent *comp = (csComponent *)Key;
  return (ts->comp < comp) ? -1 : (ts->comp > comp) ? +1 : 0;
}

void csHintManager::Add (const char *iText, csComponent *iComp)
{
  size_t sl = strlen (iText);
  HintStore *ts = (HintStore *)malloc (sizeof (HintStore) + sl);
  ts->comp = iComp;
  memcpy (ts->text, iText, sl + 1);
  InsertSorted (ts);
}

void csHintManager::Remove (csComponent *iComp)
{
  int idx = FindSortedKey (iComp);
  if (idx >= 0)
    Delete (idx);
}

void csHintManager::SetFont (iFont *iNewFont, int iSize)
{
  if (font)
    font->DecRef ();
  font = iNewFont;
  if (font)
    font->IncRef ();
  fontsize = iSize;
}

bool csHintManager::do_checkhint (csComponent *comp, void *data)
{
  csHintManager *This = (csHintManager *)data;
  return (This->FindSortedKey (comp) >= 0);
}

void csHintManager::HandleEvent (iEvent &Event)
{
  if (!timeout)
    return;

  switch (Event.Type)
  {
    case csevBroadcast:
      if ((Event.Command.Code == cscmdPreProcess)
       && (check) && (app->GetCurrentTime () - time > timeout))
      {
        check = false;
        int mx, my;
        app->GetMouse ().GetPosition (mx, my);
        // Find the children under mouse cursor
        csComponent *c = app->GetChildAt (mx, my, do_checkhint, this);
        if (c)
        {
          // Look for a hint for given component
          int idx = FindSortedKey (c);
          if (idx >= 0)
          {
            // Okay, create the floating hint object
            HintStore *ts = (HintStore *)Get (idx);
            new csHint (c, ts->text, font, fontsize);
          }
        }
      }
      break;
    case csevMouseMove:
      time = app->GetCurrentTime ();
      check = true;
      break;
  }
}
