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

#define SYSDEF_ALLOCA
#include "cssysdef.h"
#include "csws/cswssys.h"
#include "csws/csapp.h"
#include "itxtmgr.h"

// We need the Virtual File System plugin
REGISTER_STATIC_CLASS (csVFS, "crystalspace.kernel.vfs",
  "Crystal Space Virtual File System plug-in")

// -------------------------------------------------- cswsSystemDriver class ---

cswsSystemDriver::cswsSystemDriver (csApp *ParentApp) :
  SysSystemDriver ()
{
  textline = NULL;
  application = ParentApp;
}

cswsSystemDriver::~cswsSystemDriver ()
{
  delete [] linecolor;
  if (textline)
    for (int i = 0; i < maxlines; i++)
      delete [] textline [i];
  delete [] textline;
  application = NULL;
}

bool cswsSystemDriver::Initialize (int argc, const char* const argv[],
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

  textline = new char *[maxlines];
  linecolor = new int [maxlines];
  for (int i = 0; i < maxlines; i++)
  {
    textline [i] = new char [maxwidth + 1];
    memset (textline [i], 0, maxwidth + 1);
    linecolor [i] = cs_Color_Black;
  } /* endfor */

  // Open the visual, initialize keyboard and mouse
  if (!Open (application->GetText ()))
    return false;

  // For GUI apps double buffering is a serious performance hit
  System->G2D->DoubleBuffer (false);

  // Initialize palette etc (no textures at this time anyway)
  application->PrepareTextures ();

  return true;
};

/*
 * The routine to update the screen in demo mode.
 */
void cswsSystemDriver::DemoWrite (const char* buf)
{
  const char *crpos;
  while ((crpos = strchr (buf, '\n')))
  {
    size_t sl = crpos - buf;
    size_t rl = maxwidth - strlen (textline [curline]);
    if (sl > rl) sl = rl;
    if (sl > 0)
    {
      rl = strlen (textline [curline]);
      memcpy (textline [curline] + rl, buf, sl);
      textline [curline][rl + sl] = 0;
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
  RefreshConsole ();
}

void cswsSystemDriver::RefreshConsole ()
{
  if (!G2D)
    return;

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
  application->StartFrame (elapsed_time, current_time);
  SysSystemDriver::NextFrame (elapsed_time, current_time);
  application->FinishFrame ();
}

bool cswsSystemDriver::ProcessEvents ()
{
  return application->ProcessEvents ();
}
