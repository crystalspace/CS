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
#include <stdio.h>
#include "awscmdbt.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/fontserv.h"
#include "csutil/scfstr.h"
#include "csutil/csevent.h"
#include "iutil/evdefs.h"

awsCmdButton::awsCmdButton ()
  : is_down (false),
    mouse_is_over (false),
    is_switch (false),
    was_down (false),
    icon_align (0),
    stretched (false),
    caption (0)
{
  tex[0] = tex[1] = tex[2] = 0;
  style = fsNormal;
}

awsCmdButton::~awsCmdButton ()
{
  if (caption != 0)
  {
    caption->DecRef();
  }
}

const char *awsCmdButton::Type ()
{
  return "Command Button";
}

bool awsCmdButton::Setup (iAws *_wmgr, iAwsComponentNode *settings)
{
  int switch_style = is_switch;
  iAwsPrefManager *pm = _wmgr->GetPrefMgr ();
  
  // The command button can use "Image" rather than "BitmapOverlay" to
  // setup its overlay image therefore we create a BitmapOverlay key
  // from the Image key for back compatibility.
  iString *tn = 0;
  if (!pm->GetString (settings, "BitmapOverlay", tn) &&
    pm->GetString (settings, "Image", tn))
  {
    awsStringKey* temp = new awsStringKey (_wmgr, "BitmapOverlay", tn);
    csRef<iAwsStringKey> key (SCF_QUERY_INTERFACE (temp, iAwsStringKey));
    settings->Add (key);
    temp->DecRef ();
  }

  if (!awsPanel::Setup (_wmgr, settings)) return false;

  pm->GetInt (settings, "Toggle", switch_style);
  pm->GetInt (settings, "IconAlign", icon_align);
  pm->GetString (settings, "Caption", caption);

  if (caption != 0)
  {
    caption->IncRef();
  }

  is_switch = switch_style;

  if (style == fsNormal || style == fsToolbar)
  {
    iString *in = 0;
    pm->GetString (settings, "Icon", in);
    if (in) tex[0] = pm->GetTexture (in->GetData (), in->GetData ());
  }
  else if (style == fsBitmap)
  {
    iString *tn1 = 0, *tn2 = 0, *tn3 = 0;

    int stretch;
    pm->GetString (settings, "BitmapNormal", tn1);
    pm->GetString (settings, "BitmapFocused", tn2);
    pm->GetString (settings, "BitmapClicked", tn3);
    if (pm->GetInt (settings, "Stretched", stretch)) stretched = stretch;
    if (tn1) tex[0] = pm->GetTexture (tn1->GetData (), tn1->GetData ());
    if (tn2) tex[1] = pm->GetTexture (tn2->GetData (), tn2->GetData ());
    if (tn3) tex[2] = pm->GetTexture (tn3->GetData (), tn3->GetData ());
  }
  else
    return false;
  return true;
}

bool awsCmdButton::GetProperty (const char *name, void **parm)
{
  if (awsComponent::GetProperty (name, parm)) return true;

  if (strcmp ("Caption", name) == 0)
  {
    const char *st = 0;

    if (caption) st = caption->GetData ();

    iString *s = new scfString (st);
    *parm = (void *)s;
    return true;
  }
  else if (strcmp ("State", name) == 0)
  {
   *parm = (void*)is_down;
    return true;
  }
  return false;
}

bool awsCmdButton::SetProperty (const char *name, void *parm)
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
      if (caption)
        caption->DecRef ();

      caption = 0;
    }
    return true;
  }
  else if (strcmp ("Image", name) == 0)
  {
    iTextureHandle *img = (iTextureHandle *) (parm);

    if (img)
    {
      if (ovl) ovl->DecRef ();
      ovl = img;
      frame_drawer.SetOverlayTexture(ovl);
      img->IncRef ();
      Invalidate ();
    }
    return true;
  }
  else if (strcmp ("State", name) == 0)
  {
    if (is_switch)
    {
      is_down  = (bool)parm;
      was_down = (bool)parm;
      ClearGroup ();
    }
    else
    {
      if (((bool)parm) == is_down)
        return true;

      is_down = (bool)parm;
    }
    Invalidate ();
    return true;
  }   
  return false;
}

void awsCmdButton::ClearGroup ()
{
  csEvent Event;

  Event.Type = csevGroupOff;

  iAwsComponent* cmp = Parent ()->GetTopChild ();
  while (cmp)
  {
    if (cmp && cmp != this) cmp->HandleEvent (Event);
      cmp = cmp->ComponentBelow ();
  }
}

bool awsCmdButton::HandleEvent (iEvent &Event)
{
  if (awsComponent::HandleEvent (Event)) return true;

  switch (Event.Type)
  {
  case csevGroupOff:
    if (is_down && is_switch)
    {
      is_down = false;
      Invalidate ();
    }
    return true;
    break;
  }
  return false;
}

