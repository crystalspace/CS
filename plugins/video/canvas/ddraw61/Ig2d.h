#ifndef __IDD6G2D_H__
#define __IDD6G2D_H__

#include <ddraw.h>
#include "csutil/scf.h"
#include "cssys/win32/iddetect.h"

/// iGraphics2DDDraw6 interface -- for Win32-specific properties.
SCF_INTERFACE (iGraphics2DDDraw6, 0, 0, 1) : public iBase
{
  ///
  virtual void GetDirectDrawDriver (LPDIRECTDRAW4* lplpDirectDraw) = 0;
  ///
  virtual void GetDirectDrawPrimary (LPDIRECTDRAWSURFACE4* lplpDirectDrawPrimary) = 0;
  ///
  virtual void GetDirectDrawBackBuffer (LPDIRECTDRAWSURFACE4* lplpDirectDrawBackBuffer) = 0;
  ///
  virtual void GetDirectDetection (IDirectDetectionInternal** lplpDDetection) = 0;
  ///
  virtual void SetColorPalette (void) = 0;
};

#endif
