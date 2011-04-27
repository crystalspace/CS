/*
    Copyright (C) 2004 by Jorrit Tyberghein

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

#ifndef __CS_IENGINE_LIGHTMGR_H__
#define __CS_IENGINE_LIGHTMGR_H__

/**\file
 * Light manager
 */
/**
 * \addtogroup engine3d_light
 * @{ */
 
#include "csutil/flags.h"
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "csutil/weakref.h"

#include "iengine/light.h"
#include "iutil/array.h"

struct iLight;
struct iMeshWrapper;
struct iSector;
class csFrustum;
class csBox3;
class csReversibleTransform;

/**
 * 
 */
enum csLightQueryFlags
{
  // Flags for returned info
  CS_LIGHTQUERY_GET_LIGHT = 0x0001,

  // Flags for where to get the info from
  CS_LIGHTQUERY_GET_ALL_SECTORS = 0x0010,
  
  CS_LIGHTQUERY_GET_TYPE_STATIC = 0x0100,
  CS_LIGHTQUERY_GET_TYPE_DYNAMIC = 0x0200,
  CS_LIGHTQUERY_GET_TYPE_ALL =
    CS_LIGHTQUERY_GET_TYPE_STATIC | CS_LIGHTQUERY_GET_TYPE_DYNAMIC,

  // Some presets
  CS_LIGHTQUERY_GET_ALL = 0xFFFF  
};

/**
 * 
 */
struct csLightInfluence
{
  iLight* light;
  /**
   * Other useful information in one handy package
   * @{ */
  csLightType type;
  csFlags flags;
  csLightDynamicType dynamicType;
  /// @}
  /**
   * Light intensity (taking into account color and attenuation) at the
   * closest point on the supplied bounding box.
   */
  float perceivedIntensity;
  
  csLightInfluence () : light (0), type (CS_LIGHT_POINTLIGHT),
    dynamicType (CS_LIGHT_DYNAMICTYPE_DYNAMIC), perceivedIntensity (0) {}
  
  inline friend bool operator == (const csLightInfluence& r1, const csLightInfluence& r2) 
  {
    return (r1.light == r2.light);
  }
};

/**
 * 
 */
struct iLightInfluenceArray : public iArrayChangeAll<csLightInfluence>
{ SCF_IARRAYCHANGEALL_INTERFACE(iLightInfluenceArray); };

/**
 * 
 */
struct iLightInfluenceCallback : public virtual iBase
{
  SCF_INTERFACE(iLightInfluenceCallback,1,0,0);
  
  /**
   * 
   */
  virtual void LightInfluence (const csLightInfluence& li) = 0;
};

/**
 * An engine (3D or iso) can implement this interface for the benefit
 * of mesh objects so that they can request lighting information from
 * the engine. The 'logObject' parameter given to these functions is
 * the logical parent that is set in the mesh objects.
 * The engine must attempt to give this information as efficiently as
 * possible. That means only recalculating this lighting information
 * when needed (i.e. light moves, object moves, ...).
 * <p>
 * The engine registers an implementation of this object in the object
 * registry with the "iLightManager" name.
 *
 * Main creators of instances implementing this interface:
 * - 3D engine
 * 
 * Main ways to get pointers to this interface:
 * - csQueryRegistry<iLightManager>
 * 
 * Main users of this interface:
 * - render managers
 */
struct iLightManager : public virtual iBase
{
  SCF_INTERFACE(iLightManager,5,0,1);

  /**
   * Return all 'relevant' light that hit this object. Depending on 
   * implementation in the engine this can simply mean a list of all lights 
   * that affect the object or it can be a list of the N most relevant lights 
   * (with N a parameter set by the user on that object).
   * \param logObject logObject is the mesh wrapper.
   * \param lightArray lightArray is the array to fill with the relevant lights.
   * \param maxLights maxLights is the maximum number of lights that you (as
   * the caller of this function) are interested in. Even with this set the
   * light manager may still return an array containing more lights. You just
   * have to ignore the additional lights then. If you don't want to limit
   * the number of lights you can set maxLights to -1.
   * \param flags flags provided by csLightQueryFlags
   */
  virtual void GetRelevantLights (iMeshWrapper* meshObject, 
    iLightInfluenceArray* lightArray, int maxLights, uint flags = CS_LIGHTQUERY_GET_ALL) = 0;

