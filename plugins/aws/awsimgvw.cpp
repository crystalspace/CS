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

#include "awsimgvw.h"
#include "aws3dfrm.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/fontserv.h"
#include "csutil/scfstr.h"
#include "csutil/csevent.h"
#include "iutil/evdefs.h"

awsImageView::awsImageView ()
  : is_down (false),
    mouse_is_over (false),
    was_down (false),
    img1 (0),
    img2 (0),
    draw_color (false),
    color (-1),
    frame_style (0),
    alpha_level (92)
{
}

awsImageView::~awsImageView ()
{
}

const char *awsImageView::Type ()
{
  return "Image View";
}

bool awsImageView::Setup (iAws *_wmgr, iAwsComponentNode *settings)
{
  if (!awsComponent::Setup (_wmgr, settings)) return false;

  iAwsPrefManager *pm = WindowManager ()->GetPrefMgr ();

  pm->LookupIntKey ("OverlayTextureAlpha", alpha_level); // Global get.
  pm->GetInt (settings, "Style", frame_style);
  pm->GetInt (settings, "Alpha", alpha_level); // Local overrides, if present.

  iString *file = 0;
  pm->GetString (settings, "Image", file);
  if (file) 
  {
    unsigned char r=0,g=0,b=0;
    pm->GetRGB (settings, "KeyColor", r, g, b);
    img1 = pm->GetTexture (file->GetData (), file->GetData ());
  }

  img2 = pm->GetTexture ("Texture");

  unsigned char r,g,b;
  if (pm->GetRGB (settings, "Color", r, g, b))
  {
    draw_color = true;
    color = pm->FindColor (r,g,b);
  }
  return true;
}

void awsImageView::SetColor(int color_index)
{
  if (color_index >= 0)
  {
    draw_color = true;
    color = color_index;
  }
  else
  {
    draw_color = false;
    color = -1;
  }
}

int awsImageView::GetColor()
{
  return color;
}

bool awsImageView::GetProperty (const char *name, intptr_t *parm)
{
  if (awsComponent::GetProperty (name, parm)) return true;

  return false;
}

bool awsImageView::SetProperty (const char *name, intptr_t parm)
{
  if (awsComponent::SetProperty (name, parm)) return true;

  if (strcmp (name, "Color") == 0)
  {
    color = (int)parm;
    return true;
  }
  else
  {
    if (strcmp (name, "Image") == 0)
    {
      // Old img1 will stay in AWS cache.
      img1 = WindowManager ()->GetPrefMgr ()->GetTexture (
        (const char *)parm,
        (const char *)parm);
      return true;
    }
  }
  return false;
}

void awsImageView::OnDraw (csRect clip)
{
  aws3DFrame frame3d;

  frame3d.Setup(WindowManager(),img2, 255);
  frame3d.Draw (
    Frame (),
    frame_style & frameMask,
    Window ()->Frame ());

  if (draw_color)
  {
    WindowManager ()->G2D()->DrawBox (
      ClientFrame ().xmin,
      ClientFrame().ymin,
      ClientFrame().Width(),
      ClientFrame().Height(),
      color);
    return;
  }

  // Now draw the image.
  iTextureHandle *img = (img1 ? img1 : img2);
  if (img)
  {
    csRect r, t;
    iGraphics3D *g3d = WindowManager ()->G3D ();

    r = Frame ();
    switch (frame_style & frameMask)
    {
    case fsBump:
      r.Set (r.xmin + 4, r.ymin + 4, r.xmax - 3, r.ymax - 3);
      break;
    case fsFlat:
    case fsRaised:
    case fsSunken:
      r = Frame ();
      r.Set (r.xmin, r.ymin, r.xmax + 1 , r.ymax + 1);
      break;
    }

    int img_w, img_h;
    img->GetOriginalDimensions (img_w, img_h);

    switch (frame_style & imageMask)
    {
    case fsTiled:
      t.SetSize (r.Width (), r.Height ());
      break;
    case fsScaled:
      t.Set (0, 0, img_w, img_h);
      break;
    case fsFixed:
      t.Set (0, 0, MIN (img_w, r.Width ()), MIN (img_h, r.Height ()));
      r.SetSize (t.Width (), t.Height ());
      break;
    default:
      t.Set (0, 0, img_w, img_h);
      break;
    }

    g3d->DrawPixmap (
      img,
      r.xmin,
      r.ymin,
      r.Width (),
      r.Height (),
      t.xmin,
      t.ymin,
      t.Width (),
      t.Height (),
      0);
  }
}

bool awsImageView::OnMouseDown (int button, int x, int y)
{
  Broadcast (signalMouseDown);

  was_down = is_down;

  if (is_down == false) is_down = true;

  Invalidate ();
  return true;
}

bool awsImageView::OnMouseUp (int button, int x, int y)
{
  Broadcast (signalMouseUp);

  if (is_down)
  {
    Broadcast (signalClicked);
    is_down = false;
  }

  Invalidate ();
  return true;
}

bool awsImageView::OnMouseMove (int button, int x, int y)
{
  Broadcast (signalMouseMoved);
  return false;
}

bool awsImageView::OnMouseExit ()
{
  mouse_is_over = false;
  Invalidate ();

  if (is_down) is_down = false;

  return true;
}

bool awsImageView::OnMouseEnter ()
{
  mouse_is_over = true;
  Invalidate ();
  return true;
}

awsImageViewFactory::awsImageViewFactory (iAws *wmgr)
  : awsComponentFactory (wmgr)
{
  Register ("Image View");
  RegisterConstant ("ivfsBump", awsImageView::fsBump);
  RegisterConstant ("ivfsSimple", awsImageView::fsSimple);
  RegisterConstant ("ivfsRaised", awsImageView::fsRaised);
  RegisterConstant ("ivfsSunken", awsImageView::fsSunken);
  RegisterConstant ("ivfsFlat", awsImageView::fsFlat);
  RegisterConstant ("ivfsNone", awsImageView::fsNone);
  RegisterConstant ("ivfsScaled", awsImageView::fsScaled);
  RegisterConstant ("ivfsTiled", awsImageView::fsTiled);
  RegisterConstant ("ivfsFixed", awsImageView::fsFixed);

  RegisterConstant ("signalImageViewClicked", awsImageView::signalClicked);
  RegisterConstant ("signalImageViewMouseUp", awsImageView::signalMouseUp);
  RegisterConstant ("signalImageViewMouseDown", awsImageView::signalMouseDown);
  RegisterConstant ("signalImageViewMouseMoved",
    awsImageView::signalMouseMoved);
}

awsImageViewFactory::~awsImageViewFactory ()
{
  // Empty.
}

iAwsComponent *awsImageViewFactory::Create ()
{
  return new awsImageView;
}
