#include "cssysdef.h"
#include "aws3dfrm.h"
#include "aws.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"

// These constants are also defined in awsPanel
// if you edit them here make sure to edit those as well
const int aws3DFrame:: fsBump = 0;
const int aws3DFrame:: fsSimple = 1;
const int aws3DFrame:: fsRaised = 2;
const int aws3DFrame:: fsSunken = 3;
const int aws3DFrame:: fsFlat = 4;
const int aws3DFrame:: fsNone = 5;
const int aws3DFrame:: fsBevel = 6;
const int aws3DFrame:: fsThick = 7;
const int aws3DFrame:: fsBitmap = 8;
const int aws3DFrame:: fsSmallRaised = 9;
const int aws3DFrame::fsSmallSunken = 10;
const int aws3DFrame:: fsMask  = 0xf;

aws3DFrame::aws3DFrame ()
{
}

aws3DFrame::~aws3DFrame ()
{
}

void aws3DFrame::Setup(iAws* wmgr, iTextureHandle* _bkg, int _bkg_alpha,
                       iTextureHandle* _ovl, int _ovl_alpha)
{
  g2d = wmgr->G2D ();
  g3d = wmgr->G3D ();

  hi = wmgr->GetPrefMgr ()->GetColor (AC_HIGHLIGHT);
  hi2 = wmgr->GetPrefMgr ()->GetColor (AC_HIGHLIGHT2);
  lo = wmgr->GetPrefMgr ()->GetColor (AC_SHADOW);
  lo2 = wmgr->GetPrefMgr ()->GetColor (AC_SHADOW2);
  fill = wmgr->GetPrefMgr ()->GetColor (AC_FILL);
  dfill = wmgr->GetPrefMgr ()->GetColor (AC_DARKFILL);
  black = wmgr->GetPrefMgr ()->GetColor (AC_BLACK);
  bfill = wmgr->GetPrefMgr ()->GetColor(AC_BACKFILL);

  bkg = _bkg;
  bkg_alpha = _bkg_alpha;
  ovl = _ovl;
  ovl_alpha = _ovl_alpha;
}

void aws3DFrame::SetBackgroundTexture(iTextureHandle* _bkg) { bkg = _bkg; }
void aws3DFrame::SetBackgroundAlpha(int _bkg_alpha) { bkg_alpha = _bkg_alpha; }
void aws3DFrame::SetOverlayTexture(iTextureHandle* _ovl) { ovl = _ovl; }
void aws3DFrame::SetOverlayAlpha(int _ovl_alpha) { ovl_alpha = _ovl_alpha; }
void aws3DFrame::SetBackgroundColor(int _bfill) { bfill = _bfill; }

void aws3DFrame::Draw(csRect frame, 
                      int frame_style,  
                      csRectRegion* rgn)
{
  Draw(frame, frame_style, frame, rgn);
}


void aws3DFrame::Draw(csRect frame, 
                      int frame_style, 
                      csRect bkg_align, 
                      csRectRegion* rgn)
{
  Draw(frame, frame_style, bkg_align, frame, rgn);
}

