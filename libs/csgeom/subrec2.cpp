/*
    Copyright (C) 2003-2005 by Frank Richter

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
#include "csgeom/math.h"
#include "csgeom/subrec.h"
#include "csutil/blockallocator.h"

#if defined(CS_DEBUG)
#define DUMP_TO_IMAGES
#endif

#ifdef DUMP_TO_IMAGES
// SubRectangles::Dump () writes an image, stuff needed for that
#include "csqint.h"
#include "csutil/csstring.h"
#include "csutil/ref.h"
#include "csutil/array.h"
#include "csgfx/rgbpixel.h"
#include "csgfx/imagememory.h"
#include "iutil/objreg.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"
#include "igraphic/imageio.h"
#endif

namespace CS
{

/*
  How it works:
    It's based on a BSP. However, a partition isn't split when 
    a rectangle is allocated, but upon the next allocation. That
    means that the space is used more efficiently.

    Consider:
    +-----+-----+ 
    |     |     | 
    |  1  |     | 
    |     |     | 
    +-----+     + 
    |           | 
    |           | 
    +-----------+ 

    Now, if the rectangle would be split from top to bottom when 
    [1] is allocated, a request for an allocation of a rectangle
    with the size of the lower part would fail, although there would
    be space for it. However, when the  split is delayed until that 
    rect is actually requested, the allocation request can be satisfied.
 */

// --------------------------------------------------------------------------

SubRectangles::SubRect::SubRect ()
{
  splitType = SPLIT_UNSPLIT;
  splitPos = 0;
  children[0] = 0;
  children[1] = 0;
  parent = 0;
}

SubRectangles::SubRect& SubRectangles::SubRect::operator= (
  const SubRectangles::SubRect& other)
{
  if (splitType == SPLIT_UNSPLIT) superrect->RemoveLeaf (this);
  rect = other.rect;
  allocedRect = other.allocedRect;
  splitType = other.splitType;
  splitPos = other.splitPos;
  if (splitType == SPLIT_UNSPLIT) superrect->AddLeaf (this);

  if (children[0] != 0)
  {
    superrect->FreeSubrect (children[0]);
    children[0] = 0;
  }
  if (other.children[0] != 0)
  {
    children[0] = superrect->AllocSubrect ();
    children[0]->parent = this;
    children[0]->superrect = superrect;
    *(children[0]) = *(other.children[0]);
  }
  if (children[1] != 0)
  {
    superrect->FreeSubrect (children[1]);
    children[1] = 0;
  }
  if (other.children[1] != 0)
  {
    children[1] = superrect->AllocSubrect ();
    children[1]->parent = this;
    children[1]->superrect = superrect;
    *(children[1]) = *(other.children[1]);
  }
  return *this;
}

void SubRectangles::SubRect::TestAlloc (int w, int h, AllocInfo& ai)
{
  int rW = rect.Width ();
  if (w > rW) return;
  int rH = rect.Height ();
  if (h > rH) return;

  CS_ASSERT (splitType == SPLIT_UNSPLIT);
  // leaf is not split yet.
  int d = 0x7fffffff;

  if (allocedRect.IsEmpty ())
  {
    // empty leaf.
    int dw = rW - w;
    int dh = rH - h;

    if (dw < dh)
    {
      d = dw;
    }
    else
    {
      d = dh;
    }
    if (d < ai.d)
    {
      ai.d = d;
      ai.allocPos = ALLOC_NEW;
      ai.node = this;
      ai.res = true;
    }
  }
  else
  {
    // a part of the leaf is already allocated.
    int arW = allocedRect.Width ();
    int arH = allocedRect.Height ();

    int dw = rW - arW;
    int dh = rH - arH;

    // Test whether a good position is below the 
    // already allocated rect.
    if (dh >= h)
    {
      if (((d = (arW - w)) >= 0))
      {
	// below
	if (d < ai.d)
	{
	  ai.d = d;
	  ai.allocPos = ALLOC_BELOW;
	  ai.node = this;
	  ai.res = true;
	}
      }
      else
      {
	d = (dh - h);
	// below
	if (d < ai.d)
	{
	  ai.d = d;
	  ai.allocPos = ALLOC_BELOW;
	  ai.node = this;
	  ai.res = true;
	}
      }
    }
    // Test whether a good position is to right of the 
    // already allocated rect.
    if ((d != 0) && (dw >= w))
    {
      if (((d = (arH - h)) >= 0))
      {
	// right
	if (d < ai.d)
	{
	  ai.d = d;
	  ai.allocPos = ALLOC_RIGHT;
	  ai.node = this;
	  ai.res = true;
	}
      }
      else
      {
	d = (dw - w);
	// right
	if (d < ai.d)
	{
	  ai.d = d;
	  ai.allocPos = ALLOC_RIGHT;
	  ai.node = this;
	  ai.res = true;
	}
      }
    }
  }
}

