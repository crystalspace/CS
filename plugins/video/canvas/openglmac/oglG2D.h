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

#ifndef __SYSG2D_H__
#define __SYSG2D_H__

#include <QDOffscreen.h>
#include <Palettes.h>
#include <gl/gl.h>
#include <agl.h>
#include "csutil/scf.h"
#include "video/canvas/mac/iMacGraphics.h"
#include "video/canvas/openglcommon/glcommon2d.h"

/// Macintosh version.
class csGraphics2DOpenGL : public csGraphics2DGLCommon, public iMacGraphics
{
	friend class csGraphics3DOpenGL;

public:
	csGraphics2DOpenGL(iBase* iParent);
	virtual ~csGraphics2DOpenGL(void);
  
	virtual bool	Open (const char *Title);
	virtual void	Close ();
  
	virtual bool	Initialize(iSystem *pSystem);

	virtual void	Print (csRect *area = NULL);
  
	virtual void	SetRGB(int i, int r, int g, int b);

	virtual void	ActivateWindow( WindowPtr theWindow, bool active );
	virtual bool	UpdateWindow( WindowPtr theWindow );
	virtual bool	PointInWindow( Point *thePoint );
 	virtual bool	DoesDriverNeedEvent( void );
 	virtual void	SetColorPalette( void );
 	virtual void	WindowChanged( void );
	virtual bool	HandleEvent( EventRecord *inEvent );

	virtual bool	SetMouseCursor (csMouseCursorID iShape);

    virtual void	PauseDisplayContext( void );
    virtual void	ActivateDisplayContext( void );

	int mGraphicsReady;

	SCF_DECLARE_IBASE_EXT(csGraphics2DGLCommon);

protected:
	CWindowPtr			mMainWindow;
	GDHandle			mMainGDevice;
	CTabHandle			mColorTable;
	AGLContext			mGLContext;
	PaletteHandle		mMainPalette;
	bool				mPaletteChanged;
	short				mOldDepth;
	CGrafPtr			mSavedPort;
	GDHandle			mSavedGDHandle;

	void				DisplayErrorDialog( short errorIndex );
};

#endif
