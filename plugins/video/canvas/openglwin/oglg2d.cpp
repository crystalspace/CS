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

#include <windows.h>
#include <gl/gl.h>

#include "sysdef.h"
#include "cscom/com.h"
#include "cs2d/openglwin/oglg2d.h"
#include "cs3d/opengl/ogl_txtcache.h"
#include "cs3d/opengl/ogl_txtmgr.h"
#include "isystem.h"

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
			
			MessageBox (NULL, szMsg, "Fatal Error in OpenGL2D.dll", MB_OK);
			delete szMsg;

			exit(1);
		}
	}

	MessageBox(NULL, str, "Fatal Error in OpenGL2D.dll", MB_OK);
	
	exit(1);
}

/////The 2D Graphics Driver//////////////

#define NAME  "Crystal"

BEGIN_INTERFACE_TABLE(csGraphics2DOpenGL)
    IMPLEMENTS_COMPOSITE_INTERFACE_EX( IGraphics2D, XGraphics2D )
    IMPLEMENTS_COMPOSITE_INTERFACE_EX( IGraphicsInfo, XGraphicsInfo )
    IMPLEMENTS_COMPOSITE_INTERFACE_EX( IOpenGLGraphicsInfo, XOpenGLGraphicsInfo )
END_INTERFACE_TABLE()

IMPLEMENT_UNKNOWN_NODELETE(csGraphics2DOpenGL)
csGraphics2DOpenGLFontServer *csGraphics2DOpenGL::LocalFontServer = NULL;
OpenGLTextureCache *csGraphics2DOpenGL::texture_cache = NULL;

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

csGraphics2DOpenGL::csGraphics2DOpenGL(ISystem* piSystem, bool bUses3D) : 
                   csGraphics2D (piSystem),
                   m_hWnd(NULL),
                   m_bDisableDoubleBuffer(false),
                   m_bPaletteChanged(false),
                   m_bPalettized(false),
                   m_nActivePage(0),
                   m_nGraphicsReady(true),
                   m_piWin32System(NULL)
{
  HRESULT ddrval;
 
  // QI for IWin32SystemDriver //
  ddrval = piSystem->QueryInterface(IID_IWin32SystemDriver, (void**)&m_piWin32System);
  if (FAILED(ddrval))
  	  sys_fatalerror("csGraphics2DDDraw3::Open(QI) -- ISystem passed does not support IWin32SystemDriver.", ddrval);
}

csGraphics2DOpenGL::~csGraphics2DOpenGL(void)
{
  FINAL_RELEASE(m_piWin32System);
  Close();
  m_nGraphicsReady=0;
}

void csGraphics2DOpenGL::Initialize(void)
{
  csGraphics2D::Initialize();
  
  // Get the creation parameters //
  m_piWin32System->GetInstance(&m_hInstance);
  m_piWin32System->GetCmdShow(&m_nCmdShow);

  system->GetDepthSetting(Depth);
  system->GetHeightSetting(Height);
  system->GetWidthSetting(Width);
  system->GetFullScreenSetting(FullScreen);
    
  if (Depth==8)
  {
    // calculate CS's pixel format structure.
    pfmt.RedMask = pfmt.GreenMask = pfmt.BlueMask = 0;
    pfmt.PalEntries = 256; pfmt.PixelBytes = 1;
    
    complete_pixel_format();
  }
  else if (Depth==16)
  {
    // calculate CS's pixel format structure.
    pfmt.PixelBytes = 2;
    pfmt.PalEntries = 0;
    pfmt.RedMask   = 0x1f << 11;
    pfmt.GreenMask = 0x3f << 5;
    pfmt.BlueMask  = 0x1f;
    
    complete_pixel_format();
  }
  else if (Depth==32)
  {
    // calculate CS's pixel format structure.
    pfmt.PixelBytes = 4;
    pfmt.PalEntries = 0;
    pfmt.RedMask   = 0xff << 16;
    pfmt.GreenMask = 0xff << 8;
    pfmt.BlueMask  = 0xff;
    
    complete_pixel_format();
  }
  else
    sys_fatalerror("Only support 8, 16 or 32 bits color depth");
  
  SysPrintf (MSG_INITIALIZATION, "Using %d bits per pixel (%d color mode).\n", Depth, 1 << Depth);

  DrawPixel = DrawPixelGL;   WriteChar = WriteCharGL;
  GetPixelAt = GetPixelAtGL; DrawSprite = DrawSpriteGL;
}

