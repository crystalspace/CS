/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein
  
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

#ifndef __CS_LIGHT_H__
#define __CS_LIGHT_H__

#include "csgeom/transfrm.h"
#include "csutil/csobject.h"
#include "csutil/cscolor.h"
#include "csutil/flags.h"
#include "csutil/csvector.h"
#include "csengine/lview.h"
#include "csengine/sector.h"
#include "iengine/light.h"
#include "iengine/statlght.h"
#include "iengine/dynlight.h"
#include "iengine/sector.h"

class csLightMap;
class csDynLight;
class csLightPatchPool;
class csHalo;
struct iMeshWrapper;

/**
 * If CS_LIGHT_THINGSHADOWS is set for a light then things will also
 * cast shadows. This flag is set by default for static lights and unset
 * for dynamic lights.
 */
#define CS_LIGHT_THINGSHADOWS	0x00000001

/**
 * If this flag is set, the halo for this light is active and is in the 
 * engine's queue of active halos. When halo become inactive, this flag
 * is reset.
 */
#define CS_LIGHT_ACTIVEHALO	0x80000000

/**
 * Superclass of all positional lights.
 * A light subclassing from this has a color, a position
 * and a radius.<P>
 *
 * First some terminology about all the several types of lights
 * that Crystal Space supports:
 * <ul>
 * <li>Static light. This is a normal static light that cannot move
 *     and cannot change intensity/color. All lighting information from
 *     all static lights is collected in one static lightmap.
 * <li>Pseudo-dynamic light. This is a static light that still cannot
 *     move but the intensity/color can change. The shadow information
 *     from every pseudo-dynamic light is kept in a seperate shadow-map.
 *     Shadowing is very accurate with pseudo-dynamic lights since they
 *     use the same algorithm as static lights.
 * <li>Dynamic light. This is a light that can move and change
 *     intensity/color. These lights are the most flexible. All lighting
 *     information from all dynamic lights is collected in one dynamic
 *     lightmap (seperate from the pseudo-dynamic shadow-maps).
 *     Shadows for dynamic lights will be less accurate because things
 *     will not cast accurate shadows (due to computation speed limitations).
 * </ul>
 * Note that static and pseudo-dynamic lights are represented by the
 * same csStatLight class.
 */
class csLight : public csObject
{
private:
  /// ID for this light.
  unsigned long light_id;
  /// Last used ID.
  static unsigned long last_light_id;

protected:
  /// Home sector of the light.
  csSector* sector;
  /// Position of the light.
  csVector3 center;
  /// Radius of light.
  float dist;
  /// Squared radius of light.
  float sqdist;
  /// Inverse radius of light.
  float inv_dist;
  /// Color.
  csColor color;
  /// The associated halo (if not NULL)
  csHalo *halo;

  /// Attenuation type
  int attenuation;

public:
  /// Set of flags
  csFlags flags;

public:
  /// Config value: ambient red value.
  static int ambient_red;
  /// Config value: ambient green value.
  static int ambient_green;
  /// Config value: ambient blue value.
  static int ambient_blue;

public:
  /**
   * Construct a light at a given position. With
   * a given radius, a given color, a given name and
   * type. The light will not have a halo by default.
   */
  csLight (float x, float y, float z, float dist,
  	 float red, float green, float blue);

  /**
   * Destroy the light. Note that destroying a light
   * may not have the expected effect. Static lights result
   * in changes in the lightmaps. Removing them will not automatically
   * update those lightmaps as that is a time-consuming process.
   */
  virtual ~csLight ();

  /// Get the ID of this light.
  unsigned long GetLightID () { return light_id; }

  /**
   * Set the current sector for this light.
   */
  virtual void SetSector (csSector* sector) { csLight::sector = sector; }

  /**
   * Get the current sector for this light.
   */
  csSector* GetSector () const { return sector; }

  /**
   * Set the center position.
   */
  void SetCenter (const csVector3& v) { center = v; }

  /**
   * Get the center position.
   */
  const csVector3& GetCenter () { return center; }

  /**
   * Get the radius.
   */
  float GetRadius () const { return dist; }

  /**
   * Get the squared radius.
   */
  float GetSquaredRadius () const { return sqdist; }

  /**
   * Get the inverse radius.
   */
  float GetInverseRadius () const { return inv_dist; }

