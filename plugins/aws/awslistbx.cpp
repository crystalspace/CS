#include "cssysdef.h"
#include "awslistbx.h"
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

SCF_IMPLEMENT_IBASE(awsListBox)
SCF_IMPLEMENTS_INTERFACE(awsComponent)
SCF_IMPLEMENT_IBASE_END

const int awsListBox::fsBump   = 0x0;
const int awsListBox::fsSimple = 0x1;
const int awsListBox::fsRaised = 0x2;
const int awsListBox::fsSunken = 0x3;
const int awsListBox::fsNone   = 0x4;

const int awsListBox::ctList = 0x0;
const int awsListBox::ctTree = 0x1;

const int awsListBox::signalSelected=0x1;
const int awsListBox::signalScrolled=0x2;

const int alignLeft=0;
const int alignCenter=1;
const int alignRight=2;

const int hsTreeBox=0;
const int hsState=1;
const int hsRow=2;

//////////////////////////////////////////////////////////////////////////
// awsListRow implementation 
//

awsListRow::~awsListRow()
{
  if (cols)
    delete [] cols;
}

int 
awsListRow::GetHeight(iAwsPrefManager *pm, int colcount)
{
  int minheight=0;
  int i;

  for (i=0; i<colcount; ++i)
  {
    int th=cols[i].GetHeight(pm);

    if ( th > minheight)
      minheight=th;
  }

  return minheight;
}


//////////////////////////////////////////////////////////////////////////
// awsListItem implementation 
//

awsListItem::~awsListItem()
{
  SCF_DEC_REF(text);
  SCF_DEC_REF(image);
}

int
awsListItem::GetHeight(iAwsPrefManager *pm)
{
  int ih=0, iw=0;
  int th=0, tw=0;

  if (image) image->GetOriginalDimensions(iw, ih);
  if (text)  pm->GetDefaultFont()->GetDimensions(text->GetData(), tw, th);

  if (ih > th) return ih;
  else return th;
}

//////////////////////////////////////////////////////////////////////////
// awsListBox implementation 
//

awsListBox::awsListBox():is_down(false), mouse_is_over(false), 
is_switch(false), was_down(false),
bkg(NULL), highlight(NULL), 
tree_collapsed(NULL), tree_expanded(NULL),
tree_hline(NULL), tree_vline(NULL),
frame_style(0), alpha_level(92), hi_alpha_level(128),
control_type(0), ncolumns(1), sel(NULL)

{
  actions.Register("InsertItem", &InsertItem);
  actions.Register("DeleteItem", &DeleteItem);
  actions.Register("GetSelectedItem", &GetSelectedItem);
  actions.Register("ClearList", &ClearList);
}

awsListBox::~awsListBox()
{
}

char *
awsListBox::Type() 
{
  return "List Box";
}

