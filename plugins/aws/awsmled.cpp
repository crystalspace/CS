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

#include "cssysdef.h"
#include "csutil/csuctransform.h"
#include "csutil/csstring.h"
#include "csutil/inputdef.h"
#include "ivideo/fontserv.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "iutil/event.h"
#include "awsmled.h"
#include "aws3dfrm.h"
#include "iaws/awsparm.h"
#include "csutil/event.h"

static iAwsSink *textbox_sink = 0;

CS_IMPLEMENT_STATIC_VAR (GetTextBoxBlinkingCursorSlot, awsSlot,())

static awsSlot *textbox_slot = 0;

awsMultiLineEdit::awsMultiLineEdit () : actions(0)
{
  style = 0;
  toprow = 0;
  leftcol = 0;
  row = col = 0; // cursor pos
  bMarking = false;
  mark_fromrow = mark_torow = 0;
  mark_fromcol = mark_tocol = 0;

  nMarkMode = nClipMarkMode = MARK_ROWWRAP;
  xmaxchar = ymaxchar = 0;
  vText.Push (new csString);
  blink_timer = 0;
  bBlinkOn = false;
  textbox_slot = GetTextBoxBlinkingCursorSlot ();
  cursorcolor = 0;

  alpha_level = 128;
  img = 0;
}

awsMultiLineEdit::~awsMultiLineEdit ()
{
  vText.DeleteAll ();
  vClipped.DeleteAll ();

  if (blink_timer != 0)
  {
    textbox_slot->Disconnect (
        blink_timer,
        awsTimer::signalTick,
        textbox_sink,
        textbox_sink->GetTriggerID ("Blink"));
    delete blink_timer;
  }

  delete actions;
}

bool awsMultiLineEdit::Execute (const char *action, iAwsParmList* parmlist)
{
  if (awsComponent::Execute (action, parmlist)) return true;
  actions->Execute (action, (intptr_t)this, parmlist);
  return false;
}

void awsMultiLineEdit::MarkedToClipboard ()
{
  // clean up old content
  vClipped.DeleteAll ();

  nClipMarkMode = nMarkMode;

  int fromRow = MIN (mark_fromrow, mark_torow);
  int toRow = MAX (mark_fromrow, mark_torow);
  int fromCol = MIN (mark_fromcol, mark_tocol);
  int toCol = MAX (mark_fromcol, mark_tocol);

  if (nMarkMode == MARK_COLUMN)
  {
    for (int i=fromRow; i <= toRow; i++)
    {
      csString *s = vText[i];
      csString *m = new csString;
      m->Append (s->GetData () + fromCol, toCol-fromCol);
      vClipped.Push (m);
    }
  }
  else if (nMarkMode == MARK_ROWWRAP)
  {
    for (int i=fromRow; i <= toRow; i++)
    {
      csString *s = vText[i];
      csString *m = new csString;
      int off=0;
      int len=-1;

      if (i == fromRow && i == toRow)
        off = fromCol, len = toCol-fromCol;
      else if (i == fromRow)
        off = fromCol;
      else if (i == toRow)
        len = toCol;

      m->Append (s->GetData () + off, len);

      vClipped.Push (m);
    }
  }
  else if (nMarkMode == MARK_ROW)
  {
    for (int i=fromRow; i <= toRow; i++)
    {
      csString *s = vText[i];
      csString *m = new csString (*s);
      vClipped.Push (m);
    }
  }
}

void awsMultiLineEdit::InsertClipboard (int row, int col)
{
  if (vClipped.Length ())
  {
    // insert content of clipboard at (row,col)
    csString *target;

    if (nClipMarkMode == MARK_ROWWRAP)
    {
      size_t atrow = MIN ((size_t)(MAX (row, 0)), vText.Length ());
      if (atrow == vText.Length ())
      {
        target = new csString;
        vText.Push (target);
      }
      else
        target = vText[atrow];

      csString *left = new csString;
      csString *right = new csString;

      vText.DeleteIndex (atrow);

      int atcol = MIN (MAX (0, col), (int)target->Length ());

      left->Append (target->GetData (), MAX (0, atcol-1));
      right->Append (target->GetData ()+atcol, target->Length () - atcol);

      for (size_t i=0; i < vClipped.Length (); i++)
      {
        target = new csString (*vClipped[i]);
        if (i == 0)
          target->Insert (0, *left);
        if (i+1 == vClipped.Length ())
          target->Append (*right);
        vText.Insert (atrow+i, target);
      }
    }
    else if (nClipMarkMode == MARK_COLUMN)
    {
      for (size_t i=0; i < vClipped.Length (); i++, row++)
      {
        size_t atrow = MIN ((size_t)(MAX (row, 0)), vText.Length ());
        if (atrow == vText.Length ())
        {
          target = new csString;
          vText.Push (target);
        }
        else
          target = vText[atrow];

        int atcol = MIN (MAX (0, col), (int)target->Length ());

        target->Insert (atcol, *vClipped[i]);
      }
    }
    else if (nClipMarkMode == MARK_ROW)
    {
      size_t atrow = MIN ((size_t)(MAX (row, 0)), vText.Length ());
      for (size_t i=0; i < vClipped.Length (); i++, row++)
      {
        target = new csString (*vClipped[i]);
        vText.Insert (atrow, target);
      }
    }
  }
}

