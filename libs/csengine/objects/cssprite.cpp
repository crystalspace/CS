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
#include "csengine/polygon/pol2d.h"
#include "csengine/objects/cssprite.h"
#include "csengine/light/light.h"
#include "csengine/basic/triangle.h"
#include "csengine/camera.h"
#include "csengine/world.h"
#include "csengine/texture.h"
#include "csengine/sector.h"
#include "csengine/dumper.h"
#include "csgeom/polyclip.h"
#include "csgeom/fastsqrt.h"
#include "igraph3d.h"

csFrame::csFrame (int num_vertices)
{
  CHK (texels = new csVector2 [num_vertices]);
  CHK (vertices = new csVector3 [num_vertices]);
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

void csFrame::ComputeNormals (csTriangleMesh* mesh, int num_vertices)
{
  CHK (delete [] normals);
  CHK (normals = new csVector3 [num_vertices]);
  CHK (csTriangleVertices* tr_verts = new csTriangleVertices (mesh, vertices, num_vertices));
  int i, j;
  for (i = 0 ; i < num_vertices ; i++)
  {
    csTriangleVertex& vt = tr_verts->GetVertex (i);
    if (vt.num_con_vertices)
    {
      normals[i] = vertices[vt.con_vertices[0]] - vertices[i];
      for (j = 1 ; j < vt.num_con_vertices ; j++)
        normals[i] = normals[i] % (vertices[vt.con_vertices[j]] - vertices[i]);
      if (normals[i].Norm ()) normals[i] = normals[i].Unit ();
    }
  }
  CHK (delete tr_verts);
}


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

CSOBJTYPE_IMPL (csSpriteTemplate, csObject)

csSpriteTemplate::csSpriteTemplate ()
  : csObject (), frames (8, 8), actions (8, 8)
{
  cstxt = NULL;
  num_vertices = 0;
  CHK (base_mesh = new csTriangleMesh ());
  emerge_from = NULL;
}

csSpriteTemplate::~csSpriteTemplate ()
{
  CHK (delete base_mesh);
  CHK (delete [] emerge_from);
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
  tr_frame = NULL;
  persp = NULL;
  visible = NULL;
  tpl = NULL;
  force_otherskin = false;
  cur_action = NULL;
  vertex_colors = NULL;
}

csSprite3D::~csSprite3D ()
{
  CHK (delete tr_frame);
  CHK (delete [] persp);
  CHK (delete [] visible);
  CHK (delete [] vertex_colors);
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
/*
BOOL gePosition::MoveTo(csVector3 Pos)
{
  //A valid position needs to be set first!
  ASSERT(m_pSector);

  csOrthoTransform OldPos (csMatrix3(1,0,0,0,1,0,0,0,1), m_Pos);

  csVector3 NewPos     = Pos;
  csSector* pNewSector = m_pSector;

  bool mirror = false;
  pNewSector = m_pSector->FollowSegment (OldPos, NewPos, mirror);

  if (pNewSector &&
      ABS (NewPos.x-Pos.x) < SMALL_EPSILON &&
      ABS (NewPos.y-Pos.y) < SMALL_EPSILON &&
      ABS (NewPos.z-Pos.z) < SMALL_EPSILON)
  {
    m_pSector = pNewSector;
    m_Pos     = NewPos;
    return TRUE;
  }
  else
  {
    return FALSE; //Object would leave space...
  }
}
*/
bool csSprite3D::MoveTo(const csVector3 &move_to)
{
  csVector3 old_place(v_obj2world.x,v_obj2world.y,v_obj2world.z);
  csOrthoTransform OldPos (csMatrix3(1,0,0,0,1,0,0,0,1), old_place);

  csVector3 new_pos     = move_to;
  csSector* pNewSector;//  = currentSector;

  bool mirror = false;
  pNewSector = currentSector->FollowSegment (OldPos, new_pos, mirror);

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
  vertex_colors[i].red += col.red;
  vertex_colors[i].green += col.green;
  vertex_colors[i].blue += col.blue;
}

void csSprite3D::ResetVertexColors ()
{
  CHK (delete [] vertex_colors);
  vertex_colors = NULL;
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

void csSprite3D::Draw (csRenderView& rview)
{
  if (!tpl->cstxt)
  {
    CsPrintf (MSG_FATAL_ERROR, "Error! Trying to draw a sprite with no texture!\n");
    fatal_exit (0, false);
  }

  int i;

  // First create the transformation from object to camera space directly:
  //   W = Mow * O - Vow;
  //   C = Mwc * (W - Vwc)
  // ->
  //   C = Mwc * (Mow * O - Vow - Vwc)
  //   C = Mwc * Mow * O - Mwc * (Vow + Vwc)
  csMatrix3 m_o2c = rview.GetO2T ();
  m_o2c *= m_obj2world;
  csVector3 v_o2c = rview.Other2This (-v_obj2world);

  csFrame * cframe = cur_action->GetFrame (cur_frame);

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

  // Transform all vertices from object to camera space and do perspective
  // correction as well.
  for (i = 0 ; i < num_verts ; i++)
  {
    csVector3 v;
    csVector2 uv;
    if (cfg_lod_detail < 0 || cfg_lod_detail == 1 || i < num_verts-1)
    {
      v = cframe->GetVertex (i);
      uv = cframe->GetTexel (i);
    }
    else
    {
      // Morph between the last vertex and the one we morphed from.
      v = (1-fnum)*cframe->GetVertex (emerge_from[i]) + fnum*cframe->GetVertex (i);
      uv = (1-fnum)*cframe->GetTexel (emerge_from[i]) + fnum*cframe->GetTexel (i);
    }
    v = m_o2c * v + v_o2c;

    tr_frame->SetVertex (i, v.x, v.y, v.z);
    tr_frame->SetTexel (i, uv.x, uv.y);
    if (v.z >= SMALL_Z)
    {
      float iz = csCamera::aspect/v.z;
      persp[i].x = v.x * iz + csWorld::shift_x;
      persp[i].y = v.y * iz + csWorld::shift_y;
      visible[i] = true;
    }
    else
      visible[i] = false;
  }

  // Clipped polygon (assume it cannot have more than 64 vertices)
  G3DPolygonDPQ poly;
  memset (&poly, 0, sizeof(poly));

  // The triangle in question
  csVector2 triangle [3];
  rview.g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERTESTENABLE, true);
  rview.g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERFILLENABLE, true);

  if (force_otherskin)
    poly.txt_handle = cstxt->GetTextureHandle ();
  else
    poly.txt_handle = tpl->cstxt->GetTextureHandle ();

  rview.g3d->StartPolygonQuick (poly.txt_handle, vertex_colors != NULL);

  // Draw all triangles.
  for (i = 0 ; i < m->GetNumTriangles () ; i++)
  {
    int a = m->GetTriangles ()[i].a;
    int b = m->GetTriangles ()[i].b;
    int c = m->GetTriangles ()[i].c;
    if (visible[a] && visible[b] && visible[c])
    {
      //-----
      // Do backface culling.
      //-----
      if (csMath2::Area2 (persp [a].x, persp [a].y,
                          persp [b].x, persp [b].y,
                          persp [c].x, persp [c].y) >= 0)
        continue;

      triangle [0] = persp[a];
      triangle [1] = persp[b];
      triangle [2] = persp[c];
      // Clip triangle
      int rescount;
      csVector2 *cpoly = rview.view->Clip (triangle, 3, rescount);
      if (!cpoly) continue;

      poly.num = rescount;
      int trivert [3] = { a, b, c };
      int j;
      for (j = 0; j < 3; j++)
      {
        poly.vertices [j].z = 1 / tr_frame->GetVertex (trivert [j]).z;
        poly.vertices [j].u = tr_frame->GetTexel (trivert [j]).x;
        poly.vertices [j].v = tr_frame->GetTexel (trivert [j]).y;
        if (vertex_colors)
        {
          poly.vertices [j].r = vertex_colors[trivert[j]].red;
          poly.vertices [j].g = vertex_colors[trivert[j]].green;
          poly.vertices [j].b = vertex_colors[trivert[j]].blue;
        }
      }
      for (j = 0; j < rescount; j++)
      {
         poly.vertices [j].sx = cpoly [j].x;
         poly.vertices [j].sy = cpoly [j].y;
      }
      CHK (delete[] cpoly);

      PreparePolygonQuick (&poly, (csVector2 *)triangle, vertex_colors != NULL);
      // Draw resulting polygon
      rview.g3d->DrawPolygonQuick (poly);
    }
  }

  rview.g3d->FinishPolygonQuick ();
}

void csSprite3D::InitSprite ()
{
  if (!tpl)
  {
    CsPrintf (MSG_FATAL_ERROR, "There is no defined template for this sprite!\n");
    fatal_exit (0, false);
  }

  if (!cur_action) { SetFrame (0); cur_action = tpl->GetFirstAction (); }

  CHK (tr_frame = new csFrame (tpl->num_vertices));
  CHK (persp = new csVector2 [tpl->num_vertices]);
  CHK (visible = new bool [tpl->num_vertices]);

  time_t tm;
  csWorld::isys->GetTime (tm);
  last_time = tm;

  m_world2obj = m_obj2world.GetInverse ();
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
  currentSector=s;
}

void csSprite3D::RemoveFromSectors ()
{
  while (sectors.Length () > 0)
  {
    csSector* ss = (csSector*)sectors[0];
    sectors[0] = NULL;
    sectors.Pop ();
    int idx = ss->sprites.Find (this);
    ss->sprites[idx] = NULL;
    ss->sprites.Delete (idx);
  }
}

void csSprite3D::UpdateLighting (csLight** lights, int num_lights)
{
  int i, j;
  csColor color;
  float dist, cosinus;
  float r200d, g200d, b200d;
  csVector3 pos;

  // Choose one point of the sprite.
  // @@@ Note! we should try to use the center point here.
  csFrame* this_frame = tpl->GetFrame (cur_frame);
  if (!this_frame->HasNormals ()) this_frame->ComputeNormals (tpl->GetBaseMesh (), tpl->GetNumVertices ());

  ResetVertexColors ();
  for (i = 0 ; i < num_lights ; i++)
  {
    r200d = lights[i]->GetColor ().red * ((float)NORMAL_LIGHT_LEVEL/256.) / lights[i]->GetRadius ();
    g200d = lights[i]->GetColor ().green * ((float)NORMAL_LIGHT_LEVEL/256.) / lights[i]->GetRadius ();
    b200d = lights[i]->GetColor ().blue * ((float)NORMAL_LIGHT_LEVEL/256.) / lights[i]->GetRadius ();

    for (j = 0 ; j < tpl->GetNumVertices () ; j++)
    {
      // @@@ Transformation here is not efficient. First we should do the
      // inverse transformation on the light.
      pos = m_obj2world * this_frame->GetVertex (j) - v_obj2world;
      dist = FastSqrt (csSquaredDist::PointPoint (lights[i]->GetCenter (), pos));	//@@@NOT EFFICIENT!!!
      cosinus = (pos-lights[i]->GetCenter ())*(m_obj2world * this_frame->GetNormal (j));
      cosinus /= dist;
      if (cosinus < 0) cosinus = 0;
      else if (cosinus > 1) cosinus = 1;

      color.red = cosinus * r200d*(lights[i]->GetRadius () - dist);
      color.green = cosinus * g200d*(lights[i]->GetRadius () - dist);
      color.blue = cosinus * b200d*(lights[i]->GetRadius () - dist);
      AddVertexColor (j, color);
    }
  }
}

