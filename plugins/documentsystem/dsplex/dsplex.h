/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter

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

#include "csutil/scf.h"
#include "csutil/refarr.h"
#include "iutil/comp.h"
#include "iutil/document.h"

struct iObjectRegistry;

class csMplexDocumentSystem : public iDocumentSystem, public iComponent
{
private:
  friend struct csPlexDocument;

  csRef<iDocumentSystem> defaultDocSys;
  csRefArray<iDocumentSystem> orderedlist;
  csRefArray<iDocumentSystem> autolist;
  csRef<iStringArray> classlist;
  csRef<iPluginManager> plugin_mgr;

  csRef<iDocumentSystem> LoadNextPlugin (int num);
  void RewardPlugin (int num);
public:
  SCF_DECLARE_IBASE;
  
  csMplexDocumentSystem (iBase* parent = 0);
  virtual ~csMplexDocumentSystem ();
	
  virtual bool Initialize (iObjectRegistry* objreg);

  csRef<iDocument> CreateDocument ();
};

struct csPlexDocument : public iDocument
{
private:
  friend class csMplexDocumentSystem;

  csRef<csMplexDocumentSystem> plexer;

  csRef<iDocument> wrappedDoc;
  csString lasterr;
public:
  SCF_DECLARE_IBASE;

  csPlexDocument (csRef<csMplexDocumentSystem> aPlexer);
  virtual ~csPlexDocument ();

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

