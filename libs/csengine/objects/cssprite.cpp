/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein

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
#include "csengine/pol2d.h"
#include "csengine/cssprite.h"
#include "csengine/skeleton.h"
#include "csengine/light.h"
#include "csengine/triangle.h"
#include "csengine/camera.h"
#include "csengine/world.h"
#include "csengine/texture.h"
#include "csengine/sector.h"
#include "csengine/bspbbox.h"
#include "csengine/dumper.h"
#include "csgeom/polyclip.h"
#include "csgeom/fastsqrt.h"
#include "csutil/garray.h"
#include "igraph3d.h"


//--------------------------------------------------------------------------

csFrame::csFrame (int anm_idx, int tex_idx)
{
  name = NULL;
  animation_index = anm_idx;
  texturing_index = tex_idx;
  normals_calculated = false;
}

csFrame::~csFrame ()
{
  CHK (delete [] name);
}

void csFrame::SetName (char * n)
{
  CHK (delete [] name);
  if (n)
  {
    CHK (name = new char [strlen (n)+1]);
    strcpy (name, n);
  }
  else
    name = n;
}

//--------------------------------------------------------------------------

csSpriteAction::csSpriteAction() : frames (8, 8), delays (8, 8)
{
  name = NULL;
}

csSpriteAction::~csSpriteAction()
{
  CHK (delete [] name);
}

void csSpriteAction::SetName (char *n)
{
  CHK (delete [] name);
  if (n)
  {
    CHK (name = new char [strlen (n) + 1]);
    strcpy (name, n);
  }
  else
    name = n;
}

void csSpriteAction::AddFrame (csFrame * f, int d)
{
  frames.Push (f);
  delays.Push ((csSome)d);
}

//--------------------------------------------------------------------------

IMPLEMENT_CSOBJTYPE (csSpriteTemplate, csObject)

csSpriteTemplate::csSpriteTemplate ()
  : csObject (), frames (8, 8), actions (8, 8),
    texels (8, 8), vertices (8, 8), normals (8, 8)
{
  cstxt = NULL;
  emerge_from = NULL;
  skeleton = NULL;

  texel_to_normal = NULL;
  normal_mesh = NULL;

  CHK (texel_mesh = new csTriangleMesh ());

  texel_to_vertex = NULL;
  vertex_mesh = NULL;

  tri_verts = NULL;
}

csSpriteTemplate::~csSpriteTemplate ()
{
  CHK (delete texel_mesh);
  CHK (delete [] emerge_from);
  CHK (delete skeleton);
  CHK (delete tri_verts);

  CHK (delete normal_mesh);
  CHK (delete [] texel_to_normal);

  CHK (delete vertex_mesh);
  CHK (delete [] texel_to_vertex);
}

void csSpriteTemplate::AddVertices (int num)
{
  int frame, vertex;

  CHK (int* ttn = new int [GetNumTexels() + num]);
  if (texel_to_normal != NULL)
  {
    for (vertex = 0; vertex < GetNumTexels(); vertex++)
      ttn [vertex] = texel_to_normal [vertex];
    CHK (delete[] texel_to_normal);
  }
  texel_to_normal = ttn;

  CHK (int* ttv = new int [GetNumTexels() + num]);
  if (texel_to_vertex != NULL)
  {
    for (vertex = 0; vertex < GetNumTexels(); vertex++)
      ttv [vertex] = texel_to_vertex [vertex];
    CHK (delete[] texel_to_vertex);
  }
  texel_to_vertex = ttv;

  for (vertex = 0; vertex < num; vertex++)
  {
    texel_to_normal [GetNumTexels() + vertex] = GetNumNormals  () + vertex;
    texel_to_vertex [GetNumTexels() + vertex] = GetNumVertices () + vertex;
  }

  for (frame = 0; frame < frames.Length(); frame++)
  {
    normals.Get (frame)->SetNumVertices (GetNumNormals () + num);
    texels.Get (frame)->SetNumVertices (GetNumTexels () + num);
    vertices.Get (frame)->SetNumVertices (GetNumVertices () + num);
  }
}

void csSpriteTemplate::AddTriangle (int a, int b, int c)
{
  texel_mesh->AddTriangle (a, b, c);

  if (normal_mesh)
    normal_mesh->AddTriangle
      (texel_to_normal[a], texel_to_normal[b], texel_to_normal[c]);

  if (vertex_mesh)
    vertex_mesh->AddTriangle
      (texel_to_vertex[a], texel_to_vertex[b], texel_to_vertex[c]);
}

void csSpriteTemplate::SetSkeleton (csSkeleton* sk)
{
  CHK (delete skeleton);
  skeleton = sk;
}

csSprite3D* csSpriteTemplate::NewSprite ()
{
  csSprite3D* spr;
  CHK (spr = new csSprite3D ());
  spr->SetTemplate (this);
  spr->SetAction ("default");
  spr->InitSprite ();
  return spr;
}

void csSpriteTemplate::GenerateLOD ()
{
  int i;

  //@@@ turn this into a parameter or member variable?
  int lod_base_frame = 0;

  CHK (csVector3* v = new csVector3[GetNumTexels()]);
  for (i = 0; i < GetNumTexels(); i++)
    v[i] = GetVertex (lod_base_frame, i);
  CHK (csTriangleVertices* verts = new csTriangleVertices (texel_mesh, v, GetNumTexels()));
  CHK (delete [] v);

  CHK (delete [] emerge_from);
  CHK (emerge_from = new int [GetNumTexels()]);
  CHK (int* translate = new int [GetNumTexels()]);
  CHK (csTriangleMesh* new_mesh = new csTriangleMesh (*texel_mesh));

  csLOD::CalculateLOD (new_mesh, verts, translate, emerge_from);

  for (i = 0 ; i < texels.Length () ; i++)
  {
    int j;
    CHK (csVector2* new_texels = new csVector2 [GetNumTexels()]);
    csPoly2D* tx = texels.Get(i);
    for (j = 0 ; j < GetNumTexels() ; j++)
      new_texels[translate[j]] = (*tx)[j];
    for (j = 0 ; j < GetNumTexels() ; j++)
      (*tx)[j] = new_texels[j];
    CHK (delete [] new_texels);
  }

  if (skeleton) skeleton->RemapVertices (translate);

  for (i = 0 ; i < GetNumTriangles () ; i++)
  {
    csTriangle& tr = texel_mesh->GetTriangles()[i];
    tr.a = translate[tr.a];
    tr.b = translate[tr.b];
    tr.c = translate[tr.c];
  }

  if (texel_to_normal != NULL)
  {
    CHK (int* ttn = new int [GetNumTexels()]);
    for (i = 0 ; i < GetNumTexels() ; i++)
      ttn[translate[i]] = texel_to_normal[i];
    CHK (delete [] texel_to_normal);
    texel_to_normal = ttn;
  }
  if (texel_to_vertex != NULL)
  {
    CHK (int* ttv = new int [GetNumTexels()]);
    for (i = 0 ; i < GetNumTexels() ; i++)
      ttv[translate[i]] = texel_to_vertex[i];
    CHK (delete [] texel_to_vertex);
    texel_to_vertex = ttv;
  }

  CHK (delete [] translate);
  CHK (delete verts);
  CHK (delete new_mesh);
}

