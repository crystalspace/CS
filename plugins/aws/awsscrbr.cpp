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
#include "awsscrbr.h"
#include "aws3dfrm.h"
#include "awskcfct.h"
#include "awsslot.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/fontserv.h"
#include "csutil/scfstr.h"
#include "csutil/csevent.h"
#include "iutil/evdefs.h"

#include <stdio.h>

awsScrollBar::awsScrollBar () :
  is_down(false),
  mouse_is_over(false),
  was_down(false),
  orientation(0),
  decVal(0),
  incVal(0),
  knob(0),
  timer(0),
  sink(0),
  dec_slot(0),
  inc_slot(0),
  knob_slot(0),
  tick_slot(0),
  value(0),
  max(1),
  min(0),
  amntvis(0),
  value_delta(0.1f),
  value_page_delta(0.25f)
{
  //SetFlag (AWSF_CMP_ALWAYSERASE);
  captured = false;
}

awsScrollBar::~awsScrollBar ()
{
  if (dec_slot)
    dec_slot->Disconnect (
        decVal,
        awsCmdButton::signalClicked,
        sink,
        sink->GetTriggerID ("DecValue"));
  if (inc_slot)
    inc_slot->Disconnect (
        incVal,
        awsCmdButton::signalClicked,
        sink,
        sink->GetTriggerID ("IncValue"));
  if (knob_slot)
    knob_slot->Disconnect (
        knob,
        awsCmdButton::signalClicked,
        sink,
        sink->GetTriggerID ("KnobTick"));
  if (tick_slot)
    tick_slot->Disconnect (
        timer,
        awsTimer::signalTick,
        sink,
        sink->GetTriggerID ("TickTock"));

  if (incVal) incVal->DecRef ();
  if (decVal) decVal->DecRef ();
  if (knob) knob->DecRef ();
  if (sink) sink->DecRef ();
  if (inc_slot) inc_slot->DecRef ();
  if (dec_slot) dec_slot->DecRef ();
  if (knob_slot) knob_slot->DecRef ();
  if (tick_slot) tick_slot->DecRef ();
  if (timer) timer->DecRef ();

  if (captured) WindowManager ()->ReleaseMouse ();
}

const char *awsScrollBar::Type ()
{
  return "Scroll Bar";
}

