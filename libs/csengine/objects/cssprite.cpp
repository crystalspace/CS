/*
    Copyright (C) 1998 by Jorrit Tyberghein

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
#include "csengine/sysitf.h"
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
#include "igraph3d.h"

//--------------------------------------------------------------------------

csFrame::csFrame (int num_vertices)
{
  if (num_vertices)
  {
    CHK (texels = new csVector2 [num_vertices]);
    CHK (vertices = new csVector3 [num_vertices]);
  }
  else
  {
    texels = NULL;
    vertices = NULL;
  }
  name = NULL;
  max_vertex = num_vertices;
  normals = NULL;
}

csFrame::~csFrame ()
{
  CHK (delete [] texels);
  CHK (delete [] vertices);
  CHK (delete [] normals);
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

void csFrame::AddVertex (int num)
{
  if (num<1) return;

  CHK (csVector2 * new_texels = new csVector2 [max_vertex + num]);
  CHK (csVector3 * new_vertices = new csVector3 [max_vertex + num]);
    
  memcpy (new_texels, texels, max_vertex*sizeof(csVector2));
  memcpy (new_vertices, vertices, max_vertex*sizeof(csVector3));
  CHK (delete [] texels);
  CHK (delete [] vertices);

  max_vertex += num;

  texels = new_texels;
  vertices = new_vertices;
}

void csFrame::RemapVertices (int* mapping, int num_vertices)
{
  int i;
  CHK (csVector3* new_vertices = new csVector3 [num_vertices]);
  CHK (csVector2* new_texels = new csVector2 [num_vertices]);
  for (i = 0 ; i < num_vertices ; i++)
  {
    new_vertices[mapping[i]] = vertices[i];
    new_texels[mapping[i]] = texels[i];
  }
  CHK (delete [] vertices);
  CHK (delete [] texels);
  vertices = new_vertices;
  texels = new_texels;
}

void csFrame::ComputeNormals (csTriangleMesh *mesh, csVector3* object_verts, int num_vertices)
{
  CHK (delete [] normals);
  CHK (normals = new csVector3 [num_vertices]);
  CHK (csTriangleVertices *tri_verts = new csTriangleVertices (mesh, object_verts, num_vertices));
  for (int i = 0; i < num_vertices; i++)
  {
    csTriangleVertex &vt = tri_verts->GetVertex (i);
    if (vt.num_con_vertices)
    {
      csVector3 &v = object_verts [i];
      csVector3 &n = normals [i];
      // for some strange reason we have to compute the normal reversed
      n = v - object_verts [vt.con_vertices [0]];
      for (int j = 1 ; j < vt.num_con_vertices ; j++)
        n += (v - object_verts [vt.con_vertices [j]]);
      float norm = n.Norm ();
      if (norm)
        n /= norm;
    }
  }
  CHK (delete tri_verts);
}

void csFrame::ComputeBoundingBox (int num_vertices)
{
  int i;
  box_min = box_max = vertices[0];
  for (i = 1 ; i < num_vertices ; i++)
  {
    if (vertices[i].x < box_min.x) box_min.x = vertices[i].x;
    else if (vertices[i].x > box_max.x) box_max.x = vertices[i].x;
    if (vertices[i].y < box_min.y) box_min.y = vertices[i].y;
    else if (vertices[i].y > box_max.y) box_max.y = vertices[i].y;
    if (vertices[i].z < box_min.z) box_min.z = vertices[i].z;
    else if (vertices[i].z > box_max.z) box_max.z = vertices[i].z;
  }
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

CSOBJTYPE_IMPL (csSpriteTemplate, csObject)

csSpriteTemplate::csSpriteTemplate ()
  : csObject (), frames (8, 8), actions (8, 8)
{
  cstxt = NULL;
  num_vertices = 0;
  CHK (base_mesh = new csTriangleMesh ());
  emerge_from = NULL;
  skeleton = NULL;
}

csSpriteTemplate::~csSpriteTemplate ()
{
  CHK (delete base_mesh);
  CHK (delete [] emerge_from);
  CHK (delete skeleton);
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
  CHK (csTriangleVertices* verts = new csTriangleVertices (GetBaseMesh (), GetFrame (0)->GetVertices (), num_vertices));
  CHK (delete [] emerge_from);
  CHK (emerge_from = new int [num_vertices]);
  CHK (int* translate = new int [num_vertices]);
  CHK (csTriangleMesh* new_mesh = new csTriangleMesh (*GetBaseMesh ()));
  csLOD::CalculateLOD (new_mesh, verts, translate, emerge_from);
  int i;
  for (i = 0 ; i < frames.Length () ; i++)
  {
    csFrame* fr = (csFrame*)frames[i];
    fr->RemapVertices (translate, num_vertices);
  }
  if (skeleton) skeleton->RemapVertices (translate);
  for (i = 0 ; i < GetBaseMesh ()->GetNumTriangles () ; i++)
  {
    csTriangle& tr = GetBaseMesh ()->GetTriangles ()[i];
    tr.a = translate[tr.a];
    tr.b = translate[tr.b];
    tr.c = translate[tr.c];
  }

  CHK (delete [] translate);
  CHK (delete verts);
  CHK (delete new_mesh);
}

void csSpriteTemplate::ComputeBoundingBox ()
{
  int i;
  for (i = 0 ; i < GetNumFrames () ; i++)
    GetFrame (i)->ComputeBoundingBox (num_vertices);
  if (skeleton)
    skeleton->ComputeBoundingBox (GetFrame (0));
}

csFrame* csSpriteTemplate::AddFrame ()
{
  CHK (csFrame* fr = new csFrame (num_vertices));
  frames.Push (fr);
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

void csSpriteTemplate::SetTexture (csTextureList* textures, char *texname)
{
  if (!textures)
  {
    CsPrintf (MSG_FATAL_ERROR, "There are no textures defined in this world file!\n");
    fatal_exit (0, true);
    return;
  }
  csTextureHandle* texture = textures->GetTextureMM (texname);
  if (texture == NULL)
  {
    CsPrintf (MSG_FATAL_ERROR, "Couldn't find texture named '%s'!\n", texname);
    fatal_exit (0, true);
    return;
  }
  cstxt = texture;
}

csSpriteAction* csSpriteTemplate::FindAction (const char *n)
{
  for (int i = GetNumActions () - 1; i >= 0; i--)
    if (strcmp (GetAction (i)->GetName (), n) == 0)
      return GetAction (i);

  return NULL;
}

//=============================================================================

CSOBJTYPE_IMPL (csSprite3D, csObject)

csSprite3D::csSprite3D () : csObject ()
{
  v_obj2world.x = 0;
  v_obj2world.y = 0;
  v_obj2world.z = 0;
  cur_frame = 0;
  tpl = NULL;
  force_otherskin = false;
  cur_action = NULL;
  vertex_colors = NULL;
  dynamiclights = NULL;
  skeleton_state = NULL;
  MixMode = CS_FX_COPY;
  defered_num_lights = 0;
  defered_lighting_flags = 0;
  draw_callback = NULL;
  is_visible = false;
}

csSprite3D::~csSprite3D ()
{
  while (dynamiclights) CHKB (delete dynamiclights);
  CHK (delete [] vertex_colors);
  CHK (delete skeleton_state);
  RemoveFromSectors ();
}


void csSprite3D::SetMove (float x, float y, float z)
{
  v_obj2world.x = -x;
  v_obj2world.y = -y;
  v_obj2world.z = -z;
}

void csSprite3D::SetTransform (const csMatrix3& matrix)
{
  m_obj2world = matrix;
  m_world2obj = m_obj2world.GetInverse ();
}

void csSprite3D::Move (float dx, float dy, float dz)
{
  v_obj2world.x += dx;
  v_obj2world.y += dy;
  v_obj2world.z += dz;
}

bool csSprite3D::MoveTo (const csVector3 &move_to)
{
  csVector3 old_place(v_obj2world.x,v_obj2world.y,v_obj2world.z);
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
/*  v_obj2world.x=move_to.x;//new_pos.x;
    v_obj2world.y=move_to.y;//new_pos.y;
    v_obj2world.z=move_to.z;//new_pos.z;*/
    v_obj2world=-new_pos;
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
}

