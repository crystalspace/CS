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

#include "cssysdef.h"
#include "csutil/scf.h"
#include "csutil/csstring.h"
#include "video/canvas/ddraw/g2d.h"
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

IMPLEMENT_FACTORY (csGraphics2DDDraw3)

EXPORT_CLASS_TABLE (csddraw)
  EXPORT_CLASS (csGraphics2DDDraw3, SOFTWARE_2D_DRIVER,
    "Crystal Space 2D DirectDraw driver")
EXPORT_CLASS_TABLE_END

IMPLEMENT_IBASE (csGraphics2DDDraw3)
  IMPLEMENTS_INTERFACE (iPlugIn)
  IMPLEMENTS_INTERFACE (iGraphics2D)
  IMPLEMENTS_INTERFACE (iGraphics2DDDraw3)
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


bool CreateIdentityPalette(RGBPixel *p)
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
		return false;
	return true;

}

extern DirectDetection DDetection;
extern DirectDetectionDevice * DirectDevice;

csGraphics2DDDraw3::csGraphics2DDDraw3(iBase *iParent) :
  csGraphics2D (),
  m_hWnd(NULL),
  m_bPaletteChanged(false),
  m_bPalettized(false),
  m_lpDD(NULL),
  m_lpddPal(NULL),
  m_lpddsBack(NULL),
  m_lpddsPrimary(NULL),
  m_nActivePage(0),
  m_nGraphicsReady(true),
  m_bLocked(false),
  m_piWin32System(NULL),
  m_bUses3D(false),
	m_bWindowed(true),
	m_bReady(false)
{
  CONSTRUCT_IBASE (iParent);
}

csGraphics2DDDraw3::~csGraphics2DDDraw3(void)
{
  m_piWin32System->DecRef ();
  Close();
  m_nGraphicsReady=0;
}

bool csGraphics2DDDraw3::Initialize (iSystem *pSystem)
{
  if (!csGraphics2D::Initialize(pSystem))
    return false;

  // QI for iWin32SystemDriver //
  m_piWin32System = QUERY_INTERFACE (System, iWin32SystemDriver);
	ASSERT(m_piWin32System);

  // Get the creation parameters //
  m_hInstance = m_piWin32System->GetInstance();
  m_nCmdShow  = m_piWin32System->GetCmdShow();

  System->GetSettings(Width, Height, Depth, FullScreen);

  return true;
}

bool csGraphics2DDDraw3::Open(const char *Title)
{
  HRESULT ddrval;
	LPGUID pGuid = NULL;

  if (!csGraphics2D::Open (Title))
    return false;
  
  /*ASSERT(Title);
  if (*Title == '\x01')
  {
    //Having a title, that starts with '\x01' hast a special meaning. It means, we 
    //will attach to an existing window, rather than creating an own window.
    m_hWnd     = (HWND)(atol(Title+1));
    FullScreen = FALSE;
  }
  else
  {
	*/
    // create the window.
    DWORD exStyle = 0;
    DWORD style = WS_POPUP;
    if (!FullScreen)
      style |= WS_CAPTION;
  
    int wwidth,wheight;
    wwidth=Width+2*GetSystemMetrics(SM_CXSIZEFRAME);
    wheight=Height+2*GetSystemMetrics(SM_CYSIZEFRAME)+GetSystemMetrics(SM_CYCAPTION);
  
    m_hWnd = CreateWindowEx(exStyle, WINDOWCLASSNAME/*"CrystalWindow"*/, Title, style,
                          (GetSystemMetrics(SM_CXSCREEN)-wwidth)/2,
                            (GetSystemMetrics(SM_CYSCREEN)-wheight)/2,
                            wwidth, wheight, NULL, NULL, m_hInstance, NULL );
		ASSERT(m_hWnd);
  
    ShowWindow( m_hWnd, m_nCmdShow );
    UpdateWindow( m_hWnd );
    SetFocus( m_hWnd );

    // Save the window size/pos for switching modes
    GetWindowRect(m_hWnd, &m_rcWindow);

		//Get ahold of the main DirectDraw object...
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
		{
			InitFail(m_hWnd, false, "csGraphics2DDDraw3::Open(DirectDevice) -- Error creating DirectDevice.");
			return false;
		}

		if (!DirectDevice->IsPrimary2D)
			pGuid = &DirectDevice->Guid2D;
  
		// create a DD object for either the primary device or the secondary. //
		if(!pGuid)
			System->Printf(MSG_INITIALIZATION, "Use the primary DirectDraw device\n");
		else 
			System->Printf(MSG_INITIALIZATION, "Use a secondary DirectDraw device : %s (%s)\n", DirectDevice->DeviceName2D, DirectDevice->DeviceDescription2D);

		// Create DD Object 
		ddrval = DirectDrawCreate (pGuid, &m_lpDD, NULL);
		if (ddrval != DD_OK)
		{
			InitFail(m_hWnd, ddrval, "DirectDrawCreateEx FAILED");
			return false;
		}
  //}
  
  Memory=NULL;

	ddrval = InitSurfaces(m_hWnd);
	if (ddrval != DD_OK)
		return false;
	
	m_bReady = TRUE;
	return true;
}

