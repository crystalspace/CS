/*
    Copyright (C) 2004 by Jorrit Tyberghein

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
#include "csqint.h"
#include "csgeom/math3d.h"
#include "csgeom/tri.h"
#include "csgeom/vector2.h"
#include "csgeom/vector4.h"
#include "csutil/cscolor.h"
#include "csutil/scanstr.h"
#include "csutil/sysfunc.h"
#include "foliageldr.h"
#include "iengine/engine.h"
#include "iengine/lod.h"
#include "iengine/material.h"
#include "iengine/mesh.h"
#include "iengine/sharevar.h"
#include "imap/ldrctxt.h"
#include "imap/services.h"
#include "imesh/foliage.h"
#include "imesh/object.h"
#include "iutil/comp.h"
#include "iutil/document.h"
#include "iutil/eventh.h"
#include "iutil/object.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "ivaria/reporter.h"
#include "ivaria/terraform.h"
#include "ivideo/graph3d.h"
#include "ivideo/rndbuf.h"

CS_IMPLEMENT_PLUGIN

enum
{
  XMLTOKEN_FACTORY = 1,
  XMLTOKEN_TERRAFORMER,
  XMLTOKEN_SAMPLEREGION,
  XMLTOKEN_OBJECT,
  XMLTOKEN_GEOMETRY,
  XMLTOKEN_V,
  XMLTOKEN_T,
  XMLTOKEN_MATERIAL,
  XMLTOKEN_FOLIAGEPALETTE,
  XMLTOKEN_PALETTE,
  XMLTOKEN_FOLIAGE,
  XMLTOKEN_LODDISTANCE
};

SCF_IMPLEMENT_IBASE (csFoliageFactoryLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csFoliageFactoryLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csFoliageFactorySaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csFoliageFactorySaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csFoliageMeshLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csFoliageMeshLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csFoliageMeshSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csFoliageMeshSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csFoliageFactoryLoader)
SCF_IMPLEMENT_FACTORY (csFoliageFactorySaver)
SCF_IMPLEMENT_FACTORY (csFoliageMeshLoader)
SCF_IMPLEMENT_FACTORY (csFoliageMeshSaver)


csFoliageFactoryLoader::csFoliageFactoryLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csFoliageFactoryLoader::~csFoliageFactoryLoader ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csFoliageFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csFoliageFactoryLoader::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);

  xmltokens.Register ("terraformer", XMLTOKEN_TERRAFORMER);
  xmltokens.Register ("sampleregion", XMLTOKEN_SAMPLEREGION);
  xmltokens.Register ("object", XMLTOKEN_OBJECT);
  xmltokens.Register ("geometry", XMLTOKEN_GEOMETRY);
  xmltokens.Register ("v", XMLTOKEN_V);
  xmltokens.Register ("t", XMLTOKEN_T);
  xmltokens.Register ("material", XMLTOKEN_MATERIAL);
  xmltokens.Register ("foliagepalette", XMLTOKEN_FOLIAGEPALETTE);
  xmltokens.Register ("palette", XMLTOKEN_PALETTE);
  xmltokens.Register ("foliage", XMLTOKEN_FOLIAGE);
  xmltokens.Register ("loddistance", XMLTOKEN_LODDISTANCE);
  return true;
}

bool csFoliageFactoryLoader::ParseGeometry (iLoaderContext* ldr_context,
    iDocumentNode* node, iFoliageGeometry* geom)
{
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_V:
	{
	  csVector3 pos;
	  csVector2 texel;
	  csColor color;
	  csVector3 normal;
	  pos.x = child->GetAttributeValueAsFloat ("x");
	  pos.y = child->GetAttributeValueAsFloat ("y");
	  pos.z = child->GetAttributeValueAsFloat ("z");
	  texel.x = child->GetAttributeValueAsFloat ("u");
	  texel.y = child->GetAttributeValueAsFloat ("v");
	  color.red = child->GetAttributeValueAsFloat ("red");
	  color.green = child->GetAttributeValueAsFloat ("green");
	  color.blue = child->GetAttributeValueAsFloat ("blue");
	  normal.x = child->GetAttributeValueAsFloat ("nx");
	  normal.y = child->GetAttributeValueAsFloat ("ny");
	  normal.z = child->GetAttributeValueAsFloat ("nz");
	  geom->AddVertex (pos, texel, color, normal);
	}
	break;
      case XMLTOKEN_T:
	{
	  csTriangle t;
	  t.a = child->GetAttributeValueAsInt ("v1");
	  t.b = child->GetAttributeValueAsInt ("v2");
	  t.c = child->GetAttributeValueAsInt ("v3");
	  geom->AddTriangle (t);
	}
	break;
      case XMLTOKEN_MATERIAL:
	{
	  const char* matname = child->GetContentsValue ();
          iMaterialWrapper* mat = ldr_context->FindMaterial (matname);
	  if (!mat)
	  {
      	    synldr->ReportError (
		"crystalspace.foliagefactoryloader.parse.unknownmaterial",
		child, "Couldn't find material '%s'!", matname);
            return 0;
	  }
	  geom->SetMaterialWrapper (mat);
	}
	break;
      default:
	synldr->ReportBadToken (child);
	return false;
    }
  }
  return true;
}

bool csFoliageFactoryLoader::ParseObject (iLoaderContext* ldr_context,
    iDocumentNode* node, iFoliageObject* obj)
{
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_GEOMETRY:
	{
	  int lod = child->GetAttributeValueAsInt ("lod");
	  csRef<iFoliageGeometry> geom = obj->CreateGeometry (lod);
	  if (!ParseGeometry (ldr_context, child, geom))
	    return false;
	}
	break;
      case XMLTOKEN_LODDISTANCE:
	// @@@ Put this in the syntax services?
	{
	  iLODControl* lodctrl = obj->GetLODControl ();
	  csRef<iEngine> Engine = CS_QUERY_REGISTRY (object_reg, iEngine);
	  csRef<iDocumentAttribute> at;
	  at = child->GetAttribute ("varm");
	  if (at)
	  {
	    // We use variables.
	    iSharedVariable *varm = Engine->GetVariableList()->FindByName (
	    	child->GetAttributeValue ("varm"));
	    iSharedVariable *vara = Engine->GetVariableList()->FindByName (
	    	child->GetAttributeValue ("vara"));
	    lodctrl->SetLOD (varm, vara);
	    break;
	  }

	  at = child->GetAttribute ("m");
	  if (at)
	  {
	    float lodm = child->GetAttributeValueAsFloat ("m");
	    float loda = child->GetAttributeValueAsFloat ("a");
	    lodctrl->SetLOD (lodm, loda);
	  }
	  else
	  {
	    float d0 = child->GetAttributeValueAsFloat ("d0");
	    float d1 = child->GetAttributeValueAsFloat ("d1");
	    float lodm = 1.0 / (d1-d0);
	    float loda = -lodm * d0;
	    lodctrl->SetLOD (lodm, loda);
	  }
	}
	break;

      default:
	synldr->ReportBadToken (child);
	return false;
    }
  }
  return true;
}

bool csFoliageFactoryLoader::ParseFoliagePalette (iDocumentNode* node,
    iFoliageFactoryState* fact, int index)
{
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_FOLIAGE:
	{
	  const char* name = child->GetAttributeValue ("name");
	  float relative_density = child->GetAttributeValueAsFloat ("density");
	  fact->AddPaletteEntry (index, name, relative_density);
	}
	break;
      default:
	synldr->ReportBadToken (child);
	return false;
    }
  }
  return true;
}

bool csFoliageFactoryLoader::ParseFoliagePalette (iDocumentNode* node,
    iFoliageFactoryState* fact)
{
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_PALETTE:
	{
	  int index = child->GetAttributeValueAsInt ("index");
	  if (!ParseFoliagePalette (child, fact, index))
	    return false;
	}
	break;
      default:
	synldr->ReportBadToken (child);
	return false;
    }
  }
  return true;
}

csPtr<iBase> csFoliageFactoryLoader::Parse (iDocumentNode* node,
	iLoaderContext* ldr_context, iBase* /* context */)
{
  csRef<iPluginManager> plugin_mgr = CS_QUERY_REGISTRY (object_reg,
  	iPluginManager);
  csRef<iMeshObjectType> type = CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.foliage", iMeshObjectType);
  if (!type)
  {
    type = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.foliage",
    	iMeshObjectType);
  }
  if (!type)
  {
    synldr->ReportError (
		"crystalspace.foliagefactoryloader.setup.objecttype",
		node, "Could not load the general mesh object plugin!");
    return 0;
  }
  csRef<iMeshObjectFactory> fact;
  csRef<iFoliageFactoryState> state;

  fact = type->NewFactory ();
  state = SCF_QUERY_INTERFACE (fact, iFoliageFactoryState);

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_TERRAFORMER:
        {
          const char* name = child->GetContentsValue ();
          csRef<iTerraFormer> form = CS_QUERY_REGISTRY_TAG_INTERFACE (object_reg,
	    name, iTerraFormer);
	  if (form == 0) 
	  {
            synldr->ReportError ("crystalspace.foliage.factory.loader",
              child, "Unable to find TerraFormer %s", name);
            return 0;
	  }
          state->SetTerraFormer (form);
        }
        break;
      case XMLTOKEN_SAMPLEREGION:
        {
          csBox3 box;
          if (!synldr->ParseBox (child, box)) 
	  {
            synldr->ReportError ("crystalspace.foliage.factory.loader",
              child, "Unable to parse sampleregion");
            return 0;
	  }
          state->SetSamplerRegion (csBox2(box.MinX(), box.MinY(), 
		                          box.MaxX(), box.MaxY()));
        }
        break;
      case XMLTOKEN_OBJECT:
        {
	  const char* name = child->GetAttributeValue ("name");
	  csRef<iFoliageObject> obj = state->CreateObject (name);
	  if (!ParseObject (ldr_context, child, obj))
	    return 0;
        }
	break;
      case XMLTOKEN_FOLIAGEPALETTE:
        if (!ParseFoliagePalette (child, state))
	  return 0;
	break;
      default:
	synldr->ReportBadToken (child);
	return 0;
    }
  }

  return csPtr<iBase> (fact);
}
//---------------------------------------------------------------------------