void SubRectangles::SubRect::DecideBestSplit (const csRect& rect, int splitX, int splitY,
				  SubRectangles::SubRect::SplitType& splitType)
{
  int rW = rect.Width ();
  int rH = rect.Height ();
  int vsSize = rW - (splitX - rect.xmin);
  int hsSize = rH - (splitY - rect.ymin);

  if ((vsSize <= 0) || (hsSize <= 0))
  {
    splitType = (vsSize <= 0) ? SPLIT_H : SPLIT_V;
    return;
  }

  if (vsSize != hsSize)
  {
    if (vsSize > hsSize)
    {
      splitType = SPLIT_V;
    }
    else
    {
      splitType = SPLIT_H;
    }
  }
}

SubRectangles::SubRect* SubRectangles::SubRect::Alloc (int w, int h, const AllocInfo& ai, csRect& r)
{
  CS_ASSERT (splitType == SPLIT_UNSPLIT);

  switch (ai.allocPos)
  {
    case ALLOC_BELOW:
      {
	// below
	r.Set (allocedRect.xmin, allocedRect.ymax, allocedRect.xmin + w, 
	  allocedRect.ymax + h);
	splitType = SPLIT_H;
      }
      break;
    case ALLOC_RIGHT:
      {
	// right
	r.Set (allocedRect.xmax, allocedRect.ymin, allocedRect.xmax + w, 
	  allocedRect.ymin + h);

	splitType = SPLIT_V;
      }
      break;
    case ALLOC_NEW:
      {
	allocedRect.Set (rect.xmin, rect.ymin, rect.xmin + w, rect.ymin + h);
	r = allocedRect;
	return this;
      }
      break;
    default:
      break;
  }
  if (splitType != SPLIT_UNSPLIT)
  {
    superrect->RemoveLeaf (this);
    SubRectangles::SubRect* ret = 0;

    int splitX, splitY;
    splitX = allocedRect.xmax;
    splitY = allocedRect.ymax;

    if ((ai.allocPos == ALLOC_RIGHT && r.Height() <= allocedRect.Height()) ||
	(ai.allocPos == ALLOC_BELOW && r.Width () <= allocedRect.Width ()))
    {
      DecideBestSplit (rect, splitX, splitY, splitType);
    }

    // Now, split that leaf.
    // Obviously, care has to be taken that a split doesn't go through an
    // allocated rect. That would cause overlapping -> bad.
    if (splitType == SPLIT_V)
    {
      splitPos = splitX - rect.xmin;

      children[0] = superrect->AllocSubrect ();
      children[0]->parent = this;
      children[0]->superrect = superrect;
      children[0]->rect.Set (rect.xmin, rect.ymin, 
	splitX, rect.ymax);
      if (ai.allocPos == ALLOC_RIGHT)
      {
	children[0]->allocedRect = allocedRect;
	superrect->AddLeaf (children[0]);
      }
      else
      {
	SubRectangles::SubRect* subChild0 = superrect->AllocSubrect ();
	subChild0->parent = children[0];
	subChild0->superrect = superrect;
	subChild0->rect.Set (rect.xmin, rect.ymin, splitX, allocedRect.ymax);
	subChild0->allocedRect = allocedRect;
	superrect->AddLeaf (subChild0);

	SubRectangles::SubRect* subChild1 = superrect->AllocSubrect ();
	subChild1->parent = children[0];
	subChild1->superrect = superrect;
	subChild1->rect.Set (rect.xmin, allocedRect.ymax, splitX, rect.ymax);
	subChild1->allocedRect = r;
	ret = subChild1;
	superrect->AddLeaf (subChild1);

	children[0]->splitType = SPLIT_H;
	children[0]->splitPos = allocedRect.Height ();
	children[0]->allocedRect = allocedRect;
	children[0]->children[0] = subChild0;
	children[0]->children[1] = subChild1;
      }

      children[1] = superrect->AllocSubrect ();
      children[1]->parent = this;
      children[1]->superrect = superrect;
      children[1]->rect.Set (splitX, rect.ymin, 
	rect.xmax, rect.ymax);
      if (ai.allocPos == ALLOC_RIGHT)
      {
	children[1]->allocedRect = r;
	ret = children[1];
      }
      superrect->AddLeaf (children[1]);
    }
    else
    {
      splitPos = splitY - rect.ymin;

      children[0] = superrect->AllocSubrect ();
      children[0]->parent = this;
      children[0]->superrect = superrect;
      children[0]->rect.Set (rect.xmin, rect.ymin, 
	rect.xmax, splitY);
      if (ai.allocPos == ALLOC_BELOW)
      {
	children[0]->allocedRect = allocedRect;
	superrect->AddLeaf (children[0]);
      }
      else
      {
	SubRectangles::SubRect* subChild0 = superrect->AllocSubrect ();
	subChild0->parent = children[0];
	subChild0->superrect = superrect;
	subChild0->rect.Set (rect.xmin, rect.ymin, allocedRect.xmax, splitY);
	subChild0->allocedRect = allocedRect;
	superrect->AddLeaf (subChild0);

	SubRectangles::SubRect* subChild1 = superrect->AllocSubrect ();
	subChild1->parent = children[0];
	subChild1->superrect = superrect;
	subChild1->rect.Set (allocedRect.xmax, rect.ymin, rect.xmax, splitY);
	subChild1->allocedRect = r;
	ret = subChild1;
	superrect->AddLeaf (subChild1);

	children[0]->splitType = SPLIT_V;
	children[0]->splitPos = allocedRect.Width ();
	children[0]->allocedRect = allocedRect;
	children[0]->children[0] = subChild0;
	children[0]->children[1] = subChild1;
      }

      children[1] = superrect->AllocSubrect ();
      children[1]->parent = this;
      children[1]->superrect = superrect;
      children[1]->rect.Set (rect.xmin, splitY, 
	rect.xmax, rect.ymax);
      if (ai.allocPos == ALLOC_BELOW)
      {
	children[1]->allocedRect = r;
	ret = children[1];
      }
      superrect->AddLeaf (children[1]);
    }
    CS_ASSERT (ret != 0);
    return ret;
  }
  return 0;
}

