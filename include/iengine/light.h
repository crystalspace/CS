/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein
    Copyright (C) 1999 by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __IENGINE_LIGHT_H__
#define __IENGINE_LIGHT_H__

#include "csutil/scf.h"
#include "iengine/fview.h"

class csLight;
class csColor;
struct iSector;
struct iObject;
struct iCrossHalo;
struct iNovaHalo;
struct iFlareHalo;

/// Light level that is used when there is no light on the texture.
#define CS_DEFAULT_LIGHT_LEVEL 20
/// Light level that corresponds to a normally lit texture.
#define CS_NORMAL_LIGHT_LEVEL 128

/**
 * Attenuation controls how the brightness of a light fades with distance.
 * There are four attenuation formulas:
 * <ul>
 *   <li> no attenuation = light * 1
 *   <li> linear attenuation = light * (radius - distance) / radius
 *   <li> inverse attenuation = light * (radius / distance)
 *   <li> realistic attenuation = light * (radius^2 / distance^2)
 * </ul>
 */
#define CS_ATTN_NONE      0
#define CS_ATTN_LINEAR    1
#define CS_ATTN_INVERSE   2
#define CS_ATTN_REALISTIC 3

SCF_VERSION (iLight, 0, 0, 6);

/**
 * The iLight interface is the SCF interface for the csLight class.
 */
struct iLight : public iBase
{
  /// Get private pointer to light object. UGLY
  virtual csLight* GetPrivateObject () = 0;

  /// Get the id of this light.
  virtual unsigned long GetLightID () = 0;

  /// Get the iObject for this light.
  virtual iObject *QueryObject() = 0;

  /// Get the position of this light.
  virtual const csVector3& GetCenter () = 0;
  /// Set the position of this light.
  virtual void SetCenter (const csVector3& pos) = 0;

  /// Get the sector for this light.
  virtual iSector *GetSector () = 0;
  /// Set the sector for this light.
  virtual void SetSector (iSector* sector) = 0;

  /// Get the radius
  virtual float GetRadius () = 0;
  /// Get the squared radius.
  virtual float GetSquaredRadius () = 0;
  /// Get the inverse radius.
  virtual float GetInverseRadius () = 0;
  /// Set the radius
  virtual void SetRadius (float r) = 0;

  /// Get the color of this light.
  virtual const csColor& GetColor () = 0;
  /// Set the color of this light.
  virtual void SetColor (const csColor& col) = 0;

  /// Return current attenuation mode.
  virtual int GetAttenuation () = 0;
  /**
   * Set attenuation mode. The following values are possible (CS_ATTN_LINEAR
   * is default CS_ATTN_LINEAR):
   * <ul>
   * <li> CS_ATTN_NONE: light * 1
   * <li> CS_ATTN_LINEAR: light * (radius - distance) / radius
   * <li> CS_ATTN_INVERSE: light * (radius / distance)
   * <li> CS_ATTN_REALISTIC: light * (radius^2 / distance^2)
   * </ul>
   */
  virtual void SetAttenuation (int a) = 0;

  /// Create a cross halo for this light.
  virtual iCrossHalo* CreateCrossHalo (float intensity, float cross) = 0;
  /// Create a nova halo for this light.
  virtual iNovaHalo* CreateNovaHalo (int seed, int num_spokes,
  	float roundness) = 0;
  /// Create a flare halo for this light.
  virtual iFlareHalo* CreateFlareHalo () = 0;

  /// Get the brightness of a light at a given distance.
  virtual float GetBrightnessAtDistance (float d) = 0;
};

SCF_VERSION (iLightList, 0, 0, 1);

/**
 * This structure represents a list of lights.
 */
struct iLightList : public iBase
{
  /// Return the number of lights in this list.
  virtual int GetCount () const = 0;

  /// Return a light by index.
  virtual iLight *Get (int n) const = 0;

  /// Add a light.
  virtual int Add (iLight *obj) = 0;

  /// Remove a light.
  virtual bool Remove (iLight *obj) = 0;

  /// Remove the nth light.
  virtual bool Remove (int n) = 0;

  /// Remove all lights.
  virtual void RemoveAll () = 0;

  /// Find a light and return its index.
  virtual int Find (iLight *obj) const = 0;

  /// Find a light by name.
  virtual iLight *FindByName (const char *Name) const = 0;

  /// Find a light by its ID value.
  virtual iLight *FindByID (unsigned long id) const = 0;
};

SCF_VERSION (iLightingProcessInfo, 0, 0, 1);

/**
 * The iLightingProcessInfo interface holds information for the lighting
 * system. You can query the userdata from iFrustumView for this interface
 * while in a 'portal' callback. This way you can get specific information
 * from the lighting system for your null-portal.
 */
struct iLightingProcessInfo : public iFrustumViewUserdata
{
  /// Get the light.
  virtual iLight* GetLight () const = 0;

  /// Get gouraud only state.
  virtual bool GetGouraudOnly () const = 0;

  /// Return true if dynamic.
  virtual bool IsDynamic () const = 0;

  /// Set the current color.
  virtual void SetColor (const csColor& col) = 0;

  /// Get the current color.
  virtual const csColor& GetColor () const = 0;
};

#endif // __IENGINE_LIGHT_H__

