/*
    Copyright (C) 2002 by Norman Kraemer
  
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

#ifndef __CS_AWS_MLED_H__
#define __CS_AWS_MLED_H__

/**
 * This is a simple multiline edit control.
 */

#include "awscomp.h"
#include "awstimer.h"
#include "awsscr.h"
#include "iutil/csinput.h"
#include "csutil/csevent.h"
#include "csutil/parray.h"
#include "csutil/inputdef.h"

class awsMultiLineEdit : public awsComponent
{
protected:
  struct mlEvent
  {
    csInputDefinition inputDef;
    void (awsMultiLineEdit::*ring) ();
  };

  class eventVector : public csPDelArray<mlEvent>
  {
  public:
    static int Compare (mlEvent* const& Item1, mlEvent* const& Item2)
    {
      return Item1->inputDef.Compare (Item2->inputDef);
    }

    static int CompareEvent (mlEvent* const& Item, iEvent* const& Key)
    {
      return Item->inputDef.Compare (csInputDefinition (Key));
    }

    static csArrayCmp<mlEvent*,iEvent*> EventCmp(iEvent* e)
    {
      return csArrayCmp<mlEvent*,iEvent*>(e, CompareEvent);
    }

    bool Add (const csInputDefinition &e, void (awsMultiLineEdit::*ring) ())
    {
      mlEvent *ev = new mlEvent;
      ev->inputDef = e;
      ev->ring = ring;
      if (InsertSorted (ev, Compare) != csArrayItemNotFound)
	return true;
      delete ev;
      return false;
    }
  };

  csPDelArray<csString> vClipped;
  csPDelArray<csString> vText;
  eventVector vDispatcher;
  awsActionDispatcher* actions;

  csRef<iFont> font;
  iTextureHandle *img;
  int alpha_level;
  csRef<iKeyComposer> composer;

  int style;
  csRect contentRect;

  int nMarkMode; // Currently active mark mode.
  int nClipMarkMode; // Markmode the content of clipboard was created with.
  bool bMarking; // Marking in progress?
  int mark_fromrow, mark_torow;
  int mark_fromcol, mark_tocol;
  int row, col; // Cursor position.

  int toprow; // First visible row.
  int leftcol; // First visible col.
  int visrow, viscol;
  int ymaxchar, xmaxchar;
  awsTimer *blink_timer; // The timer that makes the cursor blink.
  bool bBlinkOn;
  int cursorcolor;

  void MarkedToClipboard ();
  void InsertClipboard (int row, int col);
  bool GetMarked (int theRow, int &from, int &to);
  void MoveCursor (int theRow, int theCol);
  void InsertChar (utf32_char c);
  static void BlinkCursor (intptr_t, iAwsSource *source);

  void SetDefaultHandler ();
  bool SetHandler (const char *action,  const char *event);
  void NextChar ();
  void PrevChar ();
  void NextWord ();
  void PrevWord ();
  void NextRow ();
  void PrevRow ();
  void BeginOfLine ();
  void EndOfLine ();
  void BeginOfText ();
  void EndOfText ();
  void ColumnMark ();
  void RowWrapMark ();
  void RowMark ();
  void BreakInsertRow ();
  void DeleteBackward ();
  void DeleteForward ();
  void CopyToClipboard ();
  void DeleteMarked ();
  void PasteClipboard ();
  void CutToClipboard ();

  static void actInsertRow (intptr_t owner, iAwsParmList* parmlist);
  static void actDeleteRow (intptr_t owner, iAwsParmList* parmlist);
  static void actReplaceRow (intptr_t owner, iAwsParmList* parmlist);
  static void actGetRow (intptr_t owner, iAwsParmList* parmlist);
  static void actGetRowCount (intptr_t owner, iAwsParmList* parmlist);
  static void actGetText (intptr_t owner, iAwsParmList* parmlist);
  static void actSetText (intptr_t owner, iAwsParmList* parmlist);
  static void actClear (intptr_t owner, iAwsParmList* parmlist);

public:
  /// Mark constants.
  enum
  {
    MARK_ROWWRAP = 1,
    MARK_ROW = 2,
    MARK_COLUMN = 3
  };

  /// Frame style constants.
  enum
  {
    fsBump = 0x0,
    fsSimple = 0x1,
    fsRaised = 0x2,
    fsSunken = 0x3,
    fsFlat = 0x4,
    fsNone = 0x5,
    meHScroll = 0x100,
    meVScroll = 0x200,
    meNormal = 0x400,
    meBitmap = 0x800
  };

  /// Mask constants.
  enum
  {
    frameMask = 0x7,
    styleMask = 0xff00
  };

  /// Signal constants.
  enum
  {
    signalPaste = 1,
    signalCopy = 2,
    signalCut = 3,
    signalEnter = 4,
    signalRowChanged = 5,
    signalColChanged = 6,
    signalLostFocus = 7
  };

  /// Get the texture handle and the title, plus style if there is one.
  virtual bool Setup (iAws *wmgr, iAwsComponentNode *settings);

  /// Get properties.
  bool GetProperty (const char *name, intptr_t *parm);

  /// Set properties.
  bool SetProperty (const char *name, intptr_t parm);

  /// Returns the named TYPE of the component, like "Radio Button", etc.
  virtual const char *Type ();

  /// Executes some actions.
  virtual bool Execute (const char *action, iAwsParmList* parmlist);

  awsMultiLineEdit ();
  virtual ~awsMultiLineEdit ();

  bool HandleEvent (iEvent &Event);

  /// Gets how big this button should ideally be.
  csRect getPreferredSize ();

  /// Gets the smallest this button can be.
  csRect getMinimumSize ();

  /// Triggered when the component needs to draw.
  virtual void OnDraw (csRect clip);

  /// Triggered when the user presses a mouse button down.
  virtual bool OnMouseDown (int button, int x, int y);

  /// Triggered when the user unpresses a mouse button.
  virtual bool OnMouseUp (int button, int x, int y);

  /// Triggered when the user moves the mouse.
  virtual bool OnMouseMove (int button, int x, int y);

  /// Triggered when the user clicks the mouse.
  virtual bool OnMouseClick (int button, int x, int y);

  /// Triggered when the user double clicks the mouse.
  virtual bool OnMouseDoubleClick (int button, int x, int y);

  /// Triggered when this component loses mouse focus.
  virtual bool OnMouseExit ();

  /// Triggered when this component gains mouse focus.
  virtual bool OnMouseEnter ();

  /// Triggered when the keyboard focus is lost.
  virtual bool OnLostFocus ();

  /// Triggered when the keyboard focus is gained.
  virtual bool OnGainFocus ();
};

class awsMultiLineEditFactory : public awsComponentFactory
{
public:
  /**
   * Calls register to register the component that it builds with the
   * window manager.
   */
  awsMultiLineEditFactory (iAws *wmgr);

  /// Does nothing.
  virtual ~awsMultiLineEditFactory ();

  /// Returns a newly created component of the type this factory handles.
  virtual iAwsComponent *Create ();
};

#endif // __CS_AWS_MLED_H__
