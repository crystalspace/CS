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
#include "hugeroom.h"
#include "command.h"
#include "ivaria/view.h"
#include "iengine/dynlight.h"
#include "iengine/light.h"
#include "iengine/campos.h"
#include "iengine/region.h"
#include "imesh/thing/polygon.h"
#include "csutil/scanstr.h"
#include "cstool/impexp.h"
#include "csutil/dataobj.h"
#include "cstool/crossb.h"
#include "csgeom/math3d.h"
#include "cssys/system.h"
#include "cstool/collider.h"
#include "cstool/cspixmap.h"
#include "qint.h"
#include "isound/handle.h"
#include "isound/source.h"
#include "isound/listener.h"
#include "isound/source.h"
#include "isound/renderer.h"
#include "isound/wrapper.h"
#include "ivideo/graph3d.h"
#include "ivaria/collider.h"
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

extern iMeshWrapper* add_meshobj (char* tname, char* sname, iSector* where,
	csVector3 const& pos, float size);
extern void move_mesh (iMeshWrapper* sprite, iSector* where,
	csVector3 const& pos);

//===========================================================================
// Demo particle system (rain).
//===========================================================================
void add_particles_rain (iSector* sector, char* matname, int num, float speed)
{
  // First check if the material exists.
  iMaterialWrapper* mat = Sys->view->GetEngine ()->FindMaterial (matname);
  if (!mat)
  {
    Sys->Printf (CS_MSG_CONSOLE, "Can't find material '%s' in memory!\n", matname);
    return;
  }

  csBox3 bbox;
  sector->CalculateSectorBBox (bbox, true);

  // @@@ Memory leak on factories!
  iMeshObjectType* type = CS_QUERY_PLUGIN_CLASS (Sys, "crystalspace.mesh.object.rain",
    	  "MeshObj", iMeshObjectType);
  if (!type) type = CS_LOAD_PLUGIN (Sys, "crystalspace.mesh.object.rain",
    	  "MeshObj", iMeshObjectType);
  iMeshObjectFactory* factory = type->NewFactory ();
  iMeshObject* mesh = factory->NewInstance ();
  iMeshWrapper* exp = Sys->view->GetEngine ()->CreateMeshObject (mesh, NULL,
  	sector);
  type->DecRef ();
  factory->DecRef ();
  mesh->DecRef ();

  exp->SetZBufMode(CS_ZBUF_TEST);

  iParticleState* partstate = SCF_QUERY_INTERFACE (mesh, iParticleState);
  partstate->SetMaterialWrapper (mat);
  partstate->SetMixMode (CS_FX_ADD);
  partstate->SetColor (csColor (.25,.25,.25));
  partstate->DecRef ();

  iRainState* rainstate = SCF_QUERY_INTERFACE (mesh, iRainState);
  rainstate->SetParticleCount (num);
  rainstate->SetDropSize (.3/50., .3);
  rainstate->SetLighting (false);
  rainstate->SetBox (bbox.Min (), bbox.Max ());
  rainstate->SetFallSpeed (csVector3 (0, -speed, 0));
  rainstate->DecRef ();
}

//===========================================================================
// Demo particle system (snow).
//===========================================================================
void add_particles_snow (iSector* sector, char* matname, int num, float speed)
{
  // First check if the material exists.
  iMaterialWrapper* mat = Sys->view->GetEngine ()->FindMaterial (matname);
  if (!mat)
  {
    Sys->Printf (CS_MSG_CONSOLE, "Can't find material '%s' in memory!\n", matname);
    return;
  }

  csBox3 bbox;
  sector->CalculateSectorBBox (bbox, true);

  // @@@ Memory leak on factories!
  iMeshObjectType* type = CS_QUERY_PLUGIN_CLASS (Sys, "crystalspace.mesh.object.snow",
    	  "MeshObj", iMeshObjectType);
  if (!type) type = CS_LOAD_PLUGIN (Sys, "crystalspace.mesh.object.snow",
    	  "MeshObj", iMeshObjectType);
  iMeshObjectFactory* factory = type->NewFactory ();
  iMeshObject* mesh = factory->NewInstance ();
  iMeshWrapper* exp = Sys->view->GetEngine ()->CreateMeshObject (mesh, NULL,
  	sector);
  type->DecRef ();
  factory->DecRef ();
  mesh->DecRef ();

  exp->SetZBufMode(CS_ZBUF_TEST);

  iParticleState* partstate = SCF_QUERY_INTERFACE (mesh, iParticleState);
  partstate->SetMaterialWrapper (mat);
  partstate->SetMixMode (CS_FX_ADD);
  partstate->SetColor (csColor (.25,.25,.25));
  partstate->DecRef ();

  iSnowState* snowstate = SCF_QUERY_INTERFACE (mesh, iSnowState);
  snowstate->SetParticleCount (num);
  snowstate->SetDropSize (.07, .07);
  snowstate->SetLighting (false);
  snowstate->SetBox (bbox.Min (), bbox.Max ());
  snowstate->SetFallSpeed (csVector3 (0, -speed, 0));
  snowstate->SetSwirl (0.2);
  snowstate->DecRef ();
}