bool awsMultiLineEdit::Setup (iAws *wmgr, iAwsComponentNode *settings)
{
  if (!awsComponent::Setup (wmgr, settings)) return false;

  csRef<iKeyboardDriver> currentKbd = 
    CS_QUERY_REGISTRY (wmgr->GetObjectRegistry (), iKeyboardDriver);
  if (currentKbd == 0)
  {
    return false;
  }
  composer = currentKbd->CreateKeyComposer ();

  iAwsPrefManager *pm = WindowManager ()->GetPrefMgr ();

  actions = new awsActionDispatcher(WindowManager());
  actions->Register ("InsertRow", &actInsertRow);
  actions->Register ("DeleteRow", &actDeleteRow);
  actions->Register ("ReplaceRow", &actReplaceRow);
  actions->Register ("GetRow", &actGetRow);
  actions->Register ("GetRowCount", &actGetRowCount);
  actions->Register ("GetText", &actGetText);
  actions->Register ("SetText", &actSetText);
  actions->Register ("Clear", &actClear);

  pm->LookupIntKey ("ButtonTextureAlpha", alpha_level); // global get
  pm->GetInt (settings, "Alpha", alpha_level); // local overrides, if present.
  pm->GetInt (settings, "Style", style);
  // cursor color
  unsigned char r=255,g=0,b=0;
  if (pm->GetRGB (settings, "CursorColor", r, g, b))
    cursorcolor = pm->FindColor (r, g, b);
  else
    cursorcolor = pm->GetColor (AC_TEXTFORE);

  switch (style & styleMask)
  {
  case meBitmap:
    {
      iString *tn1 = 0;
      pm->GetString (settings, "Bitmap", tn1);
      if (tn1) img = pm->GetTexture (tn1->GetData (), tn1->GetData ());
    }
    break;
  case meNormal:
  default:
    img = pm->GetTexture ("Texture");
    break;
  }

  iString *fontname=0;
  pm->GetString (settings, "Font", fontname);

  if (fontname)
    SetProperty ("Font", (intptr_t)fontname);
  else
    SetProperty ("iFont", (intptr_t)pm->GetDefaultFont ());

  contentRect = Frame ();

  // Setup blink event handling
  if (textbox_sink == 0)
  {
    textbox_sink = WindowManager ()->GetSinkMgr ()->CreateSink ((intptr_t)0);
    textbox_sink->RegisterTrigger ("Blink", &BlinkCursor);
  }

  blink_timer = new awsTimer (WindowManager ()->GetObjectRegistry (), this);
  blink_timer->SetTimer (350);

  textbox_slot->Connect (blink_timer,
                         awsTimer::signalTick,
                         textbox_sink,
                         textbox_sink->GetTriggerID ("Blink"));

  SetDefaultHandler ();

  return true;
}

bool awsMultiLineEdit::GetProperty (const char *name, intptr_t *parm)
{
  return awsComponent::GetProperty (name, parm);
}

bool awsMultiLineEdit::SetProperty (const char *name, intptr_t parm)
{
  if (awsComponent::SetProperty (name, parm)) return true;

  if (strcmp (name, "Font") == 0)
  {
    // try to load the font set in parm
    iString *fontname = (iString*)parm;
    if (fontname)
    {
      iFontServer *fs = WindowManager ()->G2D ()->GetFontServer ();
      if (fs)
      {
        csRef<iFont> fnt (fs->LoadFont (fontname->GetData ()));
        if (fnt)
        {
          font = fnt;
          fnt->GetMaxSize (xmaxchar, ymaxchar);
          return true;
        }
      }
    }
    return false;
  }

  if (strcmp (name, "iFont") == 0)
  {
    // try to load the font set in parm
    iFont *fnt = (iFont*)parm;
    if (fnt)
    {
      font = csPtr<iFont> (fnt);
      fnt->GetMaxSize (xmaxchar, ymaxchar);
      return true;
    }
    return false;
  }

  return false;
}

const char *awsMultiLineEdit::Type ()
{
  return "Multiline Edit";
}

