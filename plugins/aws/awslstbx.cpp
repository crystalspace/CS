/*
    Copyright (C) 2001 by Christopher Nelson

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
#include "awslstbx.h"
#include "awsfparm.h"
#include "aws3dfrm.h"
#include "awskcfct.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/fontserv.h"
#include "csutil/scfstr.h"
#include "csutil/csevent.h"
#include "csutil/snprintf.h"
#include "iutil/evdefs.h"

#include <stdio.h>

const int awsListBox:: ctList = 0x0;
const int awsListBox:: ctTree = 0x1;

const int awsListBox:: signalSelected      = 0x1;
const int awsListBox:: signalScrolled      = 0x2;
const int awsListBox:: signalFocused       = 0x3;
const int awsListBox:: signalStateChanged  = 0x4;

const int alignLeft = 0;
const int alignCenter = 1;
const int alignRight = 2;

const int hsTreeBox = 0;
const int hsState = 1;
const int hsRow = 2;

///////////////////////////////////////////////////////////////////////////////
// awsListRow implementation
///////////////////////////////////////////////////////////////////////////////

awsListRow::~awsListRow ()
{
  delete[] cols;
}

int awsListRow::GetHeight (iAwsPrefManager *pm, int colcount)
{
  int minheight = 0;
  int i;

  for (i = 0; i < colcount; ++i)
  {
    int th = cols[i].GetHeight (pm);

    if (th > minheight) minheight = th;
  }

  return minheight;
}

///////////////////////////////////////////////////////////////////////////////
// awsListItem implementation
///////////////////////////////////////////////////////////////////////////////

awsListItem::~awsListItem ()
{
  if (text) text->DecRef ();
  if (image) image->DecRef ();
}

int awsListItem::GetHeight (iAwsPrefManager *pm)
{
  int ih = 0, iw = 0;
  int th = 0, tw = 0;

  if (image) image->GetOriginalDimensions (iw, ih);
  if (text) pm->GetDefaultFont ()->GetDimensions (text->GetData (), tw, th);

  if (ih > th)
    return ih;
  else
    return th;
}

///////////////////////////////////////////////////////////////////////////////
// awsListRowVector implementation
///////////////////////////////////////////////////////////////////////////////
int awsListRowVector::sortcol = 0;

int awsListRowVector::Compare (awsListRow* const& r1, awsListRow* const& r2)
{
  if (r1->cols[sortcol].text && r2->cols[sortcol].text)
    return strcmp (r1->cols[sortcol].text->GetData (),
		   r2->cols[sortcol].text->GetData ());
  else if (r1->cols[sortcol].text)
    return 1;
  else if (r2->cols[sortcol].text)
    return -1;
  else
    return 0;
}

int awsListRowVector::CompareKey (awsListRow* const& r1, iString* const& Key)
{
  if (r1->cols[sortcol].text)
    return strcmp (r1->cols[sortcol].text->GetData (), Key->GetData ());
  else
    return -1;
}

///////////////////////////////////////////////////////////////////////////////
// awsListBox implementation
///////////////////////////////////////////////////////////////////////////////

awsListBox::awsListBox () :
  is_down(false),
  mouse_is_over(false),
  is_switch(false),
  was_down(false),
  highlight(0),
  tree_collapsed(0),
  tree_expanded(0),
  tree_hline(0),
  tree_vline(0),
  hi_alpha_level(128),
  control_type(0),
  ncolumns(1),
  sel(0),
  map(0),
  map_size(0),
  map_dirty(true),
  scroll_start(0),
  drawable_count(0),
  sink(0),
  slot(0),
  scrollbar(0),
  actions(0)
{
}

awsListBox::~awsListBox ()
{
  delete actions;
}

const char *awsListBox::Type ()
{
  return "List Box";
}

bool awsListBox::Setup (iAws *_wmgr, iAwsComponentNode *settings)
{
  iString *tn1 = 0, *tn2 = 0;
  char buf[64];
  int i;
  int sb_h, sb_w;
  int border = 3;
  float min = 0, max = 0, change = 1, bigchange = 1;

  if (!awsPanel::Setup (_wmgr, settings)) return false;

  iAwsPrefManager *pm = WindowManager ()->GetPrefMgr ();

  actions = new awsActionDispatcher(WindowManager());
  actions->Register ("InsertItem", &InsertItem);
  actions->Register ("DeleteItem", &DeleteItem);
  actions->Register ("GetSelectedItem", &GetSelectedItem);
  actions->Register ("GetItem", &GetItem);
  actions->Register ("ClearList", &ClearList);

  pm->LookupIntKey ("ScrollBarHeight", sb_h);
  pm->LookupIntKey ("ScrollBarWidth", sb_w);
  pm->GetInt (settings, "HiAlpha", hi_alpha_level);
  pm->GetInt (settings, "Columns", ncolumns);
  pm->GetInt (settings, "Type", control_type);
  pm->GetInt (settings, "DefaultSortCol", sortcol);

  tree_collapsed = pm->GetTexture ("TreeCollapsed");
  tree_expanded = pm->GetTexture ("TreeExpanded");
  tree_vline = pm->GetTexture ("TreeVertLine");
  tree_hline = pm->GetTexture ("TreeHorzLine");
  tree_chke = pm->GetTexture ("TreeChkUnmarked");
  tree_chkf = pm->GetTexture ("TreeChkMarked");
  tree_grpe = pm->GetTexture ("TreeGrpUnmarked");
  tree_grpf = pm->GetTexture ("TreeGrpMarked");

  pm->GetString (settings, "Background", tn1);
  pm->GetString (settings, "Highlight", tn2);

  if(style == fsBump) border = 5;
  else if(style == fsSimple) border = 1;

  rows.SetSortCol (sortcol);

  if (tn1)
    bkg = pm->GetTexture (tn1->GetData (), tn1->GetData ());

  if (tn2) highlight = pm->GetTexture (tn2->GetData (), tn2->GetData ());

  // Make sure we have at least one column
  ncolumns = (ncolumns < 1 ? 1 : ncolumns);

  // Create new column region.
  columns = new awsListColumn[ncolumns];
  memset (columns, 0, sizeof (awsListColumn) * ncolumns);

  // Get user prefs for the column headers
  for (i = 0; i < ncolumns; ++i)
  {
    cs_snprintf (buf, 64, "Column%dImg", i);
    pm->GetString (settings, buf, tn1);
    cs_snprintf (buf, 64, "Column%dBkg", i);
    pm->GetString (settings, buf, tn2);
    cs_snprintf (buf, 64, "Column%dCaption", i);
    pm->GetString (settings, buf, columns[i].caption);
    cs_snprintf (buf, 64, "Column%dWidth", i);
    pm->GetInt (settings, buf, columns[i].width);
    cs_snprintf (buf, 64, "Column%dAlign", i);
    pm->GetInt (settings, buf, columns[i].align);

    if (tn1)
      columns[i].image = pm->GetTexture (tn1->GetData (), tn1->GetData ());

    if (tn2)
      columns[i].bkg = pm->GetTexture (tn2->GetData (), tn2->GetData ());
  }

  // Setup embedded scrollbar
  scrollbar = new awsScrollBar;

  awsKeyFactory sbinfo(WindowManager());

  sbinfo.Initialize ("vertscroll", "Scroll Bar");

  sbinfo.AddRectKey (
      "Frame",
      csRect (
        Frame ().Width () - sb_w - 1,
        border,
        Frame ().Width () - 1,
        Frame ().Height () - 1));

  sbinfo.AddIntKey ("Orientation", awsScrollBar::sboVertical);

  scrollbar->SetParent (this);
  scrollbar->Setup (_wmgr, sbinfo.GetThisNode ());

  scrollbar->SetProperty ("Change", (intptr_t)&change);
  scrollbar->SetProperty ("BigChange", (intptr_t)&bigchange);
  scrollbar->SetProperty ("Max", (intptr_t)&max);
  scrollbar->SetProperty ("Min", (intptr_t)&min);

  // Setup trigger
  awsSink* _sink = new awsSink (WindowManager());
  _sink->SetParm ((intptr_t)this);
  sink = _sink;

  sink->RegisterTrigger ("ScrollChanged", &ScrollChanged);

  slot = new awsSlot ();

  slot->Connect (
      scrollbar,
      awsScrollBar::signalChanged,
      sink,
      sink->GetTriggerID ("ScrollChanged"));

  return true;
}

bool awsListBox::GetProperty (const char *name, intptr_t *parm)
{
  return awsPanel::GetProperty (name, parm);
}

bool awsListBox::SetProperty (const char *name, intptr_t parm)
{
  return awsPanel::SetProperty (name, parm);
}

void awsListBox::ScrollChanged (intptr_t sk, iAwsSource *source)
{
  awsListBox *lb = (awsListBox *)sk;
  float *curval = 0;

  source->GetComponent ()->GetProperty ("Value", (intptr_t*)&curval);

  lb->UpdateMap ();
  lb->scroll_start = (int) *curval;
  if (lb->scroll_start > lb->map_size - lb->drawable_count)
    lb->scroll_start = lb->map_size - lb->drawable_count;

  if (lb->scroll_start < 0) lb->scroll_start = 0;

  lb->Broadcast(awsListBox::signalScrolled);
  lb->Invalidate ();
}

void awsListBox::UpdateMap ()
{
  if (map_dirty)
  {
    int start = 0;

    map_dirty = false;
    map_size = 0;
    delete[] map;

    // Traverse once for the count of visible items
    map_size = CountVisibleItems (&rows);
    map = new awsListRow *[map_size];

    // Set the scroll bar's max position.  Since we're using the pagesize
    // (amount visible) property of the scrollbar, the max position will be the
    // total number of rows.  The pagesize will be the number of visible rows,
    // and the current value of the scrollbar will be the topmost visible row.
    float max_scroll;
    max_scroll = map_size;
    scrollbar->SetProperty ("Max", (intptr_t)&max_scroll);

    // Map out items
    MapVisibleItems (&rows, start, map);
  }
}

int awsListBox::CountVisibleItems (awsListRowVector *v)
{
  size_t i;
  int count = 0;
  for (i = 0; i < v->Length (); ++i)
  {
    awsListRow *r = (awsListRow *) ((*v)[i]);

    // count one for this item
    ++count;
    if (r->children && r->expanded)
      count += CountVisibleItems (r->children);
  }

  return count;
}


void awsListBox::MapVisibleItems (
  awsListRowVector *v,
  int &start,
  awsListRow **map)
{
  size_t i;
  for (i = 0; i < v->Length (); ++i)
  {
    awsListRow *r = (awsListRow *) ((*v)[i]);

    map[start++] = r;

    if (r->children && r->expanded)
      MapVisibleItems (r->children, start, map);
  }
}

static int DoFindItem (awsListRowVector *v, iString *text, bool with_delete)
{
  v->sortcol = v->local_sortcol;
  int i = (int)v->FindKey (v->KeyCmp(text));

  if (i>=0)
  {
    if (with_delete) v->DeleteIndex (i);

    return i;
  }
  else
  {
    size_t j;
    for (i = 0; i < (int)v->Length (); ++i)
    {
      awsListRow *r = (awsListRow *) ((*v)[i]);
      if (r->children &&
        (j = DoFindItem (r->children, text, with_delete)) >= 0)
      {
        if (with_delete) r->children->DeleteIndex (j);

        return (int)j;
      }
    }
  }

  return -1;
}

static void DoRecursiveClearList (awsListRowVector *v)
{
  size_t i;

  for (i = 0; i < v->Length (); ++i)
  {
    awsListRow *r = (awsListRow *) ((*v)[i]);
    if (r->children)
    {
      DoRecursiveClearList (r->children);
      delete r->children;
    }
  }

  v->DeleteAll ();
}

/////////////// Scripted Actions //////////////////////////////////////////////

void awsListBox::InsertItem (intptr_t owner, iAwsParmList* parmlist)
{
  if (!parmlist)
    return;

  awsListBox *lb = (awsListBox *)owner;

  char buf[50];
  int i;
  iString *str;

  // Create a new row and zero it out.
  awsListRow *row = new awsListRow;
  memset (row, 0, sizeof (awsListRow));

  // Create a new set of columns and zero them out.
  row->cols = new awsListItem[lb->ncolumns];
  memset (row->cols, 0, sizeof (awsListItem) * lb->ncolumns);

  parmlist->GetOpaque ("parent", (intptr_t*)&row->parent);
  row->selectable = true;
  parmlist->GetBool ("selectable", &(row->selectable));

  // Fill in the columns by looking for several parameters: textX, imageX,
  // txtalignX, imgalignX, statefulX, stateX, groupstateX, selectableX
  for (i = 0; i < lb->ncolumns; ++i)
  {
    cs_snprintf (buf, 50, "text%d", i);
    if (parmlist->GetString (buf, &(row->cols[i].text)))
      row->cols[i].text->IncRef ();

    cs_snprintf (buf, 50, "image%d", i);
    if (parmlist->GetString (buf, &str))
    {
      row->cols[i].image = lb->WindowManager ()->GetPrefMgr ()->GetTexture (
          str->GetData (),
          str->GetData ());
      if (row->cols[i].image) row->cols[i].image->IncRef ();
    }

    cs_snprintf (buf, 50, "stateful%d", i);
    parmlist->GetBool (buf, &(row->cols[i].has_state));

    cs_snprintf (buf, 50, "state%d", i);
    parmlist->GetBool (buf, &(row->cols[i].state));

    cs_snprintf (buf, 50, "groupstate%d", i);
    parmlist->GetBool (buf, &(row->cols[i].group_state));

    cs_snprintf (buf, 50, "aligntxt%d", i);
    parmlist->GetInt (buf, &(row->cols[i].txt_align));

    cs_snprintf (buf, 50, "alignimg%d", i);
    parmlist->GetInt (buf, &(row->cols[i].txt_align));

    cs_snprintf (buf, 50, "param%d", i);
    parmlist->GetInt (buf, &(row->cols[i].param));
  }

  // Add the item
  if (row->parent)
  {
    if (row->parent->children == 0)
    {
      row->parent->children = new awsListRowVector ();
      row->parent->children->SetSortCol (lb->sortcol);
    }

    row->parent->children->Push (row);
  }
  else
    lb->rows.Push (row);

  // Pass back the id of this row, in case they want it.
  parmlist->AddOpaque ("id", (intptr_t)row);

  lb->map_dirty = true;
}

void awsListBox::DeleteItem (intptr_t owner, iAwsParmList* parmlist)
{
  if (!parmlist)
    return;
  
  awsListBox *lb = (awsListBox *)owner;

  int i, selidx = -1;
  iString *str = 0;

  // Try and find out what they're searching for
  if (!parmlist->GetString ("text", &str))
    if (!parmlist->GetString ("id", &str)) return ;

  if (lb->sel)
    selidx = (int)lb->rows.Find (lb->sel);

  i = DoFindItem (&lb->rows, str, true);
  if (i == selidx && selidx > -1)
  {
    int startidx=selidx;
    while (selidx < (int)lb->rows.Length () &&
	   !((awsListRow*)lb->rows[selidx])->selectable)
      selidx++;

    if (selidx >= (int)lb->rows.Length ())
    {
      selidx = MIN (startidx, (int)lb->rows.Length ()-1);
      while (selidx >= 0 && !((awsListRow*)lb->rows[selidx])->selectable)
        selidx--;
    }

    if (selidx > -1 && (size_t)selidx < lb->rows.Length ())
    {
      lb->sel = (awsListRow*) lb->rows[selidx];
      lb->Broadcast (awsListBox::signalSelected);
    }
    else
      lb->sel = 0;
    lb->Invalidate ();
  }
  // Pass back the result, in case they want it
  parmlist->AddInt ("result", (int)i);

  lb->map_dirty = true;
}

void awsListBox::GetSelectedItem (intptr_t owner, iAwsParmList* parmlist)
{
  if (!parmlist)
    return;
  
  awsListBox *lb = (awsListBox *)owner;
  parmlist->AddBool ("success", lb->GetItems (lb->sel, parmlist));
}

void awsListBox::GetItem (intptr_t owner, iAwsParmList* parmlist)
{
  if (!parmlist)
    return;
  
  awsListBox *lb = (awsListBox *)owner;
  int row=-1;
  if (parmlist->GetInt ("row", &row) && row >= -1 &&
      (size_t)row < lb->rows.Length ())
    parmlist->AddBool ("success", lb->GetItems ((awsListRow*)lb->rows[row],
      parmlist));
  else
    parmlist->AddBool ("success", false);
}

bool awsListBox::GetItems (awsListRow *row, iAwsParmList* parmlist)
{
  if (!parmlist)
    return false;
  
  bool succ = false;

  if (row)
  {
    int i;
    char buf[50];

    bool *state = new bool[ncolumns];
    int  *param = new int [ncolumns];

    iString **str = new iString *[ncolumns];

    bool *usedt = new bool[ncolumns];
    bool *useds = new bool[ncolumns];
    bool *usedp = new bool[ncolumns];

    for (i = 0; i < ncolumns; ++i)
    {
      usedt[i] = false;
      useds[i] = false;
      usedp[i] = false;
    }

    // check if they want the text or state or what. then return those in the
    // parmlist
    for (i = 0; i < ncolumns; ++i)
    {
      cs_snprintf (buf, 50, "text%d", i);
      if (parmlist->GetString (buf, &str[i]))
      {
        str[i] = row->cols[i].text;
        usedt[i] = true;
      }

      cs_snprintf (buf, 50, "state%d", i);
      if (parmlist->GetBool (buf, &state[i]))
      {
        state[i] = row->cols[i].state;
        useds[i] = true;
      }

      cs_snprintf (buf, 50, "param%d", i);
      if (parmlist->GetInt (buf, &param[i]))
      {
        param[i] = row->cols[i].param;
        usedp[i] = true;
      }
    }

    parmlist->Clear ();

    // return parmlist
    for (i = 0; i < ncolumns; ++i)
    {
      if (usedt[i])
      {
        cs_snprintf (buf, 50, "text%d", i);
        parmlist->AddString (buf, str[i]->GetData());
      }

      if (useds[i])
      {
        cs_snprintf (buf, 50, "state%d", i);
        parmlist->AddBool (buf, state[i]);
      }

      if (usedp[i])
      {
        cs_snprintf (buf, 50, "param%d", i);
        parmlist->AddInt (buf, param[i]);
      }
    }

    delete [] state;
    delete [] str;
    delete [] param;
    delete [] useds;
    delete [] usedt;
    delete [] usedp;
    succ = true;
  }
  return succ;
}

void awsListBox::ClearList (intptr_t owner, iAwsParmList* )
{
  awsListBox *lb = (awsListBox *)owner;

  DoRecursiveClearList (&lb->rows);

  // Clear the selected item.
  lb->sel = 0;
  lb->scroll_start = 0;

  lb->map_dirty = true;
}

bool awsListBox::Execute (const char *action, iAwsParmList* parmlist)
{
  if (awsPanel::Execute (action, parmlist)) return true;

  actions->Execute (action, (intptr_t)this, parmlist);

  return false;
}

void awsListBox::ClearGroup ()
{
  csEvent Event;

  Event.Type = csevGroupOff;

  iAwsComponent* cmp = Parent()->GetTopChild();
  while(cmp)
  {
    if (cmp && cmp != this) cmp->HandleEvent (Event);
	cmp = cmp->ComponentBelow();
  }
}

bool awsListBox::RecursiveClearPeers (awsListItem *itm, awsListRow *row)
{
  int i;
  for (i = 0; i < ncolumns; ++i)
  {
    // If this is it, then clear it's friends
    if (&(row->cols[i]) == itm)
    {
      if (row->parent)
      {
        size_t j;
        for (j = 0; j < row->parent->children->Length (); ++j)
        {
          awsListRow *crow = (awsListRow *)row->parent->children->Get (j);
          crow->cols[i].state = false;
        }
      }

      return true;
    }
    else if (row->children)
    {
      // Otherwise, recusively descend the tree.
      size_t j;

      // Search through list for this guy, and clear his peers
      for (j = 0; j < row->children->Length (); ++j)
      {
        awsListRow *crow = (awsListRow *)row->children->Get (j);
        if (RecursiveClearPeers (itm, crow)) return true;
      } // end for j (number of rows)
    }
  }

  return false;
}

void awsListBox::ClearPeers (awsListItem *itm)
{
  size_t j;

  // Search through list for this guy, and clear his peers
  for (j = 0; j < rows.Length (); ++j)
  {
    awsListRow *row = (awsListRow *)rows[j];
    if (RecursiveClearPeers (itm, row)) return ;
  }     // end for j (number of rows)
}

void awsListBox::ClearHotspots ()
{
  hotspots.DeleteAll ();
}

bool awsListBox::HandleEvent (iEvent &Event)
{
  if (awsPanel::HandleEvent (Event)) return true;

  switch (Event.Type)
  {
    case csevGroupOff:
      if (is_down && is_switch)
      {
        is_down = false;
        Invalidate ();
      }

      return true;
      break;
  }

  return false;
}

int awsListBox::GetRowDepth (awsListRow *row)
{
  int depth = 0;
  awsListRow *cur = row->parent;

  // Find out how deep we are.
  while (cur)
  {
    ++depth;
    cur = cur->parent;
  }

  return depth;
}

bool awsListBox::IsLastChild (awsListRow *row)
{
  awsListRow *v = row->parent;
  size_t i;

  if (!v)
  {
    i = rows.Find (row);
    return i == rows.Length () - 1;
  }
  else
  {
    i = v->children->Find (row);
    return i == v->children->Length () - 1;
  }
}

void awsListBox::OnDraw (csRect clip)
{

  awsPanel::OnDraw(clip);

  iGraphics2D *g2d = WindowManager ()->G2D ();

  //iGraphics3D *g3d = WindowManager()->G3D();
  int hi2 = WindowManager ()->GetPrefMgr ()->GetColor (AC_HIGHLIGHT2);
  int lo2 = WindowManager ()->GetPrefMgr ()->GetColor (AC_SHADOW2);
  int i;
  int border = 3;
  int sb_w, sb_h;

  WindowManager ()->GetPrefMgr ()->LookupIntKey ("ScrollBarHeight", sb_h);
  WindowManager ()->GetPrefMgr ()->LookupIntKey ("ScrollBarWidth", sb_w);

  ClearHotspots ();


  if(style == fsBump) border = 5;
  else if(style == fsSimple) border = 1;

  int starty = Frame ().ymin + border;
  int startx = Frame ().xmin + border;

  int x = startx, y = starty;
  int hch = 15;

  for (i = 0; i < ncolumns; ++i)
  {
    if (columns[i].caption)
    {
      int tw, th, tx, ty, mcc;
      int hcw;

      if (i == ncolumns - 1)
        hcw = Frame ().xmax - x - border - sb_w;
      else
        hcw = columns[i].width;

      mcc = WindowManager ()->GetPrefMgr ()->GetDefaultFont ()->GetLength (
          columns[i].caption->GetData (),
          hcw - 5);

      scfString tmp (columns[i].caption->GetData ());
      tmp.Truncate (mcc);

      // Get the size of the text
      WindowManager ()->GetPrefMgr ()->GetDefaultFont ()->GetDimensions (
          tmp.GetData (),
          tw,
          th);

      // Calculate the center
      ty = (hch >> 1) - (th >> 1);

      switch (columns[i].align)
      {
        case alignRight:  tx = hcw - tw - 2; break;

        case alignCenter: tx = (hcw >> 1) - (tw >> 1); break;

        default:          tx = 2; break;
      }

      // Draw the text
      g2d->Write (
          WindowManager ()->GetPrefMgr ()->GetDefaultFont (),
          x + tx,
          y + ty,
          WindowManager ()->GetPrefMgr ()->GetColor (AC_TEXTFORE),
          -1,
          tmp.GetData ());

      g2d->DrawLine (x, y, x + hcw, y, hi2);
      g2d->DrawLine (x, y, x, y + hch, hi2);
      g2d->DrawLine (x, y + hch, x + hcw, y + hch, lo2);
      g2d->DrawLine (x + hcw, y, x + hcw, y + hch, lo2);
    }

    // Next column
    x += columns[i].width;
  }

  // Setup y in proper place.
  y += hch + 2;

  // Fix drawable count
  drawable_count = 0;

  // Now begin to draw actual list
  if (rows.Length ())
  {
    awsListRow *row;
    awsListRow *start;

    bool stop_drawing = false;
    bool draw_this_time = true;

    UpdateMap ();

    if (map == 0)
      start = (awsListRow *)rows[0];
    else
      start = map[scroll_start];

    row = start;

    while (stop_drawing == false)
    {
      if (draw_this_time)
      {
        // Reset row start point
        x = startx;
        stop_drawing = DrawItemsRecursively (
            row,
            x,
            y,
            border,
            GetRowDepth (row),
            IsLastChild (row));
      }

      draw_this_time = true;

      // Find parent of current row.  If there is no parent, then go to the
      // next row item.
      awsListRow *parent = row->parent;

      if (parent == 0)
      { // Get next item from main list
        int i = (int)rows.Find (row);

        // This should never occur
        if (i == -1)
        {
          printf ("awslistbox: bug: couldn't find current row!\n");
          return ;
        }
        else
          ++i;

        // If we're done, leave
        if ((size_t)i >= rows.Length ())
          break;
        else
          row = (awsListRow *)rows[i];
      } // end if no parent.
      else
      {
        int i = (int)parent->children->Find (row);

        // This should never occur
        if (i == -1)
        {
          printf ("awslistbox: bug: couldn't find current row!\n");
          return ;
        }
        else
          ++i;

        if ((size_t)i >= parent->children->Length ())
        {
          row = parent;
          draw_this_time = false;
        }
        else
          row = (awsListRow *)parent->children->Get (i);
      }
    }   // end while draw items recursively

    // Map to a float to set PageSize for scrollbar
    float drawable_float=drawable_count;
    scrollbar->SetProperty ("PageSize", (intptr_t)&drawable_float);
  } // end if there are any rows to draw
}

bool awsListBox::DrawItemsRecursively (
  awsListRow *row,
  int &x,
  int &y,
  int border,
  int depth,
  bool last_child)
{
  iGraphics2D *g2d = WindowManager ()->G2D ();
  iGraphics3D *g3d = WindowManager ()->G3D ();

  int ith = row->GetHeight (WindowManager ()->GetPrefMgr (), ncolumns);
  int i;

  int tbh = 0, tbw = 0;
  int orgx = x;

  // Find out if we need to leave now.
  if (y + ith > Frame ().ymax) return true;

  // Count how many we've drawn
  ++drawable_count;

  // Figure out how to draw boxes and stuff
  if (row->children && depth)
  {
    // When it's a child AND has children...
    tree_expanded->GetOriginalDimensions (tbw, tbh);
    g3d->DrawPixmap (
        (row->expanded ? tree_expanded : tree_collapsed),
        x + 2 + (tbw * (depth + 1)),
        y,
        tbw,
        tbh,
        0,
        0,
        tbw,
        tbh);

    g3d->DrawPixmap (
        tree_hline,
        x + 2 + (tbw * depth),
        y,
        tbw,
        tbh,
        0,
        0,
        tbw,
        tbh);

    if (last_child)
      g3d->DrawPixmap (
          tree_vline,
          x + 2 + (tbw * depth),
          y,
          tbw,
          ith >> 1,
          0,
          0,
          tbw,
          tbh);
    else
      g3d->DrawPixmap (
          tree_vline,
          x + 2 + (tbw * depth),
          y,
          tbw,
          ith + 2,
          0,
          0,
          tbw,
          tbh);

    // Create hot spot for expand/collapse
    awsListHotspot *hs = new awsListHotspot;

    hs->obj = (intptr_t)row;
    hs->type = hsTreeBox;
    hs->r.Set (
        x + 2 + (tbw * (depth + 1)),
        y,
        x + 2 + (tbw * (depth + 1)) + tbw,
        y + tbh);

    hotspots.Push (hs);
  }
  else if (row->children)
  {
    // Draw tree box if needed
    tree_expanded->GetOriginalDimensions (tbw, tbh);
    g3d->DrawPixmap (
        (row->expanded ? tree_expanded : tree_collapsed),
        x + 2,
        y,
        tbw,
        tbh,
        0,
        0,
        tbw,
        tbh);

    // Create hot spot for expand/collapse
    awsListHotspot *hs = new awsListHotspot;

    hs->obj = (intptr_t)row;
    hs->type = hsTreeBox;
    hs->r.Set (x + 2, y, x + 2 + tbw, y + tbh);

    hotspots.Push (hs);
  }
  else if (depth)
  {
    // Draw child lines if needed and figure out where to draw stuff at.
    tree_expanded->GetOriginalDimensions (tbw, tbh);
    g3d->DrawPixmap (
        tree_hline,
        x + 2 + (tbw * depth),
        y,
        tbw,
        tbh,
        0,
        0,
        tbw,
        tbh);

    if (last_child)
      g3d->DrawPixmap (
          tree_vline,
          x + 2 + (tbw * depth),
          y,
          tbw,
          ith >> 1,
          0,
          0,
          tbw,
          tbh);
    else
      g3d->DrawPixmap (
          tree_vline,
          x + 2 + (tbw * depth),
          y,
          tbw,
          ith + 2,
          0,
          0,
          tbw,
          tbh);
  }

  // Draw columns
  for (i = 0; i < ncolumns; ++i)
  {
    int tw = 0, th = 0, tx = 0, ty = 0, mcc; // text width, hght, pos, max len.
    int cw;                 // column width
    int iw = 0, ih = 0;     // stateful image width and height
    int iws = 0;            // stateful image spacer
    int iiw = 0, iih = 0;   // item image width and height
    int iix = 0, iiy = 0;   // item image x and y
    int iiws = 0;           // item image spacer
    iTextureHandle *si = 0; // stateful image

    // Text to truncate
    scfString tmp (row->cols[i].text->GetData ());

    // Get column width
    if (i == ncolumns - 1)
      cw = Frame ().xmax - x - border;
    else if (i == 0 && depth && row->children)
      cw = columns[i].width - (tbw * (depth + 2));
    else if (i == 0 && row->children)
      cw = columns[i].width - (tbw * depth);
    else if (i == 0 && depth)
      cw = columns[i].width - (tbw * (depth + 1));
    else
      cw = columns[i].width;

    // If this has state, get the size of the state image
    if (row->cols[i].has_state) tree_chke->GetOriginalDimensions (iw, ih);

    // If this has an image, get the size of the image
    if (row->cols[i].image)
      row->cols[i].image->GetOriginalDimensions (iiw, iih);

    // Get the size of the text and truncate it
    if (row->cols[i].text)
    {
      mcc = WindowManager ()->GetPrefMgr ()->GetDefaultFont ()->GetLength (
          row->cols[i].text->GetData (),
          cw - 5 - iw);

      tmp.Truncate (mcc);

      // Get the size of the text
      WindowManager ()->GetPrefMgr ()->GetDefaultFont ()->GetDimensions (
          tmp.GetData (),
          tw,
          th);

      // Calculate the center
      ty = (ith >> 1) - (th >> 1);
    }   // end if text is good

    // Perform alignment of text/state
    switch (row->cols[i].txt_align)
    {
      case alignRight:
        tx = cw - tw - 2;
        iws = -iw + 2;
        break;

      case alignCenter:
        tx = (cw >> 1) - ((tw + iw) >> 1);
        break;

      default:
        if (i == 0 && depth && row->children)
          tx = 2 + (tbw * (depth + 2));
        else if (row->children && i == 0)
          tx = 2 + tbw;
        else if (depth && i == 0)
          tx = 2 + (tbw * (depth + 1));
        else
          tx = 2;

        iws = iw + 2;
        break;
    }   // end switch text alignment
    if (row->cols[i].image)
    {
      // Perform alignment of image
      switch (row->cols[i].img_align)
      {
        case alignRight:
          iix = cw - iiw - 2;

          // adjust spacer if needed
          if (row->cols[i].text && row->cols[i].txt_align == alignRight)
          {
            iiws = iws;
            iws -= iiw + 2;
          }
          else
            iiws = -iw + 2;

          break;

        case alignCenter:
          iix = (cw >> 1) - ((iiw + iw + tw) >> 1);
          tx += iiw + 2;
          break;

        default:
          if (i == 0 && depth && row->children)
            iix = 2 + (tbw * (depth + 2));
          else if (row->children && i == 0)
            iix = 2 + tbw;
          else if (depth && i == 0)
            iix = 2 + (tbw * (depth + 1));
          else
            iix = 2;

          // Adjust spacer
          if (row->cols[i].text && row->cols[i].txt_align == alignRight)
          {
            iiws = iws;
            iws += iiw + 2;
          }
          else
            iiws = iw + 2;

          break;
      } // end switch text alignment

      // Draw image
      g3d->DrawPixmap (
          row->cols[i].image,
          x + iix + iiws,
          y + iiy,
          (cw < iiw ? cw : iiw),
          (ith < iih ? ith : iiw),
          0,
          0,
          iiw,
          iih);
    }   // end if there's an image
    if (row->cols[i].text)
    {
      // Draw the text
      g2d->Write (
          WindowManager ()->GetPrefMgr ()->GetDefaultFont (),
          x + tx + iws,
          y + ty,
          WindowManager ()->GetPrefMgr ()->GetColor (AC_TEXTFORE),
          -1,
          tmp.GetData ());
    }   // end if text is good

    // Draw state if there is some
    if (row->cols[i].has_state)
    {
      if (row->cols[i].group_state)
      {
        if (row->cols[i].state)
          si = tree_grpf;
        else
          si = tree_grpe;
      }
      else
      {
        if (row->cols[i].state)
          si = tree_chkf;
        else
          si = tree_chke;
      }

      g3d->DrawPixmap (si, x + tx, y, iw, ih, 0, 0, iw, ih);

      // Create hot spot for expand/collapse
      awsListHotspot *hs = new awsListHotspot;

      hs->obj = (intptr_t)&(row->cols[i]);
      hs->type = hsState;
      hs->r.Set (x + tx, y, x + tx + iw, y + ih);

      hotspots.Push (hs);
    }   // end if stateful

    // Next column
    x += columns[i].width;
  }     // end for i (number of cols)

  // Create hot spot for this row
  awsListHotspot *hs = new awsListHotspot;

  hs->obj = (intptr_t)row;
  hs->type = hsRow;
  hs->r.Set (Frame ().xmin + border, y, Frame ().xmax - border, y + ith);

  hotspots.Push (hs);

  // Draw the highlight, if we're highlighted
  if (sel == row)
  {
    int hw, hh;

    if (highlight)
    {
      highlight->GetOriginalDimensions (hw, hh);
      g3d->DrawPixmap (
          highlight,
          Frame ().xmin + border,
          y - 1,
          Frame ().Width () - (border * 2),
          ith + 2,
          0,
          0,
          hw,
          hh,
          hi_alpha_level);
    }
  }

  // next row.
  y += ith + (ith >> 2);

  // Draw children
  if (row->children && row->expanded)
  {
    for (size_t j = 0; j < row->children->Length (); ++j)
    {
      int cx = orgx;
      awsListRow *newrow = (awsListRow *)row->children->Get (j);

      if (
        DrawItemsRecursively (
            newrow,
            cx,
            y,
            border,
            (depth ? depth + 2 : depth + 1),
            ((size_t)i == row->children->Length () - 1 ? true : false)))
        return true;
    }
  }

  // false means that we've not yet hit the bottom of the barrel.
  return false;
}

bool awsListBox::OnMouseDown (int

/*button*/, int x, int y)
{
  size_t i;

  for (i = 0; i < hotspots.Length (); ++i)
  {
    awsListHotspot *hs = hotspots[i];
    if (hs->r.Contains (x, y))
    {
      switch (hs->type)
      {
        case hsTreeBox:
          {
            awsListRow *row = (awsListRow *)hs->obj;
            if (row->expanded)
              row->expanded = false;
            else
              row->expanded = true;

            map_dirty = true;
            Invalidate ();
            return true;
          }
          break;

        case hsState:
          {
            awsListItem *itm = (awsListItem *)hs->obj;

            if (itm->group_state) ClearPeers (itm);

            if (itm->state)
              itm->state = false;
            else
              itm->state = true;

            Broadcast (awsListBox::signalStateChanged);
            Invalidate ();
            return true;
          }
          break;

        case hsRow:
          {
            awsListRow *row = (awsListRow *)hs->obj;
            if (row->selectable)
            {
              sel = row;
              Broadcast (awsListBox::signalSelected);
              Invalidate ();
            }
            return true;
          }
          break;
      } // end switch type
    }   // end if contains
  }     // end for
  return false;
}

