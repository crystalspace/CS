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
#include "awswin.h"
#include "awslayot.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/fontserv.h"
#include "iutil/event.h"
#include "csutil/scfstr.h"
#include "csutil/snprintf.h"
#include "iaws/awsdefs.h"
#include "ivaria/view.h"
#include "iengine/engine.h"
#include "iengine/camera.h"
#include "aws3dfrm.h"

// makes sure this header file stays consistent with the project.
#include "iaws/awsecomp.h"

#include <stdio.h>
#include <string.h>

const unsigned long awsWindow::sWindowRaised = 0x1;
const unsigned long awsWindow::sWindowLowered = 0x2;
const unsigned long awsWindow::sWindowShown = 0x3;
const unsigned long awsWindow::sWindowHidden = 0x4;
const unsigned long awsWindow::sWindowClosed = 0x5;
const unsigned long awsWindow::sWindowZoomed = 0x6;
const unsigned long awsWindow::sWindowMinimized = 0x7;


const int awsWindow:: foControl = 0x1;
const int awsWindow:: foZoom = 0x2;
const int awsWindow:: foMin = 0x4;
const int awsWindow:: foClose = 0x8;
const int awsWindow:: foTitle = 0x10;
const int awsWindow:: foGrip = 0x20;
const int awsWindow:: foNoDrag = 0x40;

const int awsWindow:: foRoundBorder = 0x80;			// default
const int awsWindow:: foBeveledBorder = 0x100;
const int awsWindow:: foNoBorder = 0x200;
const int awsWindow:: foDontCaptureMouseMove = 0x400;


const int grip_size = 16;

// Set to true to get printf info about events, false to disable them.
const bool DEBUG_WINDOW_EVENTS = false;

awsWindow::awsWindow () :
  frame_options(foControl | foZoom | foClose | foTitle | foGrip |
		foRoundBorder),
  title_bar_height(0),
  title(0),
  resizing_mode(false),
  moving_mode(false),
  sink(0),
  is_minimized(false),
  popup(0),
  menu(0),
  window_focused_child(0)
{
  // Window start off hidden.
  SetFlag (AWSF_CMP_HIDDEN);
  SetFlag (AWSF_CMP_WINDOW);
  SetFlag (AWSF_CMP_TOP_SELECT);
}

awsWindow::~awsWindow ()
{
  delete sink;
  if (title)
    title->DecRef();
}

