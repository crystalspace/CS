#include <cssysdef.h>
#include "vplLoader.h"
#include <iutil/objreg.h>
#include <iutil/plugin.h>
#include <iostream>

using namespace std;

SCF_IMPLEMENT_FACTORY (csVplLoader)

csVplLoader::csVplLoader (iBase* parent) :
scfImplementationType (this, parent),
_object_reg (0)
{
}

csVplLoader::~csVplLoader ()
{
}

bool csVplLoader::Initialize (iObjectRegistry* r)
{
  _object_reg = r;

  return true;
}

csRef<iMediaContainer> csVplLoader::LoadMedia (const char * pFileName, const char *pDescription)
{
  // Get the generic media xml parser
  csRef<iLoaderPlugin> parser = csQueryRegistry<iLoaderPlugin> (_object_reg);

  // Parse XML

  // Read the xml file and create the document
  csRef<iVFS> vfs = csQueryRegistry<iVFS> (_object_reg);
  csRef<iDocumentSystem> docSys = csPtr<iDocumentSystem> (new csTinyDocumentSystem ());
  csRef<iDocument> xmlDoc = docSys->CreateDocument ();
  csRef<iDataBuffer> xmlData = vfs->ReadFile (pFileName);

  // Start parsing
  if (xmlDoc->Parse (xmlData) == 0)
  {
    // Get the root
    csRef<iDocumentNode> node = xmlDoc->GetRoot ();

    // Tell the parser to parse the xml file
    csRef<iBase> result = parser->Parse (node,0,0,0);

    csRef<iMediaLoader> loader = scfQueryInterface<iMediaLoader> (result);
    return loader->LoadMedia (pFileName,pDescription);
  }
  else
  {
    csReport (_object_reg, CS_REPORTER_SEVERITY_ERROR, QUALIFIED_PLUGIN_NAME,
      "Failed to parse '%s'.\n", pFileName);
    return NULL;
  }

  return NULL;
}

