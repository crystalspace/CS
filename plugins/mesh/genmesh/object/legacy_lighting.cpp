/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein

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

#include "iengine/engine.h"
#include "iengine/movable.h"
#include "iengine/sector.h"

#include "csgeom/math3d.h"

#include "genmesh.h"

CS_PLUGIN_NAMESPACE_BEGIN(Genmesh)
{

csGenmeshMeshObject::LegacyLightingData::LegacyLightingData ()
 : lit_mesh_colors (0), num_lit_mesh_colors (0), static_mesh_colors (0)
{
}

csGenmeshMeshObject::LegacyLightingData::~LegacyLightingData ()
{
  delete[] lit_mesh_colors;
  delete[] static_mesh_colors;
}

void csGenmeshMeshObject::LegacyLightingData::SetColorNum (int num)
{
  num_lit_mesh_colors = num;
  delete[] lit_mesh_colors;
  lit_mesh_colors = new csColor4 [num_lit_mesh_colors];
  delete[] static_mesh_colors;
  static_mesh_colors = new csColor4 [num_lit_mesh_colors];
}

void csGenmeshMeshObject::LegacyLightingData::Free ()
{
  delete[] lit_mesh_colors;
  lit_mesh_colors = 0;
  delete[] static_mesh_colors;
  static_mesh_colors = 0;
}

void csGenmeshMeshObject::LegacyLightingData::Clear ()
{
  //csColor amb;
  //factory->engine->GetAmbientLight (amb);
  int i;
  for (i = 0 ; i < num_lit_mesh_colors ; i++)
  {
    lit_mesh_colors[i].Set (0, 0, 0);
    static_mesh_colors[i].Set (0, 0, 0);
  }
}


#define VERTEX_OFFSET       (10.0f * SMALL_EPSILON)

/*
  Lighting w/o local shadows:
  - Contribution from all affecting lights is calculated and summed up
    at runtime.
  Lighting with local shadows:
  - Contribution from static lights is calculated, summed and stored.
  - For every static pseudo-dynamic lights, the intensity of contribution
    is stored.
  - At runtime, the static lighting colors are copied to the actual used
    colors, the intensities of the pseudo-dynamic lights are multiplied
    with the actual colors of that lights and added as well, and finally,
    dynamic lighst are calculated.
*/
void csGenmeshMeshObject::CastShadows (iMovable* movable, iFrustumView* fview)
{
  SetupObject ();

  if (do_manual_colors) return;
  if (!do_lighting) return;

  iBase* b = (iBase *)fview->GetUserdata ();
  csRef<iLightingProcessInfo> lpi = scfQueryInterface<iLightingProcessInfo> (b);
  CS_ASSERT (lpi != 0);

  iLight* li = lpi->GetLight ();
  bool dyn = lpi->IsDynamic ();

  if (!dyn)
  {
    if (!do_shadow_rec || li->GetDynamicType () == CS_LIGHT_DYNAMICTYPE_PSEUDO)
    {
      li->AddAffectedLightingInfo (this);
      if (li->GetDynamicType () != CS_LIGHT_DYNAMICTYPE_PSEUDO)
        affecting_lights.Add (li);
    }
  }
  else
  {
    if (!affecting_lights.In (li))
    {
      li->AddAffectedLightingInfo (this);
      affecting_lights.Add (li);
    }
    if (do_shadow_rec) return;
  }

  if (!do_shadow_rec) return;

  csReversibleTransform o2w (movable->GetFullTransform ());

  csFrustum *light_frustum = fview->GetFrustumContext ()->GetLightFrustum ();
  iShadowBlockList* shadows = fview->GetFrustumContext ()->GetShadows ();
  iShadowIterator* shadowIt = shadows->GetShadowIterator ();

  csVector3* normals = factory->GetNormals ();
  csVector3* vertices = factory->GetVertices ();
  csColor4* colors = legacyLighting.static_mesh_colors;
  // Compute light position in object coordinates
  csVector3 wor_light_pos = li->GetMovable ()->GetFullPosition ();
  csVector3 obj_light_pos = o2w.Other2This (wor_light_pos);

  bool pseudoDyn = li->GetDynamicType () == CS_LIGHT_DYNAMICTYPE_PSEUDO;
  csShadowArray* shadowArr = 0;
  if (pseudoDyn)
  {
    shadowArr = new csShadowArray ();
    pseudoDynInfo.Put (li, shadowArr);
    shadowArr->shadowmap = new float[factory->GetVertexCount ()];
    memset(shadowArr->shadowmap, 0, factory->GetVertexCount() * sizeof(float));
  }

  csColor light_color = li->GetColor () * (256. / CS_NORMAL_LIGHT_LEVEL);

  csColor col;
  int i;
  for (i = 0 ; i < factory->GetVertexCount () ; i++)
  {
    const csVector3& normal = normals[i];
#ifdef SHADOW_CAST_BACKFACE
    csVector3 v = o2w.This2Other (vertices[i]) - wor_light_pos;
#else
    /*
      A small fraction of the normal is added to prevent unwanted
      self-shadowing (due small inaccuracies, the tri(s) this vertex
      lies on may shadow it.)
     */
    csVector3 v = o2w.This2Other (vertices[i] + (normal * VERTEX_OFFSET)) -
      wor_light_pos;
    /*csVector3 vN (v); vN.Normalize();
    v -= (vN * 0.1f);*/
#endif

    if (!light_frustum->Contains (v))
    {
      continue;
    }
    
    float vrt_sq_dist = csSquaredDist::PointPoint (obj_light_pos,
      vertices[i]);
    if (vrt_sq_dist >= csSquare (li->GetCutoffDistance ())) continue;
    
    bool inShadow = false;
    shadowIt->Reset ();
    while (shadowIt->HasNext ())
    {
      csFrustum* shadowFrust = shadowIt->Next ();
      if (shadowFrust->Contains (v))
      {
	inShadow = true;
	break;
      }
    }
    if (inShadow) continue;

    float in_vrt_dist =
      (vrt_sq_dist >= SMALL_EPSILON) ? csQisqrt (vrt_sq_dist) : 1.0f;

    float cosinus;
    if (vrt_sq_dist < SMALL_EPSILON) cosinus = 1;
    else cosinus = (obj_light_pos - vertices[i]) * normal;
    // because the vector from the object center to the light center
    // in object space is equal to the position of the light

    if (cosinus > 0)
    {
      if (vrt_sq_dist >= SMALL_EPSILON) cosinus *= in_vrt_dist;
      float bright = li->GetBrightnessAtDistance (csQsqrt (vrt_sq_dist));
      if (cosinus < 1) bright *= cosinus;
      if (pseudoDyn)
      {
	// Pseudo-dynamic
	if (bright > 2.0f) bright = 2.0f; // @@@ clamp here?
	shadowArr->shadowmap[i] = bright;
      }
      else
      {
	col = light_color * bright;
	colors[i] += col;
      }
    }
  }
  shadowIt->DecRef();
}

void csGenmeshMeshObject::UpdateLightingOne (
  const csReversibleTransform& trans, iLight* li)
{
  csVector3* normals = factory->GetNormals ();
  csColor4* colors = legacyLighting.lit_mesh_colors;
  // Compute light position in object coordinates
  csVector3 wor_light_pos = li->GetMovable ()->GetFullPosition ();
  csVector3 obj_light_pos = trans.Other2This (wor_light_pos);
  float obj_sq_dist = obj_light_pos * obj_light_pos;
  if (obj_sq_dist >= csSquare (li->GetCutoffDistance ())) return;
  float in_obj_dist =
    (obj_sq_dist >= SMALL_EPSILON) ? csQisqrt (obj_sq_dist) : 1.0f;

  csColor light_color = li->GetColor () * (256. / CS_NORMAL_LIGHT_LEVEL)
      * li->GetBrightnessAtDistance (csQsqrt (obj_sq_dist));
  if (light_color.red < EPSILON && light_color.green < EPSILON
  	&& light_color.blue < EPSILON)
    return;

  csColor col;
  int i;
  if (obj_sq_dist < SMALL_EPSILON)
  {
    for (i = 0 ; i < factory->GetVertexCount () ; i++)
    {
      colors[i] += light_color;
    }
  }
  else
  {
    obj_light_pos *= in_obj_dist;
    for (i = 0 ; i < factory->GetVertexCount () ; i++)
    {
      float cosinus = obj_light_pos * normals[i];
      // because the vector from the object center to the light center
      // in object space is equal to the position of the light

      if (cosinus > 0)
      {
        col = light_color;
        if (cosinus < 1) col *= cosinus;
        colors[i] += col;
      }
    }
  }
}

/*
Rules for color calculation:
  EAmb = Static Engine Ambient
  SAmb = Dynamic Sector Ambient
  BC   = Base Color (base_color)
  FC   = Color Array from factory
  SC   = Static Color Array (static_mesh_colors)
  LC   = Colors calculated from all relevant lights
  LDC  = Colors calculated from dynamic lights only
  C    = Final Color Array (lit_mesh_colors)

  sr   = do_shadow_rec flag
  l    = lighting flag
  mc   = manual colors flag

  sr   mc   l    formula
  ----------------------
  *    1    *    C[i] = FC[i]
  *    0    0    C[i] = BC+FC[i]
  1    0    1    C[i] = BC+SC[i]+EAmb+SAmb+FC[i]+LDC[i]
  0    0    1    C[i] = BC+LC[i]+EAmb+SAmb+FC[i]
*/

void csGenmeshMeshObject::UpdateLighting (
    const csSafeCopyArray<csLightInfluence>& lights,
    iMovable* movable)
{
  int i;
  if (factory->DoFullBright ())
  {
    if (lighting_dirty)
    {
      lighting_dirty = false;
      for (i = 0 ; i < factory->GetVertexCount () ; i++)
      {
        legacyLighting.lit_mesh_colors[i].Set (1, 1, 1);
      }
    }
    return;
  }

  if (cur_movablenr != movable->GetUpdateNumber ())
  {
    lighting_dirty = true;
    cur_movablenr = movable->GetUpdateNumber ();
  }

  if (do_manual_colors) return;

  csColor4* factory_colors = factory->GetColors (false);

  if (do_lighting)
  {
    if (!lighting_dirty)
    {
      iSector* sect = movable->GetSectors ()->Get (0);
      if (dynamic_ambient_version == sect->GetDynamicAmbientVersion ())
        return;
      dynamic_ambient_version = sect->GetDynamicAmbientVersion ();
    }
    lighting_dirty = false;
    mesh_colors_dirty_flag = true;

    csColor4 col;
    if (factory->engine)
    {
      factory->engine->GetAmbientLight (col);
      col += base_color;
      iSector* sect = movable->GetSectors ()->Get (0);
      if (sect)
        col += sect->GetDynamicAmbientLight ();
    }
    else
    {
      col = base_color;
    }
    if (factory_colors) 
    {
      for (i = 0 ; i < factory->GetVertexCount () ; i++)
      {
        legacyLighting.lit_mesh_colors[i] = col + 
	  legacyLighting.static_mesh_colors[i] + factory_colors[i];
      }
    }
    else
    {
      for (i = 0 ; i < factory->GetVertexCount () ; i++)
      {
        legacyLighting.lit_mesh_colors[i] = col + legacyLighting.static_mesh_colors[i];
      }
    }
    if (do_shadow_rec)
    {
      csReversibleTransform trans = movable->GetFullTransform ();
      csSet<csPtrKey<iLight> >::GlobalIterator it = affecting_lights.
      	GetIterator ();
      while (it.HasNext ())
      {
        iLight* l = (iLight*)it.Next ();
        UpdateLightingOne (trans, l);
      }
      csHash<csShadowArray*, csPtrKey<iLight> >::GlobalIterator pdlIt =
        pseudoDynInfo.GetIterator ();
      while (pdlIt.HasNext ())
      {
        csPtrKey<iLight> l;
        csShadowArray* shadowArr = pdlIt.Next (l);
        csColor c = l->GetColor ();
        if (c.red > EPSILON || c.green > EPSILON || c.blue > EPSILON)
        {
          c = c * (256. / CS_NORMAL_LIGHT_LEVEL);
          float* intensities = shadowArr->shadowmap;
          for (int i = 0; i < legacyLighting.num_lit_mesh_colors; i++)
          {
            legacyLighting.lit_mesh_colors[i] += c * intensities[i];
          }
        }
      }
    }
    else
    {
      // Do the lighting.
      csReversibleTransform trans = movable->GetFullTransform ();
      // the object center in world coordinates. "0" because the object
      // center in object space is obviously at (0,0,0).
      int num_lights = (int)lights.GetSize ();
      for (int l = 0 ; l < num_lights ; l++)
      {
        iLight* li = lights[l].light;
        li->AddAffectedLightingInfo (this);
        affecting_lights.Add (li);
        UpdateLightingOne (trans, li);
      }
    }
    // @@@ Try to avoid this loop!
    // Clamp all vertex colors to 2.
    for (i = 0 ; i < factory->GetVertexCount () ; i++)
      legacyLighting.lit_mesh_colors[i].Clamp (2., 2., 2.);
  }
  else
  {
    if (!lighting_dirty)
      return;
    lighting_dirty = false;
    mesh_colors_dirty_flag = true;

    if (factory_colors)
    {
      for (i = 0 ; i < factory->GetVertexCount () ; i++)
      {
        legacyLighting.lit_mesh_colors[i] = base_color + factory_colors[i];
        legacyLighting.lit_mesh_colors[i].Clamp (2., 2., 2.);
      }
    }
    else
    {
      csColor4 base_color_clamped (base_color);
      base_color_clamped.Clamp (2., 2., 2.);
      for (i = 0 ; i < factory->GetVertexCount () ; i++)
      {
        legacyLighting.lit_mesh_colors[i] = base_color_clamped;
      }
    }
  }
}

}
CS_PLUGIN_NAMESPACE_END(Genmesh)