void SubRectangles::SubRect::Reclaim ()
{
  // @@@ This could be improved.
  if (splitType == SPLIT_UNSPLIT)
  {
    allocedRect.Set (0, 0, 0, 0);
    if (parent != 0) parent->TestCollapse ();
  }
  else
  {
    CS_ASSERT (children[0]);
    children[0]->Reclaim ();
  }
}

void SubRectangles::SubRect::TestCollapse ()
{
  // If both children are "empty space" we can revert the status
  // of this sub-rectangle to "unsplit" and free the children.
  if (((children[0] != 0) && (children[0]->allocedRect.IsEmpty ())) && 
    ((children[1] != 0) && (children[1]->allocedRect.IsEmpty ())))
  {
    splitType = SPLIT_UNSPLIT;
    allocedRect.Set (0, 0, 0, 0);
    superrect->RemoveLeaf (children[0]);
    superrect->RemoveLeaf (children[1]);
    superrect->FreeSubrect (children[0]); children[0] = 0;
    superrect->FreeSubrect (children[1]); children[1] = 0;
    superrect->AddLeaf (this);
    if (parent != 0) parent->TestCollapse ();
  }
}

// --------------------------------------------------------------------------

SubRectangles::SubRectangles (const csRect &region) : 
  region(region), root (0), alloc (4096 / sizeof(SubRect))
{
  Clear ();
}

