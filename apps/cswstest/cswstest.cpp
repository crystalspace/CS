/*
    Copyright (C) 2001 by Jorrit Tyberghein
    Copyright (C) 1999 by Andrew Zabolotny <bit@eltech.ru>

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


    cswstest - This is the main test application for the csws windowing
    system.
*/

#include "cssysdef.h"
#include "csws/csws.h"
#include "csver.h"
#include "ivideo/fontserv.h"
#include "iutil/cfgmgr.h"
#include "iutil/cmdline.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"
#include "cstool/initapp.h"
#include "csutil/cmdhelp.h"

CS_IMPLEMENT_APPLICATION

class csWsTest : public csApp
{
  void MiscDialog ();
  void TranspDialog ();
  void NotebookDialog ();
  void GridDialog ();
  void TreeDialog ();
  void LayoutDialog ();

public:
  /// Initialize maze editor
  csWsTest (iObjectRegistry *object_reg, csSkin &Skin);

  /// Initialize maze editor
  virtual ~csWsTest ();

  ///
  virtual bool HandleEvent (iEvent &Event);

  virtual bool Initialize ();

  csRef<iFont> VeraFont8;
  csRef<iFont> VeraFont12;
};

//csWsTest *cswstest_app;                        // The main Windowing System object

//-----------------------------------------------------------------------------
class csGridHeaderCell : public csGridCell
{
public:
  virtual void Draw ()
  {
    // Draw the column number in the cell canvas
    csGridCell::Draw ();
    char tt[20];
    sprintf (tt, "%d", col);
    int fh, fw = GetTextSize (tt, &fh);
    int tx = (bound.Width () - fw) /2;
    int ty = (bound.Height () - fh) /2;
    Text (tx, ty, CSPAL_GRIDCELL_DATA_FG, CSPAL_GRIDCELL_DATA_BG, tt );
  }
};

//-----------------------------------------------------------------------------

class cspExtDialog : public csDialog
{
public:
  cspExtDialog (csComponent *iParent) : csDialog (iParent)
  { SetColor (CSPAL_DIALOG_BACKGROUND, cs_Color_Brown_L); }
  virtual bool HandleEvent (iEvent &Event)
  {
    csNotebook *nb = (csNotebook *)parent;
    if (Event.Type == csevCommand)
      switch (Event.Command.Code)
      {
        case cscmdRadioButtonSelected:
          switch (((csRadioButton *)Event.Command.Info)->id)
          {
            case 9990:
              nb->SetStyle ((nb->GetStyle () & ~CSNBS_TABPOS_MASK) | CSNBS_TABPOS_TOP);
              break;
            case 9991:
              nb->SetStyle ((nb->GetStyle () & ~CSNBS_TABPOS_MASK) | CSNBS_TABPOS_BOTTOM);
              break;
            case 9992:
              nb->SetStyle ((nb->GetStyle () & ~CSNBS_TABPOS_MASK) | CSNBS_TABPOS_LEFT);
              break;
            case 9993:
              nb->SetStyle ((nb->GetStyle () & ~CSNBS_TABPOS_MASK) | CSNBS_TABPOS_RIGHT);
              break;
          }
          break;
        case cscmdCheckBoxSwitched:
        {
          csCheckBox *cb = (csCheckBox *)Event.Command.Info;
          int mask = (cb->id == 9980) ? CSNBS_PAGEFRAME :
                     (cb->id == 9981) ? CSNBS_PAGEINFO :
                     (cb->id == 9982) ? CSNBS_THINTABS : 0;
          if (mask)
          {
            int style = nb->GetStyle ();
            if (cb->SendCommand (cscmdCheckBoxQuery))
              style |= mask;
            else
              style &= ~mask;
            nb->SetStyle (style);
          }
          return true;
        }
      }
    return csDialog::HandleEvent (Event);
  }
};

//-----------------------------------------------------------------------------

class csThemeTestWindow : public csWindow
{
protected:
  csWindow *colordialog;
public:
  csThemeTestWindow (csComponent *iParent,char * iTitle,int iWindowStyle)
   : csWindow (iParent, iTitle, iWindowStyle) {}
  virtual void Draw ()
  {
    csWindow::Draw();
    SetFont (((csWsTest *)app)->VeraFont12);
    Text (BorderWidth+8, BorderHeight+100, CSPAL_WINDOW_LIGHT3D, -1,
      "This is a font 12 test");
    SetFont (((csWsTest *)app)->VeraFont8);
    Text (BorderWidth+8, BorderHeight+140, CSPAL_WINDOW_LIGHT3D, -1,
      "This is a font 8 test");
  }
};

//-----------------------------------------------------------------------------

// Scroll bar class default palette
static int palette_csWsTest[] =
{
  cs_Color_Gray_D,			// Application workspace
  cs_Color_Green_L,			// End points
  cs_Color_Red_L,			// lines
  cs_Color_White			// Start points
};

csWsTest::csWsTest (iObjectRegistry *object_reg, csSkin &Skin)
	: csApp (object_reg, Skin)
{
  int pal = csRegisterPalette (palette_csWsTest, sizeof (palette_csWsTest) / sizeof (int));
  SetPalette (pal);
}

csWsTest::~csWsTest ()
{
  csResetPalette();
}

