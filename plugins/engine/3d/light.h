/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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

#ifndef __CS_LIGHT_H__
#define __CS_LIGHT_H__

#include "csgeom/transfrm.h"
#include "csutil/csobject.h"
#include "csutil/cscolor.h"
#include "csutil/flags.h"
#include "csutil/nobjvec.h"
#include "csutil/hashmap.h"
#include "csutil/hash.h"
#include "csutil/hashhandlers.h"
#include "csutil/refarr.h"
#include "plugins/engine/3d/lview.h"
#include "iengine/light.h"
#include "plugins/engine/3d/halo.h"
#include "plugins/engine/3d/movable.h"

class csLightMap;
class csPolygon3D;
class csCurve;
class csKDTreeChild;
struct iMeshWrapper;
struct iLightingInfo;
struct iSector;

/**
 * Superclass of all positional lights.
 * A light subclassing from this has a color, a position
 * and a radius.
 */
class csLight : public csObject
{
private:
  /// ID for this light (16-byte MD5).
  char* light_id;

  /// Childnode representing this light in the sector light list kdtree.
  csKDTreeChild* childnode;

protected:
  /// Movable for the light
  csMovable movable;
  /// Color.
  csColor color;
  /// Specular color
  csColor specularColor;
  /// The associated halo (if not 0)
  csHalo *halo;

  /// The dynamic type of this light (one of CS_LIGHT_DYNAMICTYPE_...)
  csLightDynamicType dynamicType;
  /// Type of this light
  csLightType type;

  /// Attenuation type
  csLightAttenuationMode attenuation;
  /// Attenuation constants
  csVector3 attenuationConstants;

  /// The distance where the light have any effect at all
  float cutoffDistance; 

  /// Radial cutoff radius for directional lights
  float directionalCutoffRadius;

  /// Direction for directional and spotlight. Should be normalized.
  csVector3 direction;
  /// Falloff coefficients for spotlight.
  float spotlightFalloffInner, spotlightFalloffOuter;

  
  /// Light number. Changes when the light changes in some way (color/pos).
  uint32 lightnr;

  /**
   * List of light callbacks.
   */
  csRefArray<iLightCallback> light_cb_vector;

  /// Set of meshes that we are currently affecting.
  csHashSet lightinginfos;

  /// Get a unique ID for this light. Generate it if needed.
  const char* GenerateUniqueID ();

  /**
   * Calculate the influenceradius from the attenuation vector.
   */
  void CalculateInfluenceRadius ();

public:
  /// Set of flags
  csFlags flags;

public:
  /** 
   * Config value: ambient red value.
   */
  static int ambient_red;
  /**
   * Config value: ambient green value.
   */
  static int ambient_green;
  /** 
   * Config value: ambient blue value.
   */
  static int ambient_blue;

public:
  /**
   * Construct a light at a given position. With
   * a given radius, a given color, a given name and
   * type. The light will not have a halo by default.
   */
  csLight (float x, float y, float z, float dist,
     float red, float green, float blue, csLightDynamicType dyntype);

  /**
   * Destroy the light. Note that destroying a light
   * may not have the expected effect. Static lights result
   * in changes in the lightmaps. Removing them will not automatically
   * update those lightmaps as that is a time-consuming process.
   */
  virtual ~csLight ();

  csLightDynamicType GetDynamicType () const { return dynamicType; }

  /**
   * Shine this light on all polygons visible from the light.
   * This routine will update the lightmaps of all polygons or
   * update the vertex colors if gouraud shading is used.
   * It correctly takes pseudo-dynamic lights into account and will then
   * update the corresponding shadow map.
   * For dynamic lights this will work differently.
   */
  void CalculateLighting ();

  /**
   * Shine this light on all polygons of the mesh.
   * Only backface culling is used. The light is assumed
   * to be in the same sector as the mesh.
   * Currently only works on thing meshes.
   */
  void CalculateLighting (iMeshWrapper* mesh);

  /**
   * Set the kdtree child node used by this light (in the kdtree
   * that is maintained by the sector light list).
   */
  void SetChildNode (csKDTreeChild* childnode)
  {
    csLight::childnode = childnode;
  }

