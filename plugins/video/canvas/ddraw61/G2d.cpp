/*
    Copyright (C) 1998 by Jorrit Tyberghein

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "sysdef.h"
#include "csutil/scf.h"
#include "cs2d/ddraw61/g2d.h"
#include "cssys/win32/directdetection.h"
#include "isystem.h"

DirectDetection DDetection;
DirectDetectionDevice * DirectDevice;

void sys_fatalerror(char *str, HRESULT hRes = S_OK)
{
	LPVOID lpMsgBuf;
	char* szMsg;
	char szStdMessage[] = "Last Error: ";
	if (FAILED(hRes))
	{
		DWORD dwResult;
		dwResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
								 hRes,  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
							     (LPTSTR) &lpMsgBuf, 0, NULL );
	
		if (dwResult != 0)
		{
			szMsg = new char[strlen((const char*)lpMsgBuf) + strlen(str) + strlen(szStdMessage) + 1];
			strcpy( szMsg, str );
			strcat( szMsg, szStdMessage );
			strcat( szMsg, (const char*)lpMsgBuf );
			
			LocalFree( lpMsgBuf );
			
			MessageBox (NULL, szMsg, "Fatal Error in DirectDraw2DDX61.dll", MB_OK|MB_TOPMOST);
			delete szMsg;

			exit(1);
		}
	}

	MessageBox(NULL, str, "Fatal Error in DirectDraw2DDX61.dll", MB_OK|MB_TOPMOST);
	
	exit(1);
}

/////The 2D Graphics Driver//////////////

IMPLEMENT_FACTORY (csGraphics2DDDraw6)

EXPORT_CLASS_TABLE (ddraw6)
  EXPORT_CLASS (csGraphics2DDDraw6, "crystalspace.graphics2d.direct3d.dx6",
    "DirectDraw DX6.1 2D graphics driver for Crystal Space")
EXPORT_CLASS_TABLE_END

IMPLEMENT_IBASE (csGraphics2DDDraw6)
  IMPLEMENTS_INTERFACE (iPlugIn)
  IMPLEMENTS_INTERFACE (iGraphics2D)
  IMPLEMENTS_INTERFACE (iGraphics2DDDraw6)
IMPLEMENT_IBASE_END

///// Windowed-mode palette stuff //////

struct {
  WORD Version;
  WORD NumberOfEntries;
  PALETTEENTRY aEntries[256];
} SysPalette = 
{
  0x300,
    256
};

HPALETTE hWndPalette=NULL;

void ClearSystemPalette()
{
  struct 
  {
    WORD Version;
    WORD nEntries;
    PALETTEENTRY aEntries[256];
  } Palette =
  {
    0x300,
      256
  };
  
  HPALETTE BlackPal, OldPal;
  HDC hdc;
  
  int c;
  
  for(c=0; c<256; c++)
  {
    Palette.aEntries[c].peRed = 0;
    Palette.aEntries[c].peGreen = 0;
    Palette.aEntries[c].peBlue = 0;
    Palette.aEntries[c].peFlags = PC_NOCOLLAPSE;
  }
  
  hdc = GetDC(NULL);
  
  BlackPal = CreatePalette((LOGPALETTE *)&Palette);
  
  OldPal = SelectPalette(hdc,BlackPal,FALSE);
  RealizePalette(hdc);
  SelectPalette(hdc, OldPal, FALSE);
  DeleteObject(BlackPal);
  
  ReleaseDC(NULL, hdc);
}


void CreateIdentityPalette(RGBpaletteEntry *p)
{
  int i;
  struct {
    WORD Version;
    WORD nEntries;
    PALETTEENTRY aEntries[256];
  } Palette = 
  {
    0x300,
      256
  };
  
  if(hWndPalette)
    DeleteObject(hWndPalette);
  
  Palette.aEntries[0].peFlags = 0;
  Palette.aEntries[0].peFlags = 0;
  
  for(i=1; i<255; i++)
  {
    Palette.aEntries[i].peRed = p[i].red;
    Palette.aEntries[i].peGreen = p[i].green;
    Palette.aEntries[i].peBlue = p[i].blue;
    Palette.aEntries[i].peFlags = PC_RESERVED;
  }
  
  hWndPalette = CreatePalette((LOGPALETTE *)&Palette);
  
  if(!hWndPalette) 
    sys_fatalerror("Error creating identity palette.");
}

extern DirectDetection DDetection;
extern DirectDetectionDevice * DirectDevice;

csGraphics2DDDraw6::csGraphics2DDDraw6(iBase *iParent) : 
  csGraphics2D (),
  m_hWnd(NULL),
  m_bDisableDoubleBuffer(false),
  m_bPaletteChanged(false),
  m_bPalettized(false),
  m_lpDD(NULL),
  m_lpddClipper(NULL),
  m_lpddPal(NULL),
  m_lpddsBack(NULL),
  m_lpddsPrimary(NULL),
  m_nActivePage(0),
  m_nDepth(-1),
  m_nGraphicsReady(true),
  m_bLocked(false),
  m_bUses3D(true)
{
  CONSTRUCT_IBASE (iParent);
}

bool csGraphics2DDDraw6::Initialize(iSystem *pSystem)
{
  DDSURFACEDESC2 ddsd;
  HRESULT ddrval;
  DDPIXELFORMAT ddpf;
  //DDCAPS ddcaps;
  LPGUID pGuid = NULL;
  //iWin32SystemDriver* piWin32System;

  // Call original Initialize() function
  if (!csGraphics2D::Initialize(pSystem))
    return false;

  iWin32SystemDriver* piWin32System = QUERY_INTERFACE (System, iWin32SystemDriver);
  // Get the creation parameters //
  if (piWin32System)
  {
      m_hInstance = piWin32System->GetInstance ();
      m_nCmdShow  = piWin32System->GetCmdShow ();
      piWin32System->DecRef ();
  }
  else
  {
      sys_fatalerror ("csGraphics2DDDraw6::Open(QI) -- iSystem passed does not support iWin32SystemDriver.");
  }

  System->GetSettings(Width, Height, m_nDepth, FullScreen);

  // Create the DirectDraw device //
  if (!m_bUses3D)
  {
      DDetection.checkDevices2D();
      DirectDevice = DDetection.findBestDevice2D();
  }
  else
  {
      DDetection.checkDevices3D();
      DirectDevice = DDetection.findBestDevice3D(FullScreen);
  }
  
  if (DirectDevice == NULL)
    sys_fatalerror("csGraphics2DDDraw6::Open(DirectDevice) -- Error creating DirectDevice.");
  
  if (!DirectDevice->IsPrimary2D)
    pGuid = &DirectDevice->Guid2D;
  
  // create a DD object for either the primary device or the secondary. //
/* DAN: commented this out until i allow this class to print to iSystem.  
  if(!pGuid)
    message("Use the primary DirectDraw device : %s (%s)\n", DirectDevice->DeviceName2D, DirectDevice->DeviceDescription2D);
  else 
    message("Use a secondary DirectDraw device : %s (%s)\n", DirectDevice->DeviceName2D, DirectDevice->DeviceDescription2D); */
  
  // Create DD Object 
  ddrval = DirectDrawCreate (pGuid, &m_lpDD, NULL);
  if (ddrval != DD_OK)
    sys_fatalerror("csGraphics2DDDraw6::Open(DirectDrawCreate) Can't create DirectDraw device", ddrval);
  
  // Set cooperative level
  ddrval = m_lpDD->SetCooperativeLevel (NULL, DDSCL_NORMAL);
  if (ddrval != DD_OK)
    sys_fatalerror("Error setting normal cooperative mode.", ddrval);
  
  // here can be set the new DDSCL_FPUSETUP setting to speed up rendering

  ddrval = m_lpDD->QueryInterface(IID_IDirectDraw4, (LPVOID *)&m_lpDD4);
  if(ddrval != DD_OK)
    sys_fatalerror("Cannot get DirectDraw4 interface");

  // create a temporary surface
  memset (&ddsd, 0, sizeof (ddsd));
  ddsd.dwSize = sizeof (ddsd);
  ddsd.dwFlags = DDSD_CAPS;
  ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
  
  ddrval = m_lpDD4->CreateSurface (&ddsd, &m_lpddsPrimary, NULL);
  if (ddrval != DD_OK)
    sys_fatalerror("Cannot create primary surface for DirectDraw", ddrval);
  
  // get the pixel format
  memset(&ddpf, 0, sizeof(ddpf));
  ddpf.dwSize = sizeof(ddpf);
  ddrval = m_lpddsPrimary->GetPixelFormat(&ddpf);
  if (ddrval != DD_OK)
    sys_fatalerror("Cannot get pixel format descriptor.", ddrval);
  

  int RedMask   = 0x00FF0000;
  int GreenMask = 0x0000FF00;
  int BlueMask  = 0x000000FF;

  // automatically determine bit-depth for windowed mode //
  
  if (!FullScreen)
  {
    if(ddpf.dwFlags & DDPF_PALETTEINDEXED8)
      m_nDepth=8;
    else if(ddpf.dwRGBBitCount == 16)
      m_nDepth=16;
	else if(ddpf.dwRGBBitCount == 32)
	  m_nDepth=32;
    else
    {
      MessageBox(NULL,"Desktop display depth has to be set in either 8-bit, 16-bit or 32-bit mode to display in a window, or use full screen mode instead.", "Fatal Error", MB_OK | MB_ICONEXCLAMATION|MB_TOPMOST);
      exit(1);
    }
  }
  
  // I'm not sure whether the following stuff is nessecary to work
  // set 16bpp mode up //
  
  if (m_nDepth==16)
  {
    // desktop depth has to be set to 16bpp for DEPTH 16 to work.
    if(ddpf.dwRGBBitCount != 16)
    {
      MessageBox(NULL, "Desktop display depth has to be set to 16bpp for DEPTH 16 to work.", "Fatal Error", MB_OK | MB_ICONEXCLAMATION|MB_TOPMOST);
      exit(1);
    }
    
    _DrawPixel = DrawPixel16;   _WriteChar = WriteChar16;
    _GetPixelAt = GetPixelAt16; _DrawSprite = DrawSprite16;
    
    // calculate CS's pixel format structure.
    pfmt.PixelBytes = 2;
    pfmt.PalEntries = 0;
    pfmt.RedMask    = ddpf.dwRBitMask;
    pfmt.GreenMask  = ddpf.dwGBitMask;
    pfmt.BlueMask   = ddpf.dwBBitMask;
    
    complete_pixel_format();
  }

  // set 32-bpp-mode up

  if (m_nDepth==32)
  {
  /*  // desktop depth has to be set to 16bpp for DEPTH 16 to work.
    if(ddpf.dwRGBBitCount != 32)
    {
      MessageBox(NULL, "Desktop display depth has to be set to 32bpp for DEPTH 32 to work.", "Fatal Error", MB_OK | MB_ICONEXCLAMATION|MB_TOPMOST);
      exit(1);
    }
    */
    _DrawPixel = DrawPixel32;   _WriteChar = WriteChar32;
    _GetPixelAt = GetPixelAt32; _DrawSprite = DrawSprite32;
      
	RedMask = 0xff << 16;
	GreenMask = 0xff << 8;
	BlueMask = 0xff;
	// calculate CS's pixel format structure.
    pfmt.PixelBytes = 4;
    pfmt.PalEntries = 0;
   // pfmt.RedMask    = ddpf.dwRBitMask;
   // pfmt.GreenMask  = ddpf.dwGBitMask;
   // pfmt.BlueMask   = ddpf.dwBBitMask;
	pfmt.RedMask   = RedMask;
	pfmt.GreenMask = GreenMask;
	pfmt.BlueMask  = BlueMask;
    
    complete_pixel_format();
  }

  // release the temporary surface
  m_lpddsPrimary->Release(); m_lpddsPrimary = NULL;
  
  // message("Using %d bits per pixel (%d color mode).\n", m_nDepth, 1 << m_nDepth);

  m_nActivePage = 0;
  m_bDisableDoubleBuffer = false;

  return true;
}