void csSprite3D::SetTemplate (csSpriteTemplate* tmpl)
{
  tpl = tmpl;
  CHK (delete skeleton_state);
  skeleton_state = NULL;
  if (tmpl->GetSkeleton ()) skeleton_state = (csSkeletonState*)tmpl->GetSkeleton ()->CreateState ();
}

void csSprite3D::SetTexture (char * name, csTextureList* textures)
{
  force_otherskin = true;
  csTextureHandle* texture = textures->GetTextureMM (name);
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
    CHK (vertex_colors = new csColor [tpl->GetNumVertices ()]);
    int j;
    for (j = 0 ; j < tpl->GetNumVertices (); j++)
      vertex_colors[j].Set (0, 0, 0);
  }
  vertex_colors[i] = col;
}

void csSprite3D::AddVertexColor (int i, const csColor& col)
{
  if (!vertex_colors)
  {
    CHK (vertex_colors = new csColor [tpl->GetNumVertices ()]);
    int j;
    for (j = 0 ; j < tpl->GetNumVertices (); j++)
      vertex_colors[j].Set (0, 0, 0);
  }
  vertex_colors [i].red += col.red;
  vertex_colors [i].green += col.green;
  vertex_colors [i].blue += col.blue;
}

void csSprite3D::ResetVertexColors ()
{
  if (vertex_colors)
    for (int i = 0 ; i < tpl->GetNumVertices (); i++)
      vertex_colors [i].Set (0, 0, 0);
  //CHK (delete [] vertex_colors);
  //vertex_colors = NULL;
}

