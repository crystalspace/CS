/*
    Copyright (C) 1999 by Andrew Zabolotny <bit@eltech.ru>
    Copyright (C) 2001 by W.C.A. Wijngaards

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

#include "cssysdef.h"
#include "csws/csws.h"
#include "cstool/initapp.h"
#include "apps/tools/csfedit/csfedit.h"
#include "csver.h"
#include "ivideo/fontserv.h"
#include "ivideo/graph2d.h"
#include "ivideo/natwin.h"
#include "iutil/cfgmgr.h"
#include "ivaria/reporter.h"
#include "iutil/cmdline.h"
#include "iutil/objreg.h"
#include <sys/stat.h>
#include "csgeom/csrect.h"
#include "csutil/cmdhelp.h"
#include "csutil/util.h"
#include "csqsqrt.h"

CS_IMPLEMENT_APPLICATION

//-- Gui -------------------------------------------------------------------

static bool printable(char c)
{
  return ( (32<=c) && (c<=126) );
}


void csEditCharView::Reevaluate() //// number of chars in font and so on.
  {
    stats.Set(BorderWidth+inset, BorderHeight+inset+TitlebarHeight,
      BorderWidth+inset + 100, bound.Height()-BorderHeight-inset);
    content.Set(stats.xmax+inset*2, BorderHeight+inset*2+TitlebarHeight,
      bound.Width() - BorderWidth - scrsize, bound.Height() -
      BorderHeight - scrsize);

    scrhor->SetRect(content.xmin-inset, content.ymax,
      content.xmax, content.ymax+scrsize);
    struct csScrollBarStatus status;
    status.value = (int)scrhor->SendCommand(cscmdScrollBarQueryValue, 0);
    status.maxvalue = celsize*editchar->GetWidth() - content.Width();
    if(status.maxvalue<0) status.maxvalue = 0;
    status.size = scrsize;
    status.maxsize = scrhor->bound.Width();
    status.step = 1;
    status.pagestep = content.Width()/2;
    scrhor->SendCommand(cscmdScrollBarSet, (intptr_t)&status);
    scrvert->SetRect(content.xmax, content.ymin-inset,
      content.xmax+scrsize, content.ymax);
    status.value = (int)scrvert->SendCommand(cscmdScrollBarQueryValue, 0);
    status.maxvalue = celsize*editchar->GetHeight() - content.Height();
    if(status.maxvalue<0) status.maxvalue = 0;
    status.size = scrsize;
    status.maxsize = scrvert->bound.Height();
    status.step = 1;
    status.pagestep = content.Height()/2;
    scrvert->SendCommand(cscmdScrollBarSet, (intptr_t)&status);

  }

csEditCharView::csEditCharView(csComponent *iParent, csEditFont *fnt,
    csEditChar *chr)
  : csWindow(iParent, "Edit Character", CSWS_TITLEBAR | CSWS_BUTCLOSE |
    CSWS_BUTMAXIMIZE)
  {
    isdrawing = false;
    drawcolour = 1;
    font = fnt;
    editchar = chr;
    editchar->SetView(this);

    offx = 0; offy = 0;
    inset = 5; // inset border
    celsize = 10;
    scrsize = CSSB_DEFAULTSIZE;

    palstart = palettesize;
    int* newpalette = new int[palettesize + 6];
    memcpy(newpalette, palette, palettesize*sizeof(int));
    SetPalette(newpalette, palettesize+6);
    SetColor(palstart+0, cs_Color_White);
    SetColor(palstart+1, cs_Color_Black);
    SetColor(palstart+2, cs_Color_Gray_L);
    SetColor(palstart+3, cs_Color_Gray_D);
    SetColor(palstart+4, cs_Color_Blue_M);
    SetColor(palstart+5, cs_Color_Blue_L);
    SetFont(((CsfEdit*)app)->GetMainFont());

    scrhor = new csScrollBar(this);
    scrvert = new csScrollBar(this);
    SetSize(500,500);
    Reevaluate();
    Center();

    int stath; /// for drawing font
    GetTextSize("", &stath);
    csButton *but = new csButton(this, 66700);
    but->SetText("Settings...");
    but->SetPos(stats.xmin+inset/2, stats.ymin+inset+stath*5);
    but->SetSuggestedSize(inset, inset);
    app->HintAdd("Edit the settings for this character.", but);

    but = new csButton(this, cscmdClose);
    but->SetText("Done");
    but->SetPos(stats.xmin+inset/2, stats.ymin+inset+stath*9);
    but->SetSuggestedSize(inset, inset);
    app->HintAdd("Close this window.", but);
  }

bool csEditCharView::SetRect(int xmin, int ymin, int xmax, int ymax)
  {
    if(!csWindow::SetRect(xmin, ymin, xmax, ymax)) return false;
    Reevaluate();
    return true;
  }

csEditCharView::~csEditCharView()
  {
    editchar->SetView(0);
    delete scrhor;
    delete scrvert;
  }

void csEditCharView::Draw()
  {
    SetFont(((CsfEdit*)app)->GetMainFont());
    csWindow::Draw();

    Rect3D(stats.xmin - inset, stats.ymin, stats.xmax, stats.ymax,
      palstart+3, palstart+0);
    int fontheight = 10; /// for drawing font
    char buf[256];
    GetTextSize("", &fontheight);
    int y = stats.ymin + inset + fontheight;
    sprintf(buf, "Font: %s", font->GetFontName()?font->GetFontName():"");
    Text(stats.xmin, y, palstart+1, -1, buf);
    y += fontheight+2;
    sprintf(buf, "Char %d %c", font->GetCharNumber(editchar),
      printable(font->GetCharNumber(editchar))?
      font->GetCharNumber(editchar):' ');
    Text(stats.xmin, y, palstart+1, -1, buf);
    y += fontheight+2;
    sprintf(buf, "Size %dx%d", editchar->GetWidth(), editchar->GetHeight());
    Text(stats.xmin, y, palstart+1, -1, buf);
    y += fontheight+2;
    y += 7*(fontheight+2);
    csRect prev;
    prev.SetPos(stats.xmin, y);
    prev.SetSize(editchar->GetWidth(), editchar->GetHeight());
    Box(prev.xmin, prev.ymin, prev.xmax, prev.ymax, palstart+0);
    Rect3D(prev.xmin-1, prev.ymin-1, prev.xmax+1, prev.ymax+1,
      palstart+2, palstart+3);
    editchar->Draw(this, prev.xmin, prev.ymin, palstart+1);

    Box( content.xmin-inset, content.ymin-inset, content.xmax, content.ymax,
      palstart+0);
    Rect3D( content.xmin-inset, content.ymin-inset, content.xmax, content.ymax,
      palstart+2, palstart+3);
    SetClipRect( content.xmin-inset, content.ymin-inset, content.xmax,
      content.ymax);
    csRect cel;
    int pixcolor;
    int makeborder = 1; /// 1 makes a border
    if(celsize<=1) makeborder=0;
	int cely, celx;
    for(cely = 0; cely<editchar->GetHeight(); cely++)
      for(celx = 0; celx<editchar->GetWidth(); celx++)
      {
        if(editchar->GetPixel(celx, cely))
	  pixcolor = 1 + palstart;
	else pixcolor = 0 + palstart;
        cel.Set(content.xmin+celx*celsize, content.ymin+cely*celsize,
          content.xmin+celx*celsize + celsize - makeborder,
	  content.ymin+cely*celsize + celsize - makeborder);
	cel.Move(-offx, -offy);
	Box( cel.xmin, cel.ymin, cel.xmax, cel.ymax, pixcolor);
      }
    Rect3D( content.xmin, content.ymin,
      content.xmin+celsize*editchar->GetWidth(),
      content.ymin+celsize*editchar->GetHeight(), palstart+3, palstart+2);
    SetClipRect(); // disable clipping
    SetFont(((CsfEdit*)app)->GetMainFont());
  }

#define CHAR_MODAL_DATA 1
#define FONT_MODAL_DATA 2
#define SAVE_FONT_MODAL_DATA 3
#define LOAD_FONT_MODAL_DATA 4

/*
 * This is data that we keep for modal processing.
 */
