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

#include "cssysdef.h"
#include "csgeom/math3d.h"
#include "ball.h"
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
#include "qsqrt.h"

IMPLEMENT_IBASE (csBallMeshObject)
  IMPLEMENTS_INTERFACE (iMeshObject)
  IMPLEMENTS_EMBEDDED_INTERFACE (iBallState)
IMPLEMENT_IBASE_END

IMPLEMENT_EMBEDDED_IBASE (csBallMeshObject::BallState)
  IMPLEMENTS_INTERFACE (iBallState)
IMPLEMENT_EMBEDDED_IBASE_END

csBallMeshObject::csBallMeshObject ()
{
  CONSTRUCT_IBASE (NULL);
  CONSTRUCT_EMBEDDED_IBASE (scfiBallState);
  initialized = false;
  camera_cookie = 0;
  radiusx = radiusy = radiusz = 1;
  shiftx = shifty = shiftz = 0;
  verts_circle = 6;
  material = NULL;
  MixMode = 0;
  vis_cb = NULL;
  top_normals = NULL;
  top_mesh.vertices[0] = NULL;
  top_mesh.vertex_colors[0] = NULL;
  top_mesh.texels[0][0] = NULL;
  top_mesh.triangles = NULL;
  top_mesh.vertex_fog = NULL;
}

csBallMeshObject::~csBallMeshObject ()
{
  delete[] top_normals;
  delete[] top_mesh.vertices[0];
  delete[] top_mesh.vertex_colors[0];
  delete[] top_mesh.texels[0][0];
  delete[] top_mesh.triangles;
  delete[] top_mesh.vertex_fog;
}

void csBallMeshObject::GetTransformedBoundingBox (iTransformationManager* tranman,
    const csReversibleTransform& trans, csBox3& cbox)
{
  csTranCookie cur_cookie = tranman->GetCookie ();
  if (camera_cookie == cur_cookie)
  {
    cbox = camera_bbox;
    return;
  }
  camera_cookie = cur_cookie;

  camera_bbox.StartBoundingBox (trans * csVector3 (-radiusx/2, -radiusy/2, -radiusz/2));
  camera_bbox.AddBoundingVertexSmart (trans * csVector3 ( radiusx/2, -radiusy/2, -radiusz/2));
  camera_bbox.AddBoundingVertexSmart (trans * csVector3 (-radiusx/2,  radiusy/2, -radiusz/2));
  camera_bbox.AddBoundingVertexSmart (trans * csVector3 ( radiusx/2,  radiusy/2, -radiusz/2));
  camera_bbox.AddBoundingVertexSmart (trans * csVector3 (-radiusx/2, -radiusy/2,  radiusz/2));
  camera_bbox.AddBoundingVertexSmart (trans * csVector3 ( radiusx/2, -radiusy/2,  radiusz/2));
  camera_bbox.AddBoundingVertexSmart (trans * csVector3 (-radiusx/2,  radiusy/2,  radiusz/2));
  camera_bbox.AddBoundingVertexSmart (trans * csVector3 ( radiusx/2,  radiusy/2,  radiusz/2));

  cbox = camera_bbox;
}

void Perspective (const csVector3& v, csVector2& p, float fov,
    	float sx, float sy)
{
  float iz = fov / v.z;
  p.x = v.x * iz + sx;
  p.y = v.y * iz + sy;
}

