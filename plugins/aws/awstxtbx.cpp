/*
    Copyright (C) 2001 by Christopher Nelson

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

#include <ctype.h>

#include "awstimer.h"
#include "awstxtbx.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/fontserv.h"
#include "csutil/scfstr.h"
#include "iutil/evdefs.h"

#include <stdio.h>

static iAwsSink *textbox_sink = 0;

CS_IMPLEMENT_STATIC_VAR (GetTextBoxBlinkingCursorSlot, awsSlot,())

static awsSlot *textbox_slot = 0;

static void BlinkCursor (intptr_t, iAwsSource *source)
{
  iAwsComponent *comp = source->GetComponent ();

  // Setting blink actually forces an inversion of blink's property, and if the
  // textbox is the focus then it will be Invalidated as well.
  comp->SetProperty ("Blink", (intptr_t)0);
}

awsTextBox::awsTextBox () :
  mouse_is_over(false),
  has_focus(false),
  should_mask(0),
  bkg(0),
  frame_style(0),
  alpha_level(92),
  strStart(0),
  strCursor(0),
  blink_timer(0),
  blink(true)
{
  textbox_slot = GetTextBoxBlinkingCursorSlot ();
}

awsTextBox::~awsTextBox ()
{
  if (blink_timer != 0)
  {
    textbox_slot->Disconnect (
        blink_timer,
        awsTimer::signalTick,
        textbox_sink,
        textbox_sink->GetTriggerID ("Blink"));
    delete blink_timer;
  }
}

const char *awsTextBox::Type ()
{
  return "Text Box";
}

bool awsTextBox::Setup (iAws *_wmgr, iAwsComponentNode *settings)
{
  if (!awsComponent::Setup (_wmgr, settings)) return false;

  csRef<iKeyboardDriver> currentKbd = 
    CS_QUERY_REGISTRY (wmgr->GetObjectRegistry (), iKeyboardDriver);
  if (currentKbd == 0)
  {
    return false;
  }
  composer = currentKbd->CreateKeyComposer ();

  // Setup blink event handling
  if (textbox_sink == 0)
  {
    textbox_sink = WindowManager ()->GetSinkMgr ()->CreateSink ((intptr_t)0);
    textbox_sink->RegisterTrigger ("Blink", &BlinkCursor);
  }

  blink_timer = new awsTimer (WindowManager ()->GetObjectRegistry (), this);
  blink_timer->SetTimer (350);
  blink_timer->Start ();

  textbox_slot->Connect (
      blink_timer,
      awsTimer::signalTick,
      textbox_sink,
      textbox_sink->GetTriggerID ("Blink"));

  ////////////////
  iAwsPrefManager *pm = WindowManager ()->GetPrefMgr ();

  pm->LookupIntKey ("ButtonTextureAlpha", alpha_level); // global get
  pm->GetInt (settings, "Style", frame_style);
  pm->GetInt (settings, "Alpha", alpha_level); // local overrides, if present.
  pm->GetInt (settings, "Masked", should_mask);
  text.AttachNew (new scfString (""));
  pm->GetString (settings, "Text", text);
  disallow.AttachNew (new scfString (""));
  pm->GetString (settings, "Disallow", disallow);
  maskchar.AttachNew (new scfString (""));
  pm->GetString (settings, "MaskChar", maskchar);

  int _focusable = 0;
  pm->GetInt (settings, "Focusable", _focusable);
  focusable = _focusable;
  
  if (text) 
  {
    strCursor = (int)text->Length ();
    text = new scfString(*text);
  } 
  else
    text = new scfString ();

  switch (frame_style)
  {
    case fsNormal:
      bkg = pm->GetTexture ("Texture");
      break;

    case fsBitmap:
      {
        iString *tn1 = 0;

        pm->GetString (settings, "Bitmap", tn1);

        if (tn1) bkg = pm->GetTexture (tn1->GetData (), tn1->GetData ());
      }
      break;
  }

  return true;
}

bool awsTextBox::GetProperty (const char *name, intptr_t *parm)
{
  if (awsComponent::GetProperty (name, parm)) return true;

  if (strcmp ("Text", name) == 0)
  {
    const char *st = 0;

    if (text) st = text->GetData ();

    iString *s = new scfString (st);
    *parm = (intptr_t)s;
    return true;
  }
  else if (strcmp ("Disallow", name) == 0)
  {
    const char *st = 0;

    if (disallow) st = disallow->GetData ();

    iString *s = new scfString (st);
    *parm = (intptr_t)s;
    return true;
  }

  return false;
}

bool awsTextBox::SetProperty (const char *name, intptr_t parm)
{
  if (awsComponent::SetProperty (name, parm)) return true;

  if (strcmp ("Blink", name) == 0)
  {
    blink = !blink;
    if (has_focus) Invalidate ();
    return true;
  }
  else if (strcmp ("Text", name) == 0)
  {
    iString *s = (iString *) (parm);

    if (s)
    {
      text = s;
      Invalidate ();
      strStart = 0;
      strCursor = 0;
    }

    return true;
  }
  else if (strcmp ("Disallow", name) == 0)
  {
    iString *s = (iString *) (parm);

    if (s)
    {
      disallow = s;
    }

    return true;
  }

  return false;
}

void awsTextBox::OnDraw (csRect /*clip*/)
{
  iGraphics2D *g2d = WindowManager ()->G2D ();
  iGraphics3D *g3d = WindowManager ()->G3D ();

  int hi = WindowManager ()->GetPrefMgr ()->GetColor (AC_HIGHLIGHT);
  int hi2 = WindowManager ()->GetPrefMgr ()->GetColor (AC_HIGHLIGHT2);
  int lo = WindowManager ()->GetPrefMgr ()->GetColor (AC_SHADOW);
  int lo2 = WindowManager ()->GetPrefMgr ()->GetColor (AC_SHADOW2);
  int dfill = WindowManager ()->GetPrefMgr ()->GetColor (AC_DARKFILL);
  int black = WindowManager ()->GetPrefMgr ()->GetColor (AC_BLACK);

  switch (frame_style)
  {
    case fsNormal:
      g2d->DrawLine (
          Frame ().xmin + 0,
          Frame ().ymin + 0,
          Frame ().xmax - 1,
          Frame ().ymin + 0,
          lo2);
      g2d->DrawLine (
          Frame ().xmin + 0,
          Frame ().ymin + 0,
          Frame ().xmin + 0,
          Frame ().ymax - 1,
          lo2);
      g2d->DrawLine (
          Frame ().xmin + 1,
          Frame ().ymin + 1,
          Frame ().xmax - 0,
          Frame ().ymin + 1,
          lo);
      g2d->DrawLine (
          Frame ().xmin + 1,
          Frame ().ymin + 1,
          Frame ().xmin + 1,
          Frame ().ymax - 0,
          lo);
      g2d->DrawLine (
          Frame ().xmin + 1,
          Frame ().ymax - 0,
          Frame ().xmax - 0,
          Frame ().ymax - 0,
          hi);
      g2d->DrawLine (
          Frame ().xmax - 0,
          Frame ().ymin + 1,
          Frame ().xmax - 0,
          Frame ().ymax - 0,
          hi);

      g2d->DrawLine (
          Frame ().xmin + 2,
          Frame ().ymin + 2,
          Frame ().xmax - 1,
          Frame ().ymin + 2,
          black);
      g2d->DrawLine (
          Frame ().xmin + 2,
          Frame ().ymin + 2,
          Frame ().xmin + 2,
          Frame ().ymax - 1,
          black);
      g2d->DrawLine (
          Frame ().xmin + 2,
          Frame ().ymax - 1,
          Frame ().xmax - 1,
          Frame ().ymax - 1,
          hi2);
      g2d->DrawLine (
          Frame ().xmax - 1,
          Frame ().ymin + 2,
          Frame ().xmax - 1,
          Frame ().ymax - 1,
          hi2);

      g2d->DrawBox (
          Frame ().xmin + 3,
          Frame ().ymin + 3,
          Frame ().Width () - 3,
          Frame ().Height () - 3,
          dfill);

      if (bkg)
      {
        g3d->DrawPixmap (
            bkg,
            Frame ().xmin,
            Frame ().ymin,
            Frame ().Width () + 1,
            Frame ().Height () + 1,
            0,
            0,
            Frame ().Width () + 1,
            Frame ().Height () + 1,
            alpha_level);
      }
      break;

    case fsBitmap:
      if (bkg)
      {
        g3d->DrawPixmap (
            bkg,
            Frame ().xmin,
            Frame ().ymin,
            Frame ().Width (),
            Frame ().Height (),
            0,
            0,
            Frame ().Width (),
            Frame ().Height (),
            alpha_level);
      }
      break;
  }

  // Draw the caption, if there is one and the style permits it.
  if (text && text->Length ())
  {
    int tw, th, tx, ty, mcc;
    csRef<iString> saved;

    /*
      When masking is used, the cursor/start offsets are 
      different
     */
    int usedStrStart = strStart;
    uint usedStrCursor = strCursor;

    if (should_mask && maskchar)
    {
      saved = text->Clone ();

      usedStrStart = usedStrCursor = 0;

      unsigned int i = 0;
      utf8_char* sptr = (utf8_char*)text->GetData ();
      size_t sl = text->Length ();
      size_t sch = 0;
      while (*sptr != 0)
      {
	int chSize = csUnicodeTransform::UTF8Skip (sptr, sl);
        text->SetAt (i, maskchar->GetAt (0));
	sptr += chSize;
	sl -= chSize;

	sch += chSize;
	i++;
	if (sch <= (size_t)strStart) usedStrStart = i;
	if (sch <= strCursor) usedStrCursor = i;
      }
      text->Truncate (i);
    }

    // Get the maximum number of characters we can use
    mcc = WindowManager ()->GetPrefMgr ()->GetDefaultFont ()->GetLength (
        text->GetData () + strStart,
        Frame ().Width () - 10);

    if (mcc)
    {

      // Check to see if we're getting weird.
      // this was changed to avoid 
      // jittering in the start value
      if ((int)usedStrCursor > usedStrStart + mcc)
	  usedStrStart = usedStrCursor - mcc;
      if (usedStrStart < 0) usedStrStart = 0;

      // Make the text the right length
      csString tmp (text->GetData () + usedStrStart);
      tmp.Truncate (mcc);

      // Get the size of the text
      WindowManager ()->GetPrefMgr ()->GetDefaultFont ()->
          GetDimensions (tmp.GetData (), tw, th);

      // Calculate the center
      tx = 4;
      ty = (Frame ().Height () >> 1) - (th >> 1);

      // Draw the text
      g2d->Write (
	WindowManager ()->GetPrefMgr ()->GetDefaultFont (),
	Frame ().xmin + tx,
	Frame ().ymin + ty,
	WindowManager ()->GetPrefMgr ()->GetColor (AC_TEXTFORE),
	-1,
	tmp.GetData ());

      if (should_mask && maskchar && saved)
      {
	text->Clear ();
	text->Append (saved);
	saved = 0;
      }

    if (has_focus && blink)
    {
      int co, cx, tty;
      co = strCursor - strStart;
      tmp.Truncate (co);
      // figure out where to put the cursor
      WindowManager ()->GetPrefMgr ()->GetDefaultFont ()->
         GetDimensions (tmp, cx, tty);
      g2d->DrawLine (
          Frame ().xmin + tx + cx + 1,
          Frame ().ymin + ty,
          Frame ().xmin + tx + cx + 1,
          Frame ().ymin + ty + th,
          WindowManager ()->GetPrefMgr ()->GetColor (AC_TEXTFORE));
    }
    }
  }
  else if (has_focus && blink)
  {
    g2d->DrawLine (
        Frame ().xmin + 5,
        Frame ().ymin + 5,
        Frame ().xmin + 5,
        Frame ().ymax - 5,
        WindowManager ()->GetPrefMgr ()->GetColor (AC_TEXTFORE));
  }
}

