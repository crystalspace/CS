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

#define CS_SYSDEF_PROVIDE_ALLOCA
#include "cssysdef.h"
#include "csgeom/math3d.h"
#include "spr2d.h"
#include "iengine/movable.h"
#include "iengine/rview.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/material.h"
#include "iengine/material.h"
#include "iengine/camera.h"
#include "igeom/clip2d.h"
#include "iengine/engine.h"
#include "iengine/light.h"
#include "qsqrt.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csSprite2DMeshObject)
  SCF_IMPLEMENTS_INTERFACE (iMeshObject)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iSprite2DState)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iParticle)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSprite2DMeshObject::Sprite2DState)
  SCF_IMPLEMENTS_INTERFACE (iSprite2DState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSprite2DMeshObject::Particle)
  SCF_IMPLEMENTS_INTERFACE (iParticle)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csSprite2DMeshObject::csSprite2DMeshObject (csSprite2DMeshObjectFactory* factory)
{
  SCF_CONSTRUCT_IBASE (NULL);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiSprite2DState);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiParticle);
  csSprite2DMeshObject::factory = factory;
  ifactory = SCF_QUERY_INTERFACE (factory, iMeshObjectFactory);
  material = factory->GetMaterialWrapper ();
  lighting = factory->HasLighting ();
  MixMode = factory->GetMixMode ();
  initialized = false;
  vis_cb = NULL;
  shapenr = 0;
  current_lod = 1;
  current_features = ALL_FEATURES;
}

csSprite2DMeshObject::~csSprite2DMeshObject ()
{
  if (vis_cb) vis_cb->DecRef ();
  if (ifactory) ifactory->DecRef ();
}

void csSprite2DMeshObject::SetupObject ()
{
  if (!initialized)
  {
    initialized = true;
    if (!lighting)
    {
      // If there is no lighting then we need to copy the color_init
      // array to color.
      float max_sq_dist = 0;
      for (int i = 0 ; i < vertices.Length () ; i++)
      {
        csSprite2DVertex& v = vertices[i];
        v.color = vertices[i].color_init;
        v.color.Clamp (2, 2, 2);
	float sqdist = v.pos.x*v.pos.x + v.pos.y*v.pos.y;
	if (sqdist > max_sq_dist) max_sq_dist = sqdist;
      }
      float max_dist = qsqrt (max_sq_dist);
      radius.Set (max_dist, max_dist, max_dist);
    }
  }
}

bool csSprite2DMeshObject::DrawTest (iRenderView* rview, iMovable* movable)
{
  SetupObject ();

  // Camera transformation for the single 'position' vector.
  cam = rview->GetCamera ()->GetTransform ().Other2This (movable->GetFullPosition ());
  if (cam.z < SMALL_Z) return false;
  return true;
}

void csSprite2DMeshObject::UpdateLighting (iLight** lights, int num_lights,
    const csVector3& pos)
{
  SetupObject ();
  if (!lighting) return;
  csColor color (0, 0, 0);

  // @@@ GET AMBIENT
  //csSector* sect = movable.GetSector (0);
  //if (sect)
  //{
    //int r, g, b;
    //sect->GetAmbientColor (r, g, b);
    //color.Set (r / 128.0, g / 128.0, b / 128.0);
  //}

  int i;
  for (i = 0; i < num_lights; i++)
  {
    csColor light_color = lights[i]->GetColor () * (256. / NORMAL_LIGHT_LEVEL);
    float sq_light_radius = lights [i]->GetSquaredRadius ();
    // Compute light position.
    csVector3 wor_light_pos = lights [i]->GetCenter ();
    float wor_sq_dist =
      csSquaredDist::PointPoint (wor_light_pos, pos);
    if (wor_sq_dist >= sq_light_radius) continue;
    float wor_dist = qsqrt (wor_sq_dist);
    float cosinus = 1.;
    cosinus /= wor_dist;
    light_color *= cosinus * lights [i]->GetBrightnessAtDistance (wor_dist);
    color += light_color;
  }
  for (i = 0 ; i < vertices.Length () ; i++)
  {
    vertices[i].color = vertices[i].color_init + color;
    vertices[i].color.Clamp (2, 2, 2);
  }
}

void csSprite2DMeshObject::UpdateLighting (iLight** lights, int num_lights,
    iMovable* movable)
{
  if (!lighting) return;
  csVector3 pos = movable->GetFullPosition ();
  UpdateLighting (lights, num_lights, pos);
}