SubRectangles::SubRectangles (const SubRectangles& other) : 
  region (other.region), root (0), alloc (4096 / sizeof(SubRect))
{
  Clear();
  *root = *(other.root);
}

SubRectangles::~SubRectangles ()
{
  // No need to free the rects, the block allocator will take care
}

void SubRectangles::FreeSubrect (SubRect* sr)
{
  if (sr == 0) return;
  FreeSubrect (sr->children[0]);
  FreeSubrect (sr->children[1]);
  alloc.Free (sr);
}

static inline uint Cantor (uint x, uint y)
{
  return ((x + y) * (x + y + 1)) / 2 + y;
}

int SubRectangles::SubRectCompare (SubRect* const& sr1, SubRect* const& sr2)
{
  uint c1 = Cantor (sr1->rect.xmin, sr1->rect.ymin);
  uint c2 = Cantor (sr2->rect.xmin, sr2->rect.ymin);
  if (c1 < c2)
    return -1;
  else if (c1 > c2)
    return 1;
  else
    /* @@@ Hmm... two SubRects with the same rect coords - 
           is that supposed to happen? */
    return csComparator<SubRect*, SubRect*>::Compare (sr1, sr2);;
}

void SubRectangles::Clear ()
{
  alloc.Free (root);
  leaves.DeleteAll();

  root = alloc.Alloc ();
  root->rect = region;
  root->superrect = this;
  leaves.Push (root);
}

SubRectangles::SubRect* SubRectangles::Alloc (int w, int h, csRect &rect)
{
  SubRectangles::SubRect::AllocInfo ai;

  for (size_t i = 0; i < leaves.GetSize (); i++)
  {
    leaves[i]->TestAlloc (w, h, ai);
    if (ai.res && (ai.d == 0)) break;
  }

  if (ai.res)
  {
    SubRectangles::SubRect* sr = ai.node->Alloc (w, h, ai, rect);
    return sr;
  }

  return 0;
}

void SubRectangles::Reclaim (SubRectangles::SubRect* subrect)
{
  if (subrect) subrect->Reclaim ();
}

void SubRectangles::Grow (SubRectangles::SubRect* sr, int ow, int oh, int nw, int nh)
{
  if (sr == 0) return;

  if (sr->rect.xmax == ow) sr->rect.xmax = nw;
  if (sr->rect.ymax == oh) sr->rect.ymax = nh;

  if (sr->splitType != SubRectangles::SubRect::SPLIT_UNSPLIT)
  {
    Grow (sr->children[0], ow, oh, nw, nh);
    Grow (sr->children[1], ow, oh, nw, nh);
  }
}

bool SubRectangles::Grow (int newWidth, int newHeight)
{
  if ((newWidth < region.Width ()) || (newHeight < region.Height ()))
    return false;

  Grow (root, region.Width (), region.Height (), newWidth, newHeight);
  region.SetSize (newWidth, newHeight);
  return true;
}

