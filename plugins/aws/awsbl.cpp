#include "cssysdef.h"
#include "awsbl.h"
#include "iaws/aws.h"
#include "iaws/awsdefs.h"

awsBorderLayout::awsBorderLayout (iAwsComponent *owner, awsComponentNode* settings,
				  iAwsPrefManager *pm):

	    awsLayoutManager(owner, settings, pm), 
	    hGap(0), vGap(0)

{ 
  memset(components, 0, sizeof(components)); 

  // Get gap settings.
  pm->GetInt(settings, "VerticalGap", vGap);
  pm->GetInt(settings, "HorizontalGap", hGap);
};


csRect awsBorderLayout::AddComponent (iAwsComponent *cmp, awsComponentNode* settings)
{
  int d;

  pm->GetInt(settings, "Anchor", d);

  switch(d)
  {
  case GBS_NORTH: 
    components[GBS_NORTH]=cmp;
    break;
  case GBS_SOUTH: 
    components[GBS_SOUTH]=cmp;
    break;
  case GBS_EAST: 
    components[GBS_EAST]=cmp;
    break;
  case GBS_WEST: 
    components[GBS_WEST]=cmp;
    break;
  case GBS_CENTER: 
    components[GBS_CENTER]=cmp;
    break;
  }

  return csRect(0,0,0,0);
}


void awsBorderLayout::LayoutComponents ()
{
  csRect r(owner->Frame());
  csRect i = owner->getInsets();
  iAwsComponent *cmp;

  bool has_north=false, has_south=false, has_east=false, has_west=false;

  if (components[GBS_NORTH])
  {
    has_north=true;
    cmp=components[GBS_NORTH];

    csRect cr = cmp->getPreferredSize();
	csRect newFrame;

    newFrame.xmin=r.xmin+i.xmin;
    newFrame.ymin=r.ymin+i.ymin;
    newFrame.xmax=r.xmax-i.xmax;
    newFrame.ymax=cmp->Frame().ymin+cr.ymax;
	cmp->ResizeTo(newFrame);

    if (cmp->Frame().Width()<=0 || cmp->Frame().Height()<=0)
      cmp->SetFlag(AWSF_CMP_INVISIBLE);
    else
      cmp->ClearFlag(AWSF_CMP_INVISIBLE);
  }

  if (components[GBS_SOUTH])
  {
    has_south=true;
    cmp=components[GBS_SOUTH];

    csRect cr = cmp->getPreferredSize();
	csRect newFrame;

    newFrame.xmin=r.xmin+i.xmin;
    newFrame.ymin=r.ymax-i.ymax-cr.ymax;
    newFrame.xmax=r.xmax-i.xmax;
    newFrame.ymax=r.ymax-i.ymax;
	cmp->ResizeTo(newFrame);

    if (cmp->Frame().Width()<=0 || cmp->Frame().Height()<=0)
      cmp->SetFlag(AWSF_CMP_INVISIBLE);
    else
      cmp->ClearFlag(AWSF_CMP_INVISIBLE);
  }

  if (components[GBS_EAST])
  {
    has_east=true;
    cmp=components[GBS_EAST];

    csRect cr = cmp->getPreferredSize();
    csRect newFrame;

    newFrame.xmin=r.xmin+i.xmin;
    newFrame.ymin=r.ymin+i.ymin+(has_north ? components[GBS_NORTH]->Frame().Height() + vGap : 0);
    newFrame.xmax=r.xmin+i.xmin+cr.xmax;
    newFrame.ymax=r.ymax-i.ymax-(has_south ? components[GBS_SOUTH]->Frame().Height() + vGap : 0);
	cmp->ResizeTo(newFrame);

    if (cmp->Frame().Width()<=0 || cmp->Frame().Height()<=0)
      cmp->SetFlag(AWSF_CMP_INVISIBLE);
    else
      cmp->ClearFlag(AWSF_CMP_INVISIBLE);
  }

  if (components[GBS_WEST])
  {
    has_west=true;
    cmp=components[GBS_WEST];

    csRect cr = cmp->getPreferredSize();
	csRect newFrame;

    newFrame.xmin=r.xmax-i.xmax-cr.xmax;
    newFrame.ymin=r.ymin+i.ymin+(has_north ? components[GBS_NORTH]->Frame().Height() + vGap : 0);
    newFrame.xmax=r.xmax-i.xmax;
    newFrame.ymax=r.ymax-i.ymax-(has_south ? components[GBS_SOUTH]->Frame().Height() + vGap: 0);
	cmp->ResizeTo(newFrame);


    if (cmp->Frame().Width()<=0 || cmp->Frame().Height()<=0)
      cmp->SetFlag(AWSF_CMP_INVISIBLE);
    else
      cmp->ClearFlag(AWSF_CMP_INVISIBLE);
  }

  if (components[GBS_CENTER])
  {    
    cmp=components[GBS_CENTER];

    csRect cr = cmp->getPreferredSize();
    csRect newFrame;

    newFrame.xmin=r.xmin+i.xmin + (has_east ? components[GBS_EAST]->Frame().Width() + hGap : 0);
    newFrame.ymin=r.ymin+i.ymin + (has_north ? components[GBS_NORTH]->Frame().Height() + vGap : 0);
    newFrame.xmax=r.xmax-i.xmax - (has_west ? components[GBS_WEST]->Frame().Width() + hGap : 0);
    newFrame.ymax=r.ymax-i.ymax - (has_south ? components[GBS_SOUTH]->Frame().Height() + vGap : 0);
	cmp->ResizeTo(newFrame);

    if (cmp->Frame().Width()<=0 || cmp->Frame().Height()<=0)
      cmp->SetFlag(AWSF_CMP_INVISIBLE);
    else
      cmp->ClearFlag(AWSF_CMP_INVISIBLE);
  }
}
