/*
    Copyright (C) 1998 by Jorrit Tyberghein

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
#include "qint.h"
#include "cssys/system.h"
#include "walktest/walktest.h"
#include "walktest/bot.h"
#include "walktest/infmaze.h"
#include "walktest/hugeroom.h"
#include "apps/support/command.h"
#include "cstools/simpcons.h"
#include "csengine/camera.h"
#include "csengine/world.h"
#include "csengine/csview.h"
#include "csengine/wirefrm.h"
#include "csengine/cssprite.h"
#include "csengine/skeleton.h"
#include "csengine/triangle.h"
#include "csengine/polygon.h"
#include "csengine/light.h"
#include "csengine/sector.h"
#include "csengine/csspr2d.h"
#include "csengine/cdobj.h"
#include "csengine/collider.h"
#include "csutil/scanstr.h"
#include "csparser/impexp.h"
#include "csobject/dataobj.h"
#include "cssfxldr/common/snddata.h"
#include "csparser/snddatao.h"
#include "csparser/csloader.h"
#include "csparser/crossbld.h"
#include "csscript/csscript.h"
#include "csgeom/math3d.h"
#include "isndsrc.h"
#include "isndlstn.h"
#include "isndbuf.h"
#include "isndrdr.h"
#include "igraph3d.h"

csKeyMap* mapping = NULL;

/// Save/load camera functions
void SaveCamera (const char *fName)
{
  csCamera *c = Sys->view->GetCamera ();
  FILE *f = fopen (fName, "w");
  if (!f)
    return;
  const csMatrix3& m_o2t = c->GetO2T ();
  const csVector3& v_o2t = c->GetOrigin ();
  fprintf (f, "%f %f %f\n", v_o2t.x, v_o2t.y, v_o2t.z);
  fprintf (f, "%f %f %f\n", m_o2t.m11, m_o2t.m12, m_o2t.m13);
  fprintf (f, "%f %f %f\n", m_o2t.m21, m_o2t.m22, m_o2t.m23);
  fprintf (f, "%f %f %f\n", m_o2t.m31, m_o2t.m32, m_o2t.m33);
  fprintf (f, "%s\n", c->GetSector ()->GetName ());
  fprintf (f, "%d\n", c->IsMirrored ());
  fprintf (f, "%f %f %f\n", Sys->angle.x, Sys->angle.y, Sys->angle.z);
  fclose (f);
}

bool LoadCamera (const char *fName)
{
  char buf[100];
  FILE *f = fopen (fName, "r");
  if (!f)
  {
    CsPrintf (MSG_FATAL_ERROR, "Could not open coordinate file 'coord'!\n");
    return false;
  }
  csMatrix3 m;
  csVector3 v;
  csSector* s;
  int imirror;

  fscanf (f, "%f %f %f\n", &v.x, &v.y, &v.z);
  fscanf (f, "%f %f %f\n", &m.m11, &m.m12, &m.m13);
  fscanf (f, "%f %f %f\n", &m.m21, &m.m22, &m.m23);
  fscanf (f, "%f %f %f\n", &m.m31, &m.m32, &m.m33);
  fscanf (f, "%s\n", buf);
  s = (csSector*)Sys->world->sectors.FindByName (buf);
  if (!s)
  {
    fclose (f);
    CsPrintf (MSG_FATAL_ERROR, "Sector in coordinate file does not exist in this world!\n");
    return false;
  }
  imirror = false; fscanf (f, "%d\n", &imirror);

  // Load head angle
  fscanf (f, "%f %f %f", &Sys->angle.x, &Sys->angle.y, &Sys->angle.z);

  fclose (f);

  csCamera *c = Sys->view->GetCamera ();
  c->SetSector (s);
  c->SetMirrored ((bool)imirror);
  c->SetO2T (m);
  c->SetOrigin (v);
  return true;
}

csSprite3D *FindNextClosestSprite(csSprite3D *baseSprite, csCamera *camera, csVector2 *screenCoord);

//===========================================================================
// Some utility functions used throughout this source.
//===========================================================================

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

void move_sprite (csSprite3D* sprite, csSector* where, csVector3 const& pos)
{
  sprite->SetMove (pos);
  sprite->MoveToSector (where);
}

// Load a sprite from a general format (3DS, MD2, ...)
// This creates a sprite template which you can then add using add_sprite ().
void load_sprite (char *filename, char *templatename, char* txtname)
{
  // First check if the texture exists.
  if (!Sys->view->GetWorld ()->GetTextures ()->GetTextureMM (txtname))
  {
    Sys->Printf (MSG_CONSOLE, "Couldn't find texture '%s' in memory!\n", txtname);
    return;
  }

  // read in the model file
  CHK (converter * filedata = new converter);
  if (filedata->ivcon (filename, true, false, NULL, Sys->VFS) == ERROR)
  {
    Sys->Printf (MSG_CONSOLE, "There was an error reading the data!\n");
    CHK (delete filedata);
    return;
  }

  // convert data from the 'filedata' structure into a CS sprite template
  csCrossBuild_SpriteTemplateFactory builder;
  csSpriteTemplate *result = (csSpriteTemplate *)builder.CrossBuild (*filedata);
  CHK (delete filedata);

  // add this sprite to the world
  result->SetName (templatename);
  result->SetTexture (Sys->view->GetWorld ()->GetTextures (), txtname);

  Sys->view->GetWorld ()->sprite_templates.Push (result);
}

csSprite3D* add_sprite (char* tname, char* sname, csSector* where, csVector3 const& pos, float size)
{
  csSpriteTemplate* tmpl = Sys->view->GetWorld ()->GetSpriteTemplate (tname);
  if (!tmpl)
  {
    Sys->Printf (MSG_CONSOLE, "Unknown sprite template '%s'!\n", tname);
    return NULL;
  }
  csSprite3D* spr = tmpl->NewSprite ();
  spr->SetName (sname);
  Sys->view->GetWorld ()->sprites.Push (spr);
  spr->MoveToSector (where);
  spr->SetMove (pos);
  csMatrix3 m; m.Identity (); m = m * size;
  spr->SetTransform (m);

  spr->DeferUpdateLighting (CS_NLIGHT_STATIC|CS_NLIGHT_DYNAMIC, 10);
  return spr;
}

//===========================================================================
// Everything for skeletal tree demo.
//===========================================================================

// Recursive function to add limbs to a skeletal tree. This also builds
// the sprite template.
void add_tree_limbs (csSpriteTemplate* tmpl, csFrame* frame, csSkeletonLimb* parent, int& vertex_idx,
	int prev_par_idx, int maxdepth, int width, int recursion)
{
  int par_vertex_idx = vertex_idx;
  parent->AddVertex (vertex_idx++);
  parent->AddVertex (vertex_idx++);
  parent->AddVertex (vertex_idx++);
  parent->AddVertex (vertex_idx++);
  parent->AddVertex (vertex_idx++);
  parent->AddVertex (vertex_idx++);
  if (tmpl->GetNumVertices ()+6 >= frame->GetMaxVertices ())
  {
    int more = 6;
    tmpl->SetNumVertices (tmpl->GetNumVertices ()+more);
    frame->AddVertex (more);
  }
  frame->SetVertex (par_vertex_idx+0, -.05, 0, -.05); frame->SetTexel (par_vertex_idx+0, 0, 0);
  frame->SetVertex (par_vertex_idx+1, .05, 0, -.05); frame->SetTexel (par_vertex_idx+1, .99, 0);
  frame->SetVertex (par_vertex_idx+2, 0, 0, .05); frame->SetTexel (par_vertex_idx+2, 0, .99);
  frame->SetVertex (par_vertex_idx+3, -.05, .45, -.05); frame->SetTexel (par_vertex_idx+3, .99, .99);
  frame->SetVertex (par_vertex_idx+4, .05, .45, -.05); frame->SetTexel (par_vertex_idx+4, .5, .5);
  frame->SetVertex (par_vertex_idx+5, 0, .45, .05); frame->SetTexel (par_vertex_idx+5, .5, 0);
  if (recursion > 0)
  {
    // Create connection triangles with previous set
    tmpl->GetBaseMesh ()->AddTriangle (prev_par_idx+3, prev_par_idx+5, par_vertex_idx+0);
    tmpl->GetBaseMesh ()->AddTriangle (prev_par_idx+5, par_vertex_idx+2, par_vertex_idx+0);
    tmpl->GetBaseMesh ()->AddTriangle (prev_par_idx+4, par_vertex_idx+1, par_vertex_idx+2);
    tmpl->GetBaseMesh ()->AddTriangle (prev_par_idx+5, prev_par_idx+4, par_vertex_idx+2);
    tmpl->GetBaseMesh ()->AddTriangle (prev_par_idx+4, par_vertex_idx+0, par_vertex_idx+1);
    tmpl->GetBaseMesh ()->AddTriangle (prev_par_idx+4, prev_par_idx+3, par_vertex_idx+0);
  }
  // Create base triangles
  tmpl->GetBaseMesh ()->AddTriangle (par_vertex_idx+0, par_vertex_idx+5, par_vertex_idx+3);
  tmpl->GetBaseMesh ()->AddTriangle (par_vertex_idx+0, par_vertex_idx+2, par_vertex_idx+5);
  tmpl->GetBaseMesh ()->AddTriangle (par_vertex_idx+2, par_vertex_idx+4, par_vertex_idx+5);
  tmpl->GetBaseMesh ()->AddTriangle (par_vertex_idx+2, par_vertex_idx+1, par_vertex_idx+4);
  tmpl->GetBaseMesh ()->AddTriangle (par_vertex_idx+1, par_vertex_idx+3, par_vertex_idx+4);
  tmpl->GetBaseMesh ()->AddTriangle (par_vertex_idx+1, par_vertex_idx+0, par_vertex_idx+3);

  if (recursion >= maxdepth) return;
  csSkeletonConnection* con;
  int i;
  int rwidth;
  if (width < 0)
    rwidth = 1 + ((rand () >> 3) % (-width));
  else rwidth = width;

  for (i = 0 ; i < rwidth ; i++)
  {
    CHK (con = new csSkeletonConnection ());
    parent->AddChild (con);
    csMatrix3 tr = csYRotMatrix3 (0) * csZRotMatrix3(.15) *
                                                 csXRotMatrix3(.15);
    csTransform trans (tr, -tr.GetInverse () * csVector3 (0, .5, 0));
    con->SetTransformation (trans);
    add_tree_limbs (tmpl, frame, con, vertex_idx, par_vertex_idx, maxdepth, width, recursion+1);
  }
}

// Create a skeletal tree.
csSkeleton* create_skeltree (csSpriteTemplate* tmpl, csFrame* frame, int& vertex_idx,
	int maxdepth, int width)
{
  CHK (csSkeleton* skel = new csSkeleton ());
  add_tree_limbs (tmpl, frame, skel, vertex_idx, 0, maxdepth, width, 0);
  return skel;
}

// Object added to every skeletal tree node to keep the animation
// information.
class TreeSkelSpriteInfo : public csObject
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
  CSOBJTYPE;
};

IMPLEMENT_CSOBJTYPE (TreeSkelSpriteInfo, csObject);

// Animate a skeleton.
void animate_skeleton_tree (csSkeletonLimbState* limb)
{
  csSkeletonConnectionState* con = (csSkeletonConnectionState*)limb->GetChildren ();
  while (con)
  {
    TreeSkelSpriteInfo* o = (TreeSkelSpriteInfo*)con->GetChild (TreeSkelSpriteInfo::Type);
    if (!o)
    {
      CHK (o = new TreeSkelSpriteInfo ());
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
      con->ObjAdd (o);
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
    con->SetTransformation (trans);
    animate_skeleton_tree (con);
    con = (csSkeletonConnectionState*)(con->GetNext ());
  }
}

void animate_skeleton_tree_cb (csSprite3D* spr, csRenderView* /*rview*/)
{
  csSkeletonState* sk_state = spr->GetSkeletonState ();
  animate_skeleton_tree (sk_state);
}

