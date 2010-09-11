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
  engine = csQueryRegistry<iEngine>(object_reg);

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
     
        meshstate->SetDensityMap(tex);
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

        meshstate->SetHeightMap(tex);
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
    case XMLTOKEN_AVERAGECONTROLPOINTSCOUNT:
      {
        uint averageControlPointsCount = child->GetContentsValueAsInt();
        CHECK_MESH(meshstate);
        meshstate->SetAverageControlPointsCount(averageControlPointsCount);
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
    case XMLTOKEN_SMALLFUR:
      {
        CHECK_MESH(meshstate);
        meshstate->SetSmallFur(true);
      }
      break;
    case XMLTOKEN_MIXMODE:
      {
        uint mixmode;
        if (!synldr->ParseMixmode(child, mixmode))
          return 0;
        CHECK_MESH(meshstate);
        meshstate->SetMixmode(mixmode);
      }
      break;
    case XMLTOKEN_PRIORITY:
      {
        uint priority = (uint)engine->GetRenderPriority (child->GetContentsValue ());
        CHECK_MESH(meshstate);
        meshstate->SetPriority(priority);
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
      csZBufMode zbufmode;
      if (synldr->ParseZMode (child, zbufmode)) 
      {
        meshstate->SetZBufMode(zbufmode);
        break;
      }
      synldr->ReportBadToken (child);
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
  if (!parent) return false; //you never know...

  csRef<iDocumentNode> paramsNode = 
    parent->CreateNodeBefore (CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");

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
  engine = csQueryRegistry<iEngine> (object_reg);

  return true;
}

bool FurMeshSaver::WriteDown (iBase* obj, iDocumentNode* parent,
	iStreamSource*)
{
  if (!parent) return false; //you never know...

  csRef<iDocumentNode> paramsNode = 
    parent->CreateNodeBefore (CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");

  if (obj)
  {
    csRef<CS::Mesh::iFurMeshState> fmesh = 
      scfQueryInterface<CS::Mesh::iFurMeshState> (obj);
    if (!fmesh) return false;
    csRef<iMeshObject> mesh = 
      scfQueryInterface<iMeshObject> (obj);
    if (!mesh) return false;

    //Writedown Factory tag
    iMeshFactoryWrapper* fact = mesh->GetFactory()->GetMeshFactoryWrapper ();
    if (fact)
    {
      const char* factname = fact->QueryObject()->GetName();
      if (factname && *factname)
      {
        csRef<iDocumentNode> factNode = 
          paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        factNode->SetValue("factory");
        factNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValue(factname);
      }    
    }

    //Writedown Strand width tag
    float strandWidth = fmesh->GetStrandWidth();
    csRef<iDocumentNode> strandWidthNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    strandWidthNode->SetValue("strandwidth");
    strandWidthNode->CreateNodeBefore(CS_NODE_TEXT, 0)->
      SetValueAsFloat(strandWidth);

    //Writedown displacement tag
    float displacement = fmesh->GetDisplacement();
    csRef<iDocumentNode> displacementNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    displacementNode->SetValue("displacement");
    displacementNode->CreateNodeBefore(CS_NODE_TEXT, 0)->
      SetValueAsFloat(displacement);

    //Writedown density map tag
    iTextureWrapper* densitymap = fmesh->GetDensityMap();
    if(densitymap)
    {
      const char* densitymapName = densitymap->QueryObject()->GetName();
      if (densitymapName && *densitymapName)
      {
        csRef<iDocumentNode> densitymapNode = 
          paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        densitymapNode->SetValue("densitymap");
        densitymapNode->CreateNodeBefore(CS_NODE_TEXT, 0)->
          SetValue(densitymapName);
      }
    }

    //Writedown density factor guide furs tag
    float densityFactorGuideFurs = fmesh->GetDensityFactorGuideFurs();
    csRef<iDocumentNode> densityFactorGuideFursNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    densityFactorGuideFursNode->SetValue("densityfactorguidefurs");
    densityFactorGuideFursNode->CreateNodeBefore(CS_NODE_TEXT, 0)->
      SetValueAsFloat(densityFactorGuideFurs);

    //Writedown density factor fur strands tag
    float densityFactorFurStrands = fmesh->GetDensityFactorFurStrands();
    csRef<iDocumentNode> densityFactorFurStrandsNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    densityFactorFurStrandsNode->SetValue("densityfactorfurstrands");
    densityFactorFurStrandsNode->CreateNodeBefore(CS_NODE_TEXT, 0)->
      SetValueAsFloat(densityFactorFurStrands);

    //Writedown height map tag
    iTextureWrapper* heightmap = fmesh->GetHeightMap();
    if(heightmap)
    {
      const char* heightmapName = heightmap->QueryObject()->GetName();
      if (heightmapName && *heightmapName)
      {
        csRef<iDocumentNode> heightmapNode = 
          paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        heightmapNode->SetValue("heightmap");
        heightmapNode->CreateNodeBefore(CS_NODE_TEXT, 0)->
          SetValue(heightmapName);
      }
    }

    //Writedown height factor tag
    float heightFactor = fmesh->GetHeightFactor();
    csRef<iDocumentNode> heightFactorNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    heightFactorNode->SetValue("heightfactor");
    heightFactorNode->CreateNodeBefore(CS_NODE_TEXT, 0)->
      SetValueAsFloat(heightFactor);

    //Writedown control points disance tag
    float controlPointsDistance = fmesh->GetControlPointsDistance();
    csRef<iDocumentNode> controlPointsDistanceNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    controlPointsDistanceNode->SetValue("controlpointsdistance");
    controlPointsDistanceNode->CreateNodeBefore(CS_NODE_TEXT, 0)->
      SetValueAsFloat(controlPointsDistance);

    //Writedown control points disance tag
    float positionDeviation = fmesh->GetPositionDeviation();
    csRef<iDocumentNode> positionDeviationNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    positionDeviationNode->SetValue("positiondeviation");
    positionDeviationNode->CreateNodeBefore(CS_NODE_TEXT, 0)->
      SetValueAsFloat(positionDeviation);

    //Writedown grow tangents tag
    bool growTangents = fmesh->GetGrowTangent();
    if (growTangents)
    {
      csRef<iDocumentNode> growTangentsNode = 
        paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      growTangentsNode->SetValue("growtangents");
    }

    //Writedown Mixmode tag
    int mixmode = fmesh->GetMixmode();
    csRef<iDocumentNode> mixmodeNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    mixmodeNode->SetValue("mixmode");
    synldr->WriteMixmode(mixmodeNode, mixmode, true);

    //Writedown priority tag
    int priority = fmesh->GetPriority();
    const char* pname = engine->GetRenderPriorityName (priority);
    if (pname && *pname)
    {
      csRef<iDocumentNode> priorityNode = 
        paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      priorityNode->SetValue("priority");
      priorityNode->CreateNodeBefore(CS_NODE_TEXT, 0)->
        SetValue(pname);
    }

    //Writedown ZBufmode tag
    csZBufMode zbufmode = fmesh->GetZBufMode();
    if (zbufmode != (csZBufMode)~0)
      synldr->WriteZMode (paramsNode, zbufmode, false);
  }

  return true;
}

}
CS_PLUGIN_NAMESPACE_END(FurMeshLoader)
