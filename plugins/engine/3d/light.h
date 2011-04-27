/*
    Copyright (C) 1998-2006 by Jorrit Tyberghein

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
#include "csgeom/box.h"
#include "csgfx/lightsvcache.h"
#include "csgfx/shadervarcontext.h"
#include "cstool/objmodel.h"
#include "csutil/cscolor.h"
#include "csutil/csobject.h"
#include "csutil/flags.h"
#include "csutil/hash.h"
#include "csutil/nobjvec.h"
#include "csutil/refarr.h"
#include "csutil/scf_implementation.h"
#include "csutil/scfarray.h"
#include "csutil/weakref.h"
#include "iutil/selfdestruct.h"
#include "plugins/engine/3d/halo.h"
#include "plugins/engine/3d/movable.h"
#include "plugins/engine/3d/scenenode.h"
#include "iengine/light.h"
#include "iengine/lightmgr.h"
#include "iengine/viscull.h"

class csLightMap;
class csPolygon3D;
class csCurve;
struct iMeshWrapper;
struct iSector;

#include "csutil/deprecated_warn_off.h"

CS_PLUGIN_NAMESPACE_BEGIN(Engine)
{

class csLightObjectModel : public scfImplementationExt0<csLightObjectModel,
                                                        csObjectModel>
{
public:
  csBox3 box;
  float radius;

  csLightObjectModel ()
    : scfImplementationType (this)
  {
  }
  virtual ~csLightObjectModel ()
  {
  }

  virtual const csBox3& GetObjectBoundingBox ()
  {
    return box;
  }
  virtual void SetObjectBoundingBox (const csBox3& bbox)
  {
    box = bbox;
  }
  virtual void GetRadius (float& rad, csVector3& cent)
  {
    rad = radius;
    cent.Set (0, 0, 0);	// @@@ FIXME!
  }
  virtual iTerraFormer* GetTerraFormerColldet () { return 0; }
  virtual iTerrainSystem* GetTerrainColldet () { return 0; }
};



/**
 * Superclass of all positional lights.
 * A light subclassing from this has a color, a position
 * and a radius.
 */
