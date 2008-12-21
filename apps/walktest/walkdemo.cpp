/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein

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
#include "csqint.h"

#include "csgeom/math3d.h"
#include "csgeom/poly3d.h"
#include "cstool/collider.h"
#include "cstool/cspixmap.h"
#include "csutil/csstring.h"
#include "csutil/flags.h"
#include "csutil/scanstr.h"
#include "iengine/camera.h"
#include "iengine/campos.h"
#include "iengine/light.h"
#include "iengine/material.h"
#include "iengine/movable.h"
#include "iengine/sector.h"
#include "iengine/portal.h"
#include "iengine/sharevar.h"
#include "imesh/objmodel.h"
#include "imap/loader.h"
#include "imesh/particles.h"
#include "imesh/object.h"
#include "imesh/partsys.h"
#include "imesh/sprite3d.h"
#include "isndsys.h"
#include "ivaria/collider.h"
#include "ivaria/engseq.h"
#include "ivaria/reporter.h"
#include "ivaria/view.h"
#include "ivideo/graph3d.h"
#include "ivaria/decal.h"
#include "ivideo/material.h"

#include "bot.h"
#include "command.h"
#include "walktest.h"
#include "splitview.h"
#include "particles.h"

extern WalkTest* Sys;


void show_lightning ()
{
  csRef<iEngineSequenceManager> seqmgr(
  	csQueryRegistry<iEngineSequenceManager> (Sys->object_reg));
  if (seqmgr)
  {
    // This finds the light L1 (the colored light over the stairs) and
    // makes the lightning restore this color back after it runs.
    iLight *light = Sys->Engine->FindLight("l1");
    iSharedVariable *var = Sys->Engine->GetVariableList()
    	->FindByName("Lightning Restore Color");
    if (light && var)
    {
      var->SetColor (light->GetColor ());
    }
    seqmgr->RunSequenceByName ("seq_lightning", 0);
  }
  else
  {
    Sys->Report (CS_REPORTER_SEVERITY_NOTIFY,
    	             "Could not find engine sequence manager!");
  }
}

//===========================================================================

#if 0
static csPtr<iMeshWrapper> CreateMeshWrapper (const char* name)
{
  csRef<iMeshObjectType> ThingType = csLoadPluginCheck<iMeshObjectType> (
  	Sys->object_reg, "crystalspace.mesh.object.thing");
  if (!ThingType)
    return 0;

  csRef<iMeshObjectFactory> thing_fact = ThingType->NewFactory ();
  csRef<iMeshObject> mesh_obj = thing_fact->NewInstance ();

  csRef<iMeshWrapper> mesh_wrap =
  	Sys->Engine->CreateMeshWrapper (mesh_obj, name);
  return csPtr<iMeshWrapper> (mesh_wrap);
}
#endif

