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
#include "icursor.h"
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
  // Initialize the cursor state variables
  flash_cursor = false;
  do_flash = false;
  cursor = csConNoCursor;
  cx = cy = 0;
  custom_cursor = NULL;
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
  // Initialize flash_time for flashing cursors
  flash_time = piSystem->GetTime();
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
  // Clear the buffer and redraw the (now blank) console
  buffer->Clear();
  invalid.Set(size);
  // Reset the cursor position
  cx = cy = 0;
}

void csConsole::SetBufferSize(int lines)
{
  buffer->SetLength(lines);
}

void csConsole::PutText(const char *text)
{
  int i;
  csString *curline = NULL;

  // Scan the string for escape characters
  for(i = 0; text[i]!=0; i++) {
    switch(text[i]) {
    case '\n':
      buffer->NewLine(do_snap);
      // Update the cursor Y position
      cx = 0;
      ++cy < buffer->GetPageSize() ? cy : cy--;
      // Make sure we don't change the X position below
      curline = NULL;
      break;
    case '\b':
      // @@@ Should we handle backspaces to previous lines?  Probably...
      if(cx>0) {
	// Delete the character before the cursor, and move the cursor back
	curline = buffer->WriteLine();
	curline->DeleteAt(--cx);
      }
      break;
    case '\t':
      // Print 4-space tabs
      curline = buffer->WriteLine();
      curline->Append("    ");
      break;      
    default:
      curline = buffer->WriteLine();
      curline->Append(text[i]);
      break;
    }
  }
  // Update cursor X position
  if(curline!=NULL)
    cx = curline->Length();
}

void csConsole::Draw(csRect *area)
{
  int i, height, oldfont;
  csRect line, oldrgn;
  const csString *text;
  bool dirty;

  // Save old clipping region
  piG2D->GetClipRect(oldrgn.xmin, oldrgn.ymin, oldrgn.xmax, oldrgn.ymax);

  // Save the old font and set it to the requested font
  oldfont = piG2D->GetFontID();
  piG2D->SetFontID(font);

  // Calculate the height of the text
  height = (piG2D->GetTextHeight(font) + 2);

  // Make sure we erase everything we need to erase
  if(!(invalid.IsEmpty()||transparent)) {
    piG2D->SetClipRect(invalid.xmin, invalid.ymin, invalid.xmax, invalid.ymax);
    piG2D->Clear(bg);
    if(area)
      area->Union(invalid);
  }

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
      // Clip the the current line
      piG2D->SetClipRect(line.xmin, line.ymin, line.xmax, line.ymax);
      // Clear the bg if necessary
      if(!transparent)
	piG2D->Clear(bg);
      // Write the line
      piG2D->Write(1 + size.xmin, (i * height) + size.ymin, fg, -1, text->GetData());
    }

  }

  // Test for a change in the flash state
  if(flash_cursor) {
    int cur_time = piSystem->GetTime();
    if(cur_time > flash_time + 500) {
      do_flash = !do_flash;
      flash_time = cur_time;
    }
  } else
    do_flash = true;

  // See if we draw a cursor
  if((cursor!=csConNoCursor)&&do_flash) {

    int cx_pix, cy_pix;

    // Get the line of text that the cursor is on
    text = buffer->GetLine(cy, dirty);

    if(text==NULL) {

#ifdef DEBUG
      if(cx!=0)
	piSystem->Print(MSG_WARNING, "csConsole:  Empty line but cursor not at start!\n");
#endif // DEBUG

      cx_pix = 1;

    } else {

      // Make a copy of the text
      csString curText(*text);
      curText.SetSize(cx);
      cx_pix = piG2D->GetTextWidth(font, curText.GetData());
    }

    cy_pix = (cy * height) + size.ymin;

    line.Set(cx_pix, cy_pix, cx_pix + piG2D->GetTextWidth(font, "_"), cy_pix + height);
    piG2D->SetClipRect(line.xmin, line.ymin, line.xmax, line.ymax);

    // Draw the appropriate cursor
    switch(cursor) {
    case csConLineCursor:
      piG2D->DrawLine(cx_pix + 1, (cy * height) + (height-3), line.xmax, (cy * height) + (height-3), fg);
      break;
    case csConBlockCursor:
      piG2D->DrawBox(cx_pix + 1, (cy * height) + 1, line.xmax - 1, (cy * height) + (height-1), fg);
      break;
    case csConCustomCursor:
#ifdef DEBUG
      if(custom_cursor==NULL)
	piSystem->Print(MSG_FATAL_ERROR, "csConsole:  Tried to display NULL custom cursor!!!\n");
#endif // DEBUG
      line.ymin++; line.xmax--; line.ymax--;
      custom_cursor->Draw(line);
      break;
#ifdef DEBUG
    default:
      piSystem->Print(MSG_WARNING, "csConsole:  Invalid cursor setting!\n");
#endif // DEBUG
    }

  }

  // Restore the original clipping region
  piG2D->SetClipRect(oldrgn.xmin, oldrgn.ymin, oldrgn.xmax, oldrgn.ymax);

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

  // Reset cursor coordinates
  cx = cy = 0;

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

void csConsole::GetCursorPos(int &x, int &y) const
{
  x = cx;
  y = cy;
}

void csConsole::SetCursorPos(int x, int y)
{
  int max_y = buffer->GetPageSize();

  /* Because of dynamic width fonts, we can't know how far over
   * the cursor will be unless we have the string for that line.
   */
  cx = x;
  
  // But keep it from going off the bottom of the display
  if(y>max_y)
    cy = max_y - 1;
  
}

int csConsole::GetCursorStyle(bool &flashing, iCursor **custom) const
{
  flashing = flash_cursor;
  if(custom!=NULL)
    *custom = custom_cursor;
  return cursor;
}

void csConsole::SetCursorStyle(int style, bool flashing, iCursor *custom)
{
  // Setup the custom cursor
  if(style==csConCustomCursor) {
#ifdef DEBUG
    if(custom==NULL)
      piSystem->Print(MSG_FATAL_ERROR, "csConsole:  Tried to assign NULL pointer for cursor texture!");
#endif
    custom_cursor = custom;
  } else
    // If the cursor isn't set to custom, we ignore the texture completely
    custom_cursor = NULL;

  cursor = style;
  flash_cursor = flashing;
  do_flash = flash_cursor;

}

IMPLEMENT_IBASE(csConsole)
  IMPLEMENTS_INTERFACE(iConsole)
  IMPLEMENTS_INTERFACE(iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY(csConsole)
DECLARE_FACTORY(csConsoleInput)

EXPORT_CLASS_TABLE (cscon)
  EXPORT_CLASS (csConsole, "crystalspace.console.stdout",
		"Standard Console Plugin")
  EXPORT_CLASS (csConsoleInput, "crystalspace.console.stdin",
		"Standard Console Input Plugin")
EXPORT_CLASS_TABLE_END