bool csWsTest::Initialize ()
{
  if (!csApp::Initialize ())
    return false;

  VeraFont8 = LoadFont ("VeraSans", 8);
  VeraFont12 = LoadFont ("VeraSans", 12);
  DefaultFontSize=14;
  //  LucidiaFont = LoadFont ("/fonts/LucidiaTypewriterRegular.ttf");
  
  // CSWS apps are a lot more performant with a single-buffered canvas
  GetG2D ()->DoubleBuffer (false);

  csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,
    "crystalspace.application.cswstest",
    "Crystal Space Windowing System test version %s (%s).\n"
    "Created by Andrew Zabolotny and others...\n\n",
    CS_VERSION, CS_RELEASE_DATE);

#if 0
  //@@ A small group of controls for fine-level debugging
  {
    csWindow *window = new csWindow (this, "-- Drag me --", 0/*CSWS_TITLEBAR*/);
    window->SetAlpha (160);
    window->SetRect (100, 100, 300, 140);

    csButton *but = new csButton (window, cscmdNothing, CSBS_DEFAULTVALUE, csbfsThickRect);
    but->SetText ("-* test *-"); but->SetRect (40, 30, 140, 60);

    window = new csWindow (this, "-- help me --", 0/*CSWS_TITLEBAR*/);
    window->SetState (CSS_TOPSELECT, false);
//  window->SetAlpha (160);
    window->SetRect (200, 200, 500, 400);

    but = new csButton (this, cscmdNothing, CSBS_DEFAULTVALUE, csbfsThickRect);
    but->SetText ("Button one"); but->SetRect (10, 10, 100, 30);

    but = new csButton (this, cscmdNothing, CSBS_DEFAULTVALUE, csbfsThickRect);
    but->SetText ("Button two"); but->SetRect (210, 10, 300, 30);
    return true;
  }
#endif

  // Create a menu for all test dialogs we implement
  csMenu *menu = new csMenu (this, csmfs3D, 0);
  csMenu *submenu = new csMenu (0);
  (void)new csMenuItem (menu, "~Standard dialogs", submenu);
    (void)new csMenuItem (submenu, "~File open", 66600);
    (void)new csMenuItem (submenu, "~Color choice", 66601);

  submenu = new csMenu (0);
  (void)new csMenuItem (menu, "~Windows", submenu);
    (void)new csMenuItem (submenu, "No top when selected", 66610);
    (void)new csMenuItem (submenu, "Complex dialog", 66611);
    (void)new csMenuItem (submenu, "Semi-transparent window", 66612);

  submenu = new csMenu (0);
  (void)new csMenuItem (menu, "~Tests", submenu);
    (void)new csMenuItem (submenu, "Notebook", 66620);
    (void)new csMenuItem (submenu, "Grid", 66621);
    (void)new csMenuItem (submenu, "Tree", 66622);
    (void)new csMenuItem (submenu, "Layouts", 66623);

  submenu = new csMenu (0);
  (void)new csMenuItem (menu, "~Color scheme", submenu);
    (void)new csMenuItem (submenu, "Green", 66630);
    (void)new csMenuItem (submenu, "Red", 66631);
    (void)new csMenuItem (submenu, "White", 66632);
    (void)new csMenuItem (submenu, "Black", 66633);
    (void)new csMenuItem (submenu);
    (void)new csMenuItem (submenu, "Default", 66639);

  (void)new csMenuItem (menu);
  (void)new csMenuItem (menu, "~Window list", 66699);
  csMenuItem *mi = new csMenuItem (menu, "~Quit\tQ", cscmdQuit);

  // Show that a hint can be added to absolutely any component
  HintAdd ("Choose this menu item to quit the program", mi);

  menu->SetPos (30, 30);

  csKeyboardAccelerator *ka = new csKeyboardAccelerator (this);
  ka->Command ('q', CSMASK_SHIFT, cscmdQuit);

  return true;
}