struct ModalData : public iBase
{
  uint code;
  csWindow* parent;
  csWindow* d;
  SCF_DECLARE_IBASE;
  ModalData () { SCF_CONSTRUCT_IBASE (0); }
  virtual ~ModalData () { SCF_DESTRUCT_IBASE (); }
};

SCF_IMPLEMENT_IBASE (ModalData)
SCF_IMPLEMENT_IBASE_END

struct CharModalData : public ModalData
{
  csSpinBox* enter_width;
  csSpinBox* enter_zoom;
};

void csEditCharView::EditSettings()
  {
    csWindow *w = new csWindow(app,  "Edit Character Settings", 0);
    w->SetFont(((CsfEdit*)app)->GetMainFont());
    csDialog *d = new csDialog(w);
    w->SetDragStyle (w->GetDragStyle () & ~CS_DRAG_SIZEABLE);

    w->SetSize(500,500);
    w->Center();

    int px = 15, py = 20;
    int labelw = 150;

    csButton *but = new csButton(d, cscmdOK, CSBS_DEFAULTVALUE |
      CSBS_DISMISS | CSBS_DEFAULT);
    but->SetText("OK");
    but->SetSuggestedSize(16,8);
    but->SetPos(30, 450);
    but = new csButton(d, cscmdCancel, CSBS_DEFAULTVALUE | CSBS_DISMISS);
    but->SetText("Cancel"); but->SetSuggestedSize(16,8);
    but->SetPos(130, 450);

    /// display character number
    char buf[100];
    sprintf(buf, "Character %d %c", font->GetCharNumber(editchar),
      printable(font->GetCharNumber(editchar))?
      font->GetCharNumber(editchar):' ');
    csStatic *stat = new csStatic(d, csscsText);
    stat->SetText(buf);
    stat->SetRect(px, py, px+labelw,py+30);
    py += stat->bound.Height();

    /// width
    csSpinBox *enter_width = new csSpinBox(d);
    enter_width->SetSuggestedSize(100,20);
    enter_width->SetPos(px+labelw,py);
    enter_width->SetLimits(1,1000);
    enter_width->SetValue( editchar->GetWidth() );
    stat = new csStatic(d, enter_width, "~Width:");
    stat->SetRect(px, py, px+labelw,py+enter_width->bound.Height());
    py += enter_width->bound.Height();

    /// zoom factor === celsize
    csSpinBox *enter_zoom = new csSpinBox(d);
    enter_zoom->SetSuggestedSize(100,20);
    enter_zoom->SetPos(px+labelw,py);
    enter_zoom->SetLimits(1,100);
    enter_zoom->SetValue( celsize );
    stat = new csStatic(d, enter_zoom, "~Zoom factor:");
    stat->SetRect(px, py, px+labelw,py+enter_zoom->bound.Height());
    py += enter_zoom->bound.Height();

    CharModalData* data = new CharModalData ();
    data->parent = this;
    data->d = w;
    data->code = CHAR_MODAL_DATA;
    data->enter_width = enter_width;
    data->enter_zoom = enter_zoom;
    app->StartModal (w, data);
    data->DecRef ();
  }

void csEditCharView::HandleEditSettings (ModalData* data)
  {
    CharModalData* emd = (CharModalData*)data;
    csSpinBox* enter_width = emd->enter_width;
    csSpinBox* enter_zoom = emd->enter_zoom;

    //// see what changed
    int neww = (int)enter_width->SendCommand(cscmdSpinBoxQueryValue,0);
    int newzoom = (int)enter_zoom->SendCommand(cscmdSpinBoxQueryValue,0);
    if(newzoom != celsize)
    {
      celsize = newzoom;
      Reevaluate();
      Invalidate();
    }
    if(neww != editchar->GetWidth())
    {
      editchar->SetWidth(neww);
      font->MakeDirty();
      font->RecalcWidth();
      font->GetView()->Invalidate();
      Reevaluate();
      Invalidate();
    }
  }