bool SubRectangles::Shrink (SubRectangles::SubRect* sr, int ow, int oh, int nw, int nh)
{
  if (sr == 0) return true;

  if ((sr->allocedRect.xmax > nw) || (sr->allocedRect.ymax > nh))
  {
    return false;
  }

  if (sr->rect.xmax >= ow) sr->rect.xmax = nw;
  if (sr->rect.ymax >= oh) sr->rect.ymax = nh;

  if (sr->splitType != SubRectangles::SubRect::SPLIT_UNSPLIT)
  {
    if (!Shrink (sr->children[0], ow, oh, nw, nh))
    {
      Grow (sr->children[0], nw, nh, ow, oh);
      return false;
    }
    if (!Shrink (sr->children[1], ow, oh, nw, nh))
    {
      Grow (sr->children[1], nw, nh, ow, oh);
      return false;
    }
  }
  return true;
}

bool SubRectangles::Shrink (int newWidth, int newHeight)
{
  bool ret = Shrink (root, region.Width (), region.Height (), 
    newWidth, newHeight);
  if (ret) 
    region.SetSize (newWidth, newHeight);
  else
  {
    root->rect.xmax = region.Width ();
    root->rect.ymax = region.Height ();
  }
  return ret;
}

csRect SubRectangles::GetMinimumRectangle (SubRect* sr) const
{
  csRect r (sr->allocedRect);
  if (sr->splitType != SubRectangles::SubRect::SPLIT_UNSPLIT)
  {
    r.Union (GetMinimumRectangle (sr->children[0]));
    r.Union (GetMinimumRectangle (sr->children[1]));
  }
  return r;
}

void SubRectangles::DupeWithOffset (const SubRect* from, SubRect* to, 
  int x, int y, csHash<SubRect*, csConstPtrKey<SubRect> >* map,
  const csRect& outerAllocated, const csRect& outerRect)
{
  to->rect = from->rect;
  to->rect.Move (x, y);
  if (to->rect.xmax == outerAllocated.xmax)
    to->rect.xmax = outerRect.xmax;
  if (to->rect.ymax == outerAllocated.ymax)
    to->rect.ymax = outerRect.ymax;

  to->allocedRect = from->allocedRect;
  to->allocedRect.Move (x, y);
  to->splitPos = from->splitPos;
  to->splitType = from->splitType;

  if (from->children[0] != 0)
  {
    SubRect* newChild = AllocSubrect ();
    newChild->superrect = this;
    newChild->parent = to;
    DupeWithOffset (from->children[0], newChild, x, y, map,
      outerAllocated, outerRect);
    to->children[0] = newChild;
  }

  if (from->children[1] != 0)
  {
    SubRect* newChild = AllocSubrect ();
    newChild->superrect = this;
    newChild->parent = to;
    DupeWithOffset (from->children[1], newChild, x, y, map,
      outerAllocated, outerRect);
    to->children[1] = newChild;
  }

  if (map != 0) map->Put (from, to);

  if (to->splitType == SubRect::SPLIT_UNSPLIT)
    AddLeaf (to);
}

bool SubRectangles::PlaceInto (const SubRectangles* rectangles, 
  SubRect* subRect, 
  csHash<SubRect*, csConstPtrKey<SubRect> >* newRectangles)
{
  if ((rectangles->region.Width() > subRect->allocedRect.Width())
    || (rectangles->region.Height() > subRect->allocedRect.Height()))
    return false;

  while (subRect->children[0] != 0)
    subRect = subRect->children[0];

  RemoveLeaf (subRect);

  csRect outerAllocated (subRect->allocedRect);
  csRect outerRect (subRect->rect);
  DupeWithOffset (rectangles->root, subRect, 
    subRect->rect.xmin, subRect->rect.ymin, newRectangles,
    outerAllocated, outerRect);

  return true;
}

#if defined(DUMP_TO_IMAGES)
static void FillImgRect (uint8* data, uint8 color, int imgW, int imgH, 
			 const csRect& r)
{
  int x, y;
  uint8* p = data + (r.ymin * imgW) + r.xmin;
  for (y = r.ymin; y < r.ymax; y++)
  {
    for (x = r.xmin; x < r.xmax; x++)
    {
      *p++ = color;
    }
    p += (imgW - r.Width ());
  }
}

