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
#include "csutil/util.h"

csHint::csHint (csComponent *iParent, const char *iText, iFont *Font) : 
  csComponent (iParent)
{
  SetPalette (CSPAL_HINT);
  SetFont (Font);
  if (app)
    app->InsertClipChild (this);
  SetText (iText);
  oldmo = app->CaptureMouse (this);
}

csHint::~csHint ()
{
  app->CaptureMouse (oldmo);
}

void csHint::SetText (const char *iText)
{
  if (!iText)
  {
    csComponent::SetText (iText);
    SetRect (0, 0, 0, 0);
    return;
  }

  // First of all, decide our width and height
  iFont *font;
  GetFont (font);

  int fw, fh;
  font->GetDimensions (iText, fw, fh);
  while (fw > fh * 30)
  {
    fw >>= 1;
    fh = (fh + 2) << 1;
  }

  // Split the text (preferably between words) to approximatively given width
  char *textcopy = csStrNew (iText);
  char *txt = textcopy;
  char *lasttxt = strchr (txt, 0);
  const int max_splits = 20;
  char *split [max_splits];
  int splitcount = 0;
  int maxw = 0, maxlen = 0;
  while (txt < lasttxt && splitcount < max_splits)
  {
    // Skip all whitespace characters at the beginning of string
    while (strchr (" \t", *txt))
      txt++;

    int len = font->GetLength (txt, fw);
    // Now skip characters until we find a whitespace
    while (txt [len] && !strchr (" \t", txt [len]))
      len++;
    // Skip all the spaces backward
    while (len && strchr (" \t", txt [len - 1]))
      len--;
    // Allright, now split the string at this point
    txt [len] = 0;

    font->GetDimensions (txt, fw, fh);
    if (fw > maxw) maxw = fw;

    split [splitcount++] = txt;
    txt += ++len;
    maxlen += len;
  }

  txt = text = new char [maxlen];
  for (fh = 0; fh < splitcount; fh++)
  {
    if (fh) *txt++ = '\n';
    strcpy (txt, split [fh]);
    txt = strchr (txt, 0);
  }

  delete [] textcopy;

  // Now find our position depending on mouse position
  Invalidate ();

  int maxfw, maxfh;
  font->GetMaxSize (maxfw, maxfh);

  csRect r (0, 0, maxw + 6, 2 + 2 + splitcount * (maxfh + 2));
  int mx, my;
  app->GetMouse ().GetPosition (mx, my);
  parent->GlobalToLocal (mx, my);
  r.Move (mx - r.Width () / 2, my - r.Height ());

  mx = r.xmin; my = r.ymin;
  parent->LocalToGlobal (mx, my);
  int ox = mx, oy = my;
  mx += r.Width ();
  my += r.Height ();
  if (mx > app->bound.xmax) mx = app->bound.xmax;
  if (my > app->bound.ymax) my = app->bound.ymax;
  mx -= r.Width ();
  my -= r.Height ();
  if (mx < app->bound.xmin) mx = app->bound.xmin;
  if (my < app->bound.ymin) my = app->bound.ymin;
  r.Move (mx - ox, my - oy);

  SetRect (r);
}

void csHint::Draw ()
{
  Rect3D (0, 0, bound.Width (), bound.Height (),
    CSPAL_HINT_BORDER, CSPAL_HINT_BORDER);
  Box (1, 1, bound.Width () - 1, bound.Height () - 1, CSPAL_HINT_BACKGROUND);

  iFont *font;
  GetFont (font);

  int cury = 3;
  char *txt = text;
  while (*txt)
  {
    char *eol = strchr (txt, '\n');
    if (!eol) eol = strchr (txt, 0);
    char oldchr = *eol;
    *eol = 0;
    int fw, fh;
    font->GetDimensions (txt, fw, fh);
    Text ((bound.Width () - fw) / 2, cury, CSPAL_HINT_TEXT, -1, txt);
    *eol = oldchr;
    txt = eol + 1;
    cury += fh + 2;
  }
}

bool csHint::PreHandleEvent (iEvent &Event)
{
  if (CS_IS_MOUSE_EVENT (Event)
   || CS_IS_JOYSTICK_EVENT (Event)
   || CS_IS_KEYBOARD_EVENT (Event))
    Close ();
  return false;
}

//--//--//--//--//--//--//--//--//--//--//--//--//--//--//- Hint manager -//--//

csHintManager::csHintManager (csApp *iApp) : csArray<void*> (16, 16)
{
  app = iApp;
  check = false;
  timeout = CSHINT_DEFAULT_TIMEOUT;
  font = 0;
  fontsize = 0;
}

csHintManager::~csHintManager ()
{
  FreeAll ();
  if (font)
    font->DecRef ();
}

void csHintManager::FreeAll ()
{
  size_t i;
  for (i = 0 ; i < Length () ; i++)
    FreeItem (Get (i));
  DeleteAll ();
}

void csHintManager::FreeItem (void* Item)
{
  free (Item);
}

int csHintManager::Compare (void* const& Item1, void* const& Item2)
{
  HintStore *ts1 = (HintStore *)Item1;
  HintStore *ts2 = (HintStore *)Item2;
  return (ts1->comp < ts2->comp) ? -1 : (ts1->comp > ts2->comp) ? +1 : 0;
}

int csHintManager::CompareKey (void* const& Item, csComponent* const& comp)
{
  HintStore *ts = (HintStore *)Item;
  return (ts->comp < comp) ? -1 : (ts->comp > comp) ? +1 : 0;
}

void csHintManager::Add (const char *iText, csComponent *iComp)
{
  size_t sl = strlen (iText);
  HintStore *ts = (HintStore *)malloc (sizeof (HintStore) + sl);
  ts->comp = iComp;
  memcpy (ts->text, iText, sl + 1);
  InsertSorted (ts, Compare);
}

void csHintManager::Remove (csComponent *iComp)
{
  size_t idx = FindSortedKey (KeyCmp(iComp));
  if (idx != (size_t)-1)
    DeleteIndex (idx);
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
  return (This->FindSortedKey (KeyCmp(comp)) >= 0);
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
        if (c && !c->GetState (CSS_DISABLED))
        {
          // Look for a hint for given component
          size_t idx = FindSortedKey (KeyCmp(c));
          if (idx != (size_t)-1)
          {
            // Okay, create the floating hint object
            HintStore *ts = (HintStore *)Get (idx);
            (void)new csHint (c, ts->text, font);
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
