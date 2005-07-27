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

#ifndef __CS_IMESH_SPRITE3D_H__
#define __CS_IMESH_SPRITE3D_H__

#include "csutil/scf.h"

struct iMaterialWrapper;
struct iMeshObject;
struct iMeshObjectFactory;
struct iMeshWrapper;
struct iRenderView;

class csColor;
class csVector2;
class csVector3;
struct csTriangle;

/**
 * Macros for the csSprite3D lighting levels.
 */
enum
{
  CS_SPR_LIGHTING_HQ = 0,
  CS_SPR_LIGHTING_LQ = 1,
  CS_SPR_LIGHTING_FAST = 2,
  CS_SPR_LIGHTING_RANDOM = 3
};

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

SCF_VERSION (iSpriteFrame, 0, 0, 2);

/**
 * A frame for 3D sprite animation.
 */
struct iSpriteFrame : public iBase
{
  /// Set the name.
  virtual void SetName (char const*) = 0;
  /// Get the name.
  virtual char const* GetName () const = 0;
  ///
  virtual int GetAnmIndex () const = 0;
  ///
  virtual int GetTexIndex () const = 0;
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
  virtual int GetFrameCount () = 0;
  /// Get the specified frame.
  virtual iSpriteFrame* GetFrame (int f) = 0;
  /// Get the next frame after the specified one.
  virtual iSpriteFrame* GetNextFrame (int f) = 0;
  /// Get the delay for the specified frame.
  virtual int GetFrameDelay (int f) = 0;
  /// Get the displacement for the specified frame.
  virtual float GetFrameDisplacement (int f) = 0;
  /// Add a frame to this action.
  virtual void AddFrame (iSpriteFrame* frame, int delay, float displacement) = 0;
};

SCF_VERSION (iSpriteSocket, 0, 0, 1);

/**
 * A socket for specifying where sprites can plug into
 * other sprites.
 */
struct iSpriteSocket : public iBase
{
  /// Set the name.
  virtual void SetName (char const*) = 0;
  /// Get the name.
  virtual char const* GetName () const = 0;
  
  /// Set the attached sprite.
  virtual void SetMeshWrapper (iMeshWrapper* mesh) = 0;
  /// Get the attached sprite.
  virtual iMeshWrapper* GetMeshWrapper () const = 0;
  
  /// Set the index of the triangle for the socket.
  virtual void SetTriangleIndex (int tri_index) = 0;
  /// Get the index of the triangle for the socket.
  virtual int GetTriangleIndex () const = 0;
};

SCF_VERSION (iSprite3DFactoryState, 0, 0, 3);

/**
 * This interface describes the API for the 3D sprite factory mesh object.
 */
struct iSprite3DFactoryState : public iBase
{
  /// Set material of sprite.
  virtual void SetMaterialWrapper (iMaterialWrapper* material) = 0;
  /// Get material of sprite.
  virtual iMaterialWrapper* GetMaterialWrapper () const = 0;

  /**
   * Reserve space for the given number of vertices. A vertex includes
   * information about its position, normal and texel. This function will
   * not write any information into the reserved space. <p>
   *
   * Note that this function requires that at least one frame exists in
   * the sprite factory, otherwise this function will fail!
   */
  virtual void AddVertices (int num) = 0;
  /// Return the current number of vertices
  virtual int GetVertexCount () const = 0;

  /// Get a vertex.
  virtual const csVector3& GetVertex (int frame, int vertex) const = 0;
  /// Set a vertex.
  virtual void SetVertex (int frame, int vertex, const csVector3 &Value) = 0;
  /// Get vertex array.
  virtual csVector3* GetVertices (int frame) const = 0;
  /**
   * Set array of vertices. The array is copied. It must contain as many
   * vertices as the vertex count of this sprite.
   */
  virtual void SetVertices (csVector3 const* vert, int frame) = 0;

