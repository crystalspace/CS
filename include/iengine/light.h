/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein
    Copyright (C) 1999 by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __CS_IENGINE_LIGHT_H__
#define __CS_IENGINE_LIGHT_H__

/**\file
 */
/**
 * \addtogroup engine3d_light
 * @{ */
 
#include "csutil/scf.h"

#include "iengine/fview.h"

class csColor;
class csFlags;
class csVector3;

struct iLight;
struct iLightingInfo;
struct iMovable;
struct iObject;
struct iSector;

struct iBaseHalo;
struct iCrossHalo;
struct iNovaHalo;
struct iFlareHalo;

/** \name Light flags
 * @{ */
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
/** @} */

/** \name Light Dynamic Types
 * @{ */

enum csLightDynamicType
{
  /**
   * A fully static light. Unless you are using shaders/renderloop that treat
   * all lights as dynamic this light cannot move and cannot change color.
   * Shadows are accurate and precalculated (if you use lightmaps).
   */
  CS_LIGHT_DYNAMICTYPE_STATIC = 1, 
 
  /**
   * A pseudo-dynamic light. Unless you are using shaders/renderloop that treat
   * all lights as dynamic this light cannot move but it can change color.
   * Shadows are accurate and precalculated (if you use lightmaps).
   */
  CS_LIGHT_DYNAMICTYPE_PSEUDO = 2,

  /**
   * A fully dynamic light.
   * No shadows are calculated unless you use a shader/renderloop
   * that does that in hardware.
   */
  CS_LIGHT_DYNAMICTYPE_DYNAMIC = 3
};
/** @} */

/// Light level that is used when there is no light on the texture.
#define CS_DEFAULT_LIGHT_LEVEL 20
/// Light level that corresponds to a normally lit texture.
#define CS_NORMAL_LIGHT_LEVEL 128

/** \name Attenuation modes
 * Attenuation controls how the brightness of a light fades with distance.
 * Apart from the attenuation mode there are one to tree constants,
 * which change meaning depending on attenuation mode.
 * There are five attenuation formulas: (distance is distance between
 * point for which lighting is computed and the light)
 * <ul>
 *   <li> no attenuation = light * 1
 *   <li> linear attenuation = light * (1 - distance / constant1)
 *   <li> inverse attenuation = light / distance
 *   <li> realistic attenuation = light / distance^2
 *   <li> CLQ, Constant Linear Quadratic 
 *        = light / (constant1 + constant2*distance + constant3*distance^2)
 * </ul>
 * @{ */
enum csLightAttenuationMode
{
  /// no attenuation: light * 1
  CS_ATTN_NONE = 0,
  /// linear attenuation: light * (1 - distance / constant1)
  CS_ATTN_LINEAR = 1,
  /// inverse attenuation: light / distance
  CS_ATTN_INVERSE = 2,
  /// realistic attenuation: light / distance^2
  CS_ATTN_REALISTIC = 3,
  /** 
   * CLQ, Constant Linear Quadratic: 
   * light / (constant1 + constant2*distance + constant3*distance^2)
   */
  CS_ATTN_CLQ = 4
};
/** @} */

/**
 * Type of lightsource. 
 * There are currently three types of lightsources:
 * <ul>
 *   <li> Point lights - have a position. Shines in all directions.
 *   <li> Directional lights - have a direction and radius. Shines along it's
 *                             major axis.
 *   <li> Spot lights - have both position and direction. Shines with full
 *                      strength along major axis and out to the hotspot angle.
 *                      Between hotspot and outer angle it will falloff, outside
 *                      outer angle there shines no light.
 * </ul>
 */
enum csLightType
{
  /// Point light
  CS_LIGHT_POINTLIGHT,
  /// Directional light
  CS_LIGHT_DIRECTIONAL,
  /// Spot light
  CS_LIGHT_SPOTLIGHT
};

/**
 * Set a callback which is called when this light color is changed.
 * The given context will be either an instance of iRenderView, iFrustumView,
 * or else 0.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>Application.
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>iLight
 *   </ul>
 */