  /**
   * Set the radius.
   */
  void SetRadius (float radius)
    { dist = radius; sqdist = dist*dist; inv_dist = 1 / dist; }

  /**
   * Get the light color.
   */
  csColor& GetColor () { return color; }

  /**
   * Set the light color. Note that setting the color
   * of a light may not always have an immediate visible effect.
   * Static lights are precalculated into the lightmaps and those
   * lightmaps are not automatically updated when calling this function
   * as that is a time consuming process.
   */
  virtual void SetColor (const csColor& col) { color = col; }

  /**
   * Return the associated halo
   */
  csHalo *GetHalo () { return halo; }

  /**
   * Set the halo associated with this light.
   */
  void SetHalo (csHalo *Halo);
 
  /**
   * Get the light's attenuation type
   */
  int GetAttenuation () {return attenuation;}

  /**
   * Change the light's attenuation type
   */
  void SetAttenuation (int a) {attenuation = a;}

  /**
   * Get the brightness of a light at a given distance.
   */
  float GetBrightnessAtDistance (float d);

  //------------------------ iLight interface -----------------------------
  SCF_DECLARE_IBASE_EXT (csObject);

  struct Light : public iLight
  {
    SCF_DECLARE_EMBEDDED_IBASE (csLight);
    virtual csLight* GetPrivateObject ()
    { return scfParent; }
    virtual unsigned long GetLightID ()
    { return scfParent->GetLightID (); }
    virtual iObject *QueryObject()
    { return scfParent; }
    virtual const csVector3& GetCenter ()
    { return scfParent->GetCenter (); }
    virtual void SetCenter (const csVector3& pos)
    { scfParent->SetCenter (pos); }
    virtual iSector *GetSector ()
    { return &scfParent->GetSector ()->scfiSector; }
    virtual void SetSector (iSector* sector)
    { scfParent->SetSector (sector->GetPrivateObject ()); }
    virtual float GetRadius ()
    { return scfParent->GetRadius (); }
    virtual float GetSquaredRadius ()
    { return scfParent->GetSquaredRadius (); }
    virtual float GetInverseRadius ()
    { return scfParent->GetInverseRadius (); }
    virtual void SetRadius (float r)
    { scfParent->SetRadius (r); }
    virtual const csColor& GetColor ()
    { return scfParent->GetColor (); }
    virtual void SetColor (const csColor& col)
    { scfParent->SetColor (col); }
    virtual int GetAttenuation ()
    { return scfParent->GetAttenuation (); }
    virtual void SetAttenuation (int a)
    { scfParent->SetAttenuation (a); }
    virtual float GetBrightnessAtDistance (float d)
    { return scfParent->GetBrightnessAtDistance (d); }

    virtual iCrossHalo* CreateCrossHalo (float intensity, float cross);
    virtual iNovaHalo* CreateNovaHalo (int seed, int num_spokes,
  	float roundness);
    virtual iFlareHalo* CreateFlareHalo ();
  } scfiLight;
};

/**
 * Class for a static light. These lights cast shadows (against
 * sector boundaries and with things), they support three different
 * colors (R,G,B). They cannot move and they can only vary in
 * intensity with some memory trade-offs (in which case we call
 * it a pseudo-dynamic light).
 */
class csStatLight : public csLight
{
private:
  /**
   * The following three variables are used if the light intensity
   * can vary. 'dynamic' is set to true in that case.
   * The 'lightmaps' vector indicates all lightmaps that are
   * possibly lit by this light.
   */
  bool dynamic;

  /// Vector of lightmaps that are affected by this dynamic light.
  csVector lightmaps;

public:
  /**
   * Construct a static light at a given position. With
   * a given radius and a given color. If 'dynamic' is
   * true we have a pseudo-dynamic light which can change
   * intensity and color (but not move).
   * The light will not have a halo by default.
   */
  csStatLight (float x, float y, float z, float dist,
  	 float red, float green, float blue,
	 bool dynamic);
  /**
   * Destroy the light. Note that destroying a light
   * may not have the expected effect. Static lights result
   * in changes in the lightmaps. Removing them will not automatically
   * update those lightmaps as that is a time-consuming process.
   */
  virtual ~csStatLight ();

  /**
   * Return true if this light is pseudo-dynamic.
   */
  bool IsDynamic () { return dynamic; }

