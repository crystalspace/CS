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

#include <gl/gl.h>

#include <Windows.h>
#include <Dialogs.h>
#include <TextUtils.h>

#define SYSDEF_2DDRIVER_DEFS
#include "sysdef.h"
#include "cscom/com.h"
#include "cs2d/openglmac/oglg2d.h"
#include "cs2d/openglmac/oglFont.h"
#include "isystem.h"

/////The 2D Graphics Driver//////////////

#define NAME  "Crystal"

BEGIN_INTERFACE_TABLE(csGraphics2DOpenGL)
    IMPLEMENTS_COMPOSITE_INTERFACE_EX( IGraphics2D, XGraphics2D )
    IMPLEMENTS_COMPOSITE_INTERFACE_EX( IGraphicsInfo, XGraphicsInfo )
    IMPLEMENTS_COMPOSITE_INTERFACE_EX( IMacGraphicsInfo, XMacGraphicsInfo )
END_INTERFACE_TABLE()

IMPLEMENT_UNKNOWN(csGraphics2DOpenGL)

csGraphics2DOpenGLFontServer *csGraphics2DOpenGL::sLocalFontServer = NULL;

csGraphics2DOpenGL::csGraphics2DOpenGL(ISystem* piSystem, bool bUses3D) : 
                   csGraphics2D (piSystem)
{
	mMainWindow = NULL;
	mColorTable = NULL;
	mGLContext = NULL;
    mMainPalette = NULL;
    mPaletteChanged = false;
    mDoubleBuffering = true;
    mOldDepth = 0;
	mSavedPort = NULL;
	mSavedGDHandle = NULL;
	mActivePage = 0;
}

csGraphics2DOpenGL::~csGraphics2DOpenGL(void)
{
	if ( mOldDepth ) {
		GDHandle	theMainDevice;
		theMainDevice = GetMainDevice();
		SetDepth( theMainDevice, mOldDepth, (**theMainDevice).gdFlags, 1 );
		mOldDepth = 0;
	}

	if ( mColorTable ) {
		::DisposeHandle( (Handle)mColorTable );
		mColorTable = NULL;
	}
}

void csGraphics2DOpenGL::Initialize(void)
{
	long					pixel_format;
	OSErr					err;
	Boolean					showDialogFlag;

	csGraphics2D::Initialize ();

	/*
	 *	Get the depth of the main gdevice.
	 *	FIXMEkrb: Should ask user which gdevice to use if more then one.
	 */
	mMainGDevice = GetMainDevice();
	pixel_format = GETPIXMAPPIXELFORMAT( *((**mMainGDevice).gdPMap) );

	/*
	 *	If the programs needs a certain pixel depth and the gdevice
	 *	is not at this depth, ask the user if we can set the gdevice
	 *	to the required depth.
	 */
	if (( Depth ) && ( Depth != pixel_format )) {
		/*
		 *	Check to see if the main screen can handle the requested depth.
		 */
		if ( ! HasDepth( mMainGDevice, Depth, (**mMainGDevice).gdFlags, 1 )) {
			DisplayErrorDialog( kBadDepthString );
		}
		/*
		 *	Ask the user if we can change the depth, if not then quit.
		 */
		if ( Depth == 32 )
			ParamText( "\pmillions of", "\p", "\p", "\p" );
		else if ( Depth == 16 )
			ParamText( "\pthousands of", "\p", "\p", "\p" );
		else
			ParamText( "\p256", "\p", "\p", "\p" );
		if ( StopAlert( kAskForDepthChangeDialog, NULL ) != 1 ) {
			::ExitToShell();
		}
	}

	/*
	 *	If the program does not need a certain pixel depth, check to make
	 *	sure the pixel depth is on of the ones we can deal with.  If not,
	 *	ask the user if we can set the gdevice to the required depth.
	 */
	if (( ! Depth ) && (( pixel_format != 8 ) && ( pixel_format != 16 ) && ( pixel_format != 32 ))) {
		/*
		 *	Check to see if the main screen can handle the default depth.
		 */
		if ( ! HasDepth( mMainGDevice, 8, (**mMainGDevice).gdFlags, 1 )) {
			/*
			 *	No, see if it can deal with the other depths.
			 */
			if ( ! HasDepth( mMainGDevice, 16, (**mMainGDevice).gdFlags, 1 )) {
				if ( ! HasDepth( mMainGDevice, 32, (**mMainGDevice).gdFlags, 1 )) {
					DisplayErrorDialog( kBadDepthString );
				} else {
					Depth = 32;
				}
			} else {
				Depth = 16;
			}
		} else {
			Depth = 8;
		}
		/*
		 *	Ask the user if we can change the depth, if not then quit.
		 */
		if ( Depth == 32 )
			ParamText( "\pmillions of", "\p", "\p", "\p" );
		else if ( Depth == 16 )
			ParamText( "\pthousands of", "\p", "\p", "\p" );
		else
			ParamText( "\p256", "\p", "\p", "\p" );
		if ( StopAlert( kAskForDepthChangeDialog, NULL ) != 1 ) {
			::ExitToShell();
		}
	}

	/*
	 *	If the programs needs a certain pixel depth and the main device
	 *	is not at this depth, change the main device to the required depth.
	 */
	if (( Depth ) && ( Depth != pixel_format )) {
		SetDepth( mMainGDevice, Depth, (**mMainGDevice).gdFlags, 1 );
		mOldDepth = pixel_format;
	} else {
		Depth = pixel_format;
	}

	/*
	 *	Setup the pixel format, color table and drawing
	 *	routines to the correct pixel depth.
	 */
	if ( Depth == 32 ) {
		mColorTable = NULL;			// No color table needed
		pfmt.PalEntries = 0;
		pfmt.PixelBytes = 4;
  		pfmt.RedMask = 0xFF << 16;
  		pfmt.GreenMask = 0xFF << 8;
  		pfmt.BlueMask = 0xFF;
  		complete_pixel_format();
	} else if ( Depth == 16 ) {
		mColorTable = NULL;			// No color table needed
		pfmt.PalEntries = 0;
		pfmt.PixelBytes = 2;
  		pfmt.RedMask = 0x1F << 10;
  		pfmt.GreenMask = 0x1F << 5;
  		pfmt.BlueMask = 0x1F;
  		complete_pixel_format();
	} else {
		/*
		 *	The 8 bit pixel data was filled in by csGraphics2D
		 *	so all we need to do is make an empty color table.
		 */
		mColorTable = (CTabHandle)::NewHandleClear( sizeof(ColorSpec) * 256 + 8 );
		(*mColorTable)->ctSeed = ::GetCTSeed();
		(*mColorTable)->ctFlags = 0;
		(*mColorTable)->ctSize = 255;

		pfmt.PalEntries = 256;
		pfmt.PixelBytes = 1;
		pfmt.RedMask = 0;
		pfmt.GreenMask = 0;
		pfmt.BlueMask = 0;

  		complete_pixel_format();
	}

	DrawPixel = DrawPixelGL;
	WriteChar = WriteCharGL;
	GetPixelAt = GetPixelAtGL;
	DrawSprite = DrawSpriteGL;
  
	SysPrintf (MSG_INITIALIZATION, "Using %d bits per pixel (%d color mode).\n", Depth, 1 << Depth);

}

