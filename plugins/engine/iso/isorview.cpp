/*
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
#include "isorview.h"
#include "ivideo/graph3d.h"

SCF_IMPLEMENT_IBASE (csIsoRenderView)
  SCF_IMPLEMENTS_INTERFACE (iIsoRenderView)
SCF_IMPLEMENT_IBASE_END

csIsoRenderView::csIsoRenderView (iBase *iParent)
{
  SCF_CONSTRUCT_IBASE (iParent);
  view = 0;
  g3d = 0;
  renderpass = 0;
  clipper = 0;
  buckets = 0;
  maxbuckets = 0;
  prebuck = 0;
}

csIsoRenderView::~csIsoRenderView ()
{
  delete[] buckets;
  SCF_DESTRUCT_IBASE();
}

void csIsoRenderView::CreateBuckets(int num)
{
  delete[] buckets;
  maxbuckets = num;
  buckets = new csIsoRenderBucket* [num];
  memset(buckets, 0, sizeof(csIsoRenderBucket*)*num);
}

void csIsoRenderView::DrawBuckets()
{
  extern void IsoDrawPolygonFX (iGraphics3D* g3d, G3DPolygonDPFX& poly,
  	csZBufMode zbufmode);
  csIsoRenderBucket *p, *np;
  int i;
  for(i=0; i<maxbuckets; i++)
  {
    p = buckets[i];
    if(!p) continue;
    //int num = 0;
    while(p)
    {
      IsoDrawPolygonFX (g3d, *p->g3dpolyfx, CS_ZBUF_FILL);
      /// move to prealloc list
      np = p->next;
      p->next = prebuck;
      prebuck = p;
#ifdef CS_DEBUG
      prebuck->g3dpolyfx = 0;
#endif
      p = np;
      //num++;
    }
    //printf("Drawn index %d for %d times\n", i, num);
    buckets[i] = 0;
  }
}

void csIsoRenderView::AddPolyFX(int materialindex, G3DPolygonDPFX *g3dpolyfx,
  uint mixmode)
{
  CS_ASSERT (materialindex >= 0);
  if(materialindex>=maxbuckets)
  {
    g3dpolyfx->mixmode = mixmode;
    extern void IsoDrawPolygonFX (iGraphics3D* g3d, G3DPolygonDPFX& poly,
    	csZBufMode zbufmode);
    IsoDrawPolygonFX (g3d, *g3dpolyfx, CS_ZBUF_FILL);
    return;
  }
  csIsoRenderBucket *bucket = 0;
  if(prebuck)
  {
    // use preallocated buckets
    bucket = prebuck;
    prebuck = bucket->next;
  }
  else
  {
    /// link in to draw reversed later
    bucket = new csIsoRenderBucket;
  }
  bucket->g3dpolyfx = g3dpolyfx;
  bucket->g3dpolyfx->mixmode = mixmode;
  bucket->next = buckets[materialindex];
  buckets[materialindex] = bucket;
}

