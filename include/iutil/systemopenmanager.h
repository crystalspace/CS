/*
    Copyright (C) 2008 by Frank Richter
    
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

#ifndef __CS_IUTIL_SYSTEMOPENMANAGER_H__
#define __CS_IUTIL_SYSTEMOPENMANAGER_H__

#include "iutil/eventh.h"
#include "csutil/scf_interface.h"

/**\file
 * Manager for system open events.
 */
 
/**
 * Manager for system open events.
 * It stores whether a csevSystemOpen event was already broadcast to the
 * event handlers. If an event handler is later registered when the system
 * is already open it immediately receives an open event.
 * Thus, using iSystemOpenManager guarantees that a listener gets an
 * csevSystemOpen event, independent whether that has been broadcast yet
 * or not at the time of registration.
 */
struct iSystemOpenManager : public virtual iBase
{
  SCF_INTERFACE (iSystemOpenManager, 0,0,1);
  
  /// Register a listener to receive csevSystemOpen and csevSystemClose events.
  virtual csHandlerID Register (iEventHandler* eventh) = 0;
  /**
   * Register a weak listener to receive csevSystemOpen and csevSystemClose
   * events.
   * \sa CS::RegisterWeakListener
   */
  virtual csHandlerID RegisterWeak (iEventHandler* eventh,
    csRef<iEventHandler> &handler) = 0;
  /// Unregister a listener for csevSystemOpen and csevSystemClose events.
  virtual void RemoveListener (iEventHandler* eventh) = 0;
  /**
   * Unregister a weak listener to receive csevSystemOpen and csevSystemClose
   * events.
   * \sa CS::RemoveWeakListener
   */
  virtual void RemoveWeakListener (csRef<iEventHandler> &handler) = 0;
};

#endif // __CS_IUTIL_SYSTEMOPENMANAGER_H__