void csWsTest::MiscDialog ()
{
  // create a window
  csComponent *window = new csWindow (this, "-- Drag me --",
    CSWS_DEFAULTVALUE | CSWS_TOOLBAR | CSWS_CLIENTBORDER);

  csMenu *menu = (csMenu *)window->GetChild (CSWID_MENUBAR);
  if (menu)
  {
    csMenu *submenu;

    submenu = new csMenu (0);
    (void)new csMenuItem (menu, "~File", submenu);
      (void)new csMenuItem (submenu, "~Open\tCtrl+O", cscmdNothing);
      (void)new csMenuItem (submenu, "~Save\tCtrl+S", cscmdNothing);
      (void)new csMenuItem (submenu, "~Close", cscmdNothing);
      (void)new csMenuItem (submenu);
      (void)new csMenuItem (submenu, "~Quit\tCtrl+Q", cscmdQuit);

    submenu = new csMenu (0);
    (void)new csMenuItem (menu, "~Edit", submenu);
      (void)new csMenuItem (submenu, "~Undo\tAlt+BackSpace", cscmdNothing);
      (void)new csMenuItem (submenu);
      (void)new csMenuItem (submenu, "~Copy\tCtrl+Ins", cscmdNothing);
      (void)new csMenuItem (submenu, "Cu~t\tShift+Del", cscmdNothing);
      (void)new csMenuItem (submenu, "~Paste\tShift+Ins", cscmdNothing);
      (void)new csMenuItem (submenu);
      (void)new csMenuItem (submenu, "~Select all\tCtrl+/", cscmdNothing);
      (void)new csMenuItem (submenu, "~Deselect all\tCtrl+\\", cscmdNothing);

    submenu = new csMenu (0);
    (void)new csMenuItem (menu, "~Modify", submenu);
     (void)new csMenuItem (submenu, "~Scale", cscmdNothing);
      (void)new csMenuItem (submenu, "~Rotate", cscmdNothing);
      (void)new csMenuItem (submenu, "~Move", cscmdNothing);

    submenu = new csMenu (0);
    (void)new csMenuItem (menu, "~Test submenu", submenu);
      (void)new csMenuItem (submenu, "~One", cscmdNothing, CSMIS_DEFAULTVALUE | CSMIS_CHECKED);
      (void)new csMenuItem (submenu, "~Two", cscmdNothing);
      (void)new csMenuItem (submenu, "T~hree", cscmdNothing);
      csMenu *subsubmenu = new csMenu (0);
      (void)new csMenuItem (submenu, "~Four", subsubmenu);
        (void)new csMenuItem (subsubmenu, "Four / ~One", cscmdNothing);
        (void)new csMenuItem (subsubmenu, "Four / ~Two", cscmdNothing);
        (void)new csMenuItem (subsubmenu, "Four / T~hree", cscmdNothing);
      subsubmenu = new csMenu (0);
      (void)new csMenuItem (submenu, "Fi~ve", subsubmenu, CSMIS_NEWCOLUMN);
        (void)new csMenuItem (subsubmenu, "Five / ~One", cscmdNothing);
        (void)new csMenuItem (subsubmenu, "Five / ~Two", cscmdNothing);
        (void)new csMenuItem (subsubmenu, "Five / T~hree", cscmdNothing);
        (void)new csMenuItem (subsubmenu);
        (void)new csMenuItem (subsubmenu, "~Maximize", cscmdMaximize, CSMIS_DEFAULTVALUE | CSMIS_NOCLOSE);
        (void)new csMenuItem (subsubmenu, "~Hide", cscmdHide);
        (void)new csMenuItem (subsubmenu, "~Close", cscmdClose);
        (void)new csMenuItem (subsubmenu);
        (void)new csMenuItem (subsubmenu, "~Quit\tCtrl+Q", cscmdQuit);
        subsubmenu->GetChild (cscmdHide)->SendCommand (cscmdMenuItemCheck, true);
        subsubmenu->GetChild (cscmdQuit)->SendCommand (cscmdMenuItemCheck, true);
      (void)new csMenuItem (submenu, "~Six", cscmdNothing);
      (void)new csMenuItem (submenu, "~Seven", cscmdNothing);

    (void)new csMenuItem (menu, "~NoSubmenu");

    (void)new csMenuItem (menu, CSMIS_NEWCOLUMN);

    submenu = new csMenu (0);
    (void)new csMenuItem (menu, "~Help", submenu);
      (void)new csMenuItem (submenu, "~Index", cscmdNothing);
      (void)new csMenuItem (submenu, "~General", cscmdNothing);
      (void)new csMenuItem (submenu, "~Context\tF1", cscmdNothing);
      (void)new csMenuItem (submenu);
      (void)new csMenuItem (submenu, "~About", cscmdNothing);
      submenu->GetChild (cscmdNothing)->SendCommand (cscmdDeactivateMenu, false);
  }
  csDialog *toolbar = (csDialog *)window->GetChild (CSWID_TOOLBAR);
  if (toolbar)
  {
    HintAdd ("This is the hello button",
      csNewToolbarButton (toolbar, cscmdNothing, "hello"));
    HintAdd ("This is the hello again button",
      csNewToolbarButton (toolbar, cscmdNothing, "hello again"));
    HintAdd ("This is a test button on the toolbar with an associated hint",
      csNewToolbarButton (toolbar, cscmdNothing, "test"));
    HintAdd ("This is the another test button",
      csNewToolbarButton (toolbar, cscmdNothing, "another test"));
    HintAdd ("This is the yet another test button",
      csNewToolbarButton (toolbar, cscmdNothing, "yet another test"));
  }
  window->SetRect (80, 60, 520, 380);

  csComponent *client = new csDialog (window);

  {
    csButton *but = new csButton (client, cscmdQuit);
    but->SetText ("~Quit!"); but->SetRect (20, 20, 90, 40);
    but->SetState (CSS_GROUP, true);
    HintAdd ("Press this button to quit program", but);
    csStatic *stat = new csStatic (client, but, "Test ~Label", csscsFrameLabel);
    stat->SetRect (10, 10, 420, 110);
    but = new csButton (client, cscmdNothing, CSBS_DEFAULTVALUE | CSBS_DEFAULT);
    but->SetText ("~Another button"); but->SetRect (50, 80, 180, 100);

    csSplitter *splitter = new csSplitter (client);
    splitter->SetRect (200, 20, 400, 40);

    but = new csButton (client, cscmdNothing, CSBS_DEFAULTVALUE, csbfsThinRect);
    but->SetText ("hmm~..."); but->SetRect (20, 130, 100, 144);
    but->SetState (CSS_GROUP, true);
    stat = new csStatic (client, but, "Another ~Group", csscsFrameLabel);
    stat->SetRect (10, 110, 420, 270);
    but = new csButton (client, cscmdNothing, CSBS_DEFAULTVALUE, csbfsThickRect);
    but->SetText ("~whoops!"); but->SetRect (120, 130, 200, 144);
    but = new csButton (client, cscmdNothing, CSBS_DEFAULTVALUE, csbfsThinRect);
    but->SetText ("~crash!"); but->SetRect (220, 130, 300, 144);
    HintAdd ("Relax, this button won't crash your HD", but);

    csInputLine *il = new csInputLine (client, 40, csifsThinRect);
    il->SetRect (100, 200, 300, 216);
    il->SetText ("input line test: check it out!");
    il = new csInputLine (client, 10, csifsThickRect);
    il->SetRect (100, 220, 300, 236);
    il->SetText ("another input line");
    il->SetSelection (0, 999);

    csListBox *lb = new csListBox (client, CSLBS_HSCROLL | CSLBS_VSCROLL, cslfsThinRect);
    lb->SetRect (320, 120, 410, 250);
	int i;
    for (i = 1; i < 100; i++)
    {
      char tmp[20];
      sprintf (tmp, "item %d - dummy", i);
      (void)new csListBoxItem (lb, tmp, i);
    }
  }
}

