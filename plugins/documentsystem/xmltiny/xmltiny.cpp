/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"
#include "iutil/comp.h"
#include "iutil/databuff.h"
#include "iutil/vfs.h"
#include "csutil/scf.h"
#include "csutil/xmltiny.h"

struct csTinyDocWrapper : public iDocument
{
private:
  csRef<iDocument> tinydoc;
public:
  SCF_DECLARE_IBASE;

  csTinyDocWrapper (csRef<iDocument> doc);
  virtual ~csTinyDocWrapper ();

  virtual void Clear ();
  virtual csRef<iDocumentNode> CreateRoot ();
  virtual csRef<iDocumentNode> GetRoot ();
  virtual const char* Parse (iFile* file);
  virtual const char* Parse (iDataBuffer* buf);
  virtual const char* Parse (iString* str);
  virtual const char* Parse (const char* buf);
  virtual const char* Write (iFile* file);
  virtual const char* Write (iString* str);
  virtual const char* Write (iVFS* vfs, const char* filename);

  virtual int Changeable ();
};

SCF_IMPLEMENT_IBASE (csTinyDocWrapper)
  SCF_IMPLEMENTS_INTERFACE (iDocument)
SCF_IMPLEMENT_IBASE_END

csTinyDocWrapper::csTinyDocWrapper (csRef<iDocument> doc)
{
  SCF_CONSTRUCT_IBASE (NULL);

  tinydoc = doc;
}

csTinyDocWrapper::~csTinyDocWrapper ()
{
}

void csTinyDocWrapper::Clear ()
{
  tinydoc->Clear();
}

csRef<iDocumentNode> csTinyDocWrapper::CreateRoot ()
{
  return tinydoc->CreateRoot();
}

csRef<iDocumentNode> csTinyDocWrapper::GetRoot ()
{
  return tinydoc->GetRoot();
}

const char* csTinyDocWrapper::Parse (iFile* file)
{
  char *buf = new char[file->GetSize()];
  file->Read (buf, file->GetSize());
  const char *ret = Parse (buf);
  delete[] buf;
  return ret;
}

const char* csTinyDocWrapper::Parse (iDataBuffer* buf)
{
  return Parse ((char*)buf->GetData());
}

const char* csTinyDocWrapper::Parse (iString* str)
{
  return Parse ((const char*)str);
}

const char* csTinyDocWrapper::Parse (const char* buf)
{
  const char* b = buf;
  while ((*b == ' ') || (*b == '\n') || (*b == '\t') || 
    (*b == '\r')) b++;
  if (*b == '<')
  {
    return tinydoc->Parse (buf);
  }
  else
  {
    return "Data does not seem to be XML.";
  }
}

const char* csTinyDocWrapper::Write (iFile* file)
{
  return tinydoc->Write (file);
}

const char* csTinyDocWrapper::Write (iString* str)
{
  return tinydoc->Write (str);
}

const char* csTinyDocWrapper::Write (iVFS* vfs, const char* filename)
{
  return tinydoc->Write (vfs, filename);
}

int csTinyDocWrapper::Changeable ()
{
  return tinydoc->Changeable();
}

class csTinyXMLPlugin : public iDocumentSystem, public iComponent
{
private:
  csTinyDocumentSystem *tiny;
public:
  SCF_DECLARE_IBASE;

  csTinyXMLPlugin (iBase* parent = NULL);
  virtual ~csTinyXMLPlugin ();

  virtual bool Initialize (iObjectRegistry* objreg);

  csRef<iDocument> CreateDocument ();
};

SCF_IMPLEMENT_IBASE(csTinyXMLPlugin)
  SCF_IMPLEMENTS_INTERFACE(iDocumentSystem)
  SCF_IMPLEMENTS_INTERFACE(iComponent)
SCF_IMPLEMENT_IBASE_END

csTinyXMLPlugin::csTinyXMLPlugin(iBase* parent)
{
  SCF_CONSTRUCT_IBASE(parent);

  tiny = new csTinyDocumentSystem ();
}

csTinyXMLPlugin::~csTinyXMLPlugin()
{
  delete tiny;
}

bool csTinyXMLPlugin::Initialize (iObjectRegistry* objreg)
{
  return true;
}

csRef<iDocument> csTinyXMLPlugin::CreateDocument ()
{
  return csPtr<iDocument> (new csTinyDocWrapper (
    tiny->CreateDocument ()));
}

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csTinyXMLPlugin)

SCF_EXPORT_CLASS_TABLE(xmltiny)
  SCF_EXPORT_CLASS(csTinyXMLPlugin, "crystalspace.documentsystem.tinyxml",
      "Crystal Space TinyXML document system")
SCF_EXPORT_CLASS_TABLE_END