class csLight : 
  public scfImplementationExt4<csLight,
                               csObject,
                               iLight,                               
                               scfFakeInterface<iShaderVariableContext>,
			       iSceneNode,
			       iSelfDestruct>,
  public CS::ShaderVariableContextImpl
{
private:
  /// ID for this light (16-byte MD5).
  char* light_id;  
  
protected:
  /// Movable for the light
  mutable csMovable movable;

  /// Color.
  csColor color;
  /// Specular color
  csColor specularColor;
  /**
   * Whether the user changed the specular color.
   * This decides whether a call to SetColor() changes both the diffuse and
   * specular color (userSpecular == false) or only diffuse 
   * (userSpecular == true).
   */
  bool userSpecular;
  /// The associated halo (if not 0)
  csHalo *halo;

  /// The dynamic type of this light (one of CS_LIGHT_DYNAMICTYPE_...)
  csLightDynamicType dynamicType;
  /// Type of this light
  csLightType type;

  /// Attenuation type
  csLightAttenuationMode attenuation;
  /// Attenuation constants
  csVector4 attenuationConstants;

  /// The distance where the light have any effect at all
  float cutoffDistance; 

  /// Radial cutoff radius for directional lights
  float directionalCutoffRadius;
  // Analogue to userSpecular
  bool userDirectionalCutoffRadius;

  /// Falloff coefficients for spotlight.
  float spotlightFalloffInner, spotlightFalloffOuter;
  
  /// Light number. Changes when the light changes in some way (color/pos).
  uint32 lightnr;

  /**
   * List of light callbacks.
   */
  csRefArray<iLightCallback> light_cb_vector;

  /// Get a unique ID for this light. Generate it if needed.
  const char* GenerateUniqueID ();

  /**
   * Calculate the cutoff from the attenuation vector.
   */
  void CalculateCutoffRadius ();

  /// Compute attenuation vector from current attenuation mode.
  void CalculateAttenuationVector ();

  /// List of all sectors that this light shines in.
  csRefArrayObject<iSector> sectors;

  /// If we're currently removing a light from a sector.
  bool removingLight;

  csBox3 lightBoundingBox, worldBoundingBox;

  csEngine* engine;

protected:
  void InternalRemove() { SelfDestruct(); }

  csShaderVariable* GetPropertySV (csLightShaderVarCache::LightProperty prop);
public:
  /// Set of flags
  csFlags flags;

public:
  /**
   * Construct a light at a given position. With
   * a given radius, a given color, a given name and
   * type. The light will not have a halo by default.
   */
  csLight (csEngine* engine, float x, float y, float z, float dist,
     float red, float green, float blue, csLightDynamicType dyntype);

  /**
   * Destroy the light. Note that destroying a light
   * may not have the expected effect. Static lights result
   * in changes in the lightmaps. Removing them will not automatically
   * update those lightmaps as that is a time-consuming process.
   */
  virtual ~csLight ();

  csLightDynamicType GetDynamicType () const { return dynamicType; }

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
   * Get the current full sector (including parents).
   */
  iSector* GetFullSector ();

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
   * Get the full center position (correctly accounting for
   * parenting)..
   */
  const csVector3 GetFullCenter () const
  { 
    return movable.csMovable::GetFullPosition (); 
  }
  
  /**
   * Get the movable 
   */
  virtual iMovable *GetMovable ()
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
  {
    userSpecular = true; 
    specularColor = col; 
    GetPropertySV (csLightShaderVarCache::lightSpecular)->SetValue (col);
  }

  /**
   * Return the associated halo
   */
  iBaseHalo* GetHalo () const { return halo; }

  /**
   * Set the halo associated with this light.
   */
  void SetHalo (csHalo *Halo);

  /**
   * Get the light's attenuation type
   */
  csLightAttenuationMode GetAttenuationMode () const
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
  void SetAttenuationConstants (const csVector4& constants);
  /**
  * Get attenuation constants
  * \sa csLightAttenuationMode
  */
  const csVector4 &GetAttenuationConstants () const
  { return attenuationConstants; }

  /**
  * Get the the maximum distance at which the light is guaranteed to shine.
  * Can be seen as the distance at which we turn the light off.
  * Used for culling and selection of meshes to light, but not
  * for the lighting itself.
  */
  float GetCutoffDistance () const
  { return cutoffDistance; }

  /**
  * Set the the maximum distance at which the light is guaranteed to shine. 
  * Can be seen as the distance at which we turn the light off.
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
    userDirectionalCutoffRadius = true;
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
    GetPropertySV (csLightShaderVarCache::lightInnerFalloff)->SetValue (inner);
    GetPropertySV (csLightShaderVarCache::lightOuterFalloff)->SetValue (outer);
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
  float GetBrightnessAtDistance (float d) const;

  /**
   * Get the brightness of a light at the given distance. This
   * function correlates for the color too so it can be used to calculate
   * intensity.
   */
  float GetLuminanceAtSquaredDistance (float sqdist) const;

  /// Get the light type of this light.
  csLightType GetType () const
  { return type; }
  /// Set the light type of this light.
  void SetType (csLightType type)
  {
    this->type = type;
  }

  virtual iShaderVariableContext* GetSVContext()
  {
    return (iShaderVariableContext*)this;
  }

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
    return (int)light_cb_vector.GetSize ();
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

  virtual iObject *QueryObject() { return this; }
  virtual iSceneNode* QuerySceneNode () { return this; }
  csLight* GetPrivateObject () { return this; }
  
  /**\name iSceneNode implementation
   * @{ */
  virtual iMovable* GetMovable () const
  {
    return &movable;
  }

  virtual void SetParent (iSceneNode* parent);

  virtual iSceneNode* GetParent () const
  {
    if (movable.GetParent ())
      return movable.GetParent ()->GetSceneNode ();
    else
      return 0;
  }
  virtual const csRefArray<iSceneNode>& GetChildren () const
  {
    return movable.GetChildren ();
  }
  virtual csPtr<iSceneNodeArray> GetChildrenArray () const
  {
    return csPtr<iSceneNodeArray> (
      new scfArrayWrapConst<iSceneNodeArray, csRefArray<iSceneNode> > (
      movable.GetChildren ()));
  }
  virtual iMeshWrapper* QueryMesh () { return 0; }
  virtual iLight* QueryLight () { return this; }
  virtual iCamera* QueryCamera () { return 0; }
  /** @} */

  //--------------------- iSelfDestruct implementation -------------------//

  virtual void SelfDestruct ();

  //------------------------ iLight interface -----------------------------
  
  /// iLight implementation
  virtual iCrossHalo* CreateCrossHalo (float intensity, float cross);
  virtual iNovaHalo* CreateNovaHalo (int seed, int num_spokes,
    float roundness);
  virtual iFlareHalo* CreateFlareHalo ();

  virtual csFlags& GetFlags ()
  { return flags; }

  virtual uint32 GetLightNumber () const
  { return lightnr; }

  virtual const csBox3& GetLocalBBox () const
  {
    return lightBoundingBox;
  }
  const csBox3& GetWorldBBox () const
  { return worldBoundingBox; }

  csBox3 GetBBox () const;

  void UpdateBBox ();
};