bool csGraphics2DOpenGL::Open(char *Title)
{
	Str255			theTitle;
	Rect			theBounds;
	Rect			displayRect;
	int				i;
	int				theOffset;
	OSErr			theError;
	int				displayWidth;
	int				displayHeight;
	GLint			attrib[5];
	AGLPixelFormat	fmt;

	/*
	 *	Create the main window with the given title
	 *	centered on the main gdevice.
	 */
	displayRect = (**mMainGDevice).gdRect;
	displayWidth = displayRect.right - displayRect.left;
	displayHeight = displayRect.bottom - displayRect.top;
	
	theBounds.left = displayRect.left + ((displayWidth - Width) / 2);
	theBounds.top = displayRect.top + ((displayHeight - Height) / 2);
	
	theBounds.right = theBounds.left + Width;
	theBounds.bottom = theBounds.top + Height;

	strcpy( (char *)&theTitle[1], Title );
	theTitle[0] = strlen( Title );
	mMainWindow = (CWindowPtr)::NewCWindow( nil, &theBounds, theTitle, true, noGrowDocProc, 
											(WindowPtr) -1, false, 0 );

	// set the color table into the video card
	::SetGWorld( (CGrafPtr)mMainWindow, NULL );
	if ( Depth == 8 ) {
		mMainPalette = ::NewPalette( 256, mColorTable, /* pmExplicit + */ pmTolerant, 0x2000 );
		::SetPalette( (WindowPtr)mMainWindow, mMainPalette, true );
		mPaletteChanged = true;
	} else {
		mPaletteChanged = false;
	}
	::ShowWindow( (WindowPtr)mMainWindow );
	::SelectWindow( (WindowPtr)mMainWindow );

	mDoubleBuffering = true;
  
	/*
	 *	Allocate buffer for address of each scan line to avoid multiplication
	 */

	i = Width*Height;
	if ( Depth == 16 )
		i *= 2;
	else if ( Depth == 32 )
		i *= 4;
	Memory = new unsigned char [i];
	if ( Memory == NULL )
		DisplayErrorDialog( kFatalErrorOutOfMemory );

	/*
	 *	Allocate buffer for address of each scan line to avoid multiplication
	 */

	CHK (LineAddress = new int [Height]);

	for(int i = 0; i < Height; i++)
		LineAddress [i] = i * Width;

	/*
	 *	Choose the pixel format.
	 */
	 i = 0;
	if ( Depth != 8 )
		attrib[i++] = AGL_RGBA;
	attrib[i++] = AGL_DOUBLEBUFFER;
	attrib[i++] = AGL_DEPTH_SIZE;
	attrib[i++] = Depth;
	attrib[i++] = AGL_NONE;

	fmt = aglChoosePixelFormat( NULL, 0, attrib );

	mGLContext = aglCreateContext( fmt, NULL );

	aglDestroyPixelFormat( fmt );

	aglSetDrawable( mGLContext, mMainWindow );
	aglSetCurrentContext( mGLContext );

	/* Setup the fonts for OpenGL */
	if (sLocalFontServer == NULL) {
		sLocalFontServer = new csGraphics2DOpenGLFontServer(&FontList[0]);
		for (int fontindex=1; fontindex < 8; fontindex++)
			sLocalFontServer->AddFont(FontList[fontindex]);
	}

	/* Clear color buffer to black */
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	aglSwapBuffers( mGLContext );

	if ( glGetString( GL_RENDERER ))
		SysPrintf( MSG_INITIALIZATION, "OpenGL Renderer is %s\n", glGetString(GL_RENDERER) );
	else
		SysPrintf( MSG_INITIALIZATION, "No OpenGL renderer found!\n");
	if ( glGetString( GL_VERSION ))
		SysPrintf( MSG_INITIALIZATION, "OpenGL Version is %s\n", glGetString(GL_VERSION) );
	else
		SysPrintf( MSG_INITIALIZATION, "No OpenGL version found!\n");
  
  return true;
}