bool
awsListBox::Setup(iAws *_wmgr, awsComponentNode *settings)
{
  iString   *tn1=NULL, *tn2=NULL;
  char buf[64];
  int i;
  int sb_h, sb_w;
  int border=3;

  if (!awsComponent::Setup(_wmgr, settings)) return false;

  iAwsPrefManager *pm=WindowManager()->GetPrefMgr();

  pm->LookupIntKey("OverlayTextureAlpha", alpha_level); // global get
  pm->LookupIntKey("ScrollBarHeight", sb_h);
  pm->LookupIntKey("ScrollBarWidth", sb_w);
  pm->GetInt(settings, "Style", frame_style);
  pm->GetInt(settings, "Alpha", alpha_level);          // local overrides, if present.
  pm->GetInt(settings, "HiAlpha", hi_alpha_level);          
  pm->GetInt(settings, "Columns", ncolumns);
  pm->GetInt(settings, "Type", control_type);
  pm->GetInt(settings, "DefaultSortCol", sortcol);

  tree_collapsed = pm->GetTexture("TreeCollapsed");
  tree_expanded = pm->GetTexture("TreeExpanded");
  tree_vline = pm->GetTexture("TreeVertLine");
  tree_hline = pm->GetTexture("TreeHorzLine");
  tree_chke = pm->GetTexture("TreeChkUnmarked");
  tree_chkf = pm->GetTexture("TreeChkMarked");
  tree_grpe = pm->GetTexture("TreeGrpUnmarked");
  tree_grpf = pm->GetTexture("TreeGrpMarked");

  pm->GetString(settings, "Background", tn1);
  pm->GetString(settings, "Highlight", tn2);

  switch (frame_style)
  {
  case fsBump:
    border=5;
    break;
  case fsSimple:
    border=1;
    break;
  }

  rows.SetSortCol(sortcol);

  if (tn1) bkg=pm->GetTexture(tn1->GetData(), tn1->GetData());
  else bkg=pm->GetTexture("Texture");

  if (tn2) highlight=pm->GetTexture(tn2->GetData(), tn2->GetData());

  // Make sure we have at least one column
  ncolumns=(ncolumns<1 ? 1 : ncolumns);

  // Create new column region.
  columns = new awsListColumn[ncolumns];
  memset(columns, 0, sizeof(awsListColumn) * ncolumns);

  // Get user prefs for the column headers
  for (i=0; i<ncolumns; ++i)
  {
    cs_snprintf(buf, 64, "Column%dImg", i);
    pm->GetString(settings, buf, tn1);
    cs_snprintf(buf, 64, "Column%dBkg", i);
    pm->GetString(settings, buf, tn2);
    cs_snprintf(buf, 64, "Column%dCaption", i);
    pm->GetString(settings, buf, columns[i].caption);
    cs_snprintf(buf, 64, "Column%dWidth", i);
    pm->GetInt(settings, buf, columns[i].width);
    cs_snprintf(buf, 64, "Column%dAlign", i);
    pm->GetInt(settings, buf, columns[i].align);

    if (tn1)
      columns[i].image = pm->GetTexture(tn1->GetData(), tn1->GetData());

    if (tn2)
      columns[i].bkg = pm->GetTexture(tn2->GetData(), tn2->GetData());
  }

  // Setup embedded scrollbar
  scrollbar = new awsScrollBar;

  awsKeyFactory sbinfo;

  sbinfo.Initialize(new scfString("vertscroll"), new scfString("Scroll Bar"));

  sbinfo.AddRectKey(new scfString("Frame"), 
                    csRect(Frame().Width()-sb_w-1, border, Frame().Width()-1, Frame().Height()-1));

  sbinfo.AddIntKey(new scfString("Style"), awsScrollBar::fsVertical);

  scrollbar->SetWindow(Window());
  scrollbar->SetParent(this);
  scrollbar->Setup(_wmgr, sbinfo.GetThisNode());
  
  // Setup trigger
  sink = new awsSink(this);

  sink->RegisterTrigger("ScrollChanged", &ScrollChanged);

  slot = new awsSlot();

  slot->Connect(scrollbar, awsScrollBar::signalChanged, sink, sink->GetTriggerID("ScrollChanged"));

  return true;
}

bool 
awsListBox::GetProperty(char *name, void **parm)
{
  if (awsComponent::GetProperty(name, parm)) return true;

  return false;
}

bool 
awsListBox::SetProperty(char *name, void *parm)
{
  if (awsComponent::SetProperty(name, parm)) return true;

  return false;
}

void 
awsListBox::ScrollChanged(void *sk, iAwsSource *source)     
{
  awsListBox *lb = (awsListBox *)lb;
  

}

static 
int
DoFindItem(awsListRowVector *v, iString *text, bool with_delete)
{
  int i=v->Find(text);

  if (i)
  {
    if (with_delete)
      v->Delete(i, true);

    return i;
  }
  else
  {
    int j;
    for (i=0; i<v->Length(); ++i)
    {
      awsListRow *r = (awsListRow *)((*v)[i]);
      if (r->children && (j=DoFindItem(r->children,text,with_delete))>=0)
      {
        if (with_delete)
          r->children->Delete(j, true);

        return j;
      }
    }
  }

  return -1;
}

static
void
DoRecursiveClearList(awsListRowVector *v)
{ 
  int i;

  for (i=0; i<v->Length(); ++i)
  {
    awsListRow *r = (awsListRow *)((*v)[i]);
    if (r->children)
    {
       DoRecursiveClearList(r->children);
       delete r->children;
    }
     
  }

  v->DeleteAll(true);
}


