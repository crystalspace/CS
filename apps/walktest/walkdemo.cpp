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
#include "walktest.h"
#include "bot.h"
#include "infmaze.h"
#include "command.h"
#include "ivaria/view.h"
#include "ivaria/engseq.h"
#include "iengine/dynlight.h"
#include "iengine/light.h"
#include "iengine/campos.h"
#include "iengine/region.h"
#include "iengine/material.h"
#include "imesh/thing/polygon.h"
#include "imesh/thing/thing.h"
#include "csutil/scanstr.h"
#include "csutil/dataobj.h"
#include "csgeom/math3d.h"
#include "igeom/polymesh.h"
#include "cstool/collider.h"
#include "cstool/cspixmap.h"
#include "qint.h"
#include "isound/handle.h"
#include "isound/source.h"
#include "isound/listener.h"
#include "isound/renderer.h"
#include "isound/wrapper.h"
#include "ivideo/graph3d.h"
#include "ivaria/collider.h"
#include "ivaria/reporter.h"
#include "imesh/lighting.h"
#include "imesh/partsys.h"
#include "imesh/fountain.h"
#include "imesh/explode.h"
#include "imesh/fire.h"
#include "imesh/snow.h"
#include "imesh/rain.h"
#include "imesh/spiral.h"
#include "imesh/sprite3d.h"
#include "imesh/skeleton.h"
#include "imap/parser.h"

#include "csengine/light.h"

extern WalkTest* Sys;


void RandomColor (float& r, float& g, float& b)
{
  float sig = (float)(900+(rand () % 100))/1000.;
  float sm1= (float)(rand () % 1000)/1000.;
  float sm2 = (float)(rand () % 1000)/1000.;
  switch ((rand ()>>3) % 3)
  {
    case 0: r = sig; g = sm1; b = sm2; break;
    case 1: r = sm1; g = sig; b = sm2; break;
    case 2: r = sm1; g = sm2; b = sig; break;
  }
}

extern iMeshWrapper* add_meshobj (const char* tname, char* sname, iSector* where,
	csVector3 const& pos, float size);
extern void move_mesh (iMeshWrapper* sprite, iSector* where,
	csVector3 const& pos);

//===========================================================================
// Demo particle system (rain).
//===========================================================================
void add_particles_rain (iSector* sector, char* matname, int num, float speed,
	bool do_camera)
{
  // First check if the material exists.
  iMaterialWrapper* mat = Sys->view->GetEngine ()->GetMaterialList ()->
  	FindByName (matname);
  if (!mat)
  {
    Sys->Report (CS_REPORTER_SEVERITY_NOTIFY, "Can't find material '%s' in memory!", matname);
    return;
  }

  csBox3 bbox;
  if (do_camera)
    bbox.Set (-5, -5, -5, 5, 5, 5);
  else
    sector->CalculateSectorBBox (bbox, true);

  csRef<iMeshFactoryWrapper> mfw (Sys->view->GetEngine ()->
    CreateMeshFactory ("crystalspace.mesh.object.rain", "rain"));
  if (!mfw) return;

  csRef<iMeshWrapper> exp (
  	Sys->view->GetEngine ()->CreateMeshWrapper (mfw, "custom rain", sector,
					  csVector3 (0, 0, 0)));
  if (do_camera)
  {
    iEngine* e = Sys->view->GetEngine ();
    int c = e->GetRenderPriorityCount ()-1;
    // Create a new camera render priority if the last one isn't
    // already in camera mode.
    if (!e->GetRenderPriorityCamera (c))
    {
      c++;
      e->RegisterRenderPriority ("rain", c, CS_RENDPRI_NONE, true);
    }
    exp->SetRenderPriority (c);
    exp->GetFlags ().Set (CS_ENTITY_CAMERA);
  }
  exp->SetZBufMode(CS_ZBUF_TEST);

  csRef<iRainState> rainstate (
  	SCF_QUERY_INTERFACE (exp->GetMeshObject (), iRainState));
  rainstate->SetMaterialWrapper (mat);
  rainstate->SetMixMode (CS_FX_ADD);
  rainstate->SetColor (csColor (.25,.25,.25));
  rainstate->SetParticleCount (num);
  rainstate->SetDropSize (0.3f/50.0f, 0.3f);
  rainstate->SetLighting (false);
  rainstate->SetBox (bbox.Min (), bbox.Max ());
  rainstate->SetFallSpeed (csVector3 (0, -speed, 0));
}

//===========================================================================
// Demo particle system (snow).
//===========================================================================
void add_particles_snow (iSector* sector, char* matname, int num, float speed)
{
  // First check if the material exists.
  iMaterialWrapper* mat = Sys->view->GetEngine ()->GetMaterialList ()->
  	FindByName (matname);
  if (!mat)
  {
    Sys->Report (CS_REPORTER_SEVERITY_NOTIFY, "Can't find material '%s' in memory!", matname);
    return;
  }

  csBox3 bbox;
  sector->CalculateSectorBBox (bbox, true);

  csRef<iMeshFactoryWrapper> mfw (Sys->view->GetEngine ()->
    CreateMeshFactory ("crystalspace.mesh.object.snow", "snow"));
  if (!mfw) return;

  csRef<iMeshWrapper> exp (
  	Sys->view->GetEngine ()->CreateMeshWrapper (mfw, "custom snow", sector,
	csVector3 (0, 0, 0)));

  exp->SetZBufMode(CS_ZBUF_TEST);

  csRef<iParticleState> partstate (
  	SCF_QUERY_INTERFACE (exp->GetMeshObject (), iParticleState));
  partstate->SetMaterialWrapper (mat);
  partstate->SetMixMode (CS_FX_ADD);
  partstate->SetColor (csColor (.25,.25,.25));

  csRef<iSnowState> snowstate (
  	SCF_QUERY_INTERFACE (exp->GetMeshObject (), iSnowState));
  snowstate->SetParticleCount (num);
  snowstate->SetDropSize (0.07f, 0.07f);
  snowstate->SetLighting (false);
  snowstate->SetBox (bbox.Min (), bbox.Max ());
  snowstate->SetFallSpeed (csVector3 (0, -speed, 0));
  snowstate->SetSwirl (0.2f);
}

