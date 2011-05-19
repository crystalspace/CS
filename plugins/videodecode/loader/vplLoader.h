#ifndef __THOGGLOADER_H__
#define __THOGGLOADER_H__

#include <iutil/comp.h>
#include <videodecode/vpl_loader.h>
#include <videodecode/vpl_data.h>
#include <videodecode/vpl_structs.h>
#include <csutil/scf_implementation.h>

struct iObjectRegistry;

/**
* This is the implementation for our API and
* also the implementation of the plugin.
*/
class vplLoader : public scfImplementation2<vplLoader,iVPLLoader,iComponent>
{
private:
  iObjectRegistry* object_reg;
  
  /// Theora video loader interface
  csRef<iVPLLoader> m_pThOggLoader;

public:
  vplLoader (iBase* parent);
  virtual ~vplLoader ();

  // From iComponent.
  virtual bool Initialize (iObjectRegistry*);

  
  virtual csPtr<iVPLData> LoadSound (const char * pFileName, const char *pDescription=0);
};

#endif // __THOGGLOADER_H__