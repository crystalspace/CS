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

#include "cssysdef.h"
#include "qint.h"
#include "csengine/halo.h"
#include "csengine/engine.h"
#include "csgeom/polyclip.h"
#include "csutil/halogen.h"

// The speed at which halo brightens/vanishes in milliseconds per frame
#define HALO_FRAME_TIME        20
// The speed at which halo brightens/vanishes in 0..1 units
#define HALO_INTENSITY_STEP    0.05

//--------------------------------------------------------------+ csHalo +---//

csHalo::csHalo (csHaloType iType)
{
  Intensity = 0;
  Type = iType;
}

csHalo::~csHalo ()
{
}

//---------------------------------------------------------+ csCrossHalo +---//

csCrossHalo::csCrossHalo (float intensity_factor, float cross_factor)
  : csHalo (cshtCross)
{
  IntensityFactor = intensity_factor;
  CrossFactor = cross_factor;
}

uint8 *csCrossHalo::Generate (int Size)
{
  return GenerateHalo (Size, IntensityFactor, CrossFactor);
}

//----------------------------------------------------------+ csNovaHalo +---//

csNovaHalo::csNovaHalo (int seed, int num_spokes, float roundness)
  : csHalo (cshtNova)
{
  Seed = seed;
  NumSpokes = num_spokes;
  Roundness = roundness;
}

uint8 *csNovaHalo::Generate (int Size)
{
  return GenerateNova (Size, Seed, NumSpokes, Roundness);
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

bool csLightHalo::Process (cs_time ElapsedTime, const csEngine &Engine)
{
  // Whenever the center of halo (the light) is directly visible
  bool halo_vis = false;
  // Whenever at least a piece of halo is visible
  bool draw_halo = false;
  // top-left coordinates of halo rectangle
  float xtl = 0, ytl = 0;

  // Project the halo.
  csVector3 v = Engine.current_camera->World2Camera (Light->GetCenter ());
  // The clipped halo polygon
  csVector2 HaloClip [32];
  // Number of vertices in HaloClip array
  int HaloVCount = 32;

  if (v.z > SMALL_Z)
  {
    float iz = Engine.current_camera->GetFOV () / v.z;
    v.x = v.x * iz + Engine.current_camera->GetShiftX ();
    v.y = Engine.frame_height - 1 -
      (v.y * iz + Engine.current_camera->GetShiftY ());

    if (Engine.top_clipper->IsInside (csVector2 (v.x, v.y)))
    {
      float zv = Engine.G3D->GetZBuffValue (QRound (v.x), QRound (v.y));
      halo_vis = (v.z <= zv);
    }

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

  if (ElapsedTime > 1000)
    ElapsedTime = 1000;
  LastTime += ElapsedTime;
  float hintensity = Light->GetHalo ()->GetIntensity ();
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
  Light->GetHalo ()->SetIntensity (hintensity);

  if (draw_halo)
    Handle->Draw (xtl, ytl, -1, -1, hintensity, HaloClip, HaloVCount);
  return true;
}
