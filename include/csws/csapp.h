/*
    Crystal Space Windowing System: Windowing System Application class interface
    Copyright (C) 2001 by Jorrit Tyberghein
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

#ifndef __CS_CSAPP_H__
#define __CS_CSAPP_H__

/**\file 
 * Crystal Space Windowing System: Application class interface
 */
 
/**
 * \addtogroup csws_comps_app
 * @{ */

#include "csextern.h"
 
#include <stdarg.h>

#define CSWS_INTERNAL
#include "csws.h"
#include "cscomp.h"
#include "cswstex.h"
#include "cshints.h"
#include "csmouse.h"
#include "csgfxppl.h"
#include "csutil/cseventq.h"
#include "csutil/array.h"
#include "csutil/cfgacc.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/event.h"

class csSkin;
struct iImageIO;
struct iKeyboardDriver;
struct iMouseDriver;
struct iObjectRegistry;
struct iPluginManager;
struct iVirtualClock;
struct iEventQueue;

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
 * This class is a top-level CrystalSpace Windowing Toolkit object.
 *<p>
 * Generally there should be only one object of this class.
 * Usually it is the root of entire window hierarchy.
 * This class also owns the application-global mouse object,
 * event queue object, graphics pipeline and some others.
 *<p>
 * The Crystal Space application is a regular plugin from the
 * system driver's point of view.
 */
class CS_CRYSTALSPACE_EXPORT csApp : public csComponent
{
protected:
  friend class csMouse;

  /// The graphics pipeline
  csGraphicsPipeline GfxPpl;
  /// The mouse pointer
  csMouse *Mouse;
  /// The list of windowing system textures
  csWSTexVector Textures;
  /// The hints manager
  csHintManager *hints;
  /// Window list width and height
  int WindowListWidth, WindowListHeight;
  /// Current & old mouse pointer ID
  csMouseCursorID MouseCursorID, OldMouseCursorID;
  /// The code that dialog passed to Dismiss ()
  int DismissCode;
  /// This is equal to 8 if any of physical r,g,b masks is 0xff000000
  int PhysColorShift;
  /// The time at the start of current frame
  csTicks CurrentTime;
  /// The system event outlet
  iEventOutlet *EventOutlet;
  /// Application background style
  csAppBackgroundStyle BackgroundStyle;
  /// Are we inbetween StartFrame() and FinishFrame()?
  bool InFrame;

  /// The iComponent interface
  class csAppPlugin : public iComponent
  {
  public:
    SCF_DECLARE_IBASE;
    /// The parent application
    csApp *app;

    /// Constructor.
    csAppPlugin (csApp *iParent);
    /// Destructor.
    virtual ~csAppPlugin();

    /// Initialize the application plugin
    virtual bool Initialize (iObjectRegistry *object_reg);
    /// Handle a event and return true if processed
    virtual bool HandleEvent (iEvent &Event);

    /// iEventHandler implementation.
    struct eiEventHandler : public iEventHandler
    {
      SCF_DECLARE_EMBEDDED_IBASE(csAppPlugin);
      virtual bool HandleEvent(iEvent& e) { return scfParent->HandleEvent(e); }
    } scfiEventHandler;
    friend struct eiEventHandler;
  } *scfiPlugin;
  friend class csAppPlugin;

