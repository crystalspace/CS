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
#include "csws/csspinbx.h"
#include "csws/cstimer.h"
#include "csws/csapp.h"
#include "csws/cswsutil.h"
#include "csws/cswsaux.h"
#include "csutil/csinput.h"
#include "csutil/event.h"

// Spin box texture name
#define SPINBOX_TEXTURE_NAME		"csws::SpinBox"
// Automatical spinning period in milliseconds
#define AUTO_SPIN_INTERVAL		100
// The pause between first click and autorepeat
#define AUTO_SPIN_STARTINTERVAL		500

static csPixmap *sprspin [3] = { 0, 0, 0 };
static int spinboxref = 0;

inline int sqr(int x)
{
  return x * x;
}

csSpinBox::csSpinBox (csComponent *iParent, csInputLineFrameStyle iFrameStyle)
  : csInputLine (iParent, CSIL_DEFAULTLENGTH, iFrameStyle), Values (8, 8)
{
  SpinState = 0;
  Value = NumLimits.MinValue = NumLimits.MaxValue = 0;
  NumLimits.ValueFormat = 0;
  SpinTimer = new csTimer (this, AUTO_SPIN_INTERVAL);
  SpinTimer->Stop ();

  spinboxref++;
  if (app)
  {
    // If  images are not loaded, load them
	int i;
    for (i = 0; i < 3; i++)
      if (!sprspin [i])
        sprspin [i] = new csSimplePixmap (app->GetTexture (
          SPINBOX_TEXTURE_NAME), i * 16, 0, 16, 16);
  } /* endif */
}

csSpinBox::~csSpinBox ()
{
  if (NumLimits.ValueFormat)
    delete [] NumLimits.ValueFormat;
  if (--spinboxref == 0)
  {
	int i;
    for (i = 0; i < 3; i++)
    {
      delete sprspin [i];
      sprspin [i] = 0;
    } /* endfor */
  }
}

void csSpinBox::Draw ()
{
  SpinBoxSize = bound.Height ();
  // this is not a good practice :-)
  bound.xmax -= SpinBoxSize;
  csInputLine::Draw ();
  bound.xmax += SpinBoxSize;
  SetClipRect ();
  int State = SpinState;
  if ((app->MouseOwner == this)
   && !SpinTimer->Running ())
    State = 0;
  Pixmap (sprspin [State], bound.Width () - SpinBoxSize, 0,
    SpinBoxSize, SpinBoxSize);
}

bool csSpinBox::HandleEvent (iEvent &Event)
{
  switch (Event.Type)
  {
    case csevCommand:
      switch (Event.Command.Code)
      {
        case cscmdTimerPulse:
          if (Event.Command.Info != (intptr_t)SpinTimer)
            break;
          Spin ();
          return true;
        case cscmdSpinBoxQueryValue:
          Event.Command.Info = (intptr_t)Value;
          return true;
        case cscmdSpinBoxSetValue:
          SetValue ((int)Event.Command.Info);
          return true;
        case cscmdSpinBoxInsertItem:
        {
          csSpinBoxItem *i = (csSpinBoxItem *)Event.Command.Info;
          Event.Command.Info = (intptr_t)InsertItem (i->Value, i->Position);
          return true;
        }
        case cscmdSpinBoxSetLimits:
        {
          csSpinBoxLimits *l = (csSpinBoxLimits *)Event.Command.Info;
          SetLimits (l->MinValue, l->MaxValue, l->ValueFormat);
          return true;
        }
      } /* endswitch */
      break;
    case csevMouseDown:
      if (GetState (CSS_DISABLED))
        return true;
      Select ();
      if (!app->KeyboardOwner
       && (Event.Mouse.Button == 1)
       && (Event.Mouse.x >= bound.Width () - SpinBoxSize))
      {
        if (!app->MouseOwner)
          app->CaptureMouse (this);
        if (Event.Mouse.y < bound.Width () - Event.Mouse.x)
          SpinState = 1;
        else
          SpinState = 2;
        AutoRepeats = -1;
        Spin ();
        SpinTimer->Restart ();
        SpinTimer->Pause (AUTO_SPIN_STARTINTERVAL);
      } /* endif */
      return true;
    case csevMouseMove:
      if (app->MouseOwner == this)
      {
        bool oldSpinState = SpinTimer->Running ();
        bool inside = false;
        switch (SpinState)
        {
          case 1:
            inside = (Event.Mouse.x >= bound.Width () - SpinBoxSize)
                  && (Event.Mouse.x <= bound.Width ())
                  && (Event.Mouse.y >= 0)
                  && (Event.Mouse.y < bound.Width () - Event.Mouse.x);
            break;
          case 2:
            inside = (Event.Mouse.x >= bound.Width () - SpinBoxSize)
                  && (Event.Mouse.x <= bound.Width ())
                  && (Event.Mouse.y >= bound.Width () - Event.Mouse.x)
                  && (Event.Mouse.y <= bound.Height ());
            break;
        } /* endswitch */
        if (inside && !SpinTimer->Running ())
        {
          AutoRepeats = -1;
          Spin ();
          SpinTimer->Restart ();
          SpinTimer->Pause (AUTO_SPIN_STARTINTERVAL);
        }
        else if (!inside && SpinTimer->Running ())
          SpinTimer->Stop ();
        if (oldSpinState != SpinTimer->Running ())
          Invalidate (bound.Width () - SpinBoxSize, 0, bound.Width (), bound.Height ());
      } /* endif */
      break;
    case csevMouseUp:
      if (app->MouseOwner == this)
      {
        SpinTimer->Stop ();
        SpinState = 0;
        Invalidate (bound.Width () - SpinBoxSize, 0, bound.Width (), bound.Height ());
      } /* endif */
      break;
    case csevKeyboard:
      if (csKeyEventHelper::GetEventType (&Event) == csKeyEventTypeDown)
      {
	switch (csKeyEventHelper::GetCookedCode (&Event))
	{
	  case CSKEY_UP:
	    if (!app->MouseOwner)
	    {
	      if (!app->KeyboardOwner)
	      {
		app->CaptureKeyboard (this);
		SpinState = 1;
		AutoRepeats = -1;
	      } /* endif */
	      Spin ();
	    } /* endif */
	    break;
	  case CSKEY_DOWN:
	    if (!app->MouseOwner)
	    {
	      if (!app->KeyboardOwner)
	      {
		app->CaptureKeyboard (this);
		SpinState = 2;
		AutoRepeats = 0;
	      } /* endif */
	      Spin ();
	    } /* endif */
	    break;
	} /* endswitch */
	if (csKeyEventHelper::GetCookedCode (&Event))
	  return true;
      }
      else
      {
	switch (csKeyEventHelper::GetCookedCode (&Event))
	{
	  case CSKEY_UP:
	  case CSKEY_DOWN:
	    if (app->KeyboardOwner == this)
	    {
	      app->CaptureKeyboard (0);
	      SpinState = 0;
	      Invalidate ();
	    } /* endif */
	    break;
	} /* endswitch */
	if (csKeyEventHelper::GetCookedCode (&Event))
	  return true;
      }
      break;
  } /* endswitch */
  return csInputLine::HandleEvent (Event);
}

