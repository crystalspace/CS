/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein

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

#ifndef __CS_TERRFLDR_H__
#define __CS_TERRFLDR_H__

#include "imap/reader.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "csutil/strhash.h"

struct iEngine;
struct iPluginManager;
struct iObjectRegistry;
struct iReporter;
struct iSyntaxService;

/**
 * TerrFunc factory loader.
 */
class csTerrFuncFactoryLoader : public iLoaderPlugin
{
private:
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csTerrFuncFactoryLoader (iBase*);
  /// Destructor.
  virtual ~csTerrFuncFactoryLoader ();

  bool Initialize (iObjectRegistry* p);

  /// Parse a given node and return a new object for it.
  virtual csPtr<iBase> Parse (iDocumentNode* node,
    iLoaderContext* ldr_context, iBase* context);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csTerrFuncFactoryLoader);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize (p); }
  } scfiComponent;
  friend struct eiComponent;
};

/**
 * TerrFunc loader.
 */
class csTerrFuncLoader : public iLoaderPlugin
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;
  csRef<iReporter> reporter;
  csStringHash xmltokens;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csTerrFuncLoader (iBase*);
  /// Destructor.
  virtual ~csTerrFuncLoader ();

  bool Initialize (iObjectRegistry* p);

  /// Parse a given node and return a new object for it.
  virtual csPtr<iBase> Parse (iDocumentNode* node,
    iLoaderContext* ldr_context, iBase* context);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csTerrFuncLoader);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize (p); }
  } scfiComponent;
  friend struct eiComponent;
};

#endif // __CS_TERRFLDR_H__