/////////////// Scripted Actions /////////////////////////////////////////////////////////////
void 
awsListBox::InsertItem(void *owner, iAwsParmList &parmlist)
{
  awsListBox *lb = (awsListBox *)owner;

  char buf[50];
  int i;
  iString *str;

  // Create a new row and zero it out.
  awsListRow *row = new awsListRow;
  memset(row, 0 , sizeof(awsListRow));

  // Create a new set of columns and zero them out.
  row->cols = new awsListItem[lb->ncolumns];
  memset(row->cols, 0, sizeof(awsListItem) * lb->ncolumns);

  parmlist.GetInt("parent", (int *)&(row->parent));
  parmlist.GetBool("selectable", &(row->selectable));

  /* Fill in the columns by looking for several parameters:
   *   textX, imageX, txtalignX, imgalignX, statefulX, stateX, groupstateX, 
   *   selectableX
   */
  for (i=0; i<lb->ncolumns; ++i)
  {
    cs_snprintf(buf, 50, "text%d", i);
    if (parmlist.GetString(buf, &(row->cols[i].text)))
      row->cols[i].text->IncRef();

    cs_snprintf(buf, 50, "image%d", i);
    if (parmlist.GetString(buf, &str))
      row->cols[i].image = lb->WindowManager()->GetPrefMgr()->GetTexture(str->GetData(), str->GetData());

    cs_snprintf(buf, 50, "stateful%d", i);
    parmlist.GetBool(buf, &(row->cols[i].has_state));

    cs_snprintf(buf, 50, "state%d", i);
    parmlist.GetBool(buf, &(row->cols[i].state));

    cs_snprintf(buf, 50, "groupstate%d", i);
    parmlist.GetBool(buf, &(row->cols[i].group_state));

    cs_snprintf(buf, 50, "aligntxt%d", i);
    parmlist.GetInt(buf, &(row->cols[i].txt_align));

    cs_snprintf(buf, 50, "alignimg%d", i);
    parmlist.GetInt(buf, &(row->cols[i].txt_align));
  }

  // Add the item
  if (row->parent)
  {
    if (row->parent->children==NULL)
    {
      row->parent->children = new awsListRowVector();
      row->parent->children->SetSortCol(lb->sortcol);
    }


    row->parent->children->Push(row);
  }
  else lb->rows.Push(row);

  // Pass back the id of this row, in case they want it.
  parmlist.AddInt("id", (int)row);
} 

void 
awsListBox::DeleteItem(void *owner, iAwsParmList &parmlist)
{
  awsListBox *lb = (awsListBox *)owner;

  int i;
  iString *str=NULL;


  // Try and find out what they're searching for
  if (!parmlist.GetString("text", &str))
    if (!parmlist.GetString("id", &str))
      return;
 
  i=DoFindItem(&lb->rows, str, true);

  // Pass back the result, in case they want it
  parmlist.AddInt("result", (int)i);
}


void 
awsListBox::GetSelectedItem(void *owner, iAwsParmList &parmlist)
{
  awsListBox *lb = (awsListBox *)owner;
  int i;
  char buf[50];
  
  bool state[lb->ncolumns];
  iString *str[lb->ncolumns];
  
  bool usedt[lb->ncolumns], useds[lb->ncolumns];

  for(i=0; i<lb->ncolumns; ++i)
  {
     usedt[i]=false;
     useds[i]=false;
  }
   
  
  // check if they want the text or state or what. then return those in the parmlist
  for (i=0; i<lb->ncolumns; ++i)
  {
    cs_snprintf(buf, 50, "text%d", i);
    if (parmlist.GetString(buf, &str[i]))
    {
       str[i]=lb->sel->cols[i].text;
       usedt[i]=true;
    }

    cs_snprintf(buf, 50, "state%d", i);
    if (parmlist.GetBool(buf, &state[i]))
    {
      state[i]=lb->sel->cols[i].state;
      useds[i]=true;
    }

  }

  parmlist.Clear();

  // return parmlist
  for (i=0; i<lb->ncolumns; ++i)
  {
    if (usedt[i])
    {
      cs_snprintf(buf, 50, "text%d", i);
      parmlist.AddString(buf, str[i]);
    }

    if (useds[i])
    {
      cs_snprintf(buf, 50, "state%d", i);
      parmlist.AddBool(buf, state[i]);
    }
  }

}

void 
awsListBox::ClearList(void *owner, iAwsParmList &parmlist)
{
  awsListBox *lb = (awsListBox *)owner;

  DoRecursiveClearList(&lb->rows);
}

bool 
awsListBox::Execute(char *action, iAwsParmList &parmlist)
{
  if (awsComponent::Execute(action, parmlist)) return true;

  actions.Execute(action, this, parmlist);

  return false;
}

