/*
    Copyright (C) 2003 by Boyan Hristov

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
#include "lghtngldr.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iutil/plugin.h"
#include "imesh/partsys.h"
#include "imesh/lghtng.h"
#include "ivideo/graph3d.h"
#include "csqint.h"
#include "iutil/vfs.h"
#include "csutil/csstring.h"
#include "iutil/object.h"
#include "iutil/document.h"
#include "iengine/material.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/stringarray.h"
#include "imap/ldrctxt.h"
#include "ivaria/reporter.h"



enum
{
  XMLTOKEN_BANDWIDTH = 1,
  XMLTOKEN_DIRECTIONAL,
  XMLTOKEN_FACTORY,
  XMLTOKEN_INTERVAL,
  XMLTOKEN_LENGTH,
  XMLTOKEN_MATERIAL,
  XMLTOKEN_MIXMODE,
  XMLTOKEN_ORIGIN,
  XMLTOKEN_POINTCOUNT,
  XMLTOKEN_VIBRATION,
  XMLTOKEN_WILDNESS
};

SCF_IMPLEMENT_FACTORY (csLightningFactoryLoader)
SCF_IMPLEMENT_FACTORY (csLightningFactorySaver)
SCF_IMPLEMENT_FACTORY (csLightningLoader)
SCF_IMPLEMENT_FACTORY (csLightningSaver)

csLightningFactoryLoader::csLightningFactoryLoader (iBase* pParent) :
  scfImplementationType(this, pParent)
{
}

csLightningFactoryLoader::~csLightningFactoryLoader ()
{
}

bool csLightningFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csLightningFactoryLoader::object_reg = object_reg;
  synldr = csQueryRegistry<iSyntaxService> (object_reg);
  reporter = csQueryRegistry<iReporter> (object_reg);

  xmltokens.Register ("bandwidth", XMLTOKEN_BANDWIDTH);
  xmltokens.Register ("directional", XMLTOKEN_DIRECTIONAL);
  xmltokens.Register ("factory", XMLTOKEN_FACTORY);
  xmltokens.Register ("interval", XMLTOKEN_INTERVAL);
  xmltokens.Register ("length", XMLTOKEN_LENGTH);
  xmltokens.Register ("material", XMLTOKEN_MATERIAL);
  xmltokens.Register ("mixmode", XMLTOKEN_MIXMODE);
  xmltokens.Register ("origin", XMLTOKEN_ORIGIN);
  xmltokens.Register ("pointcount", XMLTOKEN_POINTCOUNT);
  xmltokens.Register ("vibration", XMLTOKEN_VIBRATION);
  xmltokens.Register ("wildness", XMLTOKEN_WILDNESS);
  return true;
}


csPtr<iBase> csLightningFactoryLoader::Parse (iDocumentNode* node,
  iStreamSource*, iLoaderContext* ldr_context, iBase* /* context */)
{
  csVector3 a;

  csRef<iMeshObjectType> type = csLoadPluginCheck<iMeshObjectType> (
        object_reg, "crystalspace.mesh.object.lightning");
  if (!type) return 0;
  csRef<iMeshObjectFactory> fact;
  fact = type->NewFactory ();
  csRef<iLightningFactoryState> LightningFactoryState (
        scfQueryInterface<iLightningFactoryState> (fact));
  CS_ASSERT (LightningFactoryState);

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
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
            synldr->ReportError (
                "crystalspace.lightningloader.parse.badmaterial",
                child, "Could not find material %s!",
		CS::Quote::Single (matname));
            return 0;
          }
          fact->SetMaterialWrapper (mat);
        }
        break;

      case XMLTOKEN_MIXMODE:
        {
          uint mode;
          if (!synldr->ParseMixmode (child, mode))
            return 0;
          fact->SetMixMode (mode);
        }
        break;

      case XMLTOKEN_ORIGIN:
        if (!synldr->ParseVector (child, a))
          return 0;
        LightningFactoryState->SetOrigin (a);
        break;

      case XMLTOKEN_DIRECTIONAL:
        if (!synldr->ParseVector (child, a))
          return 0;
        LightningFactoryState->SetDirectional (a);
        break;

      case XMLTOKEN_POINTCOUNT:
        {
          int pcount = child->GetContentsValueAsInt ();
          LightningFactoryState->SetPointCount (pcount);
        }
        break;

      case XMLTOKEN_LENGTH:
        {
          float len = child->GetContentsValueAsFloat ();
          LightningFactoryState->SetLength (len);
        }
        break;

      case XMLTOKEN_VIBRATION:
        {
          float vibr = child->GetContentsValueAsFloat ();
          LightningFactoryState->SetVibration (vibr);
        }
        break;

      case XMLTOKEN_WILDNESS:
        {
          float wildness = child->GetContentsValueAsFloat ();
          LightningFactoryState->SetWildness (wildness);
        }
        break;

      case XMLTOKEN_INTERVAL:
        {
          csTicks ticks = child->GetContentsValueAsInt ();
          LightningFactoryState->SetUpdateInterval (ticks);
        }
        break;

      default:
        synldr->ReportBadToken (child);
        return 0;
    }
  }

  return csPtr<iBase> (fact);
}

//---------------------------------------------------------------------------

csLightningFactorySaver::csLightningFactorySaver (iBase* pParent) :
  scfImplementationType(this, pParent)
{
}

csLightningFactorySaver::~csLightningFactorySaver ()
{
}

bool csLightningFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csLightningFactorySaver::object_reg = object_reg;
  synldr = csQueryRegistry<iSyntaxService> (object_reg);
  return true;
}

