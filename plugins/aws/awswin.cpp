#include "cssysdef.h"
#include "awswin.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/fontserv.h"
#include "iutil/event.h"
#include "csutil/scfstr.h"
#include "csutil/snprintf.h"
#include "aws/iawsdefs.h"

#include <stdio.h>
#include <string.h>

const unsigned long awsWindow::sWindowRaised  = 0x1;
const unsigned long awsWindow::sWindowLowered = 0x2;
const unsigned long awsWindow::sWindowShown   = 0x3;
const unsigned long awsWindow::sWindowHidden  = 0x4;
const unsigned long awsWindow::sWindowClosed  = 0x5;

const int awsWindow::fsNormal  = 0x0;
const int awsWindow::fsToolbar = 0x1;
const int awsWindow::fsBitmap  = 0x2;

const int awsWindow::foControl =       0x1;
const int awsWindow::foZoom    =       0x2;
const int awsWindow::foMin     =       0x4;
const int awsWindow::foClose   =       0x8;
const int awsWindow::foTitle   =       0x10;
const int awsWindow::foGrip    =       0x20;
const int awsWindow::foRoundBorder   = 0x0;
const int awsWindow::foBeveledBorder = 0x40;

const int grip_size=16;

// Set to true to get printf info about events, false to disable them.
const bool DEBUG_WINDOW_EVENTS = false;

awsWindow::awsWindow():above(NULL), below(NULL), 
  min_button(NULL), max_button(NULL), close_button(NULL), btxt(NULL),
  frame_style(fsNormal), 
  frame_options(foControl | foZoom | foClose | foTitle | foGrip | foRoundBorder),
  title(NULL),
  resizing_mode(false), moving_mode(false), 
  minp(50,5, 50+13, 5+11), maxp(34,5, 34+13, 5+11), closep(18,5, 18+13,5+11),
  min_down(false), max_down(false), close_down(false),
  is_zoomed(false), is_minimized(false), todraw_dirty(true)
{
  // Window start off hidden.
  SetFlag(AWSF_CMP_HIDDEN);
}

awsWindow::~awsWindow()
{

}

void 
awsWindow::SetWindowAbove(iAwsWindow *win)
{
  above=win;
}

void 
awsWindow::SetWindowBelow(iAwsWindow *win)
{
  below=win;
}

bool 
awsWindow::Setup(iAws *_wmgr, awsComponentNode *settings)
{
  if (!comp.Setup(_wmgr, settings)) return false;

  iAwsPrefManager *pm = WindowManager()->GetPrefMgr();
  
  // Link into the current window hierarchy, at the top.
  if (WindowManager()->GetTopWindow()==NULL)
  {
    WindowManager()->SetTopWindow(this);
  }
  else
  {
    LinkAbove(WindowManager()->GetTopWindow());
    WindowManager()->SetTopWindow(this);
  }
    
  if ((min_button=pm->GetTexture("WindowMin"))==NULL)
    printf("aws-debug: No WindowMin texture found.\n");

  if ((max_button=pm->GetTexture("WindowZoom"))==NULL)
    printf("aws-debug: No WindowZoom texture found.\n");

  if ((close_button=pm->GetTexture("WindowClose"))==NULL)
    printf("aws-debug: No WindowClose texture found.\n");

  btxt=pm->GetTexture("Texture");

  pm->GetInt(settings, "Style", frame_style);
  pm->GetInt(settings, "Options", frame_options);
  pm->GetString(settings, "Title", title);

  pm->LookupRectKey("WindowMinAt", minp);
  pm->LookupRectKey("WindowZoomAt", maxp);
  pm->LookupRectKey("WindowCloseAt", closep);

  // Arrange control rects
  minp.xmin=Frame().xmax-minp.xmin;
  minp.xmax=Frame().xmax-minp.xmax;
  minp.ymin=Frame().ymin+minp.ymin;
  minp.ymax=Frame().ymin+minp.ymax;

  maxp.xmin=Frame().xmax-maxp.xmin;
  maxp.xmax=Frame().xmax-maxp.xmax;
  maxp.ymin=Frame().ymin+maxp.ymin;
  maxp.ymax=Frame().ymin+maxp.ymax;

  closep.xmin=Frame().xmax-closep.xmin;
  closep.xmax=Frame().xmax-closep.xmax;
  closep.ymin=Frame().ymin+closep.ymin;
  closep.ymax=Frame().ymin+closep.ymax;

  return true;
}

