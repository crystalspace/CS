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
#include "csutil/csstring.h"

#include "imap/reader.h"
#include "imap/services.h"
#include "imap/writer.h"
#include "imesh/particles.h"
#include "iutil/comp.h"

CS_PLUGIN_NAMESPACE_BEGIN(ParticlesLoader)
{

  // Loaders
  class ParticlesBaseLoader : public scfImplementation2<ParticlesBaseLoader,
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

    csPtr<iParticleEmitter> ParseEmitter (iDocumentNode* node);

    csPtr<iParticleEffector> ParseEffector (iDocumentNode* node);

    bool ParseLinearEffectorParameters (
      iDocumentNode* child, csParticleParameterSet& param, int& mask);
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

    virtual bool IsThreadSafe() { return true; }
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

    virtual bool IsThreadSafe() { return true; }
  };


  // Savers
  class ParticlesBaseSaver : public scfImplementation2<ParticlesBaseSaver,
                                                       iSaverPlugin,
                                                       iComponent>
  {
  public:
    ParticlesBaseSaver (iBase* p);

    //-- iComponent
    bool Initialize (iObjectRegistry* objreg);

  protected:
    iObjectRegistry* objectRegistry;
    csRef<iSyntaxService> synldr;

    //-- iSaverPlugin helpers for derived classes
    bool WriteOrientation (iDocumentNode* paramsNode, 
      csParticleRenderOrientation orientation);

    bool WriteRotation (iDocumentNode* paramsNode, 
      csParticleRotationMode rot);

    bool WriteSort (iDocumentNode* paramsNode, 
      csParticleSortMode sort);

    bool WriteIntegration (iDocumentNode* paramsNode, 
      csParticleIntegrationMode integ);

    bool WriteTransform (iDocumentNode* paramsNode,
      csParticleTransformMode mode);

    bool WriteEmitter (iDocumentNode* paramsNode,
      iParticleEmitter* emitter);

    bool WriteEffector (iDocumentNode* paramsNode,
      iParticleEffector* effector);
  };

  class ParticlesFactorySaver : public ParticlesBaseSaver
  {
  public:
    ParticlesFactorySaver (iBase* parent) 
      : ParticlesBaseSaver (parent)
    {}

    virtual bool WriteDown (iBase*obj, iDocumentNode* parent,
      iStreamSource* ssource);
  };

  class ParticlesObjectSaver : public ParticlesBaseSaver
  {
  public:
    ParticlesObjectSaver (iBase* parent) 
      : ParticlesBaseSaver (parent)
    {}

    virtual bool WriteDown (iBase*obj, iDocumentNode* parent,
      iStreamSource* ssource);
  };
}
CS_PLUGIN_NAMESPACE_END(ParticlesLoader)


#endif