bool awsWindow::Setup (iAws *_wmgr, iAwsComponentNode *settings)
{
  style = fsNormal;

  if (!awsPanel::Setup (_wmgr, settings)) return false;
  if (Layout ()) Layout ()->SetOwner (GetComponent());

  iAwsPrefManager *pm = WindowManager ()->GetPrefMgr ();

  // set the default options depending on style
  // the constructor by default includes them all
  if(style == fsBitmap)
    frame_options = 0;

  pm->GetInt (settings, "Options", frame_options);
  pm->GetString (settings, "Title", title);
 
  // We have to incref this or else if our title is changed we release the copy
  // in the prefmanager!
  if (title)
    title->IncRef();
  pm->LookupIntKey ("TitleBarHeight", title_bar_height);

  unsigned char red, green, blue;
  if(pm->LookupRGBKey ("TitleBarTextColor", red, green, blue))
    title_text_color = pm->FindColor(red, green, blue);
  else
    title_text_color = pm->GetColor(AC_TEXTFORE);

  // setup the colors for the title bar
  for (int i = 0; i < 12; i++)
    title_color[i] = 128;

  pm->LookupRGBKey("ActiveTitleBarColor1", title_color[0], title_color[1],
		   title_color[2]);
  if(!pm->LookupRGBKey("ActiveTitleBarColor2", title_color[3], title_color[4],
		       title_color[5]))
  {
    title_color[3] = title_color[0];
    title_color[4] = title_color[1];
    title_color[5] = title_color[2];
  }

  pm->LookupRGBKey("InactiveTitleBarColor1", title_color[6], title_color[7],
		   title_color[8]);
  if(!pm->LookupRGBKey("InactiveTitleBarColor2", title_color[9],
		       title_color[10], title_color[11]))
  {
    title_color[9]  = title_color[6];
    title_color[10] = title_color[7];
    title_color[11] = title_color[8];
  }

  // setup title bar height and offset
  int tw = 0, th = 0;

  // Get the size of the text
  WindowManager ()->GetPrefMgr ()->GetDefaultFont ()->GetMaxSize (tw, th);
  // Get a good offset
  title_offset = th >> 1;
  // Increase the textheight just a bit to have more room in the title bar
  th += title_offset;
  // Set the height of the title bar
  title_bar_height = MAX(th + 3, title_bar_height);

  // register triggers with our sink
  sink = new awsSink(WindowManager()->GetStringTable());
  sink->SetParm (this);

  sink->RegisterTrigger("Close", &OnCloseClick);
  sink->RegisterTrigger("Zoom", &OnZoomClick);
  sink->RegisterTrigger("Min", &OnMinClick);

  // setup the control buttons
  iString* close_button_txt;
  pm->LookupStringKey("WindowClose", close_button_txt);
  
  csRect closep(18, 5, 18 + 13, 5 + 11);
  pm->LookupRectKey ("WindowCloseAt", closep);
  closep.xmin = Frame ().xmax - closep.xmin;
  closep.xmax = Frame ().xmax - closep.xmax;
  closep.ymin = Frame ().ymin + closep.ymin;
  closep.ymax = Frame ().ymin + closep.ymax;
  
  iAwsKeyFactory* closeinfo = pm->CreateKeyFactory();
  closeinfo->Initialize("Close Button", "Command Button");
  closeinfo->AddIntKey("Style", awsCmdButton::fsNormal);
  closeinfo->AddIntKey("IconAlign", awsCmdButton::iconLeft);
  closeinfo->AddStringKey("Icon", close_button_txt->GetData());

  close_button.SetFlag(AWSF_CMP_NON_CLIENT);
  close_button.Create(WindowManager(), this, closeinfo->GetThisNode());
  close_button.ResizeTo(closep);
  closeinfo->DecRef();

  slot_close.Connect(&close_button, awsCmdButton::signalClicked, 
    sink, sink->GetTriggerID("Close"));
  
  iString* zoom_button_txt;
  pm->LookupStringKey("WindowZoom", zoom_button_txt);
  
  csRect zoomp(34, 5, 34 + 13, 5 + 11);
  pm->LookupRectKey ("WindowZoomAt", zoomp);
  zoomp.xmin = Frame ().xmax - zoomp.xmin;
  zoomp.xmax = Frame ().xmax - zoomp.xmax;
  zoomp.ymin = Frame ().ymin + zoomp.ymin;
  zoomp.ymax = Frame ().ymin + zoomp.ymax;
  
  iAwsKeyFactory* zoominfo = pm->CreateKeyFactory();
  zoominfo->Initialize("Zoom Button", "Command Button");
  zoominfo->AddIntKey("Style", awsCmdButton::fsNormal);
  zoominfo->AddIntKey("IconAlign", awsCmdButton::iconLeft);
  zoominfo->AddStringKey("Icon", zoom_button_txt->GetData());

  zoom_button.SetFlag(AWSF_CMP_NON_CLIENT);
  zoom_button.Create(WindowManager(), this, zoominfo->GetThisNode());
  zoom_button.ResizeTo(zoomp);
  zoominfo->DecRef();

  slot_zoom.Connect(&zoom_button, awsCmdButton::signalClicked, 
    sink, sink->GetTriggerID("Zoom"));

  iString* min_button_txt;
  pm->LookupStringKey("WindowMin", min_button_txt);
  
  csRect minp(50, 5, 50 + 13, 5 + 11);
  pm->LookupRectKey ("WindowMinAt", minp);
  minp.xmin = Frame ().xmax - minp.xmin;
  minp.xmax = Frame ().xmax - minp.xmax;
  minp.ymin = Frame ().ymin + minp.ymin;
  minp.ymax = Frame ().ymin + minp.ymax;
  
  iAwsKeyFactory* mininfo = pm->CreateKeyFactory();
  mininfo->Initialize("Min Button", "Command Button");
  mininfo->AddIntKey("Style", awsCmdButton::fsNormal);
  mininfo->AddIntKey("IconAlign", awsCmdButton::iconLeft);
  mininfo->AddStringKey("Icon", min_button_txt->GetData());

  min_button.SetFlag(AWSF_CMP_NON_CLIENT);
  min_button.Create(WindowManager(), this, mininfo->GetThisNode());
  mininfo->DecRef();
  min_button.ResizeTo(minp);

  slot_min.Connect(&min_button, awsCmdButton::signalClicked, 
    sink, sink->GetTriggerID("Min"));
  
  // Hide any of the undesired controls
  if ((~frame_options & foClose) != 0)
    close_button.Hide();
  
  if ((~frame_options & foZoom) != 0)
    zoom_button.Hide();

  if ((~frame_options & foMin) != 0)
    min_button.Hide();

  return true;
}

