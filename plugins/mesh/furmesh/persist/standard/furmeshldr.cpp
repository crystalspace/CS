/*
  Copyright (C) 2010 Alexandru - Teodor Voicu
      Faculty of Automatic Control and Computer Science of the "Politehnica"
      University of Bucharest
      http://csite.cs.pub.ro/index.php/en/

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"

#include <ctype.h>

#include "csgeom/math3d.h"
#include "csgeom/tri.h"
#include "csgeom/vector2.h"
#include "csgeom/vector4.h"
#include "csgeom/sphere.h"
#include "csgfx/renderbuffer.h"
#include "cstool/primitives.h"
#include "csutil/cscolor.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/refarr.h"
#include "csutil/scanstr.h"
#include "csutil/sysfunc.h"
#include "csutil/stringconv.h"
#include "csutil/stringreader.h"

#include "iengine/engine.h"
#include "iengine/material.h"
#include "iengine/mesh.h"
#include "imap/ldrctxt.h"
#include "imap/services.h"
#include "imesh/furmesh.h"
#include "imesh/object.h"
#include "iutil/comp.h"
#include "iutil/document.h"
#include "iutil/eventh.h"
#include "iutil/object.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/stringarray.h"
#include "ivaria/reporter.h"
#include "ivideo/graph3d.h"
#include "ivideo/rndbuf.h"
#include "ivideo/shader/shader.h"

#include "furmeshldr.h"

CS_PLUGIN_NAMESPACE_BEGIN(FurMeshLoader)
{

SCF_IMPLEMENT_FACTORY (FurMeshFactoryLoader)
SCF_IMPLEMENT_FACTORY (FurMeshLoader)
SCF_IMPLEMENT_FACTORY (FurMeshFactorySaver)
SCF_IMPLEMENT_FACTORY (FurMeshSaver)

//---------------------------------------------------------------------------

FurMeshFactoryLoader::FurMeshFactoryLoader (iBase* pParent) : 
  scfImplementationType (this, pParent)
{
}

FurMeshFactoryLoader::~FurMeshFactoryLoader ()
{
}

bool FurMeshFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  FurMeshFactoryLoader::object_reg = object_reg;
  synldr = csQueryRegistry<iSyntaxService> (object_reg);

  InitTokenTable (xmltokens);
  return true;
}

csPtr<iBase> FurMeshFactoryLoader::Parse (iDocumentNode* node,
  iStreamSource*, iLoaderContext* ldr_context, iBase*)
{
  csRef<iMeshObjectType> type = csLoadPluginCheck<iMeshObjectType> (
      object_reg, "crystalspace.mesh.object.furmesh", false);
  
  if (!type)
  {
    synldr->ReportError (
      "crystalspace.furmeshfactoryloader.setup.objecttype",
      node, "Could not load the fur mesh object plugin!");
    return 0;
  }

  csRef<iMeshObjectFactory> fact;

  fact = type->NewFactory ();
  csRef<iDocumentNodeIterator> it = node->GetNodes ();

  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);

    switch (id)
    {
      default:
        synldr->ReportBadToken (child);
        return 0;
    }
  }

  return csPtr<iBase> (fact);
}

//---------------------------------------------------------------------------

FurMeshLoader::FurMeshLoader (iBase* pParent) : 
  scfImplementationType (this, pParent)
{
}

FurMeshLoader::~FurMeshLoader ()
{
}

bool FurMeshLoader::Initialize (iObjectRegistry* object_reg)
{
  FurMeshLoader::object_reg = object_reg;
  synldr = csQueryRegistry<iSyntaxService> (object_reg);

  InitTokenTable (xmltokens);
  return true;
}

#define CHECK_MESH(m) \
  if (!m) { \
  synldr->ReportError ( \
  "crystalspace.furmeshloader.parse.unknownfactory", \
  child, "Specify the factory first!"); \
  return 0; \
  }

csPtr<iBase> FurMeshLoader::Parse (iDocumentNode* node,
	iStreamSource*, iLoaderContext* ldr_context, iBase*)
{
  csRef<iMeshObject> mesh;
  csRef<CS::Mesh::iFurMeshState> meshstate;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();

  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);

    switch (id)
    {
    case XMLTOKEN_STRANDWIDTH:
      {
        float strandWidth = child->GetContentsValueAsFloat();
        CHECK_MESH(meshstate);
        meshstate->SetStrandWidth(strandWidth);
      }
      break;
    case XMLTOKEN_DISPLACEMENT:
      {
        float displacement = child->GetContentsValueAsFloat();
        CHECK_MESH(meshstate);
        meshstate->SetDisplacement(displacement);
      }
      break;
    case XMLTOKEN_DENSITYMAP:
      {
        const char* densityMapName = child->GetContentsValue ();
        iTextureWrapper* tex = ldr_context->FindTexture(densityMapName);
        if(!tex)
        {
          synldr->ReportError (
            "crystalspace.furmeshloader.parse.unknownfactory",
            child, "Couldn't find texture '%s'!", densityMapName);
          return 0;
        }

        iTextureHandle* tex_handle = tex->GetTextureHandle();
        if(!tex_handle)
        {
          synldr->ReportError (
            "crystalspace.furmeshloader.parse.unknownfactory",
            child, "Couldn't find texture handle for '%s'!", densityMapName);
          return 0;
        }

        meshstate->SetDensityMap(tex_handle);
      }
      break;
    case XMLTOKEN_HEIGHTMAP:
      {
        const char* heightMapName = child->GetContentsValue ();
        iTextureWrapper* tex = ldr_context->FindTexture(heightMapName);
        if(!tex)
        {
          synldr->ReportError (
            "crystalspace.furmeshloader.parse.unknownfactory",
            child, "Couldn't find texture '%s'!", heightMapName);
          return 0;
        }

        iTextureHandle* tex_handle = tex->GetTextureHandle();
        if(!tex_handle)
        {
          synldr->ReportError (
            "crystalspace.furmeshloader.parse.unknownfactory",
            child, "Couldn't find texture handle for '%s'!", heightMapName);
          return 0;
        }

        meshstate->SetHeightMap(tex_handle);
      }
      break;
    case XMLTOKEN_DENSITYFACTORGUIDEFURS:
      {
        float densityFactorGuideFurs = child->GetContentsValueAsFloat();
        CHECK_MESH(meshstate);
        meshstate->SetDensityFactorGuideFurs(densityFactorGuideFurs);
      }
      break;
    case XMLTOKEN_DENSITYFACTORFURSTRANDS:
      {
        float densityFactorFurStrands = child->GetContentsValueAsFloat();
        CHECK_MESH(meshstate);
        meshstate->SetDensityFactorFurStrands(densityFactorFurStrands);
      }
      break;
    case XMLTOKEN_HEIGHTFACTOR:
      {
        float heightFactor = child->GetContentsValueAsFloat();
        CHECK_MESH(meshstate);
        meshstate->SetHeightFactor(heightFactor);
      }
      break;
    case XMLTOKEN_CONTROLPOINTSDISTANCE:
      {
        float controlPointsDistance = child->GetContentsValueAsFloat();
        CHECK_MESH(meshstate);
        meshstate->SetControlPointsDistance(controlPointsDistance);
      }
      break;
    case XMLTOKEN_POSITIONDEVIATION:
      {
        float positionDeviation = child->GetContentsValueAsFloat();
        CHECK_MESH(meshstate);
        meshstate->SetPositionDeviation(positionDeviation);
      }
      break;
    case XMLTOKEN_GROWTANGENTS:
      {
        CHECK_MESH(meshstate);
        meshstate->SetGrowTangent(true);
      }
      break;
    case XMLTOKEN_MIXMODE:
      {
        uint mixmode;
        if (!synldr->ParseMixmode(child, mixmode))
          return 0;
        CHECK_MESH(meshstate);
        meshstate->SetMixmode(mixmode);

        csPrintf("%u\n", mixmode);
      }
      break;
    case XMLTOKEN_PRIORITY:
      {
        uint priority = (uint)child->GetContentsValueAsInt();
        CHECK_MESH(meshstate);
        meshstate->SetPriority(priority);
      }
      break;
    case XMLTOKEN_ZBUFMODE:
      {
        csZBufMode z_buf_mode;
        if (!synldr->ParseZMode(child, z_buf_mode))
          return 0;
        CHECK_MESH(meshstate);
        meshstate->SetZBufMode(z_buf_mode);
      }
      break;
    case XMLTOKEN_FACTORY:
      {
        const char* factname = child->GetContentsValue ();
        iMeshFactoryWrapper* fact = ldr_context->FindMeshFactory (factname);
        if(!fact)
        {
          synldr->ReportError (
            "crystalspace.furmeshloader.parse.unknownfactory",
            child, "Couldn't find factory '%s'!", factname);
          return 0;
        }

        mesh = fact->GetMeshObjectFactory ()->NewInstance ();
        CS_ASSERT (mesh != 0);

        meshstate = scfQueryInterface<CS::Mesh::iFurMeshState> (mesh);
        if (!meshstate)
        {
          synldr->ReportError (
            "crystalspace.furmeshloader.parse.badfactory",
            child, "Factory '%s' doesn't appear to be a genmesh factory!",
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

FurMeshFactorySaver::FurMeshFactorySaver (iBase* pParent) : 
  scfImplementationType (this, pParent)
{
}

FurMeshFactorySaver::~FurMeshFactorySaver ()
{
}

bool FurMeshFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  FurMeshFactorySaver::object_reg = object_reg;
  synldr = csQueryRegistry<iSyntaxService> (object_reg);
  return true;
}

bool FurMeshFactorySaver::WriteDown (iBase* obj, iDocumentNode* parent,
  iStreamSource*)
{
  return true;
}

//---------------------------------------------------------------------------

FurMeshSaver::FurMeshSaver (iBase* pParent) : 
  scfImplementationType (this, pParent)
{
}

FurMeshSaver::~FurMeshSaver ()
{
}

bool FurMeshSaver::Initialize (iObjectRegistry* object_reg)
{
  FurMeshSaver::object_reg = object_reg;
  synldr = csQueryRegistry<iSyntaxService> (object_reg);
  return true;
}

bool FurMeshSaver::WriteDown (iBase* obj, iDocumentNode* parent,
	iStreamSource*)
{
  return true;
}

}
CS_PLUGIN_NAMESPACE_END(FurMeshLoader)