void csSprite3D::FixVertexColors ()
{
  if (vertex_colors)
    for (int i = 0 ; i < tpl->GetNumVertices (); i++)
      vertex_colors [i].Clamp (2., 2., 2.);
}

csTriangleMesh csSprite3D::mesh;
float csSprite3D::cfg_lod_detail = 1;

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
  csTriangleMesh* base_mesh = tpl->GetBaseMesh ();
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

// @@@ The below arrays are never cleaned up.
static int max_work_sprite_size = 0;
csVector3* csSprite3D::tr_verts = NULL;
float* csSprite3D::z_verts = NULL;
csVector2* csSprite3D::uv_verts = NULL;
csVector2* csSprite3D::persp = NULL;
bool* csSprite3D::visible = NULL;
int csSprite3D::max_light_worktable = 0;
csLight** csSprite3D::light_worktable = NULL;

void csSprite3D::UpdateWorkTables (int max_size)
{
  if (max_size > max_work_sprite_size)
  {
    CHK (delete [] tr_verts);
    CHK (delete [] z_verts);
    CHK (delete [] uv_verts);
    CHK (delete [] persp);
    CHK (delete [] visible);
    max_work_sprite_size = max_size;
    CHK (tr_verts = new csVector3 [max_work_sprite_size]);
    CHK (z_verts = new float [max_work_sprite_size]);
    CHK (uv_verts = new csVector2 [max_work_sprite_size]);
    CHK (persp = new csVector2 [max_work_sprite_size]);
    CHK (visible = new bool [max_work_sprite_size]);
  }
}

void csSprite3D::UpdateDeferedLighting ()
{
  if (defered_num_lights)
  {
    if (defered_num_lights > max_light_worktable)
    {
      CHK (delete [] light_worktable);
      CHK (light_worktable = new csLight* [defered_num_lights]);
      max_light_worktable = defered_num_lights;
    }
    csSector* sect = (csSector*)sectors[0];
    int num_lights = csWorld::current_world->GetNearbyLights (sect,
      GetW2TTranslation (), defered_lighting_flags, light_worktable, defered_num_lights);
    UpdateLighting (light_worktable, num_lights);
  }
}