bool 
awsWindow::GetProperty(char *name, void **parm)
{
  if (comp.GetProperty(name, parm)) return true;

  if (strcmp("Title", name)==0)
  {
    char *st=NULL;

    if (title) st=title->GetData();

    iString *s = new scfString(st);
    *parm = (void *)s;
    return true;
  }

  return false;
}

bool 
awsWindow::SetProperty(char *name, void *parm)
{  
  if (comp.SetProperty(name, parm)) return true;

  if (strcmp("Title", name)==0)
  {
    iString *t = (iString *)(parm);
    if (t)
    {
      title->DecRef();
      title = new scfString(t->GetData());
      title->IncRef();

      Invalidate();
    }
    return true;
  }
  
  return false;
}

bool 
awsWindow::Execute(char *action, awsParmList &parmlist)
{
  if (comp.Execute(action, parmlist)) return true;

  return false;
}

iAwsComponent *
awsWindow::GetComponent()
{
  return this;
}

void 
awsWindow::SetRedrawTag(unsigned int tag)
{
 redraw_tag=tag;
}

void 
awsWindow::Unlink()
{
    // If there's someone below us, set their above to our above.
    if (below) below->SetWindowAbove(above);

    // If there's someone above us, then set their below to our below
    if (above) above->SetWindowBelow(below);
    else
    {
        /*  This means that we're the top level window, and we're going away.  We need to tell the window manager to
         * set the new top window. */

        if (below) WindowManager()->SetTopWindow(below);
    }

}

void 
awsWindow::LinkAbove(iAwsWindow *win)
{
  if (win) {
     above=win->WindowAbove();
     win->SetWindowAbove(this);

     below=win;
  }
}

void 
awsWindow::LinkBelow(iAwsWindow *win)
{
  if (win) {
     above=win;
     
     below=win->WindowBelow();
     win->SetWindowBelow(this);
  }
}

void 
awsWindow::Raise()
{
   // Only raise if we're not already the top
   if (above != NULL)
   {        
      // Get us out of the window hierarchy
      Unlink();

      // Put us back into the window hierachy at the top.
      if (WindowManager()->GetTopWindow())        
      {
        WindowManager()->Mark(WindowManager()->GetTopWindow()->Frame());
        LinkAbove(WindowManager()->GetTopWindow());
      }
      
      
      WindowManager()->SetTopWindow(this);
      WindowManager()->Mark(Frame());
   }
}

void 
awsWindow::Lower()
{
   // Only lower if we're not already the bottom
   if (below != NULL)
   {
      iAwsWindow *next = below;

      // Get us out of the window hierachy.
      Unlink();

      // Put us back in one level lower.
      LinkBelow(next);

      // If we were the top window, fix it
      if (WindowManager()->GetTopWindow() == this)
        WindowManager()->SetTopWindow(next);
   }
}

void
awsWindow::OnRaise()
{
   Broadcast(sWindowRaised);
   return;
}

void 
awsWindow::OnLower()
{
   Broadcast(sWindowLowered);
   return;
}

