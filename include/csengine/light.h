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
#include "csobject/csobject.h"
#include "csutil/cscolor.h"
#include "csutil/flags.h"
#include "csengine/lview.h"
#include "iengine/light.h"
#include "iengine/statlght.h"
#include "iengine/dynlight.h"

class Dumper;
class csSector;
class csLightMap;
class csDynLight;
class csThing;
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
 * Attenuation controls how the brightness of a light fades with distance.
 * There are four attenuation formulas:
 * <ul>
 *   <li> no attenuation = light * 1
 *   <li> linear attenuation = light * (radius - distance) / radius
 *   <li> inverse attenuation = light * (radius / distance)
 *   <li> realistic attenuation = light * (radius^2 / distance^2)
 * </ul>
 */
#define CS_ATTN_NONE      0
#define CS_ATTN_LINEAR    1
#define CS_ATTN_INVERSE   2
#define CS_ATTN_REALISTIC 3

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

  /**
   * Set the current sector for this light.
   */
  virtual void SetSector (csSector* sector) { csLight::sector = sector; }

  /**
   * Get the current sector for this light.
   */
  csSector* GetSector () const { return sector; }

  /**
   * Get the center position.
   */
  csVector3& GetCenter () { return center; }

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

  /**
   * Change the given r, g, b value to the current mixing mode
   * (TRUE_RGB or NOCOLOR). In NOCOLOR mode this function will
   * take the average of the three colors to return a grayscale
   * value.
   */
  static void CorrectForNocolor (unsigned char* rp, unsigned char* gp, unsigned char* bp);

  /**
   * Change the given r, g, b value to the current mixing mode
   * (TRUE_RGB or NOCOLOR). In NOCOLOR mode this function will
   * take the average of the three colors to return a grayscale
   * value.
   */
  static void CorrectForNocolor (float* rp, float* gp, float* bp);  

  CSOBJTYPE;

  //------------------------ iLight interface -----------------------------
  DECLARE_IBASE_EXT (csObject);

  struct Light : public iLight
  {
    DECLARE_EMBEDDED_IBASE (csLight);
    virtual csLight* GetPrivateObject () { return (csLight*)scfParent; }
    virtual csVector3& GetCenter () { return scfParent->GetCenter (); }
    virtual float GetSquaredRadius () const { return scfParent->GetSquaredRadius (); }
    virtual csColor& GetColor () { return scfParent->GetColor (); }
    virtual void SetColor (const csColor& col) { scfParent->SetColor (col); }
    virtual void SetSector (iSector* sector);
    virtual float GetBrightnessAtDistance (float d)
    {
      return scfParent->GetBrightnessAtDistance (d);
    }
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
   * 'num_polygon' and 'polygons' indicate all polygons that are
   * possibly lit by this light.
   */
  bool dynamic;

  /// Number of lightmaps affected by this dynamic light.
  int num_lightmap;

  /// List of lightmaps that are affected by this dynamic light.
  csLightMap** lightmaps;

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
   * Shine this light on all polygons of the csThing.
   * Only backface culling is used. The light is assumed
   * to be in the same sector as the csThing.
   */
  void CalculateLighting (csThing* th);

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

  CSOBJTYPE;

  //------------------------ iStatLight interface -----------------------------
  DECLARE_IBASE_EXT (csLight);

  struct eiStaticLight : public iStatLight
  {
    DECLARE_EMBEDDED_IBASE (csStatLight);

    /// Used by the engine to retrieve internal static light object (ugly)
    virtual csStatLight *GetPrivateObject ()
    { return scfParent; }
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
  friend class csPolygon3D;
  friend class csCurve;
  friend class csPolyTexture;
  friend class csDynLight;
  friend class Dumper;
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
   * Move the light. This will NOT automatically recalculate the
   * view frustum. You still need to call Setup() after this.
   */
  void Move (csSector* sector, csVector3& v) { Move (sector, v.x, v.y, v.z); }

  /**
   * Move the light. This will NOT automatically recalculate the
   * view frustum. You still need to call Setup() after this.
   */
  void Move (csSector* sector, float x, float y, float z);

  /**
   * Resize the light. This will NOT automatically recalculate the
   * view frustum. You still need to call Setup() after this.
   */
  void Resize (float radius) { dist = radius; sqdist = dist*dist; inv_dist = 1 / dist; }

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

  CSOBJTYPE;

  //------------------------ iDynLight interface -----------------------------
  DECLARE_IBASE_EXT (csLight);

  struct eiDynLight : public iDynLight
  {
    DECLARE_EMBEDDED_IBASE (csDynLight);

    /// Used by the engine to retrieve internal dyn light object (ugly)
    virtual csDynLight* GetPrivateObject ()
    { return scfParent; }
    virtual void Setup () { scfParent->Setup (); }
  } scfiDynLight;
  friend struct eiDynLight;
};

#endif // __CS_LIGHT_H__