void csSprite3D::AddBoundingBox (csBspContainer* container)
{
  csVector3 b_min, b_max;
  if (skeleton_state)
  {
    skeleton_state->ComputeBoundingBox (csTransform (), b_min, b_max);
  }
  else
  {
    csFrame* cframe = cur_action->GetFrame (cur_frame);
    cframe->GetBoundingBox (b_min, b_max);
  }

  // This transform should be part of the sprite class and not just calculated
  // every time we need it. @@@!!!
  csTransform trans = csTransform (m_obj2world, m_world2obj * v_obj2world);
  csBspPolygon* poly;

  // Add the eight corner points of the bounding box to the container.
  // Transform from object to world space here.
  csVector3Array& va = container->GetVertices ();
  int pt_xyz = va.AddVertex (trans.Other2This (csVector3 (b_min.x, b_min.y, b_min.z)));
  int pt_Xyz = va.AddVertex (trans.Other2This (csVector3 (b_max.x, b_min.y, b_min.z)));
  int pt_xYz = va.AddVertex (trans.Other2This (csVector3 (b_min.x, b_max.y, b_min.z)));
  int pt_XYz = va.AddVertex (trans.Other2This (csVector3 (b_max.x, b_max.y, b_min.z)));
  int pt_xyZ = va.AddVertex (trans.Other2This (csVector3 (b_min.x, b_min.y, b_max.z)));
  int pt_XyZ = va.AddVertex (trans.Other2This (csVector3 (b_max.x, b_min.y, b_max.z)));
  int pt_xYZ = va.AddVertex (trans.Other2This (csVector3 (b_min.x, b_max.y, b_max.z)));
  int pt_XYZ = va.AddVertex (trans.Other2This (csVector3 (b_max.x, b_max.y, b_max.z)));

  CHK (poly = new csBspPolygon ());
  container->AddPolygon (poly);
  poly->SetOriginator (this);
  poly->GetPolygon ().AddVertex (pt_xYz);
  poly->GetPolygon ().AddVertex (pt_XYz);
  poly->GetPolygon ().AddVertex (pt_Xyz);
  poly->GetPolygon ().AddVertex (pt_xyz);
  poly->SetPolyPlane (csPlane (0, 0, 1, -b_min.z));
  poly->Transform (trans);

  CHK (poly = new csBspPolygon ());
  container->AddPolygon (poly);
  poly->SetOriginator (this);
  poly->GetPolygon ().AddVertex (pt_XYz);
  poly->GetPolygon ().AddVertex (pt_XYZ);
  poly->GetPolygon ().AddVertex (pt_XyZ);
  poly->GetPolygon ().AddVertex (pt_Xyz);
  poly->SetPolyPlane (csPlane (-1, 0, 0, b_max.x));
  poly->Transform (trans);

  CHK (poly = new csBspPolygon ());
  container->AddPolygon (poly);
  poly->SetOriginator (this);
  poly->GetPolygon ().AddVertex (pt_XYZ);
  poly->GetPolygon ().AddVertex (pt_xYZ);
  poly->GetPolygon ().AddVertex (pt_xyZ);
  poly->GetPolygon ().AddVertex (pt_XyZ);
  poly->SetPolyPlane (csPlane (0, 0, -1, b_max.z));
  poly->Transform (trans);

  CHK (poly = new csBspPolygon ());
  container->AddPolygon (poly);
  poly->SetOriginator (this);
  poly->GetPolygon ().AddVertex (pt_xYZ);
  poly->GetPolygon ().AddVertex (pt_xYz);
  poly->GetPolygon ().AddVertex (pt_xyz);
  poly->GetPolygon ().AddVertex (pt_xyZ);
  poly->SetPolyPlane (csPlane (1, 0, 0, -b_min.x));
  poly->Transform (trans);

  CHK (poly = new csBspPolygon ());
  container->AddPolygon (poly);
  poly->SetOriginator (this);
  poly->GetPolygon ().AddVertex (pt_xYZ);
  poly->GetPolygon ().AddVertex (pt_XYZ);
  poly->GetPolygon ().AddVertex (pt_XYz);
  poly->GetPolygon ().AddVertex (pt_xYz);
  poly->SetPolyPlane (csPlane (0, -1, 0, b_max.y));
  poly->Transform (trans);

  CHK (poly = new csBspPolygon ());
  container->AddPolygon (poly);
  poly->SetOriginator (this);
  poly->GetPolygon ().AddVertex (pt_xyz);
  poly->GetPolygon ().AddVertex (pt_Xyz);
  poly->GetPolygon ().AddVertex (pt_XyZ);
  poly->GetPolygon ().AddVertex (pt_xyZ);
  poly->SetPolyPlane (csPlane (0, 1, 0, -b_min.y));
  poly->Transform (trans);
}

