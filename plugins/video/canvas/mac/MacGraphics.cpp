/*
	Copyright (C) 1998 by Jorrit Tyberghein and K. Robert Bate.
  
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

/*----------------------------------------------------------------
	Written by K. Robert Bate 1998.

	10/1998		-	Created.
	12/1998		-	Modified to use DrawSprockets when
					the FullScreen flag is set.
	2/1999		-	Added support for 32 bit pixels.
	
----------------------------------------------------------------*/

#include "cssysdef.h"
#include "csutil/scf.h"
#include "cssys/system.h"
#include "csgeom/csrect.h"

#include <SegLoad.h>
#include <Memory.h>
#include <Palettes.h>
#include <TextUtils.h>
#include <Types.h>
#include <Memory.h>
#include <Quickdraw.h>
#include <Fonts.h>
#include <Events.h>
#include <Menus.h>
#include <Windows.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <OSUtils.h>
#include <ToolUtils.h>
#include <DrawSprocket.h>

#include "MacGraphics.h"

#include <stdarg.h>
#include "cssys/mac/MacRSRCS.h"

#if !TARGET_API_MAC_CARBON && !TARGET_API_MAC_OSX
#define GetWindowPort(n) ( (CGrafPtr)(n) )
#define GetPortBounds(m,n) ( *(n) = ((CGrafPtr)(m))->portRect )
#define GetPortPixMap(m) ( (m)->portPixMap )
#define GetPortBitMapForCopyBits(m) (&(((GrafPtr)(m))->portBits))
#endif

/////The 2D Graphics Driver//////////////

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csGraphics2DMac)

SCF_EXPORT_CLASS_TABLE (Driver2D)
    SCF_EXPORT_CLASS_DEP (csGraphics2DMac, CS_SOFTWARE_2D_DRIVER,
        "Crystal Space 2D driver for Macintosh", "crystalspace.font.server.")
SCF_EXPORT_CLASS_TABLE_END

SCF_IMPLEMENT_IBASE_EXT(csGraphics2DMac)
    SCF_IMPLEMENTS_INTERFACE(iMacGraphics)
SCF_IMPLEMENT_IBASE_EXT_END


/*----------------------------------------------------------------
	Construct a graphics object.  This object provides a place on
	screen to draw, and a place offscreen to render into.
----------------------------------------------------------------*/
csGraphics2DMac::csGraphics2DMac(iBase *iParent) : csGraphics2D(iParent)
{
	mMainWindow = NULL;
	mColorTable = NULL;
	mOffscreen = NULL;
	mPixMap = NULL;
    mMainPalette = NULL;
    mPaletteChanged = false;
    mDoubleBuffering = true;
    mOldDepth = 0;
	mSavedPort = NULL;
	mSavedGDHandle = NULL;
	mDrawSprocketsEnabled = false;
	mActivePage = 0;
}


/*----------------------------------------------------------------
	All Done, get rid of the color table and reset the screen
	depth back to what it was if we changed it.
----------------------------------------------------------------*/
csGraphics2DMac::~csGraphics2DMac()
{
}

