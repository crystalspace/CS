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


#include "csgeom/math3d.h"
#include "csgeom/matrix3.h"
#include "csgeom/quaternion.h"
#include "csgeom/transfrm.h"
#include "csgeom/tri.h"
#include "csutil/cscolor.h"
#include "csutil/csstring.h"
#include "csutil/scanstr.h"
#include "csutil/sysfunc.h"
#include "csutil/util.h"

#include "iengine/engine.h"
#include "iengine/material.h"
#include "iengine/mesh.h"
#include "imap/ldrctxt.h"
#include "imesh/object.h"
#include "imesh/sprite3d.h"
#include "iutil/comp.h"
#include "iutil/document.h"
#include "iutil/eventh.h"
#include "iutil/object.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"
#include "ivideo/graph3d.h"


#include "spr3dldr.h"

CS_IMPLEMENT_PLUGIN

enum
{
  XMLTOKEN_ACTION,
  XMLTOKEN_BASECOLOR,
  XMLTOKEN_F,
  XMLTOKEN_FACTORY,
  XMLTOKEN_FRAME,
  XMLTOKEN_LIMB,
  XMLTOKEN_LIGHTING,
  XMLTOKEN_MATERIAL,
  XMLTOKEN_MATRIX,
  XMLTOKEN_MIXMODE,
  XMLTOKEN_Q,
  XMLTOKEN_SKELETON,
  XMLTOKEN_SMOOTH,
  XMLTOKEN_TRANSFORM,
  XMLTOKEN_T,
  XMLTOKEN_SOCKET,
  XMLTOKEN_TWEEN,
  XMLTOKEN_V
};

SCF_IMPLEMENT_IBASE (csSprite3DFactoryLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSprite3DFactoryLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csSprite3DFactorySaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSprite3DFactorySaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csSprite3DLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSprite3DLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csSprite3DSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSprite3DSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csSprite3DFactoryLoader)
SCF_IMPLEMENT_FACTORY (csSprite3DFactorySaver)
SCF_IMPLEMENT_FACTORY (csSprite3DLoader)
SCF_IMPLEMENT_FACTORY (csSprite3DSaver)


csSprite3DFactoryLoader::csSprite3DFactoryLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csSprite3DFactoryLoader::~csSprite3DFactoryLoader ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csSprite3DFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csSprite3DFactoryLoader::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);

  xmltokens.Register ("action", XMLTOKEN_ACTION);
  xmltokens.Register ("f", XMLTOKEN_F);
  xmltokens.Register ("frame", XMLTOKEN_FRAME);
  xmltokens.Register ("limb", XMLTOKEN_LIMB);
  xmltokens.Register ("material", XMLTOKEN_MATERIAL);
  xmltokens.Register ("matrix", XMLTOKEN_MATRIX);
  xmltokens.Register ("mixmode", XMLTOKEN_MIXMODE);
  xmltokens.Register ("q", XMLTOKEN_Q);
  xmltokens.Register ("skeleton", XMLTOKEN_SKELETON);
  xmltokens.Register ("smooth", XMLTOKEN_SMOOTH);
  xmltokens.Register ("transform", XMLTOKEN_TRANSFORM);
  xmltokens.Register ("t", XMLTOKEN_T);
  xmltokens.Register ("socket", XMLTOKEN_SOCKET);
  xmltokens.Register ("tween", XMLTOKEN_TWEEN);
  xmltokens.Register ("v", XMLTOKEN_V);
  return true;
}