float csSprite3D::GetScreenBoundingBox (csCamera *camera, csBox &boundingBox)
{
  csFrame *   theFrame;
  csVector2   oneCorner;
  csVector3   bbox_min;
  csVector3   bbox_max;
  csVector3   cameraPointMin;
  csVector3   cameraPointMax;
  csTransform trans;

  theFrame = cur_action->GetFrame(cur_frame);
  theFrame->GetBoundingBox(bbox_min, bbox_max);

  trans = *camera * csTransform(m_obj2world, m_world2obj * v_obj2world);
  cameraPointMin = trans * bbox_min;
  cameraPointMax = trans * bbox_max;

  // if the entire bounding box is behind the camera, we're done
  if ((cameraPointMin.z < 0) && (cameraPointMax.z < 0))
    return -1;

  // Transform from camera to screen space.
  oneCorner.x = cameraPointMin.x / cameraPointMin.z * camera->aspect + camera->shift_x;
  oneCorner.y = cameraPointMin.y / cameraPointMin.z * camera->aspect + camera->shift_y;
  boundingBox.StartBoundingBox(oneCorner);
  oneCorner.x = cameraPointMax.x / cameraPointMax.z * camera->aspect + camera->shift_x;
  oneCorner.y = cameraPointMax.y / cameraPointMax.z * camera->aspect + camera->shift_y;
  boundingBox.AddBoundingVertexSmart(oneCorner);

  return cameraPointMin.z;
}