#define INTERPOLATE1(component) \
  g3dpoly->vertices [i].##component## = inpoly [vt].##component## + \
    t * (inpoly [vt2].##component## - inpoly [vt].##component##);

#define INTERPOLATE(component) \
{ \
  float v1 = inpoly [edge_from [0]].##component## + \
    t1 * (inpoly [edge_to [0]].##component## - inpoly [edge_from [0]].##component##); \
  float v2 = inpoly [edge_from [1]].##component## + \
    t2 * (inpoly [edge_to [1]].##component## - inpoly [edge_from [1]].##component##); \
  g3dpoly->vertices [i].##component## = v1 + t * (v2 - v1); \
}

static void PreparePolygonFX2 (G3DPolygonDPFX* g3dpoly,
  csVector2* clipped_verts, int num_vertices, csVertexStatus* clipped_vtstats,
  int orig_num_vertices, bool gouraud)
{
  // first we copy the first texture coordinates to a local buffer
  // to avoid that they are overwritten when interpolating.
  ALLOC_STACK_ARRAY (inpoly, G3DTexturedVertex, orig_num_vertices);
  int i;
  for (i = 0; i < orig_num_vertices; i++)
    inpoly[i] = g3dpoly->vertices[i];

  int vt, vt2;
  float t;
  for (i = 0; i < num_vertices; i++)
  {
    g3dpoly->vertices [i].sx = clipped_verts [i].x;
    g3dpoly->vertices [i].sy = clipped_verts [i].y;
    switch (clipped_vtstats[i].Type)
    {
      case CS_VERTEX_ORIGINAL:
        vt = clipped_vtstats[i].Vertex;
        g3dpoly->vertices [i].z = inpoly [vt].z;
        g3dpoly->vertices [i].u = inpoly [vt].u;
        g3dpoly->vertices [i].v = inpoly [vt].v;
	if (gouraud)
	{
          g3dpoly->vertices [i].r = inpoly [vt].r;
          g3dpoly->vertices [i].g = inpoly [vt].g;
          g3dpoly->vertices [i].b = inpoly [vt].b;
	}
	break;
      case CS_VERTEX_ONEDGE:
        vt = clipped_vtstats[i].Vertex;
	vt2 = vt + 1; if (vt2 >= orig_num_vertices) vt2 = 0;
	t = clipped_vtstats [i].Pos;
	INTERPOLATE1 (z);
	INTERPOLATE1 (u);
	INTERPOLATE1 (v);
	if (gouraud)
	{
	  INTERPOLATE1 (r);
	  INTERPOLATE1 (g);
	  INTERPOLATE1 (b);
	}
	break;
      case CS_VERTEX_INSIDE:
        float x = clipped_verts [i].x;
        float y = clipped_verts [i].y;
        int edge_from [2], edge_to [2];
	int edge = 0;
	int j, j1;
	j1 = orig_num_vertices - 1;
	for (j = 0; j < orig_num_vertices; j++)
	{
          if ((y >= inpoly [j].sy && y <= inpoly [j1].sy) ||
	      (y <= inpoly [j].sy && y >= inpoly [j1].sy))
	  {
	    edge_from [edge] = j;
	    edge_to [edge] = j1;
	    edge++;
	    if (edge >= 2) break;
	  }
	  j1 = j;
	}
	if (edge == 1)
	{
	  // Safety if we only found one edge.
	  edge_from [1] = edge_from [0];
	  edge_to [1] = edge_to [0];
	}
	G3DTexturedVertex& A = inpoly [edge_from [0]];
	G3DTexturedVertex& B = inpoly [edge_to [0]];
	G3DTexturedVertex& C = inpoly [edge_from [1]];
	G3DTexturedVertex& D = inpoly [edge_to [1]];
	float t1 = (y - A.sy) / (B.sy - A.sy);
	float t2 = (y - C.sy) / (D.sy - C.sy);
	float x1 = A.sx + t1 * (B.sx - A.sx);
	float x2 = C.sx + t2 * (D.sx - C.sx);
	t = (x - x1) / (x2 - x1);
	INTERPOLATE (z);
	INTERPOLATE (u);
	INTERPOLATE (v);
	if (gouraud)
	{
	  INTERPOLATE (r);
	  INTERPOLATE (g);
	  INTERPOLATE (b);
	}
	break;
    }
  }
}

#undef INTERPOLATE
#undef INTERPOLATE1

bool csSprite2DMeshObject::Draw (iRenderView* rview, iMovable* /*movable*/,
	csZBufMode mode)
{
// @@@ TODO:
//     - Z fill vs Z use
  if (!material)
  {
    printf ("INTERNAL ERROR: sprite2D used without material!\n");
    return false;
  }
  iMaterialHandle* mat = material->GetMaterialHandle ();
  if (!mat)
  {
    printf ("INTERNAL ERROR: sprite2D used without valid material handle!\n");
    return false;
  }

  if (vis_cb) if (!vis_cb->BeforeDrawing (this, rview)) return false;

  iGraphics3D* g3d = rview->GetGraphics3D ();
  iCamera* camera = rview->GetCamera ();

  // Prepare for rendering.
  g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, mode);

  material->Visit ();

  g3dpolyfx.num = vertices.Length ();
  g3dpolyfx.mat_handle = mat;
  g3dpolyfx.mat_handle->GetTexture ()->GetMeanColor (g3dpolyfx.flat_color_r,
    g3dpolyfx.flat_color_g, g3dpolyfx.flat_color_b);

  ALLOC_STACK_ARRAY (poly2d, csVector2, vertices.Length ());
  csVector2 clipped_poly2d[MAX_OUTPUT_VERTICES];
  csVertexStatus clipped_vtstats[MAX_OUTPUT_VERTICES];

  float iz = 1. / cam.z;
  float iza = iz * camera->GetFOV ();

  int i;

  for (i = 0; i < vertices.Length (); i++)
  {
    g3dpolyfx.vertices [i].z = iz;
    g3dpolyfx.vertices [i].sx =
    poly2d [i].x = (cam.x + vertices [i].pos.x) * iza + camera->GetShiftX ();
    g3dpolyfx.vertices [i].sy =
    poly2d [i].y = (cam.y + vertices [i].pos.y) * iza + camera->GetShiftY ();
    g3dpolyfx.vertices [i].r = vertices [i].color.red;
    g3dpolyfx.vertices [i].g = vertices [i].color.green;
    g3dpolyfx.vertices [i].b = vertices [i].color.blue;
  }

  if (!uvani.animate)
  {
    for (i = 0; i < vertices.Length (); i++)
    {
      g3dpolyfx.vertices [i].u = vertices [i].u;
      g3dpolyfx.vertices [i].v = vertices [i].v;
    }
  }
  else
  {
    int n;
    const csVector2 *uv = uvani.GetVertices (n);
    for (i = 0; i < n; i++)
    {
      g3dpolyfx.vertices [i].u = uv [i].x;
      g3dpolyfx.vertices [i].v = uv [i].y;
    }
  }

  int num_clipped_verts;
  UByte clip_result = rview->GetClipper ()->Clip (poly2d, vertices.Length (),
    clipped_poly2d, num_clipped_verts, clipped_vtstats);
  if (clip_result == CS_CLIP_OUTSIDE) return false;
  g3dpolyfx.num = num_clipped_verts;

  if (clip_result != CS_CLIP_INSIDE)
    PreparePolygonFX2 (&g3dpolyfx, clipped_poly2d, num_clipped_verts,
    	clipped_vtstats, vertices.Length (), true);

  rview->CalculateFogPolygon (g3dpolyfx);
  g3dpolyfx.mixmode = MixMode | CS_FX_GOURAUD;
  g3d->DrawPolygonFX (g3dpolyfx);

  return true;
}

void csSprite2DMeshObject::GetObjectBoundingBox (csBox3& /*bbox*/, int /*type*/)
{
  SetupObject ();
  //@@@ TODO
  //bbox = object_bbox;
}

void csSprite2DMeshObject::HardTransform (const csReversibleTransform& t)
{
  (void)t;
  //@@@ TODO
}

void csSprite2DMeshObject::CreateRegularVertices (int n, bool setuv)
{
  double angle_inc = 2.0 * PI / n;
  double angle = 0.0;
  vertices.SetLimit (n);
  vertices.SetLength (n);
  for (int i = 0; i < vertices.Length (); i++, angle += angle_inc)
  {
    vertices [i].pos.y = cos (angle);
    vertices [i].pos.x = sin (angle);
    if (setuv) 
    {
      // reuse sin/cos values and scale to [0..1]
      vertices [i].u = vertices [i].pos.x / 2.0f + 0.5f;
      vertices [i].v = vertices [i].pos.y / 2.0f + 0.5f;
    }
    vertices [i].color.Set (1, 1, 1);
    vertices [i].color_init.Set (1, 1, 1);
  }
  shapenr++;
}

void csSprite2DMeshObject::NextFrame (csTime current_time)
{
  if (uvani.animate && !uvani.halted)
    uvani.Advance (current_time);
}

void csSprite2DMeshObject::Particle::UpdateLighting (iLight** lights,
    int num_lights, const csReversibleTransform& transform)
{
  csVector3 new_pos = transform.This2Other (part_pos);
  scfParent->UpdateLighting (lights, num_lights, new_pos);
}

void csSprite2DMeshObject::Particle::Draw (iRenderView* rview,
    const csReversibleTransform& transform, csZBufMode mode)
{
  scfParent->SetupObject ();

  // Camera transformation for the single 'position' vector.
  csVector3 new_pos = transform.This2Other (part_pos);
  scfParent->cam = rview->GetCamera ()->GetTransform ().Other2This (new_pos);
  if (scfParent->cam.z < SMALL_Z) return;
  scfParent->Draw (rview, NULL, mode);
}

void csSprite2DMeshObject::Particle::SetColor (const csColor& col)
{
  csColoredVertices& vertices = scfParent->GetVertices ();
  int i;
  for (i = 0 ; i < vertices.Length () ; i++)
    vertices[i].color_init = col;
  if (!scfParent->lighting)
    for (i = 0 ; i < vertices.Length () ; i++)
      vertices[i].color = col;
}

void csSprite2DMeshObject::Particle::AddColor (const csColor& col)
{
  csColoredVertices& vertices = scfParent->GetVertices ();
  int i;
  for (i = 0 ; i < vertices.Length () ; i++)
    vertices[i].color_init += col;
  if (!scfParent->lighting)
    for (i = 0 ; i < vertices.Length () ; i++)
      vertices[i].color = vertices[i].color_init;
}

void csSprite2DMeshObject::Particle::ScaleBy (float factor)
{
  csColoredVertices& vertices = scfParent->GetVertices ();
  for (int i = 0; i < vertices.Length (); i++)
    vertices[i].pos *= factor;
  scfParent->shapenr++;
}

void csSprite2DMeshObject::Particle::Rotate (float angle)
{
  csColoredVertices& vertices = scfParent->GetVertices ();
  for (int i = 0; i < vertices.Length (); i++)
    vertices[i].pos.Rotate (angle);
  scfParent->shapenr++;
}

void csSprite2DMeshObject::Sprite2DState::SetUVAnimation (const char *name, int style, bool loop)
{
  if (name)
  {
    iSprite2DUVAnimation *ani = scfParent->factory->GetUVAnimation (name);
    if (ani && ani->GetFrameCount ())
    {
      scfParent->uvani.animate = true;
      scfParent->uvani.ani = ani;
      scfParent->uvani.last_time = 0;
      scfParent->uvani.frameindex = 0;
      scfParent->uvani.framecount = ani->GetFrameCount ();
      scfParent->uvani.frame = ani->GetFrame (0);
      scfParent->uvani.style = style;
      scfParent->uvani.counter = 0;
      scfParent->uvani.loop = loop;
      scfParent->uvani.halted = false;
    }
  }
  else
  {
    // stop animation and show the normal texture
    scfParent->uvani.animate = false;
  }
}

void csSprite2DMeshObject::Sprite2DState::StopUVAnimation (int idx)
{
  if (scfParent->uvani.animate)
  {
    if (idx != -1)
    {
      scfParent->uvani.frameindex = MIN(MAX(idx, 0), scfParent->uvani.framecount-1);
      scfParent->uvani.frame = scfParent->uvani.ani->GetFrame (scfParent->uvani.frameindex);
    }
    scfParent->uvani.halted = true;
  }
}

void csSprite2DMeshObject::Sprite2DState::PlayUVAnimation (int idx, int style, bool loop)
{
  if (scfParent->uvani.animate)
  {
    if (idx != -1)
    {
      scfParent->uvani.frameindex = MIN(MAX(idx, 0), scfParent->uvani.framecount-1);
      scfParent->uvani.frame = scfParent->uvani.ani->GetFrame (scfParent->uvani.frameindex);
    }
    scfParent->uvani.halted = false;
    scfParent->uvani.counter = 0;
    scfParent->uvani.last_time = 0;
    scfParent->uvani.loop = loop;
    scfParent->uvani.style = style;
  }
}

int csSprite2DMeshObject::Sprite2DState::GetUVAnimationCount () const
{ return scfParent->factory->GetUVAnimationCount ();}
iSprite2DUVAnimation *csSprite2DMeshObject::Sprite2DState::CreateUVAnimation ()
{ return scfParent->factory->CreateUVAnimation ();}
void csSprite2DMeshObject::Sprite2DState::RemoveUVAnimation (iSprite2DUVAnimation *anim)
{ scfParent->factory->RemoveUVAnimation (anim); }
iSprite2DUVAnimation *csSprite2DMeshObject::Sprite2DState::GetUVAnimation (const char *name)
{ return scfParent->factory->GetUVAnimation (name); }
iSprite2DUVAnimation *csSprite2DMeshObject::Sprite2DState::GetUVAnimation (int idx)
{ return scfParent->factory->GetUVAnimation (idx); }


void csSprite2DMeshObject::uvAnimationControl::Advance (csTime current_time)
{
  int oldframeindex = frameindex;
  // the goal is to find the next frame to show
  if (style < 0)
  { // every (-1*style)-th frame show a new pic
    counter--;
    if (counter < style)
    {
      counter = 0;
      frameindex++;
      if (frameindex == framecount)
	if (loop)
	  frameindex = 0;
	else
	{
	  frameindex = framecount-1;
	  halted = true;
	}
    }
  }
  else
    if (style > 0)
    { // skip to next frame every <style> millisecond
      if (last_time == 0)
	last_time = current_time;
      counter += (current_time - last_time);
      last_time = current_time;
      while (counter > style)
      {
	counter -= style;
	frameindex++;
	if (frameindex == framecount)
	  if (loop)
	    frameindex = 0;
	  else
	    {
	      frameindex = framecount-1;
	      halted = true;
	    }
      }
    }
    else
    { // style == 0 -> use time indices attached to the frames
      if (last_time == 0)
	last_time = current_time;
      while (frame->GetDuration () + last_time < current_time)
      {
	frameindex++;
	if (frameindex == framecount)
	  if (loop)
	  {
	    frameindex = 0;
	  }
	  else
	  {
	    frameindex = framecount-1;
	    halted = true;
	    break;
	  }
	last_time += frame->GetDuration ();
	frame = ani->GetFrame (frameindex);
      }
    }

  if (oldframeindex != frameindex)
    frame = ani->GetFrame (frameindex);
    
}

const csVector2 *csSprite2DMeshObject::uvAnimationControl::GetVertices (int &num)
{
  num = frame->GetUVCount ();
  return frame->GetUVCoo ();
}

//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csSprite2DMeshObjectFactory)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectFactory)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iSprite2DFactoryState)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSprite2DMeshObjectFactory::Sprite2DFactoryState)
  SCF_IMPLEMENTS_INTERFACE (iSprite2DFactoryState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csSprite2DMeshObjectFactory::csSprite2DMeshObjectFactory (iBase *pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiSprite2DFactoryState);
  material = NULL;
  MixMode = 0;
  lighting = true;
}

csSprite2DMeshObjectFactory::~csSprite2DMeshObjectFactory ()
{
}

iMeshObject* csSprite2DMeshObjectFactory::NewInstance ()
{
  csSprite2DMeshObject* cm = new csSprite2DMeshObject (this);
  iMeshObject* im = SCF_QUERY_INTERFACE (cm, iMeshObject);
  im->DecRef ();
  return im;
}

//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csSprite2DMeshObjectType)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectType)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSprite2DMeshObjectType::eiPlugIn)
  SCF_IMPLEMENTS_INTERFACE (iPlugin)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csSprite2DMeshObjectType)

SCF_EXPORT_CLASS_TABLE (spr2d)
  SCF_EXPORT_CLASS (csSprite2DMeshObjectType, "crystalspace.mesh.object.sprite.2d",
    "Crystal Space Sprite2D Mesh Type")
SCF_EXPORT_CLASS_TABLE_END

csSprite2DMeshObjectType::csSprite2DMeshObjectType (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiPlugin);
}

csSprite2DMeshObjectType::~csSprite2DMeshObjectType ()
{
}

iMeshObjectFactory* csSprite2DMeshObjectType::NewFactory ()
{
  csSprite2DMeshObjectFactory* cm = new csSprite2DMeshObjectFactory (this);
  iMeshObjectFactory* ifact = SCF_QUERY_INTERFACE (cm, iMeshObjectFactory);
  ifact->DecRef ();
  return ifact;
}

