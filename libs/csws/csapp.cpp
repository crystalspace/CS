/*
    Crystal Space Windowing System: Windowing System Application class
    Copyright (C) 1998,1999,2000 by Andrew Zabolotny <bit@eltech.ru>

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

#include <limits.h>
#include <stdarg.h>
#include "cssysdef.h"
#include "qint.h"
#include "csutil/csstrvec.h"
#include "csutil/scanstr.h"
#include "csutil/csstring.h"
#include "cssys/cseventq.h"
#include "csws/cslistbx.h"
#include "csws/csmenu.h"
#include "csws/cswindow.h"
#include "csws/csdialog.h"
#include "csws/csapp.h"
#include "csws/cswsutil.h"
#include "csws/csskin.h"
#include "csws/cswsaux.h"
#include "isys/system.h"
#include "isys/vfs.h"
#include "ivideo/txtmgr.h"
#include "isys/event.h"
#include "igraphic/imageio.h"

//--//--//--//--//--//--//--//--//--//--//--//--//--//--//--//- csAppPlugIn //--

IMPLEMENT_IBASE (csApp::csAppPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

csApp::csAppPlugIn::csAppPlugIn (csApp *iParent)
{
  CONSTRUCT_IBASE (NULL);
  app = iParent;
}

bool csApp::csAppPlugIn::Initialize (iSystem *System)
{
  app->VFS = QUERY_PLUGIN_ID (System, CS_FUNCID_VFS, iVFS);
  if (!app->VFS) return false;

  // Get system event outlet for faster access
  app->EventOutlet = System->GetSystemEventOutlet ();

  // We want ALL the events! :)
  return System->CallOnEvents (this, unsigned (-1));
}

bool csApp::csAppPlugIn::HandleEvent (iEvent &Event)
{
  // If this is a pre-process or post-process event,
  // do the respective work ...
  if (Event.Type == csevBroadcast
   && Event.Command.Code == cscmdPreProcess)
    app->StartFrame ();

  // Send the event to all child components
  bool rc = app->PreHandleEvent (Event)
         || app->HandleEvent (Event)
         || app->PostHandleEvent (Event);

  if (Event.Type == csevBroadcast
   && Event.Command.Code == cscmdPostProcess)
    app->FinishFrame ();

  return rc;
}

//--//--//--//--//--//--//--//--//--//--//--//--//--//--//--//--//-- csApp -//--

csApp::csApp (iSystem *sys, csSkin &Skin)
  : csComponent (NULL)
{
  Mouse = new csMouse(this);
  hints = new csHintManager(this);
  scfiPlugIn = new csAppPlugIn(this);
  
  app = this;			// so that all inserted windows will inherit it
  MouseOwner = NULL;		// no mouse owner
  KeyboardOwner = NULL;		// no keyboard owner
  FocusOwner = NULL;		// no focus owner
  WindowListChanged = false;
  LoopLevel = 0;
  BackgroundStyle = csabsSolid;
  InsertMode = true;
  VFS = NULL;
  DefaultFont = NULL;
  InFrame = false;
  ImageLoader = NULL;
  (System = sys)->IncRef ();
  LastMouseContainer = NULL;

  OldMouseCursorID = csmcNone;
  MouseCursorID = csmcArrow;

  SetPalette (CSPAL_APP);
  state |= CSS_VISIBLE | CSS_SELECTABLE | CSS_FOCUSED;

  skin = &Skin;
}

csApp::~csApp ()
{
  System->UnloadPlugIn (scfiPlugIn);

  // Delete all children prior to shutting down the system
  DeleteAll ();

  // Free the resources allocated by the skin
  if (skin)
    skin->Deinitialize ();

  if (VFS)
    VFS->DecRef ();
  if (DefaultFont)
    DefaultFont->DecRef ();
  if (FontServer)
    FontServer->DecRef ();
  if (ImageLoader)
    ImageLoader->DecRef ();
  // Delete all textures prior to deleting the texture manager
  Textures.DeleteAll ();

  System->DecRef ();
  // Set app to NULL so that ~csComponent() won't call NotifyDelete()
  app = NULL;

  delete scfiPlugIn;
  delete hints;
  delete Mouse;
}

bool csApp::Initialize ()
{
  if (!System->RegisterPlugIn ("crystalspace.windowing.system", "CSWS", scfiPlugIn))
    return false;

  // Create the graphics pipeline
  GfxPpl.Initialize (System);

  ScreenWidth = GfxPpl.FrameWidth;
  ScreenHeight = GfxPpl.FrameHeight;
  bound.Set (0, 0, ScreenWidth, ScreenHeight);
  dirty.Set (bound);
  WindowListWidth = ScreenWidth / 3;
  WindowListHeight = ScreenWidth / 6;

  config.AddConfig(System, "/config/csws.cfg");

  FontServer = QUERY_PLUGIN_ID (System, CS_FUNCID_FONTSERVER, iFontServer);
  DefaultFont = FontServer->LoadFont (CSFONT_COURIER);
  DefaultFontSize = 8;

  ImageLoader = QUERY_PLUGIN_ID (System, CS_FUNCID_IMGLOADER, iImageIO);
  if (!ImageLoader)
    System->Printf (MSG_WARNING,
      "No image loader. Loading images will fail.\n");

  // Now initialize all skin slices
  InitializeSkin ();
  return true;
}

void csApp::SetSkin (csSkin *Skin, bool DeleteOld)
{
  if (DeleteOld)
    delete skin;
  skin = Skin;

  // Now initialize all skin slices
  InitializeSkin ();
}

csSkin *csApp::GetSkin ()
{
  return skin;
}

void csApp::InitializeSkin ()
{
  if (!skin)
    return;

  // Load all textures
  csString SectionName;
  csString DefaultSection;

  if (skin->Prefix)
    SectionName << "CSWS." << skin->Prefix << ".Textures.";
  DefaultSection << "CSWS.Textures.";

  int i;
  bool modified = false;

  // First of all, scan through all loaded textures and unload unneeded textures
  for (i = Textures.Length () - 1; i >= 0; i--)
  {
    csWSTexture *t = Textures.Get (i);
    const char *tn = t->GetName ();
    if (tn && !strncmp (tn, "csws::", 6))
    {
      // See if this texture should be loaded for the new skin
      bool unload = true;
      const char *fn1 = t->GetFileName ();
      if (fn1)
      {
        if (skin->Prefix)
        {
          csString Keyname;
          Keyname << SectionName << tn;

          const char *fn2 = config->GetStr (Keyname, NULL);
          if (fn2 && !strcmp (fn1, fn2))
            unload = false;
        }
        if (unload)
        {
          csString Keyname;
          Keyname << DefaultSection << tn;

          const char *fn2 = config->GetStr (Keyname, NULL);
          if (fn2 && !strcmp (fn1, fn2))
            unload = false;
        }
      }
      if (unload)
      {
        Textures.Delete (i);
        modified = true;
      }
    }
  }

  iConfigIterator *di;

  // Now load all textures from the skin-specific section
  if (skin->Prefix) {
    di = config->Enumerate (SectionName);
    while (di->Next ())
      modified |= LoadTexture (di->GetKey (true), di->GetStr (), CS_TEXTURE_2D);
    di->DecRef ();
  }

  // And now load the textures from the common-for-all-skins section
  di = config->Enumerate (DefaultSection);
  while (di->Next ())
    modified |= LoadTexture (di->GetKey (true), di->GetStr (), CS_TEXTURE_2D);
  di->DecRef ();

  // Load definitions for all mouse cursors
  // @@@ If the skin-specific mouse section exists, this does not load
  // mouse cursors that appear in the default section but not in the
  // skin-specific section. Is this correct behaviour?
  // (I simply copied the old behaviour when moving to the new config
  // system) -- mgeisse

  SectionName.Clear();
  SectionName << "CSWS." << skin->Prefix << ".MouseCursor.";
  DefaultSection.Clear();
  DefaultSection << "CSWS.MouseCursor.";

  di = config->Enumerate(SectionName);
  // look if there are keys in this section
  if (di->Next()) {
    di->Rewind();
  } else {
    di->DecRef();
    di = config->Enumerate(DefaultSection);
  }

  Mouse->ClearPointers ();
  while (di->Next ())
    Mouse->NewPointer (di->GetKey (true), di->GetStr ());
  di->DecRef ();

  // Compute and set the work palette (instead of console palette)
  if (modified)
    PrepareTextures ();

  skin->Initialize (this);
}

void csApp::GetFont (iFont *&oFont, int &oFontSize)
{
  oFont = DefaultFont;
  oFontSize = DefaultFontSize;
}

void csApp::printf (int mode, char* str, ...)
{
  char buf[1024];
  va_list arg;
  va_start (arg, str);
  vsprintf (buf, str, arg);
  va_end (arg);
  System->Printf (mode, "%s", buf);
}

void csApp::SetBackgroundStyle (csAppBackgroundStyle iBackgroundStyle)
{
  BackgroundStyle = iBackgroundStyle;
  Invalidate ();
}

void csApp::ShutDown ()
{
  EventOutlet->Broadcast (cscmdQuit);
}

bool csApp::LoadTexture (const char *iTexName, const char *iTexParams,
  int iFlags)
{
  if (Textures.FindTexture (iTexName))
    return false;

  if (!ImageLoader)
    return false;

  char *filename = NULL;
  float tr = -1, tg = -1, tb = -1;

  while (*iTexParams)
  {
    char tmp [100];
    iTexParams += strspn (iTexParams, " \t");
    size_t sl = strcspn (iTexParams, " \t");
    const char *parm = iTexParams;
    iTexParams += sl;
    if (sl >= sizeof (tmp)) sl = sizeof (tmp) - 1;
    memcpy (tmp, parm, sl);
    tmp [sl] = 0;

    if (!filename)
      filename = strnew (tmp);
    else if (!strncmp (tmp, "Key:", 4))
    {
      ScanStr (tmp + 4, "%f,%f,%f", &tr, &tg, &tb);
    }
    else if (!strncmp (tmp, "Dither:", 7))
    {
      if (!strcasecmp (tmp + 7, "yes"))
        iFlags |= CS_TEXTURE_DITHER;
      else if (!strcasecmp (tmp + 7, "no"))
        iFlags &= ~CS_TEXTURE_DITHER;
      else
        printf (MSG_WARNING, "Texture `%s': invalid MIPMAP() value, 'yes' or 'no' expected\n", iTexName);
    }
    else if (!strncmp (tmp, "Mipmap:", 7))
    {
      if (!strcasecmp (tmp + 7, "yes"))
        iFlags &= ~CS_TEXTURE_NOMIPMAPS;
      else if (!strcasecmp (tmp + 7, "no"))
        iFlags |= CS_TEXTURE_NOMIPMAPS;
      else
        printf (MSG_WARNING, "Texture `%s': invalid MIPMAP() value, 'yes' or 'no' expected\n", iTexName);
    }
    else
    {
      printf (MSG_WARNING, "Texture `%s': Unknown texture parameter: '%s'\n", iTexName, parm);
      delete [] filename;
      return false;
    }
  }

  if (!filename)
  {
    printf (MSG_WARNING, "Texture `%s': No file name defined!\n", iTexName);
    return false;
  }

  // Now load the texture
  iDataBuffer *fbuffer = VFS->ReadFile (filename);
  if (!fbuffer || !fbuffer->GetSize ())
  {
    printf (MSG_WARNING, "Cannot read image file \"%s\" from VFS\n", filename);
    delete [] filename;
    return false;
  }
  delete [] filename;

  iTextureManager *txtmgr = GfxPpl.G3D->GetTextureManager ();
  iImage *image = ImageLoader->Load (fbuffer->GetUint8 (), fbuffer->GetSize (),
    txtmgr->GetTextureFormat ());
  fbuffer->DecRef ();

  csWSTexture *tex = new csWSTexture (iTexName, image, iFlags);
  image->DecRef ();
  if (tb >= 0)
    tex->SetKeyColor (QInt (tr * 255.), QInt (tg * 255.), QInt (tb * 255.));
  Textures.Push (tex);

  return true;
}

void csApp::PrepareTextures ()
{
  iTextureManager *txtmgr = GfxPpl.G3D->GetTextureManager ();

  // Clear all textures from texture manager
  txtmgr->ResetPalette ();

  // Create a uniform palette: r(3)g(3)b(2)
  int r,g,b;
  for (r = 0; r < 8; r++)
    for (g = 0; g < 8; g++)
      for (b = 0; b < 4; b++)
        txtmgr->ReserveColor (r * 32, g * 32, b * 64);

  // Register all CSWS textures to the texture manager
  for (int i = 0; i < Textures.Length (); i++)
    Textures.Get (i)->Register (txtmgr);

  // Prepare all the textures.
  txtmgr->PrepareTextures ();
  // Set the palette (256-color mode)
  txtmgr->SetPalette ();
  // Look in palette for colors we need for windowing system
  SetupPalette ();
  // Finally, set up mouse pointer images
  Mouse->Setup ();
  // Invalidate entire screen
  Invalidate (true);
}

void csApp::SetupPalette ()
{
  csPixelFormat *pfmt = GfxPpl.G2D->GetPixelFormat ();
  PhysColorShift = ((pfmt->RedShift >= 24) || (pfmt->GreenShift >= 24)
    || (pfmt->BlueShift >= 24)) ? 8 : 0;

  iTextureManager* txtmgr = GfxPpl.G3D->GetTextureManager ();
  Pal [cs_Color_Black]   = txtmgr->FindRGB (  0,   0,   0);
  Pal [cs_Color_White]   = txtmgr->FindRGB (255, 255, 255);
  Pal [cs_Color_Gray_D]  = txtmgr->FindRGB (128, 128, 128);
  Pal [cs_Color_Gray_M]  = txtmgr->FindRGB (160, 160, 160);
  Pal [cs_Color_Gray_L]  = txtmgr->FindRGB (204, 204, 204);
  Pal [cs_Color_Blue_D]  = txtmgr->FindRGB (  0,  20,  80);
  Pal [cs_Color_Blue_M]  = txtmgr->FindRGB (  0,  44, 176);
  Pal [cs_Color_Blue_L]  = txtmgr->FindRGB (  0,  64, 255);
  Pal [cs_Color_Green_D] = txtmgr->FindRGB ( 20,  80,  20);
  Pal [cs_Color_Green_M] = txtmgr->FindRGB ( 44, 176,  44);
  Pal [cs_Color_Green_L] = txtmgr->FindRGB ( 64, 255,  64);
  Pal [cs_Color_Red_D]   = txtmgr->FindRGB ( 80,   0,   0);
  Pal [cs_Color_Red_M]   = txtmgr->FindRGB (176,   0,   0);
  Pal [cs_Color_Red_L]   = txtmgr->FindRGB (255,   0,   0);
  Pal [cs_Color_Cyan_D]  = txtmgr->FindRGB (  0,  60,  80);
  Pal [cs_Color_Cyan_M]  = txtmgr->FindRGB (  0, 132, 176);
  Pal [cs_Color_Cyan_L]  = txtmgr->FindRGB (  0, 192, 255);
  Pal [cs_Color_Brown_D] = txtmgr->FindRGB ( 80,  60,  20);
  Pal [cs_Color_Brown_M] = txtmgr->FindRGB (176, 132,  44);
  Pal [cs_Color_Brown_L] = txtmgr->FindRGB (255, 192,  64);
  Pal [cs_Color_Lemon]   = txtmgr->FindRGB (255, 250, 205);
}

int csApp::FindColor (int r, int g, int b)
{
  int color;
  iTextureManager *txtmgr = GfxPpl.G3D->GetTextureManager ();
  color = txtmgr->FindRGB (r, g, b);
  return (color >> PhysColorShift) | 0x80000000;
}

void csApp::Idle ()
{
  System->Sleep (1);
}

void csApp::StartFrame ()
{
  if (InFrame)
    FinishFrame ();
  InFrame = true;

  cs_time elapsed_time;
  System->GetElapsedTime (elapsed_time, CurrentTime);

  GfxPpl.StartFrame (Mouse);
}

void csApp::FinishFrame ()
{
  if (!InFrame)
    StartFrame ();

  // Consider ourselves idle if we don't have anything to redraw in this frame
  bool do_idle = true;

  // Redraw all changed windows
  if (GetState (CSS_DIRTY))
  {
    // Check dirty areas until everybody is happy
    do
    {
      app->SetState (CSS_RESTART_DIRTY_CHECK, false);
      // Check windows from bottom-up for propagated changes
      csRect r (dirty);
      CheckDirtyBU (r);
      // Now propagate dirty areas through transparent windows top-down
      r.MakeEmpty ();
      CheckDirtyTD (r);
    } while (app->GetState (CSS_RESTART_DIRTY_CHECK));

    do_idle = false;
    Redraw ();
  }

  // Now update screen
  if (OldMouseCursorID != MouseCursorID)
  {
    do_idle = false;
    Mouse->SetCursor (MouseCursorID);
    OldMouseCursorID = MouseCursorID;
  }

  // Flush application graphics operations
  GfxPpl.FinishFrame (Mouse);

  if (do_idle)
    Idle ();

  InFrame = false;
}

bool csApp::PreHandleEvent (iEvent &Event)
{
  if (MouseOwner && IS_MOUSE_EVENT (Event))
  {
    // Bring mouse coordinates to child coordinate system
    int dX = 0, dY = 0;
    MouseOwner->LocalToGlobal (dX, dY);
    Event.Mouse.x -= dX;
    Event.Mouse.y -= dY;
    bool ret = MouseOwner->PreHandleEvent (Event);
    Event.Mouse.x += dX;
    Event.Mouse.y += dY;
    return ret;
  }
  else if (KeyboardOwner && IS_KEYBOARD_EVENT (Event))
    return KeyboardOwner->PreHandleEvent (Event);
  else if (FocusOwner)
    return FocusOwner->PreHandleEvent (Event);
  else
    return csComponent::PreHandleEvent (Event);
}

bool csApp::PostHandleEvent (iEvent &Event)
{
  if (MouseOwner && IS_MOUSE_EVENT (Event))
  {
    // Bring mouse coordinates to child coordinate system
    int dX = 0, dY = 0;
    MouseOwner->LocalToGlobal (dX, dY);
    Event.Mouse.x -= dX;
    Event.Mouse.y -= dY;
    bool ret = MouseOwner->PostHandleEvent (Event);
    Event.Mouse.x += dX;
    Event.Mouse.y += dY;
    return ret;
  }
  else if (KeyboardOwner && IS_KEYBOARD_EVENT (Event))
    return KeyboardOwner->PostHandleEvent (Event);
  else if (FocusOwner)
    return FocusOwner->PostHandleEvent (Event);
  else
    return csComponent::PostHandleEvent (Event);
}

bool csApp::HandleEvent (iEvent &Event)
{
  // Mouse should always receive events
  // to reflect current mouse position
  Mouse->HandleEvent (Event);
  // Same about hint manager
  hints->HandleEvent (Event);

  // If canvas size changes, adjust our size accordingly
  if (Event.Type == csevBroadcast
   && Event.Command.Code == cscmdContextResize)
  {
    GfxPpl.CanvasResize ();
    int newScreenWidth = GfxPpl.FrameWidth;
    int newScreenHeight = GfxPpl.FrameHeight;
    int newxmax = bound.xmax;
    int newymax = bound.ymax;
    if (bound.xmax == ScreenWidth)
      newxmax = newScreenWidth;
    if (bound.ymax == ScreenHeight)
      newymax = newScreenHeight;
    ScreenWidth = newScreenWidth;
    ScreenHeight = newScreenHeight;
    SetRect (bound.xmin, bound.ymin, newxmax, newymax);
    Invalidate (true);
  }

  if ((Event.Type == csevKeyDown)
   && (Event.Key.Code == CSKEY_INS))
    InsertMode = !InsertMode;

  // If mouse moves, reset mouse cursor to arrow
  if (Event.Type == csevMouseMove)
    SetMouse (csmcArrow);

  // If a component captured the mouse,
  // forward all mouse (and keyboard) events to it
  if (MouseOwner && (IS_MOUSE_EVENT (Event) || IS_KEYBOARD_EVENT (Event)))
  {
    if (IS_MOUSE_EVENT (Event))
      MouseOwner->GlobalToLocal (Event.Mouse.x, Event.Mouse.y);
    return MouseOwner->HandleEvent (Event);
  } /* endif */

  // If a component captured the keyboard, forward all keyboard events to it
  if (KeyboardOwner && IS_KEYBOARD_EVENT (Event))
  {
    return KeyboardOwner->HandleEvent (Event);
  } /* endif */

  // If a component captured events, forward all focused events to it
  if (FocusOwner && (IS_MOUSE_EVENT (Event) || IS_KEYBOARD_EVENT (Event)))
  {
    if (IS_MOUSE_EVENT (Event))
      FocusOwner->GlobalToLocal (Event.Mouse.x, Event.Mouse.y);
    return FocusOwner->HandleEvent (Event);
  } /* endif */

  // Send event to children as appropiate
  if (csComponent::HandleEvent (Event))
    return true;

  // Handle 'window list' event
  if ((Event.Type == csevMouseDown)
   && (((System->GetMouseButton (1) && System->GetMouseButton (2)))
    || (Event.Mouse.Button == 3)))
  {
    WindowList ();
    return true;
  } /* endif */

  return false;
}

