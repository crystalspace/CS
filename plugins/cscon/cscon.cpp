/*
    Copyright (C) 2000 by Michael Dale Long

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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "sysdef.h"
#include "iconsole.h"
#include "igraph2d.h"
#include "isystem.h"
#include "itxtmgr.h"
#include "csutil/csrect.h"
#include "csutil/scf.h"
#include "cscon.h"

csConsole::csConsole(iBase *base)
{
  CONSTRUCT_IBASE(base);
  fg_rgb.red = fg_rgb.green = fg_rgb.blue = 255;    // Foreground defaults to white
  bg_rgb.red = bg_rgb.green = bg_rgb.blue = 0;    // Background defaults to black
  bg_alpha = 0; // Background defaults to no transparency
}

csConsole::~csConsole()
{
  //if(piSystem)
  //piSystem->DecRef();
  if(piG2D)
    piG2D->DecRef();
}

bool csConsole::Initialize(iSystem *system)
{
  piSystem = system;
  piG2D = QUERY_PLUGIN(piSystem, iGraphics2D);
  active = false;
  buffer = NULL;
  // Set the maximum backbuffer (4096 lines)
  SetBufferSize(4096);
  Clear();
  // Initialize ourselves with the system and return true if all is well
  return ((piG2D != NULL) && (piSystem->RegisterDriver("iConsole", this)));
}

void csConsole::Show()
{
  active = true;
}

void csConsole::Hide()
{
  active = false;
  piG2D->ClearAll(0);
}

bool csConsole::IsActive() const
{
  return active;
}

void csConsole::Clear()
{
  int i;

  for(i = 0; i<maxlines; i++) {
    if(buffer[i]) {
      delete buffer[i];
      buffer[i] = NULL;
    }
  }

  // Reset the line and top line
  line = topline = 0;
}

void csConsole::SetBufferSize(int lines)
{
  if(buffer) {
    Clear();
    delete buffer;
  }

  // Set the new maximum size
  maxlines = lines;
  buffer = new csString*[maxlines];
  Clear();
}

void csConsole::IncLine()
{
  line++;
  if(line>=maxlines)
    line = 0;

  // Clear the current line
  if(buffer[line]) {
    delete buffer[line];
    buffer[line] = NULL;
  }

  if(line==topline) {
    topline++;
    if(topline>=maxlines)
      topline = 0;
  }
}

void csConsole::PutText(const char *text)
{
  // Add string to buffer
  int i;

  for(i = 0; text[i]!=0; i++) {
    switch(text[i]) {
    case '\n':
      IncLine();
      break;
    default:
      if(!buffer[line])
	buffer[line] = new csString();
      buffer[line]->Append(text[i]);
      break;
    }
  }
}

void csConsole::Draw(csRect * = NULL)
{
  int i, pos;

  // Display the console, if active
  if(active) {
    piG2D->BeginDraw();
    piG2D->Clear(bg);
    i=topline;
    pos = 0;
    while (i!=line) {
      // Prevent blank lines from killing us
      if(buffer[i]&&(!buffer[i]->IsEmpty()))
	piG2D->Write(1, pos * piG2D->GetTextHeight(piG2D->GetFontID()), fg, -1, buffer[i]->GetData());
      i++;
      pos++;
      if(i>=maxlines)
	i=0;
    }
    // Prevent blank lines from killing us
    if(buffer[line]&&(!buffer[line]->IsEmpty()))
      piG2D->Write(1, pos * piG2D->GetTextHeight(piG2D->GetFontID()), fg, -1, buffer[line]->GetData());
    piG2D->FinishDraw();
    piG2D->Print(NULL);
  }
}

void csConsole::CacheColors(iTextureManager* txtmgr)
{
  fg = txtmgr->FindRGB(fg_rgb.red, fg_rgb.green, fg_rgb.blue);
  bg = txtmgr->FindRGB(bg_rgb.red, bg_rgb.green, bg_rgb.blue);
}

void csConsole::GetForeground(int &red, int &green, int &blue) const
{
  red = fg_rgb.red; green = fg_rgb.green; blue = fg_rgb.blue;
}

void csConsole::SetForeground(int red, int green, int blue)
{
  fg_rgb.red = red; fg_rgb.green = green; fg_rgb.blue = blue;
}

void csConsole::GetBackground(int &red, int &green, int &blue, int &alpha) const
{
  red = bg_rgb.red; green = bg_rgb.green; blue = bg_rgb.blue;
  alpha = bg_alpha;
}

void csConsole::SetBackground(int red, int green, int blue, int alpha = 0)
{
  bg_rgb.red = red; bg_rgb.green = green; bg_rgb.blue = blue;
  bg_alpha = alpha;
}

IMPLEMENT_IBASE(csConsole)
  IMPLEMENTS_INTERFACE(iConsole)
  IMPLEMENTS_INTERFACE(iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY(csConsole)

EXPORT_CLASS_TABLE (cscon)
  EXPORT_CLASS (csConsole, "crystalspace.console.standard",
		"Standard Console Plugin")
EXPORT_CLASS_TABLE_END