csPtr<iBase> csSprite3DFactoryLoader::Parse (iDocumentNode* node,
				       iStreamSource*,
				       iLoaderContext* ldr_context, 
				       iBase* context)
{
  csRef<iPluginManager> plugin_mgr (CS_QUERY_REGISTRY (object_reg,
  	iPluginManager));
  csRef<iMeshObjectType> type (CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.sprite.3d", iMeshObjectType));
  if (!type)
  {
    type = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.sprite.3d",
    	iMeshObjectType);
  }
  if (!type)
  {
    synldr->ReportError (
		"crystalspace.sprite3dfactoryloader.setup.objecttype",
		node, "Could not load the sprite.3d mesh object plugin!");
    return 0;
  }

  // @@@ Temporary fix to allow to set actions for objects loaded
  // with impexp. Once those loaders move to another plugin this code
  // below should be removed.
  csRef<iMeshObjectFactory> fact;
  if (context)
    fact = SCF_QUERY_INTERFACE (context, iMeshObjectFactory);
  // DecRef of fact will be handled later.
  // If there was no factory we create a new one.
  if (!fact)
    fact = type->NewFactory ();

  csRef<iSprite3DFactoryState> spr3dLook (
  	SCF_QUERY_INTERFACE (fact, iSprite3DFactoryState));

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
		  "crystalspace.sprite3dfactoryloader.parse.unknownmaterial",
		  child, "Couldn't find material named '%s'", matname);
            return 0;
	  }
	  spr3dLook->SetMaterialWrapper (mat);
        }
        break;

      case XMLTOKEN_SKELETON:
	synldr->ReportError (
		  "crystalspace.sprite3dfactoryloader.parse.badskeletal",
		  child, "Skeletal sprites are no longer supported! (use sprcal3d instead)");
        return 0;

      case XMLTOKEN_ACTION:
        {
          iSpriteAction* act = spr3dLook->AddAction ();
          act->SetName (child->GetAttributeValue ("name"));

	  csRef<iDocumentNodeIterator> child_it = child->GetNodes ();
	  while (child_it->HasNext ())
	  {
	    csRef<iDocumentNode> childchild = child_it->Next ();
	    if (childchild->GetType () != CS_NODE_ELEMENT) continue;
	    const char* child_value = childchild->GetValue ();
	    csStringID id = xmltokens.Request (child_value);
	    switch (id)
	    {
              case XMLTOKEN_F:
	        {
		  const char* fn = childchild->GetAttributeValue ("name");
		  int d = childchild->GetAttributeValueAsInt ("delay");
		  float disp = childchild->GetAttributeValueAsFloat (
		  	"displacement");
                  iSpriteFrame* ff = spr3dLook->FindFrame (fn);
                  if (!ff)
                  {
	            synldr->ReportError (
		      "crystalspace.sprite3dfactoryloader.parse.action",
		      childchild,
		      "Trying to add unknown frame '%s' to action '%s'!",
		      fn, act->GetName ());
                    return 0;
	          }
                  act->AddFrame (ff, d, disp);
                }
                break;
	      default:
	        synldr->ReportBadToken (childchild);
	        return 0;
            }
          }
        }
        break;

      case XMLTOKEN_FRAME:
        {
          iSpriteFrame* fr = spr3dLook->AddFrame ();
          fr->SetName (child->GetAttributeValue ("name"));
          int anm_idx = fr->GetAnmIndex ();
          int tex_idx = fr->GetTexIndex ();
          int i = 0;
          float x, y, z, u, v, nx, ny, nz;
	  csRef<iDocumentNodeIterator> child_it = child->GetNodes ();
	  while (child_it->HasNext ())
	  {
	    csRef<iDocumentNode> childchild = child_it->Next ();
	    if (childchild->GetType () != CS_NODE_ELEMENT) continue;
	    const char* child_value = childchild->GetValue ();
	    csStringID id = xmltokens.Request (child_value);
	    switch (id)
	    {
              case XMLTOKEN_V:
	        {
		  x = childchild->GetAttributeValueAsFloat ("x");
		  y = childchild->GetAttributeValueAsFloat ("y");
		  z = childchild->GetAttributeValueAsFloat ("z");
		  u = childchild->GetAttributeValueAsFloat ("u");
		  v = childchild->GetAttributeValueAsFloat ("v");
		  nx = childchild->GetAttributeValueAsFloat ("nx");
		  ny = childchild->GetAttributeValueAsFloat ("ny");
		  nz = childchild->GetAttributeValueAsFloat ("nz");
                  // check if it's the first frame
                  if (spr3dLook->GetFrameCount () == 1)
                  {
                    spr3dLook->AddVertices (1);
                  }
                  else if (i >= spr3dLook->GetVertexCount ())
                  {
	            synldr->ReportError (
		            "crystalspace.sprite3dfactoryloader.parse.frame",
		            childchild,
			    "Trying to add too many vertices to frame '%s'!",
		            fr->GetName ());
		    return 0;
                  }
                  spr3dLook->SetVertex (anm_idx, i, csVector3 (x, y, z));
                  spr3dLook->SetTexel  (tex_idx, i, csVector2 (u, v));
	          spr3dLook->SetNormal (anm_idx, i, csVector3 (nx, ny, nz));
                  i++;
                }
                break;
	      default:
		synldr->ReportBadToken (childchild);
	        return 0;
            }
	  }
          if (i < spr3dLook->GetVertexCount ())
          {
	    synldr->ReportError (
		 "crystalspace.sprite3dfactoryloader.parse.frame.vertices",
		 child, "Too few vertices in frame '%s'!", fr->GetName ());
	    return 0;
          }
        }
        break;

      case XMLTOKEN_T:
        {
          int a, b, c;
	  a = child->GetAttributeValueAsInt ("v1");
	  b = child->GetAttributeValueAsInt ("v2");
	  c = child->GetAttributeValueAsInt ("v3");
          spr3dLook->AddTriangle (a, b, c);
        }
        break;

      case XMLTOKEN_SOCKET:
        {
          int a = child->GetAttributeValueAsInt ("tri");
          iSpriteSocket* sprite_socket = spr3dLook->AddSocket ();
          sprite_socket->SetName (child->GetAttributeValue ("name"));
          sprite_socket->SetTriangleIndex (a);
        }
        break;

      case XMLTOKEN_SMOOTH:
        {
	  csRef<iDocumentAttribute> attr;
	  int base = -1;
	  int frame = -1;
	  attr = child->GetAttribute ("base");
	  if (attr) base = attr->GetValueAsInt ();
	  attr = child->GetAttribute ("frame");
	  if (attr) frame = attr->GetValueAsInt ();
	  if (base == -1 && frame != -1)
	  {
	    synldr->ReportError (
		  "crystalspace.sprite3dfactoryloader.parse.badsmooth",
		  child,
		  "Please specify 'base' when specifying 'frame' in 'smooth'!");
	    return 0;
	  }
	  if (base == -1)
	    spr3dLook->MergeNormals ();
	  else if (frame == -1)
	    spr3dLook->MergeNormals (base);
	  else
	    spr3dLook->MergeNormals (base, frame);
        }
        break;

      case XMLTOKEN_TWEEN:
        {
          bool do_tween;
          if (!synldr->ParseBool (child, do_tween, true))
	    return 0;
          spr3dLook->EnableTweening (do_tween);
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

csSprite3DFactorySaver::csSprite3DFactorySaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csSprite3DFactorySaver::~csSprite3DFactorySaver ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csSprite3DFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csSprite3DFactorySaver::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  return true;
}

