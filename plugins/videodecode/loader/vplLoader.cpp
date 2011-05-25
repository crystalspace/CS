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
  m_pThOggLoader=csLoadPlugin<iVPLLoader> (mgr,
    "crystalspace.vpl.element.thogg");

  return true;
}

csPtr<iVPLCodec> vplLoader::LoadVideo (const char * pFileName, const char *pDescription, VideoType type)
{
  if( type == VideoType::AutoDetect)
  {
    csRef<iVPLCodec> data;
    if(m_pThOggLoader)
    {
	  data=m_pThOggLoader->LoadVideo(pFileName,pDescription,type);
	  if(data.IsValid())
	    return csPtr<iVPLCodec> (data);
    }
  }
  else
  if( type == VideoType::Theora)
  {
    csRef<iVPLCodec> data;
    if(m_pThOggLoader)
    {
	  data=m_pThOggLoader->LoadVideo(pFileName,pDescription,type);
	  if(data.IsValid())
	   return csPtr<iVPLCodec> (data);
    }
  }

  return 0;
}