//===========================================================================
// Demo particle system (fire).
//===========================================================================
void add_particles_fire (iSector* sector, char* matname, int num,
	const csVector3& origin)
{
  // First check if the material exists.
  iMaterialWrapper* mat = Sys->view->GetEngine ()->GetMaterialList ()->
  	FindByName (matname);
  if (!mat)
  {
    Sys->Report (CS_REPORTER_SEVERITY_NOTIFY, "Can't find material '%s' in memory!", matname);
    return;
  }

  csRef<iMeshFactoryWrapper> mfw (Sys->view->GetEngine ()->
    CreateMeshFactory ("crystalspace.mesh.object.fire", "fire"));
  if (!mfw) return;

  csRef<iMeshWrapper> exp (
  	Sys->view->GetEngine ()->CreateMeshWrapper (mfw, "custom fire", sector,
	origin));

  exp->SetZBufMode(CS_ZBUF_TEST);

  csRef<iParticleState> partstate (
  	SCF_QUERY_INTERFACE (exp->GetMeshObject (), iParticleState));
  partstate->SetMaterialWrapper (mat);
  partstate->SetMixMode (CS_FX_ADD);

  csRef<iFireState> firestate (
  	SCF_QUERY_INTERFACE (exp->GetMeshObject (), iFireState));
  firestate->SetParticleCount (num);
  //firestate->SetDropSize (.02, .04);
  firestate->SetDropSize (0.04f, 0.08f);
  firestate->SetLighting (false);
  firestate->SetOrigin (csBox3(origin-csVector3(0.2f, 0, 0.2f),
    origin + csVector3(0.2f, 0.2f)));
  firestate->SetDirection (csVector3 (0, 1.0f, 0));
  firestate->SetSwirl (1.6f);
  firestate->SetColorScale (0.2f);
}

//===========================================================================
// Demo particle system (fountain).
//===========================================================================
void add_particles_fountain (iSector* sector, char* matname, int num,
	const csVector3& origin)
{
  // First check if the material exists.
  iMaterialWrapper* mat = Sys->view->GetEngine ()->GetMaterialList ()->
  	FindByName (matname);
  if (!mat)
  {
    Sys->Report (CS_REPORTER_SEVERITY_NOTIFY, "Can't find material '%s' in memory!", matname);
    return;
  }

  csRef<iMeshFactoryWrapper> mfw (Sys->view->GetEngine ()->
    CreateMeshFactory ("crystalspace.mesh.object.fountain", "fountain"));
  if (!mfw) return;

  csRef<iMeshWrapper> exp (
  	Sys->view->GetEngine ()->CreateMeshWrapper (mfw, "custom fountain",
	sector, origin));
  exp->SetZBufMode(CS_ZBUF_TEST);

  csRef<iParticleState> partstate (
  	SCF_QUERY_INTERFACE (exp->GetMeshObject (), iParticleState));
  partstate->SetMaterialWrapper (mat);
  partstate->SetMixMode (CS_FX_ADD);
  partstate->SetColor (csColor (0.25f, 0.35f, 0.55f));
  partstate->SetChangeRotation (7.5f);

  csRef<iFountainState> fountstate (
  	SCF_QUERY_INTERFACE (exp->GetMeshObject (), iFountainState));
  fountstate->SetParticleCount (num);
  fountstate->SetDropSize (0.1f, 0.1f);
  fountstate->SetOrigin (csVector3 (0, 0, 0));
  fountstate->SetAcceleration (csVector3 (0, -1.0f, 0));
  fountstate->SetFallTime (5.0f);
  fountstate->SetSpeed (3.0f);
  fountstate->SetElevation (3.1415926f/2.0f);
  fountstate->SetAzimuth (0);
  fountstate->SetOpening (0.2f);
}

//===========================================================================
// Demo particle system (explosion).
//===========================================================================
void add_particles_explosion (iSector* sector, const csVector3& center, char* matname)
{
  // First check if the material exists.
  iMaterialWrapper* mat = Sys->view->GetEngine ()->GetMaterialList ()->
  	FindByName (matname);
  if (!mat)
  {
    Sys->Report (CS_REPORTER_SEVERITY_NOTIFY, "Can't find material '%s' in memory!", matname);
    return;
  }

  csRef<iMeshFactoryWrapper> mfw (Sys->view->GetEngine ()->
    CreateMeshFactory ("crystalspace.mesh.object.explosion", "explosion"));
  if (!mfw) return;

  csRef<iMeshWrapper> exp (
  	Sys->view->GetEngine ()->CreateMeshWrapper (mfw, "custom explosion",
	sector, center));

  exp->SetZBufMode(CS_ZBUF_TEST);
  exp->SetRenderPriority(4); // @@@ alpha, most of the cases. should be queried from engine

  csRef<iParticleState> partstate (
  	SCF_QUERY_INTERFACE (exp->GetMeshObject (), iParticleState));
  partstate->SetMaterialWrapper (mat);
  partstate->SetMixMode (CS_FX_SETALPHA (0.50));
  partstate->SetColor (csColor (1, 1, 0));
  partstate->SetChangeRotation (5.0);
  partstate->SetChangeSize (1.25);
  partstate->SetSelfDestruct (3000);

  csRef<iExplosionState> expstate (
  	SCF_QUERY_INTERFACE (exp->GetMeshObject (), iExplosionState));
  expstate->SetParticleCount (100);
  expstate->SetCenter (csVector3 (0, 0, 0));
  expstate->SetPush (csVector3 (0, 0, 0));
  expstate->SetNrSides (6);
  expstate->SetPartRadius (0.15f);
  expstate->SetLighting (true);
  expstate->SetSpreadPos (0.6f);
  expstate->SetSpreadSpeed (2.0f);
  expstate->SetSpreadAcceleration (2.0f);
  expstate->SetFadeSprites (500);

  exp->PlaceMesh ();
}

//===========================================================================
// Demo particle system (spiral).
//===========================================================================
void add_particles_spiral (iSector* sector, const csVector3& bottom, char* matname)
{
  // First check if the material exists.
  iMaterialWrapper* mat = Sys->view->GetEngine ()->GetMaterialList ()->
  	FindByName (matname);
  if (!mat)
  {
    Sys->Report (CS_REPORTER_SEVERITY_NOTIFY, "Can't find material '%s' in memory!", matname);
    return;
  }

  csRef<iMeshFactoryWrapper> mfw (Sys->view->GetEngine ()->
    CreateMeshFactory ("crystalspace.mesh.object.spiral", "spiral"));
  if (!mfw) return;

  csRef<iMeshWrapper> exp (
  	Sys->view->GetEngine ()->CreateMeshWrapper (mfw, "custom spiral",
	sector, bottom));

  exp->SetZBufMode(CS_ZBUF_TEST);

  csRef<iParticleState> partstate (
  	SCF_QUERY_INTERFACE (exp->GetMeshObject (), iParticleState));
  partstate->SetMaterialWrapper (mat);
  partstate->SetMixMode (CS_FX_SETALPHA (0.50));
  partstate->SetColor (csColor (1, 1, 0));
  partstate->SetChangeColor (csColor(+0.01f, 0.0f, -0.012f));

  csRef<iSpiralState> spirstate (
  	SCF_QUERY_INTERFACE (exp->GetMeshObject (), iSpiralState));
  spirstate->SetParticleCount (500);
  spirstate->SetSource (csVector3 (0, 0, 0));
}

//===========================================================================
// Everything for skeletal tree demo.
//===========================================================================

