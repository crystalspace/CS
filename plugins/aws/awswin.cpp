#include "cssysdef.h"
#include "awswin.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"

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

SCF_IMPLEMENT_IBASE(awsWindow)
  SCF_IMPLEMENTS_INTERFACE(awsComponent)
SCF_IMPLEMENT_IBASE_END

awsWindow::awsWindow():above(NULL), below(NULL), 
  frame_style(fsNormal), 
  frame_options(foControl | foZoom | foMin | foClose | foTitle | foGrip | foRoundBorder)
{

}

awsWindow::~awsWindow()
{

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

        WindowManager()->SetTopWindow(below);
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
      LinkAbove(WindowManager()->GetTopWindow());
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
  return false;
}
    
bool 
awsWindow::OnMouseUp(int button, int x, int y)
{
  return false;
}

bool 
awsWindow::OnMouseMove(int button, int x, int y)
{
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
awsWindow::OnKeypress(int key)
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


  int hi    = WindowManager()->GetPrefMgr()->GetColor(AC_HIGHLIGHT),
      hi2   = WindowManager()->GetPrefMgr()->GetColor(AC_HIGHLIGHT2),
      lo    = WindowManager()->GetPrefMgr()->GetColor(AC_SHADOW),
      lo2   = WindowManager()->GetPrefMgr()->GetColor(AC_SHADOW2),
      fill  = WindowManager()->GetPrefMgr()->GetColor(AC_FILL),
      black = WindowManager()->GetPrefMgr()->GetColor(AC_BLACK);

  
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

      // inner fill
      for(int i=0; i<10; ++i)
      {
        g2d->DrawLine(Frame().xmin+i, Frame().ymin+i, Frame().xmax-i, Frame().ymin+i,  topleft[i]);
        g2d->DrawLine(Frame().xmin+i, Frame().ymin+i, Frame().xmin+i, Frame().ymax-i,  topleft[i]);
        g2d->DrawLine(Frame().xmin+i, Frame().ymax-i, Frame().xmax-i, Frame().ymax-i, botright[i]);
        g2d->DrawLine(Frame().xmax-1, Frame().ymin+i, Frame().xmax-i, Frame().ymax-i, botright[i]);
      }
      
    } 
    

    break;

  default:
    break;
  }

}

