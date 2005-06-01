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

#ifndef __CS_LVIEW_H__
#define __CS_LVIEW_H__

#include "csgeom/math3d.h"
#include "csgeom/frustum.h"
#include "csgeom/box.h"
#include "csutil/refarr.h"
#include "iengine/shadows.h"
#include "iengine/fview.h"

class csMatrix3;
class csVector3;
class csLight;
class csFrustumView;
struct csFog;
struct iGraphics3D;
struct iGraphics2D;
struct iSector;
struct iClipper2D;

/**
 * This class is a csFrustum especially used for the lighting calculations.
 * It represents a shadow. It extends csFrustum by adding the notion of
 * a 'shadow' originator.
 */
class csShadowFrustum: public csFrustum
{
private:
  void* userData;
  bool relevant;
public:
  /// Create empty frustum.
  csShadowFrustum () :
    csFrustum (csVector3 (0)),
    userData (0), relevant (false) { }
  /// Create empty frustum.
  csShadowFrustum (const csVector3& origin) :
    csFrustum (origin),
    userData (0), relevant (false) { }
  /// Create empty frustum.
  csShadowFrustum (const csVector3& origin, int num_verts) :
    csFrustum (origin, num_verts),
    userData (0), relevant (false) { }
  /// Copy constructor.
  csShadowFrustum (const csShadowFrustum& orig);
  /// Set user data.
  void SetUserData (void* ud) { userData = ud; }
  /// Get user data.
  void* GetUserData () { return userData; }
  /// Mark shadow as relevant or not.
  void MarkRelevant (bool rel = true) { relevant = rel; }
  /// Is shadow relevant?
  // @@@ Temporary problem: nothing is calling MarkRelevant() which
  // means that shadow casting through portals doesn't work. That's
  // why we return true here to fix that.
  bool IsRelevant () { return true; /*@@@ relevant; */ }
};

class csShadowBlockList;
class csShadowBlock;

/**
 * An iterator to iterate over a list of shadows.
 * This iterator can work in two directions and also supports
 * deleting the current element in the iterator.
 */
class csShadowIterator : public iShadowIterator
{
  friend class csShadowBlockList;
  friend class csShadowBlock;

private:
  csShadowBlock* first_cur;
  csShadowBlock* cur;
  int i, cur_num;
  bool onlycur;
  int dir;  // 1 or -1 for direction.
  csBox3 bbox;  // If use_bbox is true only iterate over relevant shadow blocks.
  bool use_bbox;
  csShadowIterator (csShadowBlock* cur, bool onlycur, int dir);
  csShadowIterator (const csBox3& bbox, csShadowBlock* cur,
    bool onlycur, int dir);
  virtual ~csShadowIterator();
  csShadowFrustum* cur_shad;

public:
  /// Return true if there are further elements to process.
  virtual bool HasNext ();
  /// Return the next element.
  virtual csFrustum* Next ();
  /// Get the user data for the last shadow.
  virtual void* GetUserData () { return cur_shad->GetUserData (); }
  /// Return if the last shadow is relevant or not.
  virtual bool IsRelevant () { return true; /*cur_shad->IsRelevant ();*/ }
  /// Mark the last shadow as relevant.
  virtual void MarkRelevant (bool rel) { cur_shad->MarkRelevant (rel); }
  /// Reset the iterator to start again from initial setup.
  virtual void Reset ();
  /// Delete the last element returned.
  virtual void DeleteCurrent ();
  /// Return the shadow list for the 'current' element.
  virtual iShadowBlock* GetCurrentShadowBlock ();
  /// Return the shadow list for the 'next' element.
  virtual iShadowBlock* GetNextShadowBlock ();
  /// Return the shadow list for the 'current' element.
  csShadowBlock* GetCsCurrentShadowBlock ();
  /// Return the shadow list for the 'next' element.
  csShadowBlock* GetCsNextShadowBlock () { return cur; }

  SCF_DECLARE_IBASE;
};

/**
 * A single block of shadows. This block will use IncRef()/DecRef()
 * on the shadow frustums so that it is possible and legal to put a single
 * shadow in several blocks.
 */
