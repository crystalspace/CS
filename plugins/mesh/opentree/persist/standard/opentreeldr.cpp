/*
    Copyright (C) 2006 by Christoph "Fossi" Mewes

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
#include "csgeom/tri.h"
#include "csgeom/vector2.h"
#include "csgeom/vector4.h"
#include "csutil/cscolor.h"
#include "csutil/scanstr.h"
#include "csutil/sysfunc.h"
#include "iengine/engine.h"
#include "iengine/material.h"
#include "iengine/mesh.h"
#include "imap/ldrctxt.h"
#include "imap/services.h"
#include "imesh/object.h"
#include "imesh/opentree.h"
#include "iutil/comp.h"
#include "iutil/document.h"
#include "iutil/eventh.h"
#include "iutil/object.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "ivaria/reporter.h"
#include "ivideo/graph3d.h"
#include "ivideo/rndbuf.h"

#include "opentreeldr.h"

CS_IMPLEMENT_PLUGIN

CS_PLUGIN_NAMESPACE_BEGIN(OpenTreeLoader)
{

SCF_IMPLEMENT_FACTORY (csOpenTreeFactoryLoader)
SCF_IMPLEMENT_FACTORY (csOpenTreeMeshLoader)


csOpenTreeFactoryLoader::csOpenTreeFactoryLoader (iBase* pParent) :
  scfImplementationType (this, pParent)
{
}

csOpenTreeFactoryLoader::~csOpenTreeFactoryLoader ()
{
}

bool csOpenTreeFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csOpenTreeFactoryLoader::object_reg = object_reg;
  synldr = csQueryRegistry<iSyntaxService> (object_reg);

  InitTokenTable (xmltokens);
  return true;
}


bool csOpenTreeFactoryLoader::ParseLevel (iDocumentNode* node, 
  iOpenTreeFactoryState* fact)
{
  csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
    object_reg, "crystalspace.shared.stringset", iStringSet);

  char level = (char)node->GetAttributeValueAsInt ("level");
  csStringID param = strings->Request ("LevelNumber");
  fact->SetParam (level, param , level);

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;

    const char* value = csString(child->GetValue ());
    csStringID param = strings->Request (value);

    csString downcase = csString (value);
    downcase.Downcase();
    csStringID id = xmltokens.Request (downcase);
    
    switch (id)
    {
      //trunk parameter
      case XMLTOKEN_SCALE:
      case XMLTOKEN_SCALEV:
      case XMLTOKEN_BASESPLITS:
      case XMLTOKEN_BRANCHDIST:

      //real level parameter
      case XMLTOKEN_DOWNANGLE:
      case XMLTOKEN_DOWNANGLEV:
      case XMLTOKEN_ROTATE:
      case XMLTOKEN_ROTATEV:
      case XMLTOKEN_LENGTH:
      case XMLTOKEN_LENGTHV:
      case XMLTOKEN_TAPER:
      case XMLTOKEN_SEGSPLITS:
      case XMLTOKEN_SPLITANGLE:
      case XMLTOKEN_SPLITANGLEV:
      case XMLTOKEN_CURVE:
      case XMLTOKEN_CURVEBACK:
      case XMLTOKEN_CURVEV:
        {
          fact->SetParam (level, param, child->GetContentsValueAsFloat ());
        }
	break;  
      case XMLTOKEN_BRANCHES:
      case XMLTOKEN_CURVERES:
        {
          fact->SetParam (level, param ,child->GetContentsValueAsInt ());
        }
	break;  
      default:
	synldr->ReportBadToken (child);
	return 0;
    }  
  }

  return true;
}

bool csOpenTreeFactoryLoader::ParseSpecies (iDocumentNode* node, 
  iOpenTreeFactoryState* fact)
{
  csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
    object_reg, "crystalspace.shared.stringset", iStringSet);

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;

    const char* value = csString(child->GetValue ());
    csStringID param = strings->Request (value);

    csString downcase = csString (value);
    downcase.Downcase();
    csStringID id = xmltokens.Request (downcase);
    
    switch (id)
    {
      case XMLTOKEN_RATIOPOWER:
      case XMLTOKEN_LOBEDEPTH:
      case XMLTOKEN_BASESIZE:
      case XMLTOKEN_ATTRACTIONUP:
      case XMLTOKEN_LEAFSCALEX:
      case XMLTOKEN_SCALE:
      case XMLTOKEN_SCALEV:
      case XMLTOKEN_RATIO:
      case XMLTOKEN_LEAFQUALITY:
      case XMLTOKEN_FLARE:
      case XMLTOKEN_LEAFSCALE:
      case XMLTOKEN_LEAFBEND:
      case XMLTOKEN_PRUNEWIDTH:
      case XMLTOKEN_PRUNEWIDTHPEAK:
      case XMLTOKEN_PRUNERATIO:
      case XMLTOKEN_PRUNEPOWERLOW:
      case XMLTOKEN_PRUNEPOWERHIGH:
        {
          fact->SetParam (-1, param ,child->GetContentsValueAsFloat ());
        }
	break;
      case XMLTOKEN_LEAVES:
      case XMLTOKEN_SHAPE:
      case XMLTOKEN_LOBES:
      case XMLTOKEN_LEVELS:
      case XMLTOKEN_LEAFDISTRIB:
        {
          fact->SetParam (-1, param ,child->GetContentsValueAsInt ());
        }
	break;
      case XMLTOKEN_LEVEL:
        {
          if (!ParseLevel(child, fact))
            return false;
        }
        break;
      case XMLTOKEN_LEAFSHAPE:
      case XMLTOKEN_LEAFSTEMLEN:
      case XMLTOKEN_SMOOTH:
        break;
      default:
	synldr->ReportBadToken (child);
	return 0;
    }
  }

  return true;
}

csPtr<iBase> csOpenTreeFactoryLoader::Parse (iDocumentNode* node,
                                          iStreamSource*, 
                                          iLoaderContext* /*ldr_context*/, 
                                          iBase* /* context */)
{
  csRef<iPluginManager> plugin_mgr = 
    csQueryRegistry<iPluginManager> (object_reg);
  csRef<iMeshObjectType> type = csQueryPluginClass<iMeshObjectType> (
    plugin_mgr, "crystalspace.mesh.opentree");
  if (!type)
  {
    type = csLoadPlugin<iMeshObjectType> (plugin_mgr, 
      "crystalspace.mesh.opentree");
  }
  if (!type)
  {
    synldr->ReportError (
		"crystalspace.opentreefactoryloader.setup.objecttype",
		node, "Could not load the general mesh object plugin!");
    return 0;
  }
  csRef<iMeshObjectFactory> fact;
  csRef<iOpenTreeFactoryState> state;

  fact = type->NewFactory ();
  state = scfQueryInterface<iOpenTreeFactoryState> (fact);


  //check for wrapping opentree tag
  csRef<iDocumentNode> opentreenode = 0;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);

    if (id == XMLTOKEN_OPENTREE)
    {
      if (opentreenode != 0)
      {
        synldr->ReportError (
                "crystalspace.opentreefactoryloader.parse.opentree",
                node, "Parameterfile doesn't start with opentree tag");
        return 0;
      }
      else opentreenode = child;
    } 
    else
    {
      synldr->ReportError (
                "crystalspace.opentreefactoryloader.parse.opentree",
                node, "Parameterfile doesn't start with opentree tag, ignored ");
    }
  }

  if (opentreenode == 0)
  {
    synldr->ReportError (
                "crystalspace.opentreefactoryloader.parse.opentree",
                node, "no opentree tag found in file");
    return 0;
  }

  it = opentreenode->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    
    switch (id)
    {
      case XMLTOKEN_SPECIES:
        {
          if (!ParseSpecies(child, state))
            return 0;
        }
	break;
      default:
	synldr->ReportBadToken (child);
	return 0;
    }
  }

  state->GenerateTree();

  return csPtr<iBase> (fact);
}
//---------------------------------------------------------------------------

