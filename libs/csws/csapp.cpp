/*
    Crystal Space Windowing System: Windowing System Application class
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

#include <stdarg.h>
#include "sysdef.h"
#include "qint.h"
#include "csparser/csloader.h"
#include "csutil/inifile.h"
#include "csutil/vfs.h"
#include "csutil/csstrvec.h"
#include "csutil/scanstr.h"
#include "csinput/cseventq.h"
#include "csws/cswssys.h"
#include "csws/cslistbx.h"
#include "csws/csmouse.h"
#include "csws/csmenu.h"
#include "csws/cswindow.h"
#include "csws/csdialog.h"
#include "csws/csapp.h"
#include "csws/cswsutil.h"
#include "isystem.h"
#include "itxtmgr.h"

// Archive path of CSWS.CFG
#define CSWS_CFG "csws.cfg"

TOKEN_DEF_START
  TOKEN_DEF (MOUSECURSOR)
  TOKEN_DEF (TITLEBUTTON)
  TOKEN_DEF (DIALOGBUTTON)
  TOKEN_DEF (TEXTURE)
  TOKEN_DEF (TRANSPARENT)
  TOKEN_DEF (FILE)
TOKEN_DEF_END

//--//--//--//--//--//--//--//--//--//--//--//--//--//--//--//--//- csApp  -//--

csApp::csApp (const char *AppTitle, csAppBackgroundStyle iBackgroundStyle) : csComponent (NULL)
{
  app = this;			// so that all inserted windows will inherit it
  text = NULL;
  SetText (AppTitle);		// application title string
  MouseOwner = NULL;		// no mouse owner
  KeyboardOwner = NULL;		// no keyboard owner
  FocusOwner = NULL;		// no focus owner
  titlebardefs =
  dialogdefs = NULL;
  RedrawFlag = true;
  WindowListChanged = false;
  LoopLevel = 0;
  BackgroundStyle = iBackgroundStyle;
  insert = true;

  oldMouseCursorID = csmcNone;
  MouseCursorID = csmcArrow;

  SetPalette (CSPAL_APP);
  state |= CSS_VISIBLE | CSS_SELECTABLE | CSS_FOCUSED;

  CHK (Mouse = new csMouse (NULL));
  Mouse->app = this;
  CHK (GfxPpl = new csGraphicsPipeline ());

  // Create the system driver object
  CHK (System = new cswsSystemDriver (this));
}

csApp::~csApp ()
{
  // Delete all children prior to shutting down the system
  DeleteAll ();

  if (titlebardefs)
    CHKB (delete titlebardefs);
  if (dialogdefs)
    CHKB (delete dialogdefs);

  if (Mouse)
    CHKB (delete Mouse);
  if (GfxPpl)
    CHKB (delete GfxPpl);

  if (System)
    CHK (delete System);
}

void csApp::printf (int mode, char* str, ...)
{
  char buf[1024];
  va_list arg;
  va_start (arg, str);
  vsprintf (buf, str, arg);
  va_end (arg);
  System->Printf (mode, buf);
}

void csApp::ShutDown ()
{
  System->Shutdown = true;
}

bool csApp::InitialSetup (int argc, char *argv[],
  const char *iConfigName, const char *iVfsConfigName, const char* iDataDir)
{
  System->Initialize (argc, argv, iConfigName, iVfsConfigName, NULL);

  // For GUI apps double buffering is a serious performance hit
  System->piG2D->DoubleBuffer (false);

  // Change to the directory on VFS where we keep our data
  System->Vfs->ChDir (iDataDir);

  EventQueue = System->EventQueue;

  int Width, Height;
  System->piGI->GetWidth (Width);
  System->piGI->GetHeight (Height);
  bound.Set (0, 0, Width, Height);
  dirty.Set (bound);
  WindowListWidth = Width / 3;
  WindowListHeight = Width / 6;

  // Tell printf() console is ready
  System->DemoReady = true;

  LoadConfig ();
  // Compute and set the work palette (instead of console palette)
  PrepareTextures ();
  return true;
}

int csApp::GetPage ()
{
  int Page;
  System->piGI->GetPage (Page);
  return Page;
}

void csApp::Loop ()
{
  System->Loop();
}

void csApp::NextFrame ()
{
  Process ();

  // Now update screen
  if (GfxPpl->BeginDraw ())
  {
    Update ();
    GfxPpl->FinishDraw ();
  }
}

void csApp::Update ()
{
  if (MouseCursorID != oldMouseCursorID)
  {
    Mouse->SetCursor (MouseCursorID);
    oldMouseCursorID = MouseCursorID;
  } /* endif */

  // Save background/draw mouse cursor
  Mouse->Draw ();

  // Flush application graphics operations
  GfxPpl->Flush (GetPage ());

  // Restore previous background under mouse
  Mouse->Undraw ();
}

