/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein
    Copyright (C) 2001 by W.C.A. Wijngaards

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

#ifndef __CSFEDIT_H__
#define __CSFEDIT_H__

#include <stdarg.h>
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csws/csws.h"

class csEditFontView;
class csEditCharView;
struct ModalData;

/** an editable character */
class csEditChar
{
  /// size of character
  int width, height;
  /// the pixels (0=nothing, 1=visible) (1 pixel per char)
  uint8 *pixels;

  /// my gui view
  csEditCharView *view;

public:
  /// empty char
  csEditChar();
  /// from file
  csEditChar(int w, int h, uint8 *bitmap);
  ///
  ~csEditChar();

  /// set a pixel
  void SetPixel(int x, int y, int val) {pixels[y*width+x]=val;}
  /// get a pixel
  int GetPixel(int x, int y) const {return pixels[y*width+x];}
  /// get width
  int GetWidth() const {return width;}
  /// get height
  int GetHeight() const {return height;}
  /// Draw the char to given component, given a topleft pos, and fgcol
  void Draw(csComponent *dest, int x, int y, int col);
  /// setWidth()
  void SetWidth(int w);
  /// setHeight()
  void SetHeight(int h);

  /// set gui view
  void SetView(csEditCharView *v) {view = v;}
  /// get gui view
  csEditCharView *GetView() const {return view;}

  /// get one char out of the character bitmap
  int GetBitmap(int idx);
};


/** an editable font */
class csEditFont
{
  /// the gui app
  csApp *app;
  /// a new[] alloced filename (can be 0- 'Untitled')
  char *filename;
  /// is font edited since last save?
  bool dirty;

  /// a new[] alloced font name
  char *fontname;
  /// the starting char & number of characters in the font
  int startchar, numchars;
  /// the font width and font height
  int fontwidth, fontheight;
  /// the characters in the font
  csEditChar **chars;

  /// my gui view
  csEditFontView *view;

public:
  /// create a new font from scratch
  csEditFont(csApp *iApp);
  /// read a font from file
  csEditFont(csApp *iApp, const char *fromfile);
  /// destroy font
  ~csEditFont();
  /// save the font to file
  void Save();
  /// is this font dirty (edited) ?
  bool IsDirty() const {return dirty;}
  /// make the font dirty (changed)
  void MakeDirty() {dirty = true;}

  /// get character by ascii charcode
  csEditChar *GetChar(int n) const {
    if( (n<startchar)||(n>=startchar+numchars) ) return 0;
    return chars[n-startchar];}
  /// set character content
  void SetChar(int n, csEditChar *c) {
    if( (n<startchar)||(n>=startchar+numchars) ) return;
    chars[n-startchar] = c;}

  /// get font width
  int GetWidth() const {return fontwidth;}
  /// get font height
  int GetHeight() const {return fontheight;}
  /// get startcharacter
  int GetStartChar() const {return startchar;}
  /// get number of characters in font
  int GetCharCount() const {return numchars;}
  /// get font name
  char *GetFontName() const {return fontname;}
  /// get file name
  char *GetFileName() const {return filename;}
  /// Set file name
  void SetFileName(char* fn) {filename = fn;}

  /// set gui view
  void SetView(csEditFontView *v) {view = v;}
  /// get gui view
  csEditFontView *GetView() const {return view;}
  /// set new font name
  void SetFontName(const char *val);
  /// set start char & num chars
  void SetStartNum(int start, int num);
  /// set Width --- for all characters
  void SetWidth(int w);
  /// set Height --- for all characters
  void SetHeight(int h);
  /// recalc the max width
  void RecalcWidth();
  /// get character number of an editchar
  int GetCharNumber(csEditChar *editchar);
};


/*  main app of csf font editor */
class CsfEdit : public csApp
{
  /// reliable fonts, for display
  csRef<iFont> mainfont, tinyfont;
  /// the font being edited
  csEditFont *editfont;
  /// menu item to gray or not
  csMenuItem *saveitem, *closeitem;

public:
  /// Initialize maze editor
  CsfEdit (iObjectRegistry *SysDriver, csSkin &Skin);

  /// Initialize maze editor
  virtual ~CsfEdit ();

  ///
  virtual bool HandleEvent (iEvent &Event);

  virtual bool Initialize ();

  /// get the edited font, can be 0.
  csEditFont *GetEditFont() const {return editfont;}
  /// set editfont (to 0 for example) -- does not delete the editfont
  void SetEditFont(csEditFont* f);
  /// get main font
  iFont *GetMainFont() const {return mainfont;}
  /// get a tiny font for drawing on screen
  iFont *GetTinyFont() const {return tinyfont;}
};



/** guis for char & font */
class csEditCharView : public csWindow
{
  csEditFont *font;
  csEditChar *editchar;
  int offx, offy; /// offset for scrolling content
  csScrollBar *scrhor, *scrvert;
  /// size of scrollbars
  int scrsize;
  /// border inside the window
  int inset;
  /// areas in the window
  csRect stats, content;
  /// cel size
  int celsize;
  /// the start of my palette in palettes
  int palstart;
  /// the mouse is down and drawing
  bool isdrawing;
  /// the drawing colour
  int drawcolour;

public:
  csEditCharView(csComponent *iParent, csEditFont *fnt, csEditChar *chr);
  ~csEditCharView();
  void Reevaluate(); //// number of chars in font and so on.
  virtual bool SetRect(int xmin, int ymin, int xmax, int ymax);
  virtual void Draw();
  void EditSettings();
  void HandleEditSettings(ModalData* data);
  virtual bool HandleEvent(iEvent &Event);
};


/** guis for char & font */
class csEditFontView : public csWindow
{
  csEditFont *font;
  int offx, offy; /// offset for scrolling content
  csScrollBar *scrhor, *scrvert;
  /// size of scrollbars
  int scrsize;
  /// border inside the window
  int inset;
  /// areas in the window
  csRect stats, content;
  /// number and size of cells
  int celw, celh, celperrow, celpercol;
  /// selected cell
  bool selected;
  int selx, sely;
  /// the start of my palette in palettes
  int palstart;

public:
  csEditFontView(csComponent *iParent, csEditFont *fnt);
  ~csEditFontView();
  void Reevaluate(); //// number of chars in font and so on.
  virtual bool SetRect(int xmin, int ymin, int xmax, int ymax);
  virtual void Draw();
  void EditSettings();
  void HandleEditSettings(ModalData* data);
  virtual bool HandleEvent(iEvent &Event);
};


#endif // __CSFEDIT_H__

