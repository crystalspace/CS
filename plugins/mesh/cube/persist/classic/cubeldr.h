/*
    Copyright (C) 2000 by Jorrit Tyberghein
    Copyright (C) 2001 by W.C.A. Wijngaards

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

#ifndef _CUBELDR_H_
#define _CUBELDR_H_

#include "imap/reader.h"
#include "imap/writer.h"
#include "isys/plugin.h"

struct iEngine;
struct iPluginManager;
struct iObjectRegistry;

/**
 * Cube factory loader.
 */
class csCubeFactoryLoader : public iLoaderPlugin
{
private:
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csCubeFactoryLoader (iBase*);

  /// Destructor.
  virtual ~csCubeFactoryLoader ();

  bool Initialize (iObjectRegistry* p);

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine, iBase *context);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csCubeFactoryLoader);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize (p); }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugin;
};

/**
 * Cube factory saver.
 */
class csCubeFactorySaver : public iSaverPlugin
{
private:
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csCubeFactorySaver (iBase*);

  /// Destructor.
  virtual ~csCubeFactorySaver ();

  bool Initialize (iObjectRegistry* p);

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iStrVector *str, iEngine* engine);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csCubeFactorySaver);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize (p); }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugin;
};

/**
 * Cube loader.
 */
class csCubeLoader : public iLoaderPlugin
{
private:
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csCubeLoader (iBase*);

  /// Destructor.
  virtual ~csCubeLoader ();

  bool Initialize (iObjectRegistry* p);

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine, iBase* context);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csCubeLoader);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize (p); }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugin;
};

/**
 * Cube saver.
 */
class csCubeSaver : public iSaverPlugin
{
private:
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csCubeSaver (iBase*);

  /// Destructor.
  virtual ~csCubeSaver ();

  bool Initialize (iObjectRegistry* p);

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iStrVector *str, iEngine* engine);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csCubeSaver);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize (p); }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugin;
};

#endif // _CUBELDR_H_
