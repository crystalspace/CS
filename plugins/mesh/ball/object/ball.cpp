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
#include "csgfx/renderbuffer.h"
#include "csgeom/math3d.h"
#include "csgeom/math.h"
#include "iengine/movable.h"
#include "iengine/rview.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/material.h"
#include "iengine/material.h"
#include "iengine/camera.h"
#include "iengine/sector.h"
#include "igeom/clip2d.h"
#include "iengine/engine.h"
#include "iengine/light.h"
#include "iutil/objreg.h"
#include "ball.h"

#include "csqsqrt.h"
#include "iutil/strset.h"

//#define BALL_DEBUG

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csBallMeshObject)
  SCF_IMPLEMENTS_INTERFACE (iMeshObject)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iObjectModel)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iBallState)
  {
    static scfInterfaceID iPolygonMesh_scfID = (scfInterfaceID)-1;		
    if (iPolygonMesh_scfID == (scfInterfaceID)-1)				
      iPolygonMesh_scfID = iSCF::SCF->GetInterfaceID ("iPolygonMesh");		
    if (iInterfaceID == iPolygonMesh_scfID &&				
      scfCompatibleVersion(iVersion, scfInterface<iPolygonMesh>::GetVersion()))
    {
#ifdef CS_DEBUG
      printf ("Deprecated feature use: iPolygonMesh queried from Ball "
	"object; use iMeshObject->GetObjectModel()->"
	"GetPolygonMeshColldet() instead.\n");
#endif
      (&scfiPolygonMesh)->IncRef ();						
      return CS_STATIC_CAST(iPolygonMesh*, &scfiPolygonMesh);				
    }
  }
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csBallMeshObject::BallState)
  SCF_IMPLEMENTS_INTERFACE (iBallState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csBallMeshObject::ObjectModel)
  SCF_IMPLEMENTS_INTERFACE (iObjectModel)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE(csBallMeshObject::PolyMesh)
  SCF_IMPLEMENTS_INTERFACE(iPolygonMesh)
SCF_IMPLEMENT_IBASE_END

csBallMeshObject::csBallMeshObject (iMeshObjectFactory* factory)
{
  SCF_CONSTRUCT_IBASE (0);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiObjectModel);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiBallState);

  csBallMeshObject::factory = factory;

  scfiPolygonMesh.SetBall (this);
  scfiObjectModel.SetPolygonMeshBase (&scfiPolygonMesh);
  scfiObjectModel.SetPolygonMeshColldet (&scfiPolygonMesh);
  scfiObjectModel.SetPolygonMeshViscull (0);
  scfiObjectModel.SetPolygonMeshShadows (0);

  logparent = 0;
  initialized = false;
  cur_cameranr = -1;
  cur_movablenr = -1;
  radiusx = radiusy = radiusz = 1;
  max_radius.Set (0.0f, 0.0f, 0.0f);
  shift.Set (0.0f, 0.0f, 0.0f);
  verts_circle = 6;
  material = 0;
  MixMode = 0;
  vis_cb = 0;
  top_normals = 0;
  ball_vertices = 0;
  ball_colors = 0;
  ball_texels = 0;
  ball_indices = 0;
  reversed = false;
  toponly = false;
  cyl_mapping = false;
  do_lighting = true;
  generated_colors = false;
  color.red = 0.0f;
  color.green = 0.0f;
  color.blue = 0.0f;
  current_lod = 1.0f;
  current_features = 0;
  polygons = 0;
  
  g3d = CS_QUERY_REGISTRY (
  	((csBallMeshObjectFactory*)factory)->object_reg, iGraphics3D);
  
  ball_vertices_dirty_flag = false;
  ball_texels_dirty_flag = false;
  ball_normals_dirty_flag = false;
  ball_colors_dirty_flag = false;
  ball_triangle_dirty_flag = false;

  bufferHolder.AttachNew (new csRenderBufferHolder);
}

csBallMeshObject::~csBallMeshObject ()
{
  if (vis_cb) vis_cb->DecRef ();
  delete[] top_normals;
  delete[] ball_vertices;
  delete[] ball_colors;
  delete[] ball_texels;
  delete[] polygons;
  delete[] ball_indices;

  SCF_DESTRUCT_EMBEDDED_IBASE (scfiObjectModel);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiBallState);
  SCF_DESTRUCT_IBASE();
}

