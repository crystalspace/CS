/*
  Crystal Space Windowing System: Graphics Pipeline class
  Copyright (C) 1998 by Jorrit Tyberghein
  Written by Andrew Zabolotny <bit@eltech.ru>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the 9License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CSGFXPPL_H__
#define __CSGFXPPL_H__

#include "cssys/common/system.h"	//@@@??? Is this needed?
#include "csutil/csbase.h"
#include "csgeom/csrect.h"
#include "csengine/csspr2d.h"
#include "igraph2d.h"
#include "igraph3d.h"

// Maximal number of primitives which can be drawn IN ONE FRAME
#define MAX_CSWS_PIPELINE_LENGTH 16384
// Maximal number of video pages to sync image
#define MAX_SYNC_PAGES 8

/**
 * Graphics System pipeline class<p>
 * This class implements all drawing operations (which are then passed to
 * System->piG2D object). All drawing operations are put on hold until
 * Flush() is called. This routine is usually called once per frame.<p>
 * All methods and variables of this class are private. Only csApp should
 * have access to its internals, all graphics pipeline management is done
 * through main application object.
 */
class csGraphicsPipeline : public csBase
{
private:
  /// Only csApp can manipulate the graphics pipeline
  friend class csApp;
  /// Graphics pipeline operations
  enum
  {
    pipeopNOP,
    pipeopBOX,
    pipeopLINE,
    pipeopPIXEL,
    pipeopTEXT,
    pipeopSPR2D,
    pipeopSAVAREA,
    pipeopRESAREA,
    pipeopSETCLIP,
    pipeopRESCLIP
  };
  /// Graphics pipeline entry
  typedef struct
  {
    int Op;
    union
    {
     struct
     {
       int xmin,ymin,xmax,ymax,color;
     } Box;
     struct
     {
       float x1,y1,x2,y2;
       int color;
     } Line;
     struct
     {
       int x,y,color;
     } Pixel;
     struct
     {
       int x,y,fg,bg,font;
       char *string;
     } Text;
     struct
     {
       csSprite2D *s2d;
       int x, y, w, h;
     } Spr2D;
     struct
     {
       ImageArea **Area;
       int x, y, w, h;
     } SavArea;
     struct
     {
       ImageArea *Area;
       bool Free;
     } ResArea;
     struct
     {
       int xmin, ymin, xmax, ymax;
     } ClipRect;
    };
  } csPipeEntry;

  int pipelen;
  csPipeEntry pipeline [MAX_CSWS_PIPELINE_LENGTH];
  // Used to propagate changes to all pages
  ImageArea *SyncArea [MAX_SYNC_PAGES];
  csRect SyncRect [MAX_SYNC_PAGES];
  csRect RefreshRect;
  int MaxPage;
  int CurPage;

  // Used to clear screen
  int ClearPage,ClearColor;

  /// Initialize pipeline
  csGraphicsPipeline ();
  /// Deinitialize pipeline
  virtual ~csGraphicsPipeline ();

  /// Synchronise image on this page with previous pages
  void Sync (int CurPage, int &xmin, int &ymin, int &xmax, int &ymax);

  /// Drop all synchronization rectangles
  void Desync ();

  /// Flush graphics pipeline
  void Flush (int iCurPage);

  /// Draw a box
  void Box (int xmin, int ymin, int xmax, int ymax, int color);

  /// Draw a line
  void Line (float x1, float y1, float x2, float y2, int color);

  /// Draw a pixel
  void Pixel (int x, int y, int color);

  /// Draw a text string: if bg < 0 background is not drawn
  void Text (int x, int y, int fg, int bg, int font, char *s);

  /// Draw a 2D sprite
  void Sprite2D (csSprite2D *s2d, int x, int y, int w, int h);

  /// Save a part of screen
  void SaveArea (ImageArea **Area, int x, int y, int w, int h);

  /// Restore a part of screen
  void RestoreArea (ImageArea *Area, bool Free);

  /// Free buffer used to keep an area of screen
  void FreeArea (ImageArea *Area) { System->piG2D->FreeArea (Area); }

  /// Clear screen with specified color
  void Clear (int color);

  /// Set clipping rectangle: SHOULD CALL RestoreClipRect() AFTER DRAWING!
  void SetClipRect (int xmin, int ymin, int xmax, int ymax);

  /// Restore clipping rectangle to (0, 0, ScreenW, ScreenH);
  void RestoreClipRect();

  /// Allocate a new slot in graphics pipeline
  csPipeEntry *AllocOp(int pipeOp)
  {
    if (pipelen < MAX_CSWS_PIPELINE_LENGTH)
    {
      pipeline[pipelen].Op = pipeOp;
      return &pipeline[pipelen++];
    }
    else
      return NULL;
  }

  /// Return the width of given text using selected font
  int TextWidth (char *text, int Font);

  /// Return the height of selected font
  int TextHeight (int Font);

  /// Begin painting
  bool BeginDraw ();

  /// Finish painting
  void FinishDraw ();
};

#endif // __CSGFXPPL_H__