  /// Get a texel.
  virtual const csVector2& GetTexel (int frame, int vertex) const = 0;
  /// Set a texel.
  virtual void SetTexel (int frame, int vertex, const csVector2 &Value) = 0;
  /// Get array of texels.
  virtual csVector2* GetTexels (int frame) const = 0;
  /**
   * Set array of texels. The array is copied. It must contain as many texels
   * as the vertex count of this sprite.
   */
  virtual void SetTexels (csVector2 const* tex, int frame) = 0;

  /// Get a normal.
  virtual const csVector3& GetNormal (int frame, int vertex) const = 0;
  /// Set a normal.
  virtual void SetNormal (int frame, int vertex, const csVector3 &Value) = 0;
  /// Get normal array.
  virtual csVector3* GetNormals (int frame) const = 0;
  /**
   * Set array of normals. The array is copied. It must contain as many normals
   * as the vertex count of this sprite.
   */
  virtual void SetNormals (csVector3 const* norms, int frame) = 0;

  /**
   * Add a triangle to the normal, texel, and vertex meshes
   * a, b and c are indices to texel vertices
   */
  virtual void AddTriangle (int a, int b, int c) = 0;
  /// Returns the texel indices for triangle 'x'
  virtual csTriangle GetTriangle (int x) const = 0;
  /// Returns the triangles of the texel_mesh
  virtual csTriangle* GetTriangles () const = 0;
  /// Returns the number of triangles in the sprite
  virtual int GetTriangleCount () const = 0;
  /// Set the count of triangles.
  virtual void SetTriangleCount (int count) = 0;
  /// Set array of triangles.  The array is copied.
  virtual void SetTriangles( csTriangle const* trigs, int count) = 0;

  /// Create and add a new frame to the sprite.
  virtual iSpriteFrame* AddFrame () = 0;
  /// Find a named frame.
  virtual iSpriteFrame* FindFrame (const char* name) const = 0;
  /// Query the number of frames.
  virtual int GetFrameCount () const = 0;
  /// Query the frame number f.
  virtual iSpriteFrame* GetFrame (int f) const = 0;

  /// Create and add a new action frameset to the sprite.
  virtual iSpriteAction* AddAction () = 0;
  /// Find a named action.
  virtual iSpriteAction* FindAction (const char* name) const = 0;
  /// Get the first action.
  virtual iSpriteAction* GetFirstAction () const = 0;
  /// Get number of actions in sprite.
  virtual int GetActionCount () const = 0;
  /// Get action number No
  virtual iSpriteAction* GetAction (int No) const = 0;

  /// Create and add a new socket to the sprite.
  virtual iSpriteSocket* AddSocket () = 0;
  /// find a named socket into the sprite.
  virtual iSpriteSocket* FindSocket (const char * name) const = 0;
  /// find a socked based on the sprite attached to it.
  virtual iSpriteSocket* FindSocket (iMeshWrapper *mesh) const = 0;  
  /// Query the number of sockets.
  virtual int GetSocketCount () const = 0;
  /// Query the socket number f.
  virtual iSpriteSocket* GetSocket (int f) const = 0;

  /// Enable/disable tweening.
  virtual void EnableTweening (bool en) = 0;
  /// Query state of tweening.
  virtual bool IsTweeningEnabled () const = 0;
  /// Set lighting quality (one of CS_SPR_LIGHTING_*).
  virtual void SetLightingQuality (int qual) = 0;
  /// Get lighting quality (one of CS_SPR_LIGHTING_*).
  virtual int GetLightingQuality () const = 0;
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
  virtual int GetLightingQualityConfig () const = 0;

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
  virtual int GetLodLevelConfig () const = 0;

  /**
   * Smooth out the gouraud shading by merging the precalculated
   * vertex normals along seams in frame 'frame' based on which
   * vertices are very close in frame 'base'
   */
  virtual void MergeNormals (int base, int frame) = 0;

