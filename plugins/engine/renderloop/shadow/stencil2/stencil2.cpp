/*
  Copyright (C) 2003 by Jorrit Tyberghein, Daniel Duhprey
            (C) 2003 Marten Svanfeldt
            (C) Hristo Hristov, Boyan Hristov

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
#include "csutil/scf.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/flags.h"
#include "csutil/xmltiny.h"
#include "csgeom/transfrm.h"
#include "csgeom/vector4.h"
#include "csgeom/pmtools.h"
#include "csgeom/math3d.h"
#include "csgfx/renderbuffer.h"

#include "iengine/rview.h"
#include "iengine/camera.h"
#include "iengine/light.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "iengine/sector.h"
#include "imesh/object.h"
#include "iutil/vfs.h"
#include "iutil/plugin.h"
#include "iutil/strset.h"
#include "iutil/document.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "ivideo/rendermesh.h"
#include "ivaria/reporter.h"

//#define SHADOW_CACHE_DEBUG

#define Z_PASS 0
#define Z_FAIL 1

#include "polymesh.h"
#include "stencil2.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csStencil2ShadowStep)
SCF_IMPLEMENTS_INTERFACE (iRenderStep)
SCF_IMPLEMENTS_INTERFACE (iLightRenderStep)
SCF_IMPLEMENTS_INTERFACE (iRenderStepContainer)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (csStencil2ShadowCacheEntry)
SCF_IMPLEMENTS_INTERFACE (iObjectModelListener)
SCF_IMPLEMENT_IBASE_END

csStencil2ShadowCacheEntry::csStencil2ShadowCacheEntry (
  csStencil2ShadowStep* parent, iMeshWrapper* mesh)
{
  SCF_CONSTRUCT_IBASE (0);
  shadow_vertex_buffer = 0;
  shadow_normal_buffer = 0;
  active_index_buffer = 0;

  vertex_count = 0;
  triangle_count = 0;
  edge_count = 0;

  enable_caps = false;

  meshShadows = false;

  csStencil2ShadowCacheEntry::parent = parent;
  meshWrapper = mesh;
  model = 0;
  closedMesh = 0;
  bufferHolder.AttachNew (new csRenderBufferHolder);

  csRef<iObjectModel> model = mesh->GetMeshObject ()->GetObjectModel ();
  model->AddListener (this);
  ObjectModelChanged (model);
}

csStencil2ShadowCacheEntry::~csStencil2ShadowCacheEntry ()
{
  delete closedMesh;
  SCF_DESTRUCT_IBASE();
}

void csStencil2ShadowCacheEntry::UpdateRenderBuffers(csArray<csVector3> & shadow_vertices,
                                                     csArray<int> & shadow_indeces)
{
  int vertex_count = (int)shadow_vertices.Length();
  int index_count = (int)shadow_indeces.Length();

  shadow_vertex_buffer = csRenderBuffer::CreateRenderBuffer (
    vertex_count, CS_BUF_DYNAMIC,
    CS_BUFCOMP_FLOAT, 3);

  shadow_vertex_buffer->CopyInto (&shadow_vertices[0], vertex_count);

  active_index_buffer = csRenderBuffer::CreateIndexRenderBuffer (
    index_count, CS_BUF_DYNAMIC,
    CS_BUFCOMP_UNSIGNED_INT, 0, index_count - 1);

  active_index_buffer->CopyInto (&shadow_indeces[0], index_count);
}

void csStencil2ShadowCacheEntry::ObjectModelChanged (iObjectModel* model)
{
  if (csStencil2ShadowCacheEntry::model != model)
    csStencil2ShadowCacheEntry::model = model;

  meshShadows = false;

  csRef<iPolygonMesh> mesh = model->GetPolygonMeshShadows ();
  if (mesh && mesh->GetPolygonCount () > 0)
  {
    if (closedMesh == 0)
      closedMesh = new csStencil2PolygonMesh ();
    closedMesh->CopyFrom (mesh);

    /* This is really hard method to close object ... must use better one

    const csFlags& meshFlags = mesh->GetFlags ();
    if (meshFlags.Check (CS_POLYMESH_NOTCLOSED) || (!meshFlags.Check (CS_POLYMESH_CLOSED) && 
    !csPolygonMeshTools::IsMeshClosed (mesh)))
    {
    csArray<csMeshedPolygon> newPolys;
    int* vertidx;
    int vertidx_len;
    csPolygonMeshTools::CloseMesh (mesh, newPolys, vertidx, vertidx_len);
    closedMesh->AddPolys (newPolys, vertidx);
    }
    */

    if (!CalculateEdges())
    {
      if (closedMesh)
      {
        delete closedMesh;
        closedMesh = 0;
      }
      return;
    }
  }
  else
  {
    if (closedMesh)
    {
      delete closedMesh;
      closedMesh = 0;
    }
    return;
  }

  meshShadows = true;
}

