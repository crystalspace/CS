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
#include "iutil/eventh.h"
#include "iutil/comp.h"

struct iEngine;
struct iPluginManager;
struct iObjectRegistry;

/**
 * Fire factory loader.
 */
class csFireFactoryLoader : public iLoaderPlugin
{
private:
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csFireFactoryLoader (iBase*);

  /// Destructor.
  virtual ~csFireFactoryLoader ();

  bool Initialize (iObjectRegistry* p);

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iLoaderContext* ldr_context,
  	iBase* context);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csFireFactoryLoader);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize (p); }
  } scfiComponent;
  friend struct eiComponent;
};

/**
 * Fire factory saver.
 */
class csFireFactorySaver : public iSaverPlugin
{
private:
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csFireFactorySaver (iBase*);

  /// Destructor.
  virtual ~csFireFactorySaver ();

  bool Initialize (iObjectRegistry* p);

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iFile *file);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csFireFactorySaver);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize (p); }
  } scfiComponent;
  friend struct eiComponent;
};

/**
 * Fire loader.
 */
class csFireLoader : public iLoaderPlugin
{
private:
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csFireLoader (iBase*);

  /// Destructor.
  virtual ~csFireLoader ();

  bool Initialize (iObjectRegistry* p);

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iLoaderContext* ldr_context,
  	iBase* context);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csFireLoader);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize (p); }
  } scfiComponent;
  friend struct eiComponent;
};

/**
 * Fire saver.
 */
class csFireSaver : public iSaverPlugin
{
private:
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csFireSaver (iBase*);

  /// Destructor.
  virtual ~csFireSaver ();

  bool Initialize (iObjectRegistry* p);

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iFile *file);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csFireSaver);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize (p); }
  } scfiComponent;
  friend struct eiComponent;
};

#endif // _FIRELDR_H_