bool csEditCharView::HandleEvent(iEvent &Event)
  {
    int selx, sely;
    if((Event.Type == csevCommand ) &&
      (Event.Command.Code == 66700))
    {
      /// edit character settings dialog
      EditSettings();
      return true;
    }
    if((Event.Type == csevCommand ) &&
      (Event.Command.Code == cscmdClose))
    {
      delete this; /// delete me - sets view to 0 in editchar
      return true;
    }
    if((Event.Type == csevMouseMove) &&
      (!content.Contains(Event.Mouse.x, Event.Mouse.y)))
      isdrawing = false;
    if((Event.Type == csevMouseMove) &&
      (content.Contains(Event.Mouse.x, Event.Mouse.y)))
    {
      SetMouse(csmcArrow);
      if(!isdrawing) return true;

      selx = (Event.Mouse.x - content.xmin + offx ) / celsize;
      sely = (Event.Mouse.y - content.ymin + offy) / celsize;
      csRect inv(content);
      inv.xmin -= inset;
      inv.ymin -= inset;
      if( (selx<0) || (selx>=editchar->GetWidth()) ||
          (sely<0) || (sely>=editchar->GetHeight()))
      {
        return true;
      }
      Invalidate(inv);
      Invalidate(stats);
      font->GetView()->Invalidate(); /// see the change in the font view too
      font->MakeDirty();
      editchar->SetPixel(selx, sely, drawcolour);
      return true;
    }
    if((Event.Type == csevMouseMove) &&
      (stats.Contains(Event.Mouse.x, Event.Mouse.y)))
    {
        SetMouse(csmcArrow);
      return true;
    }

    if((Event.Type == csevCommand ) &&
      (Event.Command.Code == cscmdScrollBarValueChanged))
    {
      int newoff = (int) ((csScrollBar*)Event.Command.Info)->SendCommand(
        cscmdScrollBarQueryValue, 0);
      if(Event.Command.Info == (intptr_t)scrhor) offx = newoff;
      if(Event.Command.Info == (intptr_t)scrvert) offy = newoff;
      Invalidate();
      return true;
    }
    if((Event.Type == csevMouseDown) && (Event.Mouse.Button == 1) &&
      content.Contains(Event.Mouse.x, Event.Mouse.y))
    {
      SetMouse(csmcArrow);
      isdrawing = true;
      selx = (Event.Mouse.x - content.xmin + offx ) / celsize;
      sely = (Event.Mouse.y - content.ymin + offy) / celsize;
      if( (selx<0) || (selx>=editchar->GetWidth()) ||
          (sely<0) || (sely>=editchar->GetHeight()))
      {
        drawcolour = 1;
        return true;
      }
      if(editchar->GetPixel(selx, sely))
        drawcolour = 0;
      else drawcolour = 1;

      csRect inv(content);
      inv.xmin -= inset;
      inv.ymin -= inset;
      Invalidate(inv);
      Invalidate(stats);
      font->GetView()->Invalidate(); /// see the change in the font view too
      font->MakeDirty();
      editchar->SetPixel(selx, sely, drawcolour);
      return true;
    }
    if((Event.Type == csevMouseUp) && (Event.Mouse.Button == 1) &&
      content.Contains(Event.Mouse.x, Event.Mouse.y))
    {
      SetMouse(csmcArrow);
      isdrawing = false;
      return true;
    }

    if(CS_IS_MOUSE_EVENT(Event) &&
      content.Contains(Event.Mouse.x, Event.Mouse.y))
    {
      SetMouse(csmcArrow);
      return true;
    }

    if(csWindow::HandleEvent(Event)) return true;

    return false;
  }


void csEditFontView::Reevaluate() //// number of chars in font and so on.
  {
    int tinyw = 5, tinyh = 5;
    ((CsfEdit*)app)->GetTinyFont()->GetMaxSize(tinyw, tinyh);
    celw = font->GetWidth() + inset;
    if(celw < 4*tinyw) celw = 4*tinyw;
    celh = font->GetHeight() + inset + tinyh;
    celperrow = (int) csQsqrt(font->GetCharCount());
    celpercol = (font->GetCharCount() + celperrow -1) / celperrow;

    stats.Set(BorderWidth+inset, BorderHeight+inset+TitlebarHeight,
      BorderWidth+inset + 100, bound.Height()-BorderHeight-inset);
    content.Set(stats.xmax+inset*2, BorderHeight+inset*2+TitlebarHeight,
      bound.Width() - BorderWidth - scrsize, bound.Height() -
      BorderHeight - scrsize);

    scrhor->SetRect(content.xmin-inset, content.ymax,
      content.xmax, content.ymax+scrsize);
    struct csScrollBarStatus status;
    status.value = (int)scrhor->SendCommand(cscmdScrollBarQueryValue, 0);
    status.maxvalue = celw*celperrow + inset - content.Width();
    if(status.maxvalue<0) status.maxvalue = 0;
    status.size = scrsize;
    status.maxsize = scrhor->bound.Width();
    status.step = 1;
    status.pagestep = content.Width()/2;
    scrhor->SendCommand(cscmdScrollBarSet, (intptr_t)&status);
    scrvert->SetRect(content.xmax, content.ymin-inset,
      content.xmax+scrsize, content.ymax);
    status.value = (int)scrvert->SendCommand(cscmdScrollBarQueryValue, 0);
    status.maxvalue = celh*celpercol + inset - content.Height();
    if(status.maxvalue<0) status.maxvalue = 0;
    status.size = scrsize;
    status.maxsize = scrvert->bound.Height();
    status.step = 1;
    status.pagestep = content.Height()/2;
    scrvert->SendCommand(cscmdScrollBarSet, (intptr_t)&status);

  }

csEditFontView::csEditFontView(csComponent *iParent, csEditFont *fnt)
  : csWindow(iParent, fnt->GetFontName(), CSWS_TITLEBAR | CSWS_BUTCLOSE |
    CSWS_BUTMAXIMIZE)
  {
    font = fnt;
    font->SetView(this);

    selected = false;
    selx = 0; sely = 0;
    offx = 0; offy = 0;
    inset = 5; // inset border
    scrsize = CSSB_DEFAULTSIZE;

    palstart = palettesize;
    int* newpalette = new int[palettesize + 6];
    memcpy(newpalette, palette, palettesize*sizeof(int));
    SetPalette(newpalette, palettesize+6);
    SetColor(palstart+0, cs_Color_White);
    SetColor(palstart+1, cs_Color_Black);
    SetColor(palstart+2, cs_Color_Gray_L);
    SetColor(palstart+3, cs_Color_Gray_D);
    SetColor(palstart+4, cs_Color_Blue_M);
    SetColor(palstart+5, cs_Color_Blue_L);
    SetFont(((CsfEdit*)app)->GetMainFont());

    scrhor = new csScrollBar(this);
    scrvert = new csScrollBar(this);
    SetSize(500,500);
    Reevaluate();
    Center();

    int stath; /// for drawing font
    GetTextSize("", &stath);
    csButton *but = new csButton(this, 66700);
    but->SetText("Settings...");
    but->SetPos(stats.xmin+inset/2, stats.ymin+inset+stath*10);
    but->SetSuggestedSize(inset, inset);
    app->HintAdd("Edit the font settings or change the number or size "
      "of all the characters in the font.", but);
  }

bool csEditFontView::SetRect(int xmin, int ymin, int xmax, int ymax)
  {
    if(!csWindow::SetRect(xmin, ymin, xmax, ymax)) return false;

    Reevaluate();

    return true;
  }

csEditFontView::~csEditFontView()
  {
    font->SetView(0);
    delete scrhor;
    delete scrvert;
  }

