/*
    Crystal Space Windowing System: input line class
    Copyright (C) 1998,1999 by Andrew Zabolotny <bit@eltech.ru>

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
#include "csws/csiline.h"
#include "csws/cstimer.h"
#include "csws/csapp.h"
#include "csutil/csinput.h"
#include "csutil/event.h"

// Cursor flashing period in milliseconds
#define CURSOR_FLASHING_INTERVAL	333

csInputLine::csInputLine (csComponent *iParent, int iMaxLen,
  csInputLineFrameStyle iFrameStyle) : csComponent (iParent)
{
  state |= CSS_SELECTABLE;
  maxlen = iMaxLen;
  firstchar = 0;
  cursorpos = 0;
  selstart = selend = 0;
  textx = 0;
  text = new char [iMaxLen + 1];
  SetText (0);
  FrameStyle = iFrameStyle;
  SetPalette (CSPAL_INPUTLINE);
  timer = new csTimer (this, CURSOR_FLASHING_INTERVAL);
}

void csInputLine::SetText (const char *iText)
{
  if (!iText)
    iText = "";

  size_t sl = strlen (iText);
  if (sl > maxlen)
    sl = maxlen;
  memcpy (text, iText, sl);
  text [sl] = 0;
  firstchar = 0;
  SetCursorPos (sl, false);
  SetSelection (0, GetState (CSS_DISABLED) ? 0 : sl);
  cursorvis = true;
  Invalidate ();
}

void csInputLine::SetTextExt (const char *iText)
{
  size_t sl = strlen (iText);
  if (sl > maxlen)
    sl = maxlen;
  memcpy (text, iText, sl);
  text [sl] = 0;
  if (cursorpos > sl)
    cursorpos = sl;
  if (firstchar > sl)
    firstchar = sl;
  SetSelection (0, 0);
  cursorvis = true;
  Invalidate ();
}

void csInputLine::Draw ()
{
  int dx = 0, dy = 0;
  bool Disabled = GetState (CSS_DISABLED) ? true : false;

  switch (FrameStyle)
  {
    case csifsNone:
      break;
    case csifsThinRect:
      Rect3D (0, 0, bound.Width (), bound.Height (),
          CSPAL_INPUTLINE_LIGHT3D, CSPAL_INPUTLINE_DARK3D);
      Rect3D (1, 1, bound.Width () - 1, bound.Height () - 1,
          CSPAL_INPUTLINE_DARK3D, CSPAL_INPUTLINE_LIGHT3D);
      dx = dy = 2;
      break;
    case csifsThickRect:
      Rect3D (0, 0, bound.Width (), bound.Height (),
          CSPAL_INPUTLINE_LIGHT3D, CSPAL_INPUTLINE_DARK3D);
      Rect3D (1, 1, bound.Width () - 1, bound.Height () - 1,
          CSPAL_INPUTLINE_2LIGHT3D, CSPAL_INPUTLINE_2DARK3D);
      dx = dy = 2;
      break;
  } /* endswitch */
  Box (dx, dy, bound.Width () - dx, bound.Height () - dy,
    ((FrameStyle == csifsThickRect) && !Disabled) ?
    CSPAL_INPUTLINE_BACKGROUND2 : CSPAL_INPUTLINE_BACKGROUND);
  SetClipRect (dx, dy, bound.Width () - dx, bound.Height () - dy);

  csComponent::Draw ();

  int fonth;
  GetTextSize ("", &fonth);
  textx = dx;
  texty = dy + (clip.Height () - fonth) / 2;

  size_t sels = selstart, sele = selend;
  if (sels > sele)
  {
    sels = selend;
    sele = selstart;
  } /* endif */

  if (sels < firstchar)
    sels = firstchar;
  if ((sels >= sele)
   || !GetState (CSS_FOCUSED)
   || Disabled)
    sels = sele = 0;

  // draw selection
  if (sels != sele)
  {
    int sx = GetCharX (sels);
    int ex = GetCharX (sele);
    Box (sx, dy, ex, 999, CSPAL_INPUTLINE_SELBACKGROUND);
  } /* endif */

  // compute cursor rectangle
  cursorrect.MakeEmpty ();
  if (cursorpos >= firstchar)
  {
    int cx = GetCharX (cursorpos);
    if (cx < clip.xmax)
      if (app->InsertMode || (cursorpos == strlen (text)))
        cursorrect.Set (cx - 1, texty - 1, cx + 1, texty + fonth + 1);
      else
      {
        int cex = GetCharX (cursorpos + 1);
        cursorrect.Set (cx, texty - 1, cex, texty + fonth + 1);
      }
  } /* endif */

  // draw text
  if (!app->InsertMode && cursorvis && GetState (CSS_FOCUSED) && !Disabled)
  {
    if (sels == sele)
    {
      sels = cursorpos;
      sele = cursorpos + 1;
    }
    else if (cursorpos == sele)
      sele++;

    // if cursor is in overstrike mode, draw it before text
    if (!cursorrect.IsEmpty ())
      Box (cursorrect.xmin, cursorrect.ymin, cursorrect.xmax,
        cursorrect.ymax, CSPAL_INPUTLINE_TEXT);
  } /* endif */

  if (sels != sele)
  {
    size_t sc = sels < firstchar ? firstchar : sels;
    int cx = textx;
    char tmp = text [sc];
    text [sc] = 0;
    Text (cx, texty, CSPAL_INPUTLINE_TEXT, -1, &text [firstchar]);
    cx += GetTextSize (&text [firstchar]);
    text [sc] = tmp;
    tmp = text [sele];
    text [sele] = 0;
    Text (cx, texty, CSPAL_INPUTLINE_SELTEXT, -1, &text [sc]);
    cx += GetTextSize (&text [sc]);
    text [sele] = tmp;
    if (sele < strlen (text))
      Text (cx, texty, CSPAL_INPUTLINE_TEXT, -1, &text [sele]);
  }
  else
    Text (textx, texty, CSPAL_INPUTLINE_TEXT, -1, &text [firstchar]);

  // if cursor is in insert state, draw
  if (app->InsertMode && cursorvis && GetState (CSS_FOCUSED) && !Disabled)
  {
    if (!cursorrect.IsEmpty ())
      Box (cursorrect.xmin, cursorrect.ymin, cursorrect.xmax,
        cursorrect.ymax, CSPAL_INPUTLINE_TEXT);
  } /* endif */
}

