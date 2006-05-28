/*
  Copyright (C) 2006 by Marten Svanfeldt

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

#ifndef __CS_MESH_PARTICLESLDR_H__
#define __CS_MESH_PARTICLESLDR_H__

#include "csutil/scf_implementation.h"
#include "csutil/strhash.h"

#include "imesh/particles.h"
#include "iutil/comp.h"
#include "imap/reader.h"
#include "imap/services.h"

CS_PLUGIN_NAMESPACE_BEGIN(ParticlesLoader)
{
  class ParticlesBaseLoader : public 
   scfImplementation2<ParticlesBaseLoader,
                      iLoaderPlugin,
                      iComponent>
  {
  public:
    ParticlesBaseLoader (iBase* p);

    //-- iComponent
    bool Initialize (iObjectRegistry* objreg);

  protected:
    iObjectRegistry* objectRegistry;
    csRef<iSyntaxService> synldr;
  
    csStringHash xmltokens;
#define CS_TOKEN_ITEM_FILE "plugins/mesh/particles/persist/standard/particlesldr.tok"
#include "cstool/tokenlist.h"

    //-- iLoaderPlugin helpers for derived classes
    bool ParseBaseNode (iParticleSystemBase* baseObject, iDocumentNode *node, 
      iLoaderContext* ldr_context, iBase* context);  

    bool ParseEmitter (csRef<iParticleEmitter>& newEmitter, 
      iDocumentNode* node);
  };


  class ParticlesFactoryLoader : public ParticlesBaseLoader
  {
  public:
    ParticlesFactoryLoader (iBase* parent)
      : ParticlesBaseLoader (parent)
    {
    }

    //-- iLoaderPlugin
    virtual csPtr<iBase> Parse (iDocumentNode* node,
  	iStreamSource* ssource, iLoaderContext* ldr_context,
  	iBase* context);
  };

  class ParticlesObjectLoader : public ParticlesBaseLoader
  {
  public:
    ParticlesObjectLoader (iBase* parent)
      : ParticlesBaseLoader (parent)
    {
    }

    //-- iLoaderPlugin
    virtual csPtr<iBase> Parse (iDocumentNode* node,
  	iStreamSource* ssource, iLoaderContext* ldr_context,
  	iBase* context);
  };
}
CS_PLUGIN_NAMESPACE_END(ParticlesLoader)


#endif