csGraphics2DDDraw6::~csGraphics2DDDraw6(void)
{
  Close();
  m_nGraphicsReady=0;
}

bool csGraphics2DDDraw6::Open(char *Title)
{
  if (!csGraphics2D::Open (Title))
    return false;
  
  DDSURFACEDESC2 ddsd;
  DDSCAPS2 ddscaps;
  HRESULT ddrval;
  
  // create the window.
  DWORD exStyle = 0;
  DWORD style = WS_POPUP;
  if (!FullScreen)
	  style |= WS_CAPTION;
  
  int wwidth,wheight;
  wwidth=Width+2*GetSystemMetrics(SM_CXSIZEFRAME);
  wheight=Height+2*GetSystemMetrics(SM_CYSIZEFRAME)+GetSystemMetrics(SM_CYCAPTION);
  
  m_hWnd = CreateWindowEx(exStyle, WINDOWCLASSNAME, Title, style,
	                      (GetSystemMetrics(SM_CXSCREEN)-wwidth)/2,
                          (GetSystemMetrics(SM_CYSCREEN)-wheight)/2,
                          wwidth, wheight, NULL, NULL, m_hInstance, NULL );
  if( !m_hWnd )
    sys_fatalerror("Cannot create CrystalSpace window", GetLastError());
  
  ShowWindow( m_hWnd, m_nCmdShow );
  UpdateWindow( m_hWnd );
  SetFocus( m_hWnd );
  
  Memory=NULL;
  
  // set cooperative level.
  
  if (FullScreen)
  {
    ddrval = m_lpDD4->SetCooperativeLevel (m_hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN );
    if(ddrval != DD_OK)
      sys_fatalerror("Cannot use fullscreen in this mode with this device");
  }
  else
  {
    ddrval = m_lpDD4->SetCooperativeLevel (m_hWnd, DDSCL_NORMAL);
    if(ddrval != DD_OK)
      sys_fatalerror("Cannot use windowed mode in this mode with this device");
  }
  
  ddrval = m_lpDD->QueryInterface(IID_IDirectDraw4, (LPVOID *)&m_lpDD4);
  if(ddrval != DD_OK)
    sys_fatalerror("Cannot get DirectDraw4 interface");

  // create objects for fullscreen mode.
  
  if (FullScreen)
  {
    ddrval = m_lpDD4->SetDisplayMode (Width, Height, m_nDepth, 0, 0);
    if(ddrval != DD_OK)
      sys_fatalerror("Invalid display resolution!");
    
    memset (&ddsd, 0, sizeof (ddsd));
    ddsd.dwSize = sizeof (ddsd);
    ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
    ddsd.dwBackBufferCount = 1;
    
    // set flags if this is a 3d device
    if(!DirectDevice->Only2D)
      ddsd.ddsCaps.dwCaps |= DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY;
    
	// create backbuffer

    ddrval = m_lpDD4->CreateSurface (&ddsd, &m_lpddsPrimary, NULL);
    if (ddrval != DD_OK)
      sys_fatalerror("Cannot create primary surface for DirectDraw");
    
    ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
    ddrval = m_lpddsPrimary->GetAttachedSurface (&ddscaps, &m_lpddsBack);
    
    if (ddrval != DD_OK)
      sys_fatalerror("Cannot attach primary surface to DirectDraw context");
  }
  else
  {
    
    // create objects for windowed mode.
    
    memset (&ddsd, 0, sizeof (ddsd));
    ddsd.dwSize = sizeof (ddsd);
    ddsd.dwFlags = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
    ddrval = m_lpDD4->CreateSurface (&ddsd, &m_lpddsPrimary, NULL);
    if (ddrval != DD_OK)
      sys_fatalerror("Cannot create primary surface for DirectDraw");
    
    memset (&ddsd, 0, sizeof (ddsd));
    ddsd.dwSize = sizeof (ddsd);
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH ;
    ddsd.dwWidth = Width;
    ddsd.dwHeight = Height;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
    
    if(!DirectDevice->Only2D)
      ddsd.ddsCaps.dwCaps |= DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY;
    else 
      ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
    
    ddrval = m_lpDD4->CreateSurface (&ddsd, &m_lpddsBack, NULL);
    if (ddrval != DD_OK)
      sys_fatalerror("Cannot attach primary surface to DirectDraw context");
    
    HRESULT hRes;
    hRes = DirectDrawCreateClipper(0UL, &m_lpddClipper, NULL);
    if (FAILED(hRes)) 
      sys_fatalerror("Cannot create clipper object.");
    
    hRes = m_lpddClipper->SetHWnd(0UL, m_hWnd);
    if (FAILED(hRes))
      sys_fatalerror("Cannot set clipper m_hWnd.");
    
    hRes = m_lpddsPrimary->SetClipper(m_lpddClipper);
    if (FAILED(hRes))
      sys_fatalerror("Cannot set primary surface clipper.");
  }
  
  m_lpddsBack->GetSurfaceDesc (&ddsd);
  
  for(int i = 0; i < Height; i++)
    LineAddress [i] = i * ddsd.lPitch;
  
  if(m_nDepth==8) m_bPalettized = true;
  else m_bPalettized = false;

  m_bPaletteChanged = false;

  return true;
}