class csTreeDialog : public csDialog
{
  csTreeBox *tb;
public:
  csTreeDialog (csComponent *parent) : csDialog (parent) {}
  void SetTreeBox (csTreeBox *iTB) { tb = iTB; }
  virtual bool SetRect (int xmin, int ymin, int xmax, int ymax)
  {
    if (!csDialog::SetRect (xmin, ymin, xmax, ymax))
      return false;
    tb->SetRect (0, 68, bound.Width (), bound.Height ());
    return true;
  }
  virtual bool HandleEvent (iEvent &Event)
  {
    switch (Event.Type)
    {
      case csevCommand:
        switch (Event.Command.Code)
        {
          case cscmdRadioButtonSelected:
          {
            csRadioButton *rb = (csRadioButton *)Event.Command.Info;
            tb->SetStyle (tb->GetStyle (), (rb->id == 9900) ? cstfsNone :
              (rb->id == 9901) ? cstfsThinRect :
              (rb->id == 9902) ? cstfsThickRect : cstfsNone);
            break;
          }
          case cscmdCheckBoxSwitched:
          {
            csCheckBox *cb = (csCheckBox *)Event.Command.Info;
            int mask = (cb->id == 9910) ? CSTS_HSCROLL :
                       (cb->id == 9911) ? CSTS_VSCROLL :
                       (cb->id == 9912) ? CSTS_AUTOSCROLLBAR :
                       (cb->id == 9913) ? CSTS_SMALLBUTTONS : 0;
            int style = tb->GetStyle ();
            if (cb->SendCommand (cscmdCheckBoxQuery))
              style |= mask;
            else
              style &= ~mask;
            tb->SetStyle (style, tb->GetFrameStyle ());
            break;
          }
        }
      break;
    }
    return csDialog::HandleEvent (Event);
  }
};