void csGraphics2DOpenGL::CalcPixelFormat ()
{
  PIXELFORMATDESCRIPTOR pfd = {
      sizeof(PIXELFORMATDESCRIPTOR),  /* size */
      1,                              /* version */
      PFD_SUPPORT_OPENGL |
      PFD_DOUBLEBUFFER |              /* support double-buffering */
      PFD_DRAW_TO_WINDOW,
      PFD_TYPE_RGBA,                  /* color type */
      Depth,                          /* prefered color depth */
      0, 0, 0, 0, 0, 0,               /* color bits (ignored) */
      0,                              /* no alpha buffer */
      0,                              /* alpha bits (ignored) */
      0,                              /* no accumulation buffer */
      0, 0, 0, 0,                     /* accum bits (ignored) */
      16,                             /* depth buffer */
      0,                              /* no stencil buffer */
      0,                              /* no auxiliary buffers */
      PFD_MAIN_PLANE,                 /* main layer */
      0,                              /* reserved */
      0, 0, 0                         /* no layer, visible, damage masks */
  };
  
  int pixelFormat;
  pixelFormat = ChoosePixelFormat(hDC, &pfd);
  
  if (pixelFormat == 0)
    sys_fatalerror("ChoosePixelFormat failed.");
  if (SetPixelFormat(hDC, pixelFormat, &pfd) != TRUE)
    sys_fatalerror("SetPixelFormat failed.");
    
  if (DescribePixelFormat(hDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd) == 0)
    sys_fatalerror("DescribePixelFormat failed.");
}

bool csGraphics2DOpenGL::Open(char *Title)
{
  DEVMODE dmode;
  LONG ti;

  if (!csGraphics2D::Open (Title))
    return false;
  
  // create the window.
  DWORD exStyle = 0;
  DWORD style = WS_POPUP;// | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
  if (!FullScreen)
	  style |= WS_CAPTION;
  if (FullScreen) 
  {
    ChangeDisplaySettings(NULL,0);

    EnumDisplaySettings(NULL, 0, &dmode);

    dmode.dmPelsWidth = Width;
    dmode.dmPelsHeight = Height;
    dmode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;

    if((ti = ChangeDisplaySettings(&dmode, CDS_FULLSCREEN)) != DISP_CHANGE_SUCCESSFUL) 
    {
      //The cases below need error handling, as theyare errors. 
      switch(ti) 
      {
        case DISP_CHANGE_RESTART:
        //computer must restart for mode to work.
        break;
        case DISP_CHANGE_BADFLAGS:
        //Bad Flag settings
        break;
        case DISP_CHANGE_FAILED:
        //Failure to display
        break;
        case DISP_CHANGE_NOTUPDATED:
        //No Reg Write Error
        break;
        default:
        //Unknown Error
        break;
      }
    }
  }

  int wwidth,wheight;
  wwidth=Width+2*GetSystemMetrics(SM_CXSIZEFRAME);
  wheight=Height+2*GetSystemMetrics(SM_CYSIZEFRAME)+GetSystemMetrics(SM_CYCAPTION);
  
  m_hWnd = CreateWindowEx(exStyle, NAME, Title, style,
                          (GetSystemMetrics(SM_CXSCREEN)-wwidth)/2,
                          (GetSystemMetrics(SM_CYSCREEN)-wheight)/2,
                          wwidth, wheight, NULL, NULL, m_hInstance, NULL );
  if( !m_hWnd )
    sys_fatalerror("Cannot create CrystalSpace window", GetLastError());

  ShowWindow( m_hWnd, m_nCmdShow );
  UpdateWindow( m_hWnd );
  SetFocus( m_hWnd );  
  
  Memory=new unsigned char [Width*Height*2];
  if(Memory==NULL)
    sys_fatalerror("Dummy buffer not allowed");

  for(int i = 0; i < Height; i++)
    LineAddress [i] = i * Width;

  hDC = GetDC(m_hWnd);
  CalcPixelFormat ();

  hGLRC = wglCreateContext(hDC);
  wglMakeCurrent(hDC, hGLRC);

  if(Depth==8) m_bPalettized = true;
  else m_bPalettized = false;

  m_bPaletteChanged = false;

  if (LocalFontServer == NULL)
  {
       LocalFontServer = new csGraphics2DOpenGLFontServer(&FontList[0]);
       for (int fontindex=1; 
       		fontindex < 8;
		fontindex++)
	   LocalFontServer->AddFont(FontList[fontindex]);
  }

  if (texture_cache == NULL)
  {
    CHK (texture_cache = new OpenGLTextureCache(1<<24,24));
  }

  if (glGetString(GL_RENDERER))
    SysPrintf (MSG_INITIALIZATION, "OpenGL Renderer is %s\n", glGetString(GL_RENDERER) );
  else
    SysPrintf (MSG_INITIALIZATION, "No OpenGL renderer found!\n");
  if (glGetString(GL_VERSION))
    SysPrintf (MSG_INITIALIZATION, "OpenGL Version is %s\n", glGetString(GL_VERSION) );
  else
    SysPrintf (MSG_INITIALIZATION, "No OpenGL version found!\n");
  
  return true;
}