float csBallMeshObject::GetScreenBoundingBox (iTransformationManager* tranman,
      float fov, float sx, float sy,
      const csReversibleTransform& trans, csBox2& sbox, csBox3& cbox)
{
  csVector2 oneCorner;

  GetTransformedBoundingBox (tranman, trans, cbox);

  // if the entire bounding box is behind the camera, we're done
  if ((cbox.MinZ () < 0) && (cbox.MaxZ () < 0))
  {
    return -1;
  }

  // Transform from camera to screen space.
  if (cbox.MinZ () <= 0)
  {
    // Sprite is very close to camera.
    // Just return a maximum bounding box.
    sbox.Set (-10000, -10000, 10000, 10000);
  }
  else
  {
    Perspective (cbox.Max (), oneCorner, fov, sx, sy);
    sbox.StartBoundingBox (oneCorner);
    csVector3 v (cbox.MinX (), cbox.MinY (), cbox.MaxZ ());
    Perspective (v, oneCorner, fov, sx, sy);
    sbox.AddBoundingVertexSmart (oneCorner);
    Perspective (cbox.Min (), oneCorner, fov, sx, sy);
    sbox.AddBoundingVertexSmart (oneCorner);
    v.Set (cbox.MaxX (), cbox.MaxY (), cbox.MinZ ());
    Perspective (v, oneCorner, fov, sx, sy);
    sbox.AddBoundingVertexSmart (oneCorner);
  }

  return cbox.MaxZ ();
}