void csApp::FlushEvents ()
{
  System->NextFrame ();
}

void csApp::WindowList ()
{
  csWindowList *wl = new csWindowList (this);
  int x, y;
  GetMouse ().GetPosition (x, y);
  int w = WindowListWidth;
  int h = WindowListHeight;
  csRect rect (x - w/2, y - h/2, x + w/2, y + h/2);
  if (rect.xmin < 0)
    rect.Move (-rect.xmin, 0);
  if (rect.ymin < 0)
    rect.Move (0, -rect.ymin);
  if (rect.xmax > bound.xmax)
    rect.Move (-(rect.xmax - bound.xmax), 0);
  if (rect.ymax > bound.ymax)
    rect.Move (0, -(rect.ymax - bound.ymax));
  ((csComponent *)wl)->SetRect (rect);
  wl->Select ();
}

void csApp::Draw ()
{
  switch (BackgroundStyle)
  {
    case csabsNothing:
      break;
    case csabsSolid:
      Clear (CSPAL_APP_WORKSPACE);
      break;
  } /* endswitch */
  csComponent::Draw ();
}

bool csApp::GetKeyState (int iKey)
{
  return System->GetKeyState (iKey);
}

void csApp::Insert (csComponent *comp)
{
  csComponent::Insert (comp);
  WindowListChanged = true;
}

void csApp::Delete (csComponent *comp)
{
  csComponent::Delete (comp);
  WindowListChanged = true;
}

void csApp::NotifyDelete (csComponent *comp)
{
  if (MouseOwner == comp)
    CaptureMouse (NULL);
  if (KeyboardOwner == comp)
    CaptureKeyboard (NULL);
  if (FocusOwner == comp)
    CaptureFocus (NULL);
    
   if (LastMouseContainer == comp) 
     LastMouseContainer = NULL;

  hints->Remove (comp);
}

int csApp::Execute (csComponent *comp)
{
  csComponent *oldFocusOwner = CaptureFocus (comp);

  comp->Select ();
  comp->SetState (CSS_MODAL, true);
  LoopLevel++;
  System->Loop ();
  LoopLevel--;
  comp->SetState (CSS_MODAL, false);

  CaptureFocus (oldFocusOwner);
  return DismissCode;
}

void csApp::Dismiss (int iCode)
{
  if (LoopLevel)
  {
    DismissCode = iCode;
    EventOutlet->Broadcast (cscmdQuitLoop);
  } /* endif */
}