csFoliageFactorySaver::csFoliageFactorySaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csFoliageFactorySaver::~csFoliageFactorySaver ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csFoliageFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csFoliageFactorySaver::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  return true;
}

bool csFoliageFactorySaver::WriteDown (iBase* obj, iDocumentNode* parent)
{
  if (!parent) return false; //you never know...
  
  csRef<iDocumentNode> paramsNode = 
    parent->CreateNodeBefore (CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");

  if (obj)
  {
    csRef<iFoliageFactoryState> gfact = 
      SCF_QUERY_INTERFACE (obj, iFoliageFactoryState);
    csRef<iMeshObjectFactory> meshfact = 
      SCF_QUERY_INTERFACE (obj, iMeshObjectFactory);
    if (!gfact) return false;
    if (!meshfact) return false;

    // @@@ TODO
  }
  return true;
}

//---------------------------------------------------------------------------

csFoliageMeshLoader::csFoliageMeshLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csFoliageMeshLoader::~csFoliageMeshLoader ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csFoliageMeshLoader::Initialize (iObjectRegistry* object_reg)
{
  csFoliageMeshLoader::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);

  xmltokens.Register ("factory", XMLTOKEN_FACTORY);
  return true;
}

csPtr<iBase> csFoliageMeshLoader::Parse (iDocumentNode* node,
	iLoaderContext* ldr_context, iBase*)
{
  csRef<iMeshObject> mesh;
  csRef<iFoliageMeshState> meshstate;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
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
      	    synldr->ReportError (
		"crystalspace.foliageloader.parse.unknownfactory",
		child, "Couldn't find factory '%s'!", factname);
	    return 0;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
	  CS_ASSERT (mesh != 0);
          meshstate = SCF_QUERY_INTERFACE (mesh, iFoliageMeshState);
	  if (!meshstate)
	  {
      	    synldr->ReportError (
		"crystalspace.foliageloader.parse.badfactory",
		child, "Factory '%s' doesn't appear to be a foliage factory!",
		factname);
	    return 0;
	  }
	}
	break;
      default:
        synldr->ReportBadToken (child);
	return 0;
    }
  }

  return csPtr<iBase> (mesh);
}