void csStencil2ShadowCacheEntry::UpdateBuffers ()
{
  bufferHolder->SetRenderBuffer (CS_BUFFER_POSITION, shadow_vertex_buffer);
  bufferHolder->SetRenderBuffer (CS_BUFFER_INDEX, active_index_buffer);
}

bool csStencil2ShadowCacheEntry::CalculateEdges()
{
  size_t i, j;
  csVector3 *vertices = closedMesh->GetVertices();
  size_t vert_count = closedMesh->GetVertexCount();
  csTriangle *triangles = closedMesh->GetTriangles();
  size_t tri_count = closedMesh->GetTriangleCount();

  //Checking for multiple vertices - NEED OPTIMIZATION OR REMOVE IN FUTURE!!!
  for (i = 0; i < vert_count ; i++ )
  {
    for (j = 0; j < tri_count ; j++)
    {
      if (vertices[triangles[j].a] == vertices[i]) triangles[j].a = (int)i;
      if (vertices[triangles[j].b] == vertices[i]) triangles[j].b = (int)i;
      if (vertices[triangles[j].c] == vertices[i]) triangles[j].c = (int)i;
    }
  }

  edges.DeleteAll();
  for (i = 0; i < tri_count; i++) 
  {
    AddEdge(triangles[i].a, triangles[i].b, (int)i);
    AddEdge(triangles[i].b, triangles[i].c, (int)i);
    AddEdge(triangles[i].c, triangles[i].a, (int)i);
  }

  bool result = true;

  int errors_count = 0;
  for (i = 0; i < edges.Length(); i++)
  {
    if ((edges[i]->face_2 == -1) || (edges[i]->face_1 == edges[i]->face_2))
    {
      //printf("\nerror in edge: %d, face1: %d, face2: %d, v1: %d, v2: %d", i, edges[i]->face_1, edges[i]->face_2, edges[i]->v1, edges[i]->v2);
      result = false;
      errors_count++;
      /*
      for (j = 0; j < edges.Length(); j++)
      {
      if ((((edges[j]->v1 == edges[i]->v1) && (edges[j]->v2 == edges[i]->v2)) ||
      ((edges[j]->v1 == edges[i]->v2) && (edges[j]->v2 == edges[i]->v1))) && (i != j))
      {
      printf("\n\tsame index: %d", j);
      }
      }
      */
    }
  }
  if (!result)
  {
    printf("mesh %s is incorrect,total errors: %d \n", meshWrapper->QueryObject()->GetName(), errors_count);
  }

  return result;
}

void csStencil2ShadowCacheEntry::AddEdge(int index_v1, int index_v2, int face_index)
{
  size_t i;
  bool found = false;
  for (i = 0; i < edges.Length(); i++)
  {
    if ((((edges[i]->v1 == index_v1) && (edges[i]->v2 == index_v2)) || 
      ((edges[i]->v1 == index_v2) && (edges[i]->v2 == index_v1))) && 
      (edges[i]->face_2 == -1) && (edges[i]->face_1 != face_index))
    {
      edges[i]->face_2 = face_index;
      found = true;
    }
  }

  if (!found)
  {
    Edge *edge = new Edge();
    edge->v1 = index_v1;
    edge->v2 = index_v2;
    edge->face_1 = face_index;
    edge->face_2 = -1;
    edges.Push(edge);

  }
}