class csShadowBlock : public iShadowBlock
{
  friend class csShadowBlockList;
  friend class csShadowIterator;

private:
  csShadowBlock* next, * prev;
  csRefArray<csShadowFrustum> shadows;
  uint32 shadow_region;
  csBox3 bbox;  // The bbox (in light space) for all shadows in this block.
  bool bbox_valid;  // If true bbox is valid.

  void IntAddShadow (csShadowFrustum* csf);

public:
  /// Create a new empty list.
  csShadowBlock (uint32 region = (uint32)~0, int max_shadows = 30,
    int delta = 30);

  /// Destroy the list and release all shadow references.
  virtual ~csShadowBlock ();

  /// Dereference all shadows in the list.
  virtual void DeleteShadows ()
  {
    shadows.DeleteAll ();
    bbox_valid = false;
  }

  /// Get the bounding box of this shadow block.
  virtual const csBox3& GetBoundingBox ();

  /**
   * Copy all relevant shadow frustums from another shadow block
   * into this block. The frustums are not really copied but a new
   * reference is kept. However, if a transformation is given then
   * a copy is made and the shadows are transformed.
   */
  void AddRelevantShadows (csShadowBlock* source, csTransform* trans = 0);

  /**
   * Copy all relevant shadow frustums from another shadow block
   * into this block. The frustums are not really copied but a new
   * reference is kept. However, if a transformation is given then
   * a copy is made and the shadows are transformed.
   */
  virtual void AddRelevantShadows (iShadowBlock* source,
    csTransform* trans = 0);

  /**
   * Copy all relevant shadow frustums from another shadow block list
   * into this block. The frustums are not really copied but a new
   * reference is kept.
   */
  void AddRelevantShadows (csShadowBlockList* source);

  /**
   * Copy all relevant shadow frustums from another shadow block list
   * into this block. The frustums are not really copied but a new
   * reference is kept.
   */
  virtual void AddRelevantShadows (iShadowBlockList* source);

  /**
   * Copy all shadow frustums from another shadow block list
   * into this block. The frustums are not really copied but a new
   * reference is kept.
   */
  void AddAllShadows (csShadowBlockList* source);

  /**
   * Copy all shadow frustums from another shadow block list
   * into this block. The frustums are not really copied but a new
   * reference is kept.
   */
  virtual void AddAllShadows (iShadowBlockList* source);

  /**
   * Add unique shadows. Only add relevant shadow frustums that are not
   * already in the current list. The frustums are not really copied
   * but a new reference is kept.
   */
  void AddUniqueRelevantShadows (csShadowBlockList* source);

  /**
   * Add unique shadows. Only add relevant shadow frustums that are not
   * already in the current list. The frustums are not really copied
   * but a new reference is kept.
   */
  virtual void AddUniqueRelevantShadows (iShadowBlockList* source);

  /**
   * Add a new frustum and return a reference.
   * The frustum will have the specified number of vertices but the
   * vertices still need to be initialized.
   */
  virtual csFrustum* AddShadow (const csVector3& origin, void* userData,
    int num_verts, csPlane3& backplane);

  /// Unlink a shadow frustum from the list and dereference it.
  virtual void UnlinkShadow (int idx);

  /// Get the number of shadows in this list.
  virtual int GetShadowCount ()
  {
    return (int)shadows.Length ();
  }

  /// Get the specified shadow.
  csFrustum* GetShadow (int idx)
  {
    return ((size_t)idx < shadows.Length () ? (csFrustum*)shadows[idx] : 0);
  }

  /**
   * Apply a transformation to all frustums in this list.
   */
  void Transform (csTransform* trans)
  {
    size_t i;
    for (i = 0 ; i < shadows.Length () ; i++)
    {
      csShadowFrustum* sf = shadows[i];
      CS_ASSERT (sf != 0);
      sf->Transform (trans);
    }
    bbox_valid = false;
  }

  /// Get iterator to iterate over all shadows in this block.
  csShadowIterator* GetCsShadowIterator (bool reverse = false)
  {
    return new csShadowIterator (this, true, reverse ? -1 : 1);
  }

