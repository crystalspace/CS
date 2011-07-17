/*
    Copyright (C) 2011 by Mike Gist and Jelle Hellemans

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

#ifndef __CS_DAMN_RESOURCE_H__
#define __CS_DAMN_RESOURCE_H__

#include "csutil/scf_interface.h"
#include <csutil/scf_implementation.h>


#include "imap/resource.h"

struct iMeshFactoryWrapper;



inline void iResourceTrigger(iLoadingResource* b)
{
  b->TriggerCallback();
}




/**
 * The iResourceCache interface.
 */
struct iFormatAbstractor : public virtual iBase
{
  SCF_INTERFACE (iFormatAbstractor, 1,0,0);
  
  /**
   * Add a given resource to the cache.
   * @param abstraction The abstraction's name.
   * @param format The underlying format for the abstraction.
   */
  virtual void AddAbstraction (const char* type, const char* format) = 0;

  /**
   * Add a given resource to the cache.
   * @param abstraction The abstraction of the format you want to query.
   */
  virtual const char* GetFormat (const char* type) const = 0;
};

#endif // __CS_IMAP_RESOURCE_H__

