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

#ifndef _TERRFLDR_H_
#define _TERRFLDR_H_

#include "imap/reader.h"
#include "isys/plugin.h"

struct iEngine;
struct iSystem;
struct iPluginManager;
struct iObjectRegistry;

/**
 * TerrFunc factory loader.
 */
class csTerrFuncFactoryLoader : public iLoaderPlugin
{
private:
  iSystem* pSystem;
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csTerrFuncFactoryLoader (iBase*);
  /// Destructor.
  virtual ~csTerrFuncFactoryLoader ();

  bool Initialize (iSystem* p);

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine, iBase* context);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csTerrFuncFactoryLoader);
    virtual bool Initialize (iSystem* p) { return scfParent->Initialize (p); }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugin;
};

/**
 * TerrFunc loader.
 */
class csTerrFuncLoader : public iLoaderPlugin
{
private:
  iSystem* pSystem;
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csTerrFuncLoader (iBase*);
  /// Destructor.
  virtual ~csTerrFuncLoader ();

  bool Initialize (iSystem* p);

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine, iBase* context);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csTerrFuncLoader);
    virtual bool Initialize (iSystem* p) { return scfParent->Initialize (p); }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugin;
};

#endif // _TERRFLDR_H_