void csSpriteTemplate::ComputeBoundingBox ()
{
  int frame, vertex;
  for ( frame = 0 ; frame < GetNumFrames () ; frame++ )
  {
    csBox3 box;
    GetFrame(frame)->GetBoundingBox (box);
    box.StartBoundingBox (GetVertex (frame, 0));
    for ( vertex = 1 ; vertex < GetNumVertices() ; vertex++ )
      box.AddBoundingVertexSmart (GetVertex (frame, vertex));
    GetFrame(frame)->SetBoundingBox (box);
  }
  if (skeleton)
    skeleton->ComputeBoundingBox (vertices.Get (0));
    // @@@ should the base frame for the skeleton be a variable?
}

csFrame* csSpriteTemplate::AddFrame ()
{
  CHK (csFrame* fr = new csFrame (frames.Length(), texels.Length()));
  CHK (csPoly3D* nr = new csPoly3D ());
  CHK (csPoly2D* tx = new csPoly2D ());
  CHK (csPoly3D* vr = new csPoly3D ());

  if (frames.Length() > 0)
  {
    nr->SetNumVertices (GetNumNormals  ());
    tx->SetNumVertices (GetNumTexels   ());
    vr->SetNumVertices (GetNumVertices ());
  }

  frames.Push (fr);
  normals.Push (nr);
  texels.Push (tx);
  vertices.Push (vr);

  return fr;
}

csFrame* csSpriteTemplate::FindFrame (char *n)
{
  for (int i = GetNumFrames () - 1; i >= 0; i--)
    if (strcmp (GetFrame (i)->GetName (), n) == 0)
      return GetFrame (i);

  return NULL;
}

csSpriteAction* csSpriteTemplate::AddAction ()
{
  CHK (csSpriteAction* a = new csSpriteAction ());
  actions.Push (a);
  return a;
}

void csSpriteTemplate::SetTexture (csTextureList* textures, const char *texname)
{
  if (!textures)
  {
    CsPrintf (MSG_FATAL_ERROR, "There are no textures defined in this world file!\n");
    fatal_exit (0, true);
    return;
  }
  csTextureHandle* texture = textures->FindByName (texname);
  if (texture == NULL)
  {
    CsPrintf (MSG_FATAL_ERROR, "Couldn't find texture named '%s'!\n", texname);
    fatal_exit (0, true);
    return;
  }
  cstxt = texture;
}

void csSpriteTemplate::ComputeNormals (csFrame* frame, csVector3* object_verts)
{
  // @@@ We only calculated normals once for every frame.
  // Normal calculation is too expensive to do again every time.
  // But maybe we should make this optional for fast systems?
  if (frame->NormalsCalculated ()) return;
  frame->SetNormalsCalculated (true);

  if (!tri_verts)
  {
    CHK (tri_verts = new csTriangleVertices (texel_mesh, object_verts, GetNumTexels()));
  }

  int i, j;
  csTriangle * tris = texel_mesh->GetTriangles();
  int num_triangles = texel_mesh->GetNumTriangles();
  // @@@ Avoid this allocate!
  CHK (csVector3 * tri_normals = new csVector3[num_triangles]);

  // calculate triangle normals
  // get the cross-product of 2 edges of the triangle and normalize it
  for (i = 0; i < num_triangles; i++)
  {
    csVector3 ab = object_verts [tris[i].b] - object_verts [tris[i].a];
    csVector3 bc = object_verts [tris[i].c] - object_verts [tris[i].b];
    tri_normals [i] = ab % bc;
    float norm = tri_normals[i].Norm ();
    if (norm)
      tri_normals[i] /= norm;
  }

  // calculate vertex normals, by averaging connected triangle normals
  int frame_number = frame->GetAnmIndex();
  for (i = 0; i < GetNumTexels(); i++)
  {
    csTriangleVertex &vt = tri_verts->GetVertex (i);
    if (vt.num_con_triangles)
    {
      csVector3 &n = GetNormal (frame_number, i);
      n = csVector3 (0,0,0);
      for (j = 0; j < vt.num_con_triangles; j++)
        n += tri_normals [vt.con_triangles[j]];
      float norm = n.Norm ();
      if (norm)
        n /= norm;
    }
  }
  CHK (delete[] tri_normals);
}


csSpriteAction* csSpriteTemplate::FindAction (const char *n)
{
  for (int i = GetNumActions () - 1; i >= 0; i--)
    if (strcmp (GetAction (i)->GetName (), n) == 0)
      return GetAction (i);

  return NULL;
}

int csSpriteTemplate::MergeVertices (csFrame * frame)
{
  // Minimize the number of 3D coordinates:

  csPoly3D* verts = vertices.Get (frame->GetAnmIndex ());

  // create an array of ints which maps old vertex indices to new ones
  CHK (int* new_vertices = new int [GetNumVertices()]);
  CHK (int* old_vertices = new int [GetNumVertices()]);

  // map the first new vertex to the first old vertex
  new_vertices[0] = 0;

  // set the new vertex counter to one
  int new_vertex_count = 1;

  // FOR each old vertex
  for (int old_vertex = 1; old_vertex < GetNumVertices(); old_vertex++)
  {
    bool unique = true;

    // FOR each new vertex
    for (int new_vertex = 0; new_vertex < new_vertex_count; new_vertex++)
    {
      // IF the vertices have the same coordinates
      if ((*verts)[old_vertex] == (*verts)[new_vertices[new_vertex]])
      {
        //  map this new vertex to this old vertex
        old_vertices[new_vertex] = new_vertex;

        //  next old vertex
        unique = false;
        break;
      }
    }
    if (unique)
    {
      // map this old vertex to a new new vertex
      new_vertices[new_vertex_count] = old_vertex;
      old_vertices[old_vertex] = new_vertex_count;

      // increment the new vertex counter
      new_vertex_count++;
    }
  }
  int redundant_vertex_count = GetNumVertices() - new_vertex_count;

#if 0

  // STOP!  The rest of these steps will no doubt break some things:

  // FOR each animation frame
  for (int frame_number = 0; frame_number < frames.Length(); frame_number++)
  {
    // create a new vertex array
    csPoly3D* newverts = new csPoly3D (new_vertex_count);

    verts = vertices.Get (frame_number);
    // copy the old vertex positions into the new array
    for (int v = 0; v < new_vertex_count; v++)
      (*newverts)[v] = (*verts)[new_vertices[v]];

    // replace the old vertex array with the new one
    vertices.Replace (frame_number, newverts);
  }

  // remap texel_to_vertex array
  int * ttv = new int [GetNumTexels()];
  for (int i = 0; i < GetNumTexels(); i++)
    ttv[i] = old_vertices[texel_to_vertex[i]];
  CHK (delete [] texel_to_vertex);
  texel_to_vertex = ttv;

#endif

  CHK (delete [] new_vertices);
  CHK (delete [] old_vertices);

  return redundant_vertex_count;
}

