/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein
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

#define CS_SYSDEF_PROVIDE_ALLOCA
#include "cssysdef.h"
#include "qint.h"
#include "csengine/halo.h"
#include "csengine/engine.h"
#include "csgeom/polyclip.h"
#include "csutil/halogen.h"
#include "ivideo/texture.h"
#include "iengine/texture.h"
#include "ivideo/graph3d.h"
#include "ivideo/material.h"
#include "iengine/material.h"

// The speed at which halo brightens/vanishes in milliseconds per frame
#define HALO_FRAME_TIME        20
// The speed at which halo brightens/vanishes in 0..1 units
#define HALO_INTENSITY_STEP    0.05

//--------------------------------------------------------------+ csHalo +---//

SCF_IMPLEMENT_IBASE (csHalo)
  SCF_IMPLEMENTS_INTERFACE (iBaseHalo)
SCF_IMPLEMENT_IBASE_END

csHalo::csHalo (csHaloType iType)
{
  SCF_CONSTRUCT_IBASE (NULL);
  Intensity = 0;
  Type = iType;
}

csHalo::~csHalo ()
{
}

//---------------------------------------------------------+ csCrossHalo +---//

SCF_IMPLEMENT_IBASE_EXT (csCrossHalo)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iCrossHalo)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csCrossHalo::CrossHalo)
  SCF_IMPLEMENTS_INTERFACE (iCrossHalo)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csCrossHalo::csCrossHalo (float intensity_factor, float cross_factor)
  : csHalo (cshtCross)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiCrossHalo);
  IntensityFactor = intensity_factor;
  CrossFactor = cross_factor;
}

uint8 *csCrossHalo::Generate (int Size)
{
  return csGenerateHalo (Size, IntensityFactor, CrossFactor);
}

//----------------------------------------------------------+ csNovaHalo +---//

SCF_IMPLEMENT_IBASE_EXT (csNovaHalo)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iNovaHalo)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csNovaHalo::NovaHalo)
  SCF_IMPLEMENTS_INTERFACE (iNovaHalo)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csNovaHalo::csNovaHalo (int seed, int num_spokes, float roundness)
  : csHalo (cshtNova)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiNovaHalo);
  Seed = seed;
  NumSpokes = num_spokes;
  Roundness = roundness;
}

uint8 *csNovaHalo::Generate (int Size)
{
  return csGenerateNova (Size, Seed, NumSpokes, Roundness);
}

//---------------------------------------------------------+ csFlareHalo +---//

SCF_IMPLEMENT_IBASE_EXT (csFlareHalo)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iFlareHalo)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csFlareHalo::FlareHalo)
  SCF_IMPLEMENTS_INTERFACE (iFlareHalo)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csFlareHalo::csFlareHalo ()
  : csHalo (cshtFlare)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiFlareHalo);
  components = NULL;
  last = NULL;
}

csFlareHalo::~csFlareHalo()
{
  csFlareComponent *np=NULL, *p = components;
  while(p)
  {
    np = p->next;
    if(p->image) p->image->DecRef();
    delete p;
    p = np;
  }
}

void csFlareHalo::AddComponent(float pos, float w, float h, uint mode, 
  iMaterialWrapper *image)
{
  csFlareComponent *comp = new csFlareComponent;
  comp->next = NULL;
  if(components==NULL) components = comp;
  else last->next = comp;
  last = comp;

  comp->position = pos;
  comp->width = w;
  comp->height = h;
  comp->mixmode = mode;
  comp->image = image;
  if(comp->image) comp->image->IncRef();
}

uint8 *csFlareHalo::Generate (int /*Size*/)
{
  // Not implemented for flares. (consist of multiple images)
  return NULL;
}

//---------------------------------------------------------+ csLightHalo +---//

csLightHalo::csLightHalo (csLight *iLight, iHalo *iHandle)
{
  Handle = iHandle;
  (Light = iLight)->flags.SetBool (CS_LIGHT_ACTIVEHALO, true);
  LastTime = 0;
}

