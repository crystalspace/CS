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

#include "csgfx/rgbpixel.h"
#include "cstool/proctex.h"
#include "csutil/objiter.h"
#include "csutil/scfstr.h"
#include "csutil/util.h"
#include "csutil/xmltiny.h"
#include "iengine/campos.h"
#include "iengine/engine.h"
#include "iengine/halo.h"
#include "iengine/material.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "iengine/portal.h"
#include "iengine/portalcontainer.h"
#include "iengine/scenenode.h"
#include "iengine/region.h"
#include "iengine/renderloop.h"
#include "iengine/sector.h"
#include "iengine/sharevar.h"
#include "iengine/texture.h"
#include "imesh/objmodel.h"
#include "igraphic/image.h"
#include "imap/services.h"
#include "imap/writer.h"
#include "imesh/object.h"
#include "imesh/thing.h"
#include "itexture/ifire.h"
#include "itexture/iproctex.h"
#include "itexture/itexfact.h"
#include "iutil/document.h"
#include "iutil/object.h"
#include "iutil/objreg.h"
#include "iutil/string.h"
#include "iutil/vfs.h"
#include "ivaria/engseq.h"
#include "ivaria/keyval.h"
#include "ivaria/reporter.h"
#include "ivaria/sequence.h"
#include "ivideo/graph3d.h"
#include "ivideo/halo.h"
#include "ivideo/material.h"
#include "ivideo/texture.h"
#include "ivideo/txtmgr.h"
#include "plugins/engine/3d/halo.h"
#include "csutil/flags.h"
#include "igeom/polymesh.h"

#include "cssaver.h"

#define ONE_OVER_255 (1.0/255.0)

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY(csSaver)

SCF_IMPLEMENT_IBASE(csSaver);
  SCF_IMPLEMENTS_INTERFACE(iSaver);
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent);
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csSaver::csSaver(iBase *p)
{
  SCF_CONSTRUCT_IBASE(p);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  object_reg = 0;
  curRegion = 0;
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
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);

  if (!engine->GetSaveableFlag())
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_WARNING,
      "crystalspace.plugin.cssaver",
      "'Saveable' flag not set in engine. Saved worlds can be incomplete.");
  }

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

const char* csSaver::GetPluginName (const char* plugin, const char* type)
{
  if (!plugins.In (plugin))
  {
    csString plugintype (plugin);
    size_t pos = plugintype.FindLast ('.');
    csString pluginname = plugintype.Slice (pos+1, plugintype.Length ()-pos-1);
    pluginname += type; // For uniqueness
    plugins.PutUnique (plugintype.GetData (), pluginname.GetData ());
  }
  return plugins.Get (plugin, plugin);
}

bool csSaver::SavePlugins (iDocumentNode* parent)
{
  csHash<csStrKey, csStrKey>::GlobalIterator it = plugins.GetIterator ();
  csRef<iDocumentNode> pluginNode =
    parent->CreateNodeBefore (CS_NODE_ELEMENT, before);
  pluginNode->SetValue ("plugins");
  before = 0;

  csStrKey plugintype;
  while (it.HasNext ())
  {
    csRef<iDocumentNode> plg = CreateNode (pluginNode, "plugin");
    plg->SetAttribute ("name", it.Next (plugintype));
    plg->CreateNodeBefore (CS_NODE_TEXT)->SetValue (plugintype);
  }
  return true;
}

