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
#include "iutil/eventh.h"
#include "iutil/comp.h"

struct iEngine;
struct iReporter;
struct iPluginManager;
struct iObjectRegistry;
struct iSyntaxService;

/**
 * Ball factory loader.
 */
class csBallFactoryLoader : public iLoaderPlugin
{
private:
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
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iLoaderContext* ldr_context,
  	iBase* context);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csBallFactoryLoader);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;
};

/**
 * Ball factory saver.
 */
class csBallFactorySaver : public iSaverPlugin
{
private:
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
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iStrVector *str);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csBallFactorySaver);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;
};

/**
 * Ball loader.
 */
class csBallLoader : public iLoaderPlugin
{
private:
  iReporter* reporter;
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;
  iSyntaxService *synldr;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csBallLoader (iBase*);

  /// Destructor.
  virtual ~csBallLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iLoaderContext* ldr_context,
  	iBase* context);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csBallLoader);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;
};

/**
 * Ball saver.
 */
class csBallSaver : public iSaverPlugin
{
private:
  iReporter* reporter;
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;
  iSyntaxService *synldr;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csBallSaver (iBase*);

  /// Destructor.
  virtual ~csBallSaver ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iStrVector *str);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csBallSaver);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;
};

#endif // _BALLLDR_H_
