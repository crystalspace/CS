/*
    Copyright (C) 2000 by Jorrit Tyberghein
  
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

#ifndef __IMESHSPR3D_H__
#define __IMESHSPR3D_H__

#include "csutil/scf.h"
#include "csutil/garray.h"
#include "csutil/cscolor.h"

/**
 * Macros for the csSprite3D lighting levels.
 */
#define CS_SPR_LIGHTING_HQ 0
#define CS_SPR_LIGHTING_LQ 1
#define CS_SPR_LIGHTING_FAST 2
#define CS_SPR_LIGHTING_RANDOM 3

/**
 * Use the global value for determining which lighting level is used by the 
 * sprite.
 */
#define CS_SPR_LIGHT_GLOBAL 0

/**
 * Use the sprites template lighting quality value for determining which 
 * lighting level is used by the sprite.
 */
#define CS_SPR_LIGHT_TEMPLATE 1

/**
 * Use the lighting quality value local to the sprite for determining which 
 * lighting level is used by the sprite.
 */
#define CS_SPR_LIGHT_LOCAL 2

/**
 * Use the global value for determining if LOD is used by the 
 * sprite, and what level it should be used at.
 */
#define CS_SPR_LOD_GLOBAL 0

/**
 * Use the sprites template lod value.
 */
#define CS_SPR_LOD_TEMPLATE 1

/**
 * Use the LOD value local to the sprite.
 */
#define CS_SPR_LOD_LOCAL 2

struct iMaterialWrapper;

// @@@ CONFIG TODO: global_lighting_quality
// @@@ CONFIG TODO: global_lod_level

SCF_VERSION (iSprite3DFactoryState, 0, 0, 1);

/**
 * This interface describes the API for the 3D sprite factory mesh object.
 */
struct iSprite3DFactoryState : public iBase
{
  /// Set material of sprite.
  virtual void SetMaterialWrapper (iMaterialWrapper* material) = 0;
  /// Get material of sprite.
  virtual iMaterialWrapper* GetMaterialWrapper () = 0;

  // @@@ TODO: access the vertices
  // @@@ TODO: access the triangles
  // @@@ TODO: access the normals
  // @@@ TODO: access the texcoords
  // @@@ TODO: access the frames
  // @@@ TODO: access the actions
  // @@@ TODO: access the skeleton
  // @@@ TODO: something to get the triangle mesh??? Do we need that?
  // @@@ TODO: what about HardTransform()? Needs to be extension
  //  	       to csMeshWrapper.

  /// Enable/disable tweening.
  virtual void EnableTweening (bool en) = 0;
  /// Query state of tweening.
  virtual bool IsTweeningEnabled () = 0;
  /// Set lighting quality (one of CS_SPR_LIGHTING_*).
  virtual void SetLightingQuality (int qual) = 0;
  /// Get lighting quality (one of CS_SPR_LIGHTING_*).
  virtual int GetLightingQuality () = 0;
  /**
   * Sets which lighting config variable that all new sprites created 
   * from this template will use.
   * The options are:
   * <ul>
   * <li>CS_SPR_LIGHT_GLOBAL (default)
   * <li>CS_SPR_LIGHT_TEMPLATE
   * <li>CS_SPR_LIGHT_LOCAL
   * </ul>
   */
  virtual void SetLightingQualityConfig (int qual) = 0;
  /// Get the lighting quality config.
  virtual int GetLightingQualityConfig () = 0;

  /// Returns the lod_level for this template.
  virtual float GetLodLevel () = 0;

  /// Sets the lod level for this template.  See CS_SPR_LOD_* defs.
  virtual void SetLodLevel (float level) = 0;

  /**
   * Sets which lod config variable that all new sprites created 
   * from this template will use.
   * The options are:
   * <ul>
   * <li>CS_SPR_LOD_GLOBAL (default)
   * <li>CS_SPR_LOD_TEMPLATE
   * <li>CS_SPR_LOD_LOCAL
   * </ul>
   */
  virtual void SetLodLevelConfig (int config_flag) = 0;