void csSprite3D::Draw (csRenderView& rview)
{
  if (draw_callback) draw_callback (this, &rview);

  if (!tpl->cstxt)
  {
    CsPrintf (MSG_FATAL_ERROR, "Error! Trying to draw a sprite with no texture!\n");
    fatal_exit (0, false);
  }

  UpdateWorkTables (tpl->num_vertices);
  UpdateDeferedLighting ();

  int i;
  csFrame * cframe = cur_action->GetFrame (cur_frame);

  // First create the transformation from object to camera space directly:
  //   W = Mow * O - Vow;
  //   C = Mwc * (W - Vwc)
  // ->
  //   C = Mwc * (Mow * O - Vow - Vwc)
  //   C = Mwc * Mow * O - Mwc * (Vow + Vwc)
  csTransform tr_o2c = rview * csTransform (m_obj2world, m_world2obj * v_obj2world);

  // Now we transform all vertices to camera space. There are two possibilities.
  // If we have a skeleton then we let the skeleton do the transformation.
  // Otherwise we just transform all vertices.
  if (skeleton_state)
    skeleton_state->Transform (tr_o2c, cframe, tr_verts);
  else
  {
    for (i = 0 ; i < tpl->num_vertices ; i++)
      tr_verts[i] = tr_o2c * cframe->GetVertex (i);
  }

  // Calculate the right LOD level for this sprite.
  // Select the appropriate mesh.
  csTriangleMesh* m;
  int* emerge_from = NULL;
  int num_verts;
  float fnum = 0.0f;
  if (cfg_lod_detail < 0 || cfg_lod_detail == 1)
  {
    m = tpl->GetBaseMesh ();
    num_verts = tpl->num_vertices;
  }
  else
  {
    m = &mesh;
    // We calculate the number of vertices to use for this LOD
    // level. The integer part will be the number of vertices.
    // The fractional part will determine how much to morph
    // between the new vertex and the previous last vertex.
    fnum = cfg_lod_detail*(float)(tpl->num_vertices+1);
    num_verts = (int)fnum;
    fnum -= num_verts;  // fnum is now the fractional part.
    GenerateSpriteLOD (num_verts);
    emerge_from = tpl->GetEmergeFrom ();
  }

  // Do vertex morphing if needed and then perspective correction.
  for (i = 0 ; i < num_verts ; i++)
  {
    csVector3 v;
    csVector2 uv;
    if (cfg_lod_detail < 0 || cfg_lod_detail == 1 || i < num_verts-1)
    {
      v = tr_verts[i];
      uv = cframe->GetTexel (i);
    }
    else
    {
      // Morph between the last vertex and the one we morphed from.
      v = (1-fnum)*tr_verts[emerge_from[i]] + fnum*tr_verts[i];
      uv = (1-fnum)*cframe->GetTexel (emerge_from[i]) + fnum*cframe->GetTexel (i);
    }

    z_verts[i] = v.z;
    uv_verts[i] = uv;
    if (v.z >= SMALL_Z)
    {
      float iz = rview.aspect/v.z;
      persp[i].x = v.x * iz + rview.shift_x;
      persp[i].y = v.y * iz + rview.shift_y;
      visible[i] = true;
    }
    else
      visible[i] = false;
  }

  // Clipped polygon (assume it cannot have more than 64 vertices)
  G3DPolygonDPFX poly;
  memset (&poly, 0, sizeof(poly));
  poly.inv_aspect = rview.inv_aspect;

  if (force_otherskin)
    poly.txt_handle = cstxt->GetTextureHandle ();
  else
    poly.txt_handle = tpl->cstxt->GetTextureHandle ();

  // Fill flat color if renderer decide to paint it flat-shaded
  poly.txt_handle->GetMeanColor (poly.flat_color_r,
    poly.flat_color_g, poly.flat_color_b);

  // The triangle in question
  csVector2 triangle [3];
  csVector2 clipped_triangle [10];	//@@@BAD HARCODED!
  rview.g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERTESTENABLE, true);
  rview.g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERFILLENABLE, true);

  if (!rview.callback)
    rview.g3d->StartPolygonFX (poly.txt_handle, MixMode
      | (vertex_colors ? CS_FX_GOURAUD : 0));

  // Get this field from the current view for conveniance.
  bool mirror = rview.IsMirrored ();

  // Draw all triangles.
  for (i = 0 ; i < m->GetNumTriangles () ; i++)
  {
    int a = m->GetTriangles ()[i].a;
    int b = m->GetTriangles ()[i].b;
    int c = m->GetTriangles ()[i].c;
    if (visible[a] && visible[b] && visible[c])
    {
      //-----
      // Do backface culling. Note that this depends on the
      // mirroring of the current view.
      //-----
      float area = csMath2::Area2 (persp [a].x, persp [a].y,
                          	   persp [b].x, persp [b].y,
                          	   persp [c].x, persp [c].y);
      if (mirror)
      {
        if (area <= 0) continue;
      }
      else
        if (area >= 0) continue;

      triangle [mirror ? 2 : 0] = persp[a];
      triangle [1] = persp[b];
      triangle [mirror ? 0 : 2] = persp[c];
      // Clip triangle. Note that the clipper doesn't care about the
      // orientation of the triangle vertices. It works just as well in
      // mirrored mode.
      int rescount;
      if (!rview.view->Clip (triangle, clipped_triangle, 3, rescount)) continue;

      poly.num = rescount;
      int trivert [3] = { a, b, c };
      int j, idx, dir;
      // If mirroring we store the vertices in the other direction.
      if (mirror) { idx = 2; dir = -1; }
      else { idx = 0; dir = 1; }
      for (j = 0; j < 3; j++)
      {
        poly.vertices [idx].z = 1 / z_verts[trivert [j]];
        poly.vertices [idx].u = uv_verts[trivert [j]].x;
        poly.vertices [idx].v = uv_verts[trivert [j]].y;
        if (vertex_colors)
        {
          poly.vertices [idx].r = vertex_colors[trivert[j]].red;
          poly.vertices [idx].g = vertex_colors[trivert[j]].green;
          poly.vertices [idx].b = vertex_colors[trivert[j]].blue;
        }
	idx += dir;
      }
      PreparePolygonFX (&poly, clipped_triangle, rescount, (csVector2 *)triangle,
      	vertex_colors != NULL);

      // Draw resulting polygon
      if (!rview.callback)
      {
	extern void CalculateFogPolygon (csRenderView* rview, G3DPolygonDPFX& poly);
	CalculateFogPolygon (&rview, poly);
        rview.g3d->DrawPolygonFX (poly);
      }
      else
        rview.callback (&rview, CALLBACK_POLYGONQ, (void*)&poly);
    }
  }

  if (!rview.callback)
    rview.g3d->FinishPolygonFX ();
}

