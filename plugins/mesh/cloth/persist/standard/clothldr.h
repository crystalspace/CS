
#ifndef __CS_CLOTHLDR_H__
#define __CS_CLOTHLDR_H__

#include "imap/reader.h"
#include "imap/writer.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "csutil/strhash.h"

struct iEngine;
struct iReporter;
struct iPluginManager;
struct iObjectRegistry;
struct iSyntaxService;
struct iMeshObjectType;
struct iClothMeshState;
struct csTriangle;
	
class csClothFactoryLoader : public iLoaderPlugin
{
private:
  iObjectRegistry* object_reg;
  csRef<iReporter> reporter;
  csRef<iSyntaxService> synldr;
  csRef<iMeshObjectType> type;
  csStringHash xmltokens;
  
public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csClothFactoryLoader (iBase*);

  /// Destructor.
  virtual ~csClothFactoryLoader ();

/*
bool csClothFactoryLoader::ParsePolygonAndTriangulate (  
                               iDocumentNode*       node,
                               iLoaderContext*      ldr_context,
                               iEngine*             engine,
                               csTriangle*          TriangleBuffer,
                               uint*                TriangleOffset,
	                           float                default_texlen,
	                           iClothFactoryState*  state );
*/

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Parse a given string and return a new object for it.
  virtual csPtr<iBase> Parse (const char* string, iLoaderContext* ldr_context, iBase* context) {};

  /// Parse a given node and return a new object for it.
  virtual csPtr<iBase> Parse (iDocumentNode* node, iLoaderContext* ldr_context, iBase* context);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csClothFactoryLoader);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;
};

//   cloth mesh object loader

class csClothMeshLoader : public iLoaderPlugin
{
private:
  iObjectRegistry* object_reg;
  csRef<iReporter> reporter;
  csRef<iSyntaxService> synldr;
  csStringHash xmltokens;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csClothMeshLoader (iBase*);

  /// Destructor.
  virtual ~csClothMeshLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Parse a given string and return a new object for it.
  virtual csPtr<iBase> Parse (const char* string, iLoaderContext* ldr_context, iBase* context) {};

  /// Parse a given node and return a new object for it.
  virtual csPtr<iBase> Parse (iDocumentNode* node, iLoaderContext* ldr_context, iBase* context);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csClothMeshLoader);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;
};


/**
 * Cloth Mesh factory saver.
 */
class csClothFactorySaver : public iSaverPlugin
{
private:
  iObjectRegistry* object_reg;
  csRef<iReporter> reporter;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csClothFactorySaver (iBase*);

  /// Destructor.
  virtual ~csClothFactorySaver ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iFile *file);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csClothFactorySaver);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;
};

/*
class csClothObjectSaver : public iSaverPlugin
{
private:
  iObjectRegistry* object_reg;
  csRef<iReporter> reporter;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csClothObjectSaver (iBase*);

  /// Destructor.
  virtual ~csClothObjectSaver ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iFile *file);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csClothObjectSaver);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;
};
*/

#endif // __CS_CLOTHLDR_H__