void csEditFontView::Draw()
  {
    SetFont(((CsfEdit*)app)->GetMainFont());
    csWindow::Draw();

    Rect3D(stats.xmin - inset, stats.ymin, stats.xmax, stats.ymax,
      palstart+3, palstart+0);
    int fontheight = 10; /// for drawing font
    char buf[256];
    GetTextSize("", &fontheight);
    int y = stats.ymin + inset + fontheight;
    sprintf(buf, "Font: %s", font->GetFontName()?font->GetFontName():"");
    Text(stats.xmin, y, palstart+1, -1, buf);
    y += fontheight+2;
    sprintf(buf, "File: %s", font->GetFileName()?font->GetFileName():"<none>");
    Text(stats.xmin, y, palstart+1, -1, buf);
    y += fontheight+2;
    sprintf(buf, "First: %d", font->GetStartChar());
    Text(stats.xmin, y, palstart+1, -1, buf);
    y += fontheight+2;
    sprintf(buf, "Number: %d", font->GetCharCount());
    Text(stats.xmin, y, palstart+1, -1, buf);
    y += fontheight+2;
    sprintf(buf, "Size %dx%d", font->GetWidth(), font->GetHeight());
    Text(stats.xmin, y, palstart+1, -1, buf);
    y += fontheight+2;

    SetFont(((CsfEdit*)app)->GetTinyFont());
    int tinyw = 5, tinyh = 5;
    ((CsfEdit*)app)->GetTinyFont()->GetMaxSize(tinyw, tinyh);
    GetTextSize("", &fontheight);
    Box( content.xmin-inset, content.ymin-inset, content.xmax, content.ymax,
      palstart+0);
    Rect3D( content.xmin-inset, content.ymin-inset, content.xmax, content.ymax,
      palstart+2, palstart+3);
    SetClipRect( content.xmin-inset, content.ymin-inset, content.xmax,
      content.ymax);
    csRect cel;
    int cely, celx, charnum;
    for(cely = 0; cely<celpercol; cely++)
      for(celx = 0; celx<celperrow; celx++)
      {
	charnum = cely*celperrow+ celx + font->GetStartChar();
	if(charnum >= font->GetStartChar()+font->GetCharCount() ) continue;
        cel.Set(content.xmin+celx*celw, content.ymin+cely*celh,
          content.xmin+celx*celw + font->GetChar(charnum)->GetWidth(),
	  content.ymin+cely*celh + font->GetChar(charnum)->GetHeight());
	cel.Move(-offx, -offy);
	font->GetChar(charnum)->Draw(this, cel.xmin, cel.ymin, palstart+1);
        Rect3D(cel.xmin-2, cel.ymin-2, cel.xmax+2, cel.ymax+2,
	  palstart+3, palstart+2);
        sprintf(buf, "%2.2x %c", charnum, printable(charnum)?charnum:' ');
	Text(cel.xmin-2, cel.ymax+3, palstart+1, -1, buf);
      }

    if(selected)
    {
      charnum = sely*celperrow+ selx + font->GetStartChar();
      if(charnum < font->GetStartChar()+font->GetCharCount() )
      {
       cel.Set(content.xmin+selx*celw, content.ymin+sely*celh,
         content.xmin+selx*celw + font->GetChar(charnum)->GetWidth(),
         content.ymin+sely*celh + font->GetChar(charnum)->GetHeight());
       cel.Move(-offx, -offy);
       Rect3D(cel.xmin-2, cel.ymin-2, cel.xmax+2, cel.ymax+2,
         palstart+4, palstart+5);
       Rect3D(cel.xmin-3, cel.ymin-3, cel.xmax+3, cel.ymax+3,
         palstart+4, palstart+5);
      }
    }

    SetClipRect(); // disable clipping
    SetFont(((CsfEdit*)app)->GetMainFont());
  }

struct FontModalData : public ModalData
{
  csInputLine *enter_name;
  csSpinBox *enter_first;
  csSpinBox *enter_number;
  csSpinBox *enter_w;
  csSpinBox *enter_h;
};

void csEditFontView::EditSettings()
  {
    csWindow *w = new csWindow(app,  "Edit Font Settings", 0);
    w->SetFont(((CsfEdit*)app)->GetMainFont());
    csDialog *d = new csDialog(w);
    w->SetDragStyle (w->GetDragStyle () & ~CS_DRAG_SIZEABLE);

    w->SetSize(500,500);
    w->Center();

    int px = 15, py = 20;
    int labelw = 150;

    csButton *but = new csButton(d, cscmdOK, CSBS_DEFAULTVALUE |
      CSBS_DISMISS | CSBS_DEFAULT);
    but->SetText("OK");
    but->SetSuggestedSize(16,8);
    but->SetPos(30, 450);
    but = new csButton(d, cscmdCancel, CSBS_DEFAULTVALUE | CSBS_DISMISS);
    but->SetText("Cancel"); but->SetSuggestedSize(16,8);
    but->SetPos(130, 450);

    /// fontname, first, number, size
    csInputLine *enter_name = new csInputLine(d);
    enter_name->SetSize(300,30);
    enter_name->SetPos(px+labelw,py);
    enter_name->SetText(font->GetFontName());
    csStatic *stat = new csStatic(d, enter_name, "~Font name:");
    stat->SetRect(px, py, px+labelw,py+enter_name->bound.Height());
    py += enter_name->bound.Height();

    /// first char
    csSpinBox *enter_first = new csSpinBox(d);
    enter_first->SetSuggestedSize(100,20);
    enter_first->SetPos(px+labelw,py);
    enter_first->SetLimits(0,255);
    enter_first->SetValue( font->GetStartChar() );
    stat = new csStatic(d, enter_first, "First ~Character:");
    stat->SetRect(px, py, px+labelw,py+enter_first->bound.Height());
    py += enter_first->bound.Height();

    /// number of  chars
    csSpinBox *enter_number = new csSpinBox(d);
    enter_number->SetSuggestedSize(100,20);
    enter_number->SetPos(px+labelw,py);
    enter_number->SetLimits(1,256);
    enter_number->SetValue( font->GetCharCount() );
    stat = new csStatic(d, enter_number, "~Number of chars:");
    stat->SetRect(px, py, px+labelw,py+enter_number->bound.Height());
    py += enter_number->bound.Height();

    /// number of  chars
    csSpinBox *enter_w = new csSpinBox(d);
    enter_w->SetSuggestedSize(100,20);
    enter_w->SetPos(px+labelw,py);
    enter_w->SetLimits(1,1000);
    enter_w->SetValue( font->GetWidth() );
    stat = new csStatic(d, enter_w, "~Width:");
    stat->SetRect(px, py, px+labelw,py+enter_w->bound.Height());
    py += enter_w->bound.Height();

    /// number of  chars
    csSpinBox *enter_h = new csSpinBox(d);
    enter_h->SetSuggestedSize(100,20);
    enter_h->SetPos(px+labelw,py);
    enter_h->SetLimits(1,1000);
    enter_h->SetValue( font->GetHeight() );
    stat = new csStatic(d, enter_h, "~Height:");
    stat->SetRect(px, py, px+labelw,py+enter_h->bound.Height());
    py += enter_h->bound.Height();

    FontModalData* data = new FontModalData ();
    data->parent = this;
    data->d = w;
    data->code = FONT_MODAL_DATA;
    data->enter_name = enter_name;
    data->enter_first = enter_first;
    data->enter_number = enter_number;
    data->enter_w = enter_w;
    data->enter_h = enter_h;
    app->StartModal (w, data);
    data->DecRef ();
  }