  /**
   * Smooth out the gouraud shading by merging the precalculated
   * vertex normals along seams in all frames based on which
   * vertices are very close in frame 'base'
   */
  virtual void MergeNormals (int base) = 0;

  /**
   * Smooth out the gouraud shading by merging the precalculated
   * vertex normals along seams in all frames based on which
   * vertices are very close in each frame
   */
  virtual void MergeNormals () = 0;

  /// Set default mix mode for new sprites.
  virtual void SetMixMode (uint mode) = 0;
  /// Get default mix mode for new sprites.
  virtual uint GetMixMode () const = 0;
};

SCF_VERSION (iSprite3DState, 0, 0, 6);

/**
 * This interface describes the API for the 3D sprite mesh object.
 */
struct iSprite3DState : public iBase
{
  /// Set material of sprite.
  virtual void SetMaterialWrapper (iMaterialWrapper* material) = 0;
  /// Get material of sprite.
  virtual iMaterialWrapper* GetMaterialWrapper () const = 0;
  /// Set mix mode.
  virtual void SetMixMode (uint mode) = 0;
  /// Get mix mode.
  virtual uint GetMixMode () const = 0;
  /// Set lighting.
  virtual void SetLighting (bool l) = 0;
  /// Get lighting.
  virtual bool IsLighting () const = 0;

  // @@@ TODO: what about convenience functions to set colors for verts?

  /// Go to a specified frame.
  virtual void SetFrame (int f) = 0;

  /// Get the current frame number.
  virtual int GetCurFrame () const = 0;

  /// Get the number of frames.
  virtual int GetFrameCount () const = 0;

  /**
   * Select an action by name.
   * If 'loop'==false the animation will not loop.
   */
  virtual bool SetAction (const char * name,
  	bool loop = true, float speed = 1) = 0;

  /**
   * Select an action by index.
   * If 'loop'==false the animation will not loop.
   */
  virtual bool SetAction (int index,
  	bool loop = true, float speed = 1) = 0;

  /// Set whether action should run in reverse or not.
  virtual void SetReverseAction(bool reverse) = 0;

  /// Set single-step frame advance flag on actions
  virtual void SetSingleStepAction(bool singlestep) = 0;

  /**
   * This sets an action to run one time, then the
   * sprite reverts to the prior action.
   */
  virtual bool SetOverrideAction(const char *name,
        float speed = 1) = 0;

  /**
   * This sets an action to run one time, then the
   * sprite reverts to the prior action.
   */
  virtual bool SetOverrideAction(int index,
        float speed = 1) = 0;

  /// Propogate set action to all children
  virtual bool PropagateAction (const char *name) = 0;

  /// Get the current action.
  virtual iSpriteAction* GetCurAction () const = 0;

  /// Get whether the current action is reversed or not
  virtual bool GetReverseAction () const = 0;

  /// Enable/disable tweening.
  virtual void EnableTweening (bool en) = 0;
  /// Query state of tweening.
  virtual bool IsTweeningEnabled () const = 0;

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
  virtual int GetLightingQualityConfig () const = 0;

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
  virtual int GetLodLevelConfig () const = 0;

  /**
   * Returns true if lod is enabled, else false.
   */
  virtual bool IsLodEnabled () const = 0;

  /**
   * Set the base color. This color will be added to the vertex
   * colors of the sprite. If no lighting is used then this will
   * be the color.
   */
  virtual void SetBaseColor (const csColor& col) = 0;

  /**
   * Get the base color.
   */
  virtual void GetBaseColor (csColor& col) const = 0;

  /// find a socked based on the sprite attached to it.
  virtual iSpriteSocket* FindSocket (iMeshWrapper *mesh) const = 0;  

  /// find a named socket into the sprite.
  virtual iSpriteSocket* FindSocket (const char * name) const = 0;
};

#endif // __CS_IMESH_SPRITE3D_H__