  /**
   * Return all 'relevant' light that hit this object. Depending on 
   * implementation in the engine this can simply mean a list of all lights 
   * that affect the object or it can be a list of the N most relevant lights 
   * (with N a parameter set by the user on that object).
   * \param logObject logObject is the mesh wrapper.
   * \param lightCallback lightCallback is a callback function to call for 
   * every encountered influencing light source.
   * \param maxLights maxLights is the maximum number of lights that you (as
   * the caller of this function) are interested in. Even with this set the
   * light manager may still return an array containing more lights. You just
   * have to ignore the additional lights then. If you don't want to limit
   * the number of lights you can set maxLights to -1.
   * \param flags flags provided by csLightQueryFlags
   */
  virtual void GetRelevantLights (iMeshWrapper* meshObject, 
    iLightInfluenceCallback* lightCallback, int maxLights, 
    uint flags = CS_LIGHTQUERY_GET_ALL) = 0;

  /**
   * Return all 'relevant' lights for a given sector.
   * \param sector is the sector to check for.
   * \param lightArray lightArray is the array to fill with the relevant lights.
   * \param maxLights maxLights is the maximum number of lights that you (as
   * the caller of this function) are interested in. Even with this set the
   * light manager may still return an array containing more lights. You just
   * have to ignore the additional lights then. If you don't want to limit
   * the number of lights you can set maxLights to -1.
   * \param flags flags provided by csLightQueryFlags
   */
  virtual void GetRelevantLights (iSector* sector, 
    iLightInfluenceArray* lightArray, int maxLights, 
    uint flags = CS_LIGHTQUERY_GET_ALL) = 0;

  /**
   * Return all 'relevant' lights for a given sector.
   * \param sector is the sector to check for.
   * \param lightCallback lightCallback is a callback function to call for 
   * every encountered influencing light source.
   * \param maxLights maxLights is the maximum number of lights that you (as
   * the caller of this function) are interested in. Even with this set the
   * light manager may still return an array containing more lights. You just
   * have to ignore the additional lights then. If you don't want to limit
   * the number of lights you can set maxLights to -1.
   * \param flags flags provided by csLightQueryFlags
   */
  virtual void GetRelevantLights (iSector* sector, 
    iLightInfluenceCallback* lightCallback, int maxLights, 
    uint flags = CS_LIGHTQUERY_GET_ALL) = 0;

  /**
   * Return all 'relevant' lights that intersects a giving bounding box within
   * a specified sector.
   * \param sector is the sector to check for.
   * \param boundingBox is the bounding box to use when querying lights.
   * \param lightArray lightArray is the array to fill with the relevant lights.
   * \param maxLights maxLights is the maximum number of lights that you (as
   * the caller of this function) are interested in. Even with this set the
   * light manager may still return an array containing more lights. You just
   * have to ignore the additional lights then. If you don't want to limit
   * the number of lights you can set maxLights to -1.
   * \param bboxToWorld Optional transformation from bounding box to world
   *   space. 'this' space is bounding box space, 'other' space is world space.
   *   Not specifying the transformation has the same effect as specifying
   *   an identity transform.
   * \param flags flags provided by csLightQueryFlags
   */
  virtual void GetRelevantLights (iSector* sector, const csBox3& boundingBox,
    iLightInfluenceArray* lightArray, int maxLights, 
    const csReversibleTransform* bboxToWorld = 0,
    uint flags = CS_LIGHTQUERY_GET_ALL) = 0;

  /**
   * Return all 'relevant' light/sector influence objects for a given sector.
   * \param sector is the sector to check for.
   * \param boundingBox is the bounding box to use when querying lights.
   * \param lightCallback lightCallback is a callback function to call for 
   * every encountered influencing light source.
   * \param maxLights maxLights is the maximum number of lights that you (as
   * the caller of this function) are interested in. Even with this set the
   * light manager may still return an array containing more lights. You just
   * have to ignore the additional lights then. If you don't want to limit
   * the number of lights you can set maxLights to -1.
   * \param bboxToWorld Optional transformation from bounding box to world
   *   space. 'this' space is bounding box space, 'other' space is world space.
   *   Not specifying the transformation has the same effect as specifying
   *   an identity transform.
   * \param flags flags provided by csLightQueryFlags
   */
  virtual void GetRelevantLights (iSector* sector, const csBox3& boundingBox,
    iLightInfluenceCallback* lightCallback, int maxLights, 
    const csReversibleTransform* bboxToWorld = 0,
    uint flags = CS_LIGHTQUERY_GET_ALL) = 0;

