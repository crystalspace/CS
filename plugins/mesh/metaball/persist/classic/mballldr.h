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
#include "imap/services.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"

struct iEngine;
struct iPluginManager;
struct iObjectRegistry;

/**
 * MetaBall factory loader.
 */
class csMetaBallFactoryLoader : public iLoaderPlugin
{
private:
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csMetaBallFactoryLoader (iBase*);

  /// Destructor.
  virtual ~csMetaBallFactoryLoader ();

  bool Initialize (iObjectRegistry* p);

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, 
    iLoaderContext* ldr_context, iBase* context);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csMetaBallFactoryLoader);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize (p); }
  } scfiComponent;
  friend struct eiComponent;
};

/**
 * MetaBall factory saver.
 */
class csMetaBallFactorySaver : public iSaverPlugin
{
private:
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csMetaBallFactorySaver (iBase*);

  /// Destructor.
  virtual ~csMetaBallFactorySaver ();

  bool Initialize (iObjectRegistry* p);

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iFile *file);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csMetaBallFactorySaver);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize (p); }
  } scfiComponent;
  friend struct eiComponent;
};

/**
 * MetaBall loader.
 */
class csMetaBallLoader : public iLoaderPlugin
{
private:
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;
  iSyntaxService* synldr;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csMetaBallLoader (iBase*);

  /// Destructor.
  virtual ~csMetaBallLoader ();

  bool Initialize (iObjectRegistry* p);

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, 
    iLoaderContext* ldr_context, iBase* context);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csMetaBallLoader);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize (p); }
  } scfiComponent;
  friend struct eiComponent;
};

/**
 * MetaBall saver.
 */
class csMetaBallSaver : public iSaverPlugin
{
private:
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;
  iSyntaxService* synldr;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csMetaBallSaver (iBase*);

  /// Destructor.
  virtual ~csMetaBallSaver ();

  bool Initialize (iObjectRegistry* p);

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iFile *file);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csMetaBallSaver);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize (p); }
  } scfiComponent;
  friend struct eiComponent;
};

#endif // _BALLLDR_H_