bool csApp::LoadTexture (const char *iTexName, const char *iTexParams,
  bool i2D, bool i3D)
{
  TOKEN_TABLE_START (texcommands)
    TOKEN_TABLE (FILE)
    TOKEN_TABLE (TRANSPARENT)
  TOKEN_TABLE_END

  const char *filename = iTexName;
  char *buffer = strnew (iTexParams);
  char *bufptr = buffer;
  bool transp = false;
  float tr, tg, tb;

  int cmd;
  char *name, *params;
  while ((cmd = csGetObject (&bufptr, texcommands, &name, &params)) > 0)
    switch (cmd)
    {
      case TOKEN_FILE:
        filename = params;
        break;
      case TOKEN_TRANSPARENT:
        transp = true;
        ScanStr (params, "%f,%f,%f", &tr, &tg, &tb);
        break;
      default:
        CHK (delete [] buffer);
        return false;
    }

  // Now load the texture
  ImageFile *image;
  size_t size;
  char *fbuffer = System->Vfs->ReadFile (filename, size);
  CHK (delete [] buffer);
  if (!fbuffer || !size)
  {
    printf (MSG_WARNING, "Cannot read image file \"%s\" from VFS\n", filename);
    return false;
  }

  image = ImageLoader::load ((UByte *)fbuffer, size);
  CHK (delete [] fbuffer);

  if (image && (image->get_status () & IFE_Corrupt))
  {
    printf (MSG_WARNING, "'%s': %s!\n", filename, image->get_status_mesg ());
    CHK (delete image);
    return false;
  }

  csWSTexture *tex = new csWSTexture (iTexName, image, i2D, i3D);
  if (transp)
    tex->SetTransparent (QInt (tr * 255.), QInt (tg * 255.), QInt (tb * 255.));
  Textures.Push (tex);

  return true;
}

void csApp::LoadConfig ()
{
  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (MOUSECURSOR)
    TOKEN_TABLE (TITLEBUTTON)
    TOKEN_TABLE (DIALOGBUTTON)
    TOKEN_TABLE (TEXTURE)
  TOKEN_TABLE_END

  if (!titlebardefs)
    CHKB (titlebardefs = new csStrVector (16, 16));
  if (!dialogdefs)
    CHKB (dialogdefs = new csStrVector (16, 16));

  size_t cswscfglen;
  char *cswscfg = System->Vfs->ReadFile (CSWS_CFG, cswscfglen);
  if (!cswscfg)
  {
    printf (MSG_FATAL_ERROR, "ERROR: CSWS config file `%s' not found\n", CSWS_CFG);
    fatal_exit (0, true);			// cfg file not found
    return;
  }

  int cmd;
  char *cfg = cswscfg, *name, *params;
  while ((cmd = csGetObject (&cfg, commands, &name, &params)) > 0)
    switch (cmd)
    {
      case TOKEN_MOUSECURSOR:
      {
        Mouse->NewPointer (name, params);
        break;
      }
      case TOKEN_TITLEBUTTON:
      {
        CHK (char *tmp = new char [strlen (name) + 1 + strlen (params) + 1]);
        sprintf (tmp, "%s %s", name, params);
        titlebardefs->Push (tmp);
        break;
      }
      case TOKEN_DIALOGBUTTON:
      {
        CHK (char *tmp = new char [strlen (name) + 1 + strlen (params) + 1]);
        sprintf (tmp, "%s %s", name, params);
        dialogdefs->Push (tmp);
        break;
      }
      case TOKEN_TEXTURE:
        LoadTexture (name, params, true, false);
        break;
      default:
        printf (MSG_FATAL_ERROR, "Unknown token in csws.cfg! (%s)\n", cfg);
        fatal_exit (0, false);			// Unknown token
    }
  CHK (delete[] cswscfg);
}