  /**
   * Free a light influence array earlier allocated by GetRelevantLights 
   * \param Array The light influences array returned by GetRelevantLights().
   */
  virtual void FreeInfluenceArray (csLightInfluence* Array) = 0;

  /**
   * Return all 'relevant' light that hit this object. Depending on 
   * implementation in the engine this can simply mean a list of all lights 
   * that affect the object or it can be a list of the N most relevant lights 
   * (with N a parameter set by the user on that object).
   * \param meshObject The mesh wrapper.
   * \param boundingBox The bounding box to be used when querying lights.
   * \param lightArray Returns a pointer to an arry with the influences of the
   *   relevant lights. Must be freed with FreeInfluenceArray() after use.
   * \param numLights The number of lights returned in \a lightArray.
   * \param maxLights The maximum number of lights that you (as
   *   the caller of this function) are interested in.
   * \param flags Flags provided by csLightQueryFlags.
   */
  virtual void GetRelevantLights (iMeshWrapper* meshObject, 
    csLightInfluence*& lightArray, size_t& numLights, 
    size_t maxLights = (size_t)~0,
    uint flags = CS_LIGHTQUERY_GET_ALL) = 0;
  
  /**
   * Return all 'relevant' lights for a given sector.
   * \param sector is the sector to check for.
   * \param boundingBox The bounding box to be used when querying lights.
   * \param lightArray Returns a pointer to an arry with the influences of the
   *   relevant lights. Must be freed with FreeInfluenceArray() after use.
   * \param numLights The number of lights returned in \a lightArray.
   * \param maxLights The maximum number of lights that you (as
   *   the caller of this function) are interested in.
   * \param flags Flags provided by csLightQueryFlags.
   */
  virtual void GetRelevantLights (iSector* sector, 
    csLightInfluence*& lightArray, 
    size_t& numLights, size_t maxLights = (size_t)~0,
    uint flags = CS_LIGHTQUERY_GET_ALL) = 0;
  
  /**
   * Return all 'relevant' lights that intersects a giving bounding box within
   * a specified sector.
   * \param sector is the sector to check for.
   * \param boundingBox The bounding box to be used when querying lights.
   * \param lightArray Returns a pointer to an arry with the influences of the
   *   relevant lights. Must be freed with FreeInfluenceArray() after use.
   * \param numLights The number of lights returned in \a lightArray.
   * \param maxLights The maximum number of lights that you (as
   *   the caller of this function) are interested in.
   * \param bboxToWorld Optional transformation from bounding box to world
   *   space. 'this' space is bounding box space, 'other' space is world space.
   *   Not specifying the transformation has the same effect as specifying
   *   an identity transform.
   * \param flags Flags provided by csLightQueryFlags.
   */
  virtual void GetRelevantLights (iSector* sector, 
    const csBox3& boundingBox, csLightInfluence*& lightArray, 
    size_t& numLights, size_t maxLights = (size_t)~0,
    const csReversibleTransform* bboxToWorld = 0,
    uint flags = CS_LIGHTQUERY_GET_ALL) = 0;
  
  /**
   * Return all 'relevant' lights that intersects a giving bounding box within
   * a specified sector. The returned lights are sorted by the intensity at
   * the closest point on(or in) the box.
   * \param sector is the sector to check for.
   * \param boundingBox The bounding box to be used when querying lights.
   * \param lightArray Returns a pointer to an arry with the influences of the
   *   relevant lights. Must be freed with FreeInfluenceArray() after use.
   * \param numLights The number of lights returned in \a lightArray.
   * \param maxLights The maximum number of lights that you (as
   *   the caller of this function) are interested in.
   * \param bboxToWorld Optional transformation from bounding box to world
   *   space. 'this' space is bounding box space, 'other' space is world space.
   *   Not specifying the transformation has the same effect as specifying
   *   an identity transform.
   * \param flags Flags provided by csLightQueryFlags.
   */
  virtual void GetRelevantLightsSorted (iSector* sector, 
    const csBox3& boundingBox, csLightInfluence*& lightArray, 
    size_t& numLights, size_t maxLights = (size_t)~0,
    const csReversibleTransform* bboxToWorld = 0,
    uint flags = CS_LIGHTQUERY_GET_ALL) = 0;
};

/** @} */


#endif // __CS_IENGINE_LIGHTMGR_H__