  /** \internal
   * A structure for keeping modal information on a stack.
   */
  struct csModalInfo
  {
    /// Component that is modal
    csComponent* component;	
    /// Old focus before this component was modal
    csComponent* old_focus;	
    /// Userdata
    iBase* userdata;		
  };
  /**
   * This is a stack of csModalInfo instances to keep track of modality.
   */
  csArray<csModalInfo*> ModalInfo;

public:
  /// The object registry.
  iObjectRegistry* object_reg;
  /// The virtual clock.
  csRef<iVirtualClock> vc;
  /// The event queue.
  csRef<iEventQueue> event_queue;
  /// The plugin manager.
  csRef<iPluginManager> plugin_mgr;
  /// The virtual file system
  csRef<iVFS> VFS;
  /// The system configuration
  csConfigAccess config;
  /// The font server
  csRef<iFontServer> FontServer;
  /// The image loader
  csRef<iImageIO> ImageLoader;
  /// Keyboard driver
  csRef<iKeyboardDriver> KeyboardDriver;
  /// Mouse driver
  csRef<iMouseDriver> MouseDriver;
  /// Application's adaptive palette
  int Pal [cs_Color_Last];
  /// The component that captured the mouse
  csComponent *MouseOwner;
  /// The component that captured the keyboard
  csComponent *KeyboardOwner;
  /// The component that captured all focused events (mouse & keyboard)
  csComponent *FocusOwner;
  /// The component that the mouse was last over
  csComponent *LastMouseContainer;
  /// The global skin repository
  csSkin *skin;
  /// This is set to TRUE each time top-level window list changes
  bool WindowListChanged;
  /// Global "Insert" key state
  bool InsertMode;
  /// Screen width and height (application can be actually smaller)
  int ScreenWidth, ScreenHeight;
  /// Default font
  csRef<iFont> DefaultFont;
  /// Default font size
  int DefaultFontSize;

  /// Initialize windowing system by giving a system driver and a skin
  csApp (iObjectRegistry *object_reg, csSkin &Skin);
  /// Deinitialize windowing system
  virtual ~csApp ();

  /// Set up application layout (read configs, create windows, menus etc)
  virtual bool Initialize ();

  /// Set the skin of the application
  void SetSkin (csSkin *Skin, bool DeleteOld = true);

  /// This is called once per frame by HandleEvent ()
  virtual void StartFrame ();
  /// This is called at the end of every frame
  virtual void FinishFrame ();

  /// Process all events in the queue and refresh the screen
  void FlushEvents ();

  /// Create a new event object: NEVER create event objects with `new'
  iEvent *CreateEvent ()
  { return csRef<iEvent>(EventOutlet->CreateEvent ()); }

  /// Add a previously created event to event queue
  void Post (iEvent *Event)
  { EventOutlet->Post (Event); }

  /// Shut down the program
  void ShutDown ();

  /// The windowing system is idle: do some lazy work
  virtual void Idle ();

  /// Draw the application background
  virtual void Draw ();

  /// Return default font and font size
  virtual void GetFont (iFont *&oFont);

  /// Set application background style
  void SetBackgroundStyle (csAppBackgroundStyle iBackgroundStyle);

  /// Display a string on the console using almost usual printf() syntax
  void Printf (int mode, char const* format, ...) CS_GNUC_PRINTF (3, 4);

  /// Display a string on the console using almost usual printf() syntax
  void PrintfV (int mode, char const* format, va_list) CS_GNUC_PRINTF (3, 0);

  /// Add a single texture to application's texture list
  bool LoadTexture (const char *iTexName, const char *iTexParams,
    int iFlags);

  /// Prepare textures for usage (register them with the graphics driver)
  virtual void PrepareTextures ();

  /// Return application's texture list
  csWSTexVector *GetTextures ()
  { return &Textures; }

  /// Find a texture by name
  iTextureHandle *GetTexture (const char *Name)
  {
    csWSTexture *tex = GetTextures ()->FindTexture (Name);
    return tex ? tex->GetHandle () : (iTextureHandle*)0;
  }

  /// Return application's global mouse object
  csMouse &GetMouse () { return *Mouse; }

  /// Set mouse cursor pointer
  void SetMouseCursor (csMouseCursorID ID) { MouseCursorID = ID; }

  /// Query mouse cursor pointer
  csMouseCursorID GetMouseCursor () { return MouseCursorID; }

  /// Capture all mouse events (or disable capture if 0)
  csComponent *CaptureMouse (csComponent *who)
  { csComponent *c = MouseOwner; MouseOwner = who; return c; }

  /// Capture all keyboard events (or disable capture if 0)
  csComponent *CaptureKeyboard (csComponent *who)
  { csComponent *c = KeyboardOwner; KeyboardOwner = who; return c; }

  /// Capture all focused events (or disable capture if 0)
  csComponent *CaptureFocus (csComponent *who)
  { csComponent *c = FocusOwner; FocusOwner = who; return c; }

  /// Query the current state of a key
  bool GetKeyState (int iKey);

  /// Query current time
  csTicks GetCurrentTime ()
  { return CurrentTime; }