static void IncImgRect (uint8* data, int imgW, int imgH, 
			const csRect& r)
{
  int x, y;
  uint8* p = data + (r.ymin * imgW) + r.xmin;
  for (y = r.ymin; y < r.ymax; y++)
  {
    for (x = r.xmin; x < r.xmax; x++)
    {
      (*p++)++;
    }
    p += (imgW - r.Width ());
  }
}
#endif

// Debug dump: write some rect distribution into some images.
void SubRectangles::Dump (const char* tag)
{
#if defined(DUMP_TO_IMAGES)
  if (!iSCF::SCF->object_reg) return;

  csRef<iImageIO> imgsaver =
    csQueryRegistry<iImageIO> (iSCF::SCF->object_reg);
  csRef<iVFS> vfs =
    csQueryRegistry<iVFS> (iSCF::SCF->object_reg);

  if (!imgsaver || !vfs) return;
  
  csRGBpixel pal[256];
  int i;
  for (i = 0; i < 256; i++)
  {
    pal[i].red = csQint ((((i & 0x01) << 2) + ((i & 0x08) >> 2) +
      ((i & 0x40) >> 6)) * (255.0f / 7.0f));
    pal[i].green = csQint ((((i & 0x02) << 1) + ((i & 0x10) >> 3) +
      ((i & 0x80) >> 7)) * (255.0f / 7.0f));
    pal[i].blue = csQint ((((i & 0x04) >> 1) + ((i & 0x20) >> 5)) *
      (255.0f / 3.0f));
  }

  int w = region.Width (), h = region.Height ();

  csRGBpixel* newpal = new csRGBpixel[256];
  memcpy (newpal, pal, sizeof (pal));

  csImageMemory* img = 
    new csImageMemory (w, h, (new uint8[w * h]),
    true, CS_IMGFMT_PALETTED8, newpal);

  uint8* data = (uint8*)img->GetImageData ();
  memset (data, 0, w * h);

  newpal = new csRGBpixel[256];
  memcpy (newpal, pal, sizeof (pal));

  csImageMemory* img2 = 
    new csImageMemory (w, h, (new uint8[w * h]),
    true, CS_IMGFMT_PALETTED8, newpal);

  uint8* data2 = (uint8*)img2->GetImageData ();
  memset (data2, 0, w * h);

  newpal = new csRGBpixel[256];
  memcpy (newpal, pal, sizeof (pal));

  csImageMemory* img3 = 
    new csImageMemory (w, h, (new uint8[w * h]),
    true, CS_IMGFMT_PALETTED8, newpal);

  uint8* data3 = (uint8*)img3->GetImageData ();
  memset (data3, 0, w * h);

  int c = 0;
  csArray<SubRectangles::SubRect*> nodes;
  nodes.Push (root);
  
  while (nodes.GetSize ())
  {
    SubRectangles::SubRect* node = nodes[0];
    nodes.DeleteIndex (0);

    FillImgRect (data, c + 1, w, h, node->rect);
    FillImgRect (data2, c + 1, w, h, node->allocedRect);
    // Overlap information. The whole image should be just one color.
    if (node->splitType == csSubRect::SPLIT_UNSPLIT)
      IncImgRect (data3, w, h, node->rect);

    if (node->children[0] != 0) nodes.Push (node->children[0]);
    if (node->children[1] != 0) nodes.Push (node->children[1]);
    c = (c + 1) % 255;
  }

  csString tagStr (tag);
  if (tagStr.IsEmpty()) tagStr.Format ("%p", this);

  {
    csRef<iDataBuffer> buf = imgsaver->Save (img, "image/png");
    csString outfn;
    outfn.Format ("/tmp/SubRectangles_dump_%s_r.png", tagStr.GetData());
    if (vfs->WriteFile (outfn, (char*)buf->GetInt8 (), buf->GetSize ()))
    {
      csReport (iSCF::SCF->object_reg, CS_REPORTER_SEVERITY_NOTIFY,
	"crystalspace.geom.subrects", "Successfully dumped to %s",
	outfn.GetData ());
    }
    else
    {
      csReport (iSCF::SCF->object_reg, CS_REPORTER_SEVERITY_NOTIFY,
	"crystalspace.geom.subrects", "Error dumping to %s",
	outfn.GetData ());
    }
    delete img;

    buf = imgsaver->Save (img2, "image/png");
    outfn.Format ("/tmp/SubRectangles_dump_%s_ar.png", tagStr.GetData());
    if (vfs->WriteFile (outfn, (char*)buf->GetInt8 (), buf->GetSize ()))
    {
      csReport (iSCF::SCF->object_reg, CS_REPORTER_SEVERITY_NOTIFY,
	"crystalspace.geom.subrects", "Successfully dumped to %s",
	outfn.GetData ());
    }
    else
    {
      csReport (iSCF::SCF->object_reg, CS_REPORTER_SEVERITY_NOTIFY,
	"crystalspace.geom.subrects", "Error dumping to %s",
	outfn.GetData ());
    }
    delete img2;

    buf = imgsaver->Save (img3, "image/png");
    outfn.Format ("/tmp/SubRectangles_dump_%s_ov.png", tagStr.GetData());
    if (vfs->WriteFile (outfn, (char*)buf->GetInt8 (), buf->GetSize ()))
    {
      csReport (iSCF::SCF->object_reg, CS_REPORTER_SEVERITY_NOTIFY,
	"crystalspace.geom.subrects", "Successfully dumped to %s",
	outfn.GetData ());
    }
    else
    {
      csReport (iSCF::SCF->object_reg, CS_REPORTER_SEVERITY_NOTIFY,
	"crystalspace.geom.subrects", "Error dumping to %s",
	outfn.GetData ());
    }
    delete img3;
  }
#endif
}