  /**
   * Get the kdtree child node.
   */
  csKDTreeChild* GetChildNode () const { return childnode; }

  /// Get the ID of this light.
  const char* GetLightID () { return GenerateUniqueID (); }

  /**
   * Set the current sector for this light.
   */
  void SetSector (iSector* sector)
  {
    movable.SetSector (sector);
    movable.UpdateMove ();
  }

  /**
   * Get the current sector for this light.
   */
  iSector* GetSector ()
  { 
    iSectorList* list = movable.csMovable::GetSectors ();
    iSector *s = 0;
    if (list && list->GetCount () > 0)
    {
      s = list->Get (0);
    }
    return s;
  }

  /**
   * Set the center position.
   */
  void SetCenter (const csVector3& v)
  {
    movable.csMovable::SetPosition (v);
    movable.csMovable::UpdateMove ();
  }

  /**
   * Get the center position.
   */
  const csVector3& GetCenter () const
  { 
    return movable.csMovable::GetPosition (); 
  }
  
  /**
   * Get the movable 
   */
  iMovable *GetMovable ()
  {
    return &movable;
  }

  /**
   * Get the light color.
   */
  const csColor& GetColor () const { return color; } 

  /**
   * Set the light color. Note that setting the color
   * of a light may not always have an immediate visible effect.
   * Static lights are precalculated into the lightmaps and those
   * lightmaps are not automatically updated when calling this function
   * as that is a time consuming process.
   */
  void SetColor (const csColor& col);

  /// Get the specular color of this light.
  const csColor& GetSpecularColor () const
  { return specularColor; }
  /// Set the specular color of this light.
  void SetSpecularColor (const csColor& col) 
  { specularColor = col; }

  /**
   * Return the associated halo
   */
  csHalo *GetHalo () { return halo; }

  /**
   * Set the halo associated with this light.
   */
  void SetHalo (csHalo *Halo);

  /**
   * Get the light's attenuation type
   */
  csLightAttenuationMode GetAttenuationMode () 
  {
    return attenuation;
  }

  /**
   * Change the light's attenuation type
   */
  void SetAttenuationMode (csLightAttenuationMode a); 
 
  /**
  * Set attenuation constants
  * \sa csLightAttenuationMode
  */
  void SetAttenuationConstants (const csVector3& constants);
  /**
  * Get attenuation constants
  * \sa csLightAttenuationMode
  */
  const csVector3 &GetAttenuationConstants () const
  { return attenuationConstants; }

  /**
  * Get the the maximum distance at which the light is guranteed to shine. 
  * Can be seen as the distance at which we turn the light of.
  * Used for culling and selection of meshes to light, but not
  * for the lighting itself.
  */
  float GetCutoffDistance () const
  { return cutoffDistance; }

  /**
  * Set the the maximum distance at which the light is guranteed to shine. 
  * Can be seen as the distance at which we turn the light of.
  * Used for culling and selection of meshes to light, but not
  * for the lighting itself.
  */
  void SetCutoffDistance (float distance);

  /**
  * Get radial cutoff distance for directional lights.
  * The directional light can be viewed as a cylinder with radius
  * equal to DirectionalCutoffRadius and length CutoffDistance
  */
  float GetDirectionalCutoffRadius () const
  { return directionalCutoffRadius; }

  /**
  * Set radial cutoff distance for directional lights.
  * The directional light can be viewed as a cylinder with radius
  * equal to DirectionalCutoffRadius and length CutoffDistance
  */
  void SetDirectionalCutoffRadius (float radius)
  {
    directionalCutoffRadius = radius;
    lightnr++;
  }

  /**
  * Set spot light falloff angles. Set in cosine of the angle. 
  */
  void SetSpotLightFalloff (float inner, float outer)
  {
    spotlightFalloffInner = inner;
    spotlightFalloffOuter = outer;
    lightnr++;
  }

  /**
  * Get spot light falloff angles. Get in cosine of the angle.
  */
  void GetSpotLightFalloff (float& inner, float& outer) const
  {
    inner = spotlightFalloffInner;
    outer = spotlightFalloffOuter;
  }

  /**
   * Get the brightness of a light at a given distance.
   */
  float GetBrightnessAtDistance (float d);

