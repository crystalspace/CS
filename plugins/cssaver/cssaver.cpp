/*
    Copyright (C) 2002 by Jorrit Tyberghein

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
#include "plugins/cssaver/cssaver.h"
#include "iutil/vfs.h"
#include "iutil/document.h"
#include "iutil/object.h"
#include "iutil/objreg.h"
#include "iutil/string.h"
#include "iutil/plugin.h"
#include "imesh/object.h"
#include "iengine/engine.h"
#include "iengine/texture.h"
#include "iengine/material.h"
#include "iengine/sector.h"
#include "iengine/mesh.h"
#include "iengine/campos.h"
#include "iengine/portal.h"
#include "iengine/portalcontainer.h"
#include "igraphic/image.h"
#include "ivideo/texture.h"
#include "ivideo/material.h"
#include "ivideo/txtmgr.h"
#include "ivideo/graph3d.h"
#include "ivaria/reporter.h"
#include "imap/writer.h"
#include "csutil/xmltiny.h"
#include "csutil/scfstr.h"
#include "csgfx/rgbpixel.h"

#define ONE_OVER_256 (1.0/255.0)

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY(csSaver);

SCF_IMPLEMENT_IBASE(csSaver);
  SCF_IMPLEMENTS_INTERFACE(iSaver);
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent);
SCF_IMPLEMENT_IBASE_END;

SCF_IMPLEMENT_EMBEDDED_IBASE (csSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csSaver::csSaver(iBase *p)
{
  SCF_CONSTRUCT_IBASE(p);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  object_reg = 0;
}

csSaver::~csSaver()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE();
}

bool csSaver::Initialize(iObjectRegistry* p)
{
  object_reg = p;
  engine = CS_QUERY_REGISTRY(object_reg, iEngine);
  return true;
}

csRef<iDocumentNode> csSaver::CreateNode(
  iDocumentNode* parent, const char* name)
{
  csRef<iDocumentNode> child = parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  child->SetValue (name);
  return child;
}

csRef<iDocumentNode> csSaver::CreateValueNode(
  iDocumentNode* parent, const char* name, const char* value)
{
  csRef<iDocumentNode> child = CreateNode(parent, name);
  csRef<iDocumentNode> text = child->CreateNodeBefore(CS_NODE_TEXT, 0);
  text->SetValue (value);
  return child;
}

csRef<iDocumentNode> csSaver::CreateValueNodeAsFloat(
  iDocumentNode* parent, const char* name, float value)
{
  csRef<iDocumentNode> child = CreateNode(parent, name);
  csRef<iDocumentNode> text = child->CreateNodeBefore(CS_NODE_TEXT, 0);
  text->SetValueAsFloat (value);
  return child;
}

csRef<iDocumentNode> csSaver::CreateValueNodeAsColor(
  iDocumentNode* parent, const char* name, const csColor &color)
{
  csRef<iDocumentNode> child=CreateNode(parent, name);
  child->SetAttributeAsFloat("red",   color.red  );
  child->SetAttributeAsFloat("green", color.green);
  child->SetAttributeAsFloat("blue",  color.blue );
  return child;
}

bool csSaver::SaveTextures(iDocumentNode *parent)
{
  csRef<iDocumentNode> current = CreateNode(parent, "textures");

  iTextureList* texList=engine->GetTextureList();
  for (int i = 0; i < texList->GetCount(); i++)
  {
    iTextureWrapper *texWrap=texList->Get(i);
    csRef<iDocumentNode> child = current->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    const char *name=texWrap->QueryObject()->GetName();
    child->SetValue("texture");
    if (name && *name)
      child->SetAttribute("name", name);
    iImage* img = texWrap->GetImageFile();
    if (img)
    {
      const char* filename=img->GetName();
      if (filename && *filename)
        CreateValueNode(child, "file", filename);

#if 0
      //TODO behle not currently read from map, so no point to write back out
      if (img->HasKeycolor() == 1)
      {
        int r,g,b;
        img->GetKeycolor(r, g, b);
        CreateValueNodeAsColor(child, "transparent",
	  csColor(r * ONE_OVER_256, g * ONE_OVER_256, b * ONE_OVER_256));
      }
#endif
    }
    else
    {
      //CS_ASSERT(false);
    }
  }
  return true;
}

bool csSaver::SaveMaterials(iDocumentNode *parent)
{
  csRef<iDocumentNode> current = CreateNode(parent, "materials");
  iMaterialList *matList=engine->GetMaterialList();
  for (int i = 0; i < matList->GetCount(); i++)
  {
    iMaterialWrapper* matWrap = matList->Get(i);
    CS_ASSERT(matWrap);
    iMaterial* mat = matWrap->GetMaterial();
    CS_ASSERT(mat);
    csRef<iMaterialEngine> matEngine(SCF_QUERY_INTERFACE(mat,iMaterialEngine));
    CS_ASSERT(matEngine);

    iTextureWrapper* texWrap = matEngine->GetTextureWrapper();
    csRef<iDocumentNode> child = CreateNode(current, "material");

    const char* name = matWrap->QueryObject()->GetName();
    if (name && *name)
      child->SetAttribute ("name", name);

    csRGBpixel color;
    matWrap->GetMaterial()->GetFlatColor(color, 0);
    if (color.red != 255 || color.green != 255 || color.blue != 255)
    {
      CreateValueNodeAsColor(child, "color",
        csColor(color.red * ONE_OVER_256, color.green * ONE_OVER_256,
	  color.blue * ONE_OVER_256));
    }

    if(texWrap)
    {
      const char* texname = texWrap->QueryObject()->GetName();
      if (texname && *texname)
        CreateValueNode(child, "texture", texname);
    }

#ifndef CS_USE_NEW_RENDERER
    int layerCount = mat->GetTextureLayerCount();
    for(int i = 0; i < layerCount; i++)
    {
      csRef<iDocumentNode> layerItem = CreateNode(child, "layer");

      iTextureWrapper* layerTexWrap = matEngine->GetTextureWrapper(i);
      if(layerTexWrap)
      {
        const char* texname = layerTexWrap->QueryObject()->GetName();
        if (texname && *texname)
          CreateValueNode(layerItem, "texture", texname);
      }

      csTextureLayer* texLayer = mat->GetTextureLayer(i);
      if (texLayer->uscale!=1.0f || texLayer->vscale!=1.0f)
      {
        csRef<iDocumentNode> scaleItem = CreateNode(layerItem, "scale");
        scaleItem->SetAttributeAsFloat("u", texLayer->uscale);
        scaleItem->SetAttributeAsFloat("v", texLayer->vscale);
      }
      if (texLayer->ushift != 0.0f || texLayer->vshift != 0.0f)
      {
        csRef<iDocumentNode> shiftItem = CreateNode(layerItem, "shift");
        shiftItem->SetValue("shift");
        shiftItem->SetAttributeAsFloat("u", texLayer->ushift);
        shiftItem->SetAttributeAsFloat("v", texLayer->vshift);
      }

      if (texLayer->mode != (CS_FX_ADD|CS_FX_TILING) )
      {
        int blendmode=texLayer->mode & CS_FX_MASK_MIXMODE;

        csRef<iDocumentNode> mixmodeItem = CreateNode(layerItem, "mixmode");
        if(blendmode == CS_FX_ALPHA) {
          int alpha = texLayer->mode & CS_FX_MASK_ALPHA;
          CreateValueNodeAsFloat(mixmodeItem, "alpha", alpha*ONE_OVER_256);
        }
        if (blendmode == CS_FX_ADD) {
          CreateNode(mixmodeItem, "add");
        }
        if (blendmode == CS_FX_MULTIPLY)
        {
          CreateNode(mixmodeItem, "multiply");
        }
        if (blendmode == CS_FX_MULTIPLY2)
        {
          CreateNode(mixmodeItem, "multiply2");
        }
        if (blendmode == CS_FX_TRANSPARENT)
        {
          CreateNode(mixmodeItem, "transparent");
        }
        if (texLayer->mode & CS_FX_KEYCOLOR)
        {
          CreateNode(mixmodeItem, "keycolor");
        }
        if (texLayer->mode & CS_FX_TILING)
        {
          CreateNode(mixmodeItem, "tiling");
        }
      }
    }
#endif
  }
  return true;
}

bool csSaver::SaveRenderPriorities(iDocumentNode *parent)
{
  csRef<iDocumentNode> rpnode = CreateNode(parent, "renderpriorities");
  int rpcount = engine->GetRenderPriorityCount();
  for(int i = 0; i < rpcount; i++)
  {
    const char *rpname = engine->GetRenderPriorityName(i);
    if(rpname)
    {
      csRef<iDocumentNode> prioritynode = CreateNode(rpnode, "priority");

      rpnode->SetAttribute("name", rpname);
      csRef<iDocumentNode> levelnode = CreateNode(prioritynode, "level");
      levelnode->SetValueAsInt(i);

      csRef<iDocumentNode> sortnode = CreateNode(prioritynode, "sort");
      int sorttype = engine->GetRenderPrioritySorting(i);
      switch(sorttype)
      {
        case CS_RENDPRI_NONE:
          sortnode->SetValue("NONE");
          break;
        case CS_RENDPRI_BACK2FRONT:
          sortnode->SetValue("BACK2FRONT");
          break;
        case CS_RENDPRI_FRONT2BACK:
          sortnode->SetValue("FRONT2BACK");
          break;
/*      case CS_RENDPRI_MATERIAL:
          sortnode->SetValue("MATERIAL");
          break;*/
      }
    }
  }
  return true;
}