// Recursive function to add limbs to a skeletal tree. This also builds
// the sprite template.
void add_tree_limbs (iSprite3DFactoryState* state, iSpriteFrame* frame,
	iSkeletonLimb* parent, int& vertex_idx,
	int prev_par_idx, int maxdepth, int width, int recursion)
{
  int par_vertex_idx = vertex_idx;
  parent->AddVertex (vertex_idx++);
  parent->AddVertex (vertex_idx++);
  parent->AddVertex (vertex_idx++);
  parent->AddVertex (vertex_idx++);
  parent->AddVertex (vertex_idx++);
  parent->AddVertex (vertex_idx++);

  state->AddVertices (6);

  int anm_idx = frame->GetAnmIndex ();
  int tex_idx = frame->GetTexIndex ();

  state->SetVertex (anm_idx, par_vertex_idx+0, csVector3(-0.05f, 0, -0.05f));
  state->SetVertex (anm_idx, par_vertex_idx+1, csVector3(0.05f, 0, -0.05f));
  state->SetVertex (anm_idx, par_vertex_idx+2, csVector3(0, 0, 0.05f));
  state->SetVertex (anm_idx, par_vertex_idx+3, csVector3(-0.05f, 0.45f, -0.05f));
  state->SetVertex (anm_idx, par_vertex_idx+4, csVector3(0.05f, 0.45f, -0.05f));
  state->SetVertex (anm_idx, par_vertex_idx+5, csVector3(0, 0.45f, 0.05f));

  state->SetTexel (tex_idx, par_vertex_idx+0, csVector2(0, 0));
  state->SetTexel (tex_idx, par_vertex_idx+1, csVector2(0.99f, 0));
  state->SetTexel (tex_idx, par_vertex_idx+2, csVector2(0, 0.99f));
  state->SetTexel (tex_idx, par_vertex_idx+3, csVector2(0.99f, 0.99f));
  state->SetTexel (tex_idx, par_vertex_idx+4, csVector2(0.5f, 0.5f));
  state->SetTexel (tex_idx, par_vertex_idx+5, csVector2(0.5f, 0));

  if (recursion > 0)
  {
    // Create connection triangles with previous set
    state->AddTriangle (prev_par_idx+3, prev_par_idx+5, par_vertex_idx+0);
    state->AddTriangle (prev_par_idx+5, par_vertex_idx+2, par_vertex_idx+0);
    state->AddTriangle (prev_par_idx+4, par_vertex_idx+1, par_vertex_idx+2);
    state->AddTriangle (prev_par_idx+5, prev_par_idx+4, par_vertex_idx+2);
    state->AddTriangle (prev_par_idx+4, par_vertex_idx+0, par_vertex_idx+1);
    state->AddTriangle (prev_par_idx+4, prev_par_idx+3, par_vertex_idx+0);
  }
  // Create base triangles
  state->AddTriangle (par_vertex_idx+0, par_vertex_idx+5, par_vertex_idx+3);
  state->AddTriangle (par_vertex_idx+0, par_vertex_idx+2, par_vertex_idx+5);
  state->AddTriangle (par_vertex_idx+2, par_vertex_idx+4, par_vertex_idx+5);
  state->AddTriangle (par_vertex_idx+2, par_vertex_idx+1, par_vertex_idx+4);
  state->AddTriangle (par_vertex_idx+1, par_vertex_idx+3, par_vertex_idx+4);
  state->AddTriangle (par_vertex_idx+1, par_vertex_idx+0, par_vertex_idx+3);

  if (recursion >= maxdepth) return;
  iSkeletonConnection* con;
  int i;
  int rwidth;
  if (width < 0)
    rwidth = 1 + ((rand () >> 3) % (-width));
  else rwidth = width;

  for (i = 0 ; i < rwidth ; i++)
  {
    con = parent->CreateConnection ();
    csMatrix3 tr = csYRotMatrix3 (0) * csZRotMatrix3(0.15f) *
                                                 csXRotMatrix3(0.15f);
    csTransform trans (tr, -tr.GetInverse () * csVector3 (0, 0.5f, 0));
    con->SetTransformation (trans);
    csRef<iSkeletonLimb> ilimb (SCF_QUERY_INTERFACE (con, iSkeletonLimb));
    add_tree_limbs (state, frame, ilimb,
    	vertex_idx, par_vertex_idx, maxdepth, width, recursion+1);
  }
}

// Create a skeletal tree.
iSkeleton* create_skeltree (iSprite3DFactoryState* state, iSpriteFrame* frame,
	int& vertex_idx, int maxdepth, int width)
{
  state->EnableSkeletalAnimation ();
  iSkeleton* skel = state->GetSkeleton ();
  csRef<iSkeletonLimb> ilimb (SCF_QUERY_INTERFACE (skel, iSkeletonLimb));
  add_tree_limbs (state, frame, ilimb,
  	vertex_idx, 0, maxdepth, width, 0);
  return skel;
}

// Object added to every skeletal tree node to keep the animation
// information.
class TreeSkelSpriteInfo
{
public:
  float z_angle_base;
  float z_angle;
  float x_angle_base;
  float x_angle;
  float y_angle;
  float dx;
  float dz;
  float dy;
};

// Animate a skeleton.
void animate_skeleton_tree (iSkeletonLimbState* limb)
{
  iSkeletonLimbState* child = limb->GetChildren ();
  while (child)
  {
    TreeSkelSpriteInfo* o = (TreeSkelSpriteInfo*)child->GetUserData ();
    if (!o)
    {
      o = new TreeSkelSpriteInfo ();
      if ((rand () >> 3) & 0x1)
      {
        o->x_angle_base = (((float)((rand () >> 3)&0xff)) / 255.) * .4 -.2;
        o->x_angle = 0;
        o->dx = (rand () & 0x4) ? .005 : -.005;
        o->z_angle_base = (((float)((rand () >> 3)&0xff)) / 255.) * 1.2 -.6;
        o->z_angle = 0;
        o->dz = (rand () & 0x4) ? .02 : -.02;
      }
      else
      {
        o->z_angle_base = (((float)((rand () >> 3)&0xff)) / 255.) * .4 -.2;
        o->z_angle = 0;
        o->dz = (rand () & 0x4) ? .005 : -.005;
        o->x_angle_base = (((float)((rand () >> 3)&0xff)) / 255.) * 1.2 -.6;
        o->x_angle = 0;
        o->dx = (rand () & 0x4) ? .02 : -.02;
      }
      o->y_angle = 0;
      o->dy = (rand () & 0x4) ? .04 : -.04;
      child->SetUserData (o);
    }
    o->x_angle += o->dx;
    if (o->x_angle > .1 || o->x_angle < -.1) o->dx = -o->dx;
    o->z_angle += o->dz;
    if (o->z_angle > .1 || o->z_angle < -.1) o->dz = -o->dz;
    o->y_angle += o->dy;
    if (o->y_angle > .3 || o->y_angle < -.3) o->dy = -o->dy;

    // @@@ Don't use the code below in a real-time environment.
    // This is only demo code and HIGHLY inefficient.
    csMatrix3 tr = csYRotMatrix3 (o->y_angle) *
    	csZRotMatrix3 (o->z_angle + o->z_angle_base) *
	csXRotMatrix3 (o->x_angle + o->x_angle_base);
    csTransform trans (tr, -tr.GetInverse () * csVector3 (0, .5, 0));
    csRef<iSkeletonConnectionState> con (
    	SCF_QUERY_INTERFACE (child, iSkeletonConnectionState));
    con->SetTransformation (trans);
    animate_skeleton_tree (child);
    child = child->GetNextSibling ();
  }
}

