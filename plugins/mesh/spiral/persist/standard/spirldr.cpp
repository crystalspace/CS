/*
    Copyright (C) 2001 by Jorrit Tyberghein
    Copyright (C) 2001 by W.C.A. Wijngaards

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
#include "csgeom/math3d.h"
#include "csutil/scanstr.h"
#include "csutil/cscolor.h"
#include "csutil/util.h"
#include "spirldr.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iutil/plugin.h"
#include "imesh/partsys.h"
#include "imesh/spiral.h"
#include "ivideo/graph3d.h"
#include "qint.h"
#include "csutil/csstring.h"
#include "iutil/object.h"
#include "iutil/document.h"
#include "iengine/material.h"
#include "iutil/objreg.h"
#include "iutil/vfs.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "imap/ldrctxt.h"
#include "ivaria/reporter.h"

CS_IMPLEMENT_PLUGIN

enum
{
  XMLTOKEN_COLOR = 1,
  XMLTOKEN_FACTORY,
  XMLTOKEN_MATERIAL,
  XMLTOKEN_MIXMODE,
  XMLTOKEN_NUMBER,
  XMLTOKEN_SOURCE,
  XMLTOKEN_PARTICLESIZE,
  XMLTOKEN_PARTICLETIME,
  XMLTOKEN_RADIALSPEED,
  XMLTOKEN_ROTATIONSPEED,
  XMLTOKEN_CLIMBSPEED
};

SCF_IMPLEMENT_IBASE (csSpiralFactoryLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSpiralFactoryLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csSpiralFactorySaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSpiralFactorySaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csSpiralLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSpiralLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csSpiralSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSpiralSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csSpiralFactoryLoader)
SCF_IMPLEMENT_FACTORY (csSpiralFactorySaver)
SCF_IMPLEMENT_FACTORY (csSpiralLoader)
SCF_IMPLEMENT_FACTORY (csSpiralSaver)


csSpiralFactoryLoader::csSpiralFactoryLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csSpiralFactoryLoader::~csSpiralFactoryLoader ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csSpiralFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csSpiralFactoryLoader::object_reg = object_reg;
  return true;
}

csPtr<iBase> csSpiralFactoryLoader::Parse (iDocumentNode* /*node*/,
	iLoaderContext*, iBase* /* context */)
{
  csRef<iPluginManager> plugin_mgr (CS_QUERY_REGISTRY (object_reg,
  	iPluginManager));
  csRef<iMeshObjectType> type (CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.spiral", iMeshObjectType));
  if (!type)
  {
    type = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.spiral",
    	iMeshObjectType);
  }
  csRef<iMeshObjectFactory> fact (type->NewFactory ());
  return csPtr<iBase> (fact);
}

//---------------------------------------------------------------------------
csSpiralFactorySaver::csSpiralFactorySaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csSpiralFactorySaver::~csSpiralFactorySaver ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csSpiralFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csSpiralFactorySaver::object_reg = object_reg;
  return true;
}

#define MAXLINE 100 /* max number of chars per line... */

void csSpiralFactorySaver::WriteDown (iBase* /*obj*/, iFile * /*file*/)
{
  // nothing to do
}
//---------------------------------------------------------------------------
csSpiralLoader::csSpiralLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csSpiralLoader::~csSpiralLoader ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csSpiralLoader::Initialize (iObjectRegistry* object_reg)
{
  csSpiralLoader::object_reg = object_reg;
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);

  xmltokens.Register ("color", XMLTOKEN_COLOR);
  xmltokens.Register ("factory", XMLTOKEN_FACTORY);
  xmltokens.Register ("material", XMLTOKEN_MATERIAL);
  xmltokens.Register ("mixmode", XMLTOKEN_MIXMODE);
  xmltokens.Register ("number", XMLTOKEN_NUMBER);
  xmltokens.Register ("source", XMLTOKEN_SOURCE);
  xmltokens.Register ("particlesize", XMLTOKEN_PARTICLESIZE);
  xmltokens.Register ("particletime", XMLTOKEN_PARTICLETIME);
  xmltokens.Register ("radialspeed", XMLTOKEN_RADIALSPEED);
  xmltokens.Register ("rotationspeed", XMLTOKEN_ROTATIONSPEED);
  xmltokens.Register ("climbspeed", XMLTOKEN_CLIMBSPEED);
  return true;
}