bool csSaver::SaveCameraPositions(iDocumentNode *parent)
{
  csRef<iCameraPositionList> camlist = engine->GetCameraPositions();
	
  for(int i = 0; i < camlist->GetCount(); i++)
  {
    csRef<iCameraPosition> cam = camlist->Get(i);
		
    csRef<iDocumentNode> n = CreateNode(parent, "start");
    // Set the name attribute if cam pos has a name
    const char *camname = cam->QueryObject()->GetName();
    if(camname && strcmp(camname, "") != 0)
      n->SetAttribute("name", camname);

    // write the sector
    csRef<iDocumentNode> sectornode = CreateNode(n, "sector");
    const char *sectorname = cam->GetSector();
    if(sectorname)
      sectornode->SetValue(cam->GetSector());
      
    // write position
    csRef<iDocumentNode> positionnode = CreateNode(n, "position");
    positionnode->SetAttributeAsFloat("x", cam->GetPosition().x);
    positionnode->SetAttributeAsFloat("y", cam->GetPosition().y);
    positionnode->SetAttributeAsFloat("z", cam->GetPosition().z);

    // write up vector
    csRef<iDocumentNode> upnode = CreateNode(n, "up");
    upnode->SetAttributeAsFloat("x", cam->GetUpwardVector().x);
    upnode->SetAttributeAsFloat("y", cam->GetUpwardVector().y);
    upnode->SetAttributeAsFloat("z", cam->GetUpwardVector().z);

    // write forward vector
    csRef<iDocumentNode> forwardnode = CreateNode(n, "forward");
    forwardnode->SetAttributeAsFloat("x", cam->GetForwardVector().x);
    forwardnode->SetAttributeAsFloat("y", cam->GetForwardVector().y);
    forwardnode->SetAttributeAsFloat("z", cam->GetForwardVector().z);

    // write farplane if available
    csPlane3 *fp = cam->GetFarPlane();
    if(fp)
    {
      csRef<iDocumentNode> farplanenode = CreateNode(n, "farplane");
      farplanenode->SetAttributeAsFloat("a", fp->A());
      farplanenode->SetAttributeAsFloat("b", fp->B());
      farplanenode->SetAttributeAsFloat("c", fp->C());
      farplanenode->SetAttributeAsFloat("d", fp->D());
    }
  }
  return true;
}