int csSpriteTemplate::MergeNormals (csFrame * frame)
{
  // Combine normals of adjacent vertices based on one special frame:

  csPoly3D* verts = vertices.Get (frame->GetAnmIndex ());

  // create an array of ints which maps old vertex indices to new ones
  CHK (int* new_vertices = new int [GetNumNormals()]);
  CHK (int* old_vertices = new int [GetNumNormals()]);

  // map the first new vertex to the first old vertex
  new_vertices[0] = 0;

  // set the new vertex counter to one
  int new_vertex_count = 1;

  // FOR each old vertex
  for (int old_vertex = 1; old_vertex < GetNumNormals(); old_vertex++)
  {
    bool unique = true;

    // FOR each new vertex
    for (int new_vertex = 0; new_vertex < new_vertex_count; new_vertex++)
    {
      // IF the vertices have the same coordinates
      if ((*verts)[old_vertex] == (*verts)[new_vertices[new_vertex]])
      {
        //  map this new vertex to this old vertex
        old_vertices[new_vertex] = new_vertex;

        //  next old vertex
        unique = false;
        break;
      }
    }
    if (unique)
    {
      // map this old vertex to a new new vertex
      new_vertices[new_vertex_count] = old_vertex;
      old_vertices[old_vertex] = new_vertex_count;

      // increment the new vertex counter
      new_vertex_count++;
    }
  }
  int redundant_vertex_count = GetNumVertices() - new_vertex_count;

#if 0

  // STOP!  The rest of these steps will no doubt break some things:

  // FOR each animation frame
  for (int frame_number = 0; frame_number < frames.Length(); frame_number++)
  {
    // create a new normals array
    csPoly3D* newverts = new csPoly3D (new_vertex_count);

    verts = normals.Get (frame_number);

    // copy the old normals into the new array
    for (int v = 0; v < new_vertex_count; v++)
      (*newverts)[v] = (*verts)[new_vertices[v]];

    // replace the old normals array with the new one
    normals.Replace (frame_number, newverts);
  }

  // remap texel_to_normal array
  int * ttn = new int [GetNumTexels()];
  for (int i = 0; i < GetNumTexels(); i++)
    ttn[i] = old_vertices[texel_to_normal[i]];
  CHK (delete [] texel_to_normal);
  texel_to_normal = ttn;

#endif

  CHK (delete [] new_vertices);
  CHK (delete [] old_vertices);

  return redundant_vertex_count;
}

int csSpriteTemplate::MergeTexels ()
{
  // Merge identical texel frames:

  int same_count = 0;
  int frame, map, v;
  bool same, unique;
  csPoly2D* tx;

  // start a count and a list of unique texel maps
  int unique_texel_map_count;
  CHK (csPoly2D** unique_texel_maps = new csPoly2D* [frames.Length()]);

  // add the first frame to the unique texel map list
  unique_texel_maps [0] = texels.Get (GetFrame (0)->GetTexIndex ());
  unique_texel_map_count = 1;

  // FOR each frame
  for ( frame = 1;  frame < frames.Length(); frame++ )
  {
    tx = texels.Get(GetFrame(frame)->GetTexIndex());
    unique = true;

    // FOR each unique texel map
    for ( map = 0; map < unique_texel_map_count; map++ )
    {
      // IF this texel map is already in our list
      if (tx == unique_texel_maps [map])
      {
        // next frame
        unique = false;
        break;
      }
      // IF all texture vertices are are the same in both
      same = true;
      for ( v = 0; v < GetNumTexels(); v++ )
      {
        if ((*tx)[v] != (*(unique_texel_maps[map]))[v])
        {
          same = false;
          break;
        }
      }
      if (same)
      {
        // use the texel map already in our list
        GetFrame(frame)->SetTexIndex(map);

        // next frame
        unique = false;
        break;
      }
    }
    // add this frame to the unique texel map list
    if (unique)
    {
      GetFrame(frame)->SetTexIndex(unique_texel_map_count);
      unique_texel_maps[unique_texel_map_count] = tx;
      unique_texel_map_count ++;
    }
  }

  // Delete texel frames which are not in our list of unique texel maps
  for ( frame = 0; frame < texels.Length(); frame++ )
  {
    unique = false;
    tx = texels.Get (GetFrame (frame)->GetTexIndex ());

    for ( map = 0; map < unique_texel_map_count; map++ )
    {
      if ( tx == unique_texel_maps[map])
      {
        unique = true;
        break;
      }
    }
    if (!unique)
    {
      texels.Delete(frame);
      same_count++;
      frame--;
    }
  }

  CHK (delete[] unique_texel_maps);

  return same_count;
}

//=============================================================================

IMPLEMENT_CSOBJTYPE (csSprite, csObject)

csSprite::csSprite () : csObject (), bbox (NULL)
{
  bbox.SetOwner (this);
  dynamiclights = NULL;
  MixMode = CS_FX_COPY;
  defered_num_lights = 0;
  defered_lighting_flags = 0;
  draw_callback = NULL;
  draw_callback2 = NULL;
  is_visible = false;
  camera_cookie = 0;
}

csSprite::~csSprite ()
{
  while (dynamiclights) CHKB (delete dynamiclights);
  csWorld::current_world->UnlinkSprite (this);
  //RemoveFromSectors ();
}

void csSprite::MoveToSector (csSector* s)
{
  RemoveFromSectors ();
  sectors.Push (s);
  s->sprites.Push (this);
  UpdatePolyTreeBBox ();
}

void csSprite::RemoveFromSectors ()
{
  bbox.RemoveFromTree ();
  while (sectors.Length () > 0)
  {
    csSector* ss = (csSector*)sectors.Pop ();
    if (ss)
    {
      int idx = ss->sprites.Find (this);
      if (idx >= 0)
      {
        ss->sprites[idx] = NULL;
        ss->sprites.Delete (idx);
      }
    }
  }
}

/// The list of lights that hit the sprite
static DECLARE_GROWING_ARRAY (light_worktable, csLight*);

void csSprite::UpdateDeferedLighting (const csVector3& pos)
{
  if (defered_num_lights)
  {
    if (defered_num_lights > light_worktable.Limit ())
      light_worktable.SetLimit (defered_num_lights);

    csSector* sect = (csSector*)sectors[0];
    int num_lights = csWorld::current_world->GetNearbyLights (sect,
      pos, defered_lighting_flags,
      light_worktable.GetArray (), defered_num_lights);
    UpdateLighting (light_worktable.GetArray (), num_lights);
  }
}

