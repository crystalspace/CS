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

#include "cssysdef.h"

#include "cstool/unusedresourcehelper.h"
#include "csutil/csobject.h"
#include "iengine/collection.h"
#include "iengine/engine.h"
#include "iengine/material.h"
#include "iengine/texture.h"

namespace CS
{
  namespace Utility
  {
    namespace UnusedResourceHelper
    {
      namespace Implementation
      {
        static inline void UnloadHelper(iObject* object, iEngine* engine)
        {
          csWeakRef<csObject> obj = (csObject*)object;

          size_t realRefCount = obj->GetRefCount() - obj->GetInternalRefCount();
          if(realRefCount == 1)
          {
            csRef<iCollectionArray> colArr = engine->GetCollections();
            for(size_t i=0; i<colArr->GetSize() && obj.IsValid(); i++)
            {
              colArr->Get(i)->Remove(object);
            }
          }
        }
      } // namespace Implementation

      void UnloadUnusedMaterials(iEngine* engine,
        const csWeakRefArray<iMaterialWrapper>& materials)
      {
        for(size_t i=0; i<materials.GetSize(); i++)
        {
          csWeakRef<iMaterialWrapper> mw = materials[i];
          if (!mw) continue;
          Implementation::UnloadHelper(mw->QueryObject(), engine);
        }
      }

      void UnloadUnusedTextures(iEngine* engine,
        const csWeakRefArray<iTextureWrapper>& textures)
      {
        for(size_t i=0; i<textures.GetSize(); i++)
        {
          csWeakRef<iTextureWrapper> tw = textures[i];
          if (!tw) continue;
          Implementation::UnloadHelper(tw->QueryObject(), engine);
        }
      }
    }
  }
}
