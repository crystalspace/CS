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

struct iMaterialWrapper;
struct iSkeleton;
struct iSkeletonState;
struct iMeshObject;
struct iMeshObjectFactory;
struct iRenderView;

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

// @@@ CONFIG TODO: global_lighting_quality
// @@@ CONFIG TODO: global_lod_level

SCF_VERSION (iSpriteFrame, 0, 0, 1);

/**
 * A frame for 3D sprite animation.
 */
struct iSpriteFrame : public iBase
{
  /// Set the name.
  virtual void SetName (char const*) = 0;
  /// Get the name.
  virtual char const* GetName () const = 0;
};

SCF_VERSION (iSpriteAction, 0, 0, 1);

/**
 * An action frameset for 3D sprite animation.
 */
struct iSpriteAction : public iBase
{
  /// Set the name.
  virtual void SetName (char const*) = 0;
  /// Get the name.
  virtual char const* GetName () const = 0;
  /// Get the number of frames in this action.
  virtual int GetNumFrames () = 0;
  /// Get the specified frame.
  virtual iSpriteFrame* GetFrame (int f) = 0;
  /// Get the next frame after the specified one.
  virtual iSpriteFrame* GetNextFrame (int f) = 0;
  /// Get the delay for the specified frame.
  virtual int GetFrameDelay (int f) = 0;
  /// Add a frame to this action.
  virtual void AddFrame (iSpriteFrame* frame, int delay) = 0;
};

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

  /**
   * Add some vertices, normals, and texels. This function
   * only reserves space for this. It will not actually update
   * the information.
   */
  virtual void AddVertices (int num) = 0;

  /// Query the number of texels.
  virtual int GetNumTexels () = 0;
  /// Get a texel.
  virtual csVector2& GetTexel (int frame, int vertex) = 0;
  /// Get array of texels.
  virtual csVector2* GetTexels (int frame) = 0;

  /// Query the number of vertices.
  virtual int GetNumVertices () = 0;
  /// Get a vertex.
  virtual csVector3& GetVertex (int frame, int vertex) = 0;
  /// Get vertex array.
  virtual csVector3* GetVertices (int frame) = 0;

  /// Query the number of normals.
  virtual int GetNumNormals () = 0;
  /// Get a normal.
  virtual csVector3& GetNormal (int frame, int vertex) = 0;
  /// Get normal array.
  virtual csVector3* GetNormals (int frame) = 0;

  /**
   * Add a triangle to the normal, texel, and vertex meshes
   * a, b and c are indices to texel vertices
   */
  virtual void AddTriangle (int a, int b, int c) = 0;
  /// returns the texel indices for triangle 'x'
  virtual csTriangle GetTriangle (int x) = 0;
  /// returns the triangles of the texel_mesh
  virtual csTriangle* GetTriangles () = 0;
  /// returns the number of triangles in the sprite
  virtual int GetNumTriangles () = 0;

  /// Create and add a new frame to the sprite.
  virtual iSpriteFrame* AddFrame () = 0;
  /// Find a named frame.
  virtual iSpriteFrame* FindFrame (const char* name) = 0;
  /// Query the number of frames.
  virtual int GetNumFrames () = 0;
  /// Query the frame number f.
  virtual iSpriteFrame* GetFrame (int f) = 0;

  /// Create and add a new action frameset to the sprite.
  virtual iSpriteAction* AddAction () = 0;
  /// Find a named action.
  virtual iSpriteAction* FindAction (const char* name) = 0;
  /// Get the first action.
  virtual iSpriteAction* GetFirstAction () = 0;
  /// Get number of actions in sprite.
  virtual int GetNumActions () = 0;
  /// Get action number No
  virtual iSpriteAction* GetAction (int No) = 0;

  /// Enable skeletal animation for this factory.
  virtual void EnableSkeletalAnimation () = 0;
  /**
   * Get the skeleton. Will only be valid if skeletal animation
   * has been enabled with EnableSkeletalAnimation(). Otherwise
   * it will return NULL.
   */
  virtual iSkeleton* GetSkeleton () = 0;

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

  // @@@ TODO: dynamic light support? Probably obsolete.
  // @@@ TODO: global LOD level.
  // @@@ TODO: static int global_lighting_quality;
  // @@@ TODO: query actions.
  // @@@ TODO: what about conveniance functions to set colors for verts?
  // @@@ TODO: what about GetRadius()? Add to csMeshWrapper?

  /**
   * Get the reference to the factory for this sprite.
   */
  virtual iMeshObjectFactory* GetFactory () = 0;

  /**
   * Get the skeleton state. Will only be valid if skeletal animation
   * has been enabled for the factory that this sprite was created from.
   * Otherwise it will return NULL.
   */
  virtual iSkeletonState* GetSkeletonState () = 0;

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

