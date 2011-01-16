/*
    Copyright (C) 2008 by Frank Richter

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

#include "iengine/material.h"
#include "iengine/mesh.h"
#include "imesh/object.h"
#include "imesh/terrain2.h"
#include "iutil/document.h"
#include "iutil/object.h"
#include "iutil/objreg.h"

#include "saver.h"

CS_PLUGIN_NAMESPACE_BEGIN(Terrain2Loader)
{
  bool Terrain2SaverCommon::Initialize (iObjectRegistry *objreg)
  {
    synldr = csQueryRegistry<iSyntaxService> (objreg);
    if (!synldr.IsValid()) return false;

    return true;
  }

  template<typename IProp>
  bool Terrain2SaverCommon::SaveProperties (iDocumentNode* node, IProp* props,
				            IProp* dfltProp)
  {
    if (props == 0) return false;
    if (dfltProp == 0)
    {
      size_t numProps = props->GetParameterCount ();
      if (numProps == 0) return false;
      for (size_t p = 0; p < numProps; p++)
      {
	const char* v = props->GetParameterValue (p);
	if (v == 0) continue;
        csRef<iDocumentNode> parNode = 
          node->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        parNode->SetValue ("param");
	parNode->SetAttribute ("name", props->GetParameterName (p));
	parNode->CreateNodeBefore (CS_NODE_TEXT, 0)
	  ->SetValue (v);
      }
      return true;
    }
    else
    {
      size_t numProps = props->GetParameterCount ();
      if (numProps == 0) return false;
      bool wroteProps = false;
      for (size_t p = 0; p < numProps; p++)
      {
	const char* dfltVal = dfltProp->GetParameterValue (
	  props->GetParameterName (p));
	const char* val = props->GetParameterValue (p);
	if (val == 0) continue;

	if ((dfltVal != 0)  && (strcmp (val, dfltVal) == 0)) continue;
        csRef<iDocumentNode> parNode = 
          node->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        parNode->SetValue ("param");
	parNode->SetAttribute ("name", props->GetParameterName (p));
	parNode->CreateNodeBefore (CS_NODE_TEXT, 0)->SetValue (val);
	wroteProps = true;
      }
      return wroteProps;
    }
  }

  bool Terrain2SaverCommon::SaveRenderProperties (iDocumentNode* node,
    iTerrainCellRenderProperties* props,
    iTerrainCellRenderProperties* dfltProp)
  {
    bool result = SaveProperties (node, props, dfltProp);

    csSet<csPtrKey<csShaderVariable> > dfltSV;
    if (dfltProp != 0)
    {
      const csRefArray<csShaderVariable>& dfltSVarr =
	dfltProp->GetShaderVariables();
      for (size_t i = 0; i < dfltSVarr.GetSize(); i++)
	dfltSV.Add (dfltSVarr[i]);
    }

    const csRefArray<csShaderVariable>& SVarr =
	props->GetShaderVariables();
    for (size_t i = 0; i < SVarr.GetSize(); i++)
    {
      if (dfltSV.Contains (SVarr[i])) continue;

      csRef<iDocumentNode> svNode = 
        node->CreateNodeBefore (CS_NODE_ELEMENT, 0);
      svNode->SetValue ("shadervar");
      if (!synldr->WriteShaderVar (svNode, *(SVarr[i])))
	return false;

      result = true;
    }

    return result;
  }

  bool Terrain2SaverCommon::SaveFeederProperties (iDocumentNode* node,
    iTerrainCellFeederProperties* props,
    iTerrainCellFeederProperties* dfltProp)
  {
    bool result = SaveProperties (node, props, dfltProp);

    size_t numAlphaMaps = props->GetAlphaMapCount ();
    if (numAlphaMaps == 0) return result;

    if (dfltProp != 0)
    {
      for (size_t a = 0; a < numAlphaMaps; a++)
      {
        const char* mat = props->GetAlphaMapMaterial (a);
        if (mat == 0) continue;
        const char* src = props->GetAlphaMapSource (a);
        if (src == 0) continue;
        const char* dfltSrc = dfltProp->GetAlphaMapSource (mat);
        
	if ((dfltSrc != 0)  && (strcmp (src, dfltSrc) == 0)) continue;
	
        csRef<iDocumentNode> alphaNode = 
          node->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        alphaNode->SetValue ("alphamap");
	alphaNode->SetAttribute ("material", mat);
	alphaNode->CreateNodeBefore (CS_NODE_TEXT, 0)->SetValue (src);
	
        result = true;
      }
    }
    else
    {
      for (size_t a = 0; a < numAlphaMaps; a++)
      {
        const char* mat = props->GetAlphaMapMaterial (a);
        if (mat == 0) continue;
        const char* src = props->GetAlphaMapSource (a);
        if (src == 0) continue;
        
        csRef<iDocumentNode> alphaNode = 
          node->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        alphaNode->SetValue ("alphamap");
	alphaNode->SetAttribute ("material", mat);
	alphaNode->CreateNodeBefore (CS_NODE_TEXT, 0)->SetValue (src);
	
        result = true;
      }
    }
    return result;
  }

  //-------------------------------------------------------------------------

  SCF_IMPLEMENT_FACTORY (Terrain2FactorySaver)
  SCF_IMPLEMENT_FACTORY (Terrain2ObjectSaver)

  Terrain2FactorySaver::Terrain2FactorySaver (iBase* parent)
  : scfImplementationType (this, parent) {}

  Terrain2FactorySaver::~Terrain2FactorySaver () {}

  bool Terrain2FactorySaver::Initialize (iObjectRegistry *objreg)
  {
    if (!Terrain2SaverCommon::Initialize (objreg)) return false;
    return true;
  }

  bool Terrain2FactorySaver::WriteDown (iBase *obj, iDocumentNode *parent,
					iStreamSource *ssource)
  {
    if (!parent) return false; //you never know...

    if (obj)
    {
      csRef<iDocumentNode> paramsNode = 
	parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      paramsNode->SetValue("params");

      csRef<iTerrainFactory> tfact = 
	scfQueryInterface<iTerrainFactory> (obj);
      if (!tfact) return false;

      {
	iTerrainRenderer* render (tfact->GetRenderer());
	csRef<iFactory> renderFact (scfQueryInterfaceSafe<iFactory> (render));
	if (renderFact.IsValid())
	{
	  csRef<iDocumentNode> node = 
	    paramsNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
	  node->SetValue ("renderer");
	  node->CreateNodeBefore (CS_NODE_TEXT, 0)
	    ->SetValue (renderFact->QueryClassID());
	}
      }
      {
	iTerrainCollider* collide (tfact->GetCollider());
	csRef<iFactory> collideFact (scfQueryInterfaceSafe<iFactory> (collide));
	if (collideFact.IsValid())
	{
	  csRef<iDocumentNode> node = 
	    paramsNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
	  node->SetValue ("collider");
	  node->CreateNodeBefore (CS_NODE_TEXT, 0)
	    ->SetValue (collideFact->QueryClassID());
	}
      }
      {
	iTerrainDataFeeder* feed (tfact->GetFeeder());
	csRef<iFactory> feedFact (scfQueryInterfaceSafe<iFactory> (feed));
	if (feedFact.IsValid())
	{
	  csRef<iDocumentNode> node = 
	    paramsNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
	  node->SetValue ("feeder");
	  node->CreateNodeBefore (CS_NODE_TEXT, 0)
	    ->SetValue (feedFact->QueryClassID());
	}
      }
      {
        csRef<iDocumentNode> node = 
          paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        node->SetValue ("maxloadedcells");
        node->CreateNodeBefore (CS_NODE_TEXT, 0)
	  ->SetValueAsInt ((int)tfact->GetMaxLoadedCells());
      }

      iTerrainFactoryCell* defaultCell = tfact->GetDefaultCell();

      csRef<iDocumentNode> cellsNode = 
        paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      cellsNode->SetValue ("cells");

      {
        csRef<iDocumentNode> defaultNode = 
          cellsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        defaultNode->SetValue ("celldefault");

	{
	  csRef<iDocumentNode> node = 
	    defaultNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
	  node->SetValue ("gridsize");
	  node->SetAttributeAsInt ("width", defaultCell->GetGridWidth());
	  node->SetAttributeAsInt ("height", defaultCell->GetGridHeight());
	}

	{
	  csRef<iDocumentNode> node = 
	    defaultNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
	  node->SetValue ("materialmapsize");
	  node->SetAttributeAsInt ("width", defaultCell->GetMaterialMapWidth());
	  node->SetAttributeAsInt ("height", defaultCell->GetMaterialMapHeight());
	}

	{
	  csRef<iDocumentNode> node = 
	    defaultNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
	  node->SetValue ("size");
	  synldr->WriteVector (node, defaultCell->GetSize());
	}

	{
	  iMaterialWrapper* basemat = defaultCell->GetBaseMaterial();
	  if (basemat)
	  {
	    csRef<iDocumentNode> node = 
	      defaultNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
	    node->SetValue ("basematerial");
	    node->CreateNodeBefore (CS_NODE_TEXT, 0)
	     ->SetValue (basemat->QueryObject()->GetName());
	  }
	}

	{
	  iMaterialWrapper* basemat = defaultCell->GetSplatBaseMaterial();
	  if (basemat)
	  {
	    csRef<iDocumentNode> node = 
	      defaultNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
	    node->SetValue ("splatbasematerial");
	    node->CreateNodeBefore (CS_NODE_TEXT, 0)
	     ->SetValue (basemat->QueryObject()->GetName());
	  }
	}

	synldr->WriteBool (defaultNode, "materialmappersistent",
	  defaultCell->GetMaterialPersistent(), false);

	{
	  iTerrainRenderer* render = tfact->GetRenderer();
	  csRef<iTerrainCellRenderProperties> defRenderProp =
	    render ? render->CreateProperties () : 0;

	  csRef<iDocumentNode> node = 
	    defaultNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
	  node->SetValue ("renderproperties");
	  if (!SaveRenderProperties (node, defaultCell->GetRenderProperties(),
	      defRenderProp))
	    defaultNode->RemoveNode (node);
	}

	{
	  iTerrainDataFeeder* feeder = tfact->GetFeeder();
	  csRef<iTerrainCellFeederProperties> defFeedProp =
	    feeder ? feeder->CreateProperties () : 0;

	  csRef<iDocumentNode> node = 
	    defaultNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
	  node->SetValue ("feederproperties");
	  if (!SaveFeederProperties (node, defaultCell->GetFeederProperties(),
	      defFeedProp))
	    defaultNode->RemoveNode (node);
	}

	{
	  iTerrainCollider* collider = tfact->GetCollider();
	  csRef<iTerrainCellCollisionProperties> defCollProp =
	    collider ? collider->CreateProperties () : 0;

	  csRef<iDocumentNode> node = 
	    defaultNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
	  node->SetValue ("colliderproperties");
	  if (!SaveProperties (node, defaultCell->GetCollisionProperties(),
	      (iTerrainCellCollisionProperties*)defCollProp))
	    defaultNode->RemoveNode (node);
	}
      }

      size_t numCells = tfact->GetCellCount();
      for (size_t c = 0; c < numCells; c++)
      {
	iTerrainFactoryCell* cell = tfact->GetCell (c);

        csRef<iDocumentNode> cellNode = 
          cellsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        cellNode->SetValue ("cell");
	const char* name = cell->GetName();
	if (name != 0)
	{
	  csRef<iDocumentNode> node = 
	    cellNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
	  node->SetValue ("name");
	  node->CreateNodeBefore (CS_NODE_TEXT, 0)
	    ->SetValue (name);
	}

	{
	  csRef<iDocumentNode> node = 
	    cellNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
	  node->SetValue ("position");
	  synldr->WriteVector (node, cell->GetPosition());
	}

	if ((defaultCell->GetGridWidth() != cell->GetGridWidth())
	    || (defaultCell->GetGridHeight() != cell->GetGridHeight()))
	{
	  csRef<iDocumentNode> node = 
	    cellNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
	  node->SetValue ("gridsize");
	  node->SetAttributeAsInt ("width", cell->GetGridWidth());
	  node->SetAttributeAsInt ("height", cell->GetGridHeight());
	}

	if ((defaultCell->GetMaterialMapWidth() != cell->GetMaterialMapWidth())
	    || (defaultCell->GetMaterialMapHeight() != cell->GetMaterialMapHeight()))
	{
	  csRef<iDocumentNode> node = 
	    cellNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
	  node->SetValue ("materialmapsize");
	  node->SetAttributeAsInt ("width", cell->GetMaterialMapWidth());
	  node->SetAttributeAsInt ("height", cell->GetMaterialMapHeight());
	}

	if (defaultCell->GetSize() != cell->GetSize())
	{
	  csRef<iDocumentNode> node = 
	    cellNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
	  node->SetValue ("size");
	  synldr->WriteVector (node, cell->GetSize());
	}

	if ((cell->GetBaseMaterial() != 0)
	    && (defaultCell->GetBaseMaterial() != cell->GetBaseMaterial()))
	{
	  csRef<iDocumentNode> node = 
	    cellNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
	  node->SetValue ("basematerial");
	  node->CreateNodeBefore (CS_NODE_TEXT, 0)
	    ->SetValue (cell->GetBaseMaterial()->QueryObject()->GetName());
	}

	synldr->WriteBool (cellNode, "materialmappersistent",
	  cell->GetMaterialPersistent(), defaultCell->GetMaterialPersistent());

	{
	  csRef<iDocumentNode> node = 
	    cellNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
	  node->SetValue ("renderproperties");
	  if (!SaveRenderProperties (node, cell->GetRenderProperties(),
	      defaultCell->GetRenderProperties()))
	    cellNode->RemoveNode (node);
	}

	{
	  csRef<iDocumentNode> node = 
	    cellNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
	  node->SetValue ("feederproperties");
	  if (!SaveFeederProperties (node, cell->GetFeederProperties(),
	      defaultCell->GetFeederProperties()))
	    cellNode->RemoveNode (node);
	}

	{
	  csRef<iDocumentNode> node = 
	    cellNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
	  node->SetValue ("colliderproperties");
	  if (!SaveProperties (node, cell->GetCollisionProperties(),
	      defaultCell->GetCollisionProperties()))
	    cellNode->RemoveNode (node);
	}
      }

    }
    return true;
  }

  //-------------------------------------------------------------------------

  Terrain2ObjectSaver::Terrain2ObjectSaver (iBase* parent)
  : scfImplementationType (this, parent) {}
    
  Terrain2ObjectSaver::~Terrain2ObjectSaver () {}

  bool Terrain2ObjectSaver::Initialize (iObjectRegistry *objreg)
  {
    if (!Terrain2SaverCommon::Initialize (objreg)) return false;
    return true;
  }

  bool Terrain2ObjectSaver::WriteDown (iBase *obj, iDocumentNode *parent,
				       iStreamSource *ssource)
  {
    if (!parent) return false; //you never know...

    csRef<iDocumentNode> paramsNode = 
      parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    paramsNode->SetValue("params");

    if (obj)
    {
      csRef<iTerrainSystem> tmesh = 
	scfQueryInterface<iTerrainSystem> (obj);
      if (!tmesh) return false;
      csRef<iMeshObject> mesh = 
	scfQueryInterface<iMeshObject> (obj);
      if (!mesh) return false;
      csRef<iTerrainFactory> tfact = 
	scfQueryInterface<iTerrainFactory> (mesh->GetFactory());

      // Write down factory tag
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

      const csTerrainMaterialPalette& matpal = tmesh->GetMaterialPalette();
      
      if (matpal.GetSize() > 0)
      {
        csRef<iDocumentNode> matpalNode = 
          paramsNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
        matpalNode->SetValue ("materialpalette");

	for (size_t i = 0; i < matpal.GetSize(); i++)
	{
	  csRef<iDocumentNode> node = 
	    matpalNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
	  node->SetValue ("material");
	  node->CreateNodeBefore (CS_NODE_TEXT, 0)
	    ->SetValue (matpal[i]->QueryObject()->GetName());
	}
      }

      csRef<iDocumentNode> cellsNode = 
        paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      cellsNode->SetValue ("cells");

      size_t numCells = tmesh->GetCellCount();
      for (size_t c = 0; c < numCells; c++)
      {
	iTerrainCell* cell = tmesh->GetCell (c);
	const char* name = cell->GetName();
	if (name == 0) continue;
	iTerrainFactoryCell* factCell = tfact->GetCell (name);

        csRef<iDocumentNode> cellNode = 
          cellsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        cellNode->SetValue ("cell");
	{
	  csRef<iDocumentNode> node = 
	    cellNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
	  node->SetValue ("name");
	  node->CreateNodeBefore (CS_NODE_TEXT, 0)
	    ->SetValue (name);
	}

	/* @@@ TBD what other cell settings are sensible for overriding in 
	       the object? */

	{
	  csRef<iDocumentNode> node = 
	    cellNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
	  node->SetValue ("renderproperties");
	  if (!SaveRenderProperties (node, cell->GetRenderProperties(),
	      factCell->GetRenderProperties()))
	    cellNode->RemoveNode (node);
	}

	{
	  csRef<iDocumentNode> node = 
	    cellNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
	  node->SetValue ("feederproperties");
	  if (!SaveProperties (node, cell->GetFeederProperties(),
	      factCell->GetFeederProperties()))
	    cellNode->RemoveNode (node);
	}

	{
	  csRef<iDocumentNode> node = 
	    cellNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
	  node->SetValue ("colliderproperties");
	  if (!SaveProperties (node, cell->GetCollisionProperties(),
	      factCell->GetCollisionProperties()))
	    cellNode->RemoveNode (node);
	}
      }
    }
    return true;
  }

}
CS_PLUGIN_NAMESPACE_END(Terrain2Loader)