bool awsScrollBar::Setup (iAws *_wmgr, iAwsComponentNode *settings)
{
  if (!awsPanel::Setup (_wmgr, settings)) return false;

  iAwsPrefManager *pm = WindowManager ()->GetPrefMgr ();

  pm->GetInt (settings, "Orientation", orientation);

  /*  Numbers are parsed into either floats or ints depending on
   *   the presence of a decimal point.
   *  If we're expecting a float and we don't find one, we should
   *   check for an int, since the numeric value may have just been
   *   specified as a whole number - in which case it would be parsed
   *   as an int.
   */

  int ival;

  min=0.0f;
  if (!pm->GetFloat (settings, "Min", min))
  {
	  if (pm->GetInt(settings,"Min",ival))
		  min=(float)ival;
  }

  value=0.0f;
  if (!pm->GetFloat (settings, "Value", value))
  {
	  if (pm->GetInt(settings,"Value",ival))
		  value=(float)ival;
  }

  max=1.0f;
  if (!pm->GetFloat (settings, "Max", max))
  {
	  if (pm->GetInt(settings,"Max",ival))
		  max=(float)ival;
  }

  amntvis=1.0f;
  if (!pm->GetFloat (settings, "PageSize", amntvis))
  {
	  if (pm->GetInt(settings,"PageSize",ival))
		  amntvis=(float)ival;
  }

  
  value_delta=1.0f;
  if (!pm->GetFloat (settings, "Change", value_delta))
  {
	  if (pm->GetInt(settings,"Change",ival))
		  value_delta=(float)ival;
  }

  value_page_delta=1.0f;
  if (!pm->GetFloat (settings, "BigChange", value_page_delta))
  {
	  if (pm->GetInt(settings,"BigChange",ival))
		  value_page_delta=(float)ival;
  }

  iAws* const w = WindowManager();

  // Setup embedded buttons
  incVal = new awsSliderButton;
  decVal = new awsSliderButton;
  knob = new awsSliderButton;
  timer = new awsTimer (w->GetObjectRegistry (), this);

  awsKeyFactory incinfo(w), decinfo(w), knobinfo(w);

  decinfo.Initialize ("decVal", "Slider Button");
  incinfo.Initialize ("incVal", "Slider Button");
  knobinfo.Initialize ("knob", "Slider Button");

  decinfo.AddIntKey ("Style", awsCmdButton::fsNormal);
  incinfo.AddIntKey ("Style", awsCmdButton::fsNormal);
  knobinfo.AddIntKey ("Style", awsCmdButton::fsNormal);

  switch (orientation)
  {
    case sboVertical:
      {
        incimg = pm->GetTexture ("ScrollBarDn");
        decimg = pm->GetTexture ("ScrollBarUp");

        // Abort if the images are not found
        if (!incimg || !decimg) return false;

        int img_w, img_h;

        incimg->GetOriginalDimensions (img_w, img_h);

        decinfo.AddRectKey ("Frame", csRect (0, 0, Frame ().Width (), img_h));

        incinfo.AddRectKey ("Frame", csRect (
              0,
              Frame ().Height () - img_h,
              Frame ().Width (),
              Frame ().Height ()));

        knobinfo.AddRectKey ("Frame",
	      csRect (0, img_h + 1, Frame ().Width (), 2 * img_h + 1));
      }
      break;

      default:
      {
        incimg = pm->GetTexture ("ScrollBarRt");
        decimg = pm->GetTexture ("ScrollBarLt");

        // Abort if the images are not found
        if (!incimg || !decimg) return false;

        int img_w, img_h;

        incimg->GetOriginalDimensions (img_w, img_h);

        decinfo.AddRectKey ("Frame", csRect (0, 0, img_w, Frame().Height()));

        incinfo.AddRectKey ("Frame",
              csRect (Frame ().Width () - img_w, 0,
                      Frame ().Width (), Frame().Height()));

        knobinfo.AddRectKey ("Frame",
			     csRect (img_w + 1, 0, 2 * img_w + 1, Frame().Height()));
      }
      break;
  } // end switch framestyle

  //  DEBUG_BREAK;

  decVal->SetParent (this);
  incVal->SetParent (this);
  knob->SetParent (this);

  decVal->Setup (_wmgr, decinfo.GetThisNode ());
  incVal->Setup (_wmgr, incinfo.GetThisNode ());
  knob->Setup (_wmgr, knobinfo.GetThisNode ());

  decVal->SetProperty ("Image", decimg);
  incVal->SetProperty ("Image", incimg);


  csTicks t = (csTicks) 10;
  incVal->SetProperty ("TicksPerSecond", (void *) &t);
  decVal->SetProperty ("TicksPerSecond", (void *) &t);
  knob->SetProperty ("TicksPerSecond", (void *) &t);

  awsSink* _sink = new awsSink (w);
  _sink->SetParm (this);
  sink = _sink;

  sink->RegisterTrigger ("DecValue", &DecClicked);
  sink->RegisterTrigger ("IncValue", &IncClicked);
  sink->RegisterTrigger ("TickTock", &TickTock);
  sink->RegisterTrigger ("KnobTick", &KnobTick);

  dec_slot = new awsSlot ();
  inc_slot = new awsSlot ();
  tick_slot = new awsSlot ();
  knob_slot = new awsSlot ();

  dec_slot->Connect (
      decVal,
      awsCmdButton::signalClicked,
      sink,
      sink->GetTriggerID ("DecValue"));
  inc_slot->Connect (
      incVal,
      awsCmdButton::signalClicked,
      sink,
      sink->GetTriggerID ("IncValue"));
  knob_slot->Connect (
      knob,
      awsCmdButton::signalClicked,
      sink,
      sink->GetTriggerID ("KnobTick"));

  tick_slot->Connect (
      timer,
      awsTimer::signalTick,
      sink,
      sink->GetTriggerID ("TickTock"));

  return true;
}

bool awsScrollBar::GetProperty (const char *name, void **parm)
{
  if (awsComponent::GetProperty (name, parm)) return true;

  if (strcmp ("Value", name) == 0)
  {
    *parm = (void *) &value;
    return true;
  }
  else if (strcmp ("Change", name) == 0)
  {
    *parm = (void*) &value_delta;
    return true;
  }
  else if (strcmp ("BigChange", name) == 0)
  {
    *parm = (void*) &value_page_delta;
    return true;
  }
  else if (strcmp ("Max", name) == 0)
  {
    *parm = (void*) &max;
    return true;
  }
  else if (strcmp ("Min", name) == 0)
  {
    *parm = (void*) &min;
    return true;
  }
  else if (strcmp ("PageSize", name) == 0)
  {
    *parm = (void*) &amntvis;
    return true;
  }
 
  return false;
}

