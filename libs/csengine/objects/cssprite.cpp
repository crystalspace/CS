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

#include "cssysdef.h"
#include "csengine/pol2d.h"
#include "csengine/cssprite.h"
#include "csengine/skeleton.h"
#include "csengine/light.h"
#include "csengine/triangle.h"
#include "csengine/camera.h"
#include "csengine/engine.h"
#include "csengine/texture.h"
#include "csengine/sector.h"
#include "csengine/bspbbox.h"
#include "csengine/dumper.h"
#include "csgeom/polyclip.h"
#include "csutil/garray.h"
#include "csutil/rng.h"
#include "igraph3d.h"
#include "qsqrt.h"

// Set the default lighting quality.
// See header file for CS_SPR_LIGHTING_* definitions.
//#define DEFAULT_LIGHTING CS_SPR_LIGHTING_HQ
#define DEFAULT_LIGHTING CS_SPR_LIGHTING_LQ
//#define DEFAULT_LIGHTING CS_SPR_LIGHTING_FAST

// Set the default lod used.
#define DEFAULT_LOD -1

//--------------------------------------------------------------------------

IMPLEMENT_IBASE (csFrame)
  IMPLEMENTS_INTERFACE (iSpriteFrame)
IMPLEMENT_IBASE_END

csFrame::csFrame (int anm_idx, int tex_idx)
{
  CONSTRUCT_IBASE (NULL);
  name = NULL;
  animation_index = anm_idx;
  texturing_index = tex_idx;
  normals_calculated = false;
}

csFrame::~csFrame ()
{
  delete [] name;
}

void csFrame::SetName (char const* n)
{
  delete [] name;
  if (n)
  {
    name = new char [strlen (n)+1];
    strcpy (name, n);
  }
  else
    name = 0;
}

//--------------------------------------------------------------------------

IMPLEMENT_IBASE (csSpriteAction)
  IMPLEMENTS_INTERFACE (iSpriteAction)
IMPLEMENT_IBASE_END

csSpriteAction::csSpriteAction() : frames (8, 8), delays (8, 8)
{
  CONSTRUCT_IBASE (NULL);
  name = NULL;
}

csSpriteAction::~csSpriteAction()
{
  delete [] name;
}

void csSpriteAction::SetName (char const* n)
{
  delete [] name;
  if (n)
  {
    name = new char [strlen (n) + 1];
    strcpy (name, n);
  }
  else
    name = 0;
}

void csSpriteAction::AddCsFrame (csFrame * f, int d)
{
  frames.Push (f);
  delays.Push ((csSome)d);
}

void csSpriteAction::AddFrame (iSpriteFrame * f, int d)
{
  frames.Push ((csFrame*)f);
  delays.Push ((csSome)d);
}

//--------------------------------------------------------------------------

bool csFrameVector::FreeItem (csSome Item)
{
  delete (csFrame *) Item;
  return true;
}

csFrameVector::~csFrameVector ()
{
  DeleteAll ();
}

bool csActionVector::FreeItem (csSome Item)
{
  delete (csSpriteAction *) Item;
  return true;
}

csActionVector::~csActionVector ()
{
  DeleteAll ();
}

//--------------------------------------------------------------------------

IMPLEMENT_IBASE (csSpriteTemplate)
  IMPLEMENTS_EMBEDDED_INTERFACE (iSpriteTemplate)
  IMPLEMENTS_EMBEDDED_INTERFACE (iSprite3DFactoryState)
IMPLEMENT_IBASE_END

IMPLEMENT_EMBEDDED_IBASE (csSpriteTemplate::SpriteTemplate)
  IMPLEMENTS_INTERFACE (iSpriteTemplate)
IMPLEMENT_EMBEDDED_IBASE_END

IMPLEMENT_EMBEDDED_IBASE (csSpriteTemplate::Sprite3DFactoryState)
  IMPLEMENTS_INTERFACE (iSprite3DFactoryState)
IMPLEMENT_EMBEDDED_IBASE_END

IMPLEMENT_CSOBJTYPE (csSpriteTemplate, csPObject)

csSpriteTemplate::csSpriteTemplate ()
  : csPObject (),
    texels (8, 8), vertices (8, 8), normals (8, 8)
{
  CONSTRUCT_EMBEDDED_IBASE (scfiSpriteTemplate);
  CONSTRUCT_EMBEDDED_IBASE (scfiSprite3DFactoryState);
  cstxt = NULL;
  emerge_from = NULL;
  skeleton = NULL;

  texel_mesh = new csTriangleMesh ();

  tri_verts = NULL;
  do_tweening = true;
  lighting_quality = DEFAULT_LIGHTING;
  lighting_quality_config = CS_SPR_LIGHT_GLOBAL;
  
  lod_level = DEFAULT_LOD;
  lod_level_config = CS_SPR_LOD_GLOBAL;
  
}

csSpriteTemplate::~csSpriteTemplate ()
{
  delete texel_mesh;
  delete [] emerge_from;
  delete skeleton;
  delete tri_verts;
}

void csSpriteTemplate::AddVertices (int num)
{
  int frame;

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
}

void csSpriteTemplate::SetSkeleton (csSkeleton* sk)
{
  delete skeleton;
  skeleton = sk;
}

csSprite3D* csSpriteTemplate::NewSprite (csObject* parent)
{
  csSprite3D* spr;
  spr = new csSprite3D (parent);
  spr->SetTemplate (this);
  spr->EnableTweening (do_tweening);
  
  // Set the quality config of the sprite to that of the template.
  spr->SetLightingQualityConfig( GetLightingQualityConfig() );
  spr->SetAction ("default");
  spr->InitSprite ();
  return spr;
}

