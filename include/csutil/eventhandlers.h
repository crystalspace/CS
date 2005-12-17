/*
   Crystal Space 3D engine: Event and module naming interface
   (C) 2005 by Adam D. Bradley <artdodge@cs.bu.edu>
   
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

#ifndef __CS_UTIL_EVENTHNAMES_H__
#define __CS_UTIL_EVENTHNAMES_H__

#include "cssysdef.h" /* CS_DEBUG changes our definitions... */
#include "csextern.h"
#include "iutil/eventnames.h"
#include "iutil/eventhandlers.h"
#include "iutil/objreg.h"
#include "csutil/scf_implementation.h"
#include "csutil/hash.h"
#include "csutil/strset.h"
#include "csutil/scf.h"

/**\file
 * Event handler naming, name management, indexing, and instantiation
 */
/**
 * \addtogroup event_handling
 * @{ */

struct iEventHandler;

/**
 * The csEventHandlerRegistry maintains a global one-to-one mapping from strings
 * to csHandlerIDs, and a one-to-(zero or one) mapping from csHandlerIDs to
 * iEventHandler pointers.
 */
class CS_CRYSTALSPACE_EXPORT csEventHandlerRegistry : 
  public scfImplementation1<csEventHandlerRegistry, iEventHandlerRegistry>
{
public:
  csEventHandlerRegistry(iObjectRegistry*);
  ~csEventHandlerRegistry();
  /**
   * Get a csHandlerID based upon some string.
   * This should only ever be done to reference generic 
   * (non-instantiated) handler names or single-instance handlers.
   */	
  CS_CONST_METHOD csHandlerID GetGenericID (const csString &);
  static CS_CONST_METHOD csHandlerID GetGenericID (iObjectRegistry *reg, 
    const csString &name) 
  {
    return GetRegistry (reg)->GetGenericID (name);
  }
  CS_CONST_METHOD csHandlerID GetGenericPreBoundID (csHandlerID);
  static CS_CONST_METHOD csHandlerID GetGenericPreBoundID (
    iObjectRegistry *reg, csHandlerID id) 
  {
    return GetRegistry (reg)->GetGenericPreBoundID (id);
  }
  CS_CONST_METHOD csHandlerID GetGenericPostBoundID (csHandlerID);
  static CS_CONST_METHOD csHandlerID GetGenericPostBoundID (
    iObjectRegistry *reg, csHandlerID id) 
  {
    return GetRegistry (reg)->GetGenericPostBoundID (id);
  }
    
  /**
   * Get the csHandlerID for a specified event handler, which provides
   * its own name via the iEventHandler::GetInstanceName() method.
   */
  csHandlerID GetID (iEventHandler *);
  static CS_CONST_METHOD csHandlerID GetID (iObjectRegistry *reg, 
    iEventHandler *h) 
  {
    return GetRegistry (reg)->GetID (h);
  }
  /**
   * Used when an iEventHandler is desroyed to remove our reference.
   */
  void ReleaseID (csHandlerID id);
  static CS_CONST_METHOD void ReleaseID (iObjectRegistry *reg, 
    csHandlerID id)
  {
    GetRegistry (reg)->ReleaseID (id);
  }
  /**
   * Used when an iEventHandler is destroyed to remove our reference.
   */
  void ReleaseID (iEventHandler *);
  static CS_CONST_METHOD void ReleaseID (iObjectRegistry *reg, 
    iEventHandler *h) 
  {
    GetRegistry (reg)->ReleaseID (h);
  }
  /**
   * Returns the handler registered for a csHandlerID 
   * (will be 0 if csHandlerID is a generic name, i.e.,
   * if (!csEventHandlerRegistry->IsInstance(id)).
   */
  CS_CONST_METHOD iEventHandler* GetHandler (csHandlerID id);
  static inline CS_CONST_METHOD iEventHandler* GetHandler (
    iObjectRegistry *reg, csHandlerID id) 
  {
    return GetRegistry (reg)->GetHandler (id);
  };

  /**
   * returns true if instanceid is a handler instance,
   * genericid is a generic instance, and instanceid is an
   * instance of genericid in particular.
   */
  CS_CONST_METHOD bool const IsInstanceOf (csHandlerID instanceid, 
    csHandlerID genericid);
  static inline CS_CONST_METHOD bool IsInstanceOf (iObjectRegistry *reg, 
    csHandlerID instanceid, csHandlerID genericid) 
  {
    return GetRegistry (reg)->IsInstanceOf (instanceid, genericid);
  };

  /**
   * returns true if id is a handler instance (i.e., not a generic name).
   */
  CS_CONST_METHOD bool const IsInstance (csHandlerID id);
  static inline CS_CONST_METHOD bool IsInstance (iObjectRegistry *reg, 
  csHandlerID id) 
  {
    return GetRegistry (reg)->IsInstance (id);
  };

  /**
   * Returns the csHandleID for the generic name for instance name id.
   */
  CS_CONST_METHOD csHandlerID const GetGeneric (csHandlerID id);
  static inline CS_CONST_METHOD csHandlerID GetGeneric (iObjectRegistry *reg, 
    csHandlerID id) 
  {
    return GetRegistry (reg)->GetGeneric (id);
  };

  /**
   * Returns the string name for a csHandlerID.
   */
  CS_CONST_METHOD const char* GetString (csHandlerID id);
  static inline CS_CONST_METHOD const char* GetString (
    iObjectRegistry *reg, csHandlerID id) 
  {
    return GetRegistry (reg)->GetString (id);
  };

  static csRef<iEventHandlerRegistry> GetRegistry (
    iObjectRegistry *object_reg);

 private:
  iObjectRegistry *object_reg;
  csStringSet names;
  csHash<csHandlerID, csHandlerID> instantiation; 
  csHash<csRef<iEventHandler>, csHandlerID> idToHandler;
  csHash<csHandlerID, csRef<iEventHandler> > handlerToID;
  csHash<csHandlerID, csHandlerID> handlerPres;
  csHash<csHandlerID, csHandlerID> handlerPosts;
  uint32 instanceCounter;
};

/* @} */

#endif // __CS_UTIL_EVENTHNAMES_H__