  /**
   * Set the light color. Note that setting the color
   * of a light may not always have an immediate visible effect.
   * Static lights are precalculated into the lightmaps and those
   * lightmaps are not automatically updated when calling this function
   * as that is a time consuming process.
   * However, this function works as expected for pseudo-dynamic
   * lights. In this case the lightmaps will be correctly updated
   * the next time they become visible.
   */
  virtual void SetColor (const csColor& col);

  /**
   * Register a lightmap for a pseudo-dynamic light.
   * Every lightmap which is interested in updating itself
   * as this light changes should register itself to the light.
   */
  void RegisterLightMap (csLightMap* lmap);

  /**
   * Shine this light on all polygons visible from the light.
   * This routine will update the lightmaps of all polygons or
   * update the vertex colors if gouraud shading is used.
   * It correctly takes pseudo-dynamic lights into account and will then
   * update the corresponding shadow map.
   */
  void CalculateLighting ();

  /**
   * Shine this light on all polygons of the mesh.
   * Only backface culling is used. The light is assumed
   * to be in the same sector as the mesh.
   * Currently only works on thing meshes.
   */
  void CalculateLighting (iMeshWrapper* mesh);

  /**
   * This function is similar to CalculateLighting. It will do all the stuff
   * that CalculateLighting would do except for one important thing: it will
   * not actually light the polygons. Instead it will call a callback function for
   * every entity that it was planning to light. This allows you to show
   * or draw debugging information.
   */
  void LightingFunc (csLightingFunc* callback, void* callback_data = NULL);

  //------------------------ iStatLight interface -----------------------------
  SCF_DECLARE_IBASE_EXT (csLight);

  struct eiStaticLight : public iStatLight
  {
    SCF_DECLARE_EMBEDDED_IBASE (csStatLight);

    /// Used by the engine to retrieve internal static light object (ugly)
    virtual csStatLight *GetPrivateObject ()
    { return scfParent; }
    virtual iObject *QueryObject ()
    { return scfParent; }
    virtual iLight *QueryLight ()
    { return &scfParent->scfiLight; }
  } scfiStatLight;
  friend struct eiStaticLight;
};

/**
 * A light patch. This is a 3D polygon which fits on a world level 3D
 * polygon and defines where the light hits the polygon.
 * There is a list of light patches in every polygon (all dynamic lights
 * hitting a polygon will give rise to a seperate light patch) and there
 * is a list of light patches in every dynamic light (representing all
 * polygons that are hit by that particular light).
 */
class csLightPatch
{
  friend class csLightPatchPool;

private:
  csLightPatch* next_poly;
  csLightPatch* prev_poly;
  csLightPatch* next_light;
  csLightPatch* prev_light;

  /// Vertices.
  csVector3* vertices;
  /// Current number of vertices.
  int num_vertices;
  /// Maximum number of vertices.
  int max_vertices;

  /// Polygon that this light patch is for.
  csPolygon3D* polygon;
  /// Curve that this light patch is for
  csCurve* curve;

  /// Light that this light patch originates from.
  csDynLight* light;

  /// List of shadow frustums.
  csShadowBlock shadows;

  /// frustum of where the visible light hits (for use with curves)
  csFrustum *light_frustum;

private:
  /**
   * Create an empty light patch (infinite frustum).
   */
  csLightPatch ();

  /**
   * Unlink this light patch from the polygon and the light
   * and then destroy.
   */
  ~csLightPatch ();

public:
  /**
   * Make room for the specified number of vertices and
   * initialize to start a new light patch.
   */
  void Initialize (int n);

  /**
   * Remove this light patch (unlink from all lists).
   */
  void RemovePatch ();

  /**
   * Get the polygon that this light patch belongs too.
   */
  csPolygon3D* GetPolygon () { return polygon; }

  /**
   * Get the curve that this light patch belongs too.
   */
  csCurve* GetCurve () { return curve; }

  /**
   * Get the light that this light patch belongs too.
   */
  csDynLight* GetLight () { return light; }

  /// Get a reference to the shadow list.
  csShadowBlock& GetShadowBlock () { return shadows; }

  /// Get the number of vertices in this light patch.
  int GetVertexCount () { return num_vertices; }
  /// Get all the vertices.
  csVector3* GetVertices () { return vertices; }

  /// Get a vertex.
  csVector3& GetVertex (int i)
  {
    CS_ASSERT (vertices != NULL);
    CS_ASSERT (i >= 0 && i < num_vertices);
    return vertices[i];
  }