void 
awsListBox::ClearGroup()
{
  csEvent Event;

  Event.Type = csevGroupOff;

  int i;
  for (i=0; i<Parent()->GetChildCount(); ++i)
  {
    iAwsComponent *cmp = Parent()->GetChildAt(i);

    if (cmp && cmp!=this)
      cmp->HandleEvent(Event);
  }
}

bool
awsListBox::RecursiveClearPeers(awsListItem *itm, awsListRow *row)
{
  int i;
  for (i=0; i<ncolumns; ++i)
  {
    // If this is it, then clear it's friends
    if (&(row->cols[i]) == itm)
    {
      if (row->parent)
      {
        int j;
        for (j=0; j<row->parent->children->Length(); ++j)
        {
          awsListRow *crow = (awsListRow *)row->parent->children->Get(j);
          crow->cols[i].state = false;
        }
      }

      return true;
    }
    else if (row->children)
    {
      // Otherwise, recusively descend the tree.
      int j;

      // Search through list for this guy, and clear his peers
      for (j=0; j<row->children->Length(); ++j)
      {
        awsListRow *crow = (awsListRow *)row->children->Get(j);
        if (RecursiveClearPeers(itm, crow))
          return true;
      } // end for j (number of rows)
    }
  }

  return false;
}

void 
awsListBox::ClearPeers(awsListItem *itm)
{
  int j;

  // Search through list for this guy, and clear his peers
  for (j=0; j<rows.Length(); ++j)
  {
    awsListRow *row = (awsListRow *)rows[j];
    if (RecursiveClearPeers(itm, row))
      return;

  } // end for j (number of rows)
}

void 
awsListBox::ClearHotspots()
{
  int i;
  for (i=0; i<hotspots.Length(); ++i)
  {
    awsListHotspot *hs = (awsListHotspot *)hotspots[i];
    delete hs;
  }

  hotspots.SetLength(0);
}

bool 
awsListBox::HandleEvent(iEvent& Event)
{
  if (awsComponent::HandleEvent(Event)) return true;

  switch (Event.Type)
  {
  case csevGroupOff:
    if (is_down && is_switch)
    {
      is_down=false;
      Invalidate();
    }
    return true;
    break;
  }

  return false;
}



void 
awsListBox::OnDraw(csRect clip)
{

  iGraphics2D *g2d = WindowManager()->G2D();
  //iGraphics3D *g3d = WindowManager()->G3D();

  int hi2   = WindowManager()->GetPrefMgr()->GetColor(AC_HIGHLIGHT2);
  int lo2   = WindowManager()->GetPrefMgr()->GetColor(AC_SHADOW2);    
  int i,j;
  int border=3;
  int sb_w, sb_h;

  WindowManager()->GetPrefMgr()->LookupIntKey("ScrollBarHeight", sb_h);
  WindowManager()->GetPrefMgr()->LookupIntKey("ScrollBarWidth", sb_w);

  ClearHotspots();

  aws3DFrame frame3d;

  frame3d.Draw(WindowManager(), Window(), Frame(), frame_style, bkg, alpha_level);

  switch (frame_style)
  {
  case fsBump:
    border=5;
    break;
  case fsSimple:
    border=1;
    break;
  }
  

  int starty=Frame().ymin+border;
  int startx=Frame().xmin+border;

  int x=startx, y=starty;
  int hch=15;

  for (i=0; i<ncolumns; ++i)
  {
    if (columns[i].caption)
    {
      int tw, th, tx, ty, mcc;
      int hcw;

      if (i==ncolumns-1)
        hcw = Frame().xmax-x-border-sb_w;
      else
        hcw = columns[i].width;

      mcc = WindowManager()->GetPrefMgr()->GetDefaultFont()->GetLength(columns[i].caption->GetData(), hcw-5);

      scfString tmp(columns[i].caption->GetData());
      tmp.Truncate(mcc);

      // Get the size of the text
      WindowManager()->GetPrefMgr()->GetDefaultFont()->GetDimensions(tmp.GetData(), tw, th);

      // Calculate the center
      ty = (hch>>1) - (th>>1);

      switch (columns[i].align)
      {
      case alignRight:
        tx = hcw-tw-2;
        break;

      case alignCenter:
        tx = (hcw>>1) -  (tw>>1);
        break;

      default:
        tx = 2;
        break;
      }

      // Draw the text
      g2d->Write(WindowManager()->GetPrefMgr()->GetDefaultFont(),
                 x+tx,
                 y+ty,
                 WindowManager()->GetPrefMgr()->GetColor(AC_TEXTFORE),
                 -1,
                 tmp.GetData());

      g2d->DrawLine(x, y, x+hcw, y, hi2);
      g2d->DrawLine(x, y, x, y+hch, hi2);
      g2d->DrawLine(x, y+hch, x+hcw, y+hch, lo2);
      g2d->DrawLine(x+hcw, y, x+hcw, y+hch, lo2);
    }

    // Next column
    x+=columns[i].width;
  }

  // Setup y in proper place.
  y+=hch+2;

  // Now begin to draw actual list
  for (j=0; j<rows.Length(); ++j)
  {
    x=startx;
    awsListRow *row = (awsListRow *)rows[j];

    if (DrawItemsRecursively(row, x, y, border, false, false))
      break;

  } // end for j (number of rows)
}

