#ifndef __THOGGLOADER_H__
#define __THOGGLOADER_H__

#include <iutil/comp.h>
#include <ividplay/vpl_loader.h>
#include <ividplay/vpl_structs.h>
#include <csutil/scf_implementation.h>
#include "thoggData.h"

struct iObjectRegistry;

/**
* This is the implementation for our API and
* also the implementation of the plugin.
*/
class thoggLoader : public scfImplementation2<thoggLoader,iVPLLoader,iComponent>
{
private:
  iObjectRegistry* object_reg;

public:
  thoggLoader (iBase* parent);
  virtual ~thoggLoader ();

  // From iComponent.
  virtual bool Initialize (iObjectRegistry*);

  
  virtual csPtr<iVPLData> LoadSound (const char * pFileName, const char *pDescription=0);
};

#endif // __THOGGLOADER_H__