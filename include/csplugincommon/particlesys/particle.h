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

#ifndef __CS_CSPLUGINCOMMON_PARTICLESYS_PARTICLE_H__
#define __CS_CSPLUGINCOMMON_PARTICLESYS_PARTICLE_H__

/**\file
 */

#include "csextern.h"
#include "cstool/meshobjtmpl.h"
#include "cstool/rendermeshholder.h"
#include "imesh/partsys.h"
#include "ivideo/rendermesh.h"
#include "iengine/lightmgr.h"
#include "csgfx/shadervarcontext.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/weakref.h"

/**\addtogroup plugincommon
 * @{ */

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
class CS_CSPLUGINCOMMON_EXPORT csNewParticleSystem : public csMeshObject
{
protected:
  /// the mesh factory (should be an empty frame)
  iMeshObjectFactory *Factory;
  csRef<iLightManager> light_mgr;

  bool initialized;

  csRenderMeshHolderSingle rmHolder;
  csRef<csRenderBufferHolder> bufferHolder;

  int VertexCount;
  int TriangleCount;
  csVector3* vertices;
  csRef<iRenderBuffer> vertex_buffer;
  csRef<iRenderBuffer> texel_buffer;
  csRef<iRenderBuffer> normal_buffer;
  csRef<iRenderBuffer> color_buffer;
  csRef<iRenderBuffer> index_buffer;

  csWeakRef<iGraphics3D> g3d;

  csTriangle* triangles;
  csVector2* texels;
  csColor* colors;

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

  /// uniform rotation
  float Angle;

  /// uniform base color
  csColor Color;

  /// mixing mode
  uint MixMode;

  /// uniform material
  csRef<iMaterialWrapper> Material;

  /// uniform axis alignment
  csVector3 Axis;

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

  /// Self destruct and when.
  bool self_destruct;
  csTicks time_to_live; // msec

  /// Color change
  bool change_color; csColor colorpersecond;
  /// Size change
  bool change_size; float scalepersecond;
  /// Alpha change
  bool change_alpha; float alphapersecond; float alpha_now;
  /// Rotate particles, angle in radians.
  bool change_rotation; float anglepersecond;

  /**
   * This function re-allocates the data arrays to 'newsize' and copies
   * 'copysize' items from the old arrays. Subclasses can override this
   * method to get notified (when they use their own arrays).
   */
  virtual void Allocate (int newsize, int copysize);

  virtual void SetupObject ();

  /**
   * Setup particles in the given tables right before they are drawn.
   */
  void SetupParticles (const csReversibleTransform&, csVector3* vertices);

public:
  /// constructor
  csNewParticleSystem (iEngine *, iMeshObjectFactory *, int ParticleFlags);

  /// destructor
  virtual ~csNewParticleSystem ();

  SCF_DECLARE_IBASE_EXT (csMeshObject);

  /// grow or shrink the storage area to the specified amount of particles
  void SetCount (int num);

  /// free as much storage area as possible
  void Compact ();

  /// update the bounding box based on particle positions
  void UpdateBounds ();

  /// update the system.
  virtual void Update (csTicks passedTime);

  /// Returns 0 since there is no factory for a particle system
  virtual iMeshObjectFactory* GetFactory () const;

  virtual csRenderMesh** GetRenderMeshes (int& n, iRenderView* rview, 
    iMovable* movable, uint32 frustum_mask);

  /// update lighting info
  void UpdateLighting (const csArray<iLight*>&, iMovable*);

  /// calls Update() with the amount of time passed since the previous call
  virtual void NextFrame (csTicks current_time, const csVector3& pos);

  /// Set the base color
  virtual bool SetColor (const csColor& color);

  /// Add to the current color
  virtual void AddColor (const csColor& color);

  /// Return the base color
  virtual const csColor& GetColor () const;

  /// Set the material to use
  virtual bool SetMaterialWrapper (iMaterialWrapper* material);

  /// Return the current material
  virtual iMaterialWrapper* GetMaterialWrapper () const;

  /// Return whether this particle system applies lighting
  virtual bool GetLighting () const;

  /// Set whether this particle system applies lighting
  virtual void SetLighting (bool enable);

  virtual void GetObjectBoundingBox (csBox3& bbox)
  {
    SetupObject ();
    bbox = Bounds;
  }
  virtual void SetObjectBoundingBox (const csBox3& bbox)
  {
    Bounds = bbox;
    scfiObjectModel.ShapeChanged ();
  }

  /// Set selfdestruct mode on, and msec to live.
  inline void SetSelfDestruct (csTicks t)
  { self_destruct=true; time_to_live = t; };
  /// system will no longer self destruct
  inline void UnSetSelfDestruct () { self_destruct=false; }
  /// returns whether the system will self destruct
  inline bool GetSelfDestruct () const { return self_destruct; }
  /// if the system will self destruct, returns the time to live in msec.
  inline csTicks GetTimeToLive () const { return time_to_live; }