bool
awsListBox::DrawItemsRecursively(awsListRow *row, int &x, int &y, int border, int depth, bool last_child)
{
  iGraphics2D *g2d = WindowManager()->G2D();
  iGraphics3D *g3d = WindowManager()->G3D();

  int ith=row->GetHeight(WindowManager()->GetPrefMgr(), ncolumns);
  int i;

  int tbh=0, tbw=0;
  int orgx=x;

  // Find out if we need to leave now.
  if (y+ith > Frame().ymax) return true;

  // Figure out how to draw boxes and stuff
  if (row->children && depth)
  {
    // When it's a child AND has children...
    tree_expanded->GetOriginalDimensions(tbw, tbh);
    g3d->DrawPixmap((row->expanded ? tree_expanded : tree_collapsed),
                    x+2+(tbw*(depth+1)), y, 
                    tbw, tbh, 0,0, tbw, tbh);

    g3d->DrawPixmap(tree_hline, x+2+(tbw*depth), y, tbw, tbh, 0,0, tbw, tbh);

    if (last_child)
      g3d->DrawPixmap(tree_vline, x+2+(tbw*depth), y, tbw, ith>>1, 0,0, tbw, tbh);
    else
      g3d->DrawPixmap(tree_vline, x+2+(tbw*depth), y, tbw, ith+2, 0,0, tbw, tbh);    

    // Create hot spot for expand/collapse
    awsListHotspot *hs = new awsListHotspot;

    hs->obj = row;
    hs->type = hsTreeBox;
    hs->r.Set(x+2+(tbw*(depth+1)), y, x+2+(tbw*(depth+1))+tbw, y+tbh);

    hotspots.Push(hs);

  }
  else if (row->children)
  {
    // Draw tree box if needed
    tree_expanded->GetOriginalDimensions(tbw, tbh);
    g3d->DrawPixmap((row->expanded ? tree_expanded : tree_collapsed), 
                    x+2, y, tbw, tbh, 0,0, tbw, tbh);

    // Create hot spot for expand/collapse
    awsListHotspot *hs = new awsListHotspot;

    hs->obj = row;
    hs->type = hsTreeBox;
    hs->r.Set(x+2, y, x+2+tbw, y+tbh);

    hotspots.Push(hs);
  }
  else if (depth)
  {
    // Draw child lines if needed and figure out where to draw stuff at.
    tree_expanded->GetOriginalDimensions(tbw, tbh);
    g3d->DrawPixmap(tree_hline, x+2+(tbw*depth), y, tbw, tbh, 0,0, tbw, tbh);

    if (last_child)
      g3d->DrawPixmap(tree_vline, x+2+(tbw*depth), y, tbw, ith>>1, 0,0, tbw, tbh);
    else
      g3d->DrawPixmap(tree_vline, x+2+(tbw*depth), y, tbw, ith+2, 0,0, tbw, tbh);    
  }

  // Draw columns
  for (i=0; i<ncolumns; ++i)
  {
    int tw=0, th=0, tx=0, ty=0, mcc; // text width, height, position, max len.
    int cw;         // column width

    int iw=0, ih=0; // stateful image width and height
    int iws=0;      // stateful image spacer

    int iiw=0, iih=0; // item image width and height
    int iix=0, iiy=0; // item image x and y
    int iiws=0;       // item image spacer

    iTextureHandle *si=0; // stateful image

    // Text to truncate
    scfString tmp(row->cols[i].text->GetData());

    // Get column width
    if (i==ncolumns-1)
      cw = Frame().xmax-x-border;
    else if (i==0 && depth && row->children)
      cw = columns[i].width-(tbw*(depth+2));
    else if (i==0 && row->children)
      cw = columns[i].width-(tbw*depth);
    else if (i==0 && depth)
      cw = columns[i].width-(tbw*(depth+1));
    else
      cw = columns[i].width;

    // If this has state, get the size of the state image
    if (row->cols[i].has_state)
      tree_chke->GetOriginalDimensions(iw, ih);

    // If this has an image, get the size of the image
    if (row->cols[i].image)
      row->cols[i].image->GetOriginalDimensions(iiw, iih);

    // Get the size of the text and truncate it
    if (row->cols[i].text)
    {
      mcc = WindowManager()->GetPrefMgr()->GetDefaultFont()->GetLength(row->cols[i].text->GetData(), cw-5-iw);

      tmp.Truncate(mcc);

      // Get the size of the text
      WindowManager()->GetPrefMgr()->GetDefaultFont()->GetDimensions(tmp.GetData(), tw, th);

      // Calculate the center
      ty = (ith>>1) - (th>>1);
    } // end if text is good

    // Perform alignment of text/state
    switch (row->cols[i].txt_align)
    {
    case alignRight:
      tx = cw-tw-2;
      iws=-iw+2;
      break;

    case alignCenter:
      tx = (cw>>1) -  ((tw+iw)>>1);
      break;

    default:
      if (i==0 && depth && row->children)      tx = 2+(tbw*(depth+2));
      else if (row->children && i==0)               tx = 2+tbw;
      else if (depth && i==0)                       tx = 2+(tbw*(depth+1));
      else                                          tx = 2;

      iws = iw+2; 
      break;
    } // end switch text alignment

    if (row->cols[i].image)
    {
      // Perform alignment of image
      switch (row->cols[i].img_align)
      {
      case alignRight:
        iix = cw-iiw-2;

        // adjust spacer if needed
        if (row->cols[i].text && row->cols[i].txt_align==alignRight)
        {
          iiws=iws;
          iws-=iiw+2;
        }
        else
          iiws=-iw+2;

        break;

      case alignCenter:
        iix = (cw>>1) -  ((iiw+iw+tw)>>1);
        tx+=iiw+2;
        break;

      default:
        if (i==0 && depth && row->children)      iix = 2+(tbw*(depth+2));
        else if (row->children && i==0)               iix = 2+tbw;
        else if (depth && i==0)                       iix = 2+(tbw*(depth+1));
        else                                          iix = 2;

        // Adjust spacer
        if (row->cols[i].text && row->cols[i].txt_align==alignRight)
        {
          iiws=iws;
          iws+=iiw+2;
        }
        else
          iiws=iw+2;

        break;
      } // end switch text alignment

      // Draw image
      g3d->DrawPixmap(row->cols[i].image, 
                      x+iix+iiws, y+iiy,  
                      (cw < iiw ? cw : iiw), (ith < iih ? ith : iiw),
                      0,0, iiw, iih);

    } // end if there's an image

    if (row->cols[i].text)
    {
      // Draw the text
      g2d->Write(WindowManager()->GetPrefMgr()->GetDefaultFont(),
                 x+tx+iws,
                 y+ty,
                 WindowManager()->GetPrefMgr()->GetColor(AC_TEXTFORE),
                 -1,
                 tmp.GetData());
    } // end if text is good

    // Draw state if there is some
    if (row->cols[i].has_state)
    {
      if (row->cols[i].group_state)
      {
        if (row->cols[i].state) si=tree_grpf;
        else                    si=tree_grpe;
      }
      else
      {
        if (row->cols[i].state) si=tree_chkf;
        else                    si=tree_chke;
      }

      g3d->DrawPixmap(si, x+tx, y, iw, ih, 0,0, iw, ih);

      // Create hot spot for expand/collapse
      awsListHotspot *hs = new awsListHotspot;

      hs->obj = &(row->cols[i]);
      hs->type = hsState;
      hs->r.Set(x+tx, y, x+tx+iw, y+ih);

      hotspots.Push(hs);
    } // end if stateful

    // Next column
    x+=columns[i].width;

  } // end for i (number of cols)

  // Create hot spot for this row
  awsListHotspot *hs = new awsListHotspot;

  hs->obj = row;
  hs->type = hsRow;
  hs->r.Set(Frame().xmin+border, y, Frame().xmax-border, y+ith);

  hotspots.Push(hs);

  // Draw the highlight, if we're highlighted
  if (sel==row)
  {
    int hw, hh;

    if (highlight)
    {
      highlight->GetOriginalDimensions(hw, hh);
      g3d->DrawPixmap(highlight, Frame().xmin+border, y-1, Frame().Width()-(border*2), ith+2, 0,0,hw,hh, hi_alpha_level);
    }
  }

  // next row.
  y+=ith+(ith>>2);

  // Draw children
  if (row->children && row->expanded)
  {
    for (i=0; i<row->children->Length(); ++i)
    {
      int cx=orgx;
      awsListRow *newrow = (awsListRow *)row->children->Get(i);

      if (DrawItemsRecursively(newrow, cx, y, border, (depth ? depth+2 : depth+1), (i==row->children->Length()-1 ? true : false)))
        return true;
    }
  }



  // false means that we've not yet hit the bottom of the barrel.
  return false;
}

