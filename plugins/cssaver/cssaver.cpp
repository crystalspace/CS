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
#include "cssaver.h"
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
#include "iengine/movable.h"
#include "iengine/campos.h"
#include "iengine/portal.h"
#include "iengine/portalcontainer.h"
#include "igraphic/image.h"
#include "ivideo/texture.h"
#include "ivideo/material.h"
#include "ivideo/txtmgr.h"
#include "ivideo/graph3d.h"
#include "ivaria/reporter.h"
#include "imap/services.h"
#include "imap/writer.h"
#include "csutil/xmltiny.h"
#include "csutil/scfstr.h"
#include "csutil/util.h"
#include "csgfx/rgbpixel.h"
#include "imesh/thing.h"
#include "iengine/renderloop.h"
#include "iengine/sharevar.h"
#include "plugins/engine/3d/halo.h"
#include "ivideo/halo.h"
#include "iengine/halo.h"
#include "itexture/ifire.h"
#include "itexture/itexfact.h"
#include "itexture/iproctex.h"
#include "cstool/proctex.h"
#include "ivaria/engseq.h"
#include "ivaria/sequence.h"

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
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
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
    csRef<iDocumentNode>child = current->CreateNodeBefore(CS_NODE_ELEMENT, 0);
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

      int r,g,b, r2,g2,b2;
      texWrap->GetKeyColor(r, g, b);
      if (r != -1)
      {
        if (img->HasKeyColor())
        {
          img->GetKeyColor(r2, g2, b2);
        }
        if (r != r2 || g != g2 || b != b2)
          CreateValueNodeAsColor(child, "transparent",
	    csColor(r * ONE_OVER_256, g * ONE_OVER_256, b * ONE_OVER_256));
      }
    }

    iTextureCallback* texCb = texWrap->GetUseCallback();
    if (!texCb) continue;

    csRef<iProcTexCallback> proctexCb = 
      SCF_QUERY_INTERFACE(texCb, iProcTexCallback);
    if (!proctexCb) continue;

    iProcTexture* proctex = proctexCb->GetProcTexture();
    if (!proctex) continue;

    iTextureFactory* texfact = proctex->GetFactory();
    if (!texfact) continue;

    iTextureType* textype = texfact->GetTextureType();
    if (!textype) continue;

    csRef<iFactory> fact = SCF_QUERY_INTERFACE(textype, iFactory);
    if (!fact) continue;

    char loadername[128] = "";
    csFindReplace(loadername, fact->QueryClassID(), ".type.", ".loader.",128);
    CreateValueNode(child, "type", loadername);

    char savername[128] = "";
    csFindReplace(savername, fact->QueryClassID(), ".type.", ".saver.", 128);

    //Invoke the iSaverPlugin::WriteDown
    csRef<iPluginManager> plugin_mgr = 
      CS_QUERY_REGISTRY (object_reg, iPluginManager);
    csRef<iSaverPlugin> saver = 
      CS_QUERY_PLUGIN_CLASS(plugin_mgr, savername, iSaverPlugin);
    if (!saver) 
      saver = CS_LOAD_PLUGIN(plugin_mgr, savername, iSaverPlugin);
    if (saver)
      saver->WriteDown(proctex, child);
  }
  return true;
}