//===========================================================================
// Demo particle system (fire).
//===========================================================================
void add_particles_fire (iSector* sector, char* matname, int num,
	const csVector3& origin)
{
  // First check if the material exists.
  iMaterialWrapper* mat = Sys->view->GetEngine ()->FindMaterial (matname);
  if (!mat)
  {
    Sys->Printf (CS_MSG_CONSOLE, "Can't find material '%s' in memory!\n", matname);
    return;
  }

  // @@@ Memory leak on factories!
  iMeshObjectType* type = CS_QUERY_PLUGIN_CLASS (Sys, "crystalspace.mesh.object.fire",
      	"MeshObj", iMeshObjectType);
  if (!type) type = CS_LOAD_PLUGIN (Sys, "crystalspace.mesh.object.fire",
      	"MeshObj", iMeshObjectType);
  iMeshObjectFactory* factory = type->NewFactory ();
  iMeshObject* mesh = factory->NewInstance ();
  iMeshWrapper* exp = Sys->view->GetEngine ()->CreateMeshObject (mesh, NULL,
  	sector);
  type->DecRef ();
  factory->DecRef ();
  mesh->DecRef ();

  exp->SetZBufMode(CS_ZBUF_TEST);

  iParticleState* partstate = SCF_QUERY_INTERFACE (mesh, iParticleState);
  partstate->SetMaterialWrapper (mat);
  partstate->SetMixMode (CS_FX_ADD);
  partstate->DecRef ();

  iFireState* firestate = SCF_QUERY_INTERFACE (mesh, iFireState);
  firestate->SetParticleCount (num);
  //firestate->SetDropSize (.02, .04);
  firestate->SetDropSize (.04, .08);
  firestate->SetLighting (false);
  firestate->SetOrigin (csBox3(origin-csVector3(.2,0,.2), 
    origin+csVector3(.2,0.2)));
  firestate->SetDirection (csVector3 (0, 1., 0));
  firestate->SetSwirl (1.6);
  firestate->SetColorScale (0.2);
  firestate->DecRef ();
}

//===========================================================================
// Demo particle system (fountain).
//===========================================================================
void add_particles_fountain (iSector* sector, char* matname, int num,
	const csVector3& origin)
{
  // First check if the material exists.
  iMaterialWrapper* mat = Sys->view->GetEngine ()->FindMaterial (matname);
  if (!mat)
  {
    Sys->Printf (CS_MSG_CONSOLE, "Can't find material '%s' in memory!\n", matname);
    return;
  }

  // @@@ Memory leak on factories!
  iMeshObjectType* type = CS_QUERY_PLUGIN_CLASS (Sys, "crystalspace.mesh.object.fountain",
      	"MeshObj", iMeshObjectType);
  if (!type) type = CS_LOAD_PLUGIN (Sys, "crystalspace.mesh.object.fountain",
      	"MeshObj", iMeshObjectType);
  iMeshObjectFactory* factory = type->NewFactory ();
  iMeshObject* mesh = factory->NewInstance ();
  iMeshWrapper* exp = Sys->view->GetEngine ()->CreateMeshObject (mesh, NULL,
  	sector, origin);
  type->DecRef ();
  factory->DecRef ();
  mesh->DecRef ();

  exp->SetZBufMode(CS_ZBUF_TEST);

  iParticleState* partstate = SCF_QUERY_INTERFACE (mesh, iParticleState);
  partstate->SetMaterialWrapper (mat);
  partstate->SetMixMode (CS_FX_ADD);
  partstate->SetColor (csColor (.25, .35, .55));
  partstate->SetChangeRotation (7.5);
  partstate->DecRef ();

  iFountainState* fountstate = SCF_QUERY_INTERFACE (mesh, iFountainState);
  fountstate->SetParticleCount (num);
  fountstate->SetDropSize (.1, .1);
  fountstate->SetOrigin (csVector3 (0, 0, 0));
  fountstate->SetAcceleration (csVector3 (0, -1., 0));
  fountstate->SetFallTime (5.0);
  fountstate->SetSpeed (3.0);
  fountstate->SetElevation (3.1415926/2.);
  fountstate->SetAzimuth (0);
  fountstate->SetOpening (.2);
  fountstate->DecRef ();
}

//===========================================================================
// Demo particle system (explosion).
//===========================================================================
void add_particles_explosion (iSector* sector, const csVector3& center, char* matname)
{
  // First check if the material exists.
  iMaterialWrapper* mat = Sys->view->GetEngine ()->FindMaterial (matname);
  if (!mat)
  {
    Sys->Printf (CS_MSG_CONSOLE, "Can't find material '%s' in memory!\n", matname);
    return;
  }

  // @@@ Memory leak on factories!
  iMeshObjectType* type = CS_QUERY_PLUGIN_CLASS (Sys, "crystalspace.mesh.object.explosion",
      	"MeshObj", iMeshObjectType);
  if (!type) type = CS_LOAD_PLUGIN (Sys, "crystalspace.mesh.object.explosion",
      	"MeshObj", iMeshObjectType);
  iMeshObjectFactory* factory = type->NewFactory ();
  iMeshObject* mesh = factory->NewInstance ();
  iMeshWrapper* exp = Sys->view->GetEngine ()->CreateMeshObject (mesh, NULL,
  	sector, center);
  type->DecRef ();
  factory->DecRef ();
  mesh->DecRef ();

  exp->SetZBufMode(CS_ZBUF_TEST);

  iParticleState* partstate = SCF_QUERY_INTERFACE (mesh, iParticleState);
  partstate->SetMaterialWrapper (mat);
  partstate->SetMixMode (CS_FX_SETALPHA (0.50));
  partstate->SetColor (csColor (1, 1, 0));
  partstate->SetChangeRotation (5.0);
  partstate->SetChangeSize (1.25);
  partstate->SetSelfDestruct (3000);
  partstate->DecRef ();

  iExplosionState* expstate = SCF_QUERY_INTERFACE (mesh, iExplosionState);
  expstate->SetParticleCount (100);
  expstate->SetCenter (csVector3 (0, 0, 0));
  expstate->SetPush (csVector3 (0, 0, 0));
  expstate->SetNrSides (6);
  expstate->SetPartRadius (0.15);
  expstate->SetLighting (true);
  expstate->SetSpreadPos (.6);
  expstate->SetSpreadSpeed (2.);
  expstate->SetSpreadAcceleration (2.);
  expstate->SetFadeSprites (500);
  expstate->AddLight (Sys->Engine, sector, 1000);
  expstate->DecRef ();
}