void aws3DFrame::Draw (
  csRect frame,
  int frame_style,
  csRect bkg_align,
  csRect ovl_align,
  csRectRegion* todraw)
{

  // if no region was passed make our own
  // this function gets called alot so while
  // this looks a little odd its probably better
  // to be allocating these things on the stack
  csRectRegion our_todraw;
  csRectRegion our_todraw_txt;

  if(todraw == NULL)
  {
    todraw = &our_todraw;
	  todraw->makeEmpty();
	  todraw->Include(frame);
  }

  csRect clientArea = frame;
  csRect insets = GetInsets(frame_style);
  clientArea.xmin += insets.xmin;
  clientArea.ymin += insets.ymin;
  clientArea.xmax -= insets.xmax;
  clientArea.ymax -= insets.ymax;

  // todraw should only include the area that we aren't
  // drawing the frame on
  todraw->ClipTo(clientArea);

  // todraw_txt includes the frame as well
  csRectRegion* todraw_txt = &our_todraw_txt;
  todraw_txt->Include(frame);
  todraw_txt->Exclude(clientArea);
  for(int i = 0; i < todraw->Count(); i++)
    todraw_txt->Include(todraw->RectAt(i));


  switch (frame_style & fsMask)
  {
  case fsBump:
	  DrawBumpFrame(frame);
	  DrawFlatBackground(todraw, bfill);
	  if(bkg)
		  DrawTexturedBackground(todraw_txt, bkg, bkg_alpha, bkg_align);
	  if(ovl)
		  DrawTexturedBackground(todraw_txt, ovl, ovl_alpha, ovl_align);
	  
	  break;
	  
  case fsSunken:
	  DrawSunkenFrame(frame);
	  DrawFlatBackground(todraw, bfill);
	  if(bkg)
		  DrawTexturedBackground(todraw_txt, bkg, bkg_alpha, bkg_align);
	  if(ovl)
		  DrawTexturedBackground(todraw_txt, ovl, ovl_alpha, ovl_align);
	  
	  break;
	  
  case fsRaised:
	  DrawRaisedFrame(frame);
	  DrawFlatBackground(todraw, bfill);
	  if(bkg)
		  DrawTexturedBackground(todraw_txt, bkg, bkg_alpha, bkg_align);
	  if(ovl)
		  DrawTexturedBackground(todraw_txt, ovl, ovl_alpha, ovl_align);
	  
	  break;
	  
  case fsFlat:
	  DrawFlatBackground(todraw, bfill);
	  if(bkg)
		  DrawTexturedBackground(todraw_txt, bkg, bkg_alpha, bkg_align);
	  if(ovl)
		  DrawTexturedBackground(todraw_txt, ovl, ovl_alpha, ovl_align);
	  break;
	  
  case fsSimple:
	  DrawFlatBackground(todraw, bfill);
	  break;
	  
  case fsNone:
	  break;
	  
  case fsBevel:
	  DrawBevelFrame(frame);
	  DrawFlatBackground(todraw, bfill);
	  if(bkg)
		  DrawTexturedBackground(todraw_txt, bkg, bkg_alpha, bkg_align);
	  if(ovl)
		  DrawTexturedBackground(todraw_txt, ovl, ovl_alpha, ovl_align);
	  break;
	  
  case fsThick:
	  DrawThickFrame(frame);
	  DrawFlatBackground(todraw, bfill);
	  if(bkg)
		  DrawTexturedBackground(todraw_txt, bkg, bkg_alpha, bkg_align);
	  if(ovl)
		  DrawTexturedBackground(todraw_txt, ovl, ovl_alpha, ovl_align);
	  break;
	  
  case fsBitmap:
	  DrawFlatBackground(todraw, bfill);
	  if(bkg)
		  DrawTexturedBackground(todraw_txt, bkg, bkg_alpha, bkg_align);
	  if(ovl)
		  DrawTexturedBackground(todraw_txt, ovl, ovl_alpha, ovl_align);
	  break;

  case fsSmallRaised:
    DrawSmallRaisedFrame(frame);
    DrawFlatBackground(todraw, bfill);
	  if(bkg)
		  DrawTexturedBackground(todraw_txt, bkg, bkg_alpha, bkg_align);
	  if(ovl)
		  DrawTexturedBackground(todraw_txt, ovl, ovl_alpha, ovl_align);
	  break;

  case fsSmallSunken:
    DrawSmallSunkenFrame(frame);
    DrawFlatBackground(todraw, bfill);
	  if(bkg)
		  DrawTexturedBackground(todraw_txt, bkg, bkg_alpha, bkg_align);
	  if(ovl)
		  DrawTexturedBackground(todraw_txt, ovl, ovl_alpha, ovl_align);
	  break;

  }
}

void aws3DFrame::DrawSmallRaisedFrame(csRect frame)
{
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
}

void aws3DFrame::DrawSmallSunkenFrame(csRect frame)
{
	g2d->DrawLine (
		frame.xmin + 0,
		frame.ymin + 0,
		frame.xmax - 1,
		frame.ymin + 0,
		lo);
	g2d->DrawLine (
		frame.xmin + 0,
		frame.ymin + 0,
		frame.xmin + 0,
		frame.ymax - 1,
		lo);
	g2d->DrawLine (
		frame.xmin + 0,
		frame.ymax - 1,
		frame.xmax - 1,
		frame.ymax - 1,
		hi);
	g2d->DrawLine (
		frame.xmax - 1,
		frame.ymin + 0,
		frame.xmax - 1,
		frame.ymax - 1,
		hi);
}

void aws3DFrame::DrawRaisedFrame(csRect frame)
{
	g2d->DrawLine (
		frame.xmin + 0,
		frame.ymin + 0,
		frame.xmax - 2,
		frame.ymin + 0,
		hi);
	g2d->DrawLine (
		frame.xmin + 0,
		frame.ymin + 0,
		frame.xmin + 0,
		frame.ymax - 2,
		hi);
	g2d->DrawLine (
		frame.xmin + 0,
		frame.ymax - 2,
		frame.xmax - 2,
		frame.ymax - 2,
		lo);
	g2d->DrawLine (
		frame.xmax - 2,
		frame.ymin + 0,
		frame.xmax - 2,
		frame.ymax - 2,
		lo);
	g2d->DrawLine (
		frame.xmin + 1,
		frame.ymax - 1,
		frame.xmax - 1,
		frame.ymax - 1,
		black);
	g2d->DrawLine (
		frame.xmax - 1,
		frame.ymin + 1,
		frame.xmax - 1,
		frame.ymax - 1,
		black);
	
	g2d->DrawLine (
		frame.xmin + 1,
		frame.ymin + 1,
		frame.xmax - 3,
		frame.ymin + 1,
		hi2);
	g2d->DrawLine (
		frame.xmin + 1,
		frame.ymin + 1,
		frame.xmin + 1,
		frame.ymax - 3,
		hi2);
	g2d->DrawLine (
		frame.xmin + 1,
		frame.ymax - 3,
		frame.xmax - 3,
		frame.ymax - 3,
		lo2);
	g2d->DrawLine (
		frame.xmax - 3,
		frame.ymin + 1,
		frame.xmax - 3,
		frame.ymax - 3,
		lo2);
}