#include "csutil/deprecated_warn_on.h"

/**
 * List of lights for a sector. This class implements iLightList.
 */
class csLightList : public scfImplementation1<csLightList,
                                              iLightList>
{
private:
  csRefArrayObject<iLight> list;
  csHash<iLight*, csString> lights_hash;

  class NameChangeListener : public scfImplementation1<NameChangeListener,
  	iObjectNameChangeListener>
  {
  private:
    csWeakRef<csLightList> list;

  public:
    NameChangeListener (csLightList* list) : scfImplementationType (this),
  	  list (list)
    {
    }
    virtual ~NameChangeListener () { }

    virtual void NameChanged (iObject* obj, const char* oldname,
  	  const char* newname)
    {
      if (list)
        list->NameChanged (obj, oldname, newname);
    }
  };
  csRef<NameChangeListener> listener;

public:

  void NameChanged (iObject* object, const char* oldname,
  	const char* newname);

  /// constructor
  csLightList ();
  virtual ~csLightList ();

  /// Override PrepareLight
  virtual void PrepareLight (iLight*) { }
  /// Override FreeLight
  virtual void FreeLight (iLight*) { }

  virtual int GetCount () const { return (int)list.GetSize (); }
  virtual iLight *Get (int n) const { return list.Get (n); }
  virtual int Add (iLight *obj);
  virtual bool Remove (iLight *obj);
  virtual bool Remove (int n);
  virtual void RemoveAll ();
  virtual int Find (iLight *obj) const;
  virtual iLight *FindByName (const char *Name) const;
  virtual iLight *FindByID (const char* id) const;
};

class LightAttenuationTextureAccessor :
  public scfImplementation1<LightAttenuationTextureAccessor ,
                            iShaderVariableAccessor>
{
  csEngine* engine;
  void CreateAttenuationTexture ();
public:
  csRef<iTextureHandle> attTex;

  LightAttenuationTextureAccessor (csEngine* engine);
  virtual ~LightAttenuationTextureAccessor ();

  virtual void PreGetValue (csShaderVariable *variable);
};

}
CS_PLUGIN_NAMESPACE_END(Engine)

using CS_PLUGIN_NAMESPACE_NAME(Engine)::csLightList;

#endif // __CS_LIGHT_H__

