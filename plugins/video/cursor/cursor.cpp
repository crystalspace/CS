/*
    Copyright (C)2003 by Neil Mosafi

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
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

#include "cssysdef.h"
#include "cursor.h"
#include "csutil/ref.h"
#include "csutil/cfgacc.h"
#include "csutil/csstring.h"
#include "csutil/stringarray.h"
#include "csgfx/memimage.h"
#include "iutil/objreg.h"
#include "iutil/eventq.h"
#include "iutil/evdefs.h"
#include "iutil/csinput.h"
#include "iutil/virtclk.h"
#include "iutil/vfs.h"
#include "igraphic/imageio.h"
#include "ivaria/reporter.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csCursor)

SCF_IMPLEMENT_IBASE (csCursor)
  SCF_IMPLEMENTS_INTERFACE (iCursor)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csCursor::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csCursor::eiEventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

// String to define our SCF name - only used when reporting 
static const char * const CURSOR_SCF_NAME = "crystalspace.graphic.cursor";

csCursor::csCursor (iBase *parent) :
  reg(0), isActive(false), useOS(false), checkedOSsupport(false)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiEventHandler);
}

csCursor::~csCursor ()
{
  if (eventq) eventq->RemoveListener (&scfiEventHandler);
  RemoveAllCursors ();
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiEventHandler);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csCursor::Initialize (iObjectRegistry *objreg)
{
  reg = objreg;

  // Get image IO
  io = CS_QUERY_REGISTRY (reg, iImageIO);
  if (!io) return false;

  // Get event queue
  eventq = CS_QUERY_REGISTRY (reg, iEventQueue);
  if (!eventq) return false;
  eventq->RegisterListener (&scfiEventHandler, 
                            CSMASK_Mouse | CSMASK_FrameProcess);

  return true;
}

bool csCursor::ParseConfigFile (iConfigFile* ini)
{
  csRef<iVFS> VFS = CS_QUERY_REGISTRY (reg, iVFS);
  if (!VFS)
      return false;

  const char *dir = ini->GetStr ("CursorSystem.General.Directory");

  VFS->PushDir ();
  VFS->ChDir (dir);

  csStringArray ignorelist;

  // Get all the cursor definitions in an iterator
  csString prefix = "CursorSystem.Cursors.";
  csRef<iConfigIterator> params = ini->Enumerate (prefix);
  while (params && params->Next())
  {
    // Work out the name of the cursor
    csString key = params->GetKey() + strlen(params->GetSubsection());
    csString name;
    key.SubString (name, 0, key.FindLast ('.'));

    // Check if we've already added it or there was a problem with it.
    // If so, ignore it
    if (cursors.In ((const char *) name) || 
      ignorelist.Find (name) != csArrayItemNotFound)
      continue;

    // Get all parameters for this cursor
    if (!ini->KeyExists (csString(prefix).Append(name).Append(".image")))
    {
      csReport (reg, CS_REPORTER_SEVERITY_WARNING, CURSOR_SCF_NAME,
                "No image defined for cursor %s, ignoring", name.GetData());

      ignorelist.Push (name);
      continue;
    }
    else 
    {
      // Temporary number storage
      int x, y, z;

      // Read and create image 
      // **** operator+ for csString didn't work so used ugly method
      const char *img =
        ini->GetStr (csString(prefix).Append(name).Append(".image"));
      csRef<iDataBuffer> buf = VFS->ReadFile (img, false);
      if (!buf)
      {
        csReport (reg, CS_REPORTER_SEVERITY_WARNING, CURSOR_SCF_NAME,
                  "Could not open cursor file %s, ignoring cursor %s",
                  img, name.GetData());
        
        ignorelist.Push (name);
        continue;
      }
      csRef<iImage> image = io->Load (buf, CS_IMGFMT_ANY | CS_IMGFMT_ALPHA);
      if (!image)
      {
        csReport (reg, CS_REPORTER_SEVERITY_WARNING, CURSOR_SCF_NAME,
                  "Could not create cs image for cursor %s, ignoring",
                  name.GetData());
        
        ignorelist.Push(name);
        continue;
      }

      // Key color
      bool hasKeyColor = false;
      if (ini->KeyExists(csString(prefix).Append(name).Append(".keycolor")))
      {
	sscanf (ini->GetStr (csString(prefix).Append(name).Append(".keycolor")),
	  "%d,%d,%d", &x, &y, &z);
	hasKeyColor = true;
      }
      csRGBcolor keycolor (x, y, z);

      // Hotspot
      sscanf (ini->GetStr (csString(prefix).Append(name).Append(".hotspot"),
        "0,0"), "%d,%d", &x, &y);
      csPoint hotspot (x, y);

      // Alpha
      uint8 transparency =
        ini->GetInt (csString(prefix).Append(name).Append(".transparency"), 0);

      // Foreground default
      sscanf (ini->GetStr (csString(prefix).Append(name).Append(".mono_fg"),
        "255,255,255"), "%d,%d,%d", &x, &y, &z);
      csRGBcolor fg (x, y, z);

      // Background default
      sscanf (ini->GetStr (csString(prefix).Append(name).Append(".mono_bg"),
        "0,0,0"), "%d,%d,%d", &x, &y, &z);
      csRGBcolor bg (x, y, z);

      // Create the cursor
      SetCursor (name, image, hasKeyColor ? &keycolor : 0, hotspot, 
	transparency, fg, bg);
    }
  }
    
  VFS->PopDir ();
  return true;
}

/*
 * Will always return false to allow others to catch mouse events.
 * Displays cursor on PostProcess and switches cursors on mouse events.
 */