bool csSaver::SaveTextures(iDocumentNode *parent)
{
  csRef<iDocumentNode> current = CreateNode(parent, "textures");
  before = current; // Set up for plugins

  iTextureList* texList=engine->GetTextureList();
  for (int i = 0; i < texList->GetCount(); i++)
  {
    iTextureWrapper *texWrap=texList->Get(i);
    
    if (curRegion && !curRegion->IsInRegion (texWrap->QueryObject ()))
      continue;
    
    iTextureHandle* texHand = texWrap->GetTextureHandle ();
    csRef<iDocumentNode>child = current->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    const char *name = texWrap->QueryObject()->GetName();
    if (name && *name)
      child->SetAttribute("name", name);

    // Save keys...
    SaveKeys (child, texWrap->QueryObject ());

    child->SetValue("texture"); // texture node
    int texTarget = texHand->GetTextureTarget ();
    const char* filename = texHand->GetImageName ();
    // Cubemaps and 3D textures need special "filename logic"
    if ((texTarget <= iTextureHandle::CS_TEX_IMG_2D) && filename && *filename)
    {
      CreateValueNode(child, "file", filename);
    }

    int r,g,b, r2 = -1,g2 = -1,b2 = -1;
    texWrap->GetKeyColor(r, g, b);
    if (r != -1)
    {
      iImage* img = texWrap->GetImageFile();
      if (img && img->HasKeyColor())
      {
        img->GetKeyColor(r2, g2, b2);
      }
      if (r != r2 || g != g2 || b != b2)
      {
        csColor col (r * ONE_OVER_255, g * ONE_OVER_255, b * ONE_OVER_255);
        synldr->WriteColor (CreateNode (child, "transparent"), col);
      }
    }
  
    synldr->WriteBool (child, "for2d", (texWrap->GetFlags () & CS_TEXTURE_2D) != 0, false);
    synldr->WriteBool (child, "for3d", (texWrap->GetFlags () & CS_TEXTURE_3D) != 0, true);
    synldr->WriteBool (child, "mipmap", (texWrap->GetFlags () & CS_TEXTURE_NOMIPMAPS) == 0, true);
    synldr->WriteBool (child, "clamp", (texWrap->GetFlags () & CS_TEXTURE_CLAMP) != 0, false);
    synldr->WriteBool (child, "filter", (texWrap->GetFlags () & CS_TEXTURE_NOFILTER) == 0, true);
    synldr->WriteBool (child, "keepimage", texWrap->KeepImage (), false);

    const char* texClass = texWrap->GetTextureClass();
    if ((texClass != 0) && (strcmp (texClass, "default") != 0))
      CreateValueNode (child, "class", texClass);

    if (texTarget == iTextureHandle::CS_TEX_IMG_CUBEMAP)
    {
      csString imgName (filename);

      if (imgName.FindFirst (':') == (size_t)-1)
      {
	CreateValueNode(child, "file", filename);
      }
      else
      {
	csRef<iDocumentNode> params = 
	  child->CreateNodeBefore (CS_NODE_ELEMENT, 0);
	params->SetValue ("params");
	static const char* faceNames[] = {"east", "west", "top", "bottom", 
	  "north", "south"};
	if (!imgName.IsEmpty())
	{
	  uint face = 0;
	  size_t pos = 0;

	  while ((pos < imgName.Length()) && (face < 6))
	  {
	    size_t colon = imgName.FindFirst (':', pos);
	    size_t subStrLen;
	    if (colon == (size_t)-1)
	      subStrLen = imgName.Length() - pos;
	    else
	      subStrLen = colon - pos;
	    if (subStrLen > 0)
	      CreateValueNode (params, faceNames[face], 
		imgName.Slice (pos, subStrLen));
	    face++;
	    pos += subStrLen + 1;
	  }
	}
      }
    }
    else if (texTarget == iTextureHandle::CS_TEX_IMG_3D)
    {
      csString imgName (filename);

      if (imgName.FindFirst (':') == (size_t)-1)
      {
	CreateValueNode(child, "file", filename);
      }
      else
      {
	csRef<iDocumentNode> params = 
	  child->CreateNodeBefore (CS_NODE_ELEMENT, 0);
	params->SetValue ("params");
	if (!imgName.IsEmpty())
	{
	  size_t pos = 0;

	  while (pos < imgName.Length())
	  {
	    size_t colon = imgName.FindFirst (':', pos);
	    size_t subStrLen;
	    if (colon == (size_t)-1)
	      subStrLen = imgName.Length() - pos;
	    else
	      subStrLen = colon - pos;
	    if (subStrLen > 0)
	      CreateValueNode (params, "layer", imgName.Slice (pos, subStrLen));
	    pos += subStrLen + 1;
	  }
	}
      }
    }
    else
    {
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

      csString loadername (fact->QueryClassID());
      loadername.ReplaceAll (".type.", ".loader.");
      CreateValueNode(child, "type", GetPluginName (loadername, "Tex"));

      csString savername (fact->QueryClassID());
      savername.ReplaceAll (".type.", ".saver.");

      //Invoke the iSaverPlugin::WriteDown
      csRef<iSaverPlugin> saver = csLoadPluginCheck<iSaverPlugin> (
      	plugin_mgr, savername);
      if (saver)
	saver->WriteDown(proctex, child, 0/**ssource*/);

      synldr->WriteBool (child, "alwaysanimate", proctex->GetAlwaysAnimate (), false);
    }
  }
  return true;
}