bool awsTextBox::OnMouseDown (int, int x, int y)
{
  // make sure text is valid
  if (text && text->Length() > 0)
  {
    // determine how many chars in the mouse was clicked
    if (should_mask && maskchar)
    {
      char mask[2];
      mask[0] = maskchar->GetAt (0);
      mask[1] = 0;
      int mw, mh;
      WindowManager ()->GetPrefMgr ()->GetDefaultFont ()->
	GetDimensions (mask, mw, mh);
      strCursor = strStart + ((x - 4 - Frame ().xmin) / mw);
      if(strCursor > text->Length()) // maximum is string length.
	strCursor = (int)text->Length();
    }
    else
    {
      int tp,cp;
      scfString tmp(text->GetData () + strStart);
      tp = x - 4 - Frame ().xmin;
      cp = WindowManager ()->GetPrefMgr ()->GetDefaultFont ()->
	GetLength (tmp, tp);
      strCursor = cp + strStart;
    }
  }
  // This is needed to get keyboard focus for the mouse!
  return true;
}

bool awsTextBox::OnMouseExit ()
{
  mouse_is_over = false;
  Invalidate ();
  return true;
}

bool awsTextBox::OnMouseEnter ()
{
  mouse_is_over = true;
  Invalidate ();
  return true;
}

