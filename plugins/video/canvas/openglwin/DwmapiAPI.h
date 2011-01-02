/*
 * DWMAPI stuff (resp. the subset we need).
 * dwmapi.dll is only available on Vista+, so load dynamically in every case.
 */
#ifndef __DWMAPIAPI_H__
#define __DWMAPIAPI_H__

#define DWM_BB_ENABLE			0x01
#define DWM_BB_BLURREGION		0x02
#define DWM_BB_TRANSITIONONMAXIMIZED	0x04

struct DWM_BLURBEHIND
{
  DWORD	dwFlags;
  BOOL	fEnable;
  HRGN	hRgnBlur;
  BOOL	fTransitionOnMaximized;
};

struct dwmMARGINS
{
  int cxLeftWidth;
  int cxRightWidth;
  int cyTopHeight;
  int cyBottomHeight;
};

#ifndef WM_DWMCOMPOSITIONCHANGED
#define WM_DWMCOMPOSITIONCHANGED  0x031e
#endif

#define CS_API_NAME		Dwmapi
#define CS_API_FUNCTIONS	"plugins/video/canvas/openglwin/Dwmapi.fun"
// Empty CS_API_EXPORT -> don't import or export from DLL
#define CS_API_EXPORT

#include "csutil/win32/APIdeclare.inc"

#endif // __DWMAPIAPI_H__