bool awsListBox::OnKeyboard (const csKeyEventData& eventData)
{
  ///Changing selected row with arrow keys
  /// !!! Worked correctly only for awsListHotspot.type == hsRow
  if( !(WindowManager()->GetFlags() & AWSF_KeyboardControl) )
    return false;

  switch (eventData.codeCooked)
  {
    case CSKEY_DOWN:
    {
      awsListRow *parent = sel ? sel->parent: 0;

      if (parent == 0)
      { 
	int i = (int)rows.Find (sel);
	
	if(i < (int)rows.Length() -1 && rows.Length() > 0)
	{
	  ++i;
	  sel = (awsListRow *)rows[i];
	  Broadcast (awsListBox::signalSelected);

	  /// @@@ HACK ;-]

	  /// Change scroll bar position if selected component 
	  /// is in the bottom of our list box

	  awsListRow *row = 0;
	  UpdateMap ();
	  if (map)
	    row = map[drawable_count + scroll_start];

	  if (sel == row)
	    awsScrollBar::IncClicked ((intptr_t)scrollbar, 0);

	  ///@@@ END_HACK

	  return true;
	}
      }
    }

    return true;

    case CSKEY_UP:
    {
      awsListRow *parent = sel ? sel->parent: 0;

      if (parent == 0)
      { 
	int i = (int)rows.Find (sel);

	if(i > 0 && rows.Length () > 0)
	{
	  --i;
	  sel = (awsListRow *)rows[i];
	  Broadcast (awsListBox::signalSelected);

	  /// @@@ HACK ;-]

	  /// Change scroll bar position if selected component 
	  /// is in the top of our list box

	  awsListRow *row = 0;
	  UpdateMap ();
	  if (map)
		  row = map[scroll_start -1];

	  if (sel == row)
		  awsScrollBar::DecClicked ((intptr_t)scrollbar, 0);

	  ///@@@ END_HACK

	  return true;
	}
      }
    }
  }

  Invalidate ();

  return true;
}