void awsCmdButton::OnDraw (csRect clip)
{
  int tw = 0, th = 0;

  bool can_raise = (mouse_is_over &&  !(WindowManager ()->GetFlags () &
    AWSF_KeyboardControl)) || (isFocused () &&
    (WindowManager ()->GetFlags () & AWSF_KeyboardControl));

  iGraphics2D *g2d = WindowManager ()->G2D ();
  iGraphics3D *g3d = WindowManager ()->G3D ();

  int hi = WindowManager ()->GetPrefMgr ()->GetColor (AC_HIGHLIGHT);
  //int hi2 = WindowManager ()->GetPrefMgr ()->GetColor (AC_HIGHLIGHT2);
  int lo = WindowManager ()->GetPrefMgr ()->GetColor (AC_SHADOW);
  //int lo2 = WindowManager ()->GetPrefMgr ()->GetColor (AC_SHADOW2);
  int fill = WindowManager ()->GetPrefMgr ()->GetColor (AC_FILL);
  //int dfill = WindowManager ()->GetPrefMgr ()->GetColor (AC_DARKFILL);
  //int black = WindowManager ()->GetPrefMgr ()->GetColor (AC_BLACK);

  frame_drawer.SetBackgroundColor (fill);

  if (style == fsToolbar || style == fsNormal)
  {
    int showing_style = fsFlat; // Initialize so the compiler doesn't complain.
    if (style == fsNormal)
    {
      if (is_down)
        showing_style = fsSunken;
      else
        showing_style = fsRaised;
      }
      else if (style == fsToolbar)
      {
        if (is_down)
          showing_style = fsSunken;
        else if (can_raise)
          showing_style = fsRaised;
        else
          showing_style = fsFlat;
      }
      frame_drawer.Draw (frame, showing_style, Window ()->Frame ());
    }
  
  int tx = Frame ().Width () >> 1;
  int ty = Frame ().Height () >> 1;
  
  if (caption)
  {
    // Get the size of the text.
    WindowManager ()->GetPrefMgr ()->GetDefaultFont ()->GetDimensions (
      caption->GetData (),
      tw,
      th);
  }
  
  if (tex[0])
  {
    int img_w, img_h;
    int itx = tx;
    int ity = ty;
    
    tex[0]->GetOriginalDimensions (img_w, img_h);
    itx -= (img_w >> 1);
    ity -= (img_h >> 1);

    switch (icon_align)
    {
    case iconLeft:
      itx = tx - ((tw + img_w) >> 1) - 1;
      ity = ty - (img_h >> 1);
      tx = itx + img_w + 2;
      ty = ty - (th >> 1);
      break;
    case iconRight:
      itx = tx + ((tw - img_w) >> 1) + 1;
      ity = ty - (img_h >> 1);
      tx = tx - ((tw + img_w) >> 1) - 1;
      ty = ty - (th >> 1);
      break;
    case iconTop:
      itx = tx - (img_w >> 1);
      ity = ty - ((th + img_h) >> 1) - 1;
      tx = tx - (tw >> 1);
      ty = ity + img_h + 2;
      break;
    case iconBottom:
      itx = tx - (img_w >> 1);
      ity = ty + ((th - img_h) >> 1) + 1;
      tx = tx - (tw >> 1);
      ty = ty - ((th + img_h) >> 1) - 1;
      break;
    }
    
    g3d->DrawPixmap (
      tex[0],
      Frame ().xmin + is_down + itx + 2,
      Frame ().ymin + is_down + ity + 2,
      img_w,
      img_h,
      0,
      0,
      img_w,
      img_h,
      0);
  }
  else
  {
    tx -= (tw >> 1);
    ty -= (th >> 1);
  }
  
  // Draw the caption, if there is one and the style permits it.
  if (caption)
  {
    // Draw the text.
    g2d->Write (
      WindowManager ()->GetPrefMgr ()->GetDefaultFont (),
      Frame ().xmin + tx + is_down,
      Frame ().ymin + ty + is_down,
      WindowManager ()->GetPrefMgr ()->GetColor (AC_TEXTFORE),
      -1,
      caption->GetData ());

    if (can_raise && style == fsNormal)
    {
      int x, y;
      int y1 = Frame ().ymin + ty + th + 2 + is_down, y2 = Frame ().ymin +
        ty - 2 + is_down, x1 = Frame ().xmin + is_down + 4,
        x2 = Frame ().xmax + is_down - 4;

      for (x = x1; x < x2; ++x)
      {
        g2d->DrawPixel (x, y1, (x & 1 ? hi : lo));
        g2d->DrawPixel (x, y2, (x & 1 ? hi : lo));
      }

      for (y = y2; y < y1; ++y)
      {
        g2d->DrawPixel (x1, y, (y & 1 ? hi : lo));
        g2d->DrawPixel (x2, y, (y & 1 ? hi : lo));
      }
    }
  }
  else if (style == fsBitmap)
  {
    int texindex;
    int w, h;

    if (is_down)
      texindex = 2;
    else if (can_raise)
      texindex = 1;
    else
      texindex = 0;

    tex[texindex]->GetOriginalDimensions (w, h);

    g3d->DrawPixmap (
      tex[texindex],
      Frame ().xmin+is_down,
      Frame ().ymin+is_down,
      stretched ? Frame ().Width () : w,
      stretched ? Frame ().Height () : h,
      0,
      0,
      w,
      h,
      bkg_alpha);
  }
}