bool csStencil2ShadowCacheEntry::GetShadow(csVector3 &light_pos, float shadow_length, 
                                           bool front_cap, bool extrusion, bool back_cap,
                                           csArray<csVector3> &shadow_vertices, csArray<int> &shadow_indeces)
{
  csArray<csVector3> extruded_vertices;
  csArray<bool> back_faces;

  csVector3 *vertices = closedMesh->GetVertices();
  csTriangle *triangles = closedMesh->GetTriangles();
  size_t tri_count = closedMesh->GetTriangleCount();

  csVector3 *face_normals = closedMesh->GetFaceNormals();

  size_t i;
  int j = 0;

  for (i = 0; i < tri_count; i++) // calculating dark and light faces
  {
    bool bf = (face_normals[i] * (vertices[triangles[i].a] - light_pos)) > 0;
    back_faces.Push(bf);

    //calculating front cap
    if (!bf && front_cap)
    {
      shadow_vertices.Push(vertices[triangles[i].a]);
      shadow_vertices.Push(vertices[triangles[i].b]);
      shadow_vertices.Push(vertices[triangles[i].c]);
      shadow_indeces.Push(j++);
      shadow_indeces.Push(j++);
      shadow_indeces.Push(j++);
    }
  }

  if (extrusion || back_cap)
  {
    //first calculate silhouette 
    silhouette_edges.SetLength(0);
    for (i = 0; i < edges.Length(); i++)
    {
      //if (edges[i]->face_2 > -1)
      //{
      // for silhouette edge face_1 and face_2 should be oposite situated toward the light
      if (back_faces[edges[i]->face_1] ^ back_faces[edges[i]->face_2])
      {
        silhouette_edges.Push ((int)i);
      }
      //}
    }

    if (extrusion) // building indexed triangles for shadow sides (using quads should be wiser!)
    {
      for (i = 0; i < silhouette_edges.Length(); i++ )
      {
        int index = silhouette_edges[i];
        csVector3 v0 = vertices[edges[index]->v1];
        csVector3 v1 = vertices[edges[index]->v2];
        csVector3 e0 = ((vertices[edges[index]->v1] - light_pos))*shadow_length 
          + vertices[edges[index]->v1];
        csVector3 e1 = ((vertices[edges[index]->v2] - light_pos))*shadow_length 
          + vertices[edges[index]->v2];

        if(back_faces[edges[index]->face_1]) // check edge flip
        {
          //building 2 triangles from given 4 vertices
          shadow_vertices.Push(v0);
          shadow_vertices.Push(v1);
          shadow_vertices.Push(e1);
          shadow_vertices.Push(e0);

          shadow_indeces.Push(j++); 
          shadow_indeces.Push(j++);
          shadow_indeces.Push(j++);
          shadow_indeces.Push(j-3);
          shadow_indeces.Push(j-1);
          shadow_indeces.Push(j++);
        }
        else
        {
          shadow_vertices.Push(v1);
          shadow_vertices.Push(v0);
          shadow_vertices.Push(e0);
          shadow_vertices.Push(e1);

          shadow_indeces.Push(j++);
          shadow_indeces.Push(j++);
          shadow_indeces.Push(j++);
          shadow_indeces.Push(j-3);
          shadow_indeces.Push(j-1);
          shadow_indeces.Push(j++);
        }
      }
    }

    // optimization for dark cap - building dark cap from silhouette edges
    if (back_cap && (silhouette_edges.Length() > 0)) 
    {
      csVector3 first_point = ((vertices[edges[silhouette_edges[0]]->v1] - light_pos))*shadow_length
        + vertices[edges[silhouette_edges[0]]->v1];
      for (i = 1; i < silhouette_edges.Length(); i++)
      {
        if (back_faces[edges[silhouette_edges[i]]->face_1]) //check edge flip
        {
          shadow_vertices.Push(first_point);
          csVector3 p1 = ((vertices[edges[silhouette_edges[i]]->v1] - light_pos))*shadow_length
            + vertices[edges[silhouette_edges[i]]->v1];
          csVector3 p2 = ((vertices[edges[silhouette_edges[i]]->v2] - light_pos))*shadow_length
            + vertices[edges[silhouette_edges[i]]->v2];
          shadow_vertices.Push(p1);
          shadow_vertices.Push(p2);
          shadow_indeces.Push(j++);
          shadow_indeces.Push(j++);
          shadow_indeces.Push(j++);
        }
        else
        {
          shadow_vertices.Push(first_point);
          csVector3 p1 = ((vertices[edges[silhouette_edges[i]]->v1] - light_pos))*shadow_length
            + vertices[edges[silhouette_edges[i]]->v1];
          csVector3 p2 = ((vertices[edges[silhouette_edges[i]]->v2] - light_pos))*shadow_length
            + vertices[edges[silhouette_edges[i]]->v2];
          shadow_vertices.Push(p2);
          shadow_vertices.Push(p1);
          shadow_indeces.Push(j++);
          shadow_indeces.Push(j++);
          shadow_indeces.Push(j++);
        }
      }
    }
  }

  return shadow_vertices.Length() && shadow_indeces.Length();
}


