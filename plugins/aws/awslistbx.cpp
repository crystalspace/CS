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
    cs_snprintf(buf, 64, "Column%dImg:", i);
    pm->GetString(settings, buf, tn1);
    cs_snprintf(buf, 64, "Column%dBkg:", i);
    pm->GetString(settings, buf, tn2);
    cs_snprintf(buf, 64, "Column%dCaption:", i);
    pm->GetString(settings, buf, columns[i].caption);
    cs_snprintf(buf, 64, "Column%dWidth:", i);
    pm->GetInt(settings, buf, columns[i].width);

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

  if (strcmp(action, "InsertItem"))
  {
    char buf[50];
    int i;
    iString *str;
       
    // Create a new row and zero it out.
    awsListRow *row = new awsListRow;
    memset(row, 0 , sizeof(awsListRow));
    
    // Create a new set of columns and zero them out.
    row->cols = new awsListItem[ncolumns];
    memset(row, 0, sizeof(awsListItem) * ncolumns);
    
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

  int i;
    
  
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
      break;

    case fsNone:
      break;
  }


  // Now begin to draw actual list
  for(i=0; i<rows.Length(); ++i)
  {

  }
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