bool 
awsListBox::OnMouseDown(int /*button*/,int x,int y)
{
  int i;
  
  for (i=0; i<hotspots.Length(); ++i)
  {
    awsListHotspot *hs = (awsListHotspot *)hotspots[i];
    if (hs->r.Contains(x, y))
    {
      switch (hs->type)
      {
      case hsTreeBox:
        {
          awsListRow *row =(awsListRow *)hs->obj;
          if (row->expanded) row->expanded=false;
          else row->expanded=true;

          Invalidate();
          return true;
        }
        break;

      case hsState:
        {
          awsListItem *itm =(awsListItem *)hs->obj;

          if (itm->group_state)
            ClearPeers(itm);

          if (itm->state) itm->state=false;
          else itm->state=true;

          Invalidate();
          return true;
        }
        break;

      case hsRow:
        {
          awsListRow *row =(awsListRow *)hs->obj;
          sel=row;
          Invalidate();
          return true;
        }
        break;


      } // end switch type
    } // end if contains
  } // end for

  return false;
}

bool 
awsListBox::OnMouseUp(int ,int ,int )
{
  return false;
}

bool
awsListBox::OnMouseMove(int ,int ,int )
{
  return false;
}

bool
awsListBox::OnMouseClick(int ,int ,int )
{
  return false;
}