struct AnimSkelTree : public iMeshDrawCallback
{
  SCF_DECLARE_IBASE;
  AnimSkelTree ();
  virtual bool BeforeDrawing (iMeshWrapper* spr, iRenderView* rview);
};


SCF_IMPLEMENT_IBASE (AnimSkelTree)
  SCF_IMPLEMENTS_INTERFACE (iMeshDrawCallback)
SCF_IMPLEMENT_IBASE_END

AnimSkelTree::AnimSkelTree ()
{
  SCF_CONSTRUCT_IBASE (0);
}

bool AnimSkelTree::BeforeDrawing (iMeshWrapper* spr, iRenderView* /*rview*/)
{
  iMeshObject* obj = spr->GetMeshObject ();
  csRef<iSprite3DState> state (SCF_QUERY_INTERFACE (obj, iSprite3DState));
  iSkeletonState* sk_state = state->GetSkeletonState ();
  csRef<iSkeletonLimbState> limb (
  	SCF_QUERY_INTERFACE (sk_state, iSkeletonLimbState));
  animate_skeleton_tree (limb);
  return true;
}

// Add a skeletal tree sprite. If needed it will also create
// the template for this.
void add_skeleton_tree (iSector* where, csVector3 const& pos, int depth,
	int width)
{
  char skelname[50];
  sprintf (skelname, "__skeltree__%d,%d\n", depth, width);
  csRef<iMeshFactoryWrapper> tmpl (Sys->Engine->GetMeshFactories ()
  	->FindByName (skelname));
  if (!tmpl)
  {
    tmpl = Sys->Engine->CreateMeshFactory (
    	"crystalspace.mesh.object.sprite.3d", skelname);
    if (tmpl == 0)
    {
      Sys->Report (CS_REPORTER_SEVERITY_WARNING,
      	"Could not load the sprite 3d plugin!");
      return;
    }
    iMeshObjectFactory* fact = tmpl->GetMeshObjectFactory ();
    csRef<iSprite3DFactoryState> state (SCF_QUERY_INTERFACE (fact,
    	iSprite3DFactoryState));
    state->SetMaterialWrapper (Sys->Engine->GetMaterialList ()->
			       FindByName ("white"));
    int vertex_idx = 0;
    iSpriteFrame* fr = state->AddFrame ();
    fr->SetName ("f");
    iSpriteAction* act = state->AddAction ();
    act->SetName ("a");
    act->AddFrame (fr, 100,0);
    create_skeltree (state, fr, vertex_idx, depth, width);

    iMeshWrapper* spr = add_meshobj (skelname, "__skeltree__",
	     where,
	     pos-csVector3 (0, Sys->cfg_body_height, 0), 1);
    AnimSkelTree* cb = new AnimSkelTree ();
    spr->SetDrawCallback (cb);
    cb->DecRef ();
  }
}

//===========================================================================
// Everything for skeletal ghost demo.
//===========================================================================

// Object added to every skeletal tree node to keep the animation
// information.
class GhostSkelSpriteInfo
{
public:
  float z_angle_base;
  float z_angle;
  float x_angle_base;
  float x_angle;
  float y_angle;
  float dx;
  float dz;
  float dy;
};

// Object added to the ghost sprite itself to hold some information
// about movement.
SCF_VERSION (GhostSpriteInfo, 0, 0, 1);
class GhostSpriteInfo : public csObject
{
public:
  float dir;

  SCF_DECLARE_IBASE_EXT (csObject);
};

SCF_IMPLEMENT_IBASE_EXT (GhostSpriteInfo)
  SCF_IMPLEMENTS_INTERFACE (GhostSpriteInfo)
SCF_IMPLEMENT_IBASE_EXT_END

// Recursive function to add limbs to a skeletal ghost. This also builds
// the sprite template.
void add_ghost_limbs (iSprite3DFactoryState* state, iSpriteFrame* frame,
	iSkeletonLimb* parent, int& vertex_idx,
	int prev_par_idx, int maxdepth, int width, int recursion, float dim)
{
  int par_vertex_idx = vertex_idx;
  parent->AddVertex (vertex_idx++);
  parent->AddVertex (vertex_idx++);
  parent->AddVertex (vertex_idx++);
  parent->AddVertex (vertex_idx++);
  parent->AddVertex (vertex_idx++);
  parent->AddVertex (vertex_idx++);

  state->AddVertices (6);

  int anm_idx = frame->GetAnmIndex ();
  int tex_idx = frame->GetTexIndex ();

  state->SetVertex (anm_idx, par_vertex_idx+0, csVector3(-dim, 0, -dim));
  state->SetVertex (anm_idx, par_vertex_idx+1, csVector3(dim, 0, -dim));
  state->SetVertex (anm_idx, par_vertex_idx+2, csVector3(0, 0, dim));
  state->SetVertex (anm_idx, par_vertex_idx+3, csVector3(-dim, 0.45f, -dim));
  state->SetVertex (anm_idx, par_vertex_idx+4, csVector3(dim, 0.45f, -dim));
  state->SetVertex (anm_idx, par_vertex_idx+5, csVector3(0, 0.45f, dim));

  state->SetTexel (tex_idx, par_vertex_idx+0, csVector2(0, 0));
  state->SetTexel (tex_idx, par_vertex_idx+1, csVector2(0.99f, 0));
  state->SetTexel (tex_idx, par_vertex_idx+2, csVector2(0, 0.99f));
  state->SetTexel (tex_idx, par_vertex_idx+3, csVector2(0.99f, 0.99f));
  state->SetTexel (tex_idx, par_vertex_idx+4, csVector2(0.5f, 0.5f));
  state->SetTexel (tex_idx, par_vertex_idx+5, csVector2(0.5f, 0));

  if (recursion > 0)
  {
    // Create connection triangles with previous set
    state->AddTriangle (prev_par_idx+3, prev_par_idx+5, par_vertex_idx+0);
    state->AddTriangle (prev_par_idx+5, par_vertex_idx+2, par_vertex_idx+0);
    state->AddTriangle (prev_par_idx+4, par_vertex_idx+1, par_vertex_idx+2);
    state->AddTriangle (prev_par_idx+5, prev_par_idx+4, par_vertex_idx+2);
    state->AddTriangle (prev_par_idx+4, par_vertex_idx+0, par_vertex_idx+1);
    state->AddTriangle (prev_par_idx+4, prev_par_idx+3, par_vertex_idx+0);
  }
  // Create base triangles
  state->AddTriangle (par_vertex_idx+0, par_vertex_idx+5, par_vertex_idx+3);
  state->AddTriangle (par_vertex_idx+0, par_vertex_idx+2, par_vertex_idx+5);
  state->AddTriangle (par_vertex_idx+2, par_vertex_idx+4, par_vertex_idx+5);
  state->AddTriangle (par_vertex_idx+2, par_vertex_idx+1, par_vertex_idx+4);
  state->AddTriangle (par_vertex_idx+1, par_vertex_idx+3, par_vertex_idx+4);
  state->AddTriangle (par_vertex_idx+1, par_vertex_idx+0, par_vertex_idx+3);

  if (recursion >= maxdepth) return;
  iSkeletonConnection* con;
  int i;
  for (i = 0 ; i < width ; i++)
  {
    con = parent->CreateConnection ();
    csMatrix3 tr = csYRotMatrix3 (0) *
    	csZRotMatrix3 (0.15f) *
	csXRotMatrix3 (0.15f);
    csTransform trans (tr, -tr.GetInverse () * csVector3 (0, 0.5f, 0));
    con->SetTransformation (trans);
    csRef<iSkeletonLimb> ilimb (SCF_QUERY_INTERFACE (con, iSkeletonLimb));
    add_ghost_limbs (state, frame, ilimb,
    	vertex_idx, par_vertex_idx,
    	maxdepth, 1, recursion+1, dim * 0.7f);
  }
}