bool csCursor::HandleEvent (iEvent &ev)
{
  if (!isActive)
    return false;

  if (!useOS)
  {
    if (ev.Type == csevBroadcast && ev.Command.Code == cscmdPostProcess)
    {
      CursorInfo* ci = cursors.Get (current.GetData(), 0);
      if (!ci)
        return false;

      if (!ci->pixmap)
      {
	const char* name = current.GetData();

	csRef<iImage> newImage;
	newImage.AttachNew (new csImageMemory (ci->image, 
	  txtmgr->GetTextureFormat()));
	ci->image = newImage;

	// Create texture 
	csRef<iTextureHandle> txt = txtmgr->RegisterTexture (ci->image, 
	  CS_TEXTURE_2D);
	if (!txt)
	{ 
	  csReport (reg, CS_REPORTER_SEVERITY_ERROR, CURSOR_SCF_NAME,
		    "Could not register texture for cursor %s, ignoring", name);
	  RemoveCursor (name);
	  return false;
	}

	// Prepare texture and set up keycolour
	if (ci->hasKeyColor)
	  txt->SetKeyColor (ci->keycolor.red, ci->keycolor.green, 
	  ci->keycolor.blue);

	// Create pixmap from texture
	csSimplePixmap *pixmap = new csSimplePixmap (txt);
	if (!pixmap)
	{ 
	  csReport (reg, CS_REPORTER_SEVERITY_ERROR, CURSOR_SCF_NAME,
		    "Could not create pixmap for cursor %s, ignoring", name);
	  RemoveCursor (name);
	  return false;
	}
	ci->pixmap = pixmap;
      }
      csRef<iVirtualClock> vc = CS_QUERY_REGISTRY (reg, iVirtualClock);
      ci->pixmap->Advance (vc->GetElapsedTicks ());
      csRef<iMouseDriver> mouse = CS_QUERY_REGISTRY (reg, iMouseDriver);

      ci->pixmap->Draw (g3d, mouse->GetLastX() - ci->hotspot.x, 
                             mouse->GetLastY() - ci->hotspot.y,
			     ci->transparency);
      return false;
    }
  }

  if (ev.Type == csevMouseDown)
  {
    if (strcmp (current, CSCURSOR_Default) == 0) current = CSCURSOR_MouseDown;
    return false;
  }
  
  if (ev.Type == csevMouseUp)
  {
    if (strcmp (current, CSCURSOR_MouseDown) == 0) current = CSCURSOR_Default;
    return false;
  }

  return false;
}

// Deactivates system cursor, activates event handling
bool csCursor::Setup (iGraphics3D *ig3d, bool forceEmulation)
{
  if (!ig3d) return false;
  g3d = ig3d;
  txtmgr = g3d->GetTextureManager();
  if (!txtmgr) return false;

  current = CSCURSOR_Default;

  if (forceEmulation)
  {
    checkedOSsupport = true; // pretend we've already checked
    g3d->GetDriver2D()->SetMouseCursor (csmcNone);
  }

  // We set up so let's become active
  isActive = true;
  return true;
}

// Switches to named cursor
bool csCursor::SwitchCursor (const char *name)
{
  if (!strcmp (current.GetDataSafe(), name))
    return true;

  // Get CursorInfo and return false if we can't get it
  CursorInfo *ci = cursors.Get (name, 0);
  if (!ci) return false;

  iGraphics2D *g2d = g3d->GetDriver2D();

  // First time round we check for OS support and if we 
  if (!checkedOSsupport)
  {
    checkedOSsupport = true;

    // Attempt to use image to enable OS level cursor
    if (g2d->SetMouseCursor (ci->image, ci->hasKeyColor ? &ci->keycolor : 0, 
      ci->hotspot.x, ci->hotspot.y, ci->fg, ci->bg))
    {
      useOS = true;
      return true;
    }
    else g2d->SetMouseCursor (csmcNone);
  }

  // We already know which method to use if we're here
  if (useOS) g2d->SetMouseCursor (ci->image, ci->hasKeyColor ? &ci->keycolor : 0, 
    ci->hotspot.x, ci->hotspot.y, ci->fg, ci->bg);

  current = name;
  return true;
}