void csSpriteTemplate::GenerateLOD ()
{
  int i;

  //@@@ turn this into a parameter or member variable?
  int lod_base_frame = 0;

  csVector3* v = new csVector3[GetNumTexels()];

  for (i = 0; i < GetNumTexels(); i++)
    v[i] = GetVertex (lod_base_frame, i);

  csTriangleVertices* verts = new csTriangleVertices (texel_mesh, v, GetNumTexels());
  delete [] v;

  delete [] emerge_from;
  emerge_from = new int [GetNumTexels()];
  int* translate = new int [GetNumTexels()];
  csTriangleMesh* new_mesh = new csTriangleMesh (*texel_mesh);

  csLOD::CalculateLOD (new_mesh, verts, translate, emerge_from);

  for (i = 0 ; i < texels.Length () ; i++)
  {
    int j;
    csVector2* new_texels = new csVector2 [GetNumTexels ()];
    csVector3* new_vertices = new csVector3 [GetNumTexels ()];
    csVector3* new_normals = new csVector3 [GetNumTexels ()];
    csPoly2D* tx = texels.Get (i);
    csPoly3D* vt = vertices.Get (i);
    csPoly3D* vn = normals.Get (i);
    for (j = 0 ; j < GetNumTexels () ; j++)
    {
      new_texels[translate[j]] = (*tx)[j];
      new_vertices[translate[j]] = (*vt)[j];
      new_normals[translate[j]] = (*vn)[j];
    }
    for (j = 0 ; j < GetNumTexels () ; j++)
    {
      (*tx)[j] = new_texels[j];
      (*vt)[j] = new_vertices[j];
      (*vn)[j] = new_normals[j];
    }
    delete [] new_texels;
    delete [] new_vertices;
    delete [] new_normals;
  }

  if (skeleton) skeleton->RemapVertices (translate);

  for (i = 0 ; i < GetNumTriangles () ; i++)
  {
    csTriangle& tr = texel_mesh->GetTriangles()[i];
    tr.a = translate[tr.a];
    tr.b = translate[tr.b];
    tr.c = translate[tr.c];
  }

  delete [] translate;
  delete verts;
  delete new_mesh;
}

void csSpriteTemplate::ComputeBoundingBox ()
{
  int frame, vertex;

  for ( frame = 0 ; frame < GetNumFrames () ; frame++ )
  {
    csBox3 box;
    GetFrame(frame)->GetBoundingBox (box);

    box.StartBoundingBox (GetVertex (frame, 0));
    for ( vertex = 1 ; vertex < GetNumTexels() ; vertex++ )
      box.AddBoundingVertexSmart (GetVertex (frame, vertex));

    GetFrame(frame)->SetBoundingBox (box);
  }
  if (skeleton)
    skeleton->ComputeBoundingBox (vertices.Get (0));
    // @@@ should the base frame for the skeleton be a variable?
}

