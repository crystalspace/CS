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

#ifndef _SURFLDR_H_
#define _SURFLDR_H_

#include "imap/reader.h"
#include "imap/writer.h"
#include "isys/plugin.h"

struct iEngine;
struct iSystem;

/**
 * Surf factory loader.
 */
class csSurfFactoryLoader : public iLoaderPlugin
{
private:
  iSystem* sys;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csSurfFactoryLoader (iBase*);
  /// Destructor.
  virtual ~csSurfFactoryLoader ();

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine, iBase* context);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSurfFactoryLoader);
    virtual bool Initialize (iSystem* p) { scfParent->sys = p; return true; }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugin;
};

/**
 * Surf factory saver.
 */
class csSurfFactorySaver : public iSaverPlugIn
{
private:
  iSystem* sys;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csSurfFactorySaver (iBase*);
  /// Destructor.
  virtual ~csSurfFactorySaver ();

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iStrVector *str, iEngine* engine);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSurfFactorySaver);
    virtual bool Initialize (iSystem* p) { scfParent->sys = p; return true; }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugin;
};

/**
 * Surf loader.
 */
class csSurfLoader : public iLoaderPlugin
{
private:
  iSystem* sys;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csSurfLoader (iBase*);
  /// Destructor.
  virtual ~csSurfLoader ();

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine, iBase* context);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSurfLoader);
    virtual bool Initialize (iSystem* p) { scfParent->sys = p; return true; }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugin;
};

/**
 * Surf saver.
 */
class csSurfSaver : public iSaverPlugIn
{
private:
  iSystem* sys;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csSurfSaver (iBase*);
  /// Destructor.
  virtual ~csSurfSaver ();

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iStrVector *str, iEngine* engine);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSurfSaver);
    virtual bool Initialize (iSystem* p) { scfParent->sys = p; return true; }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugin;
};

#endif // _SURFLDR_H_