bool csInputLine::HandleEvent (iEvent &Event)
{
  switch (Event.Type)
  {
    case csevCommand:
      switch (Event.Command.Code)
      {
        case cscmdTimerPulse:
          if ((Event.Command.Info == (intptr_t)timer) && GetState(CSS_FOCUSED))
          {
            cursorvis = !cursorvis;
            Invalidate (cursorrect);
          } /* endif */
          return true;
      } /* endswitch */
      break;
    case csevMouseDown:
      Select ();
      if (Event.Mouse.Button != 1)
        return true;
      app->CaptureMouse (this);
      // fallback to mouse move
    case csevMouseMove:
      if (app->MouseOwner == this)
      {
        size_t i, sl = strlen (text);
        if ((Event.Mouse.x < textx) && (firstchar > 0))
          i = firstchar - 1;
        else
          for (i = firstchar; (i < sl) && (Event.Mouse.x >= GetCharX (i + 1)); i++)
            ;
        SetCursorPos (i, Event.Type == csevMouseMove);
      } /* endif */
      return true;
    case csevMouseUp:
      if (Event.Mouse.Button == 1)
        app->CaptureMouse (0);
      return true;
    case csevMouseDoubleClick:
      if (Event.Mouse.Button == 1)
      {
        if (selstart < selend)
          SetSelection (0, strlen (text));
        else
          SetSelection (WordLeft (text, cursorpos), WordRight (text, cursorpos));
        return true;
      }
      break;
    case csevKeyboard:
      if (csKeyEventHelper::GetEventType (&Event) == csKeyEventTypeDown)
      {
	switch (csKeyEventHelper::GetCookedCode (&Event))
	{
	  case CSKEY_LEFT:
	  {
	    if (csKeyEventHelper::GetModifiersBits (&Event) & CSMASK_ALT)
	      break;
	    if (cursorpos == 0)
	      return true;
	    size_t newpos = cursorpos;
	    if (csKeyEventHelper::GetModifiersBits (&Event) & CSMASK_CTRL)
	      newpos = WordLeft (text, newpos);
	    else
	      newpos--;
	    SetCursorPos (newpos, (csKeyEventHelper::GetModifiersBits (&Event) & 
	      CSMASK_SHIFT) != 0);
	    return true;
	  }
	  case CSKEY_RIGHT:
	  {
	    if (csKeyEventHelper::GetModifiersBits (&Event) & CSMASK_ALT)
	      break;
	    if (cursorpos >= strlen (text))
	      return true;
	    size_t newpos = cursorpos;
	    if (csKeyEventHelper::GetModifiersBits (&Event) & CSMASK_CTRL)
	      newpos = WordRight (text, newpos);
	    else
	      newpos++;
	    SetCursorPos (newpos, (csKeyEventHelper::GetModifiersBits (&Event) & 
	      CSMASK_SHIFT) != 0);
	    return true;
	  }
	  case CSKEY_HOME:
	    if (csKeyEventHelper::GetModifiersBits (&Event) & CSMASK_ALT)
	      break;
	    SetCursorPos (0, (csKeyEventHelper::GetModifiersBits (&Event) & 
	      CSMASK_SHIFT) != 0);
	    return true;
	  case CSKEY_END:
	    if (csKeyEventHelper::GetModifiersBits (&Event) & CSMASK_ALT)
	      break;
	    SetCursorPos (strlen (text), (
	      csKeyEventHelper::GetModifiersBits (&Event) & CSMASK_SHIFT) != 0);
	    return true;
	  case CSKEY_BACKSPACE:
	    if (csKeyEventHelper::GetModifiersBits (&Event) & CSMASK_ALLSHIFTS)
	      break;
	    if (cursorpos == 0)
	      return true;
	    cursorpos--;
	    // fallback to 'Del' key
	  case CSKEY_DEL:
	  {
	    if (csKeyEventHelper::GetModifiersBits (&Event) & CSMASK_ALLSHIFTS)
	      break;
	    bool nosel = selstart == selend;
	    if (selstart == selend)
	      SetSelection (cursorpos, cursorpos + 1);
	    DeleteSelection ();
	    if (nosel)
	      SetSelection (0, 0);
	    return true;
	  } /* endif */
	  case CSKEY_INS:
	    if (csKeyEventHelper::GetModifiersBits (&Event) & CSMASK_ALLSHIFTS)
	      break;
	    cursorvis = true;
	    Invalidate ();
	    return true;
	  case '/':
	    if ((csKeyEventHelper::GetModifiersBits (&Event) & CSMASK_ALLSHIFTS) == 
	      CSMASK_CTRL)
	      SetSelection (0, strlen (text));
	    else
	      goto do_key;
	  case '\\':
	    if ((csKeyEventHelper::GetModifiersBits (&Event) & CSMASK_ALLSHIFTS) == 
	      CSMASK_CTRL)
	      SetSelection (0, 0);
	    else
	      goto do_key;
	  case CSKEY_TAB:
	    break;
	  default:
	  {
	    /*
	      on german keyboards some chars are only available when
		pressing AltGr = Ctrl+Alt, so allow that combination.
	    */
  do_key:   if ((((csKeyEventHelper::GetModifiersBits (&Event) & 
	      (CSMASK_CTRL | CSMASK_ALT)) 
	      != (CSMASK_CTRL | CSMASK_ALT))
	    && ((csKeyEventHelper::GetModifiersBits (&Event) & 
	      (CSMASK_CTRL | CSMASK_ALT)) != 0))
	    || (csKeyEventHelper::GetCookedCode (&Event) < 32))
	      return false;
	    if ((csKeyEventHelper::GetCookedCode (&Event) > 255) || 
	      !IsValidChar (csKeyEventHelper::GetCookedCode (&Event)))
	      return true;
	    DeleteSelection ();
	    CS_ALLOC_STACK_ARRAY (char, tmp, maxlen + 1);
	    strcpy (tmp, text);
	    size_t sl = strlen (tmp);
	    if (app->InsertMode)
	    {
	      if (sl >= maxlen)
		return true;
	      tmp [cursorpos] = (char)csKeyEventHelper::GetCookedCode (&Event);
	      strcpy (&tmp [cursorpos + 1], &text [cursorpos]);
	    } else
	    {
	      if ((cursorpos > sl) || (cursorpos >= maxlen))
		return true;
	      tmp [cursorpos] = (char)csKeyEventHelper::GetCookedCode (&Event);
	      if (cursorpos == sl)
		tmp [sl + 1] = 0;
	    } /* endif */
	    if (IsValidString (tmp))
	    {
	      SetTextExt (tmp);
	      SetCursorPos (cursorpos + 1, false);
	    } /* endif */
	    return true;
	  }
	} /* endswitch */
      }
      break;
  } /* endswitch */
  return csComponent::HandleEvent (Event);
}

