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
#include "csengine/objects/cssprite.h"
#include "csengine/basic/triangle.h"
#include "csengine/camera.h"
#include "csengine/world.h"
#include "csengine/texture.h"
#include "csengine/sector.h"
#include "csgeom/polyclip.h"
#include "igraph3d.h"

csFrame::csFrame (int num_vertices)
{
  CHK (texels = new csVector2 [num_vertices]);
  CHK (vertices = new csVector3 [num_vertices]);
  name = NULL;
  max_vertex = num_vertices;
}

csFrame::~csFrame ()
{
  CHK (delete [] texels);
  CHK (delete [] vertices);
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

csSpriteAction* csSpriteTemplate::FindAction (char *n)
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

void csSprite3D::SetTransform (csMatrix3& matrix)
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
  int* emerge_from;
  int num_verts;
  float fnum;
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
    fnum -= num_verts;	// fnum is now the fractional part.
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
  G3DPolygon poly;
  memset (&poly, 0, sizeof(G3DPolygon));

  CHK (poly.pi_texcoords = new G3DPolygon::poly_texture_def [64]);
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
      if (!cpoly)
        continue;

      // Compute U & V in vertices of the polygon
      // First find the topmost triangle vertex
      int top;
      if (triangle [0].y < triangle [1].y)
        if (triangle [0].y < triangle [2].y) top = 0;
        else top = 2;
      else
        if (triangle [1].y < triangle [2].y) top = 1;
        else top = 2;

      poly.num = rescount;
      
      poly.pi_triangle = (csVector2 *)triangle;
      int trivert [3] = { a, b, c };
      int j;
      for (j = 0; j < 3; j++)
      {
        poly.pi_tritexcoords [j].z = 1 / tr_frame->GetVertex (trivert [j]).z;
        poly.pi_tritexcoords [j].u = tr_frame->GetTexel (trivert [j]).x;
        poly.pi_tritexcoords [j].v = tr_frame->GetTexel (trivert [j]).y;
	if (vertex_colors)
	{
	  poly.pi_tritexcoords[j].r = vertex_colors[trivert[j]].red;
	  poly.pi_tritexcoords[j].g = vertex_colors[trivert[j]].green;
	  poly.pi_tritexcoords[j].b = vertex_colors[trivert[j]].blue;
	}
      } /* endfor */
      for (j = 0; j < rescount; j++)
      {
        float x = poly.vertices [j].sx = cpoly [j].x;
        float y = poly.vertices [j].sy = cpoly [j].y;

        // Find the triangle left/right edges between which
        // the given x,y, point is located.
        int vtl, vtr, vbl, vbr;
        vtl = vtr = top;
        vbl = (vtl + 1) % 3;
        vbr = (vtl + 3 - 1) % 3;
        if (y > triangle [vbl].y)
        {
          vtl = vbl;
          vbl = (vbl + 1) % 3;
        }
	else if (y > triangle [vbr].y)
        {
          vtr = vbr;
          vbr = (vbr + 3 - 1) % 3;
        }
        else
        {
          // The last two vertices of the triangle have the same height.
	  // @@@ I think we should interpolate by 'x' here but this fix at
	  // least eliminates most errors.
          vtl = vbl;
          vbl = (vbl + 1) % 3;
        }

        // Now interpolate Z,U,V by Y
        float tL = triangle [vbl].y - triangle [vtl].y;
        if (tL)
          tL = (y - triangle [vtl].y) / tL;
        float tR = triangle [vbr].y - triangle [vtr].y;
        if (tR)
          tR = (y - triangle [vtr].y) / tR;
        float xL = triangle [vtl].x + tL * (triangle [vbl].x - triangle [vtl].x);
        float xR = triangle [vtr].x + tR * (triangle [vbr].x - triangle [vtr].x);
        float tX = xR - xL;
        if (tX)
          tX = (x - xL) / tX;

        #define INTERPOLATE(val,tl,bl,tr,br)	\
        {					\
          float vl,vr;				\
          if (tl != bl)				\
            vl = tl + (bl - tl) * tL;		\
          else					\
            vl = tl;				\
          if (tr != br)				\
            vr = tr + (br - tr) * tR;		\
          else					\
            vr = tr;				\
          val = vl + (vr - vl) * tX;		\
        }

        // Calculate Z
        INTERPOLATE (poly.pi_texcoords [j].z,
          poly.pi_tritexcoords [vtl].z, poly.pi_tritexcoords [vbl].z,
          poly.pi_tritexcoords [vtr].z, poly.pi_tritexcoords [vbr].z);
        // Calculate U
        INTERPOLATE (poly.pi_texcoords [j].u,
          poly.pi_tritexcoords [vtl].u, poly.pi_tritexcoords [vbl].u,
          poly.pi_tritexcoords [vtr].u, poly.pi_tritexcoords [vbr].u);
        // Calculate V
        INTERPOLATE (poly.pi_texcoords [j].v,
          poly.pi_tritexcoords [vtl].v, poly.pi_tritexcoords [vbl].v,
          poly.pi_tritexcoords [vtr].v, poly.pi_tritexcoords [vbr].v);
	if (vertex_colors)
	{
          // Calculate R
          INTERPOLATE (poly.pi_texcoords [j].r,
            poly.pi_tritexcoords [vtl].r, poly.pi_tritexcoords [vbl].r,
            poly.pi_tritexcoords [vtr].r, poly.pi_tritexcoords [vbr].r);
          // Calculate G
          INTERPOLATE (poly.pi_texcoords [j].g,
            poly.pi_tritexcoords [vtl].g, poly.pi_tritexcoords [vbl].g,
            poly.pi_tritexcoords [vtr].g, poly.pi_tritexcoords [vbr].g);
          // Calculate B
          INTERPOLATE (poly.pi_texcoords [j].b,
            poly.pi_tritexcoords [vtl].b, poly.pi_tritexcoords [vbl].b,
            poly.pi_tritexcoords [vtr].b, poly.pi_tritexcoords [vbr].b);
	}
      } /* endfor */
      CHK (delete[] cpoly);
      // Draw resulting polygon
      rview.g3d->DrawPolygonQuick (poly, vertex_colors != NULL);
    }
  }

  rview.g3d->FinishPolygonQuick ();

  CHK (delete [] poly.pi_texcoords);
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