void csWsTest::TreeDialog ()
{
  csComponent *window = new csWindow (this, "Tree test",
    CSWS_BUTSYSMENU | CSWS_TITLEBAR | CSWS_BUTHIDE | CSWS_BUTCLOSE |
    CSWS_BUTMAXIMIZE);

  csTreeDialog *d = new csTreeDialog (window);
  window->SendCommand (cscmdWindowSetClient, (intptr_t)d);

  // Begin tree control creation

  csTreeBox *tc = new csTreeBox (d, CSTS_HSCROLL | CSTS_VSCROLL, cstfsThinRect);
  d->SetTreeBox (tc);
  tc->SetState (CSS_GROUP, true);

  window->SetSize (400, 300);
  window->Center ();

  csTreeItem *i1, *i2, *i3;
  i1 = new csTreeItem (tc, "Developement Environments");
    i2 = new csTreeItem (i1, "Free");
      (void)new csTreeItem (i2, "Linux");
      (void)new csTreeItem (i2, "FreeBSD");
    i2 = new csTreeItem (i1, "Sun");
      (void)new csTreeItem (i2, "Solaris");
    i2 = new csTreeItem (i1, "Silicon Graphics");
      (void)new csTreeItem (i2, "Irix");
    i2 = new csTreeItem (i1, "NeXT");
      (void)new csTreeItem (i2, "NextStep");
      (void)new csTreeItem (i2, "OpenStep");
    i2 = new csTreeItem (i1, "Apple");
      (void)new csTreeItem (i1, "MacOS/9");
      (void)new csTreeItem (i2, "MacOS/X Server");
      (void)new csTreeItem (i2, "MacOS/X");
      (void)new csTreeItem (i2, "Darwin");
    i2 = new csTreeItem (i1, "Microsoft");
      i3 = new csTreeItem (i2, "MS-DOS");
        (void)new csTreeItem (i2, "DJGPP");
      i3 = new csTreeItem (i2, "Win32");
        (void)new csTreeItem (i3, "MSVC");
        (void)new csTreeItem (i3, "Cygwin32");
        (void)new csTreeItem (i3, "MinGW32");
    i2 = new csTreeItem (i1, "BeOS");
    i2 = new csTreeItem (i1, "IBM");
      i3 = new csTreeItem (i2, "OS/2");
        (void)new csTreeItem (i3, "EMX+GCC");
        (void)new csTreeItem (i3, "Watcom C++");
      (void)new csTreeItem (i2, "AIX");
  i1 = new csTreeItem (tc, "Graphical environments");
    i2 = new csTreeItem (i1, "OpenGL");
      (void)new csTreeItem (i2, "glos2 (OpenGL/2 canvas)");
      (void)new csTreeItem (i2, "glx2d (GL/X)");
      (void)new csTreeItem (i2, "glwin32 (Windows OpenGL)");
      (void)new csTreeItem (i2, "glmac (MacOS/9 OpenGL)");
      (void)new csTreeItem (i2, "glbe (BeOS OpenGL)");
    i2 = new csTreeItem (i1, "Direct3D");
      (void)new csTreeItem (i2, "ddraw61 (Direct Draw 6.1)");
    i2 = new csTreeItem (i1, "X11");
      i3 = new csTreeItem (i2, "x2d (full-featured X11 driver)");
        (void)new csTreeItem (i3, "SHM");
        (void)new csTreeItem (i3, "XFree86 full-screen ext");
      (void)new csTreeItem (i2, "linex2d (debug: display just lines)");
    i2 = new csTreeItem (i1, "System-specific");
      (void)new csTreeItem (i2, "csdive (OS/2 Direct Interface to Video Extensions)");
      (void)new csTreeItem (i2, "Native NeXT graphics canvas");
      (void)new csTreeItem (i2, "Native BeOS graphics canvas");
      (void)new csTreeItem (i2, "Native MacOS graphics canvas");
      (void)new csTreeItem (i2, "dosraw (DOS with raw framebuffer access)");
    i2 = new csTreeItem (i1, "Cross-platform");
      (void)new csTreeItem (i2, "svgalib (Linux full-screen raw framebuffer access)");
      (void)new csTreeItem (i2, "asciiart (text-mode display)");
      (void)new csTreeItem (i2, "allegro (raw framebuffer access)");
      (void)new csTreeItem (i2, "mgl (uses the MGL library, full-screen w/2D accel)");
      (void)new csTreeItem (i2, "sdl (uses the SDL library)");
      (void)new csTreeItem (i2, "ggi (Unix using the GGI library)");
  // Add a bitmap to a tree item for testing
  iTextureHandle *tex = GetTexture ("csws::FileDialog");
  if (tex)
  {
    csPixmap *pix1 = new csSimplePixmap (tex,  0, 0, 16, 13);
    csPixmap *pix2 = new csSimplePixmap (tex, 16, 0, 16, 13);
    i1->SetBitmap (pix1, pix2, true);
  }

  // Insert some components to control tree style
  int y = 10;
  csRadioButton *rb = new csRadioButton (d, 9900);
  rb->SetPos (5, y); rb->SetState (CSS_GROUP, true);
  csStatic *st = new csStatic (d, rb, "cstfsNone");
  st->SetPos (21, y + 2);

  y += 14;
  rb = new csRadioButton (d, 9901);
  rb->SetPos (5, y);
  rb->SendCommand (cscmdRadioButtonSet, true);
  st = new csStatic (d, rb, "cstfsThinRect");
  st->SetPos (21, y + 2);

  y += 14;
  rb = new csRadioButton (d, 9902);
  rb->SetPos (5, y);
  st = new csStatic (d, rb, "cstfsThickRect");
  st->SetPos (21, y + 2);

  y = 10;
  csCheckBox *cb = new csCheckBox (d, 9910);
  cb->SetPos (165, y); cb->SetState (CSS_GROUP, true);
  cb->SendCommand (cscmdCheckBoxSet, true);
  st = new csStatic (d, cb, "CSTS_HSCROLL");
  st->SetPos (181, y + 2);

  y += 14;
  cb = new csCheckBox (d, 9911);
  cb->SetPos (165, y);
  cb->SendCommand (cscmdCheckBoxSet, true);
  st = new csStatic (d, cb, "CSTS_VSCROLL");
  st->SetPos (181, y + 2);

  y += 14;
  cb = new csCheckBox (d, 9912);
  cb->SetPos (165, y);
  st = new csStatic (d, cb, "CSTS_AUTOSCROLLBAR");
  st->SetPos (181, y + 2);

  y += 14;
  cb = new csCheckBox (d, 9913);
  cb->SetPos (165, y);
  st = new csStatic (d, cb, "CSTS_SMALLBUTTONS");
  st->SetPos (181, y + 2);
}