bool csSaver::SaveMaterials(iDocumentNode *parent)
{
  csRef<iStringSet> stringset =
    CS_QUERY_REGISTRY_TAG_INTERFACE(object_reg, 
    "crystalspace.shared.stringset", iStringSet);

  csStringID texdiffID = stringset->Request("tex diffuse");
  csStringID matdiffID = stringset->Request("mat diffuse");
  csStringID matambiID = stringset->Request("mat ambient");
  csStringID matreflID = stringset->Request("mat reflection");
  csStringID matflatcolID = stringset->Request("mat flatcolor");
  csStringID orlightID = stringset->Request("std_lighting");
  csStringID orcompatID = stringset->Request("standard");

  csRef<iDocumentNode> current = CreateNode(parent, "materials");
  iMaterialList *matList=engine->GetMaterialList();
  for (int i = 0; i < matList->GetCount(); i++)
  {
    iMaterialWrapper* matWrap = matList->Get(i);
    CS_ASSERT(matWrap);
    
    if (curRegion && !curRegion->IsInRegion (matWrap->QueryObject ()))
      continue;
    
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
      csColor col (color.red * ONE_OVER_255, color.green * ONE_OVER_255,
	      color.blue * ONE_OVER_255);
      synldr->WriteColor (CreateNode (child, "color"), col);     
    }

    if(texWrap)
    {
      const char* texname = texWrap->QueryObject()->GetName();
      if (texname && *texname)
        CreateValueNode(child, "texture", texname);
    }

    // Save Keys
    SaveKeys (child, matWrap->QueryObject ());

    float diffuse,ambient,reflection;
    matWrap->GetMaterial ()->GetReflection (diffuse, ambient, reflection);
    if (diffuse && diffuse != CS_DEFMAT_DIFFUSE)
      CreateNode (child, "diffuse")->CreateNodeBefore (CS_NODE_TEXT)->
        SetValueAsFloat (diffuse);
    if (ambient && ambient != CS_DEFMAT_AMBIENT)
      CreateNode (child, "ambient")->CreateNodeBefore (CS_NODE_TEXT)->
        SetValueAsFloat (ambient);
    if (reflection && reflection != CS_DEFMAT_REFLECTION)
      CreateNode (child, "reflection")->CreateNodeBefore (CS_NODE_TEXT)->
        SetValueAsFloat (reflection);

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
  }
  return true;
}

