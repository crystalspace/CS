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
#include "cssys/sysfunc.h"

#include "imap/ldrctxt.h"

#include "iutil/comp.h"
#include "iutil/document.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"

#include "iengine/material.h"
#include "iengine/mesh.h"

#include "imesh/object.h"
#include "imesh/particles.h"

#include "ivaria/reporter.h"

#include "particlesldr.h"

CS_IMPLEMENT_PLUGIN

enum
{
  XMLTOKEN_FACTORY = 1,
  XMLTOKEN_MATERIAL,
  XMLTOKEN_COLOR,
  XMLTOKEN_EMITTER,
  XMLTOKEN_INNER_RADIUS,
  XMLTOKEN_OUTER_RADIUS,
  XMLTOKEN_SIZE,
  XMLTOKEN_TIME,
  XMLTOKEN_FORCE,
  XMLTOKEN_AMOUNT,
  XMLTOKEN_RANGE,
  XMLTOKEN_FALLOFF,
  XMLTOKEN_DIRECTION,
  XMLTOKEN_CONE_RADIUS,
  XMLTOKEN_CONE_FALLOFF,
  XMLTOKEN_DIFFUSION,
  XMLTOKEN_GRAVITY,
  XMLTOKEN_TIME_TO_LIVE,
  XMLTOKEN_TIME_VARIATION,
  XMLTOKEN_INITIAL_PARTICLES,
  XMLTOKEN_PARTICLES_PER_SECOND,
  XMLTOKEN_HEAT_FUNCTION,
  XMLTOKEN_GRADIENT,
  XMLTOKEN_RADIUS,
  XMLTOKEN_DAMPENER,
  XMLTOKEN_MASS
};

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
}

csParticlesFactoryLoader::~csParticlesFactoryLoader ()
{
}