void csGraphics2DOpenGL::Close(void)
{
  if (hGLRC)
  {
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hGLRC);
  }
  ReleaseDC(m_hWnd, hDC);

  if (FullScreen) ChangeDisplaySettings(NULL,0); 

  if(!FullScreen)
  {
/*
    // restore the original system palette.
    HDC dc = GetDC(NULL);
    SetSystemPaletteUse(dc, SYSPAL_STATIC);
    PostMessage(HWND_BROADCAST, WM_SYSCOLORCHANGE, 0, 0);
    ReleaseDC(NULL, dc);
*/
  }

  if(Memory) delete Memory;

  csGraphics2D::Close ();
}

int csGraphics2DOpenGL::GetPage ()
{
  return m_nActivePage;
}

bool csGraphics2DOpenGL::DoubleBuffer (bool Enable)
{
  if (Enable) m_bDisableDoubleBuffer = false;
  else m_bDisableDoubleBuffer = true;
  return true;
}

bool csGraphics2DOpenGL::DoubleBuffer ()
{
  return m_bDisableDoubleBuffer;
}

void csGraphics2DOpenGL::Print (csRect *area)
{
  SwapBuffers(hDC);
  glFlush();
}

bool csGraphics2DOpenGL::BeginDraw()
{
  return true;
}

void csGraphics2DOpenGL::FinishDraw ()
{
  if (m_nActivePage == 0) m_nActivePage = 1;
  else m_nActivePage = 0;
}

HRESULT csGraphics2DOpenGL::SetColorPalette()
{
  HRESULT ret = S_OK;
  
  if ((Depth==8) && m_bPaletteChanged)
  {
    m_bPaletteChanged = false;

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

    return ret;
  }
  
  return S_OK;
}

void csGraphics2DOpenGL::SetRGB(int i, int r, int g, int b)
{
  csGraphics2D::SetRGB (i, r, g, b);
  m_bPaletteChanged = true;
}

bool csGraphics2DOpenGL::SetMouseCursor (int iShape, csTextureHandle* iBitmap)
{
  (void)iShape; (void)iBitmap;
  return false;
}

void csGraphics2DOpenGL::setGLColorfromint(int color)
{
  switch (pfmt.PixelBytes)
  {
  case 1: // paletted colors
    glColor3i(Palette[color].red,
    		Palette[color].green,
		Palette[color].blue);
    break;
  case 2: // 16bit color
  case 4: // truecolor
    glColor3f( ( (color & pfmt.RedMask) >> pfmt.RedShift )     / (float)pfmt.RedBits,
               ( (color & pfmt.GreenMask) >> pfmt.GreenShift ) / (float)pfmt.GreenBits,
               ( (color & pfmt.BlueMask) >> pfmt.BlueShift )   / (float)pfmt.BlueBits);
    break;
  }
}
void csGraphics2DOpenGL::DrawLine (int x1, int y1, int x2, int y2, int color)
{
  glDisable (GL_TEXTURE_2D);
  glDisable (GL_BLEND);
  glBegin (GL_LINES);
  setGLColorfromint(color);
  glVertex2i (x1, Height-y1-1);
  glVertex2i (x2, Height-y2-1);
  glEnd ();
}

