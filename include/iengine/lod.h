/*
    Copyright (C) 2001 by Jorrit Tyberghein and Richard D. Shank

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

#ifndef __IENGINE_LOD_H__
#define __IENGINE_LOD_H__

#include "csutil/scf.h"

#define CS_LOD_TRIANGLE_REDUCTION	1
#define CS_LOD_LIGHT_QUALITY		2
#define CS_LOD_LIGHT_QUANTITY		4
#define CS_LOD_FRAME_ANIMATION		8
#define CS_LOD_SKELETAL_ANIMATION	16
#define CS_LOD_TEXTURE_DETAIL		32
#define CS_LOD_TEXTURE_SIZE		64
#define CS_LOD_DISTANCE_REDUCTION	128
#define CS_LOD_ALL			(~0)

SCF_VERSION (iLODControl, 0, 0, 1);

/**
 * The iLODControl interface represents an object that has controllable
 * LOD features. In the future the LOD manager will be able to work with
 * this.
 */
struct iLODControl : public iBase
{
  /**
   * Get a mask with the currently enabled features for this mesh
   * object. For LOD purposes some features may be disabled by the engine.
   * The values in this mask are combinations of the CS_LOD_* flags.
   */
  virtual uint32 GetLODFeatures () const = 0;

  /**
   * Set the features you want this mesh object to support. Features can
   * be disabled by the engine for LOD purposes.
   * The values in this mask are combinations of the CS_LOD_* flags.
   */
  virtual void SetLODFeatures (uint32 mask, uint32 value) = 0;

  /**
   * Set the LOD level of this mesh object (for polygon count). A value
   * of 1 (default) means that the mesh object will use full detail.
   * A value of 0 means that the mesh object will use lowest possible detail
   * while still being useful (i.e. a value of 0 should not result in no
   * triangles to render).
   */
  virtual void SetLOD (float lod) = 0;

  /**
   * Get the current LOD settng for this mesh object (between 0 and 1).
   */
  virtual float GetLOD () const = 0;

  /**
   * Get a rough estimate of the number of polygons for a given LOD value
   * (between 0 and 1, similar to the value used by SetLOD()).
   * Note that a mesh object that doesn't support LOD should always return
   * the same number of polygons.
   */
  virtual int GetLODPolygonCount (float lod) const = 0;

  /**
   * Get a mask with the available LOD features for this mesh object.
   * The values in this mask are combinations of the CS_LOD_* flags.
   */
  virtual uint32 GetAvailableLODFeatures () const = 0;

  /**
   * Get a mask with the available LOD distance reduction features for
   * this mesh object. The values in this mask are combinations of the
   * CS_LOD_* flags.
   */
  virtual uint32 GetAvailableDistanceFeatures () const = 0;

  /**
   * Get a mask with the currently enabled distance reduction features for
   * this mesh object. Some features may be disabled by the engine.
   * The values in this mask are combinations of the CS_LOD_* flags.
   */
  virtual uint32 GetDistanceReduction () const = 0;

  /**
   * Set the features you want supported in distance reduction.
   * The values in this mask are combinations of the CS_LOD_* flags.
   */
  virtual void SetDistanceReduction (uint32 mask, uint32 value) = 0;

  /**
   * Set the priority level for this object can be from 0 to 255
   * 0 being the highest priority, 255 being the lowest. It is 0 by default
   */
  virtual void SetLODPriority (uint16 group) = 0;

  /**
   * Get the features priority level for this object
   */
  virtual uint16 GetLODPriority () const = 0;

  virtual void SetMinLODThreshold (float level, bool turnOff) = 0;
};

#endif // __IENGINE_LOD_H__

