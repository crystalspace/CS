#include "cssysdef.h"
#include "awscmdbt.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/fontserv.h"
#include "csutil/scfstr.h"
#include "csutil/csevent.h"
#include "iutil/evdefs.h"

#include <stdio.h>

const int awsCmdButton:: fsNormal = 0x0;
const int awsCmdButton:: fsToolbar = 0x1;
const int awsCmdButton:: fsBitmap = 0x2;
const int awsCmdButton:: iconLeft = 0x0;
const int awsCmdButton:: iconRight = 0x1;
const int awsCmdButton:: iconTop = 0x2;
const int awsCmdButton:: iconBottom = 0x3;

const int awsCmdButton:: signalClicked = 0x1;

awsCmdButton::awsCmdButton () :
  is_down(false),
  mouse_is_over(false),
  is_switch(false),
  was_down(false),
  frame_style(0),
  alpha_level(92),
  icon_align(0),
  caption(NULL)
{
  tex[0] = tex[1] = tex[2] = NULL;
}

awsCmdButton::~awsCmdButton ()
{
}

char *awsCmdButton::Type ()
{
  return "Command Button";
}

bool awsCmdButton::Setup (iAws *_wmgr, awsComponentNode *settings)
{
  int switch_style = 0;

  if (!awsComponent::Setup (_wmgr, settings)) return false;

  iAwsPrefManager *pm = WindowManager ()->GetPrefMgr ();

  pm->LookupIntKey ("OverlayTextureAlpha", alpha_level);  // global get
  pm->GetInt (settings, "Style", frame_style);
  pm->GetInt (settings, "Alpha", alpha_level);            // local overrides, if present.
  pm->GetInt (settings, "Toggle", switch_style);
  pm->GetInt (settings, "IconAlign", icon_align);
  pm->GetString (settings, "Caption", caption);

  is_switch = switch_style;

  switch (frame_style)
  {
    case fsNormal:
    case fsToolbar:
      {
        iString *tn = NULL;

        tex[0] = pm->GetTexture ("Texture");
        pm->GetString (settings, "Image", tn);

        if (tn) tex[1] = pm->GetTexture (tn->GetData (), tn->GetData ());

        iString *in = NULL;
        pm->GetString (settings, "Icon", in);
        if (in) tex[2] = pm->GetTexture (in->GetData (), in->GetData ());
      }
      break;

    case fsBitmap:
      {
        iString *tn1 = NULL, *tn2 = NULL, *tn3 = NULL;

        pm->GetString (settings, "BitmapNormal", tn1);
        pm->GetString (settings, "BitmapFocused", tn2);
        pm->GetString (settings, "BitmapClicked", tn3);

        if (tn1) tex[0] = pm->GetTexture (tn1->GetData (), tn1->GetData ());
        if (tn2) tex[1] = pm->GetTexture (tn2->GetData (), tn2->GetData ());
        if (tn3) tex[2] = pm->GetTexture (tn3->GetData (), tn3->GetData ());
      }
      break;
  }

  return true;
}

bool awsCmdButton::GetProperty (char *name, void **parm)
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

    *pb = &is_down;
    return true;
  }

  return false;
}

bool awsCmdButton::SetProperty (char *name, void *parm)
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
  else if (strcmp ("Image", name) == 0)
  {
    iTextureHandle *img = (iTextureHandle *) (parm);

    if (img)
    {
      if (tex[1]) tex[1]->DecRef ();
      tex[1] = img;
      img->IncRef ();
      Invalidate ();
    }

    return true;
  }

  return false;
}