//===========================================================================
// Demo particle system (spiral).
//===========================================================================
void add_particles_spiral (iSector* sector, const csVector3& bottom, char* matname)
{
  // First check if the material exists.
  iMaterialWrapper* mat = Sys->view->GetEngine ()->FindMaterial (matname);
  if (!mat)
  {
    Sys->Printf (CS_MSG_CONSOLE, "Can't find material '%s' in memory!\n", matname);
    return;
  }

  // @@@ Memory leak on factories!
  iMeshObjectType* type = CS_QUERY_PLUGIN_CLASS (Sys, "crystalspace.mesh.object.spiral",
      	"MeshObj", iMeshObjectType);
  if (!type) type = CS_LOAD_PLUGIN (Sys, "crystalspace.mesh.object.spiral",
      	"MeshObj", iMeshObjectType);
  iMeshObjectFactory* factory = type->NewFactory ();
  iMeshObject* mesh = factory->NewInstance ();
  iMeshWrapper* exp = Sys->view->GetEngine ()->CreateMeshObject
    (mesh, NULL, sector, bottom);
  type->DecRef ();
  factory->DecRef ();
  mesh->DecRef ();

  exp->SetZBufMode(CS_ZBUF_TEST);

  iParticleState* partstate = SCF_QUERY_INTERFACE (mesh, iParticleState);
  partstate->SetMaterialWrapper (mat);
  partstate->SetMixMode (CS_FX_SETALPHA (0.50));
  partstate->SetColor (csColor (1, 1, 0));
  partstate->SetChangeColor (csColor(+0.01,0.,-0.012));
  partstate->DecRef ();

  iSpiralState* spirstate = SCF_QUERY_INTERFACE (mesh, iSpiralState);
  spirstate->SetParticleCount (500);
  spirstate->SetSource (csVector3 (0, 0, 0));
  spirstate->DecRef ();
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

  state->GetVertex (anm_idx, par_vertex_idx+0) = csVector3(-.05, 0, -.05);
  state->GetVertex (anm_idx, par_vertex_idx+1) = csVector3(.05, 0, -.05);
  state->GetVertex (anm_idx, par_vertex_idx+2) = csVector3(0, 0, .05);
  state->GetVertex (anm_idx, par_vertex_idx+3) = csVector3(-.05, .45, -.05);
  state->GetVertex (anm_idx, par_vertex_idx+4) = csVector3(.05, .45, -.05);
  state->GetVertex (anm_idx, par_vertex_idx+5) = csVector3(0, .45, .05);

  state->GetTexel (tex_idx, par_vertex_idx+0) = csVector2(0, 0);
  state->GetTexel (tex_idx, par_vertex_idx+1) = csVector2(.99, 0);
  state->GetTexel (tex_idx, par_vertex_idx+2) = csVector2(0, .99);
  state->GetTexel (tex_idx, par_vertex_idx+3) = csVector2(.99, .99);
  state->GetTexel (tex_idx, par_vertex_idx+4) = csVector2(.5, .5);
  state->GetTexel (tex_idx, par_vertex_idx+5) = csVector2(.5, 0);

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
    csMatrix3 tr = csYRotMatrix3 (0) * csZRotMatrix3(.15) *
                                                 csXRotMatrix3(.15);
    csTransform trans (tr, -tr.GetInverse () * csVector3 (0, .5, 0));
    con->SetTransformation (trans);
    iSkeletonLimb* ilimb = SCF_QUERY_INTERFACE (con, iSkeletonLimb);
    add_tree_limbs (state, frame, ilimb,
    	vertex_idx, par_vertex_idx, maxdepth, width, recursion+1);
    ilimb->DecRef ();
  }
}

// Create a skeletal tree.
iSkeleton* create_skeltree (iSprite3DFactoryState* state, iSpriteFrame* frame,
	int& vertex_idx, int maxdepth, int width)
{
  state->EnableSkeletalAnimation ();
  iSkeleton* skel = state->GetSkeleton ();
  iSkeletonLimb* ilimb = SCF_QUERY_INTERFACE (skel, iSkeletonLimb);
  add_tree_limbs (state, frame, ilimb,
  	vertex_idx, 0, maxdepth, width, 0);
  ilimb->DecRef ();
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
    iSkeletonConnectionState* con = SCF_QUERY_INTERFACE (child, iSkeletonConnectionState);
    con->SetTransformation (trans);
    con->DecRef ();
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
  SCF_CONSTRUCT_IBASE (NULL);
}

bool AnimSkelTree::BeforeDrawing (iMeshWrapper* spr, iRenderView* /*rview*/)
{
  iMeshObject* obj = spr->GetMeshObject ();
  iSprite3DState* state = SCF_QUERY_INTERFACE (obj, iSprite3DState);
  iSkeletonState* sk_state = state->GetSkeletonState ();
  animate_skeleton_tree (SCF_QUERY_INTERFACE (sk_state, iSkeletonLimbState));
  state->DecRef ();
  return true;
}

