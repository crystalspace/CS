#ifndef __ISYSG2D_H__
#define __ISYSG2D_H__

#include "cscom/com.h"

#ifndef _WINDOW_H
#include <Window.h>
#endif
#ifndef _VIEW_H
#include <View.h>
#endif

extern const IID IID_IBeLibGraphicsInfo;

/// IXLibGraphicsInfo interface -- for XLib-specific properties.
interface IBeLibGraphicsInfo : public IUnknown
{
  ///
  STDMETHOD(GetDisplay)( BView** dpy ) = 0;
};

#endif