  /// Show window list
  void WindowList ();

  /// Set window list size
  void SetWindowListSize (int iWidth, int iHeight)
  { WindowListWidth = iWidth; WindowListHeight = iHeight; }

  /// Insert a child component
  virtual void Insert (csComponent *comp);

  /// Delete a child component
  virtual void Delete (csComponent *comp);

  /**
   * Set modality state on a component. When the component is dismissed
   * StopModal() will be called automatically. This function returns
   * immediatelly. StartModal() can fail if the component is already modal.
   */
  bool StartModal (csComponent* comp, iBase* userdata);

  /**
   * Stop top-level modality state.
   */
  void StopModal (int iCode = cscmdCancel);

  /**
   * Get the top component which is currently modal.
   * Or 0 if not in modality.
   */
  csComponent* GetTopModalComponent ();

  /**
   * Get the userdata for the top modal component.
   * Or 0 if not in modality.
   */
  iBase* GetTopModalUserdata ();

  /// Dismiss a dialog box with given return code
  void Dismiss (int iCode = cscmdCancel);

  /// Handle a event before all others
  virtual bool PreHandleEvent (iEvent &Event);

  /// Send event to all childs and return processed status
  virtual bool HandleEvent (iEvent &Event);

  /// Handle a event if nobody eaten it.
  virtual bool PostHandleEvent (iEvent &Event);

  /// Every time a component is deleted, this routine is called
  virtual void NotifyDelete (csComponent *iComp);

  /// Get the closest in window hierarchy skin object
  virtual csSkin *GetSkin ();

  /// Associate a hint with given component
  void HintAdd (const char *iText, csComponent *iComp)
  { hints->Add (iText, iComp); }

  /// Remove the hint associated with given component
  void HintRemove (csComponent *iComp);

  /// Get the hint manager object
  csHintManager &GetHintManager ()
  { return *hints; }

  /// Load a font and return its handle or 0
  csPtr<iFont> LoadFont (const char *iFontName, int fontSize = 10)
  { return FontServer->LoadFont (iFontName, fontSize); }

/*
 * The following methods are simple redirectors to csGraphicsPipeline
 * object (which is private property of csApp class).
 */

  /// Return a color identifier given R,G,B (each 0..255)
  int FindColor (int r, int g, int b);

  /// Convert a logical color into physical
  int pplColor (int color)
  { return color & 0x80000000 ? (color & 0x7fffffff) << PhysColorShift : Pal [color]; }

  /// Draw a box
  void pplBox (int x, int y, int w, int h, int color)
  { GfxPpl.Box (x, y, w, h, pplColor (color)); }

  /// Draw a line
  void pplLine (float x1, float y1, float x2, float y2, int color)
  { GfxPpl.Line (x1, y1, x2, y2, pplColor (color)); }

  /// Draw a pixel
  void pplPixel (int x, int y, int color)
  { GfxPpl.Pixel (x, y, pplColor (color)); }

  /// Draw a text string: if bg < 0 background is not drawn
  void pplText (int x, int y, int fg, int bg, iFont *Font, const char *s)
  { GfxPpl.Text (x, y, pplColor (fg), bg != -1 ? pplColor (bg) : bg, Font, s); }

  /// Draw a (scaled) pixmap
  void pplPixmap (csPixmap *s2d, int x, int y, int w, int h, uint8 Alpha)
  { GfxPpl.Pixmap (s2d, x, y, w, h, Alpha); }
  /// Draw a (unscaled but tiled) pixmap
  void pplTiledPixmap (csPixmap *s2d, int x, int y, int w, int h,
    int orgx, int orgy, uint8 Alpha)
  { GfxPpl.TiledPixmap (s2d, x, y, w, h, orgx, orgy, Alpha); }

  /// Draw a (part) of texture (possibly scaled) in given screen rectangle
  void pplTexture (iTextureHandle *hTex, int sx, int sy, int sw, int sh,
    int tx, int ty, int tw, int th, uint8 Alpha = 0)
  { GfxPpl.Texture (hTex, sx, sy, sw, sh, tx, ty, tw, th, Alpha); }