bool awsMultiLineEdit::HandleEvent (iEvent &Event)
{
  int idx;
  idx = vDispatcher.FindSortedKey (vDispatcher.EventCmp(&Event));
  if (idx != -1)
    (this->*vDispatcher.Get (idx)->ring) ();
  else
    if ((Event.Type == csevKeyboard) && 
      (csKeyEventHelper::GetEventType (&Event) == csKeyEventTypeDown))
    {
      csKeyEventData eventData;
      csKeyEventHelper::GetEventData (&Event, eventData);
      utf32_char Char[2];
      int composedSize;

      if (composer->HandleKey (eventData, Char, 
	sizeof (Char) / sizeof (utf32_char), &composedSize) != csComposeNoChar)
      {
	for (size_t n = 0; n < (size_t)composedSize; n++)
	  InsertChar (Char[n]);
      }
      //InsertChar (Event.Key.Char);
      return true;
    }
  if (awsComponent::HandleEvent (Event)) return true;

  return false;
}

void awsMultiLineEdit::SetDefaultHandler ()
{
  SetHandler ("next char", "Right");
  SetHandler ("prev char", "Left");
  SetHandler ("next word", "Ctrl+Right");
  SetHandler ("prev word", "Ctrl+Left");
  SetHandler ("next row", "Down");
  SetHandler ("prev row", "Up");
  SetHandler ("new row", "Enter");
  SetHandler ("del next char", "Del");
  SetHandler ("del prev char", "BackSpace");
  //  DEBUG_BREAK;
  SetHandler ("mark column", "Alt+c");
  SetHandler ("mark row", "Alt+l");
  SetHandler ("mark rowwrap", "Alt+m");
  SetHandler ("copy", "PAD+");
  SetHandler ("cut", "PAD-");
  SetHandler ("paste", "Ins");
  SetHandler ("eol", "End");
  SetHandler ("bol", "Home");
  SetHandler ("eot", "Ctrl+End");
  SetHandler ("bot", "Ctrl+Home");
}

bool awsMultiLineEdit::SetHandler (const char *action,  const char *event)
{
  csEvent e;
  bool bSucc = false;

  csInputDefinition inputDef (event);
  if (inputDef.IsValid ())
  {
    if (!strcmp (action, "next char"))
      bSucc = vDispatcher.Add (inputDef, &awsMultiLineEdit::NextChar);
    else if (!strcmp (action, "prev char"))
      bSucc = vDispatcher.Add (inputDef, &awsMultiLineEdit::PrevChar);
    else if (!strcmp (action, "next word"))
      bSucc = vDispatcher.Add (inputDef, &awsMultiLineEdit::NextWord);
    else if (!strcmp (action, "prev word"))
      bSucc = vDispatcher.Add (inputDef, &awsMultiLineEdit::PrevWord);
    else if (!strcmp (action, "next row"))
      bSucc = vDispatcher.Add (inputDef, &awsMultiLineEdit::NextRow);
    else if (!strcmp (action, "prev row"))
      bSucc = vDispatcher.Add (inputDef, &awsMultiLineEdit::PrevRow);
    else if (!strcmp (action, "new row"))
      bSucc = vDispatcher.Add (inputDef, &awsMultiLineEdit::BreakInsertRow);
    else if (!strcmp (action, "del next char"))
      bSucc = vDispatcher.Add (inputDef, &awsMultiLineEdit::DeleteForward);
    else if (!strcmp (action, "del prev char"))
      bSucc = vDispatcher.Add (inputDef, &awsMultiLineEdit::DeleteBackward);
    else if (!strcmp (action, "mark column"))
      bSucc = vDispatcher.Add (inputDef, &awsMultiLineEdit::ColumnMark);
    else if (!strcmp (action, "mark row"))
      bSucc = vDispatcher.Add (inputDef, &awsMultiLineEdit::RowMark);
    else if (!strcmp (action, "mark rowwrap"))
      bSucc = vDispatcher.Add (inputDef, &awsMultiLineEdit::RowWrapMark);
    else if (!strcmp (action, "copy"))
      bSucc = vDispatcher.Add (inputDef, &awsMultiLineEdit::CopyToClipboard);
    else if (!strcmp (action, "cut"))
      bSucc = vDispatcher.Add (inputDef, &awsMultiLineEdit::CutToClipboard);
    else if (!strcmp (action, "paste"))
      bSucc = vDispatcher.Add (inputDef, &awsMultiLineEdit::PasteClipboard);
    else if (!strcmp (action, "eol"))
      bSucc = vDispatcher.Add (inputDef, &awsMultiLineEdit::EndOfLine);
    else if (!strcmp (action, "bol"))
      bSucc = vDispatcher.Add (inputDef, &awsMultiLineEdit::BeginOfLine);
    else if (!strcmp (action, "eot"))
      bSucc = vDispatcher.Add (inputDef, &awsMultiLineEdit::EndOfText);
    else if (!strcmp (action, "bot"))
      bSucc = vDispatcher.Add (inputDef, &awsMultiLineEdit::BeginOfText);
  }
  return bSucc;
}

