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
#include "igraph3d.h"
#include "isystem.h"
#include "csutil/csrect.h"
#include "csutil/scf.h"
#include "cscon.h"

csConsole::csConsole(iBase *base)
{
  CONSTRUCT_IBASE(base);
}

csConsole::~csConsole()
{
  //if(piSystem)
  //piSystem->DecRef();
  if(piG2D)
    piG2D->DecRef();
  if(piG3D)
    piG3D->DecRef();
}

bool csConsole::Initialize(iSystem *system)
{
  piSystem = system;
  piG2D = QUERY_PLUGIN(piSystem, iGraphics2D);
  piG3D = QUERY_PLUGIN(piSystem, iGraphics3D);
  active = false;
  buffer = NULL;
  // Set the maximum backbuffer (4096 lines)
  SetBufferSize(4096);
  Clear();
  // Initialize ourselves with the system and return true if all is well
  return ((piG2D != NULL) && (piG3D != NULL) && (piSystem->RegisterDriver("iConsole", this)));
}

void csConsole::Show()
{
  active = true;
}

void csConsole::Hide()
{
  active = false;
  piG3D->BeginDraw(CSDRAW_2DGRAPHICS);
  piG2D->ClearAll(0);
  piG3D->FinishDraw();
  piG3D->Print(NULL);
}

bool csConsole::IsActive()
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
    piG3D->BeginDraw(CSDRAW_2DGRAPHICS);
    piG2D->Clear(0xFF);
    i=topline;
    pos = 0;
    while (i!=line) {
      // Prevent blank lines from killing us
      if(buffer[i]&&(!buffer[i]->IsEmpty()))
	piG2D->Write(1, pos * piG2D->GetTextHeight(piG2D->GetFontID()), -1, -1, buffer[i]->GetData());
      i++;
      pos++;
      if(i>=maxlines)
	i=0;
    }
    // Prevent blank lines from killing us
    if(buffer[line]&&(!buffer[line]->IsEmpty()))
      piG2D->Write(1, pos * piG2D->GetTextHeight(piG2D->GetFontID()), -1, -1, buffer[line]->GetData());
    piG3D->FinishDraw();
    piG3D->Print(NULL);
  }
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
