/*
    Copyright (C) 2003 by Jorrit Tyberghein, John Harger, Daniel Duhprey

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

#include "iutil/objreg.h"

#include "iengine/camera.h"
#include "iengine/movable.h"
#include "iengine/rview.h"

#include "ivideo/material.h"
#include "ivideo/rndbuf.h"
#include "ivideo/texture.h"
#include "ivideo/txtmgr.h"

#include "ivaria/reporter.h"

#include "csutil/randomgen.h"
#include "csutil/util.h"

#include "qsqrt.h"
#include "qint.h"

#include "particles.h"
#include <limits.h>

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csParticlesType)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectType)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csParticlesType::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csParticlesType)

csParticlesType::csParticlesType (iBase* p) : parent(p)
{
  SCF_CONSTRUCT_IBASE (parent)
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent)
}

csParticlesType::~csParticlesType ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

csPtr<iMeshObjectFactory> csParticlesType::NewFactory ()
{
  return csPtr<iMeshObjectFactory>
	(new csParticlesFactory (this, object_reg));
}

SCF_IMPLEMENT_IBASE (csParticlesFactory)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectFactory)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iParticlesFactoryState)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csParticlesFactory::eiParticlesFactoryState)
  SCF_IMPLEMENTS_INTERFACE (iParticlesFactoryState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csParticlesFactory::csParticlesFactory (csParticlesType* p,
	iObjectRegistry* objreg) : parent (p), object_reg (objreg)
{
  SCF_CONSTRUCT_IBASE (p)
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiParticlesFactoryState)

  g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  shmgr = CS_QUERY_REGISTRY (object_reg, iShaderManager);

  emit_type = CS_PART_EMIT_SPHERE;
  emit_size_1 = 0.0015f;
  emit_size_2 = 0.001f;
  emit_size_3 = 0.0f;

  force_type = CS_PART_FORCE_RADIAL;

  force_direction = csVector3(0.0f, 0.0f, 0.0f);
  force_range = 1.0f;
  force_falloff = CS_PART_FALLOFF_LINEAR;
  force_cone_radius = 0.0f;
  force_cone_radius_falloff = CS_PART_FALLOFF_CONSTANT;

  force_amount = 1.0f;

  particles_per_second = 100;
  initial_particles = 100;

  gravity = csVector3(0.0f, 0.0f, 0.0f);

  emit_time = 1.0f;
  time_to_live = 1.0f;
  time_variation = 0.0f;

  diffusion = 0.0f;

  particle_radius = 0.05f;
  particle_mass = 1.0f;
  dampener = 0.01f;

  heat_function = CS_PART_HEAT_SPEED;

  heat_callback = NULL;
  color_callback = NULL;
}

csParticlesFactory::~csParticlesFactory ()
{
}

csPtr<iMeshObject> csParticlesFactory::NewInstance ()
{
  return csPtr<iMeshObject>(new csParticlesObject (this));
}

SCF_IMPLEMENT_IBASE (csParticlesObject)
  SCF_IMPLEMENTS_INTERFACE (iMeshObject)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iParticlesObjectState)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iObjectModel)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csParticlesObject::eiParticlesObjectState)
  SCF_IMPLEMENTS_INTERFACE (iParticlesObjectState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csParticlesObject::eiObjectModel)
  SCF_IMPLEMENTS_INTERFACE (iObjectModel)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csParticlesObject::ShaderVariableAccessor)
SCF_IMPLEMENT_IBASE_END

csParticlesObject::csParticlesObject (csParticlesFactory* p)
  : logparent (p), pFactory (p), shaderVarAccessor (this)
{
  SCF_CONSTRUCT_IBASE (p)
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiParticlesObjectState)
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiObjectModel)

  mesh = 0;
  meshpp = 0;
  meshppsize = 0;

  csRef<iStringSet> strings =
    CS_QUERY_REGISTRY_TAG_INTERFACE (p->object_reg,
	  "crystalspace.shared.stringset", iStringSet);

  vertex_name = strings->Request ("vertices");
  color_name = strings->Request ("colors");
  texcoord_name = strings->Request ("texture coordinates");
  index_name = strings->Request ("indices");
  radius_name = strings->Request ("point radius");
  scale_name = strings->Request ("point scale");

  camera_fov = -1;

  emit_type = p->emit_type;
  emit_size_1 = p->emit_size_1;
  emit_size_2 = p->emit_size_2;
  emit_size_3 = p->emit_size_3;

  force_type = p->force_type;

  force_direction = p->force_direction;
  force_range = p->force_range;
  force_falloff = p->force_falloff;
  force_cone_radius = p->force_cone_radius;
  force_cone_radius_falloff = p->force_cone_radius_falloff;

  force_amount = p->force_amount;
  particle_mass = p->particle_mass;
  dampener = p->dampener;

  particles_per_second = p->particles_per_second;
  initial_particles = p->initial_particles;

  gravity = p->gravity;

  emit_time = p->emit_time;
  total_elapsed_time = 0.0f;
  time_to_live = p->time_to_live;
  time_variation = p->time_variation;

  diffusion = p->diffusion;

  particle_radius = p->particle_radius;

  gradient_colors = p->gradient_colors;

  heat_function = p->heat_function;

  heat_callback = p->heat_callback;
  color_callback = p->color_callback;

  emitter = csVector3(0.0f, 0.0f, 0.0f);
  radius = 1.0f;
  dead_particles = 0;
  point_sprites = false;//p->g3d->GetCaps ()->SupportsPointSprites;

  dynDomain = new csShaderVariableContext ();

  csShaderVariable* sv;
  sv = dynDomain->GetVariableAdd (vertex_name);
  sv->SetAccessor (&shaderVarAccessor);
  sv = dynDomain->GetVariableAdd (color_name);
  sv->SetAccessor (&shaderVarAccessor);
  sv = dynDomain->GetVariableAdd (texcoord_name);
  sv->SetAccessor (&shaderVarAccessor);
  sv = dynDomain->GetVariableAdd (index_name);
  sv->SetAccessor (&shaderVarAccessor);
  sv = dynDomain->GetVariableAdd (radius_name);
  sv->SetValue (particle_radius);
  sv = dynDomain->GetVariableAdd (scale_name);
  sv->SetValue (1.0f);

  LoadPhysicsPlugin ("crystalspace.particles.physics.simple");

  Start();
}

csParticlesObject::~csParticlesObject ()
{
  if (meshpp)
    delete [] meshpp;
  if (mesh)
    delete mesh;
}

bool csParticlesObject::LoadPhysicsPlugin (const char *plugin_id)
{
  csRef<iPluginManager> plugin_mgr = CS_QUERY_REGISTRY (
    pFactory->object_reg, iPluginManager);

  physics = CS_QUERY_PLUGIN_CLASS (plugin_mgr, plugin_id, iParticlesPhysics);
  if (!physics)
  {
    physics = CS_LOAD_PLUGIN (plugin_mgr, plugin_id, iParticlesPhysics);
  }
  if (!physics)
  {
    csReport (pFactory->object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.mesh.object.particles",
      "Could not load the particles physics plugin '%s'!", plugin_id);
    return false;
  }
  physics->RegisterParticles (&scfiParticlesObjectState, &point_data);
  return true;
}

void csParticlesObject::SetParticleRadius (float rad)
{
  particle_radius = rad;
  if (dynDomain) {
    csShaderVariable* sv = dynDomain->GetVariableAdd (radius_name);
    if (sv) sv->SetValue (particle_radius);
  }
}

bool csParticlesObject::DrawTest (iRenderView* rview, iMovable* movable)
{
  iCamera* cam = rview->GetCamera ();

  tr_o2c = cam->GetTransform ();
  emitter = movable->GetFullPosition();

  int vertnum = 0;
  if ((vertnum = point_data.Length () - dead_particles) < 1) return false;

  if (!point_sprites)
  {
    vertnum *= 6;
    csMatrix3 m = tr_o2c.GetT2O();
    corners[0] = m * csVector3(-particle_radius, particle_radius, 0.0f);
    corners[1] = m * csVector3(particle_radius, particle_radius, 0.0f);
    corners[2] = m * csVector3(particle_radius, -particle_radius, 0.0f);
    corners[3] = m * csVector3(-particle_radius, -particle_radius, 0.0f);
  }
  else
  {
    int fov = QInt (cam->GetFOVAngle ());
    int fov_pixels = cam->GetFOV ();
    if (camera_fov != fov || camera_pixels != fov_pixels)
    {
      camera_fov = fov;
      camera_pixels = fov_pixels;
      float lambda = (float)(fov_pixels / (2.0 * tan (fov / 360.0 * PI)));
      csShaderVariable* sv = dynDomain->GetVariable (scale_name);
      sv->SetValue (1.0f / (lambda * particle_radius * 3));
    }
  }

  /*if (!movable->IsFullTransformIdentity ())
    tr_o2c /= movable->GetFullTransform ();*/

  int clip_portal, clip_plane, clip_z_plane;
  csSphere s(emitter, radius);
  if (!rview->ClipBSphere (tr_o2c, s, clip_portal, clip_plane, clip_z_plane))
    return false;

  if (!mesh) {
    mesh = new csRenderMesh;
  }

  mesh->z_buf_mode = CS_ZBUF_USE;
  mesh->mixmode = CS_FX_ALPHA;
  mesh->clip_plane = clip_plane;
  mesh->clip_portal = clip_portal;
  mesh->clip_z_plane = clip_z_plane;
  mesh->do_mirror = rview->GetCamera()->IsMirrored();
  matwrap->Visit ();
  mesh->material = matwrap;
  mesh->object2camera = tr_o2c;
  mesh->indexstart = 0;
  mesh->indexend = vertnum;
  mesh->dynDomain = dynDomain;
  if (point_sprites)
    mesh->meshtype = CS_MESHTYPE_POINT_SPRITES;
  else
    mesh->meshtype = CS_MESHTYPE_TRIANGLES;

  return true;
}