void csBallMeshObject::GenerateSphere (int num, G3DTriangleMesh& mesh,
    csVector3*& normals)
{
  int num_vertices = 0;
  int num_triangles = 0;
  csVector3* vertices = new csVector3[10000];	// Temporary only
  csVector2* uvverts = new csVector2[10000];
  csTriangle* triangles = new csTriangle[10000];
  float radius = 1;

  int prev_verticesT[60];
  int prev_verticesB[60];
  float u, v;
  int i, j;

  // Number of degrees between layers.
  float radius_step = 180. / num;
  float vert_radius = radius;

  // Generate the first series of vertices (the outer circle).
  // Calculate u,v for them.
  for (j = 0 ; j < num ; j++)
  {
    float new_radius = radius;
    float new_height = 0;
    float angle = j*2.*radius_step * 2.*M_PI/360.;
    prev_verticesT[j] = num_vertices;
    prev_verticesB[j] = num_vertices;
    vertices[num_vertices].Set (
                         new_radius * cos (angle),
                         new_height,
                         new_radius * sin (angle));
    u = cos (angle) * .5 + .5;
    v = sin (angle) * .5 + .5;
    uvverts[num_vertices].Set (u, v);
    num_vertices++;
  }

  // Array with new vertex indices.
  int new_verticesT[60];         // @@@ HARDCODED == BAD == EASY!
  int new_verticesB[60];

  // First create the layered triangle strips.
  for (i = 1 ; i < num/2 ; i++)
  {
    //-----
    // First create a new series of vertices.
    //-----
    // Angle from the center to the new circle of vertices.
    float new_angle = i*radius_step * 2.*M_PI/360.;
    // Radius of the new circle of vertices.
    float new_radius = radius * cos (new_angle);
    // Height of the new circle of vertices.
    float new_height = vert_radius * sin (new_angle);
    // UV radius.
    float uv_radius = (1. - 2.*(float)i/(float)num) * .5;
    for (j = 0 ; j < num ; j++)
    {
      float angle = j*2.*radius_step * 2.*M_PI/360.;
      u = uv_radius * cos (angle) + .5;
      v = uv_radius * sin (angle) + .5;
      new_verticesT[j] = num_vertices;
      vertices[num_vertices].Set (
                         new_radius * cos (angle),
                         new_height,
                         new_radius * sin (angle));
      uvverts[num_vertices].Set (u, v);
      num_vertices++;

      new_verticesB[j] = num_vertices;
      vertices[num_vertices].Set (
                         new_radius * cos (angle),
                         -new_height,
                         new_radius * sin (angle));
      uvverts[num_vertices].Set (u, v);
      num_vertices++;
    }

    //-----
    // Now make the triangle strips.
    //-----
    for (j = 0 ; j < num ; j++)
    {
      triangles[num_triangles].c = prev_verticesT[j];
      triangles[num_triangles].b = new_verticesT[(j+1)%num];
      triangles[num_triangles].a = new_verticesT[j];
      num_triangles++;
      triangles[num_triangles].c = prev_verticesT[j];
      triangles[num_triangles].b = prev_verticesT[(j+1)%num];
      triangles[num_triangles].a = new_verticesT[(j+1)%num];
      num_triangles++;
      triangles[num_triangles].a = prev_verticesB[j];
      triangles[num_triangles].b = new_verticesB[(j+1)%num];
      triangles[num_triangles].c = new_verticesB[j];
      num_triangles++;
      triangles[num_triangles].a = prev_verticesB[j];
      triangles[num_triangles].b = prev_verticesB[(j+1)%num];
      triangles[num_triangles].c = new_verticesB[(j+1)%num];
      num_triangles++;
    }

    //-----
    // Copy the new vertex array to prev_vertices.
    //-----
    for (j = 0 ; j < num ; j++)
    {
      prev_verticesT[j] = new_verticesT[j];
      prev_verticesB[j] = new_verticesB[j];
    }
  }

  // Create the top and bottom vertices.
  int top_vertex = num_vertices;
  vertices[num_vertices].Set (0, vert_radius, 0);
  uvverts[num_vertices].Set (.5, .5);
  num_vertices++;
  int bottom_vertex = num_vertices;
  vertices[num_vertices].Set (0, -vert_radius, 0);
  uvverts[num_vertices].Set (.5, .5);
  num_vertices++;

  //-----
  // Make the top triangle fan.
  //-----
  for (j = 0 ; j < num ; j++)
  {
    triangles[num_triangles].c = top_vertex;
    triangles[num_triangles].b = prev_verticesT[j];
    triangles[num_triangles].a = prev_verticesT[(j+1)%num];
    num_triangles++;
  }

  //-----
  // Make the bottom triangle fan.
  //-----
  for (j = 0 ; j < num ; j++)
  {
    triangles[num_triangles].a = bottom_vertex;
    triangles[num_triangles].b = prev_verticesB[j];
    triangles[num_triangles].c = prev_verticesB[(j+1)%num];
    num_triangles++;
  }

  // Scale and shift all the vertices.
  normals = new csVector3[num_vertices];
  for (i = 0 ; i < num_vertices ; i++)
  {
    vertices[i].x *= radiusx/2;
    vertices[i].y *= radiusy/2;
    vertices[i].z *= radiusz/2;
    normals[i] = vertices[i].Unit ();
    vertices[i] += csVector3 (shiftx, shifty, shiftz);
  }

  // Setup the mesh and normal array.
  mesh.num_vertices = num_vertices;
  mesh.vertices[0] = new csVector3[num_vertices];
  memcpy (mesh.vertices[0], vertices, sizeof(csVector3)*num_vertices);
  mesh.texels[0][0] = new csVector2[num_vertices];
  memcpy (mesh.texels[0][0], uvverts, sizeof(csVector2)*num_vertices);
  mesh.vertex_colors[0] = new csColor[num_vertices];
  csColor* colors = mesh.vertex_colors[0];
  for (i = 0 ; i < num_vertices ; i++)
    colors[i].Set (1, 1, 1);
  mesh.vertex_fog = new G3DFogInfo[num_vertices];
  mesh.num_triangles = num_triangles;
  mesh.triangles = new csTriangle[num_triangles];
  memcpy (mesh.triangles, triangles, sizeof(csTriangle)*num_triangles);

  delete[] vertices;
  delete[] uvverts;
  delete[] triangles;
}