csRect awsMultiLineEdit::getPreferredSize ()
{
  if (set_preferred_size)
    return preferred_size;

  // the preferred size we define as the one showing the whole content
  int nHeight = 0;
  int nWidth = 0;
  int w, h;

  for (size_t i=0; i < vText.Length (); i++)
  {
    font->GetDimensions (vText[i]->GetData (), w, h);
    nHeight += h;
    nWidth = MAX (nWidth, w);
  }
  return csRect (0, 0, nWidth, nHeight);
}

csRect awsMultiLineEdit::getMinimumSize ()
{
  // minimum size we define as the rectangle big enough to show one character and the scrollbars
  // if the current type doesnt include scrollbars we return the size this control was created with

  return contentRect;
}

void awsMultiLineEdit::OnDraw (csRect)
{
  iGraphics2D *g2d = WindowManager ()->G2D ();
  iGraphics3D *g3d = WindowManager ()->G3D ();
  int fg = WindowManager ()->GetPrefMgr ()->GetColor (AC_TEXTFORE);
  int bg = WindowManager ()->GetPrefMgr ()->GetColor (AC_TEXTBACK);
  int dfill = WindowManager ()->GetPrefMgr ()->GetColor (AC_DARKFILL);

  csRect cr;
  csRect nr;
  csRect f;
  g2d->GetClipRect (cr.xmin, cr.ymin, cr.xmax, cr.ymax);

  nr = cr;
  contentRect = Frame ();

  csRect &r = contentRect;
  switch (style & frameMask)
  {
  case fsBump:
    f.Set (r.xmin + 4, r.ymin + 4, r.xmax - 3, r.ymax - 3);
    r.Set (r.xmin + 6, r.ymin + 6, r.xmax - 5, r.ymax - 5);
    break;
  case fsFlat:
  case fsRaised:
  case fsSunken:
    f.Set (r.xmin, r.ymin, r.xmax+1, r.ymax+1);
    r.Set (r.xmin+2, r.ymin+2, r.xmax, r.ymax);
    break;
  }

  int img_w=0, img_h=0;
  if (img)
    img->GetOriginalDimensions (img_w, img_h);

  switch (style & styleMask)
  {
  case meBitmap:
    if (img)
      g3d->DrawPixmap (img, f.xmin, f.ymin, f.Width (), f.Height (), 
                       0, 0, img_w, img_h, alpha_level);
    break;
  default:
  case meNormal:
      aws3DFrame frame3d;
      frame3d.Setup(WindowManager(),img, alpha_level);
  frame3d.Draw (
      Frame (),
      style & styleMask,
      Window()->Frame());
    g2d->DrawBox (f.xmin, f.ymin, f.Width (), f.Height (), dfill);
    if (img)
      g3d->DrawPixmap (img, f.xmin, f.ymin, f.Width (), f.Height (), 
                       0, 0, img_w, img_h, alpha_level);
    break;
  }

  nr.Intersect (contentRect);

  g2d->SetClipRect (nr.xmin, nr.ymin, nr.xmax, nr.ymax);

  int y=contentRect.ymin;
  char p[CS_UC_MAX_UTF8_ENCODED + 1];
  //char p[2];
  //p[1] = '\0';

  for (size_t i=toprow; i < vText.Length () && y < nr.ymax; i++)
  {
    if (y >= nr.ymin)
    {
      csString *s = vText[i];
      size_t strOfs = 0;
      int col = leftcol;
      while ((strOfs < s->Length ()) && (col > 0))
      {
	strOfs += csUnicodeTransform::UTF8Skip (
	  (utf8_char*)s->GetData () + strOfs, s->Length () - strOfs);
	col--;
      }

      //if ((int)s->Length () > leftcol)
      if (strOfs < s->Length ())
      {
        // check if we have a marked substring to draw
        int m_from=-1, m_to=-1;
        GetMarked (i, m_from, m_to);

        // this routine forces the variable width fonts to be displayed as fixed width ones
        int theCol = leftcol;
	int theOfs = strOfs;
        int fcolor, bcolor;
        const char *str = s->GetData () + theOfs;
        int x = contentRect.xmin;

        if (m_from < m_to)
        {
          int len = m_to - MAX (m_from, leftcol);
          if (len > 0)
          {
            int inv_start = MAX (m_from, leftcol) - leftcol;
            int inv_end = MIN (len, 1+contentRect.Width ()/xmaxchar);
            g2d->DrawBox (contentRect.xmin + inv_start*xmaxchar, y, inv_end*xmaxchar, ymaxchar, fg);
          }
        }

	size_t slen = s->Length () - strOfs;
        // @@@ : enhance - only try to write th chars visible inside the nr area
        while (slen > 0)
        {
	  int chSize = csUnicodeTransform::UTF8Skip ((utf8_char*)str, slen);
	  memcpy (p, str, chSize);
	  p[chSize] = 0;
	  slen -= chSize;
	  str += chSize;
          //p[0] = *str++;
          if (theCol >= m_from && theCol < m_to)
            fcolor = bg, bcolor = -1;
          else
            fcolor = fg, bcolor = -1;
          g2d->Write (font, x, y, fcolor, bcolor, p);
          x += xmaxchar;
          theCol++;
        }
      }
      y += ymaxchar;
    }
  }

  if (visrow >= 0 && viscol >= 0 && bBlinkOn)
  {
    // draw the cursor line
    int y_cur = contentRect.ymin + visrow * ymaxchar;
    int x_cur = contentRect.xmin + viscol * xmaxchar;
    g2d->DrawBox (x_cur, y_cur, xmaxchar, ymaxchar, cursorcolor);
  }

  // set original clip rect
  g2d->SetClipRect (cr.xmin, cr.ymin, cr.xmax, cr.ymax);
  
}