bool awsScrollBar::SetProperty (const char *name, void *parm)
{
  if (awsComponent::SetProperty (name, parm)) 
    return true;

  if (strcmp ("Change", name) == 0)
  {
    value_delta = *(float *)parm;
    
    Invalidate ();
    return true;
  }
  else if (strcmp ("BigChange", name) == 0)
  {
    value_page_delta = *(float *)parm;

    Invalidate ();
    return true;
  }
  else if (strcmp ("Min", name) == 0)
  {
    min = *(float *)parm;

    // Fix value in case it's out of range
    value = (value < min) ? min : value;

    Invalidate ();
    return true;
  }
  else if (strcmp ("Max", name) == 0)
  {
    max = *(float *)parm;

    // Fix the page size
    if (amntvis > max) 
      amntvis = max + 1.0f;
    int maxval = (int)(max - amntvis + 1.0f);

    // Fix value in case it's out of range
    value = (value < min ? min : (value > maxval ? maxval : value));

    Invalidate ();
    return true;
  }
  else if (strcmp ("PageSize", name) == 0)
  {
    amntvis = *(float *)parm;

    // Fix the page size
    if (amntvis > max) 
      amntvis = max + 1.0f;
    int maxval = (int)(max - amntvis + 1.0f);

    // Fix value in case it's out of range
    value = (value < min ? min : (value > maxval ? maxval : value));

    Invalidate ();
    return true;
  }
  else if (strcmp ("Value", name) == 0)
  {
    value = *(float *)parm;

    // Fix value in case it's out of range
    int maxval = (int)(max - amntvis + 1.0f);

    value = (value < min ? min : (value > maxval ? maxval : value));

    Invalidate ();
    return true;
  }

  return false;
}

/*  This function handles at least mouse scrolling on the bar by "grabbing" the scroll "knob".
 */
void awsScrollBar::KnobTick (void *sk, iAwsSource *)
{
  awsScrollBar *sb = (awsScrollBar *)sk;
  /* amntvis relates to "proportional scrollbars" which are probably broken
   *  because most everything else was prior to some fixups and I don't
   *  know how "proportional scrollbars" are supposed to work so I haven't
   *  attempted to fix them.
   */

  if (sb->orientation == sboVertical)
  {
    // Code for Vertical scrollbars.  We use height for calculations here.
    int height = 10; 

    // We get the rectangle that surrounds the entire scrollbar, including the end buttons (up/down arrows)
    csRect f (sb->Frame ());

    // Then we subtract the height of the up and down arrows to get the height of the actual "bar" area we can scroll through
    f.ymin += sb->decVal->Frame ().Height () + 1;
    f.ymax -= sb->incVal->Frame ().Height () + 1;

    /* This retrieves the knob height.  If amntvis isn't 0 you're using a "proportional" scrollbar
     *  which apparently changes the size of the knob based on some factors.
     */
    if (sb->amntvis == 0)
      sb->WindowManager ()->GetPrefMgr ()->LookupIntKey (
          "ScrollBarHeight",
          height);
    else
      height = (int)((sb->amntvis * f.Height ()) / (sb->max - sb->min) );

    // bh becomes the actual "usable" height in pixels - after we subtract the height of the knob from the scroll area
    int bh = f.Height () - height;
    
    // If there's no area to scroll through or the range (max - min) is less than 0, then there is only one possible position to be at.
    if (((sb->max - sb->min) <= 0) || (bh == 0)) 
      sb->value = 0;
    else
      /*  sb->knob->last_y is actually the last y position of the mouse while dragging the knob
       *  We take that position, subtract the y of the bottom edge of the top arrow button - that gives us how far the mouse is 
       *   "into" the scroll range.  We also subtract half of the height of the knob, otherwise the knob would end up
       *   with the top edge at the mouse cursor (instead of the more desirable and logical center of the button).
       *  We take this distance in, and divide by the total navigatable pixels in the scroll bar (bh) - this should give us a value 0.00 - 1.00 
       *  That value is multiplied by the range this bar is supposed to cover (max - min) which gives us the number in range-units that we are into
       *  the bar.  Finally we add the minimum to get the actual range-unit the mouse is at.
       */
      sb->value = ((sb->knob->last_y - (sb->knob->Frame().Height()/2) - sb->decVal->Frame ().ymax) * (sb->max - sb->min) / bh) + sb->min;

  }
  else if (sb->orientation == sboHorizontal)
  {
    // Code for Horizontal scrollbars.  We use width for calculations here.
    // For detailed documentation of what is going on here, see above in the sboVertical case.
    int width = 10;
    csRect f (sb->Frame ());

    f.xmin += sb->decVal->Frame ().Width () + 1;
    f.xmax -= sb->incVal->Frame ().Width () + 1;

    if (sb->amntvis == 0)
      sb->WindowManager ()->GetPrefMgr ()->LookupIntKey (
          "ScrollBarWidth",
          width);
    else
      width = (int)((sb->amntvis * f.Width ()) / (sb->max - sb->min));

   
    int bw = f.Width () - width;

    if ((sb->max - sb->min <= 0) || (bw == 0))
      sb->value = sb->min;
    else
      sb->value = ((sb->knob->last_x - (sb->knob->Frame().Width()/2) - sb->decVal->Frame ().xmax) * (sb->max - sb->min) / bw) + sb->min ;
  }
  else
    return ;

  sb->value =
    (
      sb->value < sb->min ? sb->min :
        (sb->value > sb->max ? sb->max : sb->value)
    );
  sb->Broadcast (signalChanged);
  sb->Invalidate ();
}