void csWsTest::GridDialog ()
{
  csComponent *window = new csWindow (this, "Grid test", CSWS_BUTSYSMENU |
    CSWS_TITLEBAR | CSWS_BUTHIDE | CSWS_BUTCLOSE | CSWS_BUTMAXIMIZE);
  window->SetSize (400, 300);
  window->Center ();

  // say how the cell should look initially
  csGridCell *gc = new csGridCell;
  gc->SetRect (0, 0, 50, 30);

  csGrid *grid = new csGrid (window, 1000, 1000, gc);

  // create a subregion that looks different
  gc = new csGridCell;
  gc->SetColor (CSPAL_GRIDCELL_BACKGROUND, cs_Color_Brown_L);
  gc->SetRect (0, 0, 50, 30);
  gc->right.style = gc->left.style = gcbsNone;
  csRect rc (3, 3, 10, 6);
  grid->CreateRegion (rc, gc);

  // because that was fun - make another subregion that crosses the previous
  gc = new csGridCell;
  gc->SetColor (CSPAL_GRIDCELL_BACKGROUND, cs_Color_Cyan_L);
  gc->SetRect (0, 0, 50, 30);
  rc.Set (5, 5, 8, 10);
  grid->CreateRegion (rc, gc);

  // make a column header row
  gc = new csGridHeaderCell;
  gc->SetColor (CSPAL_GRIDCELL_BACKGROUND, cs_Color_Gray_D);
  gc->SetColor (CSPAL_GRIDCELL_DATA_BG, cs_Color_Gray_D);
  gc->SetColor (CSPAL_GRIDCELL_DATA_FG, cs_Color_White);
  gc->SetRect (0, 0, 50, 20);
  rc.Set (0, 0, 20000, 1);
  grid->CreateRegion (rc, gc);

  // split the mainview and split the resulting new view again
  grid->GetRootView ()->SplitX (150)->SplitY (150);

  // show some important messages
  gc = new csGridCell;
  gc->SetRect (0, 0, 100, 30);
  rc.Set (2, 2, 3, 3);
  grid->CreateRegion (rc, gc);
  grid->SetStringAt (2, 2, "CS rocks");
  rc.Set (4, 5, 5, 6);
  grid->CreateRegion (rc, gc);
  grid->SetStringAt (5, 4, "HitSquad rulz");

  // make a single cell region thats blue and contains a button component
  rc.Set (2, 8, 3, 9);
  gc = new csGridCell;
  gc->SetColor (CSPAL_GRIDCELL_BACKGROUND, cs_Color_Blue_L);
  gc->SetRect (0, 0, 50, 30);
  csButton *but = new csButton (gc, cscmdNothing, CSBS_DEFAULTVALUE, csbfsThinRect);
  but->SetRect (5, 5, 45, 25);
  but->SetText ("blah");
  grid->CreateRegion (rc, gc);
}

void CreateButton (csComponent *parent, int id, const char *text, int xpos, int ypos)
{
  csButton *b= new csButton (parent, id);
  b->SetPos (xpos, ypos);
  b->SetSuggestedSize (0, 0);
  b->SetText (text);
}

void csWsTest::LayoutDialog ()
{
  csComponent *window = new csWindow (this, "Layout test",
    CSWS_DEFAULTVALUE & ~CSWS_MENUBAR);
  window->SetSize (400, 300);
  window->Center ();

  csGridBagLayout *gb = new csGridBagLayout (window);

  csBorderConstraint *blc[5] = {csBorderLayout::GetCenter (),
				csBorderLayout::GetEast (),
				csBorderLayout::GetNorth (),
				csBorderLayout::GetWest (),
				csBorderLayout::GetSouth ()};

  gb->c.fill = csGridBagConstraint::BOTH;
  gb->c.weightx = 1.0;
  CreateButton (gb, 7000, "test 1", 0, 20);
  CreateButton (gb, 7001, "test 2", 0, 20);
  CreateButton (gb, 7002, "test 3", 0, 20);
  gb->c.gridwidth = csGridBagConstraint::REMAINDER; //end row
  CreateButton (gb, 7003, "test 4", 0, 20);
  gb->c.weightx = 0.0;                  //reset to the default
  CreateButton (gb, 7004, "test 5", 0, 20);
  gb->c.gridwidth = csGridBagConstraint::RELATIVE; //next-to-last in row
  CreateButton (gb, 7005, "test 6", 0, 20);
  gb->c.gridwidth = csGridBagConstraint::REMAINDER; //end row
  gb->c.gridx = 2;                //reset to the default
  CreateButton (gb, 7006, "test 7", 0, 20);
  gb->c.gridx = csGridBagConstraint::RELATIVE;                //reset to the default
  gb->c.gridwidth = 1;                //reset to the default
  gb->c.gridheight = 2;
  gb->c.weighty = 1.0;
  CreateButton (gb, 7007, "test 8", 0, 20);
  gb->c.weighty = 0.0;                  //reset to the default
  gb->c.gridwidth = csGridBagConstraint::REMAINDER; //end row
  gb->c.gridheight = 1;               //reset to the default
  CreateButton (gb, 7008, "test 9", 0, 20);

  csBorderLayout *border = new csBorderLayout (gb);
  border->SetRect (0, 0, 50, 50);

  // a flow layout in the center of the borderlayout
  border->c = *blc[0];
  csFlowLayout *flow = new csFlowLayout (border);
  int k;
  for (k=0; k<10; k++)
  {
    char tt[20];
    sprintf (tt, "t %d", k);
    CreateButton (flow, 7200+k, tt, k*20, 20);
  }

  int j;
  for (j=1; j < 5; j++)
  {
    border->c = *blc[j];
    csButton *b= new csButton (border, 7100+j);
    b->SetPos ((9+j)*10, 20);
    b->SetSuggestedSize (0, 0);
    char text[20];
    sprintf (text, "Test %d", j);
    b->SetText (text);
  }
}

void csWsTest::TranspDialog ()
{
  // create a window
  csWindow *window = new csThemeTestWindow (this, "Theme test",
    CSWS_BUTSYSMENU | CSWS_TITLEBAR | CSWS_BUTCLOSE);
  window->SetSize (400, 300);
  window->Center ();
  window->SetAlpha (uint8 (0.5 * 255));
}