//---------------------------------------------------------------------------

SubRectanglesCompact::SubRectanglesCompact (const csRect& maxArea) :
  SubRectangles (csRect (0, 0, 0, 0)), maxArea (maxArea)
{
}

SubRectanglesCompact::SubRectanglesCompact (const SubRectanglesCompact& other) :
  SubRectangles (other), maxArea (other.maxArea)
{
}

void SubRectanglesCompact::Clear ()
{
  SubRectangles::Clear ();
  region.Set (0, 0, 0, 0);
}

static inline int GetDimension (const csRect& r, int side)
{
  return (side == 0) ? r.Width() : r.Height();
}

static inline void SetDimension (csRect& r, int side, int v)
{
  int nw = r.Width();
  int nh = r.Height();
  if (side == 0) 
    nw = v;
  else
    nh = v;
  r.SetSize (nw, nh);
}

SubRectangles::SubRect* SubRectanglesCompact::Alloc (int w, int h, 
                                                     csRect& rect)
{
  // Try if rectangle fits already.
  SubRectangles::SubRect* r = SubRectangles::Alloc (w, h, rect);
  if (r == 0)
  {
    // Otherwise try with enlarging first one side, then the other
    int smallerSide = (region.Width() <= region.Height ()) ? 0 : 1;
    for (int s = 0; s < 2; s++)
    {
      csRect oldRect (region);
      int side = s ^ smallerSide;
      csRect newRect (region);
      // Enlarge one side
      SetDimension (newRect, side, 
        csMin (GetDimension (region, side) + ((side == 0) ? w : h), 
          GetDimension (maxArea, side)));
      // Ensure other side is at least as large as requested
      if (GetDimension (newRect, side ^ 1) < ((side == 0) ? h : w))
      {
        SetDimension (newRect, side ^ 1, ((side == 0) ? h : w));
      }
      Grow (newRect.Width(), newRect.Height());
      r = SubRectangles::Alloc (w, h, rect);
      if (r != 0) break;
      // Restore old dimensions
      Shrink (oldRect.Width(), oldRect.Height());
    }
  }
  return r;
}

} // namespace CS