void csBallMeshObject::GetTransformedBoundingBox (long cameranr,
	long movablenr, const csReversibleTransform& trans, csBox3& cbox)
{
  if (cur_cameranr == cameranr && cur_movablenr == movablenr)
  {
    cbox = camera_bbox;
    return;
  }
  cur_cameranr = cameranr;
  cur_movablenr = movablenr;

  camera_bbox.StartBoundingBox (
    trans * csVector3 (-radiusx, -radiusy, -radiusz));
  camera_bbox.AddBoundingVertexSmart (
    trans * csVector3 ( radiusx,  radiusy,  radiusz));
  camera_bbox.AddBoundingVertexSmart (
    trans * csVector3 (-radiusx,  radiusy,  radiusz));
  camera_bbox.AddBoundingVertexSmart (
    trans * csVector3 ( radiusx, -radiusy,  radiusz));
  camera_bbox.AddBoundingVertexSmart (
    trans * csVector3 ( radiusx,  radiusy, -radiusz));
  camera_bbox.AddBoundingVertexSmart (
    trans * csVector3 ( radiusx, -radiusy, -radiusz));
  camera_bbox.AddBoundingVertexSmart (
    trans * csVector3 (-radiusx,  radiusy, -radiusz));
  camera_bbox.AddBoundingVertexSmart (
    trans * csVector3 (-radiusx, -radiusy,  radiusz));

  cbox = camera_bbox;
}

static void Perspective (const csVector3& v, csVector2& p, float fov,
    	float sx, float sy)
{
  float iz = fov / v.z;
  p.x = v.x * iz + sx;
  p.y = v.y * iz + sy;
}

void csBallMeshObject::SetRadius (float radiusx, float radiusy, float radiusz)
{
  initialized = false;
  csBallMeshObject::radiusx = radiusx;
  csBallMeshObject::radiusy = radiusy;
  csBallMeshObject::radiusz = radiusz;
  max_radius.Set (radiusx, radiusy, radiusz);
  scfiObjectModel.ShapeChanged ();
}