bool csSaver::SaveMaterials(iDocumentNode *parent)
{
#ifndef CS_USE_OLD_RENDERER
  csRef<iStringSet> stringset =
    CS_QUERY_REGISTRY_TAG_INTERFACE(object_reg, 
    "crystalspace.shared.stringset", iStringSet);

  csStringID texdiffID = stringset->Request("tex diffuse");
  csStringID matdiffID = stringset->Request("mat diffuse");
  csStringID matambiID = stringset->Request("mat ambient");
  csStringID matreflID = stringset->Request("mat reflection");
  csStringID matflatcolID = stringset->Request("mat flatcolor");
  csStringID orlightID = stringset->Request("or_lighting");
  csStringID orcompatID = stringset->Request("OR compatibility");
#endif

  csRef<iDocumentNode> current = CreateNode(parent, "materials");
  iMaterialList *matList=engine->GetMaterialList();
  for (int i = 0; i < matList->GetCount(); i++)
  {
    iMaterialWrapper* matWrap = matList->Get(i);
    CS_ASSERT(matWrap);
    iMaterial* mat = matWrap->GetMaterial();
    CS_ASSERT(mat);
    csRef<iMaterialEngine>matEngine(SCF_QUERY_INTERFACE(mat,iMaterialEngine));
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

#ifdef CS_USE_OLD_RENDERER
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
#else

    csHash<csRef<iShader>, csStringID> shaders = mat->GetShaders();
    csHash<csRef<iShader>, csStringID>::GlobalIterator shaderIter = 
      shaders.GetIterator();

    while (shaderIter.HasNext())
    {
      csStringID typeID;
      iShader* shader = shaderIter.Next(typeID);
      if (!shader || !typeID) continue;
      const char *shadername = shader->QueryObject()->GetName();
      const char *shadertype = stringset->Request(typeID);
      if (orcompatID == typeID && orlightID == stringset->Request(shadername))
        continue;

      csRef<iDocumentNode> shaderNode = CreateNode(child, "shader");
      shaderNode->SetAttribute("type", shadertype);
      shaderNode->CreateNodeBefore(CS_NODE_TEXT)->SetValue(shadername);
    }

    csRefArray<csShaderVariable> shaderVars = mat->GetShaderVariables();
    csRefArray<csShaderVariable>::Iterator shaderVarIter =
      shaderVars.GetIterator();

    while (shaderVarIter.HasNext())
    {
      csShaderVariable* shaderVar = shaderVarIter.Next();
      csStringID varnameid = shaderVar->GetName();
      csString varname(stringset->Request(varnameid));
      csRef<iDocumentNode> shadervarNode = CreateNode(child, "shadervar");
      shadervarNode->SetAttribute("name", (const char*)varname);

      switch(shaderVar->GetType())
      {
        case csShaderVariable::INT:
          {
            int intval;
            if(shaderVar->GetValue(intval))
            {
              shadervarNode->SetAttribute("type", "int");
              shadervarNode->CreateNodeBefore(CS_NODE_TEXT)->
                SetValueAsInt(intval);
            }
          }
          break;
        case csShaderVariable::FLOAT:
          {
            float fval;
            if(shaderVar->GetValue(fval))
            {
              if ((varnameid == matdiffID && fval == 0) ||
                  (varnameid == matambiID && fval == 0) ||
                  (varnameid == matreflID && fval == 0) )
              {
                child->RemoveNode(shadervarNode);
                break;
              }
              shadervarNode->SetAttribute("type", "float");
              shadervarNode->CreateNodeBefore(CS_NODE_TEXT)->
                SetValueAsFloat(fval);
            }
          }
          break;
        case csShaderVariable::VECTOR2:
          {
            csVector2 v2;
            if(shaderVar->GetValue(v2))
            {
              shadervarNode->SetAttribute("type", "vector2");
              csString value;
              value.Format("%f,%f",v2.x,v2.y);
              shadervarNode->CreateNodeBefore(CS_NODE_TEXT)->SetValue(
                (const char*)value);
            }
          }
          break;
        case csShaderVariable::VECTOR3:
          {
            csVector3 v3;
            if(shaderVar->GetValue(v3))
            {
              if (varnameid == matflatcolID && (v3 == 1 || v3.x*255 == color.red
                  && v3.y *255 == color.green && v3.z *255 == color.blue))
              {
                child->RemoveNode(shadervarNode);
                break;
              }

              shadervarNode->SetAttribute("type", "vector3");
              csString value;
              value.Format("%f,%f,%f",v3.x,v3.y,v3.z);
              shadervarNode->CreateNodeBefore(CS_NODE_TEXT)->SetValue(
              (const char*)value);
            }
          }
          break;
        case csShaderVariable::VECTOR4:
          {
            csVector4 v4;
            if(shaderVar->GetValue(v4))
            {
              shadervarNode->SetAttribute("type", "vector4");
              csString value;
              value.Format("%f,%f,%f,%f",v4.x,v4.y,v4.z,v4.w);
              shadervarNode->CreateNodeBefore(CS_NODE_TEXT)->SetValue(
                (const char*)value);
            }
          }
          break;
        case csShaderVariable::TEXTURE:
          {
            iTextureWrapper* tex;
            if(shaderVar->GetValue(tex) && tex && 
              !(varnameid == texdiffID && tex == texWrap))
            {
              shadervarNode->SetAttribute("type", "texture");
              shadervarNode->CreateNodeBefore(CS_NODE_TEXT)->
                SetValue(tex->QueryObject()->GetName());
            }
            else
              child->RemoveNode(shadervarNode);
          }
          break;
        default:
          //int i=0;
	  break;
      }

    }

#endif
  }
  return true;
}

