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

#include "csqint.h"
#include "csqsqrt.h"

#include "csgeom/box.h"
#include "csgfx/renderbuffer.h"
#include "csutil/cscolor.h"
#include "csutil/randomgen.h"
#include "csutil/util.h"
#include "iengine/camera.h"
#include "iengine/movable.h"
#include "iengine/rview.h"
#include "imesh/particles.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "ivaria/reporter.h"
#include "ivideo/material.h"
#include "ivideo/rndbuf.h"
#include "ivideo/texture.h"
#include "ivideo/txtmgr.h"


#include "particles.h"
#include <limits.h>

CS_LEAKGUARD_IMPLEMENT (csParticlesFactory);
CS_LEAKGUARD_IMPLEMENT (csParticlesObject);
CS_LEAKGUARD_IMPLEMENT (csParticlesObject::eiRenderBufferAccessor);

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
  SCF_IMPLEMENTS_INTERFACE (iParticlesStateBase)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csParticlesFactory::csParticlesFactory (csParticlesType* p,
  iObjectRegistry* objreg) : particles_type(p), object_reg (objreg)
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
  mass_variation = 0.0f;
  dampener = 0.01f;

  autostart = true;

  transform_mode = false;

  physics_plugin = "crystalspace.particles.physics.simple";

  color_method = CS_PART_COLOR_CONSTANT;
  constant_color = csColor4 (1.0f, 1.0f, 1.0f, 1.0f);

  mixmode = CS_FX_COPY;
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
  SCF_IMPLEMENTS_INTERFACE (iParticlesStateBase)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csParticlesObject::eiObjectModel)
  SCF_IMPLEMENTS_INTERFACE (iObjectModel)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csParticlesObject::eiRenderBufferAccessor)
  SCF_IMPLEMENTS_INTERFACE (iRenderBufferAccessor)
SCF_IMPLEMENT_IBASE_END

csParticlesObject::csParticlesObject (csParticlesFactory* p)
  : logparent (0), pFactory (p)
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

  radius_name = strings->Request ("point radius");
  scale_name = strings->Request ("point scale");

  matwrap = p->material;

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
  mass_variation = p->mass_variation;
  dampener = p->dampener;

  particles_per_second = p->particles_per_second;
  initial_particles = p->initial_particles;

  gravity = p->gravity;

  emit_time = p->emit_time;
  time_to_live = p->time_to_live;
  time_variation = p->time_variation;

  diffusion = p->diffusion;

  autostart = p->autostart;

  particle_radius = p->particle_radius;
  radius_changed=false;

  gradient_colors = p->gradient_colors;

  color_method = p->color_method;
  constant_color = p->constant_color;
  base_heat = p->base_heat;
  loop_time = p->loop_time;
  color_callback = p->color_callback;

  transform_mode = p->transform_mode;

  point_data = 0; // This gets created by the physics plugin

  emitter = csVector3(0.0f, 0.0f, 0.0f);
  radius = 1.0f;
  //dead_particles = 0;
  point_sprites = p->g3d->GetCaps ()->SupportsPointSprites;

  running = false;

  svcontext.AttachNew (new csShaderVariableContext);
  bufferHolder.AttachNew (new csRenderBufferHolder);

  scfiRenderBufferAccessor.AttachNew (new eiRenderBufferAccessor (this));

  bufferHolder->SetAccessor (scfiRenderBufferAccessor, (uint32)~0);

  csShaderVariable* sv;
  sv = svcontext->GetVariableAdd (radius_name);
  sv->SetValue (particle_radius);
  sv = svcontext->GetVariableAdd (scale_name);
  sv->SetValue (1.0f);

  LoadPhysicsPlugin (p->physics_plugin);

  mixmode = p->mixmode;

  if(autostart) Start();
}

csParticlesObject::~csParticlesObject ()
{
  if (physics)
    physics->RemoveParticles (&scfiParticlesObjectState);

  SCF_DESTRUCT_EMBEDDED_IBASE (scfiObjectModel)
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiParticlesObjectState)
  SCF_DESTRUCT_IBASE ()
  
  delete [] meshpp;
  delete mesh;
}

