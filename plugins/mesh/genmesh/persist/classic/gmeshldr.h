/*
    Copyright (C) 2002 by Jorrit Tyberghein

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

#ifndef _GMESHLDR_H_
#define _GMESHLDR_H_

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
 * General Mesh factory loader.
 */
class csGeneralFactoryLoader : public iLoaderPlugin
{
private:
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;
  iReporter* reporter;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csGeneralFactoryLoader (iBase*);

  /// Destructor.
  virtual ~csGeneralFactoryLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, 
    iLoaderContext* ldr_context, iBase* context);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csGeneralFactoryLoader);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;
};

/**
 * General Mesh factory saver.
 */
class csGeneralFactorySaver : public iSaverPlugin
{
private:
  iReporter* reporter;
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csGeneralFactorySaver (iBase*);

  /// Destructor.
  virtual ~csGeneralFactorySaver ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iFile *file);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csGeneralFactorySaver);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;
};

/**
 * General Mesh loader.
 */
class csGeneralMeshLoader : public iLoaderPlugin
{
private:
  iReporter* reporter;
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;
  iSyntaxService *synldr;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csGeneralMeshLoader (iBase*);

  /// Destructor.
  virtual ~csGeneralMeshLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, 
    iLoaderContext* ldr_context, iBase* context);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csGeneralMeshLoader);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;
};

/**
 * General Mesh saver.
 */
class csGeneralMeshSaver : public iSaverPlugin
{
private:
  iReporter* reporter;
  iPluginManager* plugin_mgr;
  iObjectRegistry* object_reg;
  iSyntaxService *synldr;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csGeneralMeshSaver (iBase*);

  /// Destructor.
  virtual ~csGeneralMeshSaver ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iFile *file);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csGeneralMeshSaver);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;
};

#endif // _GMESHLDR_H_

