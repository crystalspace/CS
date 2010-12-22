/*
    Copyright (C) 2001 by Jorrit Tyberghein
    Copyright (C) 2001 by W.C.A. Wijngaards

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
#include "csutil/sysfunc.h"
#include "csgeom/math3d.h"
#include "csutil/scanstr.h"
#include "csutil/util.h"
#include "spr2dldr.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iutil/plugin.h"
#include "iutil/document.h"
#include "imesh/sprite2d.h"
#include "ivideo/graph3d.h"
#include "csqint.h"
#include "csutil/csstring.h"
#include "iutil/object.h"
#include "iutil/vfs.h"
#include "iengine/material.h"
#include "ivaria/reporter.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/stringarray.h"
#include "imap/ldrctxt.h"



enum
{
  XMLTOKEN_UVANIMATION,
  XMLTOKEN_FRAME,
  XMLTOKEN_DURATION,

  XMLTOKEN_COLOR,
  XMLTOKEN_FACTORY,
  XMLTOKEN_LIGHTING,
  XMLTOKEN_MATERIAL,
  XMLTOKEN_MIXMODE,
  XMLTOKEN_UV,
  XMLTOKEN_V,
  XMLTOKEN_ANIMATE
};

SCF_IMPLEMENT_FACTORY (csSprite2DFactoryLoader)
SCF_IMPLEMENT_FACTORY (csSprite2DFactorySaver)
SCF_IMPLEMENT_FACTORY (csSprite2DLoader)
SCF_IMPLEMENT_FACTORY (csSprite2DSaver)

csSprite2DFactoryLoader::csSprite2DFactoryLoader (iBase* pParent) :
  scfImplementationType(this, pParent)
{
}

csSprite2DFactoryLoader::~csSprite2DFactoryLoader ()
{
}

bool csSprite2DFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csSprite2DFactoryLoader::object_reg = object_reg;
  reporter = csQueryRegistry<iReporter> (object_reg);
  synldr = csQueryRegistry<iSyntaxService> (object_reg);

  xmltokens.Register ("lighting", XMLTOKEN_LIGHTING);
  xmltokens.Register ("material", XMLTOKEN_MATERIAL);
  xmltokens.Register ("mixmode", XMLTOKEN_MIXMODE);
  xmltokens.Register ("uvanimation", XMLTOKEN_UVANIMATION);
  xmltokens.Register ("frame", XMLTOKEN_FRAME);
  xmltokens.Register ("duration", XMLTOKEN_DURATION);
  xmltokens.Register ("v", XMLTOKEN_V);
  xmltokens.Register ("uv", XMLTOKEN_UV);
  xmltokens.Register ("color", XMLTOKEN_COLOR);
  return true;
}

bool csSprite2DFactoryLoader::ParseAnim (iDocumentNode* node,
        iReporter*, 
        iSprite2DFactoryState* spr2dLook, 
        const char *animname)
{
  int maxv = 200;
  float* verts = new float[maxv];
  int duration;

  iSprite2DUVAnimation *ani = spr2dLook->CreateUVAnimation ();
  ani->SetName (animname);

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_FRAME:
        {
          duration = 1;
          int numv = 0;
          csRef<iDocumentNodeIterator> child_it = child->GetNodes ();
          while (child_it->HasNext ())
          {
            csRef<iDocumentNode> childchild = child_it->Next ();
            if (childchild->GetType () != CS_NODE_ELEMENT) continue;
            const char* child_value = childchild->GetValue ();
            csStringID id = xmltokens.Request (child_value);
            switch (id)
            {
              case XMLTOKEN_DURATION:
                duration = childchild->GetContentsValueAsInt ();
                break;
              case XMLTOKEN_V:
                verts[numv++] = childchild->GetAttributeValueAsFloat ("u");
                verts[numv++] = childchild->GetAttributeValueAsFloat ("v");
                if (numv >= maxv)
                {
                  maxv += 200;
                  float* newverts = new float[maxv];
                  memcpy (newverts, verts, numv*sizeof (float));
                  delete [] verts;
                  verts = newverts;
                }
                break;
              default:
                synldr->ReportBadToken (childchild);
                delete[] verts;
                return false;
            }
          }
          iSprite2DUVAnimationFrame *frame = ani->CreateFrame (-1);
          frame->SetFrameData (child->GetAttributeValue ("name"), duration, numv/2, verts);
        }
        break;
      default:
        synldr->ReportBadToken (child);
        delete[] verts;
        return false;
    }
  }
  delete[] verts;
  return true;
}

csPtr<iBase> csSprite2DFactoryLoader::Parse (iDocumentNode* node,
	iStreamSource*, iLoaderContext* ldr_context, iBase* /* context */)
{
  csRef<iMeshObjectType> type = csLoadPluginCheck<iMeshObjectType> (
          object_reg, "crystalspace.mesh.object.sprite.2d", false);
  if (!type)
  {
    synldr->ReportError (
      "crystalspace.sprite2dfactoryloader.setup.objecttype",
      node, "Could not load the sprite.2d mesh object plugin!");
    return 0;
  }
  csRef<iMeshObjectFactory> fact (type->NewFactory ());
  csRef<iSprite2DFactoryState> spr2dLook (
          scfQueryInterface<iSprite2DFactoryState> (fact));

  iColoredVertices* verts = spr2dLook->GetVertices ();
  int vnum = 0;
  int uvnum = 0;
  int colnum = 0;

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
                "crystalspace.sprite2dfactoryloader.parse.unknownmaterial",
                child, "Couldn't find material named %s", CS::Quote::Single (matname));
            return 0;
          }
          fact->SetMaterialWrapper (mat);
        }
        break;
      case XMLTOKEN_LIGHTING:
        {
          bool do_lighting;
          if (!synldr->ParseBool (child, do_lighting, true))
            return 0;
          spr2dLook->SetLighting (do_lighting);
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
      case XMLTOKEN_UVANIMATION:
        if (!ParseAnim (child, reporter, spr2dLook,
                child->GetAttributeValue ("name")))
          return 0;
        break;
      case XMLTOKEN_V:
        {
          float x = child->GetAttributeValueAsFloat ("x");
          float y = child->GetAttributeValueAsFloat ("y");
          vnum++;
          if (vnum > uvnum && vnum > colnum)
            verts->SetSize (vnum);
          csSprite2DVertex& v = verts->Get (vnum-1);
          v.pos.x = x;
          v.pos.y = y;
          v.color_init.Set (0, 0, 0);
          v.color.Set (0, 0, 0);
        }
        break;
      case XMLTOKEN_UV:
        {
          float u = child->GetAttributeValueAsFloat ("u");
          float v = child->GetAttributeValueAsFloat ("v");
          uvnum++;
          if (uvnum > vnum && uvnum > colnum)
            verts->SetSize (uvnum);
          csSprite2DVertex& V = verts->Get (uvnum-1);
          V.u = u;
          V.v = v;
        }
        break;
      case XMLTOKEN_COLOR:
        {
          float r = child->GetAttributeValueAsFloat ("red");
          float g = child->GetAttributeValueAsFloat ("green");
          float b = child->GetAttributeValueAsFloat ("blue");
          colnum++;
          if (colnum > vnum && colnum > uvnum)
            verts->SetSize (colnum);
          csSprite2DVertex& v = verts->Get (colnum-1);
          v.color_init.red = r;
          v.color_init.green = g;
          v.color_init.blue = b;
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
csSprite2DFactorySaver::csSprite2DFactorySaver (iBase* pParent) :
  scfImplementationType(this, pParent)
{
}

csSprite2DFactorySaver::~csSprite2DFactorySaver ()
{
}

bool csSprite2DFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csSprite2DFactorySaver::object_reg = object_reg;
  reporter = csQueryRegistry<iReporter> (object_reg);
  synldr = csQueryRegistry<iSyntaxService> (object_reg);
  return true;
}

bool csSprite2DFactorySaver::WriteDown (iBase* obj, iDocumentNode* parent,
        iStreamSource*)
{
  if (!parent) return false; //you never know...
  if (!obj) return false; //you never know...

  csRef<iDocumentNode> paramsNode = parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  paramsNode->SetValue ("params");

  csRef<iSprite2DFactoryState> spritefact = scfQueryInterface<iSprite2DFactoryState> (obj);
  csRef<iMeshObjectFactory> meshfact = scfQueryInterface<iMeshObjectFactory> (obj);
  if (!spritefact) return false;
  if (!meshfact) return false;

  //Writedown Material tag
  iMaterialWrapper* mat = meshfact->GetMaterialWrapper();
  if (mat)
  {
    const char* matname = mat->QueryObject()->GetName();
    if (matname && *matname)
    {
      csRef<iDocumentNode> matNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      matNode->SetValue("material");
      matNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValue(matname);
    }
  }

  //Writedown Lighting tag
  synldr->WriteBool(paramsNode, "lighting", spritefact->HasLighting(), true);

  for (int i=0; i<spritefact->GetUVAnimationCount(); i++)
  {
    csRef<iDocumentNode> uvaniNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    uvaniNode->SetValue("uvanimation");
    iSprite2DUVAnimation* anim = spritefact->GetUVAnimation(i);
    const char* animname = anim->GetName();
    uvaniNode->SetAttribute("name", animname);
    for (int j=0; j<anim->GetFrameCount(); j++)
    {
      csRef<iDocumentNode> frameNode = uvaniNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      frameNode->SetValue("frame");
      iSprite2DUVAnimationFrame* frame = anim->GetFrame(j);
      const char* framename = frame->GetName();
      frameNode->SetAttribute("name", framename);
      int duration = frame->GetDuration();
      csRef<iDocumentNode> durationNode = frameNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      durationNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsInt(duration);
      for (int i=0; i<frame->GetUVCount(); i++)
      {
        csVector2 uv = frame->GetUVCoo(i);
        csRef<iDocumentNode> vNode = frameNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        vNode->SetValue("v");
        vNode->SetAttributeAsFloat("u", uv.x);
        vNode->SetAttributeAsFloat("v", uv.y);
      }
    }
  }

  //Writedown Mixmode tag
  int mixmode = meshfact->GetMixMode();
  csRef<iDocumentNode> mixmodeNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  mixmodeNode->SetValue("mixmode");
  synldr->WriteMixmode(mixmodeNode, mixmode, true);

  return true;
}
//---------------------------------------------------------------------------

csSprite2DLoader::csSprite2DLoader (iBase* pParent) :
  scfImplementationType(this, pParent)
{
}

csSprite2DLoader::~csSprite2DLoader ()
{
}

bool csSprite2DLoader::Initialize (iObjectRegistry* object_reg)
{
  csSprite2DLoader::object_reg = object_reg;
  reporter = csQueryRegistry<iReporter> (object_reg);
  synldr = csQueryRegistry<iSyntaxService> (object_reg);

  xmltokens.Register ("color", XMLTOKEN_COLOR);
  xmltokens.Register ("factory", XMLTOKEN_FACTORY);
  xmltokens.Register ("lighting", XMLTOKEN_LIGHTING);
  xmltokens.Register ("material", XMLTOKEN_MATERIAL);
  xmltokens.Register ("mixmode", XMLTOKEN_MIXMODE);
  xmltokens.Register ("uv", XMLTOKEN_UV);
  xmltokens.Register ("v", XMLTOKEN_V);
  xmltokens.Register ("animate", XMLTOKEN_ANIMATE);
  return true;
}

#define CHECK_MESH(m) \
  if (!m) { \
    synldr->ReportError ( \
        "crystalspace.sprite2dloader.parse.unknownfactory", \
        child, "Specify the factory first!"); \
    return 0; \
  }


csPtr<iBase> csSprite2DLoader::Parse (iDocumentNode* node,
				iStreamSource*, iLoaderContext* ldr_context,
				iBase*)
{
  csRef<iMeshObject> mesh;
  csRef<iSprite2DState> spr2dLook;
  csRef<iSprite2DFactoryState> spr2dLookFact;
  iColoredVertices* verts = 0;
  int vnum = 0;
  int uvnum = 0;
  int colnum = 0;

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

    if(!fact)
    {
      synldr->ReportError (
        "crystalspace.sprite2dloader.parse.unknownfactory",
        child, "Couldn't find factory %s!", CS::Quote::Single (factname));
      return 0;
    }

        spr2dLookFact = scfQueryInterface<iSprite2DFactoryState> (fact->GetMeshObjectFactory ());
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          spr2dLook = scfQueryInterface<iSprite2DState> (mesh);
          if (!spr2dLook)
          {
            synldr->ReportError (
              "crystalspace.sprite2dloader.parse.badfactory",
              child, "Factory %s doesn't appear to be a spr2d factory!",
              CS::Quote::Single (factname));
            return 0;
          }
          verts = spr2dLook->GetVertices ();
        }
        break;
      case XMLTOKEN_MATERIAL:
        {
          const char* matname = child->GetContentsValue ();
          iMaterialWrapper* mat = ldr_context->FindMaterial (matname);
          if (!mat)
          {
            synldr->ReportError (
              "crystalspace.sprite2dloader.parse.unknownmaterial",
              child, "Couldn't find material %s!", CS::Quote::Single (matname));
            return 0;
          }
          CHECK_MESH (mesh);
          mesh->SetMaterialWrapper (mat);
        }
        break;
      case XMLTOKEN_MIXMODE:
        {
          uint mm;
          if (!synldr->ParseMixmode (child, mm))
            return 0;
          CHECK_MESH (mesh);
          mesh->SetMixMode (mm);
        }
        break;
      case XMLTOKEN_V:
        {
          float x = child->GetAttributeValueAsFloat ("x");
          float y = child->GetAttributeValueAsFloat ("y");
          vnum++;
          if (vnum > uvnum && vnum > colnum)
            verts->SetSize (vnum);
          csSprite2DVertex& v = verts->Get (vnum-1);
          v.pos.x = x;
          v.pos.y = y;
          v.color_init.Set (0, 0, 0);
          v.color.Set (0, 0, 0);
        }
        break;
      case XMLTOKEN_UV:
        {
          float u = child->GetAttributeValueAsFloat ("u");
          float v = child->GetAttributeValueAsFloat ("v");
          uvnum++;
          if (uvnum > vnum && uvnum > colnum)
            verts->SetSize (uvnum);
          csSprite2DVertex& V = verts->Get (uvnum-1);
          V.u = u;
          V.v = v;
        }
        break;
      case XMLTOKEN_COLOR:
        {
          float r = child->GetAttributeValueAsFloat ("red");
          float g = child->GetAttributeValueAsFloat ("green");
          float b = child->GetAttributeValueAsFloat ("blue");
          colnum++;
          if (colnum > vnum && colnum > uvnum)
            verts->SetSize (colnum);
          csSprite2DVertex& v = verts->Get (colnum-1);
          v.color_init.red = r;
          v.color_init.green = g;
          v.color_init.blue = b;
        }
        break;
      case XMLTOKEN_LIGHTING:
        {
          bool do_lighting;
          if (!synldr->ParseBool (child, do_lighting, true))
            return 0;
          CHECK_MESH (spr2dLook);
          spr2dLook->SetLighting (do_lighting);
        }
        break;
      case XMLTOKEN_ANIMATE:
        {
          CHECK_MESH (spr2dLook);
          bool loop = false;
          int timing = 0;
          const char* animname = child->GetAttributeValue ("name");
          csRef<iDocumentNode> loopnode = child->GetNode ("loop");
          if (loopnode) synldr->ParseBool (loopnode, loop, true);
          csRef<iDocumentNode> timingnode = child->GetNode ("timing");
          if (timingnode) timing = timingnode->GetContentsValueAsInt ();
          iSprite2DUVAnimation *ani = spr2dLook->GetUVAnimation (animname);
          if (ani)
            spr2dLook->SetUVAnimation (animname, timing, loop);
          else
          {
            synldr->ReportError (
              "crystalspace.sprite2dloader.parse.uvanim",
              child, "UVAnimation %s not found!", CS::Quote::Single (animname));
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

csSprite2DSaver::csSprite2DSaver (iBase* pParent) :
  scfImplementationType(this, pParent)
{
}

csSprite2DSaver::~csSprite2DSaver ()
{
}

bool csSprite2DSaver::Initialize (iObjectRegistry* object_reg)
{
  csSprite2DSaver::object_reg = object_reg;
  reporter = csQueryRegistry<iReporter> (object_reg);
  synldr = csQueryRegistry<iSyntaxService> (object_reg);
  return true;
}

bool csSprite2DSaver::WriteDown (iBase* obj, iDocumentNode* parent,
        iStreamSource*)
{
  if (!parent) return false; //you never know...
  if (!obj) return false; //you never know...

  csRef<iDocumentNode> paramsNode = parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");

  csRef<iSprite2DState> sprite = scfQueryInterface<iSprite2DState> (obj);
  csRef<iMeshObject> mesh = scfQueryInterface<iMeshObject> (obj);

  if (!sprite) return false;
  if (!mesh) return false;

  //Writedown Factory tag
  iMeshFactoryWrapper* fact = mesh->GetFactory()->GetMeshFactoryWrapper ();
  if (fact)
  {
    const char* factname = fact->QueryObject()->GetName();
    if (factname && *factname)
    {
      csRef<iDocumentNode> factNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      factNode->SetValue("factory");
      factNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValue(factname);
    }
  }

  //Writedown vertex tag
  iColoredVertices* vertices = sprite->GetVertices();
  for (size_t i = 0; i < vertices->GetSize(); i++)
  {
    const csSprite2DVertex& vertex = vertices->Get (i);

    csRef<iDocumentNode> vNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    vNode->SetValue("v");
    vNode->SetAttributeAsFloat("x", vertex.pos.x);
    vNode->SetAttributeAsFloat("y", vertex.pos.y);
  }

  //Writedown uv tag
  for (size_t i = 0; i < vertices->GetSize(); i++)
  {
    const csSprite2DVertex& vertex = vertices->Get (i);

    csRef<iDocumentNode> uvNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    uvNode->SetValue("uv");
    uvNode->SetAttributeAsFloat("u", vertex.u);
    uvNode->SetAttributeAsFloat("v", vertex.v);

  }

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

  //Writedown color tag
  for (size_t i = 0; i < vertices->GetSize(); i++)
  {
    const csSprite2DVertex& vertex = vertices->Get (i);

    csRef<iDocumentNode> colorNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    colorNode->SetValue("color");
    colorNode->SetAttributeAsFloat("red", vertex.color.red);
    colorNode->SetAttributeAsFloat("green", vertex.color.green);
    colorNode->SetAttributeAsFloat("blue", vertex.color.blue);
  }

  //Writedown Lighting tag
  synldr->WriteBool(paramsNode, "lighting", sprite->HasLighting(), true);

  //Write down animate tag
  for (int i=0; i<sprite->GetUVAnimationCount(); i++)
  {
    csRef<iDocumentNode> uvaniNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    uvaniNode->SetValue("animate");
    bool loop;
    int style;
    iSprite2DUVAnimation* anim = sprite->GetUVAnimation(i, style, loop);
    const char* animname = anim->GetName();
    uvaniNode->SetAttribute("name", animname);
    synldr->WriteBool(uvaniNode,"loop",loop,false);

    csRef<iDocumentNode> styleNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    styleNode->SetValue ("style");
    styleNode->CreateNodeBefore (CS_NODE_TEXT, 0)->SetValueAsInt(style);
  }

  //Writedown Mixmode tag
  int mixmode = mesh->GetMixMode();
  csRef<iDocumentNode> mixmodeNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  mixmodeNode->SetValue("mixmode");
  synldr->WriteMixmode(mixmodeNode, mixmode, true);

  return true;
}