struct iLightCallback : public virtual iBase
{
  SCF_INTERFACE(iLightCallback,2,0,0);
  /**
   * Light color will be changed. It is safe to delete this callback
   * in this function.
   */
  virtual void OnColorChange (iLight* light, const csColor& newcolor) = 0;

  /**
   * Light position will be changed. It is safe to delete this callback
   * in this function.
   */
  virtual void OnPositionChange (iLight* light, const csVector3& newpos) = 0;

  /**
   * Sector will be changed. It is safe to delete this callback
   * in this function.
   */
  virtual void OnSectorChange (iLight* light, iSector* newsector) = 0;

  /**
   * Radius will be changed.
   * It is safe to delete this callback in this function.
   */
  virtual void OnRadiusChange (iLight* light, float newradius) = 0;

  /**
   * Light will be destroyed.
   * It is safe to delete this callback in this function.
   */
  virtual void OnDestroy (iLight* light) = 0;

  /**
   * Attenuation will be changed.
   * It is safe to delete this callback in this function.
   */
  virtual void OnAttenuationChange (iLight* light, int newatt) = 0;
};


/**
 * The iLight interface is the SCF interface for the csLight class.
 * <p>
 * First some terminology about all the several types of lights
 * that Crystal Space supports:
 * <ul>
 * <li>Static light. This is a normal static light that cannot move
 *     and cannot change intensity/color. All lighting information from
 *     all static lights is collected in one static lightmap.
 * <li>Pseudo-dynamic light. This is a static light that still cannot
 *     move but the intensity/color can change. The shadow information
 *     from every pseudo-dynamic light is kept in a separate shadow-map.
 *     Shadowing is very accurate with pseudo-dynamic lights since they
 *     use the same algorithm as static lights.
 * <li>Dynamic light. This is a light that can move and change
 *     intensity/color. These lights are the most flexible. All lighting
 *     information from all dynamic lights is collected in one dynamic
 *     lightmap (separate from the pseudo-dynamic shadow-maps).
 *     Shadows for dynamic lights will be less accurate because things
 *     will not cast accurate shadows (due to computation speed limitations).
 * </ul>
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>iEngine::CreateLight()
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>iEngine::FindLight()
 *   <li>iEngine::FindLightID()
 *   <li>iEngine::GetLightIterator()
 *   <li>iEngine::GetNearbyLights()
 *   <li>iLightList::Get()
 *   <li>iLightList::FindByName()
 *   <li>iLoaderContext::FindLight()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>iEngine
 *   </ul>
 */
struct iLight : public virtual iBase
{
  SCF_INTERFACE(iLight,2,0,0);
  /// Get the id of this light. This is a 16-byte MD5.
  virtual const char* GetLightID () = 0;

  /// Get the iObject for this light.
  virtual iObject *QueryObject() = 0;

  /**
   * Get the dynamic type of this light.
   * Supported types:
   * <ul>
   * <li>#CS_LIGHT_DYNAMICTYPE_STATIC
   * <li>#CS_LIGHT_DYNAMICTYPE_PSEUDO
   * <li>#CS_LIGHT_DYNAMICTYPE_DYNAMIC
   * </ul>
   */
  virtual csLightDynamicType GetDynamicType () const = 0;

  /// Get the position of this light.
  virtual const csVector3& GetCenter () const = 0;
  /// Set the position of this light.
  virtual void SetCenter (const csVector3& pos) = 0;

  /// Get the sector for this light.
  virtual iSector *GetSector () = 0;

  /// Get the movable for this light.
  virtual iMovable *GetMovable () = 0;

  /// Get the diffuse color of this light.
  virtual const csColor& GetColor () const = 0;
  /// Set the diffuse color of this light.
  virtual void SetColor (const csColor& col) = 0;

  /// Get the specular color of this light.
  virtual const csColor& GetSpecularColor () const = 0;
  /// Set the specular color of this light.
  virtual void SetSpecularColor (const csColor& col) = 0;
  