// Create a skeletal ghost.
iSkeleton* create_skelghost (iSprite3DFactoryState* state, iSpriteFrame* frame,
	int& vertex_idx, int maxdepth, int width)
{
  state->EnableSkeletalAnimation ();
  iSkeleton* skel = state->GetSkeleton ();
  csRef<iSkeletonLimb> ilimb (SCF_QUERY_INTERFACE (skel, iSkeletonLimb));
  add_ghost_limbs (state, frame, ilimb,
  	vertex_idx, 0, maxdepth, width, 0, 0.2f);
  return skel;
}

// Animate a skeleton.
void animate_skeleton_ghost (iSkeletonLimbState* limb)
{
  iSkeletonLimbState* child = limb->GetChildren ();
  while (child)
  {
    GhostSkelSpriteInfo* o = (GhostSkelSpriteInfo*)child->GetUserData ();
    if (!o)
    {
      o = new GhostSkelSpriteInfo ();
      if ((rand () >> 3) & 0x1)
      {
        o->x_angle_base = (((float)((rand () >> 3)&0xff)) / 255.) * .4 -.2;
        o->x_angle = 0;
        o->dx = (rand () & 0x4) ? .005 : -.005;
        o->z_angle_base = (((float)((rand () >> 3)&0xff)) / 255.) * 1.2 -.6;
        o->z_angle = 0;
        o->dz = (rand () & 0x4) ? .02 : -.02;
      }
      else
      {
        o->z_angle_base = (((float)((rand () >> 3)&0xff)) / 255.) * .4 -.2;
        o->z_angle = 0;
        o->dz = (rand () & 0x4) ? .005 : -.005;
        o->x_angle_base = (((float)((rand () >> 3)&0xff)) / 255.) * 1.2 -.6;
        o->x_angle = 0;
        o->dx = (rand () & 0x4) ? .02 : -.02;
      }
      o->y_angle = 0;
      o->dy = (rand () & 0x4) ? .04 : -.04;
      child->SetUserData (o);
    }
    o->x_angle += o->dx;
    if (o->x_angle > .1 || o->x_angle < -.1) o->dx = -o->dx;
    o->z_angle += o->dz;
    if (o->z_angle > .1 || o->z_angle < -.1) o->dz = -o->dz;
    o->y_angle += o->dy;
    if (o->y_angle > .3 || o->y_angle < -.3) o->dy = -o->dy;

    // @@@ Don't use the code below in a real-time environment.
    // This is only demo code and HIGHLY inefficient.
    csMatrix3 tr = csYRotMatrix3 (o->y_angle) *
    	csZRotMatrix3 (o->z_angle + o->z_angle_base) *
	csXRotMatrix3 (o->x_angle + o->x_angle_base);
    csTransform trans (tr, -tr.GetInverse () * csVector3 (0, .5, 0));
    csRef<iSkeletonConnectionState> con (
    	SCF_QUERY_INTERFACE (child, iSkeletonConnectionState));
    con->SetTransformation (trans);
    animate_skeleton_ghost (child);
    child = child->GetNextSibling ();
  }
}

struct AnimSkelGhost : public iMeshDrawCallback
{
  SCF_DECLARE_IBASE;
  AnimSkelGhost ();
  virtual bool BeforeDrawing (iMeshWrapper* spr, iRenderView* rview);
};


SCF_IMPLEMENT_IBASE (AnimSkelGhost)
  SCF_IMPLEMENTS_INTERFACE (iMeshDrawCallback)
SCF_IMPLEMENT_IBASE_END

AnimSkelGhost::AnimSkelGhost ()
{
  SCF_CONSTRUCT_IBASE (0);
}

bool AnimSkelGhost::BeforeDrawing (iMeshWrapper* spr, iRenderView* /*rview*/)
{
  iMeshObject* obj = spr->GetMeshObject ();
  csRef<iSprite3DState> state (SCF_QUERY_INTERFACE (obj, iSprite3DState));
  iSkeletonState* sk_state = state->GetSkeletonState ();
  csRef<iSkeletonLimbState> isk_limb (SCF_QUERY_INTERFACE (sk_state,
  	iSkeletonLimbState));
  animate_skeleton_ghost (isk_limb);
  return true;
}

// Add a skeletal ghost sprite. If needed it will also create
// the template for this.
void add_skeleton_ghost (iSector* where, csVector3 const& pos, int maxdepth,
	int width)
{
  const char *skelname = "__skelghost__\n";
  csRef<iMeshFactoryWrapper> tmpl;
  tmpl = Sys->Engine->GetMeshFactories ()->FindByName (skelname);
  if (!tmpl)
  {
    tmpl = Sys->Engine->CreateMeshFactory (
    	"crystalspace.mesh.object.sprite.3d", skelname);
    if (!tmpl)
    {
      Sys->Report (CS_REPORTER_SEVERITY_WARNING,
      	"Could not load the sprite 3d plugin!");
      return;
    }
    iMeshObjectFactory* fact = tmpl->GetMeshObjectFactory ();
    csRef<iSprite3DFactoryState> fstate (SCF_QUERY_INTERFACE (fact,
    	iSprite3DFactoryState));
    fstate->SetMaterialWrapper (Sys->Engine->GetMaterialList ()->
			       FindByName ("green"));
    int vertex_idx = 0;
    iSpriteFrame* fr = fstate->AddFrame ();
    fr->SetName ("f");
    iSpriteAction* act = fstate->AddAction ();
    act->SetName ("a");
    act->AddFrame (fr, 100,0);
    create_skelghost (fstate, fr, vertex_idx, maxdepth, width);
  }

  iMeshWrapper* spr = add_meshobj (skelname, "__skelghost__", where, pos, 1);
  iMeshObject* obj = spr->GetMeshObject ();
  csRef<iSprite3DState> state (SCF_QUERY_INTERFACE (obj, iSprite3DState));
  state->SetMixMode (CS_FX_SETALPHA (0.75));
  csRef<iPolygonMesh> mesh (SCF_QUERY_INTERFACE (obj, iPolygonMesh));
  csRef<iObject> sprobj (SCF_QUERY_INTERFACE (spr, iObject));
  (new csColliderWrapper (sprobj, Sys->collide_system, mesh))->DecRef ();
  GhostSpriteInfo* gh_info = new GhostSpriteInfo ();
  csRef<iObject> iobj (SCF_QUERY_INTERFACE (gh_info, iObject));
  sprobj->ObjAdd (iobj);
  gh_info->dir = 1;
  AnimSkelGhost* cb = new AnimSkelGhost ();
  spr->SetDrawCallback (cb);
  cb->DecRef ();
}

