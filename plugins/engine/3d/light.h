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
  /// The associated halo (if not 0)
  csHalo *halo;

  /// The dynamic type of this light (one of CS_LIGHT_DYNAMICTYPE_...)
  csLightDynamicType dynamic_type;
  /// Type of this light
  csLightType type;

  /// Attenuation type
  csLightAttenuationMode attenuation;
  /// Attenuation vector in the format x=kc, y=kl, z=kq
  csVector3 attenuationvec;

  /// The radius where the light have any effect at all
  float influenceRadius; 
  /// Squared influence radius
  float influenceRadiusSq; 
  /// Inverse radius of light.
  float inv_dist;

  /// Direction for directional and spotlight. Should be normalized.
  csVector3 direction;
  /// Falloff coefficients for spotlight.
  csVector2 spotlight_falloff;

  /// Is the influence radius valid?
  bool influenceValid;

  /**
   * Config value: The intensity at the influenceRadius in parts of the 
   * main light intensity.
   */
  static float influenceIntensityFraction;

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

  csLightDynamicType GetDynamicType () const { return dynamic_type; }

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
    iSectorList* list = movable.GetSectors ();
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
    if (movable.csMovable::IsFullTransformIdentity () || 
        movable.csMovable::GetParent () == 0)
    {
      movable.csMovable::SetPosition (v);
    }
    else
    {
      csVector3 v2 = movable.csMovable::GetParent ()->GetFullTransform ().Other2This (v);
      movable.csMovable::SetPosition (v2);
    }

    movable.csMovable::UpdateMove ();
  }

  /**
   * Get the center position.
   */
  const csVector3 GetCenter () 
  { 
    return movable.GetFullPosition (); 
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
  csColor& GetColor () { return color; }

  /**
   * Set the light color. Note that setting the color
   * of a light may not always have an immediate visible effect.
   * Static lights are precalculated into the lightmaps and those
   * lightmaps are not automatically updated when calling this function
   * as that is a time consuming process.
   */
  void SetColor (const csColor& col);

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
  csLightAttenuationMode GetAttenuation () 
  {
    return attenuation;
  }

  /**
   * Change the light's attenuation type
   */
  void SetAttenuation (csLightAttenuationMode a); 

  /**
  * Set attenuation vector 
  * csVector3(constant, linear, quadric)
  */
  void SetAttenuationVector (const csVector3& pattenv);

  /**
  * Get attenuation vector
  * csVector3(constant, linear, quadric)
  */
  const csVector3 &GetAttenuationVector();

  /** 
   * Get the influence radius of the light
   */
  float GetInfluenceRadius ();

  /** 
   * Get the squared influence radius of the light
   */
  float GetInfluenceRadiusSq ();

  /**
   * Override the influence radius.
   */
  void SetInfluenceRadius (float radius);

  /**
   * Calculate the attenuation vector for a given attenuation type.
   * \param atttype Attenuation type constant - #CS_ATTN_NONE,
   *   #CS_ATTN_INVERSE, #CS_ATTN_REALISTIC
   * \param radius Radius where the light is \p brightness bright
   * \param brightness Brightness of the light at \p radius
   */
  void CalculateAttenuationVector (csLightAttenuationMode atttype, float radius = 1.0f,
    float brightness = 1.0f);

  /**
   * Get the distance for a given light brightness.
   * \return Returns whether the distance could be calculated. E.g.
   * when attenuation vector only has a constant part.
   * \remarks 
   * \li Might fail when \p brightness <= 0.
   */
  bool GetDistanceForBrightness (float brightness, float& distance);

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

  /**
   * Get the spotlight fall-off coefficients. First is cosine of
   * hotspot angle, second cosine of outer angle.
   */
  const csVector2& GetSpotFalloff () const
  { return spotlight_falloff; }
  /**
   * Set the spotlight fall-off coefficients. First is cosine of
   * hotspot angle, second cosine of outer angle.
   */
  virtual void SetSpotFalloff (const csVector2& v)
  { spotlight_falloff = v; }

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
    virtual iObject *QueryObject() { return scfParent; }
    virtual csLightDynamicType GetDynamicType () const
    {
      return scfParent->GetDynamicType ();
    }
    virtual const csVector3 GetCenter () { return scfParent->GetCenter (); }
    virtual void SetCenter (const csVector3& pos)
    {
      scfParent->SetCenter (pos);
    }
    virtual iSector *GetSector () { return scfParent->GetSector (); }
    virtual iMovable *GetMovable ()
    {
      return scfParent->GetMovable ();
    }
    virtual float GetInfluenceRadius ()
    {
      return scfParent->GetInfluenceRadius();
    }
    virtual float GetInfluenceRadiusSq ()
    {
      return scfParent->GetInfluenceRadiusSq();
    }
    virtual void SetInfluenceRadius (float radius)
    {
      scfParent->SetInfluenceRadius (radius);
    }
    virtual const csColor& GetColor () { return scfParent->GetColor (); }
    virtual void SetColor (const csColor& col) { scfParent->SetColor (col); }
    virtual csLightAttenuationMode GetAttenuation () { return scfParent->GetAttenuation (); }
    virtual void SetAttenuation (csLightAttenuationMode a) { scfParent->SetAttenuation (a); }
    virtual float GetBrightnessAtDistance (float d)
    {
      return scfParent->GetBrightnessAtDistance (d);
    }
    virtual void SetAttenuationVector(const csVector3& attenv) 
    { scfParent->SetAttenuationVector(attenv); }
    virtual const csVector3 &GetAttenuationVector()
    {
      return scfParent->GetAttenuationVector();
    }
    virtual void CalculateAttenuationVector (csLightAttenuationMode atttype, float radius,
      float brightness) { scfParent->CalculateAttenuationVector 
        (atttype, radius, brightness); }
    virtual bool GetDistanceForBrightness (float brightness, float& distance)
    { return scfParent->GetDistanceForBrightness (brightness, distance); }
    virtual iCrossHalo* CreateCrossHalo (float intensity, float cross);
    virtual iNovaHalo* CreateNovaHalo (int seed, int num_spokes,
    float roundness);
    virtual iFlareHalo* CreateFlareHalo ();
    virtual csFlags& GetFlags () { return scfParent->flags; }
    virtual void SetLightCallback (iLightCallback* cb)
    {
      scfParent->SetLightCallback (cb);
    }
    virtual void RemoveLightCallback (iLightCallback* cb)
    {
      scfParent->RemoveLightCallback (cb);
    }
    virtual int GetLightCallbackCount () const
    {
      return scfParent->GetLightCallbackCount ();
    }
    virtual iLightCallback* GetLightCallback (int idx) const
    {
      return scfParent->GetLightCallback (idx);
    }
    virtual uint32 GetLightNumber () const
    {
      return scfParent->lightnr;
    }
    virtual void AddAffectedLightingInfo (iLightingInfo* li)
    { scfParent->AddAffectedLightingInfo (li); }
    virtual void RemoveAffectedLightingInfo (iLightingInfo* li)
    { scfParent->RemoveAffectedLightingInfo (li); }
    virtual iBaseHalo* GetHalo () { return scfParent->GetHalo (); }
    virtual void Setup ()
    { scfParent->CalculateLighting (); }
    virtual csLightType GetType () const
    { return scfParent->GetType (); }
    virtual void SetType (csLightType type)
    { scfParent->SetType (type); }
    virtual const csVector3& GetDirection () const
    { return scfParent->GetDirection (); }
    virtual void SetDirection (const csVector3& v)
    { scfParent->SetDirection (v); }
    virtual const csVector2& GetSpotFalloff () const
    { return scfParent->GetSpotFalloff (); }
    virtual void SetSpotFalloff (const csVector2& v)
    { scfParent->SetSpotFalloff (v); }
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