/*----------------------------------------------------------------
	Construct the screen objects.  This object provides a place on
	screen to draw, and a place offscreen to render into.
----------------------------------------------------------------*/
bool csGraphics2DMac::Initialize( iSystem* piSystem )
{
	long					pixel_format;
	OSErr					err;
	Boolean					showDialogFlag;

	if ( ! csGraphics2D::Initialize( piSystem ))
		return false;

	/*
	 *	Check to see if the depth requested is 15 bits,
	 *	while this is the depth we use, apple called it 16 bits.
	 */

	if ( Depth == 15 )
		Depth = 16;

	/*
	 *	If the game is to be created in fullscreen mode and
	 *	DrawSprockets is available, then use it.
	 */

	if (( FullScreen ) && ( DSpStartup != NULL ) && ( DSpStartup() == noErr )) {
		// Create the display
		mDisplayAttributes.frequency 				= 0;
		mDisplayAttributes.displayWidth				= Width;
		mDisplayAttributes.displayHeight			= Height;
		mDisplayAttributes.reserved1 				= 0;
		mDisplayAttributes.reserved2 				= 0;
		mDisplayAttributes.colorTable 				= NULL;
		mDisplayAttributes.contextOptions 			= 0;
		mDisplayAttributes.pageCount				= 0;
		mDisplayAttributes.filler[1] 				= 0;
		mDisplayAttributes.filler[2] 				= 0;
		mDisplayAttributes.filler[3] 				= 0;
		mDisplayAttributes.gameMustConfirmSwitch	= false;
		mDisplayAttributes.reserved3[0] 			= 0;
		mDisplayAttributes.reserved3[1] 			= 0;
		mDisplayAttributes.reserved3[2] 			= 0;
		mDisplayAttributes.reserved3[3] 			= 0;

		if ( Depth == 0 ) {
			mDisplayAttributes.colorNeeds				= kDSpColorNeeds_Request;
			mDisplayAttributes.backBufferDepthMask		= kDSpDepthMask_8 | kDSpDepthMask_16 | kDSpDepthMask_32;
			mDisplayAttributes.displayDepthMask			= kDSpDepthMask_8 | kDSpDepthMask_16 | kDSpDepthMask_32;
			mDisplayAttributes.backBufferBestDepth		= 16;
			mDisplayAttributes.displayBestDepth			= 16;
		} else {
			mDisplayAttributes.colorNeeds				= kDSpColorNeeds_Require;
			if ( Depth == 32 ) {
				mDisplayAttributes.backBufferDepthMask	= kDSpDepthMask_32;
				mDisplayAttributes.displayDepthMask		= kDSpDepthMask_32;
				mDisplayAttributes.backBufferBestDepth	= 32;
				mDisplayAttributes.displayBestDepth		= 32;
			} else if ( Depth == 16 ) {
				mDisplayAttributes.backBufferDepthMask	= kDSpDepthMask_16;
				mDisplayAttributes.displayDepthMask		= kDSpDepthMask_16;
				mDisplayAttributes.backBufferBestDepth	= 16;
				mDisplayAttributes.displayBestDepth		= 16;
			} else {
				mDisplayAttributes.backBufferDepthMask	= kDSpDepthMask_8;
				mDisplayAttributes.displayDepthMask		= kDSpDepthMask_8;
				mDisplayAttributes.backBufferBestDepth	= 8;
				mDisplayAttributes.displayBestDepth		= 8;
			}
		}

#if !TARGET_API_MAC_CARBON && !TARGET_API_MAC_OSX
		err = DSpCanUserSelectContext( &mDisplayAttributes, &showDialogFlag );
		if ( err ) {
			DisplayErrorDialog( kNoDSContext );
		}
		if ( showDialogFlag == false ) {
			err = DSpFindBestContext(&mDisplayAttributes, &mDisplayContext);
			if ( err ) {
				DisplayErrorDialog( kUnableToOpenDSContext );
			}
		} else {
			err = DSpUserSelectContext( &mDisplayAttributes, 0, NULL, &mDisplayContext );
			if ( err ) {
				ExitToShell();
			}
		}
#else
		err = DSpFindBestContext(&mDisplayAttributes, &mDisplayContext);
		if ( err ) {
			DisplayErrorDialog( kUnableToOpenDSContext );
		}
#endif

		DSpContext_GetAttributes( mDisplayContext, &mDisplayAttributes );
		Depth = mDisplayAttributes.backBufferBestDepth;

		mDrawSprocketsEnabled = true;
	} else {
		mDrawSprocketsEnabled = false;

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
				ExitToShell();
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
				ExitToShell();
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

		_DrawPixel = DrawPixel32;
		_WriteString = WriteString32;
		_GetPixelAt = GetPixelAt32;
	} else if ( Depth == 16 ) {
		mColorTable = NULL;			// No color table needed
		pfmt.PalEntries = 0;
		pfmt.PixelBytes = 2;
  		pfmt.RedMask = 0x1F << 10;
  		pfmt.GreenMask = 0x1F << 5;
  		pfmt.BlueMask = 0x1F;
  		pfmt.complete ();

		_DrawPixel = DrawPixel16;
		_WriteString = WriteString16;
		_GetPixelAt = GetPixelAt16;
	} else {
		/*
		 *	The 8 bit pixel data was filled in by csGraphics2D
		 *	so all we need to do is get the default 8 bit color table.
		 */
		mColorTable = GetCTable( 72 );
	}

	if ( mDrawSprocketsEnabled ) {
		mDisplayAttributes.colorTable = mColorTable;
		err = DSpContext_Reserve(mDisplayContext, &mDisplayAttributes);
		if ( err ) {
			DisplayErrorDialog( kUnableToReserveDSContext );
		}
	}

	return true;
}


