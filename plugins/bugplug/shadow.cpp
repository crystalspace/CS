/*
    Copyright (C) 2001 by Jorrit Tyberghein

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
#include "shadow.h"
#include "igeom/polymesh.h"
#include "iengine/rview.h"
#include "iengine/camera.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iengine/sector.h"
#include "iengine/movable.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"

SCF_IMPLEMENT_IBASE (csShadow)
  SCF_IMPLEMENTS_INTERFACE (iMeshObject)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iObjectModel)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csShadow::ObjectModel)
  SCF_IMPLEMENTS_INTERFACE (iObjectModel)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csShadow::csShadow ()
{
  SCF_CONSTRUCT_IBASE (0);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiObjectModel);
  wrap = 0;
  do_bbox = true;
  do_rad = true;
  logparent = 0;
}

csShadow::~csShadow ()
{
  CS_ASSERT (wrap == 0);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiObjectModel);
  SCF_DESTRUCT_IBASE ();
}

csRenderMesh** csShadow::GetRenderMeshes (int& n, iRenderView* rview,
    iMovable*, uint32)
{
  keep_camera = rview->GetOriginalCamera ();
  n = 0;
  return 0;
}

bool csShadow::AddToEngine (iEngine* engine)
{
  if (wrap) { engine->GetMeshes ()->Remove (wrap); wrap = 0; }
  if (engine->GetSectors ()->GetCount () <= 0) return false;
  csRef<iMeshWrapper> ww (engine->CreateMeshWrapper (this, "_@Shadow@_"));
  wrap = ww;
  wrap->SetRenderPriority (engine->GetAlphaRenderPriority ());
  iMovable* movable = wrap->GetMovable ();
  int i;
  for (i = 0 ; i < engine->GetSectors ()->GetCount () ; i++)
  {
    iSector* sec = engine->GetSectors ()->Get (i);
    movable->GetSectors ()->Add (sec);
  }
  movable->UpdateMove ();
  return true;
}

void csShadow::RemoveFromEngine (iEngine* engine)
{
  if (wrap)
  {
    engine->GetMeshes ()->Remove (wrap);
    wrap = 0;
  }
}