  /// Get the light type of this light.
  csLightType GetType () const
  { return type; }
  /// Set the light type of this light.
  void SetType (csLightType type)
  { this->type = type; }

  /// Get the light direction. Used for directional and spotlight.
  const csVector3& GetDirection () const
  { return direction; }
  /// Set the light direction. Used for directional and spotlight.
  void SetDirection (const csVector3& v)
  { if (v.IsZero()) return; direction = v.Unit (); }


  //----------------------------------------------------------------------
  // Light influence stuff.
  //----------------------------------------------------------------------

  /**
   * Add a lighting info to this dynamic light. This is usually
   * called during Setup() by meshes that are hit by the
   * light.
   */
  void AddAffectedLightingInfo (iLightingInfo* li);

  /**
   * Remove a lighting info from this light.
   */
  void RemoveAffectedLightingInfo (iLightingInfo* li);

  //----------------------------------------------------------------------
  // Callbacks
  //----------------------------------------------------------------------
  void SetLightCallback (iLightCallback* cb)
  {
    light_cb_vector.Push (cb);
  }

  void RemoveLightCallback (iLightCallback* cb)
  {
    light_cb_vector.Delete (cb);
  }

  int GetLightCallbackCount () const
  {
    return light_cb_vector.Length ();
  }
  
  iLightCallback* GetLightCallback (int idx) const
  {
    return (iLightCallback*)light_cb_vector.Get (idx);
  }

  //----------------------------------------------------------------------
  // Movable interface
  //----------------------------------------------------------------------
  /**
   * Called by the movable when sector changes 
   */
  void OnSetSector (iSector* sector);

  /**
   * Called by the movable when position changes 
   */
  void OnSetPosition ();

  //------------------------ iLight interface -----------------------------
  SCF_DECLARE_IBASE_EXT (csObject);

  /// iLight implementation
  struct Light : public iLight
  {
    csLight* GetPrivateObject () { return scfParent; }

    SCF_DECLARE_EMBEDDED_IBASE (csLight);
    virtual const char* GetLightID () { return scfParent->GetLightID (); }
    virtual iObject *QueryObject() { return (iObject*)scfParent; }
    virtual csLightDynamicType GetDynamicType () const 
    { return scfParent->GetDynamicType (); }

    virtual const csVector3& GetCenter () const 
    { return scfParent->GetCenter (); }
    virtual void SetCenter (const csVector3& pos)
    { scfParent->SetCenter (pos); }

    virtual iSector *GetSector ()
    { return scfParent->GetSector (); }

    virtual iMovable *GetMovable () 
    { return scfParent->GetMovable (); }

    virtual const csColor& GetColor () const 
    { return scfParent->GetColor (); }
    virtual void SetColor (const csColor& col)
    { scfParent->SetColor (col); }

    virtual const csColor& GetSpecularColor () const 
    { return scfParent->GetSpecularColor (); }
    virtual void SetSpecularColor (const csColor& col) 
    { scfParent->SetSpecularColor (col);} 

    virtual csLightType GetType () const
    { return scfParent->GetType (); }
    virtual void SetType (csLightType type)
    { scfParent->SetType (type); }

    virtual const csVector3& GetDirection () const 
    { return scfParent->GetDirection (); }
    virtual void SetDirection (const csVector3& v)
    { scfParent->SetDirection (v); }

    virtual csLightAttenuationMode GetAttenuationMode () const
    { return scfParent->GetAttenuationMode (); }
    virtual void SetAttenuationMode (csLightAttenuationMode a)
    { scfParent->SetAttenuationMode (a); }
    
    virtual void SetAttenuationConstants (const csVector3& constants)
    { scfParent->SetAttenuationConstants (constants); }
    virtual const csVector3 &GetAttenuationConstants () const 
    { return scfParent->GetAttenuationConstants (); }

    virtual float GetCutoffDistance () const
    { return scfParent->GetCutoffDistance (); }
    virtual void SetCutoffDistance (float distance)
    { scfParent->SetCutoffDistance (distance); }

    virtual float GetDirectionalCutoffRadius () const
    { return scfParent->GetDirectionalCutoffRadius (); }
    virtual void SetDirectionalCutoffRadius (float radius) 
    { scfParent->SetDirectionalCutoffRadius (radius); }
    