bool awsMultiLineEdit::GetMarked (int theRow, int &from, int &to)
{
  if (bMarking)
  {
    // first check if we have this row in the list of marked ones
    int fromRow, toRow;
    int fromCol, toCol;

    if (mark_fromrow < mark_torow)
      fromRow = mark_fromrow, toRow = mark_torow;
    else
      fromRow = mark_torow, toRow = mark_fromrow;

    if (nMarkMode == MARK_ROWWRAP)
    {
      if (mark_fromrow < mark_torow)
        fromCol = mark_fromcol, toCol = mark_tocol;
      else if (mark_fromrow > mark_torow)
        fromCol = mark_tocol, toCol = mark_fromcol;
      else
        fromCol = MIN (mark_fromcol, mark_tocol), toCol = MAX (mark_fromcol, mark_tocol);
    }
    else if (mark_fromcol < mark_tocol)
      fromCol = mark_fromcol, toCol = mark_tocol;
    else
      fromCol = mark_tocol, toCol = mark_fromcol;

    if (theRow >= fromRow && theRow <= toRow)
    {
      if (nMarkMode == MARK_COLUMN)
        from = fromCol, to = toCol;
      else if (nMarkMode == MARK_ROWWRAP)
      {
        if (theRow == fromRow && theRow == toRow)
          from = fromCol, to = toCol;
        else if (theRow == fromRow)
          from = fromCol, to = vText[theRow]->Length ();
        else if (theRow == toRow)
          from = 0, to = toCol;
        else
          from = 0, to = vText[theRow]->Length ();
      }
      else if (nMarkMode == MARK_ROW)
        from = 0, to = vText[theRow]->Length ();
      return true;
    }
  }
  return false;
}

void awsMultiLineEdit::InsertChar (utf32_char c)
{
  if (c)
  {
    if (vText.Length () == 0)
      vText.Push (new csString);
    csString *s = vText[row];
    utf8_char ch[CS_UC_MAX_UTF8_ENCODED + 1];
    int chSize = csUnicodeTransform::EncodeUTF8 (c,
      ch, sizeof (ch) / sizeof (utf8_char));
    ch[chSize] = 0;

    size_t strOfs = 0;
    int curCol = col;
    while ((strOfs < s->Length ()) && (curCol > 0))
    {
      strOfs += csUnicodeTransform::UTF8Skip (
	(utf8_char*)s->GetData () + strOfs, s->Length () - strOfs);
      curCol--;
    }

    //s->Insert (col, (const char)c);
    s->Insert (strOfs, (char*)ch);
    MoveCursor (row, col+1);  
  }
}

bool awsMultiLineEdit::OnMouseDown (int, int, int)
{
  return true;
}

bool awsMultiLineEdit::OnMouseUp (int, int, int)
{
  return true;
}

bool awsMultiLineEdit::OnMouseMove (int, int, int)
{
  return false;
}

bool awsMultiLineEdit::OnMouseClick (int, int, int)
{
  return true;
}

bool awsMultiLineEdit::OnMouseDoubleClick (int, int, int)
{
  return true;
}

bool awsMultiLineEdit::OnMouseExit ()
{
  return true;
}

bool awsMultiLineEdit::OnMouseEnter ()
{
  return true;
}

bool awsMultiLineEdit::OnLostFocus ()
{
  blink_timer->Stop ();
  bBlinkOn = false;
  Broadcast (signalLostFocus);
  return true;
}

bool awsMultiLineEdit::OnGainFocus ()
{
  blink_timer->Start ();
  return true;
}

