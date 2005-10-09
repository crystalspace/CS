/*
    Copyright (C) 2004 by Jorrit Tyberghein
	      (C) 2004 by Frank Richter

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
#include "iutil/vfs.h"
#include "csutil/physfile.h"
#include "csutil/util.h"
#include "csutil/scf.h"
#include "csutil/scf_implementation.h"
#include "csutil/xmltiny.h"
#include "csutil/cfgdoc.h"

class csConfigDocumentIterator : public scfImplementation1<csConfigDocumentIterator,
                                                           iConfigIterator>
{
  csRef<csConfigDocument> doc;
  csHash<csConfigDocument::KeyInfo, csStrKey>::GlobalIterator* iterator;
  char* subsection;
  size_t subsectionLen;
  const csConfigDocument::KeyInfo* currentKey;
  const char* currentKeyName;
public:

  csConfigDocumentIterator (csConfigDocument* doc, const char* Subsection);
  virtual ~csConfigDocumentIterator();
  
  virtual iConfigFile *GetConfigFile () const;
  virtual const char *GetSubsection () const;

  virtual void Rewind ();
  virtual bool Next();
  virtual const char *GetKey (bool Local = false) const;
  virtual int GetInt () const;
  virtual float GetFloat () const;
  virtual const char *GetStr () const;
  virtual bool GetBool () const;
  virtual const char *GetComment () const;
};


csConfigDocumentIterator::csConfigDocumentIterator (csConfigDocument* d,
						    const char* Subsection) 
  : scfImplementationType (this), doc (d), currentKey (0), currentKeyName(0)
{
  subsection = csStrNew (Subsection);
  subsectionLen = subsection ? strlen (subsection) : 0;
  iterator = new csHash<csConfigDocument::KeyInfo, 
    csStrKey>::GlobalIterator (doc->keys.GetIterator ());
}

csConfigDocumentIterator::~csConfigDocumentIterator()
{
  delete[] subsection;
  delete iterator;
}

iConfigFile* csConfigDocumentIterator::GetConfigFile () const
{
  return doc;
}

const char* csConfigDocumentIterator::GetSubsection () const
{
  return subsection;
}

void csConfigDocumentIterator::Rewind ()
{
  currentKey = 0;
  currentKeyName = csStrKey (0);
  iterator->Reset();
}

bool csConfigDocumentIterator::Next()
{
  while (iterator->HasNext ())
  {
    const csConfigDocument::KeyInfo* key = &iterator->Next ();

    if ((subsection == 0) ||
      (strncasecmp (key->originalKey, subsection, subsectionLen) == 0))
    {
      if (!key->cachedStringValue) continue;

      currentKey = key;
      currentKeyName = key->originalKey;
      return true;
    }
  }
  return false;
}

const char* csConfigDocumentIterator::GetKey (bool Local) const
{
  return ((const char*)currentKeyName) + (Local ? subsectionLen : 0);
}

int csConfigDocumentIterator::GetInt () const
{
  if (!currentKey) return 0;
  const char* val = currentKey->cachedStringValue;
  
  int v = 0;
  sscanf (val, "%d", &v);
  return v;
}

float csConfigDocumentIterator::GetFloat () const
{
  if (!currentKey) return 0.0f;
  const char* val = currentKey->cachedStringValue;
  
  float v = 0.0f;
  sscanf (val, "%f", &v);
  return v;
}

const char* csConfigDocumentIterator::GetStr () const
{
  if (!currentKey) return 0;
  return currentKey->cachedStringValue;
}

bool csConfigDocumentIterator::GetBool () const
{
  if (!currentKey) return false;
  const char* val = currentKey->cachedStringValue;

  return (
     strcasecmp(val, "true") == 0 ||
     strcasecmp(val, "yes" ) == 0 ||
     strcasecmp(val, "on"  ) == 0 ||
     strcasecmp(val, "1"   ) == 0);
}

const char* csConfigDocumentIterator::GetComment () const
{
  if (!currentKey) return 0;
  return currentKey->cachedComment;
}

//---------------------------------------------------------------------------
csConfigDocument::csConfigDocument () 
  : scfImplementationType (this), filename(0)
{
}

csConfigDocument::csConfigDocument (const char *Filename, iVFS* vfs) 
  : scfImplementationType (this), filename(0), fileVFS(vfs)
{
  filename = csStrNew (Filename);
  csRef<iFile> file;
  if (vfs)
    file = vfs->Open (Filename, VFS_FILE_READ);
  else
    file.AttachNew (new csPhysicalFile (Filename, "rb"));

  csRef<iDocumentSystem> docsys (csPtr<iDocumentSystem> (
    new csTinyDocumentSystem()));
  csRef<iDocument> doc (docsys->CreateDocument ());
  doc->Parse (file, true);
  
  document = doc;
  ParseDocument (doc);
}

csConfigDocument::csConfigDocument (iDocument* doc) 
  : scfImplementationType (this), filename(0), document (doc)
{
  ParseDocument (doc);
}

csConfigDocument::csConfigDocument (iDocumentNode* node) 
  : scfImplementationType (this), filename(0)
{
  LoadNode (node);
}

csConfigDocument::~csConfigDocument()
{
  delete[] filename;
}

void csConfigDocument::ParseDocument (iDocument* doc, bool Merge, bool NewWins)
{
  csRef<iDocumentNode> root (doc->GetRoot ());
  csRef<iDocumentNode> config (root->GetNode ("config"));
  LoadNode (config, Merge, NewWins);
}

void csConfigDocument::ParseNode (const char* parent, iDocumentNode* node, 
				  bool NewWins)
{
  csString fullKey;
  csRef<iDocumentNode> lastComment;

  csRef<iDocumentNodeIterator> it (node->GetNodes ());
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child (it->Next ());
    if (child->GetType () == CS_NODE_COMMENT)
    {
      /*
        @@@ Doesn't handle comments spread over multiple children
	well.
       */
      lastComment = child;
      continue;
    }
    if (child->GetType () != CS_NODE_ELEMENT) continue;

    fullKey.Clear ();
    fullKey << parent << child->GetValue ();
    csString downKey (fullKey);
    downKey.Downcase ();

    if (NewWins || !keys.In ((const char*)downKey))
    {
      KeyInfo newInfo;
      newInfo.node = child;
      newInfo.comment = lastComment;
      newInfo.cachedStringValue = csStrNew (child->GetContentsValue ());
      newInfo.cachedComment = lastComment ? csStrNew (
	lastComment->GetContentsValue ()) : 0;
      newInfo.originalKey = csStrNew (fullKey);
      keys.PutUnique ((const char*)downKey, newInfo);
      lastComment = 0;
    }

    fullKey << '.';
    ParseNode (fullKey, child, NewWins);
  }
}

