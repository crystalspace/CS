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
#include "csgeom/matrix3.h"
#include "csgeom/quaterni.h"
#include "csgeom/transfrm.h"
#include "csutil/scanstr.h"
#include "csutil/util.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iutil/plugin.h"
#include "imesh/sprite3d.h"
#include "ivideo/graph3d.h"
#include "qint.h"
#include "iutil/vfs.h"
#include "csutil/csstring.h"
#include "iutil/object.h"
#include "iengine/material.h"
#include "ivaria/reporter.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/document.h"
#include "imap/ldrctxt.h"
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
  return true;
}

#define MAXLINE 100 /* max number of chars per line... */

void csSprite3DFactorySaver::WriteDown (iBase*, iFile*)
{
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
	iLoaderContext* ldr_context, iBase*)
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
	spr3dLook->SetAction (child->GetContentsValue ());
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
  return true;
}

void csSprite3DSaver::WriteDown (iBase*, iFile*)
{
}