int csParticlesObject::ZSort (csParticlesData const &item1,
  csParticlesData const &item2)
{
  if (item1.position.z > item2.position.z) return 1;
  return -1;
}

int csParticlesObject::ZSort (void const *item1, void const *item2)
{
  csParticlesData* i1 = (csParticlesData*)item1;
  csParticlesData* i2 = (csParticlesData*)item2;
  if (i1->position.z > i2->position.z) return 1;
  return -1;
}

void csParticlesObject::UpdateLighting (iLight**, int, iMovable*)
{
}

void csParticlesObject::PreGetShaderVariableValue (csShaderVariable* var)
{
  var->SetValue(GetRenderBuffer(var->Name));
}

iRenderBuffer *csParticlesObject::GetRenderBuffer (csStringID name)
{
  if (!vertex_buffer || buffer_length != point_data.Length ())
  {
    buffer_length = point_data.Length ();
    csArray<iRenderBuffer*> buffers;
    int bufsize = (point_sprites ? buffer_length : buffer_length * 4);
    pFactory->g3d->CreateInterleavedRenderBuffers (bufsize
      * sizeof(csParticlesData), CS_BUF_DYNAMIC, 2, buffers);
    vertex_buffer = buffers.Get(0);
    vertex_buffer->SetOffset (0);
    vertex_buffer->SetComponentType (CS_BUFCOMP_FLOAT);
    vertex_buffer->SetComponentCount (3);
    color_buffer = buffers.Get (1);
    color_buffer->SetOffset (sizeof(csVector3));
    color_buffer->SetComponentType (CS_BUFCOMP_FLOAT);
    color_buffer->SetComponentCount (4);
    color_buffer->SetStride (sizeof(csParticlesData));
    texcoord_buffer = pFactory->g3d->CreateRenderBuffer (
      sizeof (csVector2) * bufsize, CS_BUF_DYNAMIC,
      CS_BUFCOMP_FLOAT, 2, false);
    csVector2 *texcoords = new csVector2 [bufsize];
    if (point_sprites)
    {
      vertex_buffer->SetStride (sizeof(csParticlesData));
      color_buffer->SetStride (sizeof(csParticlesData));

      unsigned int *indices = new unsigned int[buffer_length];
      for (int i=0;i<buffer_length;i++)
      {
        indices[i] = i;
      }
      index_buffer = pFactory->g3d->CreateRenderBuffer (
        sizeof (unsigned int) * buffer_length, CS_BUF_STATIC,
        CS_BUFCOMP_UNSIGNED_INT, 1, true);
      index_buffer->CopyToBuffer(indices, sizeof(unsigned int)*buffer_length);
      delete [] indices;
    }
    else
    {
      vertex_buffer->SetStride (sizeof(i_vertex));
      color_buffer->SetStride (sizeof(i_vertex));
      int i;
      for (i=0;i<bufsize-4;i+=4)
      {
        texcoords[i].x = 0;
        texcoords[i].y = 0;
        texcoords[i+1].x = 0;
        texcoords[i+1].y = 1;
        texcoords[i+2].x = 1;
        texcoords[i+2].y = 1;
        texcoords[i+3].x = 1;
        texcoords[i+3].y = 0;
      }
      unsigned int *indices = new unsigned int[buffer_length * 6];
      int j;
      for (i=0,j=0;i<bufsize-1;i+=4,j+=6)
      {
        // First triangle
        indices[j] = i;
        indices[j+1] = i + 1;
        indices[j+2] = i + 2;
        // Second triangle
        indices[j+3] = i;
        indices[j+4] = i + 2;
        indices[j+5] = i + 3;
      }
      index_buffer = pFactory->g3d->CreateRenderBuffer (
        sizeof (unsigned int) * buffer_length * 6, CS_BUF_STATIC,
        CS_BUFCOMP_UNSIGNED_INT, 1, true);
      index_buffer->CopyToBuffer(
        indices, sizeof(unsigned int) * buffer_length * 6);
      delete [] indices;
    }
    texcoord_buffer->CopyToBuffer (texcoords, sizeof(csVector2) * bufsize);
    delete [] texcoords;
  }
  if (name == vertex_name)
  {
    void *data;
    int size;
    if (point_sprites)
    {
      data = point_data.GetArray ();
      size = point_data.Length () * (sizeof(csParticlesData));
    }
    else
    {
      int len = point_data.Length ();
      vertex_data.SetLength (len * 4);
      int i,j;
      for (i=0, j=0;i<len-1;i++,j+=4)
      {
        csParticlesData &point = point_data[i];
        i_vertex &vertex = vertex_data[j];
        vertex.position = point.position + corners[0];
        vertex.color = point.color;
        vertex = vertex_data[j + 1];
        vertex.position = point.position + corners[1];
        vertex.color = point.color;
        vertex = vertex_data[j + 2];
        vertex.position = point.position + corners[2];
        vertex.color = point.color;
        vertex = vertex_data[j + 3];
        vertex.position = point.position + corners[3];
        vertex.color = point.color;
      }

      data = vertex_data.GetArray ();
      size = vertex_data.Length () * sizeof(i_vertex);
    }
    // Vertex buffer is an interleaved buffer, so copy all the data into it
    vertex_buffer->CopyToBuffer (data, size);
    return vertex_buffer;
  }
  else if (name == color_name)
  {
    return color_buffer;
  }
  else if (name == texcoord_name)
  {
    return texcoord_buffer;
  }
  else if (name == index_name)
  {
    return index_buffer;
  }
  return 0;
}