void csApp::PrepareTextures ()
{
  ITextureManager *txtmgr;
  System->piG3D->GetTextureManager (&txtmgr);

  // Clear all textures from texture manager
  txtmgr->Initialize ();

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
  txtmgr->Prepare ();
  // Allocate a common palette for texture (256-color mode)
  txtmgr->AllocPalette ();
  // Look in palette for colors we need for windowing system
  SetupPalette ();
  // Finally, set up mouse pointer images
  Mouse->Setup ();
}

void csApp::SetupPalette ()
{
  csPixelFormat pfmt;
  System->piGI->GetPixelFormat (&pfmt);
  PhysColorShift = ((pfmt.RedShift >= 24) || (pfmt.GreenShift >= 24)
    || (pfmt.BlueShift >= 24)) ? 8 : 0;

  ITextureManager* txtmgr;
  System->piG3D->GetTextureManager (&txtmgr);
  txtmgr->FindRGB (  0,   0,   0, Pal [cs_Color_Black]);
  txtmgr->FindRGB (255, 255, 255, Pal [cs_Color_White]);
  txtmgr->FindRGB (128, 128, 128, Pal [cs_Color_Gray_D]);
  txtmgr->FindRGB (160, 160, 160, Pal [cs_Color_Gray_M]);
  txtmgr->FindRGB (204, 204, 204, Pal [cs_Color_Gray_L]);
  txtmgr->FindRGB (  0,  20,  80, Pal [cs_Color_Blue_D]);
  txtmgr->FindRGB (  0,  44, 176, Pal [cs_Color_Blue_M]);
  txtmgr->FindRGB (  0,  64, 255, Pal [cs_Color_Blue_L]);
  txtmgr->FindRGB ( 20,  80,  20, Pal [cs_Color_Green_D]);
  txtmgr->FindRGB ( 44, 176,  44, Pal [cs_Color_Green_M]);
  txtmgr->FindRGB ( 64, 255,  64, Pal [cs_Color_Green_L]);
  txtmgr->FindRGB ( 80,   0,   0, Pal [cs_Color_Red_D]);
  txtmgr->FindRGB (176,   0,   0, Pal [cs_Color_Red_M]);
  txtmgr->FindRGB (255,   0,   0, Pal [cs_Color_Red_L]);
  txtmgr->FindRGB (  0,  60,  80, Pal [cs_Color_Cyan_D]);
  txtmgr->FindRGB (  0, 132, 176, Pal [cs_Color_Cyan_M]);
  txtmgr->FindRGB (  0, 192, 255, Pal [cs_Color_Cyan_L]);
  txtmgr->FindRGB ( 80,  60,  20, Pal [cs_Color_Brown_D]);
  txtmgr->FindRGB (176, 132,  44, Pal [cs_Color_Brown_M]);
  txtmgr->FindRGB (255, 192,  64, Pal [cs_Color_Brown_L]);
}

int csApp::FindColor (int r, int g, int b)
{
  int color;
  ITextureManager *txtmgr;
  System->piG3D->GetTextureManager (&txtmgr);
  txtmgr->FindRGB (r, g, b, color);
  return (color >> PhysColorShift) | 0x80000000;
}

