/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#ifndef __CS_IUTIL_COMP_H__
#define __CS_IUTIL_COMP_H__

#include "csutil/scf_interface.h"
struct iObjectRegistry;

/**\file
 * Generic component interface
 */
/**
 * \addtogroup scf
 * @{ */

//SCF_VERSION (iComponent, 0, 0, 1);

/**
 * This interface describes a generic component in Crystal Space.
 *
 * Main creators of instances implementing this interface:
 * - All plugins implement this interface.
 *
 * Main ways to get pointers to this interface:
 * - SCF_QUERY_INTERFACE() from a plugin instance.
 *
 * Main users of this interface:
 * - csPluginManager
 */
struct iComponent : public virtual iBase
{
  SCF_INTERFACE(iComponent,2,0,0);
  /**
   * Initialize the component. This is automatically called by system driver
   * at startup so that plugin can do basic initialization stuff, register
   * with the system driver and so on.
   */
  virtual bool Initialize (iObjectRegistry*) = 0;
};

/** @} */

#endif // __CS_IUTIL_COMP_H__
