/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein

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

#ifndef __CS_IENGINE_FVIEW_H__
#define __CS_IENGINE_FVIEW_H__

/**\file
 */
/**
 * \addtogroup engine3d_vis
 * @{ */
 
#include "csutil/ref.h"
#include "csutil/scf.h"

#include "csgeom/frustum.h"

#include "iengine/shadows.h"


struct iFrustumView;
struct iMeshWrapper;
class csFrustum;
class csFrustumContext;

/**
 * User data which can be attached to iFrustumView.
 */
struct iFrustumViewUserdata : public virtual iBase
{
  SCF_INTERFACE(iFrustumViewUserdata,2,0,0);
};

/**
 * Function that will be called for every visited object (during
 * CastShadows() procedure from the visibility culler).
 */
typedef void (csFrustumViewObjectFunc)(iMeshWrapper* mesh,
  iFrustumView* lview, bool vis);

/**
 * This structure keeps track of the current frustum context.
 * It is used by iFrustumView. When recursing through a portal
 * a new frustum context will be created and set in place of the
 * old one.
 */
class csFrustumContext
{
private:
  /**
   * The list of shadow frustums. Note that this list will be
   * expanded with every traversal through a portal but it needs
   * to be restored to original state again before returning.
   */
  csRef<iShadowBlockList> shadows;
  /**
   * This flag is true if the list of shadows is shared with some
   * other csFrustumView.
   */
  bool shared;

  /// True if space is mirrored.
  bool mirror;

  /**
   * The frustum for the light. Everthing that falls in this frustum
   * is lit unless it also is in a shadow frustum.
   */
  csRef<csFrustum> light_frustum;

public:
  /// Constructor.
  csFrustumContext () :
    shared (false),
    mirror (false)
  { }

  csFrustumContext& operator= (csFrustumContext const& c)
  {
    shadows = c.shadows;
    shared = c.shared;
    mirror = c.mirror;
    light_frustum = c.light_frustum;
    return *this;
  }

  /// Get the list of shadows.
  iShadowBlockList* GetShadows () { return shadows; }
  /// Set the list of shadows.
  void SetShadows (iShadowBlockList* shad, bool sh = true)
  {
    shadows = shad;
    shared = sh;
  }
  /// Set a new (already IncRef'd) list of shadows.
  void SetNewShadows (csPtr<iShadowBlockList> shad, bool sh = false)
  {
    shadows = shad;
    shared = sh;
  }
  /// Get shared.
  bool IsShared () { return shared; }

  /// Set the light frustum.
  void SetLightFrustum (csFrustum* lf) { light_frustum = lf; }
  /// Set a new (already IncRef'd) light frustrum
  void SetNewLightFrustum (csPtr<csFrustum> lf) { light_frustum = lf; }
  /// Get the light frustum.
  csFrustum* GetLightFrustum () { return light_frustum; }

  /**
   * Set/Disable mirrored space (default false).
   * Set this to true if the frustum starts in mirrored space.
   */
  void SetMirrored (bool m) { mirror = m; }
  /// Is mirrored.
  bool IsMirrored () { return mirror; }
};


/**
 * This structure represents all information needed for the frustum
 * visibility calculator.
 */
struct iFrustumView : public virtual iBase
{
  SCF_INTERFACE(iFrustumView,2,0,0);

  /// Get the current frustum context.
  virtual csFrustumContext* GetFrustumContext () const = 0;
  /**
   * Create a new frustum context. This is typically used
   * when going through a portal. Note that you should remember
   * the old frustum context if you want to restore it later.
   * The frustum context will get all the values from the current context
   * (with SCF references properly incremented).
   */
  virtual void CreateFrustumContext () = 0;
  /**
   * Create a copy of the current frustum context and return it. This
   * can be used to later put it back. Use SetFrustumContext() for this.
   */
  virtual csFrustumContext* CopyFrustumContext () = 0;
  /**
   * This function is similar to CreateFrustumContext() but it sets the
   * given frustum context instead. Also restore with RestoreFrustumContext().
   */
  virtual void SetFrustumContext (csFrustumContext* ctxt) = 0;
  /**
   * Restore a frustum context. Use this to restore a previously overwritten
   * frustum context. This function will take care of properly cleaning
   * up the current frustum context.
   */
  virtual void RestoreFrustumContext (csFrustumContext* original) = 0;

  /// Set the object function.
  virtual void SetObjectFunction (csFrustumViewObjectFunc* func) = 0;
  /// Call the object function.
  virtual void CallObjectFunction (iMeshWrapper* mesh, bool vis) = 0;

  /// Get the radius.
  virtual float GetRadius () const = 0;
  /// Get the squared radius.
  virtual float GetSquaredRadius () const = 0;
  /// Return true if shadowing for things is enabled.
  virtual bool ThingShadowsEnabled () = 0;
  /// Check if a mask corresponds with the shadow mask.
  virtual bool CheckShadowMask (unsigned int mask) = 0;
  /// Check if a mask corresponds with the process mask.
  virtual bool CheckProcessMask (unsigned int mask) = 0;

  /// Start new shadow list for this frustum.
  virtual void StartNewShadowBlock () = 0;

  /// Set or clear userdata.
  virtual void SetUserdata (iFrustumViewUserdata* data) = 0;
  /// Get userdata.
  virtual iFrustumViewUserdata* GetUserdata () = 0;

  /// Create a new empty shadow block.
  virtual csPtr<iShadowBlock> CreateShadowBlock () = 0;
};

/** @} */

#endif // __CS_IENGINE_FVIEW_H__