void csBallMeshObject::SetupObject ()
{
  if (!initialized)
  {
    initialized = true;
    GenerateSphere (verts_circle, top_mesh, top_normals);
    object_bbox.StartBoundingBox (csVector3 (-radiusx/2, -radiusy/2, -radiusz/2));
    object_bbox.AddBoundingVertexSmart (csVector3 ( radiusx/2, -radiusy/2, -radiusz/2));
    object_bbox.AddBoundingVertexSmart (csVector3 (-radiusx/2,  radiusy/2, -radiusz/2));
    object_bbox.AddBoundingVertexSmart (csVector3 ( radiusx/2,  radiusy/2, -radiusz/2));
    object_bbox.AddBoundingVertexSmart (csVector3 (-radiusx/2, -radiusy/2,  radiusz/2));
    object_bbox.AddBoundingVertexSmart (csVector3 ( radiusx/2, -radiusy/2,  radiusz/2));
    object_bbox.AddBoundingVertexSmart (csVector3 (-radiusx/2,  radiusy/2,  radiusz/2));
    object_bbox.AddBoundingVertexSmart (csVector3 ( radiusx/2,  radiusy/2,  radiusz/2));
    top_mesh.num_materials = 1;
    top_mesh.morph_factor = 0;
    top_mesh.num_vertices_pool = 1;
    top_mesh.do_morph_texels = false;
    top_mesh.do_morph_colors = false;
    top_mesh.vertex_mode = G3DTriangleMesh::VM_WORLDSPACE;
  }
}

bool csBallMeshObject::DrawTest (iRenderView* rview, iMovable* movable)
{
  SetupObject ();
  iGraphics3D* g3d = rview->GetGraphics3D ();
  iCamera* camera = rview->GetCamera ();
  iEngine* engine = rview->GetEngine ();
  iTransformationManager* tranman = engine->GetTransformationManager ();

  // First create the transformation from object to camera space directly:
  //   W = Mow * O - Vow;
  //   C = Mwc * (W - Vwc)
  // ->
  //   C = Mwc * (Mow * O - Vow - Vwc)
  //   C = Mwc * Mow * O - Mwc * (Vow + Vwc)
  csReversibleTransform tr_o2c = camera->GetTransform ()
    	* movable->GetFullTransform ().GetInverse ();
  float fov = camera->GetFOV ();
  float shiftx = camera->GetShiftX ();
  float shifty = camera->GetShiftY ();

  // Test visibility of entire ball by clipping bounding box against clipper.
  // There are three possibilities:
  //	1. box is not visible -> ball is not visible.
  //	2. box is entirely visible -> ball is visible and need not be clipped.
  //	3. box is partially visible -> ball is visible and needs to be clipped
  //	   if rview has do_clip_plane set to true.
  csBox2 sbox;
  csBox3 cbox;
  if (GetScreenBoundingBox (tranman, fov, shiftx, shifty, tr_o2c, sbox, cbox) < 0)
    return false;
  bool do_clip;
  if (rview->ClipBBox (sbox, cbox, do_clip) == false)
    return false;

  iClipper2D* clipper = rview->GetClipper ();
  g3d->SetObjectToCamera (&tr_o2c);
  // @@@ This should only be done when aspect changes...
  g3d->SetPerspectiveAspect (fov);
  g3d->SetClipper (clipper->GetClipPoly (), clipper->GetNumVertices ());
  top_mesh.do_clip = do_clip;
  top_mesh.do_mirror = camera->IsMirrored ();
  return true;
}

