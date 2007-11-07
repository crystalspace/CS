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
#include "iutil/object.h"
#include "iengine/region.h"

namespace CS
{
  namespace Utility
  {
    namespace UnusedResourceHelper
    {
      void UnloadAllUnusedMaterials(iEngine* engine)
      {
        iMaterialList *matList = engine->GetMaterialList();

        iRegionList *regList = engine->GetRegions();
        for(int i=0; i<matList->GetCount(); i++)
        {
          bool inRegion = false;
          iRegion* region;
          for(int j=0; j<regList->GetCount(); j++)
          {
            inRegion = inRegion 
              || regList->Get(j)->FindMaterial(matList->Get(i)->QueryObject()->GetName());
            if(inRegion)
            {
              region = regList->Get(j);
              break;
            }
          }

          if(matList->Get(i)->GetRefCount() == 1 
            || (matList->Get(i)->GetRefCount() == 2 && inRegion))
          {
            if(inRegion)
                region->Remove(matList->Get(i)->QueryObject());
            matList->Remove(i);
            i--;
          }
        }
      }

      void UnloadAllUnusedTextures(iEngine* engine)
      {
        iTextureList *texList = engine->GetTextureList();

        iRegionList *regList = engine->GetRegions();
        for(int i=0; i<texList->GetCount(); i++)
        {
          bool inRegion = false;
          iRegion* region;
          for(int j=0; j<regList->GetCount(); j++)
          {
            inRegion = inRegion 
              || regList->Get(j)->FindTexture(texList->Get(i)->QueryObject()->GetName());
            if(inRegion)
            {
              region = regList->Get(j);
              break;
            }
          }

          if(texList->Get(i)->GetRefCount() == 1 
            || (texList->Get(i)->GetRefCount() == 2 && inRegion))
          {
            if(inRegion)
              region->Remove(texList->Get(i)->QueryObject());
            texList->Remove(i);
            i--;
          }
        }
      }

      void UnloadAllUnusedFactories(iEngine* engine)
      {
        iMeshFactoryList *factList = engine->GetMeshFactories();

        iRegionList *regList = engine->GetRegions();
        for(int i=0; i<factList->GetCount(); i++)
        {
          bool inRegion = false;
          iRegion* region;
          for(int j=0; j<regList->GetCount(); j++)
          {
            inRegion = inRegion 
              || regList->Get(j)->FindMeshFactory(factList->Get(i)->QueryObject()->GetName());
            if(inRegion)
            {
              region = regList->Get(j);
              break;
            }
          }

          if(factList->Get(i)->GetRefCount() == 1 
            || (factList->Get(i)->GetRefCount() == 2 && inRegion))
          {
            if(inRegion)
              region->Remove(factList->Get(i)->QueryObject());
            factList->Remove(i);
            i--;
          }
        }
      }

      void UnloadUnusedMaterials(iEngine* engine,
        const csWeakRefArray<iMaterialWrapper>& materials)
      {
        iRegionList *regList = engine->GetRegions();

        for(size_t i=0; i<materials.GetSize(); i++)
        {
          if (materials[i] == 0) continue;

          bool inRegion = false;
          iRegion* region;
          for(int j=0; j<regList->GetCount(); j++)
          {
            inRegion = inRegion 
              || regList->Get(j)->FindMaterial(materials.Get(i)->QueryObject()->GetName());
            if(inRegion)
            {
              region = regList->Get(j);
              break;
            }
          }

          if(materials.Get(i)->GetRefCount() == 1 
            || (materials.Get(i)->GetRefCount() == 2 && inRegion))
          {
            if(inRegion)
              region->Remove(materials.Get(i)->QueryObject());
            engine->GetMaterialList()->Remove(materials.Get(i));
          }
        }
      }

      void UnloadUnusedTextures(iEngine* engine,
        const csWeakRefArray<iTextureWrapper>& textures)
      {
        iRegionList *regList = engine->GetRegions();

        for(size_t i=0; i<textures.GetSize(); i++)
        {
          if (textures[i] == 0) continue;

          bool inRegion = false;
          iRegion* region;
          for(int j=0; j<regList->GetCount(); j++)
          {
            inRegion = inRegion 
              || regList->Get(j)->FindTexture(textures.Get(i)->QueryObject()->GetName());
            if(inRegion)
            {
              region = regList->Get(j);
              break;
            }
          }

          if(textures.Get(i)->GetRefCount() == 1 
            || (textures.Get(i)->GetRefCount() == 2 && inRegion))
          {
            if(inRegion)
              region->Remove(textures.Get(i)->QueryObject());
            engine->GetTextureList()->Remove(textures.Get(i));
          }
        }
      }

      void UnloadUnusedFactories(iEngine* engine,
        const csWeakRefArray<iMeshFactoryWrapper>& factories)
      {
        iRegionList *regList = engine->GetRegions();

        for(size_t i=0; i<factories.GetSize(); i++)
        {
          if (factories[i] == 0) continue;

          bool inRegion = false;
          iRegion* region;
          for(int j=0; j<regList->GetCount(); j++)
          {
            inRegion = inRegion 
              || regList->Get(j)->FindMeshFactory(factories.Get(i)->QueryObject()->GetName());
            if(inRegion)
            {
              region = regList->Get(j);
              break;
            }
          }

          if(factories.Get(i)->GetRefCount() == 1 
            || (factories.Get(i)->GetRefCount() == 2 && inRegion))
          {
            if(inRegion)
              region->Remove(factories.Get(i)->QueryObject());
            engine->GetMeshFactories()->Remove(factories.Get(i));
          }
        }
      }
    }
  }
}