bool csSaver::SaveShaders (iDocumentNode *parent)
{
#ifndef CS_USE_OLD_RENDERER
  csRef<iDocumentNode> shadersNode = CreateNode(parent, "shaders");
  csRef<iShaderManager> shaderMgr = 
    CS_QUERY_REGISTRY (object_reg, iShaderManager);
  if (!shaderMgr) return false;

  csRefArray<iShader> shaders = shaderMgr->GetShaders ();
  size_t i;
  for (i = 0 ; i < shaders.Length () ; i++)
  {
    iShader* shader = shaders[i];
    //const char* shadername = shader->QueryObject()->GetName();
    const char* shaderfile = shader->GetFileName();
    if (shaderfile && *shaderfile)
    {
      csRef<iDocumentNode> shaderNode = CreateNode(shadersNode, "shader");
      CreateNode(shaderNode, "file")
        ->CreateNodeBefore(CS_NODE_TEXT)->SetValue(shaderfile);
    }
  }
#endif
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

      prioritynode->SetAttribute("name", rpname);
      csRef<iDocumentNode> levelnode = CreateNode(prioritynode, "level");
      levelnode->CreateNodeBefore(CS_NODE_TEXT)->SetValueAsInt(i);

      csRef<iDocumentNode> sortnode = CreateNode(prioritynode, "sort");
      int sorttype = engine->GetRenderPrioritySorting(i);
      switch(sorttype)
      {
        case CS_RENDPRI_NONE:
          sortnode->CreateNodeBefore(CS_NODE_TEXT)->SetValue("NONE");
          break;
        case CS_RENDPRI_BACK2FRONT:
          sortnode->CreateNodeBefore(CS_NODE_TEXT)->SetValue("BACK2FRONT");
          break;
        case CS_RENDPRI_FRONT2BACK:
          sortnode->CreateNodeBefore(CS_NODE_TEXT)->SetValue("FRONT2BACK");
          break;
/*      case CS_RENDPRI_MATERIAL:
          sortnode->CreateNodeBefore(CS_NODE_TEXT)->SetValue("MATERIAL");
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

bool csSaver::SaveMeshFactories(iMeshFactoryList* factList, 
                                iDocumentNode *parent)
{
  for (int i=0; i<factList->GetCount(); i++)
  {
    iMeshFactoryWrapper* meshfactwrap = factList->Get(i);
    iMeshObjectFactory*  meshfact = meshfactwrap->GetMeshObjectFactory();

    //Create the Tag for the MeshObj
    csRef<iDocumentNode> factNode = CreateNode(parent, "meshfact");

    //Add the mesh's name to the MeshObj tag
    const char* name = meshfactwrap->QueryObject()->GetName();
    if (name && *name) 
    factNode->SetAttribute("name", name);

    csRef<iFactory> factory = 
      SCF_QUERY_INTERFACE(meshfact->GetMeshObjectType(), iFactory);

    const char* pluginname = factory->QueryClassID();

    if (!(pluginname && *pluginname)) continue;

    csRef<iDocumentNode> pluginNode = CreateNode(factNode, "plugin");

    //Add the plugin tag
    char loadername[128] = "";
    csFindReplace(loadername, pluginname, ".object.", ".loader.factory.",128);

    pluginNode->CreateNodeBefore(CS_NODE_TEXT)->SetValue(loadername);
    csRef<iPluginManager> plugin_mgr = 
      CS_QUERY_REGISTRY (object_reg, iPluginManager);

    char savername[128] = "";
    csFindReplace(savername, pluginname, ".object.", ".saver.factory.", 128);

    //Invoke the iSaverPlugin::WriteDown
    csRef<iSaverPlugin> saver = 
      CS_QUERY_PLUGIN_CLASS(plugin_mgr, savername, iSaverPlugin);
    if (!saver) 
      saver = CS_LOAD_PLUGIN(plugin_mgr, savername, iSaverPlugin);
    if (saver) 
      saver->WriteDown(meshfactwrap->GetMeshObjectFactory(), factNode);
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
    if(!SaveSectorMeshes(sector->GetMeshes(), sectorNode)) return false;
    if(!SaveSectorLights(sector, sectorNode)) return false;
    }
  return true;
}

bool csSaver::SaveSectorMeshes(iMeshList *meshList, iDocumentNode *parent)
{
  for (int i=0; i<meshList->GetCount(); i++)
  {
    iMeshWrapper* meshwrapper = meshList->Get(i);
    //Check if it's a portal
    csRef<iPortalContainer> portal = 
      SCF_QUERY_INTERFACE(meshwrapper->GetMeshObject(), iPortalContainer);
    if (portal) 
    {
      for (int i=0; i<portal->GetPortalCount(); i++)
        if (!SavePortals(portal->GetPortal(i), parent)) continue;

      continue;
    }

    //Create the Tag for the MeshObj
    csRef<iDocumentNode> meshNode = CreateNode(parent, "meshobj");

    //Add the mesh's name to the MeshObj tag
    const char* name = meshwrapper->QueryObject()->GetName();
    if (name && *name) 
    meshNode->SetAttribute("name", name);

    //Let the iSaverPlugin write the parameters of the mesh
    //It has to create the params node itself, maybe it might like to write
    //more things outside the params at a later stage. you never know ;)
    csRef<iFactory> factory;
    iMeshObjectFactory* meshobjectfactory = 
      meshwrapper->GetMeshObject()->GetFactory();
    if (meshobjectfactory)
      factory = 
        SCF_QUERY_INTERFACE(meshobjectfactory->GetMeshObjectType(), iFactory);
    else
    {
      char error[128];
      sprintf(error, "Factory less Mesh found! %s => \
                        Please fix or report to Jorrit ;)", name);
      csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.plugin.cssaver", error);
    }
    if (factory)
    {
      const char* pluginname = factory->QueryClassID();
      if (pluginname && *pluginname)
      {
        csRef<iDocumentNode> pluginNode = CreateNode(meshNode, "plugin");

        //Add the plugin tag
        char loadername[128] = "";
        csFindReplace(loadername, pluginname, ".object.", ".loader.", 128);

        pluginNode->CreateNodeBefore(CS_NODE_TEXT)->SetValue(loadername);
        csRef<iPluginManager> plugin_mgr = 
          CS_QUERY_REGISTRY (object_reg, iPluginManager);

        char savername[128] = "";
        csFindReplace(savername, pluginname, ".object.", ".saver.", 128);

        //Invoke the iSaverPlugin::WriteDown
        csRef<iSaverPlugin> saver = 
          CS_QUERY_PLUGIN_CLASS(plugin_mgr, savername, iSaverPlugin);
          
        if (!saver) 
          saver = CS_LOAD_PLUGIN(plugin_mgr, savername, iSaverPlugin);
          
        if (saver)
          saver->WriteDown(meshwrapper->GetMeshObject(), meshNode);
      }

    }
    if (CS_ZBUF_FILL == meshwrapper->GetZBufMode())
      CreateNode(meshNode, "zfill");

    //TBD: write <priority>
    //TBD: write other tags

    //Add the transformation tags
    csVector3 moveVect =
      -meshwrapper->GetMovable()->GetTransform().GetT2OTranslation();
    csMatrix3 moveMatrix =
      meshwrapper->GetMovable()->GetTransform().GetT2O();
    if (moveVect != 0 || !moveMatrix.IsIdentity())
    {
      //Add the move tag
      csRef<iDocumentNode> moveNode = CreateNode(meshNode, "move");

      //Add the matrix tag
      if (!moveMatrix.IsIdentity())
      {
        csRef<iDocumentNode> matrixNode = CreateNode(moveNode, "matrix");
        synldr->WriteMatrix(matrixNode, &moveMatrix);
      }

      //Add the v tag
      if (moveVect != 0)
      {
        csRef<iDocumentNode> vNode = CreateNode(moveNode, "v");
        synldr->WriteVector(vNode, &moveVect);
      }
    }

    //Save all childmeshes
    iMeshList* childlist = meshwrapper->GetChildren();
    if (childlist) SaveSectorMeshes(childlist, meshNode);
  }
  return true;
}

bool csSaver::SavePortals(iPortal *portal, iDocumentNode *parent)
{
  portal->CompleteSector(0);

  //Create the Tag for the Portal
  csRef<iDocumentNode> portalNode = CreateNode(parent, "portal");

  const char* portalname = portal->GetName();
  if (portalname && *portalname) 
    portalNode->SetAttribute("name", portalname);

  //Write the vertex tags
  for (int vertidx = 0; vertidx < portal->GetVertexIndicesCount(); vertidx++)
  {
    csRef<iDocumentNode> vertNode = CreateNode(portalNode, "v");
    int vertexidx = portal->GetVertexIndices()[vertidx];
    csVector3 vertex = portal->GetVertices()[vertexidx];
    vertNode->SetAttributeAsFloat("x", vertex.x);
    vertNode->SetAttributeAsFloat("y", vertex.y);
    vertNode->SetAttributeAsFloat("z", vertex.z);
  }  

  //Write the Sector tag
  iSector* sector = portal->GetSector();
  if (sector)
  {
    csRef<iDocumentNode> sectorNode = CreateNode(portalNode, "sector");
    const char* name = sector->QueryObject()->GetName();
    if (name && *name)
      sectorNode->CreateNodeBefore(CS_NODE_TEXT)->SetValue(name);
  }

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

    //Add the light radius node
    float radius = light->GetInfluenceRadius();
    csRef<iDocumentNode> radiusNode = CreateNode(lightNode, "radius");
    radiusNode->CreateNodeBefore(CS_NODE_TEXT)->SetValueAsFloat(radius);

    //Add the light color node
    csColor color = light->GetColor();
    csRef<iDocumentNode> colorNode = CreateNode(lightNode, "color");
    colorNode->SetAttributeAsFloat("red", color.red);
    colorNode->SetAttributeAsFloat("green", color.green);
    colorNode->SetAttributeAsFloat("blue", color.blue);

    iBaseHalo* halo = light->GetHalo();

    if (halo)
    {
      csRef<iDocumentNode> haloNode = CreateNode(lightNode, "halo");

      switch (halo->GetType())
      {
        case cshtCross:
        {
          csRef<iCrossHalo> cross = SCF_QUERY_INTERFACE(halo, iCrossHalo);
          csRef<iDocumentNode> typeNode = CreateNode(haloNode, "type");
          typeNode->CreateNodeBefore(CS_NODE_TEXT)->SetValue("cross");

          float intensity = cross->GetIntensityFactor();
          csRef<iDocumentNode>intensityNode = CreateNode(haloNode, "intensity");
          intensityNode->CreateNodeBefore(CS_NODE_TEXT)
            ->SetValueAsFloat(intensity);

          float crossfact = cross->GetCrossFactor();
          csRef<iDocumentNode>crossfactNode = CreateNode(haloNode, "cross");
          crossfactNode->CreateNodeBefore(CS_NODE_TEXT)
            ->SetValueAsFloat(crossfact);

          break;
        }
        case cshtNova:
        {
          csRef<iNovaHalo> nova = SCF_QUERY_INTERFACE(halo, iNovaHalo);
          csRef<iDocumentNode> typeNode = CreateNode(haloNode, "type");
          typeNode->CreateNodeBefore(CS_NODE_TEXT)->SetValue("nova");

          int seed = nova->GetRandomSeed();
          csRef<iDocumentNode>seedNode = CreateNode(haloNode, "seed");
          seedNode->CreateNodeBefore(CS_NODE_TEXT)->SetValueAsInt(seed);

          int numspokes = nova->GetSpokeCount();
          csRef<iDocumentNode>numspokesNode = CreateNode(haloNode, "numspokes");
          numspokesNode->CreateNodeBefore(CS_NODE_TEXT)
            ->SetValueAsInt(numspokes);

          float roundness = nova->GetRoundnessFactor();
          csRef<iDocumentNode>roundnessNode = CreateNode(haloNode, "roundness");
          roundnessNode->CreateNodeBefore(CS_NODE_TEXT)
            ->SetValueAsFloat(roundness);

          break;
        }
        case cshtFlare:
        {
          csRef<iFlareHalo> flare = SCF_QUERY_INTERFACE(halo, iFlareHalo);
          csRef<iDocumentNode> typeNode = CreateNode(haloNode, "type");
          typeNode->CreateNodeBefore(CS_NODE_TEXT)->SetValue("flare");

          //TBD: implement the saving of flare halos

          break;
        }
      }
    }
  }
  return true;
}

bool csSaver::SaveVariables (iDocumentNode* node)
{
  csRef<iDocumentNode> variablesNode = CreateNode(node, "variables");
  iSharedVariableList* vars = engine->GetVariableList();
  for (int x=0; x<vars->GetCount(); x++)
  {
    csRef<iDocumentNode> variableNode = CreateNode(variablesNode, "variable");
    iSharedVariable* var = vars->Get(x);
    variableNode->SetAttribute("name", var->GetName());
    switch (var->GetType())
    {
      case iSharedVariable::SV_FLOAT:
      {
        variableNode->CreateNodeBefore(CS_NODE_TEXT, 0)
          ->SetValueAsFloat(var->Get());
        break;
      }
      case iSharedVariable::SV_COLOR:
      {
        csString value;
        csColor c = var->GetColor();
        value.Format("%f,%f,%f",c.red,c.green,c.blue);
        variableNode->CreateNodeBefore(CS_NODE_TEXT, 0)
          ->SetValue((const char*) value);
        break;
      }
      case iSharedVariable::SV_VECTOR:
      {
        csString value;
        csVector3 v = var->GetVector();
        value.Format("%f,%f,%f",v.x,v.y,v.z);
        variableNode->CreateNodeBefore(CS_NODE_TEXT, 0)
          ->SetValue((const char*) value);
        break;
      }
    }
  }
  return true;
}

bool csSaver::SaveSettings (iDocumentNode* node)
{
  csRef<iDocumentNode> settingsNode = CreateNode(node, "settings");
//case XMLTOKEN_CLEARZBUF:
  //csRef<iDocumentNode> clrzbufNode = CreateNode(settingsNode, "clearzbuf");
  synldr->WriteBool(settingsNode,"clearzbuf",engine->GetClearScreen(), true);
//case XMLTOKEN_CLEARSCREEN:
  //csRef<iDocumentNode> clrscrNode = CreateNode(settingsNode, "clearscreen");
  synldr->WriteBool(settingsNode,"clearscreen",engine->GetClearScreen(), true);
//case XMLTOKEN_LIGHTMAPCELLSIZE:
  csRef<iDocumentNode> lghtmapcellNode = CreateNode(settingsNode, "lightmapcellsize");
  csRef<iPluginManager> plugin_mgr (CS_QUERY_REGISTRY (object_reg,
    iPluginManager));
  csRef<iMeshObjectType> type (CS_QUERY_PLUGIN_CLASS (plugin_mgr,
    "crystalspace.mesh.object.thing", iMeshObjectType));
  if (!type)
  {
    type = CS_LOAD_PLUGIN (plugin_mgr,
      "crystalspace.mesh.object.thing", iMeshObjectType);
  }
  csRef<iThingEnvironment> te = SCF_QUERY_INTERFACE (type,
    iThingEnvironment);
  int cellsize = te->GetLightmapCellSize ();
  lghtmapcellNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsInt(cellsize);
//case XMLTOKEN_MAXLIGHTMAPSIZE:
  csRef<iDocumentNode> maxlghtmapNode = CreateNode(settingsNode, "maxlightmapsize");
  int max[2];
  engine->GetMaxLightmapSize(max[0], max[1]);
  maxlghtmapNode->SetAttributeAsInt ("horizontal", max[0]);
  maxlghtmapNode->SetAttributeAsInt ("vertical"  , max[1]);
//case XMLTOKEN_AMBIENT:
  csRef<iDocumentNode> ambientNode = CreateNode(settingsNode, "ambient");
  csColor c;
  engine->GetAmbientLight(c);
  synldr->WriteColor(ambientNode, &c);

#ifndef CS_USE_OLD_RENDERER
//case XMLTOKEN_RENDERLOOP:
  iRenderLoop* renderloop = engine->GetCurrentDefaultRenderloop();
  const char* loopName = engine->GetRenderLoopManager()->GetName(renderloop);
  CreateNode(settingsNode, "renderloop")
    ->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValue(loopName);

#endif
  return true;
}

bool csSaver::SaveSequence(iDocumentNode *parent)
{
#if 0
  csRef<iEngineSequenceManager> eseqmgr =
    CS_QUERY_REGISTRY (object_reg, iEngineSequenceManager);
  if (!eseqmgr) return false;

  csRef<iDocumentNode> sequencesNode = CreateNode(parent, "sequences");

  for (int i = 0; i < eseqmgr->GetSequenceCount(); i++)
  {
    iSequenceWrapper* sequence = eseqmgr->GetSequence(i);
    csRef<iDocumentNode> sequenceNode = CreateNode(sequencesNode, "sequence");
    const char* name = sequence->QueryObject()->GetName();
    sequenceNode->SetAttribute("name", name);

    iEngineSequenceParameters* params = sequence->GetBaseParameterBlock();
    if (params)
    {
      for (int j = 0; j < params->GetParameterCount(); j++)
      {
        csRef<iDocumentNode> paramNode = CreateNode(sequenceNode, "???");
        const char* paramname = params->GetParameterName(j);
        paramNode->SetAttribute("name", paramname);
        iBase* param = params->GetParameter(j);
        if (!param) continue;
        csRef<iLight> light = SCF_QUERY_INTERFACE(param, iLight);

        if (light)
        {
          paramNode->SetValue("light");
          paramNode->SetAttribute("light", light->QueryObject()->GetName());
        }
      }
    }
  }
#endif
  return true;
}

bool csSaver::SaveTriggers(iDocumentNode *parent)
{
#if 0
  csRef<iEngineSequenceManager> eseqmgr =
    CS_QUERY_REGISTRY (object_reg, iEngineSequenceManager);
  if (!eseqmgr) return false;

  csRef<iDocumentNode> triggersNode = CreateNode(parent, "triggers");

  for (int i = 0; i < eseqmgr->GetTriggerCount(); i++)
  {
    iSequenceTrigger* trigger = eseqmgr->GetTrigger(i);
    csRef<iDocumentNode> triggerNode = CreateNode(triggersNode, "trigger");
    const char* name = trigger->QueryObject()->GetName();
    triggerNode->SetAttribute("name", name);

    iSequenceWrapper* seq = trigger->GetFiredSequence();
    csRef<iDocumentNode> sequenceNode = CreateNode(triggerNode, "fire");
    const char* seqname = seq->QueryObject()->GetName();
    sequenceNode->SetAttribute("name", seqname);

    iEngineSequenceParameters* params = trigger->GetParameters();
    if (params)
    {
      for (int j = 0; j < params->GetParameterCount(); j++)
      {
        csRef<iDocumentNode> paramNode = CreateNode(sequenceNode, "???");
        const char* paramname = params->GetParameterName(j);
        paramNode->SetAttribute("name", paramname);
        iBase* param = params->GetParameter(j);
        csRef<iLight> light = SCF_QUERY_INTERFACE(param, iLight);

        if (light)
        {
          paramNode->SetValue("light");
          paramNode->SetAttribute("light", light->QueryObject()->GetName());
        }
      }
    }
  }
#endif
  return true;
}

csRef<iString> csSaver::SaveMapFile()
{
  csRef<iDocumentSystem> xml = 
    csPtr<iDocumentSystem>(new csTinyDocumentSystem());
  csRef<iDocument> doc = xml->CreateDocument();
  csRef<iDocumentNode> root = doc->CreateRoot();
  csRef<iDocumentNode> parent = root->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  parent->SetValue("world");

  if (!SaveTextures(parent)) return 0;
  if (!SaveVariables(parent)) return 0;
  if (!SaveShaders(parent)) return 0;
  if (!SaveMaterials(parent)) return 0;
  if (!SaveSettings(parent)) return 0;
  if (!SaveRenderPriorities(parent)) return 0;
  if (!SaveMeshFactories(engine->GetMeshFactories(), parent)) return 0;
  if (!SaveSectors(parent)) return 0;
  if (!SaveSequence(parent)) return 0;
  if (!SaveTriggers(parent)) return 0;

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

  if (!SaveTextures(parent)) return 0;
  if (!SaveVariables(parent)) return 0;
  if (!SaveShaders(parent)) return 0;
  if (!SaveMaterials(parent)) return 0;
  if (!SaveSettings(parent)) return 0;
  if (!SaveRenderPriorities(parent)) return 0;
  if (!SaveMeshFactories(engine->GetMeshFactories(), parent)) return 0;
  if (!SaveSectors(parent)) return 0;
  if (!SaveSequence(parent)) return 0;
  if (!SaveTriggers(parent)) return 0;

  return true;
}