/*----------------------------------------------------------------
	Open a window.
----------------------------------------------------------------*/
bool csGraphics2DMac::Open(const char* Title)
{
	Str255			theTitle;
	Rect			theBounds;
	Rect			displayRect;
	int				i;
	int				theOffset;
	unsigned int	theRowBytes;
	OSErr			theError;
	int				displayWidth;
	int				displayHeight;

	if ( mDrawSprocketsEnabled ) {
		DSpContext_FadeGammaOut(NULL, NULL);
		DSpContext_SetState(mDisplayContext, kDSpContextState_Active);
		DSpContext_FadeGammaIn(NULL, NULL);

		// Create the pixmap draw context with the image pointing to our back buffer
		DSpContext_GetBackBuffer(mDisplayContext, kDSpBufferKind_Normal, &mOffscreen);
		mPixMap = GetGWorldPixMap(mOffscreen);

		mGetBufferAddress = false;
		mDoubleBuffering = true;
	} else {
		/*
		 *	Make the offscreen port.
		 */
		theBounds.left = 0;
		theBounds.top = 0;
		theBounds.right = Width;
		theBounds.bottom = Height;
		theError = NewGWorld( &mOffscreen, Depth, &theBounds, mColorTable, NULL, 0 );
		if (( theError != noErr ) || ( mOffscreen == NULL ))
			ExitToShell();

		/*
		 *	lock it and get its address
		 */
		mPixMap = GetGWorldPixMap( mOffscreen );
		LockPixels(mPixMap);

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
		mMainWindow = ::NewCWindow( NULL, &theBounds, theTitle, true, noGrowDocProc, 
												 (WindowPtr)-1, false, 0 );

		// set the color table into the video card
		SetGWorld( GetWindowPort(mMainWindow), mMainGDevice );
		if ( Depth == 8 ) {
			mMainPalette = ::NewPalette( 256, mColorTable, pmExplicit + pmTolerant, 0 );
			SetPalette( mMainWindow, mMainPalette, false );
			mPaletteChanged = true;
		} else {
			mPaletteChanged = false;
		}
		ShowWindow( mMainWindow );
		SelectWindow( mMainWindow );

		mDoubleBuffering = true;
	}

	Memory = (unsigned char*)GetPixBaseAddr(mPixMap);
	theRowBytes = (**mPixMap).rowBytes & 0x7fff;

	/*
	 *	Allocate buffer for address of each scan line to avoid multiplication
	 */
	LineAddress = new int [Height];

	if (LineAddress == NULL)
		return false;

	/*
	 *	Initialize scanline address array (offset)
	 */
	for (i = 0, theOffset = 0; i < Height; i++, theOffset += theRowBytes )
		LineAddress[i] = theOffset;

	SetClipRect (0, 0, Width, Height);

	BeginDraw();
	Clear( 0 );
	FinishDraw();
	Print( NULL );

	return true;
}