void awsScrollBar::TickTock (void *sk, iAwsSource *)
{
  awsScrollBar *sb = (awsScrollBar *)sk;

  if (sb->orientation == sboVertical)
  {
    if (sb->last_y < sb->knob->Frame ().ymin)
      sb->value -= sb->amntvis;
    else if (sb->last_y > sb->knob->Frame ().ymax)
      sb->value += sb->amntvis;
    else
      return ;
  }
  else
  {
    if (sb->last_x < sb->knob->Frame ().xmin)
      sb->value -= sb->amntvis;
    else if (sb->last_x > sb->knob->Frame ().xmax)
      sb->value += sb->amntvis;
    else
      return ;
  }

  float maxval = (sb->max - sb->amntvis + 1);
  sb->value =
    (
      sb->value < sb->min ? sb->min :
        (sb->value > maxval ? maxval : sb->value)
    );

  sb->Broadcast (signalChanged);
  sb->Invalidate ();
}

void awsScrollBar::IncClicked (void *sk, iAwsSource *)
{
  awsScrollBar *sb = (awsScrollBar *)sk;

  sb->value += sb->value_delta;

  /// Check floor and ceiling
  sb->value =
    (
     sb->value < sb->min ? sb->min :
     (sb->value > sb->max ? sb->max : sb->value)
     );

  sb->Broadcast (signalChanged);
  sb->Invalidate ();
}

void awsScrollBar::DecClicked (void *sk, iAwsSource *)
{
  awsScrollBar *sb = (awsScrollBar *)sk;

  sb->value -= sb->value_delta;

  /// Check floor and ceiling
  sb->value =
    (
      sb->value < sb->min ? sb->min :
        (sb->value > sb->max ? sb->max : sb->value)
    );

  sb->Broadcast (signalChanged);
  sb->Invalidate ();
}

void awsScrollBar::OnDraw (csRect clip)
{
  int height = 10, width = 10;

  csRect f (Frame ());

  if (orientation == sboVertical)
  {
    // Get the bar height
    f.ymin += decVal->Frame ().Height () + 1;
    f.ymax -= incVal->Frame ().Height () + 1;

    // Get the knob height
    if (amntvis == 0)
      WindowManager ()->GetPrefMgr ()->LookupIntKey (
          "ScrollBarHeight",
          height);
    else
	{
      height = (int)((amntvis * f.Height ()) / (max-min));
	  // Minimum height of the scrollbar is 5 pixels
	  if (height<5)
		  height=5;
	}

    // Get the actual height that we can traverse with the knob
    int bh = f.Height () - height;

    // Get the knob's position
    int ky;
    if ((max-min) - amntvis == 0)
      ky = 0;
    else
      ky = (int)(((value-min) * bh) / ((max-min)-amntvis));

    f.ymin += ky;
    f.ymax = f.ymin + height;

    if (f.ymax > incVal->Frame ().ymin - 1)
      f.ymax = incVal->Frame ().ymin - 1;
  }
  else
  {
    f.xmin += decVal->Frame ().Width () + 1;
    f.xmax -= incVal->Frame ().Width () + 1;

    if (amntvis == 0)
      WindowManager ()->GetPrefMgr ()->LookupIntKey 
        ("ScrollBarWidth", width);
    else
	{
      width = (int)((amntvis * f.Width ()) / (max-min));
	  // Minimum width of the scrollbar is 5 pixels
	  if (width<5)
		  width=5;
	}

    // Get the actual height that we can traverse with the knob
    int bw = f.Width () - width;

    // Get the knob's position
    int kx;
    if (((max-min) - amntvis) == 0)
      kx = 0;
    else
      kx = (int)(((value-min) * bw) / (max-min));

    f.xmin += kx;
    f.xmax = f.xmin + width;

    if (f.xmax > incVal->Frame ().xmin - 1)
      f.xmax = incVal->Frame ().xmin - 1;
  }

  knob->ResizeTo(f);

  awsPanel::OnDraw(clip);
}