// Uses a hash map to store named cursors
void csCursor::SetCursor (const char *name, iImage *image, csRGBcolor* key,
                          csPoint hotspot, uint8 transparency, 
                          csRGBcolor fg, csRGBcolor bg)
{
  // Set up structure
  CursorInfo *ci = new CursorInfo;
  ci->image = image;
  ci->transparency = transparency;
  if (key)
  {
    ci->keycolor = *key;
    ci->hasKeyColor = true;
  }
  else
    ci->hasKeyColor = false;
  ci->hotspot = hotspot;
  ci->fg = fg;
  ci->bg = bg;

  // If the image has no name then we give it the name of the cursor.  This is
  // so that the 2D graphics canvas can remember the image each time we switch
  // between cursors
  if (!image->GetName()) image->SetName (name);

  // Set pixmap in the structure
  ci->pixmap = 0;//pixmap;

  // Add to hashlist
  {
    csHash<CursorInfo*, csStrKey>::Iterator it =
      cursors.GetIterator (name);
    while (it.HasNext ())
    {
      CursorInfo* ci = it.Next ();
      delete ci;
    }
  }
  cursors.DeleteAll (name);
  cursors.Put (name, ci);
}

void csCursor::SetHotSpot (const char *name, csPoint hotspot)
{
  CursorInfo *ci = cursors.Get (name, 0);
  if (ci)
  {
    SetCursor (name, ci->image, ci->hasKeyColor ? &ci->keycolor : 0, 
      hotspot, ci->transparency, ci->fg, ci->bg);
    delete ci;
  }
}

void csCursor::SetTransparency (const char *name, uint8 transparency)
{
  CursorInfo *ci = cursors.Get (name, 0);
  if (ci)
  {
    SetCursor (name, ci->image, ci->hasKeyColor ? &ci->keycolor : 0, 
      ci->hotspot, transparency, ci->fg, ci->bg);
    delete ci;
  }
}

void csCursor::SetKeyColor (const char *name, csRGBcolor color)
{
  CursorInfo *ci = cursors.Get (name, 0);
  if (ci)
  {
    SetCursor (name, ci->image, &color, ci->hotspot, ci->transparency,
      ci->fg, ci->bg);
    delete ci;
  }
}

void csCursor::SetColor (const char *name, csRGBcolor fg, csRGBcolor bg)
{
  CursorInfo *ci = cursors.Get (name, 0);
  if (ci)
  {
    SetCursor (name, ci->image, ci->hasKeyColor ? &ci->keycolor : 0, 
      ci->hotspot, ci->transparency, fg, bg);
    delete ci;
  }
} 

csRef<iImage> csCursor::GetCursorImage (const char *name) const
{
  CursorInfo *ci = cursors.Get (name, 0);
  if (ci) return ci->image;

  return 0;
}

csPoint csCursor::GetHotSpot (const char *name) const
{
  CursorInfo *ci = cursors.Get (name, 0);
  if (ci) return ci->hotspot;

  return csPoint (0,0);
}

uint8 csCursor::GetTransparency (const char *name) const
{
  CursorInfo *ci = cursors.Get (name, 0);
  if (ci) return ci->transparency;

  return 0;
}

const csRGBcolor* csCursor::GetKeyColor (const char *name) const
{
  CursorInfo *ci = cursors.Get (name, 0);
  if (ci && ci->hasKeyColor) return &ci->keycolor;

  return 0;
}

csRGBcolor csCursor::GetFGColor (const char *name) const
{
  CursorInfo *ci = cursors.Get (name, 0);
  if (ci) return ci->fg;

  return csRGBcolor (255,255,255);
}

csRGBcolor csCursor::GetBGColor (const char *name) const
{
  CursorInfo *ci = cursors.Get (name, 0);
  if (ci) return ci->bg;

  return csRGBcolor (0,0,0);
}

bool csCursor::RemoveCursor (const char *name)
{
  CursorInfo *ci = cursors.Get (name, 0);
  if (ci)
  {
    cursors.Delete (name, ci);
    delete ci;
    return true;
  }
  return false;
}

void csCursor::RemoveAllCursors ()
{
  csHash<CursorInfo*, csStrKey>::GlobalIterator it = cursors.GetIterator ();
  while (it.HasNext ())
  {
    CursorInfo* ci = it.Next ();
    delete ci;
  }
  cursors.DeleteAll ();
}