/*----------------------------------------------------------------
	Toasts the window and restores the clut to the main device.
----------------------------------------------------------------*/
void csGraphics2DMac::Close(void)
{
	if ( mDrawSprocketsEnabled ) {
		if ( mDisplayContext != NULL )
		{
			DSpContext_FadeGammaOut(NULL, NULL);
			DSpContext_SetState(mDisplayContext, kDSpContextState_Inactive);
			DSpContext_FadeGammaIn(NULL, NULL);
			DSpContext_Release(mDisplayContext);
			
			mDisplayContext = NULL;
		}
		DSpShutdown();
	} else {
		if ( Depth == 8 )
			::RestoreDeviceClut( NULL );

		if ( mOffscreen ) {
			UnlockPixels( GetGWorldPixMap( mOffscreen ) );
			DisposeGWorld( mOffscreen );
			mOffscreen = NULL;
			mPixMap = NULL;
		}

		if ( mMainWindow ) {
			DisposeWindow( mMainWindow );
			mMainWindow = NULL;
		}

		if ( mMainPalette ) {
			DisposePalette( mMainPalette );
			mMainPalette = NULL;
		}

		if ( mOldDepth ) {
			SetDepth( mMainGDevice, mOldDepth, (**mMainGDevice).gdFlags, 1 );
			mOldDepth = 0;
		}
	}

	if ( mColorTable ) {
		DisposeHandle( (Handle)mColorTable );
		mColorTable = NULL;
	}

	csGraphics2D::Close();
}


/*----------------------------------------------------------------
	Set a color in the color palette.
----------------------------------------------------------------*/
void csGraphics2DMac::SetRGB(int i, int r, int g, int b)
{
	CTabHandle	theCTable;
	ColorSpec	theColor;

	if (( i < 0 ) || ( i >= pfmt.PalEntries ))
		return;

	theColor.rgb.red =  r | (r << 8);
	theColor.rgb.green = g | (g << 8);
	theColor.rgb.blue = b | (b << 8);

	if ( mDrawSprocketsEnabled ) {
		DSpContext_SetCLUTEntries( mDisplayContext,  &theColor, i, i );
	} else {
		(*mColorTable)->ctTable[i].value = i;
		(*mColorTable)->ctTable[i].rgb = theColor.rgb;
		CTabChanged( mColorTable );
		SetEntryColor( mMainPalette, i, &theColor.rgb );
		SetEntryUsage( mMainPalette, i, pmExplicit + pmTolerant, 0 );
	}
	mPaletteChanged = true;
}

void csGraphics2DMac::GetColorfromInt( int color, RGBColor *outColor )
{
	unsigned short c;

	switch (pfmt.PixelBytes) {
		case 1: // paletted colors
			outColor->red = (((unsigned short)Palette[color].red) << 8) +
									((unsigned short)Palette[color].red);
			outColor->green = (((unsigned short)Palette[color].green) << 8) +
									((unsigned short)Palette[color].green);
			outColor->blue = (((unsigned short)Palette[color].blue) << 8) +
									((unsigned short)Palette[color].blue);
			break;

		case 2: // 16bit color
			outColor->red =  (( color & pfmt.RedMask) << 1 ) + (( color & pfmt.RedMask ) >> 10);
			outColor->green = (( color & pfmt.GreenMask) << 6 ) + (( color & pfmt.GreenMask ) >> 5);
			outColor->blue = (( color & pfmt.BlueMask) << 11 ) + ( color & pfmt.BlueMask );
			break;

		case 4: // truecolor
			outColor->red =  (( color & pfmt.RedMask ) >> 8) + (( color & pfmt.RedMask ) >> 16);
			outColor->green = ( color & pfmt.GreenMask ) + (( color & pfmt.GreenMask ) >> 8);
			outColor->blue = (( color & pfmt.BlueMask ) << 8 ) + ( color & pfmt.BlueMask );
			break;
	}
}


/*----------------------------------------------------------------
	Make sure we are ready to draw.
----------------------------------------------------------------*/
bool csGraphics2DMac::BeginDraw()
{
	csGraphics2D::BeginDraw ();

	if (( FrameBufferLocked == 1 ) && mDrawSprocketsEnabled ) {
		CGrafPtr port;
		int		 i;
		int		 theOffset;
		int		 theRowBytes;

		if ( mGetBufferAddress ) {
			DSpContext_GetBackBuffer(mDisplayContext, kDSpBufferKind_Normal, &mOffscreen);

			Memory = (unsigned char*)::GetPixBaseAddr(GetPortPixMap(mOffscreen));
			mGetBufferAddress = false;
		}
	}

  return true;
}


