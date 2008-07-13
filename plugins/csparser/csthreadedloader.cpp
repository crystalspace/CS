/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein
    Copyright (C) 1998-2000 by Ivan Avramovic <ivan@avramovic.com>

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

#include "cstool/saverref.h"
#include "cstool/vfsdirchange.h"

#include "csutil/threadmanager.h"
#include "csutil/xmltiny.h"

#include "iengine/engine.h"
#include "iengine/imposter.h"
#include "iengine/lod.h"
#include "iengine/mesh.h"
#include "iengine/sharevar.h"

#include "igeom/trimesh.h"

#include "imap/saverref.h"
#include "imap/services.h"

#include "imesh/object.h"
#include "imesh/objmodel.h"
#include "imesh/nullmesh.h"

#include "iutil/document.h"
#include "iutil/object.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"

#include "ivaria/keyval.h"

#include "csthreadedloader.h"
#include "csloadercontext.h"

CS_PLUGIN_NAMESPACE_BEGIN(csparser)
{
  SCF_IMPLEMENT_FACTORY(csThreadedLoader)

  csThreadedLoader::csThreadedLoader(iBase *p) : scfImplementationType (this, p)
  {
  }

  csThreadedLoader::~csThreadedLoader()
  {
  }

  bool csThreadedLoader::Initialize(iObjectRegistry *objectreg)
  {
    object_reg = objectreg;
    loaded_plugins.SetObjectRegistry (object_reg);

    csRef<iPluginManager> plugin_mgr = csQueryRegistry<iPluginManager>(object_reg);
    loaded_plugins.plugin_mgr = plugin_mgr;

    Engine = csQueryRegistry<iEngine>(object_reg);
    if(!Engine.IsValid())
    {
      return false;
    }

    vfs = csQueryRegistry<iVFS>(object_reg);
    if(!vfs.IsValid())
    {
      return false;
    }

    SyntaxService = csQueryRegistryOrLoad<iSyntaxService> (object_reg,
  	"crystalspace.syntax.loader.service.text");
    if(!SyntaxService.IsValid())
    {
      return false;
    }

    InitTokenTable(xmltokens);

    stringSet = csQueryRegistryTagInterface<iStringSet>(object_reg,
      "crystalspace.shared.stringset");
    stringSetSvName = csQueryRegistryTagInterface<iShaderVarStringSet>(object_reg,
      "crystalspace.shader.variablenameset");

    textureLock.Initialize();
    materialLock.Initialize();
    meshWrapperLock.Initialize();
    meshFactoryWrapperLock.Initialize();

    return true;
  }

  THREADED_CALLABLE_IMPL10(csThreadedLoader, LoadNode, csRef<iDocumentNode> node,
      csRef<csLoadResult> loadResult, csRef<iCollection> collection, bool searchCollectionOnly,
      bool checkDupes, csRef<iStreamSource> ssource, const char* override_name,
      csRef<iMissingLoaderData> missingdata, uint keepFlags, bool do_verbose)
  {
    loadResult->success = false;
    loadResult->result = 0;

    csRef<iLoaderContext> ldr_context;
    ldr_context.AttachNew(new csLoaderContext(object_reg, Engine, collection,
      searchCollectionOnly, checkDupes, missingdata, keepFlags, do_verbose));

    // Mesh Factory
    csRef<iDocumentNode> meshfactnode = node->GetNode("meshfact");
    if(meshfactnode)
    {
      LoadMeshFactory(ldr_context, loadResult, meshfactnode, override_name, 0, 0, ssource);
      return;
    }
/*
    // Mesh Object
    csRef<iDocumentNode> meshobjnode = node->GetNode ("meshobj");
    if(meshobjnode)
    {
      const char* meshobjname = override_name ? override_name :
        meshobjnode->GetAttributeValue ("name");

      if(ldr_context->CheckDupes())
      {
        iMeshWrapper* mw = Engine->FindMeshObject(meshobjname);
        if(mw)
        {
          ldr_context->AddToCollection(mw->QueryObject());
          loadResult->result = mw;
          loadResult->success = true;
          return;
        }
      }

      csRef<iMeshWrapper> mw = Engine->CreateMeshWrapper (meshobjname);
      if(LoadMeshObject(ldr_context, mw, 0, meshobjnode, ssource))
      {
        ldr_context->AddToCollection(mw->QueryObject());
        loadResult->result = mw;
        loadResult->success = true;
        return;
      }
      else
      {
        // Error is already reported.
        Engine->GetMeshes()->Remove(mw);
        return;
      }
    }

    // World node.
    csRef<iDocumentNode> worldnode = node->GetNode ("world");
    if(worldnode)
    {
      loadResult->success = LoadMap (ldr_context, worldnode, ssource, missingdata);
      return;
    }

    // Library node.
    csRef<iDocumentNode> libnode = node->GetNode ("library");
    if (libnode)
    {
      loadResult->result = 0;
      loadResult->success = LoadLibrary (ldr_context, libnode, ssource, missingdata,	true);
      return;
    }

    // Portals.
    csRef<iDocumentNode> portalsnode = node->GetNode ("portals");
    if (portalsnode)
    {
      const char* portalsname = override_name ? override_name :
        portalsnode->GetAttributeValue ("name");
      if (ldr_context->CheckDupes () && portalsname)
      {
        iMeshWrapper* mw = Engine->FindMeshObject (portalsname);
        if (mw)
        {
          csRef<iPortalContainer> pc = 
            scfQueryInterface<iPortalContainer>(mw->GetMeshObject());
          if (pc)
          {
            ldr_context->AddToCollection(mw->QueryObject());
            loadResult->result = mw;
            loadResult->success = true;
            return;
          }
        }
      }
      if (ParsePortals (ldr_context, portalsnode, 0, 0, ssource))
      {
        iMeshWrapper* mw = 0;
        if (ldr_context->GetCollection())
          mw = ldr_context->GetCollection()->FindMeshObject(portalsname);

        if (mw)
        {
          mw->QueryObject()->SetName(portalsname);
          loadResult->result = mw;
          loadResult->success = true;
          return;
        }
      }

      loadResult->result = 0;
      loadResult->success = false;
      return;
    }

    // Light.
    csRef<iDocumentNode> lightnode = node->GetNode ("light");
    if (lightnode)
    {
      const char* lightname = override_name ? override_name :
        lightnode->GetAttributeValue ("name");
      iLight* light = ParseStatlight (ldr_context, lightnode);
      if (light)
      {
        light->QueryObject()->SetName(lightname);
        loadResult->result = light;
        loadResult->success = true;
        return;
      }

      loadResult->result = 0;
      loadResult->success = false;
      return;
    }

    // MeshRef
    csRef<iDocumentNode> meshrefnode = node->GetNode ("meshref");
    if (meshrefnode)
    {
      const char* meshobjname = override_name ? override_name :
        meshrefnode->GetAttributeValue ("name");
      if (ldr_context->CheckDupes () && meshobjname)
      {
        iMeshWrapper* mw = Engine->FindMeshObject (meshobjname);
        if (mw)
        {
          ldr_context->AddToCollection(mw->QueryObject());
          loadResult->result = mw;
          loadResult->success = true;
          return;
        }
      }
      csRef<iMeshWrapper> mesh = LoadMeshObjectFromFactory (ldr_context, meshrefnode, ssource);
      if (mesh)
      {
        ldr_context->AddToCollection(mesh->QueryObject());
        loadResult->result = mesh;
        loadResult->success = true;
        return;
      }
      else
      {
        // Error is already reported.
        loadResult->result = 0;
        loadResult->success = false;
        return;
      }
    }
*/
    ReportError("crystalspace.maploader.parse",
      "File doesn't seem to be a world, library, meshfact, meshobj, meshref, portals or light file!");
  }

  iMeshFactoryWrapper* csThreadedLoader::LoadMeshFactory(iLoaderContext* ldr_context,
    csLoadResult* loadResult, iDocumentNode* meshfactnode, const char* override_name,
    iMeshFactoryWrapper* parent, csReversibleTransform* transf, iStreamSource* ssource)
  {
    const char* meshfactname = override_name ? override_name : meshfactnode->GetAttributeValue("name");

    meshFactoryWrapperLock.Lock();

    if(ldr_context->CheckDupes())
    {
      iMeshFactoryWrapper* mfw = Engine->FindMeshFactory(meshfactname);
      if(mfw)
      {
        meshFactoryWrapperLock.Unlock();
        ldr_context->AddToCollection(mfw->QueryObject());
        loadResult->result = mfw;
        loadResult->success = true;
        return mfw;
      }
    }

    csRef<iMeshFactoryWrapper> mfw = Engine->CreateMeshFactory(meshfactname);
    meshFactoryWrapperLock.Unlock();

    if(LoadMeshObjectFactory(ldr_context, mfw, loadResult, parent, meshfactnode, transf, ssource))
    {
      ldr_context->AddToCollection(mfw->QueryObject());
      loadResult->result = mfw;
      loadResult->success = true;
      return mfw;
    }
    else
    {
      // Error is already reported.
      meshFactoryWrapperLock.Lock();
      Engine->GetMeshFactories()->Remove(mfw);
      meshFactoryWrapperLock.Unlock();
      return NULL;
    }
  }

  bool csThreadedLoader::LoadMeshObjectFactory (iLoaderContext* ldr_context,
    iMeshFactoryWrapper* stemp, csLoadResult* loadResult, iMeshFactoryWrapper* parent,
    iDocumentNode* node, csReversibleTransform* transf, iStreamSource* ssource)
  {
    iLoaderPlugin* plug = 0;
    iBinaryLoaderPlugin* binplug = 0;
    iMaterialWrapper *mat = 0;
    bool staticshape = false;
    csRef<iDocumentNodeIterator> prev_it;
    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (true)
    {
      if (!it->HasNext ())
      {
        // Iterator has finished. Check if we still have to continue
        // with the normal iterator first (non-defaults).
        if (!prev_it) break;
        it = prev_it;
        prev_it = 0;
        continue;
      }

      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
      case XMLTOKEN_LOD:
        {
          if (!stemp->GetMeshObjectFactory ())
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshfactory",
              child, "Please use 'params' before specifying LOD!");
            return false;
          }
          csRef<iLODControl> lodctrl (scfQueryInterface<iLODControl> (
            stemp->GetMeshObjectFactory ()));
          if (!lodctrl)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshfactory",
              child, "This mesh factory doesn't implement LOD control!");
            return false;
          }
          if (!LoadLodControl (lodctrl, child))
            return false;
        }
        break;
      case XMLTOKEN_KEY:
        {
          if (!ParseKey (child, stemp->QueryObject()))
            return false;
        }
        break;
      case XMLTOKEN_ADDON:
        if (!LoadAddOn (ldr_context, child, stemp, false, ssource))
          return false;
        break;
      case XMLTOKEN_META:
        if (!LoadAddOn (ldr_context, child, stemp, true, ssource))
          return false;
        break;
      case XMLTOKEN_LODLEVEL:
        {
          if (!parent)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.load.meshfactory", child,
              "Factory must be part of a hierarchy for <lodlevel>!");
            return false;
          }
          parent->AddFactoryToStaticLOD (child->GetContentsValueAsInt (),
            stemp);
        }
        break;
      case XMLTOKEN_STATICLOD:
        {
          iLODControl* lodctrl = stemp->CreateStaticLOD ();
          if (!LoadLodControl (lodctrl, child))
            return false;
        }
        break;
      case XMLTOKEN_STATICSHAPE:
        if (!SyntaxService->ParseBool (child, staticshape, true))
          return false;
        break;
      case XMLTOKEN_NULLMESH:
        {
          if (plug)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.load.plugin",
              child, "Don't specify the plugin if you use <nullmesh>!");
            return false;
          }
          if (stemp->GetMeshObjectFactory ())
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshfactory", child,
              "Please don't use <params> in combination with <nullmesh>!");
            return false;
          }
          csRef<iMeshObjectType> type = csLoadPluginCheck<iMeshObjectType> (
            object_reg, "crystalspace.mesh.object.null", false);
          if (!type)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.load.plugin",
              child, "Could not find the nullmesh plugin!");
            return false;
          }
          csRef<iMeshObjectFactory> fact = type->NewFactory ();
          stemp->SetMeshObjectFactory (fact);
          fact->SetMeshFactoryWrapper (stemp);
          csBox3 b;
          if (!SyntaxService->ParseBox (child, b))
            return false;
          csRef<iNullFactoryState> nullmesh = 
            scfQueryInterface<iNullFactoryState> (fact);
          nullmesh->SetBoundingBox (b);
        }
        break;
      case XMLTOKEN_PARAMS:
        if (!plug)
        {
          SyntaxService->ReportError (
            "crystalspace.maploader.load.plugin",
            child, "Could not load plugin!");
          return false;
        }
        else
        {
          // We give here the iMeshObjectFactory as the context. If this
          // is a new factory this will be 0. Otherwise it is possible
          // to append information to the already loaded factory.
          csRef<iBase> mof = plug->Parse (child, ssource, ldr_context,
            stemp->GetMeshObjectFactory ());
          if (!mof)
          {
            // Error is reported by plug->Parse().
            return false;
          }
          else
          {
            csRef<iMeshObjectFactory> mof2 (
              scfQueryInterface<iMeshObjectFactory> (mof));
            if (!mof2)
            {
              SyntaxService->ReportError (
                "crystalspace.maploader.parse.meshfactory",
                child,
                "Returned object does not implement iMeshObjectFactory!");
              return false;
            }
            stemp->SetMeshObjectFactory (mof2);
            mof2->SetMeshFactoryWrapper (stemp);
          }
        }
        break;
      case XMLTOKEN_PARAMSFILE:
        if (!plug && !binplug)
        {
          SyntaxService->ReportError (
            "crystalspace.maploader.load.plugin",
            child, "Could not load plugin!");
          return false;
        }
        else
        {
          csRef<iFile> buf (vfs->Open (child->GetContentsValue (),
            VFS_FILE_READ));
          if (!buf)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.loadingfile",
              child, "Error opening file '%s'!", child->GetContentsValue ());
            return false;
          }
          // We give here the iMeshObjectFactory as the context. If this
          // is a new factory this will be 0. Otherwise it is possible
          // to append information to the already loaded factory.
          csRef<iBase> mof;
          if (plug)
            mof = LoadStructuredMap (ldr_context,
            plug, buf, stemp->GetMeshObjectFactory (),
            child->GetContentsValue (), ssource);
          else
          {
            csRef<iDataBuffer> dbuf = vfs->ReadFile (
              child->GetContentsValue ());
            mof = binplug->Parse (dbuf,
              ssource, ldr_context, stemp->GetMeshObjectFactory ());
          }
          if (!mof)
          {
            // Error is reported by plug->Parse().
            return false;
          }
          else
          {
            csRef<iMeshObjectFactory> mof2 (
              scfQueryInterface<iMeshObjectFactory> (mof));
            if (!mof2)
            {
              SyntaxService->ReportError (
                "crystalspace.maploader.parse.meshfactory",
                child,
                "Returned object does not implement iMeshObjectFactory!");
              return false;
            }
            stemp->SetMeshObjectFactory (mof2);
            mof2->SetMeshFactoryWrapper (stemp);
          }
        }
        break;

      case XMLTOKEN_TRIMESH:
        {
          if (!stemp->GetMeshObjectFactory ())
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshfactory",
              child, "Please use 'params' before specifying 'trimesh'!");
            return false;
          }
          iObjectModel* objmodel = stemp->GetMeshObjectFactory ()
            ->GetObjectModel ();
          if (!objmodel)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshfactory", child,
              "This factory doesn't support setting of other 'trimesh'!");
            return false;
          }
          if (!ParseTriMesh (child, objmodel))
          {
            // Error already reported.
            return false;
          }
        }
        break;

      case XMLTOKEN_CLOSED:
        if (!stemp->GetMeshObjectFactory ())
        {
          SyntaxService->ReportError (
            "crystalspace.maploader.parse.meshfactory",
            child, "Please use 'params' before specifying 'closed'!");
          return false;
        }
        else
        {
          iObjectModel* objmodel = stemp->GetMeshObjectFactory ()
            ->GetObjectModel ();
          csRef<iTriangleMeshIterator> it = objmodel->GetTriangleDataIterator ();
          while (it->HasNext ())
          {
            csStringID id;
            iTriangleMesh* trimesh = it->Next (id);
            if (trimesh) trimesh->GetFlags ().Set (
              CS_TRIMESH_CLOSED | CS_TRIMESH_NOTCLOSED, CS_TRIMESH_CLOSED);
          }
        }
        break;
      case XMLTOKEN_CONVEX:
        if (!stemp->GetMeshObjectFactory ())
        {
          SyntaxService->ReportError (
            "crystalspace.maploader.parse.meshfactory",
            child, "Please use 'params' before specifying 'convex'!");
          return false;
        }
        else
        {
          iObjectModel* objmodel = stemp->GetMeshObjectFactory ()
            ->GetObjectModel ();
          csRef<iTriangleMeshIterator> it = objmodel->GetTriangleDataIterator ();
          while (it->HasNext ())
          {
            csStringID id;
            iTriangleMesh* trimesh = it->Next (id);
            if (trimesh) trimesh->GetFlags ().Set (
              CS_TRIMESH_CONVEX | CS_TRIMESH_NOTCONVEX, CS_TRIMESH_CONVEX);
          }
        }
        break;
      case XMLTOKEN_MATERIAL:
        {
          if (!stemp->GetMeshObjectFactory ())
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshfactory",
              child, "Please use 'params' before specifying 'material'!");
            return false;
          }
          const char* matname = child->GetContentsValue ();
          mat = ldr_context->FindMaterial (matname);
          if (mat)
          {
            if (!stemp->GetMeshObjectFactory ()->SetMaterialWrapper (mat))
            {
              SyntaxService->ReportError (
                "crystalspace.maploader.parse.meshfactory",
                child, "This factory doesn't support setting materials this way!");
              return false;
            }
          }
          else
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.unknownmaterial",
              child, "Material '%s' not found!", matname);
            return false;
          }
        }
        break;

      case XMLTOKEN_PLUGIN:
        {
          if (prev_it || plug || binplug)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshfact",
              child, "Please specify only one plugin!");
            return false;
          }

          iDocumentNode* defaults = 0;
          if (!loaded_plugins.FindPlugin (child->GetContentsValue (),
            plug, binplug, defaults))
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshfact",
              child, "Error loading plugin '%s'!",
              child->GetContentsValue ());
            return false;
          }
          if (defaults)
          {
            // Set aside current iterator and start a new one.
            prev_it = it;
            it = defaults->GetNodes ();
          }
        }
        break;

      case XMLTOKEN_MESHFACT:
        {
          csReversibleTransform child_transf;
          iMeshFactoryWrapper* mfw = LoadMeshFactory(ldr_context, loadResult, child, 0, stemp, &child_transf, ssource);
          if(!mfw)
          {
            return false;
          }          
          stemp->GetChildren()->Add(mfw);
          mfw->SetTransform(child_transf);
        }
        break;

      case XMLTOKEN_MOVE:
        {
          if (!transf)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.load.meshfactory",
              child,
              "'move' is only useful for hierarchical transformations!");
            return false;
          }
          csRef<iDocumentNode> matrix_node = child->GetNode ("matrix");
          if (matrix_node)
          {
            csMatrix3 m;
            if (!SyntaxService->ParseMatrix (matrix_node, m))
              return false;
            transf->SetO2T (m);
          }
          csRef<iDocumentNode> vector_node = child->GetNode ("v");
          if (vector_node)
          {
            csVector3 v;
            if (!SyntaxService->ParseVector (vector_node, v))
              return false;
            transf->SetO2TTranslation (v);
          }
        }
        break;
      case XMLTOKEN_HARDMOVE:
        {
          if (!stemp->GetMeshObjectFactory ())
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshfactory",
              child, "Please use 'params' before specifying 'hardmove'!");
            return false;
          }
          if (!stemp->GetMeshObjectFactory ()->SupportsHardTransform ())
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshfactory",
              child, "This factory doesn't support 'hardmove'!");
            return false;
          }
          csReversibleTransform tr;
          csRef<iDocumentNode> matrix_node = child->GetNode ("matrix");
          if (matrix_node)
          {
            csMatrix3 m;
            if (!SyntaxService->ParseMatrix (matrix_node, m))
              return false;
            tr.SetT2O (m);
          }
          csRef<iDocumentNode> vector_node = child->GetNode ("v");
          if (vector_node)
          {
            csVector3 v;
            if (!SyntaxService->ParseVector (vector_node, v))
              return false;
            tr.SetOrigin (v);
          }
          stemp->HardTransform (tr);
        }
        break;

      case XMLTOKEN_ZUSE:
        stemp->SetZBufMode (CS_ZBUF_USE);
        break;
      case XMLTOKEN_ZFILL:
        stemp->SetZBufMode (CS_ZBUF_FILL);
        break;
      case XMLTOKEN_ZNONE:
        stemp->SetZBufMode (CS_ZBUF_NONE);
        break;
      case XMLTOKEN_ZTEST:
        stemp->SetZBufMode (CS_ZBUF_TEST);
        break;
      case XMLTOKEN_PRIORITY:
        {
          const char* priname = child->GetContentsValue ();
          long pri = Engine->GetRenderPriority (priname);
          if (pri == 0)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshfactory",
              child, "Unknown render priority '%s'!", priname);
            return false;
          }
          stemp->SetRenderPriority (pri);
        }
        break;
      case XMLTOKEN_SHADERVAR:
        {
          if (!stemp->GetMeshObjectFactory ())
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshfactory",
              child, "Please use 'params' before specifying 'shadervar'!");
            return false;
          }
          csRef<iShaderVariableContext> svc = stemp->GetSVContext();
          CS_ASSERT (svc.IsValid());
          //create a new variable
          csRef<csShaderVariable> var;
          var.AttachNew (new csShaderVariable);
          if (!SyntaxService->ParseShaderVar (ldr_context, child, *var))
          {
            break;
          }
          svc->AddVariable (var);
        }
        break;
      case XMLTOKEN_NOLIGHTING:
        stemp->GetFlags ().Set (CS_ENTITY_NOLIGHTING, CS_ENTITY_NOLIGHTING);
        break;
      case XMLTOKEN_NOSHADOWS:
        stemp->GetFlags ().Set (CS_ENTITY_NOSHADOWS, CS_ENTITY_NOSHADOWS);
        break;
      case XMLTOKEN_NOSHADOWCAST:
        stemp->GetFlags ().Set (CS_ENTITY_NOSHADOWCAST, CS_ENTITY_NOSHADOWCAST);
        break;
      case XMLTOKEN_NOSHADOWRECEIVE:
        stemp->GetFlags ().Set (CS_ENTITY_NOSHADOWRECEIVE, CS_ENTITY_NOSHADOWRECEIVE);
        break;
      case XMLTOKEN_NOCLIP:
        stemp->GetFlags ().Set (CS_ENTITY_NOCLIP, CS_ENTITY_NOCLIP);
        break;
      case XMLTOKEN_NOHITBEAM:
        stemp->GetFlags ().Set (CS_ENTITY_NOHITBEAM, CS_ENTITY_NOHITBEAM);
        break;
      case XMLTOKEN_INVISIBLEMESH:
        stemp->GetFlags ().Set (CS_ENTITY_INVISIBLEMESH,
          CS_ENTITY_INVISIBLEMESH);
        break;
      case XMLTOKEN_INVISIBLE:
        stemp->GetFlags ().Set (CS_ENTITY_INVISIBLE, CS_ENTITY_INVISIBLE);
        break;
      case XMLTOKEN_DETAIL:
        stemp->GetFlags ().Set (CS_ENTITY_DETAIL, CS_ENTITY_DETAIL);
        break;
      case XMLTOKEN_STATICLIT:
        stemp->GetFlags ().Set (CS_ENTITY_STATICLIT, CS_ENTITY_STATICLIT);
        break;
      case XMLTOKEN_LIMITEDSHADOWCAST:
        stemp->GetFlags ().Set (CS_ENTITY_LIMITEDSHADOWCAST,
          CS_ENTITY_LIMITEDSHADOWCAST);
        break;
      case XMLTOKEN_IMPOSTER:
        {
          csRef<iImposter> imposter = scfQueryInterface<iImposter> (stemp);
          if (!imposter)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshfactory",
              node, "This factory doesn't implement impostering!");
            return false;
          }
          if (!ParseImposterSettings (imposter, child))
            return false;
        }
        break;
      default:
        SyntaxService->ReportBadToken (child);
        return false;
      }
    }

    stemp->GetMeshObjectFactory ()->GetFlags ().SetBool (CS_FACTORY_STATICSHAPE,
      staticshape);

    return true;
  }

  bool csThreadedLoader::LoadLodControl(iLODControl* lodctrl, iDocumentNode* node)
  {
    lodctrl->SetLOD (0, 1);
    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
      case XMLTOKEN_DISTANCE:
        {
          csRef<iDocumentAttribute> at = child->GetAttribute ("varm");
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
      case XMLTOKEN_FADE:
        {
          csRef<iDocumentAttribute> at = child->GetAttribute ("varf");
          if (at)
          {
            // We use variables.
            iSharedVariable *varf = Engine->GetVariableList()->FindByName (
              child->GetAttributeValue ("varf"));
            lodctrl->SetLODFade (varf);
            break;
          }

          at = child->GetAttribute ("f");
          if (at)
          {
            float lodf = child->GetAttributeValueAsFloat ("f");
            lodctrl->SetLODFade (lodf);
          }
          else
          {
            float d = child->GetAttributeValueAsFloat ("d");
            float lodm, loda;
            lodctrl->GetLOD (lodm, loda);
            float d0 = loda/-lodm;
            float d1 = 1.0/lodm + d0;
            float lodf = (d1-d0)/(2*d);
            lodctrl->SetLODFade (lodf);
          }
        }
        break;
      default:
        SyntaxService->ReportBadToken (child);
        return false;
    }
  }

    return true;
  }

  bool csThreadedLoader::LoadAddOn (iLoaderContext* ldr_context,
    iDocumentNode* node, iBase* context, bool is_meta, iStreamSource* ssource)
  {
    iLoaderPlugin* plug = 0;
    iBinaryLoaderPlugin* binplug = 0;

    const char* plugin_name = node->GetAttributeValue ("plugin");
    if (plugin_name != 0)
    {
      // Short-hand notation: <addon plugin="bla"> ... </addon>
      iDocumentNode* defaults = 0;
      if (!loaded_plugins.FindPlugin (plugin_name, plug, binplug, defaults))
      {
        if (!is_meta)
          ReportWarning (
          "crystalspace.maploader.parse.addon",
          node, "Couldn't find or load addon plugin '%s'!",
          plugin_name);
        return true;
      }
      if (!plug)
      {
        if (!is_meta)
          ReportWarning (
          "crystalspace.maploader.load.plugin",
          node, "Could not find or load addon plugin!");
        return true;
      }
      if (defaults != 0)
      {
        ReportWarning (
          "crystalspace.maploader.load.plugin",
          node, "'defaults' section is ignored for addons!");
      }
      csRef<iBase> rc = plug->Parse (node, ssource, ldr_context, context);
      if (!rc) return false;

      if (Engine->GetSaveableFlag ())
      {
        csRef<iAddonReference> addon;
        addon.AttachNew (new csAddonReference (plugin_name, 0, rc));
        object_reg->Register (addon);
        ldr_context->AddToCollection(addon->QueryObject ());
      }

      return true;
    }
    else
    {
      // Long notation: <addon> <plugin>bla</plugin> <params>...</params> </addon>
      csRef<iDocumentNodeIterator> it = node->GetNodes ();
      while (it->HasNext ())
      {
        csRef<iDocumentNode> child = it->Next ();
        if (child->GetType () != CS_NODE_ELEMENT) continue;
        const char* value = child->GetValue ();
        csStringID id = xmltokens.Request (value);
        switch (id)
        {
        case XMLTOKEN_PARAMS:
          if (!plug)
          {
            if (!is_meta)
              ReportWarning (
              "crystalspace.maploader.load.plugin",
              child, "Could not find or load plugin!");
            return true;
          }
          else
          {
            csRef<iBase> rc = plug->Parse (child, ssource, ldr_context,
              context);

            if (!rc) return false;

            if (Engine->GetSaveableFlag ())
            {
              csRef<iAddonReference> addon;
              addon.AttachNew (new csAddonReference (plugin_name, 0, rc));
              object_reg->Register (addon);
              ldr_context->AddToCollection(addon->QueryObject());
            }
          }
          break;

        case XMLTOKEN_PARAMSFILE:
          if (!plug && !binplug)
          {
            if (!is_meta)
              ReportWarning (
              "crystalspace.maploader.load.plugin",
              child, "Could not find or load plugin!");
            return true;
          }
          else
          {
            const char* fname = child->GetContentsValue ();
            if (!fname)
            {
              SyntaxService->ReportError (
                "crystalspace.maploader.parse.loadingfile",
                child, "Specify a VFS filename with 'paramsfile'!");
              return false;
            }
            csRef<iFile> buf (vfs->Open (fname, VFS_FILE_READ));
            if (!buf)
            {
              SyntaxService->ReportError (
                "crystalspace.maploader.parse.loadingfile",
                child, "Error opening file '%s'!", fname);
              return false;
            }
            bool rc;
            csRef<iBase> ret;
            if (plug)
            {
              ret = LoadStructuredMap (ldr_context,
                plug, buf, 0, fname, ssource);
              rc = (ret != 0);
            }
            else
            {
              csRef<iDataBuffer> dbuf = vfs->ReadFile (fname);
              ret = binplug->Parse (dbuf,
                ssource, ldr_context, 0);
              rc = (ret != 0);
            }

            if (!rc)
              return false;

            if (Engine->GetSaveableFlag ())
            {
              csRef<iAddonReference> addon;
              addon.AttachNew (new csAddonReference (plugin_name,
                fname, ret));
              object_reg->Register (addon);
              ldr_context->AddToCollection(addon->QueryObject());
            }
          }
          break;

        case XMLTOKEN_PLUGIN:
          {
            iDocumentNode* defaults = 0;
            plugin_name = child->GetContentsValue ();
            if (!loaded_plugins.FindPlugin (plugin_name,
              plug, binplug, defaults))
            {
              if (!is_meta)
                ReportWarning (
                "crystalspace.maploader.parse.addon",
                child, "Could not find or load plugin '%s'!",
                child->GetContentsValue ());
              return true;
            }
            if (defaults != 0)
            {
              ReportWarning (
                "crystalspace.maploader.parse.addon",
                child, "'defaults' section is ignored for addons!");
            }
          }
          break;
        default:
          SyntaxService->ReportBadToken (child);
          return false;
        }
      }
    }
    return true;
  }

  bool csThreadedLoader::ParseKey (iDocumentNode* node, iObject* obj)
  {
    csRef<iKeyValuePair> kvp = SyntaxService->ParseKey (node);
    if (!kvp.IsValid())
      return false;

    bool editoronly = node->GetAttributeValueAsBool ("editoronly");
    if (!editoronly || !Engine || Engine->GetSaveableFlag())
      obj->ObjAdd (kvp->QueryObject ());

    return true;
  }

  csPtr<iBase> csThreadedLoader::LoadStructuredMap (iLoaderContext* ldr_context,
    iLoaderPlugin* plug, iFile* buf,
    iBase* context, const char* fname, iStreamSource* ssource)
  {
    csRef<iDocument> doc;
    csString filename (fname);
    csVfsDirectoryChanger dirChanger (vfs);
    size_t slashPos = filename.FindLast ('/');
    if (slashPos != (size_t)-1)
    {
      dirChanger.ChangeTo (filename);
      filename.DeleteAt (0, slashPos + 1);
    }
    bool er = LoadStructuredDoc (filename, buf, doc);
    csRef<iBase> ret;
    if (er)
    {
      if (doc)
      {
        // First find the <params> node in the loaded file.
        csRef<iDocumentNode> paramsnode = doc->GetRoot ()->GetNode ("params");
        if (!paramsnode)
        {
          SyntaxService->ReportError (
            "crystalspace.maploader.load.plugin",
            doc->GetRoot (), "Could not find <params> in '%s'!", fname);
        }
        else
        {
          ret = plug->Parse (paramsnode, ssource, ldr_context, context);
        }
      }
      else
      {
        ReportError ("crystalspace.maploader.load.plugin",
          "File does not appear to be a structured map file (%s)!", fname);
      }
    }
    return csPtr<iBase> (ret);
  }

  bool csThreadedLoader::LoadStructuredDoc (const char* file, iFile* buf,
    csRef<iDocument>& doc)
  {
    csRef<iDocumentSystem> docsys (
      csQueryRegistry<iDocumentSystem> (object_reg));
    if (!docsys) docsys = csPtr<iDocumentSystem> (new csTinyDocumentSystem ());
    doc = docsys->CreateDocument ();
    const char* error = doc->Parse (buf, true);
    if (error != 0)
    {
      ReportError (
        "crystalspace.maploader.parse.plugin",
        "Document system error for file '%s': %s!", file, error);
      doc = 0;
      return false;
    }
    return true;
  }

  bool csThreadedLoader::LoadStructuredDoc (const char* file, iDataBuffer* buf,
    csRef<iDocument>& doc)
  {
    csRef<iDocumentSystem> docsys (
      csQueryRegistry<iDocumentSystem> (object_reg));
    if (!docsys) docsys = csPtr<iDocumentSystem> (new csTinyDocumentSystem ());
    doc = docsys->CreateDocument ();
    const char* error = doc->Parse (buf, true);
    if (error != 0)
    {
      ReportError (
        "crystalspace.maploader.parse.plugin",
        file
        ? "Document system error for file '%s': %s!"
        : "Document system error for buffer%s: %s!",
        file ? file : "", error);
      doc = 0;
      return false;
    }
    return true;
  }

  //-------------------------------------------------------------------------//

  void csThreadedLoader::ReportError (const char* id, const char* description, ...)
  {
    va_list arg;
    va_start (arg, description);
    csReportV (object_reg, CS_REPORTER_SEVERITY_ERROR, id, description, arg);
    va_end (arg);
  }

  void csThreadedLoader::ReportNotify (const char* description, ...)
  {
    va_list arg;
    va_start (arg, description);
    ReportNotifyV ("crystalspace.maploader", description, arg);
    va_end (arg);
  }

  void csThreadedLoader::ReportNotifyV (const char* id, const char* description, va_list arg)
  {
    csReportV (object_reg, CS_REPORTER_SEVERITY_NOTIFY, id, description, arg);
  }

  void csThreadedLoader::ReportNotify2 (const char* id, const char* description, ...)
  {
    va_list arg;
    va_start (arg, description);
    ReportNotifyV (id, description, arg);
    va_end (arg);
  }

  void csThreadedLoader::ReportWarning (const char* id, const char* description, ...)
  {
    va_list arg;
    va_start (arg, description);
    csReportV (object_reg, CS_REPORTER_SEVERITY_WARNING, id, description, arg);
    va_end (arg);
  }

  void csThreadedLoader::ReportWarning (const char* id, iDocumentNode* node, const char* description, ...)
  {
    va_list arg;
    va_start (arg, description);
    csString buf;
    buf.FormatV (description, arg);
    va_end (arg);
    SyntaxService->Report (id, CS_REPORTER_SEVERITY_WARNING, node, "%s", 
      buf.GetData());
  }
}
CS_PLUGIN_NAMESPACE_END(csparser)