  /// Get iterator to iterate over all shadows in this block.
  iShadowIterator* GetShadowIterator (bool reverse = false)
  {
    return (iShadowIterator*)(new csShadowIterator (this, true,
      reverse ? -1 : 1));
  }

  /// Get the region for this shadow block.
  uint32 GetShadowRegion () const { return shadow_region; }

  SCF_DECLARE_IBASE;
};

/**
 * A list of shadow blocks.
 */
class csShadowBlockList : public iShadowBlockList
{
private:
  csShadowBlock* first;
  csShadowBlock* last;
  uint32 cur_shadow_region;

public:
  /// Create a new empty list.
  csShadowBlockList ();
  /// Destroy the list and all shadow blocks in it.
  virtual ~csShadowBlockList ();

  /// Create a new shadow block and append to the list.
  virtual iShadowBlock* NewShadowBlock (int num_shadows = 30);

  /// Append a shadow block to this list.
  void AppendShadowBlock (csShadowBlock* slist)
  {
    CS_ASSERT (slist->prev == 0 && slist->next == 0);
    CS_ASSERT ((!!first) == (!!last));
    slist->next = 0;
    if (!last)
    {
      first = last = slist;
      slist->prev = 0;
    }
    else
    {
      slist->prev = last;
      last->next = slist;
      last = slist;
    }
  }

  /// Remove the last shadow block from this list.
  virtual void RemoveLastShadowBlock ()
  {
    CS_ASSERT ((!!first) == (!!last));
    if (last)
    {
      CS_ASSERT (last->next == 0);
      CS_ASSERT (first->prev == 0);
      csShadowBlock* old = last;
      last = old->prev;
      if (last) last->next = 0;
      else first = 0;
      old->prev = old->next = 0;
    }
  }

  /// Clear first and last pointers without deleting anything!
  void Clear ()
  {
    CS_ASSERT ((!!first) == (!!last));
#   ifdef CS_DEBUG
    // If we are in debug mode then we additionally set all next/prev
    // fields in the list to 0 so that our assert's above will work.
    while (first)
    {
      csShadowBlock* old = first;
      first = old->next;
      old->prev = old->next = 0;
    }
#   endif
    last = 0;
  }

  /// Destroy all shadow lists and shadows in the list.
  virtual void DeleteAllShadows ()
  {
    CS_ASSERT ((!!first) == (!!last));
    while (first)
    {
      first->DeleteShadows ();
      csShadowBlock* todel = first;
      first = first->next;
      delete todel;
    }
    last = 0;
  }

  virtual iShadowBlock* GetFirstShadowBlock () { return (iShadowBlock*)first; }
  virtual iShadowBlock* GetLastShadowBlock () { return (iShadowBlock*)last; }
  virtual iShadowBlock* GetNextShadowBlock (iShadowBlock* s)
  {
    return (iShadowBlock*)(((csShadowBlock*)s)->next);
  }
  virtual iShadowBlock* GetPreviousShadowBlock (iShadowBlock* s)
  {
    return (iShadowBlock*)(((csShadowBlock*)s)->prev);
  }

  /**
   * Return an iterator to iterate over all shadows in this list.
   */
  csShadowIterator* GetCsShadowIterator (bool reverse = false)
  {
    return new csShadowIterator (first, false, reverse ? -1 : 1);
  }

  /**
   * Return an iterator to iterate over all shadows in this list.
   */
  virtual iShadowIterator* GetShadowIterator (bool reverse = false)
  {
    return (iShadowIterator*)(new csShadowIterator (first, false,
      reverse ? -1 : 1));
  }
  virtual iShadowIterator* GetShadowIterator (
    const csBox3& bbox, bool reverse = false)
  {
    return (iShadowIterator*)(new csShadowIterator (bbox, first, false,
      reverse ? -1 : 1));
  }

  virtual uint32 MarkNewRegion ()
  {
    cur_shadow_region++;
    return cur_shadow_region-1;
  }

