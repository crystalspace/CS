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
#include "awsradbt.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/fontserv.h"
#include "csutil/scfstr.h"
#include "csutil/csevent.h"
#include "iutil/evdefs.h"

#include <stdio.h>

awsRadButton::awsRadButton () :
  is_down(false),
  mouse_is_over(false),
  is_on(false),
  frame_style(0),
  alpha_level(96),
  alignment(0)
{
  tex[0] = tex[1] = tex[2] = tex[3] = 0;
  SetFlag (AWSF_CMP_ALWAYSERASE);
}

awsRadButton::~awsRadButton ()
{
}

const char *awsRadButton::Type ()
{
  return "Radio Button";
}

bool awsRadButton::Setup (iAws *_wmgr, iAwsComponentNode *settings)
{
  if (!awsComponent::Setup (_wmgr, settings)) return false;

  iAwsPrefManager *pm = WindowManager ()->GetPrefMgr ();

  pm->GetInt (settings, "Alpha", alpha_level);
  pm->GetInt (settings, "Align", alignment);
  caption.AttachNew (new scfString ());//??
  pm->GetString (settings, "Caption", caption);

  tex[0] = pm->GetTexture ("RadioButtonUp");
  tex[1] = pm->GetTexture ("RadioButtonDn");
  tex[2] = pm->GetTexture ("RadioButtonOn");
  tex[3] = pm->GetTexture ("RadioButtonOff");

  int _focusable = 0;
  pm->GetInt (settings, "Focusable", _focusable);
	focusable = _focusable;


  return true;
}

bool awsRadButton::GetProperty (const char *name, intptr_t *parm)
{
  if (awsComponent::GetProperty (name, parm)) return true;

  if (strcmp ("Caption", name) == 0)
  {
    const char *st = 0;

    if (caption) st = caption->GetData ();

    iString *s = new scfString (st);
    *parm = (intptr_t)s;
    return true;
  }
  else if (strcmp ("State", name) == 0)
  {
    // in this case, the parm should point to a bool.
    bool **pb = (bool **)parm;

    *pb = &is_on;
    return true;
  }

  return false;
}

bool awsRadButton::SetProperty (const char *name, intptr_t parm)
{
  if (awsComponent::SetProperty (name, parm)) return true;

  if (strcmp ("Caption", name) == 0)
  {
    iString *s = (iString *) (parm);

    if (s && s->Length ())
    {
      //??if (caption) caption->DecRef ();
      caption = s;
      //??caption->IncRef ();
      Invalidate ();
    }
    else
    {
      //??if (caption) caption->DecRef ();
      caption = 0;
    }

    return true;
  }

  else if (strcmp ("State", name) == 0)

  {

   

    is_on = *(bool *)parm;

    return true;

  }

  return false;
}

void awsRadButton::ClearGroup ()
{
  csEvent Event;

  Event.Type = csevGroupOff;

  for (iAwsComponent* cmp = Parent()->GetTopChild(); cmp; cmp = cmp->ComponentBelow())
  {
    if (cmp && cmp != this) cmp->HandleEvent (Event);
  }



  Broadcast (signalTurnedOn);
}

bool awsRadButton::HandleEvent (iEvent &Event)
{
  if (awsComponent::HandleEvent (Event)) return true;

  switch (Event.Type)
  {
  case csevGroupOff:
    if (is_on)
    {
      is_on = false;
      Broadcast (signalTurnedOff);
      Broadcast (signalClicked);
      Invalidate ();
    }

    return true;
    break;
  }

  return false;
}