// Add a skeletal tree sprite. If needed it will also create
// the template for this.
void add_skeleton_tree (csSector* where, csVector3 const& pos, int depth, int width)
{
  char skelname[50];
  sprintf (skelname, "__skeltree__%d,%d\n", depth, width);
  csSpriteTemplate* tmpl = Sys->view->GetWorld ()->GetSpriteTemplate (skelname);
  if (!tmpl)
  {
    CHK (tmpl = new csSpriteTemplate ());
    tmpl->SetName (skelname);
    Sys->world->sprite_templates.Push (tmpl);
    tmpl->SetTexture (Sys->world->GetTextures (), "white.gif");
    int vertex_idx = 0;
    csFrame* fr = tmpl->AddFrame ();
    fr->SetName ("f");
    csSpriteAction* act = tmpl->AddAction ();
    act->SetName ("a");
    act->AddFrame (fr, 100);
    tmpl->SetSkeleton (create_skeltree (tmpl, fr, vertex_idx, depth, width));
    tmpl->GenerateLOD ();
    tmpl->ComputeBoundingBox ();
  }
  csSprite3D* spr = add_sprite (skelname, "__skeltree__", where, pos-csVector3 (0, Sys->cfg_body_height, 0), 1);
  spr->SetDrawCallback (animate_skeleton_tree_cb);
}

//===========================================================================
// Everything for skeletal ghost demo.
//===========================================================================

// Object added to every skeletal tree node to keep the animation
// information.
class GhostSkelSpriteInfo : public csObject
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
  CSOBJTYPE;
};

IMPLEMENT_CSOBJTYPE (GhostSkelSpriteInfo, csObject);

// Object added to the ghost sprite itself to hold some information
// about movement.
class GhostSpriteInfo : public csObject
{
public:
  float dir;
  CSOBJTYPE;
};

IMPLEMENT_CSOBJTYPE (GhostSpriteInfo, csObject);

// Recursive function to add limbs to a skeletal ghost. This also builds
// the sprite template.
void add_ghost_limbs (csSpriteTemplate* tmpl, csFrame* frame, csSkeletonLimb* parent, int& vertex_idx,
	int prev_par_idx, int maxdepth, int width, int recursion, float dim)
{
  int par_vertex_idx = vertex_idx;
  parent->AddVertex (vertex_idx++);
  parent->AddVertex (vertex_idx++);
  parent->AddVertex (vertex_idx++);
  parent->AddVertex (vertex_idx++);
  parent->AddVertex (vertex_idx++);
  parent->AddVertex (vertex_idx++);
  if (tmpl->GetNumVertices ()+6 >= frame->GetMaxVertices ())
  {
    int more = 6;
    tmpl->SetNumVertices (tmpl->GetNumVertices ()+more);
    frame->AddVertex (more);
  }
  frame->SetVertex (par_vertex_idx+0, -dim, 0, -dim); frame->SetTexel (par_vertex_idx+0, 0, 0);
  frame->SetVertex (par_vertex_idx+1, dim, 0, -dim); frame->SetTexel (par_vertex_idx+1, .99, 0);
  frame->SetVertex (par_vertex_idx+2, 0, 0, dim); frame->SetTexel (par_vertex_idx+2, 0, .99);
  frame->SetVertex (par_vertex_idx+3, -dim, .45, -dim); frame->SetTexel (par_vertex_idx+3, .99, .99);
  frame->SetVertex (par_vertex_idx+4, dim, .45, -dim); frame->SetTexel (par_vertex_idx+4, .5, .5);
  frame->SetVertex (par_vertex_idx+5, 0, .45, dim); frame->SetTexel (par_vertex_idx+5, .5, 0);
  if (recursion > 0)
  {
    // Create connection triangles with previous set
    tmpl->GetBaseMesh ()->AddTriangle (prev_par_idx+3, prev_par_idx+5, par_vertex_idx+0);
    tmpl->GetBaseMesh ()->AddTriangle (prev_par_idx+5, par_vertex_idx+2, par_vertex_idx+0);
    tmpl->GetBaseMesh ()->AddTriangle (prev_par_idx+4, par_vertex_idx+1, par_vertex_idx+2);
    tmpl->GetBaseMesh ()->AddTriangle (prev_par_idx+5, prev_par_idx+4, par_vertex_idx+2);
    tmpl->GetBaseMesh ()->AddTriangle (prev_par_idx+4, par_vertex_idx+0, par_vertex_idx+1);
    tmpl->GetBaseMesh ()->AddTriangle (prev_par_idx+4, prev_par_idx+3, par_vertex_idx+0);
  }
  // Create base triangles
  tmpl->GetBaseMesh ()->AddTriangle (par_vertex_idx+0, par_vertex_idx+5, par_vertex_idx+3);
  tmpl->GetBaseMesh ()->AddTriangle (par_vertex_idx+0, par_vertex_idx+2, par_vertex_idx+5);
  tmpl->GetBaseMesh ()->AddTriangle (par_vertex_idx+2, par_vertex_idx+4, par_vertex_idx+5);
  tmpl->GetBaseMesh ()->AddTriangle (par_vertex_idx+2, par_vertex_idx+1, par_vertex_idx+4);
  tmpl->GetBaseMesh ()->AddTriangle (par_vertex_idx+1, par_vertex_idx+3, par_vertex_idx+4);
  tmpl->GetBaseMesh ()->AddTriangle (par_vertex_idx+1, par_vertex_idx+0, par_vertex_idx+3);

  if (recursion >= maxdepth) return;
  csSkeletonConnection* con;
  int i;
  for (i = 0 ; i < width ; i++)
  {
    CHK (con = new csSkeletonConnection ());
    parent->AddChild (con);
    csMatrix3 tr = csYRotMatrix3 (0) *
    	csZRotMatrix3 (.15) *
	csXRotMatrix3 (.15);
    csTransform trans (tr, -tr.GetInverse () * csVector3 (0, .5, 0));
    con->SetTransformation (trans);
    add_ghost_limbs (tmpl, frame, con, vertex_idx, par_vertex_idx, maxdepth, 1, recursion+1, dim * .7);
  }
}

// Create a skeletal ghost.
csSkeleton* create_skelghost (csSpriteTemplate* tmpl, csFrame* frame, int& vertex_idx,
	int maxdepth, int width)
{
  CHK (csSkeleton* skel = new csSkeleton ());
  add_ghost_limbs (tmpl, frame, skel, vertex_idx, 0, maxdepth, width, 0, .2);
  return skel;
}