bool csSaver::SaveSectors(iDocumentNode *parent)
{
  iSectorList* sectorList=engine->GetSectors();
  for (int i=0; i<sectorList->GetCount(); i++)
  {
    iSector* sector = sectorList->Get(i);
    csRef<iDocumentNode> sectorNode = CreateNode(parent, "sector");
    const char* name = sector->QueryObject()->GetName();
    if (name && *name) sectorNode->SetAttribute("name", name);
    if(!SaveSectorMeshes(sector, sectorNode)) return false;
    if(!SaveSectorLights(sector, sectorNode)) return false;
	}
	return true;
}

bool csSaver::SaveSectorMeshes(iSector *s, iDocumentNode *parent)
{
  iMeshList* meshList = s->GetMeshes();
  for (int i=0; i<meshList->GetCount(); i++)
  {
    iMeshWrapper* meshwrapper = meshList->Get(i);
    //Create the Tag for the MeshObj
    csRef<iDocumentNode> meshNode = CreateNode(parent, "meshobj");
    //Check if it's a portal
    csRef<iPortalContainer> portal = SCF_QUERY_INTERFACE(meshwrapper->GetMeshObject(),
                                                         iPortalContainer);
    if (portal) 
    {
      meshNode->SetValue ("portal");
      for (int i=0; i<portal->GetPortalCount(); i++)
        if (!SavePortals(portal->GetPortal(i), meshNode)) return false;
    }
    //Add the mesh's name to the MeshObj tag
    const char* name = meshwrapper->QueryObject()->GetName();
    if (name && *name) 
    meshNode->SetAttribute("name", name);

    //Let the iSaverPlugin write the parameters of the mesh
    //It has to create the params node itself, maybe it might like to write
    //more things outside the params at a later stage. you never know ;)
    csRef<iFactory> factory;
    iMeshObjectFactory* meshobjectfactory = meshwrapper->GetMeshObject()->GetFactory();
    if (meshobjectfactory)
      factory = SCF_QUERY_INTERFACE(meshobjectfactory->GetMeshObjectType(), iFactory);
    else
    {
      char error[128];
      sprintf(error, "Factory less Mesh found! %s => Please fix or report to Jorrit ;)", name);
      csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.plugin.cssaver", error);
    }
    if (factory)
    {
      csString pluginname = factory->QueryClassID();
      if (pluginname && *pluginname)
      {
        csRef<iDocumentNode> pluginNode = CreateNode(meshNode, "plugin");

        //Add the plugin tag
        pluginNode->CreateNodeBefore(CS_NODE_TEXT)->SetValue((const char*)pluginname);
        csRef<iPluginManager> plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);

        //Replace the object with saver in the ID
        const char* origstring = ".object.";
        const char* newstring = ".saver.";
        for (int i=0; i<pluginname.Length()-8; i++)
        {
          if (pluginname.Slice(i,8).Compare(origstring))
            pluginname=pluginname.Slice(0,i)+newstring + pluginname.Slice(i+8,pluginname.Length());
        }

        //Invoke the iSaverPlugin::WriteDown
        csRef<iSaverPlugin> saver = CS_QUERY_PLUGIN_CLASS(plugin_mgr, pluginname, iSaverPlugin);
        if (!saver) saver = CS_LOAD_PLUGIN(plugin_mgr, pluginname, iSaverPlugin);
        if (saver) saver->WriteDown(meshwrapper->GetMeshObject(), meshNode);
      }

    }      
    //TBD: write <priority>
    //TBD: write other tags
  }
	return true;
}