void csBallMeshObject::UpdateLighting (iLight** lights, int num_lights,
    iMovable* movable)
{
  SetupObject ();

  int i, l;
  csColor* colors = top_mesh.vertex_colors[0];

  // Set all colors to ambient light (@@@ NEED TO GET AMBIENT!)
  for (i = 0 ; i < top_mesh.num_vertices ; i++)
    colors[i].Set (0, 0, 0);

  // Do the lighting.
  csVector3 obj_center (0);
  csReversibleTransform trans = movable->GetFullTransform ();
  csVector3 wor_center = trans.This2Other (obj_center);
  csColor color;
  for (l = 0 ; l < num_lights ; l++)
  {
    iLight* li = lights[l];
    // Compute light position in object coordinates
    csVector3 wor_light_pos = li->GetCenter ();
    float wor_sq_dist = csSquaredDist::PointPoint (wor_light_pos, wor_center);
    if (wor_sq_dist >= li->GetSquaredRadius ()) continue;

    csVector3 obj_light_pos = trans.Other2This (wor_light_pos);
    float obj_sq_dist = csSquaredDist::PointPoint (obj_light_pos, obj_center);
    float in_obj_dist = qisqrt (obj_sq_dist);

    csVector3 obj_light_dir = (obj_light_pos - obj_center);

    csColor light_color = li->GetColor () * (256. / NORMAL_LIGHT_LEVEL)
      * li->GetBrightnessAtDistance (qsqrt (wor_sq_dist));

    for (i = 0 ; i < top_mesh.num_vertices ; i++)
    {
      csVector3 normal = top_normals[i];
      float cosinus;
      if (obj_sq_dist < SMALL_EPSILON) cosinus = 1;
      else cosinus = obj_light_dir * normal;

      if (cosinus > 0)
      {
        color = light_color;
        if (obj_sq_dist >= SMALL_EPSILON) cosinus *= in_obj_dist;
        if (cosinus < 1) color *= cosinus;
	colors[i] += color;
      }
    }
  }

  // Clamp all vertex colors to 2.
  for (i = 0 ; i < top_mesh.num_vertices ; i++)
    colors[i].Clamp (2., 2., 2.);
}

bool csBallMeshObject::Draw (iRenderView* rview, iMovable* /*movable*/)
{
// @@@ TODO:
//     - Z fill vs Z use
  if (!material)
  {
    printf ("INTERNAL ERROR: ball used without material!\n");
    return false;
  }
  iMaterialHandle* mat = material->GetMaterialHandle ();
  if (!mat)
  {
    printf ("INTERNAL ERROR: ball used without valid material handle!\n");
    return false;
  }

  if (vis_cb) vis_cb (this, rview, vis_cbData);

  iGraphics3D* g3d = rview->GetGraphics3D ();

  // Prepare for rendering.
  g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_USE);

  material->Visit ();
  top_mesh.mat_handle[0] = mat;
  top_mesh.use_vertex_color = true;
  top_mesh.fxmode = MixMode | CS_FX_GOURAUD;
  rview->CalculateFogMesh (g3d->GetObjectToCamera (), top_mesh);
  g3d->DrawTriangleMesh (top_mesh);

  return true;
}

void csBallMeshObject::GetObjectBoundingBox (csBox3& bbox, bool /*accurate*/)
{
  SetupObject ();
  bbox = object_bbox;
}

//----------------------------------------------------------------------

IMPLEMENT_IBASE (csBallMeshObjectFactory)
  IMPLEMENTS_INTERFACE (iMeshObjectFactory)
IMPLEMENT_IBASE_END

csBallMeshObjectFactory::csBallMeshObjectFactory ()
{
  CONSTRUCT_IBASE (NULL);
}

csBallMeshObjectFactory::~csBallMeshObjectFactory ()
{
}

iMeshObject* csBallMeshObjectFactory::NewInstance ()
{
  csBallMeshObject* cm = new csBallMeshObject ();
  return QUERY_INTERFACE (cm, iMeshObject);
}

//----------------------------------------------------------------------

IMPLEMENT_IBASE (csBallMeshObjectType)
  IMPLEMENTS_INTERFACE (iMeshObjectType)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csBallMeshObjectType)

EXPORT_CLASS_TABLE (ball)
  EXPORT_CLASS (csBallMeshObjectType, "crystalspace.mesh.object.ball",
    "Crystal Space Ball Mesh Type")
EXPORT_CLASS_TABLE_END

csBallMeshObjectType::csBallMeshObjectType (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csBallMeshObjectType::~csBallMeshObjectType ()
{
}

bool csBallMeshObjectType::Initialize (iSystem*)
{
  return true;
}

iMeshObjectFactory* csBallMeshObjectType::NewFactory ()
{
  csBallMeshObjectFactory* cm = new csBallMeshObjectFactory ();
  return QUERY_INTERFACE (cm, iMeshObjectFactory);
}

