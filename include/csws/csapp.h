/*
    Crystal Space Windowing System: Windowing System Application class interface
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

#ifndef __CSAPP_H__
#define __CSAPP_H__

#define CSWS_INTERNAL
#include "csws.h"
#include "cscomp.h"
#include "csgfxppl.h"
#include "csinput/cseventq.h"
#include "csutil/csstrvec.h"
#include "csengine/texture.h"

/**
 * Application's background styles
 */
enum csAppBackgroundStyle
{
  /// Nothing: use only if desktop is not visible to avoid extra drawing
  csabsNothing = 0,
  /// Solid background of palette[0] color
  csabsSolid
};

/**
 * This class is a top-level CrystalSpace Windowing Toolkit object.<p>
 * Generally there should be only one object of this class.
 * Usually it is the root of entire window hierarchy.
 * This class also owns the application-global mouse object,
 * event queue object, graphics pipeline and some others.
 */
class csApp : public csComponent
{
protected:
  friend class appSystemDriver;

  /// The graphics pipeline
  csGraphicsPipeline *GfxPpl;
  /// The mouse pointer
  csMouse *Mouse;
  /// The event queue
  csEventQueue *EventQueue;
  /// The world object
  csWorld *World;
  /// Application background style
  csAppBackgroundStyle BackgroundStyle;
  /// Window list width and height
  int WindowListWidth, WindowListHeight;
  /// Current & old mouse pointer ID
  csMouseCursorID MouseCursorID, oldMouseCursorID;
  /// The recursion level within System->Loop ()
  int LoopLevel;
  /// The code that dialog passed to Dismiss ()
  int DismissCode;

  /// Set up initial application layout (read configs, create windows, menus etc)
  virtual bool InitialSetup (int argc, char *argv[],
    const char *ConfigName, const char *VfsConfigName, const char* dataDir);

public:
  /// Application's adaptive palette
  int Pal[cs_Color_Last];
  /// The component that captured the mouse
  csComponent *MouseOwner;
  /// The component that captured the keyboard
  csComponent *KeyboardOwner;
  /// The component that captured all focused events (mouse & keyboard)
  csComponent *FocusOwner;
  /// If this flag is set, there are still unrefreshed components
  bool RedrawFlag;
  /// This is set to TRUE each time top-level window list changes
  bool WindowListChanged;

  /// The following variables are initialized by reading sys/csws.cfg
  /// Mouse cursor texture
  char *mousetexturename, *mousetextureparm;
  /// Titlebar buttons texture
  char *titletexturename, *titletextureparm;
  /// Titlebar buttons definitions
  csStrVector *titlebardefs;
  /// Dialog buttons (radio- and check- boxes) texture
  char *dialogtexturename, *dialogtextureparm;
  /// Dialog buttons definitions
  csStrVector *dialogdefs;
  /// Custom textures
  csStrVector *customtexturename, *customtextureparm;
  /// Global "Insert" key state
  bool insert;

  /// Initialize windowing system
  csApp (const char *AppTitle, csAppBackgroundStyle iBackgroundStyle = csabsSolid);
  /// Deinitialize windowing system
  virtual ~csApp ();

  /// Shut down the program
  void ShutDown ()
  { System->Shutdown = true; }

  /// The windowing system is idle: do some lazy work
  void Idle ();

  /// Draw the application background
  virtual void Draw ();

  /// This should be called once per frame by system driver
  virtual void NextFrame ();

  /// Set world for textures, config files etc.
  virtual void SetWorld (csWorld *AppWorld);

  /// Get world for textures, config files etc.
  csWorld* GetWorld () { return World; }

  /// Start endless event loop
  virtual void Loop ();

  /// Return application's texture list
  csTextureList *GetTextures ()
  { return World->GetTextures (); }

  /// Find a texture by name
  csTextureHandle *GetTexture (char *Name)
  { return GetTextures ()->GetTextureMM (Name); }

  /// Return application's global mouse object
  csMouse *GetMouse () { return Mouse; }

  /// Set mouse cursor pointer
  void SetMouseCursor (csMouseCursorID ID) { MouseCursorID = ID; }

  /// Query mouse cursor pointer
  csMouseCursorID GetMouseCursor () { return MouseCursorID; }

  /// Add a event to event queue
  void PutEvent (csEvent *Event) { EventQueue->Put (Event); }

  /// Process all queued messages
  void Process ();

  /// Handle a event and return true if processed
  virtual bool HandleEvent (csEvent &Event);

  /// Return active page number
  int GetPage ()
  { int Page; System->piGI->GetPage (Page); return Page; }