void awsMultiLineEdit::MoveCursor (int theRow, int theCol)
{
  theRow = MAX (MIN (theRow, (int)vText.Length ()-1), 0);
  csString *s = vText[theRow];
  theCol = MAX (MIN (theCol, (int)s->Length ()), 0);

  visrow = theRow - toprow;
  viscol = theCol - leftcol;

  // calculate new toprow/leftcol
  if (visrow < 0)
  {
    // scroll to become visible at the top
    visrow = 0;
    toprow = theRow;
  }
  else
  {
    int y = (visrow+1) * ymaxchar;
    int diff = contentRect.Height () - y;
    if (diff < 0)
    {
      y = MAX (1, (-diff/ymaxchar));
      visrow -= y;
      toprow += y;
    }
  }

  if (viscol < 0)
  {
    // scroll to become visible at the top
    viscol = 0;
    leftcol = theCol;
  }
  else
  {
    int x = (viscol+1) * xmaxchar;
    int diff = contentRect.Width () - x;
    if (diff < 0)
    {
      x = MAX (1, (-diff/xmaxchar));
      viscol -= x;
      leftcol += x;
    }
  }
  
  if (theRow != row)
    Broadcast (signalRowChanged);

  if (theCol != col)
    Broadcast (signalColChanged);

  row = theRow;
  col = theCol;

  if (bMarking)
  {
    mark_torow = row;
    mark_tocol = col;
  }
}

void awsMultiLineEdit::NextChar ()
{
  csString *s = vText[row];
  if (col < (int)s->Length ())
    MoveCursor (row, col+1);
}

void awsMultiLineEdit::PrevChar ()
{
  if (col > 0)
    MoveCursor (row, col-1);
}

void awsMultiLineEdit::NextWord ()
{
  int from=col;
  bool found_space=false;
  while ((size_t)row < vText.Length ())
  {
    csString *s = vText[row];
    const char *p = s->GetData () + from;
    from = 0;
    
    if (!found_space)
    {
      found_space = true; // we either find a space or hit the end (which we count as space too)
      size_t len = strcspn (p, " \t\n");
      if (len == strlen (p))
      {
        row++;
        col = s->Length ()+1;
        continue;
      }
      p += len;
      col += len;
    }
    // find something thats not a space
    size_t len = strspn (p, " \t\n");
    if (len == strlen (p))
    {
      row++;
      col = s->Length ()+1;
      continue;
    }
    col += len;
    break;
  }

  if ((size_t)row == vText.Length ())
    row--;
  MoveCursor (row, col);
}

void awsMultiLineEdit::PrevWord ()
{
  int from=col;
  bool found_space=false;

  while (row >= 0)
  {
    csString *s = vText[row];
    const char *start = s->GetData ();
    const char *p = s->GetData ();
    if (found_space)
    {
      p += s->Length ();
      col = s->Length ();
    }
    else
      p += from;
    
    p--;
    col--;

    found_space = true;

    while (p > start && strspn (p, " \t\n") > 0) 
    { 
      p--;
      col--;
    }

    if (p <= start)
    {
      row--;
      continue;
    }
      
    // find something thats not a space
    while (p > start && strcspn (p, " \t\n") > 0) 
    { 
      p--;
      col--;
    }
    
    if (p!=start || strspn (p, " \t\n") > 0)
      col++;
    break;
  }

  if (row < 0)
  {
    row = 0;
    col = 0;
  }
  MoveCursor (row, col);
}

void awsMultiLineEdit::NextRow ()
{
  if (row < (int)vText.Length ()-1)
    MoveCursor (row+1, col);
}

void awsMultiLineEdit::PrevRow ()
{
  if (row > 0)
    MoveCursor (row-1, col);
}

void awsMultiLineEdit::EndOfText ()
{
  row = vText.Length ()-1;
  col = vText[row]->Length ();
  MoveCursor (row, col);
}

void awsMultiLineEdit::BeginOfText ()
{
  row = 0;
  col = 0;
  MoveCursor (row, col);
}

void awsMultiLineEdit::EndOfLine ()
{
  col = vText[row]->Length ();
  MoveCursor (row, col);
}

void awsMultiLineEdit::BeginOfLine ()
{
  col = 0;
  MoveCursor (row, col);
}

void awsMultiLineEdit::BreakInsertRow ()
{
  // at the current col we break the current row and insert a new one
  csString *s = vText[row];
  csString *sn = new csString;
  sn->Append (s->GetData () + col);
  s->Truncate (col);
  vText.Insert (row+1, sn);
  col = 0;
  MoveCursor (row+1, col);
  Broadcast (signalEnter);
}

void awsMultiLineEdit::DeleteBackward ()
{
  // delete the character before the cursor, append the current line to the previous
  // if the current cursor pos is 0
  if (col > 0 || row > 0)
  {
    csString *s = vText[row];
    if (col > 0)
    {
      s->DeleteAt (col-1);
      col--;
    }
    else
    {
      csString *sp = vText[row-1];
      col = sp->Length ();
      sp->Append (*s);
      vText.DeleteIndex (row);
      row--;
    }
    MoveCursor (row, col);
  }
}

void awsMultiLineEdit::DeleteForward ()
{
  // delete the character after the cursor, append the next line to the current
  // if the current cursor pos is at the end of line
  
  csString *s = vText[row];
  if (col < (int)s->Length () || row < (int)vText.Length ()-1)
  {
    if (col < (int)s->Length ())
      s->DeleteAt (col);
    else
    {
      csString *sn = vText[row+1];
      s->Append (*sn);
      vText.DeleteIndex (row+1);
    }
  }
}