  /// Get the light type of this light.
  virtual csLightType GetType () const = 0;
  /// Set the light type of this light.
  virtual void SetType (csLightType type) = 0;

  /// Get the light direction. Used for directional and spotlight.
  virtual const csVector3& GetDirection () const = 0;
  /// Set the light direction. Used for directional and spotlight.
  virtual void SetDirection (const csVector3& v) = 0;

  /**
   * Return current attenuation mode.
   * \sa csLightAttenuationMode
   */
  virtual csLightAttenuationMode GetAttenuationMode () const = 0;
  /**
   * Set attenuation mode.
   * \sa csLightAttenuationMode
   */
  virtual void SetAttenuationMode (csLightAttenuationMode a) = 0;

  /**
   * Set attenuation constants
   * \sa csLightAttenuationMode
   */
  virtual void SetAttenuationConstants (const csVector3& constants) = 0;
  /**
   * Get attenuation constants
   * \sa csLightAttenuationMode
   */
  virtual const csVector3 &GetAttenuationConstants () const = 0;

  /**
   * Get the the maximum distance at which the light is guranteed to shine. 
   * Can be seen as the distance at which we turn the light of.
   * Used for culling and selection of meshes to light, but not
   * for the lighting itself.
   */
  virtual float GetCutoffDistance () const = 0;

  /**
   * Set the the maximum distance at which the light is guranteed to shine. 
   * Can be seen as the distance at which we turn the light of.
   * Used for culling and selection of meshes to light, but not
   * for the lighting itself.
   */
  virtual void SetCutoffDistance (float distance) = 0;

  /**
   * Get radial cutoff distance for directional lights.
   * The directional light can be viewed as a cylinder with radius
   * equal to DirectionalCutoffRadius and length CutoffDistance
   */
  virtual float GetDirectionalCutoffRadius () const = 0;

  /**
   * Set radial cutoff distance for directional lights.
   * The directional light can be viewed as a cylinder with radius
   * equal to DirectionalCutoffRadius and length CutoffDistance
   */
  virtual void SetDirectionalCutoffRadius (float radius) = 0;

  /**
   * Set spot light falloff angles. Set in cosine of the angle. 
   */
  virtual void SetSpotLightFalloff (float inner, float outer) = 0;

  /**
   * Get spot light falloff angles. Get in cosine of the angle.
   */
  virtual void GetSpotLightFalloff (float& inner, float& outer) const = 0;

  /// Create a cross halo for this light.
  virtual iCrossHalo* CreateCrossHalo (float intensity, float cross) = 0;
  /// Create a nova halo for this light.
  virtual iNovaHalo* CreateNovaHalo (int seed, int num_spokes,
  	float roundness) = 0;
  /// Create a flare halo for this light.
  virtual iFlareHalo* CreateFlareHalo () = 0;

  /// Return the associated halo
  virtual iBaseHalo* GetHalo () const = 0;

  /// Get the brightness of a light at a given distance.
  virtual float GetBrightnessAtDistance (float d) const = 0;

  /**
   * Get flags for this light.
   * Supported flags:
   * <ul>
   * <li>#CS_LIGHT_ACTIVEHALO
   * <li>#CS_LIGHT_THINGSHADOWS
   * </ul>
   */
  virtual csFlags& GetFlags () = 0;

  /**
   * Set the light callback. This will call IncRef() on the callback
   * So make sure you call DecRef() to release your own reference.
   */
  virtual void SetLightCallback (iLightCallback* cb) = 0;

  /**
   * Remove a light callback.
   */
  virtual void RemoveLightCallback (iLightCallback* cb) = 0;

  /// Get the number of light callbacks.
  virtual int GetLightCallbackCount () const = 0;
  
  /// Get the specified light callback.
  virtual iLightCallback* GetLightCallback (int idx) const = 0;

  /**
   * Return a number that changes when the light changes (color,
   * or position).
   */
  virtual uint32 GetLightNumber () const = 0;