csFrame* csSpriteTemplate::AddFrame ()
{
  csFrame* fr = new csFrame (frames.Length(), texels.Length());
  csPoly3D* nr = new csPoly3D ();
  csPoly2D* tx = new csPoly2D ();
  csPoly3D* vr = new csPoly3D ();

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

csFrame* csSpriteTemplate::FindFrame (const char *n)
{
  for (int i = GetNumFrames () - 1; i >= 0; i--)
    if (strcmp (GetFrame (i)->GetName (), n) == 0)
      return GetFrame (i);

  return NULL;
}

csSpriteAction* csSpriteTemplate::AddAction ()
{
  csSpriteAction* a = new csSpriteAction ();
  actions.Push (a);
  return a;
}

void csSpriteTemplate::SetMaterial (csMaterialWrapper *material)
{
  cstxt = material;
}

void csSpriteTemplate::ComputeNormals (csFrame* frame)
{
  int i, j;

  // @@@ We only calculated normals once for every frame.
  // Normal calculation is too expensive to do again every time.
  // But maybe we should make this optional for fast systems?
  if (frame->NormalsCalculated ()) return;
  frame->SetNormalsCalculated (true);

  csVector3* object_verts = GetVertices (frame->GetAnmIndex());

  if (!tri_verts)
  {
    tri_verts = new csTriangleVertices (texel_mesh, object_verts, GetNumTexels());
  }

  csTriangle * tris = texel_mesh->GetTriangles();
  int num_triangles = texel_mesh->GetNumTriangles();
  // @@@ Avoid this allocate!
  csVector3 * tri_normals = new csVector3[num_triangles];

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

  delete[] tri_normals;
}

void csSpriteTemplate::MergeNormals ()
{
  int i;
  for (i = 0; i < GetNumFrames (); i++)
    MergeNormals (i, i);
}
void csSpriteTemplate::MergeNormals (int base)
{
  if (base > GetNumFrames())
  {
    CsPrintf (MSG_WARNING, "No frame number: \n", base);
    CsPrintf (MSG_WARNING, "no smoothing performed\n");
    return;
  }
  int i;
  for (i = 0; i < GetNumFrames (); i++)
    MergeNormals (base, i);
}
void csSpriteTemplate::MergeNormals (int base, int frame)
{
  int i, j;

  int num_frames = GetNumFrames();
  if (base  > num_frames) CsPrintf (MSG_WARNING, "No frame number: \n", base);
  if (frame > num_frames) CsPrintf (MSG_WARNING, "No frame number: \n", frame);
  if (frame > num_frames || base > num_frames)
  {
    CsPrintf (MSG_WARNING, "no smoothing performed\n");
    return;
  }

  GetFrame(frame)->SetNormalsCalculated (true);

  csVector3* obj_verts  = GetVertices (frame);
  csVector3* base_verts = GetVertices (base);

  if (!tri_verts)
  {
    tri_verts = new csTriangleVertices (texel_mesh, obj_verts, GetNumTexels());
  }

  csTriangle * tris = texel_mesh->GetTriangles();
  int num_triangles = texel_mesh->GetNumTriangles();
  // @@@ Avoid this allocate!
  csVector3 * tri_normals = new csVector3[num_triangles];

  // calculate triangle normals
  // get the cross-product of 2 edges of the triangle and normalize it
  for (i = 0; i < num_triangles; i++)
  {
    csVector3 ab = obj_verts [tris[i].b] - obj_verts [tris[i].a];
    csVector3 bc = obj_verts [tris[i].c] - obj_verts [tris[i].b];
    tri_normals [i] = ab % bc;
    float norm = tri_normals[i].Norm ();
    if (norm)
      tri_normals[i] /= norm;
  }

  // create a table that maps each vertex to the
  // first vertex that has the same coordinates
  int * merge = new int [GetNumTexels()];
  for (i = 0; i < GetNumTexels(); i++)
  {
    merge[i] = i;
    for (j = 0; j < i; j++)
    {
      csVector3 difference = base_verts[i] - base_verts[j];
      if (difference.Norm () < 0.01)
      {
        merge[i] = j;
        break;
      }
    }
  }

  // create a mesh which only uses the vertex indices in the table
  csTriangleMesh merge_mesh;
  for (i = 0; i < num_triangles; i++)
    merge_mesh.AddTriangle (merge[tris[i].a], merge[tris[i].b], merge[tris[i].c]);
  csTriangleVertices * tv = new csTriangleVertices (&merge_mesh, obj_verts, GetNumTexels());

  // calculate vertex normals, by averaging connected triangle normals
  for (i = 0; i < GetNumTexels(); i++)
  {
    csTriangleVertex &vt = tv->GetVertex (i);
    if (vt.num_con_triangles)
    {
      csVector3 &n = GetNormal (frame, i);
      n = csVector3 (0,0,0);
      for (j = 0; j < vt.num_con_triangles; j++)
        n += tri_normals [vt.con_triangles[j]];
      float norm = n.Norm ();
      if (norm)
        n /= norm;
    }
  }

  // one last loop to fill in all of the merged vertex normals
  for (i = 0; i < GetNumTexels(); i++)
  {
  	csVector3 &n = GetNormal (frame, i);
	n = GetNormal (frame, merge [i]);
  }

  delete[] tri_normals;
  delete[] merge;
  delete tv;
}


csSpriteAction* csSpriteTemplate::FindAction (const char *n)
{
  for (int i = GetNumActions () - 1; i >= 0; i--)
    if (strcmp (GetAction (i)->GetName (), n) == 0)
      return GetAction (i);

  return NULL;
}

void csSpriteTemplate::HardTransform (const csReversibleTransform& t)
{
  int num = GetNumVertices ();
  int numf = GetNumFrames ();
  int i, j;
  for (i = 0 ; i < numf ; i++)
  {
    csVector3* verts = GetVertices (i);
    for (j = 0 ; j < num ; j++)
      verts[j] = t.This2Other (verts[j]);
  }
}

void csSpriteTemplate::Sprite3DFactoryState::EnableSkeletalAnimation ()
{
  csSkeleton* skel = new csSkeleton ();
  scfParent->SetSkeleton (skel);
}

iSkeleton* csSpriteTemplate::Sprite3DFactoryState::GetSkeleton ()
{
  return QUERY_INTERFACE_SAFE (scfParent->GetSkeleton (), iSkeleton);
}

//=============================================================================

IMPLEMENT_CSOBJTYPE (csSprite, csPObject)

IMPLEMENT_IBASE_EXT (csSprite)
  IMPLEMENTS_EMBEDDED_INTERFACE (iSprite)
IMPLEMENT_IBASE_EXT_END

IMPLEMENT_EMBEDDED_IBASE (csSprite::Sprite)
  IMPLEMENTS_INTERFACE (iSprite)
IMPLEMENT_EMBEDDED_IBASE_END

csSprite::csSprite (csObject* theParent) : csPObject ()
{
  CONSTRUCT_EMBEDDED_IBASE (scfiSprite);
  movable.scfParent = this;
  MixMode = CS_FX_COPY;
  defered_num_lights = 0;
  defered_lighting_flags = 0;
  draw_callback = NULL;
  draw_callback2 = NULL;
  is_visible = false;
  camera_cookie = 0;
  ptree_obj = NULL;
  myOwner = NULL;
  parent = theParent;
  movable.SetObject (this);
  if (parent->GetType () >= csSprite::Type)
  {
    csSprite* sparent = (csSprite*)parent;
    movable.SetParent (&sparent->GetMovable ());
  }
  csEngine::current_engine->AddToCurrentRegion (this);
}

csSprite::~csSprite ()
{
  if (parent->GetType () == csEngine::Type)
  {
    csEngine* engine = (csEngine*)parent;
    engine->UnlinkSprite (this);
  }
}

void csSprite::UpdateMove ()
{
  UpdateInPolygonTrees ();
}

void csSprite::MoveToSector (csSector* s)
{
  s->sprites.Push (this);
}

void csSprite::RemoveFromSectors ()
{
  if (GetPolyTreeObject ())
    GetPolyTreeObject ()->RemoveFromTree ();
  if (parent->GetType () != csEngine::Type) return;
  int i;
  csVector& sectors = movable.GetSectors ();
  for (i = 0 ; i < sectors.Length () ; i++)
  {
    csSector* ss = (csSector*)sectors[i];
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
static DECLARE_GROWING_ARRAY_REF (light_worktable, csLight*);

void csSprite::UpdateDeferedLighting (const csVector3& pos)
{
  if (defered_num_lights)
  {
    if (defered_num_lights > light_worktable.Limit ())
      light_worktable.SetLimit (defered_num_lights);

    csSector* sect = movable.GetSector (0);
    int num_lights = csEngine::current_engine->GetNearbyLights (sect,
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

bool csSprite::HitBeam (const csVector3& start, const csVector3& end,
	csVector3& isect, float* pr)
{
  const csReversibleTransform& trans = movable.GetTransform ();
  csVector3 startObj = trans * start;
  csVector3 endObj = trans * end;
  bool rc = HitBeamObject (startObj, endObj, isect, pr);
  if (rc)
    isect = trans.This2Other (isect);
  return rc;
}

void csSprite::ScaleBy (float factor)
{
  csMatrix3 trans = movable.GetTransform ().GetT2O ();
  trans.m11 *= factor;
  trans.m22 *= factor;
  trans.m33 *= factor;
  movable.SetTransform (trans);
  UpdateMove ();
}


void csSprite::Rotate (float angle)
{
  csZRotMatrix3 rotz(angle);
  movable.Transform (rotz);
  csXRotMatrix3 rotx(angle);
  movable.Transform (rotx);
  UpdateMove ();
}

//=============================================================================

IMPLEMENT_CSOBJTYPE (csSprite3D, csSprite)

IMPLEMENT_IBASE_EXT (csSprite3D)
  IMPLEMENTS_EMBEDDED_INTERFACE (iPolygonMesh)
IMPLEMENT_IBASE_EXT_END

IMPLEMENT_EMBEDDED_IBASE (csSprite3D::PolyMesh)
  IMPLEMENTS_INTERFACE (iPolygonMesh)
IMPLEMENT_EMBEDDED_IBASE_END

/// Static vertex array.
static DECLARE_GROWING_ARRAY_REF (tr_verts, csVector3);
/// Static uv array.
static DECLARE_GROWING_ARRAY_REF (uv_verts, csVector2);
/// The list of fog vertices
static DECLARE_GROWING_ARRAY_REF (fog_verts, G3DFogInfo);
/// The list of object vertices.
static DECLARE_GROWING_ARRAY_REF (obj_verts, csVector3);
/// The list of tween vertices.
static DECLARE_GROWING_ARRAY_REF (tween_verts, csVector3);

csSprite3D::csSprite3D (csObject* theParent) : csSprite (theParent), bbox (NULL)
{
  CONSTRUCT_EMBEDDED_IBASE (scfiPolygonMesh);
  bbox.SetOwner (this);
  ptree_obj = &bbox;
  cur_frame = 0;
  tpl = NULL;
  force_otherskin = false;
  cur_action = NULL;
  vertex_colors = NULL;
  skeleton_state = NULL;
  tween_ratio = 0;
  num_verts_for_lod = -1;

  tr_verts.IncRef ();
  uv_verts.IncRef ();
  fog_verts.IncRef ();
  obj_verts.IncRef ();
  tween_verts.IncRef ();
  light_worktable.IncRef ();
  
  rand_num = new csRandomGen();
  
  do_tweening = true;
}

csSprite3D::~csSprite3D ()
{
  light_worktable.DecRef ();
  uv_verts.DecRef ();
  tr_verts.DecRef ();
  fog_verts.DecRef ();
  obj_verts.DecRef ();
  tween_verts.DecRef ();

  delete [] vertex_colors;
  delete skeleton_state;
  delete rand_num;
}

void csSprite3D::SetTemplate (csSpriteTemplate* tmpl)
{
  tpl = tmpl;
  delete skeleton_state;
  skeleton_state = NULL;
  if (tmpl->GetSkeleton ())
    skeleton_state = (csSkeletonState*)tmpl->GetSkeleton ()->CreateState ();
  EnableTweening (tmpl->IsTweeningEnabled ());
}

void csSprite3D::SetMaterial (csMaterialWrapper *material)
{
  force_otherskin = true;
  cstxt = material;
}

void csSprite3D::SetColor (const csColor& col)
{
  for (int i=0 ; i < tpl->GetNumTexels () ; i++)
    SetVertexColor (i, col);
}


void csSprite3D::AddColor (const csColor& col)
{
  for (int i=0 ; i < tpl->GetNumTexels () ; i++)
    AddVertexColor (i, col);
}


void csSprite3D::SetVertexColor (int i, const csColor& col)
{
  if (!vertex_colors)
  {
    vertex_colors = new csColor [tpl->GetNumTexels ()];
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
    vertex_colors = new csColor [tpl->GetNumTexels ()];
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
  //delete [] vertex_colors;
  //vertex_colors = NULL;
}

void csSprite3D::FixVertexColors ()
{
  if (vertex_colors)
    for (int i = 0 ; i < tpl->GetNumTexels (); i++)
      vertex_colors [i].Clamp (2., 2., 2.);
}

csTriangleMesh csSprite3D::mesh;
float csSprite3D::global_lod_level = DEFAULT_LOD;

// Set the default lighting quality.
int csSprite3D::global_lighting_quality = DEFAULT_LIGHTING;

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





int csSprite3D::GetNumVertsToLight ()
{
  if(IsLodEnabled())
  {
    if(num_verts_for_lod == -1)
      return tpl->GetNumTexels();
    else
      return num_verts_for_lod;
  }
  else
    return tpl->GetNumTexels();
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

void csSprite3D::UpdateInPolygonTrees ()
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
  csVector& sects = movable.GetSectors ();
  for (i = 0 ; i < sects.Length () ; i++)
  {
    tree = ((csSector*)sects[i])->GetStaticTree ();
    if (tree) break;
  }
  if (!tree) return;

  csBox3 b;
  GetObjectBoundingBox (b);

  // This transform should be part of the sprite class and not just calculated
  // every time we need it. @@@!!!
  csTransform trans = movable.GetTransform ().GetInverse ();

  bbox.Update (b, trans, this);

  // Here we need to insert in trees where this sprite lives.
  for (i = 0 ; i < sects.Length () ; i++)
  {
    tree = ((csSector*)sects[i])->GetStaticTree ();
    if (tree)
    {
      // Temporarily increase reference to prevent free.
      bbox.GetBaseStub ()->IncRef ();
      tree->AddObject (&bbox);
      bbox.GetBaseStub ()->DecRef ();
    }
  }
}

void csSprite3D::GetObjectBoundingBox (csBox3& b)
{
  if (skeleton_state)
  {
    skeleton_state->ComputeBoundingBox (csTransform (), b);
  }
  else
  {
    csFrame* cframe = cur_action->GetCsFrame (cur_frame);
    cframe->GetBoundingBox (b);
  }
}

void csSprite3D::GetWorldBoundingBox (csBox3& b)
{
  csBox3 ob;
  GetObjectBoundingBox (ob);
  csTransform& trans = GetMovable ().GetTransform ();
  b.StartBoundingBox (trans * ob.GetCorner (0));
  b.AddBoundingVertexSmart (trans * ob.GetCorner (1));
  b.AddBoundingVertexSmart (trans * ob.GetCorner (2));
  b.AddBoundingVertexSmart (trans * ob.GetCorner (3));
  b.AddBoundingVertexSmart (trans * ob.GetCorner (4));
  b.AddBoundingVertexSmart (trans * ob.GetCorner (5));
  b.AddBoundingVertexSmart (trans * ob.GetCorner (6));
  b.AddBoundingVertexSmart (trans * ob.GetCorner (7));
}

csVector3 csSprite3D::GetRadius ()
{
  csBox3 b;
  GetObjectBoundingBox (b);
  return (b.Max () - b.Min ()) * .5f;
}

void csSprite3D::GetCameraBoundingBox (const csCamera& camtrans, csBox3& cbox)
{
  csTranCookie cur_cookie = csEngine::current_engine->tr_manager.GetCookie ();
  if (camera_cookie == cur_cookie)
  {
    cbox = camera_bbox;
    return;
  }
  camera_cookie = cur_cookie;

  csTransform trans = camtrans * movable.GetTransform ().GetInverse ();
  if (skeleton_state)
  {
    skeleton_state->ComputeBoundingBox (trans, camera_bbox);
  }
  else
  {
    csFrame* cframe = cur_action->GetCsFrame (cur_frame);
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

float csSprite3D::GetScreenBoundingBox (const csCamera& camtrans, csBox2& boundingBox, csBox3 &bbox3)
{
  csVector2 oneCorner;

  // @@@ Note. The bounding box created by this function greatly
  // exagerates the real bounding box. However, this function
  // needs to be fast. I'm not sure how to do this more accuratelly.

  GetCameraBoundingBox (camtrans, bbox3);

  // if the entire bounding box is behind the camera, we're done
  if ((bbox3.MinZ () < 0) && (bbox3.MaxZ () < 0))
  {
    return -1;
  }

  // Transform from camera to screen space.
  if (bbox3.MinZ () <= 0)
  {
    // Sprite is very close to camera.
    // Just return a maximum bounding box.
    boundingBox.Set (-10000, -10000, 10000, 10000);
  }
  else
  {
    camtrans.Perspective (bbox3.Max (), oneCorner);
    boundingBox.StartBoundingBox (oneCorner);
    csVector3 v (bbox3.MinX (), bbox3.MinY (), bbox3.MaxZ ());
    camtrans.Perspective (v, oneCorner);
    boundingBox.AddBoundingVertexSmart (oneCorner);
    camtrans.Perspective (bbox3.Min (), oneCorner);
    boundingBox.AddBoundingVertexSmart (oneCorner);
    v.Set (bbox3.MaxX (), bbox3.MaxY (), bbox3.MinZ ());
    camtrans.Perspective (v, oneCorner);
    boundingBox.AddBoundingVertexSmart (oneCorner);
  }

  return bbox3.MaxZ ();
}

// New version of sprite drawing routine using DrawTriangleMesh.
void csSprite3D::Draw (csRenderView& rview)
{
  int i;
  if (draw_callback) draw_callback (this, &rview);

  if (!tpl->cstxt)
  {
    CsPrintf (MSG_FATAL_ERROR, "Error! Trying to draw a sprite with no material!\n");
    fatal_exit (0, false);
  }

  // Test visibility of entire sprite by clipping bounding box against clipper.
  // There are three possibilities:
  //	1. box is not visible -> sprite is not visible.
  //	2. box is entirely visible -> sprite is visible and need not be clipped.
  //	3. box is partially visible -> sprite is visible and needs to be clipped
  //	   if rview has do_clip_plane set to true.
  csBox2 bbox;
  csBox3 bbox3;
  
  if (GetScreenBoundingBox (rview, bbox, bbox3) < 0) return;	// Not visible.
  bool do_clip;
  if (rview.ClipBBox (bbox, bbox3, do_clip) == false) return;
 
  UpdateWorkTables (tpl->GetNumTexels());
  
// Moving the lighting to below the lod.
//  UpdateDeferedLighting (movable.GetPosition ());

  csFrame * cframe = cur_action->GetCsFrame (cur_frame);

  // Get next frame for animation tweening.
  csFrame * next_frame;
  if (cur_frame + 1 < cur_action->GetNumFrames())
    next_frame = cur_action->GetCsFrame (cur_frame + 1);
  else
    next_frame = cur_action->GetCsFrame (0);

  // First create the transformation from object to camera space directly:
  //   W = Mow * O - Vow;
  //   C = Mwc * (W - Vwc)
  // ->
  //   C = Mwc * (Mow * O - Vow - Vwc)
  //   C = Mwc * Mow * O - Mwc * (Vow + Vwc)
  csReversibleTransform tr_o2c = rview * movable.GetTransform ().GetInverse ();
  rview.g3d->SetObjectToCamera (&tr_o2c);
  rview.g3d->SetClipper (rview.view->GetClipPoly (), rview.view->GetNumVertices ());
  // @@@ This should only be done when aspect changes...
  rview.g3d->SetPerspectiveAspect (rview.GetFOV ());

  rview.g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_USE);

  bool do_tween = false;
  if (!skeleton_state && tween_ratio) do_tween = true;

  int cf_idx = cframe->GetAnmIndex();

  csVector3* real_obj_verts;
  csVector3* real_tween_verts = NULL;

  real_obj_verts = tpl->GetVertices (cf_idx);
  if (do_tween)
  {
    int nf_idx = next_frame->GetAnmIndex();
    real_tween_verts = tpl->GetVertices (nf_idx);
  }

  // If we have a skeleton then we transform all vertices through
  // the skeleton. In that case we also include the camera transformation
  // so that the 3D renderer does not need to do it anymore.
  csVector3* verts;
  if (skeleton_state)
  {
    skeleton_state->Transform (tr_o2c, real_obj_verts, tr_verts.GetArray ());
    verts = tr_verts.GetArray ();
  }
  else
  {
    verts = real_obj_verts;
  }

  // Calculate the right LOD level for this sprite.

  // Select the appropriate mesh.
  csTriangleMesh* m;
  int* emerge_from = NULL;

  float fnum = 0.0f;

  // level of detail is GetLodLevel() squared because the LOD
  // decreases with distance squared.
  // GetLodLevel() is the distance at which you will see full detail
  float level_of_detail = GetLodLevel() * GetLodLevel();

  if (IsLodEnabled())
  {
    // reduce LOD based on distance from camera to center of sprite
    csBox3 obox;
    GetObjectBoundingBox (obox);
    csVector3 obj_center = (obox.Min () + obox.Max ()) / 2;
    csVector3 wor_center = movable.GetTransform ().This2Other (obj_center);
    csVector3 cam_origin = rview.GetOrigin ();
    float wor_sq_dist = csSquaredDist::PointPoint (cam_origin, wor_center);
    level_of_detail /= MAX (wor_sq_dist, SMALL_EPSILON);

    // reduce LOD based on field-of-view
    float aspect = 2 * tan (rview.GetFOVAngle () * M_PI / 360);
    level_of_detail *= aspect;
  }

  if (IsLodEnabled() && level_of_detail < 1)
  {
    // We calculate the number of vertices to use for this LOD
    // level. The integer part will be the number of vertices.
    // The fractional part will determine how much to morph
    // between the new vertex and the previous last vertex.
    fnum = level_of_detail * (tpl->GetNumTexels() + 1);
    num_verts_for_lod = (int)fnum;
    fnum -= num_verts_for_lod;  // fnum is now the fractional part.

    GenerateSpriteLOD (num_verts_for_lod);
    emerge_from = tpl->GetEmergeFrom ();
    m = &mesh;
  }
  else
  {
    num_verts_for_lod = tpl->GetNumTexels ();
    m = tpl->GetTexelMesh ();
  }

  // @@@ moved down past lod to take advantage of the 
  UpdateDeferedLighting (movable.GetPosition ());
  
  
  
  csVector2* real_uv_verts;
  // Do vertex morphing if needed.
  // 
  // @@@ Don't understand this piece of code.
  //   Why is it checking if the level == 0, and negative?  neg is supposed 
  //    to be off.  zero is a valid on number...???
  if (level_of_detail <= 0 || level_of_detail >= 1)
  {
    real_uv_verts = tpl->GetTexels (cf_idx);
  }
  else
  {
    for (i = 0 ; i < num_verts_for_lod ; i++)
    {
      csVector2 uv;
      if (i < num_verts_for_lod-1)
      {
        uv = tpl->GetTexel (cf_idx, i);
      }
      else
      {
        // Morph between the last vertex and the one we morphed from.
        uv = (1-fnum) * tpl->GetTexel (cf_idx, emerge_from[i])
          + fnum * tpl->GetTexel (cf_idx, i);
      }

      uv_verts[i] = uv;
    }
    real_uv_verts = uv_verts.GetArray ();
  }

  // Setup the structure for DrawTriangleMesh.
  G3DTriangleMesh mesh;
  if (force_otherskin)
  {
    mesh.mat_handle[0] = cstxt->GetMaterialHandle ();
    cstxt->Visit ();
  }
  else
  {
    mesh.mat_handle[0] = tpl->cstxt->GetMaterialHandle ();
    tpl->cstxt->Visit ();
  }
  mesh.num_vertices = num_verts_for_lod;
  mesh.vertices[0] = verts;
  mesh.texels[0][0] = real_uv_verts;
  mesh.vertex_colors[0] = vertex_colors;
  if (do_tween)
  {
    mesh.morph_factor = tween_ratio;
    mesh.num_vertices_pool = 2;
    mesh.vertices[1] = real_tween_verts;
    mesh.texels[1][0] = real_uv_verts;
    mesh.vertex_colors[1] = vertex_colors;
  }
  else
  {
    mesh.morph_factor = 0;
    mesh.num_vertices_pool = 1;
  }
  mesh.num_materials = 1;

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

  rview.CalculateFogMesh (tr_o2c, mesh);

  if (rview.callback)
    rview.callback (&rview, CALLBACK_MESH, (void*)&mesh);
  else
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

  last_time = csEngine::System->GetTime ();

  MixMode = CS_FX_COPY;
}

bool csSprite3D::OldNextFrame (cs_time current_time, bool onestep, bool stoptoend)
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

  if (do_tweening)
    tween_ratio = (current_time - last_time) / (float)cur_action->GetFrameDelay (cur_frame);
  else
    tween_ratio = 0;

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
  defered_num_lights = 0;

  // Make sure the normals are computed
  tpl->ComputeNormals (cur_action->GetCsFrame (cur_frame));
  if (tween_ratio && GetLightingQuality() != CS_SPR_LIGHTING_FAST)
    tpl->ComputeNormals (cur_action->GetCsNextFrame (cur_frame));

  // Make sure that the color array is initialized.
  AddVertexColor (0, csColor (0, 0, 0));

  if (GetLightingQuality() == CS_SPR_LIGHTING_LQ ||
      GetLightingQuality() == CS_SPR_LIGHTING_HQ )
  {
    csSector * sect = movable.GetSector (0);
    if (sect)
    {
      int r, g, b;
      int num_texels = tpl->GetNumTexels();

      sect->GetAmbientColor (r, g, b);
      float rr = r / 128.0;
      float gg = g / 128.0;
      float bb = b / 128.0;

      // Reseting all of the vertex_colors to the ambient light.
      for (int i = 0 ; i < num_texels; i++)
        vertex_colors [i].Set (rr, gg, bb);
    }
  }
  
// @@@
// NOTE: lighting fast does not need to reset the vertex colors, it does this.
// 
//  else
//    ResetVertexColors();

  switch (GetLightingQuality())
  {
    case CS_SPR_LIGHTING_HQ:   UpdateLightingHQ   (lights, num_lights); break;
    case CS_SPR_LIGHTING_LQ:   UpdateLightingLQ   (lights, num_lights); break;
    case CS_SPR_LIGHTING_FAST: UpdateLightingFast (lights, num_lights); break;
    case CS_SPR_LIGHTING_RANDOM: UpdateLightingRandom (); break;
  }

  // @@@ TODO: Make FixVertexColors an option.
  // I would like lighting fast to not bother clamping the colors.
  //   Could we instead put some debug code in lighting fast to check if
  //    in the application programmers app that the colors don't go
  //    over 2.0?
  FixVertexColors ();  // Clamp all vertex colors to 2.0
}


void csSprite3D::UpdateLightingRandom ()
{
//  int num_texels = tpl->GetNumTexels();
  int num_texels = GetNumVertsToLight();
  float r,g,b;

  for (int i = 0; i < num_texels; i++)
  {
    // By seeding the rng with the current time each time, we get a slower
    //  cycling of colors.
    // rand_num->Initialize()
    r = rand_num->Get()*2;
    g = rand_num->Get()*2;
    b = rand_num->Get()*2;
    
    vertex_colors[i].Set(r,g,b);
  }
}




void csSprite3D::UpdateLightingFast (csLight** lights, int num_lights)
{
  int light_num, j;
  
  float cosinus;
  //int num_texels = tpl->GetNumTexels();
  int num_texels = GetNumVertsToLight();
  
  float light_bright_wor_dist;
  
  // convert frame number in current action to absolute frame number
  int tf_idx = cur_action->GetCsFrame (cur_frame)->GetAnmIndex();

  csBox3 obox;
  GetObjectBoundingBox (obox);
  csVector3 obj_center = (obox.Min () + obox.Max ()) / 2;
  csVector3 wor_center = movable.GetTransform ().This2Other (obj_center);
  csColor color;
  
  csColor light_color;
  float sq_light_radius;
  float cosinus_light;
  float light_color_r;
  float light_color_g;
  float light_color_b;

  // ambient colors.
  int r, g, b;
  
  csSector * sect = movable.GetSector (0);
  sect->GetAmbientColor (r, g, b);
  float amb_r = r / 128.0;
  float amb_g = g / 128.0;
  float amb_b = b / 128.0;


  for (light_num = 0 ; light_num < num_lights ; light_num++)
  {
    light_color = lights [light_num]->GetColor () * (256. / NORMAL_LIGHT_LEVEL);
    sq_light_radius = lights [light_num]->GetSquaredRadius ();

    // Compute light position in object coordinates
    csVector3 wor_light_pos = lights [light_num]->GetCenter ();
    float wor_sq_dist = csSquaredDist::PointPoint (wor_light_pos, wor_center);
    if (wor_sq_dist >= sq_light_radius) continue;

    csVector3 obj_light_pos = movable.GetTransform ().Other2This (wor_light_pos);
    float obj_sq_dist = csSquaredDist::PointPoint (obj_light_pos, obj_center);
    float inv_obj_dist = qisqrt (obj_sq_dist);
    float wor_dist = qsqrt (wor_sq_dist);

    csVector3 obj_light_dir = (obj_light_pos - obj_center);
    light_bright_wor_dist = lights[light_num]->GetBrightnessAtDistance (wor_dist);

    color = light_color;
  
    // This part of code will hardly ever be called.
    if (obj_sq_dist < SMALL_EPSILON)
    {
      for (j = 0 ; j < num_texels ; j++)
      {
        // TODO: need to add the ambient color here.
        AddVertexColor (j, color);
      }
      continue;
    }

    light_color_r = light_color.red;
    light_color_g = light_color.green;
    light_color_b = light_color.blue;

    // NOTE: Doing this to get rid of a divide in the loop.
    //obj_dist = 1 / obj_dist;

    // The first light should have the ambient color added.
    if(light_num == 0)
    {
      for (j = 0 ; j < num_texels ; j++)
      {
        // this obj_dist is not the obj_dist, see NOTE above.
        cosinus = (obj_light_dir * tpl->GetNormal (tf_idx, j)) * inv_obj_dist;
        cosinus_light = (cosinus * light_bright_wor_dist);
        vertex_colors[j].Set(light_color_r * cosinus_light + amb_r,
                             light_color_g * cosinus_light + amb_g,
                             light_color_b * cosinus_light + amb_b);
      }
    }
    else  // The next lights should have the light color added.
    {
      for (j = 0 ; j < num_texels ; j++)
      {
        // this obj_dist is not the obj_dist, see NOTE above.
        cosinus = (obj_light_dir * tpl->GetNormal (tf_idx, j)) * inv_obj_dist;
        cosinus_light = (cosinus * light_bright_wor_dist);
        vertex_colors[j].Add(light_color_r * cosinus_light,
                             light_color_g * cosinus_light,
                             light_color_b * cosinus_light);
      }
    }
  } // end of light loop.
}


void csSprite3D::UpdateLightingLQ (csLight** lights, int num_lights)
{
  int i, j;

  //int num_texels = tpl->GetNumTexels ();
  int num_texels = GetNumVertsToLight();

  float remainder = 1 - tween_ratio;

  // convert frame number in current action to absolute frame number
  int tf_idx = cur_action->GetCsFrame     (cur_frame)->GetAnmIndex ();
  int nf_idx = cur_action->GetCsNextFrame (cur_frame)->GetAnmIndex ();

  csBox3 obox;
  GetObjectBoundingBox (obox);
  csVector3 obj_center = (obox.Min () + obox.Max ()) / 2;
  csVector3 wor_center = movable.GetTransform ().This2Other (obj_center);
  csColor color;

  for (i = 0 ; i < num_lights ; i++)
  {
    // Compute light position in object coordinates
    csVector3 wor_light_pos = lights [i]->GetCenter ();
    float wor_sq_dist = csSquaredDist::PointPoint (wor_light_pos, wor_center);
    if (wor_sq_dist >= lights[i]->GetSquaredRadius ()) continue;

    csVector3 obj_light_pos = movable.GetTransform ().Other2This (wor_light_pos);
    float obj_sq_dist = csSquaredDist::PointPoint (obj_light_pos, obj_center);

    float in_obj_dist = 0.0f;

    // FIX: Check for divide by zero
    if (obj_sq_dist > SMALL_EPSILON)
      in_obj_dist = qisqrt (obj_sq_dist);

    csVector3 obj_light_dir = (obj_light_pos - obj_center);

    csColor light_color = lights[i]->GetColor () * (256. / NORMAL_LIGHT_LEVEL)
      * lights[i]->GetBrightnessAtDistance (qsqrt (wor_sq_dist));

    for (j = 0 ; j < num_texels ; j++)
    {
      csVector3 normal = tpl->GetNormal (tf_idx, j);
      if (tween_ratio)
      {
        normal = remainder * normal + tween_ratio * tpl->GetNormal (nf_idx, j);
	float norm = normal.Norm ();
	if (ABS (norm) > SMALL_EPSILON)
          normal /= norm;
      }

      float cosinus;
      if (obj_sq_dist < SMALL_EPSILON) cosinus = 1;
      else cosinus = obj_light_dir * normal;

      if (cosinus > 0)
      {
        color = light_color;
        if (obj_sq_dist >= SMALL_EPSILON) cosinus *= in_obj_dist;
        if (cosinus < 1) color *= cosinus;
        AddVertexColor (j, color);
      }
    }
  }
}

void csSprite3D::UpdateLightingHQ (csLight** lights, int num_lights)
{
  int i, j;

  // convert frame number in current action to absolute frame number
  int tf_idx = cur_action->GetCsFrame     (cur_frame)->GetAnmIndex ();
  int nf_idx = cur_action->GetCsNextFrame (cur_frame)->GetAnmIndex ();

  float remainder = 1 - tween_ratio;
//  int num_texels = tpl->GetNumTexels ();
  int num_texels = GetNumVertsToLight();

  // need vertices to calculate distance from light to each vertex
  csVector3* object_vertices;
  if (tween_ratio)
  {
    UpdateWorkTables (num_texels); // make room in obj_verts;

    for (i = 0 ; i < num_texels ; i++)
      obj_verts[i] = tween_ratio * tpl->GetVertex (tf_idx, i)
                   + remainder   * tpl->GetVertex (nf_idx, i);

    object_vertices = obj_verts.GetArray ();
  }
  else
    object_vertices = GetObjectVerts (cur_action->GetCsFrame (cur_frame));

  csColor color;

  for (i = 0 ; i < num_lights ; i++)
  {
    csColor light_color = lights [i]->GetColor () * (256. / NORMAL_LIGHT_LEVEL);
    float sq_light_radius = lights [i]->GetSquaredRadius ();

    // Compute light position in object coordinates
    csVector3 wor_light_pos = lights [i]->GetCenter ();
    csVector3 obj_light_pos = movable.GetTransform ().Other2This (wor_light_pos);

    for (j = 0 ; j < num_texels ; j++)
    {
      csVector3& obj_vertex = object_vertices[j];
      csVector3 wor_vertex = movable.GetTransform ().This2Other (obj_vertex);

      // @@@ We have the distance in object space. Can't we use
      // that to calculate the distance in world space as well?
      // These calculations aren't optimal. I have the feeling they
      // can be optimized somewhat.
      float obj_sq_dist = csSquaredDist::PointPoint (obj_light_pos, obj_vertex);
      float wor_sq_dist = csSquaredDist::PointPoint (wor_light_pos, wor_vertex);
      float obj_dist = qsqrt (obj_sq_dist);

      csVector3 normal = tpl->GetNormal (tf_idx, j);
      if (tween_ratio)
      {
        normal = remainder * normal + tween_ratio * tpl->GetNormal (nf_idx, j);
	float norm = normal.Norm ();
	if (ABS (norm) > SMALL_EPSILON)
          normal /= norm;
      }

      float cosinus;
      if (obj_sq_dist < SMALL_EPSILON) cosinus = 1;
      else cosinus = (obj_light_pos - obj_vertex) * normal;

      if ((cosinus > 0) && (wor_sq_dist < sq_light_radius))
      {
        color = light_color;
        if (obj_sq_dist >= SMALL_EPSILON) cosinus /= obj_dist;
        if (cosinus < 1) color *= cosinus * lights[i]->GetBrightnessAtDistance (obj_dist);
        AddVertexColor (j, color);
      }
    }
  }
}

bool csSprite3D::HitBeamObject (const csVector3& start, const csVector3& end,
	csVector3& isect, float* pr)
{
  // @@@ We might consider checking to a lower LOD version only.
  // This function is not very fast if the bounding box test succeeds.
  csBox3 b;
  GetObjectBoundingBox (b);
  csSegment3 seg (start, end);
  if (!csIntersect3::BoxSegment (b, seg, isect, pr))
    return false;
  csFrame* cframe = cur_action->GetCsFrame (cur_frame);
  csVector3* verts = GetObjectVerts (cframe);
  csTriangle* tris = tpl->GetTriangles ();
  int i;
  for (i = 0 ; i < tpl->GetNumTriangles () ; i++)
  {
    csTriangle& tr = tris[i];
    if (csIntersect3::IntersectTriangle (verts[tr.a], verts[tr.b],
    	verts[tr.c], seg, isect))
    {
      if (pr)
      {
        *pr = qsqrt (csSquaredDist::PointPoint (start, isect) /
		csSquaredDist::PointPoint (start, end));
      }
      return true;
    }
  }
  return false;
}

//--------------------------------------------------------------------------

csMeshedPolygon* csSprite3D::PolyMesh::GetPolygons ()
{
  if (!polygons)
  {
    csSpriteTemplate* tmpl = scfParent->GetTemplate ();
    csTriangle* triangles = tmpl->GetTriangles ();
    polygons = new csMeshedPolygon [GetNumPolygons ()];
    int i;
    for (i = 0 ; i < GetNumPolygons () ; i++)
    {
      polygons[i].num_vertices = 3;
      polygons[i].vertices = &triangles[i].a;
    }
  }
  return polygons;
}
