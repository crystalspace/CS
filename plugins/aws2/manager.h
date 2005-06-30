/*
    Copyright (C) 2005 by Christopher Nelson

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

#ifndef __AWS_MANAGER_H__
#define __AWS_MANAGER_H__

#include "iaws/aws2.h"
#include "preferences.h"
#include "csgeom/csrectrg.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"

/** The management object for AWS: creates windows, destroys windows, keeps track of all the windows, etc. */
class awsManager : public iAws
{
  /** Contains all loaded preferences, skins, etc. */
  aws::preferences prefs;

  /** Store the object registry so we can get at it later. */
  iObjectRegistry *object_reg;

   /**
   * This is the dirty region.  All clean/dirty code now utilizes the
   * update region facility for non-contiguous rectangular spaces.  This
   * buffer holds an infinite amount of optimal rectangular regions.
   */
  csRectRegion dirty;

  /**
   * This is the erase region.  All windows will call the Erase() function
   * if the AlwaysEraseWindows flag is set.  That function will add a rect
   * into this region which requires erasure.  Right before final redraw, all
   * dirty regions will be excluded from the erasure region, and the erasure
   * region will be painted with the transparent color.
   */
  csRectRegion erase;

  /**
   * This is the update store.  The update store contains all of the regions
   * that actually contain anything useful, and thus the only regions that
   * need to be thrown to the screen.  The store must be cleared and rethrown
   * during window move operations.
   */
  csRectRegion updatestore;

  /// True if the update store needs to be cleared and updated.
  bool updatestore_dirty;

public:
  /////////////////////// Accessors //////////////////////////////

  /// Get the iObjectRegistry interface so that components can use it.
  virtual iObjectRegistry *GetObjectRegistry () { return object_reg; }

public:
  SCF_DECLARE_IBASE;

  awsManager(iBase *the_base);
  virtual ~awsManager();

  /** Initializes the manager. Must be called before anything else. */
  bool Initialize (iObjectRegistry *_object_reg);


public:
 //////////////////////// Event Handling ////////////////////////

  /// Dispatches events to the proper components.
  virtual bool HandleEvent (iEvent &);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(awsManager);
    virtual bool Initialize (iObjectRegistry *p)
    {
      return scfParent->Initialize (p);
    }
  } scfiComponent;

  struct EventHandler : public iEventHandler
  {
  private:
    awsManager *parent;
  public:
    SCF_DECLARE_IBASE;
    EventHandler (awsManager * parent)
    {
      SCF_CONSTRUCT_IBASE (0);
      EventHandler::parent = parent;
    }
    virtual ~EventHandler ()
    {
      SCF_DESTRUCT_IBASE ();
    }
    virtual bool HandleEvent (iEvent &) { return false; }
  } *scfiEventHandler;


};


#endif