// Animate a skeleton.
void animate_skeleton_ghost (csSkeletonLimbState* limb)
{
  csSkeletonConnectionState* con = (csSkeletonConnectionState*)limb->GetChildren ();
  while (con)
  {
    GhostSkelSpriteInfo* o = (GhostSkelSpriteInfo*)con->GetChild (GhostSkelSpriteInfo::Type);
    if (!o)
    {
      CHK (o = new GhostSkelSpriteInfo ());
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
      con->ObjAdd (o);
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
    con->SetTransformation (trans);
    animate_skeleton_ghost (con);
    con = (csSkeletonConnectionState*)(con->GetNext ());
  }
}

void animate_skeleton_ghost_cb (csSprite3D* spr, csRenderView* /*rview*/)
{
  csSkeletonState* sk_state = spr->GetSkeletonState ();
  animate_skeleton_ghost (sk_state);
}


// Add a skeletal ghost sprite. If needed it will also create
// the template for this.
void add_skeleton_ghost (csSector* where, csVector3 const& pos, int maxdepth, int width)
{
  char skelname[50];
  sprintf (skelname, "__skelghost__\n");
  csSpriteTemplate* tmpl = Sys->view->GetWorld ()->GetSpriteTemplate (skelname);
  if (!tmpl)
  {
    CHK (tmpl = new csSpriteTemplate ());
    tmpl->SetName (skelname);
    Sys->world->sprite_templates.Push (tmpl);
    tmpl->SetTexture (Sys->world->GetTextures (), "green.gif");
    int vertex_idx = 0;
    csFrame* fr = tmpl->AddFrame ();
    fr->SetName ("f");
    csSpriteAction* act = tmpl->AddAction ();
    act->SetName ("a");
    act->AddFrame (fr, 100);
    tmpl->SetSkeleton (create_skelghost (tmpl, fr, vertex_idx, maxdepth, width));
    tmpl->GenerateLOD ();
    tmpl->ComputeBoundingBox ();
  }
  csSprite3D* spr = add_sprite (skelname, "__skelghost__", where, pos, 1);
  spr->SetMixmode (CS_FX_SETALPHA (0.75));
  CHK (csCollider* col = new csCollider (spr));
  csColliderPointerObject::SetCollider (*spr, col, true);
  CHK (GhostSpriteInfo* gh_info = new GhostSpriteInfo ());
  spr->ObjAdd (gh_info);
  gh_info->dir = 1;
  spr->SetDrawCallback (animate_skeleton_ghost_cb);
}

#define MAXSECTORSOCCUPIED  20

extern int FindSectors (csVector3 v, csVector3 d, csSector *s, csSector **sa);
extern int CollisionDetect (csCollider *c, csSector* sp, csTransform *cdt);
extern collision_pair our_cd_contact[1000];//=0;
extern int num_our_cd;

void move_ghost (csSprite3D* spr)
{
  csCollider* col = csColliderPointerObject::GetCollider (*spr);
  csSector* first_sector = (csSector*)(spr->sectors[0]);

  // Create a transformation 'test' which indicates where the ghost is moving too.
  const csVector3& pos = spr->GetW2TTranslation ();
  csVector3 vel (0, 0, .1);
  vel = spr->GetW2T () * vel;
  csVector3 new_pos = pos+vel;
  csMatrix3 m;
  csOrthoTransform test (m, new_pos);

  // Find all sectors that the ghost will occupy on the new position.
  csSector *n[MAXSECTORSOCCUPIED];
  int num_sectors = FindSectors (new_pos, 4*col->GetBbox()->d, first_sector, n);

  // Start collision detection.
  csCollider::CollideReset ();
  num_our_cd = 0;
  csCollider::firstHit = false;
  int hits = 0;
  for ( ; num_sectors-- ; )
    hits += CollisionDetect (col, n[num_sectors], &test);

  // Change our velocity according to the collisions.
  for (int j=0 ; j<hits ; j++)
  {
    CDTriangle *wall = our_cd_contact[j].tr2;
    csVector3 n = ((wall->p3-wall->p2)%(wall->p2-wall->p1)).Unit();
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
    spr->MoveToSector (first_sector);
    spr->SetMove (new_pos);
  }

  // Turn around at random intervals.
  GhostSpriteInfo* gh_info = (GhostSpriteInfo*)spr->GetChild (GhostSpriteInfo::Type);
  if (rand () % 40 == 1) gh_info->dir = -gh_info->dir;

  // OpenStep compiler bug prevents Transform(GetYRotation()), which is why
  // the expressions are split across two statements below.
  if (vel < 0.01)
  {
    // We did not move much. Turn around quickly.
    csMatrix3 m = csYRotMatrix3 (gh_info->dir*.2);
    spr->Transform (m);
  }
  else if (vel < 0.05)
  {
    // We did a bit. Turn around slightly.
    csMatrix3 m = csYRotMatrix3 (gh_info->dir*.1);
    spr->Transform (m);
  }
  else
  {
    csMatrix3 m = csYRotMatrix3 (gh_info->dir*.01);
    spr->Transform (m);
  }
}

//===========================================================================
// Everything for bots.
//===========================================================================

Bot* first_bot = NULL;
bool do_bots = false;

// Add a bot with some size at the specified positin.
void add_bot (float size, csSector* where, csVector3 const& pos, float dyn_radius)
{
  csDynLight* dyn = NULL;
  if (dyn_radius)
  {
    float r, g, b;
    RandomColor (r, g, b);
    //@@@ MEMORY LEAK?
    CHK (dyn = new csDynLight (pos.x, pos.y, pos.z, dyn_radius, r, g, b));
    Sys->view->GetWorld ()->AddDynLight (dyn);
    dyn->SetSector (where);
    dyn->Setup ();
  }
  csSpriteTemplate* tmpl = Sys->view->GetWorld ()->GetSpriteTemplate ("bot");
  if (!tmpl) return;
  Bot* bot;
  CHK (bot = new Bot (tmpl));
  bot->SetName ("bot");
  Sys->view->GetWorld ()->sprites.Push (bot);
  bot->MoveToSector (where);
  csMatrix3 m; m.Identity (); m = m * size;
  bot->SetTransform (m);
  bot->set_bot_move (pos);
  bot->set_bot_sector (where);
  bot->SetAction ("default");
  bot->InitSprite ();
  bot->next = first_bot;
  bot->light = dyn;
  first_bot = bot;
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
  csSprite3D* sprite;
  iSoundSource *snd;
};

struct ExplosionStruct
{
  int type;		// type == DYN_TYPE_EXPLOSION
  float radius;
  int dir;
  iSoundSource *snd;
};

struct RandomLight
{
  int type;		// type == DYN_TYPE_RANDOM
  float dyn_move_dir;
  float dyn_move;
  float dyn_r1, dyn_g1, dyn_b1;
};

void HandleDynLight (csDynLight* dyn)
{
  LightStruct* ls = (LightStruct*)(csDataObject::GetData(*dyn));
  switch (ls->type)
  {
    case DYN_TYPE_MISSILE:
    {
      MissileStruct* ms = (MissileStruct*)(csDataObject::GetData(*dyn));
      csVector3 v (0, 0, 2.5);
      csVector3 old = dyn->GetCenter ();
      v = old + ms->dir.GetT2O () * v;
      csSector* s = dyn->GetSector ();
      bool mirror = false;
      csVector3 old_v = v;
      s = s->FollowSegment (ms->dir, v, mirror);
      if (ABS (v.x-old_v.x) > SMALL_EPSILON || ABS (v.y-old_v.y) > SMALL_EPSILON && ABS (v.z-old_v.z) > SMALL_EPSILON)
      {
        v = old;
        if (ms->sprite)
      	{
          if ((rand () & 0x3) == 1)
	  {
	    int i;
	    if (do_bots)
	      for (i = 0 ; i < 40 ; i++)
            add_bot (1, dyn->GetSector (), dyn->GetCenter (), 0);
	  }
	  ms->sprite->RemoveFromSectors ();
	  Sys->view->GetWorld ()->RemoveSprite (ms->sprite);
	}
        dyn->ObjRemove(dyn->GetChild (csDataObject::Type));
        CHK (delete ms);
        CHK (ExplosionStruct* es = new ExplosionStruct);
        if (Sys->Sound)
          if ((es->snd = Sys->Sound->CreateSource (Sys->wMissile_boom)))
          {
            es->snd->SetPosition (v.x, v.y, v.z);
            iSoundBuffer *sb = es->snd->GetSoundBuffer();
            sb->Play ();
          }
        es->type = DYN_TYPE_EXPLOSION;
        es->radius = 2;
        es->dir = 1;
        CHK (csDataObject* esdata = new csDataObject (es));
        dyn->ObjAdd (esdata);
        return;
      }
      dyn->Move (s, v.x, v.y, v.z);
      dyn->Setup ();
      if (ms->sprite) move_sprite (ms->sprite, s, v);
      if (Sys->Sound && ms->snd) ms->snd->SetPosition (v.x, v.y, v.z);
      break;
    }
    case DYN_TYPE_EXPLOSION:
    {
      ExplosionStruct* es = (ExplosionStruct*)(csDataObject::GetData(*dyn));
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
          if (Sys->Sound && es->snd)
	  {
            iSoundBuffer *sb = es->snd->GetSoundBuffer();
            sb->Stop ();
            es->snd->DecRef ();
	  }
	  CHK (delete es);
          Sys->view->GetWorld ()->RemoveDynLight (dyn);
          CHK (delete dyn);
	  return;
	}
      }
      dyn->Resize (es->radius);
      dyn->Setup ();
      break;
    }
    case DYN_TYPE_RANDOM:
    {
      RandomLight* rl = (RandomLight*)(csDataObject::GetData(*dyn));
      rl->dyn_move += rl->dyn_move_dir;
      if (rl->dyn_move < 0 || rl->dyn_move > 2) rl->dyn_move_dir = -rl->dyn_move_dir;
      if (ABS (rl->dyn_r1-dyn->GetColor ().red) < .01 && ABS (rl->dyn_g1-dyn->GetColor ().green) < .01 && ABS (rl->dyn_b1-dyn->GetColor ().blue) < .01)
        RandomColor (rl->dyn_r1, rl->dyn_g1, rl->dyn_b1);
      else
        dyn->SetColor (csColor ((rl->dyn_r1+7.*dyn->GetColor ().red)/8., (rl->dyn_g1+7.*dyn->GetColor ().green)/8., (rl->dyn_b1+7.*dyn->GetColor ().blue)/8.));
      dyn->Move (dyn->GetSector (), dyn->GetCenter ().x, dyn->GetCenter ().y+rl->dyn_move_dir, dyn->GetCenter ().z);
      dyn->Setup ();
      break;
    }
  }
}

//===========================================================================
// Everything for key mapping and binding.
//===========================================================================

void map_key (char* keyname, csKeyMap* map)
{
  map->shift = 0;
  map->alt = 0;
  map->ctrl = 0;
  map->need_status = 0;
  char* dash = strchr (keyname, '-');
  while (dash)
  {
    *dash = 0;
    if (!strcmp (keyname, "shift")) map->shift = 1;
    else if (!strcmp (keyname, "alt")) map->alt = 1;
    else if (!strcmp (keyname, "ctrl")) map->ctrl = 1;
    else if (!strcmp (keyname, "status")) map->need_status = 1;
    else Sys->Printf (MSG_CONSOLE, "Bad modifier '%s'!\n", keyname);

    *dash = '-';
    keyname = dash+1;
    dash = strchr (dash+1, '-');
  }

  if (!strcmp (keyname, "tab")) map->key = CSKEY_TAB;
  else if (!strcmp (keyname, "space")) map->key = ' ';
  else if (!strcmp (keyname, "esc")) map->key = CSKEY_ESC;
  else if (!strcmp (keyname, "enter")) map->key = CSKEY_ENTER;
  else if (!strcmp (keyname, "bs")) map->key = CSKEY_BACKSPACE;
  else if (!strcmp (keyname, "up")) map->key = CSKEY_UP;
  else if (!strcmp (keyname, "down")) map->key = CSKEY_DOWN;
  else if (!strcmp (keyname, "right")) map->key = CSKEY_RIGHT;
  else if (!strcmp (keyname, "left")) map->key = CSKEY_LEFT;
  else if (!strcmp (keyname, "pgup")) map->key = CSKEY_PGUP;
  else if (!strcmp (keyname, "pgdn")) map->key = CSKEY_PGDN;
  else if (!strcmp (keyname, "home")) map->key = CSKEY_HOME;
  else if (!strcmp (keyname, "end")) map->key = CSKEY_END;
  else if (!strcmp (keyname, "ins")) map->key = CSKEY_INS;
  else if (!strcmp (keyname, "del")) map->key = CSKEY_DEL;
  else if (!strcmp (keyname, "f1")) map->key = CSKEY_F1;
  else if (!strcmp (keyname, "f2")) map->key = CSKEY_F2;
  else if (!strcmp (keyname, "f3")) map->key = CSKEY_F3;
  else if (!strcmp (keyname, "f4")) map->key = CSKEY_F4;
  else if (!strcmp (keyname, "f5")) map->key = CSKEY_F5;
  else if (!strcmp (keyname, "f6")) map->key = CSKEY_F6;
  else if (!strcmp (keyname, "f7")) map->key = CSKEY_F7;
  else if (!strcmp (keyname, "f8")) map->key = CSKEY_F8;
  else if (!strcmp (keyname, "f9")) map->key = CSKEY_F9;
  else if (!strcmp (keyname, "f10")) map->key = CSKEY_F10;
  else if (!strcmp (keyname, "f11")) map->key = CSKEY_F11;
  else if (!strcmp (keyname, "f12")) map->key = CSKEY_F12;
  else if (*(keyname+1) != 0) Sys->Printf (MSG_CONSOLE, "Bad key '%s'!\n", keyname);
  else if ((*keyname >= 'A' && *keyname <= 'Z') || strchr ("!@#$%^&*()_+", *keyname))
  {
    map->shift = 1;
    map->key = *keyname;
  }
  else if (*keyname >= 'a' && *keyname <= 'z')
  {
    if (map->shift) map->key = (*keyname)+'A'-'a';
    else map->key = *keyname;
  }
  else map->key = *keyname;
}