void awsTextBox::EnsureCursorToStartDistance (int dist)
{
  int charsToCur = 0;
  size_t bytesToCur = 0;
  while (strStart + bytesToCur < strCursor)
  {
    bytesToCur += csUnicodeTransform::UTF8Skip (
      (utf8_char*)text->GetData () + strStart + bytesToCur,
      strCursor - bytesToCur - strStart);
    charsToCur++;
  }
  if (charsToCur < dist)
  {
    strStart = strCursor;
    charsToCur = 0;
    while ((charsToCur < dist) && (strStart > 0))
    {
      strStart -= csUnicodeTransform::UTF8Rewind (
	(utf8_char*)text->GetData () + strStart,
	strStart);
      charsToCur++;
    }
  }
}

bool awsTextBox::OnKeyboard (const csKeyEventData& eventData)
{
  switch (eventData.codeCooked)
  {
    case CSKEY_ENTER:
      Broadcast (signalEnterPressed);
      break;

    case CSKEY_TAB:
      Broadcast (signalTabPressed);
      break;

    case CSKEY_BACKSPACE:
      if (strCursor > 0) 
      {
	int chSize = csUnicodeTransform::UTF8Rewind (
	  (utf8_char*)text->GetData () + strCursor, 
	  strCursor);

	strCursor -= chSize;
	EnsureCursorToStartDistance (5);
        if (strStart < 0) strStart = 0;
        if (text && (text->Length () > 1))
        {
          csString tmp (text->GetData ());
         tmp.DeleteAt (strCursor, chSize);
		text->Clear ();
		text->Append (tmp);
        }
        else
          text->Clear ();

/*        cursor--;
        if (cursor - start < 5) start = cursor - 5;
        if (start < 0) start = 0;
        if (text && (text->Length () > 1))
          if (cursor == text->Length ())
            text->Truncate (text->Length () - 1);
          else
          {
            scfString tmp(text->GetData());
            tmp.Truncate (cursor);
            tmp.Append (text->GetData() + cursor + 1);
            text->Replace(&tmp);
          }
        else
          text->Clear ();*/
      }

      break;

    case CSKEY_DEL:
      if (strCursor < text->Length ())
      {
        int chSize = csUnicodeTransform::UTF8Skip (
	  (utf8_char*)text->GetData () + strCursor, 
	  text->Length () - strCursor);

	EnsureCursorToStartDistance (5);
        if (strStart < 0) strStart = 0;
        if (text && (text->Length () > 1))
        {
          csString tmp (text->GetData ());
          tmp.DeleteAt (strCursor, chSize);
	  text->Clear ();
	  text->Append (tmp);
        }
        else
          text->Clear ();
      }
/*      if (cursor > 0 && cursor == text->Length ()) cursor--;
      if (cursor - start < 5) start = cursor - 5;
      if (start < 0) start = 0;
      if (text && (text->Length () > 1))
        if (cursor == text->Length ())
          text->Truncate (text->Length () - 1);
        else
        {
          scfString tmp(text->GetData());
          tmp.Truncate (cursor);
          tmp.Append (text->GetData()+cursor+1);
          text->Replace(&tmp);
        }
      else
        text->Clear ();*/

      break;

    case CSKEY_LEFT:
      if (strCursor > 0)
      {
	strCursor -= csUnicodeTransform::UTF8Rewind (
	  (utf8_char*)text->GetData () + strCursor,
	  strCursor);
      }
      EnsureCursorToStartDistance (5);
      /*if (cursor > 0) cursor--;
      if (cursor - start < 5) start = cursor - 5;
      if (start < 0) start = 0;*/

      break;

    case CSKEY_RIGHT:
      if (strCursor < text->Length ())
      {
	strCursor += csUnicodeTransform::UTF8Skip (
	  (utf8_char*)text->GetData () + strCursor,
	  text->Length () - strCursor);
      }
      /*if (cursor < text->Length ()) {
        cursor++;
      }*/

      break;

    case CSKEY_HOME:
      strCursor=0;
      strStart=0;

      break;

    case CSKEY_END:
      strCursor=(int)text->Length();

      break;

    default:
      {
	//if (!isprint (Char))
	//  break;
	if (CSKEY_IS_SPECIAL (eventData.codeCooked))
	  break;

	utf32_char composedCh[2];
	int composedCount;
	if (composer->HandleKey (eventData, composedCh, 
	  sizeof (composedCh) / sizeof (utf32_char), 
	  &composedCount) != csComposeNoChar)
	{
	  bool changed = false;
	  for (int n = 0; n < composedCount; n++)
	  {
	    utf8_char ch[CS_UC_MAX_UTF8_ENCODED + 1];
	    size_t chSize = csUnicodeTransform::EncodeUTF8 (composedCh[n],
	      ch, sizeof (ch) / sizeof (utf8_char));
	    ch[chSize] = 0;

	    if (disallow && (strstr (disallow->GetData (), (char*)ch) != 0))
	      continue;
	    //if (disallow && (strchr (disallow->GetData (), Char) != 0))
	    //  break;
    	
	    /*char str[2];
	    str[0] = (char)Char;
	    str[1] = 0;*/

	    if (strCursor == text->Length ()) 
	    {
	      text->Append ((char*)ch);
	    }
	    else
	    {
	      scfString tmp ((char*)ch);
	      text->Insert (strCursor, &tmp);
	    }
	    strCursor += (uint)chSize;
	    changed = true;
	  }
	  if (changed) Broadcast (signalChanged);
	}
      }
  }         // end switch
  Invalidate ();
  return true;
}

