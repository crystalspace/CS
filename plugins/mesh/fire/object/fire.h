/*
    Copyright (C) 2000 by W.C.A. Wijngaards

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

#ifndef __CS_FIRE_H__
#define __CS_FIRE_H__

#include "csgeom/vector3.h"
#include "csgeom/box.h"
#include "csutil/cscolor.h"
#include "csutil/refarr.h"
#include "csutil/scf_implementation.h"
#include "csplugincommon/particlesys/partgen.h"
#include "imesh/fire.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"

struct iMaterialWrapper;
struct iLight;
struct iEngine;
struct iSector;

/**
 * A Fire particle system. Each x msec n particles shoot out of the fire,
 */
class csFireMeshObject :
  public scfImplementationExt1<csFireMeshObject, csParticleSystem, iFireState>
{
protected:
  enum { MAX_COLORS = 5 };
  struct ColorInfo { csColor c; float age; float dage; };
  static ColorInfo* Colors;
  static void SetupColors();
  static ColorInfo const* GetColorInfo(int n) { return Colors + n; }
  static csColor const GetColor(int n) { return Colors[n].c;    }
  static float GetColorAge(int n)      { return Colors[n].age;  }
  static float GetColorDAge(int n)     { return Colors[n].dage; }

  // The following two colors are precalculated from the base colors, ages,
  // and dages (see above).  If precalc_valid == false this table has to be
  // recalculated.
  csColor precalc_add[5], precalc_mul[5];
  bool precalc_valid;

  int amt;
  csVector3 direction;
  csBox3 origin;
  float swirl;
  float color_scale;
  csVector3* part_speed;
  float *part_age;
  float total_time;
  float inv_total_time;
  float time_left; // from previous update
  int next_oldest;

  float drop_width, drop_height;
  bool lighted_particles;

  csRef<iLight> dynlight;
  int light_time;
  bool delete_light;
  iEngine* light_engine;

  int FindOldest ();
  void RestartParticle (int index, float pre_move);
  void MoveAndAge (int index, float delta_t);

  void SetupObject ();

public:
  /**
   * creates a fire particle system.
   * number : number of particles visible at one time
   * mat: material of particles. mixmode = mixmode used.
   * lighted: the particles will be lighted if true.
   * drop_width, drop_height: size of rectangular particles.
   * total_time is the seconds a particle gets to burn.
   * dir is direction of fire.
   * origin is the starting point of the flame
   * swirl is the amount of swirling of particles.
   * color_scale scales the colour the particles are set to.
   */
  csFireMeshObject (iObjectRegistry* object_reg, iMeshObjectFactory* factory);
  /// Destructor.
  virtual ~csFireMeshObject ();

  /// You can set a pseudo-static light here
  void SetControlledLight (iLight *l);
  /**
   * Add a new dynamic light (no need to call SetControlledLight).
   */
  void AddLight (iEngine*, iSector*);

  //------------------------- iFireState implementation ----------------
  /// Set the size of the fire drops.
  void SetDropSize (float dropwidth, float dropheight)
  {
    initialized = false;
    drop_width = dropwidth;
    drop_height = dropheight;
    ShapeChanged ();
  }
  /// Get the size of the fire drops.
  void GetDropSize (float& dropwidth, float& dropheight) const
  { dropwidth = drop_width; dropheight = drop_height; }
  /// Set origin of the fire.
  void SetOrigin (const csBox3& origin)
  {
    initialized = false;
    csFireMeshObject::origin = origin;
    ShapeChanged ();
  }
  /// Get origin of the fire.
  const csBox3& GetOrigin () const
  { return origin; }
  /// Set direction of the fire.
  void SetDirection (const csVector3& direction)
  {
    initialized = false;
    csFireMeshObject::direction = direction;
    ShapeChanged ();
  }
  /// Get direction of the fire.
  const csVector3& GetDirection () const
  { return direction; }
  /// Enable or disable lighting.
  void SetLighting (bool l)
  {
    initialized = false;
    lighted_particles = l;
  }
  /// See if lighting is enabled.
  bool GetLighting () const
  { return lighted_particles; }
  /// Set swirl.
  void SetSwirl (float swirl)
  {
    initialized = false;
    csFireMeshObject::swirl = swirl;
    ShapeChanged ();
  }
  /// Get swirl.
  float GetSwirl () const
  { return swirl; }
  /// Set color scale.
  void SetColorScale (float colscale)
  {
    initialized = false;
    color_scale = colscale;
    precalc_valid = false;
  }
  /// Get color scale.
  float GetColorScale () const
  { return color_scale; }
  /// Set total time.
  void SetTotalTime (float tottime)
  {
    initialized = false;
    total_time = tottime;
    inv_total_time = 1. / total_time;
  }
  /// Get total time.
  float GetTotalTime () const
  { return total_time; }

  /// Update the particle system.
  virtual void Update (csTicks elapsed_time);

  /// For iMeshObject.
  virtual void HardTransform (const csReversibleTransform& t);
  virtual bool SupportsHardTransform () const { return true; }

  // Redirect these iFireState functions to csParticleSystem
  virtual void SetParticleCount (int num)
  { csParticleSystem::SetParticleCount (num); }
  virtual size_t GetParticleCount () const
  { return csParticleSystem::GetParticleCount (); }
};

/**
 * Factory for fire.
 */
class csFireMeshObjectFactory :
  public scfImplementation1<csFireMeshObjectFactory, iMeshObjectFactory>
{
private:
  iObjectRegistry* object_reg;
  iMeshFactoryWrapper* logparent;
  iMeshObjectType* fire_type;
  csFlags flags;

public:
  /// Constructor.
  csFireMeshObjectFactory (iMeshObjectType *pParent,
  	iObjectRegistry* object_reg);

  /// Destructor.
  virtual ~csFireMeshObjectFactory ();

  //------------------------ iMeshObjectFactory implementation --------------
  virtual csFlags& GetFlags () { return flags; }
  virtual csPtr<iMeshObject> NewInstance ();
  virtual csPtr<iMeshObjectFactory> Clone () { return 0; }
  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () const { return false; }
  virtual void SetMeshFactoryWrapper (iMeshFactoryWrapper* lp)
  { logparent = lp; }
  virtual iMeshFactoryWrapper* GetMeshFactoryWrapper () const
  { return logparent; }
  virtual iMeshObjectType* GetMeshObjectType () const { return fire_type; }
  virtual iObjectModel* GetObjectModel () { return 0; }
  virtual bool SetMaterialWrapper (iMaterialWrapper*) { return false; }
  virtual iMaterialWrapper* GetMaterialWrapper () const { return 0; }
  virtual void SetMixMode (uint) { }
  virtual uint GetMixMode () const { return 0; }
};

/**
 * Fire type. This is the plugin you have to use to create instances
 * of csFireMeshObjectFactory.
 */
class csFireMeshObjectType :
  public scfImplementation2<csFireMeshObjectType, iMeshObjectType, iComponent>
{
private:
  iObjectRegistry* object_reg;

public:
  /// Constructor.
  csFireMeshObjectType (iBase*);
  /// Destructor.
  virtual ~csFireMeshObjectType ();
  /// Draw.
  virtual csPtr<iMeshObjectFactory> NewFactory ();

  virtual bool Initialize (iObjectRegistry* p)
  { this->object_reg = p; return true; }
};

#endif // __CS_FIRE_H__