//---------------------------------------------------------------------------


csStencil2ShadowStep::csStencil2ShadowStep (csStencil2ShadowType* type) :  
shadowDrawVisCallback ()
{
  SCF_CONSTRUCT_IBASE (0);
  csStencil2ShadowStep::type = type;
  shadowDrawVisCallback.parent = this;
  enableShadows = false;
}

csStencil2ShadowStep::~csStencil2ShadowStep ()
{
  SCF_DESTRUCT_IBASE();
}

void csStencil2ShadowStep::Report (int severity, const char* msg, ...)
{
  va_list args;
  va_start (args, msg);
  csReportV (object_reg, severity, 
    "crystalspace.renderloop.step.shadow.stencil2", msg,
    args);
  va_end (args);
}

bool csStencil2ShadowStep::Initialize (iObjectRegistry* objreg)
{
  object_reg = objreg;
  g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  shmgr = CS_QUERY_REGISTRY (object_reg, iShaderManager);

  const csGraphics3DCaps* caps = g3d->GetCaps();
  enableShadows = caps->StencilShadows;
  if (!enableShadows)
  {
    Report (CS_REPORTER_SEVERITY_NOTIFY, 
      "Renderer does not support stencil shadows");
  }

  csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (object_reg,
    "crystalspace.shared.stringset", iStringSet);
  string_object2world = strings->Request ("object2world transform");

  return true;
}

void csStencil2ShadowStep::Perform (iRenderView* rview, iSector* sector,
                                    csShaderVarStack &stacks)
{
  /// TODO: Report error (no light)
  return;
}

void csStencil2ShadowStep::ModelInFrustum(csVector3 &light_pos, float shadow_length, csPlane3* frustum_planes, 
                                          uint32& frustum_mask, csBox3 &model_bounding_box, bool & front_cap_in_frustum, 
                                          bool & extrusion_in_frustum, bool & back_cap_in_frustum)
{
  //front cap
  uint32 out_clip_mask;
  front_cap_in_frustum = csIntersect3::BoxFrustum(model_bounding_box, 
    frustum_planes, 6, out_clip_mask);

  // back cap
  int planes_count = 0;
  if (frustum_mask == 0x3f)
  {
    planes_count = 5;
  }
  else
    if (frustum_mask == 0x40)
    {
      planes_count = 6;
    }

    csArray<csVector3> projected_points;

    int i;
    for (i = 0; i < 8; i++)
    {
      projected_points.Push(
        ((model_bounding_box.GetCorner(i) - light_pos))*shadow_length 
        + model_bounding_box.GetCorner(i)
        );
    }

    back_cap_in_frustum = true;
    for (i = 0; i < planes_count; i++) 
    {
      bool behind_plane = true;
      size_t v = 0;
      while ((v < projected_points.Length()) && behind_plane) 
      {
        behind_plane = (frustum_planes[i].Classify(projected_points[v]) < 0);
        v++;
      }

      // if all points are behind only one plane
      // then back cap is not in frustum
      if (behind_plane)
      {
        back_cap_in_frustum = false;
        break;
      }
    }

    //extrusion
    for (i = 0; i < 8; i++)
      projected_points.Push(model_bounding_box.GetCorner(i));

    extrusion_in_frustum = true;
    for (i = 0; i < planes_count; i++) 
    {
      bool behind_plane = true;
      size_t v = 0;
      while ((v < projected_points.Length()) && behind_plane) 
      {
        behind_plane = (frustum_planes[i].Classify(projected_points[v]) < 0);
        v++;
      }

      // if all points are behind only one plane
      // then extrusion is not in frustum
      if (behind_plane)
      {
        extrusion_in_frustum = false;
        break;
      }
    }
    //printf("front=%d sides=%d back=%d\n", front_cap_in_frustum, extrusion_in_frustum, back_cap_in_frustum);
}


