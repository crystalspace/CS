/*
    Copyright (C) 1998 by Jorrit Tyberghein

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
#include "csutil/bitset.h"
#include "csutil/hashmap.h"
#include "csgeom/math3d.h"
#include "csgeom/math2d.h"
#include "csgeom/polyclip.h"
#include "csgeom/polypool.h"
#include "csgeom/frustum.h"
#include "csengine/dumper.h"
#include "csengine/bsp.h"
#include "csengine/bsp2d.h"
#include "csengine/octree.h"
#include "csengine/quadtree.h"
#include "csengine/quadcube.h"
#include "csengine/lppool.h"
#include "csengine/camera.h"
#include "csengine/curve.h"
#include "csengine/polytmap.h"
#include "csengine/polygon.h"
#include "csengine/pol2d.h"
#include "csengine/polytext.h"
#include "csengine/texture.h"
#include "csengine/polyset.h"
#include "csengine/thing.h"
#include "csengine/sector.h"
#include "csengine/engine.h"
#include "csengine/cssprite.h"
#include "csengine/light.h"
#include "csengine/lghtmap.h"

void Dumper::dump (csMatrix3* m, char const* name)
{
  CsPrintf (MSG_DEBUG_0, "Matrix '%s':\n", name);
  CsPrintf (MSG_DEBUG_0, "/\n");
  CsPrintf (MSG_DEBUG_0, "| %3.2f %3.2f %3.2f\n", m->m11, m->m12, m->m13);
  CsPrintf (MSG_DEBUG_0, "| %3.2f %3.2f %3.2f\n", m->m21, m->m22, m->m23);
  CsPrintf (MSG_DEBUG_0, "| %3.2f %3.2f %3.2f\n", m->m31, m->m32, m->m33);
  CsPrintf (MSG_DEBUG_0, "\\\n");
}

void Dumper::dump (csVector3* v, char const* name)
{
  CsPrintf (MSG_DEBUG_0, "Vector '%s': (%f,%f,%f)\n", name, v->x, v->y, v->z);
}

void Dumper::dump (csVector2* v, char const* name)
{
  CsPrintf (MSG_DEBUG_0, "Vector '%s': (%f,%f)\n", name, v->x, v->y);
}

void Dumper::dump (csPlane3* p)
{
  CsPrintf (MSG_DEBUG_0, "A=%2.2f B=%2.2f C=%2.2f D=%2.2f\n",
            p->norm.x, p->norm.y, p->norm.z, p->DD);
}

void Dumper::dump (csBox2* b)
{
  CsPrintf (MSG_DEBUG_0, "(%2.2f,%2.2f)-(%2.2f,%2.2f)",
  	b->MinX (), b->MinY (), b->MaxX (), b->MaxY ());
}

void Dumper::dump (csCamera* c)
{
  CsPrintf (MSG_DEBUG_0, "csSector: %s\n", 
    c->sector ? c->sector->GetName () : "(?)");
  dump (&c->v_o2t, "Camera vector");
  dump (&c->m_o2t, "Camera matrix");
}

void Dumper::dump (csPolyTxtPlane* p)
{
  const char* pl_name = p->GetName ();
  CsPrintf (MSG_DEBUG_0, "PolyTxtPlane '%s' id=%ld:\n", pl_name ? pl_name : "(no name)",
            p->GetID ());
  dump (&p->m_obj2tex, "Mot");
  dump (&p->v_obj2tex, "Vot");
  dump (&p->m_world2tex, "Mwt");
  dump (&p->v_world2tex, "Vwt");
}

void Dumper::dump (csPolygon3D* p)
{
  csPolygonSet* ps = (csPolygonSet*)p->GetParent ();
  const char* ps_name = ps->GetName ();
  const char* p_name = p->GetName ();
  CsPrintf (MSG_DEBUG_0, "Dump polygon '%s/%s' id=%ld:\n", ps_name ? ps_name :
  	"null", p_name ? p_name : "null", p->GetID ());
  if (p->GetUnsplitPolygon ())
  {
    CsPrintf (MSG_DEBUG_0, "    Split from polygon with id=%ld\n",
    	((csPolygon3D*)(p->GetUnsplitPolygon ()))->GetID ());
  }
  CsPrintf (MSG_DEBUG_0, "    num_vertices=%d  max_vertices=%d\n",
  	p->GetVertices ().GetNumVertices (), p->GetVertices ().max_vertices);
  if (p->portal)
  {
    csPortal* portal = p->portal;
    CsPrintf (MSG_DEBUG_0, "    Polygon is a CS portal to sector '%s'.\n", 
      portal->GetSector()->GetName ());
    CsPrintf (MSG_DEBUG_0, "    Alpha=%d\n", p->GetAlpha ());
  }
  if (p->poly_set->cam_verts == NULL)
  {
    int i;
    for (i = 0 ; i < p->GetVertices ().GetNumVertices () ; i++)
      CsPrintf (MSG_DEBUG_0, "        v[%d]: (%f,%f,%f)\n",
    	i, p->Vwor (i).x, p->Vwor (i).y, p->Vwor (i).z);
  }
  else
  {
    int i;
    for (i = 0 ; i < p->GetVertices ().GetNumVertices () ; i++)
      CsPrintf (MSG_DEBUG_0, "        v[%d]: (%f,%f,%f)  camera:(%f,%f,%f)\n",
    	i,
    	p->Vwor (i).x, p->Vwor (i).y, p->Vwor (i).z,
    	p->Vcam (i).x, p->Vcam (i).y, p->Vcam (i).z);
  }
  if (p->GetTextureType () == POLYTXT_LIGHTMAP)
  {
    csPolyTexLightMap* lmi = p->GetLightMapInfo ();
    dump (lmi->GetTxtPlane ());
    if (lmi->GetPolyTex ()) dump (lmi->GetPolyTex (), "PolyTexture");
  }

  csLightPatch* lp = p->light_info.lightpatches;
  while (lp)
  {
    CsPrintf (MSG_DEBUG_0, "  LightPatch (num_vertices=%d, light=%08lx)\n", lp->num_vertices,
    	lp->GetLight ());
    lp = lp->GetNextPoly ();
  }
}

void Dumper::dump (csPolygonSet* p)
{
  CsPrintf (MSG_DEBUG_0, "========================================================\n");
  CsPrintf (MSG_DEBUG_0, "Dump sector '%s' id=%ld:\n", 
    p->GetName (), p->GetID ());
  CsPrintf (MSG_DEBUG_0, "    num_vertices=%d max_vertices=%d polygons=%d\n",
    p->num_vertices, p->max_vertices, p->GetNumPolygons ());
  int i;

  CsPrintf (MSG_DEBUG_0, "  Vertices:\n");
  for (i = 0 ; i < p->num_vertices ; i++)
  {
    CsPrintf (MSG_DEBUG_0, "    Vertex[%d]=\n", i);
    dump (&p->wor_verts[i], "world");
    dump (&p->obj_verts[i], "object");
    //dump (&p->cam_verts[i], "camera");
    CsPrintf (MSG_DEBUG_0, "\n");
  }
  CsPrintf (MSG_DEBUG_0, "  Polygons:\n");
  for (i = 0 ; i < p->GetNumPolygons () ; i++)
  {
    CsPrintf (MSG_DEBUG_0, "------------------------------------------\n");
    csPolygon3D* pp = p->GetPolygon3D (i);
    dump (pp);
  }
}

void Dumper::dump (csSector* s)
{
  dump ((csPolygonSet*)s);
  int i;
  //for (i = 0 ; i < s->sprites.Length () ; i++)
  //{
    //csSprite* sp = (csSprite*)s->sprites[i];
  //}
  for (i = 0 ; i < s->things.Length () ; i++)
  {
    csThing* th = (csThing*)s->things[i];
    dump ((csPolygonSet*)th);
  }
  for (i = 0 ; i < s->skies.Length () ; i++)
  {
    csThing* th = (csThing*)s->skies[i];
    dump ((csPolygonSet*)th);
  }
}

void Dumper::dump (csEngine* e)
{
  int sn = e->sectors.Length ();
  while (sn > 0)
  {
    sn--;
    csSector* s = (csSector*)e->sectors[sn];
    dump (s);
  }
}

void Dumper::dump (csPolyTexture* p, char const* name)
{
  CsPrintf (MSG_DEBUG_0, "  PolyTexture '%s'\n", name);
  CsPrintf (MSG_DEBUG_0, "    Imin_u=%d Imin_v=%d Imax_u=%d Imax_v=%d\n", p->Imin_u, p->Imin_v, p->Imax_u, p->Imax_v);
  CsPrintf (MSG_DEBUG_0, "    Fmin_u=%f Fmin_v=%f Fmax_u=%f Fmax_v=%f\n", p->Fmin_u, p->Fmin_v, p->Fmax_u, p->Fmax_v);
  if (p->lm)
  {
    CsPrintf (MSG_DEBUG_0, "    lm: lwidth=%d lheight=%d rwidth=%d rheight=%d lm_size=%d\n", p->lm->lwidth,
  	p->lm->lheight, p->lm->rwidth, p->lm->rheight, p->lm->lm_size);
    CsPrintf (MSG_DEBUG_0, "    lm: lm_size=%ld\n",
    	p->lm->lm_size);
  }
  else
    CsPrintf (MSG_DEBUG_0, "    lm: no lightmap\n");
  CsPrintf (MSG_DEBUG_0, "    shf_u=%d and_u=%d\n", p->shf_u, p->and_u);
  CsPrintf (MSG_DEBUG_0, "    fdu=%f fdv=%f\n", p->fdu, p->fdv);
  CsPrintf (MSG_DEBUG_0, "    w=%d h=%d w_orig=%d\n", p->w, p->h, p->w_orig);
}

void Dumper::dump (csPolygon2D* p, char const* name)
{
  CsPrintf (MSG_DEBUG_0, "Dump polygon 2D '%s':\n", name);
  CsPrintf (MSG_DEBUG_0, "    num_vertices=%d  max_vertices=%d\n", p->num_vertices, p->max_vertices);
  int i;
  for (i = 0 ; i < p->num_vertices ; i++)
    CsPrintf (MSG_DEBUG_0, "        v[%d]: (%f,%f)\n", i, p->vertices[i].x, p->vertices[i].y);
}


void Dumper::dump (csOctree* tree, csOctreeNode* node, int indent)
{
  if (!node) return;
  int i;
  char spaces[256];
  for (i = 0 ; i < indent ; i++) spaces[i] = ' ';
  spaces[indent] = 0;
  const csBox3& b = node->GetBox ();
  const csVector3& c = node->GetCenter ();
  CsPrintf (MSG_DEBUG_0, "%sbbox=%f,%f,%f %f,%f,%f.\n", spaces,
	b.MinX (), b.MinY (), b.MinZ (), b.MaxX (), b.MaxY (), b.MaxZ ());
  CsPrintf (MSG_DEBUG_0, "%scenter=%f,%f,%f.\n", spaces, c.x, c.y, c.z);
  CsPrintf (MSG_DEBUG_0, "%ssolid x=%4x X=%4x y=%4x Y=%4x z=%4x Z=%4x\n",
  	spaces,
  	node->GetSolidMask (BOX_SIDE_x), node->GetSolidMask (BOX_SIDE_X),
  	node->GetSolidMask (BOX_SIDE_y), node->GetSolidMask (BOX_SIDE_Y),
  	node->GetSolidMask (BOX_SIDE_z), node->GetSolidMask (BOX_SIDE_Z));
  if (node->GetMiniBsp ())
  {
    CsPrintf (MSG_DEBUG_0, "%s BSP\n", spaces);
    dump (node->GetMiniBsp (), (csBspNode*)(node->GetMiniBsp ()->root), indent+2);
  }
  for (i = 0 ; i < 8 ; i++)
  {
    CsPrintf (MSG_DEBUG_0, "%s Child %d:\n", spaces, i);
    dump (tree, (csOctreeNode*)(node->children[i]), indent+2);
  }
}

void Dumper::dump (csOctree* tree)
{
  dump (tree, (csOctreeNode*)(tree->root), 0);
}

void Dumper::dump (csBspTree* tree, csBspNode* node, int indent)
{
  if (!node) return;
  int i;
  char spaces[256];
  for (i = 0 ; i < indent ; i++) spaces[i] = ' ';
  spaces[indent] = 0;
  CsPrintf (MSG_DEBUG_0, "%sThere are %d polygons in this node (pol split=%d).\n",
  	spaces, node->polygons.GetNumPolygons (), node->polygons_on_splitter);
  for (i = 0 ; i < node->polygons.GetNumPolygons () ; i++)
  {
    if (node->polygons.GetPolygon (i)->GetType () == 1)
    {
      csPolygon3D* p = (csPolygon3D*)node->polygons.GetPolygon (i);
      CsPrintf (MSG_DEBUG_0, "%s  %d='%s'\n", spaces, i, p->GetName () ? p->GetName () : "<noname>");
    }
  }
  if (node->front || node->back)
  {
    CsPrintf (MSG_DEBUG_0, "%s Splitter: %f,%f,%f,%f\n", spaces,
    	node->splitter.A (), node->splitter.B (),
    	node->splitter.C (), node->splitter.D ());
    CsPrintf (MSG_DEBUG_0, "%s Front:\n", spaces);
    dump (tree, node->front, indent+2);
    CsPrintf (MSG_DEBUG_0, "%s Back:\n", spaces);
    dump (tree, node->back, indent+2);
  }
}

void Dumper::dump (csBspTree2D* tree, csBspNode2D* node, int indent)
{
  if (!node) return;
  int i;
  char spaces[256];
  for (i = 0 ; i < indent ; i++) spaces[i] = ' ';
  spaces[indent] = 0;
  CsPrintf (MSG_DEBUG_0, "%sThere are %d segments in this node.\n", spaces,
    node->segments.Length ());
  for (i = 0 ; i < node->segments.Length () ; i++)
  {
    csSegment2* s = node->segments.Get (i);
    CsPrintf (MSG_DEBUG_0, "%s%d: (%f,%f)-(%f,%f)\n", spaces,
      i, s->Start ().x, s->Start ().y, s->End ().x, s->End ().y);
  }
  CsPrintf (MSG_DEBUG_0, "%s Front:\n", spaces);
  dump (tree, node->front, indent+2);
  CsPrintf (MSG_DEBUG_0, "%s Back:\n", spaces);
  dump (tree, node->back, indent+2);
}

void Dumper::dump (csBspTree* tree)
{
  dump (tree, (csBspNode*)(tree->root), 0);
}

void Dumper::dump (csBspTree2D* tree)
{
  dump (tree, tree->root, 0);
}

void Dumper::dump (csPolygonClipper* clipper, char const* name)
{
  CsPrintf (MSG_DEBUG_0, "PolygonClipper '%s'\n", name);
  int i;
  for (i = 0 ; i < clipper->ClipPolyVertices ; i++)
    CsPrintf (MSG_DEBUG_0, "  %d: (%f,%f)\n", i, clipper->ClipPoly[i].x, clipper->ClipPoly[i].y);
}

void Dumper::dump (csFrustum* frustum, char const* name)
{
  CsPrintf (MSG_DEBUG_0, "csFrustum '%s'\n", name);
  if (!frustum)
  {
    CsPrintf (MSG_DEBUG_0, "  NULL\n");
    return;
  }
  if (frustum->IsEmpty ())
  {
    CsPrintf (MSG_DEBUG_0, "  EMPTY\n");
    return;
  }
  if (frustum->IsInfinite ())
  {
    CsPrintf (MSG_DEBUG_0, "  INFINITE\n");
    return;
  }
  int i;
  CsPrintf (MSG_DEBUG_0, "  "); dump (&frustum->GetOrigin (), "origin");
  for (i = 0 ; i < frustum->GetNumVertices () ; i++)
  {
    CsPrintf (MSG_DEBUG_0, "  ");
    char buf[20];
    sprintf (buf, "[%d]", i);
    dump (&frustum->GetVertex (i), buf);
  }
  if (frustum->GetBackPlane ())
  {
    CsPrintf (MSG_DEBUG_0, "  ");
    dump (frustum->GetBackPlane ());
  }
}

void Dumper::dump (csPoly2DPool* pool, char const* name)
{
  int cnt;
  ULong tot_size;
  int max_size;

  cnt = 0;
  tot_size = 0;
  max_size = 0;
  csPoly2DPool::PoolObj* po = pool->alloced;
  while (po)
  {
    tot_size += po->pol2d->max_vertices;
    if (po->pol2d->max_vertices > max_size) max_size = po->pol2d->max_vertices;
    cnt++;
    po = po->next;
  }
  CsPrintf (MSG_DEBUG_0, "Poly2D pool (%s): %d allocated (max=%d, tot=%ld), ",
  	name, cnt, max_size, tot_size);
  cnt = 0;
  tot_size = 0;
  max_size = 0;
  po = pool->freed;
  while (po)
  {
    tot_size += po->pol2d->max_vertices;
    if (po->pol2d->max_vertices > max_size) max_size = po->pol2d->max_vertices;
    cnt++;
    po = po->next;
  }
  CsPrintf (MSG_DEBUG_0, "%d freed (max=%d, tot=%ld).\n",
  	cnt, max_size, tot_size);
}

void Dumper::dump (csLightPatchPool* pool, char const* name)
{
  int cnt;
  cnt = 0;
  csLightPatchPool::PoolObj* po = pool->alloced;
  while (po) { cnt++; po = po->next; }
  CsPrintf (MSG_DEBUG_0, "LightPatch pool (%s): %d allocated, ", name, cnt);
  cnt = 0;
  po = pool->freed;
  while (po) { cnt++; po = po->next; }
  CsPrintf (MSG_DEBUG_0, "%d freed.\n", cnt);
}


#if 0
void Dumper::dump (csQuadtreeNode* node, char* buf, int bufdim, int depth,
  	int x1, int y1, int x2, int y2)
{
  if (node->GetState () == CS_QUAD_PARTIAL && node->children)
  {
    int cx = x1+(x2-x1+1)/2;
    int cy = y1+(y2-y1+1)/2;
    dump (&node->children[0], buf, bufdim, depth-1, x1, y1, cx-1, cy-1);
    dump (&node->children[1], buf, bufdim, depth-1, cx, y1, x2, cy-1);
    dump (&node->children[2], buf, bufdim, depth-1, cx, cy, x2, y2);
    dump (&node->children[3], buf, bufdim, depth-1, x1, cy, cx-1, y2);
  }
  else
  {
    char fchar;
    if (node->GetState () == CS_QUAD_EMPTY) fchar = '.';
    else if (node->GetState () == CS_QUAD_FULL) fchar = '#';
    else fchar = 'x';
    int i, j;
    for (i = x1 ; i <= x2 ; i++)
      for (j = y1 ; j <= y2 ; j++)
        buf[j*bufdim+i] = fchar;
  }
}

void Dumper::dump (csQuadcube* cube)
{
  CsPrintf (MSG_DEBUG_0, "---------------------------------------\n");
  CsPrintf (MSG_DEBUG_0, "Quadtree 0:\n"); dump (cube->trees[0]);
  CsPrintf (MSG_DEBUG_0, "Quadtree 1:\n"); dump (cube->trees[1]);
  CsPrintf (MSG_DEBUG_0, "Quadtree 2:\n"); dump (cube->trees[2]);
  CsPrintf (MSG_DEBUG_0, "Quadtree 3:\n"); dump (cube->trees[3]);
  CsPrintf (MSG_DEBUG_0, "Quadtree 4:\n"); dump (cube->trees[4]);
  CsPrintf (MSG_DEBUG_0, "Quadtree 5:\n"); dump (cube->trees[5]);
  CsPrintf (MSG_DEBUG_0, "---------------------------------------\n");
}

void Dumper::dump (csQuadtree* tree)
{
  // First calculate depth of tree.
  int depth = 0;
  csQuadtreeNode* n = tree->root;
  while (n)
  {
    depth++;
    n = n->children;
  }

  // Calculate 2^depth.
  int depthp = 1;
  int depth2 = depth-1;
  while (depth2 > 0)
  {
    depthp <<= 1;
    depth2--;
  }

  char* buf;
  buf = new char [depthp*depthp];

  CsPrintf (MSG_DEBUG_0, "Depth=%d depthp=%d\n", depth, depthp);
  memset (buf, '?', depthp*depthp);
  dump (tree->root, buf, depthp, depth-1,
  	0, 0, depthp-1, depthp-1);
  char buf2[255];
  int y;
  for (y = 0 ; y < depthp ; y++)
  {
    memcpy (buf2, &buf[y*depthp], depthp);
    buf2[depthp] = 0;
    CsPrintf (MSG_DEBUG_0, "%s\n", buf2);
  }

  delete [] buf;
}
#endif // quad removal

char* spaces (int nr)
{
  static char spc[200];
  spc[0] = 0;
  int i;
  for (i = 0 ; i < nr ; i++) spc[i] = ' ';
  spc[nr] = 0;
  return spc;
}

bool Dumper::check_stubs (csBspNode* node)
{
  if (!node) return false;
  if (node->first_stub || node->todo_stubs) return true;
  if (check_stubs (node->front)) return true;
  if (check_stubs (node->back)) return true;
  return false;
}

bool Dumper::check_stubs (csOctreeNode* node)
{
  if (!node) return false;
  if (node->first_stub || node->todo_stubs) return true;
  if (check_stubs ((csOctreeNode*)node->children[0])) return true;
  if (check_stubs ((csOctreeNode*)node->children[1])) return true;
  if (check_stubs ((csOctreeNode*)node->children[2])) return true;
  if (check_stubs ((csOctreeNode*)node->children[3])) return true;
  if (check_stubs ((csOctreeNode*)node->children[4])) return true;
  if (check_stubs ((csOctreeNode*)node->children[5])) return true;
  if (check_stubs ((csOctreeNode*)node->children[6])) return true;
  if (check_stubs ((csOctreeNode*)node->children[7])) return true;
  if (node->minibsp && check_stubs ((csBspNode*)node->minibsp->root)) return true;
  return false;
}

void Dumper::dump_stubs_node (csObjectStub* stub, char const* name, int level)
{
  while (stub)
  {
    CsPrintf (MSG_DEBUG_0, "%s %s this=%08lx obj=%08lx node=%08lx ref=%d ",
    	spaces (level), name,
    	stub, stub->object, stub->node, stub->ref_count);
    csPolygonStub* ps = (csPolygonStub*)stub;
    CsPrintf (MSG_DEBUG_0, "numpol=%d\n", ps->GetNumPolygons ());
    if (stub->next_tree && stub->next_tree->prev_tree != stub)
      CsPrintf (MSG_DEBUG_0, "%s !!! next_tree link broken !!!\n", spaces (level));
    if (stub->prev_tree && stub->prev_tree->next_tree != stub)
      CsPrintf (MSG_DEBUG_0, "%s !!! prev_tree link broken !!!\n", spaces (level));
    if (!stub->prev_tree && stub->node->first_stub != stub &&
    	stub->node->todo_stubs != stub)
      CsPrintf (MSG_DEBUG_0, "%s !!! stub should be first in node but it isn't !!!\n", spaces (level));
    if (stub->prev_tree && (stub->node->first_stub == stub ||
    	stub->node->todo_stubs == stub))
      CsPrintf (MSG_DEBUG_0, "%s !!! stub should not be first in node but it is !!!\n", spaces (level));
    if (stub->next_obj && stub->next_obj->prev_obj != stub)
      CsPrintf (MSG_DEBUG_0, "%s !!! next_obj link broken !!!\n", spaces (level));
    if (stub->prev_obj && stub->prev_obj->next_obj != stub)
      CsPrintf (MSG_DEBUG_0, "%s !!! prev_obj link broken !!!\n", spaces (level));
    if (!stub->prev_obj && stub->object->first_stub != stub)
      CsPrintf (MSG_DEBUG_0, "%s !!! stub should be first in object but it isn't !!!\n", spaces (level));
    if (stub->prev_obj && stub->object->first_stub == stub)
      CsPrintf (MSG_DEBUG_0, "%s !!! stub should not be first in object but it is !!!\n", spaces (level));
    stub = stub->next_tree;
  }
}

void Dumper::dump_stubs_obj (csObjectStub* stub, char const* name, int level)
{
  while (stub)
  {
    CsPrintf (MSG_DEBUG_0, "%s %s this=%08lx obj=%08lx node=%08lx ref=%d ",
    	spaces (level), name,
    	stub, stub->object, stub->node, stub->ref_count);
    csPolygonStub* ps = (csPolygonStub*)stub;
    CsPrintf (MSG_DEBUG_0, "numpol=%d\n", ps->GetNumPolygons ());
    if (stub->next_tree && stub->next_tree->prev_tree != stub)
      CsPrintf (MSG_DEBUG_0, "%s !!! next_tree link broken !!!\n", spaces (level));
    if (stub->prev_tree && stub->prev_tree->next_tree != stub)
      CsPrintf (MSG_DEBUG_0, "%s !!! prev_tree link broken !!!\n", spaces (level));
    if (!stub->prev_tree && stub->node->first_stub != stub &&
    	stub->node->todo_stubs != stub)
      CsPrintf (MSG_DEBUG_0, "%s !!! stub should be first in node but it isn't !!!\n", spaces (level));
    if (stub->prev_tree && (stub->node->first_stub == stub ||
    	stub->node->todo_stubs == stub))
      CsPrintf (MSG_DEBUG_0, "%s !!! stub should not be first in node but it is !!!\n", spaces (level));
    if (stub->next_obj && stub->next_obj->prev_obj != stub)
      CsPrintf (MSG_DEBUG_0, "%s !!! next_obj link broken !!!\n", spaces (level));
    if (stub->prev_obj && stub->prev_obj->next_obj != stub)
      CsPrintf (MSG_DEBUG_0, "%s !!! prev_obj link broken !!!\n", spaces (level));
    if (!stub->prev_obj && stub->object->first_stub != stub)
      CsPrintf (MSG_DEBUG_0, "%s !!! stub should be first in object but it isn't !!!\n", spaces (level));
    if (stub->prev_obj && stub->object->first_stub == stub)
      CsPrintf (MSG_DEBUG_0, "%s !!! stub should not be first in object but it is !!!\n", spaces (level));
    stub = stub->next_obj;
  }
}

void Dumper::dump_stubs (csBspNode* bnode, char const* name, int level)
{
  if (!bnode) return;
  CsPrintf (MSG_DEBUG_0, "%s bnode(%s)\n", spaces (level), name);
  if (!check_stubs (bnode)) { CsPrintf (MSG_DEBUG_0, "%s  ..\n", spaces (level)); return; }
  dump_stubs_node (bnode->first_stub, "stub", level+1);
  dump_stubs_node (bnode->todo_stubs, "todo", level+1);
  dump_stubs (bnode->front, "front", level+1);
  dump_stubs (bnode->back, "back", level+1);
}
void Dumper::dump_stubs (csOctreeNode* onode, char const* name, int level)
{
  if (!onode) return;
  CsPrintf (MSG_DEBUG_0, "%s onode(%s) (%f,%f,%f)\n", spaces (level),
  	name,
  	onode->bbox.MaxX () - onode->bbox.MinX (),
  	onode->bbox.MaxY () - onode->bbox.MinY (),
  	onode->bbox.MaxZ () - onode->bbox.MinZ ());
  if (!check_stubs (onode)) { CsPrintf (MSG_DEBUG_0, "%s  ..\n", spaces (level)); return; }
  dump_stubs_node (onode->first_stub, "stub", level+1);
  dump_stubs_node (onode->todo_stubs, "todo", level+1);
  if (onode->minibsp) dump_stubs ((csBspNode*)(onode->minibsp->root),
  	"root", level+1);
  dump_stubs ((csOctreeNode*)onode->children[0], "0", level+1);
  dump_stubs ((csOctreeNode*)onode->children[1], "1", level+1);
  dump_stubs ((csOctreeNode*)onode->children[2], "2", level+1);
  dump_stubs ((csOctreeNode*)onode->children[3], "3", level+1);
  dump_stubs ((csOctreeNode*)onode->children[4], "4", level+1);
  dump_stubs ((csOctreeNode*)onode->children[5], "5", level+1);
  dump_stubs ((csOctreeNode*)onode->children[6], "6", level+1);
  dump_stubs ((csOctreeNode*)onode->children[7], "7", level+1);
}

void Dumper::dump_stubs (csOctree* octree)
{
  csDetailedPolyTreeObject::stub_pool.Dump ();
  CsPrintf (MSG_DEBUG_0, "Dump octree\n");
  dump_stubs ((csOctreeNode*)octree->root, "root", 0);
}

void Dumper::dump_stubs (csPolyTreeObject* ptobj)
{
  CsPrintf (MSG_DEBUG_0, "Dump csPolyTreeObject\n");
  dump_stubs_obj (ptobj->first_stub, "root", 0);
}

long Dumper::Memory (csCurve* c, int verbose_level, csHashSet* done)
{
  bool alloc = false;
  if (!done) { done = new csHashSet (211); alloc = true; }
  long memory = sizeof (*c), itemmem;
  memory += sizeof (csReversibleTransform);

  csLightMap* lm = c->lightmap;
  if (!done->In (lm))
  {
    done->Add (lm);
    itemmem = sizeof (csLightMap);
    itemmem += lm->static_lm.GetMaxSize () * 3;
    itemmem += lm->real_lm.GetMaxSize () * 3;
    csShadowMap* sm = lm->first_smap;
    while (sm)
    {
      itemmem += lm->lm_size;
      sm = sm->next;
    }
    memory += itemmem;
    if (verbose_level > 0)
      CsPrintf (MSG_DEBUG_0, "(curve '%s') lightmap (%ld bytes)\n",
    	  c->GetName () ? c->GetName () : "<noname>",
	  itemmem);
  }

  memory += sizeof (csVector3)*2*lm->lm_size;

  if (alloc) delete done;
  return memory;
}

long Dumper::Memory (csPolygon3D* p, int verbose_level, csHashSet* done)
{
  bool alloc = false;
  if (!done) { done = new csHashSet (211); alloc = true; }
  long memory = sizeof (*p), itemmem;
  memory += p->vertices.max_vertices * sizeof (int);
  if (p->material && !done->In (p->material))
  {
    done->Add (p->material);
    memory += sizeof (csMaterial);
  }
  if (p->portal && !done->In (p->portal))
  {
    done->Add (p->portal);
    itemmem = sizeof (csPortal);
    memory += itemmem;
    if (verbose_level > 0)
      CsPrintf (MSG_DEBUG_0, "(poly '%s') portal (%ld bytes)\n",
      	p->GetName () ? p->GetName () : "<noname>",
	itemmem);
  }
  if (!done->In (p->plane))
  {
    done->Add (p->plane);
    memory += sizeof (csPolyPlane);
  }

  // @@@ Here we assume that csMaterial are not shared.
  // This is not true but it is a good approx.
  itemmem = sizeof (csMaterial);
  memory += itemmem;

  csPolyTexLightMap* lmi = p->GetLightMapInfo ();
  if (lmi && !done->In (lmi))
  {
    done->Add (lmi);
    itemmem = sizeof (csPolyTexLightMap);
    itemmem += sizeof (csPolyTexture);
    itemmem += sizeof (csPolyTxtPlane);
    memory += itemmem;
    if (verbose_level > 0)
      CsPrintf (MSG_DEBUG_0, "(poly '%s') lmi (%ld bytes)\n",
      	p->GetName () ? p->GetName () : "<noname>",
	itemmem);
    csLightMap* lm = lmi->GetPolyTex ()->lm;
    if (lm && !done->In (lm))
    {
      done->Add (lm);
      itemmem = sizeof (csLightMap);
      itemmem += lm->static_lm.GetMaxSize () * 3;
      itemmem += lm->real_lm.GetMaxSize () * 3;
      csShadowMap* sm = lm->first_smap;
      while (sm)
      {
        itemmem += lm->lm_size;
        sm = sm->next;
      }
      memory += itemmem;
      if (verbose_level > 0)
        CsPrintf (MSG_DEBUG_0, "(poly '%s') lightmap (%ld bytes)\n",
      	  p->GetName () ? p->GetName () : "<noname>",
	  itemmem);
    }
  }
  csPolyTexGouraud* gs = p->GetGouraudInfo ();
  if (gs && !done->In (gs))
  {
    done->Add (gs);
    itemmem = sizeof (csPolyTexGouraud);
    if (gs->GetUVCoords ()) itemmem += p->GetNumVertices () * sizeof (csVector2);
    if (gs->GetColors ()) itemmem += p->GetNumVertices () * sizeof (csColor);
    if (gs->GetStaticColors ()) itemmem += p->GetNumVertices () * sizeof (csColor);
    memory += itemmem;
    if (verbose_level > 0)
      CsPrintf (MSG_DEBUG_0, "(poly '%s') gs (%ld bytes)\n",
      	p->GetName () ? p->GetName () : "<noname>",
	itemmem);
  }

  if (alloc) delete done;
  return memory;
}

long Dumper::Memory (csPolygonSet* pset, int verbose_level, csHashSet* done)
{
  bool alloc = false;
  if (!done) { done = new csHashSet (50119); alloc = true; }
  long memory = sizeof (*pset), itemmem;
  itemmem = pset->max_vertices * sizeof (csVector3);
  memory += itemmem+itemmem;
  if (verbose_level > 0)
    CsPrintf (MSG_DEBUG_0, "(pset '%s') vertices (2*%ld)\n",
    	pset->GetName () ? pset->GetName () : "<noname>",
	itemmem);
  memory += sizeof (csPolygonSetBBox);
  int i;
  itemmem = 0;
  for (i = 0 ; i < pset->polygons.Length () ; i++)
  {
    csPolygon3D* p = pset->polygons.Get (i);
    itemmem += Memory (p, verbose_level-1, done);
  }
  memory += itemmem;
  if (verbose_level > 0)
    CsPrintf (MSG_DEBUG_0, "(pset '%s') polygons (%ld)\n",
    	pset->GetName () ? pset->GetName () : "<noname>",
	itemmem);
  itemmem = 0;
  for (i = 0 ; i < pset->curves.Length () ; i++)
  {
    csCurve* c = pset->curves.Get (i);
    itemmem += Memory (c, verbose_level-1, done);
  }
  memory += itemmem;
  if (verbose_level > 0)
    CsPrintf (MSG_DEBUG_0, "(pset '%s') curves (%ld)\n",
    	pset->GetName () ? pset->GetName () : "<noname>",
	itemmem);

  if (alloc) delete done;
  return memory;
}

long Dumper::Memory (csThing* thing, int verbose_level, csHashSet* done)
{
  bool alloc = false;
  if (!done) { done = new csHashSet (50119); alloc = true; }
  long memory = sizeof (*thing), itemmem;

  // Subtract csPolygonSet size because that will be counted in the next call.
  itemmem = Memory ((csPolygonSet*)thing, verbose_level, done) - sizeof (csPolygonSet);
  memory += itemmem;
  if (verbose_level > 0) CsPrintf (MSG_DEBUG_0, "(thing '%s') pset (%ld bytes)\n",
    	thing->GetName () ? thing->GetName () : "<noname>",
	itemmem);

  if (alloc) delete done;
  return memory;
}

long Dumper::Memory (csSector* sect, int verbose_level, csHashSet* done)
{
  bool alloc = false;
  if (!done) { done = new csHashSet (3541); alloc = true; }
  long memory = sizeof (*sect), itemmem;
  int i;
  for (i = 0 ; i < sect->things.Length () ; i++)
  {
    csThing* thing = (csThing*)sect->things[i];
    itemmem = Memory (thing, verbose_level-1, done);
    memory += itemmem;
    if (verbose_level > 0) CsPrintf (MSG_DEBUG_0, "(sect '%s') thing '%s' (%ld bytes)\n",
    	sect->GetName () ? sect->GetName () : "<noname>",
    	thing->GetName () ? thing->GetName () : "<noname>", itemmem);
  }
  for (i = 0 ; i < sect->skies.Length () ; i++)
  {
    csThing* thing = (csThing*)sect->skies[i];
    itemmem = Memory (thing, verbose_level-1, done);
    memory += itemmem;
    if (verbose_level > 0) CsPrintf (MSG_DEBUG_0, "(sect '%s') sky '%s' (%ld bytes)\n",
    	sect->GetName () ? sect->GetName () : "<noname>",
    	thing->GetName () ? thing->GetName () : "<noname>", itemmem);
  }
  // Subtract csPolygonSet size because that will be counted in the next call.
  itemmem = Memory ((csPolygonSet*)sect, verbose_level, done) - sizeof (csPolygonSet);
  memory += itemmem;
  if (verbose_level > 0) CsPrintf (MSG_DEBUG_0, "(sect '%s') pset (%ld bytes)\n",
    	sect->GetName () ? sect->GetName () : "<noname>",
	itemmem);

  if (alloc) delete done;
  return memory;
}

long Dumper::Memory (csBspNode* bnode, int verbose_level, csHashSet* done)
{
  bool alloc = false;
  if (!done) { done = new csHashSet (211); alloc = true; }
  long memory = sizeof (*bnode), itemmem;

  itemmem = 0;
  if (bnode->front)
    itemmem += Memory ((csBspNode*)bnode->front, verbose_level-1, done);
  if (bnode->back)
    itemmem += Memory ((csBspNode*)bnode->back, verbose_level-1, done);
  memory += itemmem;
  if (verbose_level > 0) CsPrintf (MSG_DEBUG_0, "(octnode) children (%ld bytes)\n", itemmem);

  itemmem = bnode->polygons.max * sizeof (void*);
  memory += itemmem;

  if (alloc) delete done;
  return memory;
}

long Dumper::Memory (csBspTree* btree, int verbose_level, csHashSet* done)
{
  bool alloc = false;
  if (!done) { done = new csHashSet (211); alloc = true; }
  long memory = sizeof (*btree);
  if (btree->root) memory += Memory ((csBspNode*)(btree->root), verbose_level-1, done);

  if (alloc) delete done;
  return memory;
}

long Dumper::Memory (csOctreeNode* onode, int verbose_level, csHashSet* done)
{
  bool alloc = false;
  if (!done) { done = new csHashSet (211); alloc = true; }
  long memory = sizeof (*onode), itemmem;

  int i;
  itemmem = 0;
  for (i = 0 ; i < 8 ; i++)
  {
    if (onode->children[i])
      itemmem += Memory ((csOctreeNode*)onode->children[i], verbose_level-1, done);
  }
  memory += itemmem;
  if (verbose_level > 0) CsPrintf (MSG_DEBUG_0, "(octnode) children (%ld bytes)\n", itemmem);

  if (onode->minibsp)
  {
    itemmem = Memory (onode->minibsp, verbose_level-1, done);
    memory += itemmem;
    if (verbose_level > 0) CsPrintf (MSG_DEBUG_0, "(octnode) bsp (%ld bytes)\n", itemmem);
    itemmem = onode->minibsp_numverts * sizeof (int);
    memory += itemmem;
  }

  itemmem = sizeof (csPVS);
  csPVS& pvs = onode->pvs;
  csOctreeVisible* ovis = pvs.GetFirst ();
  while (ovis)
  {
    itemmem += sizeof (*ovis);
    itemmem += ovis->GetPolygons ().Limit () * sizeof (void*);
    ovis = pvs.GetNext (ovis);
  }
  memory += itemmem;
  if (verbose_level > 0) CsPrintf (MSG_DEBUG_0, "(octnode) pvs (%ld bytes)\n", itemmem);

  itemmem = onode->unsplit_polygons.max * sizeof (void*);
  memory += itemmem;

  if (alloc) delete done;
  return memory;
}

long Dumper::Memory (csOctree* otree, int verbose_level, csHashSet* done)
{
  bool alloc = false;
  if (!done) { done = new csHashSet (211); alloc = true; }
  long memory = sizeof (*otree);
  if (otree->root) memory += Memory ((csOctreeNode*)(otree->root), verbose_level-1, done);

  if (alloc) delete done;
  return memory;
}

long Dumper::TotalTexels (csTextureList* txtlist)
{
  long texels = 0;

  int i;
  for (i = 0 ; i < txtlist->Length () ; i++)
  {
    iTextureHandle* handle = txtlist->Get (i)->GetTextureHandle ();
    int mw, mh;
    if (handle->GetMipMapDimensions (0, mw, mh))
      texels += mw*mh;
    if (handle->GetMipMapDimensions (1, mw, mh))
      texels += mw*mh;
    if (handle->GetMipMapDimensions (2, mw, mh))
      texels += mw*mh;
    if (handle->GetMipMapDimensions (3, mw, mh))
      texels += mw*mh;
  }

  return texels;
}