// Add a skeletal tree sprite. If needed it will also create
// the template for this.
void add_skeleton_tree (iSector* where, csVector3 const& pos, int depth,
	int width)
{
  char skelname[50];
  sprintf (skelname, "__skeltree__%d,%d\n", depth, width);
  iMeshFactoryWrapper* tmpl = Sys->Engine->FindMeshFactory (skelname);
  if (!tmpl)
  {
    tmpl = Sys->Engine->CreateMeshFactory (
    	"crystalspace.mesh.object.sprite.3d", skelname);
    if (tmpl == NULL)
    {
      Sys->Printf (CS_MSG_WARNING, "Could not load the sprite 3d plugin!\n");
      return;
    }
    iMeshObjectFactory* fact = tmpl->GetMeshObjectFactory ();
    iSprite3DFactoryState* state = SCF_QUERY_INTERFACE (fact, iSprite3DFactoryState);
    state->SetMaterialWrapper (Sys->Engine->FindMaterial ("white"));
    int vertex_idx = 0;
    iSpriteFrame* fr = state->AddFrame ();
    fr->SetName ("f");
    iSpriteAction* act = state->AddAction ();
    act->SetName ("a");
    act->AddFrame (fr, 100);
    create_skeltree (state, fr, vertex_idx, depth, width);
    state->DecRef ();
  }
  iMeshWrapper* spr = add_meshobj (skelname, "__skeltree__",
  	where, pos-csVector3 (0, Sys->cfg_body_height, 0), 1);
  AnimSkelTree* cb = new AnimSkelTree ();
  spr->SetDrawCallback (cb);
  cb->DecRef ();
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

  state->GetVertex (anm_idx, par_vertex_idx+0) = csVector3(-dim, 0, -dim);
  state->GetVertex (anm_idx, par_vertex_idx+1) = csVector3(dim, 0, -dim);
  state->GetVertex (anm_idx, par_vertex_idx+2) = csVector3(0, 0, dim);
  state->GetVertex (anm_idx, par_vertex_idx+3) = csVector3(-dim, .45, -dim);
  state->GetVertex (anm_idx, par_vertex_idx+4) = csVector3(dim, .45, -dim);
  state->GetVertex (anm_idx, par_vertex_idx+5) = csVector3(0, .45, dim);

  state->GetTexel (tex_idx, par_vertex_idx+0) = csVector2(0, 0);
  state->GetTexel (tex_idx, par_vertex_idx+1) = csVector2(.99, 0);
  state->GetTexel (tex_idx, par_vertex_idx+2) = csVector2(0, .99);
  state->GetTexel (tex_idx, par_vertex_idx+3) = csVector2(.99, .99);
  state->GetTexel (tex_idx, par_vertex_idx+4) = csVector2(.5, .5);
  state->GetTexel (tex_idx, par_vertex_idx+5) = csVector2(.5, 0);

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
    	csZRotMatrix3 (.15) *
	csXRotMatrix3 (.15);
    csTransform trans (tr, -tr.GetInverse () * csVector3 (0, .5, 0));
    con->SetTransformation (trans);
    iSkeletonLimb* ilimb = SCF_QUERY_INTERFACE (con, iSkeletonLimb);
    add_ghost_limbs (state, frame, ilimb,
    	vertex_idx, par_vertex_idx,
    	maxdepth, 1, recursion+1, dim * .7);
    ilimb->DecRef ();
  }
}

// Create a skeletal ghost.
iSkeleton* create_skelghost (iSprite3DFactoryState* state, iSpriteFrame* frame,
	int& vertex_idx, int maxdepth, int width)
{
  state->EnableSkeletalAnimation ();
  iSkeleton* skel = state->GetSkeleton ();
  iSkeletonLimb* ilimb = SCF_QUERY_INTERFACE (skel, iSkeletonLimb);
  add_ghost_limbs (state, frame, ilimb,
  	vertex_idx, 0, maxdepth, width, 0, .2);
  ilimb->DecRef ();
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
    iSkeletonConnectionState* con = SCF_QUERY_INTERFACE (child, iSkeletonConnectionState);
    con->SetTransformation (trans);
    con->DecRef ();
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
  SCF_CONSTRUCT_IBASE (NULL);
}

bool AnimSkelGhost::BeforeDrawing (iMeshWrapper* spr, iRenderView* /*rview*/)
{
  iMeshObject* obj = spr->GetMeshObject ();
  iSprite3DState* state = SCF_QUERY_INTERFACE (obj, iSprite3DState);
  iSkeletonState* sk_state = state->GetSkeletonState ();
  iSkeletonLimbState* isk_limb = SCF_QUERY_INTERFACE (sk_state, iSkeletonLimbState);
  animate_skeleton_ghost (isk_limb);
  isk_limb->DecRef ();
  state->DecRef ();
  return true;
}

// Add a skeletal ghost sprite. If needed it will also create
// the template for this.
void add_skeleton_ghost (iSector* where, csVector3 const& pos, int maxdepth,
	int width)
{
  char skelname[50];
  sprintf (skelname, "__skelghost__\n");
  iMeshFactoryWrapper* tmpl = Sys->Engine->FindMeshFactory (skelname);
  if (!tmpl)
  {
    tmpl = Sys->Engine->CreateMeshFactory (
    	"crystalspace.mesh.object.sprite.3d", skelname);
    if (tmpl == NULL)
    {
      Sys->Printf (CS_MSG_WARNING, "Could not load the sprite 3d plugin!\n");
      return;
    }
    iMeshObjectFactory* fact = tmpl->GetMeshObjectFactory ();
    iSprite3DFactoryState* state = SCF_QUERY_INTERFACE (fact, iSprite3DFactoryState);
    state->SetMaterialWrapper (Sys->Engine->FindMaterial ("green"));
    int vertex_idx = 0;
    iSpriteFrame* fr = state->AddFrame ();
    fr->SetName ("f");
    iSpriteAction* act = state->AddAction ();
    act->SetName ("a");
    act->AddFrame (fr, 100);
    create_skelghost (state, fr, vertex_idx, maxdepth, width);
    state->DecRef ();
  }
  iMeshWrapper* spr = add_meshobj (skelname, "__skelghost__", where, pos, 1);
  iMeshObject* obj = spr->GetMeshObject ();
  iSprite3DState* state = SCF_QUERY_INTERFACE (obj, iSprite3DState);
  state->SetMixMode (CS_FX_SETALPHA (0.75));
  iPolygonMesh* mesh = SCF_QUERY_INTERFACE (obj, iPolygonMesh);
  iObject* sprobj = SCF_QUERY_INTERFACE (spr, iObject);
  (void)new csColliderWrapper (sprobj, Sys->collide_system, mesh);
  GhostSpriteInfo* gh_info = new GhostSpriteInfo ();
  iObject* iobj = SCF_QUERY_INTERFACE (gh_info, iObject);
  sprobj->ObjAdd (iobj);
  gh_info->dir = 1;
  AnimSkelGhost* cb = new AnimSkelGhost ();
  spr->SetDrawCallback (cb);
  cb->DecRef ();
  iobj->DecRef ();
  state->DecRef ();
  mesh->DecRef ();
  sprobj->DecRef ();
}

