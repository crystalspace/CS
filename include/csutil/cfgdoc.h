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

#ifndef __CS_UTIL_CFGDOC_H__
#define __CS_UTIL_CFGDOC_H__

/**\file
 * Implementation for iConfigFile with configurations stored in iDocuments.
 */

#include "csextern.h"
#include "iutil/cfgfile.h"
#include "iutil/document.h"
#include "csutil/hash.h"

class csConfigDocumentIterator;

/**
 * iConfigFile implementation for configurations stored in documents.
 * \todo Write support
 */
class CS_CRYSTALSPACE_EXPORT csConfigDocument : public iConfigFile
{
  friend class csConfigDocumentIterator;

  char* filename;
  csRef<iDocument> document;
  csRef<iVFS> fileVFS;

  struct CS_CRYSTALSPACE_EXPORT KeyInfo
  {
    csRef<iDocumentNode> comment;
    csRef<iDocumentNode> node;
    char* cachedStringValue;
    char* cachedComment;
    char* originalKey;

    KeyInfo () : cachedStringValue(0), cachedComment(0), originalKey(0) {}
    KeyInfo (const KeyInfo& other)
    {
      cachedStringValue = csStrNew (other.cachedStringValue);
      cachedComment = csStrNew (other.cachedComment);
      originalKey = csStrNew (other.originalKey);
    }
    ~KeyInfo()
    {
      delete[] cachedStringValue;
      delete[] cachedComment;
      delete[] originalKey;
    }
  };
  csHash<KeyInfo, csStrKey> keys;

  void ParseDocument (iDocument* doc, bool Merge = false,
    bool NewWins = true);
  void ParseNode (const char* parent, iDocumentNode* node, 
    bool NewWins = true);
public:
  SCF_DECLARE_IBASE;
  
  csConfigDocument ();
  csConfigDocument (const char *Filename, iVFS* = 0);
  csConfigDocument (iDocument* doc);
  csConfigDocument (iDocumentNode* node);
  virtual ~csConfigDocument();
  
  virtual const char* GetFileName () const;

  virtual iVFS* GetVFS () const;

  virtual void SetFileName (const char*, iVFS*);

  virtual bool Load (const char* iFileName, iVFS* = 0, bool Merge = false,
    bool NewWins = true);
  bool LoadNode (iDocumentNode* node, bool Merge = false, bool NewWins = true);

  virtual bool Save ();

  virtual bool Save (const char *iFileName, iVFS* = 0);

  virtual void Clear ();

  virtual csPtr<iConfigIterator> Enumerate (const char *Subsection = 0);

  virtual bool KeyExists (const char *Key) const;
  virtual bool SubsectionExists (const char *Subsection) const;

  virtual int GetInt (const char *Key, int Def = 0) const;
  virtual float GetFloat (const char *Key, float Def = 0.0) const;
  virtual const char *GetStr (const char *Key, const char *Def = "") const;
  virtual bool GetBool (const char *Key, bool Def = false) const;
  virtual const char *GetComment (const char *Key) const;

  virtual void SetStr (const char *Key, const char *Val);
  virtual void SetInt (const char *Key, int Value);
  virtual void SetFloat (const char *Key, float Value);
  virtual void SetBool (const char *Key, bool Value);
  virtual bool SetComment (const char *Key, const char *Text);
  virtual void DeleteKey (const char *Key);
  virtual const char *GetEOFComment () const;
  virtual void SetEOFComment (const char *Text);
};

#endif // __CS_UTIL_CFGDOC_H__