#if 0
static csPtr<iMeshWrapper> CreatePortalThing (const char* name, iSector* room,
    	iMaterialWrapper* tm, int& portalPoly)
{
  csRef<iMeshWrapper> thing = CreateMeshWrapper (name);
  csRef<iThingFactoryState> thing_fact_state = 
    scfQueryInterface<iThingFactoryState> (
    thing->GetMeshObject ()->GetFactory());
  thing->GetMovable ()->SetSector (room);
  float dx = 1, dy = 3, dz = 0.3f;
  float border = 0.3f; // width of border around the portal

  // bottom
  thing_fact_state->AddQuad (
    csVector3 (-dx, 0, -dz),
    csVector3 (dx, 0, -dz),
    csVector3 (dx, 0, dz),
    csVector3 (-dx, 0, dz));
  thing_fact_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST,
      csVector2 (0.25, 0.875),
      csVector2 (0.75, 0.875),
      csVector2 (0.75, 0.75));

  // top
  thing_fact_state->AddQuad (
    csVector3 (-dx, dy, dz),
    csVector3 (dx, dy, dz),
    csVector3 (dx, dy, -dz),
    csVector3 (-dx, dy, -dz));
  thing_fact_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST,
      csVector2 (0.25, 0.25),
      csVector2 (0.75, 0.25),
      csVector2 (0.75, 0.125));

  // back
  thing_fact_state->AddQuad (
    csVector3 (-dx, 0, dz),
    csVector3 (dx, 0, dz),
    csVector3 (dx, dy, dz),
    csVector3 (-dx, dy, dz));
  thing_fact_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST,
      csVector2 (0.25, 0.75),
      csVector2 (0.75, 0.75),
      csVector2 (0.75, 0.25));

  // right
  thing_fact_state->AddQuad (
    csVector3 (dx, 0, dz),
    csVector3 (dx, 0, -dz),
    csVector3 (dx, dy, -dz),
    csVector3 (dx, dy, dz));
  thing_fact_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST,
      csVector2 (0.75, 0.75),
      csVector2 (0.875, 0.75),
      csVector2 (0.875, 0.25));

  // left
  thing_fact_state->AddQuad (
    csVector3 (-dx, 0, -dz),
    csVector3 (-dx, 0, dz),
    csVector3 (-dx, dy, dz),
    csVector3 (-dx, dy, -dz));
  thing_fact_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST,
      csVector2 (0.125, 0.75),
      csVector2 (0.25, 0.75),
      csVector2 (0.25, 0.25));

  // front border
  // border top
  thing_fact_state->AddQuad (
    csVector3 (-dx+border, dy, -dz),
    csVector3 (dx-border, dy, -dz),
    csVector3 (dx-border, dy-border, -dz),
    csVector3 (-dx+border, dy-border, -dz));
  thing_fact_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST,
      csVector2 (0.125, 0.125),
      csVector2 (0.875, 0.125),
      csVector2 (0.875, 0.0));
  // border right
  thing_fact_state->AddQuad (
    csVector3 (dx-border, dy-border, -dz),
    csVector3 (dx, dy-border, -dz),
    csVector3 (dx, border, -dz),
    csVector3 (dx-border, border, -dz));
  thing_fact_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST,
      csVector2 (1.0, 0.125),
      csVector2 (0.875, 0.125),
      csVector2 (0.875, 0.875));
  // border bottom
  thing_fact_state->AddQuad (
    csVector3 (-dx+border, border, -dz),
    csVector3 (+dx-border, border, -dz),
    csVector3 (+dx-border, 0.0, -dz),
    csVector3 (-dx+border, 0.0, -dz));
  thing_fact_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST,
      csVector2 (0.125, 1.0),
      csVector2 (0.875, 1.0),
      csVector2 (0.875, 0.875));
  // border left
  thing_fact_state->AddQuad (
    csVector3 (-dx, dy-border, -dz),
    csVector3 (-dx+border, dy-border, -dz),
    csVector3 (-dx+border, border, -dz),
    csVector3 (-dx, border, -dz));
  thing_fact_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST,
      csVector2 (0.125, 0.125),
      csVector2 (0.0, 0.125),
      csVector2 (0.0, 0.875));
  // border topleft
  thing_fact_state->AddQuad (
    csVector3 (-dx, dy, -dz),
    csVector3 (-dx+border, dy, -dz),
    csVector3 (-dx+border, dy-border, -dz),
    csVector3 (-dx, dy-border, -dz));
  thing_fact_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST,
      csVector2 (0.125, 0.125),
      csVector2 (0.0, 0.125),
      csVector2 (0.0, 0.0));
  // border topright
  thing_fact_state->AddQuad (
    csVector3 (dx-border, dy, -dz),
    csVector3 (dx, dy, -dz),
    csVector3 (dx, dy-border, -dz),
    csVector3 (dx-border, dy-border, -dz));
  thing_fact_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST,
      csVector2 (1.0, 0.125),
      csVector2 (0.875, 0.125),
      csVector2 (0.875, 0.0));
  // border botright
  thing_fact_state->AddQuad (
    csVector3 (dx-border, border, -dz),
    csVector3 (dx, border, -dz),
    csVector3 (dx, 0.0, -dz),
    csVector3 (dx-border, 0.0, -dz));
  thing_fact_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST,
      csVector2 (1.0, 1.0),
      csVector2 (0.875, 1.0),
      csVector2 (0.875, 0.875));
  // border botleft
  thing_fact_state->AddQuad (
    csVector3 (-dx, border, -dz),
    csVector3 (-dx+border, border, -dz),
    csVector3 (-dx+border, 0.0, -dz),
    csVector3 (-dx, 0.0, -dz));
  thing_fact_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST,
      csVector2 (0.125, 1.0),
      csVector2 (0.0, 1.0),
      csVector2 (0.0, 0.875));

  // front - the portal
  portalPoly = thing_fact_state->AddQuad (
    csVector3 (dx-border, border, -dz),
    csVector3 (-dx+border, border, -dz),
    csVector3 (-dx+border, dy-border, -dz),
    csVector3 (dx-border, dy-border, -dz));
  thing_fact_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST,
      csVector2 (0, 0),
      csVector2 (1, 0),
      csVector2 (1, 1));

  thing_fact_state->SetPolygonMaterial (CS_POLYRANGE_ALL, tm);
  thing_fact_state->SetPolygonFlags (CS_POLYRANGE_ALL, CS_POLY_COLLDET);

  csRef<iLightingInfo> linfo (
    scfQueryInterface<iLightingInfo> (thing->GetMeshObject ()));
  linfo->InitializeDefault (true);
  room->ShineLights (thing);
  linfo->PrepareLighting ();

  return csPtr<iMeshWrapper> (thing);
}
#endif

