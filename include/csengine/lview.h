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
#include "csutil/csvector.h"
#include "iengine/shadows.h"

class csMatrix3;
class csVector3;
class csLight;
class csFrustumView;
class csClipper;
struct csFog;
struct iGraphics3D;
struct iGraphics2D;
struct iPolygon3D;
struct iSector;
struct iClipper2D;

/// A callback function for csLight::LightingFunc().
typedef void (csLightingFunc) (csFrustumView* lview, int type, void* entity);

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
    csFrustum (csVector3 (0), &csPooledVertexArrayPool::GetDefaultPool()),
    userData (NULL) { }
  /// Create empty frustum.
  csShadowFrustum (const csVector3& origin) :
    csFrustum (origin, &csPooledVertexArrayPool::GetDefaultPool()),
    userData (NULL) { }
  /// Create empty frustum.
  csShadowFrustum (const csVector3& origin, int num_verts) :
    csFrustum (origin, num_verts, &csPooledVertexArrayPool::GetDefaultPool()),
    userData (NULL) { }
  /// Copy constructor.
  csShadowFrustum (const csShadowFrustum& orig);
  /// Set user data.
  void SetUserData (void* ud) { userData = ud; }
  /// Get user data.
  void* GetUserData () { return userData; }
  /// Mark shadow as relevant or not.
  void MarkRelevant (bool rel = true) { relevant = rel; }
  /// Is shadow relevant?
  bool IsRelevant () { return relevant; }
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
  int dir;	// 1 or -1 for direction.
  csShadowIterator (csShadowBlock* cur, bool onlycur, int dir);
  csShadowFrustum* cur_shad;

public:
  /// Return true if there are further elements to process.
  virtual bool HasNext ()
  {
    return cur != NULL && i < cur_num && i >= 0;
  }
  /// Return the next element.
  virtual csFrustum* Next ();
  /// Get the user data for the last shadow.
  virtual void* GetUserData () { return cur_shad->GetUserData (); }
  /// Return if the last shadow is relevant or not.
  virtual bool IsRelevant () { return cur_shad->IsRelevant (); }
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

  DECLARE_IBASE;
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
  csVector shadows;
  iSector* sector;
  int draw_busy;

public:
  /// Create a new empty list for a sector.
  csShadowBlock (iSector* sector, int draw_busy, int max_shadows = 30,
  	int delta = 30);
  /// Create a new empty list.
  csShadowBlock (int max_shadows = 30, int delta = 30);

  /// Destroy the list and release all shadow references.
  virtual ~csShadowBlock ();

  /// Dereference all shadows in the list.
  virtual void DeleteShadows ()
  {
    int i;
    for (i = 0 ; i < shadows.Length () ; i++)
    {
      csShadowFrustum* sf = (csShadowFrustum*)shadows[i];
      sf->DecRef ();
    }
    shadows.DeleteAll ();
  }

  /**
   * Copy all relevant shadow frustums from another shadow block
   * into this block. The frustums are not really copied but a new
   * reference is kept. However, if a transformation is given then
   * a copy is made and the shadows are transformed.
   */
  void AddRelevantShadows (csShadowBlock* source, csTransform* trans = NULL);
  
  /**
   * Copy all relevant shadow frustums from another shadow block
   * into this block. The frustums are not really copied but a new
   * reference is kept. However, if a transformation is given then
   * a copy is made and the shadows are transformed.
   */
  virtual void AddRelevantShadows (iShadowBlock* source,
  	csTransform* trans = NULL);
  
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
  virtual int GetNumShadows ()
  {
    return shadows.Length ();
  }

  /// Get the specified shadow.
  csFrustum* GetShadow (int idx)
  {
    return (csFrustum*)(
	(csShadowFrustum*)(idx < shadows.Length () ? shadows[idx] : NULL));
  }

  /**
   * Apply a transformation to all frustums in this list.
   */
  void Transform (csTransform* trans)
  {
    int i;
    for (i = 0 ; i < shadows.Length () ; i++)
      ((csShadowFrustum*)shadows[i])->Transform (trans);
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

  /// Get Sector.
  virtual iSector* GetSector () { return sector; }
  /// Get draw_busy for sector.
  virtual int GetRecLevel () { return draw_busy; }

  DECLARE_IBASE;
};

/**
 * A list of shadow blocks.
 */
class csShadowBlockList : public iShadowBlockList
{
private:
  csShadowBlock* first;
  csShadowBlock* last;

public:
  /// Create a new empty list.
  csShadowBlockList ();
  /// Destroy the list and all shadow blocks in it.
  virtual ~csShadowBlockList ()
  {
    DeleteAllShadows ();
  }

  /// Create a new shadow block and append to the list.
  virtual iShadowBlock* NewShadowBlock (iSector* sector,
  	int draw_busy, int num_shadows = 30);
  /// Create a new shadow block and append to the list.
  virtual iShadowBlock* NewShadowBlock ();

