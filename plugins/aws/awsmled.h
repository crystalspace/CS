#ifndef _AWS_MULTILINE_EDIT_H_
#define _AWS_MULTILINE_EDIT_H_
/*
    Copyright (C) 2002 by Norman Krämer
  
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

/**
 * This is a simple multiline edit control.
 */

#include "awscomp.h"
#include "awstimer.h"
#include "awsscr.h"
#include "csutil/csevent.h"
#include "csutil/csvector.h"

struct drawingContext
{
  iGraphics2D *g2d;
  iGraphics3D *g3d;

  /**
   * Get minimal rectangle needed to display content.
   * If maxExtend > -1 it is a constraint in x direction (xy=true) or y-direction (xy=false).
   */
  virtual void GetMinRect (drawingContext *dc, csRect &r, int maxExtend=-1, bool xy=true)=0;
  /**
   * Get optimal rectangle needed to display content.
   * If maxExtend > -1 it is a constraint in x direction (xy=true) or y-direction (xy=false).
   */
  virtual void GetOptRect (drawingContext *dc, csRect &r, int maxExtend=-1, bool xy=true)=0;
  /**
   * Expose the data inside rectangle r. Return in r the minimum enclosing rectangle of your data.
   */
  virtual void Expose (drawingContext *dc, csRect &r)=0;
  /**
   * Answer the request above if you had content to be exposed. If content is NULL then use the
   * content that is given to you in some other way.
   */
  virtual void WithData (void *content)=0;
};

struct textContext : public drawingContext
{
  int size;
  int fgcolor, bgcolor;
  csPoint pen;
  bool blink;
  bool underline;
  bool bold;
  bool italic;
};

class awsMultiLineEdit : public awsComponent
{
 protected:
  struct mlEvent
  {
    csEvent e;
    void (awsMultiLineEdit::*ring)();
  };

  class eventVector : public csVector
  {
  public:
    virtual ~eventVector () {DeleteAll (); }
    virtual bool FreeItem (csSome Item){delete (mlEvent*)Item; return true;}
    virtual int DoCompare (const csEvent *e1, const csEvent *e2, int Mode=0) const
    {
      int d = (int)e1->Type - (int)e2->Type;
      (void)Mode;
      if (d == 0)
      {
        if (CS_IS_KEYBOARD_EVENT (*e1))
          return memcmp (&e1->Key, &e2->Key, sizeof (e1->Key));
        if (CS_IS_MOUSE_EVENT (*e1))
          return memcmp (&e1->Mouse, &e2->Mouse, sizeof (e1->Mouse));
        if (CS_IS_JOYSTICK_EVENT (*e1))
          return memcmp (&e1->Joystick, &e2->Joystick, sizeof (e1->Joystick));
        return memcmp (e1, e1, sizeof (*e1));
      }
      return d;
    }
    virtual int Compare (csSome Item1, csSome Item2, int Mode=0) const
    {
      return DoCompare (&((mlEvent*)Item1)->e, &((mlEvent*)Item2)->e, Mode);
    }
    virtual int CompareKey (csSome Item, csConstSome Key, int Mode=0) const
    {
      return DoCompare (&((mlEvent*)Item)->e, (csEvent*)Key, Mode);
    }
    
    mlEvent* Get (int idx) const {return (mlEvent*)csVector::Get (idx);}

    bool Add (const csEvent &e, void (awsMultiLineEdit::*ring)())
    {
      mlEvent *ev = new mlEvent;
      ev->e = e;
      ev->ring = ring;
      if (InsertSorted ((csSome)ev) >= 0)
        return true;
      delete ev;
      return false;
    }
  };

  csVector vClipped;
  csVector vText;
  eventVector vDispatcher;
  awsActionDispatcher actions;

  iFont *font;
  iTextureHandle *img;
  int alpha_level;

  int style;
  csRect contentRect;

