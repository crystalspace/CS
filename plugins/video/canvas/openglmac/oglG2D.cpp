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

#include "cssysdef.h"
#include "csutil/scf.h"
#include "video/canvas/openglmac/oglg2d.h"
#include "cssys/mac/MacRSRCS.h"
#include "isys/system.h"

#if !TARGET_API_MAC_CARBON && !TARGET_API_MAC_OSX
#define GetWindowPort(n) ( (CGrafPtr)(n) )
#define GetPortBounds(m,n) ( *(n) = ((CGrafPtr)(m))->portRect )
#define GetPortPixMap(m) ( (m)->portPixMap )
#define GetPortBitMapForCopyBits(m) (&(((GrafPtr)(m))->portBits))
#endif

/////The 2D Graphics Driver//////////////

SCF_IMPLEMENT_FACTORY (csGraphics2DOpenGL)

SCF_EXPORT_CLASS_TABLE (Driver2DGL)
  SCF_EXPORT_CLASS_DEP (csGraphics2DOpenGL, CS_OPENGL_2D_DRIVER,
    "Crystal Space OpenGL 2D driver for Macintosh", "crystalspace.font.server.")
SCF_EXPORT_CLASS_TABLE_END

SCF_IMPLEMENT_IBASE_EXT (csGraphics2DOpenGL)
  SCF_IMPLEMENTS_INTERFACE (iMacGraphics)
SCF_IMPLEMENT_IBASE_EXT_END

csGraphics2DOpenGL::csGraphics2DOpenGL(iBase* iParent) : 
                   csGraphics2DGLCommon (iParent)
{
	mMainWindow = NULL;
	mColorTable = NULL;
	mGLContext = NULL;
	mMainPalette = NULL;
	mPaletteChanged = false;
	mOldDepth = 0;
	mSavedPort = NULL;
	mSavedGDHandle = NULL;
}

csGraphics2DOpenGL::~csGraphics2DOpenGL(void)
{
}

bool csGraphics2DOpenGL::Initialize (iSystem *pSystem)
{
	long					pixel_format;
	OSErr					err;
	Boolean					showDialogFlag;

	if (!csGraphics2DGLCommon::Initialize (pSystem))
		return false;

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
  		pfmt.complete ();
	} else if ( Depth == 16 ) {
		mColorTable = NULL;			// No color table needed
		pfmt.PalEntries = 0;
		pfmt.PixelBytes = 2;
  		pfmt.RedMask = 0x1F << 10;
  		pfmt.GreenMask = 0x1F << 5;
  		pfmt.BlueMask = 0x1F;
  		pfmt.complete ();
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

  		pfmt.complete ();
	}
  
	CsPrintf (CS_MSG_INITIALIZATION, "Using %d bits per pixel (%d color mode).\n", Depth, 1 << Depth);

	return true;
}

bool csGraphics2DOpenGL::Open(const char *Title)
{
  if (is_open) return true;
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

	aglSetDrawable( mGLContext, GetWindowPort(mMainWindow) );
	aglSetCurrentContext( mGLContext );

	csGraphics2DGLCommon::Open( Title );

	aglSwapBuffers( mGLContext );
  
	return true;
}

void csGraphics2DOpenGL::Close(void)
{
  if (!is_open) return;
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

	if ( mMainPalette ) {
		::RestoreDeviceClut( NULL );
	}

	if ( mOldDepth ) {
		GDHandle	theMainDevice;
		theMainDevice = GetMainDevice();
		SetDepth( mMainGDevice, mOldDepth, (**theMainDevice).gdFlags, 1 );
		mOldDepth = 0;
	}

	if ( mColorTable ) {
		::DisposeHandle( (Handle)mColorTable );
		mColorTable = NULL;
	}

	csGraphics2DGLCommon::Close();
}

void csGraphics2DOpenGL::Print( csRect *area )
{
	aglSwapBuffers( mGLContext );
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
	CTabHandle		theCTable;
	ColorSpec		theColor;
	PixMapHandle	thePortPixMap;

	if (( i < 0 ) || ( i >= pfmt.PalEntries ))
		return;

	theColor.rgb.red =  r | (r << 8);
	theColor.rgb.green = g | (g << 8);
	theColor.rgb.blue = b | (b << 8);

	thePortPixMap = GetPortPixMap( GetWindowPort( mMainWindow ));
	theCTable = (**(thePortPixMap)).pmTable;
	(*theCTable)->ctTable[i].value = i;
	(*theCTable)->ctTable[i].rgb = theColor.rgb;
	CTabChanged( theCTable );

	if ( mMainPalette ) {
		SetEntryColor( mMainPalette, i, &theColor.rgb );
	}

	mPaletteChanged = true;
}

bool csGraphics2DOpenGL::SetMouseCursor( csMouseCursorID iShape)
{
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
bool csGraphics2DOpenGL::UpdateWindow( WindowPtr inWindow )
{
	return false;
}

bool csGraphics2DOpenGL::PointInWindow( Point *thePoint )
{
	GrafPtr	thePort;
	bool	inWindow = false;

#if !TARGET_API_MAC_CARBON && !TARGET_API_MAC_OSX
	if ( mMainWindow && PtInRgn( *thePoint, ((WindowPeek)mMainWindow)->strucRgn )) {
		::GetPort( &thePort );
		::SetPort( (WindowPtr)mMainWindow );
		::GlobalToLocal( thePoint );
		::SetPort( thePort );

		inWindow = true;
	}
#else
	RgnHandle	theRgn;

	if ( mMainWindow ) {
		theRgn = NewRgn();
		GetWindowRegion( mMainWindow, kWindowStructureRgn, theRgn );
		if ( PtInRgn( *thePoint, theRgn )) {
			::GetPort( &thePort );
			::SetPort( GetWindowPort( mMainWindow ));
			::GlobalToLocal( thePoint );
			::SetPort( thePort );

			inWindow = true;
		}
	}
#endif

	return inWindow;
}


bool csGraphics2DOpenGL::DoesDriverNeedEvent( void )
{
	return false;
}


void csGraphics2DOpenGL::WindowChanged( void )
{
	aglUpdateContext( mGLContext );
	return;
}


bool csGraphics2DOpenGL::HandleEvent( EventRecord *inEvent )
{
	return false;
}


void csGraphics2DOpenGL::PauseDisplayContext( void )
{
}


void csGraphics2DOpenGL::ActivateDisplayContext( void )
{
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