bool awsWindow::GetProperty (const char *name, void **parm)
{
  if (awsComponent::GetProperty (name, parm))
    return true;

  if (strcmp ("Title", name) == 0)
  {
    const char *st = 0;
    if (title)
      st = title->GetData ();
    iString *s = new scfString (st);
    *parm = (void *)s;
    return true;
  }
  else if( strcmp("Active", name) == 0)
  {
    *parm = (void*) IsActiveWindow();
    return true;
  }
  else if( strcmp("PopupMenu", name) == 0)
  {
    *parm = popup;
    return true;
  }
  else if( strcmp("Menu", name) == 0)
  {
    *parm = menu;
    return true;
  }

  return false;
}

bool awsWindow::SetProperty (const char *name, void *parm)
{
  if (awsComponent::SetProperty (name, parm))
    return true;

  if (strcmp ("Title", name) == 0)
  {
    iString *t = (iString *) (parm);
    if (t)
    {
      title->DecRef ();
      title = new scfString (t->GetData ());
      Invalidate ();
    }
    return true;
  }
  else if (strcmp("PopupMenu", name) == 0)
  {
    if (popup) popup->DecRef();
    popup = (awsPopupMenu*) parm;
    if (popup) popup->IncRef();
    return true;
  }
  else if (strcmp("Menu", name) == 0)
  {
    SetMenu((awsMenuBar*)parm);
    return true;
  }

  return false;
}

bool awsWindow::Execute (const char *action, iAwsParmList* parmlist)
{
  return awsComponent::Execute (action, parmlist);
}

void awsWindow::Show ()
{
  awsPanel::Show ();

  // Focusing last focused window child
  iAwsComponent *comp = GetFocusedChild ();

  if (!comp)
    comp = GetFirstFocusableChild(this);

  if (comp)
  {
    WindowManager ()->SetFocusedComponent (comp);
    comp->SetFocus();
  }

  Broadcast (sWindowShown);
}

void awsWindow::Hide ()
{
  awsPanel::Hide ();
  // Save last focused component
  SetFocusedChild (WindowManager ()->GetFocusedComponent());
  Broadcast (sWindowHidden);
}

bool awsWindow::IsActiveWindow()
{
  // check to see if there are any sibbling windows above this one
  iAwsComponent* cmp = ComponentAbove();
  while (cmp)
  {
    if (cmp->Flags() & AWSF_CMP_WINDOW)
      return false;
    cmp = cmp->ComponentAbove();
  }

  // check to see if our Parent window is active, if we have one
  if (Parent())
  {
    bool active = false;
    Parent()->Window()->GetProperty("Active", (void**) &active);
    return active;
  }
  return true;
}


