
#include "cscom/com.h"
#include "cs2d/beglide2/ig2d.h"

#ifndef __XG2XG2D_H__
#define __XG2XG2D_H__

/// csGraphics2DGlide's implementation of IGlide2xGraphicsInfo
class IXGlide2xGraphicsInfo : public IGlide2xGraphicsInfo
{
  DECLARE_IUNKNOWN()
//protected: 
//  AUTO_LONG m_cRef; 
//public: 
//  STDMETHOD (QueryInterface) (REFIID riid, void** ppv); 
//  STDMETHOD_(ULong, AddRef) (); 
//  STDMETHOD_(ULong, Release) ();
  STDMETHOD(Open)(char* szTitle);
  ///
  STDMETHOD(Close)();
  ///
  STDMETHOD(GetBDirectWindow)( CrystGlideWindow ** dpy );

};

#endif