    virtual void SetSpotLightFalloff (float inner, float outer)
    { scfParent->SetSpotLightFalloff (inner, outer); }
    virtual void GetSpotLightFalloff (float& inner, float& outer) const
    { scfParent->GetSpotLightFalloff (inner, outer); }

    virtual iCrossHalo* CreateCrossHalo (float intensity, float cross);
    virtual iNovaHalo* CreateNovaHalo (int seed, int num_spokes,
      float roundness);
    virtual iFlareHalo* CreateFlareHalo ();
    virtual iBaseHalo* GetHalo () const 
    { return scfParent->GetHalo (); }

    virtual float GetBrightnessAtDistance (float d) const
    { return scfParent->GetBrightnessAtDistance (d); }

    virtual csFlags& GetFlags ()
    { return scfParent->flags; }

    virtual void SetLightCallback (iLightCallback* cb)
    { scfParent->SetLightCallback (cb); }
    virtual void RemoveLightCallback (iLightCallback* cb) 
    { scfParent->RemoveLightCallback (cb); }
    virtual int GetLightCallbackCount () const
    { return scfParent->GetLightCallbackCount (); }
    virtual iLightCallback* GetLightCallback (int idx) const 
    { return scfParent->GetLightCallback (idx); }

    virtual uint32 GetLightNumber () const
    { return scfParent->lightnr; }

    virtual void AddAffectedLightingInfo (iLightingInfo* li) 
    { scfParent->AddAffectedLightingInfo (li); }    
    virtual void RemoveAffectedLightingInfo (iLightingInfo* li) 
    { scfParent->RemoveAffectedLightingInfo (li); }
    virtual void Setup ()
    { scfParent->CalculateLighting (); }
  } scfiLight;
  friend struct Light;
};

/**
 * List of lights for a sector. This class implements iLightList.
 */
class csLightList : public iLightList
{
private:
  csRefArrayObject<iLight> list;
  csHash<iLight*,csStrKey,csConstCharHashKeyHandler> lights_hash;

public:
  SCF_DECLARE_IBASE;

  /// constructor
  csLightList ();
  virtual ~csLightList ();

  /// Override PrepareLight
  virtual void PrepareLight (iLight*) { }
  /// Override FreeLight
  virtual void FreeLight (iLight*) { }

  virtual int GetCount () const { return list.Length (); }
  virtual iLight *Get (int n) const { return list.Get (n); }
  virtual int Add (iLight *obj);
  virtual bool Remove (iLight *obj);
  virtual bool Remove (int n);
  virtual void RemoveAll ();
  virtual int Find (iLight *obj) const;
  virtual iLight *FindByName (const char *Name) const;
  virtual iLight *FindByID (const char* id) const;
};

/**
 * This is user-data for iFrustumView for the lighting process.
 */
struct csLightingProcessInfo : public iLightingProcessInfo
{
private:
  // Light.
  csLight* light;
  // For dynamic lighting.
  bool dynamic;
  // Current lighting color.
  csColor color;
  // Array of user data.
  csRefArray<iLightingProcessData> userdatas;

public:
  csLightingProcessInfo (csLight* light, bool dynamic);
  virtual ~csLightingProcessInfo ();

  /**
   * Get the light.
   */
  csLight* GetCsLight () const { return light; }
  virtual iLight* GetLight () const { return &(light->scfiLight); }

  /**
   * Return true if dynamic.
   */
  virtual bool IsDynamic () const { return dynamic; }

  /**
   * Set the current color.
   */
  virtual void SetColor (const csColor& col) { color = col; }

  /**
   * Get the current color.
   */
  virtual const csColor& GetColor () const { return color; }

  /// Attach userdata.
  virtual void AttachUserdata (iLightingProcessData* userdata);

  /// Query for userdata based on SCF type.
  virtual csPtr<iLightingProcessData> QueryUserdata (scfInterfaceID id,
    int version);

  /// Finalize lighting.
  virtual void FinalizeLighting ();

  SCF_DECLARE_IBASE;
};

#endif // __CS_LIGHT_H__

