/*
    Copyright (C) 2003 by Jorrit Tyberghein, John Harger

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
#include "csutil/sysfunc.h"

#include "imap/ldrctxt.h"

#include "iutil/comp.h"
#include "iutil/document.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"

#include "iengine/material.h"
#include "iengine/mesh.h"

#include "imesh/object.h"
#include "imesh/particles.h"
#include "iutil/object.h"
#include "ivaria/reporter.h"

#include "particlesldr.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csParticlesFactoryLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csParticlesFactoryLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csParticlesFactoryLoader)

csParticlesFactoryLoader::csParticlesFactoryLoader (iBase* parent)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);

  InitTokenTable (xmltokens);
}

csParticlesFactoryLoader::~csParticlesFactoryLoader ()
{
}

bool csParticlesFactoryLoader::Initialize (iObjectRegistry* objreg)
{
  object_reg = objreg;
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  vfs = CS_QUERY_REGISTRY (object_reg, iVFS);

  return true;
}

csPtr<iBase> csParticlesFactoryLoader::Parse (iDocumentNode* node,
  iLoaderContext* ldr_context, iBase* context)
{
  csRef<iPluginManager> plugin_mgr = CS_QUERY_REGISTRY (object_reg,
    iPluginManager);

  csRef<iMeshObjectType> type (CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.particles", iMeshObjectType));
  if (!type)
  {
    type = CS_LOAD_PLUGIN (plugin_mgr,
      "crystalspace.mesh.object.particles", iMeshObjectType);
  }
  if (!type)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.particles.loader.factory",
		  "Could not load the particles mesh object plugin!");
    return 0;
  }
  csRef<iMeshObjectFactory> fact = type->NewFactory ();
  if (!fact)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.particles.loader.factory",
		  "Could not create the particles factory!");
    return 0;
  }

  csRef<iParticlesFactoryState> state =
    SCF_QUERY_INTERFACE(fact, iParticlesFactoryState);
  if (!state)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.particles.loader.factory",
      "Could not query iParticlesFactoryState from factory object!");
    return 0;
  }

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_MATERIAL:
      {
        const char* matname = child->GetContentsValue ();
        iMaterialWrapper* mat = ldr_context->FindMaterial (matname);
        if (!mat)
        {
          synldr->ReportError ("crystalspace.particles.factory.loader",
	    child, "Couldn't find material '%s'!", matname);
          return 0;
        }
        state->SetMaterial (mat);
        break;
      }
      case XMLTOKEN_EMITTER:
        ParseEmitter (child, state);
        break;
      case XMLTOKEN_DIFFUSION:
        state->SetDiffusion (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_GRAVITY:
      {
        csVector3 gravity;
        synldr->ParseVector (child, gravity);
        state->SetGravity (gravity);
        break;
      }
      case XMLTOKEN_TTL:
        state->SetTimeToLive (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_TIMEVARIATION:
        state->SetTimeVariation (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_INITIAL:
        state->SetInitialParticleCount (child->GetContentsValueAsInt ());
        break;
      case XMLTOKEN_PPS:
        state->SetParticlesPerSecond (child->GetContentsValueAsInt ());
        break;
      case XMLTOKEN_RADIUS:
        state->SetParticleRadius (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_MASS:
        state->SetMass (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_MASSVARIATION:
        state->SetMassVariation (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_AUTOSTART:
      {
        const char *autostart = child->GetContentsValue ();
        if(!strcmp(autostart, "no")) state->SetAutoStart (false);
        else if(!strcmp(autostart, "yes")) state->SetAutoStart (true);
        else 
        {
          synldr->ReportError ("crystalspace.particles.factory.loader",
            child, "Unknown autostart parameter '%s'!", autostart);
        }
        break;
      }
      case XMLTOKEN_TRANSFORMMODE:
      {
        const char *mode = child->GetContentsValue ();
        if(!strcmp(mode, "no")) state->SetTransformMode (false);
        else if(!strcmp(mode, "yes")) state->SetTransformMode (true);
        else 
        {
          synldr->ReportError ("crystalspace.particles.factory.loader",
            child, "Unknown transform mode parameter '%s'!", mode);
        }
        break;
      }
      case XMLTOKEN_PHYSICSPLUGIN:
        state->SetPhysicsPlugin (child->GetContentsValue ());
        break;
      case XMLTOKEN_DAMPENER:
        state->SetDampener (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_COLORMETHOD:
      {
        const char *str = child->GetAttributeValue ("type");
        if (!str)
        {
          synldr->ReportError ("crystalspace.particles.factory.loader",
            child, "No color method type specified!");
          return 0;
        }
        if (!strcmp (str, "constant"))
        {
          ParseColorConstant (child, state);
        }
        else if (!strcmp (str, "linear"))
        {
          ParseColorLinear (child, state);
        }
        else if (!strcmp (str, "looping"))
        {
          ParseColorLooping (child, state);
        }
        else if (!strcmp (str, "heat"))
        {
          ParseColorHeat (child, state);
        }
        else if (!strcmp (str, "callback"))
        {
          synldr->ReportError ("crystalspace.particles.factory.loader",
            child, "You cannot specify callback color method in loader!");
        }
        else
        {
          synldr->ReportError ("crystalspace.particles.factory.loader",
            child, "Unknown color method '%s'!", str);
        }
        break;
      }
      case XMLTOKEN_MIXMODE:
	{
	  uint mixmode;
	  if (!synldr->ParseMixmode (child, mixmode)) return 0;
	  state->SetMixMode (mixmode);
	}
      default:
        synldr->ReportError ("crystalspace.particles.factory.loader",
          child, "Unknown token '%s'!", value);
    }
  }

  return csPtr<iBase> (fact);
}

bool csParticlesFactoryLoader::ParseEmitter (iDocumentNode *node,
  iParticlesFactoryState *state)
{
  const char *type = node->GetAttributeValue ("type");
  if (!type) {
    synldr->ReportError ("crystalspace.particles.factory.loader",
      node, "No type specified for emitter!");
    return false;
  }

  float x_size = 0.0f, y_size = 0.0f, z_size = 0.0f;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_OUTERRADIUS:
        x_size = child->GetContentsValueAsFloat ();
        break;
      case XMLTOKEN_INNERRADIUS:
        y_size = child->GetContentsValueAsFloat ();
        break;
      case XMLTOKEN_SIZE:
        x_size = child->GetAttributeValueAsFloat ("x");
        y_size = child->GetAttributeValueAsFloat ("y");
        z_size = child->GetAttributeValueAsFloat ("z");
        break;
      case XMLTOKEN_TIME:
        state->SetEmitTime (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_FORCE:
        ParseForce (child, state);
        break;
      default:
        synldr->ReportError ("crystalspace.particles.factory.loader",
          child, "Unknown token '%s'!", value);
    }
  }

  if (!strcmp (type, "point"))
  {
    state->SetPointEmitType ();
  }
  else if (!strcmp (type, "sphere"))
  {
    state->SetSphereEmitType (x_size, y_size);;
  }
  else if (!strcmp (type, "plane"))
  {
    state->SetPlaneEmitType (x_size, y_size);
  }
  else if (!strcmp (type, "box"))
  {
    state->SetBoxEmitType (x_size, y_size, z_size);
  }
  else if (!strcmp (type, "cylinder"))
  {
    state->SetCylinderEmitType (x_size, y_size);
  }
  else
  {
    synldr->ReportError ("crystalspace.particles.factory.loader",
      node, "Unknown emitter type '%s'!", type);
    return false;
  }
  return true;
}

bool csParticlesFactoryLoader::ParseForce (iDocumentNode *node,
  iParticlesFactoryState *state)
{
  const char *type = node->GetAttributeValue ("type");
  if (!type) {
    synldr->ReportError ("crystalspace.particles.factory.loader",
      node, "No type specified for force!");
    return false;
  }

  csVector3 direction (0.0f, 0.0f, 0.0f);
  float range = 0.0f, radius = 0.0f;
  csParticleFalloffType falloff = CS_PART_FALLOFF_LINEAR,
    radius_falloff = CS_PART_FALLOFF_LINEAR;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_RANGE:
        range = child->GetContentsValueAsFloat ();
        break;
      case XMLTOKEN_FALLOFF:
      {
        const char *str = child->GetContentsValue ();
        if (!str)
        {
          synldr->ReportError ("crystalspace.particles.factory.loader",
            child, "No falloff type specified!");
          return 0;
        }
        if (!strcmp (str, "constant"))
	        falloff = CS_PART_FALLOFF_CONSTANT;
        else if (!strcmp (str, "linear"))
	        falloff = CS_PART_FALLOFF_LINEAR;
        else if (!strcmp (str, "parabolic"))
	        falloff = CS_PART_FALLOFF_PARABOLIC;
        else
        {
          synldr->ReportError ("crystalspace.particles.factory.loader",
            child, "Unknown falloff type '%s'!", str);
        }
        break;
      }
      case XMLTOKEN_DIRECTION:
        synldr->ParseVector (child, direction);
        direction.Normalize ();
        break;
      case XMLTOKEN_CONERADIUS:
        radius = child->GetContentsValueAsFloat ();
        break;
      case XMLTOKEN_CONEFALLOFF:
      {
        const char *str = child->GetContentsValue ();
        if (!str)
        {
          synldr->ReportError ("crystalspace.particles.factory.loader",
            child, "No cone falloff type specified!");
          return false;
        }
        if (!strcmp (str, "constant"))
	        radius_falloff = CS_PART_FALLOFF_CONSTANT;
        else if (!strcmp (str, "linear"))
	        radius_falloff = CS_PART_FALLOFF_LINEAR;
        else if (!strcmp (str, "parabolic"))
	        radius_falloff = CS_PART_FALLOFF_PARABOLIC;
        else
        {
          synldr->ReportError ("crystalspace.particles.factory.loader",
            child, "Unknown cone falloff type '%s'!", str);
        }
        break;
      }
      case XMLTOKEN_AMOUNT:
        state->SetForce (child->GetContentsValueAsFloat ());
        break;
      default:
        synldr->ReportError ("crystalspace.particles.factory.loader",
          child, "Unknown token '%s'!", value);
    }
  }

  if (!strcmp(type, "radial"))
  {
    state->SetRadialForceType (range, falloff);
  }
  else if (!strcmp(type, "linear"))
  {
    state->SetLinearForceType (direction, range, falloff);
  }
  else if (!strcmp(type, "cone"))
  {
    state->SetConeForceType(direction, range, falloff, radius, radius_falloff);
  }
  else
  {
    synldr->ReportError ("crystalspace.particles.factory.loader",
      node, "Unknown force type '%s'!", type);
    return false;
  }
  return true;
}

bool csParticlesFactoryLoader::ParseColorConstant (iDocumentNode *node,
  iParticlesFactoryState *state)
{
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  bool method_set = false;
  while (it->HasNext())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_COLOR:
      {
        csColor4 color;
        synldr->ParseColor (child, color);
        state->SetConstantColorMethod (color);
        method_set = true;
        break;
      }
      default:
        synldr->ReportError ("crystalspace.particles.factory.loader",
          child, "Unknown token '%s'!", value);
    }
  }
  if(!method_set)
  {
     synldr->ReportError ("crystalspace.particles.factory.loader",
          node, "No constant color specified!");
  }
  return true;
}

bool csParticlesFactoryLoader::ParseColorLinear (iDocumentNode *node,
  iParticlesFactoryState *state)
{
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  bool method_set = false;
  while (it->HasNext())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_GRADIENT:
      {
        ParseGradient (child, state);
        state->SetLinearColorMethod ();
        method_set = true;
        break;
      }
      default:
        synldr->ReportError ("crystalspace.particles.factory.loader",
          child, "Unknown token '%s'!", value);
    }
  }
  if(!method_set)
  {
     synldr->ReportError ("crystalspace.particles.factory.loader",
          node, "No gradient specified!");
  }
  return true;
}

bool csParticlesFactoryLoader::ParseColorLooping (iDocumentNode *node,
  iParticlesFactoryState *state)
{
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  int actions = 0;
  while (it->HasNext())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_GRADIENT:
      {
        ParseGradient(child, state);
        actions |= 1;
        break;
      }
      case XMLTOKEN_TIME:
      {
        float time = child->GetContentsValueAsFloat ();
        state->SetLoopingColorMethod (time);
        actions |= 2;
        break;
      }
      default:
        synldr->ReportError ("crystalspace.particles.factory.loader",
          child, "Unknown token '%s'!", value);
    }
  }
  if(actions != 3)
  {
     synldr->ReportError ("crystalspace.particles.factory.loader",
          node, "You must specify a gradient and loop time!");
  }
  return true;
}

bool csParticlesFactoryLoader::ParseColorHeat (iDocumentNode *node,
  iParticlesFactoryState *state)
{
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  bool method_set = false;
  while (it->HasNext())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_TEMP:
      {
        float base_heat = child->GetContentsValueAsFloat ();
        state->SetHeatColorMethod ((int)base_heat);
        method_set = true;
        break;
      }
      default:
        synldr->ReportError ("crystalspace.particles.factory.loader",
          child, "Unknown token '%s'!", value);
    }
  }
  if(!method_set)
  {
     synldr->ReportError ("crystalspace.particles.factory.loader",
          node, "You must specify a base heat (temp)!");
  }
  return true;
}

bool csParticlesFactoryLoader::ParseGradient (iDocumentNode *node,
  iParticlesFactoryState *state)
{
  state->ClearColors ();

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_COLOR:
      {
        csColor4 color;
        synldr->ParseColor (child, color);
        state->AddColor (color);
        break;
      }
      default:
        synldr->ReportError ("crystalspace.particles.factory.loader",
          child, "Unknown token '%s'!", value);
    }
  }
  return true;
}

SCF_IMPLEMENT_IBASE (csParticlesFactorySaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csParticlesFactorySaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csParticlesFactorySaver)

csParticlesFactorySaver::csParticlesFactorySaver (iBase* parent)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
}

csParticlesFactorySaver::~csParticlesFactorySaver ()
{
}

bool csParticlesFactorySaver::Initialize (iObjectRegistry* objreg)
{
  object_reg = objreg;
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  return true;
}

bool csParticlesFactorySaver::WriteDown (iBase* obj, iDocumentNode* parent)
{
  if (!parent) return false; //you never know...
  if (!obj)    return false; //you never know...

  csRef<iMeshObjectFactory> fact = 
    SCF_QUERY_INTERFACE(obj, iMeshObjectFactory);
  csRef<iParticlesFactoryState> state =
    SCF_QUERY_INTERFACE(obj, iParticlesFactoryState);

  if (fact && state)
  {
    csRef<iDocumentNode> paramsNode = 
      parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    paramsNode->SetValue("params");

    //TBD
    //Writedown Material tag
    iMaterialWrapper* mat = 0; //state->GetMaterialWrapper();
    if (mat)
    {
      const char* matname = mat->QueryObject()->GetName();
      if (matname && *matname)
      {
        csRef<iDocumentNode> matNode = 
          paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        matNode->SetValue("material");
        csRef<iDocumentNode> matnameNode = 
          matNode->CreateNodeBefore(CS_NODE_TEXT, 0);
        matnameNode->SetValue(matname);
      }    
    }    

    //Writedown emitter tag
    WriteEmitter(state, paramsNode);

    //Writedown dampener tag:
    float damp = state->GetDampener ();
    csRef<iDocumentNode> dampNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    dampNode->SetValue("dampener");
    dampNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(damp);

    //Writedown mass tag
    float mass = state->GetMass ();
    csRef<iDocumentNode> massNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    massNode->SetValue("mass");
    massNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(mass);

    //Writedown massvar tag
    float massvar = state->GetMassVariation ();
    csRef<iDocumentNode> massvarNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    massvarNode->SetValue("massvariation");
    massvarNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(massvar);

    //Writedown partpersec tag
    int pps = state->GetParticlesPerSecond ();
    csRef<iDocumentNode> ppsNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    ppsNode->SetValue("pps");
    ppsNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsInt(pps);

    //Writedown initial tag
    int initial = state->GetInitialParticleCount ();
    csRef<iDocumentNode> initialNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    initialNode->SetValue("initial");
    initialNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsInt(initial);

    //Writedown gravity tag
    csVector3 gravity;
    state->GetGravity(gravity);
    csRef<iDocumentNode> gravityNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    gravityNode->SetValue("gravity");
    synldr->WriteVector(gravityNode, &gravity);

    //Writedown diffusion tag
    float diffuse = state->GetDiffusion ();
    csRef<iDocumentNode> diffuseNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    diffuseNode->SetValue("diffusion");
    diffuseNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(diffuse);

    //Writedown radius tag
    float radius = state->GetParticleRadius ();
    csRef<iDocumentNode> radiusNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    radiusNode->SetValue("radius");
    radiusNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(radius);

    //Writedown timetolive tag (ttl)
    float ttl = state->GetTimeToLive ();
    csRef<iDocumentNode> ttlNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    ttlNode->SetValue("ttl");
    ttlNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(ttl);

    //Writedown timevariation tag
    float tv = state->GetTimeVariation ();
    csRef<iDocumentNode> tvNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    tvNode->SetValue("timevariation");
    tvNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(tv);

    //TBD
    //case XMLTOKEN_AUTOSTART:
    //state->GetAutoStart ();

    //case XMLTOKEN_TRANSFORM_MODE:
    synldr->WriteBool(paramsNode, "transformnode", 
                      state->GetTransformMode(), false);

    //TBD
    //case XMLTOKEN_PHYSICS_PLUGIN:
    //const char* pysplug = state->GetPhysicsPlugin ();

    //Write colormethod tags
    WriteColorMethode(state, paramsNode);
  }
  return true;
}

//TBD
bool csParticlesFactorySaver::WriteEmitter (iParticlesFactoryState* state,
                                            iDocumentNode* parent)
{
  csRef<iDocumentNode> emitterNode =
    parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  emitterNode->SetValue("emitter");
  
  //Write time tags
  float emittime = state->GetEmitTime();
  csRef<iDocumentNode> emittimeNode =
    emitterNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  emittimeNode->SetValue("time");
  emittimeNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(emittime);

  switch (state->GetEmitType())
  {
    case CS_PART_EMIT_BOX:
    {
      //float x = state->GetEmitXSize();
      //float y = state->GetEmitYSize();
      //float z = state->GetEmitZSize();
      break;
    }

    case CS_PART_EMIT_CYLINDER:
    {
      break;
    }

    case CS_PART_EMIT_PLANE:
    {
      break;
    }

    case CS_PART_EMIT_SPHERE:
    {
      emitterNode->SetAttribute("type", "sphere");

      float innerradius = state->GetEmitXSize();
      csRef<iDocumentNode> innerradiusNode =
        emitterNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      innerradiusNode->SetValue("innerradius");
      innerradiusNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(innerradius);
      
      float outerradius = state->GetEmitYSize();
      csRef<iDocumentNode> outerradiusNode =
        emitterNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      outerradiusNode->SetValue("outerradius");
      outerradiusNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(outerradius);

      break;
    }
  }

  //paramsNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValue(matname);

  //Write force tags
  csRef<iDocumentNode> forceNode =
    emitterNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  forceNode->SetValue("force");

  switch (state->GetForceType())
  {
    case CS_PART_FORCE_RADIAL:
    {
      break;
    }

    case CS_PART_FORCE_LINEAR:
    {
      forceNode->SetAttribute("type", "linear");

      csVector3 direction;
      state->GetForceDirection(direction);
      csRef<iDocumentNode> directionNode =
        forceNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      directionNode->SetValue("direction");
      synldr->WriteVector(directionNode, &direction);

      float amount = state->GetForce();
      csRef<iDocumentNode> amountNode =
        forceNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      amountNode->SetValue("amount");
      amountNode->CreateNodeBefore(CS_NODE_TEXT,0)->SetValueAsFloat(amount);

      float range = state->GetForceRange();
      csRef<iDocumentNode> rangeNode =
        forceNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      rangeNode->SetValue("range");
      rangeNode->CreateNodeBefore(CS_NODE_TEXT,0)->SetValueAsFloat(range);

      break;
    }

    case CS_PART_FORCE_CONE:
    {
      float radius = state->GetForceConeRadius();
      csRef<iDocumentNode> radiusNode =
        forceNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      radiusNode->SetValue("radius");
      radiusNode->CreateNodeBefore(CS_NODE_TEXT,0)->SetValueAsFloat(radius);

      break;
    }
  }


  return true;
}

//TBD
bool csParticlesFactorySaver::WriteColorMethode (iParticlesFactoryState* state,
                                                 iDocumentNode* parent)
{
  //state->GetConstantColor();

  return true;
}

SCF_IMPLEMENT_IBASE (csParticlesObjectLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csParticlesObjectLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csParticlesObjectLoader)

csParticlesObjectLoader::csParticlesObjectLoader (iBase* parent)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);

  InitTokenTable (xmltokens);
}

csParticlesObjectLoader::~csParticlesObjectLoader ()
{
}

bool csParticlesObjectLoader::Initialize (iObjectRegistry* objreg)
{
  object_reg = objreg;
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  vfs = CS_QUERY_REGISTRY (object_reg, iVFS);

  return true;
}

csPtr<iBase> csParticlesObjectLoader::Parse (iDocumentNode* node,
  iLoaderContext* ldr_context, iBase* context)
{
  csRef<iMeshObject> mesh;
  csRef<iParticlesObjectState> state;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_FACTORY:
      {
        const char* factname = child->GetContentsValue ();
        iMeshFactoryWrapper* fact = ldr_context->FindMeshFactory (factname);
        if (!fact)
        {
      	  synldr->ReportError ("crystalspace.particles.object.loader",
            child, "Couldn't find factory '%s'!", factname);
          return 0;
        }
        mesh = fact->GetMeshObjectFactory ()->NewInstance ();
        state = SCF_QUERY_INTERFACE(mesh, iParticlesObjectState);
	if (!state)
	{
      	  synldr->ReportError (
		"crystalspace.particles.parse.badfactory", child,
                "Factory '%s' doesn't appear to be a particles factory!",
		factname);
	  return 0;
	}
        break;
      }
      case XMLTOKEN_COLOR:
      {
        csColor color;
        synldr->ParseColor (child, color);
        mesh->SetColor (color);
        break;
      }
      case XMLTOKEN_MATERIAL:
      {
        const char* matname = child->GetContentsValue ();
        iMaterialWrapper* mat = ldr_context->FindMaterial (matname);
        if (!mat)
        {
          synldr->ReportError("crystalspace.ballloader.parse.unknownmaterial",
	    child, "Couldn't find material '%s'!", matname);
          return 0;
        }
        mesh->SetMaterialWrapper (mat);
        break;
      }
      case XMLTOKEN_EMITTER:
        ParseEmitter (child, state);
        break;
      case XMLTOKEN_DIFFUSION:
        state->SetDiffusion (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_GRAVITY:
      {
        csVector3 gravity;
        synldr->ParseVector (child, gravity);
        state->SetGravity (gravity);
        break;
      }
      case XMLTOKEN_TTL:
        state->SetTimeToLive (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_TIMEVARIATION:
        state->SetTimeVariation (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_INITIAL:
        state->SetInitialParticleCount (child->GetContentsValueAsInt ());
        break;
      case XMLTOKEN_PPS:
        state->SetParticlesPerSecond (child->GetContentsValueAsInt ());
        break;
      case XMLTOKEN_RADIUS:
        state->SetParticleRadius (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_MASS:
        state->SetMass (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_MASSVARIATION:
        state->SetMassVariation (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_TRANSFORMMODE:
      {
        const char *mode = child->GetContentsValue ();
        if(!strcmp(mode, "no")) state->SetTransformMode (false);
        else if(!strcmp(mode, "yes")) state->SetTransformMode (true);
        else 
        {
          synldr->ReportError ("crystalspace.particles.factory.loader",
            child, "Unknown transform mode parameter '%s'!", mode);
        }
        break;
      }
      case XMLTOKEN_PHYSICSPLUGIN:
        state->ChangePhysicsPlugin (child->GetContentsValue ());
        break;
      case XMLTOKEN_DAMPENER:
        state->SetDampener (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_COLORMETHOD:
      {
        const char *str = child->GetAttributeValue ("type");
        if (!str)
        {
          synldr->ReportError ("crystalspace.particles.factory.loader",
            child, "No color method type specified!");
          return 0;
        }
        if (!strcmp (str, "constant"))
        {
          ParseColorConstant (child, state);
        }
        else if (!strcmp (str, "linear"))
        {
          ParseColorLinear (child, state);
        }
        else if (!strcmp (str, "looping"))
        {
          ParseColorLooping (child, state);
        }
        else if (!strcmp (str, "heat"))
        {
          ParseColorHeat (child, state);
        }
        else if (!strcmp (str, "callback"))
        {
          synldr->ReportError ("crystalspace.particles.factory.loader",
            child, "You cannot specify callback color method in loader!");
        }
        else
        {
          synldr->ReportError ("crystalspace.particles.factory.loader",
            child, "Unknown color method '%s'!", str);
        }
        break;
      }
      case XMLTOKEN_MIXMODE:
	{
	  uint mixmode;
	  if (!synldr->ParseMixmode (child, mixmode)) return 0;
	  state->SetMixMode (mixmode);
	}
      default:
        synldr->ReportError ("crystalspace.particles.object.loader",
          child, "Unknown token '%s'!", value);
    }
  }

  return csPtr<iBase>(mesh);
}

bool csParticlesObjectLoader::ParseEmitter (iDocumentNode *node,
  iParticlesObjectState *state)
{
  const char *type = node->GetAttributeValue ("type");
  if (!type)
  {
    synldr->ReportError ("crystalspace.particles.object.loader",
      node, "No type specified for emitter!");
    return false;
  }

  float x_size = 0.0f, y_size = 0.0f, z_size = 0.0f;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_OUTERRADIUS:
        x_size = child->GetContentsValueAsFloat ();
        break;
      case XMLTOKEN_INNERRADIUS:
        y_size = child->GetContentsValueAsFloat ();
        break;
      case XMLTOKEN_SIZE:
        x_size = child->GetAttributeValueAsFloat ("x");
        y_size = child->GetAttributeValueAsFloat ("y");
        z_size = child->GetAttributeValueAsFloat ("z");
        break;
      case XMLTOKEN_TIME:
        state->SetEmitTime (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_FORCE:
        ParseForce (child, state);
        break;
      default:
        synldr->ReportError ("crystalspace.particles.factory.loader",
          child, "Unknown token '%s'!", value);
    }
  }

  if (!strcmp (type, "point"))
  {
    state->SetPointEmitType ();
  }
  else if (!strcmp (type, "sphere"))
  {
    state->SetSphereEmitType (x_size, y_size);;
  }
  else if (!strcmp (type, "plane"))
  {
    state->SetPlaneEmitType (x_size, y_size);
  }
  else if (!strcmp (type, "box"))
  {
    state->SetBoxEmitType (x_size, y_size, z_size);
  }
  else if (!strcmp (type, "cylinder"))
  {
    state->SetCylinderEmitType (x_size, y_size);
  }
  else
  {
    synldr->ReportError ("crystalspace.particles.object.loader",
      node, "Unknown emitter type '%s'!", type);
    return false;
  }
  return true;
}

bool csParticlesObjectLoader::ParseForce (iDocumentNode *node,
  iParticlesObjectState *state)
{
  const char *type = node->GetAttributeValue ("type");
  if (!type)
  {
    synldr->ReportError ("crystalspace.particles.object.loader",
      node, "No type specified for force!");
    return false;
  }

  csVector3 direction (0.0f, 0.0f, 0.0f);
  float range = 0.0f, radius = 0.0f;
  csParticleFalloffType falloff = CS_PART_FALLOFF_LINEAR,
    radius_falloff = CS_PART_FALLOFF_LINEAR;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_RANGE:
        range = child->GetContentsValueAsFloat ();
        break;
      case XMLTOKEN_FALLOFF:
      {
        const char *str = child->GetContentsValue ();
        if (!str)
        {
          synldr->ReportError ("crystalspace.particles.object.loader",
            child, "No falloff type specified!");
          return false;
        }
        if (!strcmp (str, "constant"))
      	  falloff = CS_PART_FALLOFF_CONSTANT;
        else if (!strcmp (str, "linear"))
	        falloff = CS_PART_FALLOFF_LINEAR;
        else if (!strcmp (str, "parabolic"))
	        falloff = CS_PART_FALLOFF_PARABOLIC;
        else
        {
          synldr->ReportError ("crystalspace.particles.object.loader",
            child, "Unknown falloff type '%s'!", str);
        }
        break;
      }
      case XMLTOKEN_DIRECTION:
        synldr->ParseVector (child, direction);
        direction.Normalize ();
        break;
      case XMLTOKEN_CONERADIUS:
        radius = child->GetContentsValueAsFloat ();
        break;
      case XMLTOKEN_CONEFALLOFF:
      {
        const char *str = child->GetContentsValue ();
        if (!str)
        {
          synldr->ReportError ("crystalspace.particles.object.loader",
            child, "No cone falloff type specified!");
          return false;
        }
        if (!strcmp (str, "constant"))
      	  radius_falloff = CS_PART_FALLOFF_CONSTANT;
        else if (!strcmp (str, "linear"))
	        radius_falloff = CS_PART_FALLOFF_LINEAR;
        else if (!strcmp (str, "parabolic"))
	        radius_falloff = CS_PART_FALLOFF_PARABOLIC;
        else
        {
          synldr->ReportError ("crystalspace.particles.object.loader",
            child, "Unknown cone falloff type '%s'!", str);
        }
        break;
      }
      case XMLTOKEN_AMOUNT:
        state->SetForce (child->GetContentsValueAsFloat ());
        break;
      default:
        synldr->ReportError ("crystalspace.particles.object.loader",
          child, "Unknown token '%s'!", value);
    }
  }

  if (!strcmp(type, "radial"))
  {
    state->SetRadialForceType (range, falloff);
  }
  else if (!strcmp(type, "linear"))
  {
    state->SetLinearForceType (direction, range, falloff);
  }
  else if (!strcmp(type, "cone"))
  {
    state->SetConeForceType(direction, range, falloff, radius,
      radius_falloff);
  }
  else
  {
    synldr->ReportError ("crystalspace.particles.object.loader",
      node, "Unknown force type '%s'!", type);
    return false;
  }
  return true;
}

bool csParticlesObjectLoader::ParseColorConstant (iDocumentNode *node,
  iParticlesObjectState *state)
{
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  bool method_set = false;
  while (it->HasNext())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_COLOR:
      {
        csColor4 color;
        synldr->ParseColor (child, color);
        state->SetConstantColorMethod (color);
        method_set = true;
        break;
      }
      default:
        synldr->ReportError ("crystalspace.particles.object.loader",
          child, "Unknown token '%s'!", value);
    }
  }
  if(!method_set)
  {
     synldr->ReportError ("crystalspace.particles.object.loader",
          node, "No constant color specified!");
  }
  return true;
}

bool csParticlesObjectLoader::ParseColorLinear (iDocumentNode *node,
  iParticlesObjectState *state)
{
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  bool method_set = false;
  while (it->HasNext())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_GRADIENT:
      {
        ParseGradient (child, state);
        state->SetLinearColorMethod ();
        method_set = true;
        break;
      }
      default:
        synldr->ReportError ("crystalspace.particles.object.loader",
          child, "Unknown token '%s'!", value);
    }
  }
  if(!method_set)
  {
     synldr->ReportError ("crystalspace.particles.object.loader",
          node, "No gradient specified!");
  }
  return true;
}

bool csParticlesObjectLoader::ParseColorLooping (iDocumentNode *node,
  iParticlesObjectState *state)
{
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  int actions = 0;
  while (it->HasNext())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_GRADIENT:
      {
        ParseGradient(child, state);
        actions |= 1;
        break;
      }
      case XMLTOKEN_TIME:
      {
        float time = child->GetContentsValueAsFloat ();
        state->SetLoopingColorMethod (time);
        actions |= 2;
        break;
      }
      default:
        synldr->ReportError ("crystalspace.particles.object.loader",
          child, "Unknown token '%s'!", value);
    }
  }
  if(actions != 3)
  {
     synldr->ReportError ("crystalspace.particles.object.loader",
          node, "You must specify a gradient and loop time!");
  }
  return true;
}

bool csParticlesObjectLoader::ParseColorHeat (iDocumentNode *node,
  iParticlesObjectState *state)
{
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  bool method_set = false;
  while (it->HasNext())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_TEMP:
      {
        float base_heat = child->GetContentsValueAsFloat ();
        state->SetHeatColorMethod ((int)base_heat);
        method_set = true;
        break;
      }
      default:
        synldr->ReportError ("crystalspace.particles.object.loader",
          child, "Unknown token '%s'!", value);
    }
  }
  if(!method_set)
  {
     synldr->ReportError ("crystalspace.particles.object.loader",
          node, "You must specify a base heat (temp)!");
  }
  return true;
}

bool csParticlesObjectLoader::ParseGradient (iDocumentNode *node,
  iParticlesObjectState *state)
{
  state->ClearColors ();

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_COLOR:
      {
        csColor4 color;
        synldr->ParseColor (child, color);
        state->AddColor (color);
        break;
      }
      default:
        synldr->ReportError ("crystalspace.particles.object.loader",
          child, "Unknown token '%s'!", value);
    }
  }
  return true;
}

SCF_IMPLEMENT_IBASE (csParticlesObjectSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csParticlesObjectSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csParticlesObjectSaver)

csParticlesObjectSaver::csParticlesObjectSaver (iBase* parent)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
}

csParticlesObjectSaver::~csParticlesObjectSaver ()
{
}

bool csParticlesObjectSaver::Initialize (iObjectRegistry *objreg)
{
  object_reg = objreg;
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  return true;
}


bool csParticlesObjectSaver::WriteDown (iBase* obj, iDocumentNode* parent)
{
  if (!parent) return false; //you never know...
  if (!obj)    return false; //you never know...

  csRef<iMeshObject> mesh = 
    SCF_QUERY_INTERFACE(obj, iMeshObject);
  csRef<iParticlesObjectState> object =
    SCF_QUERY_INTERFACE(obj, iParticlesObjectState);

  if (mesh && object)
  {
    csRef<iDocumentNode> paramsNode = 
      parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    paramsNode->SetValue("params");

    //Writedown factory tag
    csRef<iMeshWrapper> meshwrap = SCF_QUERY_INTERFACE(mesh->GetLogicalParent(), iMeshWrapper);
    iMeshFactoryWrapper* factwrap = meshwrap->GetFactory();
    if (factwrap)
    {
      const char* factname = factwrap->QueryObject()->GetName();
      if (factname && *factname)
      {
        csRef<iDocumentNode> factNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        factNode->SetValue("factory");
        factNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValue(factname);
      }    
    }

    //TBD
    //Writedown Material tag
    iMaterialWrapper* mat = 0; //object->GetMaterialWrapper();
    if (mat)
    {
      const char* matname = mat->QueryObject()->GetName();
      if (matname && *matname)
      {
        csRef<iDocumentNode> matNode = 
          paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        matNode->SetValue("material");
        csRef<iDocumentNode> matnameNode = 
          matNode->CreateNodeBefore(CS_NODE_TEXT, 0);
        matnameNode->SetValue(matname);
      }    
    }    

    //Writedown emitter tag
    WriteEmitter(object, paramsNode);

    //Writedown dampener tag:
    float damp = object->GetDampener ();
    csRef<iDocumentNode> dampNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    dampNode->SetValue("dampener");
    dampNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(damp);

    //Writedown mass tag
    float mass = object->GetMass ();
    csRef<iDocumentNode> massNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    massNode->SetValue("mass");
    massNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(mass);

    //Writedown massvar tag
    float massvar = object->GetMassVariation ();
    csRef<iDocumentNode> massvarNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    massvarNode->SetValue("massvariation");
    massvarNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(massvar);

    //Writedown partpersec tag
    //int pps = object->GetParticlesPerSecond ();
    //csRef<iDocumentNode> ppsNode = 
    //  paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    //ppsNode->SetValue("pps");
    //ppsNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsInt(pps);

    //Writedown initial tag
    //int initial = object->GetInitialParticleCount ();
    //csRef<iDocumentNode> initialNode = 
    //  paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    //initialNode->SetValue("initial");
    //initialNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsInt(initial);

    //Writedown gravity tag
    csVector3 gravity;
    object->GetGravity(gravity);
    csRef<iDocumentNode> gravityNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    gravityNode->SetValue("gravity");
    synldr->WriteVector(gravityNode, &gravity);

    //Writedown diffusion tag
    float diffuse = object->GetDiffusion ();
    csRef<iDocumentNode> diffuseNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    diffuseNode->SetValue("diffusion");
    diffuseNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(diffuse);

    //Writedown radius tag
    float radius = object->GetParticleRadius ();
    csRef<iDocumentNode> radiusNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    radiusNode->SetValue("radius");
    radiusNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(radius);

    //Writedown timetolive tag (ttl)
    float ttl = object->GetTimeToLive ();
    csRef<iDocumentNode> ttlNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    ttlNode->SetValue("ttl");
    ttlNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(ttl);

    //Writedown timevariation tag
    float tv = object->GetTimeVariation ();
    csRef<iDocumentNode> tvNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    tvNode->SetValue("timevariation");
    tvNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(tv);

    //TBD
    //case XMLTOKEN_AUTOSTART:
    //object->GetAutoStart ();

    //case XMLTOKEN_TRANSFORM_MODE:
    synldr->WriteBool(paramsNode, "transformnode", 
                      object->GetTransformMode(), false);

    //TBD
    //case XMLTOKEN_PHYSICS_PLUGIN:
    //const char* pysplug = object->GetPhysicsPlugin ();

    //Write colormethod tags
    WriteColorMethode(object, paramsNode);
  }
  return true;
}

//TBD
bool csParticlesObjectSaver::WriteEmitter (iParticlesObjectState* object,
                                            iDocumentNode* parent)
{
  csRef<iDocumentNode> emitterNode =
    parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  emitterNode->SetValue("emitter");
  
  //Write time tags
  float emittime = object->GetEmitTime();
  csRef<iDocumentNode> emittimeNode =
    emitterNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  emittimeNode->SetValue("time");
  emittimeNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(emittime);

  switch (object->GetEmitType())
  {
    case CS_PART_EMIT_BOX:
    {
      //float x = object->GetEmitXSize();
      //float y = object->GetEmitYSize();
      //float z = object->GetEmitZSize();
      break;
    }

    case CS_PART_EMIT_CYLINDER:
    {
      break;
    }

    case CS_PART_EMIT_PLANE:
    {
      break;
    }

    case CS_PART_EMIT_SPHERE:
    {
      emitterNode->SetAttribute("type", "sphere");

      float innerradius = object->GetEmitXSize();
      csRef<iDocumentNode> innerradiusNode =
        emitterNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      innerradiusNode->SetValue("innerradius");
      innerradiusNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(innerradius);
      
      float outerradius = object->GetEmitYSize();
      csRef<iDocumentNode> outerradiusNode =
        emitterNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      outerradiusNode->SetValue("outerradius");
      outerradiusNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(outerradius);

      break;
    }
  }

  //paramsNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValue(matname);

  //Write force tags
  csRef<iDocumentNode> forceNode =
    emitterNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  forceNode->SetValue("force");

  switch (object->GetForceType())
  {
    case CS_PART_FORCE_RADIAL:
    {
      break;
    }

    case CS_PART_FORCE_LINEAR:
    {
      forceNode->SetAttribute("type", "linear");

      csVector3 direction;
      object->GetForceDirection(direction);
      csRef<iDocumentNode> directionNode =
        forceNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      directionNode->SetValue("direction");
      synldr->WriteVector(directionNode, &direction);

      float amount = object->GetForce();
      csRef<iDocumentNode> amountNode =
        forceNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      amountNode->SetValue("amount");
      amountNode->CreateNodeBefore(CS_NODE_TEXT,0)->SetValueAsFloat(amount);

      float range = object->GetForceRange();
      csRef<iDocumentNode> rangeNode =
        forceNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      rangeNode->SetValue("range");
      rangeNode->CreateNodeBefore(CS_NODE_TEXT,0)->SetValueAsFloat(range);

      break;
    }

    case CS_PART_FORCE_CONE:
    {
      float radius = object->GetForceConeRadius();
      csRef<iDocumentNode> radiusNode =
        forceNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      radiusNode->SetValue("radius");
      radiusNode->CreateNodeBefore(CS_NODE_TEXT,0)->SetValueAsFloat(radius);

      break;
    }
  }


  return true;
}

//TBD
bool csParticlesObjectSaver::WriteColorMethode (iParticlesObjectState* object,
                                                 iDocumentNode* parent)
{
  //state->GetConstantColor();

  return true;
}
