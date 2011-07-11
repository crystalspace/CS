#include <cssysdef.h>
#include "vplLoader.h"
#include <iutil/objreg.h>
#include <iutil/plugin.h>
#include <iostream>

using namespace std;

SCF_IMPLEMENT_FACTORY (vplLoader)

vplLoader::vplLoader (iBase* parent) :
scfImplementationType (this, parent),
object_reg(0)
{
}

vplLoader::~vplLoader ()
{
}

bool vplLoader::Initialize (iObjectRegistry* r)
{
  object_reg = r;

  csRef<iPluginManager> mgr=csQueryRegistry<iPluginManager> (object_reg);
  m_pThOggLoader=csLoadPlugin<iMediaLoader> (mgr,
    "crystalspace.vpl.element.thogg");

  return true;
}

csRef<iMediaContainer> vplLoader::LoadMedia (const char * pFileName, const char *pDescription, const char* pMediaType)
{
  if (strcmp (pMediaType,"AutoDetect") == 0)
  {
    csRef<iMediaContainer> data;
    if (m_pThOggLoader)
    {
      data=m_pThOggLoader->LoadMedia(pFileName,pDescription,pMediaType);

      if (data.IsValid ())
      {
        return data;
      }
    }
  }
  else if (strcmp (pMediaType,"TheoraVideo") == 0)
  {
    csRef<iMediaContainer> data;
    if (m_pThOggLoader)
    {
      data=m_pThOggLoader->LoadMedia (pFileName,pDescription,pMediaType);
      if (data.IsValid ())
      {
        return data;
      }
    }
  }

  return NULL;
}

