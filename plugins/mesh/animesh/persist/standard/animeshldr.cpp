/*
  Copyright (C) 2008 by Marten Svanfeldt

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

#include "csutil/ref.h"
#include "iengine/engine.h"
#include "imap/services.h"
#include "imesh/animesh.h"
#include "imesh/object.h"
#include "iutil/document.h"
#include "iutil/plugin.h"
#include "ivaria/reporter.h"
#include "imap/ldrctxt.h"
#include "iengine/mesh.h"

#include "animeshldr.h"

CS_IMPLEMENT_PLUGIN

CS_PLUGIN_NAMESPACE_BEGIN(Animeshldr)
{
  SCF_IMPLEMENT_FACTORY(AnimeshFactoryLoader);
  SCF_IMPLEMENT_FACTORY(AnimeshObjectLoader);
  SCF_IMPLEMENT_FACTORY(AnimeshFactorySaver);
  SCF_IMPLEMENT_FACTORY(AnimeshObjectSaver);


  AnimeshFactoryLoader::AnimeshFactoryLoader (iBase* parent)
    : scfImplementationType (this, parent)
  {
  }

  csPtr<iBase> AnimeshFactoryLoader::Parse (iDocumentNode* node,
    iStreamSource* ssource, iLoaderContext* ldr_context,
    iBase* context)
  {
    static const char* msgid = "crystalspace.animeshfactoryloader";

    csRef<iMeshObjectType> type = csLoadPluginCheck<iMeshObjectType> (
      object_reg, "crystalspace.mesh.object.animesh", false);

    if (!type)
    {
      synldr->ReportError (msgid, node, 
        "Could not load the animesh object plugin!");
      return 0;
    }

    // Create a factory
    csRef<iMeshObjectFactory> fact = type->NewFactory ();
    csRef<iAnimatedMeshFactory> amfact = 
      scfQueryInterfaceSafe<iAnimatedMeshFactory> (fact);

    if (!amfact)
    {
      synldr->ReportError (msgid, node, 
        "Could not load the animesh object plugin!");
      return 0;
    }

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
            synldr->ReportError (msgid, child, "Couldn't find material '%s'!", 
              matname);
            return 0;
          }
          fact->SetMaterialWrapper (mat);
        }
        break;
      case XMLTOKEN_MIXMODE:
        {
          uint mm;
          if (!synldr->ParseMixmode (child, mm))
            return 0;
          fact->SetMixMode (mm);
        }
        break;
      case XMLTOKEN_VERTEXCOUNT:
        {
          int numv = child->GetContentsValueAsInt ();
          amfact->SetVertexCount (numv);
        }
        break;
      case XMLTOKEN_VERTEX:
        {
          iRenderBuffer* rb = amfact->GetVertices ();
          if (!synldr->ParseRenderBuffer (child, rb))
          {
            synldr->ReportError (msgid, child, "Could not parse render buffer!");
            return 0;
          }
        }
        break;
      case XMLTOKEN_TEXCOORD:
        {
          iRenderBuffer* rb = amfact->GetTexCoords ();
          if (!synldr->ParseRenderBuffer (child, rb))
          {
            synldr->ReportError (msgid, child, "Could not parse render buffer!");
            return 0;
          }
        }
        break;
      case XMLTOKEN_NORMAL:
        {
          iRenderBuffer* rb = amfact->GetNormals ();
          if (!synldr->ParseRenderBuffer (child, rb))
          {
            synldr->ReportError (msgid, child, "Could not parse render buffer!");
            return 0;
          }
        }
        break;
      case XMLTOKEN_TANGENT:
        {
          iRenderBuffer* rb = amfact->GetTangents ();
          if (!synldr->ParseRenderBuffer (child, rb))
          {
            synldr->ReportError (msgid, child, "Could not parse render buffer!");
            return 0;
          }
        }
        break;
      case XMLTOKEN_BINORMAL:
        {
          iRenderBuffer* rb = amfact->GetBinormals ();
          if (!synldr->ParseRenderBuffer (child, rb))
          {
            synldr->ReportError (msgid, child, "Could not parse render buffer!");
            return 0;
          }
        }
        break;
      case XMLTOKEN_COLOR:
        {
          iRenderBuffer* rb = amfact->GetColors ();
          if (!synldr->ParseRenderBuffer (child, rb))
          {
            synldr->ReportError (msgid, child, "Could not parse render buffer!");
            return 0;
          }
        }
        break;
      case XMLTOKEN_SUBMESH:
        {
          // Handle submesh
          csRef<iRenderBuffer> indexBuffer;

          csRef<iDocumentNodeIterator> it = child->GetNodes ();
          while (it->HasNext ())
          {
            csRef<iDocumentNode> child2 = it->Next ();
            if (child2->GetType () != CS_NODE_ELEMENT) continue;
            const char* value = child2->GetValue ();
            csStringID id = xmltokens.Request (value);
            switch (id)
            {
            case XMLTOKEN_INDEX:
              {
                indexBuffer = synldr->ParseRenderBuffer (child2);
                if (!indexBuffer)
                {
                  synldr->ReportError (msgid, child2, "Could not parse render buffer!");
                  return 0;
                }
              }
              break;
            default:
              synldr->ReportBadToken (child2);
              return 0;
            }
          }


          if (indexBuffer)
          {
            amfact->CreateSubMesh (indexBuffer);
          }
        }
        break;
      default:
        synldr->ReportBadToken (child);
        return 0;
      }
    }

    // Recalc stuff
    amfact->Invalidate ();

    return csPtr<iBase> (fact);
  }
  
  bool AnimeshFactoryLoader::Initialize (iObjectRegistry* objReg)
  {
    object_reg = objReg;

    synldr = csQueryRegistry<iSyntaxService> (object_reg);

    InitTokenTable (xmltokens);
    return true;
  }



  AnimeshFactorySaver::AnimeshFactorySaver (iBase* parent)
    : scfImplementationType (this, parent)
  {}

  bool AnimeshFactorySaver::WriteDown (iBase *obj, iDocumentNode* parent,
    iStreamSource*)
  {
    return false;
  }
  
  bool AnimeshFactorySaver::Initialize (iObjectRegistry*)
  {
    return false;
  }




  AnimeshObjectLoader::AnimeshObjectLoader (iBase* parent)
    : scfImplementationType (this, parent)
  {}

  csPtr<iBase> AnimeshObjectLoader::Parse (iDocumentNode* node,
    iStreamSource* ssource, iLoaderContext* ldr_context,
    iBase* context)
  {
    static const char* msgid = "crystalspace.animeshloader";

    csRef<iMeshObject> mesh;
    csRef<iAnimatedMesh> ammesh;

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
            synldr->ReportError (msgid, child, 
              "Couldn't find factory '%s'!", factname);
            return 0;
          }

          csRef<iAnimatedMeshFactory> amfact = 
            scfQueryInterface<iAnimatedMeshFactory> (fact->GetMeshObjectFactory ());
          if (!amfact)
          {
            synldr->ReportError (msgid, child, 
              "Factory '%s' doesn't appear to be a animesh factory!", factname);
            return 0;
          }

          mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          ammesh = scfQueryInterface<iAnimatedMesh> (mesh);
          if (!ammesh)
          {
            synldr->ReportError (msgid, child, 
              "Factory '%s' doesn't appear to be a animesh factory!", factname);
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
            synldr->ReportError (msgid, child, "Couldn't find material '%s'!", 
              matname);
            return 0;
          }
          mesh->SetMaterialWrapper (mat);
        }
        break;
      case XMLTOKEN_MIXMODE:
        {
          uint mm;
          if (!synldr->ParseMixmode (child, mm))
            return 0;
          mesh->SetMixMode (mm);
        }
        break;
      default:
        synldr->ReportBadToken (child);
        return 0;
      }
    }

    return csPtr<iBase> (mesh);    
  }

  bool AnimeshObjectLoader::Initialize (iObjectRegistry* objReg)
  {
    object_reg = objReg;

    synldr = csQueryRegistry<iSyntaxService> (object_reg);

    InitTokenTable (xmltokens);
    return true;
  }




  AnimeshObjectSaver::AnimeshObjectSaver (iBase* parent)
    : scfImplementationType (this, parent)
  {}

  bool AnimeshObjectSaver::WriteDown (iBase *obj, iDocumentNode* parent,
    iStreamSource*)
  {
    return false;
  }

  bool AnimeshObjectSaver::Initialize (iObjectRegistry*)
  {
    return false;
  }

}
CS_PLUGIN_NAMESPACE_END(Animeshldr)

