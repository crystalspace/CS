/*
    Copyright (C) 2003 by Martin Geisse <mgeisse@gmx.net>

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

#ifndef __CS_PARTICLE_H__
#define __CS_PARTICLE_H__

#include "cstool/meshobjtmpl.h"
#include "imesh/partsys.h"

/**
 * flag value to indicate that the system should be deleted when all
 * particles are gone.
 */
const int CS_PARTICLE_AUTODELETE        = 1;

/// enable particle scaling
const int CS_PARTICLE_SCALE             = 2;

/// enable particle rotation
const int CS_PARTICLE_ROTATE            = 4;

/// enable axis alignment (screen alignment otherwise)
const int CS_PARTICLE_AXIS              = 8;

/// use separate scaling values per particle
const int CS_PARTICLE_SEP_SCALE         = 16;

/// use separate rotation values per particle
const int CS_PARTICLE_SEP_ROTATE        = 32;

/// use separate base colors per particle (uniform color otherwise)
const int CS_PARTICLE_SEP_COLOR         = 64;

/// use separate materials per particle (uniform material otherwise)
const int CS_PARTICLE_SEP_MATERIAL      = 128;

/// use separate axes per particle
const int CS_PARTICLE_SEP_AXIS          = 256;

/// use the y axis for alignment instead of x
const int CS_PARTICLE_ALIGN_Y           = 512;

/**
 * This is an abstract implementation of a particle system mesh object. It
 * stores particle information (like position, rotation, scale, etc.) and
 * knows how to draw the particles. It is abstract because it does not know
 * how the particles move. This is done in the Update() method which must
 * be implemented by subclasses. All features like scale and rotation can
 * be disabled, enabled with global values and enabled with per-particle
 * values.
 */
class csNewParticleSystem : public csMeshObject
{
protected:
  /// the mesh factory (should be an empty frame)
  iMeshObjectFactory *Factory;

#ifndef CS_USE_NEW_RENDERER
  /// The vertex buffer.
  csRef<iVertexBuffer> vbuf;
#endif

  /// currently allocated amount of storage for particles
  int StorageCount;

  /// flags
  int ParticleFlags;

  /// number of particles in the system
  int ParticleCount;

  /// position values
  csVector3 *PositionArray;

  /// uniform scaling
  csVector2 Scale;

  /// per-particle scaling
  csVector2 *ScaleArray;

  /// uniform rotation
  float Angle;

  /// per-particle rotation
  float *AngleArray;

  /// uniform base color
  csColor Color;

  /// per-particle base color
  csColor *ColorArray;

  /// mixing mode
  uint MixMode;

  /// uniform material
  csRef<iMaterialWrapper> Material;

  /// per-particle material
  csRef<iMaterialWrapper> *MaterialArray;

  /// uniform axis alignment
  csVector3 Axis;

  /// per-particle axis alignment
  csVector3 *AxisArray;

  /// previous time in the NextFrame() method
  csTicks PrevTime;

  // bounding box
  csBox3 Bounds;

  // clipping flags (passed from DrawTest to Draw)
  int ClipPortal, ClipPlane, ClipZ;

  // use lighting ?
  bool Lighting;

  // lighting data
  csColor *LitColors;

  /**
   * This function re-allocates the data arrays to 'newsize' and copies
   * 'copysize' items from the old arrays. Subclasses can override this
   * method to get notified (when they use their own arrays).
   */
  virtual void Allocate (int newsize, int copysize);

public:
  /// constructor
  csNewParticleSystem (iEngine *, iMeshObjectFactory *, int ParticleFlags);

  /// destructor
  virtual ~csNewParticleSystem ();

  /// grow or shrink the storage area to the specified amount of particles
  void SetCount (int num);

  /// free as much storage area as possible
  void Compact ();

  /// update the bounding box based on particle positions
  void UpdateBounds ();

  /// update the system.
  virtual void Update (csTicks passedTime) = 0;

  /// Returns 0 since there is no factory for a particle system
  virtual iMeshObjectFactory* GetFactory () const;

  /// quick visibility test
  virtual bool DrawTest (iRenderView* rview, iMovable* movable);
  virtual csRenderMesh** GetRenderMeshes (int& n) { n = 0; return 0; }

  /// update lighting info
  void UpdateLighting (iLight**, int, iMovable*);

  /// draw this particle system
  virtual bool Draw (iRenderView*, iMovable*, csZBufMode);

  /// calls Update() with the amount of time passed since the previous call
  virtual void NextFrame (csTicks current_time, const csVector3& pos);

  /// Set the base color (only if a global base color is used)
  virtual bool SetColor (const csColor& color);

  /// Return the base color (only if a global base color is used)
  virtual bool GetColor (csColor& color) const;

  /// Set the material to use (only if a global material is used)
  virtual bool SetMaterialWrapper (iMaterialWrapper* material);

  /// Return the current material (only if a global material is used)
  virtual iMaterialWrapper* GetMaterialWrapper () const;

  /// Return whether this particle system applies lighting
  virtual bool GetLighting () const;

  /// Set whether this particle system applies lighting
  virtual void SetLighting (bool enable);

  virtual void GetObjectBoundingBox (csBox3& bbox, int)
  {
    bbox = Bounds;
  }
};

#endif // __CS_PARTICLE_H__