csRenderMesh** csParticlesObject::GetRenderMeshes (int &n)
{
  n = 1;
  if (!meshpp) {
    meshpp = new csRenderMesh*[1];
    meshppsize = 1;
  }

  meshpp[0] = mesh;

  return meshpp;
}

bool csParticlesObject::HitBeamOutline (const csVector3& start,
	const csVector3& end, csVector3& isect, float* pr)
{
  return false;
}

bool csParticlesObject::HitBeamObject (const csVector3& start,
	const csVector3& end, csVector3& isect, float* pr,
	int* polygon_idx)
{
  if (polygon_idx) *polygon_idx = -1;
  return false;
}

void csParticlesObject::GetObjectBoundingBox (csBox3& bbox, int type)
{
  bbox.StartBoundingBox ();
  switch (type) {
  case CS_BBOX_NORMAL:
    bbox.AddBoundingVertex (-radius, -radius, -radius);
    bbox.AddBoundingVertex (radius, radius, radius);
    break;
  case CS_BBOX_ACCURATE:
    // TODO: Vertices, not just points (and radius)
    /*for (int i=0;i<max_particles;i++) {
      bbox.AddBoundingVertex (positions[i]);
    }*/
    bbox.AddBoundingVertex (-radius, -radius, -radius);
    bbox.AddBoundingVertex (radius, radius, radius);
    break;
  case CS_BBOX_MAX:
    // TODO: Come up with a better estimated maximum
    bbox.AddBoundingVertex (-100, -100, -100);
    bbox.AddBoundingVertex (100, 100, 100);
    break;
  }
}

