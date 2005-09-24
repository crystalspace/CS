/*
    Copyright (C) 2000-2001 by Andrew Zabolotny
    Copyright (C) 2001 by Jorrit Tyberghein

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
#include "csgeom/polyclip.h"
#include "iengine/camera.h"
#include "igeom/clip2d.h"
#include "ivideo/graph3d.h"
#include "plugins/engine/3d/engine.h"
#include "plugins/engine/3d/lview.h"


csFrustumView::csFrustumView () 
  : scfImplementationType (this),
  object_func (0), things_shadow (false), ctxt (0)
{
  ctxt = new csFrustumContext ();
  ctxt->SetNewShadows (new csShadowBlockList ());
}

csFrustumView::~csFrustumView ()
{
  delete ctxt;
}

void csFrustumView::StartNewShadowBlock ()
{
  ctxt->SetNewShadows (new csShadowBlockList ());
}

void csFrustumView::CreateFrustumContext ()
{
  csFrustumContext *old_ctxt = ctxt;

  // @@@ Use a pool for frustum contexts?

  // A pool would work very well here since we have limited recusion depth.
  ctxt = new csFrustumContext ();
  *ctxt = *old_ctxt;
  ctxt->SetShadows (old_ctxt->GetShadows ());
}

void csFrustumView::SetFrustumContext (csFrustumContext *new_ctxt)
{
  ctxt = new_ctxt;
}

csFrustumContext *csFrustumView::CopyFrustumContext ()
{
  csFrustumContext *new_ctxt = new csFrustumContext ();
  *new_ctxt = *ctxt;
  new_ctxt->SetShadows (ctxt->GetShadows ());
  return new_ctxt;
}

void csFrustumView::RestoreFrustumContext (csFrustumContext *original)
{
  csFrustumContext *old_ctxt = ctxt;
  ctxt = original;

  delete old_ctxt;
}

//---------------------------------------------------------------------------

csShadowBlock::csShadowBlock (uint32 region, int max_shadows, int delta) 
  : scfImplementationType (this), next(0), prev(0), shadows(max_shadows, delta), 
  shadow_region (region), bbox_valid (false)
{
}

csShadowBlock::~csShadowBlock ()
{
  DeleteShadows ();
  SCF_DESTRUCT_IBASE ();
}

void csShadowBlock::IntAddShadow (csShadowFrustum* csf)
{
  shadows.Push (csf);
  bbox_valid = false;
}

void csShadowBlock::AddRelevantShadows (
  csShadowBlock *source,
  csTransform *trans)
{
  csShadowIterator *shadow_it = source->GetCsShadowIterator ();
  while (shadow_it->HasNext ())
  {
    csShadowFrustum *csf = (csShadowFrustum *)shadow_it->Next ();
    if (csf->IsRelevant ())
    {
      if (trans)
      {
        csRef<csShadowFrustum> copycsf;
        copycsf.AttachNew (new csShadowFrustum (*csf));
        copycsf->Transform (trans);
        IntAddShadow (copycsf);
      }
      else
      {
        IntAddShadow (csf);
      }
    }
  }

  delete shadow_it;
}

void csShadowBlock::AddRelevantShadows (
  iShadowBlock *source,
  csTransform *trans)
{
  AddRelevantShadows ((csShadowBlock *)source, trans);
}

void csShadowBlock::AddRelevantShadows (csShadowBlockList *source)
{
  csShadowIterator *shadow_it = source->GetCsShadowIterator ();
  while (shadow_it->HasNext ())
  {
    csShadowFrustum *csf = (csShadowFrustum *)shadow_it->Next ();
    if (csf->IsRelevant ())
    {
      IntAddShadow (csf);
    }
  }

  delete shadow_it;
}

void csShadowBlock::AddRelevantShadows (iShadowBlockList *source)
{
  AddRelevantShadows ((csShadowBlockList *)source);
}

void csShadowBlock::AddAllShadows (csShadowBlockList *source)
{
  csShadowIterator *shadow_it = source->GetCsShadowIterator ();
  while (shadow_it->HasNext ())
  {
    csShadowFrustum *csf = (csShadowFrustum *)shadow_it->Next ();
    IntAddShadow (csf);
  }

  delete shadow_it;
}

void csShadowBlock::AddAllShadows (iShadowBlockList *source)
{
  AddAllShadows ((csShadowBlockList *)source);
}

void csShadowBlock::AddUniqueRelevantShadows (csShadowBlockList *source)
{
  int i;
  int cnt = (int)shadows.Length ();

  csShadowIterator *shadow_it = source->GetCsShadowIterator ();
  while (shadow_it->HasNext ())
  {
    csShadowFrustum *csf = (csShadowFrustum *)shadow_it->Next ();
    if (csf->IsRelevant ())
    {
      for (i = 0; i < cnt; i++)
        if (shadows[i] == csf) break;
      if (i >= cnt)
      {
        IntAddShadow (csf);
      }
    }
  }

  delete shadow_it;
}

void csShadowBlock::AddUniqueRelevantShadows (iShadowBlockList *source)
{
  AddUniqueRelevantShadows ((csShadowBlockList *)source);
}

csFrustum *csShadowBlock::AddShadow (
  const csVector3 &origin,
  void *userData,
  int num_verts,
  csPlane3 &backplane)
{
  csRef<csShadowFrustum> sf;
  sf.AttachNew (new csShadowFrustum (origin, num_verts));
  sf->SetBackPlane (backplane);
  sf->SetUserData (userData);
  IntAddShadow (sf);
  return sf;
}

void csShadowBlock::UnlinkShadow (int idx)
{
  shadows.DeleteIndexFast (idx);
  bbox_valid = false;
}

const csBox3& csShadowBlock::GetBoundingBox ()
{
  if (!bbox_valid)
  {
    bbox.StartBoundingBox ();
    size_t i;
    int j;
    for (i = 0 ; i < shadows.Length () ; i++)
    {
      csShadowFrustum *sf = shadows[i];
      for (j = 0 ; j < sf->GetVertexCount () ; j++)
        bbox.AddBoundingVertex (sf->GetVertex (j));
    }
    bbox_valid = true;
  }
  return bbox;
}

//---------------------------------------------------------------------------
csShadowBlockList::csShadowBlockList () 
  : scfImplementationType (this),
  first(0), last(0),cur_shadow_region (0)
{  
}

csShadowBlockList::~csShadowBlockList ()
{
  DeleteAllShadows ();
}

iShadowBlock *csShadowBlockList::NewShadowBlock (
  int num_shadows)
{
  csShadowBlock *n = new csShadowBlock (cur_shadow_region, num_shadows);
  AppendShadowBlock (n);
  return (iShadowBlock *)n;
}

//---------------------------------------------------------------------------
csShadowFrustum::csShadowFrustum (const csShadowFrustum &orig) :
  csFrustum((const csFrustum &)orig)
{
  this->userData = orig.userData;
  this->relevant = orig.relevant;
}

//---------------------------------------------------------------------------
csShadowIterator::csShadowIterator (csShadowBlock *cur, bool onlycur,int dir)
  : scfImplementationType (this), cur (cur), onlycur (onlycur), dir (dir),
  first_cur (cur), use_bbox (false)
{
  Reset ();
}

csShadowIterator::csShadowIterator (const csBox3& bbox, csShadowBlock *cur,
  bool onlycur, int dir)
  : scfImplementationType (this), cur (cur), onlycur (onlycur), dir (dir),
  first_cur (cur), bbox (bbox), use_bbox (true)
{
  Reset ();
}

csShadowIterator::~csShadowIterator()
{
}

void csShadowIterator::Reset ()
{
  cur = first_cur;
  if (cur) cur_num = cur->GetShadowCount ();
  if (dir == 1)
    i = 0;
  else
    i = cur_num - 1;
}

bool csShadowIterator::HasNext ()
{
  if (cur == 0)
    return false;
  if (i >= 0 && i < cur_num)
    return true;
  if (onlycur)
  {
    cur = 0;
    return false;
  }

  if (dir == 1)
  {
    cur = cur->next;
    while (cur && cur->GetShadowCount () == 0)
      cur = cur->next;

    bool hn = cur && cur->GetShadowCount () != 0;
    if (hn)
    {
      if (cur) cur_num = cur->GetShadowCount ();
      if (dir == 1)
        i = 0;
      else
        i = cur_num - 1;
    }
    else
      cur = 0;
    return hn;
  }
  else
  {
    cur = cur->prev;
    while (cur && cur->GetShadowCount () == 0)
      cur = cur->prev;

    bool hn = cur && cur->GetShadowCount () != 0;
    if (hn)
    {
      if (cur) cur_num = cur->GetShadowCount ();
      if (dir == 1)
        i = 0;
      else
        i = cur_num - 1;
    }
    else
      cur = 0;
    return hn;
  }
}

csFrustum *csShadowIterator::Next ()
{
  if (!cur)
  {
    cur_shad = 0;
    return 0;
  }

  csShadowFrustum *s;
  if (i >= 0 && i < cur_num)
    s = (csShadowFrustum *)cur->GetShadow (i);
  else
    s = 0;
  i += dir;
  if (i < 0 || i >= cur_num)
  {
    if (onlycur)
      cur = 0;
    else if (dir == 1)
      cur = cur->next;
    else
      cur = cur->prev;
    if (cur) cur_num = cur->GetShadowCount ();
    if (dir == 1)
      i = 0;
    else
      i = cur_num - 1;
  }

  cur_shad = s;
  return s;
}

csShadowBlock *csShadowIterator::GetCsCurrentShadowBlock ()
{
  if (dir == -1)
  {
    if (i < cur_num - 1)
      return cur;
    else if (onlycur || !cur->next)
      return 0;
    else
      return cur->next;
  }
  else
  {
    if (i > 0)
      return cur;
    else if (onlycur || !cur->prev)
      return 0;
    else
      return cur->prev;
  }
}

void csShadowIterator::DeleteCurrent ()
{
  if (dir == -1)
  {
    if (i < cur_num - 1)
    {
      // Delete the previous element in the current list.
      cur->UnlinkShadow (i + 1);
      cur_num--;
    }
    else if (onlycur || !cur || !cur->next)
    {
      // We are at the very first element of the iterator. Nothing to do.
      return ;
    }
    else
    {
      // We are the first element of this list (last since we do reverse)
      // so we delete the last element (first) of the previous (next) list.
      cur->next->UnlinkShadow (0);
    }
  }
  else
  {
    if (i > 0)
    {
      // Delete the previous element in the current list.
      i--;
      cur->UnlinkShadow (i);
      cur_num--;
    }
    else if (onlycur || !cur || !cur->prev)
    {
      // We are at the very first element of the iterator. Nothing to do.
      return ;
    }
    else
    {
      // We are the first element of this list so we delete the last
      // element of the previous list.
      cur->prev->UnlinkShadow (cur->prev->GetShadowCount () - 1);
    }
  }
}

iShadowBlock *csShadowIterator::GetCurrentShadowBlock ()
{
  return (iShadowBlock *)GetCsCurrentShadowBlock ();
}

iShadowBlock *csShadowIterator::GetNextShadowBlock ()
{
  return (iShadowBlock *)GetCsNextShadowBlock ();
}
