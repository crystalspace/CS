/*
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
*/

#define SYSDEF_PATH
#include "cssysdef.h"
#include "cssys/system.h"
#include "csws/csws.h"

// We need the Virtual File System plugin
REGISTER_STATIC_CLASS (csVFS, "crystalspace.kernel.vfs",
  "Crystal Space Virtual File System plug-in")

class csWsTest : public csApp
{
  void NotebookDialog ();
  void GridDialog ();
  void TreeDialog ();

public:
  /// Initialize maze editor
  csWsTest (char *AppTitle);

  ///
  virtual bool HandleEvent (csEvent &Event);

  virtual bool InitialSetup (int argc, const char* const argv[],
    const char *ConfigName, const char* DataDir);
};

csWsTest *app;                        // The main Windowing System object

//-----------------------------------------------------------------------------
class csGridHeaderCell : public csGridCell
{
public:
  csGridHeaderCell (){};
  void Draw (){
    // Draw the column number in the cell canvas
    //    printf("ping\n");
    csGridCell::Draw();
    char tt[20];
    sprintf( tt, "%d", col);
    int tx = (bound.Width () - TextWidth (tt)) /2;
    int ty = (bound.Height () - TextHeight ()) /2;
    Text (tx, ty, CSPAL_GRIDCELL_DATA_FG, CSPAL_GRIDCELL_DATA_BG, tt );
  }
};

//-----------------------------------------------------------------------------