void awsWindow::SetMenu(awsMenuBar* _menu)
{
  if (menu)
  {
    menu->DecRef();
    RemoveChild(menu);
    Invalidate();
  }
  menu = _menu;
  if (menu)
  {
    menu->IncRef();
    AddChild(menu);
    menu->SetFlag(AWSF_CMP_NON_CLIENT);

    csRect insets = frame_drawer.GetInsets(style);
    if (frame_options & foTitle)
      insets.ymin += title_bar_height;

    menu->MoveTo(Frame().xmin + insets.xmin, Frame().ymin + insets.ymin);
    menu->Resize(Frame().Width() - insets.xmin - insets.xmax,
		 menu->Frame().Height());
    menu->Show();

    Invalidate();
  }
}

awsMenuBar* awsWindow::GetMenu() { return menu; }



void awsWindow::OnRaise ()
{
  iAwsComponent *comp = GetFocusedChild ();
  if (comp)
  {
    WindowManager ()->SetFocusedComponent (comp);
    comp->SetFocus();
  }
  Broadcast (sWindowRaised);
}

void awsWindow::OnLower ()
{
  SetFocusedChild(WindowManager()->GetFocusedComponent());
  Broadcast (sWindowLowered);
}

bool awsWindow::OnMouseDown (int button, int x, int y)
{
  if (button == 2 && popup)
  {
    popup->MoveTo(x,y);
    popup->Show();
    popup->Raise();
    popup->TrackMouse();
  }

  if (style == fsBitmap || style == fsNone || style == fsFlat)
    return false;

  if (IsMaximized())
    return false;

  down_x = x;
  down_y = y;

  // Check for resizing
  if ((frame_options & foGrip) &&
      x < Frame ().xmax &&
      x > Frame ().xmax - grip_size &&
      y < Frame ().ymax &&
      y > Frame ().ymax - grip_size)
  {
    orig_x = Frame().Width();
    orig_y = Frame().Height();
    resizing_mode = true;
    WindowManager ()->CaptureMouse (this);
    return true;
  }

  // Check for moving
  else if (!(frame_options & foNoDrag)
	&& (
          // Move using titlebar if it's a normal window
            (
              (style == fsNormal && !(frame_options & foBeveledBorder)) &&
              (
                x < Frame ().xmax &&
                x > Frame ().xmin &&
                y < Frame ().ymin +
                title_bar_height &&
                y > Frame ().ymin
              )
            ) ||
          // Move using whole window frame if it's not
            (style != fsNormal || (frame_options & foBeveledBorder)))
	)
  {
    orig_x = Frame().xmin;
    orig_y = Frame().ymin;
    moving_mode = true;
    WindowManager ()->CaptureMouse (this);
    return true;
  }
  return false;
}

bool awsWindow::OnMouseUp (int /* button */, int, int)
{
  if (resizing_mode || moving_mode)
  {
    resizing_mode = false;
    moving_mode = false;
    WindowManager ()->ReleaseMouse ();
    return true;
  }
  return false;
}

bool awsWindow::OnMouseMove (int button, int x, int y)
{
  awsComponent::OnMouseMove(button, x, y);
  (void)button;

  if (resizing_mode)
    Resize(orig_x + x - down_x, orig_y + y - down_y); 
  else if (moving_mode)
    MoveTo(orig_x + x - down_x, orig_y + y - down_y);

  // If we are set to not capture mouse movements return false here. 
  return (frame_options & foDontCaptureMouseMove) == 0;
}

