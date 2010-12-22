/*
    Crystal Space
    Copyright (C) 2008 by Jorrit Tyberghein

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
    Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "cssysdef.h"
#include "csutil/xmltiny.h"
#include "csutil/stringquote.h"
#include "iutil/objreg.h"
#include "iutil/document.h"
#include "iutil/object.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"
#include "imap/services.h"
#include "imap/ldrctxt.h"
#include "iengine/sector.h"
#include "csgeom/path.h"

#include "pathloader.h"

//---------------------------------------------------------------------------



SCF_IMPLEMENT_FACTORY (csPathLoader)

enum
{
  XMLTOKEN_NODE,
  XMLTOKEN_FORWARD,
  XMLTOKEN_UP,
  XMLTOKEN_POSITION,
};


csPathLoader::csPathLoader (iBase* parent)
  : scfImplementationType (this, parent)
{
  object_reg = 0;
}

csPathLoader::~csPathLoader ()
{
}

bool csPathLoader::Initialize (iObjectRegistry* object_reg)
{
  csPathLoader::object_reg = object_reg;
  synldr = csQueryRegistry<iSyntaxService> (object_reg);
  if (!synldr)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.addons.pathloader",
	"Can't find syntax services!");
    return false;
  }
  xmltokens.Register ("node", XMLTOKEN_NODE);
  xmltokens.Register ("up", XMLTOKEN_UP);
  xmltokens.Register ("position", XMLTOKEN_POSITION);
  xmltokens.Register ("forward", XMLTOKEN_FORWARD);
  
  return true;
}

csPtr<iBase> csPathLoader::Parse (iDocumentNode* node,
	iStreamSource*, iLoaderContext* ldr_context, iBase* context)
{
  csRef<iSector> sector = scfQueryInterface<iSector>(context);
  if (!sector)
  {
    synldr->ReportError (
	"crystalspace.addons.pathloader", node,
	"Path addons must be placed inside sectors!");
    return 0;
  }
  csRef<iPath> path;
  path = Load (node);
  csRef<iBase> path_return = scfQueryInterface<iBase>(path);
  sector->QueryObject()->ObjAdd(path->QueryObject());
  return csPtr<iBase> (path_return);
}

csPtr<iPath> csPathLoader::Load (const char* path, const char* file)
{
  csRef<iVFS> vfs = csQueryRegistry<iVFS> (object_reg);
  if (path)
  {
    vfs->PushDir ();
    vfs->ChDir (path);
  }

  csRef<iFile> buf = vfs->Open (file, VFS_FILE_READ);
  if (!buf)
  {
    if (path)
      csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
	"crystalspace.addons.pathloader",
	"Can't load file %s from %s!",
	CS::Quote::Single (file), CS::Quote::Single (path));
    else
      csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
	"crystalspace.addons.pathloader",
	"Can't load file %s!", CS::Quote::Single (file));
    return 0;
  }

  csRef<iDocumentSystem> docsys = csQueryRegistry<iDocumentSystem> (object_reg);
  if (!docsys) docsys = csPtr<iDocumentSystem> (new csTinyDocumentSystem ());
  csRef<iDocument> doc = docsys->CreateDocument ();
  const char* error = doc->Parse (buf, true);
  if (error != 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
	    "crystalspace.addons.pathloader",
	    "Document system error for file %s: %s!",
	    CS::Quote::Single (file), error);
    return 0;
  }
  csRef<iPath> cspath = Load (doc->GetRoot ()->GetNode ("addon"));

  if (path)
  {
    vfs->PopDir ();
  }

  return csPtr<iPath>(cspath);
}

bool csPathLoader::ParseNode(iDocumentNode *node, csPath *path)
{
  int curridx = path->Length();
  path->InsertPoint(curridx);
  csRef<iDocumentAttribute> timeattr = node->GetAttribute("time");
  if (timeattr)
    path->SetTime(curridx,timeattr->GetValueAsFloat());

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
    case XMLTOKEN_POSITION:
      {
	csVector3 pos;
	synldr->ParseVector (child, pos);
	path->SetPositionVector(curridx,pos);
	break;
      }
    case XMLTOKEN_FORWARD:
      {
	csVector3 fw;
	synldr->ParseVector (child, fw);
	path->SetForwardVector(curridx,fw);
	break;
      }
    case XMLTOKEN_UP:
      {
	csVector3 up;
	synldr->ParseVector (child, up);
	path->SetUpVector(curridx,up);
	break;
      }
    }
  }
  return true;
}

csPtr<iPath> csPathLoader::Load (iDocumentNode* node)
{
  const char* pathname = node->GetAttributeValue ("name");
  csPath *path = new csPath(0);
  path->QueryObject()->SetName(pathname);

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_NODE:
      {
        ParseNode(child,path);
        break;
      }
    }
  }
  return csPtr<iPath>((iPath*)path);
}