csPtr<iMeshObject> csParticlesObject::Clone ()
{
  csParticlesObject* new_obj = new csParticlesObject(pFactory);

  new_obj->vis_cb = vis_cb;
  
  new_obj->matwrap = matwrap;
  new_obj->meshppsize = meshppsize;
  
  new_obj->vis_cb = vis_cb;

  new_obj->tr_o2c = tr_o2c;
  new_obj->rotation_matrix = rotation_matrix;
  new_obj->tricount = tricount;

  new_obj->vertex_name = vertex_name;
  new_obj->color_name = color_name;
  new_obj->texcoord_name = texcoord_name;
  new_obj->index_name = index_name;
  new_obj->radius_name = radius_name;
  new_obj->scale_name = scale_name;

  new_obj->camera_fov = camera_fov;
  new_obj->camera_pixels = camera_pixels;

  new_obj->basecolor = basecolor;

  new_obj->emit_type = emit_type;
  new_obj->emit_size_1 = emit_size_1;
  new_obj->emit_size_2 = emit_size_2;
  new_obj->emit_size_3 = emit_size_3;

  new_obj->force_type = force_type;

  new_obj->force_direction = force_direction;
  new_obj->force_range = force_range;
  new_obj->force_falloff = force_falloff;
  new_obj->force_cone_radius = force_cone_radius;
  new_obj->force_cone_radius_falloff = force_cone_radius_falloff;

  new_obj->force_amount = force_amount;

  new_obj->particles_per_second = particles_per_second;
  new_obj->initial_particles = initial_particles;

  new_obj->gravity = gravity;

  new_obj->emit_time = emit_time;
  new_obj->time_to_live = time_to_live;
  new_obj->time_variation = time_variation;
  
  new_obj->particle_mass = particle_mass;
  new_obj->mass_variation = mass_variation;
  new_obj->dampener = dampener;

  new_obj->autostart = autostart;
  new_obj->running = running;
  new_obj->transform_mode = transform_mode;

  new_obj->diffusion = diffusion;

  new_obj->particle_radius = particle_radius;

  new_obj->gradient_colors = gradient_colors;
  new_obj->loop_time = loop_time;
  new_obj->base_heat = base_heat;
  new_obj->constant_color = constant_color;
  new_obj->color_method = color_method;

  new_obj->buffer_length = buffer_length;

  new_obj->point_sprites = point_sprites;
  new_obj->corners[0] = corners[0];
  new_obj->corners[1] = corners[1];
  new_obj->corners[2] = corners[2];
  new_obj->corners[3] = corners[3];

  new_obj->emitter = emitter;
  new_obj->radius = radius;

  new_obj->flags = flags;
  new_obj->mixmode = mixmode;
  
  return csPtr<iMeshObject> (new_obj);
}

bool csParticlesObject::LoadPhysicsPlugin (const char *plugin_id)
{
  csRef<iPluginManager> plugin_mgr = CS_QUERY_REGISTRY (
    pFactory->object_reg, iPluginManager);

  if(physics)
  {
    physics->RemoveParticles (&scfiParticlesObjectState);
  }

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
  point_data = physics->RegisterParticles (&scfiParticlesObjectState);
  return true;
}

void csParticlesObject::SetParticleRadius (float rad)
{
  particle_radius = rad;
  if (svcontext)
  {
    csShaderVariable* sv = svcontext->GetVariableAdd (radius_name);
    if (sv) sv->SetValue (particle_radius);
    radius_changed=true;
  }
}

void csParticlesObject::PreGetBuffer (csRenderBufferHolder* holder, csRenderBufferName buffer)
{
  if (!holder) return;
  holder->SetRenderBuffer (buffer, GetRenderBuffer(buffer));
}