void awsWindow::OnDraw (csRect clip)
{
  iGraphics2D *g2d = WindowManager ()->G2D ();

  awsPanel::OnDraw(clip);
  if (style == fsNormal && !(frame_options & foNoBorder))
  {
    csRect r = Frame();
    csRect insets = frame_drawer.GetInsets(fsNormal);
    r.xmin += insets.xmin;
    r.ymin += insets.ymin 
      + (frame_options & foTitle ? title_bar_height : 0)
      + (menu ? menu->Frame().Height() : 0);
    r.xmax -= insets.xmax;
    r.ymax -= insets.ymax;
    csRectRegion reg;
    reg.makeEmpty();
    if (!r.IsEmpty())
      frame_drawer.Draw(r, fsSunken, Frame(), Frame(), &reg);
  }

  if ((frame_options & foTitle) != 0)
  {
    const int step = 6;
    
    if (style != fsBitmap)
    {
      csRect title_frame = Frame();
      csRect insets = frame_drawer.GetInsets(style);
      title_frame.xmin += insets.xmin;
      title_frame.ymin += insets.ymin;
      title_frame.xmax -= insets.xmax;
      title_frame.ymax = title_frame.ymin + title_bar_height;
      
      // if the title is active
      if (IsActiveWindow())
      {
        DrawGradient(title_frame,
		     title_color[0],
		     title_color[1],
		     title_color[2],
		     title_color[3],
		     title_color[4],
		     title_color[5]);
      }
      else
      {
        DrawGradient(title_frame,
		     title_color[6],
		     title_color[7],
		     title_color[8],
		     title_color[9],
		     title_color[10],
		     title_color[11]);
      }
    }

    if (title)
    {
      // find how far to the right can we write the title bar
      int right_border = ClientFrame().xmax;
      if (frame_options & foMin)
        right_border = MIN(min_button.Frame().xmin, right_border);
      if (frame_options & foZoom)
        right_border = MIN(zoom_button.Frame().xmin, right_border);
      if (frame_options & foClose)
        right_border = MIN(close_button.Frame().xmin, right_border);
      
      int mcc = WindowManager ()->GetPrefMgr ()->GetDefaultFont ()->
        GetLength (title->GetData (), right_border - ClientFrame().xmin - 10);
      
      scfString tmp (title->GetData ());
      tmp.Truncate (mcc);
      if(mcc < (int) title->Length())
      {
        // set the last 3 characters to ...
        for(unsigned int i = MAX(0, (int)tmp.Length()-3); i<tmp.Length(); i++)
          tmp.SetAt(i, '.');
      }
      
      // now draw the title
      g2d->Write (
        WindowManager ()->GetPrefMgr ()->GetDefaultFont (),
        ClientFrame ().xmin + 5,
        Frame ().ymin + (step >> 1) + title_offset,
        title_text_color,
        -1,
        tmp.GetData ());
    }
  }   // end if title bar
}

void awsWindow::DrawGradient(csRect frame,
			     unsigned char r1,
			     unsigned char g1,
			     unsigned char b1,
			     unsigned char r2,
			     unsigned char g2,
			     unsigned char b2)
{
  iGraphics2D *g2d = WindowManager()->G2D();
  iAwsPrefManager *pm = WindowManager()->GetPrefMgr();

  float r_step, g_step, b_step;
  r_step = (float)(r2 - r1) / frame.Width();
  g_step = (float)(g2 - g1) / frame.Width();
  b_step = (float)(b2 - b1) / frame.Width();

  for (int i = 0; i < frame.Width(); i++)
  {
    int color = pm->FindColor(r1 + (unsigned char)(i*r_step),
			      g1 + (unsigned char)(i*g_step),
			      b1 + (unsigned char)(i*b_step));
    g2d->DrawLine(frame.xmin + i, frame.ymin, frame.xmin + i, frame.ymax,
		  color);
  }
}

void awsWindow::SetOptions(int o)
{
  frame_options = o;
}

void awsWindow::Resize(int width, int height)
{
  // don't shrink too far
  csRect insets = frame_drawer.GetInsets(style);
  if (frame_options & foTitle) insets.ymin += title_bar_height;
  int min_height = insets.ymin + insets.ymax;
  int right_border = ClientFrame().xmax;
  if (frame_options & foMin)
    right_border = MIN(min_button.Frame().xmin, right_border);
  if (frame_options & foZoom)
    right_border = MIN(zoom_button.Frame().xmin, right_border);
  if (frame_options & foClose)
    right_border = MIN(close_button.Frame().xmin, right_border);
  int min_width = (Frame().xmax - right_border) + insets.xmin; 

  width = MAX(width, min_width);
  height = MAX(height, min_height);

  int delta_x = width - Frame().Width();
  min_button.Move(delta_x, 0);
  zoom_button.Move(delta_x, 0);
  close_button.Move(delta_x, 0);

  // resize the menu bar
  if (menu)
  {
    insets = frame_drawer.GetInsets(style);
    menu->SizeToFitVert();
    menu->Resize(width - insets.xmin - insets.xmax, 
      MIN(menu->Frame().Height(), height - min_height));
  }

  // change size
  awsComponent::Resize(width, height);
}

