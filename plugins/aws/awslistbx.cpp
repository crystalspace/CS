#include "cssysdef.h"
#include "awslistbx.h"
#include "awsfparm.h"
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

const int awsListBox::fsBump =0x0;
const int awsListBox::fsRaised=0x1;
const int awsListBox::fsSunken =0x2;
const int awsListBox::fsSimple =0x3;
const int awsListBox::fsNone =0x4;

const int awsListBox::ctList = 0x0;
const int awsListBox::ctTree = 0x1;

const int awsListBox::signalSelected=0x1;
const int awsListBox::signalScrolled=0x2;

const int alignLeft=0;
const int alignCenter=1;
const int alignRight=2;

//////////////////////////////////////////////////////////////////////////
// awsListRow implementation 
//

int 
awsListRow::GetHeight(iAwsPrefManager *pm, int colcount)
{
  int minheight=0;
  int i;

  for(i=0; i<colcount; ++i)
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
                         control_type(0), ncolumns(1)
                         
{
}

awsListBox::~awsListBox()
{
}

char *
awsListBox::Type() 
{ return "List Box"; }

bool
awsListBox::Setup(iAws *_wmgr, awsComponentNode *settings)
{ 
  iString   *tn1=NULL, *tn2=NULL;
  char buf[64];
  int i;
 
  if (!awsComponent::Setup(_wmgr, settings)) return false;

  iAwsPrefManager *pm=WindowManager()->GetPrefMgr();
 
  pm->LookupIntKey("OverlayTextureAlpha", alpha_level); // global get
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
 
  if (tn1) bkg=pm->GetTexture(tn1->GetData(), tn1->GetData());
  else bkg=pm->GetTexture("Texture");

  if (tn2) highlight=pm->GetTexture(tn2->GetData(), tn2->GetData());

  // Make sure we have at least one column
  ncolumns=(ncolumns<1 ? 1 : ncolumns);

  // Create new column region.
  columns = new awsListColumn[ncolumns];
  memset(columns, 0, sizeof(awsListColumn) * ncolumns);
 
  // Get user prefs for the column headers
  for(i=0; i<ncolumns; ++i)
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

bool 
awsListBox::Execute(char *action, iAwsParmList &parmlist)
{
  if (awsComponent::Execute(action, parmlist)) return true;

  if (strcmp(action, "InsertItem")==0)
  {
    char buf[50];
    int i;
    iString *str;
       
    // Create a new row and zero it out.
    awsListRow *row = new awsListRow;
    memset(row, 0 , sizeof(awsListRow));
    
    // Create a new set of columns and zero them out.
    row->cols = new awsListItem[ncolumns];
    memset(row->cols, 0, sizeof(awsListItem) * ncolumns);
    
    parmlist.GetInt("parent", (int *)&(row->parent));

    /* Fill in the columns by looking for several parameters:
     *   textX, imageX, txtalignX, imgalignX, statefulX, stateX, groupstateX, 
     *   selectableX
     */
    for(i=0; i<ncolumns; ++i)
    {
      cs_snprintf(buf, 50, "text%d", i);
      if (parmlist.GetString(buf, &(row->cols[i].text)))
        row->cols[i].text->IncRef();

      cs_snprintf(buf, 50, "image%d", i);
      if (parmlist.GetString(buf, &str))
        row->cols[i].image = WindowManager()->GetPrefMgr()->GetTexture(str->GetData(), str->GetData());
      
      cs_snprintf(buf, 50, "stateful%d", i);
      parmlist.GetBool(buf, &(row->cols[i].has_state));

      cs_snprintf(buf, 50, "state%d", i);
      parmlist.GetBool(buf, &(row->cols[i].state));

      cs_snprintf(buf, 50, "groupstate%d", i);
      parmlist.GetBool(buf, &(row->cols[i].group_state));

      cs_snprintf(buf, 50, "selectable%d", i);
      parmlist.GetBool(buf, &(row->cols[i].selectable));

      cs_snprintf(buf, 50, "aligntxt%d", i);
      parmlist.GetInt(buf, &(row->cols[i].txt_align));

      cs_snprintf(buf, 50, "alignimg%d", i);
      parmlist.GetInt(buf, &(row->cols[i].txt_align));
    }

    // Add the item
    if (row->parent) 
    {
      if (row->parent->children==NULL)
        row->parent->children = new awsListRowVector();

      row->parent->children->Push(row);
    }
    else rows.Push(row);

    // Pass back the id of this row, in case they want it.
    parmlist.AddInt("id", (int)row);
  }
  return false;
}

void 
awsListBox::ClearGroup()
{
 csEvent Event;

 Event.Type = csevGroupOff;
 
 int i;
 for(i=0; i<Parent()->GetChildCount(); ++i)
 {
   iAwsComponent *cmp = Parent()->GetChildAt(i);

   if (cmp && cmp!=this)
     cmp->HandleEvent(Event);
 }
}

bool 
awsListBox::HandleEvent(iEvent& Event)
{
  if (awsComponent::HandleEvent(Event)) return true;

  switch(Event.Type)
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
  iGraphics3D *g3d = WindowManager()->G3D();

  int hi    = WindowManager()->GetPrefMgr()->GetColor(AC_HIGHLIGHT);
  int hi2   = WindowManager()->GetPrefMgr()->GetColor(AC_HIGHLIGHT2);
  int lo    = WindowManager()->GetPrefMgr()->GetColor(AC_SHADOW);
  int lo2   = WindowManager()->GetPrefMgr()->GetColor(AC_SHADOW2);
  int  fill = WindowManager()->GetPrefMgr()->GetColor(AC_FILL);
  int dfill = WindowManager()->GetPrefMgr()->GetColor(AC_DARKFILL);
  int black = WindowManager()->GetPrefMgr()->GetColor(AC_BLACK);

  int i,j;
  int border=3;
    
  
  switch(frame_style)
  {
  case fsBump:
      g2d->DrawLine(Frame().xmin+0, Frame().ymin+0, Frame().xmax-1, Frame().ymin+0, hi);
      g2d->DrawLine(Frame().xmin+0, Frame().ymin+0, Frame().xmin+0, Frame().ymax-1, hi);
      g2d->DrawLine(Frame().xmin+0, Frame().ymax-1, Frame().xmax-1, Frame().ymax-1, lo);
      g2d->DrawLine(Frame().xmax-1, Frame().ymin+0, Frame().xmax-1, Frame().ymax-1, lo);
      g2d->DrawLine(Frame().xmin+1, Frame().ymax-0, Frame().xmax-0, Frame().ymax-0, black);
      g2d->DrawLine(Frame().xmax-0, Frame().ymin+1, Frame().xmax-0, Frame().ymax-0, black);

      g2d->DrawLine(Frame().xmin+1, Frame().ymin+1, Frame().xmax-2, Frame().ymin+1, hi2);
      g2d->DrawLine(Frame().xmin+1, Frame().ymin+1, Frame().xmin+1, Frame().ymax-2, hi2);
      g2d->DrawLine(Frame().xmin+1, Frame().ymax-2, Frame().xmax-2, Frame().ymax-2, lo2);
      g2d->DrawLine(Frame().xmax-2, Frame().ymin+1, Frame().xmax-2, Frame().ymax-2, lo2);

      g2d->DrawLine(Frame().xmin+2, Frame().ymin+2, Frame().xmax-3, Frame().ymin+2, lo2);
      g2d->DrawLine(Frame().xmin+2, Frame().ymin+2, Frame().xmin+2, Frame().ymax-3, lo2);
      g2d->DrawLine(Frame().xmin+2, Frame().ymax-3, Frame().xmax-3, Frame().ymax-3, hi2);
      g2d->DrawLine(Frame().xmax-3, Frame().ymin+2, Frame().xmax-3, Frame().ymax-3, hi2);

      g2d->DrawLine(Frame().xmin+3, Frame().ymin+3, Frame().xmax-4, Frame().ymin+3, black);
      g2d->DrawLine(Frame().xmin+3, Frame().ymin+3, Frame().xmin+3, Frame().ymax-4, black);

      if (bkg)
       g3d->DrawPixmap(bkg, Frame().xmin+4, Frame().ymin+4, Frame().Width()-7, Frame().Height()-7, 0, 0, Frame().Width(), Frame().Height(), 0);
      else
       g2d->DrawBox(Frame().xmin+4, Frame().ymin+4, Frame().Width()-7, Frame().Height()-7, fill);

      border=5;

      break;

  case fsSunken:
      g2d->DrawLine(Frame().xmin+0, Frame().ymin+0, Frame().xmax-1, Frame().ymin+0, lo2);
      g2d->DrawLine(Frame().xmin+0, Frame().ymin+0, Frame().xmin+0, Frame().ymax-1, lo2);
      g2d->DrawLine(Frame().xmin+1, Frame().ymin+1, Frame().xmax-0, Frame().ymin+1, lo);
      g2d->DrawLine(Frame().xmin+1, Frame().ymin+1, Frame().xmin+1, Frame().ymax-0, lo);
      g2d->DrawLine(Frame().xmin+1, Frame().ymax-0, Frame().xmax-0, Frame().ymax-0, hi);
      g2d->DrawLine(Frame().xmax-0, Frame().ymin+1, Frame().xmax-0, Frame().ymax-0, hi);

      g2d->DrawLine(Frame().xmin+2, Frame().ymin+2, Frame().xmax-1, Frame().ymin+2, black);
      g2d->DrawLine(Frame().xmin+2, Frame().ymin+2, Frame().xmin+2, Frame().ymax-1, black);
      g2d->DrawLine(Frame().xmin+2, Frame().ymax-1, Frame().xmax-1, Frame().ymax-1, hi2);
      g2d->DrawLine(Frame().xmax-1, Frame().ymin+2, Frame().xmax-1, Frame().ymax-1, hi2);

      g2d->DrawBox(Frame().xmin+3, Frame().ymin+3, Frame().Width()-3, Frame().Height()-3, dfill);

      if (bkg)
       g3d->DrawPixmap(bkg, Frame().xmin, Frame().ymin, Frame().Width()+1, Frame().Height()+1, 0, 0, Frame().Width()+1, Frame().Height()+1, alpha_level);

      break;
      
  case fsRaised:
      g2d->DrawLine(Frame().xmin+0, Frame().ymin+0, Frame().xmax-1, Frame().ymin+0, hi);
      g2d->DrawLine(Frame().xmin+0, Frame().ymin+0, Frame().xmin+0, Frame().ymax-1, hi);
      g2d->DrawLine(Frame().xmin+0, Frame().ymax-1, Frame().xmax-1, Frame().ymax-1, lo);
      g2d->DrawLine(Frame().xmax-1, Frame().ymin+0, Frame().xmax-1, Frame().ymax-1, lo);
      g2d->DrawLine(Frame().xmin+1, Frame().ymax-0, Frame().xmax-0, Frame().ymax-0, black);
      g2d->DrawLine(Frame().xmax-0, Frame().ymin+1, Frame().xmax-0, Frame().ymax-0, black);

      g2d->DrawLine(Frame().xmin+1, Frame().ymin+1, Frame().xmax-2, Frame().ymin+1, hi2);
      g2d->DrawLine(Frame().xmin+1, Frame().ymin+1, Frame().xmin+1, Frame().ymax-2, hi2);
      g2d->DrawLine(Frame().xmin+1, Frame().ymax-2, Frame().xmax-2, Frame().ymax-2, lo2);
      g2d->DrawLine(Frame().xmax-2, Frame().ymin+1, Frame().xmax-2, Frame().ymax-2, lo2);

      g2d->DrawBox(Frame().xmin+2, Frame().ymin+2, Frame().Width()-3, Frame().Height()-3, fill);

      if (bkg)
       g3d->DrawPixmap(bkg, Frame().xmin, Frame().ymin, Frame().Width()+1, Frame().Height()+1, 0, 0, Frame().Width()+1, Frame().Height()+1, alpha_level);

      break;

    case fsSimple:
      g2d->DrawLine(Frame().xmin, Frame().ymin, Frame().xmax, Frame().ymin, black);
      g2d->DrawLine(Frame().xmin, Frame().ymin, Frame().xmin, Frame().ymax, black);
      g2d->DrawLine(Frame().xmin, Frame().ymax, Frame().xmax, Frame().ymax, black);
      g2d->DrawLine(Frame().xmax, Frame().ymin, Frame().xmax, Frame().ymax, black);

      border=1;
      break;

    case fsNone:
      break;
  }

  int starty=Frame().ymin+border;
  int startx=Frame().xmin+border;

  int x=startx, y=starty;
  int hch=15;
  
  for(i=0; i<ncolumns; ++i)
  {
    if (columns[i].caption)
    {
     int tw, th, tx, ty, mcc;
     int hcw;

     if (i==ncolumns-1)
       hcw = Frame().xmax-x-border;
     else
       hcw = columns[i].width;
         
     mcc = WindowManager()->GetPrefMgr()->GetDefaultFont()->GetLength(columns[i].caption->GetData(), hcw-5);

     scfString tmp(columns[i].caption->GetData());
     tmp.Truncate(mcc);

     // Get the size of the text
     WindowManager()->GetPrefMgr()->GetDefaultFont()->GetDimensions(tmp.GetData(), tw, th);

     // Calculate the center
     ty = (hch>>1) - (th>>1);

     switch(columns[i].align)
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
  for(j=0; j<rows.Length(); ++j)
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
    g3d->DrawPixmap(tree_expanded, x+2+(tbw*(depth+1)), y, tbw, tbh, 0,0, tbw, tbh);
    g3d->DrawPixmap(tree_hline, x+2+(tbw*depth), y, tbw, tbh, 0,0, tbw, tbh);

    if (last_child)
      g3d->DrawPixmap(tree_vline, x+2+(tbw*depth), y, tbw, ith>>1, 0,0, tbw, tbh);    
    else
      g3d->DrawPixmap(tree_vline, x+2+(tbw*depth), y, tbw, ith+2, 0,0, tbw, tbh);    
    
  }
  else if (row->children)
  {
    // Draw tree box if needed
    tree_expanded->GetOriginalDimensions(tbw, tbh);
    g3d->DrawPixmap(tree_expanded, x+2, y, tbw, tbh, 0,0, tbw, tbh);
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
  for(i=0; i<ncolumns; ++i)
    {
      int tw=0, th=0, tx=0, ty=0, mcc;
      int cw;

      int iw=0, ih=0; // stateful image width and height
      int iws=0;      // stateful image spacer

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
      switch(row->cols[i].txt_align)
      {
        case alignRight:
          tx = cw-tw-2;
          iws=-iw+2;
          break;

        case alignCenter:
          tx = (cw>>1) -  ((tw+iw)>>1);
          break;

        default:
          if      (i==0 && depth && row->children)      tx = 2+(tbw*(depth+2));
          else if (row->children && i==0)               tx = 2+tbw;
          else if (depth && i==0)                       tx = 2+(tbw*(depth+1));
          else                                          tx = 2;

          iws = iw+2; 
          break;
      } // end switch text alignment

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
      } // end if stateful

      // Next column
      x+=columns[i].width;

    } // end for i (number of cols)

    // next row.
    y+=ith+2;

    // Draw children
    if (row->children)
    {
      for(i=0; i<row->children->Length(); ++i)
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
awsListBox::OnMouseDown(int button, int x, int y)
{
  was_down=is_down;

  if (!is_switch || is_down==false)
    is_down=true;
  
  Invalidate();
  return true;
}
    
bool 
awsListBox::OnMouseUp(int button, int x, int y)
{
  if (!is_switch)
  {
    if (is_down)
      Broadcast(signalSelected);

    is_down=false;
  }
  else
  {
    if (was_down)
      is_down=false;
    else
      ClearGroup();

    Broadcast(signalSelected);
  }

  Invalidate();
  return true;
}
    
bool
awsListBox::OnMouseMove(int button, int x, int y)
{
  return false;
}

bool
awsListBox::OnMouseClick(int button, int x, int y)
{
  return false;
}

bool
awsListBox::OnMouseDoubleClick(int button, int x, int y)
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
awsListBox::OnKeypress(int key, int modifiers)
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