//---------------------------------------------------------------------------

csFoliageMeshSaver::csFoliageMeshSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csFoliageMeshSaver::~csFoliageMeshSaver ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csFoliageMeshSaver::Initialize (iObjectRegistry* object_reg)
{
  csFoliageMeshSaver::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  return true;
}

bool csFoliageMeshSaver::WriteDown (iBase* obj, iDocumentNode* parent)
{
  if (!parent) return false; //you never know...
  
  csRef<iDocumentNode> paramsNode = 
    parent->CreateNodeBefore (CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");

  if (obj)
  {
    csRef<iFoliageMeshState> gmesh = 
      SCF_QUERY_INTERFACE (obj, iFoliageMeshState);
    csRef<iMeshObject> mesh = SCF_QUERY_INTERFACE (obj, iMeshObject);
    if (!gmesh) return false;
    if (!mesh) return false;

    // Writedown Factory tag
    csRef<iMeshFactoryWrapper> fact = 
      SCF_QUERY_INTERFACE (mesh->GetFactory()->GetLogicalParent(),
                          iMeshFactoryWrapper);
    if (fact)
    {
      const char* factname = fact->QueryObject()->GetName();
      if (factname && *factname)
      {
        csRef<iDocumentNode> factNode = 
          paramsNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
        factNode->SetValue ("factory");
        factNode->CreateNodeBefore (CS_NODE_TEXT, 0)->SetValue (factname);
      }    
    }
  }
  return true;
}