  /**
   * Add a mesh to this light. This is usually
   * called during Setup() by meshes that are hit by the
   * light.
   */
  virtual void AddAffectedLightingInfo (iLightingInfo* li) = 0; 

  /**
   * Remove a mesh from this light.
   */
  virtual void RemoveAffectedLightingInfo (iLightingInfo* li) = 0; 

  /**
   * For a dynamic light you need to call this to do the actual
   * lighting calculations.
   */
  virtual void Setup () = 0;
};

/**
 * This structure represents a list of lights.
 * <p>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>iSector::GetLights()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>iEngine
 *   </ul>
 */
struct iLightList : public virtual iBase
{
  SCF_INTERFACE(iLightList,2,0,0);
  /// Return the number of lights in this list.
  virtual int GetCount () const = 0;

  /// Return a light by index.
  virtual iLight *Get (int n) const = 0;

  /// Add a light.
  virtual int Add (iLight *obj) = 0;

  /// Remove a light.
  virtual bool Remove (iLight *obj) = 0;

  /// Remove the nth light.
  virtual bool Remove (int n) = 0;

  /// Remove all lights.
  virtual void RemoveAll () = 0;

  /// Find a light and return its index.
  virtual int Find (iLight *obj) const = 0;

  /// Find a light by name.
  virtual iLight *FindByName (const char *Name) const = 0;

  /// Find a light by its ID value (16-byte MD5).
  virtual iLight *FindByID (const char* id) const = 0;
};

SCF_VERSION (iLightingProcessData, 0, 0, 1);

/**
 * The iLightingProcessData interface can be implemented by a mesh
 * object so that it can attach additional information for the lighting
 * process.
 */
struct iLightingProcessData : public iBase
{
  /**
   * Finalize lighting. This function is called by the lighting
   * routines after performing CheckFrustum().
   */
  virtual void FinalizeLighting () = 0;
};

/**
 * The iLightingProcessInfo interface holds information for the lighting
 * system. You can query the userdata from iFrustumView for this interface
 * while in a 'portal' callback. This way you can get specific information
 * from the lighting system for your null-portal.
 */
struct iLightingProcessInfo : public iFrustumViewUserdata
{
  SCF_INTERFACE(iLightingProcessInfo,2,0,0);
  /// Get the light.
  virtual iLight* GetLight () const = 0;

  /// Return true if dynamic.
  virtual bool IsDynamic () const = 0;

  /// Set the current color.
  virtual void SetColor (const csColor& col) = 0;

  /// Get the current color.
  virtual const csColor& GetColor () const = 0;

  /**
   * Attach some userdata to the process info. You can later query
   * for this by doing QueryUserdata() with the correct SCF version
   * number.
   */
  virtual void AttachUserdata (iLightingProcessData* userdata) = 0;

  /**
   * Query for userdata based on SCF type.
   */
  virtual csPtr<iLightingProcessData> QueryUserdata (scfInterfaceID id,
  	int version) = 0;

  /**
   * Finalize lighting. This function is called by the lighting
   * routines after performing CheckFrustum(). It will call
   * FinalizeLighting() on all user datas.
   */
  virtual void FinalizeLighting () = 0;
};

SCF_VERSION (iLightIterator, 0, 1, 0);

/**
 * Iterator to iterate over all static lights in the engine.
 * This iterator assumes there are no fundamental changes
 * in the engine while it is being used.
 * If changes to the engine happen the results are unpredictable.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>iEngine::GetLightIterator()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>Application.
 *   </ul>
 */
struct iLightIterator : public virtual iBase
{
  /// Return true if there are more elements.
  virtual bool HasNext () = 0;

  /// Get light from iterator. Return 0 at end.
  virtual iLight* Next () = 0;

  /// Get the sector for the last fetched light.
  virtual iSector* GetLastSector () = 0;

  /// Restart iterator.
  virtual void Reset () = 0;

};

/** @} */

#endif // __CS_IENGINE_LIGHT_H__

