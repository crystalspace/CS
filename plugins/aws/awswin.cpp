#include "cssysdef.h"
#include "awswin.h"

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
OnRaise()
{
   return;
}

void 
OnLower()
{
   return;
}