int csStencil2ShadowStep::CalculateShadowMethod(iRenderView *rview, csVector3 &light_pos, 
                                                const csReversibleTransform &t, csBox3 &model_bounding_box)
{
  float lx, rx, ty, dy;
  rview->GetFrustum(lx, rx, ty, dy);

  // get view corners in object space
  csVector3 vll = t.Other2This(rview->GetCamera()->GetTransform().This2Other(csVector3(lx, dy, 0)));
  csVector3 vlr = t.Other2This(rview->GetCamera()->GetTransform().This2Other(csVector3(rx, dy, 0)));
  csVector3 vul = t.Other2This(rview->GetCamera()->GetTransform().This2Other(csVector3(lx, ty, 0)));
  csVector3 vur = t.Other2This(rview->GetCamera()->GetTransform().This2Other(csVector3(rx, ty, 0)));

  // building oclusion pyramid from light to view corners 
  // and check whether view is shadowed by the object
  // if view is in object shadow we use Z_FAIL tehnique, otherwise Z_PASS

  csArray<csPlane3> oclusion_pyramid;
  oclusion_pyramid.Push(csPlane3(vul, vur, vlr));
  oclusion_pyramid.Push(csPlane3(light_pos, vur, vul));
  oclusion_pyramid.Push(csPlane3(light_pos, vlr, vur));
  oclusion_pyramid.Push(csPlane3(light_pos, vll, vlr));
  oclusion_pyramid.Push(csPlane3(light_pos, vul, vll));

  csVector3 camera_pos = t.Other2This(rview->GetCamera()->GetTransform().GetOrigin());

  csVector3 light_dir = camera_pos - light_pos;

  oclusion_pyramid.Push(csPlane3(light_dir, -(light_dir*light_pos)));

  csVector3 forward_view_vector = 
    t.Other2ThisRelative(rview->GetCamera()->GetTransform().This2OtherRelative(csVector3(0, 0, 1)));

  // flip pyramid planes if light is behind camera
  size_t i;
  if ((light_dir*forward_view_vector) > 0)
  {
    for (i = 0; i < oclusion_pyramid.Length() ; i++)
    {
      oclusion_pyramid[i].Invert();
    }
  }

  for (i = 0; i < oclusion_pyramid.Length(); i++) 
  {
    bool behind_plane = true;
    int v = 0;
    while ((v < 8) && behind_plane) 
    {
      behind_plane = (oclusion_pyramid[i].Classify(model_bounding_box.GetCorner(v)) < 0);
      v++;
    }

    // if all points are behind only one plane
    // then object bounding box is not in oclusion pyramid
    if (behind_plane) 
    {
      return Z_PASS;
    }
  }

  return Z_FAIL;
}