bool 
awsWindow::OnMouseDown(int button, int x, int y)
{
  
  if (!(frame_style & fsBitmap))
  {
    if (DEBUG_WINDOW_EVENTS) 
      printf("mousedown: x=%d, y=%d, fx=%d, fy=%d\n", x,y,Frame().xmax, Frame().ymax);

    // check controls first
    if ((frame_options & foMin) && minp.Contains(x, y))
    {
      min_down=true;
      Invalidate();
      return true;
    }

    if ((frame_options & foZoom) && maxp.Contains(x, y))
    {
      max_down=true;
      Invalidate();
      return true;
    }

    if ((frame_options & foClose) && closep.Contains(x, y))
    {
      close_down=true;
      Invalidate();
      return true;
    }

    ////////// NOTE!! Past this point is where all events that are not allowed to happen while zoomed go!!
    if (is_zoomed) return false;

    ///// Check for resizing
    if ((frame_options & foGrip) &&
        x<Frame().xmax && x>Frame().xmax-grip_size &&
        y<Frame().ymax && y>Frame().ymax-grip_size)
    {
      resizing_mode=true;
      WindowManager()->CaptureMouse();
      WindowManager()->Mark(Frame());

      if (DEBUG_WINDOW_EVENTS)
        printf("aws-debug: Window resize mode=true\n");

      return true;
    } 
    ///// Check for moving
    else if    (
                // Move using titlebar if it's a normal window
               ((frame_style==fsNormal && !(frame_options & foBeveledBorder)) && 
               (x<Frame().xmax && x>Frame().xmin && 
                y<Frame().ymin + title_bar_height  && y>Frame().ymin))

                ||

                // Move using whole window frame if it's not
                (frame_style!=fsNormal || (frame_options & foBeveledBorder))                          
               
               )
    {
      moving_mode=true;
      WindowManager()->CaptureMouse();
      last_x=x;
      last_y=y;

      if (DEBUG_WINDOW_EVENTS)
        printf("aws-debug: Window move mode=true\n");

      return true;
    }
  } 
  
  return false;
}
    
bool 
awsWindow::OnMouseUp(int button, int x, int y)
{
  if (max_down && (frame_options & foZoom) && maxp.Contains(x, y))
    {
      max_down=false;

      // Zoom/de-zoom window
      if (is_zoomed)
      {
        int delta_x = unzoomed_frame.xmax - Frame().xmax, delta_y=unzoomed_frame.ymin;
        is_zoomed=false;
        
        // Fix controls
        minp.Move(delta_x, delta_y);
        maxp.Move(delta_x, delta_y);
        closep.Move(delta_x, delta_y);

        // Fix kids
        MoveChildren(unzoomed_frame.xmin, delta_y);

        // Fix frame
        Frame().Set(unzoomed_frame);
      }
      else
      {
        is_zoomed=true;
        unzoomed_frame.Set(Frame());
        
        Frame().xmin=Frame().ymin=0;
        Frame().xmax=WindowManager()->G2D()->GetWidth();
        Frame().ymax=WindowManager()->G2D()->GetHeight();

        int delta_x = Frame().xmax - unzoomed_frame.xmax, delta_y=-unzoomed_frame.ymin;

        // Fix controls
        minp.Move(delta_x, delta_y);
        maxp.Move(delta_x, delta_y);
        closep.Move(delta_x, delta_y);

        // Fix kids
        MoveChildren(-unzoomed_frame.xmin, delta_y);

      }

      Invalidate();
      WindowManager()->InvalidateUpdateStore();
      return true;
    }

  if (min_down || max_down || close_down)
  {
    min_down=false;
    max_down=false;
    close_down=false;
    Invalidate();
    return true;
  }

  if (resizing_mode)
  {
    resizing_mode=false;
    WindowManager()->ReleaseMouse();
    WindowManager()->Mark(Frame());

    if (DEBUG_WINDOW_EVENTS)
        printf("aws-debug: Window resize mode=false\n");

    return true;
  } else if (moving_mode)
  {
    moving_mode=false;
    WindowManager()->ReleaseMouse();

    if (DEBUG_WINDOW_EVENTS)
        printf("aws-debug: Window move mode=false\n");
  }

  return false;
}

