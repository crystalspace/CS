/*
    Copyright (C) 2003 by Jorrit Tyberghein, Daniel Duhprey

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

#ifndef __CS_TERRAINLDR_H
#define __CS_TERRAINLDR_H

#include "imap/reader.h"
#include "imap/writer.h"
#include "iutil/comp.h"
#include "csutil/strhash.h"

struct iObjectRegistry;
struct iSyntaxService;
struct iVFS;

/**
 *
 */
class csTerrainFactoryLoader : public iLoaderPlugin
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;
  csRef<iVFS> vfs;
  csStringHash xmltokens;

public:
  SCF_DECLARE_IBASE;

  /// Constructor
  csTerrainFactoryLoader (iBase*);
  /// Destructor
  virtual ~csTerrainFactoryLoader ();

  /// Setup the plugin with the system driver
  bool Initialize (iObjectRegistry *objreg);

  /// Parse the given node block and build the terrain factory
  csPtr<iBase> Parse (iDocumentNode *node,
    iStreamSource*, iLoaderContext *ldr_context,
    iBase* context);	

  struct eiComponent : public iComponent
  { 
    SCF_DECLARE_EMBEDDED_IBASE (csTerrainFactoryLoader);
    virtual bool Initialize (iObjectRegistry *p)
    { return scfParent->Initialize (p); }
  } scfiComponent;
};

/**
 *
 */
class csTerrainFactorySaver : public iSaverPlugin
{
private:
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor
  csTerrainFactorySaver (iBase*);

  /// Destructor
  virtual ~csTerrainFactorySaver ();

  /// Register plugin with system driver
  bool Initialize (iObjectRegistry *objreg);

  /// Write down given object and add to iDocumentNode.
  virtual bool WriteDown (iBase *obj, iDocumentNode* parent,
  	iStreamSource*);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csTerrainFactorySaver);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize (p); }
  } scfiComponent;

};

/**
 *
 */
class csTerrainObjectLoader : public iLoaderPlugin
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;
  csRef<iVFS> vfs;
  csStringHash xmltokens;

  bool ParseMaterialPalette (iDocumentNode* node, iLoaderContext *ldr_context,
  	csArray<iMaterialWrapper*>& palette);
public:
  SCF_DECLARE_IBASE;

  /// Constructor
  csTerrainObjectLoader (iBase*);

  /// Destructor
  virtual ~csTerrainObjectLoader ();

  /// Register plugin with system driver
  bool Initialize (iObjectRegistry *objreg);

  /// Parse the given block to create a new Terrain object
  csPtr<iBase> Parse (iDocumentNode* node,
    iStreamSource*, iLoaderContext* ldr_context,
    iBase *context);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csTerrainObjectLoader);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize (p); }
  } scfiComponent;
};

/**
 *
 */
class csTerrainObjectSaver : public iSaverPlugin
{
private:
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor
  csTerrainObjectSaver (iBase*);

  /// Destructor
  virtual ~csTerrainObjectSaver ();

  /// Register plugin with the system driver
  bool Initialize (iObjectRegistry *objreg);

  /// Write down given object and add to iDocumentNode.
  virtual bool WriteDown (iBase *obj, iDocumentNode* parent,
  	iStreamSource*);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csTerrainObjectSaver);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize (p); }
  } scfiComponent;
};

#endif // __CS_CHUNKLDR_H