void csSprite::DeferUpdateLighting (int flags, int num_lights)
{
  defered_num_lights = num_lights;
  defered_lighting_flags = flags;
}

void csSprite::UnlinkDynamicLight (csLightHitsSprite* lp)
{
  if (lp->next_sprite) lp->next_sprite->prev_sprite = lp->prev_sprite;
  if (lp->prev_sprite) lp->prev_sprite->next_sprite = lp->next_sprite;
  else dynamiclights = lp->next_sprite;
  lp->prev_sprite = lp->next_sprite = NULL;
  lp->sprite = NULL;
}

void csSprite::AddDynamicLight (csLightHitsSprite* lp)
{
  lp->next_sprite = dynamiclights;
  lp->prev_sprite = NULL;
  if (dynamiclights) dynamiclights->prev_sprite = lp;
  dynamiclights = lp;
  lp->sprite = this;
}


//=============================================================================

IMPLEMENT_CSOBJTYPE (csSprite3D, csSprite)

/// Static vertex array.
static DECLARE_GROWING_ARRAY (tr_verts, csVector3);
/// Static uv array.
static DECLARE_GROWING_ARRAY (uv_verts, csVector2);
/// The list of fog vertices
static DECLARE_GROWING_ARRAY (fog_verts, G3DFogInfo);
/// The list of object vertices.
static DECLARE_GROWING_ARRAY (obj_verts, csVector3);
/// The list of tween vertices.
static DECLARE_GROWING_ARRAY (tween_verts, csVector3);

csSprite3D::csSprite3D () : csSprite ()
{
  v_obj2world.x = 0;
  v_obj2world.y = 0;
  v_obj2world.z = 0;
  cur_frame = 0;
  tpl = NULL;
  force_otherskin = false;
  cur_action = NULL;
  vertex_colors = NULL;
  skeleton_state = NULL;
  tween_ratio = 0;

  tr_verts.IncRef ();
  uv_verts.IncRef ();
  fog_verts.IncRef ();
  obj_verts.IncRef ();
  tween_verts.IncRef ();
  light_worktable.IncRef ();
}

csSprite3D::~csSprite3D ()
{
  light_worktable.DecRef ();
  uv_verts.DecRef ();
  tr_verts.DecRef ();
  fog_verts.DecRef ();
  obj_verts.DecRef ();
  tween_verts.DecRef ();

  CHK (delete [] vertex_colors);
  CHK (delete skeleton_state);
}

void csSprite3D::SetPosition (const csVector3& p)
{
  v_obj2world = p;
  UpdatePolyTreeBBox ();
}

void csSprite3D::SetTransform (const csMatrix3& matrix)
{
  m_obj2world = matrix;
  m_world2obj = m_obj2world.GetInverse ();
  UpdatePolyTreeBBox ();
}

void csSprite3D::MovePosition (const csVector3& rel)
{
  v_obj2world += rel;
  UpdatePolyTreeBBox ();
}

bool csSprite3D::SetPositionSector (const csVector3 &move_to)
{
  csVector3 old_place(-v_obj2world);
  csOrthoTransform OldPos (csMatrix3(1,0,0,0,1,0,0,0,1), old_place);

  csVector3 new_pos     = move_to;
  csSector* pNewSector;//  = currentSector;

  bool mirror = false;
  pNewSector = ((csSector*)sectors[0])->FollowSegment (OldPos, new_pos, mirror);

  if (pNewSector &&
      ABS (new_pos.x-move_to.x) < SMALL_EPSILON &&
      ABS (new_pos.y-move_to.y) < SMALL_EPSILON &&
      ABS (new_pos.z-move_to.z) < SMALL_EPSILON)
  {
    MoveToSector(pNewSector);
    //Move(new_pos);
/*  v_obj2world=-move_to;//-new_pos;*/
    v_obj2world=new_pos;
    UpdatePolyTreeBBox ();
    return true;
  }
  else
  {
    return false; //Object would leave space...
  }
}

void csSprite3D::Transform (csMatrix3& matrix)
{
  csMatrix3 m = matrix;
  m *= m_obj2world;
  m_obj2world = m;
  m_world2obj = m_obj2world.GetInverse ();
  UpdatePolyTreeBBox ();
}

void csSprite3D::SetTemplate (csSpriteTemplate* tmpl)
{
  tpl = tmpl;
  CHK (delete skeleton_state);
  skeleton_state = NULL;
  if (tmpl->GetSkeleton ()) skeleton_state = (csSkeletonState*)tmpl->GetSkeleton ()->CreateState ();
}

void csSprite3D::SetTexture (const char* name, csTextureList* textures)
{
  force_otherskin = true;
  csTextureHandle* texture = textures->FindByName (name);
  if (texture == NULL)
  {
    CsPrintf (MSG_FATAL_ERROR, "Couldn't find texture named '%s'!\n", name);
    exit (0);
  }
  cstxt = texture;
}

void csSprite3D::SetVertexColor (int i, const csColor& col)
{
  if (!vertex_colors)
  {
    CHK (vertex_colors = new csColor [tpl->GetNumTexels ()]);
    int j;
    for (j = 0 ; j < tpl->GetNumTexels (); j++)
      vertex_colors[j].Set (0, 0, 0);
  }
  vertex_colors[i] = col;
}

void csSprite3D::AddVertexColor (int i, const csColor& col)
{
  if (!vertex_colors)
  {
    CHK (vertex_colors = new csColor [tpl->GetNumTexels ()]);
    int j;
    for (j = 0 ; j < tpl->GetNumTexels (); j++)
      vertex_colors[j].Set (0, 0, 0);
  }
  vertex_colors [i] += col;
}

void csSprite3D::ResetVertexColors ()
{
  if (vertex_colors)
    for (int i = 0 ; i < tpl->GetNumTexels (); i++)
      vertex_colors [i].Set (0, 0, 0);
  //CHK (delete [] vertex_colors);
  //vertex_colors = NULL;
}

void csSprite3D::FixVertexColors ()
{
  if (vertex_colors)
    for (int i = 0 ; i < tpl->GetNumTexels (); i++)
      vertex_colors [i].Clamp (2., 2., 2.);
}

csTriangleMesh csSprite3D::mesh;
float csSprite3D::cfg_lod_detail = 1;
bool csSprite3D::do_quality_lighting = false;

int map (int* emerge_from, int idx, int num_verts)
{
  if (num_verts <= 0) return 0;
  while (idx >= num_verts)
  {
    int idx2 = emerge_from[idx];
    // @@@ THIS SHOULD NOT BE NEEDED! DEBUG WHY IT IS NEEDED
    if (idx == idx2) return idx;
    idx = idx2;
  }
  return idx;
}

