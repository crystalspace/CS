/*
    Copyright (C) 2000 by Jorrit Tyberghein

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

#ifndef __MOTLDR_H__
#define __MOTLDR_H__

#include "imap/reader.h"
#include "imap/writer.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "csutil/parser.h"
#include "csutil/strhash.h"

class csVector3;
class csQuaternion;
struct iEngine;
struct iObjectRegistry;
struct iVFS;
struct iMotionManager;
struct iMotionTemplate;
struct iLoaderContext;
struct iObjectRegistry;
struct iSyntaxService;

/**
 * Motion Loader.
 */
class csMotionLoader : public iLoaderPlugin
{
private:
  iObjectRegistry *object_reg;
  iVFS *vfs;
  iMotionManager *motman;
  iSyntaxService *synldr;
  csStringHash xmltokens;

  bool load_transform (iDocumentNode* node, csVector3 &v, csQuaternion &q,
  	float& time);

public:
  SCF_DECLARE_IBASE;

  bool LoadBone (csParser* parser, iMotionTemplate* mot, int bone, char* buf);
  bool LoadBone (iDocumentNode* node, iMotionTemplate* mot, int bone);

  iMotionTemplate* LoadMotion ( const char *fname );
  bool LoadMotion (csParser* parser, iMotionTemplate *mot, char *buf );
  bool LoadMotion (iDocumentNode* node, iMotionTemplate *mot);

  /// Constructor
  csMotionLoader (iBase *);
  virtual ~csMotionLoader();
  virtual bool Initialize( iObjectRegistry *object_reg);
  virtual csPtr<iBase> Parse (const char* string, 
    iLoaderContext* ldr_context, iBase *context);

  /// Parse a given node and return a new object for it.
  virtual csPtr<iBase> Parse (iDocumentNode* node,
    iLoaderContext* ldr_context, iBase* context);

  void Report (int severity, const char* msg, ...);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csMotionLoader);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;
};

class csMotionSaver : public iSaverPlugin
{
private:
  iObjectRegistry *object_reg;

public:
  SCF_DECLARE_IBASE;

  csMotionSaver (iBase *);
  virtual ~csMotionSaver ();
  virtual void WriteDown ( iBase *obj, iFile *file);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csMotionSaver);
    virtual bool Initialize (iObjectRegistry* p)
    { scfParent->object_reg = p; return true; }
  } scfiComponent;
  friend struct eiComponent;
};

#endif