bool 
awsWindow::OnMouseMove(int button, int x, int y)
{
  if (resizing_mode)
  {
    bool marked=false;

    if (x<Frame().xmax || y<Frame().ymax)
    {
      WindowManager()->Mark(Frame());
      marked=true;
    }

    int delta_x, old_x = Frame().xmax;

    Frame().xmax=x;
    Frame().ymax=y;
    
    if (Frame().xmax - Frame().xmin < grip_size<<1)
      Frame().xmax = Frame().xmin+(grip_size<<1);

    if (Frame().ymax - Frame().ymin < grip_size<<1)
      Frame().ymax = Frame().ymin+(grip_size<<1);
    
    if (Frame().xmax > WindowManager()->G2D()->GetWidth())
      Frame().xmax = WindowManager()->G2D()->GetWidth();

    if (Frame().ymax > WindowManager()->G2D()->GetHeight())
      Frame().ymax = WindowManager()->G2D()->GetHeight();

    delta_x = Frame().xmax - old_x;

    minp.Move(delta_x,   0);
    maxp.Move(delta_x,   0);
    closep.Move(delta_x, 0);

    if (!marked)
      WindowManager()->Mark(Frame());
  
    WindowManager()->InvalidateUpdateStore();

  } 
  else if (moving_mode)
  {
    int delta_x = x-last_x;
    int delta_y = y-last_y;

    last_x=x;
    last_y=y;

    if (delta_x+Frame().xmin <0)
      delta_x=-Frame().xmin;
    else if (delta_x+Frame().xmax > WindowManager()->G2D()->GetWidth())
      delta_x=WindowManager()->G2D()->GetWidth() - Frame().xmax;

    if (delta_y+Frame().ymin <0)
      delta_y=-Frame().ymin;
    else if (delta_y+Frame().ymax > WindowManager()->G2D()->GetHeight())
      delta_y=WindowManager()->G2D()->GetHeight() - Frame().ymax;
         
    csRect dirty1(Frame());
   
    // Move frame
    Frame().xmin+=delta_x;
    Frame().ymin+=delta_y;
    Frame().xmax+=delta_x;
    Frame().ymax+=delta_y;

    minp.Move(delta_x, delta_y);
    maxp.Move(delta_x, delta_y);
    closep.Move(delta_x, delta_y);

    // Move children
    MoveChildren(delta_x, delta_y);

    //if (DEBUG_WINDOW_EVENTS)
        //printf("aws-debug: deltas for move: %d, %d\n", delta_x, delta_y);

    csRect dirty2(Frame());

    // Mark changed pos
    WindowManager()->Mark(dirty1);
    WindowManager()->Mark(dirty2);

    WindowManager()->InvalidateUpdateStore();
    todraw_dirty=true;
  }
  return false;
}

bool 
awsWindow::HandleEvent(iEvent& Event)
{  
  switch(Event.Type)
  {
  case csevMouseMove:
    return OnMouseMove(Event.Mouse.Button, Event.Mouse.x, Event.Mouse.y);

  case csevMouseUp:
    return OnMouseUp(Event.Mouse.Button, Event.Mouse.x, Event.Mouse.y);

  case csevMouseDown:
    return OnMouseDown(Event.Mouse.Button, Event.Mouse.x, Event.Mouse.y);

  case csevMouseClick:
    return OnMouseClick(Event.Mouse.Button, Event.Mouse.x, Event.Mouse.y);

  case csevMouseEnter:
    return OnMouseEnter();

  case csevMouseExit:
    return OnMouseExit();

  case csevKeyDown:
    return OnKeypress(Event.Key.Char, Event.Key.Modifiers);
    
  }

  return false;

}

bool 
awsWindow::OnMouseExit()
{
  return false;
}

bool 
awsWindow::OnMouseEnter()
{
  return false;
}

bool
awsWindow::OnMouseClick(int button, int x, int y)
{
  return false;
}

bool
awsWindow::OnMouseDoubleClick(int button, int x, int y)
{
  return false;
}

bool 
awsWindow::OnKeypress(int key, int modifiers)
{
  return false;
}

bool 
awsWindow::OnLostFocus()
{
  return false;
}

bool 
awsWindow::OnGainFocus()
{
  return false;
}

