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

#ifndef _FOUNTLDR_H_
#define _FOUNTLDR_H_

#include "imap/reader.h"
#include "imap/writer.h"
#include "isys/plugin.h"

struct iEngine;
struct iSystem;
struct iPluginManager;
struct iObjectRegistry;

/**
 * Fountain factory loader.
 */
class csFountainFactoryLoader : public iLoaderPlugin
{
private:
  iSystem* sys;
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csFountainFactoryLoader (iBase*);

  /// Destructor.
  virtual ~csFountainFactoryLoader ();

  bool Initialize (iSystem* p);

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine, iBase* context);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csFountainFactoryLoader);
    virtual bool Initialize (iSystem* p) { return scfParent->Initialize (p); }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugin;
};

/**
 * Fountain factory saver.
 */
class csFountainFactorySaver : public iSaverPlugin
{
private:
  iSystem* sys;
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csFountainFactorySaver (iBase*);

  /// Destructor.
  virtual ~csFountainFactorySaver ();

  bool Initialize (iSystem* p);

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iStrVector *str, iEngine* engine);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csFountainFactorySaver);
    virtual bool Initialize (iSystem* p) { return scfParent->Initialize (p); }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugin;
};


/**
 * Fountain loader.
 */
class csFountainLoader : public iLoaderPlugin
{
private:
  iSystem* sys;
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csFountainLoader (iBase*);

  /// Destructor.
  virtual ~csFountainLoader ();

  bool Initialize (iSystem* p);

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine, iBase* context);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csFountainLoader);
    virtual bool Initialize (iSystem* p) { return scfParent->Initialize (p); }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugin;
};

/**
 * Fountain saver.
 */
class csFountainSaver : public iSaverPlugin
{
private:
  iSystem* sys;
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csFountainSaver (iBase*);

  /// Destructor.
  virtual ~csFountainSaver ();

  bool Initialize (iSystem* p);

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iStrVector *str, iEngine* engine);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csFountainSaver);
    virtual bool Initialize (iSystem* p) { return scfParent->Initialize (p); }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugin;
};

#endif // _FOUNTLDR_H_
