/*
    Copyright (C) 2003 by Martin Geisse <mgeisse@gmx.net>

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
#include "plugins/mesh/partgen/particle.h"
#include "iengine/rview.h"
#include "iengine/camera.h"
#include "iengine/movable.h"
#include "iengine/light.h"
#include "ivideo/material.h"
#include "ivideo/vbufmgr.h"
#include "iutil/objreg.h"
#include "qint.h"

SCF_IMPLEMENT_IBASE_EXT (csNewParticleSystem)
#ifndef CS_USE_NEW_RENDERER
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iVertexBufferManagerClient)
#endif
SCF_IMPLEMENT_IBASE_EXT_END

#ifndef CS_USE_NEW_RENDERER
SCF_IMPLEMENT_EMBEDDED_IBASE (csNewParticleSystem::eiVertexBufferManagerClient)
  SCF_IMPLEMENTS_INTERFACE (iVertexBufferManagerClient)
SCF_IMPLEMENT_EMBEDDED_IBASE_END
#endif

csNewParticleSystem::csNewParticleSystem (
	iEngine *eng, iMeshObjectFactory *fact, int flags) : csMeshObject (eng)
{
#ifndef CS_USE_NEW_RENDERER 
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiVertexBufferManagerClient);
#endif
  Factory = fact;
  ParticleFlags = flags;
  ParticleCount = 0;
  StorageCount = 0;
  PositionArray = 0;
  Scale = csVector2 (1.0f, 1.0f);
  ScaleArray = 0;
  Angle = 0.0f;
  AngleArray = 0;
  Color = csColor (0, 0, 0);
  ColorArray = 0;
  Material = 0;
  MaterialArray = 0;
  Axis = csVector3 (0, 1, 0);
  AxisArray = 0;
  PrevTime = 0;
  MixMode = CS_FX_COPY;
  Lighting = true;
  LitColors = 0;
#ifdef CS_USE_NEW_RENDERER 
  csMeshFactory* mf = (csMeshFactory*)fact;
  iObjectRegistry* object_reg = mf->GetObjectRegistry ();
  g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);

  initialized = false;
  vertices = 0;
  colors = 0;
  texels = 0;
  triangles = 0;

  mesh.dynDomain.AttachNew (new csShaderVariableContext);
#endif
}

csNewParticleSystem::~csNewParticleSystem ()
{
  delete[] PositionArray;
  delete[] ScaleArray;
  delete[] AngleArray;
  delete[] ColorArray;
  delete[] MaterialArray;
  delete[] AxisArray;
  delete[] LitColors;
#ifdef CS_USE_NEW_RENDERER 
  delete[] vertices;
  delete[] colors;
  delete[] texels;
  delete[] triangles;
#else
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiVertexBufferManagerClient);
#endif
}

#define UPDATE_ARRAY_FLAG(FLAG,NAME,TYPE)                               \
  if (ParticleFlags & FLAG)                                             \
    UPDATE_ARRAY_ALWAYS (NAME, TYPE);

#define UPDATE_ARRAY_ALWAYS(NAME,TYPE) {                                \
  TYPE *old = NAME;                                                     \
  NAME = new TYPE [newsize];                                            \
  memcpy (NAME, old, sizeof (TYPE) * copysize);                         \
  delete[] old;                                                         \
}

void csNewParticleSystem::Allocate (int newsize, int copysize)
{
  UPDATE_ARRAY_ALWAYS (PositionArray, csVector3);
  UPDATE_ARRAY_FLAG (CS_PARTICLE_SEP_SCALE, ScaleArray, csVector2);
  UPDATE_ARRAY_FLAG (CS_PARTICLE_SEP_ROTATE, AngleArray, float);
  UPDATE_ARRAY_FLAG (CS_PARTICLE_SEP_COLOR, ColorArray, csColor);
  UPDATE_ARRAY_FLAG (CS_PARTICLE_SEP_AXIS, AxisArray, csVector3);
  if (Lighting) UPDATE_ARRAY_ALWAYS (LitColors, csColor);

  /// special handling for material references
  if (ParticleFlags & CS_PARTICLE_SEP_MATERIAL)
  {
    csRef<iMaterialWrapper> *old = MaterialArray;
    MaterialArray = new csRef<iMaterialWrapper> [newsize];
    for (int i=0; i<StorageCount; i++)
      MaterialArray [i] = old [i];
    delete[] old;
  }

  StorageCount = newsize;
}

void csNewParticleSystem::SetCount (int c)
{
  if (c > StorageCount)
    Allocate (c, ParticleCount);
  ParticleCount = c;
#ifdef CS_USE_NEW_RENDERER
  initialized = false;
#endif
}

void csNewParticleSystem::Compact ()
{
  if (ParticleCount < StorageCount)
    Allocate (ParticleCount, ParticleCount);
}

void csNewParticleSystem::UpdateBounds ()
{
  Bounds.StartBoundingBox ();
  for (int i=0; i<ParticleCount; i++)
    Bounds.AddBoundingVertex (PositionArray [i]);
}

iMeshObjectFactory* csNewParticleSystem::GetFactory () const
{
  return Factory;
}

void csNewParticleSystem::SetupObject ()
{
#ifdef CS_USE_NEW_RENDERER
  if (!initialized)
  {
    csStringID vertex_name, texel_name, normal_name, color_name, index_name;
    csMeshFactory* mf = (csMeshFactory*)Factory;
    iObjectRegistry* object_reg = mf->GetObjectRegistry ();
    csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (object_reg, 
	"crystalspace.shared.stringset", iStringSet);
    vertex_name = strings->Request ("vertices");
    texel_name = strings->Request ("texture coordinates");
    normal_name = strings->Request ("normals");
    color_name = strings->Request ("colors");
    index_name = strings->Request ("indices");

    delete[] vertices;
    delete[] colors;
    delete[] texels;
    delete[] triangles;
    initialized = true;
    VertexCount = ParticleCount * 4;
    TriangleCount = ParticleCount * 2;
    vertices = new csVector3 [VertexCount];
    colors = new csColor [VertexCount];
    texels = new csVector2 [VertexCount];
    triangles = new csTriangle [TriangleCount];
    vertex_buffer = g3d->CreateRenderBuffer (
        sizeof (csVector3)*VertexCount, CS_BUF_STATIC, 
        CS_BUFCOMP_FLOAT, 3, false);
    texel_buffer = g3d->CreateRenderBuffer (
        sizeof (csVector2)*VertexCount, CS_BUF_STATIC, 
        CS_BUFCOMP_FLOAT, 2, false);
#if 0
    normal_buffer = g3d->CreateRenderBuffer (
        sizeof (csVector3)*VertexCount, CS_BUF_STATIC,
        CS_BUFCOMP_FLOAT, 3, false);
#endif
    color_buffer = g3d->CreateRenderBuffer (
        sizeof (csColor)*VertexCount, CS_BUF_STATIC,
        CS_BUFCOMP_FLOAT, 3, false);
    index_buffer = g3d->CreateRenderBuffer (
        sizeof (unsigned int)*TriangleCount*3, CS_BUF_STATIC,
        CS_BUFCOMP_UNSIGNED_INT, 1, true);
    csShaderVariable *sv;
    sv = mesh.dynDomain->GetVariableAdd (vertex_name);
    sv->SetValue (vertex_buffer);
    sv = mesh.dynDomain->GetVariableAdd (texel_name);
    sv->SetValue (texel_buffer);
#if 0
    sv = mesh.dynDomain->GetVariableAdd (normal_name);
    sv->SetValue (normal_buffer);
#endif
    sv = mesh.dynDomain->GetVariableAdd (color_name);
    sv->SetValue (color_buffer);
    sv = mesh.dynDomain->GetVariableAdd (index_name);
    sv->SetValue (index_buffer);
  }
#endif
}

bool csNewParticleSystem::DrawTest (iRenderView* rview, iMovable* movable)
{
  SetupObject ();

  // get the object-to-camera transformation
  iCamera *camera = rview->GetCamera ();
  csReversibleTransform trans = camera->GetTransform ();
  if (!movable->IsFullTransformIdentity ())
    trans /= movable->GetFullTransform ();

  // build a bounding box in camera space
  csBox3 cbox;
  int i;
  for (i=0; i<8; i++)
    cbox.AddBoundingVertexSmart (trans * Bounds.GetCorner (i));
  if (cbox.MaxZ () < 0) return false;

   // transform from camera to screen space
  csBox2 sbox;

  if (cbox.MinZ () <= 0)
  {
    sbox.Set (-CS_BOUNDINGBOX_MAXVALUE, -CS_BOUNDINGBOX_MAXVALUE,
	          +CS_BOUNDINGBOX_MAXVALUE, +CS_BOUNDINGBOX_MAXVALUE);
  }
  else
  {
    sbox.StartBoundingBox ();
    float fov = camera->GetFOV ();
    float shift_x = camera->GetShiftX ();
    float shift_y = camera->GetShiftY ();

    for (i=0; i<8; i++)
    {
      csVector3 cv = cbox.GetCorner (i);
      int inv_z = QInt (fov / cv.z);
      int sx = QInt (cv.x * inv_z + shift_x);
      int sy = QInt (cv.y * inv_z + shift_y);
      sbox.AddBoundingVertex (sx, sy);
    }
  }

  // Test visibility of bounding box with the clipper
  if (!rview->ClipBBox (sbox, cbox, ClipPortal, ClipPlane, ClipZ))
    return false;

#ifdef CS_USE_NEW_RENDERER
  mesh.object2camera = csReversibleTransform ();
  mesh.clip_portal = ClipPortal;
  mesh.clip_plane = ClipPlane;
  mesh.clip_z_plane = ClipZ;
  mesh.do_mirror = camera->IsMirrored ();  

  // set up the data for DrawTriangleMesh
  for (int i=0; i<ParticleCount; i++)
  {
    // transform to eye coordinates
    csVector3 pos = trans.Other2This (PositionArray [i]);

    // compute modified axes for rotation or axis alignment
    csVector3 x_axis, y_axis;

    if (ParticleFlags & CS_PARTICLE_ROTATE)
    {
      float an = (ParticleFlags & CS_PARTICLE_SEP_ROTATE) ?
        AngleArray [i] : Angle;
      x_axis = csVector3 (cos (an), sin (an), 0);
      y_axis = csVector3 (-sin (an), cos (an), 0);
    }
    else if (ParticleFlags & CS_PARTICLE_AXIS)
    {
      csVector3 effectiveAxis = trans.Other2ThisRelative (
        (ParticleFlags & CS_PARTICLE_SEP_AXIS) ? AxisArray [i] : Axis);

      if (ParticleFlags & CS_PARTICLE_ALIGN_Y)
      {
        y_axis = effectiveAxis;
        x_axis = csVector3 (0, 0, -1) % y_axis;
      }
      else
      {
        x_axis = effectiveAxis;
        y_axis = csVector3 (0, 0, +1) % x_axis;
      }

      float norm = x_axis.Norm ();
      if (ABS (norm) < EPSILON) continue;
      x_axis /= norm;

      norm = y_axis.Norm ();
      if (ABS (norm) < EPSILON) continue;
      y_axis /= norm;
    }
    else
    {
      x_axis = csVector3 (1, 0, 0);
      y_axis = csVector3 (0, 1, 0);
    }


    // apply scaling
    if (ParticleFlags & CS_PARTICLE_SCALE)
    {
      csVector2 sc = (ParticleFlags & CS_PARTICLE_SEP_SCALE) ?
        ScaleArray [i] : Scale;
      x_axis *= sc.x;
      y_axis *= sc.y;
    }

    // compute the actual vertices
    x_axis /= 2;
    y_axis /= 2;
    vertices [i*4+0] = pos - x_axis - y_axis;
    vertices [i*4+1] = pos - x_axis + y_axis;
    vertices [i*4+2] = pos + x_axis + y_axis;
    vertices [i*4+3] = pos + x_axis - y_axis;

    // fill the color table
    csColor col = Lighting ? LitColors [i] :
      (ParticleFlags & CS_PARTICLE_SEP_COLOR) ?
          ColorArray [i] : Color;
    colors [i*4+0] = col;
    colors [i*4+1] = col;
    colors [i*4+2] = col;
    colors [i*4+3] = col;

    // fill the texel table
    texels [i*4+0] = csVector2 (0, 0);
    texels [i*4+1] = csVector2 (0, 1);
    texels [i*4+2] = csVector2 (1, 1);
    texels [i*4+3] = csVector2 (1, 0);

    // fill the triangle table
    triangles [i*2+0] = csTriangle (i*4+0, i*4+1, i*4+2);
    triangles [i*2+1] = csTriangle (i*4+0, i*4+2, i*4+3);
  }

#endif

  return true;
}

void csNewParticleSystem::UpdateLighting (iLight** lights, int num,
  iMovable* movable)
{
  if (!Lighting) return;
  const csReversibleTransform &transform = movable->GetTransform ();

  for (int i=0; i<ParticleCount; i++)
  {
    csColor lightColor = (ParticleFlags & CS_PARTICLE_SEP_COLOR) ?
      ColorArray [i] : Color;
    csVector3 wpos = transform.This2Other (PositionArray [i]);

    for (int j=0; j<num; j++)
    {
      float d = (wpos - lights [j]->GetCenter ()).Norm ();
      float br = lights [j]->GetBrightnessAtDistance (d);
      lightColor += br * lights [j]->GetColor ();
    }

    LitColors [i] = lightColor;
  }
}

#ifndef CS_USE_NEW_RENDERER
void csNewParticleSystem::eiVertexBufferManagerClient::ManagerClosing ()
{
  scfParent->vbuf = 0;
}
#endif

bool csNewParticleSystem::Draw (iRenderView* rview, iMovable* mov,
	csZBufMode mode)
{
#ifndef CS_USE_NEW_RENDERER
  // some generic setup
  if (VisCallback) VisCallback->BeforeDrawing (this, rview);
  iGraphics3D* g3d = rview->GetGraphics3D ();
  iCamera* camera = rview->GetCamera ();
  g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, mode);

  // Get object space in view coordinates
  csReversibleTransform objspace = camera->GetTransform ();
  if (!mov->IsFullTransformIdentity ())
    objspace /= mov->GetFullTransform ();

  // these are the final data chunks for DrawTriangleMesh
  int VertexCount = ParticleCount * 4;
  int TriangleCount = ParticleCount * 2;
  CS_ALLOC_STACK_ARRAY (csVector3, vertices, VertexCount);
  CS_ALLOC_STACK_ARRAY (csColor, colors, VertexCount);
  CS_ALLOC_STACK_ARRAY (csVector2, texels, VertexCount);
  CS_ALLOC_STACK_ARRAY (csTriangle, triangles, TriangleCount);
  csBox3 bbox;

  // if the particles use separate materials, sort them
  if (ParticleFlags & CS_PARTICLE_SEP_MATERIAL)
  {
    // @@@
  }
  Material->Visit ();

  // set up the data for DrawTriangleMesh
  for (int i=0; i<ParticleCount; i++)
  {
    // transform to eye coordinates
    csVector3 pos = objspace.Other2This (PositionArray [i]);

    // compute modified axes for rotation or axis alignment
    csVector3 x_axis, y_axis;

    if (ParticleFlags & CS_PARTICLE_ROTATE)
    {
      float an = (ParticleFlags & CS_PARTICLE_SEP_ROTATE) ?
        AngleArray [i] : Angle;
      x_axis = csVector3 (cos (an), sin (an), 0);
      y_axis = csVector3 (-sin (an), cos (an), 0);
    }
    else if (ParticleFlags & CS_PARTICLE_AXIS)
    {
      csVector3 effectiveAxis = objspace.Other2ThisRelative (
        (ParticleFlags & CS_PARTICLE_SEP_AXIS) ? AxisArray [i] : Axis);

      if (ParticleFlags & CS_PARTICLE_ALIGN_Y)
      {
        y_axis = effectiveAxis;
        x_axis = csVector3 (0, 0, -1) % y_axis;
      }
      else
      {
        x_axis = effectiveAxis;
        y_axis = csVector3 (0, 0, +1) % x_axis;
      }

      float norm = x_axis.Norm ();
      if (ABS (norm) < EPSILON) continue;
      x_axis /= norm;

      norm = y_axis.Norm ();
      if (ABS (norm) < EPSILON) continue;
      y_axis /= norm;
    }
    else
    {
      x_axis = csVector3 (1, 0, 0);
      y_axis = csVector3 (0, 1, 0);
    }


    // apply scaling
    if (ParticleFlags & CS_PARTICLE_SCALE)
    {
      csVector2 sc = (ParticleFlags & CS_PARTICLE_SEP_SCALE) ?
        ScaleArray [i] : Scale;
      x_axis *= sc.x;
      y_axis *= sc.y;
    }

    // compute the actual vertices
    x_axis /= 2;
    y_axis /= 2;
    vertices [i*4+0] = pos - x_axis - y_axis;
    vertices [i*4+1] = pos - x_axis + y_axis;
    vertices [i*4+2] = pos + x_axis + y_axis;
    vertices [i*4+3] = pos + x_axis - y_axis;
    bbox.AddBoundingVertex (vertices [i*4+0]);
    bbox.AddBoundingVertex (vertices [i*4+1]);
    bbox.AddBoundingVertex (vertices [i*4+2]);
    bbox.AddBoundingVertex (vertices [i*4+3]);

    // fill the color table
    csColor col = Lighting ? LitColors [i] :
      (ParticleFlags & CS_PARTICLE_SEP_COLOR) ?
          ColorArray [i] : Color;
    colors [i*4+0] = col;
    colors [i*4+1] = col;
    colors [i*4+2] = col;
    colors [i*4+3] = col;

    // fill the texel table
    texels [i*4+0] = csVector2 (0, 0);
    texels [i*4+1] = csVector2 (0, 1);
    texels [i*4+2] = csVector2 (1, 1);
    texels [i*4+3] = csVector2 (1, 0);

    // fill the triangle table
    triangles [i*2+0] = csTriangle (i*4+0, i*4+1, i*4+2);
    triangles [i*2+1] = csTriangle (i*4+0, i*4+2, i*4+3);
  }

  // set up a vertex buffer
  iVertexBufferManager *vbufmgr = g3d->GetVertexBufferManager ();
  if (vbuf == 0)
  {
    vbufmgr->AddClient (&scfiVertexBufferManagerClient);
    vbuf = vbufmgr->CreateBuffer (100);
  }

  // set up the G3DTriangleMesh
  G3DTriangleMesh trimesh;
  trimesh.num_vertices_pool = 1;
  trimesh.num_triangles = ParticleCount * 2;
  trimesh.triangles = triangles;
  trimesh.clip_portal = ClipPortal;
  trimesh.clip_plane = ClipPlane;
  trimesh.clip_z_plane = ClipZ;
  trimesh.use_vertex_color = false;
  trimesh.do_fog = false;
  trimesh.do_mirror = false;
  trimesh.do_morph_texels = false;
  trimesh.do_morph_colors = false;
  trimesh.vertex_mode = G3DTriangleMesh::VM_VIEWSPACE;
  trimesh.mixmode = MixMode;
  trimesh.morph_factor = 0;
  trimesh.buffers[0] = vbuf;
  trimesh.mat_handle = Material->GetMaterialHandle ();
  trimesh.vertex_fog = 0;

  // draw it!
  vbufmgr->LockBuffer (vbuf, vertices, texels, colors, VertexCount, 0, bbox);
  g3d->DrawTriangleMesh (trimesh);
  vbufmgr->UnlockBuffer (vbuf);
#endif

  return true;
}

csRenderMesh **csNewParticleSystem::GetRenderMeshes (int &num)
{
#ifdef CS_USE_NEW_RENDERER
  SetupObject ();
  // some generic setup
  //@@@?if (VisCallback) VisCallback->BeforeDrawing (this, rview);

  // if the particles use separate materials, sort them
  if (ParticleFlags & CS_PARTICLE_SEP_MATERIAL)
  {
    // @@@
  }
  Material->Visit ();

  vertex_buffer->CopyToBuffer (vertices, sizeof (csVector3) * VertexCount);
  texel_buffer->CopyToBuffer (texels, sizeof (csVector2) * VertexCount);
  color_buffer->CopyToBuffer (colors, sizeof (csColor) * VertexCount);
  index_buffer->CopyToBuffer (triangles,
      	sizeof (unsigned int) * TriangleCount *3);

  // Prepare for rendering.
  mesh.mixmode = MixMode;

  mesh.indexstart = 0;
  mesh.indexend = TriangleCount * 3;
  //mesh.mathandle = mater->GetMaterialHandle();
  mesh.material = Material;

  mesh.meshtype = CS_MESHTYPE_TRIANGLES;
  meshPtr = &mesh;
  num = 1;
  return &meshPtr;
#else
  num = 0;
  return 0;
#endif
}

void csNewParticleSystem::NextFrame (csTicks current, const csVector3&)
{
  if (PrevTime != 0)
    Update (current - PrevTime);
  PrevTime = current;
}

bool csNewParticleSystem::SetColor (const csColor& c)
{
  if (ParticleFlags & CS_PARTICLE_SEP_COLOR) return false;
  Color = c;
  return true;
}

bool csNewParticleSystem::GetColor (csColor& c) const
{
  if (ParticleFlags & CS_PARTICLE_SEP_COLOR) return false;
  c = Color;
  return true;
}

bool csNewParticleSystem::SetMaterialWrapper (iMaterialWrapper* m)
{
  if (ParticleFlags & CS_PARTICLE_SEP_MATERIAL) return false;
  Material = m;
  return true;
}

iMaterialWrapper* csNewParticleSystem::GetMaterialWrapper () const
{
  if (ParticleFlags & CS_PARTICLE_SEP_MATERIAL) return 0;
  else return Material;
}

bool csNewParticleSystem::GetLighting () const
{
  return Lighting;
}

void csNewParticleSystem::SetLighting (bool enable)
{
  delete[] LitColors;
  Lighting = enable;
  if (Lighting) LitColors = new csColor [StorageCount];
  else LitColors = 0;
}

