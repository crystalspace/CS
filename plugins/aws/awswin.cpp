#include "cssysdef.h"
#include "awswin.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/fontserv.h"

#include <stdio.h>

const unsigned long awsWindow::sWindowRaised  = 0x1;
const unsigned long awsWindow::sWindowLowered = 0x2;
const unsigned long awsWindow::sWindowShown   = 0x3;
const unsigned long awsWindow::sWindowHidden  = 0x4;
const unsigned long awsWindow::sWindowClosed  = 0x5;

const unsigned int awsWindow::fsNormal  = 0x0;
const unsigned int awsWindow::fsToolbar = 0x1;
const unsigned int awsWindow::fsBitmap  = 0x2;

const unsigned int awsWindow::foControl =       0x1;
const unsigned int awsWindow::foZoom    =       0x2;
const unsigned int awsWindow::foMin     =       0x4;
const unsigned int awsWindow::foClose   =       0x8;
const unsigned int awsWindow::foTitle   =       0x10;
const unsigned int awsWindow::foGrip    =       0x20;
const unsigned int awsWindow::foRoundBorder   = 0x0;
const unsigned int awsWindow::foBeveledBorder = 0x40;

const int grip_size=16;

// Set to true to get printf info about events, false to disable them.
const bool DEBUG_WINDOW_EVENTS = true;


SCF_IMPLEMENT_IBASE(awsWindow)
  SCF_IMPLEMENTS_INTERFACE(awsComponent)
SCF_IMPLEMENT_IBASE_END

awsWindow::awsWindow():above(NULL), below(NULL), 
  frame_style(fsNormal), 
  frame_options(foControl | foZoom | foMin | foClose | foTitle | foGrip | foRoundBorder),
  resizing_mode(false), moving_mode(false),
  min_button(NULL), max_button(NULL), close_button(NULL)
{

}

awsWindow::~awsWindow()
{

}

bool 
awsWindow::Setup(iAws *_wmgr, awsComponentNode *settings)
{
  if (!awsComponent::Setup(_wmgr, settings)) return false;

  iAwsPrefs *pm = WindowManager()->GetPrefMgr();
  
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

  printf("aws-debug: texture for min_button is: %p\n", min_button); 

  return true;
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
    if (below) below->above = above;

    // If there's someone above us, then set their below to our below
    if (above) above->below = below;
    else
    {
        /*  This means that we're the top level window, and we're going away.  We need to tell the window manager to
         * set the new top window. */

        if (below) WindowManager()->SetTopWindow(below);
    }

}

void 
awsWindow::LinkAbove(awsWindow *win)
{
  if (win) {
     above=win->above;
     win->above=this;

     below=win;
  }
}

void 
awsWindow::LinkBelow(awsWindow *win)
{
  if (win) {
     above=win;
     
     below=win->below;
     win->below=this;
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
      awsWindow *next = below;

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
    printf("mousedown: x=%d, y=%d, fx=%d, fy=%d\n", x,y,Frame().xmax, Frame().ymax);

    if (x<Frame().xmax && x>Frame().xmax-grip_size &&
        y<Frame().ymax && y>Frame().ymax-grip_size)
    {
      resizing_mode=true;
      WindowManager()->CaptureMouse();
      WindowManager()->Mark(Frame());

      if (DEBUG_WINDOW_EVENTS)
        printf("aws-debug: Window resize mode=true\n");

      return true;
    } else if (x<Frame().xmax && x>Frame().xmin &&
               y<Frame().ymin + title_bar_height  && y>Frame().ymin)
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

    if (!marked)
      WindowManager()->Mark(Frame());
  
    WindowManager()->InvalidateUpdateStore();

  } else if (moving_mode)
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

    //if (DEBUG_WINDOW_EVENTS)
        //printf("aws-debug: deltas for move: %d, %d\n", delta_x, delta_y);

    csRect dirty2(Frame());

    // Mark changed pos
    WindowManager()->Mark(dirty1);
    WindowManager()->Mark(dirty2);

    WindowManager()->InvalidateUpdateStore();
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

  int tw, th, toff;

  // Get the size of the text
  WindowManager()->GetPrefMgr()->GetDefaultFont()->GetMaxSize(tw, th);

  // Get a good offset
  toff=th>>1;

  // Increase the textheight just a bit to have more room in the title bar
  th+=toff;

  // Set the height of the title bar
  title_bar_height = th+3;
  
  switch(frame_style)
  {
  case fsNormal:
    // Draw the solid fill (or texture)
    if (frame_options & foBeveledBorder)
    {
      g2d->DrawBox(Frame().xmin+2, Frame().ymin+2, Frame().xmax-Frame().xmin-2, Frame().ymax-Frame().ymin-2, fill);

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
      g2d->DrawBox(Frame().xmin+9, Frame().ymin+9, Frame().xmax-Frame().xmin-9, Frame().ymax-Frame().ymin-9, fill);

      int topleft[10] =  { fill, hi, hi2, fill, fill, fill, lo2, lo, black };
      int botright[10] = { black, lo, lo2, fill, fill, fill, hi2, hi, fill };
      int i, titleback;
      const int step=6;

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

        // now draw the title
        g2d->Write(WindowManager()->GetPrefMgr()->GetDefaultFont(),
                   Frame().xmin+10,
                   Frame().ymin+(step>>1)+toff,
                   WindowManager()->GetPrefMgr()->GetColor(AC_TEXTFORE),
                   -1,
                   "AWS Test Window");

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
      //g3d->DrawPixmap(min_button, Frame().xmin+4, Frame().ymin+4, 16, 16, 0,0, 16, 16, 0);
      
    } 
    break;

  default:
    break;
  }

}
