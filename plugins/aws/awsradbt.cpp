#include "cssysdef.h"
#include "awsradbt.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/fontserv.h"
#include "csutil/scfstr.h"
#include "csutil/csevent.h"
#include "iutil/evdefs.h"

#include <stdio.h>

const int awsRadButton:: alignLeft = 0x0;
const int awsRadButton:: alignRight = 0x1;
const int awsRadButton:: alignCenter = 0x2;

const int awsRadButton:: signalClicked   = 0x1;
const int awsRadButton:: signalTurnedOff = 0x2;
const int awsRadButton:: signalTurnedOn  = 0x3;

awsRadButton::awsRadButton () :
  is_down(false),
  mouse_is_over(false),
  is_on(false),
  frame_style(0),
  alpha_level(96),
  alignment(0),
  caption(NULL)
{
  tex[0] = tex[1] = tex[2] = tex[3] = NULL;
  SetFlag (AWSF_CMP_ALWAYSERASE);
}

awsRadButton::~awsRadButton ()
{
}

char *awsRadButton::Type ()
{
  return "Radio Button";
}

bool awsRadButton::Setup (iAws *_wmgr, awsComponentNode *settings)
{
  if (!awsComponent::Setup (_wmgr, settings)) return false;

  iAwsPrefManager *pm = WindowManager ()->GetPrefMgr ();

  pm->GetInt (settings, "Alpha", alpha_level);
  pm->GetInt (settings, "Align", alignment);
  pm->GetString (settings, "Caption", caption);

  tex[0] = pm->GetTexture ("RadioButtonUp");
  tex[1] = pm->GetTexture ("RadioButtonDn");
  tex[2] = pm->GetTexture ("RadioButtonOn");
  tex[3] = pm->GetTexture ("RadioButtonOff");

  return true;
}

bool awsRadButton::GetProperty (char *name, void **parm)
{
  if (awsComponent::GetProperty (name, parm)) return true;

  if (strcmp ("Caption", name) == 0)
  {
    char *st = NULL;

    if (caption) st = caption->GetData ();

    iString *s = new scfString (st);
    *parm = (void *)s;
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

bool awsRadButton::SetProperty (char *name, void *parm)
{
  if (awsComponent::SetProperty (name, parm)) return true;

  if (strcmp ("Caption", name) == 0)
  {
    iString *s = (iString *) (parm);

    if (s && s->Length ())
    {
      if (caption) caption->DecRef ();
      caption = s;
      caption->IncRef ();
      Invalidate ();
    }
    else
    {
      if (caption) caption->DecRef ();
      caption = NULL;
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

  int i;
  for (i = 0; i < Parent ()->GetChildCount (); ++i)
  {
    iAwsComponent *cmp = Parent ()->GetChildAt (i);

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
		Broadcast (signalTurnedOff);
		Broadcast (signalClicked);
        is_on = false;
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
  if (is_down) Broadcast (signalClicked);

  if (!is_on)
  {
    is_on = true;
    ClearGroup ();
  }

  is_down = false;

  Invalidate ();
  return true;
}

bool awsRadButton::OnMouseMove (int, int, int)
{
  return false;
}

bool awsRadButton::OnMouseClick (int, int, int)
{
  return false;
}

bool awsRadButton::OnMouseDoubleClick (int, int, int)
{
  return false;
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

bool awsRadButton::OnKeypress (int, int)
{
  return false;
}

bool awsRadButton::OnLostFocus ()
{
  return false;
}

bool awsRadButton::OnGainFocus ()
{
  return false;
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
}

awsRadButtonFactory::~awsRadButtonFactory ()
{
  // empty
}

iAwsComponent *awsRadButtonFactory::Create ()
{
  return new awsRadButton;
}
