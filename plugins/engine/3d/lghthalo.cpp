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
#include "cssysdef.h"
#include "csutil/sysfunc.h"
#include "csqint.h"
#include "plugins/engine/3d/halo.h"
#include "plugins/engine/3d/engine.h"
#include "csgeom/polyclip.h"
#include "plugins/engine/3d/halogen.h"
#include "ivideo/texture.h"
#include "iengine/texture.h"
#include "ivideo/material.h"
#include "iengine/material.h"
#include "ivideo/graph3d.h"



// The speed at which halo brightens/vanishes in milliseconds per frame
#define HALO_FRAME_TIME 20

// The speed at which halo brightens/vanishes in 0..1 units
#define HALO_INTENSITY_STEP 0.05f

//--------------------------------------------------------------+ csHalo +---//
SCF_IMPLEMENT_IBASE(csHalo)
  SCF_IMPLEMENTS_INTERFACE(iBaseHalo)
SCF_IMPLEMENT_IBASE_END

csHalo::csHalo (csHaloType iType)
{
  SCF_CONSTRUCT_IBASE (0);
  Intensity = 0;
  Type = iType;
}

csHalo::~csHalo ()
{
  SCF_DESTRUCT_IBASE ();
}

//---------------------------------------------------------+ csCrossHalo +---//
SCF_IMPLEMENT_IBASE_EXT(csCrossHalo)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iCrossHalo)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csCrossHalo::CrossHalo)
  SCF_IMPLEMENTS_INTERFACE(iCrossHalo)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csCrossHalo::csCrossHalo (
  float intensity_factor,
  float cross_factor) :
    csHalo(cshtCross)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiCrossHalo);
  IntensityFactor = intensity_factor;
  CrossFactor = cross_factor;
}

csCrossHalo::~csCrossHalo()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiCrossHalo);
}

uint8 *csCrossHalo::Generate (int Size)
{
  return csGenerateHalo (Size, IntensityFactor, CrossFactor);
}

//----------------------------------------------------------+ csNovaHalo +---//
SCF_IMPLEMENT_IBASE_EXT(csNovaHalo)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iNovaHalo)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csNovaHalo::NovaHalo)
  SCF_IMPLEMENTS_INTERFACE(iNovaHalo)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csNovaHalo::csNovaHalo (
  int seed,
  int num_spokes,
  float roundness) :
    csHalo(cshtNova)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiNovaHalo);
  Seed = seed;
  NumSpokes = num_spokes;
  Roundness = roundness;
}

csNovaHalo::~csNovaHalo()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiNovaHalo);
}

uint8 *csNovaHalo::Generate (int Size)
{
  return csGenerateNova (Size, Seed, NumSpokes, Roundness);
}

//---------------------------------------------------------+ csFlareHalo +---//
SCF_IMPLEMENT_IBASE_EXT(csFlareHalo)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iFlareHalo)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csFlareHalo::FlareHalo)
  SCF_IMPLEMENTS_INTERFACE(iFlareHalo)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csFlareHalo::csFlareHalo () :
  csHalo(cshtFlare)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiFlareHalo);
  components = 0;
  last = 0;
}

csFlareHalo::~csFlareHalo ()
{
  csFlareComponent *np = 0, *p = components;
  while (p)
  {
    np = p->next;
    if (p->image) p->image->DecRef ();
    delete p;
    p = np;
  }
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiFlareHalo);
}

void csFlareHalo::AddComponent (
  float pos,
  float w,
  float h,
  uint mode,
  iMaterialWrapper *image)
{
  csFlareComponent *comp = new csFlareComponent;
  comp->next = 0;
  if (components == 0)
    components = comp;
  else
    last->next = comp;
  last = comp;

  comp->position = pos;
  comp->width = w;
  comp->height = h;
  comp->mixmode = mode;
  comp->image = image;
  if (comp->image) comp->image->IncRef ();
}

uint8 *csFlareHalo::Generate (int

/*Size*/ )
{
  // Not implemented for flares. (consist of multiple images)
  return 0;
}

//---------------------------------------------------------+ csLightHalo +---//
csLightHalo::csLightHalo (csLight *ilight, iHalo *ihandle)
{
  Handle = ihandle;
  (Light = ilight)->flags.SetBool (CS_LIGHT_ACTIVEHALO, true);
  LastTime = 0;
}

csLightHalo::~csLightHalo ()
{
  if (Handle) Handle->DecRef ();
  if (Light) Light->flags.SetBool (CS_LIGHT_ACTIVEHALO, false);
}

