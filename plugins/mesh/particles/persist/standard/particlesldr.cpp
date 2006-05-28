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


#include "cssysdef.h"

#include "particlesldr.h"

#include "iengine/mesh.h"
#include "imap/ldrctxt.h"
#include "imesh/object.h"
#include "imesh/particles.h"
#include "iutil/document.h"
#include "iutil/plugin.h"

CS_IMPLEMENT_PLUGIN

CS_PLUGIN_NAMESPACE_BEGIN(ParticlesLoader)
{
  SCF_IMPLEMENT_FACTORY(ParticlesFactoryLoader);
  SCF_IMPLEMENT_FACTORY(ParticlesObjectLoader);

  ParticlesBaseLoader::ParticlesBaseLoader (iBase* parent)
    : scfImplementationType (this, parent), objectRegistry (0)
  {
    InitTokenTable (xmltokens);
  }

  bool ParticlesBaseLoader::Initialize (iObjectRegistry* objreg)
  {
    objectRegistry = objreg;
    synldr = csQueryRegistry<iSyntaxService> (objectRegistry);
    return true;
  }

  bool ParticlesBaseLoader::ParseBaseNode (iParticleSystemBase* baseObject, 
    iDocumentNode *node, iLoaderContext* ldr_context, iBase* context)
  {
    if (!node || !baseObject)
      return false;

    if (node->GetType () != CS_NODE_ELEMENT)
      return false;

    const char* value = node->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
    case XMLTOKEN_RENDERORIENTATION:
      {
        const char* orient = node->GetContentsValue ();
        
        csParticleRenderOrientation r = CS_PARTICLE_CAMERAFACE_APPROX;
        if (!strcasecmp (orient, "camface"))
          r = CS_PARTICLE_CAMERAFACE;
        else if (!strcasecmp (orient, "camface approx"))
          r = CS_PARTICLE_CAMERAFACE_APPROX;
        else if (!strcasecmp (orient, "common"))
          r = CS_PARTICLE_ORIENT_COMMON;
        else if (!strcasecmp (orient, "common approx"))
          r = CS_PARTICLE_ORIENT_COMMON_APPROX;
        else if (!strcasecmp (orient, "velocity"))
          r = CS_PARTICLE_ORIENT_VELOCITY;
        else if (!strcasecmp (orient, "self"))
          r = CS_PARTICLE_ORIENT_SELF;
        else if (!strcasecmp (orient, "self forward"))
          r = CS_PARTICLE_ORIENT_SELF_FORWARD;
        else
        {
          synldr->ReportError ("crystalspace.particleloader.parsebase", node,
            "Unknown orientation mode (%s)!", orient);
          return false;
        }

        baseObject->SetParticleRenderOrientation (r);
      }
      break;
    case XMLTOKEN_ROTATIONMODE:
      {
        const char* rotation = node->GetContentsValue ();
        csParticleRotationMode m = CS_PARTICLE_ROTATE_NONE;

        if (!strcasecmp (rotation, "none"))
          m = CS_PARTICLE_ROTATE_NONE;
        else if (!strcasecmp (rotation, "texcoord"))
          m = CS_PARTICLE_ROTATE_TEXCOORD;
        else if (!strcasecmp (rotation, "vertex"))
          m = CS_PARTICLE_ROTATE_VERTICES;
        else
        {
          synldr->ReportError ("crystalspace.particleloader.parsebase", node,
            "Unknown rotation mode (%s)!", rotation);
          return false;
        }

        baseObject->SetRotationMode (m);
      }
      break;
    case XMLTOKEN_SORTMODE:
      {
        const char* sortmode = node->GetContentsValue ();
        csParticleSortMode m = CS_PARTICLE_SORT_NONE;
        
        if (!strcasecmp (sortmode, "none"))
          m = CS_PARTICLE_SORT_NONE;
        else if (!strcasecmp (sortmode, "distance"))
          m = CS_PARTICLE_SORT_DISTANCE;
        else if (!strcasecmp (sortmode, "dot"))
          m = CS_PARTICLE_SORT_DOT;
        else
        {
          synldr->ReportError ("crystalspace.particleloader.parsebase", node,
            "Unknown sorting mode (%s)!", sortmode);
          return false;
        }

        baseObject->SetSortMode (m);
      }
      break;
    case XMLTOKEN_INTEGRATIONMODE:
      {
        const char* integ = node->GetContentsValue ();
        csParticleIntegrationMode m = CS_PARTICLE_INTEGRATE_LINEAR;

        if (!strcasecmp (integ, "none"))
          m = CS_PARTICLE_INTEGRATE_NONE;
        else if (!strcasecmp (integ, "linear"))
          m = CS_PARTICLE_INTEGRATE_LINEAR;
        else if (!strcasecmp (integ, "both"))
          m = CS_PARTICLE_INTEGRATE_BOTH;
        else
        {
          synldr->ReportError ("crystalspace.particleloader.parsebase", node,
            "Unknown integration mode (%s)!", integ);
          return false;
        }

        baseObject->SetIntegrationMode (m);
      }
      break;
    case XMLTOKEN_COMMONDIRECTION:
      {
        csVector3 dir;
        if (!synldr->ParseVector (node, dir))
        {
          return false;
        }
        baseObject->SetCommonDirection (dir);
      }
      break;
    case XMLTOKEN_LOCALMODE:
      {
        bool local;
        if (!synldr->ParseBool (node, local, true))
        {
          return false;
        }
        baseObject->SetLocalMode (local);
      }
      break;
    case XMLTOKEN_INDIVIDUALSIZE:
      {
        bool ind;
        if (!synldr->ParseBool (node, ind, true))
        {
          return false;
        }
        baseObject->SetUseIndividualSize (ind);
      }
      break;
    case XMLTOKEN_PARTICLESIZE:
      {
        csVector2 size (1.0f);
        if (!synldr->ParseVector (node, size))
        {
          return false;
        }
        baseObject->SetParticleSize (size);
      }
      break;
    case XMLTOKEN_EMITTER:
      {
        csRef<iParticleEmitter> emitter;

        if (!emitter)
        {
          synldr->ReportError ("crystalspace.particleloader.parsebase", node,
            "Error loading emitter!");
          return false;
        }
        baseObject->AddEmitter (emitter);
      }
      break;
    case XMLTOKEN_EFFECTOR:
      {
        csRef<iParticleEffector> effector;

        if (!effector)
        {
          synldr->ReportError ("crystalspace.particleloader.parsebase", node,
            "Error loading effector!");
          return false;
        }
        baseObject->AddEffector (effector);
      }
      break;
    default:
      synldr->ReportBadToken (node);
    }

    return true;
  }


  bool ParticlesBaseLoader::ParseEmitter (csRef<iParticleEmitter>& newEmitter,
    iDocumentNode* node)
  {
    const char* emitterType = node->GetAttributeValue ("type");
    
    if (!emitterType)
    {
      synldr->ReportError ("crystalspace.particleloader.parseemitter", node,
        "No emitter type specified!");
      return false;
    }

    csRef<iParticleBuiltinEmitterFactory> factory = 
      csLoadPluginCheck<iParticleBuiltinEmitterFactory> (
        objectRegistry, "crystalspace.mesh.object.particles.emitter", false);
    
    if (!factory)
    {
      synldr->ReportError ("crystalspace.particleloader.parseemitter", node,
        "Could not load particle emitter factory!");
      return false;
    }

    //properties
    float radius = 1.0f, coneAngle = PI/4;
    csVector3 position (0.0f), extent (0.0f), initialVelocity (0.0f);
    bool enabled = true;
    float startTime = -1.0f, duration = FLT_MAX, emissionRate = 0.0f, 
      minTTL = FLT_MAX, maxTTL = FLT_MAX, minMass = 1.0f, maxMass = 1.0f;
    csOBB box;
    csParticleBuiltinEmitterPlacement placement = CS_PARTICLE_BUILTIN_CENTER;
    bool unifromVelocity = false;

    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      
      if (child->GetType () != CS_NODE_ELEMENT) 
        continue;

      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch(id)
      {
      case XMLTOKEN_ENABLED:
        {
          if(!synldr->ParseBool (child, enabled, true))
          {
            return false;
          }
        }
        break;
      case XMLTOKEN_STARTTIME:
        startTime = child->GetContentsValueAsFloat ();
        break;
      case XMLTOKEN_DURATION:
        duration = child->GetContentsValueAsFloat ();
        break;
      case XMLTOKEN_EMISSIONRATE:
        emissionRate = child->GetContentsValueAsFloat ();
        break;
      case XMLTOKEN_INITIALTTL:
        {
          minTTL = child->GetAttributeValueAsFloat ("min");
          maxTTL = child->GetAttributeValueAsFloat ("max");
        }
        break;
      case XMLTOKEN_MASS:
        {
          minMass = child->GetAttributeValueAsFloat ("min");
          maxMass = child->GetAttributeValueAsFloat ("max");
        }
        break;
      case XMLTOKEN_POSITION:
        {
          if (!synldr->ParseVector (child, position))
          {
            synldr->ReportError ("crystalspace.particleloader.parsebase", child,
              "Error parsing position!");
          }
        }
        break;
      case XMLTOKEN_PLACEMENT:
        {
          const char* p = child->GetContentsValue ();
          if (!strcasecmp (p, "center"))
            placement = CS_PARTICLE_BUILTIN_CENTER;
          else if (!strcasecmp (p, "volume"))
            placement = CS_PARTICLE_BUILTIN_VOLUME;
          else if (!strcasecmp (p, "surface"))
            placement = CS_PARTICLE_BUILTIN_SURFACE;
          else
          {
            synldr->ReportError ("crystalspace.particleloader.parsebase", child,
              "Unknown particle placement mode (%s)!", p);
            return false;
          }
        }
        break;
      case XMLTOKEN_UNIFORMVELOCITY:
        if (!synldr->ParseBool (child, unifromVelocity, true))
        {
          return false;
        }       
        break;
      case XMLTOKEN_INITIALVELOCITY:
        if (!synldr->ParseVector (child, initialVelocity))
        {
          return false;
        }
        break;
      case XMLTOKEN_RADIUS:
        radius = child->GetContentsValueAsFloat ();
        break;
      case XMLTOKEN_EXTENT:
        if (!synldr->ParseVector (child, extent))
        {
          return false;
        }
        break;
      case XMLTOKEN_CONEANGLE:
        coneAngle = child->GetContentsValueAsFloat ();
        break;
      case XMLTOKEN_BOX:
        if (!synldr->ParseBox (child, box))
        {
          return false;
        }
        break;
      default:
        synldr->ReportBadToken (child);
        return false;
      }
    }

    csRef<iParticleBuiltinEmitterBase> baseEmitter;

    // Create it..
    if (!strcasecmp (emitterType, "sphere")) 
    {
      csRef<iParticleBuiltinEmitterSphere> sphereEmitter = factory->CreateSphere ();
      baseEmitter = sphereEmitter;
      // Set individual properties
      sphereEmitter->SetRadius (radius);
    }
    else if (!strcasecmp (emitterType, "box"))
    {
      csRef<iParticleBuiltinEmitterBox> boxEmitter = factory->CreateBox ();
      baseEmitter = boxEmitter;
      // Set individual properties
      boxEmitter->SetBox (box);
    }
    else if (!strcasecmp (emitterType, "cylinder"))
    {
      csRef<iParticleBuiltinEmitterCylinder> cylinderEmitter = factory->CreateCylinder ();
      baseEmitter = cylinderEmitter;
      // Set individual properties
      cylinderEmitter->SetRadius (radius);
      cylinderEmitter->SetExtent (extent);
    }
    else if (!strcasecmp (emitterType, "cone"))
    {
      csRef<iParticleBuiltinEmitterCone> coneEmitter = factory->CreateCone ();
      baseEmitter = coneEmitter;
      // Set individual properties
      coneEmitter->SetExtent (extent);
      coneEmitter->SetConeAngle (coneAngle);
    }
    else
    {
      synldr->ReportError ("crystalspace.particleloader.parsebase", node,
              "Unknown emitter type (%s)!", emitterType);
      return false;
    }

    if (!baseEmitter)
    {
      return false;
    }

    // Set base properties
    baseEmitter->SetPosition (position);
    baseEmitter->SetInitialVelocity (initialVelocity, csVector3 (0.0f));
    baseEmitter->SetUniformVelocity (unifromVelocity);
    baseEmitter->SetParticlePlacement (placement);
    
    // Set common properties
    newEmitter = baseEmitter;
    newEmitter->SetEnabled (enabled);
    newEmitter->SetStartTime (startTime);
    newEmitter->SetDuration (duration);
    newEmitter->SetEmissionRate (emissionRate);
    newEmitter->SetInitialTTL (minTTL, maxTTL);
    newEmitter->SetInitialMass (minMass, maxMass);

    return true;
  }


  csPtr<iBase> ParticlesFactoryLoader::Parse (iDocumentNode* node,
    iStreamSource* ssource, iLoaderContext* ldr_context, iBase* context)
  {
    csRef<iMeshObjectType> type = csLoadPluginCheck<iMeshObjectType> (
  	objectRegistry, "crystalspace.mesh.loader.factory.particles", false);
    if (!type)
    {
      synldr->ReportError (
		  "crystalspace.particleloader.parsefactory",
		  node, "Could not load the general mesh object plugin!");
      return 0;
    }

    csRef<iMeshObjectFactory> factoryObj = type->NewFactory ();
    csRef<iParticleSystemFactory> particleFact = 
      scfQueryInterfaceSafe<iParticleSystemFactory> (factoryObj);

    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch(id)
      {
      case XMLTOKEN_DEEPCREATION:
        {
          bool deep = false;
          if (!synldr->ParseBool (child, deep, true))
          {
            synldr->ReportError ("crystalspace.particleloader.parsefactory",
              node, "Could not parse factory!");
            return 0;
          }
        }
        break;
      default:
        {
          if (!ParseBaseNode (particleFact, child, ldr_context, context))
          {
            synldr->ReportError ("crystalspace.particleloader.parsefactory",
              node, "Could not parse factory!");
            return 0;
          }
        }
      }
    }

    return csPtr<iBase> (factoryObj);
  }

  csPtr<iBase> ParticlesObjectLoader::Parse (iDocumentNode* node,
    iStreamSource* ssource, iLoaderContext* ldr_context, iBase* context)
  {
    
    csRef<iMeshObject> meshObj;
    csRef<iParticleSystem> particleSystem;

    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch(id)
      {
      case XMLTOKEN_FACTORY:
        {
          const char* factname = child->GetContentsValue ();
	  iMeshFactoryWrapper* fact = ldr_context->FindMeshFactory (factname);

          if (!fact)
          {
            synldr->ReportError ("crystalspace.particleloader.parsesystem",
              child, "Could not find factory '%s'!", factname);

            return 0;
          }

          meshObj = fact->GetMeshObjectFactory ()->NewInstance ();
          particleSystem = scfQueryInterface<iParticleSystem> (meshObj);

          if (!particleSystem)
          {
            synldr->ReportError ("crystalspace.particleloader.parsesystem",
              child, "Factory '%s' does not seem to be a particle system factory!", 
              factname);

            return 0;
          }
        }
        break;
      default:
        {
          if (!particleSystem)
          {
            synldr->ReportError ("crystalspace.particleloader.parsesystem",
              child, "Specify factory first!");

            return 0;
          }

          if (!ParseBaseNode (particleSystem, child, ldr_context, context))
          {
            synldr->ReportError ("crystalspace.particleloader.parsesystem",
              node, "Could not parse particle system!");
            return 0;
          }
        }
      }
    }

    return csPtr<iBase> (meshObj);
  }

}

CS_PLUGIN_NAMESPACE_END(ParticlesLoader)