void csGraphics2DDDraw3::Close(void)
{
  if(m_lpddPal)
  {
    m_lpddPal->Release();
    m_lpddPal=NULL;
  }
  
  if( m_lpDD != NULL )
  {
    if( m_lpddsPrimary != NULL )
    {
      m_lpddsPrimary->Release();
      m_lpddsPrimary = NULL;
    }
    
    m_lpDD->Release();
    m_lpDD = NULL;
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

int csGraphics2DDDraw3::GetPage ()
{
  return m_nActivePage;
}

bool csGraphics2DDDraw3::DoubleBuffer (bool Enable)
{
	Enable;
  return true;
}

bool csGraphics2DDDraw3::GetDoubleBufferState ()
{
  return true;
}

void csGraphics2DDDraw3::Print (csRect* /*area*/)
{
  HRESULT hRet;
  /*
	TODO:
		Use csRect for our Viewport in the blt operation????
	*/

	if (m_bReady)
	{
		while( 1 )
		{

			// If we are in windowed mode, perform a blt.
 			if (m_bWindowed)
			{
				if(m_bPalettized)
				{
					HDC      hdc;
					HPALETTE oldPal;

					hdc = GetDC(m_hWnd);
        
					oldPal = SelectPalette(hdc, hWndPalette, FALSE);
					RealizePalette(hdc);

					hRet = m_lpddsPrimary->Blt(&m_rcScreen, m_lpddsBack, 
																		 NULL/*&m_rcViewport*/, DDBLT_WAIT, NULL);

					SelectPalette(hdc, oldPal, FALSE);
				}
				else
				{

					hRet = m_lpddsPrimary->Blt(&m_rcScreen, m_lpddsBack, 
																		 NULL/*&m_rcViewport*/, DDBLT_WAIT, NULL);
				}
			}
			else
			{
					// Else we are in fullscreen mode, so perform a flip.
					hRet = m_lpddsPrimary->Flip( NULL, 0L );
			}
			
			if (hRet == DD_OK )
					break;
			if (hRet == DDERR_SURFACELOST )
			{
					hRet = m_lpddsPrimary->Restore();
					if (hRet != DD_OK )
						break;
			}
			if (hRet != DDERR_WASSTILLDRAWING )
					break;
		}
	}
}
/*
HRESULT csGraphics2DDDraw3::RestoreAll()
{
  HRESULT ddrval;
  
  ddrval = m_lpddsPrimary->Restore();
  return ddrval;
}
*/

unsigned char *csGraphics2DDDraw3::LockBackBuf()
{
  DDSURFACEDESC ddsd;
  HRESULT ret=DDERR_WASSTILLDRAWING;

  if (m_bLocked)
  {
      m_lpddsBack->Unlock(NULL);
      m_bLocked = false;      
  }
  
  ddsd.dwSize = sizeof(ddsd);

  do
  {
    ret=m_lpddsBack->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR, NULL);
  }
  while (ret==DDERR_WASSTILLDRAWING);
  
  if (ret!=DD_OK)
		{
		InitFail(m_hWnd, ret, "There was an error locking the DirectDraw surface.");
		System->StartShutdown();
		}

  m_bLocked = true;

  return ret==DD_OK ? (unsigned char *)ddsd.lpSurface : NULL;
}

void csGraphics2DDDraw3::FinishDraw ()
{
  csGraphics2D::FinishDraw ();
  if (FrameBufferLocked)
    return;

  m_lpddsBack->Unlock(NULL);
  m_bLocked = false;
  Memory = NULL;
  if (m_nActivePage == 0) m_nActivePage = 1;
  else m_nActivePage = 0;
}

HRESULT csGraphics2DDDraw3::SetColorPalette()
{
  HRESULT ret;
  
#ifndef __DD_FALSE
  HRESULT DD_FALSE;
#endif
  

  if ((Depth==8) && m_bPaletteChanged)
  {
    m_bPaletteChanged = false;

    if (m_lpddPal)
    {
      m_lpddPal->Release ();
      m_lpddPal = NULL;
    }
    
    ret=m_lpDD->CreatePalette(DDPCAPS_8BIT, (PALETTEENTRY *)Palette, &m_lpddPal, NULL);
    if(ret==DD_OK) m_lpddsPrimary->SetPalette(m_lpddPal);
    
    if(!FullScreen)
    {
      HPALETTE oldPal;
      HDC dc = GetDC(NULL);
      
      SetSystemPaletteUse(dc, SYSPAL_NOSTATIC);
      PostMessage(HWND_BROADCAST, WM_SYSCOLORCHANGE, 0, 0);
      
      if (!CreateIdentityPalette(Palette))
			{
				InitFail(m_hWnd, DD_FALSE, "Error creating Identity Palette.");
				return DD_FALSE;
			}
      ClearSystemPalette();
      
      oldPal = SelectPalette(dc, hWndPalette, FALSE);
      
      RealizePalette(dc);
      SelectPalette(dc, oldPal, FALSE);
      ReleaseDC(NULL, dc);
    }

    return ret;
  }
  
  return DD_OK;
}

bool csGraphics2DDDraw3::BeginDraw()
{
  csGraphics2D::BeginDraw ();
  if (FrameBufferLocked != 1)
    return true;

  Memory = LockBackBuf();
  return (Memory != NULL);
}

void csGraphics2DDDraw3::SetRGB(int i, int r, int g, int b)
{
  csGraphics2D::SetRGB (i, r, g, b);
  m_bPaletteChanged = true;
}

bool csGraphics2DDDraw3::SetMouseCursor (csMouseCursorID iShape)
{
  HCURSOR hCursor;
	
  switch(iShape)
  {
    case csmcNone:     hCursor = NULL; break;
    case csmcArrow:    hCursor = LoadCursor (NULL, IDC_ARROW);    break;
    case csmcMove:     hCursor = LoadCursor (NULL, IDC_SIZEALL);  break;
    case csmcSizeNWSE: hCursor = LoadCursor (NULL, IDC_SIZENWSE); break;
    case csmcSizeNESW: hCursor = LoadCursor (NULL, IDC_SIZENESW); break;
    case csmcSizeNS:   hCursor = LoadCursor (NULL, IDC_SIZENS);   break;
    case csmcSizeEW:   hCursor = LoadCursor (NULL, IDC_SIZEWE);   break;
    case csmcStop:     hCursor = LoadCursor (NULL, IDC_NO);       break;
    case csmcWait:     hCursor = LoadCursor (NULL, IDC_WAIT);     break;
    default: hCursor = NULL;		//return false;
  }
	
	if (hCursor)
	{
		SetCursor(hCursor);
		return true;
	}
	else
	{
		SetCursor(NULL);
		return false;
	}
}

bool csGraphics2DDDraw3::SetMousePosition (int x, int y)
{
  POINT p;
  
  p.x = x;
  p.y = y;

  ClientToScreen(m_hWnd, &p);

  ::SetCursorPos(p.x, p.y);

  return true;
}

void csGraphics2DDDraw3::GetDirectDrawDriver (LPDIRECTDRAW* lplpDirectDraw)
{
  *lplpDirectDraw = m_lpDD;
}

void csGraphics2DDDraw3::GetDirectDrawPrimary (LPDIRECTDRAWSURFACE* lplpDirectDrawPrimary)
{
  *lplpDirectDrawPrimary = m_lpddsPrimary;
}

void csGraphics2DDDraw3::GetDirectDrawBackBuffer (LPDIRECTDRAWSURFACE* lplpDirectDrawBackBuffer)
{
  *lplpDirectDrawBackBuffer = m_lpddsBack;
}

extern DirectDetectionDevice* DirectDevice;
void csGraphics2DDDraw3::GetDirectDetection (IDirectDetectionInternal** lplpDDetection)
{
  *lplpDDetection = static_cast<IDirectDetectionInternal*>(DirectDevice);
}

void csGraphics2DDDraw3::SetFor3D(bool For3D)
{
  m_bUses3D = For3D;
}

bool csGraphics2DDDraw3::PerformExtension (const char *args)
{
	csString ext(args);

	if (ext.CompareNoCase("fullscreen"))
	{
		if (m_bReady)
		{
			System->Printf(MSG_INITIALIZATION,"Fullscreen toggle.");
			m_bReady = FALSE;
			if (m_bWindowed)
					GetWindowRect(m_hWnd, &m_rcWindow);
			m_bWindowed = !m_bWindowed;
			ChangeCoopLevel(m_hWnd);
			m_bReady = TRUE;	
		}
	}
	
	return true;
}


//-----------------------------------------------------------------------------
// Name: ReleaseAllObjects()
// Desc: Release all DDraw objects we use
//-----------------------------------------------------------------------------
HRESULT csGraphics2DDDraw3::ReleaseAllObjects(HWND hWnd)
{
    if (m_lpDD != NULL)
    {
        m_lpDD->SetCooperativeLevel(hWnd, DDSCL_NORMAL);
				m_lpDD->RestoreDisplayMode();
        if (m_lpddsBack != NULL)
        {
            m_lpddsBack->Release();
            m_lpddsBack = NULL;
        }
        if (m_lpddsPrimary != NULL)
        {
            m_lpddsPrimary->Release();
            m_lpddsPrimary = NULL;
        }
    }
    return DD_OK;
}



//-----------------------------------------------------------------------------
// Name: InitSurfaces()
// Desc: Create all the needed DDraw surfaces and set the coop level
//-----------------------------------------------------------------------------
HRESULT csGraphics2DDDraw3::InitSurfaces(HWND hWnd)
{
    HRESULT		        hRet;
		DDSURFACEDESC      ddsd;
		DDSCAPS            ddscaps;
    LPDIRECTDRAWCLIPPER pClipper;

	  DDPIXELFORMAT ddpf;

		int RedMask   = 0x00FF0000;
		int GreenMask = 0x0000FF00;
		int BlueMask  = 0x000000FF;

    if (m_bWindowed)
    {
        // Get normal windowed mode
        hRet = m_lpDD->SetCooperativeLevel(hWnd, DDSCL_NORMAL);
        if (hRet != DD_OK)
            return InitFail(hWnd, hRet, "SetCooperativeLevel FAILED");

    		// Get the dimensions of the viewport and screen bounds
    		GetClientRect(hWnd, &m_rcViewport);
    		GetClientRect(hWnd, &m_rcScreen);
    		ClientToScreen(hWnd, (POINT*)&m_rcScreen.left);
    		ClientToScreen(hWnd, (POINT*)&m_rcScreen.right);

        // Create the primary surface
        ZeroMemory(&ddsd,sizeof(ddsd));
        ddsd.dwSize = sizeof(ddsd);
        ddsd.dwFlags = DDSD_CAPS;
        ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
        hRet = m_lpDD->CreateSurface(&ddsd, &m_lpddsPrimary, NULL);
        if (hRet != DD_OK)
            return InitFail(hWnd, hRet, "CreateSurface FAILED");

        // Create a clipper object since this is for a Windowed render
        hRet = m_lpDD->CreateClipper(0, &pClipper, NULL);
        if (hRet != DD_OK)
            return InitFail(hWnd, hRet, "CreateClipper FAILED");

        // Associate the clipper with the window
        pClipper->SetHWnd(0, hWnd);
        m_lpddsPrimary->SetClipper(pClipper);
        pClipper->Release();
        pClipper = NULL;

        // Get the backbuffer. For fullscreen mode, the backbuffer was created
        // along with the primary, but windowed mode still needs to create one.
        ddsd.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
        ddsd.dwWidth        = Width;
        ddsd.dwHeight       = Height;
        ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;

				if(!DirectDevice->Only2D)
					ddsd.ddsCaps.dwCaps |= DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY;
				else 
					ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;

        hRet = m_lpDD->CreateSurface(&ddsd, &m_lpddsBack, NULL);
        if (hRet != DD_OK)
            return InitFail(hWnd, hRet, "CreateSurface2 FAILED");

				// get the pixel format
				memset(&ddpf, 0, sizeof(ddpf));
				ddpf.dwSize = sizeof(ddpf);
				hRet = m_lpddsPrimary->GetPixelFormat(&ddpf);
				if (hRet != DD_OK)
					return InitFail(hWnd, hRet, "Can't get pixel format descriptor FAILED");
    
				RedMask = ddpf.dwRBitMask;
				GreenMask = ddpf.dwGBitMask;
				BlueMask = ddpf.dwBBitMask;

				// automatically determine bit-depth for windowed mode
				if(ddpf.dwFlags & DDPF_PALETTEINDEXED8)
					Depth=8;
				else if(ddpf.dwRGBBitCount == 16)
					Depth=16;
				else if(ddpf.dwRGBBitCount == 32)
					Depth=32;
				else
				{
					return InitFail(hWnd, hRet, "Crystal Space requires desktop to be in either 8-bit, 16-bit or 32-bit mode, or to use full screen mode.");
				}
    }
    else
    {
        // Get exclusive mode
        hRet = m_lpDD->SetCooperativeLevel(hWnd, DDSCL_EXCLUSIVE |
                                                DDSCL_FULLSCREEN);
        if (hRet != DD_OK)
            return InitFail(hWnd, hRet, "SetCooperativeLevel FAILED");

        //Set FS video mode
				hRet = m_lpDD->SetDisplayMode( Width, Height, Depth);
        if (hRet != DD_OK)
            return InitFail(hWnd, hRet, "SetDisplayMode FAILED");

    		// Get the dimensions of the viewport and screen bounds
    		// Store the rectangle which contains the renderer
    		SetRect(&m_rcViewport, 0, 0, Width, Height );
    		memcpy(&m_rcScreen, &m_rcViewport, sizeof(RECT) );

        // Create the primary surface with 1 back buffer
        ZeroMemory(&ddsd,sizeof(ddsd));
        ddsd.dwSize = sizeof(ddsd);
        ddsd.dwFlags = DDSD_CAPS |
                       DDSD_BACKBUFFERCOUNT;
        ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE |
                			  DDSCAPS_FLIP |
                			  DDSCAPS_COMPLEX;
        ddsd.dwBackBufferCount = 1;

				// set flags if this is a 3d device
				if(!DirectDevice->Only2D)
					ddsd.ddsCaps.dwCaps |= DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY;

        hRet = m_lpDD->CreateSurface( &ddsd, &m_lpddsPrimary, NULL);
        if (hRet != DD_OK)
            return InitFail(hWnd, hRet, "CreateSurface FAILED");

        ZeroMemory(&ddscaps, sizeof(ddscaps));
        ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
        hRet = m_lpddsPrimary->GetAttachedSurface(&ddscaps, &m_lpddsBack);
        if (hRet != DD_OK)
            return InitFail(hWnd, hRet, "GetAttachedSurface FAILED");

				if(Depth == 16)
				{
					RedMask   = 0x1f << 11;
					GreenMask = 0x3f << 5;
					BlueMask  = 0x1f;
				}
				else if (Depth == 32)
				{
					RedMask   = 0xff << 16;
					GreenMask = 0xff << 8;
					BlueMask  = 0xff;
				}

    }


		// set xx bpp mode up //

		if (Depth == 16)
		{
			_DrawPixel = DrawPixel16;
			_WriteChar = WriteChar16;
			_GetPixelAt = GetPixelAt16;

			// Set pixel format
			pfmt.PixelBytes = 2;
			pfmt.PalEntries = 0;
			pfmt.RedMask = RedMask;
			pfmt.GreenMask = GreenMask;
			pfmt.BlueMask = BlueMask;

			pfmt.complete ();
		}
		else if (Depth == 32)
		{
			_DrawPixel = DrawPixel32;
			_WriteChar = WriteChar32;
			_GetPixelAt = GetPixelAt32;
    
			// calculate CS's pixel format structure.
			pfmt.PixelBytes = 4;
			pfmt.PalEntries = 0;
   
			pfmt.RedMask = RedMask;
			pfmt.GreenMask = GreenMask;
			pfmt.BlueMask = BlueMask;
    
			pfmt.complete ();
		}

		m_lpddsBack->GetSurfaceDesc (&ddsd);
  
		for(int i = 0; i < Height; i++)
			LineAddress [i] = i * ddsd.lPitch;
  
		if(Depth==8) m_bPalettized = true;
		else m_bPalettized = false;

		m_bPaletteChanged = false;

    return DD_OK;
}


//-----------------------------------------------------------------------------
// Name: ChangeCoopLevel()
// Desc: Called when the user wants to toggle between Full-Screen & Windowed
//-----------------------------------------------------------------------------
HRESULT csGraphics2DDDraw3::ChangeCoopLevel(HWND hWnd )
{
    HRESULT hRet;

    // Release all objects that need to be re-created for the new device
    if (FAILED(hRet = ReleaseAllObjects(hWnd)))
        return InitFail(hWnd, hRet, "ReleaseAllObjects FAILED");

    // In case we're coming from a fullscreen mode, restore the window size
    if (m_bWindowed)
    {
				m_lpDD->RestoreDisplayMode();
        SetWindowPos(hWnd, HWND_NOTOPMOST, m_rcWindow.left, m_rcWindow.top,
                     (m_rcWindow.right - m_rcWindow.left), 
                     (m_rcWindow.bottom - m_rcWindow.top), SWP_SHOWWINDOW );
    }

    // Re-create the surfaces
    hRet = InitSurfaces(hWnd);
    return hRet;
}


//-----------------------------------------------------------------------------
// Name: InitFail()
// Desc: This function is called if an initialization function fails
//-----------------------------------------------------------------------------
HRESULT csGraphics2DDDraw3::InitFail(HWND hWnd, HRESULT hRet, LPCTSTR szError, ...)
{
    char            szBuff[128];
    va_list         vl;

    va_start(vl, szError);
    vsprintf(szBuff, szError, vl);
    ReleaseAllObjects(hWnd);
		System->Printf(MSG_STDOUT, szBuff);
    MessageBox(hWnd, szBuff, "csDirectDraw", MB_OK);
    DestroyWindow(hWnd);
    va_end(vl);
    return hRet;
}