/*----------------------------------------------------------------
	Finish up drawing.
----------------------------------------------------------------*/
void csGraphics2DMac::FinishDraw()
{
	csGraphics2D::FinishDraw ();
}


/*----------------------------------------------------------------
	Put the offscreen buffer into the window so the user can see it.
----------------------------------------------------------------*/
void csGraphics2DMac::Print( csRect * area )
{
	Rect	theRect;

	if ( mDrawSprocketsEnabled ) {
		/*
		 *	If area is not null, then use it to select the
		 *	region that is to be drawn.  Otherwise the whole
		 *	window is drawn.
		 */

#if !TARGET_API_MAC_CARBON && !TARGET_API_MAC_OSX
		if ( area ) {
			if (( area->xmin == 0 ) && ( area->ymin == 0 ) &&
					( area->xmax == 0 ) && ( area->ymax == 0 )) {
				SetRect( &theRect, 0, 0, Width, Height );
			} else {
				theRect.top = area->ymin;
				theRect.left = area->xmin;
				theRect.bottom = area->ymax;
				theRect.right = area->xmax;
			}
			DSpContext_InvalBackBufferRect( mDisplayContext, &theRect );
		}
#endif

		DSpContext_SwapBuffers( mDisplayContext, NULL, 0 );
		mGetBufferAddress = true;
	} else {
		CGrafPtr	thePort;
		GDHandle	theGDHandle;

		if ( mDoubleBuffering && mMainWindow ) {
			GetGWorld( &thePort, &theGDHandle );

			SetGWorld(GetWindowPort(mMainWindow), mMainGDevice );

			/*
			 *	If area is not null, then use it to select the
			 *	region that is to be drawn.  Otherwise the whole
			 *	window is drawn.
			 */

			if ( area ) {
				if (( area->xmin == 0 ) && ( area->ymin == 0 ) &&
						( area->xmax == 0 ) && ( area->ymax == 0 )) {
					GetPortBounds( mOffscreen, &theRect );
				} else {
					theRect.top = area->ymin;
					theRect.left = area->xmin;
					theRect.bottom = area->ymax;
					theRect.right = area->xmax;
				}
			} else {
				GetPortBounds( mOffscreen, &theRect );
			}

			CopyBits((BitMap*)*mPixMap, GetPortBitMapForCopyBits(GetWindowPort(mMainWindow)),
				&theRect, &theRect, srcCopy, NULL );

			SetGWorld( thePort, theGDHandle );
		}
	}
	if ( mActivePage == 0 )
		mActivePage = 1;
	else
		mActivePage = 0;
}


void csGraphics2DMac::Clear( int color )
{
	RGBColor	theColor;
	Rect		theRect;
	CGrafPtr	thePort;
	GDHandle	theGDHandle;

	GetGWorld( &thePort, &theGDHandle );

	if ( mDoubleBuffering )
		SetGWorld( mOffscreen, NULL );
	else
		SetGWorld( GetWindowPort(mMainWindow), mMainGDevice );

	GetColorfromInt( color, &theColor );
	RGBBackColor( &theColor );
	SetRect( &theRect, 0, 0, Width, Height );
	EraseRect( &theRect );

	SetGWorld( thePort, theGDHandle );
}