void 
awsWindow::OnDraw(csRect clip)
{
  iGraphics2D *g2d = WindowManager()->G2D();
  iGraphics3D *g3d = WindowManager()->G3D();

  /******************************************
   * When drawing the window, we have to take 
   *  certain things into account.  First, the
   *  frame type defines a number of differences.
   *  Second, some normal behaviors may be turned
   *  off, and thus we wouldn't want to draw those.
   *  Finally, the type of window also describes 
   *  what kind of borders and title windows we draw.
   **************************************************/


  int hi    = WindowManager()->GetPrefMgr()->GetColor(AC_HIGHLIGHT);
  int hi2   = WindowManager()->GetPrefMgr()->GetColor(AC_HIGHLIGHT2);
  int lo    = WindowManager()->GetPrefMgr()->GetColor(AC_SHADOW);
  int lo2   = WindowManager()->GetPrefMgr()->GetColor(AC_SHADOW2);
  int fill  = WindowManager()->GetPrefMgr()->GetColor(AC_FILL);
  int black = WindowManager()->GetPrefMgr()->GetColor(AC_BLACK);
//int white = WindowManager()->GetPrefMgr()->GetColor(AC_WHITE);

  int tw, th, toff, btw, bth;
  int i;

  if (todraw_dirty)
  {
    todraw_dirty=false;

    todraw.makeEmpty();
    todraw.Include(Frame());
    
    for(i=0; i<GetChildCount(); ++i)
    {
      if (!(GetChildAt(i)->Flags() & AWSF_CMP_ALWAYSERASE))
      todraw.Exclude(GetChildAt(i)->Frame());
    }
  }

  // Get the size of the text
  WindowManager()->GetPrefMgr()->GetDefaultFont()->GetMaxSize(tw, th);

  // Get a good offset
  toff=th>>1;

  // Increase the textheight just a bit to have more room in the title bar
  th+=toff;

  // Set the height of the title bar
  title_bar_height = th+3;

  // Get the texture size, if there is one.
  if (btxt) btxt->GetOriginalDimensions(btw, bth);
  
  switch(frame_style)
  {
  case fsNormal:
    // Draw the solid fill (or texture)
    if (frame_options & foBeveledBorder)
    {      
      for (i=0; i<todraw.Count(); ++i)
      {
        csRect r(todraw.RectAt(i));
               
        if (btxt==NULL) 
          g2d->DrawBox(r.xmin, r.ymin, r.Width(), r.Height(), fill);
        else
          g3d->DrawPixmap(btxt, r.xmin, r.ymin, r.Width(), r.Height(), 0,0, r.Width(), r.Height(), 0);
      }

      // Draw a beveled border, fill-hi on top and left, black-shadow on bot and right
      g2d->DrawLine(Frame().xmin, Frame().ymin, Frame().xmax, Frame().ymin, fill);
      g2d->DrawLine(Frame().xmin+1, Frame().ymin+1, Frame().xmax-1, Frame().ymin+1, hi);
      
      g2d->DrawLine(Frame().xmin, Frame().ymin+1, Frame().xmin, Frame().ymax, fill);
      g2d->DrawLine(Frame().xmin+1, Frame().ymin+2, Frame().xmin+1, Frame().ymax-1, hi);
      
      g2d->DrawLine(Frame().xmin, Frame().ymax, Frame().xmax, Frame().ymax, black);
      g2d->DrawLine(Frame().xmin+1, Frame().ymax-1, Frame().xmax-1, Frame().ymax-1, lo);
      
      g2d->DrawLine(Frame().xmax, Frame().ymin, Frame().xmax, Frame().ymax-1, black);
      g2d->DrawLine(Frame().xmax-1, Frame().ymin+1, Frame().xmax-1, Frame().ymax-2, lo);
   
    }
    else
    {
      int topleft[10] =  { fill, hi, hi2, fill, fill, fill, lo2, lo, black };
      int botright[10] = { black, lo, lo2, fill, fill, fill, hi2, hi, fill };
      int titleback;
      const int step=6;
     
      for (i=0; i<todraw.Count(); ++i)
      {
        csRect r(todraw.RectAt(i));
               
        if (btxt==NULL) 
          g2d->DrawBox(r.xmin, r.ymin, r.Width(), r.Height(), fill);
        else
          g3d->DrawPixmap(btxt, r.xmin, r.ymin, r.Width(), r.Height(), 0,0, r.Width(), r.Height(), 0);
      }
      
      if (frame_options & foTitle)
      {
        // start with even border
        for(i=0; i<step; ++i)
        {
          g2d->DrawLine(Frame().xmin+i, Frame().ymin+i, Frame().xmax-i, Frame().ymin+i,  topleft[i]);
          g2d->DrawLine(Frame().xmin+i, Frame().ymin+i, Frame().xmin+i, Frame().ymax-i,  topleft[i]);
          g2d->DrawLine(Frame().xmin+i, Frame().ymax-i, Frame().xmax-i, Frame().ymax-i, botright[i]);
          g2d->DrawLine(Frame().xmax-i, Frame().ymin+i, Frame().xmax-i, Frame().ymax-i, botright[i]);
        }

        // now add some more fill for the title bar
        if (WindowManager()->GetTopWindow()==this)  titleback=hi;
        else                                        titleback=fill;
        
        for(i=step; i<step+th-1; ++i)
          g2d->DrawLine(Frame().xmin+step, Frame().ymin+i, Frame().xmax-step+1, Frame().ymin+i,  titleback);
        
          
        // finish with an offset top
        for(i=step; i<9; ++i)
        {
          g2d->DrawLine(Frame().xmin+i, Frame().ymin+i+th-1, Frame().xmax-i, Frame().ymin+i+th-1,  topleft[i]);
          g2d->DrawLine(Frame().xmin+i, Frame().ymin+i+th-1, Frame().xmin+i, Frame().ymax-i,       topleft[i]);
          g2d->DrawLine(Frame().xmin+i, Frame().ymax-i,      Frame().xmax-i, Frame().ymax-i,       botright[i]);
          g2d->DrawLine(Frame().xmax-i, Frame().ymin+i+th-1, Frame().xmax-i, Frame().ymax-i,       botright[i]);
        } 

        if (title)
        {
          int mcc = WindowManager()->GetPrefMgr()->GetDefaultFont()->GetLength(title->GetData(), Frame().Width()-15);

          scfString tmp(title->GetData());
          tmp.Truncate(mcc);
        
          // now draw the title
          g2d->Write(WindowManager()->GetPrefMgr()->GetDefaultFont(),
                     Frame().xmin+10,
                     Frame().ymin+(step>>1)+toff,
                     WindowManager()->GetPrefMgr()->GetColor(AC_TEXTFORE),
                     -1,
                     tmp.GetData());
        }

      } // end if title bar
      else
      {
        // create even border all the way around
        for(i=0; i<9; ++i)
        {
          g2d->DrawLine(Frame().xmin+i, Frame().ymin+i, Frame().xmax-i, Frame().ymin+i,  topleft[i]);
          g2d->DrawLine(Frame().xmin+i, Frame().ymin+i, Frame().xmin+i, Frame().ymax-i,  topleft[i]);
          g2d->DrawLine(Frame().xmin+i, Frame().ymax-i, Frame().xmax-i, Frame().ymax-i, botright[i]);
          g2d->DrawLine(Frame().xmax-i, Frame().ymin+i, Frame().xmax-i, Frame().ymax-i, botright[i]);
        }
      }  // end else not title

      if (frame_options & foGrip)
      {
        int   x,y;
 
        g2d->DrawBox(Frame().xmax-grip_size+2,  Frame().ymax-grip_size+2, grip_size-4, grip_size-4, fill);
        g2d->DrawLine(Frame().xmax-grip_size,   Frame().ymax-grip_size+1, Frame().xmax-7,           Frame().ymax-grip_size+1, hi2);
        g2d->DrawLine(Frame().xmax-grip_size+1, Frame().ymax-grip_size,   Frame().xmax-grip_size+1, Frame().ymax-7, hi2);
        g2d->DrawLine(Frame().xmax-grip_size,   Frame().ymax-grip_size,   Frame().xmax-6,           Frame().ymax-grip_size, hi);
        g2d->DrawLine(Frame().xmax-grip_size,   Frame().ymax-grip_size,   Frame().xmax-grip_size,   Frame().ymax-6, hi);


        for(x=Frame().xmax-grip_size+4; x<Frame().xmax-4; x+=2)
          for(y=Frame().ymax-grip_size+4; y<Frame().ymax-4; y+=2)
          {
            if (resizing_mode)
            {
             g2d->DrawPixel(x+1,y+1,hi2);
             g2d->DrawPixel(x,y,lo2);
            }
            else
            {
             g2d->DrawPixel(x+1,y+1,lo2);
             g2d->DrawPixel(x,y,hi2);
            }
          }

      }

      
      // Draw min/max/close buttons
      int mtw, mth, mxtw, mxth, ctw, cth;

      min_button->GetOriginalDimensions(mtw, mth);
      max_button->GetOriginalDimensions(mxtw, mxth);
      close_button->GetOriginalDimensions(ctw, cth);

      if (frame_options & foMin)
      {
        g3d->DrawPixmap(min_button, minp.xmin+min_down, minp.ymin+min_down, mtw, mth, 0,0, mtw, mth, 0);
        if (min_down) Draw3DRect(g2d, minp, lo2, hi2);
        else          Draw3DRect(g2d, minp, hi2, lo2);
      }

      if (frame_options & foZoom)
      {
        g3d->DrawPixmap(max_button, maxp.xmin+max_down, maxp.ymin+max_down, mxtw, mth, 0,0, mxtw, mxth, 0);
        if (max_down) Draw3DRect(g2d, maxp, lo2, hi2);
        else          Draw3DRect(g2d, maxp, hi2, lo2);
      }

      if (frame_options & foClose)
      {
        g3d->DrawPixmap(close_button, closep.xmin+close_down, closep.ymin+close_down, ctw, cth, 0,0, ctw, cth, 0);
        if (close_down) Draw3DRect(g2d, closep, lo2, hi2);
        else            Draw3DRect(g2d, closep, hi2, lo2);
      }
      
    } 
    break;

  default:
    break;
  }

}

