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

#ifndef _FIRELDR_H_
#define _FIRELDR_H_

#include "imap/reader.h"
#include "imap/writer.h"
#include "isys/plugin.h"

struct iEngine;
struct iSystem;
struct iPluginManager;
struct iObjectRegistry;

/**
 * Fire factory loader.
 */
class csFireFactoryLoader : public iLoaderPlugin
{
private:
  iSystem* sys;
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csFireFactoryLoader (iBase*);

  /// Destructor.
  virtual ~csFireFactoryLoader ();

  bool Initialize (iSystem* p);

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine, iBase* context);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csFireFactoryLoader);
    virtual bool Initialize (iSystem* p) { return scfParent->Initialize (p); }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugin;
};

/**
 * Fire factory saver.
 */
class csFireFactorySaver : public iSaverPlugin
{
private:
  iSystem* sys;
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csFireFactorySaver (iBase*);

  /// Destructor.
  virtual ~csFireFactorySaver ();

  bool Initialize (iSystem* p);

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iStrVector *str, iEngine* engine);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csFireFactorySaver);
    virtual bool Initialize (iSystem* p) { return scfParent->Initialize (p); }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugin;
};

/**
 * Fire loader.
 */
class csFireLoader : public iLoaderPlugin
{
private:
  iSystem* sys;
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csFireLoader (iBase*);

  /// Destructor.
  virtual ~csFireLoader ();

  bool Initialize (iSystem* p);

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine, iBase* context);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csFireLoader);
    virtual bool Initialize (iSystem* p) { return scfParent->Initialize (p); }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugin;
};

/**
 * Fire saver.
 */
class csFireSaver : public iSaverPlugin
{
private:
  iSystem* sys;
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csFireSaver (iBase*);

  /// Destructor.
  virtual ~csFireSaver ();

  bool Initialize (iSystem* p);

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iStrVector *str, iEngine* engine);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csFireSaver);
    virtual bool Initialize (iSystem* p) { return scfParent->Initialize (p); }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugin;
};

#endif // _FIRELDR_H_

