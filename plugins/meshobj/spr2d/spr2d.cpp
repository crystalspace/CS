/*
    Copyright (C) 2000 by Jorrit Tyberghein

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

#define SYSDEF_ALLOCA
#include "cssysdef.h"
#include "csgeom/math3d.h"
#include "csgeom/fastsqrt.h"
#include "plugins/meshobj/spr2d/spr2d.h"
#include "imovable.h"
#include "irview.h"
#include "igraph3d.h"
#include "igraph2d.h"
#include "imater.h"
#include "icamera.h"
#include "iclip2.h"
#include "iengine.h"
#include "itranman.h"
#include "ilight.h"
#include "lightdef.h"

IMPLEMENT_IBASE (csSprite2DMeshObject)
  IMPLEMENTS_INTERFACE (iMeshObject)
  IMPLEMENTS_EMBEDDED_INTERFACE (iSprite2DState)
  IMPLEMENTS_EMBEDDED_INTERFACE (iParticle)
IMPLEMENT_IBASE_END

IMPLEMENT_EMBEDDED_IBASE (csSprite2DMeshObject::Sprite2DState)
  IMPLEMENTS_INTERFACE (iSprite2DState)
IMPLEMENT_EMBEDDED_IBASE_END

IMPLEMENT_EMBEDDED_IBASE (csSprite2DMeshObject::Particle)
  IMPLEMENTS_INTERFACE (iParticle)
IMPLEMENT_EMBEDDED_IBASE_END

csSprite2DMeshObject::csSprite2DMeshObject (csSprite2DMeshObjectFactory* factory)
{
  CONSTRUCT_IBASE (NULL);
  CONSTRUCT_EMBEDDED_IBASE (scfiSprite2DState);
  CONSTRUCT_EMBEDDED_IBASE (scfiParticle);
  csSprite2DMeshObject::factory = factory;
  material = factory->GetMaterialWrapper ();
  lighting = factory->HasLighting ();
  MixMode = factory->GetMixMode ();
}

csSprite2DMeshObject::~csSprite2DMeshObject ()
{
}

void csSprite2DMeshObject::SetupObject ()
{
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
    float wor_dist = sqrt (wor_sq_dist);
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

bool csSprite2DMeshObject::Draw (iRenderView* rview, iMovable* /*movable*/)
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

  iGraphics3D* g3d = rview->GetGraphics3D ();
  iCamera* camera = rview->GetCamera ();

  // Prepare for rendering.
  g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_USE);

  material->Visit ();

  g3dpolyfx.num = vertices.Length ();
  g3dpolyfx.mat_handle = mat;
  g3dpolyfx.inv_aspect = camera->GetInvFOV ();
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
    g3dpolyfx.vertices [i].u = vertices [i].u;
    g3dpolyfx.vertices [i].v = vertices [i].v;
    g3dpolyfx.vertices [i].r = vertices [i].color.red;
    g3dpolyfx.vertices [i].g = vertices [i].color.green;
    g3dpolyfx.vertices [i].b = vertices [i].color.blue;
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
  g3d->StartPolygonFX (g3dpolyfx.mat_handle,
    	MixMode | CS_FX_GOURAUD);
  g3d->DrawPolygonFX (g3dpolyfx);
  g3d->FinishPolygonFX ();

  return true;
}

void csSprite2DMeshObject::GetObjectBoundingBox (csBox3& /*bbox*/, bool /*accurate*/)
{
  SetupObject ();
  //@@@ TODO
  //bbox = object_bbox;
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
    vertices [i].color.Set (0, 0, 0);
    vertices [i].color_init.Set (0, 0, 0);
  }
}

void csSprite2DMeshObject::Particle::UpdateLighting (iLight** lights,
    int num_lights, const csReversibleTransform& transform)
{
  csVector3 new_pos = transform.This2Other (part_pos);
  scfParent->UpdateLighting (lights, num_lights, new_pos);
}

void csSprite2DMeshObject::Particle::Draw (iRenderView* rview,
    const csReversibleTransform& transform)
{
  scfParent->SetupObject ();

  // Camera transformation for the single 'position' vector.
  csVector3 new_pos = transform.This2Other (part_pos);
  scfParent->cam = rview->GetCamera ()->GetTransform ().Other2This (new_pos);
  if (scfParent->cam.z < SMALL_Z) return;
  scfParent->Draw (rview, NULL);
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
}

void csSprite2DMeshObject::Particle::Rotate (float angle)
{
  csColoredVertices& vertices = scfParent->GetVertices ();
  for (int i = 0; i < vertices.Length (); i++)
    vertices[i].pos.Rotate (angle);
}

//----------------------------------------------------------------------

IMPLEMENT_IBASE (csSprite2DMeshObjectFactory)
  IMPLEMENTS_INTERFACE (iMeshObjectFactory)
  IMPLEMENTS_EMBEDDED_INTERFACE (iSprite2DFactoryState)
IMPLEMENT_IBASE_END

IMPLEMENT_EMBEDDED_IBASE (csSprite2DMeshObjectFactory::Sprite2DFactoryState)
  IMPLEMENTS_INTERFACE (iSprite2DFactoryState)
IMPLEMENT_EMBEDDED_IBASE_END

csSprite2DMeshObjectFactory::csSprite2DMeshObjectFactory ()
{
  CONSTRUCT_IBASE (NULL);
  CONSTRUCT_EMBEDDED_IBASE (scfiSprite2DFactoryState);
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
  return QUERY_INTERFACE (cm, iMeshObject);
}

//----------------------------------------------------------------------

IMPLEMENT_IBASE (csSprite2DMeshObjectType)
  IMPLEMENTS_INTERFACE (iMeshObjectType)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csSprite2DMeshObjectType)

EXPORT_CLASS_TABLE (spr2d)
  EXPORT_CLASS (csSprite2DMeshObjectType, "crystalspace.meshobj.spr2d",
    "Crystal Space Sprite2D Mesh Type")
EXPORT_CLASS_TABLE_END

csSprite2DMeshObjectType::csSprite2DMeshObjectType (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csSprite2DMeshObjectType::~csSprite2DMeshObjectType ()
{
}

bool csSprite2DMeshObjectType::Initialize (iSystem*)
{
  return true;
}

iMeshObjectFactory* csSprite2DMeshObjectType::NewFactory ()
{
  csSprite2DMeshObjectFactory* cm = new csSprite2DMeshObjectFactory ();
  return QUERY_INTERFACE (cm, iMeshObjectFactory);
}

