/*  virtual iBase* Parse (const char* string, iEngine* engine);

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

#ifndef _RAINLDR_H_
#define _RAINLDR_H_

#include "imap/reader.h"
#include "imap/writer.h"
#include "isys/plugin.h"

struct iEngine;
struct iSystem;
struct iPluginManager;
struct iObjectRegistry;

/**
 * Rain factory loader.
 */
class csRainFactoryLoader : public iLoaderPlugin
{
private:
  iSystem* sys;
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csRainFactoryLoader (iBase*);

  /// Destructor.
  virtual ~csRainFactoryLoader ();

  bool Initialize (iSystem* p);

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine, iBase* context);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csRainFactoryLoader);
    virtual bool Initialize (iSystem* p) { return scfParent->Initialize (p); }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugin;
};

/**
 * Rain factory saver.
 */
class csRainFactorySaver : public iSaverPlugin
{
private:
  iSystem* sys;
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csRainFactorySaver (iBase*);

  /// Destructor.
  virtual ~csRainFactorySaver ();

  bool Initialize (iSystem* p);

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iStrVector *str, iEngine* engine);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csRainFactorySaver);
    virtual bool Initialize (iSystem* p) { return scfParent->Initialize (p); }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugin;
};


/**
 * Rain loader.
 */
class csRainLoader : public iLoaderPlugin
{
private:
  iSystem* sys;
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csRainLoader (iBase*);

  /// Destructor.
  virtual ~csRainLoader ();

  bool Initialize (iSystem* p);

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine, iBase* context);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csRainLoader);
    virtual bool Initialize (iSystem* p) { return scfParent->Initialize (p); }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugin;
};

/**
 * Rain saver.
 */
class csRainSaver : public iSaverPlugin
{
private:
  iSystem* sys;
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csRainSaver (iBase*);

  /// Destructor.
  virtual ~csRainSaver ();

  bool Initialize (iSystem* p);

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iStrVector *str, iEngine* engine);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csRainSaver);
    virtual bool Initialize (iSystem* p) { return scfParent->Initialize (p); }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugin;
};

#endif // _RAINLDR_H_
