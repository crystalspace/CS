/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein

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
#include "csengine/csspr2d.h"
#include "csengine/world.h"
#include "csengine/sector.h"

//=============================================================================

IMPLEMENT_CSOBJTYPE (csSprite2D, csObject)

csSprite2D::csSprite2D () : csObject (), position (0, 0, 0)
{
  MixMode = CS_FX_COPY;
  cstxt = NULL;
}

csSprite2D::~csSprite2D ()
{
  RemoveFromSectors ();
}

void csSprite2D::Draw (csRenderView& rview)
{
  if (!cstxt)
  {
    CsPrintf (MSG_FATAL_ERROR, "Error! Trying to draw a 2D sprite with no texture!\n");
    fatal_exit (0, false);
  }

  rview.g3d->SetZBufMode (CS_ZBUF_USE);
}

void csSprite2D::MoveToSector (csSector* s)
{
  RemoveFromSectors ();
  sectors.Push (s);
  s->sprites.Push (this);
}

void csSprite2D::RemoveFromSectors ()
{
  while (sectors.Length () > 0)
  {
    csSector* ss = (csSector*)sectors.Pop ();
    if (ss)
    {
      int idx = ss->sprites.Find (this);
      if (idx >= 0)
      {
        ss->sprites[idx] = NULL;
        ss->sprites.Delete (idx);
      }
    }
  }
}


