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
  RepMaterial () : oldmat (0), newmat (0) { }
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

  ThingLoadInfo () : default_material (0), default_texlen (1) {}
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
  csRef<iMeshObjectType> thing_type;
  
  csStringHash xmltokens;

  /**
   * Parse a texture mapping specification.
   * <ul>
   * <li>vref: is the array containing vertices which can be referenced
   *     by indices in the description.
   * <li>texspec: describes the data found for the texture transformation.
   *     It consists of or'ed CSTEX_.
   * <li>tx_orig, tx1, tx2, len: texture transformation is given by 3
   *     points describing a 3d space (third vector is implicitly given to
   *     be perpendicular on the 2 vectors described by the 3 points),
   * <li>width and height of the texture.
   * <li>tx_m and tx_v: if texture transformation is given explicitly by
   *     matrix/vector.
   * <li>uv_shift: contains UV_SHIFT value.
   * <li>idx? and uv?: if texture mapping is given explicitly by defining
   *     the u,v coordinate that belongs to vertex idx? of the polygon.
   * <li>polyname: name of polygon to which this texture description belongs.
   *     This is used to make errormessages more verbose.
   * </ul>
   * \sa #CSTEX_UV
   */
  bool ParseTextureMapping (iDocumentNode* node,
  	const csVector3* vref, uint &texspec,
	csVector3 &tx_orig, csVector3 &tx1,
	csVector3 &tx2, csVector3 &len,
	csMatrix3 &tx_m, csVector3 &tx_v,
	csVector2 &uv_shift,
	int &idx1, csVector2 &uv1,
	int &idx2, csVector2 &uv2,
	int &idx3, csVector2 &uv3,
	const char *polyname);
  bool ParsePortal (
	iDocumentNode* node, iLoaderContext* ldr_context,
	uint32 &flags, bool &mirror, bool &warp, int& msv,
	csMatrix3 &m, csVector3 &before, csVector3 &after,
	iString* destSector, bool& autoresolve);
  // Parse a polygon. If 'poly_delete' is set to true the caller must delete
  // the polygon. This is used in case the polygon is only a portal.
  bool ParsePoly3d (iDocumentNode* node,
   	iLoaderContext* ldr_context,
  	iEngine* engine,
	float default_texlen,
	iThingFactoryState* thing_fact_state,
	int vt_offset, bool& poly_delete,
	iMeshWrapper* mesh, bool& baduv);

  bool LoadThingPart (iThingEnvironment* te,
  	iDocumentNode* node, iLoaderContext* ldr_context,
	iObjectRegistry* object_reg, iReporter* reporter,
	iSyntaxService *synldr, ThingLoadInfo& info,
	iEngine* engine, int vt_offset, bool isParent,
	iMeshWrapper* mesh, bool& baduv);

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
  csRef<iSyntaxService> synldr;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csThingSaver (iBase*);
  /// Destructor.
  virtual ~csThingSaver ();

  bool Initialize (iObjectRegistry* p);

  /// Write down given object and add to iDocumentNode.
  virtual bool WriteDown (iBase *obj, iDocumentNode* parent);
  virtual bool WriteFactory (iBase *obj, iDocumentNode* parent);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csThingSaver);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize (p); }
  } scfiComponent;
  friend struct eiComponent;
};

/**
 * Thing factory saver.
 */
class csThingFactorySaver : public csThingSaver
{
public:
  /// Constructor.
  csThingFactorySaver (iBase* parent) : csThingSaver (parent) {}
  /// Destructor.
  virtual ~csThingFactorySaver () {}

  /// Write down given factory and add to iDocumentNode.
  virtual bool WriteDown (iBase *obj, iDocumentNode* parent)
  {
    csRef<iDocumentNode> paramsNode = parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    paramsNode->SetValue("params");
    return WriteFactory (obj, paramsNode);
  };
};

#endif // __CS_THINGLDR_H__