void awsMultiLineEdit::ColumnMark ()
{
  nMarkMode = MARK_COLUMN;
  mark_fromrow = mark_torow = row;
  mark_fromcol = mark_tocol = col;
  bMarking = !bMarking;
}

void awsMultiLineEdit::RowWrapMark ()
{
  nMarkMode = MARK_ROWWRAP;
  mark_fromrow = mark_torow = row;
  mark_fromcol = mark_tocol = col;
  bMarking = !bMarking;
}

void awsMultiLineEdit::RowMark ()
{
  nMarkMode = MARK_ROW;
  mark_fromrow = mark_torow = row;
  mark_fromcol = mark_tocol = col;
  bMarking = !bMarking;
}

void awsMultiLineEdit::CopyToClipboard ()
{
  if (!bMarking)
  {
    // copy current line to clipboard
    RowMark ();
  }
  MarkedToClipboard ();
  bMarking = false;
  Broadcast (signalCopy);
}

void awsMultiLineEdit::DeleteMarked ()
{
  if (bMarking)
  {
    int fromRow, toRow;
    int fromCol, toCol;

    if (mark_fromrow < mark_torow)
      fromRow = mark_fromrow, toRow = mark_torow;
    else
      fromRow = mark_torow, toRow = mark_fromrow;

    if (nMarkMode == MARK_ROWWRAP)
    {
      if (mark_fromrow < mark_torow)
        fromCol = mark_fromcol, toCol = mark_tocol;
      else if (mark_fromrow > mark_torow)
        fromCol = mark_tocol, toCol = mark_fromcol;
      else
        fromCol = MIN (mark_fromcol, mark_tocol), toCol = MAX (mark_fromcol, mark_tocol);
    }
    else if (mark_fromcol < mark_tocol)
      fromCol = mark_fromcol, toCol = mark_tocol;
    else
      fromCol = mark_tocol, toCol = mark_fromcol;

    if (nMarkMode == MARK_ROWWRAP)
      for (int i=toRow; i >= fromRow; i--)
      {
        csString *s = vText[i];
        int from = 0, to = 0;

        if (i == fromRow && i == toRow)
          from = fromCol, to = MIN (toCol, (int)s->Length ());
        else if (i == fromRow)
          from = fromCol, to = s->Length ();
        else if (i == fromRow)
          from = 0, to = MIN (toCol, (int)s->Length ());

        if (i > fromRow && i < toRow)
        {
          vText.DeleteIndex (i);
        }
        else
        {
          if (from < to)
            s->DeleteAt (from, to-from);
        }
      }
    else if (nMarkMode == MARK_COLUMN)
      for (int i=toRow; i >= fromRow; i--)
      {
        csString *s = vText[i];
        int from = fromCol, to = MIN (toCol, (int)s->Length ());
        if (from < to)
          s->DeleteAt (from, to-from);
      }
    else if (nMarkMode == MARK_ROW)
      for (int i=toRow; i >= fromRow; i--)
      {
        vText.DeleteIndex (i);
      }

    // make sure we didnt delete everything, if so put in a new empty first line
    if (vText.Length () == 0)
      vText.Push (new csString);

    MoveCursor (fromRow, fromCol);
  }
}

void awsMultiLineEdit::PasteClipboard ()
{
  DeleteMarked ();
  bMarking = false;
  InsertClipboard (row, col);
  MoveCursor (row, col);
  Broadcast (signalPaste);
}

void awsMultiLineEdit::CutToClipboard ()
{
  if (!bMarking)
  {
    // copy current line to clipboard
    RowMark ();
  }
  MarkedToClipboard ();
  DeleteMarked ();
  bMarking = false;
  Broadcast (signalCut);
}


void awsMultiLineEdit::actInsertRow (intptr_t owner, iAwsParmList* parmlist)
{
  if (!parmlist)
    return;
  
  awsMultiLineEdit *me = (awsMultiLineEdit *)owner;
  int row;
  iString *str=0;

  if (parmlist->GetInt ("row", &row)
  	&& parmlist->GetString ("string", &str)
	&& row <= (int)me->vText.Length () && row >= 0)
  {
    csString *s = new csString (str->GetData ());
    me->vText.Insert (row, s);
  }
}

void awsMultiLineEdit::actDeleteRow (intptr_t owner, iAwsParmList* parmlist)
{
  if (parmlist)
    return;
  
  awsMultiLineEdit *me = (awsMultiLineEdit *)owner;
  int row;

  if (parmlist->GetInt ("row", &row) && row < (int)me->vText.Length () && row >= 0)
  {
    me->vText.DeleteIndex (row);
    me->MoveCursor (me->row, me->col); // in case we removed the last line
  }
}