void csGraphics2DDDraw6::Close(void)
{
  if(m_lpddPal)
  {
    m_lpddPal->Release();
    m_lpddPal=NULL;
  }
  
  if( m_lpDD4 != NULL )
  {
    if( m_lpddsPrimary != NULL )
    {
      m_lpddsPrimary->Release();
      m_lpddsPrimary = NULL;
    }
    
    if(m_lpddClipper)
    {
      m_lpddClipper->Release();
      m_lpddClipper=NULL;
    }
    
    m_lpDD4->Release();
    m_lpDD4 = NULL;
  }
  
  if(!FullScreen)
  {
    // restore the original system palette.
    HDC dc = GetDC(NULL);
    SetSystemPaletteUse(dc, SYSPAL_STATIC);
    PostMessage(HWND_BROADCAST, WM_SYSCOLORCHANGE, 0, 0);
    ReleaseDC(NULL, dc);
  }
  csGraphics2D::Close ();
}

int csGraphics2DDDraw6::GetPage ()
{
  return m_nActivePage;
}

bool csGraphics2DDDraw6::DoubleBuffer (bool Enable)
{
  if (Enable) m_bDisableDoubleBuffer = false;
  else m_bDisableDoubleBuffer = true;
  return true;
}

bool csGraphics2DDDraw6::DoubleBuffer ()
{
  return false;
}

