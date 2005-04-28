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

#ifndef __CS_VIDEO_CURSOR_H__
#define __CS_VIDEO_CURSOR_H__

#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/custcursor.h"
#include "ivideo/txtmgr.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"
#include "iutil/event.h"
#include "csutil/csstring.h"
#include "csutil/hash.h"
#include "csutil/weakref.h"
#include "cstool/cspixmap.h"

struct iObjectRegistry;
struct iEventQueue;
struct iEvent;
struct iTextureManager;
struct iImageIO;

/**
 * This class implements the iCursor interface and handles custom
 * pixmap cursors. Any number of cursors can be set and easily
 * switched between. You need to set a keycolor for your cursor and you can
 * optionally set transparency and foregound and background colors for
 * monochrome cursor systems
 */
class csCursor : public iCursor
{
private:
  /// Internal structure for use in the cursors hash list
  struct CursorInfo
  {
    csPixmap *pixmap;
    csRef<iImage> image;
    csPoint hotspot;
    uint8 transparency;
    csRGBcolor keycolor, fg, bg;
    bool hasKeyColor;

    CursorInfo () : pixmap(0), image(0) {}
    ~CursorInfo () { delete pixmap; }
  };

  iObjectRegistry* reg;
  csWeakRef<iEventQueue> eventq;
  csWeakRef<iGraphics3D> g3d;
  iTextureManager* txtmgr;
  csRef<iImageIO> io;
  csHash<CursorInfo *, csStrKey> cursors;
  
  /// The currently selected cursor
  csString current;

  /// Is the plugin is active (e.g. has Setup() has been called)
  bool isActive;

  /**
   * Is the plugin using OS or doing manual rendering?  If the OS does not
   * support custom cursors it will disable the OS cursor and render direct to
   * the canvas
   */
  bool useOS;

  /// Did we already check for OS support for custom cursors?
  bool checkedOSsupport;

protected:
  /// Initializes
  bool Initialize (iObjectRegistry *);

  /**
   * Handles displaying of software cursors.  Also handles switching to and
   * from CS_CURSOR_MouseDown cursor upon MouseDown event (if defined)
   */
  bool HandleEvent (iEvent &);

public:
  SCF_DECLARE_IBASE;

  /// Constructor
  csCursor (iBase *);
  /// Destructor
  virtual ~csCursor ();

  /// Sets up cursors for display on the graphics context
  virtual bool Setup (iGraphics3D *ig3d, bool ForceEmulation);

  /// Load cursors from a config file
  virtual bool ParseConfigFile (iConfigFile* iFile);

  /// Adds or replaces a cursor based on the name
  virtual void SetCursor (const char *name, iImage *image, csRGBcolor* key,
			  csPoint hotspot, uint8 transparency,
                          csRGBcolor fg, csRGBcolor bg);
  /// Sets the hotspot (center) of a cursor
  virtual void SetHotSpot (const char *name, csPoint hotspot);
  /// Sets transparency transparency of cursor
  virtual void SetTransparency (const char *name, uint8 transparency);
  /// Set the key colour of a cursor
  virtual void SetKeyColor (const char *name, csRGBcolor);
  /**
   * Set the foreground and background colors to be used if the OS only
   * only supports monochrome cursors
   */
  virtual void SetColor (const char *name, csRGBcolor fg, csRGBcolor bg);

  /// Get cursor image of the specified cursor
  virtual csRef<iImage> GetCursorImage (const char *name) const;
  /// Get the hotspot (center) of the specified cursor on the pixmap.
  virtual csPoint GetHotSpot (const char *name) const;
  /// Get the transparency transparency of the specified cursor.  
  virtual uint8 GetTransparency (const char *name) const;
  /// Get key colour of the specified cursor.
  virtual const csRGBcolor* GetKeyColor (const char *name) const;
  /**
   * Get the foreground color of the cursor.  These will only be used when 
   * in OS mode on systems which only support monochrome cursors
   */
  virtual csRGBcolor GetFGColor (const char *name) const;
  /**
   * Get the background color of the cursor.  These will only be used when 
   * in OS mode on systems which only support monochrome cursors
   */
  virtual csRGBcolor GetBGColor (const char *name) const;

  /// Removes a cursor
  virtual bool RemoveCursor (const char *);
  /// Removes all cursors
  virtual void RemoveAllCursors ();

  /// Switches from the current cursor to the specified cursor
  virtual bool SwitchCursor (const char *);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csCursor);
    virtual bool Initialize (iObjectRegistry* r)
    {
      return scfParent->Initialize(r);
    }
  } scfiComponent;
  friend struct eiComponent;

  struct eiEventHandler : public iEventHandler
  {
    SCF_DECLARE_EMBEDDED_IBASE (csCursor);
    virtual bool HandleEvent (iEvent &ev)
      { return scfParent->HandleEvent (ev); }
  } scfiEventHandler;
  friend struct eiEventHandler;
};

#endif // __CS_VIDEO_CURSOR_H__
