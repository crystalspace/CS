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

#ifndef __ISYS_SYSTEM_H__
#define __ISYS_SYSTEM_H__

#include "csutil/scf.h"

struct iObjectRegistry;

//@@@ This is a temporary define that MUST be used to get the iSystem
// pointer from the object registry. Normally you can also use
// CS_QUERY_REGISTRY() but since we want to eventually remove iSystem
// we need to be able to find queries to iSystem fast so we can remove them
// too. That's why we put this in a seperate define that we will later remove.
#define CS_GET_SYSTEM(object_reg) CS_QUERY_REGISTRY(object_reg, iSystem)


SCF_VERSION (iSystem, 13, 0, 0);

/**
 * This interface serves as a way for plug-ins to query Crystal Space about
 * miscelaneous settings. It also serves as a way for plug-ins to print
 * through Crystal Space's printing interfaces.
 *<p>
 * Notes on plugin support: the list of plugins is queried from the [Plugins]
 * section in the config file. The plugins are loaded in the order they come
 * in the .cfg file.
 *<p>
 * The plugin can insert itself into the event processing chain and perform
 * some actions depending on events. It also can supply an private independent
 * API but in this case client program should know this in advance.
 */
struct iSystem : public iBase
{
  //--------------------------- Initialization ------------------------------//

  /**
   * The ::Loop method calls this method once per frame.
   * This method can be called manually as well if you don't use the
   * Loop method.
   */
  virtual void NextFrame () = 0;

  //---------------------------- Object Registry ----------------------------//

  /**
   * Get the global object registry (temporary function).
   */
  virtual iObjectRegistry* GetObjectRegistry () = 0;
};

#endif // __ISYS_SYSTEM_H__