csRect awsCmdButton::getMinimumSize ()
{
  if (style==fsBitmap)
  {
    int texindex;
    int w, h;

    if (is_down)
      texindex = 2;
    else if (mouse_is_over)
      texindex = 1;
    else
      texindex = 0;

    tex[texindex]->GetOriginalDimensions (w, h);

    return csRect (0, 0, w, h);
  }
  else if (style == fsNormal && tex[0])
  {
    int tw = 0, th = 0;
    int img_w = 0, img_h = 0;

    if (caption)
    {
      // Get the size of the text.
      WindowManager ()->GetPrefMgr ()->GetDefaultFont ()->GetDimensions (
        caption->GetData (),
        tw,
        th);
    }

    tex[0]->GetOriginalDimensions (img_w, img_h);
    
    if (icon_align == iconLeft || icon_align == iconRight)
    {
      tw += img_w + 2;
      th = MAX (th, img_h);
    }
    else
    {
      th += img_h + 2;
      tw = MAX (tw, img_w);
    }

    return csRect (0, 0, tw + 6 + (tw >> 2), th + 6 + (th >> 1));

  }
  else
  {
    int tw = 0, th = 0;

    if (caption)
    {
      // Get the size of the text.
      WindowManager ()->GetPrefMgr ()->GetDefaultFont ()->GetDimensions (
        caption->GetData (),
        tw,
        th);
    }
     return csRect (0, 0, tw + 6 + (tw >> 2), th + 6 + (th >> 1));
  }
}

bool awsCmdButton::OnMouseDown (int button, int x, int y)
{
  was_down = is_down;

  if (!is_switch || is_down == false) is_down = true;

  Invalidate ();
  return true;
}

bool awsCmdButton::OnMouseUp (int button, int x, int y)
{
  if (!is_switch)
  {
    if (is_down) Broadcast (signalClicked);

    is_down = false;
  }
  else
  {
    if (was_down)
      is_down = false;
    else
      ClearGroup ();

    Broadcast (signalClicked);
  }
  Invalidate ();
  return true;
}

bool awsCmdButton::OnMouseExit ()
{
  mouse_is_over = false;
  Invalidate ();

  if (is_down && !is_switch) is_down = false;

  return true;
}

bool awsCmdButton::OnMouseEnter ()
{
  mouse_is_over = true;
  Invalidate ();
  return true;
}

bool awsCmdButton::OnKeyboard (const csKeyEventData& eventData)
{
  bool eventEaten = false;
  switch(eventData.codeCooked)
  {
  case CSKEY_ENTER:
    eventEaten = true;
    was_down = is_down;
    
    if (!is_switch || is_down == false) is_down = true;
    
    if (!is_switch)
    {
      if (is_down) 
        Broadcast (signalClicked);
      is_down = false;
    }
    else
    {
      if (was_down)
        is_down = false;
      else
        ClearGroup ();
      
      Broadcast (signalClicked);
    }
    break;
  }
  Invalidate ();
  return eventEaten;
}

void awsCmdButton::OnSetFocus ()
{
  Broadcast (signalFocused);
}

awsCmdButtonFactory::awsCmdButtonFactory ( iAws *wmgr)
  : awsComponentFactory (wmgr)
{
  Register ("Command Button");
  RegisterConstant ("bfsNormal", awsCmdButton::fsNormal);
  RegisterConstant ("bfsToolbar", awsCmdButton::fsToolbar);
  RegisterConstant ("bfsBitmap", awsCmdButton::fsBitmap);
  RegisterConstant ("biaLeft", awsCmdButton::iconLeft);
  RegisterConstant ("biaRight", awsCmdButton::iconRight);
  RegisterConstant ("biaTop", awsCmdButton::iconTop);
  RegisterConstant ("biaBottom", awsCmdButton::iconBottom);

  RegisterConstant ("signalCmdButtonClicked", awsCmdButton::signalClicked);
  RegisterConstant ("signalCmdButtonFocused", awsCmdButton::signalFocused);
}

awsCmdButtonFactory::~awsCmdButtonFactory ()
{
  // Empty.
}

iAwsComponent *awsCmdButtonFactory::Create ()
{
  return new awsCmdButton;
}