bool csSaver::SaveShaders (iDocumentNode *parent)
{

  csRef<iDocumentNode> shadersNode = CreateNode(parent, "shaders");
  csRef<iShaderManager> shaderMgr = 
    CS_QUERY_REGISTRY (object_reg, iShaderManager);
  if (!shaderMgr) return false;

  csRefArray<iShader> shaders = shaderMgr->GetShaders ();
  size_t i;
  for (i = 0 ; i < shaders.Length () ; i++)
  {
    iShader* shader = shaders[i];
    
    if (curRegion && !curRegion->IsInRegion (shader->QueryObject ()))
      continue;
    
    //const char* shadername = shader->QueryObject()->GetName();
    const char* shaderfile = shader->GetFileName();
    if (shaderfile && *shaderfile)
    {
      csRef<iDocumentNode> shaderNode = CreateNode(shadersNode, "shader");
      CreateNode(shaderNode, "file")
        ->CreateNodeBefore(CS_NODE_TEXT)->SetValue(shaderfile);
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
		
    if (curRegion && !curRegion->IsInRegion (cam->QueryObject ()))
      continue;
    
    csRef<iDocumentNode> n = CreateNode(parent, "start");
    // Set the name attribute if cam pos has a name
    const char *camname = cam->QueryObject()->GetName();
    if(camname && strcmp(camname, "") != 0)
      n->SetAttribute("name", camname);

    // write the sector
    csRef<iDocumentNode> sectornode = CreateNode(n, "sector");
    const char *sectorname = cam->GetSector();
    if(sectorname && *sectorname)
      sectornode->CreateNodeBefore (CS_NODE_TEXT)->SetValue(sectorname);
      
    // write position
    csVector3 pos = cam->GetPosition ();
    synldr->WriteVector (CreateNode (n, "position"), pos);

    // write up vector
    csVector3 up = cam->GetUpwardVector ();
    synldr->WriteVector (CreateNode (n, "up"), up);

    // write forward vector
    csVector3 forward = cam->GetForwardVector ();
    synldr->WriteVector (CreateNode (n, "forward"), forward);

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
                                iDocumentNode *parent, iMeshFactoryWrapper* parentfact)
{
  for (int i=0; i<factList->GetCount(); i++)
  {
    csRef<iMeshFactoryWrapper> meshfactwrap = factList->Get(i);
    if (!parentfact && meshfactwrap->GetParentContainer ())
      continue;
    
    if (curRegion && !curRegion->IsInRegion (meshfactwrap->QueryObject ()))
      continue;
    
    csRef<iMeshObjectFactory>  meshfact = meshfactwrap->GetMeshObjectFactory();

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

    // TBD: Nullmesh

    csRef<iDocumentNode> pluginNode = CreateNode(factNode, "plugin");

    // A special case for bruteblock
    char basepluginname[128] = "";
    csReplaceAll(basepluginname, pluginname, ".terrain.bruteblock", ".terrain", 128);
    
    //Add the plugin tag
    char loadername[128] = "";
    csReplaceAll(loadername, basepluginname, ".object.", ".loader.factory.",128);

    pluginNode->CreateNodeBefore(CS_NODE_TEXT)->SetValue(GetPluginName (loadername, "Fact"));

    char savername[128] = "";
    csReplaceAll(savername, basepluginname, ".object.", ".saver.factory.", 128);

    //Invoke the iSaverPlugin::WriteDown
    csRef<iSaverPlugin> saver = csLoadPluginCheck<iSaverPlugin> (
    	plugin_mgr, savername);
    if (saver) 
      saver->WriteDown(meshfact, factNode, 0/**ssource*/);

    //Save some factory params...
    if (!SaveKeys (factNode, meshfactwrap->QueryObject ())) return false;

    synldr->WriteBool (factNode, "staticshape",
      meshfact->GetFlags ().Check (CS_FACTORY_STATICSHAPE), false);

    csRef<iObjectModel> objmdl = meshfact->GetObjectModel ();
    if (objmdl.IsValid ())
    {
      csRef<iPolygonMesh> poly = objmdl->GetPolygonMeshShadows ();
      if (poly.IsValid ())
      {
        synldr->WriteBool (factNode, "closed",
          poly->GetFlags ().Check (CS_POLYMESH_CLOSED),
          false);
        synldr->WriteBool (factNode, "convex",
          poly->GetFlags ().Check (CS_POLYMESH_CONVEX),
          false);
      }

      //TBD: Polymesh
    }

    const char* pname = engine->GetRenderPriorityName (meshfactwrap->GetRenderPriority ());
    if (pname && *pname)
      CreateNode (factNode, "priority")->CreateNodeBefore (CS_NODE_TEXT)->SetValue (pname);

    csZBufMode zmode = meshfactwrap->GetZBufMode ();
    synldr->WriteZMode (factNode, zmode, false);

    // Save sprite3d specific material...Because it's not handled in sprite3d loader?
      if (meshfact->GetMaterialWrapper ())
        CreateNode (factNode, "material")->CreateNodeBefore (CS_NODE_TEXT)
          ->SetValue (meshfact->GetMaterialWrapper ()->QueryObject ()->GetName ());

    // TBD: Static LOD, LOD
    
    if (parentfact)
    {
      csReversibleTransform rt = meshfactwrap->GetTransform ();
      csRef<iDocumentNode> move = CreateNode (factNode, "move");
      csMatrix3 t2o = rt.GetT2O ();
      csVector3 t2ot = rt.GetT2OTranslation ();
      synldr->WriteMatrix (CreateNode (move, "matrix"), t2o);
      synldr->WriteVector (CreateNode (move, "v"), t2ot);

      // LBD: LOD level
    }

    //Save children
    if (meshfactwrap->GetChildren ()->GetCount ())
      if (!SaveMeshFactories (meshfactwrap->GetChildren (), factNode, meshfactwrap))
        return false;
  }
  return true;
}

bool csSaver::SaveSectors(iDocumentNode *parent)
{
  iSectorList* sectorList=engine->GetSectors();
  for (int i=0; i<sectorList->GetCount(); i++)
  {
    iSector* sector = sectorList->Get(i);
    
    if (curRegion && !curRegion->IsInRegion (sector->QueryObject ()))
      continue;
    
    csRef<iDocumentNode> sectorNode = CreateNode(parent, "sector");
    const char* name = sector->QueryObject()->GetName();
    if (name && *name) sectorNode->SetAttribute("name", name);
    
    iRenderLoop* renderloop = sector->GetRenderLoop ();
    if (renderloop)
    {
      const char* loopName = engine->GetRenderLoopManager()->GetName(renderloop);
      if (strcmp (loopName, CS_DEFAULT_RENDERLOOP_NAME))
        CreateNode(sectorNode, "renderloop")
          ->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValue(loopName);
    }

    csColor ambient = sector->GetDynamicAmbientLight ();
    csColor defaultAmbient;
    engine->GetDefaultAmbientLight (defaultAmbient);
    if (ambient != defaultAmbient)
    {
      synldr->WriteColor (CreateNode (sectorNode, "ambient"), ambient);
    }
    
    // TBD: cullerp, polymesh, node

    if (sector->HasFog ())
    {
      csFog* fog = sector->GetFog ();
      csRef<iDocumentNode> fogNode = CreateNode (sectorNode, "fog");
      fogNode->SetAttributeAsFloat ("red", fog->red);
      fogNode->SetAttributeAsFloat ("green", fog->green);
      fogNode->SetAttributeAsFloat ("blue", fog->blue);
      fogNode->SetAttributeAsFloat ("density", fog->density);
    }
    
    if (!SaveKeys (sectorNode, sector->QueryObject ())) return false;
    if (!SaveSectorMeshes(sector->GetMeshes(), sectorNode)) return false;
    if (!SaveSectorLights(sector, sectorNode)) return false;
    }
  return true;
}

bool csSaver::SaveSectorMeshes(iMeshList* meshList,
		iDocumentNode *parent)
{
  for (int i=0; i<meshList->GetCount (); i++)
  {
    iMeshWrapper* meshwrapper = meshList->Get (i);
    
    if (curRegion && !curRegion->IsInRegion (meshwrapper->QueryObject ()))
      continue;
    
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
      csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.plugin.cssaver", 
	"Factory-less Mesh found! %s => Please fix or report to Jorrit ;)", 
	name);
    }
    if (factory)
    {
      const char* pluginname = factory->QueryClassID();
      if (pluginname && *pluginname)
      {
        csRef<iDocumentNode> pluginNode = CreateNode(meshNode, "plugin");

        // A special case for bruteblock
        char basepluginname[128] = "";
        csReplaceAll(basepluginname, pluginname, ".terrain.bruteblock", ".terrain", 128);
        
        //Add the plugin tag
        char loadername[128] = "";
        csReplaceAll(loadername, basepluginname, ".object.", ".loader.", 128);

        pluginNode->CreateNodeBefore(CS_NODE_TEXT)->SetValue(GetPluginName (loadername, "Mesh"));

        char savername[128] = "";
        csReplaceAll(savername, basepluginname, ".object.", ".saver.", 128);

        //Invoke the iSaverPlugin::WriteDown
        csRef<iSaverPlugin> saver = csLoadPluginCheck<iSaverPlugin> (
		plugin_mgr, savername);
        if (saver)
          saver->WriteDown(meshwrapper->GetMeshObject(), meshNode,
	  	0/**ssource*/);
      }

    }

    csZBufMode zmode = meshwrapper->GetZBufMode ();
    synldr->WriteZMode (meshNode, zmode, false);

    const char* pname = engine->GetRenderPriorityName (meshwrapper->GetRenderPriority ());
    if (pname && *pname)
      CreateNode (meshNode, "priority")->CreateNodeBefore (CS_NODE_TEXT)->SetValue (pname);
    //TBD: write other tags

    //Add the transformation tags
    csVector3 moveVect =
      meshwrapper->GetMovable()->GetTransform().GetO2TTranslation();
    csMatrix3 moveMatrix =
      meshwrapper->GetMovable()->GetTransform().GetO2T();
    if (moveVect != 0 || !moveMatrix.IsIdentity())
    {
      //Add the move tag
      csRef<iDocumentNode> moveNode = CreateNode(meshNode, "move");

      //Add the matrix tag
      if (!moveMatrix.IsIdentity())
      {
        csRef<iDocumentNode> matrixNode = CreateNode(moveNode, "matrix");
        synldr->WriteMatrix(matrixNode, moveMatrix);
      }

      //Add the v tag
      if (moveVect != 0)
      {
        csRef<iDocumentNode> vNode = CreateNode(moveNode, "v");
        synldr->WriteVector(vNode, moveVect);
      }
    }

    //Save all childmeshes
    const csRefArray<iSceneNode>& childlist = meshwrapper->QuerySceneNode ()->
	    GetChildren ();
    if (childlist.Length () > 0) SaveSectorMeshes(childlist, meshNode);
  }
  return true;
}