char* keyname (csKeyMap* map)
{
  static char buf[100];
  buf[0] = 0;
  if (map->need_status) strcat (buf, "status-");
  if (map->shift) strcat (buf, "shift-");
  if (map->ctrl) strcat (buf, "ctrl-");
  if (map->alt) strcat (buf, "alt-");
  switch (map->key)
  {
    case CSKEY_TAB: strcat (buf, "tab"); break;
    case ' ': strcat (buf, "space"); break;
    case CSKEY_ESC: strcat (buf, "esc"); break;
    case CSKEY_ENTER: strcat (buf, "enter"); break;
    case CSKEY_BACKSPACE: strcat (buf, "bs"); break;
    case CSKEY_UP: strcat (buf, "up"); break;
    case CSKEY_DOWN: strcat (buf, "down"); break;
    case CSKEY_RIGHT: strcat (buf, "right"); break;
    case CSKEY_LEFT: strcat (buf, "left"); break;
    case CSKEY_PGUP: strcat (buf, "pgup"); break;
    case CSKEY_PGDN: strcat (buf, "pgdn"); break;
    case CSKEY_HOME: strcat (buf, "home"); break;
    case CSKEY_END: strcat (buf, "end"); break;
    case CSKEY_INS: strcat (buf, "ins"); break;
    case CSKEY_DEL: strcat (buf, "del"); break;
    case CSKEY_F1: strcat (buf, "f1"); break;
    case CSKEY_F2: strcat (buf, "f2"); break;
    case CSKEY_F3: strcat (buf, "f3"); break;
    case CSKEY_F4: strcat (buf, "f4"); break;
    case CSKEY_F5: strcat (buf, "f5"); break;
    case CSKEY_F6: strcat (buf, "f6"); break;
    case CSKEY_F7: strcat (buf, "f7"); break;
    case CSKEY_F8: strcat (buf, "f8"); break;
    case CSKEY_F9: strcat (buf, "f9"); break;
    case CSKEY_F10: strcat (buf, "f10"); break;
    case CSKEY_F11: strcat (buf, "f11"); break;
    case CSKEY_F12: strcat (buf, "f12"); break;
    default:
    {
      char* s = strchr (buf, 0);
      *s++ = map->key;
      *s = 0;
    }
  }
  return buf;
}

csKeyMap* find_mapping (char* keyname)
{
  csKeyMap map;
  map_key (keyname, &map);

  csKeyMap* m = mapping;
  while (m)
  {
    if (map.key == m->key && map.shift == m->shift && map.ctrl == m->ctrl && map.alt == m->alt
    	&& map.need_status == m->need_status)
      return m;
    m = m->next;
  }
  return NULL;
}

void bind_key (char* arg)
{
  if (!arg)
  {
    csKeyMap* map = mapping;
    while (map)
    {
      Sys->Printf (MSG_CONSOLE, "Key '%s' bound to '%s'.\n", keyname (map), map->cmd);
      map = map->next;
    }
    return;
  }
  char* space = strchr (arg, ' ');
  if (space)
  {
    *space = 0;
    csKeyMap* map = find_mapping (arg);
    if (map)
    {
      CHK (delete [] map->cmd);
    }
    else
    {
      CHK (map = new csKeyMap ());
      map->next = mapping;
      map->prev = NULL;
      if (mapping) mapping->prev = map;
      mapping = map;
      map_key (arg, map);
    }
    CHK (map->cmd = new char [strlen (space+1)+1]);
    strcpy (map->cmd, space+1);
    *space = ' ';
  }
  else
  {
    csKeyMap* map = find_mapping (arg);
    if (map) Sys->Printf (MSG_CONSOLE, "Key bound to '%s'!\n", map->cmd);
    else Sys->Printf (MSG_CONSOLE, "Key not bound!\n");
  }
}

void free_keymap ()
{
  csKeyMap *prev, *cur = mapping;
  while (cur)
  {
    prev = cur;
    cur = cur->next;
    CHK (delete [] prev->cmd);
    CHK (delete prev);
  }
  mapping = NULL;
}

//===========================================================================

// Light all sprites and animate the skeletal trees.
// This function does no effort at all to optimize stuff. It does
// not test if the sprite is visible or not.
void light_statics ()
{
  csWorld *w = Sys->view->GetWorld ();
  for (int i = 0 ; i < w->sprites.Length () ; i++)
  {
    csSprite3D *spr = (csSprite3D *)w->sprites [i];
    csSkeletonState* sk_state = spr->GetSkeletonState ();
    if (sk_state)
    {
      const char* name = spr->GetName ();
      //if (!strcmp (name, "__skeltree__")) animate_skeleton_tree (sk_state);
      if (!strcmp (name, "__skelghost__"))
      {
        //animate_skeleton_ghost (sk_state);
        move_ghost (spr);
      }
    }
    spr->DeferUpdateLighting (CS_NLIGHT_STATIC|CS_NLIGHT_DYNAMIC, 10);
  }
}

float safe_atof (char* arg)
{
  if (arg) return atof (arg);
  else return 1;
}

