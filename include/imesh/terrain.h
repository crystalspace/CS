/*
    Copyright (C) 2003 by Jorrit Tyberghein, Daniel Duhprey

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

#ifndef __CS_IMESH_TERRAIN_H__
#define __CS_IMESH_TERRAIN_H__

/**\file
 * Terrain mesh object
 */ 

#include "csutil/scf.h"

#include "csutil/array.h"

/**\addtogroup meshplugins
 * @{ */

struct iImage;
struct iMaterialWrapper;
struct iMovable;
struct iTerraFormer;

class csBox2;
class csTransform;

SCF_VERSION (iTerrainObjectState, 0, 1, 0);

/**
 * This will override the settings for material in the parent
 */
struct iTerrainObjectState : public iBase
{
  /**
   * Set/Get the material palette, this is used to specify materials on a 
   * bytemap representing the material makeup of the terrain
   */
  virtual bool SetMaterialPalette (const csArray<iMaterialWrapper*>& pal) = 0;
  virtual const csArray<iMaterialWrapper*>& GetMaterialPalette () const = 0;

  /**
   * In short, the materialmap paints the palette onto the terrain like
   * an indexed image format paints a color palette onto the screen
   * It sets the materials per pixel in the material map.  If x and y are not
   * equal to the heightmap the materialmap will scale accordingly.  If they
   * are equal than the material map will map a single material to a single
   * high level quad in the final terrain.
   * Note x and y must be 2^n and usually they must be equal
   */
  virtual bool SetMaterialMap (const csArray<char>& data, int x, int y) = 0;
  virtual bool SetMaterialMap (iImage* map) = 0;

  /**
   * In short, the materialmap paints the palette onto the terrain like
   * an indexed image format paints a color palette onto the screen
   * It sets the materials per pixel in the material map.  If x and y are not
   * equal to the heightmap the materialmap will scale accordingly.  If they
   * are equal than the material map will map a single material to a single
   * high level quad in the final terrain.
   * Note x and y must be 2^n and usually they must be equal
   * This version expects an array of alpha maps (typically gray scale images).
   * For every material in the palette except for the last one(!) there will be
   * an alpha map so this array should have one element less compared to the
   * palette. The alpha map for the last palette entry will be calculated so
   * that the alpha values add up to 100%.
   */
  virtual bool SetMaterialAlphaMaps (const csArray<csArray<char> >& data,
  	int x, int y) = 0;
  virtual bool SetMaterialAlphaMaps (const csArray<iImage*>& maps) = 0;

  /**
   * Set a LOD parameter.
   * 
   * The following parameters can be used:
   * \li <i>"lod distance"</i> - The distance at which splatting is no longer 
   *  in effect and the base texture is the only layer.
   * \li <i>"error tolerance"</i>  - The screenspace error tolerance in 
   *  numbers of pixels. Error will be less than the given tolerance so 
   *  setting the tolerance to values less than 1 is worthless.
   * 
   * \return Whether the value was accepted by the terrain object.
   */
  virtual bool SetLODValue (const char* parameter, float value) = 0;
  /**
   * Get a LOD parameter.
   */
  virtual float GetLODValue (const char* parameter) const = 0;

  /**
   * Save/Restore preprocessing information, the algorithm will 
   * do some preprocessing based on the material and height information
   * this allows the process to be saved out to a file and cached 
   * for later reuse (maybe this should be the caching system)
   * In some cases it may actually memorymap this file
   */
  virtual bool SaveState (const char *filename) = 0;
  virtual bool RestoreState (const char *filename) = 0;

  /// Detects collision with a specific transform
  virtual int CollisionDetect (iMovable *m, csTransform *p) = 0;

  /**
   * Enable or disable the use of static lighting.
   */
  virtual void SetStaticLighting (bool enable) = 0;
  /**
   * Retrieve whether static lighting is enabled.
   */
  virtual bool GetStaticLighting () = 0;

  /**
   * Enable or disable shadow casting by this terrain mesh.
   */
  virtual void SetCastShadows (bool enable) = 0;
  /**
   * Retrieve whether shadow casting is enabled.
   */
  virtual bool GetCastShadows () = 0;
};

SCF_VERSION (iTerrainFactoryState, 0, 0, 1);

/**
 * Allows the setting of a set of generic terrain parameters outside
 * any specific algorithm.  It is up to the algorithm to determine the
 * best use of the information provided in the interface.
 */
struct iTerrainFactoryState : public iBase
{

  /**
   * The terraformer defines the height, scale and other properties
   * related to the formation and structure of the terrain.
   * The terrain rendering plugin will sample from this data model
   * based on its algorithm.  The terraformer can be shared between
   * plugins, especially plugins which use the information for 
   * vegatation rendering.
   */
  virtual void SetTerraFormer (iTerraFormer *form) = 0;
  virtual iTerraFormer *GetTerraFormer () = 0;

  /**
   * This specifies the max region the renderer will sample from.  This 
   * is more of a hint to the renderer as it may try to optimally sample
   * only from regions near the camera. 
   */
  virtual void SetSamplerRegion (const csBox2& region) = 0;
  virtual const csBox2& GetSamplerRegion () = 0;

  /**
   * Save/Restore preprocessing information, the algorithm will 
   * do some preprocessing based on the material and height information
   * this allows the process to be saved out to a file and cached 
   * for later reuse (maybe this should be the caching system)
   * In some cases it may actually memorymap this file
   */
  virtual bool SaveState (const char *filename) = 0;
  virtual bool RestoreState (const char *filename) = 0;
};

/** @} */

#endif // __CS_IMESH_TERRAIN_H__
