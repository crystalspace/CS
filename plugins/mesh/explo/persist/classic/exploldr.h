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

#ifndef _EXPLOLDR_H_
#define _EXPLOLDR_H_

#include "imap/reader.h"
#include "imap/writer.h"
#include "isys/plugin.h"

struct iEngine;
struct iSystem;

/**
 * Explosion factory loader.
 */
class csExplosionFactoryLoader : public iLoaderPlugin
{
private:
  iSystem* sys;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csExplosionFactoryLoader (iBase*);

  /// Destructor.
  virtual ~csExplosionFactoryLoader ();

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine, iBase* context);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csExplosionFactoryLoader);
    virtual bool Initialize (iSystem* p) { scfParent->sys = p; return true; }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugin;
};

/**
 * Explosion factory saver.
 */
class csExplosionFactorySaver : public iSaverPlugin
{
private:
  iSystem* sys;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csExplosionFactorySaver (iBase*);

  /// Destructor.
  virtual ~csExplosionFactorySaver ();

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iStrVector *str, iEngine* engine);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csExplosionFactorySaver);
    virtual bool Initialize (iSystem* p) { scfParent->sys = p; return true; }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugin;
};

/**
 * Explosion loader.
 */
class csExplosionLoader : public iLoaderPlugin
{
private:
  iSystem* sys;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csExplosionLoader (iBase*);

  /// Destructor.
  virtual ~csExplosionLoader ();

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine, iBase* context);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csExplosionLoader);
    virtual bool Initialize (iSystem* p) { scfParent->sys = p; return true; }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugin;
};

/**
 * Explosion saver.
 */
class csExplosionSaver : public iSaverPlugin
{
private:
  iSystem* sys;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csExplosionSaver (iBase*);
  /// Destructor.
  virtual ~csExplosionSaver ();
  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iStrVector *str, iEngine* engine);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csExplosionSaver);
    virtual bool Initialize (iSystem* p) { scfParent->sys = p; return true; }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugin;
};

#endif // _EXPLOLDR_H_