/*----------------------------------------------------------------
	Enable or disable double buffering; return TRUE if supported
----------------------------------------------------------------*/
bool csGraphics2DMac::DoubleBuffer(bool Enable)
{
	int				i;
	int				theOffset;
	unsigned int	theRowBytes;

	/*
	 *	If there is no pix map then we have not gotten to
	 *	open yet so just save the enable.
	 */

	if ( mPixMap == NULL ) {
		mDoubleBuffering = Enable;
		return true;
	}

	/*
	 *	Otherwise, if we are not doing drawsprockets,
	 *	allow the double buffering to be turned on or off.
	 */

	if ( ! mDrawSprocketsEnabled ) {
		if ( Enable != mDoubleBuffering ) {
			mDoubleBuffering = Enable;

			if ( Enable ) {  
				mPixMap = ::GetGWorldPixMap(mOffscreen);
				Memory = (unsigned char*)::GetPixBaseAddr(mPixMap);
				theRowBytes = (**mPixMap).rowBytes & 0x7fff;
			} else {
				mPixMap = GetPortPixMap( GetWindowPort(mMainWindow) );
				Memory = (unsigned char*)::GetPixBaseAddr(mPixMap);
				theRowBytes = (**mPixMap).rowBytes & 0x7fff;
				Memory += ( theRowBytes * ( - (**mPixMap).bounds.top )) +
							( ((**mPixMap).pixelSize / 8 ) * ( - (**mPixMap).bounds.left ));
			}

			/*
			 *	Setup the scanline address array (offsets).
			 */
			for (i = 0, theOffset = 0; i < Height; i++, theOffset += theRowBytes )
				LineAddress[i] = theOffset;
		}
	} else {
		/*
		 *	Since we are doing drawsprockets, make sure double buffering on.
		 */
		mDoubleBuffering = true;
	}

	return true;
}

/*----------------------------------------------------------------
	Get the current drawing page.
----------------------------------------------------------------*/
int csGraphics2DMac::GetPage()
{
	return mActivePage;
}


/*----------------------------------------------------------------
	Set the mouse cursor.
----------------------------------------------------------------*/
bool csGraphics2DMac::SetMouseCursor( csMouseCursorID iShape )
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
void csGraphics2DMac::ActivateWindow( WindowPtr inWindow, bool active )
{
	if ( ! mDrawSprocketsEnabled ) {
		if ( inWindow != mMainWindow )
			return;

		if ( active ) {
			SelectWindow( mMainWindow );
			if ( mMainPalette )
				ActivatePalette( mMainWindow );
		}
	}
	return;
}


/*----------------------------------------------------------------
	Update the window.
----------------------------------------------------------------*/
bool csGraphics2DMac::UpdateWindow( WindowPtr inWindow )
{
	CGrafPtr	thePort;
	GDHandle	theGDHandle;
	Rect		theRect;
	bool		updated = false;

	if ( ! mDrawSprocketsEnabled && mMainWindow ) {
		if ( inWindow != mMainWindow )
			return false;

		GetGWorld( &thePort, &theGDHandle );

		SetGWorld(GetWindowPort(mMainWindow), mMainGDevice );

		BeginUpdate( mMainWindow );

		if ( mDoubleBuffering ) {
			GetPortBounds( mOffscreen, &theRect );
			CopyBits((BitMap*)*mPixMap, GetPortBitMapForCopyBits( GetWindowPort( mMainWindow )),
					&theRect, &theRect,
					srcCopy, NULL );
		}

		EndUpdate( mMainWindow );

		SetGWorld( thePort, theGDHandle );

		updated = true;
	}

	return updated;
}

bool csGraphics2DMac::PointInWindow( Point *thePoint )
{
	bool inWindow = false;

	if ( mDrawSprocketsEnabled ) {
		DSpContextReference outContext;
		OSStatus			err;

		if ( DSpFindContextFromPoint( *thePoint, &outContext ) == noErr ) {
			if ( outContext == mDisplayContext ) {
				DSpContext_GlobalToLocal ( outContext, thePoint );
				inWindow = true;
			}
		}
	} else {
		CGrafPtr	thePort;
		GDHandle	theGDHandle;

#if !TARGET_API_MAC_CARBON && !TARGET_API_MAC_OSX
		if ( mMainWindow && PtInRgn( *thePoint, ((WindowPeek)mMainWindow)->strucRgn )) {
			GetGWorld( &thePort, &theGDHandle );
			SetGWorld( (CGrafPtr)mMainWindow, mMainGDevice );
			GlobalToLocal( thePoint );
			SetGWorld( thePort, theGDHandle );
			inWindow = true;
		}
#else
		RgnHandle	theRgn;

		if ( mMainWindow ) {
			theRgn = NewRgn();
			GetWindowRegion( mMainWindow, kWindowStructureRgn, theRgn );
			if ( PtInRgn( *thePoint, theRgn )) {
				GetGWorld( &thePort, &theGDHandle );
				SetGWorld( GetWindowPort(mMainWindow), mMainGDevice );
				GlobalToLocal( thePoint );
				SetGWorld( thePort, theGDHandle );
				inWindow = true;
			}
		}
#endif
	}

	return inWindow;
}


