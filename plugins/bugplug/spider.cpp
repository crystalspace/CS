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
#include "spider.h"
#include "iengine/rview.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iengine/sector.h"
#include "iengine/movable.h"

SCF_IMPLEMENT_IBASE (csSpider)
  SCF_IMPLEMENTS_INTERFACE (iMeshObject)
SCF_IMPLEMENT_IBASE_END

csSpider::csSpider ()
{
  SCF_CONSTRUCT_IBASE (NULL);
  camera = NULL;
  wrap = NULL;
}

csSpider::~csSpider ()
{
  CS_ASSERT (wrap == NULL);
}

bool csSpider::DrawTest (iRenderView* rview, iMovable*)
{
  if (!camera)
  {
    camera = rview->GetCamera ();
    // @@@ Should Spider IncRef() camera to keep it alive?
  }
  return false;
}

bool csSpider::WeaveWeb (iEngine* engine)
{
  if (wrap) { engine->RemoveMesh (wrap); wrap = NULL; }
  if (engine->GetSectors ()->GetSectorCount () <= 0) return false;
  wrap = engine->CreateMeshObject (this, "_@Spider@_");
  iMovable* movable = wrap->GetMovable ();
  int i;
  for (i = 0 ; i < engine->GetSectors ()->GetSectorCount () ; i++)
  {
    iSector* sec = engine->GetSectors ()->GetSector (i);
    movable->AddSector (sec);
  }
  movable->UpdateMove ();
  return true;
}

void csSpider::UnweaveWeb (iEngine* engine)
{
  if (wrap)
  {
    engine->RemoveMesh (wrap);
    wrap = NULL;
  }
}