iAwsComponent *awsWindow::GetFocusedChild ()
{
  return window_focused_child;
}

void awsWindow::SetFocusedChild (iAwsComponent *comp)
{
  window_focused_child = comp;
}

csRect awsWindow::getMinimumSize ()
{
  return csRect(0,0, Frame().Width(), Frame().Height());
}

csRect awsWindow::getInsets ()
{
  csRect r = awsPanel::getInsets();
  if (frame_options & foTitle)
    r.ymin += title_bar_height;
  if (menu)
    r.ymin += menu->Frame().Height();
  if (style == fsNormal && !(frame_options & foNoBorder))
  {
    csRect more_insets = frame_drawer.GetInsets(fsSunken);
    r.xmin += more_insets.xmin;
    r.ymin += more_insets.ymin;
    r.xmax += more_insets.xmax;
    r.ymax += more_insets.ymax;
  }
  return r;
}

bool awsWindow::IsMoving()
{
  return moving_mode;
}

void awsWindow::OnCloseClick(void *p, iAwsSource *)
{
  ((iAwsComponent*)p)->Broadcast(sWindowClosed);
}

void awsWindow::OnZoomClick(void *p, iAwsSource *)
{
  iAwsComponent* comp = (iAwsComponent*)p;
  if(comp->IsMaximized())
    comp->UnMaximize();
  else
    comp->Maximize();
}

void awsWindow::OnMinClick(void *p, iAwsSource *)
{
  ((iAwsComponent*)p)->Broadcast(sWindowMinimized);
}


/* ---------------------------- Window Factory ----------------------------- */

awsWindowFactory::awsWindowFactory(iAws* wmgr) :
awsComponentFactory(wmgr)
{
  Register ("Window");
  // for back compatibilty we also register under "Default"
  Register("Default");

  iAwsPrefManager *pm = wmgr->GetPrefMgr ();
  pm->RegisterConstant ("signalWindowRaised", awsWindow::sWindowRaised);
  pm->RegisterConstant ("signalWindowLowered", awsWindow::sWindowLowered);
  pm->RegisterConstant ("signalWindowShown", awsWindow::sWindowShown);
  pm->RegisterConstant ("signalWindowHidden", awsWindow::sWindowHidden);
  pm->RegisterConstant ("signalWindowClosed", awsWindow::sWindowClosed);
  pm->RegisterConstant ("signalWindowZoomed", awsWindow::sWindowZoomed);
  pm->RegisterConstant ("signalWindowMinimized", awsWindow::sWindowMinimized);

  RegisterConstant ("wfsNormal", awsWindow::fsNormal);
  RegisterConstant ("wfsBitmap", awsWindow::fsBitmap);
  RegisterConstant ("wfsNone", awsWindow::fsNone);
  RegisterConstant ("wfoControl", awsWindow::foControl);
  RegisterConstant ("wfoZoom", awsWindow::foZoom);
  RegisterConstant ("wfoMin", awsWindow::foMin);
  RegisterConstant ("wfoClose", awsWindow::foClose);
  RegisterConstant ("wfoTitle", awsWindow::foTitle);
  RegisterConstant ("wfoGrip", awsWindow::foGrip);
  RegisterConstant ("wfoNoDrag", awsWindow::foNoDrag);
  RegisterConstant ("wfoRoundBorder", awsWindow::foRoundBorder);
  RegisterConstant ("wfoBeveledBorder", awsWindow::foBeveledBorder);
  RegisterConstant ("wfoNoBorder", awsWindow::foNoBorder);
  RegisterConstant ("wfoDontCaptureMouseMove",
		    awsWindow::foDontCaptureMouseMove);
}

awsWindowFactory::~awsWindowFactory()
{
}

iAwsComponent* awsWindowFactory::Create()
{
  return (new awsWindow())->GetComponent();
}
