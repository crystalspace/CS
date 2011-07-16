/*
  Copyright (C) 2011 by Michael Gist

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
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

#ifndef __CS_CSUTIL_RESOURCE_H__
#define __CS_CSUTIL_RESOURCE_H__

#include "imap/resource.h"

namespace CS
{
  namespace Resource
  {
    /**
     * Class which can be inherited by resources with
     * no resource dependencies.
     */
    class NoDepResource : public virtual iResource
    {
    public:
      virtual const csArray<ResourceReference>& GetDependencies () const
      {
        return emptyDeps;
      }

      virtual void SetProperty (csStringID propery, iResource* resource) {};

    private:
      csArray<ResourceReference> emptyDeps;
    };
  }
}

#endif // __CS_CSUTIL_RESOURCE_H__
