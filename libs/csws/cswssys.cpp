/*
    Crystal Space Windowing System: Windowing System client system driver
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by Andrew Zabolotny <bit@eltech.ru>

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
#include "itxtmgr.h"

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
  if (System && ((appSystemDriver *)System)->application)
    CHKB (delete ((appSystemDriver *)System)->application);
  System = NULL;
}

// -------------------------------------------------- appSystemDriver class ---

appSystemDriver::appSystemDriver (csApp *ParentApp, csWorld *ParentWorld) :
  SysSystemDriver ()
{
  textline = NULL;
  application = ParentApp;
  world = ParentWorld;
}

appSystemDriver::~appSystemDriver ()
{
  application = NULL;
  world = NULL;
}

bool appSystemDriver::Initialize (int argc, char *argv[],
  const char *iConfigName, const char *iVfsConfigName, IConfig *config)
{
  if (!SysSystemDriver::Initialize (argc, argv, iConfigName, iVfsConfigName, config))
    return false;

  int Width, Height;
  piGI->GetWidth (Width);
  piGI->GetHeight (Height);

  // Initialize console data
  curline = 0;
  textcolor = cs_Color_White;
  maxwidth = Width / 4;
  maxlines = Height / (((csComponent *) application)->TextHeight () + 2);
  CHK (textline = new char *[maxlines]);
  CHK (linecolor = new int [maxlines]);
  for (int i = 0; i < maxlines; i++)
  {
    CHK (textline [i] = new char [maxwidth + 1]);
    memset (textline [i], 0, maxwidth + 1);
    linecolor [i] = cs_Color_Black;
  } /* endfor */

  // Now open temporary console
  world->Initialize (GetISystemFromSystem (System), piG3D, Config, Vfs);

  // Open the visual, initialize keyboard and mouse
  Open (application->GetText ());

  // Setup application palette etc
  application->SetWorld (world);

  // Can only set up the palette after SetWorld() has been called (above),
  // since it calls csWorld::Prepare() which blasts any existing palette.
  // Unfortunately, Prepare() leaves the palette in such a state that we can
  // not add any additional palette entries, but must instead re-initialize
  // it.  Fortunately re-initializing it is harmless at this point in time
  // during application start-up since there was no useful information in the
  // palette anyhow.

  // Create a uniform palette: r(3)g(3)b(2)
  ITextureManager* txtmgr;
  piG3D->GetTextureManager (&txtmgr);
  txtmgr->Initialize ();

  int r,g,b;
  for (r = 0; r < 8; r++)
    for (g = 0; g < 8; g++)
      for (b = 0; b < 4; b++)
        txtmgr->ReserveColor (r * 32, g * 32, b * 64);

  txtmgr->Prepare ();
  txtmgr->AllocPalette ();
  application->SetupPalette ();

  atexit (cleanup);
  return true;
};

void appSystemDriver::CloseConsole ()
{
  // Compute and set the work palette (instead of console palette)
  application->SetWorld (world);
}

/*
 * The routine to update the screen in demo mode.
 */
void appSystemDriver::DemoWrite (const char* buf)
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

  if (piG2D)
  {
    if (SUCCEEDED (piG2D->BeginDraw ()))
    {
      application->Invalidate(true);
      piG2D->Clear (application->Pal [cs_Color_Gray_D]);
      piG2D->SetFontID (application->GetFont ());
      int fh = ((csComponent *) application)->TextHeight () + 2;
      for (int i = 0; i < maxlines; i++)
        if (*textline [i])
        {
          piG2D->Write (1, 1 + i * fh,
            application->Pal [cs_Color_Black], -1, textline [i]);
          piG2D->Write (0, 0 + i * fh,
            application->Pal [linecolor [i]], -1, textline [i]);
        } /* endfor */
      piG2D->FinishDraw ();
      piG2D->Print (NULL);
    } /* endif */
  } /* endif */
}

void appSystemDriver::Alert (char* msg)
{
  if (DemoReady)
  {
    int oldcolor = textcolor;
    textcolor = cs_Color_Red_L;
    DemoWrite (msg);
    textcolor = oldcolor;
  }
  else
    csSystemDriver::Alert (msg);
}

void appSystemDriver::Warn (char* msg)
{
  if (DemoReady)
  {
    int oldcolor = textcolor;
    textcolor = cs_Color_Yellow;
    DemoWrite (msg);
    textcolor = oldcolor;
  }
  else
    csSystemDriver::Alert (msg);
}

void appSystemDriver::NextFrame (long elapsed_time, long current_time)
{
  SysSystemDriver::NextFrame (elapsed_time, current_time);
  application->NextFrame ();
}