  /// Save a part of screen
  void pplSaveArea (csImageArea *&Area, int x, int y, int w, int h)
  { GfxPpl.SaveArea (&Area, x, y, w, h); }
  /// Restore a part of screen
  void pplRestoreArea (csImageArea *Area, bool Free = false)
  { GfxPpl.RestoreArea (Area, Free); }
  /// Free buffer used to keep an area of screen
  void pplFreeArea (csImageArea *Area)
  { GfxPpl.FreeArea (Area); }

  /// Clear page with specified color
  void pplClear (int color)
  { GfxPpl.Clear (pplColor (color)); }

  /// Set clipping rectangle: SHOULD CALL pplRestoreClipRect() AFTER DRAWING!
  void pplSetClipRect (int xmin, int ymin, int xmax, int ymax)
  { GfxPpl.SetClipRect (xmin, ymin, xmax, ymax); }

  /// Same, but with csRect argument
  void pplSetClipRect (csRect &clip)
  { GfxPpl.SetClipRect (clip.xmin, clip.ymin, clip.xmax, clip.ymax); }

  /// Restore clipping rectangle to (0, 0, ScreenW, ScreenH);
  void pplRestoreClipRect ()
  { GfxPpl.RestoreClipRect (); }

  /// Clip a line against a rectangle and return true if its clipped out
  bool ClipLine (float &x1, float &y1, float &x2, float &y2,
    int ClipX1, int ClipY1, int ClipX2, int ClipY2)
  { return GfxPpl.ClipLine (x1, y1, x2, y2, ClipX1, ClipY1, ClipX2, ClipY2); }

  /// Change system mouse cursor and return success status
  bool SwitchMouseCursor (csMouseCursorID Shape)
  { return GfxPpl.SwitchMouseCursor (Shape); }

  /// Get R,G,B at given screen location
  void GetPixel (int x, int y, uint8 &oR, uint8 &oG, uint8 &oB)
  { GfxPpl.GetPixel (x, y, oR, oG, oB); }

  //--- 3D drawing ---//

  /// Draw a 3D polygon
  //@@@REIMPLEMENT THIS FOR NR
  /*void pplPolygon3D (G3DPolygonDPFX &poly, uint mode)
  { GfxPpl.Polygon3D (poly, mode); }*/

  /// Clear the Z-buffer in the given area
  void pplClearZbuffer (int x1, int y1, int x2, int y2)
  { GfxPpl.ClearZbuffer (x1, y1, x2, y2); }

  /// Clear the entire Z-buffer (takes effect before next 3D draw operation)
  void pplClearZbuffer ()
  { GfxPpl.ClearZbuffer (); }

  /// Set the respective Z-buffer mode (one of CS_ZBUF_XXX constants)
  void SetZbufferMode (unsigned mode)
  { GfxPpl.SetZbufferMode (mode); }

  /// Begin drawing: users of CSWS should NEVER invoke G2D/G3D->BeginDraw!
  void pplBeginDraw (unsigned mode)
  { GfxPpl.BeginDraw (mode); }

  /// Force blitting of the respective rectangle at the end of frame
  void pplInvalidate (csRect &rect)
  { GfxPpl.Invalidate (rect); }

  /**
   * Tell the graphics pipeline that you are going to update
   * the entire screen during NEXT frame. This will force graphics
   * pipeline to NOT cache the image of the current frame so that it
   * can be propagated to next frame (if during next frame you are
   * going to repaint just a part of screen, propagating image
   * changes through multiple video pages is a MUST, otherwise
   * you will get flickering images).
   */
  void pplDontCacheFrame ()
  { GfxPpl.DontCacheFrame = true; }

  /**
   * Get the pointer to 2D graphics driver for direct manipulations.
   * WARNING! Don't abuse of this function!
   */
  iGraphics2D *GetG2D ()
  { return GfxPpl.G2D; }

  /**
   * Get the pointer to 2D graphics driver for direct manipulations.
   * WARNING! Don't abuse of this function!
   */
  iGraphics3D *GetG3D ()
  { return GfxPpl.G3D; }

protected:
  /// Initialize all skin slices with textures and colors etc
  void InitializeSkin ();
  /// setup palette
  void SetupPalette ();
};

/** @} */

#endif // __CS_CSAPP_H__