#define MAXSECTORSOCCUPIED  20

extern int FindSectors (csVector3 v, csVector3 d, iSector *s, iSector **sa);
extern int CollisionDetect (iEngine* Engine, iCollider *c, iSector* sp,
	csReversibleTransform* cdt);
extern csCollisionPair our_cd_contact[1000];//=0;
extern int num_our_cd;

void move_ghost (iMeshWrapper* spr)
{
  csColliderWrapper* col = csColliderWrapper::GetColliderWrapper (
  	spr->QueryObject ());
  iSector* first_sector = spr->GetMovable ()->GetSectors ()->Get (0);

  // Create a transformation 'test' which indicates where the ghost
  // is moving too.
  const csVector3& pos = spr->GetMovable ()->GetPosition ();
  csVector3 vel (0, 0, 0.1f), rad, cent;
  vel = spr->GetMovable ()->GetTransform ().GetO2T () * vel;
  csVector3 new_pos = pos+vel;
  csMatrix3 m;
  csOrthoTransform test (m, new_pos);

  // Find all sectors that the ghost will occupy on the new position.
  iSector *n[MAXSECTORSOCCUPIED];
  spr->GetRadius(rad,cent);
  int num_sectors = FindSectors (new_pos, 4.0f * rad, first_sector, n);

  // Start collision detection.
  Sys->collide_system->ResetCollisionPairs ();
  num_our_cd = 0;
  Sys->collide_system->SetOneHitOnly (false);
  int hits = 0;
  for ( ; num_sectors-- ; )
    hits += CollisionDetect (Sys->view->GetEngine (), col->GetCollider (),
	n[num_sectors], &test);

  // Change our velocity according to the collisions.
  int j;
  for (j=0 ; j<hits ; j++)
  {
    csCollisionPair& cd = our_cd_contact[j];
    csVector3 n = ((cd.c2-cd.b2)%(cd.b2-cd.a2)).Unit();
    if (n*vel<0)
      continue;
    vel = -(vel%n)%n;
  }

  if (!(vel < EPSILON))
  {
    // We move to our new position.
    new_pos = pos+vel;
    test = csReversibleTransform (csMatrix3 (), pos);
    bool mirror = true;
    first_sector = first_sector->FollowSegment (test, new_pos, mirror);
    spr->GetMovable ()->SetSector (first_sector);
    spr->GetMovable ()->SetPosition (new_pos);
  }

  // Turn around at random intervals.
  csRef<GhostSpriteInfo> gh_info (CS_GET_CHILD_OBJECT (spr->QueryObject (),
						      GhostSpriteInfo));
  if (rand () % 40 == 1) gh_info->dir = -gh_info->dir;

  // OpenStep compiler bug prevents Transform(GetYRotation()), which is why
  // the expressions are split across two statements below.
  if (vel < 0.01f)
  {
    // We did not move much. Turn around quickly.
    csMatrix3 m = csYRotMatrix3 (gh_info->dir*.2);
    spr->GetMovable ()->Transform (m);
  }
  else if (vel < 0.05f)
  {
    // We did a bit. Turn around slightly.
    csMatrix3 m = csYRotMatrix3 (gh_info->dir*.1);
    spr->GetMovable ()->Transform (m);
  }
  else
  {
    csMatrix3 m = csYRotMatrix3 (gh_info->dir*.01);
    spr->GetMovable ()->Transform (m);
  }
  spr->GetMovable ()->UpdateMove ();
}

//===========================================================================
// Everything for bots.
//===========================================================================

Bot* first_bot = 0;
bool do_bots = false;

// Add a bot with some size at the specified positin.
void add_bot (float size, iSector* where, csVector3 const& pos,
	float dyn_radius)
{
  csRef<iDynLight> dyn;
  if (dyn_radius)
  {
    float r, g, b;
    RandomColor (r, g, b);
    dyn = Sys->view->GetEngine ()->CreateDynLight (
    	pos, dyn_radius, csColor(r, g, b));
    dyn->QueryLight ()->SetSector (where);
    dyn->Setup ();
  }
  iMeshFactoryWrapper* tmpl = Sys->view->GetEngine ()->GetMeshFactories ()
  	->FindByName ("bot");
  if (!tmpl) return;
  csRef<iMeshObject> botmesh (tmpl->GetMeshObjectFactory ()->NewInstance ());
  Bot* bot;
  bot = new Bot (Sys->view->GetEngine(), botmesh);
  bot->SetName ("bot");
  Sys->view->GetEngine ()->GetMeshes ()->Add (&(bot->scfiMeshWrapper));
  bot->GetCsMovable ().SetSector (where);
  csMatrix3 m; m.Identity (); m = m * size;
  bot->GetCsMovable ().SetTransform (m);
  bot->set_bot_move (pos);
  bot->set_bot_sector (where);
  bot->GetCsMovable ().UpdateMove ();
  csRef<iSprite3DState> state (SCF_QUERY_INTERFACE (botmesh, iSprite3DState));
  state->SetAction ("default");
  bot->next = first_bot;
  bot->light = dyn;
  first_bot = bot;
  bot->DecRef ();
}

void del_bot ()
{
  if (first_bot)
  {
    Bot* bot = first_bot;
    first_bot = bot->next;
    Sys->view->GetEngine ()->GetMeshes ()->Remove (&(bot->scfiMeshWrapper));
  }
}

void move_bots (csTicks elapsed_time)
{
  if (first_bot)
  {
    Bot* bot = first_bot;
    while (bot)
    {
      bot->move (elapsed_time);
      bot = bot->next;
    }
  }
}

//===========================================================================
// Everything for the missile.
//===========================================================================

#define DYN_TYPE_MISSILE 1
#define DYN_TYPE_RANDOM 2
#define DYN_TYPE_EXPLOSION 3

struct LightStruct
{
  int type;
};

struct MissileStruct
{
  int type;		// type == DYN_TYPE_MISSILE
  csOrthoTransform dir;
  csRef<iMeshWrapper> sprite;
  csRef<iSoundSource> snd;
};

struct ExplosionStruct
{
  int type;		// type == DYN_TYPE_EXPLOSION
  float radius;
  int dir;
};

struct RandomLight
{
  int type;		// type == DYN_TYPE_RANDOM
  float dyn_move_dir;
  float dyn_move;
  float dyn_r1, dyn_g1, dyn_b1;
};