void csWsTest::NotebookDialog ()
{
  // create a window
  csComponent *window = new csWindow (this, "Notebook test",
    CSWS_BUTSYSMENU | CSWS_TITLEBAR | CSWS_BUTHIDE | CSWS_BUTCLOSE |
    CSWS_BUTMAXIMIZE);
  window->SetSize (400, 300);
  window->Center ();

  // Now create the notebook window
  csNotebook *nb = new csNotebook (window, CSNBS_TABPOS_TOP);
  window->SendCommand (cscmdWindowSetClient, (intptr_t)nb);

  csComponent *page = new cspExtDialog (nb);
  nb->AddPrimaryTab (page, "~Style", "Change notebook style");

//------------------------------
  int y = 10;
  csRadioButton *rb = new csRadioButton (page, 9990);
  rb->SetPos (5, y); rb->SetState (CSS_GROUP, true);
  rb->SendCommand (cscmdRadioButtonSet, true);
  csStatic *st = new csStatic (page, rb, "CSNBS_TABPOS_TOP");
  st->SetPos (21, y + 2);

  y += 14;
  rb = new csRadioButton (page, 9991);
  rb->SetPos (5, y);
  st = new csStatic (page, rb, "CSNBS_TABPOS_BOTTOM");
  st->SetPos (21, y + 2);

  y += 14;
  rb = new csRadioButton (page, 9992);
  rb->SetPos (5, y);
  st = new csStatic (page, rb, "CSNBS_TABPOS_LEFT");
  st->SetPos (21, y + 2);

  y += 14;
  rb = new csRadioButton (page, 9993);
  rb->SetPos (5, y);
  st = new csStatic (page, rb, "CSNBS_TABPOS_RIGHT");
  st->SetPos (21, y + 2);

  //---

  y += 20;
  csCheckBox *cb = new csCheckBox (page, 9980);
  cb->SetPos (5, y); cb->SetState (CSS_GROUP, true);
  st = new csStatic (page, cb, "CSNBS_PAGEFRAME");
  st->SetPos (21, y + 2);

  y += 14;
  cb = new csCheckBox (page, 9981);
  cb->SetPos (5, y);
  st = new csStatic (page, cb, "CSNBS_PAGEINFO");
  st->SetPos (21, y + 2);

  y += 14;
  cb = new csCheckBox (page, 9982);
  cb->SetPos (5, y);
  st = new csStatic (page, cb, "CSNBS_THINTABS");
  st->SetPos (21, y + 2);
//------------------------------

  page = new csDialog (nb);
  nb->AddPrimaryTab (page, "- ~2 -", "Page two");
  page->SetColor (CSPAL_DIALOG_BACKGROUND, cs_Color_Blue_D);

  page = new csDialog (nb);
  nb->AddSecondaryTab (page, "Page two subpage two");
  page->SetColor (CSPAL_DIALOG_BACKGROUND, cs_Color_Blue_M);

  page = new csDialog (nb);
  nb->AddSecondaryTab (page, "Page two subpage three");
  page->SetColor (CSPAL_DIALOG_BACKGROUND, cs_Color_Blue_L);

  page = new csDialog (nb);
  nb->AddPrimaryTab (page, "[-= ~3 =-]", "Page three");
  page->SetColor (CSPAL_DIALOG_BACKGROUND, cs_Color_Cyan_M);

  page = new csDialog (nb);
  iTextureHandle *tex = GetTexture ("csws::FileDialog");
  nb->AddPrimaryTab (page, new csSimplePixmap (tex, 16, 0, 16, 13),
    true, "Page four");
}

/*
 * This is data that we keep for modal processing.
 */
struct ModalData : public iBase
{
  uint code;
  csWindow* d;
  SCF_DECLARE_IBASE;
  ModalData () { SCF_CONSTRUCT_IBASE (0); }
  virtual ~ModalData () { SCF_DESTRUCT_IBASE (); }
};

SCF_IMPLEMENT_IBASE (ModalData)
SCF_IMPLEMENT_IBASE_END

