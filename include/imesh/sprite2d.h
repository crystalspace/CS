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

#ifndef __CS_IMESH_SPRITE2D_H__
#define __CS_IMESH_SPRITE2D_H__

/**\file
 * 2D sprite (billboard) mesh object
 */ 

#include "csutil/scf.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/cscolor.h"
#include "csgeom/vector2.h"

/**\addtogroup meshplugins
 * @{ */

struct iMaterialWrapper;

/// A single 2D Sprite vertex.
struct csSprite2DVertex
{
  csVector2 pos;
  csColor color_init;
  csColor color;
  float u, v;
};

typedef csDirtyAccessArray<csSprite2DVertex> csColoredVertices;

SCF_VERSION (iSprite2DUVAnimationFrame, 0, 0, 1);

/**
 * This is a single frame in a UV animation. So its not much more than a set of
 * (u.v) coordinates and a duration time.
 */
struct iSprite2DUVAnimationFrame : public iBase
{
  /**
   * Give this frame a name.
   */
  virtual void SetName (const char *name) = 0;

  /**
   * Return the name of this frame.
   */
  virtual const char *GetName () const = 0;

  /**
   * Get the u,v coordinates of the idx'th vertex
   */
  virtual csVector2 &GetUVCoo (int idx) = 0;

  /**
   * Get all u,v coordinates
   */
  virtual const csVector2 *GetUVCoo () = 0;

  /**
   * Get the number of (u,v) coordinates
   */
  virtual int GetUVCount () = 0;

  /**
   * Set the (u,v) coordinate of idx'th coordinate. Set idx to -1 to append it.
   */
  virtual void SetUV (int idx, float u, float v) = 0;

  /**
   * Set all (u,v) coordinates and the name and duration
   */
  virtual void SetFrameData (const char *name, int duration, int num,
  	float *uv) = 0;

  /**
   * Remove the idx'th coordinate.
   */
  virtual void RemoveUV (int idx) = 0;

  /**
   * Return the duration of this frame.
   */
  virtual int GetDuration () = 0;

  /**
   * Set the duration of this frame.
   */
  virtual void SetDuration (int duration) = 0;
};

SCF_VERSION (iSprite2DUVAnimation, 0, 0, 1);

/**
 * The animation works by having all frames of an animation sequence
 * in a texture at different (u,v) locations, hence the name.
 * So it is basically a set of (u,v) coordinates plus a duration number.
 * for every frame.
 */
struct iSprite2DUVAnimation : public iBase
{
  /**
   * Give this sequence a name.
   */
  virtual void SetName (const char *name) = 0;

  /**
   * return the name of this sequence.
   */
  virtual const char *GetName () const = 0;

  /**
   * Retrieve the number of frames in this animation.
   */
  virtual int GetFrameCount () = 0;

  /**
   * Get the idx'th frame in the animation.
   * Set idx to -1 to get the current to be played.
   */
  virtual iSprite2DUVAnimationFrame *GetFrame (int idx) = 0;

  /**
   * Get the frame name in the animation.
   */
  virtual iSprite2DUVAnimationFrame *GetFrame (const char *name) = 0;

  /**
   * Create a new frame that will be inserted before the idx'th frame.
   * Set `idx' to -1 to append the frame to the sequence.
   */
  virtual iSprite2DUVAnimationFrame *CreateFrame (int idx) = 0;

  /**
   * Move the frame'th frame before the idx'th frame. Set idx to -1
   * to move the frame to the end of the sequence.
   */
  virtual void MoveFrame (int frame, int idx) = 0;

  /**
   * Remove the idx'th from the animation
   */
  virtual void RemoveFrame (int idx) = 0;

};

SCF_VERSION (iSprite2DFactoryState, 0, 0, 1);

/**
 * This interface describes the API for the sprite factory mesh object.
 */
struct iSprite2DFactoryState : public iBase
{
  /// Set material of sprite.
  virtual void SetMaterialWrapper (iMaterialWrapper* material) = 0;
  /// Get material of sprite.
  virtual iMaterialWrapper* GetMaterialWrapper () const = 0;
  /// Set mix mode.
  virtual void SetMixMode (uint mode) = 0;
  /// Get mix mode.
  virtual uint GetMixMode () const = 0;

  /**
   * Set true if this sprite needs lighting (default).
   * Otherwise the given colors are used.
   * If lighting is disabled then the color_init array
   * is copied to the color array.
   */
  virtual void SetLighting (bool l) = 0;

  /// Return the value of the lighting flag.
  virtual bool HasLighting () const = 0;

  /**
   * Get the number of UVAnimations.
   */
  virtual int GetUVAnimationCount () const = 0;

  /**
   * Create a new UV animation
   */
  virtual iSprite2DUVAnimation *CreateUVAnimation () = 0;

  /**
   * Remove an UV animation
   */
  virtual void RemoveUVAnimation (iSprite2DUVAnimation *anim) = 0;

  /**
   * Get a specific UV animation by name. Returns 0 if not found.
   */
  virtual iSprite2DUVAnimation *GetUVAnimation (const char *name) const = 0;

  /**
   * Get a specific UV animation by index. Returns 0 if not found.
   */
  virtual iSprite2DUVAnimation *GetUVAnimation (int idx) const = 0;

};

SCF_VERSION (iSprite2DState, 0, 0, 1);

/**
 * This interface describes the API for the sprite factory mesh object.
 * iSprite2DState inherits from iSprite2DFactoryState.
 */
struct iSprite2DState : public iSprite2DFactoryState
{
  /// Get the vertex array.
  virtual csColoredVertices& GetVertices () = 0;
  /**
   * Set vertices to form a regular n-polygon around (0,0),
   * optionally also set u,v to corresponding coordinates in a texture.
   * Large n approximates a circle with radius 1. n must be > 2.
   */
  virtual void CreateRegularVertices (int n, bool setuv) = 0;

  /**
   * Select an UV animation to play. Set name to 0 to select
   * no animation to show.
   * Style:
   * 0   .. use the time values supplied in the frames
   * > 0 .. every `style' millisecond skip to next frame
   * < 0 .. every -1*`style'th frame skip to next frame
   * Loop:
   * true  .. after last frame animations starts over from the beginning
   * false .. after last frame the normal texture is shown
   */
  virtual void SetUVAnimation (const char *name, int style, bool loop) = 0;

  /**
   * Get a specific UV animation by name. Returns 0 if not found.
   */
  virtual iSprite2DUVAnimation *GetUVAnimation (const char *name) const = 0;

  /**
   * Get a specific UV animation by index. Returns 0 if not found.
   */
  virtual iSprite2DUVAnimation *GetUVAnimation (int idx) const = 0;

  /**
   * Get a specific UV animation by index. Returns 0 if not found.
   */
  virtual iSprite2DUVAnimation *GetUVAnimation (int idx, int &style,
                                                bool &loop) const = 0;

  /**
   * Stop the animation and show the idx'th frame.
   * Set idx to -1 to stop it at its current position.
   */
  virtual void StopUVAnimation (int idx) = 0;

  /**
   * Play the animation starting from the idx'th frame.
   * Set idx to -1 to start it fro its current position.
   * Style:
   * 0   .. use the time values supplied in the frames
   * > 0 .. every `style' millisecond skip to next frame
   * < 0 .. every -1*`style'th frame skip to next frame
   * Loop:
   * true  .. after last frame animations starts over from the beginning
   * false .. after last frame the normal texture is shown
   */
  virtual void PlayUVAnimation (int idx, int style, bool loop) = 0;
};

/** @} */

#endif // __CS_IMESH_SPRITE2D_H__