void csStencil2ShadowStep::DrawShadow(iRenderView *rview, int method, csStencil2ShadowCacheEntry *cache_entry, 
                                      iMeshWrapper *mesh, csArray<csVector3> & shadow_vertices, csArray<int> & shadow_indeces, 
                                      iShader* shader, size_t shaderTicket, size_t pass)
{
  if (!cache_entry->MeshCastsShadow() || !cache_entry->ShadowCaps()) return;

  iCamera* camera = rview->GetCamera ();  

  iGraphics3D* g3d = rview->GetGraphics3D ();

  csRenderMesh rmesh;
  rmesh.variablecontext.AttachNew (new csShaderVariableContext);
  rmesh.variablecontext->GetVariableAdd (string_object2world)->SetValue (
    mesh->GetMovable()->GetFullTransform ());
  rmesh.z_buf_mode = CS_ZBUF_TEST;
  rmesh.material = 0;
  rmesh.buffers = cache_entry->bufferHolder;
  rmesh.meshtype = CS_MESHTYPE_TRIANGLES;
  rmesh.indexstart = 0;
  rmesh.indexend = (uint)shadow_indeces.Length();

  cache_entry->UpdateRenderBuffers(shadow_vertices, shadow_indeces);

  //csRenderMeshModes modes (rmesh);

  cache_entry->UpdateBuffers();

  static csShaderVarStack stacks; // @@@ use STATIC macros

  stacks.Empty ();
  shmgr->PushVariables (stacks);
  g3d->SetWorldToCamera (camera->GetTransform ());
  shader->SetupPass (shaderTicket, &rmesh, rmesh, stacks);

  switch (method)
  {
  case Z_PASS:
    g3d->SetShadowState (CS_SHADOW_VOLUME_PASS1);
    g3d->DrawMesh (&rmesh, rmesh, stacks);
    g3d->SetShadowState (CS_SHADOW_VOLUME_PASS2);
    g3d->DrawMesh (&rmesh, rmesh, stacks);
    break;
  case Z_FAIL:
    g3d->SetShadowState (CS_SHADOW_VOLUME_FAIL1);
    g3d->DrawMesh (&rmesh, rmesh, stacks);
    g3d->SetShadowState (CS_SHADOW_VOLUME_FAIL2);
    g3d->DrawMesh (&rmesh, rmesh, stacks);
    break;
  }

  shader->TeardownPass (shaderTicket);
  shmgr->PopVariables (stacks);
}

void csStencil2ShadowStep::Perform (iRenderView* rview, iSector* sector,
                                    iLight* light, csShaderVarStack &stacks)
{
  iShader* shadow;
  if (!enableShadows || ((shadow = type->GetShadow ()) == 0))
  {
    for (size_t i = 0; i < steps.Length (); i++)
    {
      steps[i]->Perform (rview, sector, light, stacks);
    }
    return;
  }

  csSphere lightSphere (light->GetCenter (), light->GetCutoffDistance ());
  csRef<iVisibilityCuller> culler = sector->GetVisibilityCuller ();
  shadowMeshes.Truncate (0);
  culler->VisTest (lightSphere, &shadowDrawVisCallback);
  size_t numShadowMeshes;
  if ((numShadowMeshes = shadowMeshes.Length ()) > 0)
  {
    g3d->SetZMode (CS_ZBUF_TEST);
    g3d->SetShadowState (CS_SHADOW_VOLUME_BEGIN);
    csRenderMeshModes modes;
    modes.z_buf_mode = CS_ZBUF_TEST;
    size_t shaderTicket = shadow->GetTicket (modes, stacks);
    for (size_t p = 0; p < shadow->GetNumberOfPasses (shaderTicket); p ++) 
    {
      shadow->ActivatePass (shaderTicket, p);
      for (size_t m = 0; m < numShadowMeshes; m++)
      {
        iMeshWrapper*& sp = shadowMeshes[m];

        csRef<csStencil2ShadowCacheEntry> shadowCacheEntry = shadowcache.Get (sp, 0);

        if (!shadowCacheEntry) 
        {
          csRef<iObjectModel> model = sp->GetMeshObject ()->GetObjectModel ();
          if (!model) { continue; } // Can't do shadows on this
          shadowCacheEntry = new csStencil2ShadowCacheEntry (this, sp);
          shadowcache.Put (sp, shadowCacheEntry);
        }

        shadowCacheEntry->EnableShadowCaps ();

        if (!shadowCacheEntry->MeshCastsShadow ()) 
        {
          //printf("mesh %s can't cast shadow\n", sp->QueryObject()->GetName());
          continue;
        }

        csBox3 model_bounding_box;
        sp->GetMeshObject()->GetObjectModel()
		->GetObjectBoundingBox(model_bounding_box);

        const csReversibleTransform& tf = sp->GetMovable ()->GetTransform ();

        csPlane3 frustum_planes[6];
        uint32 frustum_mask;
        csReversibleTransform tr_o2c = rview->GetCamera()->GetTransform()/tf;
        rview->SetupClipPlanes(tr_o2c, frustum_planes, frustum_mask);

        float shadow_length = 100;//(light->GetInfluenceRadius() + maxRadius);
        csVector3 light_pos2object = tf.Other2This(light->GetCenter());

        bool front_cap_in_frustum;
        bool extrusion_in_frustum;
        bool back_cap_in_frustum;

        ModelInFrustum(light_pos2object, shadow_length, frustum_planes, frustum_mask, 
          model_bounding_box, front_cap_in_frustum, extrusion_in_frustum, back_cap_in_frustum);

        if (!(front_cap_in_frustum || extrusion_in_frustum || back_cap_in_frustum))
          continue;

        int method = CalculateShadowMethod(rview, light_pos2object, tf, model_bounding_box);

        csArray<csVector3> shadow_vertices;
        csArray<int> shadow_indeces;

        switch (method)
        {
        case Z_PASS:
          if (shadowCacheEntry->GetShadow(light_pos2object, shadow_length,
            false, extrusion_in_frustum, back_cap_in_frustum, 
            shadow_vertices, shadow_indeces))
          {
            DrawShadow(rview, method, shadowCacheEntry, sp, shadow_vertices,
              shadow_indeces,shadow, shaderTicket, p);
          }
          break;

        case Z_FAIL:
          if (shadowCacheEntry->GetShadow(light_pos2object, shadow_length, 
            front_cap_in_frustum, extrusion_in_frustum, back_cap_in_frustum, 
            shadow_vertices, shadow_indeces))
          {
            DrawShadow(rview, method, shadowCacheEntry, sp, shadow_vertices, 
              shadow_indeces, shadow, shaderTicket, p);
          }
          break;
        }
      }
      shadow->DeactivatePass (shaderTicket);
    }

    g3d->SetShadowState (CS_SHADOW_VOLUME_USE);

    for (size_t i = 0; i < steps.Length (); i++)
    {
      steps[i]->Perform (rview, sector, light, stacks);
    }

    g3d->SetShadowState (CS_SHADOW_VOLUME_FINISH);
  }
}