const char* csConfigDocument::GetFileName () const
{
  return filename ? filename : "#csConfigDocument";
}

iVFS* csConfigDocument::GetVFS () const
{
  return fileVFS;
}

void csConfigDocument::SetFileName (const char*, iVFS*)
{
}

bool csConfigDocument::Load (const char* /*iFileName*/, iVFS*, bool /*Merge*/,
  bool /*NewWins*/)
{
  return false;
}

bool csConfigDocument::LoadNode (iDocumentNode* node, bool Merge, bool NewWins)
{
  if (!Merge) keys.DeleteAll ();
  ParseNode ("", node, NewWins);
  return true;
}

bool csConfigDocument::Save ()
{
  return false;
}

bool csConfigDocument::Save (const char * /*iFileName*/, iVFS* /*vfs*/)
{
  return false;
}

void csConfigDocument::Clear ()
{
  keys.DeleteAll ();
}

csPtr<iConfigIterator> csConfigDocument::Enumerate (const char* Subsection)
{
  csRef<iConfigIterator> iter;
  iter.AttachNew (new csConfigDocumentIterator (this, Subsection));
  return csPtr<iConfigIterator> (iter);
}

bool csConfigDocument::KeyExists (const char *Key) const
{
  csString dcKey (Key); dcKey.Downcase ();
  if (keys.In ((const char*)dcKey))
  {
    const KeyInfo* info = keys.GetElementPointer ((const char*)dcKey);

    return (info && (info->cachedStringValue != 0));
  }
  return false;
}

bool csConfigDocument::SubsectionExists (const char *Subsection) const
{
  csString dcSubsection (Subsection); dcSubsection.Downcase ();
  return keys.In ((const char*)dcSubsection);
}

int csConfigDocument::GetInt (const char *Key, int Def) const
{
  csString dcKey (Key); dcKey.Downcase ();
  const KeyInfo* info = keys.GetElementPointer ((const char*)dcKey);
  const char* val = info ? info->cachedStringValue : 0;

  if (!val) return Def;
  
  int v = Def;
  sscanf (val, "%d", &v);
  return v;
}

float csConfigDocument::GetFloat (const char *Key, float Def) const
{
  csString dcKey (Key); dcKey.Downcase ();
  const KeyInfo* info = keys.GetElementPointer ((const char*)dcKey);
  const char* val = info ? info->cachedStringValue : 0;

  if (!val) return Def;
  
  float v = Def;
  sscanf (val, "%f", &v);
  return v;
}

const char* csConfigDocument::GetStr (const char *Key, const char* Def) const
{
  csString dcKey (Key); dcKey.Downcase ();
  const KeyInfo* info = keys.GetElementPointer ((const char*)dcKey);
  const char* val = info ? info->cachedStringValue : 0;
  if (!val) return Def;
  
  return val;
}

bool csConfigDocument::GetBool (const char *Key, bool Def) const
{
  csString dcKey (Key); dcKey.Downcase ();
  const KeyInfo* info = keys.GetElementPointer ((const char*)dcKey);
  const char* val = info ? info->cachedStringValue : 0;

  if (!val) return Def;
  
  return (
     strcasecmp(val, "true") == 0 ||
     strcasecmp(val, "yes" ) == 0 ||
     strcasecmp(val, "on"  ) == 0 ||
     strcasecmp(val, "1"   ) == 0);
}

const char* csConfigDocument::GetComment (const char *Key) const
{
  csString dcKey (Key); dcKey.Downcase ();
  const KeyInfo* info = keys.GetElementPointer ((const char*)dcKey);
  return info ? info->cachedComment : 0;
}

void csConfigDocument::SetStr (const char * /*Key*/, const char * /*Val*/)
{
}

void csConfigDocument::SetInt (const char * /*Key*/, int /*Value*/)
{
}

void csConfigDocument::SetFloat (const char * /*Key*/, float /*Value*/)
{
}

void csConfigDocument::SetBool (const char * /*Key*/, bool /*Value*/)
{
}

bool csConfigDocument::SetComment (const char * /*Key*/, const char * /*Text*/)
{
  return false;
}

void csConfigDocument::DeleteKey (const char * /*Key*/)
{
}

const char* csConfigDocument::GetEOFComment () const
{
  return 0;
}

void csConfigDocument::SetEOFComment (const char * /*Text*/)
{
}