float csBallMeshObject::GetScreenBoundingBox (long cameranr,
      long movablenr, float fov, float sx, float sy,
      const csReversibleTransform& trans, csBox2& sbox, csBox3& cbox)
{
  csVector2 oneCorner;

  GetTransformedBoundingBox (cameranr, movablenr, trans, cbox);

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
    sbox.Set (-10000.0f, -10000.0f, 10000.0f, 10000.0f);
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

void csBallMeshObject::GenerateSphere (int num)
{
  int num_vertices = 0;
  int num_triangles = 0;
  csVector3* vertices = new csVector3[10000];	// Temporary only
  csVector2* uvverts = new csVector2[10000];
  csTriangle* triangles = new csTriangle[10000];
  float radius = 1.0f;

  int prev_verticesT[60];
  int prev_verticesB[60];
  float u, v;
  int i, j;

  // Number of degrees between layers.
  float radius_step = 180.0f / num;
  float vert_radius = radius;

  // If cylindrical mapping is used we duplicate the last column of
  // vertices. That is because we need to connect the two sides of the
  // texture and a vertex can only have one texture coordinate.
  int num2 = num;
  if (cyl_mapping) num2++;

  // Generate the first series of vertices (the outer circle).
  // Calculate u,v for them.
  for (j = 0; j < num2; j++)
  {
    float new_radius = radius;
    float new_height = 0.0f;
    float angle = j * 2.0f * radius_step * TWO_PI / 360.0f;
    prev_verticesT[j] = num_vertices;
    prev_verticesB[j] = num_vertices;
    vertices[num_vertices].Set (new_radius * (float) cos (angle),
      new_height, new_radius * (float) sin (angle));

    if (cyl_mapping)
    {
      u = float (j) / float (num);
      v = 0.5f;
    }
    else
    {
      u = (float) cos (angle) * 0.5f + 0.5f;
      v = (float) sin (angle) * 0.5f + 0.5f;
    }
    uvverts[num_vertices].Set (u, v);
    num_vertices++;
  }

  // Array with new vertex indices.
  int new_verticesT[60];         // @@@ HARDCODED == BAD == EASY!
  int new_verticesB[60];

  // First create the layered triangle strips.
  for (i = 1; i < (num / 2); i++)
  {
    //-----
    // First create a new series of vertices.
    //-----
    // Angle from the center to the new circle of vertices.
    float new_angle = i * radius_step * TWO_PI / 360.0f;
    // Radius of the new circle of vertices.
    float new_radius = radius * (float) cos (new_angle);
    // Height of the new circle of vertices.
    float new_height = vert_radius * (float) sin (new_angle);
    // UV radius.
    float uv_radius = (1.0f - 2.0f * (float) i / (float)num) * 0.5f;
    for (j = 0; j < num2; j++)
    {
      float angle = j * 2.0f * radius_step * TWO_PI / 360.0f;
      if (cyl_mapping)
      {
        u = float (j) / float (num);
        v = 1.0f - float (i + num / 2) / float (num);
      }
      else
      {
        u = uv_radius * (float) cos (angle) + 0.5f;
        v = uv_radius * (float) sin (angle) + 0.5f;
      }
      new_verticesT[j] = num_vertices;
      vertices[num_vertices].Set (new_radius * (float) cos (angle),
        new_height, new_radius * (float) sin (angle));
      uvverts[num_vertices].Set (u, v);
      num_vertices++;

      if (!toponly)
      {
        new_verticesB[j] = num_vertices;
        vertices[num_vertices].Set (new_radius * (float) cos (angle),
          -new_height, new_radius * (float) sin (angle));
        
        if (cyl_mapping) v = 1.0f - v;
        uvverts[num_vertices].Set (u, v);
        num_vertices++;
      }
    }

    //-----
    // Now make the triangle strips.
    //-----
    for (j = 0; j < num; j++)
    {
      int j1num;
      if (cyl_mapping) j1num = j+1;
      else j1num = (j+1)%num;
      triangles[num_triangles].c = prev_verticesT[j];
      triangles[num_triangles].b = new_verticesT[j1num];
      triangles[num_triangles].a = new_verticesT[j];
      num_triangles++;
      triangles[num_triangles].c = prev_verticesT[j];
      triangles[num_triangles].b = prev_verticesT[j1num];
      triangles[num_triangles].a = new_verticesT[j1num];
      num_triangles++;
      if (!toponly)
      {
        triangles[num_triangles].a = prev_verticesB[j];
        triangles[num_triangles].b = new_verticesB[j1num];
        triangles[num_triangles].c = new_verticesB[j];
        num_triangles++;
        triangles[num_triangles].a = prev_verticesB[j];
        triangles[num_triangles].b = prev_verticesB[j1num];
        triangles[num_triangles].c = new_verticesB[j1num];
        num_triangles++;
      }
    }

    //-----
    // Copy the new vertex array to prev_vertices.
    //-----
    for (j = 0 ; j < num2 ; j++)
    {
      prev_verticesT[j] = new_verticesT[j];
      if (!toponly) prev_verticesB[j] = new_verticesB[j];
    }
  }

  // Create the top and bottom vertices.
  int top_vertex = num_vertices;
  vertices[num_vertices].Set (0.0f, vert_radius, 0.0f);
  if (cyl_mapping)
    uvverts[num_vertices].Set (0.5f, 0.0f);
  else
    uvverts[num_vertices].Set (0.5f, 0.5f);
  num_vertices++;
  int bottom_vertex = 0;
  if (!toponly)
  {
    bottom_vertex = num_vertices;
    vertices[num_vertices].Set (0.0f, -vert_radius, 0.0f);
    if (cyl_mapping)
      uvverts[num_vertices].Set (0.5f, 1.0f);
    else
      uvverts[num_vertices].Set (0.5f, 0.5f);
    num_vertices++;
  }

  //-----
  // Make the top triangle fan.
  //-----
  for (j = 0 ; j < num ; j++)
  {
    int j1num;
    if (cyl_mapping) j1num = j+1;
    else j1num = (j+1)%num;
    triangles[num_triangles].c = top_vertex;
    triangles[num_triangles].b = prev_verticesT[j];
    triangles[num_triangles].a = prev_verticesT[j1num];
    num_triangles++;
  }

  //-----
  // Make the bottom triangle fan.
  //-----
  if (!toponly)
    for (j = 0 ; j < num ; j++)
    {
      int j1num;
      if (cyl_mapping) j1num = j+1;
      else j1num = (j+1)%num;
      triangles[num_triangles].a = bottom_vertex;
      triangles[num_triangles].b = prev_verticesB[j];
      triangles[num_triangles].c = prev_verticesB[j1num];
      num_triangles++;
    }

  // Scale and shift all the vertices.
  top_normals = new csVector3[num_vertices];
  for (i = 0 ; i < num_vertices ; i++)
  {
    vertices[i].x *= radiusx;
    vertices[i].y *= radiusy;
    vertices[i].z *= radiusz;
    top_normals[i] = vertices[i].Unit ();
    vertices[i] += shift;
  }

  // Swap all triangles if needed.
  if (reversed)
  {
    for (i = 0 ; i < num_triangles ; i++)
    {
      int s = triangles[i].a;
      triangles[i].a = triangles[i].c;
      triangles[i].c = s;
    }
  }

  // Setup the mesh and normal array.
  num_ball_vertices = num_vertices;
  ball_vertices = new csVector3[num_vertices];
  memcpy (ball_vertices, vertices, sizeof(csVector3)*num_vertices);
  ball_texels = new csVector2[num_vertices];
  memcpy (ball_texels, uvverts, sizeof(csVector2)*num_vertices);
  ball_colors = new csColor[num_vertices];
  for (i = 0 ; i < num_vertices ; i++)
    ball_colors[i].Set (1.0f, 1.0f, 1.0f);

  ball_triangles = num_triangles;
  ball_indices = new unsigned int[num_triangles*3];
  memcpy (ball_indices, triangles, sizeof(csTriangle)*num_triangles);

  ball_vertices_dirty_flag = true;
  ball_texels_dirty_flag = true;
  ball_normals_dirty_flag = true;
  ball_colors_dirty_flag = true;
  ball_triangle_dirty_flag = true;

  delete[] vertices;
  delete[] uvverts;
  delete[] triangles;
}


void csBallMeshObject::SetupObject ()
{
  if (!initialized)
  {
    initialized = true;

    delete[] ball_indices;
    ball_indices = 0;
   
    delete[] top_normals;
    delete[] ball_vertices;
    delete[] ball_colors;
    delete[] ball_texels;
    
    top_normals = 0;
    ball_vertices = 0;
    ball_colors = 0;
    ball_texels = 0;

    GenerateSphere (verts_circle);
    object_bbox.StartBoundingBox (
      csVector3 (-radiusx, -radiusy, -radiusz) + shift);
    object_bbox.AddBoundingVertexSmart (
      csVector3 ( radiusx,  radiusy,  radiusz) + shift);
  }
}

void csBallMeshObject::UpdateBuffers()
{
  if (ball_vertices_dirty_flag)
  {
    vertex_buffer = csRenderBuffer::CreateRenderBuffer (
      num_ball_vertices, CS_BUF_STATIC, 
      CS_BUFCOMP_FLOAT, 3);
    ball_vertices_dirty_flag = false;
    vertex_buffer->CopyInto (ball_vertices, num_ball_vertices);

    bufferHolder->SetRenderBuffer (CS_BUFFER_POSITION, vertex_buffer);
  }
  if (ball_texels_dirty_flag)
  {
    texel_buffer = csRenderBuffer::CreateRenderBuffer (
      num_ball_vertices, CS_BUF_STATIC, 
      CS_BUFCOMP_FLOAT, 2);
    ball_texels_dirty_flag = false;
    texel_buffer->CopyInto (ball_texels, num_ball_vertices);

    bufferHolder->SetRenderBuffer (CS_BUFFER_TEXCOORD0, texel_buffer);
  }
  if (ball_normals_dirty_flag)
  {
    normal_buffer = csRenderBuffer::CreateRenderBuffer (
      num_ball_vertices, CS_BUF_STATIC,
      CS_BUFCOMP_FLOAT, 3);
    ball_normals_dirty_flag = false;
    normal_buffer->CopyInto (top_normals, num_ball_vertices);

    bufferHolder->SetRenderBuffer (CS_BUFFER_NORMAL, normal_buffer);
  }
  if (ball_colors_dirty_flag)
  {
    color_buffer = csRenderBuffer::CreateRenderBuffer (
      num_ball_vertices, CS_BUF_STATIC,
      CS_BUFCOMP_FLOAT, 3);
    ball_colors_dirty_flag = false;
    color_buffer->CopyInto (ball_colors, num_ball_vertices);

    bufferHolder->SetRenderBuffer (CS_BUFFER_COLOR, color_buffer);
  }
  if (ball_triangle_dirty_flag)
  {
    index_buffer = csRenderBuffer::CreateIndexRenderBuffer (
      ball_triangles*3, CS_BUF_STATIC,
      CS_BUFCOMP_UNSIGNED_INT, 0, num_ball_vertices - 1);
    ball_triangle_dirty_flag = false;
    index_buffer->CopyInto (ball_indices, ball_triangles *3);

    bufferHolder->SetRenderBuffer (CS_BUFFER_INDEX, index_buffer);
  }
}

csRenderMesh **csBallMeshObject::GetRenderMeshes (int &num, iRenderView* rview, 
                                                  iMovable* movable,
						  uint32 frustum_mask)
{
  SetupObject ();

  num = 0;

  iCamera* camera = rview->GetCamera ();

  int clip_portal, clip_plane, clip_z_plane;
  rview->CalculateClipSettings (frustum_mask, clip_portal, clip_plane,
  	clip_z_plane);
  
  if (((csBallMeshObjectFactory*)factory)->light_mgr)
  {
    const csArray<iLight*>& relevant_lights
      = ((csBallMeshObjectFactory*)factory)->light_mgr
      ->GetRelevantLights (logparent, -1, false);
    UpdateLighting (relevant_lights, movable);
  }

  iMaterialWrapper* mater = material;

  if (!mater)
  {
    printf ("INTERNAL ERROR: mesh used without material!\n");
    return 0;
  }
  UpdateBuffers ();
  mater->Visit ();

  bool rmCreated;
  csRenderMesh*& meshPtr = rmHolder.GetUnusedMesh (rmCreated,
    rview->GetCurrentFrameNumber ());
  
  const csReversibleTransform o2wt = movable->GetFullTransform ();
  const csVector3 &wo = o2wt.GetOrigin ();

  meshPtr->mixmode = MixMode;
  meshPtr->clip_portal = clip_portal;
  meshPtr->clip_plane = clip_plane;
  meshPtr->clip_z_plane = clip_z_plane;
  meshPtr->do_mirror = camera->IsMirrored ();  
  meshPtr->meshtype = CS_MESHTYPE_TRIANGLES;
  meshPtr->indexstart = 0;
  meshPtr->indexend = ball_triangles * 3;
  meshPtr->material = mater;
  meshPtr->worldspace_origin = wo;
  if (rmCreated)
  {
    meshPtr->buffers = bufferHolder;
    meshPtr->variablecontext.AttachNew (new csShaderVariableContext);
  }
  meshPtr->variablecontext->GetVariableAdd (((csBallMeshObjectFactory*)factory)->string_object2world)->
    SetValue (o2wt);
  meshPtr->geometryInstance = (void*)factory;
  
  num = 1;
  return &meshPtr;

}


void csBallMeshObject::UpdateLighting (const csArray<iLight*>& lights,
    iMovable* movable)
{
  if (generated_colors) return;

  ball_colors_dirty_flag = true;

  int i, l;
  csColor* colors = ball_colors;

  // Set all colors to ambient light.
  csColor col;
  if (((csBallMeshObjectFactory*)factory)->engine)
  {
    ((csBallMeshObjectFactory*)factory)->engine->GetAmbientLight (col);
    col += color;
    iSector* sect = movable->GetSectors ()->Get (0);
    if (sect)
      col += sect->GetDynamicAmbientLight ();
  }
  else
  {
    col = color;
  }
  for (i = 0 ; i < num_ball_vertices ; i++)
    colors[i] = col;

  if (!do_lighting) return;
    // @@@ it is not efficient to do this all the time.

  // Do the lighting.
  csReversibleTransform trans = movable->GetFullTransform ();
  // the object center in world coordinates. "0" because the object
  // center in object space is obviously at (0,0,0).
  csColor color;
  int num_lights = (int)lights.Length ();
  for (l = 0 ; l < num_lights ; l++)
  {
    iLight* li = lights[l];
    // Compute light position in object coordinates
    csVector3 wor_light_pos = li->GetCenter ();
    csVector3 obj_light_pos = trans.Other2This (wor_light_pos);
    float obj_sq_dist = csSquaredDist::PointPoint (obj_light_pos, 0);
    if (obj_sq_dist >= csSquare (li->GetDirectionalCutoffRadius ())) continue;
    float in_obj_dist = (obj_sq_dist >= SMALL_EPSILON)
    	? csQisqrt (obj_sq_dist)
	: 1.0f;

    csColor light_color = li->GetColor () * (256.0f / CS_NORMAL_LIGHT_LEVEL)
      * li->GetBrightnessAtDistance (csQsqrt (obj_sq_dist));

    for (i = 0; i < num_ball_vertices; i++)
    {
      csVector3 normal = top_normals[i];
      float cosinus;
      if (obj_sq_dist < SMALL_EPSILON) cosinus = 1;
      else cosinus = obj_light_pos * normal; 
      // because the vector from the object center to the light center
      // in object space is equal to the position of the light

      if (cosinus > 0)
      {
        color = light_color;
        if (obj_sq_dist >= SMALL_EPSILON) cosinus *= in_obj_dist;
        if (cosinus < 1.0f) color *= cosinus;
	colors[i] += color;
      }
    }
  }

  // Clamp all vertex colors to 2.
  for (i = 0 ; i < num_ball_vertices ; i++)
    colors[i].Clamp (2.0f, 2.0f, 2.0f);
}


void csBallMeshObject::GetObjectBoundingBox (csBox3& bbox)
{
  SetupObject ();
  bbox = object_bbox;
}

void csBallMeshObject::SetObjectBoundingBox (const csBox3& bbox)
{
  object_bbox = bbox;
  scfiObjectModel.ShapeChanged ();
}

void csBallMeshObject::HardTransform (const csReversibleTransform& t)
{
  shift = t.This2Other (shift);
  initialized = false;
  scfiObjectModel.ShapeChanged ();
}

bool csBallMeshObject::HitBeamOutline (const csVector3& start,
  const csVector3& end, csVector3& isect, float* pr)
{
  // This is now closer to an outline hitting method. It will
  // return as soon as it touches any triangle in the ball, and
  // will be a bit faster than its more accurate cousin (below).

  csSegment3 seg (start, end);
  int i, max;

  csTriangle *tr = (csTriangle*)ball_indices;
  max = ball_triangles;

  csVector3 *vrt = ball_vertices;
  for (i = 0 ; i < max ; i++)
  {
    if (csIntersect3::SegmentTriangle (seg, vrt[tr[i].a], vrt[tr[i].b],
    	vrt[tr[i].c], isect))
    {
      if (pr) *pr = csQsqrt (csSquaredDist::PointPoint (start, isect) /
        csSquaredDist::PointPoint (start, end));

      return true;
    }
  }
  return false;
}

bool csBallMeshObject::HitBeamObject(const csVector3& start,
  const csVector3& end, csVector3& isect, float *pr, int* polygon_idx)
{
  if (polygon_idx) *polygon_idx = -1;
  // This is the slow version. Use for an accurate hit on the object.
  // It will cycle through every triangle in the mesh serching for the
  // closest intersection. Slower, but returns the closest hit.
  // Usegae is optional.

  csSegment3 seg (start, end);
  int i, max;
  float tot_dist = csSquaredDist::PointPoint (start, end);
  float dist, temp;
  float itot_dist = 1.0f / tot_dist;
  dist = temp = tot_dist;
  csVector3 *vrt = ball_vertices, tmp;

  csTriangle *tr = (csTriangle*)ball_indices;
  max = ball_triangles;

  for (i = 0; i < max; i++)
  {
    if (csIntersect3::SegmentTriangle (seg, vrt[tr[i].a], vrt[tr[i].b],
    	vrt[tr[i].c], tmp))
    {
      temp = csSquaredDist::PointPoint (start, tmp);
      if (temp < dist)
      {
        isect = tmp;
	dist = temp;
      }
    }
  }
  if (pr) *pr = csQsqrt(dist * itot_dist);
  if (dist >= tot_dist)
      return false;
  return true;
}

/// interpolate a gradient
static void GetGradientColor(float **gradient, float val, csColor& col)
{
  if(gradient == 0 || gradient[0] == 0)
  {
    col.Set(0.0f, 0.0f, 0.0f);
    return;
  }
  int entry = 0;
  while( gradient[entry] && (gradient[entry][0] < val) ) entry++;
  if(gradient[entry] == 0)
  {
    if(entry>0) col.Set( gradient[entry-1][1], gradient[entry-1][2],
      gradient[entry-1][3]);
    else col.Set(0.0f, 0.0f, 0.0f);
    return;
  }
  if(entry <= 0)
  {
    col.Set( gradient[0][1], gradient[0][2], gradient[0][3]);
    return;
  }
  // entry at least 1 and exists - interpolate between entry-1 and entry
  // assumes sorted entries.
  CS_ASSERT( gradient[entry][0] >= gradient[entry-1][0] );
  float sc1 = val - gradient[entry-1][0];
  float sc2 = gradient[entry][0] - val;
  float invdist = 1.0f / (gradient[entry][0] - gradient[entry-1][0]);
  sc1 *= invdist;
  sc2 *= invdist;

  col.red = gradient[entry-1][1] * sc2 + gradient[entry][1] * sc1;
  col.green = gradient[entry-1][2] * sc2 + gradient[entry][2] * sc1;
  col.blue = gradient[entry-1][3] * sc2 + gradient[entry][3] * sc1;
  //printf("val %g, entry %d gives %g,%g,%g\n", val, entry,
   // col.red, col.green,  col.blue);
}

void csBallMeshObject::ApplyVertGradient(float horizon_height,
  float zenith_height, float**gradient)
{
  SetupObject();
  CS_ASSERT( zenith_height > horizon_height );
  //printf("ApplyVertGradient\n");
  float invdist = 1.0f / (zenith_height - horizon_height);
  csColor color(0,0,0);
  for(int i=0; i<num_ball_vertices; i++)
  {
    float val = (ball_vertices[i].y - horizon_height) * invdist;
    GetGradientColor(gradient, val, color);
    ball_colors[i] = color;
  }
  generated_colors = true;
}

void csBallMeshObject::ApplyLightSpot(const csVector3& position, float size,
  float**gradient)
{
  /// compute position on the sphere.
  csVector3 pos = position - shift;
  float len = pos.Norm();
  pos = pos * (radiusy/len);
  pos += shift;
  /// see if gradient is given
  float sun1[] = {0.0f, 1.0f, 1.0f, 0.6f};
  float sun2[] = {1.0f, 1.0f, 0.8f, 0.6f};
  float *sungrad[] = {sun1, sun2, 0};
  float **grad = sungrad;
  if(gradient) grad = gradient;
  /// compute the max distance for the lightspot given radius.
  float maxdist = radiusy * size / 1.4f;
  //float maxdist = radiusy * radiusy * size / 2.4;
  /// apply the spot
  csColor color(0,0,0);
  for(int i=0; i<num_ball_vertices; i++)
  {
    float val = (ball_vertices[i] - pos).Norm() / maxdist;
    GetGradientColor(grad, val, color);
    float apply = 1.0f - val;
    if(apply <= 0.0f) continue;
    //if(apply >= 1.0) apply = 1.0;
    //ball_colors[i] = ball_colors[i]*(1-apply) + color*apply;
    ball_colors[i] += color*apply;
    ball_colors[i].Clamp(2.0f, 2.0f, 2.0f);
    ball_colors[i].ClampDown();
  }
  generated_colors = true;
}


void csBallMeshObject::PaintSky (float time, float **dayvert, float **nightvert,
  float **topsun, float **sunset)
{
  // apply defaults if needed
  float sky1[] = {0.0f, 0.5f, 0.6f, 1.0f};
  float sky2[] = {1.0f, 0.1f, 0.3f, 0.8f};
  float* def_dayvert[] = {sky1, sky2, 0};

  float night1[] = {0.0f, 0.1f, 0.1f, 0.1f};
  float night2[] = {1.0f, 0.0f, 0.0f, 0.0f};
  float* def_nightvert[] = {night1, night2, 0};

  float sun0[] = {0.0f, 1.0f, 1.0f, 0.6f};
  float sun1[] = {0.5f, 1.0f, 0.9f, 0.6f};
  float sun2[] = {1.0f, 1.0f, 0.8f, 0.6f};
  float *def_topsun[] = {sun0, sun1, sun2, 0};

  float sunset0[] = {0.0f, 0.9f, 0.9f, -0.9f};
  float sunset1[] = {0.5f, 0.1f, -0.8f, -0.6f};
  float sunset2[] = {1.0f, 1.0f, -0.9f, 1.0f};
  float* def_sunset[] = {sunset0, sunset1, sunset2, 0};

  if(!dayvert) dayvert = def_dayvert;
  if(!nightvert) nightvert = def_nightvert;
  if(!topsun) topsun = def_topsun;
  if(!sunset) sunset = def_sunset;
  // scale time to 0-1.
  while(time < 0.0f) time += 1.0f;
  while(time >= 1.0f) time -= 1.0f;
  // allocate application gradients
  int vertlen = 0;
  int sunlen = 0;
  int i = 0;
  while(dayvert[i] != 0) {vertlen++; i++;}
  i = 0;
  while(topsun[i] != 0) {sunlen++; i++;}
  float ** applyvert = new float* [vertlen+1];
  applyvert[vertlen] = 0;
  float ** applysun = new float* [sunlen+1];
  applysun[sunlen] = 0;
  for(i=0; i<vertlen; i++)
    applyvert[i] = new float [4];
  for(i=0; i<sunlen; i++)
    applysun[i] = new float [4];

  // interpolate the gradients
  float day_amount = 1.0f;
  // 0.9 -- 0.1 is night to day shift
  if(time <= 0.1f) day_amount = (time + 0.1f) / 0.2f;
  if(time >= 0.9f) day_amount = (time - 0.9f) / 0.2f;
  // 0.4 -- 0.6 is day to night shift
  if( (time >= 0.4f) && (time <= 0.6f))
    day_amount = (0.6f - time) / 0.2f;
  // 0.6 -- 0.9 is night
  if( (time > 0.6f) && (time < 0.9f))
    day_amount = 0.0f;

  float sc1 = day_amount;
  float sc2 = 1.0f - day_amount;
  for(i=0; i<vertlen; i++)
  {
    CS_ASSERT( dayvert[i][0] == nightvert[i][0] ) ;
    applyvert[i][0] = dayvert[i][0];
    applyvert[i][1] = sc1 * dayvert[i][1] + sc2 * nightvert[i][1];
    applyvert[i][2] = sc1 * dayvert[i][2] + sc2 * nightvert[i][2];
    applyvert[i][3] = sc1 * dayvert[i][3] + sc2 * nightvert[i][3];
  }

  float sunset_amount = 0.0f;
  if(time <= 0.1f) sunset_amount = (0.1f - time) / 0.2f;
  if(time >= 0.9f) sunset_amount = (1.1f - time) / 0.2f;
  if( (time >= 0.4f) && (time <= 0.6f))
    sunset_amount = (time - 0.4f) / 0.2f;
  if( (time > 0.6f) && (time < 0.9f))
    sunset_amount = 1.0f;

  sc1 = 1.0f - sunset_amount;
  sc2 = sunset_amount;
  for(i=0; i<sunlen; i++)
  {
    CS_ASSERT( topsun[i][0] == sunset[i][0] ) ;
    applysun[i][0] = topsun[i][0];
    applysun[i][1] = sc1 * topsun[i][1] + sc2 * sunset[i][1];
    applysun[i][2] = sc1 * topsun[i][2] + sc2 * sunset[i][2];
    applysun[i][3] = sc1 * topsun[i][3] + sc2 * sunset[i][3];
  }
  float sun_size = 1.0;
  float sunset_size = 2.0;
  float apply_size = sun_size * sc1 + sunset_size * sc2;

  /// apply the gradients
  ApplyVertGradient(shift.y, shift.y + radiusy, applyvert);
  /// compute sun position
  // 0.0 is east, 0.5 is west.
  csVector3 sunpos = shift;
  float scaletime = time * TWO_PI;
  sunpos.x += (float) cos(scaletime) * radiusx;
  sunpos.y += (float) sin(scaletime) * radiusy;
  sunpos.z -= (float) sin(scaletime) * radiusz;
  ApplyLightSpot(sunpos, apply_size, applysun);

  /// compute apparent light
  csColor apparent(1,1,1);
  if(sunpos.y >= shift.y)
    GetGradientColor(applysun, 0.0f, apparent);
  else
  {
    float maxdist = radiusy * apply_size / 1.4f;
    if( ((time > 0.6f) && (time < 0.9f))
       || (sunpos.y < shift.y - maxdist))
      apparent.Set(0.0f, 0.0f, 0.0f); // deep night
    else
      GetGradientColor(applysun, (shift.y - sunpos.y)/maxdist, apparent);
  }
  csColor skyatsun;
  GetGradientColor(applyvert, 0.0f, skyatsun);
  apparent += skyatsun;
  apparent.Clamp(2.0f, 2.0f, 2.0f);
  apparent.ClampDown();
  //printf("Apparent is %g, %g, %g\n", apparent.red, apparent.green,
    //apparent.blue);
  for(i=0; i<num_ball_vertices; i++)
    if(ball_vertices[i].y < shift.y)
      ball_colors[i] = apparent;
  generated_colors = true;

  /// delete gradients
  for(i=0; i<vertlen; i++)
    delete[] applyvert[i];
  delete[] applyvert;
  for(i=0; i<sunlen; i++)
    delete[] applysun[i];
  delete[] applysun;
}

csMeshedPolygon* csBallMeshObject::GetPolygons ()
{
  if (!polygons)
  {
    csTriangle* triangles = (csTriangle*)ball_indices;
    polygons = new csMeshedPolygon [ball_triangles];
    int i;
    for (i = 0 ; i < ball_triangles ; i++)    
    {
      polygons[i].num_vertices = 3;
      polygons[i].vertices = &triangles[i].a;
    }
  }

  return polygons;
}

void csBallMeshObject::PolyMesh::Cleanup ()
{
  delete[] ball->polygons;
  ball->polygons = 0;
}

int csBallMeshObject::PolyMesh::GetVertexCount ()
{
  return ball->GetVertexCount ();
}

csVector3* csBallMeshObject::PolyMesh::GetVertices ()
{
  return ball->GetVertices ();
}

int csBallMeshObject::PolyMesh::GetPolygonCount ()
{
  return ball->GetTriangleCount ();
}

csMeshedPolygon* csBallMeshObject::PolyMesh::GetPolygons ()
{
  return ball->GetPolygons ();
}

//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csBallMeshObjectFactory)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectFactory)
SCF_IMPLEMENT_IBASE_END

csBallMeshObjectFactory::csBallMeshObjectFactory (iMeshObjectType* pParent,
	iObjectRegistry* object_reg)
{
  SCF_CONSTRUCT_IBASE (pParent);
  csBallMeshObjectFactory::object_reg = object_reg;
  logparent = 0;
  ball_type = pParent;
  csRef<iEngine> eng = CS_QUERY_REGISTRY (object_reg, iEngine);
  engine = eng;	// We don't want a circular reference.
  light_mgr = CS_QUERY_REGISTRY (object_reg, iLightManager);
  csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
    object_reg, 
    "crystalspace.shared.stringset", iStringSet);
  string_object2world = strings->Request ("object2world transform");
}

csBallMeshObjectFactory::~csBallMeshObjectFactory ()
{
  SCF_DESTRUCT_IBASE ();
}

csPtr<iMeshObject> csBallMeshObjectFactory::NewInstance ()
{
  csBallMeshObject* cm = new csBallMeshObject ((iMeshObjectFactory*)this);
  csRef<iMeshObject> im (SCF_QUERY_INTERFACE (cm, iMeshObject));
  cm->DecRef ();
  return csPtr<iMeshObject> (im);
}

//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csBallMeshObjectType)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectType)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csBallMeshObjectType::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csBallMeshObjectType)


csBallMeshObjectType::csBallMeshObjectType (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csBallMeshObjectType::~csBallMeshObjectType ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

csPtr<iMeshObjectFactory> csBallMeshObjectType::NewFactory ()
{
  csBallMeshObjectFactory* cm = new csBallMeshObjectFactory (this,
  	object_reg);
  csRef<iMeshObjectFactory> ifact (
  	SCF_QUERY_INTERFACE (cm, iMeshObjectFactory));
  cm->DecRef ();
  return csPtr<iMeshObjectFactory> (ifact);
}