class cspExtDialog : public csDialog
{
public:
  cspExtDialog (csComponent *iParent) : csDialog (iParent)
  { SetColor (CSPAL_DIALOG_BACKGROUND, cs_Color_Brown_L); }
  virtual bool HandleEvent (csEvent &Event)
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

// Scroll bar class default palette
static int palette_csWsTest[] =
{
  cs_Color_Gray_D,			// Application workspace
  cs_Color_Green_L,			// End points
  cs_Color_Red_L,			// lines
  cs_Color_White			// Start points
};

csWsTest::csWsTest (char *AppTitle) : csApp (AppTitle)
{
  SetPalette (palette_csWsTest, sizeof (palette_csWsTest) / sizeof (int));
}

bool csWsTest::InitialSetup (int argc, const char* const argv[],
  const char *ConfigName, const char* DataDir)
{
  if (!csApp::InitialSetup (argc, argv, ConfigName, DataDir))
    return false;

  printf (MSG_INITIALIZATION, "Crystal Space Windowing System test version %s (%s).\n", VERSION, RELEASE_DATE);
  printf (MSG_INITIALIZATION, "Created by Andrew Zabolotny and others...\n\n");

  // create a window
  csComponent *window = new csWindow (this, "-- Drag me --",
    CSWS_DEFAULTVALUE | CSWS_TOOLBAR | CSWS_CLIENTBORDER);

  csMenu *menu = (csMenu *)window->GetChild (CSWID_MENUBAR);
  if (menu)
  {
    menu->SetFont (csFontCourier);
    csMenu *submenu;

    submenu = new csMenu (NULL);
    (void)new csMenuItem (menu, "~File", submenu);
      (void)new csMenuItem (submenu, "~Open\tCtrl+O", cscmdNothing);
      (void)new csMenuItem (submenu, "~Save\tCtrl+S", cscmdNothing);
      (void)new csMenuItem (submenu, "~Close", cscmdNothing);
      (void)new csMenuItem (submenu);
      (void)new csMenuItem (submenu, "~Quit\tCtrl+Q", cscmdQuit);

    submenu = new csMenu (NULL);
    (void)new csMenuItem (menu, "~Edit", submenu);
      (void)new csMenuItem (submenu, "~Undo\tAlt+BackSpace", cscmdNothing);
      (void)new csMenuItem (submenu);
      (void)new csMenuItem (submenu, "~Copy\tCtrl+Ins", cscmdNothing);
      (void)new csMenuItem (submenu, "Cu~t\tShift+Del", cscmdNothing);
      (void)new csMenuItem (submenu, "~Paste\tShift+Ins", cscmdNothing);
      (void)new csMenuItem (submenu);
      (void)new csMenuItem (submenu, "~Select all\tCtrl+/", cscmdNothing);
      (void)new csMenuItem (submenu, "~Deselect all\tCtrl+\\", cscmdNothing);

    submenu = new csMenu (NULL);
    (void)new csMenuItem (menu, "~Modify", submenu);
     (void)new csMenuItem (submenu, "~Scale", cscmdNothing);
      (void)new csMenuItem (submenu, "~Rotate", cscmdNothing);
      (void)new csMenuItem (submenu, "~Move", cscmdNothing);

    submenu = new csMenu (NULL);
    (void)new csMenuItem (menu, "~Test submenu", submenu);
      (void)new csMenuItem (submenu, "~One", cscmdNothing, CSMIS_DEFAULTVALUE | CSMIS_CHECKED);
      (void)new csMenuItem (submenu, "~Two", cscmdNothing);
      (void)new csMenuItem (submenu, "T~hree", cscmdNothing);
      csMenu *subsubmenu = new csMenu (NULL);
      (void)new csMenuItem (submenu, "~Four", subsubmenu);
        (void)new csMenuItem (subsubmenu, "Four / ~One", cscmdNothing);
        (void)new csMenuItem (subsubmenu, "Four / ~Two", cscmdNothing);
        (void)new csMenuItem (subsubmenu, "Four / T~hree", cscmdNothing);
      subsubmenu = new csMenu (NULL);
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
        subsubmenu->GetChild (cscmdHide)->SendCommand (cscmdMenuItemCheck, (void *)true);
        subsubmenu->GetChild (cscmdQuit)->SendCommand (cscmdMenuItemCheck, (void *)true);
      (void)new csMenuItem (submenu, "~Six", cscmdNothing);
      (void)new csMenuItem (submenu, "~Seven", cscmdNothing);

    (void)new csMenuItem (menu, "~NoSubmenu");

    (void)new csMenuItem (menu, CSMIS_NEWCOLUMN);

    submenu = new csMenu (NULL);
    (void)new csMenuItem (menu, "~Help", submenu);
      (void)new csMenuItem (submenu, "~Index", cscmdNothing);
      (void)new csMenuItem (submenu, "~General", cscmdNothing);
      (void)new csMenuItem (submenu, "~Context\tF1", cscmdNothing);
      (void)new csMenuItem (submenu);
      (void)new csMenuItem (submenu, "~About", cscmdNothing);
      submenu->GetChild (cscmdNothing)->SendCommand (cscmdDeactivateMenu, (void *)false);
  }
  csDialog *toolbar = (csDialog *)window->GetChild (CSWID_TOOLBAR);
  if (toolbar)
  {
    csNewToolbarButton (toolbar, cscmdNothing, "hello");
    csNewToolbarButton (toolbar, cscmdNothing, "hello again");
    csNewToolbarButton (toolbar, cscmdNothing, "test");
    csNewToolbarButton (toolbar, cscmdNothing, "another test");
    csNewToolbarButton (toolbar, cscmdNothing, "yet another test");
  }
  window->SetRect (80, 60, 520, 380);

  csComponent *client = new csDialog (window);

  {
    csButton *but = new csButton (client, cscmdQuit);
    but->SetText ("~Quit!"); but->SetRect (20, 20, 90, 40);
    but->SetState (CSS_GROUP, true);
    csStatic *stat = new csStatic (client, but, "Test ~Label", csscsFrameLabel);
    stat->SetRect (10, 10, 420, 110);
    but = new csButton (client, cscmdNothing, CSBS_DEFAULTVALUE | CSBS_DEFAULT);
    but->SetText ("~Another button"); but->SetRect (50, 80, 180, 100);

    but = new csButton (client, cscmdNothing, CSBS_DEFAULTVALUE, csbfsThinRect);
    but->SetText ("hmm~..."); but->SetRect (20, 130, 100, 144);
    but->SetState (CSS_GROUP, true);
    stat = new csStatic (client, but, "Another ~Group", csscsFrameLabel);
    stat->SetRect (10, 110, 420, 270);
    but = new csButton (client, cscmdNothing, CSBS_DEFAULTVALUE, csbfsThickRect);
    but->SetText ("~whoops!"); but->SetRect (120, 130, 200, 144);
    but = new csButton (client, cscmdNothing, CSBS_DEFAULTVALUE, csbfsThinRect);
    but->SetText ("~crash!"); but->SetRect (220, 130, 300, 144);

    csInputLine *il = new csInputLine (client, 40, csifsThinRect);
    il->SetRect (100, 200, 300, 216);
    il->SetText ("input line test: check it out!");
    il = new csInputLine (client, 10, csifsThickRect);
    il->SetRect (100, 220, 300, 236);
    il->SetText ("another input line");
    il->SetFont (csFontCourier); il->SetSelection (0, 999);

    csListBox *lb = new csListBox (client, CSLBS_HSCROLL | CSLBS_VSCROLL, cslfsThinRect);
    lb->SetRect (320, 120, 410, 250);
    lb->SetFont (csFontCourier);
    for (int i = 1; i < 100; i++)
    {
      char tmp[20];
      sprintf (tmp, "item %d - dummy", i);
      (void)new csListBoxItem (lb, tmp, i);
    }

  }

  window = new csWindow (this, "SetState (CSS_TOPSELECT, false)",
    CSWS_DEFAULTVALUE, cswfsThin);
  window->SetState (CSS_TOPSELECT, false);
  window->SetRect (20, 20, 400, 80);

  {
    csButton *but = new csButton (this, 9999);
    but->SetText ("File dialog"); but->SetRect (10, 10, 100, 30);
    but->SetFont (csFontTiny);

    but = new csButton (this, 9998);
    but->SetText ("Color dialog"); but->SetRect (210, 10, 360, 30);

    but = new csButton (this, 9997);
    but->SetText ("Notebook"); but->SetRect (400, 15, 500, 35);

    but = new csButton (this, 9996);
    but->SetText ("Grid"); but->SetRect (400, 45, 500, 65);

    but = new csButton (this, 9995);
    but->SetText ("Tree"); but->SetRect (520, 45, 620, 65);
  }

  return true;
}

void csWsTest::TreeDialog ()
{
  csComponent *window = new csWindow (this, "Tree test",
    CSWS_BUTSYSMENU | CSWS_TITLEBAR | CSWS_BUTHIDE | CSWS_BUTCLOSE |
    CSWS_BUTMAXIMIZE | CSWS_TOOLBAR | CSWS_TBPOS_BOTTOM);
  window->SetSize (400, 300);
  window->Center ();

  csTreeCtrl *tc = new csTreeCtrl (window, CSTS_HSCROLL | CSTS_VSCROLL | CSTS_MULTIPLESEL, cstfsThinRect);
  //  tc->SetRect (320, 20, 410, 110);
  tc->SetFont (csFontCourier);
  csTreeItem *ti;
  csComponent *p = tc;
  ti = new csTreeItem (p, "CrystalSpace Platforms");
  
  csComponent *u = new csTreeItem (ti, "Unix");
  (void)new csTreeItem (u, "FreeBSD");
  csComponent *li = new csTreeItem (u, "Linux");
  csComponent *nx = new csTreeItem (u, "NeXT");
  (void)new csTreeItem (nx, "NextStep");
  (void)new csTreeItem (nx, "OpenStep");
  csComponent *ap = new csTreeItem (u, "Apple");
  (void)new csTreeItem (ap, "MacOS/X Server");
  (void)new csTreeItem (ap, "MacOS/X");
  (void)new csTreeItem (ap, "Darwin");
  (void)new csTreeItem (u, "Solaris");
  (void)new csTreeItem (u, "Cygnus");
  (void)new csTreeItem (ti, "Macintosh");
  (void)new csTreeItem (ti, "BeOS");
  (void)new csTreeItem (ti, "OS/2");
  (void)new csTreeItem (ti, "DJGPP");
  csComponent *w = new csTreeItem (ti, "Windows");
  csComponent *w32 = new csTreeItem (w, "Win32");
  (void)new csTreeItem (w32, "Win95");
  (void)new csTreeItem (w32, "Win98");
  (void)new csTreeItem (w32, "Win2000");
  (void)new csTreeItem (w32, "WinNT");

  csComponent *d3 = new csTreeItem (li, "3D Renderer");
  csComponent *sw = new csTreeItem (d3, "Software");
  (void)new csTreeItem (sw, "X-Windows");
  (void)new csTreeItem (sw, "SVGALIB console");
  (void)new csTreeItem (sw, "GGI console");
  csComponent *hw = new csTreeItem (d3, "Hardware");
  csComponent *gl = new csTreeItem (hw, "Glide/Voodoo");
  (void)new csTreeItem (gl, "Glide2");
  (void)new csTreeItem (gl, "Glide3");
  csComponent *ogl= new csTreeItem (hw, "OpenGL");
  (void)new csTreeItem (ogl, "Mesa");
  (void)new csTreeItem (ogl, "MGL");
  
  csComponent *sd = new csTreeItem (li, "SoundDriver");
  (void)new csTreeItem (sd, "OSS");
  
  //  ti->SetState (CSS_FOCUSED|CSS_TREEITEM_SELECTED, true);
  Execute (window);
  delete window;
}

void csWsTest::GridDialog ()
{
  csComponent *window = new csWindow (this, "Grid test",
    CSWS_BUTSYSMENU | CSWS_TITLEBAR | CSWS_BUTHIDE | CSWS_BUTCLOSE |
    CSWS_BUTMAXIMIZE | CSWS_TOOLBAR | CSWS_TBPOS_BOTTOM);
  window->SetSize (400, 300);
  window->Center ();

  // say how the cell should look initially
  csGridCell *gc = new csGridCell;
  gc->SetRect (0, 0, 50, 30);

  csGrid *grid = new csGrid (window, 20000, 20000, CSGVS_DEFAULTVALUE|CSGS_VSPLIT|CSGS_HSPLIT, gc);

  // create a subregion that looks different
  gc = new csGridCell;
  gc->SetColor (CSPAL_GRIDCELL_BACKGROUND, cs_Color_Brown_L);
  gc->SetRect (0, 0, 50, 30);
  gc->right.style = gc->left.style = GCBS_NONE;
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

  Execute (window);
  delete window;
}

void csWsTest::NotebookDialog ()
{
  // create a window
  csComponent *window = new csWindow (this, "Notebook test",
    CSWS_BUTSYSMENU | CSWS_TITLEBAR | CSWS_BUTHIDE | CSWS_BUTCLOSE |
    CSWS_BUTMAXIMIZE | CSWS_TOOLBAR | CSWS_TBPOS_BOTTOM);
  window->SetSize (400, 300);
  window->Center ();

  csDialog *toolbar = (csDialog *)window->GetChild (CSWID_TOOLBAR);
  if (toolbar)
  {
    toolbar->SetFrameStyle (csdfsNone);
    csNewToolbarButton (toolbar, cscmdOK, "~Ok");
    csNewToolbarButton (toolbar, cscmdCancel, "Cancel");
  }

  // Now create the notebook window
  csNotebook *nb = new csNotebook (window, CSNBS_TABPOS_TOP);
  window->SendCommand (cscmdWindowSetClient, (void *)nb);

  csComponent *page = new cspExtDialog (nb);
  nb->AddPrimaryTab (page, "~Style", "Change notebook style");

//------------------------------
  int y = 10;
  csRadioButton *rb = new csRadioButton (page, 9990);
  rb->SetPos (5, y); rb->SetState (CSS_GROUP, true);
  rb->SendCommand (cscmdRadioButtonSet, (void *)true);
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
  nb->AddPrimaryTab (page, new csPixmap (tex, 16, 0, 16, 13),
    true, "Page four");

  Execute (window);
  delete window;
}

bool csWsTest::HandleEvent (csEvent &Event)
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
        case 9999:
        {
          csWindow *d = csFileDialog (this, "test file dialog");
          if (d)
          {
            if (Execute (d) == cscmdCancel)
            {
              delete d;
              return true;
            }
            char filename [MAXPATHLEN + 1];
            csQueryFileDialog (d, filename, sizeof (filename));
            delete d;
            csMessageBox (app, "Result", filename);
          }
          return true;
        }
        case 9998:
        {
          csWindow *d = csColorDialog (this, "test color dialog");
          if (d)
          {
            if (Execute (d) == cscmdCancel)
            {
              delete d;
              return true;
            }
            int color;
            csQueryColorDialog (d, color);
            delete d;
            char buff [100];
            sprintf (buff, "color value: %08X\n", color);
            csMessageBox (app, "Result", buff);
          }
          return true;
        }
        case 9997:
        {
          NotebookDialog ();
          return true;
        }
        case 9996:
        {
          GridDialog ();
          return true;
        }
        case 9995:
        {
          TreeDialog ();
          return true;
        }
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

//---------------------------------------------------------------------------

/*
 * Main function
 */
int main (int argc, char* argv[])
{
  app = new csWsTest ("Crystal Space 3D maze editor");

  if (app->InitialSetup (argc, argv, "/config/MazeD.cfg", "/lib/MazeD"))
  {
    app->Loop ();
  }

  delete app;

  return (0);
}