bool awsScrollBar::OnMouseDown (int btn, int x, int y)
{
  if (btn == 1 && !captured)
  {
    WindowManager ()->CaptureMouse (this);
    captured = true;
    timer->SetTimer (100);
    timer->Start ();
    last_x = x;
    last_y = y;
    return true;
  }

  return false;
}

bool awsScrollBar::OnMouseUp (int btn, int, int)
{
  if (captured && btn == 1)
  {
    WindowManager ()->ReleaseMouse ();
    captured = false;
    timer->Stop ();
  }

  return true;
}

bool awsScrollBar::OnMouseMove (int, int x, int y)
{
  if (captured)
  {
    last_x = x;
    last_y = y;
    return true;
  }

  return false;
}

bool awsScrollBar::OnMouseClick (int btn, int x, int y)
{
  return HandleClicking (btn, x, y);
}

bool awsScrollBar::OnMouseDoubleClick (int btn, int x, int y)
{
  return HandleClicking (btn, x, y);
}

bool awsScrollBar::HandleClicking (int btn, int x, int y)
{
  if (btn == 1)
  {
    if (captured) WindowManager ()->ReleaseMouse ();
    if (orientation == sboVertical)
    {
      if (y < knob->Frame ().ymin && y > decVal->Frame ().ymax)
        value -= amntvis;
      else if (y > knob->Frame ().ymax && y < incVal->Frame ().ymin)
        value += amntvis;
    }
    else
    {
      if (x < knob->Frame ().xmin && x > decVal->Frame ().xmax)
        value -= amntvis;
      else if (x > knob->Frame ().xmax && x < incVal->Frame ().xmin)
        value += amntvis;
    }

    // Check floor and ceiling
    value = (value < min ? min : (value > max ? max : value));

    Broadcast (signalChanged);
    Invalidate ();
    return true;
  }

  return false;
}

bool awsScrollBar::OnMouseExit ()
{
  mouse_is_over = false;
  Invalidate ();

  if (is_down) is_down = false;

  return true;
}

bool awsScrollBar::OnMouseEnter ()
{
  mouse_is_over = true;
  Invalidate ();
  return true;
}

bool awsScrollBar::OnKeyboard (const csKeyEventData& eventData)
{
  switch(eventData.codeCooked)
 {
  case CSKEY_UP:
    if(orientation == sboVertical)
		{
		 value -= value_delta;
		 Broadcast (signalChanged);
		 value = (value < min ? min : (value > max ? max : value));
		}
		break;
  case CSKEY_DOWN:
    if(orientation == sboVertical)
		{
		 value += value_delta;
		 Broadcast (signalChanged);
		 value = (value < min ? min : (value > max ? max : value));
		}
		break;
  case CSKEY_LEFT:
    if(orientation == sboHorizontal)
		{
		 value -= value_delta;
		 Broadcast (signalChanged);
		 value = (value < min ? min : (value > max ? max : value));
		}
		break;
  case CSKEY_RIGHT:
    if(orientation == sboHorizontal)
		{
		 value += value_delta;
		 Broadcast (signalChanged);
		 value = (value < min ? min : (value > max ? max : value));
		}
		break;
	}

	Invalidate ();

	return true;
}

void awsScrollBar::OnSetFocus ()
{
	Broadcast (signalFocused);
}

void awsScrollBar::OnAdded ()
{
  AddChild (incVal);
  AddChild (decVal);
  AddChild (knob);
}

void awsScrollBar::OnResized ()
{
  int h = incVal->Frame ().Height ();
  int w = incVal->Frame ().Width ();

  decVal->MoveTo(Frame ().xmax - w, Frame ().ymin);
  incVal->MoveTo(Frame ().xmax - w, Frame ().ymax - h);

}

/************************************* Command Button Factory ****************/

