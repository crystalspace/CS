/*
    Copyright (C) 2004 Anders Stenberg, Daniel Duhprey

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
#include "iutil/objreg.h"
#include "iutil/document.h"
#include "iutil/object.h"
#include "iutil/plugin.h"
#include "iengine/engine.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "imap/services.h"
#include "imap/loader.h"
#include "imap/ldrctxt.h"
#include "imap/parser.h"
#include "ivaria/reporter.h"

#include "ivaria/terraform.h"
#include "ivaria/simpleformer.h"
#include "loader.h"

CS_IMPLEMENT_PLUGIN

enum
{
  XMLTOKEN_NAME,
  XMLTOKEN_HEIGHTMAP,
  XMLTOKEN_SCALE,
  XMLTOKEN_OFFSET
};

SCF_IMPLEMENT_IBASE (csSimpleFormerLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSimpleFormerLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csSimpleFormerLoader)

csSimpleFormerLoader::csSimpleFormerLoader (iBase* parent)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
}

csSimpleFormerLoader::~csSimpleFormerLoader ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csSimpleFormerLoader::Initialize (iObjectRegistry* object_reg)
{
  objreg = object_reg;
  synldr = CS_QUERY_REGISTRY (objreg, iSyntaxService);
  pluginmgr = CS_QUERY_REGISTRY (objreg, iPluginManager);

  xmltokens.Register ("name", XMLTOKEN_NAME);
  xmltokens.Register ("heightmap", XMLTOKEN_HEIGHTMAP);
  xmltokens.Register ("scale", XMLTOKEN_SCALE);
  xmltokens.Register ("offset", XMLTOKEN_OFFSET);
  return true;
}

csPtr<iBase> csSimpleFormerLoader::Parse (iDocumentNode* node,
		iLoaderContext* /*ldr_context*/,
		iBase* /*context*/)
{
  csRef<iTerraFormer> former = CS_LOAD_PLUGIN (pluginmgr, 
  	"crystalspace.terraformer.simple", iTerraFormer);
  if (!former) 
  {
    synldr->ReportError ("crystalspace.terraformer.simple.loader",
      node, "Could not loader crystalspace.terraformer.simple plugin");
    return 0;
  }
  csRef<iSimpleFormerState> state = SCF_QUERY_INTERFACE (former, 
        iSimpleFormerState);

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char *value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_NAME:
      {
        const char *name = child->GetContentsValue ();
        objreg->Register (former, name);
        break;
      }
      case XMLTOKEN_HEIGHTMAP: 
      {
        const char *image = child->GetContentsValue ();
	csRef<iLoader> loader = CS_QUERY_REGISTRY (objreg, iLoader);
        csRef<iImage> map = loader->LoadImage (image);
        if (map == 0) 
        {
          synldr->ReportError ("crystalspace.terraformer.simple.loader",
            child, "Error reading in image file for heightmap '%s'", image);
          return 0;
        }
        state->SetHeightmap (map);
        break;
      }
      case XMLTOKEN_SCALE:
      {
        csVector3 v;
        if (!synldr->ParseVector (child, v))
        {
          synldr->ReportError ("crystalspace.terraformer.simple.loader",
            child, "Error parsing scale vector");
          return 0;
        }
        state->SetScale (v);
        break;
      }
      case XMLTOKEN_OFFSET:
      {
        csVector3 v;
        if (!synldr->ParseVector (child, v))
        {
          synldr->ReportError ("crystalspace.terraformer.simple.loader",
            child, "Error parsing scale vector");
          return 0;
        }
        state->SetOffset (v);
        break;
      }
      default:
        synldr->ReportError ("crystalspace.terraformer.simple.loader",
          child, "Unkown token!");
    }
  }
  return csPtr<iBase> (former);
}