void csGraphics2DDDraw6::Print (csRect* /*area*/)
{
  RECT r={0,0,Width,Height};
  POINT	pt;
  HRESULT ddrval;
  
  while( 1 )
  {
    if (FullScreen)
    {
      ddrval = m_lpddsPrimary->Flip( NULL, 0 );
      if( ddrval == DDERR_SURFACELOST ) 
      { 
        ddrval = m_lpddsPrimary->Restore();
	if( m_lpddsBack ) 
        { 
          if( m_lpddsBack->IsLost() != DD_OK ) 
            m_lpddsBack->Restore(); 
	} 
      }
    }
    else
    {
      GetClientRect (m_hWnd, &r);
      pt.x = pt.y = 0;
      ClientToScreen (m_hWnd, &pt);
      
      r.left = pt.x;
      r.top = pt.y;
      
      r.right += pt.x;
      r.bottom += pt.y;
      
      HDC hdc = 0;
      HPALETTE oldPal = 0;
      
      if(m_bPalettized)
      {
        hdc = GetDC(m_hWnd);
        
        oldPal = SelectPalette(hdc, hWndPalette, FALSE);
        RealizePalette(hdc);
      }
      
      ddrval = m_lpddsPrimary->Blt(&r, m_lpddsBack, NULL, DDBLT_WAIT, NULL);
      
      if(m_bPalettized)
        SelectPalette(hdc, oldPal, FALSE);
    }
    if( ddrval == DD_OK ) break;
    
    if( ddrval == DDERR_SURFACELOST )
    {
      ddrval = RestoreAll();
      if( ddrval != DD_OK ) break;
    }
    if( ddrval != DDERR_WASSTILLDRAWING ) break;
  }
}