bool csGraphics2DMac::DoesDriverNeedEvent( void )
{
	return mDrawSprocketsEnabled;
}


void csGraphics2DMac::WindowChanged( void )
{
	int				i;
	int				theOffset;
	unsigned int	theRowBytes;

	/*
	 *	If DrawSprockets is not enabled and we are not double buffered,
	 *	we need to recalc the scanline address array as the window may
	 *	have been moved.
	 */

	if (( ! mDrawSprocketsEnabled ) && ( ! mDoubleBuffering )) {
		mPixMap = GetPortPixMap( GetWindowPort( mMainWindow ));
		Memory = (unsigned char*)::GetPixBaseAddr(mPixMap);
		theRowBytes = (**mPixMap).rowBytes & 0x7fff;
		Memory += ( theRowBytes * ( - (**mPixMap).bounds.top )) +
							( ((**mPixMap).pixelSize / 8 ) * ( - (**mPixMap).bounds.left ));

		/*
		 *	Setup the scanline address array (offsets).
		 */
		for (i = 0, theOffset = 0; i < Height; i++, theOffset += theRowBytes )
			LineAddress[i] = theOffset;
	}

	return;
}


bool csGraphics2DMac::HandleEvent( EventRecord *inEvent )
{
	bool outEventWasProcessed = false;
	Boolean	processed = FALSE;

	if ( mDrawSprocketsEnabled ) {
		DSpProcessEvent( inEvent, &processed );

		if ( processed )
			outEventWasProcessed = true;
	}

	return outEventWasProcessed;
}


void csGraphics2DMac::SetColorPalette( void )
{
	CGrafPtr	thePort;
	GDHandle	theGDHandle;

	if ( ! mDrawSprocketsEnabled && mMainPalette ) {
		/*
		 *	If the palette has changed, make sure the offscreen gworld
		 *	(if there is one)and the window are correctly set up.
		 */
		if ( mPaletteChanged ) {
			SelectWindow( mMainWindow );

			if ( mOffscreen ) {
				Rect	theBounds;

				theBounds.left = 0;
				theBounds.top = 0;
				theBounds.right = Width;
				theBounds.bottom = Height;
				UpdateGWorld( &mOffscreen, Depth, &theBounds, mColorTable, NULL, 0 );
			}

			ActivatePalette( mMainWindow );

			mPaletteChanged = false;
		}
	}

	return;
}


void csGraphics2DMac::PauseDisplayContext( void )
{
	if ( mDrawSprocketsEnabled ) {
		if ( mDisplayContext )
			DSpContext_SetState(mDisplayContext, kDSpContextState_Paused);
	}
}


void csGraphics2DMac::ActivateDisplayContext( void )
{
	if ( mDrawSprocketsEnabled ) {
		if ( mDisplayContext )
			DSpContext_SetState(mDisplayContext, kDSpContextState_Active);
	}
}


void csGraphics2DMac::DisplayErrorDialog( short errorIndex )
{
	Str255	theString;

	if ( mDrawSprocketsEnabled ) {
		if ( mDisplayContext )
			DSpContext_SetState(mDisplayContext, kDSpContextState_Inactive);
		DSpShutdown();
	}

	GetIndString( theString, kErrorStrings, errorIndex );
	ParamText( theString, "\p",  "\p", "\p" );
	StopAlert( kGeneralErrorDialog, NULL );
	ExitToShell();
}
