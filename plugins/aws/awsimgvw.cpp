#include "cssysdef.h"
#include "awsimgvw.h"
#include "aws3dfrm.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/fontserv.h"
#include "csutil/scfstr.h"
#include "csutil/csevent.h"
#include "iutil/evdefs.h"

#include <stdio.h>

const int awsImageView:: signalClicked = 0x1;
const int awsImageView:: signalMouseDown = 0x2;
const int awsImageView:: signalMouseUp = 0x3;
const int awsImageView:: signalMouseMoved = 0x4;

const int awsImageView:: fsBump = 0x0;
const int awsImageView:: fsSimple = 0x1;
const int awsImageView:: fsRaised = 0x2;
const int awsImageView:: fsSunken = 0x3;
const int awsImageView:: fsFlat = 0x4;
const int awsImageView:: fsNone = 0x5;
const int awsImageView:: fsScaled = 0x8;
const int awsImageView:: fsTiled = 0x10;
const int awsImageView:: fsFixed = 0x20;

const int awsImageView:: frameMask = 0x7;
const int awsImageView:: imageMask = ~awsImageView::frameMask;


awsImageView::awsImageView () :
  is_down(false),
  mouse_is_over(false),
  was_down(false),
  img1(NULL),
  img2(NULL),
  frame_style(0),
  alpha_level(92)
{
}

awsImageView::~awsImageView ()
{
}

char *awsImageView::Type ()
{
  return "Image View";
}

bool awsImageView::Setup (iAws *_wmgr, awsComponentNode *settings)
{
  if (!awsComponent::Setup (_wmgr, settings)) return false;

  iAwsPrefManager *pm = WindowManager ()->GetPrefMgr ();

  pm->LookupIntKey ("OverlayTextureAlpha", alpha_level);  // global get
  pm->GetInt (settings, "Style", frame_style);
  pm->GetInt (settings, "Alpha", alpha_level);            // local overrides, if present.

  iString *file = NULL;
  pm->GetString (settings, "Image", file);
  if (file) 
  {
    unsigned char r=0,g=0,b=0;
    pm->GetRGB (settings, "KeyColor", r, g, b);
    img1 = pm->GetTexture (file->GetData (), file->GetData (), r, g, b);
  }

  img2 = pm->GetTexture ("Texture");

  return true;
}

bool awsImageView::GetProperty (char *name, void **parm)
{
  if (awsComponent::GetProperty (name, parm)) return true;

  return false;
}

bool awsImageView::SetProperty (char *name, void *parm)
{
  if (awsComponent::SetProperty (name, parm)) return true;

  return false;
}

void awsImageView::OnDraw (csRect /*clip*/)
{
  aws3DFrame frame3d;

  frame3d.Draw (
      WindowManager (),
      Window (),
      Frame (),
      frame_style & frameMask,
      img2,
      255);

  // now draw the image
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

    g3d->DrawPixmap (img,
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

bool awsImageView::OnMouseDown (int, int, int)
{
  Broadcast (signalMouseDown);

  was_down = is_down;

  if (is_down == false) is_down = true;

  Invalidate ();
  return true;
}

bool awsImageView::OnMouseUp (int, int, int)
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

bool awsImageView::OnMouseMove (int, int, int)
{
  Broadcast (signalMouseMoved);
  return false;
}

bool awsImageView::OnMouseClick (int, int, int)
{
  return false;
}

bool awsImageView::OnMouseDoubleClick (int, int, int)
{
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

bool awsImageView::OnKeypress (int, int)
{
  return false;
}

bool awsImageView::OnLostFocus ()
{
  return false;
}

bool awsImageView::OnGainFocus ()
{
  return false;
}

/************************************* Command Button Factory ****************/

awsImageViewFactory::awsImageViewFactory (
  iAws *wmgr) :
    awsComponentFactory(wmgr)
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
  RegisterConstant (
    "signalImageViewMouseDown",
    awsImageView::signalMouseDown);
  RegisterConstant (
    "signalImageViewMouseMoved",
    awsImageView::signalMouseMoved);
}

awsImageViewFactory::~awsImageViewFactory ()
{
  // empty
}

iAwsComponent *awsImageViewFactory::Create ()
{
  return new awsImageView;
}