HRESULT csGraphics2DDDraw6::RestoreAll()
{
  HRESULT ddrval;
  
  ddrval = m_lpddsPrimary->Restore();
  return ddrval;
}

unsigned char *csGraphics2DDDraw6::LockBackBuf()
{
  DDSURFACEDESC2 ddsd;
  ddsd.lpSurface = NULL;

  HRESULT ret=DDERR_WASSTILLDRAWING;

  if (m_bLocked)
  {
      m_lpddsBack->Unlock(NULL);
      m_bLocked = false;      
  }
  
  ddsd.dwSize = sizeof(ddsd);
  while (ret== DDERR_WASSTILLDRAWING)
    ret=m_lpddsBack->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR, NULL);
  
  if (ret!=DD_OK)
	  sys_fatalerror("There was an error locking the DirectDraw surface.", ret);

  m_bLocked = true;

  return ret==DD_OK ? (unsigned char *)ddsd.lpSurface : NULL;
}

void csGraphics2DDDraw6::FinishDraw ()
{
  csGraphics2D::FinishDraw ();
  if (FrameBufferLocked)
    return;

  m_lpddsBack->Unlock(NULL);
  m_bLocked = false;
  Memory = NULL;
  if (m_nActivePage == 0)
    m_nActivePage = 1;
  else
    m_nActivePage = 0;
}