void csInputLine::SetState (int mask, bool enable)
{
  int oldstate = state;
  csComponent::SetState (mask, enable);
  if ((oldstate ^ state) & CSS_FOCUSED)
  {
    if (selstart != selend)
      Invalidate ();
    else
      Invalidate (cursorrect);
  } /* endif */
}

void csInputLine::SetSelection (size_t iStart, size_t iEnd)
{
  if ((selstart != iStart)
   || (selend != iEnd))
  {
    size_t sl = strlen (text);
    if (iStart > sl)
      iStart = sl;
    if (iEnd > sl)
      iEnd = sl;
    selstart = iStart;
    selend = iEnd;
    cursorvis = true;
    Invalidate ();
  } /* endif */
}

int csInputLine::GetCharX (size_t iNum)
{
  if (iNum > maxlen)
    iNum = maxlen;
  if (iNum < firstchar)
    iNum = firstchar;
  char tmp = text [iNum];
  text [iNum] = 0;
  int x = GetTextSize (&text [firstchar]);
  text [iNum] = tmp;
  if (iNum == maxlen)
    x++;
  return textx + x;
}

void csInputLine::SetCursorPos (size_t NewPos, bool ExtendSel)
{
  if (NewPos == cursorpos)
  {
    if (!ExtendSel)
      SetSelection (0, 0);
  } else if (IsValidPos (NewPos))
  {
    // extend or remove selection
    if (ExtendSel)
      if (selstart != selend)
      {
        if (selstart == cursorpos)
          SetSelection (NewPos, selend);
        else if (selend == cursorpos)
          SetSelection (selstart, NewPos);
      } else
        SetSelection (cursorpos, NewPos);
    else
      SetSelection (0, 0);

    // adjust first displayed character
    if (NewPos < cursorpos)
    {
      if (NewPos < firstchar)
        firstchar = NewPos;
    }
    else
    {
      int curx = GetCharX (NewPos + 1);
      int delta = curx - (bound.Width () - textx);
      while (delta > 0)
      {
        char tmp[2];
        tmp[1] = 0;
        tmp[0] = text [firstchar];
        delta -= GetTextSize (tmp);
        firstchar++;
      } /* endwhile */
    } /* endif */

    cursorpos = NewPos;
    cursorvis = true;
    Invalidate ();
  } /* endif */
}

void csInputLine::SuggestSize (int &w, int &h)
{
  if (text)
    w = GetTextSize (text, &h);
  else
    w = h = 0;

  w = MAX (w, bound.Width ());
  h = MAX (h, bound.Height ());
}

bool csInputLine::IsValidPos (size_t NewPos)
{
  return (NewPos >= 0) && (NewPos <= strlen (text));
}

bool csInputLine::IsValidString (const char *iText)
{
  (void)iText;
  return true;
}

bool csInputLine::IsValidChar (char iChar)
{
  (void)iChar;
  return true;
}

void csInputLine::DeleteSelection ()
{
  if (selstart != selend)
  {
    size_t ss = selstart, se = selend;
    if (ss > se)
    { ss = selend; se = selstart; }
    CS_ALLOC_STACK_ARRAY (char, tmp, maxlen + 1);
    strcpy (tmp, text);
    strcpy (&tmp [ss], &tmp [se]);
    if (IsValidString (tmp))
    {
      SetTextExt (tmp);
      if (cursorpos >= se)
        SetCursorPos (cursorpos - (se - ss), false);
      else if (cursorpos > ss)
        SetCursorPos (ss, false);
      else
        SetSelection (0, 0);
    } /* endif */
  } /* endif */
}