void awsMultiLineEdit::actReplaceRow (intptr_t owner, iAwsParmList* parmlist)
{
  if (!parmlist)
    return;
  
  awsMultiLineEdit *me = (awsMultiLineEdit *)owner;
  int row;
  iString *str=0;

  if (parmlist->GetInt ("row", &row)
  	&& parmlist->GetString ("string", &str)
	&& row <= (int)me->vText.Length () && row >= 0)
  {
    csString *s = new csString (str->GetData ());
    me->vText.Insert (row, s);
    me->vText.DeleteIndex (row+1);
  }
}

void awsMultiLineEdit::actGetRow (intptr_t owner, iAwsParmList* parmlist)
{
  if (!parmlist)
    return;
  
  awsMultiLineEdit *me = (awsMultiLineEdit *)owner;
  int row;

  if (parmlist->GetInt ("row", &row) && row < (int)me->vText.Length () && row >= 0)
  {
    parmlist->AddString ("string", *(me->vText[row]) );
  }
}

void awsMultiLineEdit::actClear (intptr_t owner, iAwsParmList* )
{
  awsMultiLineEdit *me = (awsMultiLineEdit *)owner;
  me->vText.DeleteAll ();
  me->MoveCursor (me->row, me->col); // in case we removed the last line
}

void awsMultiLineEdit::actGetRowCount (intptr_t owner, iAwsParmList* parmlist)
{
  if (!parmlist)
    return;
  
  awsMultiLineEdit *me = (awsMultiLineEdit *)owner;
  parmlist->AddInt ("count", me->vText.Length ());
}

void awsMultiLineEdit::actGetText (intptr_t owner, iAwsParmList* parmlist)
{
  if (!parmlist)
    return;
  
  awsMultiLineEdit *me = (awsMultiLineEdit *)owner;
  csString text;
  for (size_t i = 0; i < me->vText.Length (); i++)
  {
    text.Append (me->vText[i]->GetData ());
    if (i < me->vText.Length ()-1)
      text.Append ("\n");
  }
  parmlist->AddString ("text", text);
}

void awsMultiLineEdit::actSetText (intptr_t owner, iAwsParmList* parmlist)
{
  if (!parmlist)
    return;
  
  awsMultiLineEdit *me = (awsMultiLineEdit *)owner;
  iString *text = 0;

  if (parmlist->GetString ("text", &text))
  {
    me->vText.DeleteAll ();
    
    if (text)
    {
      const char *p = text->GetData ();
      int len;
      while (*p)
      {
        csString *s = new csString;
        len=strcspn (p, "\n");
        s->Append (p, len);
        me->vText.Push (s);
        p += len+1;
      }
    }
  }
}

void awsMultiLineEdit::BlinkCursor (intptr_t, iAwsSource *source)
{
  awsMultiLineEdit *comp = (awsMultiLineEdit*)source->GetComponent ();
  comp->bBlinkOn = !comp->bBlinkOn;
}

/************************************* Multiline Edit Factory ****************/
awsMultiLineEditFactory::awsMultiLineEditFactory (iAws *wmgr) 
  :  awsComponentFactory(wmgr)
{
  Register ("Multiline Edit");
  RegisterConstant ("mefsBump", awsMultiLineEdit::fsBump);
  RegisterConstant ("mefsSimple", awsMultiLineEdit::fsSimple);
  RegisterConstant ("mefsRaised", awsMultiLineEdit::fsRaised);
  RegisterConstant ("mefsSunken", awsMultiLineEdit::fsSunken);
  RegisterConstant ("mefsFlat", awsMultiLineEdit::fsFlat);
  RegisterConstant ("mefsNone", awsMultiLineEdit::fsNone);
  RegisterConstant ("meNormal", awsMultiLineEdit::meNormal);
  RegisterConstant ("meBitmap", awsMultiLineEdit::meBitmap);
  RegisterConstant ("meHScroll", awsMultiLineEdit::meHScroll);
  RegisterConstant ("meVScroll", awsMultiLineEdit::meVScroll);

  RegisterConstant ("signalPaste", awsMultiLineEdit::signalPaste);
  RegisterConstant ("signalCopy", awsMultiLineEdit::signalCopy);
  RegisterConstant ("signalCut", awsMultiLineEdit::signalCut);
  RegisterConstant ("signalEnter", awsMultiLineEdit::signalEnter);
  RegisterConstant ("signalRowChanged", awsMultiLineEdit::signalRowChanged);
  RegisterConstant ("signalColChanged", awsMultiLineEdit::signalColChanged);
  RegisterConstant ("signalLostFocus", awsMultiLineEdit::signalLostFocus);
}

awsMultiLineEditFactory::~awsMultiLineEditFactory ()
{
  // empty
}

iAwsComponent *awsMultiLineEditFactory::Create ()
{
  return new awsMultiLineEdit;
}