csPtr<iBase> csSpiralLoader::Parse (iDocumentNode* node,
	iLoaderContext* ldr_context, iBase*)
{
  csRef<iMeshObject> mesh;
  csRef<iParticleState> partstate;
  csRef<iSpiralState> spiralstate;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
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
	  if (!synldr->ParseColor (child, color))
	    return 0;
	  partstate->SetColor (color);
	}
	break;
      case XMLTOKEN_SOURCE:
	{
	  csVector3 s;
	  if (!synldr->ParseVector (child, s))
	    return 0;
	  spiralstate->SetSource (s);
	}
	break;
      case XMLTOKEN_FACTORY:
	{
	  const char* factname = child->GetContentsValue ();
	  iMeshFactoryWrapper* fact = ldr_context->FindMeshFactory (factname);
	  if (!fact)
	  {
      	    synldr->ReportError (
		"crystalspace.spiralloader.parse.unknownfactory",
		child, "Couldn't find factory '%s'!", factname);
	    return 0;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          partstate = SCF_QUERY_INTERFACE (mesh, iParticleState);
          spiralstate = SCF_QUERY_INTERFACE (mesh, iSpiralState);
	  if (!spiralstate)
	  {
      	    synldr->ReportError (
		"crystalspace.spiralloader.parse.badfactory",
		child, "Factory '%s' doesn't appear to be a spiral factory!",
		factname);
	    return 0;
	  }
	}
	break;
      case XMLTOKEN_MATERIAL:
	{
	  const char* matname = child->GetContentsValue ();
          iMaterialWrapper* mat = ldr_context->FindMaterial (matname);
	  if (!mat)
	  {
      	    synldr->ReportError (
		"crystalspace.ballloader.parse.unknownmaterial",
		child, "Couldn't find material '%s'!", matname);
            return 0;
	  }
	  partstate->SetMaterialWrapper (mat);
	}
	break;
      case XMLTOKEN_MIXMODE:
	{
	  uint mode;
	  if (!synldr->ParseMixmode (child, mode))
	    return 0;
          partstate->SetMixMode (mode);
	}
	break;
      case XMLTOKEN_NUMBER:
        spiralstate->SetParticleCount (child->GetContentsValueAsInt ());
        break;
      case XMLTOKEN_PARTICLESIZE:
        {
            
	  float dw, dh;
	  dw = child->GetAttributeValueAsFloat ("w");
	  dh = child->GetAttributeValueAsFloat ("h");
          spiralstate->SetParticleSize (dw, dh);
        }
        break;
      case XMLTOKEN_PARTICLETIME:
        spiralstate->SetParticleTime (child->GetContentsValueAsInt ());
        break;
      case XMLTOKEN_RADIALSPEED:
        spiralstate->SetRadialSpeed (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_ROTATIONSPEED:
        spiralstate->SetRotationSpeed (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_CLIMBSPEED:
        spiralstate->SetClimbSpeed (child->GetContentsValueAsFloat ());
        break;
      default:
	synldr->ReportBadToken (child);
	return 0;
    }
  }

  return csPtr<iBase> (mesh);
}

//---------------------------------------------------------------------------

csSpiralSaver::csSpiralSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csSpiralSaver::~csSpiralSaver ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csSpiralSaver::Initialize (iObjectRegistry* object_reg)
{
  csSpiralSaver::object_reg = object_reg;
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  return true;
}

void csSpiralSaver::WriteDown (iBase*, iFile*)
{
}

