#ifndef __VPLLOADER_H__
#define __VPLLOADER_H__

#include <iutil/comp.h>
#include <videodecode/medialoader.h>
#include <videodecode/mediacontainer.h>
#include <videodecode/media.h>
#include <videodecode/vpl_structs.h>
#include <csutil/scf_implementation.h>

struct iObjectRegistry;

/**
* This is the implementation for our API and
* also the implementation of the plugin.
*/
class vplLoader : public scfImplementation2<vplLoader,iMediaLoader,iComponent>
{
private:
  iObjectRegistry* object_reg;
  
  /// Theora video loader interface
  csRef<iMediaLoader> m_pThOggLoader;

public:
  vplLoader (iBase* parent);
  virtual ~vplLoader ();

  // From iComponent.
  virtual bool Initialize (iObjectRegistry*);

  
  virtual csRef<iMediaContainer> LoadMedia (const char * pFileName, const char *pDescription=0, const char* pMediaType = "AutoDetect");
};

#endif // __VPLLOADER_H__