awsScrollBarFactory::awsScrollBarFactory (
  iAws *wmgr) :
    awsComponentFactory(wmgr)
{
  Register ("Scroll Bar");
  RegisterConstant ("sboVertical", awsScrollBar::sboVertical);
  RegisterConstant ("sboHorizontal", awsScrollBar::sboHorizontal);

  RegisterConstant ("signalScrollBarChanged", awsScrollBar::signalChanged);
  RegisterConstant ("signalScrollBarFocused", awsScrollBar::signalFocused);
}

awsScrollBarFactory::~awsScrollBarFactory ()
{
  // empty
}

iAwsComponent *awsScrollBarFactory::Create ()
{
  return new awsScrollBar;
}

/************************************* Slider Button ****************/

awsSliderButton::awsSliderButton () :
  timer(0),
  captured(false),
  nTicks((csTicks) 0),
  sink(0),
  tick_slot(0)
{
}

awsSliderButton::~awsSliderButton ()
{
  if (tick_slot)
    tick_slot->Disconnect (
        timer,
        awsTimer::signalTick,
        sink,
        sink->GetTriggerID ("TickTock"));

  if (captured) WindowManager ()->ReleaseMouse ();

  if (tick_slot) tick_slot->DecRef ();
  if (timer) timer->DecRef ();
}

bool awsSliderButton::Setup (iAws *wmgr, iAwsComponentNode *settings)
{
  if (!awsCmdButton::Setup (wmgr, settings)) return false;

  timer = new awsTimer (WindowManager ()->GetObjectRegistry (), this);
  awsSink* _sink = new awsSink (WindowManager());
  _sink->SetParm (this);
  sink = _sink;

  sink->RegisterTrigger ("TickTock", &TickTock);

  tick_slot = new awsSlot ();

  tick_slot->Connect (
      timer,
      awsTimer::signalTick,
      sink,
      sink->GetTriggerID ("TickTock"));
  return true;
}

bool awsSliderButton::GetProperty (const char *name, void **parm)
{
  if (awsCmdButton::GetProperty (name, parm)) return true;

  if (strcmp ("TicksPerSecond", name) == 0)
  {
    *parm = (void *) &nTicks;
    return true;
  }

  return false;
}

bool awsSliderButton::SetProperty (const char *name, void *parm)
{
  if (awsCmdButton::SetProperty (name, parm)) return true;

  if (strcmp ("TicksPerSecond", name) == 0)
  {
    csTicks n = *(csTicks *)parm;
    if (n <= 0)
      nTicks = (csTicks) 0;
    else
      nTicks = (csTicks) (1000 / n);
    timer->SetTimer (nTicks);

    return true;
  }

  return false;
}

const char *awsSliderButton::Type ()
{
  return "Slider Button";
}

void awsSliderButton::TickTock (void *sk, iAwsSource *)
{
  awsSliderButton *sb = (awsSliderButton *)sk;
  sb->Broadcast (signalClicked);
}

bool awsSliderButton::OnMouseDown (int btn, int x, int y)
{
  bool succ = awsCmdButton::OnMouseDown (btn, x, y);

  if (!is_switch && btn == 1 && nTicks != 0 && !captured)
  {
    timer->Start ();
    WindowManager ()->CaptureMouse (this);
    last_x = x;
    last_y = y;
    captured = true;
  }

  return succ;
}

bool awsSliderButton::OnMouseUp (int btn, int x, int y)
{
  bool succ = awsCmdButton::OnMouseUp (btn, x, y);
  if (!is_switch && captured)
  {
    timer->Stop ();
    WindowManager ()->ReleaseMouse ();
    captured = false;
  }

  return succ;
}

bool awsSliderButton::OnMouseMove (int, int x, int y)
{
  if (captured) last_x = x, last_y = y;

  return false;
}

bool awsSliderButton::OnMouseClick (int, int, int)
{
  if (captured)
  {
    timer->Stop ();
    WindowManager ()->ReleaseMouse ();
    captured = false;
  }

  return false;
}

bool awsSliderButton::OnMouseDoubleClick (int, int, int)
{
  if (captured)
  {
    timer->Stop ();
    WindowManager ()->ReleaseMouse ();
    captured = false;
  }

  return false;
}

/************************************* Slider Button Factory ****************/

awsSliderButtonFactory::awsSliderButtonFactory (
  iAws *wmgr) :
    awsCmdButtonFactory(wmgr)
{
  Register ("Slider Button");
}

awsSliderButtonFactory::~awsSliderButtonFactory ()
{
  // empty
}

iAwsComponent *awsSliderButtonFactory::Create ()
{
  return new awsSliderButton;
}