void 
awsWindow::Draw3DRect(iGraphics2D *g2d, csRect &f, int hi, int lo)
{
  g2d->DrawLine(f.xmin, f.ymin, f.xmax, f.ymin, hi);
  g2d->DrawLine(f.xmin, f.ymin, f.xmin, f.ymax, hi);
  g2d->DrawLine(f.xmin, f.ymax, f.xmax, f.ymax, lo);
  g2d->DrawLine(f.xmax, f.ymin, f.xmax, f.ymax, lo);
}
    
void 
awsWindow::Invalidate() 
{ comp.Invalidate(); }

void 
awsWindow::Invalidate(csRect area) 
{ comp.Invalidate(area); }

csRect& 
awsWindow::Frame()
{ return comp.Frame(); }

bool 
awsWindow::Overlaps(csRect &r)
{ return comp.Overlaps(r); }

bool 
awsWindow::isHidden()
{ return comp.isHidden(); }

void 
awsWindow::Hide()
{ 
  comp.Hide(); 
  WindowManager()->InvalidateUpdateStore();
}

void 
awsWindow::Show()
{ 
  comp.Show(); 
  WindowManager()->InvalidateUpdateStore();
}

unsigned 
long 
awsWindow::GetID()
{ return comp.GetID(); }

void 
awsWindow::SetID(unsigned long _id)
{ comp.SetID(_id); }