  /// Capture all mouse events (or disable capture if NULL)
  csComponent *CaptureMouse (csComponent *who)
  { csComponent *c = MouseOwner; MouseOwner = who; return c; }

  /// Capture all keyboard events (or disable capture if NULL)
  csComponent *CaptureKeyboard (csComponent *who)
  { csComponent *c = KeyboardOwner; KeyboardOwner = who; return c; }

  /// Capture all focused events (or disable capture if NULL)
  csComponent *CaptureFocus (csComponent *who)
  { csComponent *c = FocusOwner; FocusOwner = who; return c; }

  /// Query shift key state
  bool GetKeyState (int iKey);

  /// Show window list
  void WindowList ();

  /// Set window list size
  void SetWindowListSize (int iWidth, int iHeight)
  { WindowListWidth = iWidth; WindowListHeight = iHeight; }

  /// Insert a child component
  virtual void Insert (csComponent *comp);

  /// Delete a child component
  virtual void Delete (csComponent *comp);

  /// Execute a dialog box (or entire app if NULL) and return its dismiss code
  int Execute (csComponent *comp);

  /// Dismiss a dialog box with given return code
  void Dismiss (int iCode = cscmdCancel);

  /// Handle a event before all others
  virtual bool PreHandleEvent (csEvent &Event);

  /// Handle a event if nobody eaten it.
  virtual bool PostHandleEvent (csEvent &Event);

/*
 * The following methods are simple redirectors to csGraphicsPipeline
 * object (which is private property of csApp class). All operations
 * are deferred until Update () is called.
 */

  /// Draw a box
  void pplBox (int x, int y, int w, int h, int color)
  { GfxPpl->Box (x, y, w, h, Pal [color]); }

  /// Draw a line
  void pplLine (float x1, float y1, float x2, float y2, int color)
  { GfxPpl->Line (x1, y1, x2, y2, Pal [color]); }

  /// Draw a pixel
  void pplPixel (int x, int y, int color)
  { GfxPpl->Pixel (x, y, Pal [color]); }

  /// Draw a text string: if bg < 0 background is not drawn
  void pplText (int x, int y, int fg, int bg, int Font, const char *s)
  { GfxPpl->Text (x, y, Pal[fg], bg >= 0 ? Pal[bg] : bg, Font, s); }

  /// Draw a (scaled) 2D sprite
  void pplSprite2D (csSprite2D *s2d, int x, int y, int w, int h)
  { GfxPpl->Sprite2D (s2d, x, y, w, h); }

  /// Save a part of screen
  void pplSaveArea (ImageArea *&Area, int x, int y, int w, int h)
  { GfxPpl->SaveArea (&Area, x, y, w, h); }
  /// Restore a part of screen
  void pplRestoreArea (ImageArea *Area, bool Free = false)
  { GfxPpl->RestoreArea (Area, Free); }
  /// Free buffer used to keep an area of screen
  void pplFreeArea (ImageArea *Area)
  { GfxPpl->FreeArea (Area); }

  /// Clear page with specified color
  void pplClear (int color)
  { GfxPpl->Clear (Pal[color]); }

  /// Set clipping rectangle: SHOULD CALL pplRestoreClipRect() AFTER DRAWING!
  void pplSetClipRect (int xmin, int ymin, int xmax, int ymax)
  { GfxPpl->SetClipRect (xmin, ymin, xmax, ymax); }

  /// Same, but with csRect argument
  void pplSetClipRect (csRect &clip)
  { GfxPpl->SetClipRect (clip.xmin, clip.ymin, clip.xmax, clip.ymax); }

  /// Restore clipping rectangle to (0, 0, ScreenW, ScreenH);
  void pplRestoreClipRect ()
  { GfxPpl->RestoreClipRect (); }

  /// Draw a 3D polygon
  void pplPolygon3D (G3DPolygonDPFX &poly, UInt mode)
  { GfxPpl->Polygon3D (poly, mode); }

  /// Return the width of given text using currently selected font
  int TextWidth (const char *text, int Font)
  { return GfxPpl->TextWidth (text, Font); }
  /// Return the height of currently selected font
  int TextHeight (int Font)
  { return GfxPpl->TextHeight (Font); }

protected:
  /// Initialize configuration data: load csws.cfg
  virtual void LoadConfig ();

private:
  /// load textures used by all child objects
  void LoadTextures ();
  /// setup palette
  void SetupPalette ();
  /// Flush graphics pipeline
  void Update ();
};

#endif // __CSAPP_H__
