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

#ifndef __CS_THINGLDR_H__
#define __CS_THINGLDR_H__

#include "imap/reader.h"
#include "imap/writer.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "csutil/strhash.h"
#include "csutil/array.h"

struct iEngine;
struct iPluginManager;
struct iObjectRegistry;
struct iSyntaxService;
struct iReporter;
struct iThingState;
struct iThingFactoryState;
struct iMeshObject;
struct iMeshObjectType;

struct RepMaterial
{
  char* oldmat;
  char* newmat;
  RepMaterial () : oldmat (NULL), newmat (NULL) { }
  ~RepMaterial () { delete[] oldmat; delete[] newmat; }
};

/**
 * Private information during the loading process of a thing.
 */
class ThingLoadInfo
{
public:
  csRef<iMeshObjectType> type;
  csRef<iMeshObject> obj;
  csRef<iThingState> thing_state;
  csRef<iThingFactoryState> thing_fact_state;
  iMaterialWrapper* default_material;
  float default_texlen;
  bool load_factory;	// If true we are loading a factory.
  bool global_factory;	// We are using a global factory ('factory' or 'clone').
  csArray<RepMaterial> replace_materials;

  ThingLoadInfo () : default_material (NULL), default_texlen (1) {}
};

/**
 * Thing loader.
 */
class csThingLoader : public iLoaderPlugin
{
public:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;
  csRef<iReporter> reporter;
  
  csStringHash xmltokens;

  bool LoadThingPart (iThingEnvironment* te,
  	iDocumentNode* node, iLoaderContext* ldr_context,
	iObjectRegistry* object_reg, iReporter* reporter,
	iSyntaxService *synldr, ThingLoadInfo& info,
	iEngine* engine, int vt_offset, bool isParent);

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csThingLoader (iBase*);
  /// Destructor.
  virtual ~csThingLoader ();

  bool Initialize (iObjectRegistry* p);

  /// Parse a given node and return a new object for it.
  virtual csPtr<iBase> Parse (iDocumentNode* node,
    iLoaderContext* ldr_context, iBase* context);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csThingLoader);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize (p); }
  } scfiComponent;
  friend struct eiComponent;
};

/**
 * Thing factory loader.
 */
class csThingFactoryLoader : public csThingLoader
{
public:
  /// Constructor.
  csThingFactoryLoader (iBase* parent) : csThingLoader (parent) { }
  /// Destructor.
  virtual ~csThingFactoryLoader () { }

  /// Parse a given node and return a new object for it.
  virtual csPtr<iBase> Parse (iDocumentNode* node,
    iLoaderContext* ldr_context, iBase* context);
};

/**
 * Thing saver.
 */
class csThingSaver : public iSaverPlugin
{
private:
  iObjectRegistry* object_reg;
  csRef<iReporter> reporter;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csThingSaver (iBase*);
  /// Destructor.
  virtual ~csThingSaver ();

  bool Initialize (iObjectRegistry* p);

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iFile *file);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csThingSaver);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize (p); }
  } scfiComponent;
  friend struct eiComponent;
};

#endif // __CS_THINGLDR_H__