#define MAXSECTORSOCCUPIED  20

extern int FindSectors (csVector3 v, csVector3 d, iSector *s, iSector **sa);
extern int CollisionDetect (csColliderWrapper *c, iSector* sp, csTransform *cdt);
extern csCollisionPair our_cd_contact[1000];//=0;
extern int num_our_cd;

void move_ghost (iMeshWrapper* spr)
{
  printf("Moving ghost\n");
  csColliderWrapper* col = csColliderWrapper::GetColliderWrapper (
  	spr->QueryObject ());
  iSector* first_sector = spr->GetMovable ()->GetSector (0);

  // Create a transformation 'test' which indicates where the ghost
  // is moving too.
  const csVector3& pos = spr->GetMovable ()->GetPosition ();
  csVector3 vel (0, 0, .1), rad, cent;
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
    hits += CollisionDetect (col, n[num_sectors], &test);

  // Change our velocity according to the collisions.
  for (int j=0 ; j<hits ; j++)
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
  GhostSpriteInfo* gh_info = CS_GET_CHILD_OBJECT_FAST(spr->QueryObject (),
  	GhostSpriteInfo);
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

Bot* first_bot = NULL;
bool do_bots = false;

// Add a bot with some size at the specified positin.
void add_bot (float size, iSector* where, csVector3 const& pos,
	float dyn_radius)
{
  iDynLight* dyn = NULL;
  if (dyn_radius)
  {
    float r, g, b;
    RandomColor (r, g, b);
    //@@@ MEMORY LEAK?
    dyn = Sys->view->GetEngine ()->CreateDynLight (pos, dyn_radius, csColor(r, g, b));
    dyn->QueryLight ()->SetSector (where);
    dyn->Setup ();
  }
  csMeshFactoryWrapper* tmpl = (csMeshFactoryWrapper*)
  	Sys->view->GetEngine ()->GetCsEngine ()->mesh_factories.FindByName ("bot");
  if (!tmpl) return;
  iMeshObject* botmesh = tmpl->GetMeshObjectFactory ()->NewInstance ();
  Bot* bot;
  bot = new Bot (Sys->view->GetEngine()->GetCsEngine (), botmesh);
  botmesh->DecRef ();
  bot->SetName ("bot");
  Sys->view->GetEngine ()->GetCsEngine ()->meshes.Push (bot);
  bot->GetMovable ().SetSector (where);
  csMatrix3 m; m.Identity (); m = m * size;
  bot->GetMovable ().SetTransform (m);
  bot->set_bot_move (pos);
  bot->set_bot_sector (where);
  bot->GetMovable ().UpdateMove ();
  iSprite3DState* state = SCF_QUERY_INTERFACE (botmesh, iSprite3DState);
  state->SetAction ("default");
  state->DecRef ();
  bot->next = first_bot;
  bot->light = dyn;
  first_bot = bot;
}

void del_bot ()
{
  if (first_bot)
  {
    Bot* bot = first_bot;
    first_bot = bot->next;
    Sys->view->GetEngine ()->GetCsEngine ()->RemoveMesh (bot);
  }
}

void move_bots (csTime elapsed_time)
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
  csMeshWrapper* sprite;
  iSoundSource *snd;
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
  iLight* l = SCF_QUERY_INTERFACE (dyn, iLight);
  l->DecRef ();
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
	  ms->sprite->GetMovable ().ClearSectors ();
	  Sys->view->GetEngine ()->GetCsEngine ()->RemoveMesh (ms->sprite);
	}
        dyn->QueryObject ()->ObjRemove (CS_GET_CHILD_OBJECT_FAST (
	  dyn->QueryObject (), iDataObject)->QueryObject ());
        if (ms->snd)
        {
          ms->snd->Stop();
          ms->snd->DecRef();
        }
        delete ms;
        if (Sys->mySound)
        {
          iSoundSource *sndsrc;
          if ((sndsrc = Sys->wMissile_boom->CreateSource (SOUND3D_ABSOLUTE)))
          {
            sndsrc->SetPosition (v);
            sndsrc->Play();
            sndsrc->DecRef();
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
      if (ms->sprite) move_mesh (&(ms->sprite->scfiMeshWrapper), s, v);
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
	  delete es;
          Sys->view->GetEngine ()->GetCsEngine ()->RemoveDynLight (dyn);
          dyn->DecRef ();
	  return;
	}
      }
      l->SetRadius (es->radius);
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