csLightHalo::~csLightHalo ()
{
  if (Handle)
    Handle->DecRef ();
  if (Light)
    Light->flags.SetBool (CS_LIGHT_ACTIVEHALO, false);
}


bool csLightHalo::IsVisible(csEngine const& Engine, csVector3& v)
{
  if (v.z > SMALL_Z)
  {
    float iz = Engine.current_camera->GetFOV () / v.z;
    v.x = v.x * iz + Engine.current_camera->GetShiftX ();
    v.y = Engine.frame_height - 1 -
      (v.y * iz + Engine.current_camera->GetShiftY ());

    if (Engine.top_clipper->IsInside (csVector2 (v.x, v.y)))
    {
      float zv = Engine.G3D->GetZBuffValue (QRound (v.x), QRound (v.y));
      return (v.z <= zv);
    }
  }
  return false;
}


bool csLightHalo::ComputeNewIntensity(csTicks ElapsedTime, float& hintensity,
  bool halo_vis)
{
  if (ElapsedTime > 1000)
    ElapsedTime = 1000;
  LastTime += ElapsedTime;
  while (LastTime >= HALO_FRAME_TIME)
  {
    LastTime -= HALO_FRAME_TIME;

    if (halo_vis)
    {
      if (hintensity < 1.0 - HALO_INTENSITY_STEP)
        hintensity += HALO_INTENSITY_STEP;
      else
      {
        hintensity = 1.0;
        LastTime = 0;
      }
    }
    else
    {
      hintensity -= HALO_INTENSITY_STEP;

      // this halo is completely invisible. kill it.
      if (hintensity <= 0)
        return false;
    }
  }
  return true;
}



bool csLightHalo::Process (csTicks ElapsedTime, const csEngine &Engine)
{
  // Whenever the center of halo (the light) is directly visible
  bool halo_vis = false;
  // Whenever at least a piece of halo is visible
  bool draw_halo = false;
  // top-left coordinates of halo rectangle
  float xtl = 0, ytl = 0;

  // Project the halo.
  csVector3 v = Engine.current_camera->GetTransform ().Other2This (Light->GetCenter ());
  // The clipped halo polygon
  csVector2 HaloClip [32];
  // Number of vertices in HaloClip array
  int HaloVCount = 32;

  halo_vis = IsVisible(Engine, v);

  //if (halo_vis) // originally here, but this removes any fading out
                  // of the halo. I commented it out, Wouter 24/2/2001.
  {
    // Create a rectangle containing the halo and clip it against screen
    int hw = Handle->GetWidth ();
    int hh = Handle->GetHeight ();
    float hw2 = float (hw) / 2.0;
    float hh2 = float (hh) / 2.0;
    csVector2 HaloPoly [4] =
    {
      csVector2 (v.x - hw2, v.y - hh2),
      csVector2 (v.x - hw2, v.y + hh2),
      csVector2 (v.x + hw2, v.y + hh2),
      csVector2 (v.x + hw2, v.y - hh2)
    };
    // Clip the halo against clipper
    if (Engine.top_clipper->Clip (HaloPoly, 4, HaloClip, HaloVCount))
    {
      xtl = HaloPoly [0].x;
      ytl = HaloPoly [0].y;
      draw_halo = true;
    }
  }

  float hintensity = Light->GetHalo ()->GetIntensity ();
  if(!ComputeNewIntensity(ElapsedTime, hintensity, halo_vis))
    return false; // halo is invisible now, kill it
  Light->GetHalo ()->SetIntensity (hintensity);

  if (draw_halo)
    Handle->Draw (xtl, ytl, -1, -1, hintensity, HaloClip, HaloVCount);
  return true;
}

//----------------------------------------------------+ csLightFlareHalo +---//
csLightFlareHalo::csLightFlareHalo(csLight *light, csFlareHalo *halo, 
  int iHaloSize)
  : csLightHalo(light, 0)
{
  /// passes NULL as iHalo to csLightHalo.
  halosize = iHaloSize;
  flare = halo;
}

csLightFlareHalo::~csLightFlareHalo()
{
}