void csSpinBox::Spin (int iDelta)
{
  int NewValue = Value + iDelta;
  if (Values.Length ())
  {
    if (NewValue < 0)
      NewValue += (int)Values.Length ();
    if ((size_t)NewValue >= Values.Length ())
      NewValue -= (int)Values.Length ();
  }
  else
  {
    if (NewValue < NumLimits.MinValue)
      NewValue += (NumLimits.MaxValue - NumLimits.MinValue + 1);
    if (NewValue > NumLimits.MaxValue)
      NewValue -= (NumLimits.MaxValue - NumLimits.MinValue + 1);
  } /* endif */
  SetValue (NewValue);
}

void csSpinBox::Spin ()
{
  AutoRepeats++;
  int Delta;
  size_t NumValues = Values.Length ();
  if (!NumValues)
    NumValues = (NumLimits.MaxValue - NumLimits.MinValue) + 1;
  if ((NumValues >= 40) && (AutoRepeats >= 40))
    Delta = 10;
  else if ((NumValues >= 20) && (AutoRepeats >= 20))
    Delta = 5;
  else
    Delta = 1;
  switch (SpinState)
  {
    case 0:
      break;
    case 1:
      Spin (+Delta);
      break;
    case 2:
      Spin (-Delta);
      break;
  } /* endswitch */
}

void csSpinBox::SetValue (int iValue)
{
  if (Values.Length ())
  {
    if (iValue >= (int)Values.Length ())
      iValue = (int)Values.Length () - 1;
    if (iValue < 0)
      iValue = 0;
    SetText (Values [iValue]);
  }
  else
  {
    if (iValue > NumLimits.MaxValue)
      iValue = NumLimits.MaxValue;
    if (iValue < NumLimits.MinValue)
      iValue = NumLimits.MinValue;
    char str [32];
    sprintf (str, NumLimits.ValueFormat, iValue);
    SetText (str);
  } /* endif */
  SetCursorPos (strlen (text), 0);
  Value = iValue;
  if (parent)
    parent->SendCommand (cscmdSpinBoxValueChanged, (intptr_t)this);
  Invalidate ();
}

void csSpinBox::SetLimits (int iMin, int iMax, char *iFormat)
{
  Values.DeleteAll ();
  NumLimits.MinValue = iMin;
  NumLimits.MaxValue = iMax;
  if (NumLimits.ValueFormat)
    delete [] NumLimits.ValueFormat;
  NumLimits.ValueFormat = csStrNew (iFormat);
  SetValue (iMin);
}

int csSpinBox::InsertItem (char *iValue, int iPosition)
{
  if (iPosition == CSSB_ITEM_AFTERALL)
    iPosition = (int)Values.Length ();
  Values.Insert (iPosition, iValue);
  return iPosition;
}

void csSpinBox::SetText (const char *iText)
{
  csInputLine::SetText (iText);
  SetSelection (0, 0);
}