//--//--//--//--//--//--//--//--//--//--//-- Handle our additional commands --//
static bool CommandHandler (char *cmd, char *arg)
{
  if (!strcasecmp (cmd, "help"))
  {
    Command::perform (cmd, arg);
    Sys->Printf (MSG_CONSOLE, "-*- Additional commands -*-\n");
    Sys->Printf (MSG_CONSOLE, " coordsave, coordload, dumpvis\n");
    Sys->Printf (MSG_CONSOLE, " bind, fclear, addlight, dellight, dellights\n");
    Sys->Printf (MSG_CONSOLE, " picklight, droplight, colldet, stats, hi, frustrum\n");
    Sys->Printf (MSG_CONSOLE, " fps, perftest, capture, coordshow, zbuf, freelook\n");
    Sys->Printf (MSG_CONSOLE, " map, fire, debug0, debug1, debug2, edges, p_alpha, s_fog\n");
    Sys->Printf (MSG_CONSOLE, " snd_play, snd_volume, do_gravity, cbuffer, quadtree, covtree\n");
    Sys->Printf (MSG_CONSOLE, " addbot, delbot, loadsprite, addsprite, addskel, addghost\n");
    Sys->Printf (MSG_CONSOLE, " step_forward, step_backward, strafe_left, strafe_right\n");
    Sys->Printf (MSG_CONSOLE, " look_up, look_down, rotate_left, rotate_right, jump, move3d\n");
    Sys->Printf (MSG_CONSOLE, " i_forward, i_backward, i_left, i_right, i_up, i_down\n");
    Sys->Printf (MSG_CONSOLE, " i_rotleftc, i_rotleftw, i_rotrightc, i_rotrightw\n");
    Sys->Printf (MSG_CONSOLE, " i_rotleftx, i_rotleftz, i_rotrightx, i_rotrightz\n");
    Sys->Printf (MSG_CONSOLE, " clrlights, setlight, palette\n");
  }
  else if (!strcasecmp (cmd, "coordsave"))
  {
    Sys->Printf (MSG_CONSOLE, "SAVE COORDS\n");
    SaveCamera ("coord");
  }
  else if (!strcasecmp (cmd, "coordload"))
  {
    Sys->Printf (MSG_CONSOLE, "LOAD COORDS\n");
    LoadCamera ("coord");
  }
  else if (!strcasecmp (cmd, "dumpvis"))
  {
    extern int dump_visible_indent;
    dump_visible_indent = 0;
    Sys->Printf (MSG_DEBUG_0, "====================================================================\n");
    extern void dump_visible (csRenderView* rview, int type, void* entity);
    Sys->view->GetWorld ()->DrawFunc (Sys->view->GetCamera (), Sys->view->GetClipper (), dump_visible);
    Sys->Printf (MSG_DEBUG_0, "====================================================================\n");
  }
  else if (!strcasecmp (cmd, "bind"))
    bind_key (arg);
  else if (!strcasecmp (cmd, "fclear"))
    Command::change_boolean (arg, &Sys->do_clear, "fclear");
  else if (!strcasecmp (cmd, "fps"))
    Command::change_boolean (arg, &Sys->do_fps, "fps");
  else if (!strcasecmp (cmd, "edges"))
    Command::change_boolean (arg, &Sys->do_edges, "do_edges");
  else if (!strcasecmp (cmd, "do_gravity"))
    Command::change_boolean (arg, &Sys->do_gravity, "do_gravity");
  else if (!strcasecmp (cmd, "inverse_mouse"))
    Command::change_boolean (arg, &Sys->inverse_mouse, "inverse_mouse");
  else if (!strcasecmp (cmd, "colldet"))
    Command::change_boolean (arg, &Sys->do_cd, "colldet");
  else if (!strcasecmp (cmd, "frustrum"))
    Command::change_boolean (arg, &Sys->do_light_frust, "frustrum");
  else if (!strcasecmp (cmd, "zbuf"))
    Command::change_boolean (arg, &Sys->do_show_z, "zbuf");
  else if (!strcasecmp (cmd, "palette"))
    Command::change_boolean (arg, &Sys->do_show_palette, "palette");
  else if (!strcasecmp (cmd, "move3d"))
    Command::change_boolean (arg, &Sys->move_3d, "move3d");
  else if (!strcasecmp (cmd, "cbuffer"))
  {
    bool en = Sys->world->GetCBuffer () != NULL;
    Command::change_boolean (arg, &en, "cbuffer");
    Sys->world->EnableCBuffer (en);
  }
  else if (!strcasecmp (cmd, "quadtree"))
  {
    bool en = Sys->world->GetQuadtree () != NULL;
    Command::change_boolean (arg, &en, "quadtree");
    Sys->world->EnableQuadtree (en);
  }
  else if (!strcasecmp (cmd, "covtree"))
  {
    bool en = Sys->world->GetCovtree () != NULL;
    Command::change_boolean (arg, &en, "covtree");
    Sys->world->EnableCovtree (en);
  }
  else if (!strcasecmp (cmd, "freelook"))
  {
    Command::change_boolean (arg, &Sys->do_freelook, "freelook");
    if (Sys->do_freelook)
      System->G2D->SetMousePosition (FRAME_WIDTH / 2, FRAME_HEIGHT / 2);
  }
  else if (!strcasecmp (cmd, "stats"))
  {
    Command::change_boolean (arg, &Sys->do_stats, "stats");
    if (Sys->do_stats) Sys->do_show_coord = false;
  }
  else if (!strcasecmp (cmd, "coordshow"))
  {
    Command::change_boolean (arg, &Sys->do_show_coord, "coordshow");
    if (Sys->do_show_coord) Sys->do_stats = false;
  }
  else if (!strcasecmp (cmd, "hi"))
  {
    csPolygon3D* hi = arg ? Sys->view->GetCamera ()->GetSector ()->GetPolygon3D (arg) : (csPolygon3D*)NULL;
    if (hi) Sys->Printf (MSG_CONSOLE, "Hilighting polygon: '%s'\n", arg);
    else Sys->Printf (MSG_CONSOLE, "Disabled hilighting.\n");
    Sys->selected_polygon = hi;
  }
  else if (!strcasecmp (cmd, "p_alpha"))
  {
    csPolygon3D* hi = Sys->selected_polygon;
    if (hi)
    {
      if (hi->GetPortal ())
      {
        int a = hi->GetAlpha ();
        Command::change_int (arg, &a, "portal alpha", 0, 100);
	hi->SetAlpha (a);
      }
      else Sys->Printf (MSG_CONSOLE, "Only for portals!\n");
    }
    else Sys->Printf (MSG_CONSOLE, "No polygon selected!\n");
  }
  else if (!strcasecmp (cmd, "s_fog"))
  {
    csFog& f = Sys->view->GetCamera ()->GetSector ()->GetFog ();
    if (!arg)
    {
      Sys->Printf (MSG_CONSOLE, "Fog in current sector (%f,%f,%f) density=%f\n",
      	f.red, f.green, f.blue, f.density);
    }
    else
    {
      float r, g, b, dens;
      if (ScanStr (arg, "%f,%f,%f,%f", &r, &g, &b, &dens) != 4)
      {
        Sys->Printf (MSG_CONSOLE, "Expected r,g,b,density. Got something else!\n");
        return false;
      }
      f.enabled = true;
      f.density = dens;
      f.red = r;
      f.green = g;
      f.blue = b;
    }
  }
  else if (!strcasecmp (cmd, "capture"))
    CaptureScreen ();
  else if (!strcasecmp (cmd, "perftest"))
    perf_test ();
  else if (!strcasecmp (cmd, "debug0"))
  {
    Sys->Printf (MSG_CONSOLE, "No debug0 implementation in this version.\n");
  }
  else if (!strcasecmp (cmd, "debug1"))
  {
    extern int covtree_level;
    covtree_level++;
    if (covtree_level > 5) covtree_level = 1;
    printf ("covtree_level=%d\n", covtree_level);
    //Sys->Printf (MSG_CONSOLE, "No debug1 implementation in this version.\n");
  }
  else if (!strcasecmp (cmd, "debug2"))
  {
    extern bool do_covtree_dump;
    do_covtree_dump = !do_covtree_dump;
    //Sys->Printf (MSG_CONSOLE, "No debug2 implementation in this version.\n");
  }
  else if (!strcasecmp (cmd, "strafe_left"))
  {
    float f = safe_atof (arg);
    if (Sys->move_3d || Sys->map_mode) { if (f) Sys->imm_left (.1, false, false); }
    else Sys->strafe (-1*f,0);
  }
  else if (!strcasecmp (cmd, "strafe_right"))
  {
    float f = safe_atof (arg);
    if (Sys->move_3d || Sys->map_mode) { if (f) Sys->imm_right (.1, false, false); }
    else Sys->strafe (1*f,0);
  }
  else if (!strcasecmp (cmd, "step_forward"))
  {
    float f = safe_atof (arg);
    if (Sys->move_3d || Sys->map_mode) { if (f) Sys->imm_forward (.1, false, false); }
    else Sys->step (1*f,0);
  }
  else if (!strcasecmp (cmd, "step_backward"))
  {
    float f = safe_atof (arg);
    if (Sys->move_3d || Sys->map_mode) { if (f) Sys->imm_backward (.1, false, false); }
    else Sys->step (-1*f,0);
  }
  else if (!strcasecmp (cmd, "rotate_left"))
  {
    float f = safe_atof (arg);
    if (Sys->move_3d || Sys->map_mode) { if (f) Sys->imm_rot_left_camera (.1, false, false); }
    else Sys->rotate (-1*f,0);
  }
  else if (!strcasecmp (cmd, "rotate_right"))
  {
    float f = safe_atof (arg);
    if (Sys->move_3d || Sys->map_mode) { if (f) Sys->imm_rot_right_camera (.1, false, false); }
    else Sys->rotate (1*f,0);
  }
  else if (!strcasecmp (cmd, "look_up"))
  {
    float f = safe_atof (arg);
    if (Sys->move_3d || Sys->map_mode) { if (f) Sys->imm_rot_right_xaxis (.1, false, false); }
    else Sys->look (-1*f,0);
  }
  else if (!strcasecmp (cmd, "look_down"))
  {
    float f = safe_atof (arg);
    if (Sys->move_3d || Sys->map_mode) { if (f) Sys->imm_rot_left_xaxis (.1, false, false); }
    else Sys->look (1*f,0);
  }
  else if (!strcasecmp (cmd, "jump"))
  {
    if (Sys->do_gravity && Sys->on_ground)
      Sys->velocity.y = Sys->cfg_jumpspeed;
  }
  else if (!strcasecmp (cmd, "i_forward"))
  {
    int slow = 0, fast = 0; if (arg) ScanStr (arg, "%d,%d", &slow, &fast);
    Sys->imm_forward (.1, (bool)slow, (bool)fast);
  }
  else if (!strcasecmp (cmd, "i_backward"))
  {
    int slow = 0, fast = 0; if (arg) ScanStr (arg, "%d,%d", &slow, &fast);
    Sys->imm_backward (.1, (bool)slow, (bool)fast);
  }
  else if (!strcasecmp (cmd, "i_left"))
  {
    int slow = 0, fast = 0; if (arg) ScanStr (arg, "%d,%d", &slow, &fast);
    Sys->imm_left (.1, (bool)slow, (bool)fast);
  }
  else if (!strcasecmp (cmd, "i_right"))
  {
    int slow = 0, fast = 0; if (arg) ScanStr (arg, "%d,%d", &slow, &fast);
    Sys->imm_right (.1, (bool)slow, (bool)fast);
  }
  else if (!strcasecmp (cmd, "i_up"))
  {
    int slow = 0, fast = 0; if (arg) ScanStr (arg, "%d,%d", &slow, &fast);
    Sys->imm_up (.1, (bool)slow, (bool)fast);
  }
  else if (!strcasecmp (cmd, "i_down"))
  {
    int slow = 0, fast = 0; if (arg) ScanStr (arg, "%d,%d", &slow, &fast);
    Sys->imm_down (.1, (bool)slow, (bool)fast);
  }
  else if (!strcasecmp (cmd, "i_rotleftc"))
  {
    int slow = 0, fast = 0; if (arg) ScanStr (arg, "%d,%d", &slow, &fast);
    Sys->imm_rot_left_camera (.1, (bool)slow, (bool)fast);
  }
  else if (!strcasecmp (cmd, "i_rotleftw"))
  {
    int slow = 0, fast = 0; if (arg) ScanStr (arg, "%d,%d", &slow, &fast);
    Sys->imm_rot_left_world (.1, (bool)slow, (bool)fast);
  }
  else if (!strcasecmp (cmd, "i_rotrightc"))
  {
    int slow = 0, fast = 0; if (arg) ScanStr (arg, "%d,%d", &slow, &fast);
    Sys->imm_rot_right_camera (.1, (bool)slow, (bool)fast);
  }
  else if (!strcasecmp (cmd, "i_rotrightw"))
  {
    int slow = 0, fast = 0; if (arg) ScanStr (arg, "%d,%d", &slow, &fast);
    Sys->imm_rot_right_world (.1, (bool)slow, (bool)fast);
  }
  else if (!strcasecmp (cmd, "i_rotleftx"))
  {
    int slow = 0, fast = 0; if (arg) ScanStr (arg, "%d,%d", &slow, &fast);
    Sys->imm_rot_left_xaxis (.1, (bool)slow, (bool)fast);
  }
  else if (!strcasecmp (cmd, "i_rotleftz"))
  {
    int slow = 0, fast = 0; if (arg) ScanStr (arg, "%d,%d", &slow, &fast);
    Sys->imm_rot_left_zaxis (.1, (bool)slow, (bool)fast);
  }
  else if (!strcasecmp (cmd, "i_rotrightx"))
  {
    int slow = 0, fast = 0; if (arg) ScanStr (arg, "%d,%d", &slow, &fast);
    Sys->imm_rot_right_xaxis (.1, (bool)slow, (bool)fast);
  }
  else if (!strcasecmp (cmd, "i_rotrightz"))
  {
    int slow = 0, fast = 0; if (arg) ScanStr (arg, "%d,%d", &slow, &fast);
    Sys->imm_rot_right_zaxis (.1, (bool)slow, (bool)fast);
  }
  else if (!strcasecmp (cmd, "fire"))
  {
    csVector3 dir (0, 0, 0);
    csVector3 pos = Sys->view->GetCamera ()->Camera2World (dir);
    float r, g, b;
    RandomColor (r, g, b);
    CHK (csDynLight* dyn = new csDynLight (pos.x, pos.y, pos.z, 4, r, g, b));
    Sys->view->GetWorld ()->AddDynLight (dyn);
    dyn->SetSector (Sys->view->GetCamera ()->GetSector ());
    dyn->Setup ();
    CHK (MissileStruct* ms = new MissileStruct);
    if (Sys->Sound)
      if ((ms->snd = Sys->Sound->CreateSource (Sys->wMissile_whoosh)))
      {
        ms->snd->SetPosition (pos.x, pos.y, pos.z);
        iSoundBuffer *sb = ms->snd->GetSoundBuffer();
        sb->Play ();
      }
    ms->type = DYN_TYPE_MISSILE;
    ms->dir = (csOrthoTransform)*(Sys->view->GetCamera ());
    ms->sprite = NULL;
    CHK(csDataObject* msdata = new csDataObject(ms));
    dyn->ObjAdd(msdata);

    char misname[10];
    sprintf (misname, "missile%d", ((rand () >> 3) & 1)+1);

    csSpriteTemplate* tmpl = Sys->view->GetWorld ()->GetSpriteTemplate (misname);
    if (!tmpl)
      Sys->Printf (MSG_CONSOLE, "Could not find '%s' sprite template!\n", misname);
    else
    {
      csSprite3D* sp = tmpl->NewSprite ();
      sp->SetName ("missile");
      Sys->view->GetWorld ()->sprites.Push (sp);
      sp->MoveToSector (Sys->view->GetCamera ()->GetSector ());
      ms->sprite = sp;
      sp->SetMove (pos);
      csMatrix3 m = ms->dir.GetT2O ();
      sp->SetTransform (m);
      move_sprite (sp, Sys->view->GetCamera ()->GetSector (), pos);
    }
  }
  else if (!strcasecmp (cmd, "loadsprite"))
  {
    char filename[100], tempname[100], txtname[100];
    int cnt = 0;
    if (arg) cnt = ScanStr (arg, "%s,%s,%s", filename, tempname, txtname);
    if (cnt != 3)
    {
      Sys->Printf (MSG_CONSOLE, "Expected parameters 'file','template','texture'!\n");
    }
    else load_sprite (filename, tempname, txtname);
  }
  else if (!strcasecmp (cmd, "addsprite"))
  {
    char name[100];
    float size;
    if (arg) ScanStr (arg, "%s,%f", name, &size);
    else { *name = 0; size = 1; }
    add_sprite (name, name, Sys->view->GetCamera ()->GetSector (), Sys->view->GetCamera ()->GetOrigin (), size);
  }
  else if (!strcasecmp (cmd, "addskel"))
  {
    int depth, width;
    if (arg) ScanStr (arg, "%d,%d", &depth, &width);
    else { depth = 3; width = 3; }
    add_skeleton_tree (Sys->view->GetCamera ()->GetSector (), Sys->view->GetCamera ()->GetOrigin (),
    	depth, width);
  }
  else if (!strcasecmp (cmd, "addghost"))
  {
    int depth, width;
    if (arg) ScanStr (arg, "%d,%d", &depth, &width);
    else { depth = 5; width = 8; }
    add_skeleton_ghost (Sys->view->GetCamera ()->GetSector (), Sys->view->GetCamera ()->GetOrigin (),
    	depth, width);
  }
  else if (!strcasecmp (cmd, "addbot"))
  {
    float radius = 0;
    if (arg) ScanStr (arg, "%f", &radius);
    add_bot (2, Sys->view->GetCamera ()->GetSector (), Sys->view->GetCamera ()->GetOrigin (), radius);
  }
  else if (!strcasecmp (cmd, "delbot"))
  {
    if (first_bot)
    {
      Bot* bot = first_bot;
      first_bot = bot->next;
      Sys->view->GetWorld ()->RemoveSprite (bot);
    }
  }
  else if (!strcasecmp (cmd, "clrlights"))
  {
    csLightIt* lit = Sys->view->GetWorld ()->NewLightIterator ();
    csLight* l;
    while ((l = lit->Fetch ()) != NULL)
    {
      l->SetColor (csColor (0, 0, 0));
    }
  }
  else if (!strcasecmp (cmd, "setlight"))
  {
    if (Sys->selected_light)
    {
      float r, g, b;
      if (arg && ScanStr (arg, "%f,%f,%f", &r, &g, &b) == 3)
        Sys->selected_light->SetColor (csColor (r, g, b));
      else
        CsPrintf (MSG_CONSOLE, "Arguments missing or invalid!\n");
    }
    else
      CsPrintf (MSG_CONSOLE, "No light selected!\n");
  }
  else if (!strcasecmp (cmd, "addlight"))
  {
    csVector3 dir (0,0,0);
    csVector3 pos = Sys->view->GetCamera ()->Camera2World (dir);
    csDynLight* dyn;

    bool rnd;
    float r, g, b, radius, thing_shadows;
    if (arg && ScanStr (arg, "%f,%f,%f,%f,%d", &r, &g, &b, &radius, &thing_shadows) == 5)
    {
      CHK (dyn = new csDynLight (pos.x, pos.y, pos.z, radius, r, g, b));
      if (thing_shadows) dyn->SetFlags (CS_LIGHT_THINGSHADOWS, CS_LIGHT_THINGSHADOWS);
      rnd = false;
    }
    else
    {
      CHK (dyn = new csDynLight (pos.x, pos.y, pos.z, 6, 1, 1, 1));
      rnd = true;
    }
    Sys->view->GetWorld ()->AddDynLight (dyn);
    dyn->SetSector (Sys->view->GetCamera ()->GetSector ());
    dyn->Setup ();
    if (rnd)
    {
      CHK (RandomLight* rl = new RandomLight);
      rl->type = DYN_TYPE_RANDOM;
      rl->dyn_move_dir = .2;
      rl->dyn_move = 0;
      rl->dyn_r1 = rl->dyn_g1 = rl->dyn_b1 = 1;
      CHK(csDataObject* rldata = new csDataObject(rl));
      dyn->ObjAdd(rldata);
    }
    Sys->Printf (MSG_CONSOLE, "Dynamic light added.\n");
  }
  else if (!strcasecmp (cmd, "dellight"))
  {
    csDynLight* dyn;
    if ((dyn = Sys->view->GetWorld ()->GetFirstDynLight ()) != NULL)
    {
      Sys->view->GetWorld ()->RemoveDynLight (dyn);
      CHK (delete dyn);
      Sys->Printf (MSG_CONSOLE, "Dynamic light deleted.\n");
    }
  }
  else if (!strcasecmp (cmd, "dellights"))
  {
    csDynLight* dyn;
    while ((dyn = Sys->view->GetWorld ()->GetFirstDynLight ()) != NULL)
    {
      Sys->view->GetWorld ()->RemoveDynLight (dyn);
      CHK (delete dyn);
    }
    Sys->Printf (MSG_CONSOLE, "All dynamic lights deleted.\n");
  }
  else if (!strcasecmp (cmd, "picklight"))
  {
#   if 0
    pickup_light = Sys->view->GetWorld ()->GetFirstFltLight ();
    if (pickup_light) Sys->Printf (MSG_CONSOLE, "Floating light taken.\n");
    else Sys->Printf (MSG_CONSOLE, "No floating light to take.\n");
#   endif
  }
  else if (!strcasecmp (cmd, "droplight"))
  {
#   if 0
    if (pickup_light) Sys->Printf (MSG_CONSOLE, "Floating light dropped.\n");
    else Sys->Printf (MSG_CONSOLE, "No floating light to drop.\n");
    pickup_light = NULL;
#   endif
  }
  else if (!strcasecmp (cmd, "map"))
  {
    char* choices[4] = { "off", "overlay", "on", NULL };
    Command::change_choice (arg, &Sys->map_mode, "map", choices, 3);
  }
  else if (!strcasecmp (cmd, "snd_play"))
  {
    if (Sys->Sound)
    {
      csSoundData *sb =
        csSoundDataObject::GetSound(*(Sys->view->GetWorld()), arg);
      if (sb)
        Sys->Sound->PlayEphemeral(sb);
      else
        Sys->Printf (MSG_CONSOLE, "Sound '%s' not found!\n", arg);
    }
  }
  else if (!strcasecmp (cmd, "snd_volume"))
  {
    if (Sys->Sound)
    {
      float vol = Sys->Sound->GetVolume ();
      Command::change_float (arg, &vol, "snd_volume", 0.0, 1.0);
      Sys->Sound->SetVolume (vol);
    }
  }
  else
    return false;
  return true;
}