bool csLightFlareHalo::Process (csTicks elapsed_time, csEngine const& engine)
{
  // Whenever the center of halo (the light) is directly visible
  bool halo_vis = false;
  // Project the halo.
  csVector3 v = engine.current_camera->GetTransform ().Other2This (Light->GetCenter ());
  halo_vis = IsVisible(engine, v);

  /// compute new intensity
  float hintensity = Light->GetHalo ()->GetIntensity ();
  if(!ComputeNewIntensity(elapsed_time, hintensity, halo_vis))
    return false; // halo is invisible now, kill it
  Light->GetHalo ()->SetIntensity (hintensity);

  /// the perspective center of the view is the axis of the flare
  csVector2 center (engine.current_camera->GetShiftX (), 
    engine.current_camera->GetShiftY ());
  /// start point of the flare is the (projected) light position
  csVector2 start (v.x, engine.current_camera->GetShiftY()*2.-v.y);
  /// deltaposition, 1.0 positional change.
  csVector2 deltapos = center - start;

  /// take each part of the flare and process it (clip & draw)
  csFlareComponent *p = flare->GetComponents();
  while(p)
  {
    ProcessFlareComponent(engine, p, start, deltapos);
    p = p->next;
  }

  return true;
}


#define INTERPOLATE1(component) \
  g3dpoly->vertices [i].component = inpoly [vt].component + \
    t * (inpoly [vt2].component - inpoly [vt].component);

#define INTERPOLATE(component) \
{ \
  float v1 = inpoly [edge_from [0]].component + \
    t1 * (inpoly [edge_to [0]].component - inpoly [edge_from [0]].component); \
  float v2 = inpoly [edge_from [1]].component + \
    t2 * (inpoly [edge_to [1]].component - inpoly [edge_from [1]].component); \
  g3dpoly->vertices [i].component = v1 + t * (v2 - v1); \
}