void csGraphics2DDDraw6::SetColorPalette()
{
  HRESULT ret;
  
  if ((m_nDepth==8) && m_bPaletteChanged)
  {
    m_bPaletteChanged = false;

    if (m_lpddPal)
    {
      m_lpddPal->Release ();
      m_lpddPal = NULL;
    }
    
    ret=m_lpDD4->CreatePalette(DDPCAPS_8BIT, (PALETTEENTRY *)Palette, &m_lpddPal, NULL);
    if(ret==DD_OK) m_lpddsPrimary->SetPalette(m_lpddPal);
    
    if(!FullScreen)
    {
      HPALETTE oldPal;
      HDC dc = GetDC(NULL);
      
      SetSystemPaletteUse(dc, SYSPAL_NOSTATIC);
      PostMessage(HWND_BROADCAST, WM_SYSCOLORCHANGE, 0, 0);
      
      CreateIdentityPalette(Palette);
      ClearSystemPalette();
      
      oldPal = SelectPalette(dc, hWndPalette, FALSE);
      
      RealizePalette(dc);
      SelectPalette(dc, oldPal, FALSE);
      ReleaseDC(NULL, dc);
    }
  }
}

bool csGraphics2DDDraw6::BeginDraw()
{
  csGraphics2D::BeginDraw ();
  if (FrameBufferLocked != 1)
    return true;

  if (m_bDisableDoubleBuffer)
    Print (NULL);
  Memory = LockBackBuf();
  return (Memory != NULL);
}

void csGraphics2DDDraw6::SetRGB(int i, int r, int g, int b)
{
  csGraphics2D::SetRGB (i, r, g, b);
  m_bPaletteChanged = true;
}

bool csGraphics2DDDraw6::SetMouseCursor (csMouseCursorID iShape, iTextureHandle *hBitmap)
{
  (void)hBitmap;
  switch(iShape)
  {
    case csmcNone: SetCursor(NULL); break;
    case csmcArrow: SetCursor(LoadCursor (NULL, IDC_ARROW)); break;
    case csmcMove: SetCursor(LoadCursor (NULL, IDC_SIZEALL)); break;
    case csmcSizeNWSE: SetCursor(LoadCursor (NULL, IDC_SIZENWSE)); break;
    case csmcSizeNESW: SetCursor(LoadCursor (NULL, IDC_SIZENESW)); break;
    case csmcSizeNS: SetCursor(LoadCursor (NULL, IDC_SIZENS)); break;
    case csmcSizeEW: SetCursor(LoadCursor (NULL, IDC_SIZEWE)); break;
    case csmcStop: SetCursor(LoadCursor (NULL, IDC_NO)); break;
    case csmcWait: SetCursor(LoadCursor (NULL, IDC_WAIT)); break;
    default: return false;
  }
  return true;
}

bool csGraphics2DDDraw6::SetMousePosition (int x, int y)
{
  POINT p;
  p.x = x;
  p.y = y;

  ClientToScreen (m_hWnd, &p);
  ::SetCursorPos (p.x, p.y);
  
  return true;
}

void csGraphics2DDDraw6::GetDirectDrawDriver (LPDIRECTDRAW4* lplpDirectDraw)
{
  *lplpDirectDraw = m_lpDD4;
}

void csGraphics2DDDraw6::GetDirectDrawPrimary (LPDIRECTDRAWSURFACE4* lplpDirectDrawPrimary)
{
  *lplpDirectDrawPrimary = m_lpddsPrimary;
}

void csGraphics2DDDraw6::GetDirectDrawBackBuffer (LPDIRECTDRAWSURFACE4* lplpDirectDrawBackBuffer)
{
  *lplpDirectDrawBackBuffer = m_lpddsBack;
}

extern DirectDetectionDevice* DirectDevice;
void csGraphics2DDDraw6::GetDirectDetection (IDirectDetectionInternal** lplpDDetection)
{
  *lplpDDetection = static_cast<IDirectDetectionInternal*>(DirectDevice);
}