void csGraphics2DOpenGL::Close(void)
{
	if ( mGLContext ) {
		aglSetCurrentContext(NULL);
		aglSetDrawable(mGLContext, NULL);
		aglDestroyContext( mGLContext );
		mGLContext = NULL;
	}

	if ( mMainWindow ) {
		::DisposeWindow( (WindowPtr)mMainWindow );
		mMainWindow = NULL;
	}

	::RestoreDeviceClut( NULL );

	csGraphics2D::Close();
}

int csGraphics2DOpenGL::GetPage ()
{
	return mActivePage;
}

bool csGraphics2DOpenGL::DoubleBuffer( bool Enable )
{
	if ( Enable )
		mDoubleBuffering = false;
	else
		mDoubleBuffering = true;

	return true;
}

bool csGraphics2DOpenGL::DoubleBuffer()
{
	return mDoubleBuffering;
}

void csGraphics2DOpenGL::Print( csRect *area )
{
	aglSwapBuffers( mGLContext );
	glFlush ();
}

bool csGraphics2DOpenGL::BeginDraw()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.,(GLdouble)Width,0.,(GLdouble)Height,-1.0,100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	return true;
}

void csGraphics2DOpenGL::FinishDraw()
{
	if (mActivePage == 0)
		mActivePage = 1;
	else
		mActivePage = 0;
}

void csGraphics2DOpenGL::SetColorPalette()
{  
	if (( Depth == 8 ) && ( mPaletteChanged )) {
		mPaletteChanged = false;

		aglUpdateContext( mGLContext );
	}
}

void csGraphics2DOpenGL::SetRGB(int i, int r, int g, int b)
{
	CTabHandle	theCTable;
	ColorSpec	theColor;

	if (( i < 0 ) || ( i >= pfmt.PalEntries ))
		return;

	theColor.rgb.red =  r | (r << 8);
	theColor.rgb.green = g | (g << 8);
	theColor.rgb.blue = b | (b << 8);

	theCTable = (**(mMainWindow->portPixMap)).pmTable;
	(*theCTable)->ctTable[i].value = i;
	(*theCTable)->ctTable[i].rgb = theColor.rgb;
	CTabChanged( theCTable );

	if ( mMainPalette ) {
		SetEntryColor( mMainPalette, i, &theColor.rgb );
	}

	mPaletteChanged = true;
}

bool csGraphics2DOpenGL::SetMouseCursor( int iShape, ITextureHandle* iBitmap )
{
#pragma unused( iBitmap )
	bool		cursorSet = true;
	CursHandle	theCursor;

	if ( iShape == csmcNone )
		HideCursor();
	else if ( iShape == csmcArrow )
		InitCursor();
	else {
		if ( iShape == csmcWait )
			theCursor = GetCursor( watchCursor );
		else if ( iShape == csmcCross )
			theCursor = GetCursor( crossCursor );
		else
			theCursor = GetCursor( iShape + kArrowCursor );

		if ( theCursor )
			SetCursor( *theCursor );
		else {
			HideCursor();
			cursorSet = false;
		}
	}

  return cursorSet;
}