bool csSaver::SaveSectorMeshes(const csRefArray<iSceneNode>& meshList,
		iDocumentNode *parent)
{
  for (size_t i=0; i<meshList.Length(); i++)
  {
    iMeshWrapper* meshwrapper = meshList[i]->QueryMesh ();
    if (!meshwrapper) continue;
    
    if (curRegion && !curRegion->IsInRegion (meshwrapper->QueryObject ()))
      continue;
    
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
      csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.plugin.cssaver", 
	"Factory-less Mesh found! %s => Please fix or report to Jorrit ;)", 
	name);
    }
    if (factory)
    {
      const char* pluginname = factory->QueryClassID();
      if (pluginname && *pluginname)
      {
        csRef<iDocumentNode> pluginNode = CreateNode(meshNode, "plugin");
        
        // A special case for bruteblock
        char basepluginname[128] = "";
        csReplaceAll(basepluginname, pluginname, ".terrain.bruteblock", ".terrain", 128);
        
        //Add the plugin tag
        char loadername[128] = "";
        csReplaceAll(loadername, basepluginname, ".object.", ".loader.", 128);

        pluginNode->CreateNodeBefore(CS_NODE_TEXT)->SetValue(GetPluginName (loadername, "Mesh"));

        char savername[128] = "";
        csReplaceAll(savername, basepluginname, ".object.", ".saver.", 128);

        //Invoke the iSaverPlugin::WriteDown
        csRef<iSaverPlugin> saver = csLoadPluginCheck<iSaverPlugin> (
          plugin_mgr, savername);
        if (saver)
          saver->WriteDown(meshwrapper->GetMeshObject(), meshNode,
	  	0/**ssource*/);
      }

    }

    csZBufMode zmode = meshwrapper->GetZBufMode ();
    synldr->WriteZMode (meshNode, zmode, false);

    //TBD: write <priority>
    //TBD: write other tags

    //Add the transformation tags
    csVector3 moveVect =
      meshwrapper->GetMovable()->GetTransform().GetO2TTranslation();
    csMatrix3 moveMatrix =
      meshwrapper->GetMovable()->GetTransform().GetO2T();
    if (moveVect != 0 || !moveMatrix.IsIdentity())
    {
      //Add the move tag
      csRef<iDocumentNode> moveNode = CreateNode(meshNode, "move");

      //Add the matrix tag
      if (!moveMatrix.IsIdentity())
      {
        csRef<iDocumentNode> matrixNode = CreateNode(moveNode, "matrix");
        synldr->WriteMatrix(matrixNode, moveMatrix);
      }

      //Add the v tag
      if (moveVect != 0)
      {
        csRef<iDocumentNode> vNode = CreateNode(moveNode, "v");
        synldr->WriteVector(vNode, moveVect);
      }
    }

    //Save all childmeshes
    const csRefArray<iSceneNode>& childlist = meshwrapper->QuerySceneNode ()->
	    GetChildren ();
    if (childlist.Length () > 0) SaveSectorMeshes(childlist, meshNode);
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
    
    if (curRegion && !curRegion->IsInRegion (light->QueryObject ()))
      continue;
    
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
    float radius = light->GetCutoffDistance ();
    csRef<iDocumentNode> radiusNode = CreateNode(lightNode, "radius");
    radiusNode->CreateNodeBefore(CS_NODE_TEXT)->SetValueAsFloat(radius);

    //Add the light color node
    csColor color = light->GetColor();
    csRef<iDocumentNode> colorNode = CreateNode(lightNode, "color");
    colorNode->SetAttributeAsFloat("red", color.red);
    colorNode->SetAttributeAsFloat("green", color.green);
    colorNode->SetAttributeAsFloat("blue", color.blue);

    iBaseHalo* halo = light->GetHalo();

    /// TBD: Make halos work
    if (0&&halo)
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
    iSharedVariable* var = vars->Get(x);
    if (curRegion && !curRegion->IsInRegion (var->QueryObject ()))
      continue;
    
    csRef<iDocumentNode> variableNode = CreateNode(variablesNode, "variable");
    variableNode->SetAttribute("name", var->GetName());
    switch (var->GetType())
    {
      case iSharedVariable::SV_FLOAT:
      {
        variableNode->SetAttributeAsFloat ("value", var->Get ());
        break;
      }
      case iSharedVariable::SV_COLOR:
      {
        csColor c = var->GetColor ();
        synldr->WriteColor (CreateNode (variableNode, "color"), c);
        break;
      }
      case iSharedVariable::SV_VECTOR:
      {
        csVector3 v = var->GetVector ();
        synldr->WriteVector (CreateNode (variableNode, "v"), v);
        break;
      }
      default:
        break;
    }
  }
  return true;
}

