/*
    Copyright (C) 1998 by Jorrit Tyberghein and Steve Israelson
  
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

#ifndef __SYSG2D_H__
#define __SYSG2D_H__

#include <QDOffscreen.h>
#include <Palettes.h>
#include <DrawSprocket.h>
#include "csutil/scf.h"
#include "cs2d/common/graph2d.h"
#include "isysg2d.h"

/// Macintosh version.
class csGraphics2DMac : public csGraphics2D, public iMacGraphicsInfo
{
  friend class csGraphics3DSoftware;

public:
	 					 csGraphics2DMac(iBase *iParent);
	virtual 			~csGraphics2DMac();

					// this is handled in the constuctor
  	virtual bool 		Initialize(iSystem *theSystem);
  	virtual bool 		Open( const char *Title );
  	virtual void 		Close();

	virtual void 		Print( csRect *area = NULL );

	virtual void 		SetRGB( int i, int r, int g, int b );

	virtual bool 		BeginDraw();
	virtual void		FinishDraw ();

	virtual bool		SetMouseCursor( csMouseCursorID iShape, iTextureHandle *hBitmap );
	virtual int			GetPage();
	virtual bool		DoubleBuffer( bool Enable );
	virtual bool		DoubleBuffer() { return mDoubleBuffering; }
	virtual void		Clear( int color );

	virtual void		ActivateWindow( WindowPtr theWindow, bool active );
	virtual void		UpdateWindow( WindowPtr theWindow, bool *updated );
	virtual void		PointInWindow( Point *thePoint, bool *inWindow );
 	virtual void		DoesDriverNeedEvent( bool *isEnabled );
 	virtual void		SetColorPalette( void );
 	virtual void		WindowChanged( void );
	virtual void		HandleEvent( EventRecord *inEvent, bool *outEventWasProcessed );

	DECLARE_IBASE;

protected:
	CWindowPtr			mMainWindow;
	GDHandle			mMainGDevice;
	CTabHandle			mColorTable;
	GWorldPtr			mOffscreen;
	PixMapHandle		mPixMap;
    PaletteHandle		mMainPalette;
    bool				mPaletteChanged;
    bool				mDoubleBuffering;
    short				mOldDepth;
	CGrafPtr			mSavedPort;
	GDHandle			mSavedGDHandle;
	bool				mDrawSprocketsEnabled;
	DSpContextReference		mDisplayContext;
	DSpContextAttributes	mDisplayAttributes;
	bool					mGetBufferAddress;
	short				mActivePage;

	void				DisplayErrorDialog( short errorIndex );
	void				GetColorfromInt( int color, RGBColor *outColor );
};

#endif
