/*
    Copyright (C) 2007 by Seth Yastrov

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

#ifndef __IEDITOR_INTERFACEWRAPPERMANAGER_H__
#define __IEDITOR_INTERFACEWRAPPERMANAGER_H__

#include <csutil/scf.h>
#include <csutil/scf_implementation.h>

namespace CS {
namespace EditorApp {

struct iInterfaceWrapperFactory;
  
/**
 * Manages the interface wrappers.
 */
struct iInterfaceWrapperManager : public virtual iBase
{
  SCF_INTERFACE (iInterfaceWrapperManager, 0, 0, 1);

  /**
   * Register an interface wrapper factory with the interface it wraps.
   */
  virtual void Register (iInterfaceWrapperFactory* wrapper) = 0;

  /**
   * Get the interface wrapper factory associated with the specified interface.
   */
  virtual iInterfaceWrapperFactory* GetFactory (scfInterfaceID interface_id) = 0;
};

} // namespace EditorApp
} // namespace CS

#endif