iRenderBuffer *csParticlesObject::GetRenderBuffer (csRenderBufferName name)
{
  if (!vertex_buffer || buffer_length != point_data->Length ())
  {
    buffer_length = point_data->Length ();
    static const csInterleavedSubBufferOptions interleavedElements[2] =
      {{CS_BUFCOMP_FLOAT, 3}, {CS_BUFCOMP_FLOAT, 4}};
    csRef<iRenderBuffer> buffers[2];
    int bufsize = (int)(point_sprites ? buffer_length : buffer_length * 4);
    masterBuffer = csRenderBuffer::CreateInterleavedRenderBuffers (bufsize, 
      CS_BUF_DYNAMIC, 2, interleavedElements, buffers);
    vertex_buffer = buffers[0];
    color_buffer = buffers[1];

    texcoord_buffer = csRenderBuffer::CreateRenderBuffer (
      bufsize, CS_BUF_DYNAMIC,
      CS_BUFCOMP_FLOAT, 2);
    csVector2 *texcoords = new csVector2 [bufsize];
    unsigned int *indices;
    size_t indices_size;
    if (point_sprites)
    {
      indices_size = buffer_length;
      indices = new unsigned int[indices_size];
      for (size_t i = 0; i < buffer_length; i++)
        indices[i] = (uint)i;
    }
    else
    {
      indices_size = buffer_length * 6;
      int i;
      for (i = 0; i < bufsize - 4; i += 4)
      {
        texcoords[i].x = 0; texcoords[i].y = 0;
        texcoords[i+1].x = 0; texcoords[i+1].y = 1;
        texcoords[i+2].x = 1; texcoords[i+2].y = 1;
        texcoords[i+3].x = 1; texcoords[i+3].y = 0;
      }
      indices = new unsigned int[indices_size];
      int j;
      for (i = 0, j = 0; i < bufsize-4; i += 4, j += 6)
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
    }
    index_buffer = csRenderBuffer::CreateIndexRenderBuffer (
        indices_size, CS_BUF_STATIC,
        CS_BUFCOMP_UNSIGNED_INT, 0, bufsize - 1);
    index_buffer->CopyInto (indices, indices_size);
    delete [] indices;

    texcoord_buffer->CopyInto (texcoords, bufsize);
    delete [] texcoords;
  }
  if (name == CS_BUFFER_POSITION)
  {
    if (point_sprites)
    {
      int len = (int)point_data->Length ();
      vertex_data.SetLength (len);
      for (int i = 0; i < len - 1; i++)
      {
        const csParticlesData &point = point_data->Get(i);
        i_vertex vertex;
        vertex.position = point.position - emitter;
        vertex.color = point.color;
        vertex_data.Put (i, vertex);
      }
    }
    else
    {
      int len = (int)point_data->Length ();
      vertex_data.SetLength (len * 4);
      int i,j;
      for (i = 0, j = 0; i < len - 1; i++, j += 4)
      {
        const csParticlesData &point = point_data->Get(i);
        i_vertex vertex;
        vertex.position = (point.position - emitter) + corners[0];
        vertex.color = point.color;
        vertex_data.Put (j, vertex);
        vertex.position = (point.position - emitter) + corners[1];
        vertex.color = point.color;
        vertex_data.Put (j+1, vertex);
        vertex.position = (point.position - emitter) + corners[2];
        vertex.color = point.color;
        vertex_data.Put (j+2, vertex);
        vertex.position = (point.position - emitter) + corners[3];
        vertex.color = point.color;
        vertex_data.Put (j+3, vertex);
      }
    }
    // Vertex buffer is an interleaved buffer, so copy all the data into it
    masterBuffer->CopyInto (vertex_data.GetArray (), vertex_data.Length ());
    return vertex_buffer;
  }
  else if (name == CS_BUFFER_COLOR)
  {
    return color_buffer;
  }
  else if (name == CS_BUFFER_TEXCOORD0)
  {
    return texcoord_buffer;
  }
  else if (name == CS_BUFFER_INDEX)
  {
    return index_buffer;
  }
  return 0;
}