size_t csStencil2ShadowStep::AddStep (iRenderStep* step)
{
  csRef<iLightRenderStep> lrs = 
    SCF_QUERY_INTERFACE (step, iLightRenderStep);
  if (!lrs) return csArrayItemNotFound;
  return steps.Push (lrs);
}

size_t csStencil2ShadowStep::GetStepCount ()
{
  return steps.Length();
}

SCF_IMPLEMENT_IBASE(csStencil2ShadowStep::ShadowDrawVisCallback)
SCF_IMPLEMENTS_INTERFACE(iVisibilityCullerListener)
SCF_IMPLEMENT_IBASE_END

csStencil2ShadowStep::ShadowDrawVisCallback::ShadowDrawVisCallback ()
{
  SCF_CONSTRUCT_IBASE(0);
}

csStencil2ShadowStep::ShadowDrawVisCallback::~ShadowDrawVisCallback ()
{
  SCF_DESTRUCT_IBASE();
}

void csStencil2ShadowStep::ShadowDrawVisCallback::ObjectVisible (
  iVisibilityObject *visobject, iMeshWrapper *mesh, uint32 /*frustum_mask*/)
{
  parent->shadowMeshes.Push (mesh);
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csStencil2ShadowFactory);
SCF_IMPLEMENTS_INTERFACE (iRenderStepFactory);
SCF_IMPLEMENT_EMBEDDED_IBASE_END;

csStencil2ShadowFactory::csStencil2ShadowFactory (iObjectRegistry* object_reg,
                                                  csStencil2ShadowType* type)
{
  SCF_CONSTRUCT_IBASE (0);
  csStencil2ShadowFactory::object_reg = object_reg;
  csStencil2ShadowFactory::type = type;
}

csStencil2ShadowFactory::~csStencil2ShadowFactory ()
{
  SCF_DESTRUCT_IBASE();
}

