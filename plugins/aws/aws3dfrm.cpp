#include "cssysdef.h"
#include "aws3dfrm.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"

const int aws3DFrame:: fsBump = 0x0;
const int aws3DFrame:: fsSimple = 0x1;
const int aws3DFrame:: fsRaised = 0x2;
const int aws3DFrame:: fsSunken = 0x3;
const int aws3DFrame:: fsFlat = 0x4;
const int aws3DFrame:: fsNone = 0x5;

aws3DFrame::aws3DFrame ()
{
}

aws3DFrame::~aws3DFrame ()
{
}

void aws3DFrame::Draw (
  iAws *wmgr,
  iAwsWindow *window,
  csRect &frame,
  int frame_style,
  iTextureHandle *bkg,
  int alpha_level)
{
  iGraphics2D *g2d = wmgr->G2D ();
  iGraphics3D *g3d = wmgr->G3D ();

  int hi = wmgr->GetPrefMgr ()->GetColor (AC_HIGHLIGHT);
  int hi2 = wmgr->GetPrefMgr ()->GetColor (AC_HIGHLIGHT2);
  int lo = wmgr->GetPrefMgr ()->GetColor (AC_SHADOW);
  int lo2 = wmgr->GetPrefMgr ()->GetColor (AC_SHADOW2);
  int fill = wmgr->GetPrefMgr ()->GetColor (AC_FILL);
  int dfill = wmgr->GetPrefMgr ()->GetColor (AC_DARKFILL);
  int black = wmgr->GetPrefMgr ()->GetColor (AC_BLACK);

  switch (frame_style)
  {
    case fsBump:
      g2d->DrawLine (
          frame.xmin + 0,
          frame.ymin + 0,
          frame.xmax - 1,
          frame.ymin + 0,
          hi);
      g2d->DrawLine (
          frame.xmin + 0,
          frame.ymin + 0,
          frame.xmin + 0,
          frame.ymax - 1,
          hi);
      g2d->DrawLine (
          frame.xmin + 0,
          frame.ymax - 1,
          frame.xmax - 1,
          frame.ymax - 1,
          lo);
      g2d->DrawLine (
          frame.xmax - 1,
          frame.ymin + 0,
          frame.xmax - 1,
          frame.ymax - 1,
          lo);
      g2d->DrawLine (
          frame.xmin + 1,
          frame.ymax - 0,
          frame.xmax - 0,
          frame.ymax - 0,
          black);
      g2d->DrawLine (
          frame.xmax - 0,
          frame.ymin + 1,
          frame.xmax - 0,
          frame.ymax - 0,
          black);

      g2d->DrawLine (
          frame.xmin + 1,
          frame.ymin + 1,
          frame.xmax - 2,
          frame.ymin + 1,
          hi2);
      g2d->DrawLine (
          frame.xmin + 1,
          frame.ymin + 1,
          frame.xmin + 1,
          frame.ymax - 2,
          hi2);
      g2d->DrawLine (
          frame.xmin + 1,
          frame.ymax - 2,
          frame.xmax - 2,
          frame.ymax - 2,
          lo2);
      g2d->DrawLine (
          frame.xmax - 2,
          frame.ymin + 1,
          frame.xmax - 2,
          frame.ymax - 2,
          lo2);

      g2d->DrawLine (
          frame.xmin + 2,
          frame.ymin + 2,
          frame.xmax - 3,
          frame.ymin + 2,
          lo2);
      g2d->DrawLine (
          frame.xmin + 2,
          frame.ymin + 2,
          frame.xmin + 2,
          frame.ymax - 3,
          lo2);
      g2d->DrawLine (
          frame.xmin + 2,
          frame.ymax - 3,
          frame.xmax - 3,
          frame.ymax - 3,
          hi2);
      g2d->DrawLine (
          frame.xmax - 3,
          frame.ymin + 2,
          frame.xmax - 3,
          frame.ymax - 3,
          hi2);

      g2d->DrawLine (
          frame.xmin + 3,
          frame.ymin + 3,
          frame.xmax - 4,
          frame.ymin + 3,
          black);
      g2d->DrawLine (
          frame.xmin + 3,
          frame.ymin + 3,
          frame.xmin + 3,
          frame.ymax - 4,
          black);

      if (bkg)
      {
        g3d->DrawPixmap (
            bkg,
            frame.xmin + 4,
            frame.ymin + 4,
            frame.Width () - 7,
            frame.Height () - 7,
            frame.xmin + 4 - window->Frame ().xmin,
            frame.ymin + 4 - window->Frame ().ymin,
            frame.Width () - 7,
            frame.Height () - 7,
            0);

        g3d->DrawPixmap (
            bkg,
            frame.xmin,
            frame.ymin,
            frame.Width (),
            4,
            frame.xmin - window->Frame ().xmin,
            frame.ymin - window->Frame ().ymin,
            frame.Width (),
            4,
            alpha_level);

        g3d->DrawPixmap (
            bkg,
            frame.xmin,
            frame.ymin + 4,
            4,
            frame.Height (),
            frame.xmin - window->Frame ().xmin,
            frame.ymin + 4 - window->Frame ().ymin,
            4,
            frame.Height (),
            alpha_level);

        g3d->DrawPixmap (
            bkg,
            frame.xmax - 4,
            frame.ymin + 4,
            4,
            frame.Height (),
            frame.xmax - 4 - window->Frame ().xmin,
            frame.ymin + 4 - window->Frame ().ymin,
            4,
            frame.Height (),
            alpha_level);

        g3d->DrawPixmap (
            bkg,
            frame.xmin + 4,
            frame.ymax - 4,
            frame.Width () - 4,
            4,
            frame.xmin + 4 - window->Frame ().xmin,
            frame.ymax - 4 - window->Frame ().ymin,
            frame.Width () - 4,
            4,
            alpha_level);
      }
      else
        g2d->DrawBox (
            frame.xmin + 4,
            frame.ymin + 4,
            frame.Width () - 7,
            frame.Height () - 7,
            fill);

      break;

    case fsSunken:
      g2d->DrawLine (
          frame.xmin + 0,
          frame.ymin + 0,
          frame.xmax - 1,
          frame.ymin + 0,
          lo2);
      g2d->DrawLine (
          frame.xmin + 0,
          frame.ymin + 0,
          frame.xmin + 0,
          frame.ymax - 1,
          lo2);
      g2d->DrawLine (
          frame.xmin + 1,
          frame.ymin + 1,
          frame.xmax - 0,
          frame.ymin + 1,
          lo);
      g2d->DrawLine (
          frame.xmin + 1,
          frame.ymin + 1,
          frame.xmin + 1,
          frame.ymax - 0,
          lo);
      g2d->DrawLine (
          frame.xmin + 1,
          frame.ymax - 0,
          frame.xmax - 0,
          frame.ymax - 0,
          hi);
      g2d->DrawLine (
          frame.xmax - 0,
          frame.ymin + 1,
          frame.xmax - 0,
          frame.ymax - 0,
          hi);

      g2d->DrawLine (
          frame.xmin + 2,
          frame.ymin + 2,
          frame.xmax - 1,
          frame.ymin + 2,
          black);
      g2d->DrawLine (
          frame.xmin + 2,
          frame.ymin + 2,
          frame.xmin + 2,
          frame.ymax - 1,
          black);
      g2d->DrawLine (
          frame.xmin + 2,
          frame.ymax - 1,
          frame.xmax - 1,
          frame.ymax - 1,
          hi2);
      g2d->DrawLine (
          frame.xmax - 1,
          frame.ymin + 2,
          frame.xmax - 1,
          frame.ymax - 1,
          hi2);

      g2d->DrawBox (
          frame.xmin + 3,
          frame.ymin + 3,
          frame.Width () - 3,
          frame.Height () - 3,
          dfill);

      if (bkg)
      {
        g3d->DrawPixmap (
            bkg,
            frame.xmin,
            frame.ymin,
            frame.Width () + 1,
            frame.Height () + 1,
            frame.xmin - window->Frame ().xmin,
            frame.ymin - window->Frame ().ymin,
            frame.Width () + 1,
            frame.Height () + 1,
            alpha_level);
      }
      break;

    case fsRaised:
      g2d->DrawLine (
          frame.xmin + 0,
          frame.ymin + 0,
          frame.xmax - 1,
          frame.ymin + 0,
          hi);
      g2d->DrawLine (
          frame.xmin + 0,
          frame.ymin + 0,
          frame.xmin + 0,
          frame.ymax - 1,
          hi);
      g2d->DrawLine (
          frame.xmin + 0,
          frame.ymax - 1,
          frame.xmax - 1,
          frame.ymax - 1,
          lo);
      g2d->DrawLine (
          frame.xmax - 1,
          frame.ymin + 0,
          frame.xmax - 1,
          frame.ymax - 1,
          lo);
      g2d->DrawLine (
          frame.xmin + 1,
          frame.ymax - 0,
          frame.xmax - 0,
          frame.ymax - 0,
          black);
      g2d->DrawLine (
          frame.xmax - 0,
          frame.ymin + 1,
          frame.xmax - 0,
          frame.ymax - 0,
          black);

      g2d->DrawLine (
          frame.xmin + 1,
          frame.ymin + 1,
          frame.xmax - 2,
          frame.ymin + 1,
          hi2);
      g2d->DrawLine (
          frame.xmin + 1,
          frame.ymin + 1,
          frame.xmin + 1,
          frame.ymax - 2,
          hi2);
      g2d->DrawLine (
          frame.xmin + 1,
          frame.ymax - 2,
          frame.xmax - 2,
          frame.ymax - 2,
          lo2);
      g2d->DrawLine (
          frame.xmax - 2,
          frame.ymin + 1,
          frame.xmax - 2,
          frame.ymax - 2,
          lo2);

      g2d->DrawBox (
          frame.xmin + 2,
          frame.ymin + 2,
          frame.Width () - 3,
          frame.Height () - 3,
          fill);

      if (bkg)
      {
        g3d->DrawPixmap (
            bkg,
            frame.xmin,
            frame.ymin,
            frame.Width () + 1,
            frame.Height () + 1,
            frame.xmin - window->Frame ().xmin,
            frame.ymin - window->Frame ().ymin,
            frame.Width () + 1,
            frame.Height () + 1,
            alpha_level);
      }
      break;

    case fsFlat:
      g2d->DrawBox (
          frame.xmin,
          frame.ymin,
          frame.Width (),
          frame.Height (),
          fill);

      if (bkg)
      {
        g3d->DrawPixmap (
            bkg,
            frame.xmin,
            frame.ymin,
            frame.Width () + 1,
            frame.Height () + 1,
            frame.xmin - window->Frame ().xmin,
            frame.ymin - window->Frame ().ymin,
            frame.Width () + 1,
            frame.Height () + 1,
            alpha_level);
      }
      break;

    case fsSimple:
      g2d->DrawBox (
          frame.xmin,
          frame.ymin,
          frame.Width (),
          frame.Height (),
          black);
      break;
    case fsNone:
      break;
  }
}