void csEditFontView::HandleEditSettings(ModalData* data)
  {
    FontModalData* emd = (FontModalData*)data;
    csInputLine* enter_name = emd->enter_name;
    csSpinBox* enter_first = emd->enter_first;
    csSpinBox* enter_number = emd->enter_number;
    csSpinBox* enter_w = emd->enter_w;
    csSpinBox* enter_h = emd->enter_h;

    //// see what changed
    if(strcmp(enter_name->GetText(), font->GetFontName()) != 0)
    {
      font->SetFontName(enter_name->GetText());
      font->MakeDirty();
    }
    int newfirst = (int)enter_first->SendCommand(cscmdSpinBoxQueryValue,0);
    int newnum = (int)enter_number->SendCommand(cscmdSpinBoxQueryValue,0);
    int neww = (int)enter_w->SendCommand(cscmdSpinBoxQueryValue,0);
    int newh = (int)enter_h->SendCommand(cscmdSpinBoxQueryValue,0);
    if((newfirst != font->GetStartChar()) || (newnum != font->GetCharCount()))
    {
      font->SetStartNum( newfirst, newnum );
      font->MakeDirty();
      Reevaluate();
    }
    if(neww != font->GetWidth())
    {
      font->SetWidth(neww);
      font->MakeDirty();
      Reevaluate();
    }
    if(newh != font->GetHeight())
    {
      font->SetHeight(newh);
      font->MakeDirty();
      Reevaluate();
    }
    Invalidate();
  }

bool csEditFontView::HandleEvent(iEvent &Event)
  {
    if((Event.Type == csevCommand ) &&
      (Event.Command.Code == 66700))
    {
      /// edit font settings dialog
      EditSettings();
      return true;
    }
    if((Event.Type == csevCommand ) &&
      (Event.Command.Code == cscmdClose))
    {
      delete font; /// also deletes me
      return true;
    }
    if((Event.Type == csevMouseMove) &&
      (content.Contains(Event.Mouse.x, Event.Mouse.y)))
    {
      SetMouse(csmcArrow);
      return true;
    }
    if((Event.Type == csevMouseMove) &&
      (stats.Contains(Event.Mouse.x, Event.Mouse.y)))
    {
        SetMouse(csmcArrow);
      return true;
    }

    if((Event.Type == csevCommand ) &&
      (Event.Command.Code == cscmdScrollBarValueChanged))
    {
      int newoff = (int) ((csScrollBar*)Event.Command.Info)->SendCommand(
        cscmdScrollBarQueryValue, 0);
      if(Event.Command.Info == (intptr_t)scrhor) offx = newoff;
      if(Event.Command.Info == (intptr_t)scrvert) offy = newoff;
      Invalidate();
      return true;
    }
    if((Event.Type == csevMouseDown) && (Event.Mouse.Button == 1) &&
      content.Contains(Event.Mouse.x, Event.Mouse.y))
    {
      selected = true;
      selx = (Event.Mouse.x - content.xmin + offx + inset) / celw;
      sely = (Event.Mouse.y - content.ymin + offy + inset/2) / celh;
      //// modified as follows: the text beneath each cell belongs to
      //// that cell. But the area left&right is divided evenly.
      csRect inv(content);
      inv.xmin -= inset;
      inv.ymin -= inset;
      Invalidate(inv);
      if( (selx<0) || (selx>=celperrow) || (sely<0) || (sely>=celpercol))
      {
        selected = false;
        return true;
      }
    }

    if((Event.Type == csevMouseClick) && (Event.Mouse.Button == 1) &&
      content.Contains(Event.Mouse.x, Event.Mouse.y))
    {
      selected = true;
      selx = (Event.Mouse.x - content.xmin + offx + inset) / celw;
      sely = (Event.Mouse.y - content.ymin + offy + inset/2) / celh;
      //// modified as follows: the text beneath each cell belongs to
      //// that cell. But the area left&right is divided evenly.
      csRect inv(content);
      inv.xmin -= inset;
      inv.ymin -= inset;
      Invalidate(inv);
      if( (selx<0) || (selx>=celperrow) || (sely<0) || (sely>=celpercol))
      {
        selected = false;
        return true;
      }
      /// open a character window
      int zecharnum = sely * celperrow + selx;
      if(!font->GetChar(zecharnum)) return true; /// robust
      csComponent *v = 0;
      if(font->GetChar(zecharnum)->GetView())
      {
        v = font->GetChar(zecharnum)->GetView();
      }
      else
      {
        v = new csEditCharView(app, font, font->GetChar(zecharnum));
      }
      app->SetZorder(v, this);
      v->Show();
      v->Invalidate();
    }

    if(CS_IS_MOUSE_EVENT(Event) &&
      content.Contains(Event.Mouse.x, Event.Mouse.y))
    {
      SetMouse(csmcArrow);
      return true;
    }

    if(csWindow::HandleEvent(Event)) return true;

    return false;
  }


//-- csEditChar ------------------------------------------------------------
csEditChar::csEditChar()
{
  view = 0;
  pixels = 0;
  width=10; height=10;
  pixels = new uint8[width*height];
  int i;
  for(i=0; i<width*height; i++) pixels[i] = 0;
}


csEditChar::csEditChar(int w, int h, uint8 *bitmap)
{
  view = 0;
  int l, i;
  width=w; height=h;
  pixels = new uint8[width*height];
  for(i=0; i<width*height; i++) pixels[i] = 0;

  for (l = 0; l < h; l++)
  {
    uint8 *line = bitmap + l * ((w + 7) / 8);
    for (i = 0; i < w; i++)
      if(line [i / 8] & (0x80 >> (i & 7)))
        SetPixel(i,l,1);
      else SetPixel(i,l,0);
  }

}

csEditChar::~csEditChar()
{
  delete view;
  delete[] pixels;
}


void csEditChar::Draw(csComponent *dest, int minx, int miny, int col)
{
  int y, x;
  for(y = 0; y < height ; y++)
    for(x = 0; x<width ; x++)
    {
      if(GetPixel(x,y)) dest->Pixel(minx+x, miny+y, col);
    }
}