bool
awsListBox::OnMouseDoubleClick(int ,int ,int )
{
  return false;
}

bool 
awsListBox::OnMouseExit()
{
  mouse_is_over=false;
  Invalidate();

  if (is_down && !is_switch)
    is_down=false;

  return true;
}

bool
awsListBox::OnMouseEnter()
{
  mouse_is_over=true;
  Invalidate();
  return true;
}

bool
awsListBox::OnKeypress(int ,int )
{
  return false;
}

bool
awsListBox::OnLostFocus()
{
  return false;
}

bool 
awsListBox::OnGainFocus()
{
  return false;
}

void 
awsListBox::OnAdded()
{
    AddChild(scrollbar);
}


/************************************* Command Button Factory ****************/
SCF_IMPLEMENT_IBASE(awsListBoxFactory)
SCF_IMPLEMENTS_INTERFACE(iAwsComponentFactory)
SCF_IMPLEMENT_IBASE_END

awsListBoxFactory::awsListBoxFactory(iAws *wmgr):awsComponentFactory(wmgr)
{
  Register("List Box");
  RegisterConstant("lbfsBump",  awsListBox::fsBump);
  RegisterConstant("lbfsSunken", awsListBox::fsSunken);
  RegisterConstant("lbfsRaised",  awsListBox::fsRaised);
  RegisterConstant("lbfsSimple", awsListBox::fsSimple);
  RegisterConstant("lbfsNone",  awsListBox::fsNone);

  RegisterConstant("lbtTree", awsListBox::ctTree);
  RegisterConstant("lbtList",  awsListBox::ctList);

  RegisterConstant("lbAlignLeft", alignLeft);
  RegisterConstant("lbAlignRight",  alignRight);
  RegisterConstant("lbAlignCenter",  alignCenter);

  RegisterConstant("signalListBoxSelectionChanged",  awsListBox::signalSelected);
  RegisterConstant("signalListBoxScrolled",  awsListBox::signalScrolled);
}

awsListBoxFactory::~awsListBoxFactory()
{
  // empty
}

iAwsComponent *
awsListBoxFactory::Create()
{
  return new awsListBox; 
}