bool csLightningFactorySaver::WriteDown (iBase* obj, iDocumentNode* parent,
	iStreamSource*)
{
  if (!parent) return false; //you never know...
  if (!obj) return false; //you never know...
  
  csRef<iDocumentNode> paramsNode = parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");

  csRef<iLightningState> light = scfQueryInterface<iLightningState> (obj);
  csRef<iMeshObject> mesh = scfQueryInterface<iMeshObject> (obj);

  if (mesh && light)
  {
    //Writedown Material tag
    iMaterialWrapper* mat = mesh->GetMaterialWrapper();
    if (mat)
    {
      const char* matname = mat->QueryObject()->GetName();
      if (matname && *matname)
      {
        csRef<iDocumentNode> matNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        matNode->SetValue("material");
        csRef<iDocumentNode> matnameNode = matNode->CreateNodeBefore(CS_NODE_TEXT, 0);
        matnameNode->SetValue(matname);
      }    
    }    

    //Writedown Directional tag
    csVector3 direct = light->GetDirectional();
    csRef<iDocumentNode> directNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    directNode->SetValue("directional");
    synldr->WriteVector(directNode, direct);

    //Writedown Origin tag
    csVector3 orig = light->GetOrigin();
    csRef<iDocumentNode> origNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    origNode->SetValue("origin");
    synldr->WriteVector(origNode, orig);

    //Writedown Vibration tag
    float vibr = light->GetVibration();
    csRef<iDocumentNode> vibrNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    vibrNode->SetValue("vibration");
    vibrNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(vibr);

    //Writedown Length tag
    float len = light->GetLength();
    csRef<iDocumentNode> lenNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    lenNode->SetValue("length");
    lenNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(len);

    //Writedown Wildness tag
    float wild = light->GetWildness();
    csRef<iDocumentNode> wildNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    wildNode->SetValue("wildness");
    wildNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(wild);

    //Writedown PointCount tag
    int pc = light->GetPointCount();
    csRef<iDocumentNode> pcNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    pcNode->SetValue("pointcount");
    pcNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(pc);

    //Writedown Interval tag
    csTicks ticks = light->GetUpdateInterval();
    csRef<iDocumentNode> ticksNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    ticksNode->SetValue("interval");
    ticksNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsInt(ticks);

    //Writedown Mixmode tag
    int mixmode = mesh->GetMixMode();
    csRef<iDocumentNode> mixmodeNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    mixmodeNode->SetValue("mixmode");
    synldr->WriteMixmode(mixmodeNode, mixmode, true);
  }
  return true;
}

//---------------------------------------------------------------------------

csLightningLoader::csLightningLoader (iBase* pParent) :
  scfImplementationType(this, pParent)
{
}

csLightningLoader::~csLightningLoader ()
{
}

bool csLightningLoader::Initialize (iObjectRegistry* object_reg)
{
  csLightningLoader::object_reg = object_reg;
  synldr = csQueryRegistry<iSyntaxService> (object_reg);
  reporter = csQueryRegistry<iReporter> (object_reg);

  xmltokens.Register ("factory", XMLTOKEN_FACTORY);
  // @@@TODO
  return true;
}

csPtr<iBase> csLightningLoader::Parse (iDocumentNode* node,
    iStreamSource*, iLoaderContext* ldr_context, iBase*)
{
  csRef<iMeshObject> mesh;
  csRef<iLightningFactoryState> LightningFactoryState;
  csRef<iLightningState> Lightningstate;

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
          iMeshFactoryWrapper* fact =
              ldr_context->FindMeshFactory (factname);

          if(!fact)
          {
            synldr->ReportError (
              "crystalspace.lightningloader.parse.badfactory",
              child, "Could not find factory %s!",
	      CS::Quote::Single (factname));
            return 0;
          }

          mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          Lightningstate = scfQueryInterface<iLightningState> (mesh);
	  if (!Lightningstate)
	  {
      	    synldr->ReportError (
		"crystalspace.lightningloader.parse.badfactory",
		child, "Factory %s doesn't appear to be a lightning factory!",
		CS::Quote::Single (factname));
	    return 0;
	  }
          LightningFactoryState = scfQueryInterface<iLightningFactoryState> (
              fact->GetMeshObjectFactory());
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


csLightningSaver::csLightningSaver (iBase* pParent) :
  scfImplementationType(this, pParent)
{
}

csLightningSaver::~csLightningSaver ()
{
}

bool csLightningSaver::Initialize (iObjectRegistry* object_reg)
{
  csLightningSaver::object_reg = object_reg;
  return true;
}

bool csLightningSaver::WriteDown (iBase* obj, iDocumentNode* parent,
	iStreamSource*)
{
  if (!parent) return false; //you never know...
  if (!obj)    return false; //you never know...
  
  csRef<iDocumentNode> paramsNode = parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");

  csRef<iMeshObject> mesh = scfQueryInterface<iMeshObject> (obj);

  if ( mesh )
  {
    //Writedown Factory tag
    iMeshFactoryWrapper* fact = mesh->GetFactory()->GetMeshFactoryWrapper ();
    if (fact)
    {
      const char* factname = fact->QueryObject()->GetName();
      if (factname && *factname)
      {
        csRef<iDocumentNode> factNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        factNode->SetValue("factory");
        csRef<iDocumentNode> factnameNode = factNode->CreateNodeBefore(CS_NODE_TEXT, 0);
        factnameNode->SetValue(factname);
      }    
    }    
  }
  return true;
}