bool csSaver::SaveSettings (iDocumentNode* node)
{
  csRef<iDocumentNode> settingsNode = CreateNode(node, "settings");

  synldr->WriteBool(settingsNode,"clearzbuf",engine->GetClearZBuf (),
    engine->GetDefaultClearZBuf ());
  synldr->WriteBool(settingsNode,"clearscreen",engine->GetClearScreen (),
    engine->GetDefaultClearScreen ());  

  csRef<iMeshObjectType> type = csLoadPluginCheck<iMeshObjectType> (
  	object_reg, "crystalspace.mesh.object.thing");
  if (!type) return false;
  csRef<iThingEnvironment> te = SCF_QUERY_INTERFACE (type,
    iThingEnvironment);
  int cellsize = te->GetLightmapCellSize ();
  csRef<iDocumentNode> lghtmapcellNode = CreateNode(settingsNode, "lightmapcellsize");
  lghtmapcellNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsInt(cellsize);

  int max[2];
  engine->GetMaxLightmapSize(max[0], max[1]);
  csRef<iDocumentNode> maxlghtmapNode = CreateNode(settingsNode, "maxlightmapsize");
  maxlghtmapNode->SetAttributeAsInt ("horizontal", max[0]);
  maxlghtmapNode->SetAttributeAsInt ("vertical"  , max[1]);

  csRef<iDocumentNode> ambientNode = CreateNode(settingsNode, "ambient");
  csColor c;
  engine->GetAmbientLight(c);
  synldr->WriteColor(ambientNode, c);

  iRenderLoop* renderloop = engine->GetCurrentDefaultRenderloop();
  const char* loopName = engine->GetRenderLoopManager()->GetName(renderloop);
  if (strcmp (loopName, CS_DEFAULT_RENDERLOOP_NAME))
    CreateNode(settingsNode, "renderloop")
      ->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValue(loopName);

  return true;
}