void awsListBox::OnSetFocus ()
{
	Broadcast (signalFocused);
}

bool awsListBox::OnMouseExit ()
{
  mouse_is_over = false;
  Invalidate ();

  if (is_down && !is_switch) is_down = false;

  return true;
}

bool awsListBox::OnMouseEnter ()
{
  mouse_is_over = true;
  Invalidate ();
  return true;
}

void awsListBox::OnAdded ()
{
  AddChild (scrollbar);
}

void awsListBox::OnResized ()
{
  int w = scrollbar->Frame ().Width (); //, h = scrollbar->Frame ().Width ();

  csRect newFrame;
  newFrame.SetPos(Frame ().xmax - w - 1, Frame ().ymin + 2);
  newFrame.xmax = Frame ().xmax - 1;
  newFrame.ymax = Frame ().ymax - 2;
  scrollbar->ResizeTo(newFrame);
}

/************************************* List Box Factory ****************/

awsListBoxFactory::awsListBoxFactory (iAws *wmgr) :
  awsComponentFactory(wmgr)
{
  Register ("List Box");
  RegisterConstant ("lbfsBump", awsListBox::fsBump);
  RegisterConstant ("lbfsSunken", awsListBox::fsSunken);
  RegisterConstant ("lbfsRaised", awsListBox::fsRaised);
  RegisterConstant ("lbfsSimple", awsListBox::fsSimple);
  RegisterConstant ("lbfsFlat", awsListBox::fsFlat);
  RegisterConstant ("lbfsNone", awsListBox::fsNone);

  RegisterConstant ("lbtTree", awsListBox::ctTree);
  RegisterConstant ("lbtList", awsListBox::ctList);

  RegisterConstant ("lbAlignLeft", alignLeft);
  RegisterConstant ("lbAlignRight", alignRight);
  RegisterConstant ("lbAlignCenter", alignCenter);

  RegisterConstant ("signalListBoxSelectionChanged",
		    awsListBox::signalSelected);
  RegisterConstant ("signalListBoxScrolled", awsListBox::signalScrolled);
  RegisterConstant ("signalListBoxFocused", awsListBox::signalFocused);
  RegisterConstant ("signalListBoxStateChanged",
		    awsListBox::signalStateChanged);
}

awsListBoxFactory::~awsListBoxFactory ()
{
  // empty
}

iAwsComponent *awsListBoxFactory::Create ()
{
  return new awsListBox;
}