  virtual void RestoreRegion (uint32 prev)
  {
    cur_shadow_region = prev;
  }

  virtual bool FromCurrentRegion (iShadowBlock* block)
  {
    return ((csShadowBlock*)block)->GetShadowRegion () == cur_shadow_region;
  }

  SCF_DECLARE_IBASE;
};

/**
 * This structure represents all information needed for the frustum
 * visibility calculator.
 */
class csFrustumView : public iFrustumView
{
private:
  /// A function that is called for every node that is visited.
  csFrustumViewObjectFunc* object_func;
  /// User data for the entire process.
  csRef<iFrustumViewUserdata> userdata;

  /// Radius we want to check.
  float radius;

  /// Squared radius.
  float sq_radius;

  /// If true the we process shadows for things.
  bool things_shadow;

  /**
   * Mask and value which will be checked against the flags of every
   * encountered thing to see if it will be included in the shadow
   * processing.
   */
  unsigned int shadow_thing_mask, shadow_thing_value;
  /**
   * Mask and value which will be checked against the flags of every
   * encountered thing to see if CheckFrustum must recursively call
   * itself for this thing.
   */
  unsigned int process_thing_mask, process_thing_value;

  /// Current frustum context.
  csFrustumContext* ctxt;

public:
  /// Constructor.
  csFrustumView ();

  /// Destroy the object
  virtual ~csFrustumView ();

  /// Get the current frustum context.
  virtual csFrustumContext* GetFrustumContext () const { return ctxt; }
  /// Create a new frustum context.
  virtual void CreateFrustumContext ();
  /// Create a copy.
  virtual csFrustumContext* CopyFrustumContext ();
  /// Set the frustum context.
  virtual void SetFrustumContext (csFrustumContext* ctxt);
  /// Restore a frustum context.
  virtual void RestoreFrustumContext (csFrustumContext* original);

  /// Start new shadow list for this frustum.
  virtual void StartNewShadowBlock ();

  /// Set the function that is called for every object.
  virtual void SetObjectFunction (csFrustumViewObjectFunc* func)
  {
    object_func = func;
  }
  /// Call the object function.
  virtual void CallObjectFunction (iMeshWrapper* mesh, bool vis)
  {
    if (object_func) object_func (mesh, this, vis);
  }
  /// Set the maximum radius to use for visiting objects.
  void SetRadius (float rad)
  {
    radius = rad;
    sq_radius = rad*rad;
  }
  /// Get the radius.
  virtual float GetRadius () const { return radius; }
  /// Get the squared radius.
  virtual float GetSquaredRadius () const { return sq_radius; }
  /// Enable shadowing for things (off by default). @@@SUSPECT!!!
  void EnableThingShadows (bool e) { things_shadow = e; }
  /// Return true if shadowing for things is enabled.
  virtual bool ThingShadowsEnabled () { return things_shadow; }
  /// Set shadow mask.
  void SetShadowMask (unsigned int mask, unsigned int value)
  {
    shadow_thing_mask = mask;
    shadow_thing_value = value;
  }
  /// Set process mask.
  void SetProcessMask (unsigned int mask, unsigned int value)
  {
    process_thing_mask = mask;
    process_thing_value = value;
  }
  /// Check if a mask corresponds with the shadow mask.
  virtual bool CheckShadowMask (unsigned int mask)
  {
    return ((mask & shadow_thing_mask) == shadow_thing_value);
  }
  /// Check if a mask corresponds with the process mask.
  virtual bool CheckProcessMask (unsigned int mask)
  {
    return ((mask & process_thing_mask) == process_thing_value);
  }

  /// Set or clear userdata.
  virtual void SetUserdata (iFrustumViewUserdata* data)
  {
    userdata = data;
  }
  /// Get userdata.
  virtual iFrustumViewUserdata* GetUserdata ()
  {
    return userdata;
  }
  virtual csPtr<iShadowBlock> CreateShadowBlock ()
  {
    return csPtr<iShadowBlock> (new csShadowBlock ());
  }
  SCF_DECLARE_IBASE;
};

#endif // __CS_LVIEW_H__