void csGraphics2DOpenGL::DrawHorizLine (int x1, int x2, int y, int color)
{
  glDisable (GL_TEXTURE_2D);
  glDisable (GL_BLEND);
  glBegin (GL_LINES);
  setGLColorfromint(color);
  glVertex2i (x1, Height-y-1);
  glVertex2i (x2, Height-y-1);
  glEnd ();
}

void csGraphics2DOpenGL::DrawPixelGL (int x, int y, int color)
{
  // prepare for 2D drawing--so we need no fancy GL effects!
  glDisable (GL_TEXTURE_2D);
  glDisable (GL_BLEND);
  glDisable (GL_DEPTH_TEST);
  setGLColorfromint(color);
  glBegin (GL_POINTS);
  glVertex2i (x, Height-y-1);
  glEnd ();
}

void csGraphics2DOpenGL::WriteCharGL (int x, int y, int fg, int bg, char c)
{
  // prepare for 2D drawing--so we need no fancy GL effects!
  glDisable (GL_TEXTURE_2D);
  glDisable (GL_BLEND);
  glDisable (GL_DEPTH_TEST);
  
  setGLColorfromint(fg);

  // FIXME: without the 0.5 shift rounding errors in the
  // openGL renderer can misalign text!
  // maybe we should modify the glOrtho() in glrender to avoid
  // having to does this fractional shift?
  //glRasterPos2i (x, Height-y-1-FontList[Font].Height);
  glRasterPos2f (x+0.5, Height-y-0.5-FontList[Font].Height);

  LocalFontServer->WriteCharacter(c,Font);
}

void csGraphics2DOpenGL::DrawSpriteGL (ITextureHandle *hTex, int sx, int sy,
  int sw, int sh, int tx, int ty, int tw, int th)
{
  texture_cache->Add (hTex);

  // cache the texture if we haven't already.
  csTextureMMOpenGL* txt_mm = (csTextureMMOpenGL*)GetcsTextureMMFromITextureHandle (hTex);

  HighColorCache_Data *cachedata;
  cachedata = txt_mm->get_hicolorcache ();
  GLuint texturehandle = *( (GLuint *) (cachedata->pData) );

  glShadeModel(GL_FLAT);
  glEnable(GL_TEXTURE_2D);
  glDisable (GL_BLEND);
  glDisable (GL_DEPTH_TEST);
  glBindTexture(GL_TEXTURE_2D,texturehandle);
  
  int bitmapwidth=0, bitmapheight=0;
  hTex->GetBitmapDimensions(bitmapwidth,bitmapheight);

  // convert texture coords given above to normalized (0-1.0) texture coordinates
  float ntx1,nty1,ntx2,nty2;
  ntx1 = tx/bitmapwidth;
  ntx2 = (tx+tw)/bitmapwidth;
  nty1 = ty/bitmapheight;
  nty2 = (ty+th)/bitmapheight;

  // draw the bitmap
  glBegin(GL_TRIANGLE_FAN);
  glTexCoord2f(ntx1,nty1);
  glVertex2i(sx,Height-sy-1);
  glTexCoord2f(ntx2,nty1);
  glVertex2i(sx+sw,Height-sy-1);
  glTexCoord2f(ntx2,nty2);
  glVertex2i(sx+sw,Height-sy-sh-1);
  glTexCoord2f(ntx1,nty2);
  glVertex2i(sx,Height-sy-sh-1);
  glEnd();
}

unsigned char* csGraphics2DOpenGL::GetPixelAtGL (int /*x*/, int /*y*/)
{
  return NULL;
}

bool csGraphics2DOpenGL::SetMousePosition (int x, int y)
{
  POINT p;
  
  p.x = x;
  p.y = y;

  ClientToScreen(m_hWnd, &p);

  ::SetCursorPos(p.x, p.y);

  return true;
}