//-----------------------------------------------------------------------------

char WalkTest::world_dir [100];
bool WalkTest::move_3d = false;

WalkTest::WalkTest () :
  SysSystemDriver (), pos (0, 0, 0), velocity (0, 0, 0)
{
  Command::ExtraHandler = CommandHandler;
  auto_script = NULL;
  layer = NULL;
  view = NULL;
  infinite_maze = NULL;
  huge_room = NULL;
  wMissile_boom = NULL;
  wMissile_whoosh = NULL;
  cslogo = NULL;
  world = NULL;

  wf = NULL;
  map_mode = MAP_OFF;
  do_fps = true;
  do_stats = false;
  do_clear = false;
  do_edges = false;
  do_light_frust = false;
  do_show_coord = false;
  busy_perf_test = false;
  do_show_z = false;
  do_show_palette = false;
  do_infinite = false;
  do_huge = false;
  do_cd = true;
  do_freelook = false;
  player_spawned = false;
  do_gravity = true;
  inverse_mouse = false;
  selected_light = NULL;
  selected_polygon = NULL;
  move_forward = false;

  velocity.Set (0, 0, 0);
  angle.Set (0, 0, 0);
  angle_velocity.Set (0, 0, 0);

//pl=new PhysicsLibrary;

  timeFPS = 0.0;
}

WalkTest::~WalkTest ()
{
  if (World)
    World->DecRef ();
  CHK (delete wf);
  CHK (delete [] auto_script);
  CHK (delete layer);
  CHK (delete view);
  CHK (delete infinite_maze);
  CHK (delete huge_room);
  CHK (delete cslogo);
  CHK (delete body);
  CHK (delete legs);
}

void WalkTest::SetSystemDefaults (csIniFile *Config)
{
  superclass::SetSystemDefaults (Config);
  do_fps = Config->GetYesNo ("WalkTest", "FPS", true);
  do_stats = Config->GetYesNo ("WalkTest", "STATS", false);
  do_cd = Config->GetYesNo ("WalkTest", "COLLDET", true);

  const char *val;
  if (!(val = GetNameCL ()))
    val = Config->GetStr ("World", "WORLDFILE", "world");
  sprintf (world_dir, "/lev/%s", val);

  if (GetOptionCL ("clear"))
  {
    do_clear = true;
    Sys->Printf (MSG_INITIALIZATION, "Screen will be cleared every frame.\n");
  }
  else if (GetOptionCL ("noclear"))
  {
    do_clear = false;
    Sys->Printf (MSG_INITIALIZATION, "Screen will not be cleared every frame.\n");
  }

  if (GetOptionCL ("stats"))
  {
    do_stats = true;
    Sys->Printf (MSG_INITIALIZATION, "Statistics enabled.\n");
  }
  else if (GetOptionCL ("nostats"))
  {
    do_stats = false;
    Sys->Printf (MSG_INITIALIZATION, "Statistics disabled.\n");
  }

  if (GetOptionCL ("fps"))
  {
    do_fps = true;
    Sys->Printf (MSG_INITIALIZATION, "Frame Per Second enabled.\n");
  }
  else if (GetOptionCL ("nofps"))
  {
    do_fps = false;
    Sys->Printf (MSG_INITIALIZATION, "Frame Per Second disabled.\n");
  }

  if (GetOptionCL ("infinite"))
    do_infinite = true;

  if (GetOptionCL ("huge"))
    do_huge = true;

  if (GetOptionCL ("bots"))
    do_bots = true;

  if (GetOptionCL ("colldet"))
  {
    do_cd = true;
    Sys->Printf (MSG_INITIALIZATION, "Enabled collision detection system.\n");
  }
  else if (GetOptionCL ("nocolldet"))
  {
    do_cd = false;
    Sys->Printf (MSG_INITIALIZATION, "Disabled collision detection system.\n");
  }

  if ((val = GetOptionCL ("exec")))
  {
    CHK (delete [] auto_script);
    CHK (auto_script = strnew (val));
  }
}