void awsCmdButton::ClearGroup ()
{
  csEvent Event;

  Event.Type = csevGroupOff;

  int i;
  for (i = 0; i < Parent ()->GetChildCount (); ++i)
  {
    iAwsComponent *cmp = Parent ()->GetChildAt (i);

    if (cmp && cmp != this) cmp->HandleEvent (Event);
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
  int tw=0, th=0, tx, ty, itx=0, ity=0;

  iGraphics2D *g2d = WindowManager ()->G2D ();
  iGraphics3D *g3d = WindowManager ()->G3D ();

  int hi = WindowManager ()->GetPrefMgr ()->GetColor (AC_HIGHLIGHT);
  int hi2 = WindowManager ()->GetPrefMgr ()->GetColor (AC_HIGHLIGHT2);
  int lo = WindowManager ()->GetPrefMgr ()->GetColor (AC_SHADOW);
  int lo2 = WindowManager ()->GetPrefMgr ()->GetColor (AC_SHADOW2);
  int fill = WindowManager ()->GetPrefMgr ()->GetColor (AC_FILL);
  int dfill = WindowManager ()->GetPrefMgr ()->GetColor (AC_DARKFILL);
  int black = WindowManager ()->GetPrefMgr ()->GetColor (AC_BLACK);

  switch (frame_style)
  {
    case fsNormal:
    case fsToolbar:
      if (is_down)
      {
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
      }
      else
      {
        g2d->DrawLine (
            Frame ().xmin + 0,
            Frame ().ymin + 0,
            Frame ().xmax - 1,
            Frame ().ymin + 0,
            hi);
        g2d->DrawLine (
            Frame ().xmin + 0,
            Frame ().ymin + 0,
            Frame ().xmin + 0,
            Frame ().ymax - 1,
            hi);
        g2d->DrawLine (
            Frame ().xmin + 0,
            Frame ().ymax - 1,
            Frame ().xmax - 1,
            Frame ().ymax - 1,
            lo);
        g2d->DrawLine (
            Frame ().xmax - 1,
            Frame ().ymin + 0,
            Frame ().xmax - 1,
            Frame ().ymax - 1,
            lo);
        g2d->DrawLine (
            Frame ().xmin + 1,
            Frame ().ymax - 0,
            Frame ().xmax - 0,
            Frame ().ymax - 0,
            black);
        g2d->DrawLine (
            Frame ().xmax - 0,
            Frame ().ymin + 1,
            Frame ().xmax - 0,
            Frame ().ymax - 0,
            black);

        g2d->DrawLine (
            Frame ().xmin + 1,
            Frame ().ymin + 1,
            Frame ().xmax - 2,
            Frame ().ymin + 1,
            hi2);
        g2d->DrawLine (
            Frame ().xmin + 1,
            Frame ().ymin + 1,
            Frame ().xmin + 1,
            Frame ().ymax - 2,
            hi2);
        g2d->DrawLine (
            Frame ().xmin + 1,
            Frame ().ymax - 2,
            Frame ().xmax - 2,
            Frame ().ymax - 2,
            lo2);
        g2d->DrawLine (
            Frame ().xmax - 2,
            Frame ().ymin + 1,
            Frame ().xmax - 2,
            Frame ().ymax - 2,
            lo2);

        g2d->DrawBox (
            Frame ().xmin + 2,
            Frame ().ymin + 2,
            Frame ().Width () - 3,
            Frame ().Height () - 3,
            fill);
      }

      if (tex[0])
      {
        g3d->DrawPixmap (
            tex[0],
            Frame ().xmin,
            Frame ().ymin,
            Frame ().Width () + 1,
            Frame ().Height () + 1,
            Frame ().xmin - Window ()->Frame ().xmin,
            Frame ().ymin - Window ()->Frame ().ymin,
            Frame ().Width () + 1,
            Frame ().Height () + 1,
            alpha_level);
      }

      if (tex[1])
      {
        int img_w, img_h;

        tex[1]->GetOriginalDimensions (img_w, img_h);

        g3d->DrawPixmap (
            tex[1],
            Frame ().xmin + is_down,
            Frame ().ymin + is_down,
            Frame ().Width (),
            Frame ().Height (),
            0,
            0,
            img_w,
            img_h,
            0);
      }

      tx = Frame ().Width () >> 1;
      ty = Frame ().Height () >> 1;

      if (caption && frame_style == fsNormal)
      {
        // Get the size of the text
        WindowManager ()->GetPrefMgr ()->GetDefaultFont ()->GetDimensions (
            caption->GetData (),
            tw,
            th);
      }

      if (tex[2])
      {
        int img_w, img_h;
        itx = tx, ity = ty;

        tex[2]->GetOriginalDimensions (img_w, img_h);

        itx -= (img_w>>1);
        ity -= (img_h>>1);

        switch (icon_align)
        {
        case iconLeft:
          itx = tx - ((tw+img_w)>>1) - 1;
          ity = ty - (img_h>>1);
          tx = itx + img_w + 2;
          ty = ty - (th>>1);
          break;
        case iconRight:
          itx = tx + ((tw-img_w)>>1) + 1;
          ity = ty - (img_h>>1);
          tx = tx - ((tw+img_w)>>1) - 1;
          ty = ty - (th>>1);
          break;
        case iconTop:
          itx = tx - (img_w>>1);
          ity = ty - ((th+img_h)>>1) - 1;
          tx = tx - (tw>>1);
          ty = ity + img_h + 2;
          break;
        case iconBottom:
          itx = tx - (img_w>>1);
          ity = ty + ((th-img_h)>>1) + 1;
          tx = tx - (tw>>1);
          ty = ty - ((th+img_h)>>1) - 1;
          break;
        }
        
        g3d->DrawPixmap (
            tex[2],
            Frame ().xmin + is_down + itx,
            Frame ().ymin + is_down + ity,
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
        tx -= (tw>>1);
        ty -= (th>>1);
      }

      // Draw the caption, if there is one and the style permits it.
      if (caption && frame_style == fsNormal)
      {

        // Draw the text
        g2d->Write (
            WindowManager ()->GetPrefMgr ()->GetDefaultFont (),
            Frame ().xmin + tx + is_down,
            Frame ().ymin + ty + is_down,
            WindowManager ()->GetPrefMgr ()->GetColor (AC_TEXTFORE),
            -1,
            caption->GetData ());

        if (mouse_is_over)
        {
          int x, y, y1 = Frame ().ymin +
            ty +
            th +
            2 +
            is_down, y2 = Frame ().ymin +
            ty -
            2 +
            is_down, x1 = Frame ().xmin +
            is_down +
            4, x2 = Frame ().xmax +
            is_down -
            4;

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
      break;

    case fsBitmap:
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

        g3d->DrawPixmap (
            tex[texindex],
            Frame ().xmin+is_down,
            Frame ().ymin+is_down,
            w,
            h,
            0,
	    0,
            w,
            h,
            alpha_level);
      }
      break;
  }
}

csRect awsCmdButton::getPreferredSize ()
{
  return getMinimumSize ();
}

csRect awsCmdButton::getMinimumSize ()
{
  if (frame_style==fsBitmap)
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

    return csRect(0,0,w,h);
  }
  else if (frame_style == fsNormal && tex[2])
  {
    int tw = 0, th = 0;
    int img_w = 0, img_h = 0;

    if (caption)
    {
      // Get the size of the text
      WindowManager ()->GetPrefMgr ()->GetDefaultFont ()->GetDimensions (
	  caption->GetData (),
	  tw,
	  th);
    }

    tex[2]->GetOriginalDimensions (img_w, img_h);
    
    if (icon_align == iconLeft || icon_align == iconRight)
    {
      tw += img_w + 2;
      th = MAX(th, img_h);
    }
    else
    {
      th += img_h + 2;
      tw = MAX(tw, img_w);
    }

    return csRect (0, 0, tw + 6 + (tw >> 2), th + 6 + (th >> 1));

  }
  else
  {
    int tw = 0, th = 0;

    if (caption)
    {
      // Get the size of the text
      WindowManager ()->GetPrefMgr ()->GetDefaultFont ()->GetDimensions (
	  caption->GetData (),
	  tw,
	  th);
    }

     return csRect (0, 0, tw + 6 + (tw >> 2), th + 6 + (th >> 1));

  }
}

bool awsCmdButton::OnMouseDown (int, int, int)
{
  was_down = is_down;

  if (!is_switch || is_down == false) is_down = true;

  Invalidate ();
  return true;
}

bool awsCmdButton::OnMouseUp (int, int, int)
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

bool awsCmdButton::OnMouseMove (int, int, int)
{
  return true;
}

bool awsCmdButton::OnMouseClick (int, int, int)
{
  return false;
}

bool awsCmdButton::OnMouseDoubleClick (int, int, int)
{
  return false;
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

bool awsCmdButton::OnKeypress (int, int)
{
  return false;
}

bool awsCmdButton::OnLostFocus ()
{
  return true;
}

bool awsCmdButton::OnGainFocus ()
{
  return true;
}

/************************************* Command Button Factory ****************/
awsCmdButtonFactory::awsCmdButtonFactory (
  iAws *wmgr) :
    awsComponentFactory(wmgr)
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
}

awsCmdButtonFactory::~awsCmdButtonFactory ()
{
  // empty
}

iAwsComponent *awsCmdButtonFactory::Create ()
{
  return new awsCmdButton;
}