void csEditChar::SetWidth(int w)
{
  //// (clip / expand) to new width
  uint8* newpix = new uint8 [ w*height ];
  int x,y;
  for(y = 0; y < height ; y++)
    for(x = 0; x<w ; x++)
      newpix[y*w+x] = 0;
  /// copy overlap
  int overw = w;
  if(w>width) overw = width;
  for(y = 0; y < height ; y++)
    for(x = 0; x<overw ; x++)
      newpix[y*w+x] = GetPixel(x,y);
  // swap in
  delete[] pixels;
  pixels = newpix;
  width = w;
}

void csEditChar::SetHeight(int h)
{
  //// (clip / expand) to new height
  uint8* newpix = new uint8 [ width*h ];
  int x,y;
  for(y = 0; y < h; y++)
    for(x = 0; x<width ; x++)
      newpix[y*width+x] = 0;
  /// copy overlap
  int overh = h;
  if(h>height) overh = height;
  for(y = 0; y < overh ; y++)
    for(x = 0; x<width ; x++)
      newpix[y*width+x] = GetPixel(x,y);
  // swap in
  delete[] pixels;
  pixels = newpix;
  height = h;
}


int csEditChar::GetBitmap(int idx)
{
  int bpc = ((width + 7)/8) *height; /// bytes for char
  int bpl = (width + 7)/8; // bytes per line
  if( (idx<0) || (idx>bpc)) return 0;
  int y = idx / bpl;
  int xstart = (idx - y*bpl)*8;
  int res = 0;
  int x;
  for(x=xstart; x<xstart+8; x++)
  {
    res<<=1;
    if((x<width) && GetPixel(x,y)) res |=1;
  }
  return res;
}



//-- csEditFont ------------------------------------------------------------
csEditFont::csEditFont(csApp *iApp)
{
  app = iApp;
  filename = 0;
  view = 0;
  dirty = false;
  fontname = csStrNew("Untitled");
  startchar = 0;
  numchars = 256;
  fontwidth = 10;
  fontheight = 10;
  chars = new csEditChar* [numchars];
  int i;
  for(i=0; i<numchars; i++) chars[i] = new csEditChar();
}

csEditFont::csEditFont(csApp *iApp, const char *fromfile)
{
  int i;
  app = iApp;
  filename = csStrNew(fromfile);
  view = 0;
  dirty = false;
  fontname = 0;
  startchar = 0;
  numchars = 256;
  fontwidth = 10;
  fontheight = 10;
  chars = 0;

  /// read the file
  /// taken from csfont plugin...

  /*  @@@ need to be able to get VFS names
  csRef<iVFS> VFS (CS_QUERY_REGISTRY (System->object_reg, iVFS));
  csRef<iDataBuffer> fntfile (VFS->ReadFile (fromfile));
  if (!fntfile)
  {
    csReport (System->object_reg, CS_REPORTER_SEVERITY_WARNING,
	"crystalspace.application.csfedit",
    	"Could not read font file %s.", fromfile);
    return;
  }

  char *data = **fntfile;
  */
  struct stat statbuf;
  stat(fromfile, &statbuf);
  int filelen = statbuf.st_size;
  FILE * in = fopen(fromfile, "rb");
  char *data = new char[filelen]; /// @@memleak
  fread(data, 1, filelen, in);
  fclose(in);



  if (data [0] != 'C' || data [1] != 'S' ||  data [2] != 'F')
  {
error:
    ///// create empty font
    fontname = csStrNew("LoadingError");
    chars = new csEditChar* [numchars];
    for(i=0; i<numchars; i++) chars[i] = new csEditChar();
    return;
  }

  /// the new fontdef to store info into

  numchars = 256;

  char *end = strchr (data, '\n');
  char *cur = strchr (data, '[');
  if (!end || !cur)
    goto error;

  char *binary = end + 1;
  while ((end > data) && ((end [-1] == ' ') || (end [-1] == ']')))
    end--;

  cur++;
  while (cur < end)
  {
    while ((cur < end) && (*cur == ' '))
      cur++;

    char kw [20];
    size_t kwlen = 0;
    while ((cur < end) && (*cur != '=') && (kwlen < sizeof (kw) - 1))
      kw [kwlen++] = *cur++;
    kw [kwlen] = 0;
    if (!kwlen)
      break;

    cur = strchr (cur, '=');
    if (!cur) break;
    cur++;

    if (!strcmp (kw, "Font"))
    {
      char *start = cur;
      while ((cur < end) && (*cur != ' '))
        cur++;
      fontname = new char [cur - start + 1];
      memcpy (fontname, start, cur - start);
      fontname[cur - start] = 0;
    }
    else
    {
      char val [20];
      size_t vallen = 0;
      while ((cur < end) && (*cur != ' ') && (vallen < sizeof (val) - 1))
        val [vallen++] = *cur++;
      val [vallen] = 0;
      int n = atoi (val);

      if (!strcmp (kw, "Width"))
        fontwidth = n;
      else if (!strcmp (kw, "Height"))
        fontheight = n;
      else if (!strcmp (kw, "First"))
        startchar = n;
      else if (!strcmp (kw, "Glyphs"))
        numchars = n;
    }
  }

#if defined(CS_DEBUG)
  printf("Reading Font %s, width %d, height %d, First %d, %d Glyphs.\n",
    fontname, fontwidth, fontheight, startchar, numchars);
#endif

  uint8* IndividualWidth = new uint8 [numchars];
  memcpy (IndividualWidth, binary, numchars);

  // Compute the font size
  int fontsize = 0;
  for (i = 0; i < numchars; i++)
    fontsize += ((IndividualWidth [i] + 7) / 8) * fontheight;

  // allocate memory and copy the font
  uint8* FontBitmap = new uint8 [fontsize];
  memcpy (FontBitmap, binary + numchars, fontsize);

  ///// further processing of InividualWidth and FontBitmap
  /// create characters
  uint8 *curbitmap = FontBitmap;
  uint8 **GlyphBitmap = new uint8 * [numchars];
  for(i = 0; i < numchars; i++)
  {
    GlyphBitmap [i] = curbitmap;
    curbitmap += ((IndividualWidth [i] + 7) / 8) * fontheight;
  }

  chars = new csEditChar* [numchars];
  for(i=0; i<numchars; i++)
  {
    chars[i] = new csEditChar(IndividualWidth[i], fontheight,
      GlyphBitmap[i]);
  }
  delete[] GlyphBitmap;
  delete[] IndividualWidth;
  delete[] FontBitmap;
}