void fire_missile ()
{
  csVector3 dir (0, 0, 0);
  csVector3 pos = Sys->view->GetCamera ()->GetTransform ().This2Other (dir);
  float r, g, b;
  RandomColor (r, g, b);
  iDynLight* dyn = Sys->view->GetEngine ()->CreateDynLight (pos, 4, csColor (r, g, b));
  dyn->QueryLight ()->SetSector (Sys->view->GetCamera ()->GetSector ());
  dyn->Setup ();

  MissileStruct* ms = new MissileStruct;
  ms->snd = NULL;
  if (Sys->mySound)
    if ((ms->snd = Sys->wMissile_whoosh->CreateSource (SOUND3D_ABSOLUTE)))
    {
      ms->snd->SetPosition (pos);
      ms->snd->Play();
    }
  ms->type = DYN_TYPE_MISSILE;
  ms->dir = (csOrthoTransform)(Sys->view->GetCamera ()->GetTransform ());
  ms->sprite = NULL;
  csDataObject* msdata = new csDataObject(ms);
  dyn->QueryObject ()->ObjAdd(msdata);
  msdata->DecRef ();

  char misname[10];
  sprintf (misname, "missile%d", ((rand () >> 3) & 1)+1);

  iMeshFactoryWrapper *tmpl = Sys->view->GetEngine ()->FindMeshFactory (misname);
  if (!tmpl)
    Sys->Printf (CS_MSG_CONSOLE, "Could not find '%s' sprite factory!\n", misname);
  else
  {
    iMeshWrapper* sp = Sys->view->GetEngine ()->CreateMeshObject (tmpl, "missile");

    sp->GetMovable ()->SetSector (Sys->view->GetCamera ()->GetSector ());
    ms->sprite = sp->GetPrivateObject ();
    sp->GetMovable ()->SetPosition (pos);
    csMatrix3 m = ms->dir.GetT2O ();
    sp->GetMovable ()->SetTransform (m);
    move_mesh (sp, Sys->view->GetCamera ()->GetSector (), pos);
    sp->GetMovable ()->UpdateMove ();
  } 
}

void AttachRandomLight (csDynLight* light)
{
  RandomLight* rl = new RandomLight;
  rl->type = DYN_TYPE_RANDOM;
  rl->dyn_move_dir = .2;
  rl->dyn_move = 0;
  rl->dyn_r1 = rl->dyn_g1 = rl->dyn_b1 = 1;
  csDataObject* rldata = new csDataObject (rl);
  light->ObjAdd (rldata);
  rldata->DecRef ();
}

//===========================================================================

// Light all meshes and animate the skeletal trees.
// This function does no effort at all to optimize stuff. It does
// not test if the mesh is visible or not.
void light_statics ()
{
  iEngine* e = Sys->view->GetEngine ();
  for (int i = 0 ; i < e->GetMeshObjectCount () ; i++)
  {
    iMeshWrapper* sp = e->GetMeshObject (i);
    iSprite3DState* state = SCF_QUERY_INTERFACE (sp->GetMeshObject (), iSprite3DState);
    if (state != NULL)
    {
      if (state->GetSkeletonState ())
      {
        const char* name = sp->QueryObject ()->GetName ();
        if (!strcmp (name, "__skelghost__")) move_ghost (sp);
      }
      state->DecRef ();
    }
    sp->DeferUpdateLighting (CS_NLIGHT_STATIC|CS_NLIGHT_DYNAMIC, 10);
  }
}

//===========================================================================

static iMeshWrapper* CreateMeshWrapper (const char* name)
{
  iMeshObjectFactory* thing_fact = Sys->Engine->GetThingType ()->NewFactory ();
  iMeshObject* mesh_obj = SCF_QUERY_INTERFACE (thing_fact, iMeshObject);
  thing_fact->DecRef ();

  iMeshWrapper* mesh_wrap = Sys->Engine->CreateMeshObject (mesh_obj, name);
  mesh_obj->DecRef ();
  return mesh_wrap;
}