  int nMarkMode; // currently active mark mode
  int nClipMarkMode; // markmode the content of clipboard was created with
  bool bMarking; // marking in progress ?
  int mark_fromrow, mark_torow;
  int mark_fromcol, mark_tocol;
  int row, col; // cursor position

  int toprow; // first visible row
  int leftcol; // first visible col - doesn't make much sense for variable sized fonts
  int visrow, viscol;
  int ymaxchar, xmaxchar;
  /// The timer that makes the cursor blink.
  awsTimer *blink_timer;
  bool bBlinkOn;
  int cursorcolor;

  void MarkedToClipboard ();
  void InsertClipboard (int row, int col);
  bool GetMarked (int theRow, int &from, int &to);
  void MoveCursor (int theRow, int theCol);
  void InsertChar (int c);
  static void BlinkCursor (void *, iAwsSource *source);

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

  // actions
  static void actInsertRow (void *owner, iAwsParmList &parmlist);
  static void actDeleteRow (void *owner, iAwsParmList &parmlist);
  static void actReplaceRow (void *owner, iAwsParmList &parmlist);
  static void actGetRow (void *owner, iAwsParmList &parmlist);
  static void actGetRowCount (void *owner, iAwsParmList &parmlist);
  static void actClear (void *owner, iAwsParmList &parmlist);


 public:
  static const int MARK_ROWWRAP;
  static const int MARK_COLUMN;
  static const int MARK_ROW;

  static const int fsBump;
  static const int fsSimple;
  static const int fsRaised;
  static const int fsSunken;
  static const int fsFlat;
  static const int fsNone;
  static const int meHScroll;
  static const int meVScroll;
  static const int meNormal;
  static const int meBitmap;
  static const int frameMask;
  static const int styleMask;

  /**** signals ****/
  static const int signalPaste;
  static const int signalCopy;
  static const int signalCut;
  static const int signalEnter;
  static const int signalRowChanged;
  static const int signalColChanged;
  static const int signalLostFocus;

  /// Get's the texture handle and the title, plus style if there is one.
  virtual bool Setup (iAws *wmgr, awsComponentNode *settings);

  /// Gets properties
  bool GetProperty (char *name, void **parm);

  /// Sets properties
  bool SetProperty (char *name, void *parm);

  /// Returns the named TYPE of the component, like "Radio Button", etc.
  virtual char *Type ();

  /// Executes some actions
  virtual bool Execute (char *action, iAwsParmList &parmlist);

 public:

  awsMultiLineEdit ();
  virtual ~awsMultiLineEdit ();

  bool HandleEvent (iEvent &Event);

  /// Gets how big this button should ideally be.
  csRect getPreferredSize ();

  /// Gets the smallest this button can be.
  csRect getMinimumSize ();

  /// Triggered when the component needs to draw
  virtual void OnDraw (csRect clip);

  /// Triggered when the user presses a mouse button down
  virtual bool OnMouseDown (int button, int x, int y);

  /// Triggered when the user unpresses a mouse button
  virtual bool OnMouseUp (int button, int x, int y);

  /// Triggered when the user moves the mouse
  virtual bool OnMouseMove (int button, int x, int y);

  /// Triggered when the user clicks the mouse
  virtual bool OnMouseClick (int button, int x, int y);

  /// Triggered when the user double clicks the mouse
  virtual bool OnMouseDoubleClick (int button, int x, int y);

  /// Triggered when this component loses mouse focus
  virtual bool OnMouseExit ();

  /// Triggered when this component gains mouse focus
  virtual bool OnMouseEnter ();

  /// Triggered when the keyboard focus is lost
  virtual bool OnLostFocus ();

  /// Triggered when the keyboard focus is gained
  virtual bool OnGainFocus ();
};

class awsMultiLineEditFactory :
  public awsComponentFactory
{
public:

  /// Calls register to register the component that it builds with the window manager
  awsMultiLineEditFactory (iAws *wmgr);

  /// Does nothing
  virtual ~awsMultiLineEditFactory ();

  /// Returns a newly created component of the type this factory handles.
  virtual iAwsComponent *Create ();
};

#endif