bool csSaver::SaveSequence(iDocumentNode* /*parent*/)
{
#if 0
  csRef<iEngineSequenceManager> eseqmgr =
    CS_QUERY_REGISTRY (object_reg, iEngineSequenceManager);
  if (!eseqmgr) return false;

  csRef<iDocumentNode> sequencesNode = CreateNode(parent, "sequences");

  for (int i = 0; i < eseqmgr->GetSequenceCount(); i++)
  {
    iSequenceWrapper* sequence = eseqmgr->GetSequence(i);
    
    if (curRegion && !curRegion->IsInRegion (sequence->QueryObject ()))
      continue;
    
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

bool csSaver::SaveTriggers(iDocumentNode* /*parent*/)
{
#if 0
  csRef<iEngineSequenceManager> eseqmgr =
    CS_QUERY_REGISTRY (object_reg, iEngineSequenceManager);
  if (!eseqmgr) return false;

  csRef<iDocumentNode> triggersNode = CreateNode(parent, "triggers");

  for (int i = 0; i < eseqmgr->GetTriggerCount(); i++)
  {
    iSequenceTrigger* trigger = eseqmgr->GetTrigger(i);
    
    if (curRegion && !curRegion->IsInRegion (trigger->QueryObject ()))
      continue;
    
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

bool csSaver::SaveKeys (iDocumentNode* node, iObject* object)
{
  csRef<iObjectIterator> it = object->GetIterator ();
  while (it->HasNext ())
  {
    csRef<iObject> obj = it->Next ();
    
    if (curRegion && !curRegion->IsInRegion (obj))
      continue;
    
    csRef<iKeyValuePair> key = SCF_QUERY_INTERFACE (obj, iKeyValuePair);
    if (key.IsValid ())
    {
      synldr->WriteKey (CreateNode (node, "key"), key);
    }
  }
  return true;
}

bool csSaver::SaveLibraryReferences (iDocumentNode* parent)
{
  if (!curRegion)
    return false;
  
  csRef<iObjectIterator> it = curRegion->QueryObject ()->GetIterator ();
  while (it->HasNext ())
  {
    csRef<iObject> obj = it->Next ();
    
    csRef<iLibraryReference> ref = scfQueryInterface <iLibraryReference> (obj);
    if (ref.IsValid ())
    {
      csRef<iDocumentNode> node = CreateNode (parent, "library");
      
      synldr->WriteBool (node, "checkdupes", ref->GetCheckDupes (), false);
      
      if (ref->GetPath ())
      {
        node->SetAttribute ("file", ref->GetFile ());
        node->SetAttribute ("path", ref->GetPath ());
      }
      else
      {
        node->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValue (ref->GetFile ());
      }
    }
  }
  return true;
}

bool csSaver::SaveAddons (iDocumentNode* parent)
{
  csRef<iObjectRegistryIterator> it = object_reg->Get (
      scfInterfaceTraits<iAddonReference>::GetID (),
      scfInterfaceTraits<iAddonReference>::GetVersion ());
  while (it->HasNext ())
  {
    iBase* obj = it->Next ();
    
    if (!obj)
      continue;
    
    csRef<iAddonReference> addon = scfQueryInterface<iAddonReference> (obj);
    
    if (addon.IsValid ())
    {
      if (curRegion && !curRegion->IsInRegion (addon->QueryObject ()))
        continue;

      if (!addon->GetPlugin ())
        continue;

      csRef<iDocumentNode> node = CreateNode (parent, "addon");
      
      if (addon->GetParamsFile ())
      {
        CreateValueNode(node, "plugin", addon->GetPlugin ()); 
        CreateValueNode(node, "paramsfile", addon->GetParamsFile ());
      }
      else if (addon->GetAddonObject ())
      {
        csRef<iDocumentNode> pluginNode = CreateValueNode(node, "plugin",
          addon->GetPlugin ());

        const char* pluginname = addon->GetPlugin ();
        
        char savername[128] = "";
        csReplaceAll(savername, pluginname, ".loader", ".saver", 128);

        //Invoke the iSaverPlugin::WriteDown
        csRef<iSaverPlugin> saver = csLoadPluginCheck<iSaverPlugin> (plugin_mgr,
		savername);
        if (saver)
          saver->WriteDown(addon->GetAddonObject (), node, 0/**ssource*/);
      }
    }
  }
  return true;
}

csRef<iString> csSaver::SaveMapFile()
{
  csRef<iDocumentSystem> xml = 
    csPtr<iDocumentSystem>(new csTinyDocumentSystem());
  csRef<iDocument> doc = xml->CreateDocument();
  csRef<iDocumentNode> root = doc->CreateRoot();
  
  if (!SaveMapFile (root))
    return 0;

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
  plugins.DeleteAll (); // Clear plugin list
  csRef<iDocumentNode> parent = root->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  parent->SetValue("world");

  curRegion = 0;
  fileType = CS_SAVER_FILE_WORLD;
  
  if (!SaveTextures(parent)) return false;
  if (!SaveVariables(parent)) return false;
  if (!SaveKeys (parent, engine->QueryObject ())) return false;
  if (!SaveShaders(parent)) return false;
  if (!SaveMaterials(parent)) return false;
  if (!SaveSettings(parent)) return false;
  if (!SaveCameraPositions (parent)) return false;
  if (!SaveAddons(parent)) return false;
  if (!SaveMeshFactories(engine->GetMeshFactories(), parent)) return false;
  if (!SaveSectors(parent)) return false;
  if (!SaveSequence(parent)) return false;
  if (!SaveTriggers(parent)) return false;
  if (!SavePlugins(parent)) return false;

  return true;
}

bool csSaver::SaveAllRegions()
{
  // Get list of regions
  iRegionList* regions = engine->GetRegions ();
  for (int i = 0; i < regions->GetCount (); i++)
  {
    iRegion* region = regions->Get (i);
    
    // Get the attached iSaverFile
    iObject* obj = region->QueryObject ()->GetChild (
      scfInterfaceTraits<iSaverFile>::GetID (),
      scfInterfaceTraits<iSaverFile>::GetVersion ());
    csRef<iSaverFile> saverFile = scfQueryInterface<iSaverFile> (obj);
    
    SaveRegionFile (region, saverFile->GetFile (), saverFile->GetFileType ());
  }
  
  return true;
}

csRef<iString> csSaver::SaveRegion(iRegion* region, int filetype)
{
  csRef<iDocumentSystem> xml = 
    csPtr<iDocumentSystem>(new csTinyDocumentSystem());
  csRef<iDocument> doc = xml->CreateDocument();
  csRef<iDocumentNode> root = doc->CreateRoot();
  
  if (!SaveRegion (region, filetype, root))
    return 0;

  iString* str = new scfString();
  if (doc->Write(str) != 0)
  {
    delete str;
    return 0;
  }

  return csPtr<iString>(str);
}

bool csSaver::SaveRegion(iRegion* region, int type, csRef<iDocumentNode>& root)
{
  if (!region)
    return false;
  
  plugins.DeleteAll (); // Clear plugin list
  
  curRegion = region;
  fileType = type;
  
  // @@@: Support saving meshfact and params files
  if (fileType == CS_SAVER_FILE_MESHFACT || fileType == CS_SAVER_FILE_PARAMS)
    return false;
  
  const char* nodeName = 0;
  switch (fileType)
  {
  case CS_SAVER_FILE_WORLD:
    nodeName = "world";
    break;
  case CS_SAVER_FILE_LIBRARY:
    nodeName = "library";
    break;
  case CS_SAVER_FILE_MESHFACT:
    nodeName = "meshfact";
    break;
  case CS_SAVER_FILE_PARAMS:
    nodeName = "params";
    break;
  default:
    nodeName = "world";
    break;
  }
  
  csRef<iDocumentNode> parent = root->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  parent->SetValue(nodeName);
  
  if (!SaveTextures(parent)) return false;
  if (!SaveVariables(parent)) return false;
  if (!SaveKeys (parent, engine->QueryObject ())) return false;
  if (!SaveShaders(parent)) return false;
  if (!SaveMaterials(parent)) return false;
  
  if (fileType == CS_SAVER_FILE_WORLD)
    if (!SaveSettings(parent)) return false;
  
  if (!SaveLibraryReferences(parent)) return false;
  if (!SaveCameraPositions (parent)) return false;
  if (!SaveAddons(parent)) return false;
  if (!SaveMeshFactories(engine->GetMeshFactories(), parent)) return false;
  
  if (fileType == CS_SAVER_FILE_WORLD)
    if (!SaveSectors(parent)) return false;
  
  if (!SaveSequence(parent)) return false;
  if (!SaveTriggers(parent)) return false;
  if (!SavePlugins(parent)) return false;
  
  return true;
}

bool csSaver::SaveRegionFile(iRegion* region, const char* file, int filetype)
{
  csRef<iVFS> vfs(CS_QUERY_REGISTRY(object_reg, iVFS));
  CS_ASSERT(vfs.IsValid());
  
  csRef<iString> str(SaveRegion(region, filetype));
  if (!str)
    return 0;

  return vfs->WriteFile(file, str->GetData(), str->Length());
}