void 
awsWindow::SetFlag(unsigned int flag)
{ comp.SetFlag(flag); }

void 
awsWindow::ClearFlag(unsigned int flag)
{ comp.ClearFlag(flag); }

unsigned int 
awsWindow::Flags()
{ return comp.Flags(); }

void 
awsWindow::MoveChildren(int delta_x, int delta_y)
{ comp.MoveChildren(delta_x, delta_y); }

void 
awsWindow::AddChild(iAwsComponent* child, bool owner=true)
{ 
  todraw_dirty=true;
  comp.AddChild(child, owner); 
}

void 
awsWindow::RemoveChild(iAwsComponent *child)
{ 
  todraw_dirty=true;
  comp.RemoveChild(child); 
}

int 
awsWindow::GetChildCount()
{ return comp.GetChildCount(); }

iAwsComponent *
awsWindow::GetChildAt(int i)
{ return comp.GetChildAt(i); }

bool 
awsWindow::HasChildren()
{ return comp.HasChildren(); }

iAwsWindow *
awsWindow::Window()
{ return comp.Window(); }
    
iAwsComponent *
awsWindow::Parent()
{ return comp.Parent(); }

void 
awsWindow::SetWindow(iAwsWindow *win)
{ comp.SetWindow(win); }

void 
awsWindow::SetParent(iAwsComponent *parent)
{ comp.SetParent(parent); }

bool 
awsWindow::RegisterSlot(iAwsSlot *slot, unsigned long signal)
{ return comp.RegisterSlot(slot, signal); }

bool 
awsWindow::UnregisterSlot(iAwsSlot *slot, unsigned long signal)
{ return comp.UnregisterSlot(slot, signal); } 

void 
awsWindow::Broadcast(unsigned long signal)
{ comp.Broadcast(signal); }