bool csWsTest::HandleEvent (iEvent &Event)
{
  static csMouseCursorID mousecursors [] =
  {
    csmcNone, csmcArrow, csmcLens, csmcCross, csmcPen, csmcMove,
    csmcSizeNWSE, csmcSizeNESW, csmcSizeNS, csmcSizeEW, csmcStop, csmcWait
  };

  static int mousecursor = 1;
  static int px = -1, py;
  static bool draw = false;

  if (csApp::HandleEvent (Event))
    return true;

  switch (Event.Type)
  {
    case csevCommand:
      switch (Event.Command.Code)
      {
        case cscmdStopModal:
	{
	  csComponent* d = GetTopModalComponent ();
	  int rc = (int)Event.Command.Info;
	  if (rc == 0 || rc == cscmdCancel) { delete d; return true; }
	  csRef<iMessageBoxData> mbd (
	  	SCF_QUERY_INTERFACE (GetTopModalUserdata (), iMessageBoxData));
	  if (mbd) { delete d; return true; }

	  ModalData* data = (ModalData*)GetTopModalUserdata ();
	  CS_ASSERT (data != 0);
	    switch (data->code)
	    {
	      case 66600:
	      {
                char filename [CS_MAXPATHLEN + 1];
                csQueryFileDialog ((csWindow*)d, filename, sizeof (filename));
                csMessageBox (app, "Result", filename, 0);
	      }
	      break;
              case 66601:
	      {
                int color;
                csQueryColorDialog ((csWindow*)d, color);
                char buff [100];
                sprintf (buff, "color value: %08X\n", color);
                csMessageBox (app, "Result", buff, 0);
	      }
	      break;
	    }
	  delete d;
          return true;
	}
        case 66600:
        {
          csWindow* d = csFileDialog (this, "test file dialog");
	  ModalData* data = new ModalData ();
	  data->d = d;
	  data->code = Event.Command.Code;
	  StartModal (d, data);
	  data->DecRef ();
          return true;
        }
        case 66601:
        {
          csWindow *d = csColorDialog (this, "test color dialog");
	  ModalData* data = new ModalData ();
	  data->d = d;
	  data->code = Event.Command.Code;
	  StartModal (d, data);
	  data->DecRef ();
          return true;
        }
        case 66610:
        {
          csWindow *window = new csWindow (this, "SetState (CSS_TOPSELECT, false)",
            CSWS_DEFAULTVALUE, cswfsThin);
          window->SetState (CSS_TOPSELECT, false);
          window->SetRect (20, 20, 400, 80);
          return true;
        }
        case 66611:
          MiscDialog ();
          return true;
        case 66612:
          TranspDialog ();
          return true;
        case 66620:
          NotebookDialog ();
          return true;
        case 66621:
          GridDialog ();
          return true;
        case 66622:
          TreeDialog ();
          return true;
        case 66623:
          LayoutDialog ();
          return true;
        case 66630:
        case 66631:
        case 66632:
        case 66633:
        {
          static csColorScheme Schemes [4] =
          {
            { cs_Color_Green_D, 100, 0, 10 },
            { cs_Color_Red_D, 100, 0, 10 },
            { cs_Color_White, 100, -50, 10 },
            { cs_Color_Black, 100, -50, 10 },
          };
          csSetColorScheme (this, Schemes [Event.Command.Code - 66630]);
          return true;
	}
        case 66639:
          csSetColorScheme (this, *(csColorScheme *)0);
          return true;
        case 66699:
          WindowList ();
          return true;
      }
      break;

    case csevMouseMove:
      SetMouse (mousecursors [mousecursor]);
      if (draw)
      {
drawline:
        // kludge: set dirty rectangle so that line won't get clipped
        csRect old (dirty);
        dirty.Set (px, py, Event.Mouse.x, Event.Mouse.y);
        dirty.Normalize ();
        dirty.xmax++; dirty.ymax++;
        Line (px, py, Event.Mouse.x, Event.Mouse.y, 2);
        dirty.Set (old);

        px = Event.Mouse.x;
        py = Event.Mouse.y;
      } /* endif */
      return true;

    case csevMouseDown:
    case csevMouseDoubleClick:
      if (Event.Mouse.Button == 1)
      {
        if (Event.Mouse.Modifiers & CSMASK_CTRL)
          if (px >= 0 && py >= 0)
            goto drawline;

        draw = true;
        px = Event.Mouse.x;
        py = Event.Mouse.y;

        // kludge: set dirty rectangle so that line won't get clipped
        csRect old (dirty);
        dirty.Set (px - 1, py - 1, px + 2, py + 2);
        Box (px - 1, py - 1, px + 2, py + 2, 3);
        dirty.Set (old);
      }
      else if (Event.Mouse.Button == 2)
      {
        mousecursor = (mousecursor + 1) % (sizeof (mousecursors) / sizeof (int));
        SetMouse (mousecursors [mousecursor]);
      } /* endif */
      return true;

    case csevMouseUp:
    {
      // kludge: set dirty rectangle so that line won't get clipped
      csRect old (dirty);
      dirty.Set (px - 1, py - 1, px + 2, py + 2);
      Box (px - 1, py - 1, px + 2, py + 2, 1);
      dirty.Set (old);

      draw = false;
      return true;
    }
  } /* endswitch */

  return false;
}

// Define the skin for windowing system
CSWS_SKIN_DECLARE_DEFAULT (DefaultSkin);

/*
 * Main function
 */
int main (int argc, char* argv[])
{
  iObjectRegistry* object_reg = csInitializer::CreateEnvironment (argc, argv);
  if (!object_reg) return -1;

  if (!csInitializer::SetupConfigManager (object_reg, "/config/cswstest.cfg"))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.cswstest",
	"Can't initialize!");
    return -1;
  }

  if (!csInitializer::RequestPlugins (object_reg, CS_REQUEST_END))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.cswstest",
	"Can't initialize!");
    return -1;
  }

  // Check for commandline help.
  if (csCommandLineHelper::CheckHelp (object_reg))
  {
    csCommandLineHelper::Help (object_reg);
    exit (0);
  }

  if (!csInitializer::OpenApplication (object_reg))
  {
    csInitializer::DestroyApplication (object_reg);
    return -1;
  }

  csRef<iCommandLineParser> cmdline (CS_QUERY_REGISTRY (object_reg,
  	iCommandLineParser));
  csRef<iConfigManager> config (CS_QUERY_REGISTRY (object_reg, iConfigManager));

  // Look for skin variant from config file
  DefaultSkin.Prefix = cmdline->GetOption ("skin");
  if (!DefaultSkin.Prefix)
    DefaultSkin.Prefix = config->GetStr ("CSWS.Skin.Variant", 0);

  // Create our application object
  csWsTest *app = new csWsTest (object_reg, DefaultSkin);

  if (app->Initialize ())
    csDefaultRunLoop(object_reg);

  // Explicit release before DestroyApplication().
  cmdline = 0;
  config = 0;

  delete app;
  csInitializer::DestroyApplication (object_reg);
  return 0;
}

