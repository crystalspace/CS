#ifndef __ISYSG2D_H__
#define __ISYSG2D_H__

#include "cscom/com.h"

#include <DirectWindow.h>

extern const IID IID_IBeLibGraphicsInfo;

interface IBeLibGraphicsInfo : public IUnknown
{
  STDMETHOD(DirectConnect)(direct_buffer_info *info) = 0;
};

#endif