bool csParticlesFactoryLoader::Initialize (iObjectRegistry* objreg)
{
  object_reg = objreg;
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  loader = CS_QUERY_REGISTRY (object_reg, iLoader);
  vfs = CS_QUERY_REGISTRY (object_reg, iVFS);

  xmltokens.Register ("color", XMLTOKEN_COLOR);
  xmltokens.Register ("emitter", XMLTOKEN_EMITTER);
  xmltokens.Register ("innerradius", XMLTOKEN_INNER_RADIUS);
  xmltokens.Register ("outerradius", XMLTOKEN_OUTER_RADIUS);
  xmltokens.Register ("size", XMLTOKEN_SIZE);
  xmltokens.Register ("time", XMLTOKEN_TIME);
  xmltokens.Register ("force", XMLTOKEN_FORCE);
  xmltokens.Register ("amount", XMLTOKEN_AMOUNT);
  xmltokens.Register ("range", XMLTOKEN_RANGE);
  xmltokens.Register ("falloff", XMLTOKEN_FALLOFF);
  xmltokens.Register ("direction", XMLTOKEN_DIRECTION);
  xmltokens.Register ("coneradius", XMLTOKEN_CONE_RADIUS);
  xmltokens.Register ("conefalloff", XMLTOKEN_CONE_FALLOFF);
  xmltokens.Register ("diffusion", XMLTOKEN_DIFFUSION);
  xmltokens.Register ("gravity", XMLTOKEN_GRAVITY);
  xmltokens.Register ("ttl", XMLTOKEN_TIME_TO_LIVE);
  xmltokens.Register ("timevariation", XMLTOKEN_TIME_VARIATION);
  xmltokens.Register ("initial", XMLTOKEN_INITIAL_PARTICLES);
  xmltokens.Register ("pps", XMLTOKEN_PARTICLES_PER_SECOND);
  xmltokens.Register ("heatfunction", XMLTOKEN_HEAT_FUNCTION);
  xmltokens.Register ("gradient", XMLTOKEN_GRADIENT);
  xmltokens.Register ("radius", XMLTOKEN_RADIUS);
  xmltokens.Register ("dampener", XMLTOKEN_DAMPENER);
  xmltokens.Register ("mass", XMLTOKEN_MASS);
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
      case XMLTOKEN_EMITTER:
        ParseEmitter (child, state);
        break;
      case XMLTOKEN_FORCE:
        ParseForce (child, state);
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
      case XMLTOKEN_TIME_TO_LIVE:
        state->SetTimeToLive (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_TIME_VARIATION:
        state->SetTimeVariation (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_GRADIENT:
        ParseGradient (child, state);
        break;
      case XMLTOKEN_INITIAL_PARTICLES:
        state->SetInitialParticleCount (child->GetContentsValueAsInt ());
        break;
      case XMLTOKEN_PARTICLES_PER_SECOND:
        state->SetParticlesPerSecond (child->GetContentsValueAsInt ());
        break;
      case XMLTOKEN_RADIUS:
        state->SetParticleRadius (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_MASS:
        state->SetMass (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_DAMPENER:
        state->SetDampener (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_HEAT_FUNCTION:
      {
        csParticleHeatFunction heat = CS_PART_HEAT_SPEED;
        const char *str = child->GetContentsValue ();
        if (!str)
        {
          synldr->ReportError ("crystalspace.particles.factory.loader",
            child, "No falloff type specified!");
          return false;
        }
        if (!strcmp (str, "constant"))
	  heat = CS_PART_HEAT_CONSTANT;
        else if (!strcmp (str, "time_linear"))
	  heat = CS_PART_HEAT_TIME_LINEAR;
        else if (!strcmp (str, "speed"))
	  heat = CS_PART_HEAT_SPEED;
        else
        {
          synldr->ReportError ("crystalspace.particles.factory.loader",
            child, "Unknown heat function '%s'!", str);
        }
        state->SetParticleHeatFunction (heat);
        break;
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
      case XMLTOKEN_OUTER_RADIUS:
        x_size = child->GetContentsValueAsFloat ();
        break;
      case XMLTOKEN_INNER_RADIUS:
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
          synldr->ReportError ("crystalspace.particles.factory.loader",
            child, "Unknown falloff type '%s'!", str);
        }
        break;
      }
      case XMLTOKEN_DIRECTION:
        synldr->ParseVector (child, direction);
        direction.Normalize ();
        break;
      case XMLTOKEN_CONE_RADIUS:
        radius = child->GetContentsValueAsFloat ();
        break;
      case XMLTOKEN_CONE_FALLOFF:
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
        csColor color;
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
  return true;
}

void csParticlesFactorySaver::WriteDown (iBase *obj, iFile *file)
{
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
}

csParticlesObjectLoader::~csParticlesObjectLoader ()
{
}

bool csParticlesObjectLoader::Initialize (iObjectRegistry* objreg)
{
  object_reg = objreg;
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  loader = CS_QUERY_REGISTRY (object_reg, iLoader);
  vfs = CS_QUERY_REGISTRY (object_reg, iVFS);

  xmltokens.Register ("factory", XMLTOKEN_FACTORY);
  xmltokens.Register ("material", XMLTOKEN_MATERIAL);
  xmltokens.Register ("color", XMLTOKEN_COLOR);
  xmltokens.Register ("emitter", XMLTOKEN_EMITTER);
  xmltokens.Register ("innerradius", XMLTOKEN_INNER_RADIUS);
  xmltokens.Register ("outerradius", XMLTOKEN_OUTER_RADIUS);
  xmltokens.Register ("size", XMLTOKEN_SIZE);
  xmltokens.Register ("time", XMLTOKEN_TIME);
  xmltokens.Register ("force", XMLTOKEN_FORCE);
  xmltokens.Register ("amount", XMLTOKEN_AMOUNT);
  xmltokens.Register ("range", XMLTOKEN_RANGE);
  xmltokens.Register ("falloff", XMLTOKEN_FALLOFF);
  xmltokens.Register ("direction", XMLTOKEN_DIRECTION);
  xmltokens.Register ("coneradius", XMLTOKEN_CONE_RADIUS);
  xmltokens.Register ("conefalloff", XMLTOKEN_CONE_FALLOFF);
  xmltokens.Register ("diffusion", XMLTOKEN_DIFFUSION);
  xmltokens.Register ("gravity", XMLTOKEN_GRAVITY);
  xmltokens.Register ("ttl", XMLTOKEN_TIME_TO_LIVE);
  xmltokens.Register ("timevariation", XMLTOKEN_TIME_VARIATION);
  xmltokens.Register ("heatfunction", XMLTOKEN_HEAT_FUNCTION);
  xmltokens.Register ("gradient", XMLTOKEN_GRADIENT);
  xmltokens.Register ("radius", XMLTOKEN_RADIUS);
  xmltokens.Register ("dampener", XMLTOKEN_DAMPENER);
  xmltokens.Register ("mass", XMLTOKEN_MASS);
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
        state = SCF_QUERY_INTERFACE(fact, iParticlesObjectState);
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
          synldr->ReportError ("crystalspace.ballloader.parse.unknownmaterial",
	    child, "Couldn't find material '%s'!", matname);
          return 0;
        }
        mesh->SetMaterialWrapper (mat);
        break;
      }
      case XMLTOKEN_EMITTER:
        ParseEmitter (child, state);
        break;
      case XMLTOKEN_FORCE:
        ParseForce (child, state);
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
      case XMLTOKEN_TIME_TO_LIVE:
        state->SetTimeToLive (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_TIME_VARIATION:
        state->SetTimeVariation (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_GRADIENT:
        ParseGradient (child, state);
        break;
      case XMLTOKEN_INITIAL_PARTICLES:
        state->SetInitialParticleCount (child->GetContentsValueAsInt ());
        break;
      case XMLTOKEN_PARTICLES_PER_SECOND:
        state->SetParticlesPerSecond (child->GetContentsValueAsInt ());
        break;
      case XMLTOKEN_RADIUS:
        state->SetParticleRadius (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_MASS:
        state->SetMass (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_DAMPENER:
        state->SetDampener (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_HEAT_FUNCTION:
      {
        csParticleHeatFunction heat = CS_PART_HEAT_SPEED;
        const char *str = child->GetContentsValue ();
        if (!str)
        {
          synldr->ReportError ("crystalspace.particles.object.loader",
            child, "No falloff type specified!");
          return false;
        }
        if (!strcmp (str, "constant"))
	  heat = CS_PART_HEAT_CONSTANT;
        else if (!strcmp (str, "time_linear"))
	  heat = CS_PART_HEAT_TIME_LINEAR;
        else if (!strcmp (str, "speed"))
	  heat = CS_PART_HEAT_SPEED;
        else
        {
          synldr->ReportError ("crystalspace.particles.object.loader",
            child, "Unknown heat function '%s'!", str);
        }
        state->SetParticleHeatFunction (heat);
        break;
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
  if (!type) {
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
      case XMLTOKEN_OUTER_RADIUS:
        x_size = child->GetContentsValueAsFloat ();
        break;
      case XMLTOKEN_INNER_RADIUS:
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
  if (!type) {
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
      case XMLTOKEN_CONE_RADIUS:
        radius = child->GetContentsValueAsFloat ();
        break;
      case XMLTOKEN_CONE_FALLOFF:
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
    state->SetConeForceType(direction, range, falloff, radius, radius_falloff);
  }
  else
  {
    synldr->ReportError ("crystalspace.particles.object.loader",
      node, "Unknown force type '%s'!", type);
    return false;
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
        csColor color;
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
  return true;
}

void csParticlesObjectSaver::WriteDown (iBase* obj, iFile* file)
{
}