bool csSprite3DFactorySaver::WriteDown (iBase* obj, iDocumentNode* parent,
	iStreamSource*)
{
  if (!parent) return false; //you never know...
  
  csRef<iDocumentNode> paramsNode = 
    parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");

  if (obj)
  {
    csRef<iSprite3DFactoryState> spritefact = 
      SCF_QUERY_INTERFACE (obj, iSprite3DFactoryState);
    csRef<iMeshObjectFactory> meshfact = 
      SCF_QUERY_INTERFACE (obj, iMeshObjectFactory);
    if (!spritefact) return false;
    if (!meshfact) return false;

    //Writedown Material tag
    iMaterialWrapper* mat = spritefact->GetMaterialWrapper();
    if (mat)
    {
      const char* matname = mat->QueryObject()->GetName();
      if (matname && *matname)
      {
        csRef<iDocumentNode> matNode = 
          paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        matNode->SetValue("material");
        csRef<iDocumentNode> matnameNode = 
          matNode->CreateNodeBefore(CS_NODE_TEXT, 0);
        matnameNode->SetValue(matname);
      }    
    }    
    int i;
    //Write Frame Tags
    for (i=0; i<spritefact->GetFrameCount(); i++)
    {
      csRef<iDocumentNode> frameNode = 
        paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      frameNode->SetValue("frame");
      iSpriteFrame* sprite_frame = spritefact->GetFrame(i);
      frameNode->SetAttribute("name", sprite_frame->GetName());
      //Write v Tags
      csRef<iDocumentNode> vertexNode;
      for (int j=0; j<spritefact->GetVertexCount(); j++)
      {
        vertexNode = 
          frameNode->CreateNodeBefore(CS_NODE_ELEMENT, vertexNode);
        vertexNode->SetValue("v");
        csVector3 vertex = spritefact->GetVertex(i,j);
        csVector2 texel = spritefact->GetTexel(i,j);
        //vertexNode->SetAttribute("name", sprite_frame->GetName());
        synldr->WriteVector(vertexNode, &vertex);
        vertexNode->SetAttributeAsFloat("u", texel.x);
        vertexNode->SetAttributeAsFloat("v", texel.y);
      }
    }

    //Write Action Tags
    for (i=0; i<spritefact->GetActionCount(); i++)
    {
      csRef<iDocumentNode> actionNode = 
        paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      actionNode->SetValue("action");
      iSpriteAction* sprite_action = spritefact->GetAction(i);
      actionNode->SetAttribute("name", sprite_action->GetName());
      //Write f Tags
      for (int j=0; j<sprite_action->GetFrameCount(); j++)
      {
        csRef<iDocumentNode> frameNode = 
          actionNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        frameNode->SetValue("f");
        iSpriteFrame* sprite_frame = sprite_action->GetFrame(j);
        frameNode->SetAttribute("name", sprite_frame->GetName());
        frameNode->SetAttributeAsInt("delay", 
          sprite_action->GetFrameDelay(j));
      }
    }

    //Write Triangle Tags
    for (i=0; i<spritefact->GetTriangleCount(); i++)
    {
      csRef<iDocumentNode> triaNode = 
        paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      triaNode->SetValue("t");
      csTriangle sprite_tria = spritefact->GetTriangle(i);
      triaNode->SetAttributeAsInt("v1", sprite_tria.a);
      triaNode->SetAttributeAsInt("v2", sprite_tria.b);
      triaNode->SetAttributeAsInt("v3", sprite_tria.c);
    }

    //Write Socket Tags
    for (i=0; i<spritefact->GetSocketCount(); i++)
    {
      csRef<iDocumentNode> socketNode = 
        paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      socketNode->SetValue("socket");
      iSpriteSocket* sprite_socket = spritefact->GetSocket(i);
      socketNode->SetAttribute("name", sprite_socket->GetName());
      socketNode->SetAttributeAsInt("tri", 
        sprite_socket->GetTriangleIndex());
    }

    //Writedown Tween tag
    synldr->WriteBool(paramsNode, "tween", 
      spritefact->IsTweeningEnabled(), true);

    //TBD: Writedown Smooth tag

  }
  return true;
}
//---------------------------------------------------------------------------
csSprite3DLoader::csSprite3DLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csSprite3DLoader::~csSprite3DLoader ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csSprite3DLoader::Initialize (iObjectRegistry* object_reg)
{
  csSprite3DLoader::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);

  xmltokens.Register ("action", XMLTOKEN_ACTION);
  xmltokens.Register ("basecolor", XMLTOKEN_BASECOLOR);
  xmltokens.Register ("factory", XMLTOKEN_FACTORY);
  xmltokens.Register ("lighting", XMLTOKEN_LIGHTING);
  xmltokens.Register ("material", XMLTOKEN_MATERIAL);
  xmltokens.Register ("mixmode", XMLTOKEN_MIXMODE);
  xmltokens.Register ("tween", XMLTOKEN_TWEEN);
  return true;
}