bool csSaver::SavePortals(iPortal *portal, iDocumentNode *parent)
{
  csRef<iDocumentNode> sectorNode = CreateNode(parent, "sector");
  portal->CompleteSector(0);
  iSector* sector = portal->GetSector();
  if (sector)
  {
    const char* name = sector->QueryObject()->GetName();
    if (name && *name) sectorNode->SetAttribute("name", name);
  }
  //saving the vertices (portal->GetVertices(), portal->GetVertexIndicesCount());

  return true;
}

bool csSaver::SaveSectorLights(iSector *s, iDocumentNode *parent)
{
  iLightList* lightList = s->GetLights();
  for (int i=0; i<lightList->GetCount(); i++)
  {
    iLight* light = lightList->Get(i);
    //Create the Tag for the Light
    csRef<iDocumentNode> lightNode = CreateNode(parent, "light");

    //Add the light's name to the MeshObj tag
    csString name = light->QueryObject()->GetName();
    if (!name.Compare("__light__")) lightNode->SetAttribute("name", name);

    //Add the light center node
    csVector3 center = light->GetCenter();
    csRef<iDocumentNode> centerNode = CreateNode(lightNode, "center");
    centerNode->SetAttributeAsFloat("x", center.x);
    centerNode->SetAttributeAsFloat("y", center.y);
    centerNode->SetAttributeAsFloat("z", center.z);

    //Add the light center node
    int radius = light->GetInfluenceRadius();
    csRef<iDocumentNode> radiusNode = CreateNode(lightNode, "radius");
    radiusNode->CreateNodeBefore(CS_NODE_TEXT)->SetValueAsFloat(radius);

    //Add the light center node
    csColor color = light->GetColor();
    csRef<iDocumentNode> colorNode = CreateNode(lightNode, "center");
    colorNode->SetAttributeAsFloat("red", color.red);
    colorNode->SetAttributeAsFloat("green", color.green);
    colorNode->SetAttributeAsFloat("blue", color.blue);
  }
  return true;
}

csRef<iString> csSaver::SaveMapFile()
{
  csRef<iDocumentSystem> xml = csPtr<iDocumentSystem>(new csTinyDocumentSystem());
  csRef<iDocument> doc = xml->CreateDocument();
  csRef<iDocumentNode> root = doc->CreateRoot();
  csRef<iDocumentNode> parent = root->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  parent->SetValue("world");

  //TBD: this crashes for me, dunno why, have to look at it more closley.
  if (!SaveTextures(parent)) return 0;
  if (!SaveMaterials(parent)) return 0;
  
  //TBD: Save the Factories
  //if (!SaveFactories(parent)) return 0;
  
  if (!SaveSectors(parent)) return 0;

  iString* str = new scfString();
  if (doc->Write(str) != 0)
  {
    delete str;
    return 0;
  }

  return csPtr<iString>(str);
}

bool csSaver::SaveMapFile(const char* filename)
{
  csRef<iVFS> vfs(CS_QUERY_REGISTRY(object_reg, iVFS));
  CS_ASSERT(vfs.IsValid());

  csRef<iString> str(SaveMapFile());
  if (!str)
    return 0;

  return vfs->WriteFile(filename, str->GetData(), str->Length());
}

bool csSaver::SaveMapFile(csRef<iDocumentNode> &root)
{
  csRef<iDocumentNode> parent = root->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  parent->SetValue("world");

  if (!SaveTextures(parent)) return false;
  if (!SaveMaterials(parent)) return false;

  /// TODO: write mesh objects, factories
  return true;
}
