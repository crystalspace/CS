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

#ifndef __CS_IUTIL_EVENTNAMES_H__
#define __CS_IUTIL_EVENTNAMES_H__

#include "iutil/strset.h"
class csString;

/**\file
 * General event name resolver interface
 */
/**
 * \addtogroup event_handling
 * @{ */
 
#include "csutil/scf_interface.h"

/**
 * A csEventID is a handle for a string representing an event's hierarchical
 * name (e.g., "crystalspace.input.keyboard.down").
 */
typedef csStringID csEventID;

#define CS_EVENT_INVALID csInvalidStringID
#define CS_EVENTLIST_END csInvalidStringID


/**
 * This interface represents a general event name resolver.
 * 
 * Event name resolvers transform string representations of event names
 * (e.g., "crystalspace.input.joystick.2.move") into csEventIDs, which 
 * are used to key event subscription and delivery.  Typically, one 
 * instance of this object is available from the shared-object registry 
 * (iObjectRegistry).
 *
 * Main creators of instances implementing this interface:
 * - csInitializer::CreateEnvironment()
 * - csInitializer::CreateEventNameRegistry()
 * 
 * Main ways to get pointers to this interface:
 * - csQueryRegistry()
 */
struct iEventNameRegistry : public virtual iBase
{
  SCF_INTERFACE(iEventNameRegistry, 1,1,0);

  /**
   * Get the csEventID for an event name string 
   * (e.g., "crystalspace.input.keyboard.down").
   */
  virtual csEventID GetID (const char* name) = 0;

  /**
   * Get the name represented by a csEventID.
   */
  virtual const char* GetString (const csEventID id) = 0;
  /**
   * Get the csEventID of the parent of the current event
   * (e.g., the parent of "crystalspace.input.mouse" is "crystalspace.input").
   */
  virtual csEventID GetParentID (const csEventID id) = 0;
  /**
   * Determine whether the name of the first csEventID is an immediate child
   * of the name of the second csEventID (e.g., "crysalspace.input.mouse" is
   * an immediate child of "crystalspace.input" but not of "crystalspace" or
   * of "").
   */
  virtual bool IsImmediateChildOf (const csEventID child, 
						   const csEventID parent) = 0;
  /**
   * Determine whether the name of the first csEventID is equivalent to or
   * a child of the name of the second csEventID (e.g., 
   * "crystalspace.input.mouse" is a "kind" of "crystalspace.input.mouse",
   * "crystalspace.input", "crystalspace", and "", but not of 
   * "crystalspace.input.mouse.2").
   */
  virtual bool IsKindOf (const csEventID child, 
    const csEventID parent) = 0;

};

/** @} */

#endif // __CS_IUTIL_EVENTNAMES_H__
