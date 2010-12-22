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

#include "csutil/csstring.h"
#include "csutil/csendian.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/scf.h"

#include "iengine/engine.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "imap/ldrctxt.h"
#include "imap/loader.h"
#include "imap/services.h"
#include "iutil/databuff.h"
#include "iutil/document.h"
#include "iutil/object.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"
#include "ivaria/pagingformer.h"
#include "ivaria/terraform.h"

#include "loader.h"



namespace cspluginPagingFormerLoader
{

SCF_IMPLEMENT_FACTORY (csPagingFormerLoader)

csPagingFormerLoader::csPagingFormerLoader (iBase* parent) :
  scfImplementationType(this, parent)
{
}

csPagingFormerLoader::~csPagingFormerLoader ()
{
}

bool csPagingFormerLoader::Initialize (iObjectRegistry* object_reg)
{
  objreg = object_reg;
  synldr = csQueryRegistry<iSyntaxService> (objreg);
  pluginmgr = csQueryRegistry<iPluginManager> (objreg);

  InitTokenTable (xmltokens);
  return true;
}

csPtr<iBase> csPagingFormerLoader::Parse (iDocumentNode* node,
		iStreamSource*, iLoaderContext* /*ldr_context*/,
		iBase* /*context*/)
{
  csRef<iTerraFormer> former = csLoadPlugin<iTerraFormer> (pluginmgr, 
  	"crystalspace.terraformer.paging");
  if (!former) 
  {
    synldr->ReportError ("crystalspace.terraformer.paging.loader",
      node, "Could not loader crystalspace.terraformer.paging plugin");
    return 0;
  }
  csRef<iPagingFormerState> state =  
        scfQueryInterface<iPagingFormerState> (former);

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
      case XMLTOKEN_HEIGHTMAPDIR:
      {
        const char* formatstring = child->GetAttributeValue ("format");
        const char *dir = child->GetContentsValue ();
        state->SetHeightmapDir (dir, 
          (formatstring == 0) ? "image" : formatstring);
        break;
      }
      case XMLTOKEN_INTMAPDIR:
      {
        const char* typestring = child->GetAttributeValue ("type");
        const char *dir = child->GetContentsValue ();
        csRef<iStringSet> strings = csQueryRegistryTagInterface<iStringSet> (
          objreg, "crystalspace.shared.stringset");
        state->SetIntmapDir (strings->Request(typestring),dir);
        break;
      }
      case XMLTOKEN_FLOATMAPDIR:
      {
        const char* typestring = child->GetAttributeValue ("type");
        const char *dir = child->GetContentsValue ();
        csRef<iStringSet> strings = csQueryRegistryTagInterface<iStringSet> (
          objreg, "crystalspace.shared.stringset");
        state->SetFloatmapDir (strings->Request(typestring),dir);
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
      case XMLTOKEN_INTMAP:
      {
        const char *image = child->GetContentsValue ();
        csRef<iLoader> loader = csQueryRegistry<iLoader> (objreg);
        csRef<iImage> map = loader->LoadImage (image);
        if (map == 0)
        {
          synldr->ReportError ("crystalspace.terraformer.simple.loader",
            child, "Error reading in image file for intmap %s",
	    CS::Quote::Single (image));
          return 0;
        }
        int scale = child->GetAttributeValueAsInt ("scale");
        int offset = child->GetAttributeValueAsInt ("offset");
        const char* typestring = child->GetAttributeValue ("type");
        csRef<iStringSet> strings = csQueryRegistryTagInterface<iStringSet> (
          objreg, "crystalspace.shared.stringset");
        csStringID type = strings->Request (typestring);
        state->SetIntegerMap (type, map, scale, offset);
        break;
      }
      case XMLTOKEN_FLOATMAP:
      {
        const char *image = child->GetContentsValue ();
        csRef<iLoader> loader = csQueryRegistry<iLoader> (objreg);
        csRef<iImage> map = loader->LoadImage (image);
        if (map == 0)
        {
          synldr->ReportError ("crystalspace.terraformer.simple.loader",
            child, "Error reading in image file for floatmap %s",
	    CS::Quote::Single (image));
          return 0;
        }
        float scale = child->GetAttributeValueAsFloat ("scale");
        float offset = child->GetAttributeValueAsFloat ("offset");
        const char* typestring = child->GetAttributeValue ("type");
        csRef<iStringSet> strings = csQueryRegistryTagInterface<iStringSet> (
          objreg, "crystalspace.shared.stringset");
        csStringID type = strings->Request (typestring);
        state->SetFloatMap (type, map, scale, offset);
        break;
      }
      default:
        synldr->ReportError ("crystalspace.terraformer.simple.loader",
          child, "Unknown token!");
    }
  }
  return csPtr<iBase> (former);
}

} // namespace cspluginPagingFormerLoader