csOpenTreeMeshLoader::csOpenTreeMeshLoader (iBase* pParent) : 
  scfImplementationType (this, pParent)
{
}

csOpenTreeMeshLoader::~csOpenTreeMeshLoader ()
{
}

bool csOpenTreeMeshLoader::Initialize (iObjectRegistry* object_reg)
{
  csOpenTreeMeshLoader::object_reg = object_reg;
  synldr = csQueryRegistry<iSyntaxService> (object_reg);

  InitTokenTable (xmltokens);
  return true;
}

#define CHECK_MESH(m) \
  if (!m) { \
    synldr->ReportError ( \
	"crystalspace.opentreeloader.parse.unknownfactory", \
	child, "Specify the factory first!"); \
    return 0; \
  }


csPtr<iBase> csOpenTreeMeshLoader::Parse (iDocumentNode* node,
	iStreamSource*, iLoaderContext* ldr_context, iBase*)
{
  csRef<iMeshObject> mesh;
  csRef<iOpenTreeState> meshstate;

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
	}
	break;
      case XMLTOKEN_FACTORY:
	{
          const char* factname = child->GetContentsValue ();
          iMeshFactoryWrapper* fact = ldr_context->FindMeshFactory (factname);
          if (!fact)
          {
            synldr->ReportError (
                "crystalspace.opentreeloader.parse.unknownfactory",
                child, "Couldn't find factory '%s'!", factname);
            return 0;
          }
          mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          CS_ASSERT (mesh != 0);
          meshstate = SCF_QUERY_INTERFACE (mesh, iOpenTreeState);
          if (!meshstate)
          {
            synldr->ReportError (
                "crystalspace.opentreeloader.parse.badfactory",
                child, "Factory '%s' doesn't appear to be a protomesh factory!",
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
                "crystalspace.opentreeloader.parse.unknownmaterial",
                child, "Couldn't find material '%s'!", matname);
            return 0;
          }
          CHECK_MESH (mesh);
          mesh->SetMaterialWrapper (mat);
        }
	break;
      case XMLTOKEN_MIXMODE:
        {
	}
	break;
      default:
        synldr->ReportBadToken (child);
	return 0;
    }
  }

  return csPtr<iBase> (mesh);
}

}                
CS_PLUGIN_NAMESPACE_END(OpenTreeLoader)