void csApp::Process ()
{
  bool did_some_work = false;
  csEvent *ev;

  // Broadcast a "pre-process" event
  {
    csEvent ev (0, csevBroadcast, cscmdPreProcess);
    HandleEvent (ev);
  }

  while ((ev = (EventQueue->Get ())))
  {
    did_some_work = true;
    if (!PreHandleEvent (*ev))
      if (!HandleEvent (*ev))
        PostHandleEvent (*ev);
    CHK (delete ev);
  }

  // Broadcast a "post-process" event
  {
    csEvent ev (0, csevBroadcast, cscmdPostProcess);
    HandleEvent (ev);
  }

  // Redraw all changed windows
  while (RedrawFlag)
  {
    did_some_work = true;
    RedrawFlag = false;
    csEvent ev (0, csevBroadcast, cscmdRedraw);
    HandleEvent (ev);
  }

  // If event queue is empty, sleep for one timeslice
  if (!did_some_work)
    Idle ();
}

void csApp::Idle ()
{
  System->Sleep (1);
}

bool csApp::PreHandleEvent (csEvent &Event)
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

bool csApp::PostHandleEvent (csEvent &Event)
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

bool csApp::HandleEvent (csEvent &Event)
{
  // Mouse should always receive events
  // to reflect current mouse position
  Mouse->HandleEvent (Event);

  if ((Event.Type == csevKeyDown)
   && (Event.Key.Code == CSKEY_INS))
    insert = !insert;

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
   && (((System->Mouse->Button[1] && System->Mouse->Button[2]))
    || (Event.Mouse.Button == 3)))
  {
    WindowList ();
    return true;
  } /* endif */

  return false;
}

void csApp::WindowList ()
{
  CHK (csWindowList *wl = new csWindowList (this));
  int x, y;
  GetMouse ()->GetPosition (x, y);
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
      Box (0, 0, bound.Width (), bound.Height (), CSPAL_APP_WORKSPACE);
      break;
  } /* endswitch */
  csComponent::Draw ();
}

bool csApp::GetKeyState (int iKey)
{
  switch (iKey)
  {
    case CSKEY_ESC:       return System->Keyboard->Key.esc;
    case CSKEY_ENTER:     return System->Keyboard->Key.enter;
    case CSKEY_TAB:       return System->Keyboard->Key.tab;
    case CSKEY_BACKSPACE: return System->Keyboard->Key.backspace;
    case CSKEY_UP:        return System->Keyboard->Key.up;
    case CSKEY_DOWN:      return System->Keyboard->Key.down;
    case CSKEY_LEFT:      return System->Keyboard->Key.left;
    case CSKEY_RIGHT:     return System->Keyboard->Key.right;
    case CSKEY_PGUP:      return System->Keyboard->Key.pgup;
    case CSKEY_PGDN:      return System->Keyboard->Key.pgdn;
    case CSKEY_HOME:      return System->Keyboard->Key.home;
    case CSKEY_END:       return System->Keyboard->Key.end;
    case CSKEY_INS:       return System->Keyboard->Key.ins;
    case CSKEY_DEL:       return System->Keyboard->Key.del;
    case CSKEY_CTRL:      return System->Keyboard->Key.ctrl;
    case CSKEY_ALT:       return System->Keyboard->Key.alt;
    case CSKEY_SHIFT:     return System->Keyboard->Key.shift;
  }
  return false;
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

int csApp::Execute (csComponent *comp)
{
  csComponent *oldFocusOwner = CaptureFocus (comp);
  bool oldExitLoop = System->ExitLoop;
  System->ExitLoop = false;

  comp->Select ();
  comp->SetState (CSS_MODAL, true);
  LoopLevel++;
  System->Loop ();
  System->ExitLoop = oldExitLoop;
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
    System->ExitLoop = true;
  } /* endif */
}