void csSprite3D::GenerateSpriteLOD (int num_vts)
{
  int* emerge_from = tpl->GetEmergeFrom ();
  csTriangleMesh* base_mesh = tpl->GetTexelMesh ();
  mesh.Reset ();
  int i;
  int a, b, c;
  for (i = 0 ; i < base_mesh->GetNumTriangles () ; i++)
  {
    csTriangle& tr = base_mesh->GetTriangles ()[i];
    a = map (emerge_from, tr.a, num_vts);
    b = map (emerge_from, tr.b, num_vts);
    c = map (emerge_from, tr.c, num_vts);
    if (a != b && b != c && a != c) mesh.AddTriangle (a, b, c);
  }
}

void csSprite3D::UpdateWorkTables (int max_size)
{
  if (max_size > tr_verts.Limit ())
  {
    tr_verts.SetLimit (max_size);
    uv_verts.SetLimit (max_size);
    fog_verts.SetLimit (max_size);
    obj_verts.SetLimit (max_size);
    tween_verts.SetLimit (max_size);
  }
}

void csSprite3D::UpdatePolyTreeBBox ()
{
  bbox.RemoveFromTree ();

  // If we are not in a sector which has a polygon tree
  // then we don't really update. We should consider if this is
  // a good idea. Do we only want this object updated when we
  // want to use it in a polygon tree? It is certainly more
  // efficient to do it this way when the sprite is currently
  // moving in normal convex sectors.
  int i;
  csPolygonTree* tree = NULL;
  for (i = 0 ; i < sectors.Length () ; i++)
  {
    tree = ((csSector*)sectors[i])->GetStaticTree ();
    if (tree) break;
  }
  if (!tree) return;

  csBox3 b;
  if (skeleton_state)
  {
    skeleton_state->ComputeBoundingBox (csTransform (), b);
  }
  else
  {
    csFrame* cframe = cur_action->GetFrame (cur_frame);
    cframe->GetBoundingBox (b);
  }

  // This transform should be part of the sprite class and not just calculated
  // every time we need it. @@@!!!
  csTransform trans = csTransform (m_obj2world, m_world2obj * -v_obj2world);
  csBspPolygon* poly;

  // Add the eight corner points of the bounding box to the container.
  // Transform from object to world space here.
  csVector3Array& va = bbox.GetVertices ();
  int pt_xyz = va.AddVertex (trans.Other2This (csVector3 (b.MinX (), b.MinY (), b.MinZ ())));
  int pt_Xyz = va.AddVertex (trans.Other2This (csVector3 (b.MaxX (), b.MinY (), b.MinZ ())));
  int pt_xYz = va.AddVertex (trans.Other2This (csVector3 (b.MinX (), b.MaxY (), b.MinZ ())));
  int pt_XYz = va.AddVertex (trans.Other2This (csVector3 (b.MaxX (), b.MaxY (), b.MinZ ())));
  int pt_xyZ = va.AddVertex (trans.Other2This (csVector3 (b.MinX (), b.MinY (), b.MaxZ ())));
  int pt_XyZ = va.AddVertex (trans.Other2This (csVector3 (b.MaxX (), b.MinY (), b.MaxZ ())));
  int pt_xYZ = va.AddVertex (trans.Other2This (csVector3 (b.MinX (), b.MaxY (), b.MaxZ ())));
  int pt_XYZ = va.AddVertex (trans.Other2This (csVector3 (b.MaxX (), b.MaxY (), b.MaxZ ())));

  poly = (csBspPolygon*)csBspPolygon::poly_pool.Alloc ();
  bbox.AddPolygon (poly);
  poly->SetOriginator (this);
  poly->GetPolygon ().AddVertex (pt_xYz);
  poly->GetPolygon ().AddVertex (pt_XYz);
  poly->GetPolygon ().AddVertex (pt_Xyz);
  poly->GetPolygon ().AddVertex (pt_xyz);
  poly->SetPolyPlane (csPlane3 (0, 0, 1, -b.MinZ ()));
  poly->Transform (trans);

  poly = (csBspPolygon*)csBspPolygon::poly_pool.Alloc ();
  bbox.AddPolygon (poly);
  poly->SetOriginator (this);
  poly->GetPolygon ().AddVertex (pt_XYz);
  poly->GetPolygon ().AddVertex (pt_XYZ);
  poly->GetPolygon ().AddVertex (pt_XyZ);
  poly->GetPolygon ().AddVertex (pt_Xyz);
  poly->SetPolyPlane (csPlane3 (-1, 0, 0, b.MaxX ()));
  poly->Transform (trans);

  poly = (csBspPolygon*)csBspPolygon::poly_pool.Alloc ();
  bbox.AddPolygon (poly);
  poly->SetOriginator (this);
  poly->GetPolygon ().AddVertex (pt_XYZ);
  poly->GetPolygon ().AddVertex (pt_xYZ);
  poly->GetPolygon ().AddVertex (pt_xyZ);
  poly->GetPolygon ().AddVertex (pt_XyZ);
  poly->SetPolyPlane (csPlane3 (0, 0, -1, b.MaxZ ()));
  poly->Transform (trans);

  poly = (csBspPolygon*)csBspPolygon::poly_pool.Alloc ();
  bbox.AddPolygon (poly);
  poly->SetOriginator (this);
  poly->GetPolygon ().AddVertex (pt_xYZ);
  poly->GetPolygon ().AddVertex (pt_xYz);
  poly->GetPolygon ().AddVertex (pt_xyz);
  poly->GetPolygon ().AddVertex (pt_xyZ);
  poly->SetPolyPlane (csPlane3 (1, 0, 0, -b.MinX ()));
  poly->Transform (trans);

  poly = (csBspPolygon*)csBspPolygon::poly_pool.Alloc ();
  bbox.AddPolygon (poly);
  poly->SetOriginator (this);
  poly->GetPolygon ().AddVertex (pt_xYZ);
  poly->GetPolygon ().AddVertex (pt_XYZ);
  poly->GetPolygon ().AddVertex (pt_XYz);
  poly->GetPolygon ().AddVertex (pt_xYz);
  poly->SetPolyPlane (csPlane3 (0, -1, 0, b.MaxY ()));
  poly->Transform (trans);

  poly = (csBspPolygon*)csBspPolygon::poly_pool.Alloc ();
  bbox.AddPolygon (poly);
  poly->SetOriginator (this);
  poly->GetPolygon ().AddVertex (pt_xyz);
  poly->GetPolygon ().AddVertex (pt_Xyz);
  poly->GetPolygon ().AddVertex (pt_XyZ);
  poly->GetPolygon ().AddVertex (pt_xyZ);
  poly->SetPolyPlane (csPlane3 (0, 1, 0, -b.MinY ()));
  poly->Transform (trans);

  // Here we need to insert in trees where this sprite lives.
  for (i = 0 ; i < sectors.Length () ; i++)
  {
    tree = ((csSector*)sectors[i])->GetStaticTree ();
    if (tree)
    {
      // Temporarily increase reference to prevent free.
      bbox.GetBaseStub ()->IncRef ();
      tree->AddObject (&bbox);
      bbox.GetBaseStub ()->DecRef ();
    }
  }
}

