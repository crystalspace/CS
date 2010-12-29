/*
  Copyright (C) 2005 by Adam D. Bradley <artdodge@cs.bu.edu>
  
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

#ifndef __CS_IUTIL_EVENTHANDLERS_H__
#define __CS_IUTIL_EVENTHANDLERS_H__

#include "csutil/hash.h"

/**\file
 * General event handler naming, name management, indexing, and instantiation interface.
 */
/**
 * \addtogroup event_handling
 * @{ */


typedef csStringID csHandlerID;
#define CS_HANDLER_INVALID csInvalidStringID
#define CS_HANDLERLIST_END csInvalidStringID

#ifndef CSHASHCOMPUTER_EVENTENGINE_IDS
#define CSHASHCOMPUTER_EVENTENGINE_IDS
template<>
class csHashComputer<const csHandlerID>
{
public:
  static uint ComputeHash (const csHandlerID &mid) 
  {
    return (uint) mid;
  }
};
#endif // CSHASHCOMPUTER_EVENTENGINE_IDS

struct iEventHandler;

/**
 * This interface represents a general event handler registry/resolver.
 *
 * An iHandlerRegistry maintains a one-to-one mapping from strings
 * to csHandlerIDs, and a one-to-(zero or one) mapping from csHandlerIDs to
 * iEventHandler pointers.
 */
struct iEventHandlerRegistry : public virtual iBase
{
  SCF_INTERFACE(iEventHandlerRegistry, 1, 3, 0);

  /**
   * Get a csHandlerID based upon some string.
   * This should only ever be done to reference generic 
   * (non-instantiated) handler names or single-instance handlers.
   */	
  virtual csHandlerID GetGenericID (const char*) = 0;
  virtual csHandlerID GetGenericPreBoundID (csHandlerID) = 0;
  virtual csHandlerID GetGenericPostBoundID (csHandlerID) = 0;
  /**
   * Get the csHandlerID for a specified event handler, which provides
   * its own name via the iEventHandler::GetInstanceName() method.
   * Does not set up any mappings, implicit names, or anything else that you 
   * need; a handler needs be registered with RegisterID() beforehand.
   */
  virtual csHandlerID GetID (iEventHandler *) = 0;
  /**
   * Get the csHandlerID for an arbitrary handler name.  Does not set
   * up any mappings, implicit names, or anything else that you need;
   * a handler needs be registered with RegisterID() beforehand.
   */
  virtual csHandlerID GetID (const char*) = 0;

  /**
   * Register an event handler to obtain a handler ID. 
   * \remarks Every call must be balanced with a call to ReleaseID() to
   *   ensure proper housekeeping. Otherwise, event handler instances may
   *   be leaking.
   */
  virtual csHandlerID RegisterID (iEventHandler *) = 0;
  /**
   * Used when an iEventHandler is destroyed to remove our reference.
   */
  virtual void ReleaseID (csHandlerID id) = 0;
  /**
   * Used when an iEventHandler is destroyed to remove our reference.
   */
  virtual void ReleaseID (iEventHandler *) = 0;
  /**
   * Returns the handler registered for a csHandlerID 
   * (will be NULL if csHandlerID is a generic name, i.e.,
   * if (!csEventHandlerRegistry->IsInstance(id)).
   */
  virtual iEventHandler *GetHandler (csHandlerID id) = 0;
  /**
   * returns true if instanceid is a handler instance,
   * genericid is a generic instance, and instanceid is an
   * instance of genericid in particular.
   */
  virtual bool IsInstanceOf (csHandlerID instanceid, 
    csHandlerID genericid) = 0;
  /**
   * returns true if id is a handler instance (i.e., not a generic name).
   */
  virtual bool IsInstance (csHandlerID id) = 0;
  /**
   * Returns the csHandleID for the generic name for instance name id.
   */
  virtual csHandlerID const GetGeneric (csHandlerID id) = 0;
  /**
   * Returns the string name for a csHandlerID.
   */
  virtual const char *GetString (csHandlerID id) = 0;
}; 

/** @} */

#endif // __CS_IUTIL_EVENTHANDLERS_H__