bool csLightHalo::IsVisible (iCamera* camera, csEngine* Engine, csVector3 &v)
{
  if (v.z > SMALL_Z)
  {
    float iz = camera->GetFOV () / v.z;
    v.x = v.x * iz + camera->GetShiftX ();
    v.y = Engine->frame_height - 1 - (v.y * iz + camera->GetShiftY ());

    if (Engine->top_clipper->GetClipper ()->IsInside (csVector2 (v.x, v.y)))
    {
      float zv = Engine->G3D->GetZBuffValue (csQround (v.x), csQround (v.y));
      return v.z <= zv;
    }
  }

  return false;
}

bool csLightHalo::ComputeNewIntensity (
  csTicks ElapsedTime,
  float &hintensity,
  bool halo_vis)
{
  if (ElapsedTime > 1000) ElapsedTime = 1000;
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
      if (hintensity <= 0) return false;
    }
  }

  return true;
}

bool csLightHalo::Process (csTicks ElapsedTime, iCamera* camera, 
			   csEngine* Engine)
{
  // Whenever the center of halo (the light) is directly visible
  bool halo_vis = false;

  // Whenever at least a piece of halo is visible
  bool draw_halo = false;

  // top-left coordinates of halo rectangle
  float xtl = 0, ytl = 0;

  // Project the halo.
  csVector3 v = camera->GetTransform ().Other2This (
      Light->GetCenter ());

  // The clipped halo polygon
  csVector2 HaloClip[32];

  // Number of vertices in HaloClip array
  int HaloVCount = 32;

  halo_vis = IsVisible (camera, Engine, v);

  // Create a rectangle containing the halo and clip it against screen
  float hw = Handle->GetWidth () * 0.5f;
  float hh = Handle->GetHeight () * 0.5f;
  
  csVector2 HaloPoly[4] =
  {
    csVector2 (v.x - hw, v.y - hh),
    csVector2 (v.x - hw, v.y + hh),
    csVector2 (v.x + hw, v.y + hh),
    csVector2 (v.x + hw, v.y - hh)
  };

  // Clip the halo against clipper
  if (Engine->top_clipper->GetClipper ()->Clip (
  	HaloPoly, 4, HaloClip, HaloVCount))
  {
    xtl = HaloPoly[0].x;
    ytl = HaloPoly[0].y;
    draw_halo = true;
  }

  float hintensity = Light->GetHalo ()->GetIntensity ();
  if (!ComputeNewIntensity (ElapsedTime, hintensity, halo_vis))
    return false; // halo is invisible now, kill it
  Light->GetHalo ()->SetIntensity (hintensity);

  if (draw_halo)
    Handle->Draw (xtl, ytl, -1, -1, hintensity, HaloClip, HaloVCount);
  return true;
}

//----------------------------------------------------+ csLightFlareHalo +---//
csLightFlareHalo::csLightFlareHalo (
  csLight *light,
  csFlareHalo *halo,
  int iHaloSize) :
    csLightHalo(light, 0)
{
  /// passes 0 as iHalo to csLightHalo.
  halosize = iHaloSize;
  flare = halo;
}

csLightFlareHalo::~csLightFlareHalo ()
{
}

bool csLightFlareHalo::Process (csTicks elapsed_time, iCamera* camera, 
				csEngine* engine)
{
  // Whenever the center of halo (the light) is directly visible
  bool halo_vis = false;

  // Project the halo.
  csVector3 v = camera->GetTransform ().Other2This (
      Light->GetCenter ());
  halo_vis = IsVisible (camera, engine, v);

  /// compute new intensity
  float hintensity = Light->GetHalo ()->GetIntensity ();
  if (!ComputeNewIntensity (elapsed_time, hintensity, halo_vis))
    return false; // halo is invisible now, kill it
  Light->GetHalo ()->SetIntensity (hintensity);

  /// the perspective center of the view is the axis of the flare
  csVector2 center (
              camera->GetShiftX (),
              camera->GetShiftY ());

  /// start point of the flare is the (projected) light position
  csVector2 start (v.x, camera->GetShiftY () * 2.0f - v.y);

  /// deltaposition, 1.0 positional change.
  csVector2 deltapos = center - start;

  /// take each part of the flare and process it (clip & draw)
  csFlareComponent *p = flare->GetComponents ();
  while (p)
  {
    ProcessFlareComponent (*engine, p, start, deltapos);
    p = p->next;
  }

  return true;
}