csEditFont::~csEditFont()
{
  //@@@ TODO: rewrite using new non-modal/modal system.
  //if(dirty)
  //{
    //if(csMessageBox(app, "Save changes?", "There are unsaved changes. "
      //"Do you wish to save before continuing?",
      //CSMBS_QUESTION | CSMBS_IGNORE | CSMBS_OK) == cscmdOK)
        //Save();
  //}
  /// delete font from memory
  ((CsfEdit*)app)->SetEditFont(0);
  delete view;
  delete[] filename;
  delete[] fontname;
  if(chars) {
	int i;
    for(i=0; i<numchars; i++) delete chars[i];
    delete[] chars;
  }
}

struct SaveFontModalData : public ModalData
{
  csEditFont* edit_font;
  csEditFontView* view;
};


void csEditFont::Save()
{
  if(!filename)
  {
    /// ask a filename
    csWindow *d = csFileDialog (app, "Select a .csf file", "./", "~Save");
    if (d)
    {
      SaveFontModalData* data = new SaveFontModalData ();
      data->parent = 0;
      data->d = d;
      data->code = SAVE_FONT_MODAL_DATA;
      data->edit_font = this;
      data->view = view;
      app->StartModal (d, data);
      data->DecRef ();
    }
    return;
  }
  printf("saving font\n");
  dirty = false;


  int i, c, w, h;
  int maxwidth = fontwidth;
  int maxheight = fontheight;
  int first = startchar;
  int glyphs = numchars;
  int lastglyph = first + glyphs;
  bool sourcecode = false; //// output for inclusion in sourcecode

  FILE *out = fopen (filename, sourcecode ? "w" : "wb");
  if (!out)
  {
    printf ("Could not open output file %s\n", filename);
    return;
  }

  if (sourcecode)
  {
    /// make a text version
    fprintf (out, "// %s %dx%d font\n", fontname, maxwidth, maxheight);
    fprintf (out, "// FontDef: { \"%s\", %d, %d, %d, %d, font_%s, "
	"width_%s }\n",
      fontname, maxwidth, maxheight, first, glyphs, fontname, fontname);
    fprintf (out, "\n");
  }
  else
    fprintf (out, "CSF [Font=%s Width=%d Height=%d First=%d Glyphs=%d]\n",
      fontname, maxwidth, maxheight, first, glyphs);

  int arrsize = 0;
  uint8 width [256];
  for (c = first; c < lastglyph; c++)
  {
    width[c] = GetChar(c)->GetWidth();
    arrsize += ((width [c] + 7) / 8) * GetChar(c)->GetHeight();
  }

  // Character widths go first
  if (sourcecode)
  {
    fprintf (out, "unsigned char width_%s [%d] =\n{\n  ",
      fontname, glyphs);
    for (i = first; i < lastglyph; i++)
    {
      fprintf (out, "%2d%s", width [i], (i < lastglyph - 1) ? "," : "");
      if ((i & 15) == 15)
      {
        fprintf (out, "\t// %02x..%02x\n", i - 15, i);
        if (i < lastglyph - 1)
          fprintf (out, "  ");
      }
    }
    if (glyphs & 15)
      fprintf (out, "\n");
    fprintf (out, "};\n\n");
    fprintf (out, "unsigned char font_%s [%d] =\n{\n",
      fontname, arrsize);
  }
  else
    fwrite (width + first, glyphs, 1, out);

  // Output every character in turn
  for (c = first; c < lastglyph; c++)
  {
    // get bitmap
    w = GetChar(c)->GetWidth();
    h = GetChar(c)->GetHeight();

    int bpc = ((width [c] + 7) / 8) * h;
    int bitmap;

    if (GetChar(c))
      if (sourcecode)
      {
        fprintf (out, "  ");
        for (i = 0; i < bpc; i++)
	{
	  bitmap = GetChar(c)->GetBitmap(i);
          fprintf (out, "0x%02x%s", bitmap, (i >= bpc - 1) &&
	    (c >= lastglyph - 1) ? "" : ",");
	}
        fprintf (out, "\t// %02x\n", c);
      }
      else if (width [c])
      {
        for (i = 0; i < bpc; i++)
	{
	  bitmap = GetChar(c)->GetBitmap(i);
          fwrite (&bitmap, 1, 1, out);
	}
      }
  }

  fprintf (out, "};\n\n");
  fclose (out);

}


void csEditFont::SetFontName(const char *val)
{
  delete[] fontname;
  fontname = csStrNew(val);
}

void csEditFont::SetStartNum(int start, int num)
{
  if(start+num > 256) return;
  if( (start<0) || (num<=0) ) return;
  ////move characters
  startchar = start;
  csEditChar **newchars = new csEditChar *[num];
  int i;
  int max = num;
  if(numchars>num) max = numchars;
  for(i=0; i<max; i++)
  {
    if((i<numchars) && (i<num)) newchars[i] = chars[i];
    else
    {
      if(i<numchars) delete chars[i];
      if(i<num)
      {
      newchars[i] = new csEditChar();
      newchars[i]->SetWidth(fontwidth);
      newchars[i]->SetHeight(fontheight);
      }
    }
  }
  /// swap new list of editchars in.
  delete[] chars;
  chars = newchars;
  numchars = num;
}

void csEditFont::SetWidth(int w)
{
  int c;
  for(c=0; c<numchars; c++)
    chars[c]->SetWidth(w);
  fontwidth = w;
}

void csEditFont::SetHeight(int h)
{
  int c;
  for(c=0; c<numchars; c++)
    chars[c]->SetHeight(h);
  fontheight = h;
}

void csEditFont::RecalcWidth()
{
  int w= 0, c;
  for(c=0; c<numchars; c++)
    if(chars[c]->GetWidth() > w)
      w = chars[c]->GetWidth();
  fontwidth = w;
}

int csEditFont::GetCharNumber(csEditChar *editchar)
{
  int c;
  for(c=0; c<numchars; c++)
    if(chars[c] == editchar)
      return c + startchar;
  printf("Error: no such character in font, ignoring...\n");
  return startchar + 0;
}


//-- CsfEdit ---------------------------------------------------------------

// Scroll bar class default palette
static int palette_CsfEdit[] =
{
  cs_Color_Gray_D,			// Application workspace
  cs_Color_Green_L,			// End points
  cs_Color_Red_L,			// lines
  cs_Color_White			// Start points
};

CsfEdit::CsfEdit (iObjectRegistry *object_reg, csSkin &Skin)
	: csApp (object_reg, Skin)
{
  int pal = csRegisterPalette (palette_CsfEdit, sizeof (palette_CsfEdit) / sizeof (int));
  SetPalette (pal);
  editfont = 0;
  saveitem = 0;
  closeitem = 0;
}

CsfEdit::~CsfEdit ()
{
  /// first delete the editfont - might save itself.
  delete editfont;
  csResetPalette();
}