void aws3DFrame::DrawSunkenFrame(csRect frame)
{
	g2d->DrawLine (
		frame.xmin + 0,
		frame.ymin + 0,
		frame.xmax - 2,
		frame.ymin + 0,
		lo2);
	g2d->DrawLine (
		frame.xmin + 0,
		frame.ymin + 0,
		frame.xmin + 0,
		frame.ymax - 2,
		lo2);
	g2d->DrawLine (
		frame.xmin + 1,
		frame.ymin + 1,
		frame.xmax - 1,
		frame.ymin + 1,
		lo);
	g2d->DrawLine (
		frame.xmin + 1,
		frame.ymin + 1,
		frame.xmin + 1,
		frame.ymax - 1,
		lo);
	g2d->DrawLine (
		frame.xmin + 1,
		frame.ymax - 1,
		frame.xmax - 1,
		frame.ymax - 1,
		hi);
	g2d->DrawLine (
		frame.xmax - 1,
		frame.ymin + 1,
		frame.xmax - 1,
		frame.ymax - 1,
		hi);
	
	g2d->DrawLine (
		frame.xmin + 2,
		frame.ymin + 2,
		frame.xmax - 2,
		frame.ymin + 2,
		black);
	g2d->DrawLine (
		frame.xmin + 2,
		frame.ymin + 2,
		frame.xmin + 2,
		frame.ymax - 2,
		black);
	g2d->DrawLine (
		frame.xmin + 2,
		frame.ymax - 2,
		frame.xmax - 2,
		frame.ymax - 2,
		hi2);
	g2d->DrawLine (
		frame.xmax - 2,
		frame.ymin + 2,
		frame.xmax - 2,
		frame.ymax - 2,
		hi2);
}



void aws3DFrame::DrawBumpFrame(csRect frame)
{
	g2d->DrawLine (
          frame.xmin + 0,
          frame.ymin + 0,
          frame.xmax - 2,
          frame.ymin + 0,
          hi);
      g2d->DrawLine (
          frame.xmin + 0,
          frame.ymin + 0,
          frame.xmin + 0,
          frame.ymax - 2,
          hi);
      g2d->DrawLine (
          frame.xmin + 0,
          frame.ymax - 2,
          frame.xmax - 2,
          frame.ymax - 2,
          lo);
      g2d->DrawLine (
          frame.xmax - 2,
          frame.ymin + 0,
          frame.xmax - 2,
          frame.ymax - 2,
          lo);
      g2d->DrawLine (
          frame.xmin + 1,
          frame.ymax - 1,
          frame.xmax - 1,
          frame.ymax - 1,
          black);
      g2d->DrawLine (
          frame.xmax - 1,
          frame.ymin + 1,
          frame.xmax - 1,
          frame.ymax - 1,
          black);

      g2d->DrawLine (
          frame.xmin + 1,
          frame.ymin + 1,
          frame.xmax - 3,
          frame.ymin + 1,
          hi2);
      g2d->DrawLine (
          frame.xmin + 1,
          frame.ymin + 1,
          frame.xmin + 1,
          frame.ymax - 3,
          hi2);
      g2d->DrawLine (
          frame.xmin + 1,
          frame.ymax - 3,
          frame.xmax - 3,
          frame.ymax - 3,
          lo2);
      g2d->DrawLine (
          frame.xmax - 3,
          frame.ymin + 1,
          frame.xmax - 3,
          frame.ymax - 3,
          lo2);

      g2d->DrawLine (
          frame.xmin + 2,
          frame.ymin + 2,
          frame.xmax - 4,
          frame.ymin + 2,
          lo2);
      g2d->DrawLine (
          frame.xmin + 2,
          frame.ymin + 2,
          frame.xmin + 2,
          frame.ymax - 4,
          lo2);
      g2d->DrawLine (
          frame.xmin + 2,
          frame.ymax - 4,
          frame.xmax - 4,
          frame.ymax - 4,
          hi2);
      g2d->DrawLine (
          frame.xmax - 4,
          frame.ymin + 2,
          frame.xmax - 4,
          frame.ymax - 4,
          hi2);

      g2d->DrawLine (
          frame.xmin + 3,
          frame.ymin + 3,
          frame.xmax - 5,
          frame.ymin + 3,
          black);
      g2d->DrawLine (
          frame.xmin + 3,
          frame.ymin + 3,
          frame.xmin + 3,
          frame.ymax - 5,
          black);
}