  /// Change color of all particles, by col per second.
  inline void SetChangeColor(const csColor& col)
  {change_color = true; colorpersecond = col;}
  /// Stop change of color
  inline void UnsetChangeColor() {change_color=false;}
  /// see if change color is enabled, and get a copy if so.
  inline bool GetChangeColor (csColor& col) const
  { if(!change_color) return false; col = colorpersecond; return true; }

  /// Change size of all particles, by factor per second.
  inline void SetChangeSize(float factor)
  {change_size = true; scalepersecond = factor;}
  /// Stop change of size
  inline void UnsetChangeSize() {change_size=false;}
  /// see if change size is enabled, and get the value if so.
  inline bool GetChangeSize (float& factor) const
  { if(!change_size) return false; factor = scalepersecond; return true; }

  /// Set the alpha of particles.
  inline void SetAlpha(float alpha)
  {alpha_now = alpha; MixMode = CS_FX_SETALPHA (alpha); }
  /// Get the probable alpha of the particles
  inline float GetAlpha() const {return alpha_now;}
  /// Change alpha of all particles, by factor per second.
  inline void SetChangeAlpha(float factor)
  {change_alpha = true; alphapersecond = factor;}
  /// Stop change of alpha
  inline void UnsetChangeAlpha() {change_alpha=false;}
  /// see if change alpha is enabled, and get the value if so.
  inline bool GetChangeAlpha (float& factor) const
  { if(!change_alpha) return false; factor = alphapersecond; return true; }

  /// Change rotation of all particles, by angle in radians per second.
  inline void SetChangeRotation(float angle)
  {
    change_rotation = true;
    anglepersecond = angle;
    // @@@??? Ok?
    ParticleFlags |= CS_PARTICLE_ROTATE;
  }
  /// Stop change of rotation
  inline void UnsetChangeRotation() { change_rotation=false; }
  /// see if change rotation is enabled, and get the angle if so.
  inline bool GetChangeRotation (float& angle) const
  { if(!change_rotation) return false; angle = anglepersecond; return true; }


  struct eiParticleState : public iParticleState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csNewParticleSystem);
    virtual void SetMaterialWrapper (iMaterialWrapper* material)
    {
      scfParent->SetMaterialWrapper (material);
    }
    virtual iMaterialWrapper* GetMaterialWrapper () const
    { 
      return scfParent->GetMaterialWrapper ();
    }
    virtual void SetMixMode (uint mode)
    {
      scfParent->MixMode = mode;
    }
    virtual uint GetMixMode () const
    {
      return scfParent->MixMode;
    }
    virtual void SetColor (const csColor& color)
    {
      scfParent->SetColor (color);
    }
    virtual const csColor& GetColor () const
    {
      return scfParent->GetColor ();
    }
    virtual void SetAlpha(float alpha) { scfParent->SetAlpha(alpha); }
    virtual float GetAlpha() const { return scfParent->GetAlpha (); }
    virtual void SetChangeColor (const csColor& color)
    {
      scfParent->SetChangeColor (color);
    }
    virtual void UnsetChangeColor ()
    {
      scfParent->UnsetChangeColor ();
    }
    virtual bool GetChangeColor (csColor& col) const
    {
      return scfParent->GetChangeColor(col); }
    virtual void SetChangeSize (float factor)
    {
      scfParent->SetChangeSize (factor);
    }
    virtual void UnsetChangeSize ()
    {
      scfParent->UnsetChangeSize ();
    }
    virtual bool GetChangeSize (float& factor) const
    {
      return scfParent->GetChangeSize(factor);
    }
    virtual void SetChangeRotation (float angle)
    {
      scfParent->SetChangeRotation (angle);
    }
    virtual void UnsetChangeRotation ()
    {
      scfParent->UnsetChangeRotation ();
    }
    virtual bool GetChangeRotation (float& angle) const
    {
      return scfParent->GetChangeRotation(angle);
    }
    virtual void SetChangeAlpha (float factor)
    {
      scfParent->SetChangeAlpha (factor);
    }
    virtual void UnsetChangeAlpha ()
    {
      scfParent->UnsetChangeAlpha ();
    }
    virtual bool GetChangeAlpha (float& factor) const
    {
      return scfParent->GetChangeAlpha(factor);
    }
    virtual void SetSelfDestruct (csTicks t)
    {
      scfParent->SetSelfDestruct (t);
    }
    virtual void UnSetSelfDestruct ()
    {
      scfParent->UnSetSelfDestruct ();
    }
  } scfiParticleState;
  friend struct eiParticleState;
};

/** @} */

#endif // __CS_CSPLUGINCOMMON_PARTICLESYS_PARTICLE_H__