bool CsfEdit::Initialize ()
{
  if (!csApp::Initialize ())
    return false;

  mainfont = LoadFont (CSFONT_LARGE);
  tinyfont = LoadFont (CSFONT_COURIER);
  SetFont(mainfont);

  // CSWS apps are a lot more performant with a single-buffered canvas
  GetG2D ()->DoubleBuffer (false);

  csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,
	"crystalspace.application.csfedit",
    "Crystal Space version %s (%s).\n"
    "CSF Editor.\n",
    CS_VERSION, CS_RELEASE_DATE);

  csKeyboardAccelerator *ka = new csKeyboardAccelerator (this);
  csMenu *menu = new csMenu (this, csmfsBar, 0);
  menu->id = CSWID_MENUBAR;
  menu->SetFont(mainfont);
  csMenu *submenu = new csMenu (0);
  (void)new csMenuItem (menu, "~File", submenu);
    (void)new csMenuItem (submenu, "~New Font\tCtrl+N", 66600);
    (void)new csMenuItem (submenu, "~Open Font\tCtrl+O", 66601);
    closeitem=new csMenuItem (submenu, "~Close\tCtrl+W", 66603);
    closeitem->SetState(CSS_SELECTABLE, false);
    saveitem=new csMenuItem (submenu, "~Save\tCtrl+S", 66602);
    saveitem->SetState(CSS_SELECTABLE, false);
    (void)new csMenuItem (submenu);
    csMenuItem *mi = new csMenuItem (submenu, "~Quit\tCtrl+Q", 66698);
    HintAdd ("Choose this menu item to quit the program", mi);
    ka->Command ('q', CSMASK_CTRL, 66698);
    ka->Command ('n', CSMASK_CTRL, 66600);
    ka->Command ('o', CSMASK_CTRL, 66601);
    ka->Command ('s', CSMASK_CTRL, 66602);
    ka->Command ('w', CSMASK_CTRL, 66603);

  submenu = new csMenu (0);
  (void)new csMenuItem (menu, "~Windows", submenu);
    (void)new csMenuItem (submenu, "~Window list", 66699);


  int fh; menu->GetTextSize("", &fh);
  menu->SetRect (0, 0, bound.xmax, fh + 8);

  return true;
}


void CsfEdit::SetEditFont(csEditFont *f)
{
  editfont = f;
  if(saveitem!=0)
    saveitem->SetState(CSS_SELECTABLE, editfont!=0);
  if(closeitem!=0)
    closeitem->SetState(CSS_SELECTABLE, editfont!=0);
}


bool CsfEdit::HandleEvent (iEvent &Event)
{
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
	  	SCF_QUERY_INTERFACE (GetTopModalUserdata (),
	  	iMessageBoxData));
	  if (mbd)
	  {
            if (rc == cscmdOK) editfont->Save();
	    delete d;
	    return true;
	  }

	  ModalData* data = (ModalData*)GetTopModalUserdata ();
	  CS_ASSERT (data != 0);
	  csWindow* w = data->d;
	    switch (data->code)
	    {
	      case CHAR_MODAL_DATA:
	        ((csEditCharView*)(data->parent))->HandleEditSettings (data);
		break;
	      case FONT_MODAL_DATA:
	        ((csEditFontView*)(data->parent))->HandleEditSettings (data);
		break;
      	      case SAVE_FONT_MODAL_DATA:
	      {
	        SaveFontModalData* sfdata = (SaveFontModalData*)data;
		char fname [CS_MAXPATHLEN + 1];
		csQueryFileDialog (w, fname, sizeof (fname));
		csEditFont* edit_font = sfdata->edit_font;
		edit_font->SetFileName (csStrNew(fname));
		if (sfdata->view) sfdata->view->Invalidate();
		edit_font->Save ();
		break;
	      }
	      case LOAD_FONT_MODAL_DATA:
	      {
		char filename [CS_MAXPATHLEN + 1];
		csQueryFileDialog (w, filename, sizeof (filename));
		SetEditFont(new csEditFont(this, filename));
		(void)new csEditFontView(this, editfont);
	        break;
	      }
	    }
	  delete w;
	  return true;
	}
        case 66600:
        {
	  if(editfont){ delete editfont; editfont = 0; }
	  SetEditFont(new csEditFont(this));
	  (void)new csEditFontView(this, editfont);
          return true;
        }
        case 66601:
        {
	  if(editfont){ delete editfont; editfont = 0; }
	  csWindow *d = csFileDialog (this, "Select a .csf file");
          if (d)
          {
	    ModalData* data = new ModalData ();
      	    data->parent = 0;
      	    data->d = d;
      	    data->code = LOAD_FONT_MODAL_DATA;
      	    app->StartModal (d, data);
      	    data->DecRef ();
	  }
          return true;
        }
        case 66602:
        {
	  if(editfont) editfont->Save();
          return true;
        }
        case 66603:
        {
	  if(editfont){ delete editfont; editfont = 0; }
          return true;
        }
        case 66698:
        {
          if(editfont && editfont->IsDirty())
          {
            csMessageBox(app, "Save changes?", "There are unsaved changes. "
              "Do you wish to save before continuing?", 0,
              CSMBS_QUESTION | CSMBS_IGNORE | CSMBS_OK);
	    return true;
          }
	  ShutDown();
          return true;
        }
        case 66699:
          WindowList ();
          return true;
      }
      break;

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
    	"crystalspace.application.csfedit",
	"Error initializing system !");
    return -1;
  }

  if (!csInitializer::RequestPlugins (object_reg, CS_REQUEST_END))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.csfedit",
	"Can't initialize!");
    return -1;
  }

  // Check for commandline help.
  if (csCommandLineHelper::CheckHelp (object_reg))
  {
    csCommandLineHelper::Help (object_reg);
    exit (0);
  }

  csRef<iGraphics3D> g3d (CS_QUERY_REGISTRY (object_reg, iGraphics3D));
  iNativeWindow* nw = g3d->GetDriver2D ()->GetNativeWindow ();
  if (nw) nw->SetTitle ("Crystal Space Font Editor");

  if (!csInitializer::OpenApplication (object_reg))
  {
    csInitializer::DestroyApplication (object_reg);
    return -1;
  }

  // Look for skin variant from config file
  csRef<iCommandLineParser> cmdline (CS_QUERY_REGISTRY (object_reg,
  	iCommandLineParser));
  csRef<iConfigManager> cfg (CS_QUERY_REGISTRY (object_reg, iConfigManager));
  DefaultSkin.Prefix = cmdline->GetOption ("skin");
  if (!DefaultSkin.Prefix)
    DefaultSkin.Prefix = cfg->GetStr ("CSWS.Skin.Variant", 0);

  // Create our application object
  CsfEdit app (object_reg, DefaultSkin);

  if (app.Initialize ())
    csDefaultRunLoop(object_reg);

  return 0;
}