void WalkTest::Help ()
{
  SysSystemDriver::Help ();
  Sys->Printf (MSG_STDOUT, "  -exec=<script>     execute given script at startup\n");
  Sys->Printf (MSG_STDOUT, "  -[no]clear         clear display every frame (default '%sclear')\n", do_clear ? "" : "no");
  Sys->Printf (MSG_STDOUT, "  -[no]stats         statistics (default '%sstats')\n", do_stats ? "" : "no");
  Sys->Printf (MSG_STDOUT, "  -[no]fps           frame rate printing (default '%sfps')\n", do_fps ? "" : "no");
  Sys->Printf (MSG_STDOUT, "  -[no]colldet       collision detection system (default '%scolldet')\n", do_cd ? "" : "no");
  Sys->Printf (MSG_STDOUT, "  -infinite          special infinite level generation (ignores world file!)\n");
  Sys->Printf (MSG_STDOUT, "  -huge              special huge level generation (ignores world file!)\n");
  Sys->Printf (MSG_STDOUT, "  -bots              allow random generation of bots\n");
  Sys->Printf (MSG_STDOUT, "  <path>             load world from VFS <path> (default '%s')\n", Config->GetStr ("World", "WORLDFILE", "world"));
}

/*------------------------------------------------------------------
 * The following handle_key_... routines are general movement
 * routines that are called by do_update() for the new movement
 * system and by do_keypress() for the old movement system (see
 * system.h for an explanation of the difference between the two
 * systems).
 *------------------------------------------------------------------*/

extern csCamera c;
extern WalkTest* Sys;

void WalkTest::strafe (float speed,int keep_old)
{
  if (move_3d || map_mode) return;
  static bool pressed = false;
  static float strafe_speed = 0;
  static long start_time = Time ();

  long cur_time = Time ();
  if (!keep_old)
  {
    bool new_pressed = fabs (speed) > 0.001;
    if (new_pressed != pressed)
    {
      pressed = new_pressed;
      strafe_speed = speed * cfg_walk_accelerate;
      start_time = cur_time - 100;
    }
  }

  while ((cur_time - start_time) >= 100)
  {
    if (pressed)
    {
      // accelerate
      if (fabs (velocity.x) < cfg_walk_maxspeed)
        velocity.x += strafe_speed;
    }
    else
    {
      // brake!
      if (velocity.x > cfg_walk_brake)
        velocity.x -= cfg_walk_brake;
      else if (velocity.x < -cfg_walk_brake)
        velocity.x += cfg_walk_brake;
      else
        velocity.x = 0;
    }
    start_time += 100;
  }
}

void WalkTest::step (float speed,int keep_old)
{
  if (move_3d || map_mode) return;

  static bool pressed = false;
  static float step_speed = 0;
  static long start_time = Time ();

  long cur_time = Time ();
  if (!keep_old)
  {
    bool new_pressed = fabs (speed) > 0.001;
    if (new_pressed != pressed)
    {
      pressed = new_pressed;
      step_speed = speed * cfg_walk_accelerate;
      start_time = cur_time - 100;
    }
  }

  while ((cur_time - start_time) >= 100)
  {
    if (pressed)
    {
      // accelerate
      if (fabs (velocity.z) < cfg_walk_maxspeed)
        velocity.z += step_speed;
    }
    else
    {
      // brake!
      if (velocity.z > cfg_walk_brake)
        velocity.z -= cfg_walk_brake;
      else if (velocity.z < -cfg_walk_brake)
        velocity.z += cfg_walk_brake;
      else
        velocity.z = 0;
    }
    start_time += 100;
  }
}

void WalkTest::rotate (float speed,int keep_old)
{
  if (move_3d || map_mode) return;

  static bool pressed = false;
  static float angle_accel = 0;
  static long start_time = Time ();

  long cur_time = Time ();
  if (!keep_old)
  {
    bool new_pressed = fabs (speed) > 0.001;
    if (new_pressed != pressed)
    {
      pressed = new_pressed;
      angle_accel = speed * cfg_rotate_accelerate;
      start_time = cur_time - 100;
    }
  }

  while ((cur_time - start_time) >= 100)
  {
    if (pressed)
    {
      // accelerate rotation
      if (fabs (angle_velocity.y) < cfg_rotate_maxspeed)
        angle_velocity.y += angle_accel;
    }
    else
    {
      // brake!
      if (angle_velocity.y > cfg_rotate_brake)
        angle_velocity.y -= cfg_rotate_brake;
      else if (angle_velocity.y < -cfg_rotate_brake)
        angle_velocity.y += cfg_rotate_brake;
      else
        angle_velocity.y = 0;
    }
    start_time += 100;
  }
}

void WalkTest::look (float speed,int keep_old)
{
  if (move_3d || map_mode) return;
  static float step_speed = 0;
  if (!keep_old)
    step_speed = speed*cfg_look_accelerate;
  if (fabs (angle.x+step_speed) <= (355.0/113.0/4))
    angle.x += step_speed;
}

void WalkTest::imm_forward (float speed, bool slow, bool fast)
{
  if (map_mode) { wf->KeyUp (speed, slow, fast); return; }
  if (slow)
    Sys->view->GetCamera ()->Move (speed * 0.01 * VEC_FORWARD);
  else if (fast)
    Sys->view->GetCamera ()->Move (speed * 4.0 * VEC_FORWARD);
  else
    Sys->view->GetCamera ()->Move (speed * 1.0 * VEC_FORWARD);
}

void WalkTest::imm_backward (float speed, bool slow, bool fast)
{
  if (map_mode) { wf->KeyDown (speed, slow, fast); return; }
  if (slow) Sys->view->GetCamera ()->Move (speed*.01*VEC_BACKWARD);
  else if (fast) Sys->view->GetCamera ()->Move (speed*1.2*VEC_BACKWARD);
  else Sys->view->GetCamera ()->Move (speed*.6*VEC_BACKWARD);
}

void WalkTest::imm_left (float speed, bool slow, bool fast)
{
  if (map_mode) return;
  if (slow)
    Sys->view->GetCamera ()->Move (speed * 0.01 * VEC_LEFT);
  else if (fast)
    Sys->view->GetCamera ()->Move (speed * 4.0 * VEC_LEFT);
  else
    Sys->view->GetCamera ()->Move (speed * 1.0 * VEC_LEFT);
}

void WalkTest::imm_right (float speed, bool slow, bool fast)
{
  if (map_mode) return;
  if (slow)
    Sys->view->GetCamera ()->Move (speed * 0.01 * VEC_RIGHT);
  else if (fast)
    Sys->view->GetCamera ()->Move (speed * 4.0 * VEC_RIGHT);
  else
    Sys->view->GetCamera ()->Move (speed * 1.0 * VEC_RIGHT);
}

void WalkTest::imm_up (float speed, bool slow, bool fast)
{
  if (map_mode) return;
  if (slow)
    Sys->view->GetCamera ()->Move (speed * 0.01 * VEC_UP);
  else if (fast)
    Sys->view->GetCamera ()->Move (speed * 4.0 * VEC_UP);
  else
    Sys->view->GetCamera ()->Move (speed * 1.0 * VEC_UP);
}

void WalkTest::imm_down (float speed, bool slow, bool fast)
{
  if (map_mode) return;
  if (slow)
    Sys->view->GetCamera ()->Move (speed * 0.01 * VEC_DOWN);
  else if (fast)
    Sys->view->GetCamera ()->Move (speed * 4.0 * VEC_DOWN);
  else
    Sys->view->GetCamera ()->Move (speed * 1.0 * VEC_DOWN);
}

void WalkTest::imm_rot_left_camera (float speed, bool slow, bool fast)
{
  if (map_mode) { wf->KeyLeft (speed, slow, fast); return; }
  if (slow)
    Sys->view->GetCamera ()->Rotate (VEC_ROT_LEFT, speed * .005);
  else if (fast)
    Sys->view->GetCamera ()->Rotate (VEC_ROT_LEFT, speed * .4);
  else
    Sys->view->GetCamera ()->Rotate (VEC_ROT_LEFT, speed * .2);
}

void WalkTest::imm_rot_left_world (float speed, bool slow, bool fast)
{
  if (map_mode) return;
  if (slow)
    Sys->view->GetCamera ()->RotateWorld (VEC_ROT_LEFT, speed * .005);
  else if (fast)
    Sys->view->GetCamera ()->RotateWorld (VEC_ROT_LEFT, speed * .4);
  else
    Sys->view->GetCamera ()->RotateWorld (VEC_ROT_LEFT, speed * .2);
}

void WalkTest::imm_rot_right_camera (float speed, bool slow, bool fast)
{
  if (map_mode) { wf->KeyRight (speed, slow, fast); return; }
  if (slow)
    Sys->view->GetCamera ()->Rotate (VEC_ROT_RIGHT, speed * .005);
  else if (fast)
    Sys->view->GetCamera ()->Rotate (VEC_ROT_RIGHT, speed * .4);
  else
    Sys->view->GetCamera ()->Rotate (VEC_ROT_RIGHT, speed * .2);
}

void WalkTest::imm_rot_right_world (float speed, bool slow, bool fast)
{
  if (map_mode) return;
  if (slow)
    Sys->view->GetCamera ()->RotateWorld (VEC_ROT_RIGHT, speed * .005);
  else if (fast)
    Sys->view->GetCamera ()->RotateWorld (VEC_ROT_RIGHT, speed * .4);
  else
    Sys->view->GetCamera ()->RotateWorld (VEC_ROT_RIGHT, speed * .2);
}

void WalkTest::imm_rot_left_xaxis (float speed, bool slow, bool fast)
{
  if (map_mode) { wf->KeyPgDn (speed, slow, fast); return; }
  if (slow)
    Sys->view->GetCamera ()->Rotate (VEC_TILT_DOWN, speed * .005);
  else if (fast)
    Sys->view->GetCamera ()->Rotate (VEC_TILT_DOWN, speed * .4);
  else
    Sys->view->GetCamera ()->Rotate (VEC_TILT_DOWN, speed * .2);
}

void WalkTest::imm_rot_right_xaxis (float speed, bool slow, bool fast)
{
  if (map_mode) { wf->KeyPgUp (speed, slow, fast); return; }
  if (slow)
    Sys->view->GetCamera ()->Rotate (VEC_TILT_UP, speed * .005);
  else if (fast)
    Sys->view->GetCamera ()->Rotate (VEC_TILT_UP, speed * .4);
  else
    Sys->view->GetCamera ()->Rotate (VEC_TILT_UP, speed * .2);
}

void WalkTest::imm_rot_left_zaxis (float speed, bool slow, bool fast)
{
  if (map_mode) return;
  if (slow)
    Sys->view->GetCamera ()->Rotate (VEC_TILT_LEFT, speed * .005);
  else if (fast)
    Sys->view->GetCamera ()->Rotate (VEC_TILT_LEFT, speed * .4);
  else
    Sys->view->GetCamera ()->Rotate (VEC_TILT_LEFT, speed * .2);
}

