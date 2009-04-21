/*
    Copyright (C) 2008 by Jorrit Tyberghein

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
#include "iengine/engine.h"
#include "iengine/sector.h"
#include "iengine/material.h"
#include "iengine/camera.h"
#include "ivaria/decal.h"
#include "imap/loader.h"

#include "walktest.h"
#include "decaltest.h"
#include "splitview.h"

void WalkTestDecalTester::TestDecal (WalkTest* Sys)
{
  csRef<iDecalManager> decalMgr = csLoadPluginCheck<iDecalManager> (
  	Sys->object_reg, "crystalspace.decal.manager");
  if (!decalMgr)
    return;

  iMaterialWrapper * material = 
    Sys->Engine->GetMaterialList()->FindByName("decal");
  if (!material)
  {
    csRef<iLoader> loader = csQueryRegistry<iLoader>(Sys->object_reg);
    if (!loader)
    {
      Sys->Report(CS_REPORTER_SEVERITY_NOTIFY, "Couldn't find iLoader");
      return; 
    }

    if (!loader->LoadTexture ("decal", "/lib/std/cslogo2.png"))
      Sys->Report(CS_REPORTER_SEVERITY_NOTIFY, 
                  "Couldn't load decal texture!");

    material = Sys->Engine->GetMaterialList()->FindByName("decal");
    if (!material)
    {
      Sys->Report(CS_REPORTER_SEVERITY_NOTIFY, 
                  "Error finding decal material");
      return;
    }
  }

  // create a template for our new decal
  csRef<iDecalTemplate> decalTemplate = 
      decalMgr->CreateDecalTemplate(material);
  decalTemplate->SetTimeToLive(5.0f);
  
  csVector3 start = Sys->views->GetCamera()->GetTransform().GetOrigin();

  csVector3 normal = 
    Sys->views->GetCamera()->GetTransform().This2OtherRelative(csVector3(0,0,-1));

  csVector3 end = start - normal * 10000.0f;

  csSectorHitBeamResult result = Sys->views->GetCamera()->GetSector()->HitBeamPortals(start, end);
  if (!result.mesh)
      return;
      
/*
  // figure out the starting point
  csRef<iCollideSystem> cdsys = csQueryRegistry<iCollideSystem>(Sys->object_reg);
  if (!cdsys)
  {
    Sys->Report(CS_REPORTER_SEVERITY_NOTIFY, "Couldn't find iCollideSystem");
    return;
  }


  // intersect with world to get a decal position
  csVector3 iSect;
  csIntersectingTriangle closestTri;
  iMeshWrapper * selMesh;
  if (csColliderHelper::TraceBeam(cdsys, Sys->views->GetCamera()->GetSector(), 
        start, end, true, closestTri, iSect, &selMesh) <= 0.0f)
  {
      printf("No Decal Tracebeam\n");
      return;
  }

  start = iSect;
*/

  // make the up direction of the decal the same as the camera
  csVector3 up =
    Sys->views->GetCamera()->GetTransform().This2OtherRelative(
	csVector3(0,1,0));

  // create the decal
  decalMgr->CreateDecal(decalTemplate, result.final_sector,
	  result.isect, up, normal, 1.0f, 0.5f);
}