csPtr<iBase> csSprite3DLoader::Parse (iDocumentNode* node,
	iStreamSource*, iLoaderContext* ldr_context, iBase*)
{
  csRef<iMeshObject> mesh;
  csRef<iSprite3DState> spr3dLook;

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
      	    synldr->ReportError (
		"crystalspace.sprite3dloader.parse.unknownfactory",
		child, "Couldn't find factory '%s'!", factname);
	    return 0;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          spr3dLook = SCF_QUERY_INTERFACE (mesh, iSprite3DState);
	  if (!spr3dLook)
	  {
      	    synldr->ReportError (
		"crystalspace.sprite3dloader.parse.badfactory",
		child, "Factory '%s' doesn't appear to be a spr3d factory!",
		factname);
	    return 0;
	  }
	}
	break;
      case XMLTOKEN_ACTION:
	if (!spr3dLook)
	{
      	  synldr->ReportError (
		"crystalspace.sprite3dloader.parse.missingfactory",
		child,
		"No Factory! Please define 'factory' before 'action'!");
	  return 0;
	}
	else
	{
	  const char* action = child->GetContentsValue ();
	  if (!spr3dLook->SetAction (action))
	  {
      	    synldr->ReportError (
		  "crystalspace.sprite3dloader.parse.action",
		  child,
		  "Action '%s' failed to start!", action);
	    return 0;
	  }
	}
        break;
      case XMLTOKEN_BASECOLOR:
	if (!spr3dLook)
	{
      	  synldr->ReportError (
		"crystalspace.sprite3dloader.parse.missingfactory",
		child,
		"No Factory! Please define 'factory' before 'basecolor'!");
	  return 0;
	}
	else
	{
	  csColor col;
	  if (!synldr->ParseColor (child, col))
	    return 0;
	  spr3dLook->SetBaseColor (col);
	}
        break;
      case XMLTOKEN_LIGHTING:
	if (!spr3dLook)
	{
      	  synldr->ReportError (
		"crystalspace.sprite3dloader.parse.missingfactory",
		child,
		"No Factory! Please define 'factory' before 'lighting'!");
	  return 0;
	}
	else
	{
	  bool do_lighting;
	  if (!synldr->ParseBool (child, do_lighting, true))
	    return 0;
	  spr3dLook->SetLighting (do_lighting);
	}
        break;
      case XMLTOKEN_MATERIAL:
	if (!spr3dLook)
	{
      	  synldr->ReportError (
		"crystalspace.sprite3dloader.parse.missingfactory",
		child,
		"No Factory! Please define 'factory' before 'material'!");
	  return 0;
	}
	else
	{
	  const char* matname = child->GetContentsValue ();
          iMaterialWrapper* mat = ldr_context->FindMaterial (matname);
	  if (!mat)
	  {
      	    synldr->ReportError (
		"crystalspace.sprite3dloader.parse.unknownmaterial",
		child, "Couldn't find material '%s'!", matname);
            return 0;
	  }
	  spr3dLook->SetMaterialWrapper (mat);
	}
	break;
      case XMLTOKEN_MIXMODE:
	if (!spr3dLook)
	{
      	  synldr->ReportError (
		"crystalspace.sprite3dloader.parse.missingfactory",
		child,
		"No Factory! Please define 'factory' before 'mixmode'!");
	  return 0;
	}
	else
        {
	  uint mm;
	  if (!synldr->ParseMixmode (child, mm))
	    return 0;
          spr3dLook->SetMixMode (mm);
	}
	break;
      case XMLTOKEN_TWEEN:
	if (!spr3dLook)
	{
      	  synldr->ReportError (
		"crystalspace.sprite3dloader.parse.missingfactory",
		child,
		"No Factory! Please define 'factory' before 'tween'!");
	  return 0;
	}
	else
	{
	  bool do_tween;
	  if (!synldr->ParseBool (child, do_tween, true))
	    return 0;
          spr3dLook->EnableTweening (do_tween);
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

csSprite3DSaver::csSprite3DSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csSprite3DSaver::~csSprite3DSaver ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csSprite3DSaver::Initialize (iObjectRegistry* object_reg)
{
  csSprite3DSaver::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  return true;
}

bool csSprite3DSaver::WriteDown (iBase* obj, iDocumentNode* parent,
	iStreamSource*)
{
  if (!parent) return false; //you never know...
  
  csRef<iDocumentNode> paramsNode = 
    parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");

  if (obj)
  {
    csRef<iSprite3DState> sprite = SCF_QUERY_INTERFACE (obj, iSprite3DState);
    csRef<iMeshObject> mesh = SCF_QUERY_INTERFACE (obj, iMeshObject);
    if (!sprite) return false;
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

    //Writedown Lighting tag
    synldr->WriteBool(paramsNode, "lighting", sprite->IsLighting(), true);

    //Writedown Tween tag
    synldr->WriteBool(paramsNode, "tween", sprite->IsTweeningEnabled(),true);

    //Writedown Basecolor tag
    csColor col;
    sprite->GetBaseColor(col);
    csRef<iDocumentNode> colorNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    colorNode->SetValue("basecolor");
    synldr->WriteColor(colorNode, &col);

    //Writedown Action tag
    const char* actionname = sprite->GetCurAction()->GetName();
    if (actionname && *actionname)
    {
      csRef<iDocumentNode> actionNode = 
        paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      actionNode->SetValue("action");
      actionNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValue(actionname);
    }    
    
    //Writedown Material tag
    iMaterialWrapper* mat = sprite->GetMaterialWrapper();
    if (mat)
    {
      const char* matname = mat->QueryObject()->GetName();
      if (matname && *matname)
      {
        csRef<iDocumentNode> matNode = 
          paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        matNode->SetValue("material");
        csRef<iDocumentNode> matnameNode = 
          matNode->CreateNodeBefore(CS_NODE_TEXT, 0);
        matnameNode->SetValue(matname);
      }    
    }    

    //Writedown Mixmode tag
    int mixmode = sprite->GetMixMode();
    csRef<iDocumentNode> mixmodeNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    mixmodeNode->SetValue("mixmode");
    synldr->WriteMixmode(mixmodeNode, mixmode, true);
  }
  return true;
}