void awsRadButton::OnDraw (csRect /*clip*/)
{
  iGraphics2D *g2d = WindowManager ()->G2D ();
  iGraphics3D *g3d = WindowManager ()->G3D ();

  int txw = 0, txh = 0;
  int txy = 0, txx = 0;

  // int hi = WindowManager ()->GetPrefMgr ()->GetColor (AC_HIGHLIGHT);
  // int lo = WindowManager ()->GetPrefMgr ()->GetColor (AC_SHADOW);

  /// Get the size of our textures
  if (tex[0]) tex[0]->GetOriginalDimensions (txw, txh);

  txy = (Frame ().Height () >> 1) - (txh >> 1);

  switch (alignment)
  {
    case alignLeft:   txx = 0; break;
    case alignRight:  txx = Frame ().Width () - txw; break;
  }

  if (!is_down && tex[0])
  {
    g3d->DrawPixmap (
        tex[0],
        Frame ().xmin + txx + is_down,
        Frame ().ymin + txy + is_down,
        txw,
        txh,
        0,
        0,
        txw,
        txh,
        alpha_level);
  }
  else if (is_down && tex[1])
  {
    g3d->DrawPixmap (
        tex[1],
        Frame ().xmin + txx + is_down,
        Frame ().ymin + txy + is_down,
        txw,
        txh,
        0,
        0,
        txw,
        txh,
        alpha_level);
  }

  if (is_on && tex[2])
    g3d->DrawPixmap (
        tex[2],
        Frame ().xmin + txx + is_down,
        Frame ().ymin + txy + is_down,
        txw,
        txh,
        0,
        0,
        txw,
        txh,
        0);
  else if (!is_on && tex[3])
    g3d->DrawPixmap (
        tex[3],
        Frame ().xmin + txx + is_down,
        Frame ().ymin + txy + is_down,
        txw,
        txh,
        0,
        0,
        txw,
        txh,
        0);

  // Draw the caption, if there is one and the style permits it.
  if (caption)
  {
    int tw, th, tx, ty, mcc;

    mcc = WindowManager ()->GetPrefMgr ()->GetDefaultFont ()->GetLength (
        caption->GetData (),
        Frame ().Width () - txw - 2);

    scfString tmp (caption->GetData ());
    tmp.Truncate (mcc);

    // Get the size of the text
    WindowManager ()->GetPrefMgr ()->GetDefaultFont ()->GetDimensions (
        tmp.GetData (),
        tw,
        th);

    // Calculate the center
    ty = (Frame ().Height () >> 1) - (th >> 1);

    switch (alignment)
    {
      case alignRight:
        tx = Frame ().Width () - txw - tw - 2;
        break;
      case alignLeft:
      default:
        tx = txw + 2;
        break;
    }

    // Draw the text
    g2d->Write (
        WindowManager ()->GetPrefMgr ()->GetDefaultFont (),
        Frame ().xmin + tx + is_down,
        Frame ().ymin + ty + is_down,
        WindowManager ()->GetPrefMgr ()->GetColor (AC_TEXTFORE),
        -1,
        tmp.GetData ());
	}
}

bool awsRadButton::OnMouseDown (int, int, int)
{
  is_down = true;
  Invalidate ();
  return true;
}

bool awsRadButton::OnMouseUp (int, int, int)
{
  if (is_down) 
  {
    if (!is_on)
    {
      is_on = true;
      ClearGroup ();
    }

    is_down = false;
    Broadcast (signalClicked);
  }

  Invalidate ();
  return true;
}

bool awsRadButton::OnMouseExit ()
{
  mouse_is_over = false;
  Invalidate ();

  if (is_down) is_down = false;

  return true;
}

bool awsRadButton::OnMouseEnter ()
{
  mouse_is_over = true;
  Invalidate ();
  return true;
}

bool awsRadButton::OnKeyboard (const csKeyEventData& eventData)
{
  bool eventEaten = false;
  switch(eventData.codeCooked)
  {
  case CSKEY_ENTER:
    eventEaten = true;
    if (!is_on)
    {
      is_on = true;
      ClearGroup ();
    }
    Broadcast (signalClicked);
    break;
  }
  
  Invalidate ();
  
  return eventEaten;
}

void awsRadButton::OnSetFocus ()
{
	Broadcast (signalFocused);
}

/************************************* Command Button Factory ****************/

awsRadButtonFactory::awsRadButtonFactory (
  iAws *wmgr) :
    awsComponentFactory(wmgr)
{
  Register ("Radio Button");

  RegisterConstant ("rbAlignLeft", awsRadButton::alignLeft);
  RegisterConstant ("rbAlignRight", awsRadButton::alignRight);
  RegisterConstant ("signalRadButtonClicked", awsRadButton::signalClicked);

  RegisterConstant ("signalRadButtonTurnedOff", awsRadButton::signalTurnedOff);

  RegisterConstant ("signalRadButtonTurnedOn", awsRadButton::signalTurnedOn);

  RegisterConstant ("signalRadButtonFocused", awsRadButton::signalFocused);
}

awsRadButtonFactory::~awsRadButtonFactory ()
{
  // empty
}

iAwsComponent *awsRadButtonFactory::Create ()
{
  return new awsRadButton;
}