#define INTERPOLATE1_S(var) \
      g3dpoly->var[i] = inpoly_##var [vt] + \
    t * (inpoly_##var [vt2] - inpoly_##var [vt]);

#define INTERPOLATE1(var,component) \
      g3dpoly->var[i].component = inpoly_##var [vt].component + \
    t * (inpoly_##var [vt2].component - inpoly_##var [vt].component);

#define INTERPOLATE_S(var) \
  { \
    float v1 = inpoly_##var [edge_from[0]] + t1 * \
      (inpoly_##var [edge_to[0]]- inpoly_##var [edge_from[0]]); \
    float v2 = inpoly_##var [edge_from[1]]+ t2 * \
      (inpoly_##var [edge_to[1]]- inpoly_##var [edge_from[1]]); \
    g3dpoly->var[i] = v1 + t * (v2 - v1); \
  }

#define INTERPOLATE(var,component) \
  { \
    float v1 = inpoly_##var [edge_from[0]].component + t1 * \
      (inpoly_##var [edge_to[0]].component - inpoly_##var [edge_from[0]].component); \
    float v2 = inpoly_##var [edge_from[1]].component + t2 * \
      (inpoly_##var [edge_to[1]].component - inpoly_##var [edge_from[1]].component); \
    g3dpoly->var[i].component = v1 + t * (v2 - v1); \
  }

#ifdef CS_USE_OLD_RENDERER
static void PreparePolygonFX2 (
  G3DPolygonDPFX *g3dpoly,
  csVector2 *clipped_verts,
  int num_vertices,
  csVertexStatus *clipped_vtstats,
  int orig_num_vertices,
  bool gouraud)
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
    g3dpoly->vertices[i].x = clipped_verts[i].x;
    g3dpoly->vertices[i].y = clipped_verts[i].y;
    switch (clipped_vtstats[i].Type)
    {
      case CS_VERTEX_ORIGINAL:
        vt = clipped_vtstats[i].Vertex;
        g3dpoly->z[i] = inpoly_z[vt];
        g3dpoly->texels[i] = inpoly_texels[vt];
        if (gouraud)
          g3dpoly->colors[i] = inpoly_colors[vt];
        break;
      case CS_VERTEX_ONEDGE:
        vt = clipped_vtstats[i].Vertex;
        vt2 = vt + 1;
        if (vt2 >= orig_num_vertices) vt2 = 0;
        t = clipped_vtstats[i].Pos;
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
        float x = clipped_verts[i].x;

        float y = clipped_verts[i].y;
        int edge_from[2], edge_to[2];
        int edge = 0;
        int j, j1;
        j1 = orig_num_vertices - 1;
        for (j = 0; j < orig_num_vertices; j++)
        {
          if (
            (y >= inpoly_vertices[j].y && y <= inpoly_vertices[j1].y) ||
            (y <= inpoly_vertices[j].y && y >= inpoly_vertices[j1].y))
          {
            edge_from[edge] = j;
            edge_to[edge] = j1;
            edge++;
            if (edge >= 2) break;
          }

          j1 = j;
        }

        if (edge == 1)
        {
          // Safety if we only found one edge.
          edge_from[1] = edge_from[0];
          edge_to[1] = edge_to[0];
        }

        csVector2 &A = inpoly_vertices[edge_from[0]];
        csVector2 &B = inpoly_vertices[edge_to[0]];
        csVector2 &C = inpoly_vertices[edge_from[1]];
        csVector2 &D = inpoly_vertices[edge_to[1]];
        float t1 = (y - A.y) / (B.y - A.y);
        float t2 = (y - C.y) / (D.y - C.y);
        float x1 = A.x + t1 * (B.x - A.x);
        float x2 = C.x + t2 * (D.x - C.x);
        t = (x - x1) / (x2 - x1);
        INTERPOLATE_S (z);
        INTERPOLATE (texels,x);
        INTERPOLATE (texels,y);
        if (gouraud)
        {
          INTERPOLATE (colors,red);
          INTERPOLATE (colors,green);
          INTERPOLATE (colors,blue);
        }
        break;
    }
  }
}
#endif // CS_USE_OLD_RENDERER

#undef INTERPOLATE
#undef INTERPOLATE1
void csLightFlareHalo::ProcessFlareComponent (
  csEngine const &engine,
  csFlareComponent *comp,
  csVector2 const &start,
  csVector2 const &deltapos)
{
  csVector2 pos = start + comp->position * deltapos;

  /// Compute size and position of this component
  float hw = (halosize * comp->width) * 0.5f;
  float hh = (halosize * comp->height) * 0.5f;
  
  csVector2 HaloUV[4] =
  {
    csVector2 (0.0, 0.0),
    csVector2 (0.0, 1.0),
    csVector2 (1.0, 1.0),
    csVector2 (1.0, 0.0)
  };

#ifdef CS_USE_OLD_RENDERER
  // Create a rectangle containing the halo and clip it against screen
  csVector2 HaloPoly[4] =
  {
    csVector2 (pos.x - hw, pos.y - hh),
    csVector2 (pos.x - hw, pos.y + hh),
    csVector2 (pos.x + hw, pos.y + hh),
    csVector2 (pos.x + hw, pos.y - hh)
  };

  /// drawing info for the polygon
  G3DPolygonDPFX dpfx;
  
  // Clip the halo against clipper
  int num_clipped_verts;
  csVector2 clipped_poly2d[MAX_OUTPUT_VERTICES];
  csVertexStatus clipped_vtstats[MAX_OUTPUT_VERTICES];

  uint8 clip_result = engine.top_clipper->GetClipper ()->Clip (
      HaloPoly,
      4,
      clipped_poly2d,
      num_clipped_verts,
      clipped_vtstats);
  if (clip_result == CS_CLIP_OUTSIDE) return ;  // nothing to do

  // draw the halo
  float intensity = flare->GetIntensity ();
  uint mode = comp->mixmode;
  if (!(mode & CS_FX_ADD) || (intensity >= 1.0f))
  {
    mode |= CS_FX_FLAT;
    intensity = 1.0f;
  }

  //if(flare->GetIntensity() < 1.0)
  //return; // many drivers do not support combinations of weird modes
  //mode |= CS_FX_SETALPHA(1.0 - flare->GetIntensity());
  // prepare for drawing
  dpfx.num = num_clipped_verts;
  dpfx.use_fog = false;
  if (!comp->image)
  {
    csEngine::current_engine->Warn ("INTERNAL ERROR: flare used without material.");
    return ;
  }

  dpfx.mat_handle = comp->image->GetMaterialHandle ();
  if (!dpfx.mat_handle)
  {
    csEngine::current_engine->Warn ("INTERNAL ERROR: flare used without valid material handle.");
    return ;
  }

  comp->image->Visit ();
  dpfx.mat_handle->GetTexture ()->GetMeanColor (
      dpfx.flat_color_r,
      dpfx.flat_color_g,
      dpfx.flat_color_b);
  engine.G3D->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_NONE);

  // copy info
  int i;
  for (i = 0; i < 4; i++)
  {
    dpfx.z[i] = SMALL_Z;
    dpfx.vertices[i] = HaloPoly[i];
    dpfx.texels[i] = HaloUV[i];
    dpfx.colors[i].red = dpfx.colors[i].green = dpfx.colors[i].blue = intensity;
  }

  if (clip_result != CS_CLIP_INSIDE)
  {
    // need to interpolate u,v
    PreparePolygonFX2 (
      &dpfx,
      clipped_poly2d,
      num_clipped_verts,
      clipped_vtstats,
      4,
      true);
  }

  /// draw
  dpfx.mixmode = mode;
  engine.G3D->DrawPolygonFX (dpfx);
#else
  if (!comp->image)
  {
    csEngine::current_engine->Warn ("INTERNAL ERROR: flare used without material.");
    return ;
  }
  iMaterialHandle* mat_handle = comp->image->GetMaterialHandle ();
  if (!mat_handle)
  {
    csEngine::current_engine->Warn ("INTERNAL ERROR: flare used without valid material handle.");
    return ;
  }

  comp->image->Visit ();

  csSimpleRenderMesh mesh;

  float intensity = flare->GetIntensity ();
  uint mode = comp->mixmode;
  if (!((mode & CS_FX_MASK_MIXMODE) == CS_FX_ADD) || (intensity >= 1.0f))
  {
    mode |= CS_FX_FLAT;
    intensity = 1.0f;
  }
  else if ((mode & CS_FX_MASK_MIXMODE) == CS_FX_ALPHA)
  {
    intensity *= (float (mode & CS_FX_MASK_ALPHA) / 255.0f);
    mesh.alphaType.autoAlphaMode = false;
    mesh.alphaType.alphaType = csAlphaMode::alphaSmooth;
  }

  static uint indices[4] = {0, 1, 2, 3};
  csVector4 colors[4] = {csVector4 (intensity), csVector4 (intensity), 
    csVector4 (intensity), csVector4 (intensity)};

  pos.y = ((float)engine.G3D->GetHeight()) - pos.y;

  // Create a rectangle containing the halo and clip it against screen
  csVector3 HaloPoly[4] =
  {
    csVector3 (pos.x - hw, pos.y - hh, 0),
    csVector3 (pos.x + hw, pos.y - hh, 0),
    csVector3 (pos.x + hw, pos.y + hh, 0),
    csVector3 (pos.x - hw, pos.y + hh, 0)
  };

  mesh.meshtype = CS_MESHTYPE_QUADS;
  mesh.indexCount = 4;
  mesh.indices = indices;
  mesh.vertexCount = 4;
  mesh.vertices = HaloPoly;
  mesh.texcoords = HaloUV;
  mesh.colors = colors;
  mesh.texture = mat_handle->GetTexture();
  mesh.mixmode = ((mode & CS_FX_MASK_MIXMODE) != CS_FX_ALPHA) ? mode : CS_FX_COPY;

  engine.G3D->DrawSimpleMesh (mesh, csSimpleMeshScreenspace);
#endif // CS_USE_OLD_RENDERER
}