void HandleDynLight (iDynLight* dyn)
{
  LightStruct* ls = (LightStruct*)(csDataObject::GetData(dyn->QueryObject ()));
  csRef<iLight> l (SCF_QUERY_INTERFACE (dyn, iLight));
  switch (ls->type)
  {
    case DYN_TYPE_MISSILE:
    {
      MissileStruct* ms = (MissileStruct*)(csDataObject::GetData(dyn->QueryObject ()));
      csVector3 v (0, 0, 2.5);
      csVector3 old = l->GetCenter ();
      v = old + ms->dir.GetT2O () * v;
      iSector* s = l->GetSector ();
      bool mirror = false;
      csVector3 old_v = v;
      s = s->FollowSegment (ms->dir, v, mirror);
      if (ABS (v.x-old_v.x) > SMALL_EPSILON ||
      	ABS (v.y-old_v.y) > SMALL_EPSILON ||
	ABS (v.z-old_v.z) > SMALL_EPSILON)
      {
        v = old;
        if (ms->sprite)
      	{
          if ((rand () & 0x3) == 1)
	  {
	    int i;
	    if (do_bots)
	      for (i = 0 ; i < 40 ; i++)
            add_bot (1, l->GetSector (), l->GetCenter (), 0);
	  }
	  ms->sprite->GetMovable ()->ClearSectors ();
	  Sys->view->GetEngine ()->GetMeshes ()->Remove (ms->sprite);
	}
	csRef<iDataObject> ido (
		CS_GET_CHILD_OBJECT (dyn->QueryObject (), iDataObject));
        dyn->QueryObject ()->ObjRemove (ido->QueryObject ());
        if (ms->snd)
        {
          ms->snd->Stop();
        }
        delete ms;
        if (Sys->mySound)
        {
          csRef<iSoundSource> sndsrc (Sys->wMissile_boom->CreateSource (SOUND3D_ABSOLUTE));
          if (sndsrc)
          {
            sndsrc->SetPosition (v);
            sndsrc->Play();
          }
        }
        ExplosionStruct* es = new ExplosionStruct;
        es->type = DYN_TYPE_EXPLOSION;
        es->radius = 2;
        es->dir = 1;
        csDataObject* esdata = new csDataObject (es);
        dyn->QueryObject ()->ObjAdd (esdata);
	esdata->DecRef ();
        add_particles_explosion (l->GetSector (), l->GetCenter (), "explo");
        return;
      }
      else ms->dir.SetOrigin (v);
      l->SetSector (s);
      l->SetCenter (v);
      dyn->Setup ();
      if (ms->sprite) move_mesh (ms->sprite, s, v);
      if (Sys->mySound && ms->snd) ms->snd->SetPosition (v);
      break;
    }
    case DYN_TYPE_EXPLOSION:
    {
      ExplosionStruct* es = (ExplosionStruct*)(csDataObject::GetData(
								     dyn->QueryObject ()));
      if (es->dir == 1)
      {
        es->radius += 3;
	if (es->radius > 6) es->dir = -1;
      }
      else
      {
        es->radius -= 2;
	if (es->radius < 1)
	{
	  csRef<iDataObject> ido (
	  	CS_GET_CHILD_OBJECT (dyn->QueryObject (), iDataObject));
	  dyn->QueryObject ()->ObjRemove (ido->QueryObject ());
	  delete es;
          Sys->view->GetEngine ()->RemoveDynLight (dyn);
	  return;
	}
      }
      l->SetInfluenceRadius (es->radius);
      dyn->Setup ();
      break;
    }
    case DYN_TYPE_RANDOM:
    {
      RandomLight* rl = (RandomLight*)(csDataObject::GetData(dyn->QueryObject ()));
      rl->dyn_move += rl->dyn_move_dir;
      if (rl->dyn_move < 0 || rl->dyn_move > 2)
      	rl->dyn_move_dir = -rl->dyn_move_dir;
      if (ABS (rl->dyn_r1-l->GetColor ().red) < .01 &&
      	  ABS (rl->dyn_g1-l->GetColor ().green) < .01 &&
	  ABS (rl->dyn_b1-l->GetColor ().blue) < .01)
        RandomColor (rl->dyn_r1, rl->dyn_g1, rl->dyn_b1);
      else
        l->SetColor (csColor ((rl->dyn_r1+7.*l->GetColor ().red)/8.,
		(rl->dyn_g1+7.*l->GetColor ().green)/8.,
		(rl->dyn_b1+7.*l->GetColor ().blue)/8.));
      l->SetCenter (l->GetCenter () + csVector3 (0, rl->dyn_move_dir, 0));
      dyn->Setup ();
      break;
    }
  }
}

void show_lightning ()
{
  csRef<iEngineSequenceManager> seqmgr(CS_QUERY_REGISTRY (Sys->object_reg,
  	iEngineSequenceManager));
  if (seqmgr)
  {
    // This finds the light L1 (the colored light over the stairs) and
    // makes the lightning restore this color back after it runs.
    iStatLight *light = Sys->view->GetEngine ()->FindLight("l1");
    iSharedVariable *var = Sys->view->GetEngine ()->GetVariableList()
    	->FindByName("Lightning Restore Color");
    if (light && var)
    {
      var->SetColor (light->GetPrivateObject ()->GetColor ());
    }
    seqmgr->RunSequenceByName ("seq_lightning", 0);
  }
  else
  {
    Sys->Report (CS_REPORTER_SEVERITY_NOTIFY,
    	             "Could not find engine sequence manager!");
  }
}

void fire_missile ()
{
  csVector3 dir (0, 0, 0);
  csVector3 pos = Sys->view->GetCamera ()->GetTransform ().This2Other (dir);
  float r, g, b;
  RandomColor (r, g, b);
  csRef<iDynLight> dyn (
  	Sys->view->GetEngine ()->CreateDynLight (pos, 4, csColor (r, g, b)));
  dyn->QueryLight ()->SetSector (Sys->view->GetCamera ()->GetSector ());
  dyn->Setup ();

  MissileStruct* ms = new MissileStruct;
  ms->snd = 0;
  if (Sys->mySound)
  {
    ms->snd = Sys->wMissile_whoosh->CreateSource (SOUND3D_ABSOLUTE);
    if (ms->snd)
    {
      ms->snd->SetPosition (pos);
      ms->snd->Play();
    }
  }
  ms->type = DYN_TYPE_MISSILE;
  ms->dir = (csOrthoTransform)(Sys->view->GetCamera ()->GetTransform ());
  ms->sprite = 0;
  csDataObject* msdata = new csDataObject(ms);
  dyn->QueryObject ()->ObjAdd(msdata);
  msdata->DecRef ();

  char misname[10];
  sprintf (misname, "missile%d", ((rand () >> 3) & 1)+1);

  iMeshFactoryWrapper *tmpl = Sys->view->GetEngine ()->GetMeshFactories ()
  	->FindByName (misname);
  if (!tmpl)
    Sys->Report (CS_REPORTER_SEVERITY_NOTIFY,
    	"Could not find '%s' sprite factory!", misname);
  else
  {
    csRef<iMeshWrapper> sp (
    	Sys->view->GetEngine ()->CreateMeshWrapper (tmpl,
	"missile",Sys->view->GetCamera ()->GetSector (), pos));

    ms->sprite = sp;
    csMatrix3 m = ms->dir.GetT2O ();
    sp->GetMovable ()->SetTransform (m);
    sp->GetMovable ()->UpdateMove ();
  }
}

