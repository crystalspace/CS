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

#ifndef _BALLLDR_H_
#define _BALLLDR_H_

#include "imap/reader.h"
#include "imap/writer.h"
#include "isys/plugin.h"

struct iEngine;
struct iSystem;
struct iReporter;
struct iPluginManager;
struct iObjectRegistry;

/**
 * Ball factory loader.
 */
class csBallFactoryLoader : public iLoaderPlugin
{
private:
  iSystem* sys;
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;
  iReporter* reporter;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csBallFactoryLoader (iBase*);

  /// Destructor.
  virtual ~csBallFactoryLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine, iBase* context);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csBallFactoryLoader);
    virtual bool Initialize (iSystem* p) { return scfParent->Initialize(p); }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
};

/**
 * Ball factory saver.
 */
class csBallFactorySaver : public iSaverPlugin
{
private:
  iSystem* sys;
  iReporter* reporter;
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csBallFactorySaver (iBase*);

  /// Destructor.
  virtual ~csBallFactorySaver ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iStrVector *str, iEngine* engine);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csBallFactorySaver);
    virtual bool Initialize (iSystem* p) { return scfParent->Initialize(p); }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
};

/**
 * Ball loader.
 */
class csBallLoader : public iLoaderPlugin
{
private:
  iSystem* sys;
  iReporter* reporter;
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csBallLoader (iBase*);

  /// Destructor.
  virtual ~csBallLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine, iBase* context);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csBallLoader);
    virtual bool Initialize (iSystem* p) { return scfParent->Initialize(p); }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
};

/**
 * Ball saver.
 */
class csBallSaver : public iSaverPlugin
{
private:
  iSystem* sys;
  iReporter* reporter;
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csBallSaver (iBase*);

  /// Destructor.
  virtual ~csBallSaver ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iStrVector *str, iEngine* engine);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csBallSaver);
    virtual bool Initialize (iSystem* p) { return scfParent->Initialize(p); }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
};

#endif // _BALLLDR_H_