void csSprite3D::GetObjectBoundingBox (csBox3& obox)
{
  if (skeleton_state)
  {
    skeleton_state->ComputeBoundingBox (csTransform (), obox);
  }
  else
  {
    csFrame* cframe = cur_action->GetFrame (cur_frame);
    cframe->GetBoundingBox (obox);
  }
}

void csSprite3D::GetCameraBoundingBox (const csCamera& camtrans, csBox3& cbox)
{
  csTranCookie cur_cookie = csWorld::current_world->tr_manager.GetCookie ();
  if (camera_cookie == cur_cookie)
  {
    cbox = camera_bbox;
    return;
  }
  camera_cookie = cur_cookie;

  csTransform trans = camtrans * csTransform (m_obj2world,
  	m_world2obj * -v_obj2world);
  if (skeleton_state)
  {
    skeleton_state->ComputeBoundingBox (trans, camera_bbox);
  }
  else
  {
    csFrame* cframe = cur_action->GetFrame (cur_frame);
    csBox3 box;
    cframe->GetBoundingBox (box);
    camera_bbox.StartBoundingBox (trans * box.GetCorner (0));
    camera_bbox.AddBoundingVertexSmart (trans * box.GetCorner (1));
    camera_bbox.AddBoundingVertexSmart (trans * box.GetCorner (2));
    camera_bbox.AddBoundingVertexSmart (trans * box.GetCorner (3));
    camera_bbox.AddBoundingVertexSmart (trans * box.GetCorner (4));
    camera_bbox.AddBoundingVertexSmart (trans * box.GetCorner (5));
    camera_bbox.AddBoundingVertexSmart (trans * box.GetCorner (6));
    camera_bbox.AddBoundingVertexSmart (trans * box.GetCorner (7));
  }
  cbox = camera_bbox;
}

float csSprite3D::GetScreenBoundingBox (const csCamera& camtrans, csBox2& boundingBox)
{
  csVector2   oneCorner;
  csBox3      cbox;

  // @@@ Note. The bounding box created by this function greatly
  // exagerates the real bounding box. However, this function
  // needs to be fast. I'm not sure how to do this more accuratelly.

  GetCameraBoundingBox (camtrans, cbox);

  // if the entire bounding box is behind the camera, we're done
  if ((cbox.MinZ () < 0) && (cbox.MaxZ () < 0))
    return -1;

  // Transform from camera to screen space.
  if (cbox.MinZ () <= 0)
  {
    // Sprite is very close to camera.
    // Just return a maximum bounding box.
    boundingBox.Set (-10000, -10000, 10000, 10000);
  }
  else
  {
    oneCorner.x = cbox.MaxX () / cbox.MaxZ () * camtrans.aspect + camtrans.shift_x;
    oneCorner.y = cbox.MaxY () / cbox.MaxZ () * camtrans.aspect + camtrans.shift_y;
    boundingBox.StartBoundingBox (oneCorner);
    oneCorner.x = cbox.MinX () / cbox.MaxZ () * camtrans.aspect + camtrans.shift_x;
    oneCorner.y = cbox.MinY () / cbox.MaxZ () * camtrans.aspect + camtrans.shift_y;
    boundingBox.AddBoundingVertexSmart (oneCorner);
    oneCorner.x = cbox.MinX () / cbox.MinZ () * camtrans.aspect + camtrans.shift_x;
    oneCorner.y = cbox.MinY () / cbox.MinZ () * camtrans.aspect + camtrans.shift_y;
    boundingBox.AddBoundingVertexSmart (oneCorner);
    oneCorner.x = cbox.MaxX () / cbox.MinZ () * camtrans.aspect + camtrans.shift_x;
    oneCorner.y = cbox.MaxY () / cbox.MinZ () * camtrans.aspect + camtrans.shift_y;
    boundingBox.AddBoundingVertexSmart (oneCorner);
  }

  return cbox.MaxZ ();
}