  /// Append a shadow block to this list.
  void AppendShadowBlock (csShadowBlock* slist)
  {
    slist->next = NULL;
    if (!last)
    {
      first = last = slist;
      slist->prev = NULL;
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
    if (last)
    {
      last = last->prev;
      if (last) last->next = NULL;
      else first = NULL;
    }
  }

  /// Clear first and last pointers without deleting anything!
  void Clear () { first = last = NULL; }

  /// Destroy all shadow lists and shadows in the list.
  virtual void DeleteAllShadows ()
  {
    while (first)
    {
      first->DeleteShadows ();
      csShadowBlock* todel = first;
      first = first->next;
      delete todel;
    }
    last = NULL;
  }

  csShadowBlock* GetCsFirstShadowBlock () { return first; }
  csShadowBlock* GetCsLastShadowBlock () { return last; }
  csShadowBlock* GetNextShadowBlock (csShadowBlock* s) { return s->next; }
  csShadowBlock* GetPreviousShadowBlock (csShadowBlock* s) { return s->prev; }
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

  DECLARE_IBASE;
};

class csFrustumView;
class csObject;
class csOctreeNode;
typedef void (csFrustumViewFunc)(csObject* obj, csFrustumView* lview);
typedef void (csFrustumViewNodeFunc)(csOctreeNode* node, csFrustumView* lview);

/**
 * The structure for registering cleanup actions.  You can register with any
 * frustumlist object any number of cleanup routines.  For this you create
 * such a structure and pass it to RegisterCleanup () method of csFrustumList.
 * You can derive a subclass from csFrustumViewCleanup and keep all
 * additional data there.
 */
struct csFrustumViewCleanup
{
  // Pointer to next cleanup action in chain
  csFrustumViewCleanup *next;
  // The routine that is called for cleanup
  void (*action) (csFrustumView *, csFrustumViewCleanup *);
};

/**
 * This structure represents all information needed for the frustum
 * visibility calculator.
 * @@@ This structure needs some cleanup. It contains too many
 * fields that are lighting related. These should probably go to
 * the 'userdata'.
 */
class csFrustumView
{
private:
  /**
   * The list of shadow frustums. Note that this list will be
   * expanded with every traversal through a portal but it needs
   * to be restored to original state again before returning.
   */
  csShadowBlockList* shadows;
  /**
   * This flag is true if the list of shadows is shared with some
   * other csFrustumView.
   */
  bool shared;

public:
  /// The head of cleanup actions
  csFrustumViewCleanup *cleanup;

  /// Data for the functions below.
  void* userdata;
  /// A function that is called for every node that is visited.
  csFrustumViewNodeFunc* node_func;
  /// A function that is called for every polygon that is hit.
  csFrustumViewFunc* poly_func;
  /// A function that is called for every curve that is hit.
  csFrustumViewFunc* curve_func;

  /**
   * The current color of the light. Initially this is the same as the
   * light in csStatLight but portals may change this.
   */
  float r, g, b;

  /// Radius we want to check.
  float radius;

  /// Squared radius.
  float sq_radius;

  /// If true the we process shadows for things.
  bool things_shadow;

  /// If space is mirrored.
  bool mirror;

  /**
   * If this structure is used for dynamic light frustum calculation
   * then this flag is true.
   */
  bool dynamic;

  /**
   * If only gouraud shading should be updated then this flag is true.
   */
  bool gouraud_only;

  /**
   * If 'true' then the gouraud vertices need to be initialized (set to
   * black) first. Only the parent PolygonSet of a polygon can know this
   * because it is calculated using the current_light_frame_number.
   */
  bool gouraud_color_reset;

  /**
   * The frustum for the light. Everthing that falls in this frustum
   * is lit unless it also is in a shadow frustum.
   */
  csFrustum* light_frustum;

  /**
   * A callback function. If this is set then no actual
   * lighting is done.
   * Instead the callback function is called.
   */
  csLightingFunc* callback;

  /// Userdata belonging to the callback.
  void* callback_data;

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

public:
  /// Constructor.
  csFrustumView ();
  /// Copy constructor. Everything is copied.
  csFrustumView (const csFrustumView &iCopy);

  /// Destroy the object
  ~csFrustumView ();

  /// Register a cleanup action to be called from destructor
  void RegisterCleanup (csFrustumViewCleanup *action)
  { action->next = cleanup; cleanup = action; }
  /// Deregister a cleanup action
  bool DeregisterCleanup (csFrustumViewCleanup *action);

  /// Start new shadow list for this frustum.
  void StartNewShadowBlock ();

  /// Get the list of shadows.
  iShadowBlockList* GetShadows () { return (iShadowBlockList*)shadows; }
};

#endif // __CS_LVIEW_H__