void csSprite3D::InitSprite ()
{
  if (!tpl)
  {
    CsPrintf (MSG_FATAL_ERROR, "There is no defined template for this sprite!\n");
    fatal_exit (0, false);
  }

  if (!cur_action) { SetFrame (0); cur_action = tpl->GetFirstAction (); }

  time_t tm;
  csWorld::isys->GetTime (tm);
  last_time = tm;

  m_world2obj = m_obj2world.GetInverse ();

  MixMode = CS_FX_COPY;
}

bool csSprite3D::NextFrame (long current_time, bool onestep, bool stoptoend)
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

  return ret;
}

void csSprite3D::MoveToSector (csSector* s)
{
  RemoveFromSectors ();
  sectors.Push (s);
  s->sprites.Push (this);
}

void csSprite3D::RemoveFromSectors ()
{
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

csVector3* csSprite3D::GetObjectVerts (csFrame* fr)
{
  if (skeleton_state)
  {
    UpdateWorkTables (tpl->num_vertices);
    skeleton_state->Transform (csTransform (), fr, tr_verts);
    return tr_verts;
  }
  else
    return fr->GetVertices ();
}

void csSprite3D::DeferUpdateLighting (int flags, int num_lights)
{
  defered_num_lights = num_lights;
  defered_lighting_flags = flags;
}

void csSprite3D::UpdateLighting (csLight** lights, int num_lights)
{
  int i, j;

  defered_num_lights = 0;

  csFrame* this_frame = tpl->GetFrame (cur_frame);
  csVector3* object_vertices = GetObjectVerts (this_frame);

  if (!this_frame->HasNormals ())
    this_frame->ComputeNormals (tpl->GetBaseMesh (), object_vertices, tpl->GetNumVertices ());

  ResetVertexColors ();

  // this is so that sprite gets blackened if no light strikes it
  AddVertexColor (0, csColor (0, 0, 0));

  for (i = 0 ; i < num_lights ; i++)
  {
    csColor &light_color = lights [i]->GetColor ();
    float light_radius = lights [i]->GetRadius ();
    float inv_light_radius = (256. / NORMAL_LIGHT_LEVEL) / light_radius;
    float r2 = light_color.red   * inv_light_radius;
    float g2 = light_color.green * inv_light_radius;
    float b2 = light_color.blue  * inv_light_radius;

    // Compute light position in object coordinates
    csVector3 light_pos = m_world2obj * (lights [i]->GetCenter () + v_obj2world);

    for (j = 0 ; j < tpl->GetNumVertices () ; j++)
    {
      csVector3& vertex = object_vertices[j];
      csVector3 light_vec = light_pos - vertex;
      float dist = sqrt (csSquaredDist::PointPoint (light_pos, vertex));
      float cosinus;
      if (fabs (dist) < SMALL_EPSILON)
        cosinus = 1;
      else
        cosinus = (light_vec * this_frame->GetNormal (j)) / dist;

      if ((cosinus > 0) && (dist < light_radius))
      {
        csColor color;
        if (cosinus >= 1)
          color.Set (
            light_color.red   * (256. / NORMAL_LIGHT_LEVEL),
            light_color.green * (256. / NORMAL_LIGHT_LEVEL),
            light_color.blue  * (256. / NORMAL_LIGHT_LEVEL));
        else
        {
          color.red   = cosinus * r2 * (light_radius - dist);
          color.green = cosinus * g2 * (light_radius - dist);
          color.blue  = cosinus * b2 * (light_radius - dist);
        }
        AddVertexColor (j, color);
      }
    }
  }

  // Clamp all vertice colors to 2.0
  FixVertexColors ();
}

void csSprite3D::UnlinkDynamicLight (csLightHitsSprite* lp)
{
  if (lp->next_sprite) lp->next_sprite->prev_sprite = lp->prev_sprite;
  if (lp->prev_sprite) lp->prev_sprite->next_sprite = lp->next_sprite;
  else dynamiclights = lp->next_sprite;
  lp->prev_sprite = lp->next_sprite = NULL;
  lp->sprite = NULL;
}

void csSprite3D::AddDynamicLight (csLightHitsSprite* lp)
{
  lp->next_sprite = dynamiclights;
  lp->prev_sprite = NULL;
  if (dynamiclights) dynamiclights->prev_sprite = lp;
  dynamiclights = lp;
  lp->sprite = this;
}
