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

#ifndef _SPR2DLDR_H_
#define _SPR2DLDR_H_

#include "imap/reader.h"
#include "imap/writer.h"
#include "isys/plugin.h"

struct iEngine;
struct iSystem;
struct iReporter;

/**
 * Sprite 2D factory loader.
 */
class csSprite2DFactoryLoader : public iLoaderPlugin
{
private:
  iSystem* sys;
  iReporter* reporter;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csSprite2DFactoryLoader (iBase*);

  /// Destructor.
  virtual ~csSprite2DFactoryLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine, iBase* context);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSprite2DFactoryLoader);
    virtual bool Initialize (iSystem* p) { return scfParent->Initialize(p); }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
};

/**
 * Sprite2D factory saver.
 */
class csSprite2DFactorySaver : public iSaverPlugin
{
private:
  iSystem* sys;
  iReporter* reporter;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csSprite2DFactorySaver (iBase*);

  /// Destructor.
  virtual ~csSprite2DFactorySaver ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iStrVector *str, iEngine* engine);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSprite2DFactorySaver);
    virtual bool Initialize (iSystem* p) { return scfParent->Initialize(p); }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
};

/**
 * Sprite 2D loader.
 */
class csSprite2DLoader : public iLoaderPlugin
{
private:
  iSystem* sys;
  iReporter* reporter;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csSprite2DLoader (iBase*);

  /// Destructor.
  virtual ~csSprite2DLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine, iBase* context);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSprite2DLoader);
    virtual bool Initialize (iSystem* p) { return scfParent->Initialize(p); }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
};

/**
 * Sprite2D saver.
 */
class csSprite2DSaver : public iSaverPlugin
{
private:
  iSystem* sys;
  iReporter* reporter;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csSprite2DSaver (iBase*);

  /// Destructor.
  virtual ~csSprite2DSaver ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iStrVector *str, iEngine* engine);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSprite2DSaver);
    virtual bool Initialize (iSystem* p) { return scfParent->Initialize(p); }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
};

#endif // _SPR2DLDR_H_
