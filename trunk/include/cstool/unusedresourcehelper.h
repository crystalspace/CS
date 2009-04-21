/*
  Copyright (C) 2007 by Mike Gist

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

#ifndef __CS_CSUTIL_UNUSED_RESOURCE_HELPER__
#define __CS_CSUTIL_UNUSED_RESOURCE_HELPER__

#include "csutil/weakrefarr.h"

struct iEngine;
struct iMaterialWrapper;
struct iTextureWrapper;

namespace CS
{
  namespace Utility
  {
    namespace UnusedResourceHelper
    {
      void CS_CRYSTALSPACE_EXPORT UnloadUnusedMaterials(iEngine* engine,
        const csWeakRefArray<iMaterialWrapper>& materials);
      void CS_CRYSTALSPACE_EXPORT UnloadUnusedTextures(iEngine* engine,
        const csWeakRefArray<iTextureWrapper>& textures);
    }
  }
}

#endif // __CS_CSUTIL_UNUSED_RESOURCE_HELPER__
