/*
    Crystal Space Windowing System: mouse support
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by Andrew Zabolotny <bit@eltech.ru>

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

#include "sysdef.h"
#include "csutil/csstrvec.h"
#include "csutil/scanstr.h"
#include "csinput/csevent.h"
#include "csengine/texture.h"
#include "csws/csmouse.h"
#include "csws/csapp.h"

#define MOUSE_CFG "sys/mouse.cfg"

csMousePointer::csMousePointer (csComponent *iParent, int ID, int x, int y,
  int w, int h, int hsx, int hsy) : csComponent (iParent)
{
  id = ID;
  tX = x; tY = y; tW = w; tH = h;
  hsX = hsx; hsY = hsy;
  Cursor = NULL;
  Under = NULL;
  state |= CSS_SELECTABLE;
}

csMousePointer::~csMousePointer ()
{
  if (Cursor)
    CHKB (delete Cursor);
  CHK (Free ());
}

void csMousePointer::Draw (int x, int y)
{
  if (Cursor)
  {
    x -= hsX; y -= hsY;
    app->pplSaveArea (Under, x, y, tW, tH);
    app->pplSprite2D (Cursor, x, y, tW, tH);
  } /* endif */
}

void csMousePointer::Undraw ()
{
  if (Under)
  {
    app->pplRestoreArea (Under, true);
    Under = NULL;
  } /* endif */
}

void csMousePointer::Free ()
{
  if (Under)
  {
    app->pplFreeArea (Under);
    Under = NULL;
  }
}

void csMousePointer::SetTexture (csTextureHandle *tex)
{
  if (Cursor)
    CHKB (delete Cursor);
  CHK (Cursor = new csSprite2D (tex, tX, tY, tW, tH));
  if (!Cursor->ok ())
  {
    CHK (delete Cursor);
    Cursor = NULL;
  } /* endif */
}

csMouse::csMouse (csComponent *iParent) : csComponent (iParent)
{
  MouseX = 0;
  MouseY = 0;
  Visible = 0;
  invisible = false;
}

csMouse::~csMouse ()
{
}

void csMouse::Move (int x, int y)
{
  MouseX = x;
  MouseY = y;
}

void csMouse::Draw ()
{
  // Draw mouse pointer
  if ((focused) && (Visible == 0) && (!invisible))
    ((csMousePointer *)focused)->Draw (MouseX, MouseY);
}

void csMouse::Undraw ()
{
  // Restore the square under the mouse
  if (focused)
    ((csMousePointer *)focused)->Undraw ();
}

void csMouse::NewPointer (char *id, char *posdef)
{
  int cID;
  ScanStr (id, "%d", &cID);
  int cX, cY, cW, cH, chX, chY;
  ScanStr (posdef, "%d,%d,%d,%d,%d,%d", &cX, &cY, &cW, &cH, &chX, &chY);
  CHK ((void)new csMousePointer (this, cID, cX, cY, cW, cH, chX, chY));
}

static bool do_set_texture (csComponent *child, void *param)
{
  ((csMousePointer *)child)->SetTexture ((csTextureHandle *)param);
  return false;
}

void csMouse::Setup ()
{
  ForEach (do_set_texture, app->GetTexture (app->mousetexturename));
}

bool csMouse::HandleEvent (csEvent &Event)
{
  switch (Event.Type)
  {
    case csevMouseMove:
    case csevMouseDoubleClick:
    case csevMouseDown:
    case csevMouseUp:
      Move (Event.Mouse.x, Event.Mouse.y);
      return true;
    case csevBroadcast:
      switch (Event.Command.Code)
      {
        case cscmdFocusChanged:
          if (Event.Command.Info == NULL)
            Hide ();
          else
            Show ();
          break;
      } /* endswitch */
      return true;
  } /* endswitch */
  return false;
}

bool csMouse::SetCursor (csMouseCursorID ID)
{
  csMousePointer *Cur = (csMousePointer *)GetChild (ID);
  if (Cur)
  {
    if (SUCCEEDED (System->piG2D->SetMouseCursor (ID,
      Cur->Cursor->GetTextureHandle ()->GetTextureHandle ())))
      invisible = true;
    else
    {
      SetFocused (Cur);
      invisible = false;
    } /* endif */
    return true;
  } else
  {
    System->piG2D->SetMouseCursor (csmcNone, NULL);
    invisible = true;
    return false;
  }
}
