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
#include "csutil/refarr.h"
#include "csengine/lview.h"
#include "iengine/light.h"
#include "iengine/statlght.h"
#include "iengine/dynlight.h"

class csLightMap;
class csDynLight;
class csHalo;
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
  /// Home sector of the light.
  iSector* sector;
  /// Position of the light.
  csVector3 center;
  /// Color.
  csColor color;
  /// The associated halo (if not 0)
  csHalo *halo;

  /// Attenuation type
  int attenuation;
  /// Attenuation vector in the format x=kc, y=kl, z=kq
  csVector3 attenuationvec;

  /// The radius where the light have any effect at all
  float influenceRadius; 
  /// Squared influence radius
  float influenceRadiusSq; 
  /// Inverse radius of light.
  float inv_dist;

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
     float red, float green, float blue);

  /**
   * Destroy the light. Note that destroying a light
   * may not have the expected effect. Static lights result
   * in changes in the lightmaps. Removing them will not automatically
   * update those lightmaps as that is a time-consuming process.
   */
  virtual ~csLight ();

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

  /// Return true if the light is pseudo-dynamic.
  virtual bool IsDynamic () const { return false; }

  /**
   * Set the current sector for this light.
   */
  virtual void SetSector (iSector* sector);

  /**
   * Get the current sector for this light.
   */
  iSector* GetSector () const { return sector; }

  /**
   * Set the center position.
   */
  void SetCenter (const csVector3& v);

  /**
   * Get the center position.
   */
  const csVector3& GetCenter () { return center; }

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
  virtual void SetColor (const csColor& col);

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
  int GetAttenuation () {return attenuation;}

  /**
   * Change the light's attenuation type
   */
  void SetAttenuation (int a); 

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
  void CalculateAttenuationVector (int atttype, float radius = 1.0f,
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

  //------------------------ iLight interface -----------------------------
  SCF_DECLARE_IBASE_EXT (csObject);

  /// iLight implementation
  struct Light : public iLight
  {
    SCF_DECLARE_EMBEDDED_IBASE (csLight);
    virtual csLight* GetPrivateObject () { return scfParent; }
    virtual const char* GetLightID () { return scfParent->GetLightID (); }
    virtual iObject *QueryObject() { return scfParent; }
    virtual const csVector3& GetCenter () { return scfParent->GetCenter (); }
    virtual void SetCenter (const csVector3& pos)
    {
      scfParent->SetCenter (pos);
    }
    virtual iSector *GetSector () { return scfParent->GetSector (); }
    virtual void SetSector (iSector* sector) { scfParent->SetSector (sector); }
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
    virtual bool IsDynamic () const { return scfParent->IsDynamic (); }
    virtual int GetAttenuation () { return scfParent->GetAttenuation (); }
    virtual void SetAttenuation (int a) { scfParent->SetAttenuation (a); }
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
    virtual void CalculateAttenuationVector (int atttype, float radius,
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
  } scfiLight;
  friend struct Light;
};

/**
 * Class for a static light. These lights cast shadows (against
 * sector boundaries and with things), they support three different
 * colors (R,G,B). They cannot move and they can only vary in
 * intensity with some memory trade-offs (in which case we call
 * it a pseudo-dynamic light).
 */
class csStatLight : public csLight
{
private:
  /**
   * The following three variables are used if the light intensity
   * can vary. 'dynamic' is set to true in that case.
   * The 'lightmaps' vector indicates all lightmaps that are
   * possibly lit by this light.
   */
  bool dynamic;

  /// Set of meshes that we are currently affecting.
  csHashSet lightinginfos;

public:
  /**
   * Construct a static light at a given position. With
   * a given radius and a given color. If 'dynamic' is
   * true we have a pseudo-dynamic light which can change
   * intensity and color (but not move).
   * The light will not have a halo by default.
   */
  csStatLight (float x, float y, float z, float dist,
     float red, float green, float blue,
   bool dynamic);
  /**
   * Destroy the light. Note that destroying a light
   * may not have the expected effect. Static lights result
   * in changes in the lightmaps. Removing them will not automatically
   * update those lightmaps as that is a time-consuming process.
   */
  virtual ~csStatLight ();

  /**
   * Return true if this light is pseudo-dynamic.
   */
  virtual bool IsDynamic () const { return dynamic; }

  /**
   * Set the light color. Note that setting the color
   * of a light may not always have an immediate visible effect.
   * Static lights are precalculated into the lightmaps and those
   * lightmaps are not automatically updated when calling this function
   * as that is a time consuming process.
   * However, this function works as expected for pseudo-dynamic
   * lights. In this case the lightmaps will be correctly updated
   * the next time they become visible.
   */
  virtual void SetColor (const csColor& col);

  /// Add affected mesh.
  void AddAffectedLightingInfo (iLightingInfo* li);

  /**
   * Shine this light on all polygons visible from the light.
   * This routine will update the lightmaps of all polygons or
   * update the vertex colors if gouraud shading is used.
   * It correctly takes pseudo-dynamic lights into account and will then
   * update the corresponding shadow map.
   */
  void CalculateLighting ();

  /**
   * Shine this light on all polygons of the mesh.
   * Only backface culling is used. The light is assumed
   * to be in the same sector as the mesh.
   * Currently only works on thing meshes.
   */
  void CalculateLighting (iMeshWrapper* mesh);

  //------------------------ iStatLight interface -----------------------------
  SCF_DECLARE_IBASE_EXT (csLight);

  /// iStatLight implementation.
  struct eiStaticLight : public iStatLight
  {
    SCF_DECLARE_EMBEDDED_IBASE (csStatLight);

    /// Used by the engine to retrieve internal static light object (ugly)
    virtual csStatLight *GetPrivateObject ()
    { return scfParent; }
    virtual iObject *QueryObject ()
    { return scfParent; }
    virtual iLight *QueryLight ()
    { return &scfParent->scfiLight; }
    virtual void AddAffectedLightingInfo (iLightingInfo* li)
    { scfParent->AddAffectedLightingInfo (li); }
  } scfiStatLight;
  friend struct eiStaticLight;
};

/**
 * Class for a dynamic light. These lights only cast shadows
 * for sectors/portals and not for things. However, they can
 * freely move and change color intensity.
 */
class csDynLight : public csLight
{
private:
  csDynLight* next;
  csDynLight* prev;

  /// Set of meshes that we are currently affecting.
  csHashSet lightinginfos;

public:
  /**
   * Create a new dynamic light at the given position and with the
   * given radius and color. Initially the light will
   * not be visible. You need to set the current
   * sector and call 'Setup()' first.
   */
  csDynLight (float x, float y, float z, float dist,
     float red, float green, float blue);

  /**
   * Remove the dynamic light from all polygons (i.e.
   * remove all light patches) and then destroy the light itself.
   */
  virtual ~csDynLight ();

  /**
   * Initial placement of the light. This routine generates a view
   * frustum as seen from the light. The clipped polygons that
   * result from this are light patches and are put in the
   * lightpatches list. This routine needs to be called whenever
   * the light moves.
   */
  void Setup ();

  /**
   * Call this when the color of the light changes. This is more
   * efficient than calling Setup().
   */
  virtual void SetColor (const csColor& col);

  /**
   * Add a lighting info to this dynamic light. This is usually
   * called during Setup() by meshes that are hit by the
   * dynamic light.
   */
  void AddAffectedLightingInfo (iLightingInfo* li);

  /**
   * Remove a lighting info from this dynamic light.
   */
  void RemoveAffectedLightingInfo (iLightingInfo* li);

  ///
  void SetNext (csDynLight* n) { next = n; }
  ///
  void SetPrev (csDynLight* p) { prev = p; }
  ///
  csDynLight* GetNext () { return next; }
  ///
  csDynLight* GetPrev () { return prev; }

  //------------------------ iDynLight interface -----------------------------
  SCF_DECLARE_IBASE_EXT (csLight);

  /// iDynLight implementation.
  struct eiDynLight : public iDynLight
  {
    SCF_DECLARE_EMBEDDED_IBASE (csDynLight);

    /// Used by the engine to retrieve internal dyn light object (ugly)
    virtual csDynLight* GetPrivateObject ()
    { return scfParent; }
    virtual void AddAffectedLightingInfo (iLightingInfo* li)
    { scfParent->AddAffectedLightingInfo (li); }
    virtual void RemoveAffectedLightingInfo (iLightingInfo* li)
    { scfParent->RemoveAffectedLightingInfo (li); }
    virtual void Setup ()
    { scfParent->Setup (); }
    virtual iObject *QueryObject ()
    { return scfParent; }
    virtual iLight *QueryLight ()
    { return &(scfParent->scfiLight); }
    virtual iDynLight* GetNext ()
    {
      csDynLight* n = scfParent->GetNext ();
      return n ? &(n->scfiDynLight) : 0;
    }
  } scfiDynLight;
  friend struct eiDynLight;
};


/**
 * List of lights for a sector. This class implements iLightList.
 */
class csLightList : public iLightList
{
private:
  csRefArrayObject<iLight> list;

public:
  SCF_DECLARE_IBASE;

  /// constructor
  csLightList ();
  virtual ~csLightList () { RemoveAll (); }

  /// Override PrepareItem
  virtual void PrepareItem (iLight*) { }
  /// Override FreeItem
  virtual void FreeItem (iLight*) { }

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
  virtual ~csLightingProcessInfo () { }

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

