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
#include "csutil/csendian.h"
#include "iutil/objreg.h"
#include "iutil/document.h"
#include "iutil/object.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"
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
  XMLTOKEN_HEIGHTMAP32,
  XMLTOKEN_INTMAP,
  XMLTOKEN_FLOATMAP,
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
  xmltokens.Register ("heightmap32", XMLTOKEN_HEIGHTMAP32);
  xmltokens.Register ("intmap", XMLTOKEN_INTMAP);
  xmltokens.Register ("floatmap", XMLTOKEN_FLOATMAP);
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
            child, "Error reading in image file '%s' for heightmap", image);
          return 0;
        }
        state->SetHeightmap (map);
        break;
      }
      case XMLTOKEN_HEIGHTMAP32: 
      {
        const char *filename = child->GetContentsValue ();
	csRef<iVFS> vfs = CS_QUERY_REGISTRY (objreg, iVFS);
	csRef<iDataBuffer> buf = vfs->ReadFile (filename, false);
        if (buf == 0) 
        {
          synldr->ReportError ("crystalspace.terraformer.simple.loader",
            child, "Error reading in file '%s' for heightmap", filename);
          return 0;
        }
	char* data = buf->GetData ();
	char c1 = *data++;
	char c2 = *data++;
	char c3 = *data++;
	char c4 = *data++;
	if (c1 != 'H' || c2 != 'M' || c3 != '3' || c4 != '2')
	{
          synldr->ReportError ("crystalspace.terraformer.simple.loader",
            child, "File '%s' is not a heightmap32 file", filename);
          return 0;
	}
	uint32 width = csGetLittleEndianLong (data); data += 4;
	uint32 height = csGetLittleEndianLong (data); data += 4;
	if (buf->GetSize () != (4+4+4+ width*height*4))
	{
          synldr->ReportError ("crystalspace.terraformer.simple.loader",
            child, "File '%s' is not a valid heightmap32 file: size mismatch",
	    	filename);
          return 0;
	}
	float* fdata = new float[width*height];
	uint32 i;
	for (i = 0 ; i < width * height ; i++)
	{
	  long d = csGetLittleEndianLong (data); data += 4;
	  fdata[i] = float (d) / 4294967296.0f;
	}
        state->SetHeightmap (fdata, width, height);
	delete[] fdata;
        break;
      }
      case XMLTOKEN_INTMAP: 
      {
        const char *image = child->GetContentsValue ();
	csRef<iLoader> loader = CS_QUERY_REGISTRY (objreg, iLoader);
        csRef<iImage> map = loader->LoadImage (image);
        if (map == 0) 
        {
          synldr->ReportError ("crystalspace.terraformer.simple.loader",
            child, "Error reading in image file for intmap '%s'", image);
          return 0;
        }
	int scale = child->GetAttributeValueAsInt ("scale");
	int offset = child->GetAttributeValueAsInt ("offset");
	const char* typestring = child->GetAttributeValue ("type");
        csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
	  objreg, "crystalspace.shared.stringset", iStringSet);
        csStringID type = strings->Request (typestring);
        state->SetIntegerMap (type, map, scale, offset);
        break;
      }
      case XMLTOKEN_FLOATMAP: 
      {
        const char *image = child->GetContentsValue ();
	csRef<iLoader> loader = CS_QUERY_REGISTRY (objreg, iLoader);
        csRef<iImage> map = loader->LoadImage (image);
        if (map == 0) 
        {
          synldr->ReportError ("crystalspace.terraformer.simple.loader",
            child, "Error reading in image file for floatmap '%s'", image);
          return 0;
        }
	float scale = child->GetAttributeValueAsFloat ("scale");
	float offset = child->GetAttributeValueAsFloat ("offset");
	const char* typestring = child->GetAttributeValue ("type");
        csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
	  objreg, "crystalspace.shared.stringset", iStringSet);
        csStringID type = strings->Request (typestring);
        state->SetFloatMap (type, map, scale, offset);
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