static void PreparePolygonFX2 (G3DPolygonDPFX* g3dpoly,
  csVector2* clipped_verts, int num_vertices, csVertexStatus* clipped_vtstats,
  int orig_num_vertices, bool gouraud)
{
  // first we copy the first texture coordinates to a local buffer
  // to avoid that they are overwritten when interpolating.
  ALLOC_STACK_ARRAY (inpoly, G3DTexturedVertex, orig_num_vertices);
  int i;
  for (i = 0; i < orig_num_vertices; i++)
    inpoly[i] = g3dpoly->vertices[i];

  int vt, vt2;
  float t;
  for (i = 0; i < num_vertices; i++)
  {
    g3dpoly->vertices [i].x = clipped_verts [i].x;
    g3dpoly->vertices [i].y = clipped_verts [i].y;
    switch (clipped_vtstats[i].Type)
    {
      case CS_VERTEX_ORIGINAL:
        vt = clipped_vtstats[i].Vertex;
        g3dpoly->vertices [i].z = inpoly [vt].z;
        g3dpoly->vertices [i].u = inpoly [vt].u;
        g3dpoly->vertices [i].v = inpoly [vt].v;
        if (gouraud)
        {
          g3dpoly->vertices [i].r = inpoly [vt].r;
          g3dpoly->vertices [i].g = inpoly [vt].g;
          g3dpoly->vertices [i].b = inpoly [vt].b;
        }
        break;
      case CS_VERTEX_ONEDGE:
        vt = clipped_vtstats[i].Vertex;
        vt2 = vt + 1; if (vt2 >= orig_num_vertices) vt2 = 0;
        t = clipped_vtstats [i].Pos;
        INTERPOLATE1 (z);
        INTERPOLATE1 (u);
        INTERPOLATE1 (v);
        if (gouraud)
        {
          INTERPOLATE1 (r);
          INTERPOLATE1 (g);
          INTERPOLATE1 (b);
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
          if ((y >= inpoly [j].y && y <= inpoly [j1].y) ||
              (y <= inpoly [j].y && y >= inpoly [j1].y))
          {
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
        G3DTexturedVertex& A = inpoly [edge_from [0]];
        G3DTexturedVertex& B = inpoly [edge_to [0]];
        G3DTexturedVertex& C = inpoly [edge_from [1]];
        G3DTexturedVertex& D = inpoly [edge_to [1]];
        float t1 = (y - A.y) / (B.y - A.y);
        float t2 = (y - C.y) / (D.y - C.y);
        float x1 = A.x + t1 * (B.x - A.x);
        float x2 = C.x + t2 * (D.x - C.x);
        t = (x - x1) / (x2 - x1);
        INTERPOLATE (z);
        INTERPOLATE (u);
        INTERPOLATE (v);
        if (gouraud)
        {
          INTERPOLATE (r);
          INTERPOLATE (g);
          INTERPOLATE (b);
        }
        break;
    }
  }
}

#undef INTERPOLATE
#undef INTERPOLATE1

void csLightFlareHalo::ProcessFlareComponent(csEngine const& engine, 
  csFlareComponent *comp, csVector2 const& start, csVector2 const& deltapos)
{
  int i;
  /// compute size and position of this component
  float compw = float(halosize)*comp->width;
  float comph = float(halosize)*comp->height;
  csVector2 pos = start + comp->position * deltapos;
  
  /// drawing info for the polygon
  G3DPolygonDPFX dpfx;

  // Create a rectangle containing the halo and clip it against screen
  float hw2 = compw / 2.0;
  float hh2 = comph / 2.0;
  csVector2 HaloPoly [4] =
  {
    csVector2 (pos.x - hw2, pos.y - hh2),
    csVector2 (pos.x - hw2, pos.y + hh2),
    csVector2 (pos.x + hw2, pos.y + hh2),
    csVector2 (pos.x + hw2, pos.y - hh2)
  };
  csVector2 HaloUV [4] =
  {
    csVector2 (0.0, 0.0),
    csVector2 (0.0, 1.0),
    csVector2 (1.0, 1.0),
    csVector2 (1.0, 0.0)
  };
  // Clip the halo against clipper
  int num_clipped_verts;
  csVector2 clipped_poly2d[MAX_OUTPUT_VERTICES];
  csVertexStatus clipped_vtstats[MAX_OUTPUT_VERTICES];

  uint8 clip_result = engine.top_clipper->Clip (HaloPoly, 4, clipped_poly2d, 
    num_clipped_verts, clipped_vtstats);
  if (clip_result == CS_CLIP_OUTSIDE) return; // nothing to do

  // draw the halo
  uint mode = comp->mixmode;
  if(flare->GetIntensity() < 1.0)
    return; // many drivers do not support combinations of weird modes
    //mode |= CS_FX_SETALPHA(1.0 - flare->GetIntensity());

  // prepare for drawing
  dpfx.num = num_clipped_verts;
  dpfx.use_fog = false;
  if(!comp->image) 
  {
    printf("INTERNAL ERROR: flare used without material.\n");
    return;
  }
  dpfx.mat_handle = comp->image->GetMaterialHandle();
  if(!dpfx.mat_handle) 
  {
    printf("INTERNAL ERROR: flare used without valid material handle.\n");
    return;
  }
  comp->image->Visit();
  dpfx.mat_handle->GetTexture ()->GetMeanColor (dpfx.flat_color_r,
        dpfx.flat_color_g, dpfx.flat_color_b);
  engine.G3D->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_NONE);
  // copy info
  for(i=0; i<4; i++)
  {
    dpfx.vertices[i].z = SMALL_Z;
    dpfx.vertices[i].x = HaloPoly[i].x;
    dpfx.vertices[i].y = HaloPoly[i].y;
    dpfx.vertices[i].u = HaloUV[i].x;
    dpfx.vertices[i].v = HaloUV[i].y;
    dpfx.vertices[i].r = 1.0;
    dpfx.vertices[i].g = 1.0;
    dpfx.vertices[i].b = 1.0;
  }

  if(clip_result != CS_CLIP_INSIDE)
  {
    // need to interpolate u,v
    PreparePolygonFX2 (&dpfx, clipped_poly2d, num_clipped_verts,
	        clipped_vtstats, 4, false);
  }

  /// draw
  dpfx.mixmode = mode;
  engine.G3D->DrawPolygonFX (dpfx);
}
