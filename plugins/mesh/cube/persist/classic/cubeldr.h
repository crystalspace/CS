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
struct iSystem;

/**
 * Cube factory loader.
 */
class csCubeFactoryLoader : public iLoaderPlugIn
{
private:
  iSystem* sys;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csCubeFactoryLoader (iBase*);

  /// Destructor.
  virtual ~csCubeFactoryLoader ();

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine, iBase *context);

  struct eiPlugIn : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csCubeFactoryLoader);
    virtual bool Initialize (iSystem* p) { scfParent->sys = p; return true; }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugIn;
};

/**
 * Cube factory saver.
 */
class csCubeFactorySaver : public iSaverPlugIn
{
private:
  iSystem* sys;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csCubeFactorySaver (iBase*);

  /// Destructor.
  virtual ~csCubeFactorySaver ();

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iStrVector *str, iEngine* engine);

  struct eiPlugIn : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csCubeFactorySaver);
    virtual bool Initialize (iSystem* p) { scfParent->sys = p; return true; }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugIn;
};

/**
 * Cube loader.
 */
class csCubeLoader : public iLoaderPlugIn
{
private:
  iSystem* sys;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csCubeLoader (iBase*);

  /// Destructor.
  virtual ~csCubeLoader ();

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine, iBase* context);

  struct eiPlugIn : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csCubeLoader);
    virtual bool Initialize (iSystem* p) { scfParent->sys = p; return true; }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugIn;
};

/**
 * Cube saver.
 */
class csCubeSaver : public iSaverPlugIn
{
private:
  iSystem* sys;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csCubeSaver (iBase*);

  /// Destructor.
  virtual ~csCubeSaver ();

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iStrVector *str, iEngine* engine);

  struct eiPlugIn : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csCubeSaver);
    virtual bool Initialize (iSystem* p) { scfParent->sys = p; return true; }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugIn;
};

#endif // _CUBELDR_H_