// New version of sprite drawing routine using DrawTriangleMesh.
void csSprite3D::Draw (csRenderView& rview)
{
  int i;
  if (draw_callback) draw_callback (this, &rview);

  if (!tpl->cstxt)
  {
    CsPrintf (MSG_FATAL_ERROR, "Error! Trying to draw a sprite with no texture!\n");
    fatal_exit (0, false);
  }

  // Test visibility of entire sprite by clipping bounding box against clipper.
  // There are three possibilities:
  //	1. box is not visible -> sprite is not visible.
  //	2. box is entirely visible -> sprite is visible and need not be clipped.
  //	3. box is partially visible -> sprite is visible and needs to be clipped
  //	   if rview has do_clip_plane set to true.
  csBox2 bbox;
  if (GetScreenBoundingBox (rview, bbox) < 0) return;	// Not visible.
  //@@@ Debug output: this should be an optional feature for WalkTest.
  //{
    //csPolygon2D* p2d = new csPolygon2D ();
    //p2d->AddVertex (bbox.GetCorner (0));
    //p2d->AddVertex (bbox.GetCorner (1));
    //p2d->AddVertex (bbox.GetCorner (3));
    //p2d->AddVertex (bbox.GetCorner (2));
    //p2d->Draw (rview.g2d, 255);
    //delete p2d;
  //}

  // Test if we need and should clip to the current portal.
  int box_class;
  box_class = rview.view->ClassifyBox (bbox);
  if (box_class == -1) return; // Not visible.
  bool do_clip = false;
  if (rview.do_clip_plane || rview.do_clip_frustrum)
  {
    if (box_class == 0) do_clip = true;
  }

  // If we don't need to clip to the current portal then we
  // test if we need to clip to the top-level portal.
  // Top-level clipping is always required unless we are totally
  // within the top-level frustrum.
  // IF it is decided that we need to clip here then we still
  // clip to the inner portal. We have to do clipping anyway so
  // why not do it to the smallest possible clip area.
  if (!do_clip)
  {
    box_class = csWorld::current_world->top_clipper->ClassifyBox (bbox);
    if (box_class == 0) do_clip = true;
  }

  UpdateWorkTables (tpl->GetNumTexels());
  UpdateDeferedLighting (GetW2TTranslation ());

  csFrame * cframe = cur_action->GetFrame (cur_frame);

  // Get next frame for animation tweening.
  csFrame * next_frame;
  if (cur_frame + 1 < cur_action->GetNumFrames())
    next_frame = cur_action->GetFrame (cur_frame + 1);
  else
    next_frame = cur_action->GetFrame (0);

  // First create the transformation from object to camera space directly:
  //   W = Mow * O - Vow;
  //   C = Mwc * (W - Vwc)
  // ->
  //   C = Mwc * (Mow * O - Vow - Vwc)
  //   C = Mwc * Mow * O - Mwc * (Vow + Vwc)
  csReversibleTransform tr_o2c = rview * csTransform (m_obj2world, m_world2obj * -v_obj2world);
  rview.g3d->SetObjectToCamera (&tr_o2c);
  rview.g3d->SetClipper (rview.view->GetClipPoly (), rview.view->GetNumVertices ());
  // @@@ This should only be done when aspect changes...
  rview.g3d->SetPerspectiveAspect (rview.aspect);

  rview.g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_USE);

  bool do_tween = false;
  if (!skeleton_state && tween_ratio) do_tween = true;

  // @@@ Can't this copy be avoided?
  int cf_idx = cframe->GetAnmIndex();
  for (i = 0 ; i < tpl->GetNumTexels () ; i++)
  {
    obj_verts[i] = tpl->GetVertex (cf_idx, i);
  }
  if (do_tween)
  {
    int nf_idx = next_frame->GetAnmIndex();
    for (i = 0 ; i < tpl->GetNumTexels () ; i++)
    {
      tween_verts[i] = tpl->GetVertex (nf_idx, i);
    }
  }

  // If we have a skeleton then we transform all vertices through
  // the skeleton. In that case we also include the camera transformation
  // so that the 3D renderer does not need to do it anymore.
  csVector3* verts;
  if (skeleton_state)
  {
    skeleton_state->Transform (tr_o2c, obj_verts.GetArray (), tr_verts.GetArray ());
    verts = tr_verts.GetArray ();
  }
  else
  {
    verts = obj_verts.GetArray ();
  }

  // Calculate the right LOD level for this sprite.
  // Select the appropriate mesh.
  csTriangleMesh* m;
  int* emerge_from = NULL;
  int num_verts;
  float fnum = 0.0f;
  if (cfg_lod_detail < 0 || cfg_lod_detail == 1)
  {
    m = tpl->GetTexelMesh ();
    num_verts = tpl->GetNumTexels ();
  }
  else
  {
    m = &mesh;
    // We calculate the number of vertices to use for this LOD
    // level. The integer part will be the number of vertices.
    // The fractional part will determine how much to morph
    // between the new vertex and the previous last vertex.
    fnum = cfg_lod_detail*(float)(tpl->GetNumTexels()+1);
    num_verts = (int)fnum;
    fnum -= num_verts;  // fnum is now the fractional part.
    GenerateSpriteLOD (num_verts);
    emerge_from = tpl->GetEmergeFrom ();
  }

  // Do vertex morphing if needed.
  for (i = 0 ; i < num_verts ; i++)
  {
    csVector3 v;
    csVector2 uv;
    if (cfg_lod_detail < 0 || cfg_lod_detail == 1 || i < num_verts-1)
    {
      v = verts[i];
      uv = tpl->GetTexel (cf_idx, i);
    }
    else
    {
      // Morph between the last vertex and the one we morphed from.
      v = (1-fnum)*verts[emerge_from[i]] + fnum*verts[i];
      uv = (1-fnum) * tpl->GetTexel (cf_idx, emerge_from[i])
        + fnum * tpl->GetTexel (cf_idx, i);
    }

    uv_verts[i] = uv;
  }

  // Setup the structure for DrawTriangleMesh.
  G3DTriangleMesh mesh;
  if (force_otherskin)
    mesh.txt_handle[0] = cstxt->GetTextureHandle ();
  else
    mesh.txt_handle[0] = tpl->cstxt->GetTextureHandle ();
  mesh.num_vertices = num_verts;
  mesh.vertices[0] = verts;
  mesh.texels[0][0] = uv_verts.GetArray ();
  mesh.vertex_colors[0] = vertex_colors;
  if (do_tween)
  {
    mesh.morph_factor = tween_ratio;
    mesh.num_vertices_pool = 2;
    mesh.vertices[1] = tween_verts.GetArray ();
    mesh.texels[1][0] = uv_verts.GetArray ();
    mesh.vertex_colors[1] = vertex_colors;
  }
  else
  {
    mesh.morph_factor = 0;
    mesh.num_vertices_pool = 1;
  }
  mesh.num_textures = 1;

  mesh.num_triangles = m->GetNumTriangles ();
  mesh.triangles = m->GetTriangles ();

  mesh.use_vertex_color = !!vertex_colors;
  mesh.do_clip = do_clip;
  mesh.do_mirror = rview.IsMirrored ();
  mesh.do_morph_texels = false;
  mesh.do_morph_colors = false;
  mesh.vertex_fog = fog_verts.GetArray ();

  if (skeleton_state)
    mesh.vertex_mode = G3DTriangleMesh::VM_VIEWSPACE;
  else
    mesh.vertex_mode = G3DTriangleMesh::VM_WORLDSPACE;
  mesh.fxmode = MixMode | (vertex_colors ? CS_FX_GOURAUD : 0);

  extern void CalculateFogMesh (csRenderView* rview, csTransform* tr_o2c,
	G3DTriangleMesh& mesh);
  CalculateFogMesh (&rview, &tr_o2c, mesh);

  if (!rview.callback)
    rview.g3d->DrawTriangleMesh (mesh);
  //else
  // @@@ Provide functionality for visible edges here...

  if (draw_callback2)
    draw_callback2 (this, &rview, myOwner);
}

void csSprite3D::InitSprite ()
{
  if (!tpl)
  {
    CsPrintf (MSG_FATAL_ERROR, "There is no defined template for this sprite!\n");
    fatal_exit (0, false);
  }

  if (!cur_action) { SetFrame (0); cur_action = tpl->GetFirstAction (); }

  last_time = csWorld::System->GetTime ();

  m_world2obj = m_obj2world.GetInverse ();

  MixMode = CS_FX_COPY;
}

bool csSprite3D::NextFrame (time_t current_time, bool onestep, bool stoptoend)
{
  bool ret = false;

  if(onestep)
  {
    if(current_time>last_time+cur_action->GetFrameDelay (cur_frame))
    {
      last_time = current_time;
      cur_frame++;
      if (cur_frame >= cur_action->GetNumFrames ())
      {
        if(stoptoend) cur_frame --;
        else cur_frame = 0;
        ret = true;
      }
    }
  }
  else
  {
    while(1)
    {
      if(current_time>last_time+cur_action->GetFrameDelay (cur_frame))
      {
        last_time += cur_action->GetFrameDelay (cur_frame);
        cur_frame++;
        if (cur_frame >= cur_action->GetNumFrames ())
        {
          if(stoptoend)
          {
            cur_frame--;
            ret = true;
            break;
          }
          cur_frame = 0;
          ret = true;
        }        
      }
      else break;
    }
  }

  tween_ratio = (current_time - last_time) / (float)cur_action->GetFrameDelay (cur_frame);

  return ret;
}