csPtr<iRenderStep> csStencil2ShadowFactory::Create ()
{
  csStencil2ShadowStep* step = new csStencil2ShadowStep (type);
  step->Initialize (object_reg);
  return csPtr<iRenderStep> (step);
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_FACTORY(csStencil2ShadowType);

csStencil2ShadowType::csStencil2ShadowType (iBase *p) : csBaseRenderStepType (p)
{
  shadowLoaded = false;
}

csStencil2ShadowType::~csStencil2ShadowType ()
{
}

void csStencil2ShadowType::Report (int severity, const char* msg, ...)
{
  va_list args;
  va_start (args, msg);
  csReportV (object_reg, severity, 
    "crystalspace.renderloop.step.shadow.stencil2.type", msg,
    args);
  va_end (args);
}

csPtr<iRenderStepFactory> csStencil2ShadowType::NewFactory ()
{
  return csPtr<iRenderStepFactory> (new csStencil2ShadowFactory (
    object_reg, this));
}

iShader* csStencil2ShadowType::GetShadow ()
{
  if (!shadowLoaded)
  {
    shadowLoaded = true;

    csRef<iPluginManager> plugin_mgr (
      CS_QUERY_REGISTRY (object_reg, iPluginManager));

    // Load the shadow vertex program 
    csRef<iShaderManager> shmgr = CS_QUERY_REGISTRY (object_reg,
      iShaderManager);

    if (!shmgr) 
    {
      shmgr = CS_LOAD_PLUGIN (plugin_mgr,
        "crystalspace.graphics3d.shadermanager",
        iShaderManager);
    }

    if (!shmgr) 
    {
      Report (CS_REPORTER_SEVERITY_ERROR, "Unable to retrieve shader manager!");
      return 0;
    }

    csRef<iShaderCompiler> shcom (shmgr->GetCompiler ("XMLShader"));

    csRef<iVFS> vfs = CS_QUERY_REGISTRY (object_reg, iVFS);
    //csRef<iDataBuffer> buf = vfs->ReadFile ("/shader/shadow.xml");
    csRef<iDataBuffer> buf = vfs->ReadFile ("/shader/shadow2.xml");
    //csRef<iDataBuffer> buf = vfs->ReadFile ("/shader/shadowdebug.xml");
    csRef<iDocumentSystem> docsys (
      CS_QUERY_REGISTRY(object_reg, iDocumentSystem));
    if (docsys == 0)
    {
      docsys.AttachNew (new csTinyDocumentSystem ());
    }

    csRef<iDocument> shaderDoc = docsys->CreateDocument ();
    shaderDoc->Parse (buf, true);

    shadow = shcom->CompileShader (shaderDoc->GetRoot ()->GetNode ("shader"));

    if (!shadow)
    {
      Report (CS_REPORTER_SEVERITY_ERROR, "Unable to load shadow shader");
      return 0;
    }
  }

  return shadow;
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_FACTORY(csStencil2ShadowLoader);

csStencil2ShadowLoader::csStencil2ShadowLoader (iBase *p) 
: csBaseRenderStepLoader (p)
{
  InitTokenTable (tokens);
}

csStencil2ShadowLoader::~csStencil2ShadowLoader ()
{
}

bool csStencil2ShadowLoader::Initialize (iObjectRegistry* object_reg)
{
  if (csBaseRenderStepLoader::Initialize (object_reg))
  {
    return rsp.Initialize (object_reg);
  }
  else
  {
    return false;
  }
}

csPtr<iBase> csStencil2ShadowLoader::Parse (iDocumentNode* node,
                                            iLoaderContext* ldr_context,
                                            iBase* context)
{
  csRef<iPluginManager> plugin_mgr (CS_QUERY_REGISTRY (object_reg,
    iPluginManager));
  csRef<iRenderStepType> type (CS_LOAD_PLUGIN (plugin_mgr,
    "crystalspace.renderloop.step.shadow.stencil2.type", 
    iRenderStepType));

  csRef<iRenderStepFactory> factory = type->NewFactory();
  csRef<iRenderStep> step = factory->Create ();

  csRef<iRenderStepContainer> steps =
    SCF_QUERY_INTERFACE (step, iRenderStepContainer);

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    csStringID id = tokens.Request (child->GetValue ());
    switch (id)
    {
    case XMLTOKEN_STEPS:
      {
        if (!rsp.ParseRenderSteps (steps, child))
          return 0;
      }
      break;
    default:
      if (synldr) synldr->ReportBadToken (child);
      return 0;
    }
  }

  return csPtr<iBase> (step);
}

