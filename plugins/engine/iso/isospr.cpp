/*
    Copyright (C) 2001 by W.C.A. Wijngaards

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
#include "isospr.h"
#include "ivideo/graph3d.h"
#include "csgeom/math2d.h"
#include "csgeom/polyclip.h"
#include "ivideo/material.h"
#include "ivideo/texture.h"
#include "ivideo/txtmgr.h"
#include "iengine/material.h"

SCF_IMPLEMENT_IBASE (csIsoSprite)
  SCF_IMPLEMENTS_INTERFACE (iIsoSprite)
SCF_IMPLEMENT_IBASE_END

csIsoSprite::csIsoSprite (iBase *iParent)
{
  SCF_CONSTRUCT_IBASE (iParent);
  position.Set(0,0,0);
  material = 0;
  g3dpolyfx.num = 0;
  g3dpolyfx.use_fog = false;
  g3dpolyfx.mat_handle = 0;
  g3dpolyfx.mixmode = CS_FX_COPY;
  g3dpolyfx.flat_color_r = 1;
  g3dpolyfx.flat_color_g = 1;
  g3dpolyfx.flat_color_b = 1;
  grid = 0;
}

csIsoSprite::~csIsoSprite ()
{
  SCF_DESTRUCT_IBASE();
}

int csIsoSprite::GetVertexCount() const
{
  return poly.GetVertexCount();
}

void csIsoSprite::AddVertex(const csVector3& coord, float u, float v)
{
  poly.AddVertex(coord);
  uv.AddVertex(u,v);
  colors.AddVertex(1.,1.,1.);
  static_colors.AddVertex(1.,1.,1.);
}

void csIsoSprite::SetPosition(const csVector3& newpos)
{
  /// manage movement of the sprite, oldpos, newpos
  csVector3 oldpos = position;
  position = newpos; // already set, so it can be overridden
  if(grid) grid->MoveSprite(this, oldpos, newpos);
}

void csIsoSprite::MovePosition(const csVector3& delta)
{
  SetPosition(position + delta);
}

#define INTERPOLATE1_S(var) \
  g3dpoly->var [i]= inpoly_##var [vt]+ \
    t * (inpoly_##var [vt2]- inpoly_##var [vt]);

#define INTERPOLATE1(var,component) \
  g3dpoly->var [i].component = inpoly_##var [vt].component + \
    t * (inpoly_##var [vt2].component - inpoly_##var [vt].component);

#define INTERPOLATE_S(var) \
{ \
  float v1 = inpoly_##var [edge_from [0]] + \
    t1 * (inpoly_##var [edge_to [0]] - inpoly_##var [edge_from [0]]); \
  float v2 = inpoly_##var [edge_from [1]] + \
    t2 * (inpoly_##var [edge_to [1]] - inpoly_##var [edge_from [1]]); \
  g3dpoly->var [i] = v1 + t * (v2 - v1); \
}

#define INTERPOLATE(var,component) \
{ \
  float v1 = inpoly_##var [edge_from [0]].component + \
    t1 * (inpoly_##var [edge_to [0]].component - inpoly_##var [edge_from [0]].component); \
  float v2 = inpoly_##var [edge_from [1]].component + \
    t2 * (inpoly_##var [edge_to [1]].component - inpoly_##var [edge_from [1]].component); \
  g3dpoly->var [i].component = v1 + t * (v2 - v1); \
}

static void PreparePolygonFX2 (G3DPolygonDPFX* g3dpoly,
  csVector2* clipped_verts, int num_vertices, csVertexStatus* clipped_vtstats,
  int orig_num_vertices, bool gouraud)
{
  // first we copy the first texture coordinates to a local buffer
  // to avoid that they are overwritten when interpolating.
  CS_ALLOC_STACK_ARRAY (csVector2, inpoly_vertices, orig_num_vertices);
  CS_ALLOC_STACK_ARRAY (csVector2, inpoly_texels, orig_num_vertices);
  CS_ALLOC_STACK_ARRAY (csColor, inpoly_colors, orig_num_vertices);
  CS_ALLOC_STACK_ARRAY (float, inpoly_z, orig_num_vertices);

  int i;
  memcpy (inpoly_vertices, g3dpoly->vertices,
  	orig_num_vertices*sizeof (csVector2));
  memcpy (inpoly_texels, g3dpoly->texels, orig_num_vertices*sizeof (csVector2));
  memcpy (inpoly_colors, g3dpoly->colors, orig_num_vertices*sizeof (csColor));
  memcpy (inpoly_z, g3dpoly->z, orig_num_vertices*sizeof (float));

  int vt, vt2;
  float t;
  for (i = 0; i < num_vertices; i++)
  {
    g3dpoly->vertices [i] = clipped_verts [i];
    switch (clipped_vtstats[i].Type)
    {
      case CS_VERTEX_ORIGINAL:
        vt = clipped_vtstats[i].Vertex;
	CS_ASSERT (vt >= 0 && vt < orig_num_vertices);
        g3dpoly->z [i] = inpoly_z [vt];
        g3dpoly->texels [i] = inpoly_texels [vt];
        if (gouraud)
          g3dpoly->colors [i] = inpoly_colors [vt];
        break;
      case CS_VERTEX_ONEDGE:
        vt = clipped_vtstats[i].Vertex;
	CS_ASSERT (vt >= 0 && vt < orig_num_vertices);
        vt2 = vt + 1; if (vt2 >= orig_num_vertices) vt2 = 0;
        t = clipped_vtstats [i].Pos;
        INTERPOLATE1_S (z);
        INTERPOLATE1 (texels,x);
        INTERPOLATE1 (texels,y);
        if (gouraud)
        {
          INTERPOLATE1 (colors,red);
          INTERPOLATE1 (colors,green);
          INTERPOLATE1 (colors,blue);
        }
        break;
      case CS_VERTEX_INSIDE:
        float x = clipped_verts [i].x;
        float y = clipped_verts [i].y;
        int edge_from [2], edge_to [2];
        int edge = 0;
        int j, j1;
        j1 = orig_num_vertices - 1;
        for (j = 0; j < orig_num_vertices; j++)
        {
          if ((y >= inpoly_vertices [j].y && y <= inpoly_vertices [j1].y) ||
              (y <= inpoly_vertices [j].y && y >= inpoly_vertices [j1].y))
          {
	    CS_ASSERT (edge >= 0 && edge < 2);
            edge_from [edge] = j;
            edge_to [edge] = j1;
            edge++;
            if (edge >= 2) break;
          }
          j1 = j;
        }
        if (edge == 1)
        {
          // Safety if we only found one edge.
          edge_from [1] = edge_from [0];
          edge_to [1] = edge_to [0];
        }
        csVector2& A = inpoly_vertices [edge_from [0]];
        csVector2& B = inpoly_vertices [edge_to [0]];
        csVector2& C = inpoly_vertices [edge_from [1]];
        csVector2& D = inpoly_vertices [edge_to [1]];
        float t1 = (y - A.y) / (B.y - A.y);
        float t2 = (y - C.y) / (D.y - C.y);
        float x1 = A.x + t1 * (B.x - A.x);
        float x2 = C.x + t2 * (D.x - C.x);
        t = (x - x1) / (x2 - x1);
        INTERPOLATE_S (z);
        INTERPOLATE (texels, x);
        INTERPOLATE (texels, y);
        if (gouraud)
        {
          INTERPOLATE (colors, red);
          INTERPOLATE (colors, green);
          INTERPOLATE (colors, blue);
        }
        break;
    }
  }
}

#undef INTERPOLATE
#undef INTERPOLATE1


void csIsoSprite::Draw(iIsoRenderView *rview)
{
  //printf("isosprite::Draw()\n");
  if (!material)
  {
    printf ("INTERNAL ERROR: IsoSpr used without valid material handle!\n");
    return;// false;
  }

  iGraphics3D* g3d = rview->GetG3D ();
  iIsoView* view = rview->GetView ();

  // Prepare for rendering.
  csZBufMode zbufmode;
  if((g3dpolyfx.mixmode & CS_FX_MASK_MIXMODE) == CS_FX_COPY)
    zbufmode = CS_ZBUF_USE;
  else
    zbufmode = CS_ZBUF_TEST;
  g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, zbufmode);
  //g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_NONE);

  //material->Visit ();

  g3dpolyfx.num = poly.GetVertexCount ();
  g3dpolyfx.mat_handle = material->GetMaterialHandle();
  /// guesstimate of fov (angle) of view. 1/fov.
  g3d->SetPerspectiveAspect (180.);
  g3dpolyfx.mat_handle->GetTexture ()->GetMeanColor (g3dpolyfx.flat_color_r,
    g3dpolyfx.flat_color_g, g3dpolyfx.flat_color_b);

  CS_ALLOC_STACK_ARRAY (csVector2, poly2d, g3dpolyfx.num);

  csVector2 clipped_poly2d[MAX_OUTPUT_VERTICES];
  csVertexStatus clipped_vtstats[MAX_OUTPUT_VERTICES];

  csVector3 screenpos;
  int i;
  float zlowerbound = rview->GetMinZ();

  //view->W2S(position, screenpos);
  //printf("position %g,%g,%g to %g,%g,%g\n",
    //position.x, position.y, position.z, screenpos.x, screenpos.y, screenpos.z);
  //g3d->DrawLine(screenpos, screenpos+csVector3(5,5,0), g3dpolyfx.inv_aspect,
    //g2d->FindRGB(255,255,255));

  for (i = 0; i < g3dpolyfx.num; i++)
  {
    view->W2S(position + poly[i], screenpos);
    //if(screenpos.z == 0.0) g3dpolyfx.vertices [i].z = 0.5; else
    //if(screenpos.z < 0.0)
         //g3dpolyfx.vertices [i].z = 1.0-.5/(-screenpos.z+1.);
    //else g3dpolyfx.vertices [i].z = .5/(screenpos.z+1.);
    g3dpolyfx.z [i] = 1./(screenpos.z-zlowerbound);
    g3dpolyfx.vertices [i].x = poly2d [i].x = screenpos.x;
    g3dpolyfx.vertices [i].y = poly2d [i].y = screenpos.y;
    g3dpolyfx.colors [i].red = colors [i].x;
    g3dpolyfx.colors [i].green = colors [i].y;
    g3dpolyfx.colors [i].blue = colors [i].z;
    g3dpolyfx.texels [i].x = uv [i].x;
    g3dpolyfx.texels [i].y = uv [i].y;
  }

  int num_clipped_verts;
  uint8 clip_result = rview->GetClipper()->Clip (poly2d, g3dpolyfx.num,
    clipped_poly2d, num_clipped_verts, clipped_vtstats);
  if (clip_result == CS_CLIP_OUTSIDE) return;// false;
  g3dpolyfx.num = num_clipped_verts;

  if (clip_result != CS_CLIP_INSIDE)
    PreparePolygonFX2 (&g3dpolyfx, clipped_poly2d, num_clipped_verts,
        clipped_vtstats, poly.GetVertexCount(), true);

  csRef<iIsoMaterialWrapperIndex> wrapindex (SCF_QUERY_INTERFACE(
    material, iIsoMaterialWrapperIndex));
  if((rview->GetRenderPass()==CSISO_RENDERPASS_MAIN) && wrapindex)
  {
    /// delay drawing, put into buckets
    //printf("Drawing index %d \n", wrapindex->GetIndex());
    rview->AddPolyFX (wrapindex->GetIndex(), &g3dpolyfx,
    	g3dpolyfx.mixmode);
    return;
  }
  // for non-iso-engine created materials, we have to draw them now.
  //rview->CalculateFogPolygon (g3dpolyfx);
  g3dpolyfx.mixmode = g3dpolyfx.mixmode;
  extern void IsoDrawPolygonFX (iGraphics3D* g3d, G3DPolygonDPFX& poly,
  	csZBufMode zbufmode);
  IsoDrawPolygonFX (g3d, g3dpolyfx, zbufmode);

  return;// true;
}

void csIsoSprite::SetGrid(iIsoGrid *grid)
{
  if(csIsoSprite::grid != grid)
  {
    csIsoSprite::grid = grid;
    if (gridcall) gridcall->GridChange (this);
  }
}

void csIsoSprite::SetAllColors(const csColor& color)
{
  CS_ASSERT (poly.GetVertexCount () == colors.GetVertexCount ());
  CS_ASSERT (poly.GetVertexCount () == static_colors.GetVertexCount ());
  int i;
  for(i=0; i<poly.GetVertexCount(); i++)
  {
    colors[i].Set(color.red, color.green, color.blue);
  }
}

void csIsoSprite::AddToVertexColor(int i, const csColor& color)
{
  CS_ASSERT (poly.GetVertexCount () == colors.GetVertexCount ());
  CS_ASSERT (i >= 0 && i < colors.GetVertexCount ());
  colors[i].x += color.red;
  if(colors[i].x>1.0f) colors[i].x=1.0f;
  else if(colors[i].x < 0.0f) colors[i].x = 0.0f;
  colors[i].y += color.green;
  if(colors[i].y>1.0f) colors[i].y=1.0f;
  else if(colors[i].y < 0.0f) colors[i].y = 0.0f;
  colors[i].z += color.blue;
  if(colors[i].z>1.0f) colors[i].z=1.0f;
  else if(colors[i].z < 0.0f) colors[i].z = 0.0f;
}

void csIsoSprite::ResetAllColors()
{
  int i;
  for(i=0; i<poly.GetVertexCount(); i++)
  {
    colors[i] = static_colors[i];
  }
}

void csIsoSprite::SetAllStaticColors(const csColor& color)
{
  int i;
  for(i=0; i<poly.GetVertexCount(); i++)
  {
    static_colors[i].Set(color.red, color.green, color.blue);
  }
}

void csIsoSprite::AddToVertexStaticColor(int i, const csColor& color)
{
  CS_ASSERT (poly.GetVertexCount () == static_colors.GetVertexCount ());
  CS_ASSERT (i >= 0 && i < static_colors.GetVertexCount ());
  static_colors[i].x += color.red;
  if(static_colors[i].x>1.0f) static_colors[i].x=1.0f;
  else if(static_colors[i].x < 0.0f) static_colors[i].x = 0.0f;
  static_colors[i].y += color.green;
  if(static_colors[i].y>1.0f) static_colors[i].y=1.0f;
  else if(static_colors[i].y < 0.0f) static_colors[i].y = 0.0f;
  static_colors[i].z += color.blue;
  if(static_colors[i].z>1.0f) static_colors[i].z=1.0f;
  else if(static_colors[i].z < 0.0f) static_colors[i].z = 0.0f;
}


