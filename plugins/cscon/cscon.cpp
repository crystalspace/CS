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
#include "csutil/csstring.h"
#include "cscon.h"
#include "conbuffr.h"

csConsole::csConsole(iBase *base)
{
  CONSTRUCT_IBASE(base);
  fg_rgb.red = fg_rgb.green = fg_rgb.blue = 255;    // Foreground defaults to white
  bg_rgb.red = bg_rgb.green = bg_rgb.blue = 0;    // Background defaults to black
  transparent = false;  // Default to no transparency
  do_snap = true;      // Default to snapping
}

csConsole::~csConsole()
{
  //@@@ This is disabled due to a bug in some implementations of iSystem
  //if(piSystem)
  //piSystem->DecRef();
  if(piG2D)
    piG2D->DecRef();
  delete buffer;
}

bool csConsole::Initialize(iSystem *system)
{
  piSystem = system;
  piG2D = QUERY_PLUGIN(piSystem, iGraphics2D);
  if(!piG2D)
    return false;
  // Initialize the display rectangle to the entire display
  size.Set(0, 0, piG2D->GetWidth() - 1, piG2D->GetHeight() - 1);
  invalid.Set(size); // Invalidate the entire console
  font = piG2D->GetFontID();
  // Create the backbuffer (4096 lines max)
  buffer = new csConsoleBuffer(4096, (size.Height() / (piG2D->GetTextHeight(font) + 2)));
  // Initialize ourselves with the system and return true if all is well
  return piSystem->RegisterDriver("iConsole", this);
}

void csConsole::Show()
{
  /***DEPRECATED***/
}

void csConsole::Hide()
{
  /***DEPRECATED***/
}

bool csConsole::IsActive() const
{
  /***DEPRECATED***/
  return true;
}

void csConsole::Clear()
{
  buffer->Clear();
}

void csConsole::SetBufferSize(int lines)
{
  buffer->SetLength(lines);
}

void csConsole::PutText(const char *text)
{
  int i;

  // Scan the string for escape characters
  for(i = 0; text[i]!=0; i++) {
    switch(text[i]) {
    case '\n':
      buffer->NewLine(do_snap);
      break;
    default:
      csString *curline = buffer->WriteLine();
      curline->Append(text[i]);
      break;
    }
  }
}

void csConsole::Draw(csRect *area)
{
  int i, height, oldfont;
  csRect line, oldrgn;
  const csString *text;
  bool dirty;

  // Save old clipping region and assign the console clipping region
  piG2D->GetClipRect(oldrgn.xmin, oldrgn.ymin, oldrgn.xmax, oldrgn.ymax);
  piG2D->SetClipRect(size.xmin, size.ymin, size.xmax, size.ymax);

  // Save the old font and set it to the requested font
  oldfont = piG2D->GetFontID();
  piG2D->SetFontID(font);

  // If we're not transparent, clear the background
  if(!transparent)
    piG2D->Clear(bg);

  // Calculate the height of the text
  height = (piG2D->GetTextHeight(font) + 2);

  // Print all lines on the current page
  for (i=0; i<buffer->GetPageSize(); i++) {

    // Retrieve the line from the buffer and it's dirty flag
    text = buffer->GetLine(i, dirty);

    // A NULL line indicates it's the last printed line on the page
    if(text==NULL)
      break;

    // Calculate the rectangle of this line
    line.Set(size.xmin, (i * height) + size.ymin, size.xmax, (i * height) + size.ymin + height);

    // See if the line changed or if the line intersects with the invalid area
    if(dirty||line.Intersects(invalid)) {
      // If area is a valid pointer, add this line's rectangle to it
      if(area)
	area->Union(line);
      // Write the line
      piG2D->Write(1 + size.xmin, (i * height) + size.ymin, fg, -1, text->GetData());
    }

  }

  // Restore the original clipping region
  piG2D->SetClipRect(oldrgn.xmin, oldrgn.ymin, oldrgn.xmax, oldrgn.ymax);

  // Include the invalid area of the console as part of the update
  if(area&&(!invalid.IsEmpty()))
    area->Union(invalid);

  // No more invalid area
  invalid.MakeEmpty();

  // Restore the original font
  piG2D->SetFontID(oldfont);

}

void csConsole::CacheColors(iTextureManager* txtmgr)
{
  // Update the colors from the texture manager
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

void csConsole::GetBackground(int &red, int &green, int &blue) const
{
  red = bg_rgb.red; green = bg_rgb.green; blue = bg_rgb.blue;
}

void csConsole::SetBackground(int red, int green, int blue)
{
  bg_rgb.red = red; bg_rgb.green = green; bg_rgb.blue = blue;
}

void csConsole::GetPosition(int &x, int &y, int &width, int &height) const
{
  x = size.xmin;
  y = size.ymin;
  width = size.Width();
  height = size.Height();
}

void csConsole::SetPosition(int x, int y, int width, int height)
{
  if(x>=0)
    size.xmin = x;
  if(y>=0)
    size.ymin = y;
  if(width>=0)
    size.xmax = size.xmin + width;
  if(height>=0)
    size.ymax = size.ymin + height;

  // Make sure we don't go off the current screen
  if(size.xmax>=piG2D->GetWidth())
    size.xmax = piG2D->GetWidth() - 1;
  if(size.ymax>=piG2D->GetHeight())
    size.ymax = piG2D->GetHeight() - 1;
  
  // Calculate the number of lines on the console
  buffer->SetPageSize(size.Height() / (piG2D->GetTextHeight(font) + 2));

  // Invalidate the entire new area of the console
  invalid.Set(size); 

}

void csConsole::Invalidate(csRect &area)
{
  // Make sure we only update within our rectangle, otherwise 2D driver may crash!
  csRect console(size);
  console.Intersect(area);
  if(!console.IsEmpty())
    invalid.Union(console);
}

bool csConsole::GetTransparency() const
{
  return transparent;
}

void csConsole::SetTransparency(bool trans)
{
  transparent = trans;
}

int csConsole::GetFontID() const
{
  return font;
}

void csConsole::SetFontID(int FontID)
{
  font = FontID;

  // Calculate the number of lines on the console with the new font
  buffer->SetPageSize(size.Height() / (piG2D->GetTextHeight(font) + 2));

}

int csConsole::GetTopLine() const
{
  return buffer->GetTopLine();
}

void csConsole::ScrollTo(int top, bool snap)
{
  switch(top) {
  case csConPageUp:
    buffer->SetTopLine(buffer->GetTopLine() - buffer->GetPageSize());
    break;
  case csConPageDown:
    buffer->SetTopLine(buffer->GetTopLine() + buffer->GetPageSize());
    break;
  case csConVeryTop:
    buffer->SetTopLine(0);
    break;
  case csConVeryBottom:
    buffer->SetTopLine(buffer->GetCurLine() - buffer->GetPageSize());
    break;
  default:
    buffer->SetTopLine(top);
    break;
  }
  do_snap = snap;
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