csRenderMesh** csParticlesObject::GetRenderMeshes (int& n, iRenderView* rview, 
  iMovable* movable, uint32 frustum_mask)
{
  iCamera* cam = rview->GetCamera ();

  csReversibleTransform o2wt;

  if (!transform_mode)
  {
    emitter = movable->GetFullPosition();
    // @@@ The code below is not very efficient!
    o2wt.Identity();
    o2wt.SetOrigin (emitter);
  }
  else
  {
    o2wt = movable->GetFullTransform ();
  }
  // @@@ GetTransform??? Shouldn't this be GetFullTransform?
  rotation_matrix = movable->GetFullTransform ().GetT2O ();

  int vertnum = 0;
  float new_radius = 0.0f;

  size_t i;
  for (i=0 ; i<point_data->Length () ; i++)
  {
    const csParticlesData &point = point_data->Get(i);
    if (point.time_to_live < 0.0f) break;

    vertnum ++;

    // For calculating radius
    csVector3 dist_vect = point.position - emitter;
    if (dist_vect.SquaredNorm() > new_radius)
    {
      new_radius = dist_vect.SquaredNorm();
    }
  }
  //dead_particles = point_data->Length () - vertnum;

  if (vertnum>0) 
  {
    new_radius = csQsqrt (new_radius);
    if (new_radius>radius)
    {
      radius = new_radius;
      scfiObjectModel.ShapeChanged ();
    }
    running = true;
  }
  else
  {
    radius = 0.0f;
    running = false;
  }

  int clip_portal, clip_plane, clip_z_plane;
  rview->CalculateClipSettings (frustum_mask, clip_portal, clip_plane,
  	clip_z_plane);

  tr_o2c = cam->GetTransform () / o2wt;

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
    int fov = csQint (cam->GetFOVAngle ());
    int fov_pixels = cam->GetFOV ();
    if (radius_changed || camera_fov != fov || camera_pixels != fov_pixels)
    {
      camera_fov = fov;
      camera_pixels = fov_pixels;
      float lambda = (float)(fov_pixels / (2.0 * tan (fov / 360.0 * PI)));
      csShaderVariable* sv = svcontext->GetVariable (scale_name);
      sv->SetValue (1.0f / (lambda * particle_radius * 3));
      radius_changed=false;
    }
  }

  if (!mesh)
    mesh = new csRenderMesh;

  mesh->mixmode = mixmode;
  mesh->clip_plane = clip_plane;
  mesh->clip_portal = clip_portal;
  mesh->clip_z_plane = clip_z_plane;
  mesh->do_mirror = rview->GetCamera()->IsMirrored();
  matwrap->Visit ();
  mesh->material = matwrap;
  mesh->worldspace_origin = o2wt.GetOrigin ();
  mesh->indexstart = 0;
  mesh->indexend = vertnum;
  mesh->variablecontext = svcontext;
  mesh->object2world = o2wt;
  mesh->buffers = bufferHolder;
  if (point_sprites)
    mesh->meshtype = CS_MESHTYPE_POINT_SPRITES;
  else
    mesh->meshtype = CS_MESHTYPE_TRIANGLES;

  n = 1;
  if (!meshpp)
  {
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

void csParticlesObject::GetObjectBoundingBox (csBox3& bbox)
{
  bbox.Set (csVector3 (-radius, -radius, -radius),
  	csVector3 (radius, radius, radius));
}

void csParticlesObject::SetObjectBoundingBox (const csBox3&)
{
  // @@@ TODO
}

void csParticlesObject::GetRadius(csVector3 &rad, csVector3 &c)
{
  rad = csVector3(radius,radius,radius);
  c = emitter;
}

void csParticlesObject::Start ()
{
  if(point_data->Length () < 1) {
    buffer_length = 0;
  }
  physics->Start (&scfiParticlesObjectState);
  running = true;
}

void csParticlesObject::Stop ()
{
  physics->Stop (&scfiParticlesObjectState);
}

void csParticlesObject::NextFrame (csTicks, const csVector3 &)
{
}