bool awsTextBox::OnLostFocus ()
{
  has_focus = false;
  Broadcast (signalLostFocus);
  Invalidate ();
  return true;
}

bool awsTextBox::OnGainFocus ()
{
  has_focus = true;
  Broadcast (signalFocused);
  Invalidate ();
  return true;
}

void awsTextBox::OnSetFocus ()
{
	Broadcast (signalFocused);
}

/************************************* Command Button Factory ****************/
awsTextBoxFactory::awsTextBoxFactory (iAws *wmgr) :
  awsComponentFactory(wmgr)
{
  Register ("Text Box");
  RegisterConstant ("tbfsNormal", awsTextBox::fsNormal);
  RegisterConstant ("tbfsBitmap", awsTextBox::fsBitmap);

  RegisterConstant ("signalTextBoxChanged", awsTextBox::signalChanged);
  RegisterConstant ("signalTextBoxLostFocus", awsTextBox::signalLostFocus);
  RegisterConstant ("signalEnterKeyPressed", awsTextBox::signalEnterPressed);
  RegisterConstant ("signalTabKeyPressed", awsTextBox::signalTabPressed);
  RegisterConstant ("signalTextBoxFocused", awsTextBox::signalFocused);
}

awsTextBoxFactory::~awsTextBoxFactory ()
{
  // empty
}

iAwsComponent *awsTextBoxFactory::Create ()
{
  return new awsTextBox;
}