  /// Returns what this template is using for determining the lod quality.
  virtual int GetLodLevelConfig () = 0;
};

SCF_VERSION (iSprite3DState, 0, 0, 1);

/**
 * This interface describes the API for the 2D sprite mesh object.
 */
struct iSprite3DState : public iBase
{
  /// Set material of sprite.
  virtual void SetMaterialWrapper (iMaterialWrapper* material) = 0;
  /// Get material of sprite.
  virtual iMaterialWrapper* GetMaterialWrapper () = 0;
  /// Set mix mode.
  virtual void SetMixMode (UInt mode) = 0;
  /// Get mix mode.
  virtual UInt GetMixMode () = 0;

  // @@@ TODO: what about the two types of callbacks?
  // @@@ TODO: dynamic light support? Probably obsolete.
  // @@@ TODO: global LOD level.
  // @@@ TODO: static int global_lighting_quality;
  // @@@ TODO: query skeleton state and allow to modify it.
  // @@@ TODO: query actions.
  // @@@ TODO: what about conveniance functions to set colors for verts?
  // @@@ TODO: what about GetRadius()? Add to csMeshWrapper?

  /// Go to a specified frame.
  virtual void SetFrame (int f) = 0;

  /// Get the current frame number.
  virtual int GetCurFrame () = 0;

  /// Get the number of frames.
  virtual int GetNumFrames () = 0;

  /// Select an action.
  virtual bool SetAction (const char * name) = 0;

  /// Enable/disable tweening.
  virtual void EnableTweening (bool en) = 0;
  /// Query state of tweening.
  virtual bool IsTweeningEnabled () = 0;

  /// Unset the texture (i.e. use the one from the factory).
  virtual void UnsetTexture () = 0;

  /**
   * Returns the lighting quality level used by this sprite.
   * See SPR_LIGHTING_* macros defined in this header for the different types
   * of lighting.
   */ 
  virtual int GetLightingQuality () = 0;

  /**
   * Sets the local lighting quality for this sprite.  NOTE: you must use
   * SetLightingQualityConfig (CS_SPR_LIGHT_LOCAL) for the sprite to use this.
   */
  virtual void SetLocalLightingQuality (int lighting_quality) = 0;

  /**
   * Sets which lighting config variable this sprite will use.
   * The options are:
   * <ul>
   * <li>CS_SPR_LIGHT_GLOBAL (default)
   * <li>CS_SPR_LIGHT_TEMPLATE
   * <li>CS_SPR_LIGHT_LOCAL
   * </ul>
   */
  virtual void SetLightingQualityConfig (int config_flag) = 0;

  /**
   * Returns what this sprite is using for determining the lighting quality.
   */
  virtual int GetLightingQualityConfig () = 0;

  /**
   * Returns the lod level used by this sprite.
   */ 
  virtual float GetLodLevel () = 0;

  /**
   * Sets the local lod level for this sprite.  NOTE: you must use
   * SetLodLevelConfig (CS_SPR_LOD_LOCAL) for the sprite to use this.
   */
  virtual void SetLocalLodLevel (float lod_level) = 0;

  /**
   * Sets which lighting config variable this sprite will use.
   * The options are:
   * <ul>
   *   <li>CS_SPR_LOD_GLOBAL (default)
   *   <li>CS_SPR_LOD_TEMPLATE
   *   <li>CS_SPR_LOD_LOCAL
   * </ul>
   */
  virtual void SetLodLevelConfig (int config_flag) = 0;
   
  /**
   * Returns what this sprite is using for determining the lighting quality.
   */
  virtual int GetLodLevelConfig () = 0;

  /**
   * Returns true if lod is enabled, else false.
   */
  virtual bool IsLodEnabled () = 0;
};

#endif