iMeshWrapper* CreatePortalThing (const char* name, iSector* room,
    	iMaterialWrapper* tm, iPolygon3D*& portalPoly)
{
  iMeshWrapper* thing = CreateMeshWrapper (name);
  iThingState* thing_state = SCF_QUERY_INTERFACE (thing->GetMeshObject (),
  	iThingState);
  thing_state->SetMovingOption (CS_THING_MOVE_OCCASIONAL);
  thing->GetMovable ()->SetSector (room);
  float dx = 1, dy = 3, dz = .3;
  float border = 0.3; // width of border around the portal

  // bottom
  iPolygon3D* p;
  p = thing_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->GetFlags ().Reset (CS_POLY_COLLDET);
  p->CreateVertex (csVector3 (-dx, 0, -dz));
  p->CreateVertex (csVector3 (dx, 0, -dz));
  p->CreateVertex (csVector3 (dx, 0, dz));
  p->CreateVertex (csVector3 (-dx, 0, dz));
  p->SetTextureSpace (
      p->GetVertex (0), csVector2 (0.25, 0.875),
      p->GetVertex (1), csVector2 (0.75, 0.875),
      p->GetVertex (2), csVector2 (0.75, 0.75));

  // top
  p = thing_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->GetFlags ().Reset (CS_POLY_COLLDET);
  p->CreateVertex (csVector3 (-dx, dy, dz));
  p->CreateVertex (csVector3 (dx, dy, dz));
  p->CreateVertex (csVector3 (dx, dy, -dz));
  p->CreateVertex (csVector3 (-dx, dy, -dz));
  p->SetTextureSpace (
      p->GetVertex (0), csVector2 (0.25, 0.25),
      p->GetVertex (1), csVector2 (0.75, 0.25),
      p->GetVertex (2), csVector2 (0.75, 0.125));

  // back
  p = thing_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->GetFlags ().Reset (CS_POLY_COLLDET);
  p->CreateVertex (csVector3 (-dx, 0, dz));
  p->CreateVertex (csVector3 (dx, 0, dz));
  p->CreateVertex (csVector3 (dx, dy, dz));
  p->CreateVertex (csVector3 (-dx, dy, dz));
  p->SetTextureSpace (
      p->GetVertex (0), csVector2 (0.25, 0.75),
      p->GetVertex (1), csVector2 (0.75, 0.75),
      p->GetVertex (2), csVector2 (0.75, 0.25));

  // right
  p = thing_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->GetFlags ().Reset (CS_POLY_COLLDET);
  p->CreateVertex (csVector3 (dx, 0, dz));
  p->CreateVertex (csVector3 (dx, 0, -dz));
  p->CreateVertex (csVector3 (dx, dy, -dz));
  p->CreateVertex (csVector3 (dx, dy, dz));
  p->SetTextureSpace (
      p->GetVertex (0), csVector2 (0.75, 0.75),
      p->GetVertex (1), csVector2 (0.875, 0.75),
      p->GetVertex (2), csVector2 (0.875, 0.25));

  // left
  p = thing_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->GetFlags ().Reset (CS_POLY_COLLDET);
  p->CreateVertex (csVector3 (-dx, 0, -dz));
  p->CreateVertex (csVector3 (-dx, 0, dz));
  p->CreateVertex (csVector3 (-dx, dy, dz));
  p->CreateVertex (csVector3 (-dx, dy, -dz));
  p->SetTextureSpace (
      p->GetVertex (0), csVector2 (0.125, 0.75),
      p->GetVertex (1), csVector2 (0.25, 0.75),
      p->GetVertex (2), csVector2 (0.25, 0.25));

  // front border
  // border top
  p = thing_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->GetFlags ().Reset (CS_POLY_COLLDET);
  p->CreateVertex (csVector3 (-dx+border, dy, -dz));
  p->CreateVertex (csVector3 (dx-border, dy, -dz));
  p->CreateVertex (csVector3 (dx-border, dy-border, -dz));
  p->CreateVertex (csVector3 (-dx+border, dy-border, -dz));
  p->SetTextureSpace (
      p->GetVertex (0), csVector2 (0.125, 0.125),
      p->GetVertex (1), csVector2 (0.875, 0.125),
      p->GetVertex (2), csVector2 (0.875, 0.0));
  // border right
  p = thing_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->GetFlags ().Reset (CS_POLY_COLLDET);
  p->CreateVertex (csVector3 (dx-border, dy-border, -dz));
  p->CreateVertex (csVector3 (dx, dy-border, -dz));
  p->CreateVertex (csVector3 (dx, border, -dz));
  p->CreateVertex (csVector3 (dx-border, border, -dz));
  p->SetTextureSpace (
      p->GetVertex (0), csVector2 (1.0, 0.125),
      p->GetVertex (1), csVector2 (0.875, 0.125),
      p->GetVertex (2), csVector2 (0.875, 0.875));
  // border bottom
  p = thing_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->GetFlags ().Reset (CS_POLY_COLLDET);
  p->CreateVertex (csVector3 (-dx+border, border, -dz));
  p->CreateVertex (csVector3 (+dx-border, border, -dz));
  p->CreateVertex (csVector3 (+dx-border, 0.0, -dz));
  p->CreateVertex (csVector3 (-dx+border, 0.0, -dz));
  p->SetTextureSpace (
      p->GetVertex (0), csVector2 (0.125, 1.0),
      p->GetVertex (1), csVector2 (0.875, 1.0),
      p->GetVertex (2), csVector2 (0.875, 0.875));
  // border left
  p = thing_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->GetFlags ().Reset (CS_POLY_COLLDET);
  p->CreateVertex (csVector3 (-dx, dy-border, -dz));
  p->CreateVertex (csVector3 (-dx+border, dy-border, -dz));
  p->CreateVertex (csVector3 (-dx+border, border, -dz));
  p->CreateVertex (csVector3 (-dx, border, -dz));
  p->SetTextureSpace (
      p->GetVertex (0), csVector2 (0.125, 0.125),
      p->GetVertex (1), csVector2 (0.0, 0.125),
      p->GetVertex (2), csVector2 (0.0, 0.875));
  // border topleft
  p = thing_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->GetFlags ().Reset (CS_POLY_COLLDET);
  p->CreateVertex (csVector3 (-dx, dy, -dz));
  p->CreateVertex (csVector3 (-dx+border, dy, -dz));
  p->CreateVertex (csVector3 (-dx+border, dy-border, -dz));
  p->CreateVertex (csVector3 (-dx, dy-border, -dz));
  p->SetTextureSpace (
      p->GetVertex (0), csVector2 (0.125, 0.125),
      p->GetVertex (1), csVector2 (0.0, 0.125),
      p->GetVertex (2), csVector2 (0.0, 0.0));
  // border topright
  p = thing_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->GetFlags ().Reset (CS_POLY_COLLDET);
  p->CreateVertex (csVector3 (dx-border, dy, -dz));
  p->CreateVertex (csVector3 (dx, dy, -dz));
  p->CreateVertex (csVector3 (dx, dy-border, -dz));
  p->CreateVertex (csVector3 (dx-border, dy-border, -dz));
  p->SetTextureSpace (
      p->GetVertex (0), csVector2 (1.0, 0.125),
      p->GetVertex (1), csVector2 (0.875, 0.125),
      p->GetVertex (2), csVector2 (0.875, 0.0));
  // border botright
  p = thing_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->GetFlags ().Reset (CS_POLY_COLLDET);
  p->CreateVertex (csVector3 (dx-border, border, -dz));
  p->CreateVertex (csVector3 (dx, border, -dz));
  p->CreateVertex (csVector3 (dx, 0.0, -dz));
  p->CreateVertex (csVector3 (dx-border, 0.0, -dz));
  p->SetTextureSpace (
      p->GetVertex (0), csVector2 (1.0, 1.0),
      p->GetVertex (1), csVector2 (0.875, 1.0),
      p->GetVertex (2), csVector2 (0.875, 0.875));
  // border botleft
  p = thing_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->GetFlags ().Reset (CS_POLY_COLLDET);
  p->CreateVertex (csVector3 (-dx, border, -dz));
  p->CreateVertex (csVector3 (-dx+border, border, -dz));
  p->CreateVertex (csVector3 (-dx+border, 0.0, -dz));
  p->CreateVertex (csVector3 (-dx, 0.0, -dz));
  p->SetTextureSpace (
      p->GetVertex (0), csVector2 (0.125, 1.0),
      p->GetVertex (1), csVector2 (0.0, 1.0),
      p->GetVertex (2), csVector2 (0.0, 0.875));

  // front - the portal
  p = thing_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->GetFlags ().Reset (CS_POLY_COLLDET);
  // old
  //p->AddVertex (dx, 0, -dz);
  //p->AddVertex (-dx, 0, -dz);
  //p->AddVertex (-dx, dy, -dz);
  //p->AddVertex (dx, dy, -dz);
  p->CreateVertex (csVector3 (dx-border, border, -dz));
  p->CreateVertex (csVector3 (-dx+border, border, -dz));
  p->CreateVertex (csVector3 (-dx+border, dy-border, -dz));
  p->CreateVertex (csVector3 (dx-border, dy-border, -dz));
  p->SetTextureSpace (
      p->GetVertex (0), csVector2 (0, 0),
      p->GetVertex (1), csVector2 (1, 0),
      p->GetVertex (2), csVector2 (1, 1));
  portalPoly = p;
  thing_state->DecRef ();

  iLightingInfo* linfo = SCF_QUERY_INTERFACE (thing->GetMeshObject (),
    iLightingInfo);
  linfo->InitializeDefault ();
  room->ShineLights (thing);
  linfo->PrepareLighting ();
  linfo->DecRef ();

  return thing;
}