void csParticlesObject::GetRadius(csVector3 &rad, csVector3 &c)
{
  rad = csVector3(radius,radius,radius);
  c = emitter;
}

void csParticlesObject::Start ()
{
  int start_size = 1000;
  if (initial_particles > start_size) start_size = initial_particles;

  buffer_length = 0;

  point_data.SetLength (start_size);
  for (int i=0;i<start_size;i++) {
    csParticlesData &p = point_data.Get (i);
    p.position.z = FLT_MAX;
    p.color.w = 0.0f;
    p.time_to_live = -1.0f;
  }

  new_particles = (float)initial_particles;
  total_elapsed_time = 0.0f;
}

bool csParticlesObject::Update (float elapsed_time)
{
  float new_radius = 0.0f;

  if (total_elapsed_time < emit_time) {
    total_elapsed_time += elapsed_time;
    new_particles += elapsed_time * (float)particles_per_second;
  }

  dead_particles = 0;

  for (int i=0;i<point_data.Length ();i++)
  {
    csParticlesData &point = point_data[i];

    if (point.time_to_live < 0.0f)
    {
      if (new_particles >= 1.0f)
      {
        // Emission
        csVector3 start;

        switch (emit_type)
        {
        case CS_PART_EMIT_SPHERE:
          start = csVector3((rng.Get() - 0.5) * 2,(rng.Get() - 0.5) * 2,
	    (rng.Get() - 0.5) * 2);
          start.Normalize ();
          start = emitter + (start * ((rng.Get() *
	    (emit_size_1 - emit_size_2)) + emit_size_2));
          break;
        case CS_PART_EMIT_PLANE:
          break;
        case CS_PART_EMIT_BOX:
          break;
        }

        point.position = start;
        point.color = csVector4 (0.0f, 0.0f, 0.0f, 0.0f);
        point.velocity = csVector3 (0.0f, 0.0f, 0.0f);
        point.time_to_live = time_to_live + (time_variation * rng.Get());

        new_particles -= 1.0f;
      }
      else
      {
        // Deletion :(
        point.color.x = 0.0f;
        point.color.y = 0.0f;
        point.color.z = 0.0f;
        point.color.w = 0.0f;
        point.position.z = FLT_MAX;
        dead_particles ++;
        continue;
      }
    }

    // Time until death
    point.time_to_live -= elapsed_time;

    float heat = 1.0f;
    switch (heat_function)
    {
    case CS_PART_HEAT_CONSTANT:
      break;
    case CS_PART_HEAT_TIME_LINEAR:
      if (point.time_to_live < 0.0f) heat = 0.0f;
      else heat = point.time_to_live / (time_to_live + time_variation);
      break;
    case CS_PART_HEAT_SPEED:
      heat = 1.0f;
      if (heat < 0.0f) heat = 0.0f;
      break;
    case CS_PART_HEAT_CALLBACK:
      // TODO: this needs to be re-evaluated
      /*heat = heat_callback (times[i] / time_to_live,
        force_amount / force_range_squared,
        dist_squared / force_range_squared);*/
      break;
    }

    // The color functions
    int color_len;
    if ((color_len=gradient_colors.Length()))
    {
      // With a gradient
      float cref = (1.0f - heat) * (float)(color_len-1);
      int index = (int)floor(cref);
      csColor color1 = gradient_colors.Get(index);
      csColor color2 = color1;
      if (index != color_len - 1)
      {
        color2 = gradient_colors.Get(index + 1);
      }

      float pos = cref - floor(cref);

      point.color.x = ((1.0f - pos) * color1.red) + (pos * color2.red);
      point.color.y = ((1.0f - pos) * color1.green) + (pos * color2.green);
      point.color.z = ((1.0f - pos) * color1.blue) + (pos * color2.blue);
    }
    else
    {
      // With no gradient set, use mesh's base color instead (fade to black)
      point.color.x = basecolor.red * heat;
      point.color.y = basecolor.green * heat;
      point.color.z = basecolor.blue * heat;
    }
    point.color.w = point.time_to_live / (time_to_live + time_variation);

    // For calculating radius
    csVector3 dist_vect = point.position - emitter;
    if (dist_vect.SquaredNorm() > new_radius)
      new_radius = dist_vect.SquaredNorm();
  }
  point_data.Sort (ZSort);

  if (dead_particles > (int)((float)point_data.Length () * 0.70f))
  {
    point_data.Truncate ((point_data.Length () >> 1));
  }
  else if (dead_particles < (int)((float)point_data.Length () * 0.30f))
  {
    int oldlen = point_data.Length ();
    point_data.SetLength ((oldlen << 1));
    for (int i=oldlen;i<point_data.Length ();i++) {
      csParticlesData &p = point_data.Get (i);
      p.position.z = FLT_MAX;
      p.color.w = 0.0f;
      p.time_to_live = -1.0f;
    }
  }

  radius = qsqrt(new_radius);

  return true;
}

void csParticlesObject::NextFrame (csTicks, const csVector3 &)
{
}
