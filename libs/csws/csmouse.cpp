/*
    Crystal Space Windowing System: mouse support
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
#include "csutil/scanstr.h"
#include "csws/csmouse.h"
#include "csws/csapp.h"
#include "ivaria/reporter.h"

#define MOUSE_TEXTURE_NAME	"csws::Mouse"
#define NO_VIRTUAL_POS		-999999

csMousePointer::csMousePointer (csMouse *iParent, int ID,
  int x, int y, int w, int h, int hsx, int hsy)
{
  parent = iParent;
  id = ID;
  tX = x; tY = y; tW = w; tH = h;
  hsX = hsx; hsY = hsy;
}

void csMousePointer::Draw (int x, int y, csImageArea *&Under)
{
  if (parent->Texture)
  {
    x -= hsX; y -= hsY;
    parent->app->pplSaveArea (Under, x, y, tW, tH);
    parent->app->pplTexture (parent->Texture, x, y, tW, tH, tX, tY, tW, tH);
  } /* endif */
}

//--//--//--//--//--//--//--//--//--//--//--//--//--//-- csMouse -//--//--//--//

csMouse::csMouse (csApp *iApp)
{
  app = iApp;
  MouseX = 0;
  MouseY = 0;
  Visible = 0;
  invisible = false;
  VirtualX = NO_VIRTUAL_POS;
  AppFocused = true;
  ActiveCursor = 0;
  memset (&Under, 0, sizeof (Under));
  Texture = 0;
  LastVirtual = false;
}

csMouse::~csMouse ()
{
  int i;
  for (i = 0; i < MAX_SYNC_PAGES; i++)
    if (Under [i])
      app->pplFreeArea (Under [i]);
}

void csMouse::Move (int x, int y)
{
  MouseX = x;
  MouseY = y;
}

void csMouse::Draw (int Page)
{
  if (!ActiveCursor)
    return;

  bool virt = (VirtualX != NO_VIRTUAL_POS);
  if (LastVirtual != virt)
    SetCursor (csMouseCursorID (ActiveCursor->id));
  LastVirtual = virt;

  // Draw mouse pointer
  if (AppFocused && (Visible == 0) && (!invisible))
    if (VirtualX != NO_VIRTUAL_POS)
      ActiveCursor->Draw (VirtualX, VirtualY, Under [Page]);
    else
      ActiveCursor->Draw (MouseX, MouseY, Under [Page]);
}

void csMouse::Undraw (int Page)
{
  csImageArea *under = Under [Page];
  if (under)
  {
    app->pplRestoreArea (under, true);
    Under [Page] = 0;
  }
}

void csMouse::ClearPointers ()
{
  Pointers.DeleteAll ();
}

void csMouse::NewPointer (const char *id, const char *posdef)
{
  static char *cursor_ids [] =
  {
    "Arrow", "Lens", "Cross", "Pen", "Move",
    "SizeNWSE", "SizeNESW", "SizeNS", "SizeEW",
    "Stop", "Wait", 0
  };

  int cID;
  for (cID = 0; ; cID++)
    if (!cursor_ids [cID])
    {
      app->Printf (CS_REPORTER_SEVERITY_WARNING, "WARNING: Unkown mouse cursor id (%s)\n", id);
      return;
    }
    else if (!strcmp (id, cursor_ids [cID]))
      break;

  int cX, cY, cW, cH, chX, chY;
  csScanStr (posdef, "%d,%d,%d,%d,%d,%d", &cX, &cY, &cW, &cH, &chX, &chY);
  Pointers.Push (new csMousePointer (this, cID, cX, cY, cW, cH, chX, chY));
}

void csMouse::Setup ()
{
  Texture = app->GetTexture (MOUSE_TEXTURE_NAME);
}

bool csMouse::HandleEvent (iEvent &Event)
{
  switch (Event.Type)
  {
    case csevMouseMove:
    case csevMouseDown:
    case csevMouseUp:
    case csevMouseClick:
    case csevMouseDoubleClick:
      // If mouse moves, reset virtual mouse position
      if (Event.Type == csevMouseMove)
        VirtualX = NO_VIRTUAL_POS;
      Move (Event.Mouse.x, Event.Mouse.y);
      return true;
    case csevBroadcast:
      switch (Event.Command.Code)
      {
        case cscmdFocusChanged:
          AppFocused = Event.Command.Info ? true : false;
          break;
      } /* endswitch */
      return true;
  } /* endswitch */
  return false;
}

bool csMouse::SetCursor (csMouseCursorID ID)
{
  // Do not set hardware mouse cursor if virtual pos != mouse pos
  bool virt = (VirtualX != NO_VIRTUAL_POS) &&
    ((VirtualX != MouseX) || (VirtualY != MouseY));

  ActiveCursor = 0;
  size_t i;
  for (i = 0; i < Pointers.Length (); i++)
    if (Pointers.Get (i)->id == ID)
    {
      ActiveCursor = Pointers.Get (i);
      break;
    }

  if (ActiveCursor)
  {
    if (virt)
    {
      app->SwitchMouseCursor (csmcNone);
      invisible = false;
    }
    else if (app->SwitchMouseCursor (ID))
      invisible = true;
    else
      invisible = false;
    return true;
  }
  else
  {
    app->SwitchMouseCursor (csmcNone);
    invisible = true;
    return false;
  }
}
