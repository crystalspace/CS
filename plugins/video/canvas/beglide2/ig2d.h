
#include "cscom/com.h"

#ifndef __IG2XG2D_H__
#define __IG2XG2D_H__

extern const IID IID_IGlide2xGraphicsInfo;

/// IGlide2xGraphicsInfo interface -- for Glide-specific properties.
interface IGlide2xGraphicsInfo : public IUnknown
{
  ///
  STDMETHOD(Open)(char* szTitle) = 0;
  ///
  STDMETHOD(Close)() = 0;
   
  STDMETHOD(GetBDirectWindow)( CrystGlideWindow ** win ) = 0;
    
};

#endif