void OpenPortal (iView* view, char* lev)
{
#if 0
  iSector* room = view->GetCamera ()->GetSector ();
  csVector3 pos = view->GetCamera ()->GetTransform ().This2Other (
  	csVector3 (0, 0, 1));
  iMaterialWrapper* tm = Sys->Engine->GetMaterialList ()->
  	FindByName ("portal");

  int portalPoly;
  csRef<iMeshWrapper> thing = CreatePortalThing ("portalTo", room, tm,
  	portalPoly);
  csRef<iThingFactoryState> thing_fact_state = 
    scfQueryInterface<iThingFactoryState> (
    thing->GetMeshObject ()->GetFactory());

  bool collectionExists = (Sys->Engine->GetCollection (lev) != 0);
  iCollection* collection = Sys->Engine->CreateCollection (lev);
  // If the collection did not already exist then we load the level in it.
  if (!collectionExists)
  {
    // @@@ No error checking!
    csString buf;
    buf.Format ("/lev/%s", lev);
    Sys->myVFS->ChDir (buf);

    if(Sys->threaded)
    {
      Sys->TLevelLoader->LoadMapFile ("world", false, collection);
    }
    else
    {
      Sys->LevelLoader->LoadMapFile ("world", false, collection);
    }
  }

  iMovable* tmov = thing->GetMovable ();
  tmov->SetPosition (pos + csVector3 (0, Sys->cfg_legs_offset, 0));
  tmov->Transform (view->GetCamera ()->GetTransform ().GetT2O ());
  tmov->UpdateMove ();

  // First make a portal to the new level.
  iCameraPosition* cp = collection->FindCameraPosition ("Start");
  const char* room_name;
  csVector3 topos;
  if (cp) { room_name = cp->GetSector (); topos = cp->GetPosition (); }
  else { room_name = "room"; topos.Set (0, 0, 0); }
  topos.y -= Sys->cfg_eye_offset;
  iSector* start_sector = collection->FindSector (room_name);
  if (start_sector)
  {
    csPoly3D poly;
    poly.AddVertex (thing_fact_state->GetPolygonVertex (portalPoly, 0));
    poly.AddVertex (thing_fact_state->GetPolygonVertex (portalPoly, 1));
    poly.AddVertex (thing_fact_state->GetPolygonVertex (portalPoly, 2));
    poly.AddVertex (thing_fact_state->GetPolygonVertex (portalPoly, 3));
    iPortal* portal;
    csRef<iMeshWrapper> portalMesh = Sys->Engine->CreatePortal (
    	"new_portal", tmov->GetSectors ()->Get (0), csVector3 (0),
	start_sector, poly.GetVertices (), (int)poly.GetVertexCount (),
	portal);
    //iPortal* portal = portalPoly->CreatePortal (start_sector);
    portal->GetFlags ().Set (CS_PORTAL_ZFILL);
    portal->GetFlags ().Set (CS_PORTAL_CLIPDEST);
    portal->SetWarp (view->GetCamera ()->GetTransform ().GetT2O (), topos, pos);

    if (!collectionExists)
    {
      // Only if the collection did not already exist do we create a portal
      // back. So even if multiple portals go to the collection we only have
      // one portal back.
      int portalPolyBack;
      csRef<iMeshWrapper> thingBack = CreatePortalThing ("portalFrom",
	  	start_sector, tm, portalPolyBack);
      csRef<iThingFactoryState> thing_fact_state = 
	scfQueryInterface<iThingFactoryState> (
	thingBack->GetMeshObject ()->GetFactory());
      iMovable* tbmov = thingBack->GetMovable ();
      tbmov->SetPosition (topos + csVector3 (0, Sys->cfg_legs_offset, -0.1f));
      tbmov->Transform (csYRotMatrix3 (PI));//view->GetCamera ()->GetW2C ());
      tbmov->UpdateMove ();
      iPortal* portalBack;
      csPoly3D polyBack;
      polyBack.AddVertex (thing_fact_state->GetPolygonVertex (portalPoly, 0));
      polyBack.AddVertex (thing_fact_state->GetPolygonVertex (portalPoly, 1));
      polyBack.AddVertex (thing_fact_state->GetPolygonVertex (portalPoly, 2));
      polyBack.AddVertex (thing_fact_state->GetPolygonVertex (portalPoly, 3));
      csRef<iMeshWrapper> portalMeshBack = Sys->Engine->CreatePortal (
    	  "new_portal_back", tbmov->GetSectors ()->Get (0), csVector3 (0),
	  tmov->GetSectors ()->Get (0), polyBack.GetVertices (),
	  (int)polyBack.GetVertexCount (),
	  portalBack);
      //iPortal* portalBack = portalPolyBack->CreatePortal (room);
      portalBack->GetFlags ().Set (CS_PORTAL_ZFILL);
      portalBack->GetFlags ().Set (CS_PORTAL_CLIPDEST);
      portalBack->SetWarp (view->GetCamera ()->GetTransform ().GetO2T (),
      	-pos, -topos);
    }
  }

  if (!collectionExists)
    Sys->InitCollDet (Sys->Engine, collection);
#endif
}

