/*
    Crystal Space Windowing System: Windowing System client system driver
    Copyright (C) 1998,1999 by Andrew Zabolotny <bit@eltech.ru>

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

#include "sysdef.h"
#include "csws/cswssys.h"
#include "csws/csapp.h"
#include "itxtmgr.h"

// We need the Virtual File System plugin
REGISTER_STATIC_CLASS (csVFS, "crystalspace.kernel.vfs",
  "Crystal Space Virtual File System plug-in")

/*
 * Do a large debug dump just before the program
 * exits. This function can be installed as a last signal
 * handler if the program crashes (for systems that support
 * this).
 */
void debug_dump ()
{
}

/*
 * This function is installed with atexit()
 */
void cleanup ()
{
}

// -------------------------------------------------- cswsSystemDriver class ---

cswsSystemDriver::cswsSystemDriver (csApp *ParentApp) :
  SysSystemDriver ()
{
  textline = NULL;
  application = ParentApp;
}

cswsSystemDriver::~cswsSystemDriver ()
{
  CHK (delete [] linecolor);
  if (textline)
    for (int i = 0; i < maxlines; i++)
      CHKB (delete [] textline [i]);
  CHK (delete [] textline);
  application = NULL;
}

bool cswsSystemDriver::Initialize (int argc, char *argv[],
  const char *iConfigName)
{
  if (!SysSystemDriver::Initialize (argc, argv, iConfigName))
    return false;

  int Width = G2D->GetWidth ();
  int Height = G2D->GetHeight ();

  // Initialize console data
  curline = 0;
  textcolor = cs_Color_White;
  maxwidth = Width / 4;
  maxlines = Height / (G2D->GetTextHeight (application->GetFont ()) + 2);

  CHK (textline = new char *[maxlines]);
  CHK (linecolor = new int [maxlines]);
  for (int i = 0; i < maxlines; i++)
  {
    CHK (textline [i] = new char [maxwidth + 1]);
    memset (textline [i], 0, maxwidth + 1);
    linecolor [i] = cs_Color_Black;
  } /* endfor */

  // Open the visual, initialize keyboard and mouse
  Open (application->GetText ());

  // Initialize palette etc (no textures at this time anyway)
  application->PrepareTextures ();

  atexit (cleanup);
  return true;
};

/*
 * The routine to update the screen in demo mode.
 */
void cswsSystemDriver::DemoWrite (const char* buf)
{
  char *crpos;
  while ((crpos = strchr (buf, '\n')))
  {
    *crpos = 0;
    int len = maxwidth - strlen (textline [curline]);
    if (len > 0)
    {
      strncat (textline [curline], buf, len);
      linecolor [curline] = textcolor;
    }
    buf = crpos + 1;
    curline++;
    if (curline >= maxlines)
    {
      for (int i = 1; i < maxlines; i++)
      {
        strcpy (textline [i - 1], textline [i]);
        linecolor [i - 1] = linecolor [i];
      } /* endfor */
      curline--;
    } /* endif */
    strcpy (textline [curline], "");
  } /* endwhile */
  strcpy (textline [curline], buf);
  linecolor [curline] = textcolor;

  if (G2D)
  {
    if (G2D->BeginDraw ())
    {
      application->Invalidate(true);
      G2D->Clear (application->Pal [cs_Color_Gray_D]);
      G2D->SetFontID (application->GetFont ());
      int fh = ((csComponent *) application)->TextHeight () + 2;
      for (int i = 0; i < maxlines; i++)
        if (*textline [i])
        {
          G2D->Write (1, 1 + i * fh,
            application->Pal [cs_Color_Black], -1, textline [i]);
          G2D->Write (0, 0 + i * fh,
            application->Pal [linecolor [i]], -1, textline [i]);
        } /* endfor */
      G2D->FinishDraw ();
      G2D->Print (NULL);
    } /* endif */
  } /* endif */
}

void cswsSystemDriver::Alert (const char *msg)
{
  csSystemDriver::Alert (msg);
  if (ConsoleReady)
  {
    int oldcolor = textcolor;
    textcolor = cs_Color_Red_L;
    DemoWrite (msg);
    textcolor = oldcolor;
  }
}

void cswsSystemDriver::Warn (const char *msg)
{
  csSystemDriver::Warn (msg);
  if (ConsoleReady)
  {
    int oldcolor = textcolor;
    textcolor = cs_Color_Yellow;
    DemoWrite (msg);
    textcolor = oldcolor;
  }
}

void cswsSystemDriver::NextFrame (time_t elapsed_time, time_t current_time)
{
  SysSystemDriver::NextFrame (elapsed_time, current_time);
  application->NextFrame (elapsed_time, current_time);
}

bool cswsSystemDriver::ProcessEvents ()
{
  return application->ProcessEvents ();
}