void csGraphics2DOpenGL::SetGLColorfromInt( int color )
{
	switch (pfmt.PixelBytes) {
		case 1: // paletted colors
			glColor3i(Palette[color].red, Palette[color].green, Palette[color].blue);
			break;

		case 2: // 16bit color
		case 4: // truecolor
			glColor3f(  (float)((color & pfmt.RedMask) >> pfmt.RedShift )     / (float)pfmt.RedBits,
						(float)((color & pfmt.GreenMask) >> pfmt.GreenShift ) / (float)pfmt.GreenBits,
						(float)((color & pfmt.BlueMask) >> pfmt.BlueShift )   / (float)pfmt.BlueBits );
			break;
	}
}

void csGraphics2DOpenGL::DrawLine( float x1, float y1, float x2, float y2, int color )
{
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glBegin(GL_LINES);
	SetGLColorfromInt( color );
	glVertex2f(x1, (float)Height-y1-1.0f);
	glVertex2f(x2, (float)Height-y2-1.0f);
	glEnd();
}

void csGraphics2DOpenGL::DrawPixelGL (int x, int y, int color)
{
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glBegin(GL_POINTS);
	SetGLColorfromInt( color );
	glVertex2i(x, Height-y-1);
	glEnd();
}

void csGraphics2DOpenGL::WriteCharGL(int x, int y, int fg, int bg, char c)
{
	glDisable( GL_TEXTURE_2D );
	glDisable( GL_BLEND );
	glDisable( GL_DEPTH_TEST );
  
	SetGLColorfromInt( fg );
	glRasterPos2i( x, Height-y-1 );

	sLocalFontServer->WriteCharacter( c, Font );
}

void csGraphics2DOpenGL::DrawSpriteGL(ITextureHandle *hTex, int sx, int sy,
		int sw, int sh, int tx, int ty, int tw, int th)
{
}

unsigned char* csGraphics2DOpenGL::GetPixelAtGL(int /*x*/, int /*y*/)
{
	return NULL;
}

void csGraphics2DOpenGL::Clear( int color )
{
	switch (pfmt.PixelBytes) {
		case 1: // paletted colors
			glClearColor(Palette[color].red, Palette[color].green, Palette[color].blue, 0.0f);
			break;

		case 2: // 16bit color
		case 4: // truecolor
			glClearColor(  (float)((color & pfmt.RedMask) >> pfmt.RedShift )     / (float)pfmt.RedBits,
						(float)((color & pfmt.GreenMask) >> pfmt.GreenShift ) / (float)pfmt.GreenBits,
						(float)((color & pfmt.BlueMask) >> pfmt.BlueShift )   / (float)pfmt.BlueBits, 0.0f );
			break;
	}
	glClear(GL_COLOR_BUFFER_BIT);
}


/*----------------------------------------------------------------
	Activate the window.
----------------------------------------------------------------*/
void csGraphics2DOpenGL::ActivateWindow( WindowPtr inWindow, bool active )
{
	CGrafPtr	thePort;
	GDHandle	theGDHandle;

	if ( inWindow != (WindowPtr)mMainWindow )
		return;

	if ( active ) {
		::SelectWindow( (WindowPtr)mMainWindow );
		::ActivatePalette( (WindowPtr)mMainWindow );
	}

	return;
}


/*----------------------------------------------------------------
	Update the window.
----------------------------------------------------------------*/
void csGraphics2DOpenGL::UpdateWindow( WindowPtr inWindow, bool *updated )
{
	*updated = false;
}

void csGraphics2DOpenGL::PointInWindow( Point *thePoint, bool *inWindow )
{
	GrafPtr	thePort;

	if ( PtInRgn( *thePoint, ((WindowPeek)mMainWindow)->strucRgn )) {
		::GetPort( &thePort );
		::SetPort( (WindowPtr)mMainWindow );
		::GlobalToLocal( thePoint );
		::SetPort( thePort );

		*inWindow = true;
	} else
		*inWindow = false;

	return;
}


void csGraphics2DOpenGL::DoesDriverNeedEvent( bool *isEnabled )
{
	*isEnabled = false;

	return;
}


void csGraphics2DOpenGL::WindowChanged()
{
	aglUpdateContext( mGLContext );
	return;
}


void csGraphics2DOpenGL::HandleEvent( EventRecord *inEvent, bool *outEventWasProcessed )
{
	*outEventWasProcessed = false;

	return;
}


void csGraphics2DOpenGL::DisplayErrorDialog( short errorIndex )
{
	Str255	theString;
	Str255	theString2;

	GetIndString( theString, kErrorStrings, kFatalErrorInOpenGL2D );
	GetIndString( theString2, kErrorStrings, errorIndex );
	ParamText( theString, theString2,  "\p", "\p" );
	StopAlert( kGeneralErrorDialog, NULL );

	ExitToShell();
}