void aws3DFrame::DrawBevelFrame(csRect frame)
{
	// Draw a beveled border, fill-hi on top and left, black-shadow on bot and right
 
	g2d->DrawLine (
		frame.xmin,
		frame.ymin,
		frame.xmax,
		frame.ymin,
		fill);
	g2d->DrawLine (
		frame.xmin + 1,
		frame.ymin + 1,
		frame.xmax - 2,
		frame.ymin + 1,
		hi);
	
	g2d->DrawLine (
		frame.xmin,
		frame.ymin + 1,
		frame.xmin,
		frame.ymax - 1,
		fill);
	g2d->DrawLine (
		frame.xmin + 1,
		frame.ymin + 2,
		frame.xmin + 1,
		frame.ymax - 2,
		hi);
	
	g2d->DrawLine (
		frame.xmin,
		frame.ymax - 1,
		frame.xmax - 1,
		frame.ymax - 1,
		black);
	g2d->DrawLine (
		frame.xmin + 1,
		frame.ymax - 2,
		frame.xmax - 2,
		frame.ymax - 2,
		lo);
	
	g2d->DrawLine (
		frame.xmax - 1,
		frame.ymin,
		frame.xmax - 1,
		frame.ymax - 2,
		black);
	g2d->DrawLine (
		frame.xmax - 2,
		frame.ymin + 1,
		frame.xmax - 2,
		frame.ymax - 3,
		lo);
}

void aws3DFrame::DrawThickFrame(csRect frame)
{
	
	int topleft[9] = { fill, hi, hi2, fill, fill, fill, fill, fill, fill };
    int botright[9] = { black, lo, lo2, fill, fill, fill, fill, fill, fill };
    int step = 4;
	
	for (int i = 0; i < step; ++i)
	{
		g2d->DrawLine (
			frame.xmin + i,
			frame.ymin + i,
			frame.xmax - i-1,
			frame.ymin + i,
			topleft[i]);
		g2d->DrawLine (
			frame.xmin + i,
			frame.ymin + i,
			frame.xmin + i,
			frame.ymax - i-1,
			topleft[i]);
		g2d->DrawLine (
			frame.xmin + i,
			frame.ymax - i-1,
			frame.xmax - i-1,
			frame.ymax - i-1,
			botright[i]);
		g2d->DrawLine (
			frame.xmax - i-1,
			frame.ymin + i,
			frame.xmax - i-1,
			frame.ymax - i-1,
			botright[i]);
	}
}


csRect aws3DFrame::GetInsets(int style)
{
	csRect r;
	switch(style & fsMask)
	{
	case fsNone:
	case fsSimple:
	case fsFlat:
  case fsBitmap:
		r = csRect(0,0,0,0);
		break;
  case fsSmallRaised:
  case fsSmallSunken:
    r = csRect(1,1,1,1);
    break;
	case fsRaised:
    r = csRect(2,2,3,3);
    break;
	case fsSunken:
		r = csRect(3,3,2,2);
		break;
	case fsBevel:
		r = csRect(2,2,2,2);
		break;
	case fsBump:
	case fsThick:
		r = csRect(4,4,4,4);
    break;
	}

	return r;
}


csRect aws3DFrame::SubRectToAlign(csRect comp_frame, csRect txt_sub_rect)
{
	csRect ret;
	ret.xmin = comp_frame.xmin - txt_sub_rect.xmin;
	ret.ymin = comp_frame.ymin - txt_sub_rect.ymin;
    return ret;
}



void aws3DFrame::DrawTexturedBackground(csRectRegion* todraw,
                                        iTextureHandle* bkg,
                                        int alpha_level,
                                        csRect bkg_align)
{
	for (int i = 0; i < todraw->Count (); ++i)
	{
		csRect r (todraw->RectAt (i));
		g3d->DrawPixmap (
			bkg,
			r.xmin,
			r.ymin,
			r.Width (),
			r.Height (),
			r.xmin - bkg_align.xmin,
			r.ymin - bkg_align.ymin,
			r.Width (),
			r.Height (),
			alpha_level);
	}
}

void aws3DFrame::DrawFlatBackground(csRectRegion* todraw,    // the areas to fill
	                                int color)        
{
	for (int i = 0; i < todraw->Count (); ++i)
	{
		csRect r (todraw->RectAt (i));
		g2d->DrawBox(r.xmin, r.ymin, r.Width(), r.Height(), color);
	}
}

