/*
    Copyright (C) 2001 by W.C.A. Wijngaards
  
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
#include "isoengin.h"
#include "isoworld.h"
#include "isoview.h"
#include "isospr.h"
#include "isolight.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"
#include "isys/evdefs.h"
#include "isys/system.h"
#include "iutil/cfgmgr.h"
#include "csutil/util.h"

IMPLEMENT_IBASE (csIsoEngine)
  IMPLEMENTS_INTERFACE (iIsoEngine)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csIsoEngine)

EXPORT_CLASS_TABLE (iso)
  EXPORT_CLASS_DEP (csIsoEngine, "crystalspace.engine.iso",
    "Crystal Space Isometric Engine",
    "crystalspace.kernel., crystalspace.graphics3d., crystalspace.graphics2d.")
EXPORT_CLASS_TABLE_END

csIsoEngine::csIsoEngine (iBase *iParent)
{
  CONSTRUCT_IBASE (iParent);
  system = NULL;
  g2d = NULL;
  g3d = NULL;
  txtmgr = NULL;

  world = NULL;
}

csIsoEngine::~csIsoEngine ()
{
  if(g3d) g3d->DecRef();
  if(system) system->DecRef();
}

bool csIsoEngine::Initialize (iSystem* p)
{
  (system = p)->IncRef();
  g3d = QUERY_PLUGIN_ID (system, CS_FUNCID_VIDEO, iGraphics3D);
  if (!g3d) return false;
  g2d = g3d->GetDriver2D ();
  txtmgr = g3d->GetTextureManager();
  return true;
}

bool csIsoEngine::HandleEvent (iEvent& /*e*/)
{
  return false;
}

iIsoWorld* csIsoEngine::CreateWorld()
{
  return new csIsoWorld(this);
}

iIsoView* csIsoEngine::CreateView(iIsoWorld *world)
{
  return new csIsoView(this, this, world);
}

iIsoSprite* csIsoEngine::CreateSprite()
{
  return new csIsoSprite(this);
}

int csIsoEngine::GetBeginDrawFlags () const
{
  return CSDRAW_CLEARZBUFFER;
}

iIsoSprite* csIsoEngine::CreateFloorSprite(const csVector3& pos, float w, 
      float h)
{
  iIsoSprite *spr = new csIsoSprite(this);
  spr->AddVertex(csVector3(0,0,0), 0, 0);
  spr->AddVertex(csVector3(0, 0, w), 1, 0);
  spr->AddVertex(csVector3(h, 0, w), 1, 1);
  spr->AddVertex(csVector3(h, 0, 0), 0, 1);
  spr->SetPosition(pos);
  return spr;
}

iIsoSprite* csIsoEngine::CreateFrontSprite(const csVector3& pos, float w, 
      float h)
{
  iIsoSprite *spr = new csIsoSprite(this);
  float hw = w * 0.5;
  spr->AddVertex(csVector3(-hw, 0,-hw), 0, 0);
  spr->AddVertex(csVector3(-hw, h, -hw), 1, 0);
  spr->AddVertex(csVector3(+hw, h, +hw), 1, 1);
  spr->AddVertex(csVector3(+hw, 0, +hw), 0, 1);
  spr->SetPosition(pos);
  return spr;
}


iIsoLight* csIsoEngine::CreateLight()
{
  return new csIsoLight(this);
}