csVector3* csSprite3D::GetObjectVerts (csFrame* fr)
{
  UpdateWorkTables (tpl->GetNumTexels ());
  int fr_idx = fr->GetAnmIndex();
  for (int i = 0; i < tpl->GetNumTexels (); i++)
    obj_verts[i] = tpl->GetVertex(fr_idx, i);

  if (skeleton_state)
  {
    UpdateWorkTables (tpl->GetNumTexels());
    skeleton_state->Transform (csTransform (), obj_verts.GetArray (), tr_verts.GetArray ());
    return tr_verts.GetArray ();
  }
  else
    return obj_verts.GetArray ();
}

void csSprite3D::UpdateLighting (csLight** lights, int num_lights)
{
  int i;

  defered_num_lights = 0;

  csFrame* this_frame = cur_action->GetFrame (cur_frame);
  csVector3* work_obj_verts;

  if (tween_ratio)
  {
    UpdateWorkTables (tpl->GetNumTexels ());

    csFrame* next_frame;
    if (cur_frame + 1 < cur_action->GetNumFrames())
      next_frame = cur_action->GetFrame (cur_frame + 1);
    else
      next_frame = cur_action->GetFrame (0);

    int tf_idx = this_frame->GetAnmIndex();
    int nf_idx = next_frame->GetAnmIndex();
    float remainder = 1 - tween_ratio;

    for (i = 0 ; i < tpl->GetNumTexels() ; i++)
      obj_verts[i] = tween_ratio * tpl->GetVertex (tf_idx, i)
        + remainder * tpl->GetVertex (nf_idx, i);

    work_obj_verts = obj_verts.GetArray ();
  }
  else
    work_obj_verts = GetObjectVerts (this_frame);

  tpl->ComputeNormals (this_frame, work_obj_verts);

  ResetVertexColors ();

  // this is so that sprite gets blackened if no light strikes it
  AddVertexColor (0, csColor (0, 0, 0));

  csSector * sect = (csSector*)sectors[0];
  if (sect)
  {
    int r, g, b;
    sect->GetAmbientColor (r, g, b);
    csColor ambient_color (r / 128.0, g / 128.0, b / 128.0);
    for (i = 0 ; i < tpl->GetNumTexels (); i++)
      AddVertexColor (i, ambient_color);
  }

  if (do_quality_lighting)
    UpdateLightingHQ (lights, num_lights, work_obj_verts);
  else
    UpdateLightingLQ (lights, num_lights, work_obj_verts);
}

void csSprite3D::UpdateLightingLQ (csLight** lights, int num_lights, csVector3* object_vertices)
{
  int i, j;
  csFrame* this_frame = cur_action->GetFrame (cur_frame);
  int tf_idx = this_frame->GetAnmIndex();

  csBox3 obox;
  GetObjectBoundingBox (obox);
  csVector3 obj_center = (obox.Min () + obox.Max ()) / 2;
  csVector3 wor_center = m_obj2world * obj_center + v_obj2world;
  csColor color;

  for (i = 0 ; i < num_lights ; i++)
  {
    csColor light_color = lights [i]->GetColor () * (256. / NORMAL_LIGHT_LEVEL);
    float sq_light_radius = lights [i]->GetSquaredRadius ();

    // Compute light position in object coordinates
    csVector3 wor_light_pos = lights [i]->GetCenter ();
    float wor_sq_dist = csSquaredDist::PointPoint (wor_light_pos, wor_center);
    if (wor_sq_dist >= sq_light_radius) continue;

    csVector3 obj_light_pos = m_world2obj * (wor_light_pos - v_obj2world);
    float obj_sq_dist = csSquaredDist::PointPoint (obj_light_pos, obj_center);
    float obj_dist = FastSqrt (obj_sq_dist);
    float wor_dist = FastSqrt (wor_sq_dist);

    for (j = 0 ; j < tpl->GetNumTexels () ; j++)
    {
      csVector3& obj_vertex = object_vertices[j];

      float cosinus;
      if (obj_sq_dist < SMALL_EPSILON)
        cosinus = 1;
      else
        cosinus = (obj_light_pos - obj_vertex) * tpl->GetNormal (tf_idx, j);

      if (cosinus > 0)
      {
        color = light_color;
        if (obj_sq_dist >= SMALL_EPSILON)
          cosinus /= obj_dist;
        if (cosinus < 1)
          color *= cosinus * lights[i]->GetBrightnessAtDistance (wor_dist);
        AddVertexColor (j, color);
      }
    }
  }

  // Clamp all vertice colors to 2.0
  FixVertexColors ();
}

void csSprite3D::UpdateLightingHQ (csLight** lights, int num_lights, csVector3* object_vertices)
{
  int i, j;
  csFrame* this_frame = cur_action->GetFrame (cur_frame);
  int tf_idx = this_frame->GetAnmIndex ();
  csColor color;

  for (i = 0 ; i < num_lights ; i++)
  {
    csColor light_color = lights [i]->GetColor () * (256. / NORMAL_LIGHT_LEVEL);
    float sq_light_radius = lights [i]->GetSquaredRadius ();

    // Compute light position in object coordinates
    csVector3 wor_light_pos = lights [i]->GetCenter ();
    csVector3 obj_light_pos = m_world2obj * (wor_light_pos - v_obj2world);

    for (j = 0 ; j < tpl->GetNumTexels () ; j++)
    {
      csVector3& obj_vertex = object_vertices[j];
      csVector3 wor_vertex = m_obj2world * obj_vertex + v_obj2world;

      // @@@ We have the distance in object space. Can't we use
      // that to calculate the distance in world space as well?
      // These calculations aren't optimal. I have the feeling they
      // can be optimized somewhat.
      float obj_sq_dist = csSquaredDist::PointPoint (obj_light_pos, obj_vertex);
      float wor_sq_dist = csSquaredDist::PointPoint (wor_light_pos, wor_vertex);

      float cosinus;
      if (obj_sq_dist < SMALL_EPSILON)
        cosinus = 1;
      else
        cosinus = (obj_light_pos - obj_vertex) * tpl->GetNormal (tf_idx, j);

      if ((cosinus > 0) && (wor_sq_dist < sq_light_radius))
      {
        color = light_color;
        if (obj_sq_dist >= SMALL_EPSILON)
          cosinus /= FastSqrt (obj_sq_dist);
        if (cosinus < 1)
          color *= cosinus * lights[i]->GetBrightnessAtDistance (FastSqrt (wor_sq_dist));
        AddVertexColor (j, color);
      }
    }
  }

  // Clamp all vertice colors to 2.0
  FixVertexColors ();
}

