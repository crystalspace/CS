/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#ifndef _THINGLDR_H
#define _THINGLDR_H

#include "imap/reader.h"
#include "imap/writer.h"
#include "isys/plugin.h"

struct iEngine;
struct iSystem;
struct iPluginManager;
struct iObjectRegistry;

/**
 * Thing loader.
 */
class csThingLoader : public iLoaderPlugin
{
private:
  iSystem* sys;
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csThingLoader (iBase*);
  /// Destructor.
  virtual ~csThingLoader ();

  bool Initialize (iSystem* p);

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine, iBase* context);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csThingLoader);
    virtual bool Initialize (iSystem* p) { return scfParent->Initialize (p); }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugin;
};

/**
 * Thing saver.
 */
class csThingSaver : public iSaverPlugin
{
private:
  iSystem* sys;
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csThingSaver (iBase*);
  /// Destructor.
  virtual ~csThingSaver ();

  bool Initialize (iSystem* p);

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iStrVector *str, iEngine* engine);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csThingSaver);
    virtual bool Initialize (iSystem* p) { return scfParent->Initialize (p); }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugin;
};

/**
 * Plane loader.
 */
class csPlaneLoader : public iLoaderPlugin
{
private:
  iSystem* sys;
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csPlaneLoader (iBase*);
  /// Destructor.
  virtual ~csPlaneLoader ();

  bool Initialize (iSystem* p);

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine, iBase* context);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csPlaneLoader);
    virtual bool Initialize (iSystem* p) { return scfParent->Initialize (p); }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugin;
};

/**
 * Plane saver.
 */
class csPlaneSaver : public iSaverPlugin
{
private:
  iSystem* sys;
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csPlaneSaver (iBase*);
  /// Destructor.
  virtual ~csPlaneSaver ();

  bool Initialize (iSystem* p);

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iStrVector *str, iEngine* engine);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csPlaneSaver);
    virtual bool Initialize (iSystem* p) { return scfParent->Initialize (p); }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugin;
};

/**
 * Bezier template loader.
 */
class csBezierLoader : public iLoaderPlugin
{
private:
  iSystem* sys;
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csBezierLoader (iBase*);
  /// Destructor.
  virtual ~csBezierLoader ();

  bool Initialize (iSystem* p);

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine, iBase* context);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csBezierLoader);
    virtual bool Initialize (iSystem* p) { return scfParent->Initialize (p); }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugin;
};

/**
 * Bezier template saver.
 */
class csBezierSaver : public iSaverPlugin
{
private:
  iSystem* sys;
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csBezierSaver (iBase*);
  /// Destructor.
  virtual ~csBezierSaver ();

  bool Initialize (iSystem* p);

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iStrVector *str, iEngine* engine);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csBezierSaver);
    virtual bool Initialize (iSystem* p) { return scfParent->Initialize (p); }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugin;
};

#endif // _THINGLDR_H