void WalkTest::imm_rot_right_zaxis (float speed, bool slow, bool fast)
{
  if (map_mode) return;
  if (slow)
    Sys->view->GetCamera ()->Rotate (VEC_TILT_RIGHT, speed * .005);
  else if (fast)
    Sys->view->GetCamera ()->Rotate (VEC_TILT_RIGHT, speed * .4);
  else
    Sys->view->GetCamera ()->Rotate (VEC_TILT_RIGHT, speed * .2);
}

void WalkTest::eatkeypress (int status, int key, bool shift, bool alt, bool ctrl)
{
  if (System->Console->IsActive () && status)
    ((csSimpleConsole *)System->Console)->AddChar (key);
  else switch (key)
  {
    case CSKEY_TAB:
      if (status)
        System->Console->Show ();
      break;

    default:
      {
        csKeyMap *m = mapping;
        while (m)
        {
          if (key == m->key && shift == m->shift
           && alt == m->alt && ctrl == m->ctrl)
          {
            if (m->need_status)
            {
              // Don't perform the command again if the key is already down
              if (m->is_on != status)
              {
                char buf [256];
                sprintf (buf,"%s %d", m->cmd, status);
                Command::perform_line (buf);
                m->is_on = status;
              }
            }
            else
            {
              if (status)
                Command::perform_line (m->cmd);
            }
          }
          else if (!status && m->is_on && m->need_status)
          {
            if (key == m->key || shift != m->shift || alt != m->alt || ctrl != m->ctrl)
            {
              char buf [256];
              sprintf (buf,"%s 0", m->cmd);
              Command::perform_line (buf);
              m->is_on = 0;
            }
          }
	  m = m->next;
        }
      }
      break;
  }
}

void WalkTest::NextFrame (time_t elapsed_time, time_t current_time)
{
  // The following will fetch all events from queue and handle them
  SysSystemDriver::NextFrame (elapsed_time, current_time);

  // Record the first time this routine is called.
  if(do_bots)
  {
    static long first_time = -1;
    static time_t next_bot_at;
    if (first_time == -1) { first_time = current_time; next_bot_at = current_time+1000*10; }
    if (current_time > next_bot_at)
    {
      add_bot (2, view->GetCamera ()->GetSector (), view->GetCamera ()->GetOrigin (), 0);
      next_bot_at = current_time+1000*10;
    }
  }
#if 0 // don't like it. Yet ;)
  {
    static long first_time = -1;
    static long next_bot_at;
    if (first_time == -1) { first_time = current_time; next_bot_at = current_time+1000*3; }
    if (current_time > next_bot_at)
    {
      csSpriteTemplate* tmpl = Sys->view->GetWorld ()->GetSpriteTemplate ("bot");
      if (tmpl)
      {
        csSprite3D *sp = tmpl->NewSprite ();
        sp->MoveToSector (view->GetCamera()->GetSector());
/*      csMatrix3 m; m.Identity ();
        sp->SetTransform (m);
        sp->SetMove (view->GetCamera()->GetOrigin());*/
        csVector3 forward=view->GetCamera()->GetW2C().Col3();
        pl->AddObject (sp,view->GetCamera ()->GetOrigin ()+forward*3); //(2, view->GetCamera ()->GetSector (), view->GetCamera ()->GetOrigin (), 0);
      }
      next_bot_at = current_time+1000*3;
    }
  }

  pl->MakeStep ();
#endif
  if (!System->Console->IsActive ())
  {
    int alt,shift,ctrl;
    float speed = 1;

    alt = GetKeyState (CSKEY_ALT);
    ctrl = GetKeyState (CSKEY_CTRL);
    shift = GetKeyState (CSKEY_SHIFT);
    if (ctrl) speed = .5;
    if (shift) speed = 2;

    /// Act as usual...
    strafe (0,1); look (0,1); step (0,1); rotate (0,1);

    if (Sys->Sound)
    {
      iSoundListener *sndListener = Sys->Sound->GetListener();
      if(sndListener)
      {
        // take position/direction from view->GetCamera ()
        csVector3 v = view->GetCamera ()->GetOrigin ();
        csMatrix3 m = view->GetCamera ()->GetC2W();
        csVector3 f = m.Col3();
        csVector3 t = m.Col2();
        sndListener->SetPosition(v.x, v.y, v.z);
        sndListener->SetDirection(f.x,f.y,f.z,t.x,t.y,t.z);
        //sndListener->SetDirection(...);
      }
    }
  } /* endif */

  if(first_bot)
  {
    Bot* bot = first_bot;
    while (bot)
    {
      bot->move (elapsed_time);
      bot = bot->next;
    }
  }

  if (move_forward) step (1, 0);

  PrepareFrame (elapsed_time, current_time);
  DrawFrame (elapsed_time, current_time);

  // Execute one line from the script.
  if (!busy_perf_test)
  {
    char buf[256];
    if (Command::get_script_line (buf, 255)) Command::perform_line (buf);
  }
}

bool WalkTest::HandleEvent (csEvent &Event)
{
  // First pass the event to all plugins
  if (SysSystemDriver::HandleEvent (Event))
    return true;

  switch (Event.Type)
  {
    case csevKeyDown:
      eatkeypress (1,Event.Key.Code,
        (Event.Key.ShiftKeys & CSMASK_SHIFT) != 0,
        (Event.Key.ShiftKeys & CSMASK_ALT) != 0,
        (Event.Key.ShiftKeys & CSMASK_CTRL) != 0);
      break;
    case csevKeyUp:
      eatkeypress (0,Event.Key.Code,
        (Event.Key.ShiftKeys & CSMASK_SHIFT) != 0,
        (Event.Key.ShiftKeys & CSMASK_ALT) != 0,
        (Event.Key.ShiftKeys & CSMASK_CTRL) != 0);
      break;
    case csevMouseDown:
      if (Event.Mouse.Button == 1)
        move_forward = true;
      else if (Event.Mouse.Button == 3)
      {
        csVector2   screenPoint;
        csSprite3D *closestSprite;

        screenPoint.x = Event.Mouse.x;
        screenPoint.y = Event.Mouse.y;
        closestSprite = FindNextClosestSprite(NULL, view->GetCamera(), &screenPoint);
        if (closestSprite)
          Sys->Printf (MSG_CONSOLE, "Selected sprite %s\n", closestSprite->GetName ());
        else
          Sys->Printf (MSG_CONSOLE, "No sprite selected!\n");
      }
      else if (Event.Mouse.Button == 2)
      {
        unsigned long* zb = System->G3D->GetZBufPoint(Event.Mouse.x, Event.Mouse.y);

        if (zb)
        {
          csVector3 v;
          v.z = 1. / (((float)*zb)/(256.*65536.));
          v.x = (Event.Mouse.x-FRAME_WIDTH/2) * v.z / view->GetCamera ()->aspect;
          v.y = (FRAME_HEIGHT-1-Event.Mouse.y-FRAME_HEIGHT/2) * v.z / view->GetCamera ()->aspect;
          csVector3 vw = view->GetCamera ()->Camera2World (v);

          Sys->Printf (MSG_CONSOLE, "LMB down : z_buf=%ld cam:(%f,%f,%f) world:(%f,%f,%f)\n", zb, v.x, v.y, v.z, vw.x, vw.y, vw.z);
          Sys->Printf (MSG_DEBUG_0, "LMB down : z_buf=%ld cam:(%f,%f,%f) world:(%f,%f,%f)\n", zb, v.x, v.y, v.z, vw.x, vw.y, vw.z);
        }

        extern csVector2 coord_check_vector;
        coord_check_vector.x = Event.Mouse.x;
        coord_check_vector.y = FRAME_HEIGHT-Event.Mouse.y;
        extern bool check_poly, check_light;
        extern void select_object (csRenderView* rview, int type, void* entity);
        check_poly = check_light = true;
	  view->GetWorld ()->DrawFunc (view->GetCamera (), view->GetClipper (), select_object);
      }
      break;
    case csevMouseMove:
      // additional command by Leslie Saputra -> freelook mode.
      {
        static bool first_time = true;
        if (do_freelook)
        {
          int last_x, last_y;
          last_x = Event.Mouse.x;
          last_y = Event.Mouse.y;

          System->G2D->SetMousePosition (FRAME_WIDTH / 2, FRAME_HEIGHT / 2);
          if (!first_time)
          {
          /*
            if(move_3d)
              view->GetCamera ()->Rotate (VEC_ROT_RIGHT, ((float)( last_x - (FRAME_WIDTH / 2) )) / (FRAME_WIDTH*2) );
            else
              view->GetCamera ()->RotateWorld (VEC_ROT_RIGHT, ((float)( last_x - (FRAME_WIDTH / 2) )) / (FRAME_WIDTH*2) );
            view->GetCamera ()->Rotate (VEC_TILT_UP, -((float)( last_y - (FRAME_HEIGHT / 2) )) / (FRAME_HEIGHT*2) );
          */

            this->angle.y+=((float)(last_x - (FRAME_WIDTH / 2) )) / (FRAME_WIDTH*2);
            this->angle.x+=((float)(last_y - (FRAME_HEIGHT / 2) )) / (FRAME_HEIGHT*2)*(1-2*(int)inverse_mouse);
          }
          else
            first_time = false;
        }
        else
          first_time = true;
      }
      break;
    case csevMouseUp:
      if (Event.Mouse.Button == 1)
        move_forward = false;
      break;
  }

  return false;
}

csSprite3D *FindNextClosestSprite(csSprite3D *baseSprite, csCamera *camera, csVector2 *screenCoord)
{
  int spriteIndex;
  float thisZLocation;
  float closestZLocation;
  csSprite3D *closestSprite;
  csSprite3D *nextSprite;
  csBox screenBoundingBox;

  if (baseSprite)
  {
    closestSprite = baseSprite;
    closestZLocation = baseSprite->GetScreenBoundingBox(*camera, screenBoundingBox);
    // if the baseSprite isn't in front of the camera, return
    if (closestZLocation < 0)
      return NULL;
  }
  else
  {
    closestSprite = NULL;
    closestZLocation = 32000;
  }

  for (spriteIndex = 0; spriteIndex < Sys->world->sprites.Length(); spriteIndex++)
  {
    nextSprite = (csSprite3D*)Sys->world->sprites[spriteIndex];

//  Sys->Printf(MSG_CONSOLE, "Checking sprite %s\n", nextSprite->GetName ());
    if (nextSprite != baseSprite)
    {
      thisZLocation = nextSprite->GetScreenBoundingBox(*camera, screenBoundingBox);
      if ((thisZLocation > 0) && (thisZLocation < closestZLocation))
      {
        if (screenBoundingBox.In(screenCoord->x, screenCoord->y))
        {
          closestZLocation = thisZLocation;
          closestSprite = nextSprite;
        }
      }
    }
  }

  return closestSprite;
}