void AttachRandomLight (iDynLight* light)
{
  RandomLight* rl = new RandomLight;
  rl->type = DYN_TYPE_RANDOM;
  rl->dyn_move_dir = 0.2f;
  rl->dyn_move = 0;
  rl->dyn_r1 = rl->dyn_g1 = rl->dyn_b1 = 1;
  csDataObject* rldata = new csDataObject (rl);
  light->QueryObject ()->ObjAdd (rldata);
  rldata->DecRef ();
}

//===========================================================================

// Light all meshes and animate the skeletal trees.
// This function does no effort at all to optimize stuff. It does
// not test if the mesh is visible or not.
void light_statics ()
{
#if 0
  iEngine* e = Sys->view->GetEngine ();
  iMeshList* meshes = e->GetMeshes ();
  int i;
  for (i = 0 ; i < meshes->GetCount () ; i++)
  {
    iMeshWrapper* sp = meshes->Get (i);
    csRef<iSprite3DState> state (SCF_QUERY_INTERFACE (sp->GetMeshObject (),
    	iSprite3DState));
    if (state != 0)
    {
      if (state->GetSkeletonState ())
      {
        const char* name = sp->QueryObject ()->GetName ();
        if (!strcmp (name, "__skelghost__")) move_ghost (sp);
      }
    }
    sp->DeferUpdateLighting (CS_NLIGHT_STATIC|CS_NLIGHT_DYNAMIC, 10);
  }
#endif
}

//===========================================================================

static csPtr<iMeshWrapper> CreateMeshWrapper (const char* name)
{
  csRef<iPluginManager> plugin_mgr = CS_QUERY_REGISTRY (Sys->object_reg,
  	iPluginManager);
  csRef<iMeshObjectType> ThingType = CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.thing", iMeshObjectType);
  if (!ThingType)
    ThingType = CS_LOAD_PLUGIN (plugin_mgr,
    	"crystalspace.mesh.object.thing", iMeshObjectType);

  csRef<iMeshObjectFactory> thing_fact = ThingType->NewFactory ();
  csRef<iMeshObject> mesh_obj = thing_fact->NewInstance ();

  csRef<iMeshWrapper> mesh_wrap =
  	Sys->Engine->CreateMeshWrapper (mesh_obj, name);
  return csPtr<iMeshWrapper> (mesh_wrap);
}

static csPtr<iMeshWrapper> CreatePortalThing (const char* name, iSector* room,
    	iMaterialWrapper* tm, int& portalPoly)
{
  csRef<iMeshWrapper> thing = CreateMeshWrapper (name);
  csRef<iThingState> thing_state =
  	SCF_QUERY_INTERFACE (thing->GetMeshObject (),
  	iThingState);
  csRef<iThingFactoryState> thing_fact_state = thing_state->GetFactory ();
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

  csRef<iLightingInfo> linfo (SCF_QUERY_INTERFACE (thing->GetMeshObject (),
    iLightingInfo));
  linfo->InitializeDefault (true);
  room->ShineLights (thing);
  linfo->PrepareLighting ();

  return csPtr<iMeshWrapper> (thing);
}

void OpenPortal (iLoader *LevelLoader, iView* view, char* lev)
{
  iSector* room = view->GetCamera ()->GetSector ();
  csVector3 pos = view->GetCamera ()->GetTransform ().This2Other (
  	csVector3 (0, 0, 1));
  iMaterialWrapper* tm = Sys->Engine->GetMaterialList ()->
  	FindByName ("portal");

  int portalPoly;
  csRef<iMeshWrapper> thing = CreatePortalThing ("portalTo", room, tm,
  	portalPoly);
  csRef<iThingState> thing_state = SCF_QUERY_INTERFACE (
  	thing->GetMeshObject (), iThingState);
  csRef<iThingFactoryState> thing_fact_state = thing_state->GetFactory ();
printf ("b\n"); fflush (stdout);

  bool regionExists = (Sys->Engine->GetRegions ()->FindByName (lev) != 0);
  iRegion* cur_region = Sys->Engine->CreateRegion (lev);
  // If the region did not already exist then we load the level in it.
  if (!regionExists)
  {
    // @@@ No error checking!
    char buf[255];
    sprintf (buf, "/lev/%s", lev);
    Sys->myVFS->ChDir (buf);
    LevelLoader->LoadMapFile ("world", false, cur_region, true);
    cur_region->Prepare ();
  }

  iMovable* tmov = thing->GetMovable ();
  tmov->SetPosition (pos + csVector3 (0, Sys->cfg_legs_offset, 0));
  tmov->Transform (view->GetCamera ()->GetTransform ().GetT2O ());
  tmov->UpdateMove ();

  // First make a portal to the new level.
  iCameraPosition* cp = cur_region->FindCameraPosition ("Start");
  const char* room_name;
  csVector3 topos;
  if (cp) { room_name = cp->GetSector (); topos = cp->GetPosition (); }
  else { room_name = "room"; topos.Set (0, 0, 0); }
  topos.y -= Sys->cfg_eye_offset;
  iSector* start_sector = cur_region->FindSector (room_name);
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
	start_sector, poly.GetVertices (), poly.GetVertexCount (),
	portal);
    //iPortal* portal = portalPoly->CreatePortal (start_sector);
    portal->GetFlags ().Set (CS_PORTAL_ZFILL);
    portal->GetFlags ().Set (CS_PORTAL_CLIPDEST);
    portal->SetWarp (view->GetCamera ()->GetTransform ().GetT2O (), topos, pos);

    if (!regionExists)
    {
      // Only if the region did not already exist do we create a portal
      // back. So even if multiple portals go to the region we only have
      // one portal back.
      int portalPolyBack;
      csRef<iMeshWrapper> thingBack = CreatePortalThing ("portalFrom",
	  	start_sector, tm, portalPolyBack);
      thing_state = SCF_QUERY_INTERFACE (thingBack->GetMeshObject (),
      	iThingState);
      thing_fact_state = thing_state->GetFactory ();
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
	  polyBack.GetVertexCount (),
	  portalBack);
      //iPortal* portalBack = portalPolyBack->CreatePortal (room);
      portalBack->GetFlags ().Set (CS_PORTAL_ZFILL);
      portalBack->GetFlags ().Set (CS_PORTAL_CLIPDEST);
      portalBack->SetWarp (view->GetCamera ()->GetTransform ().GetO2T (),
      	-pos, -topos);
    }
  }

  if (!regionExists)
    Sys->InitCollDet (Sys->Engine, cur_region);
}

