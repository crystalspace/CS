/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#ifndef __PICVIEW_H__
#define __PICVIEW_H__

#include <stdarg.h>
#include "csws/csws.h"
#include "csutil/ref.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "ivideo/graph3d.h"
#include "imap/parser.h"

class ceImageView;
class csButton;
struct iVFS;
struct iImageIO;
struct iObjectRegistry;
struct iGraphics3D;
struct iStringArray;

enum
{
  cmdQuit = 9000,
  cmdPrev,
  cmdNext,
  cmdFirst,
  cmdNothing
};

/**
 * This subclass of csApp is our main entry point
 * for the CSWS application. It controls everything including
 * the image view.
 */
class PicViewApp : public csApp
{
public:
  csRef<iGraphics3D> pG3D;
  csRef<iVFS> VFS;
  csRef<iImageIO> image_loader;
  ceImageView* image_view;
  csWindow* image_window;
  csRef<iStringArray> files;
  int cur_idx;
  csButton* label1;
  csButton* label2;

  void LoadNextImage (int idx, int step);

public:
  PicViewApp (iObjectRegistry *object_reg, csSkin &skin);
  ~PicViewApp ();

  virtual bool HandleEvent (iEvent &Event);
  virtual bool Initialize ();
};

/**
 * This is a view of an image. It is a subclass of csComponent
 * so that it behaves nicely in the CSWS framework.
 */
class ceImageView : public csComponent
{
public:
  csPixmap* image;

public:
  ceImageView (csComponent *iParent, iGraphics3D *G3D);
  virtual ~ceImageView ();

  // Redraw the view.
  virtual void Draw ();
  // Do motion etc.
  virtual bool HandleEvent (iEvent &Event);
};

/**
 * This is a small window with three buttons.
 */
class ceControlWindow : public csWindow
{
public:
  ceControlWindow (csComponent *iParent, char *iTitle,
  	int iWindowStyle = CSWS_DEFAULTVALUE,
	csWindowFrameStyle iFrameStyle = cswfs3D) :
	csWindow (iParent, iTitle, iWindowStyle, iFrameStyle) { }
  virtual bool HandleEvent (iEvent& Event);
};

#endif // __PICVIEW_H__