  /**
   * Get next light patch as seen from the standpoint
   * of the polygon.
   */
  csLightPatch* GetNextPoly () { return next_poly; }

  /**
   * Get the next light patch as seen from the standpoint
   * of the light.
   */
  csLightPatch* GetNextLight () { return next_light; }

  /// Set polygon.
  void SetPolyCurve (csPolygon3D* pol) { polygon = pol; curve = NULL; }
  /// Set curve.
  void SetPolyCurve (csCurve* c) { curve = c; polygon = NULL; }
  /// Set light.
  void SetLight (csDynLight* l) { light = l; }
  /// Add to poly list.
  void AddPolyList (csLightPatch*& first)
  {
    next_poly = first;
    prev_poly = NULL;
    if (first) 
      first->prev_poly = this;
    first = this;
  }
  /// Remove from poly list.
  void RemovePolyList (csLightPatch*& first)
  {
    if (next_poly) next_poly->prev_poly = prev_poly;
    if (prev_poly) prev_poly->next_poly = next_poly;
    else first = next_poly;
    prev_poly = next_poly = NULL;
    polygon = NULL;
    curve = NULL;
  }
  /// Add to light list.
  void AddLightList (csLightPatch*& first)
  {
    next_light = first;
    prev_light = NULL;
    if (first) 
      first->prev_light = this;
    first = this;
  }
  /// Remove from light list.
  void RemoveLightList (csLightPatch*& first)
  {
    if (next_light) next_light->prev_light = prev_light;
    if (prev_light) prev_light->next_light = next_light;
    else first = next_light;
    prev_light = next_light = NULL;
    light = NULL;
  }

  /// Set the light frustum.
  void SetLightFrustum (csFrustum* lf) { light_frustum = lf; }
  /// Get the light frustum.
  csFrustum* GetLightFrustum () { return light_frustum; }
};

/**
 * Class for a dynamic light. These lights only cast shadows
 * for sectors/portals and not for things. However, they can
 * freely move and change color intensity.
 */
class csDynLight : public csLight
{
private:
  csDynLight* next;
  csDynLight* prev;

  /// List of light patches for this dynamic light.
  csLightPatch* lightpatches;

public:
  /**
   * Create a new dynamic light at the given position and with the
   * given radius and color. Initially the light will
   * not be visible. You need to set the current
   * sector and call 'Setup()' first.
   */
  csDynLight (float x, float y, float z, float dist,
  	 float red, float green, float blue);

  /**
   * Remove the dynamic light from all polygons (i.e.
   * remove all light patches) and then destroy the light itself.
   */
  virtual ~csDynLight ();

  /**
   * Initial placement of the light. This routine generates a view
   * frustum as seen from the light. The clipped polygons that
   * result from this are light patches and are put in the
   * lightpatches list. This routine needs to be called whenever
   * the light moves.
   */
  void Setup ();

  /**
   * Call this when the color of the light changes. This is more
   * efficient than calling Setup().
   */
  virtual void SetColor (const csColor& col);

  /**
   * Unlink a light patch from the light patch list.
   * Warning! This function does not test if the light patch
   * is really on the list!
   */
  void UnlinkLightpatch (csLightPatch* lp);

  /**
   * Add a light patch to the light patch list.
   */
  void AddLightpatch (csLightPatch* lp);

  ///
  void SetNext (csDynLight* n) { next = n; }
  ///
  void SetPrev (csDynLight* p) { prev = p; }
  ///
  csDynLight* GetNext () { return next; }
  ///
  csDynLight* GetPrev () { return prev; }

  //------------------------ iDynLight interface -----------------------------
  SCF_DECLARE_IBASE_EXT (csLight);

  struct eiDynLight : public iDynLight
  {
    SCF_DECLARE_EMBEDDED_IBASE (csDynLight);

    /// Used by the engine to retrieve internal dyn light object (ugly)
    virtual csDynLight* GetPrivateObject ()
    { return scfParent; }
    virtual void Setup ()
    { scfParent->Setup (); }
    virtual iObject *QueryObject ()
    { return scfParent; }
    virtual iLight *QueryLight ()
    { return &(scfParent->scfiLight); }
    virtual iDynLight* GetNext ()
    { return &(scfParent->GetNext())->scfiDynLight; }
  } scfiDynLight;
  friend struct eiDynLight;
};

#endif // __CS_LIGHT_H__