void OpenPortal (iLoader *LevelLoader, iView* view, char* lev)
{
  iSector* room = view->GetCamera ()->GetSector ();
  csVector3 pos = view->GetCamera ()->GetTransform ().This2Other (csVector3 (0, 0, 1));
  iMaterialWrapper* tm = Sys->Engine->FindMaterial ("portal");

  iPolygon3D* portalPoly;
  iMeshWrapper* thing = CreatePortalThing ("portalTo", room, tm, portalPoly);

  bool regionExists = (Sys->Engine->FindRegion(lev) != NULL);
  Sys->Engine->SelectRegion (lev);
  // If the region did not already exist then we load the level in it.
  if (!regionExists)
  {
    // @@@ No error checking!
    char buf[255];
    sprintf (buf, "/lev/%s", lev);
    Sys->myVFS->ChDir (buf);
    LevelLoader->LoadMapFile ("world", false);
    Sys->Engine->GetCurrentRegion ()->Prepare ();
  }

  thing->GetMovable ()->SetPosition (pos + csVector3 (0, Sys->cfg_legs_offset, 0));
  thing->GetMovable ()->Transform (view->GetCamera ()->GetTransform ().GetT2O ());
  thing->GetMovable ()->UpdateMove ();

  // First make a portal to the new level.
  iRegion* cur_region = Sys->Engine->GetCurrentRegion ();
  iCameraPosition* cp = cur_region->FindCameraPosition ("Start");
  const char* room_name;
  csVector3 topos;
  if (cp) { room_name = cp->GetSector (); topos = cp->GetPosition (); }
  else { room_name = "room"; topos.Set (0, 0, 0); }
  topos.y -= Sys->cfg_eye_offset;
  iSector* start_sector = Sys->Engine->GetCurrentRegion ()->FindSector (room_name);
  if (start_sector)
  {
    iPortal* portal = portalPoly->CreatePortal (start_sector);
    portal->GetFlags ().Set (CS_PORTAL_ZFILL);
    portal->GetFlags ().Set (CS_PORTAL_CLIPDEST);
    portal->SetWarp (view->GetCamera ()->GetTransform ().GetT2O (), topos, pos);

    if (!regionExists)
    {
      // Only if the region did not already exist do we create a portal
      // back. So even if multiple portals go to the region we only have
      // one portal back.
      iPolygon3D* portalPolyBack;
      iMeshWrapper* thingBack = CreatePortalThing ("portalFrom",
	  	start_sector, tm, portalPolyBack);
      thingBack->GetMovable ()->SetPosition (topos + csVector3 (0, Sys->cfg_legs_offset, -.1));
      thingBack->GetMovable ()->Transform (csYRotMatrix3 (M_PI));//view->GetCamera ()->GetW2C ());
      thingBack->GetMovable ()->UpdateMove ();
      iSector* iroom = SCF_QUERY_INTERFACE (room, iSector);
      iPortal* portalBack = portalPolyBack->CreatePortal (iroom);
      iroom->DecRef ();
      portalBack->GetFlags ().Set (CS_PORTAL_ZFILL);
      portalBack->GetFlags ().Set (CS_PORTAL_CLIPDEST);
      portalBack->SetWarp (view->GetCamera ()->GetTransform ().GetO2T (), pos, topos);
    }
  }

  if (!regionExists)
    Sys->InitCollDet (Sys->Engine, Sys->Engine->GetCurrentRegion ());
  Sys->Engine->SelectRegion ((iRegion*)NULL);
}

