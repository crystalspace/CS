/*
    Copyright (C) 2003 by Jorrit Tyberghein, Daniel Duhprey

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

#include "iutil/objreg.h"
#include "iutil/vfs.h"

#include "imesh/terrain.h"

#include "iengine/rview.h"
#include "iengine/camera.h"
#include "iengine/movable.h"

#include "ivideo/graph3d.h"
#include "ivideo/rndbuf.h"
#include "ivideo/txtmgr.h"
#include "ivideo/material.h"
#include "ivideo/texture.h"

#include "igraphic/image.h"

#include "csgfx/rgbpixel.h"
#include "csgfx/memimage.h"

#include "csutil/util.h"

#include "chunklod.h"

#define MIN_TERRAIN 33

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csChunkLodTerrainType)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectType)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csChunkLodTerrainType::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csChunkLodTerrainType)

csChunkLodTerrainType::csChunkLodTerrainType (iBase* p) : parent(p)
{
  SCF_CONSTRUCT_IBASE (parent)
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent)
}

csChunkLodTerrainType::~csChunkLodTerrainType ()
{
}

csPtr<iMeshObjectFactory> csChunkLodTerrainType::NewFactory ()
{
  return csPtr<iMeshObjectFactory> 
	(new csChunkLodTerrainFactory (this, object_reg));
}

SCF_IMPLEMENT_IBASE (csChunkLodTerrainFactory)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectFactory)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iObjectModel)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iTerrainFactoryState)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csChunkLodTerrainFactory::eiObjectModel)
  SCF_IMPLEMENTS_INTERFACE (iObjectModel)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csChunkLodTerrainFactory::eiTerrainFactoryState)
  SCF_IMPLEMENTS_INTERFACE (iTerrainFactoryState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csChunkLodTerrainFactory::csChunkLodTerrainFactory (csChunkLodTerrainType* p, 
	iObjectRegistry* objreg) : parent (p), object_reg (objreg)
{
  SCF_CONSTRUCT_IBASE (p)
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiObjectModel)
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiTerrainFactoryState)

  root = 0;
  hm_x = 0; hm_y = 0;

  r3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  shmgr = CS_QUERY_REGISTRY (object_reg, iShaderManager);

  csRef<iStringSet> strings = 
	CS_QUERY_REGISTRY_TAG_INTERFACE (object_reg,
	"crystalspace.renderer.stringset", iStringSet);
  vertex_name = strings->Request ("vertices");
  texcors_name = strings->Request ("texture coordinates");
  normal_name = strings->Request ("normals");
  tangent_name = strings->Request ("tangents");
  binormal_name = strings->Request ("binormals");
  color_name = strings->Request ("colors");
  index_name = strings->Request ("indices");
}

csChunkLodTerrainFactory::~csChunkLodTerrainFactory ()
{
  if (root) delete root;
}

csPtr<iMeshObject> csChunkLodTerrainFactory::NewInstance ()
{
  return csPtr<iMeshObject>(new csChunkLodTerrainObject (this));
}

void csChunkLodTerrainFactory::GetObjectBoundingBox (csBox3& bbox, int type)
{
  bbox.StartBoundingBox ();
  switch (type) {
  case CS_BBOX_NORMAL:
    bbox.AddBoundingVertex (
	- (hm_x * scale.x / 2),
	FLT_MIN * scale.y,
	- (hm_y * scale.z / 2));
    bbox.AddBoundingVertex (
	hm_x * scale.x / 2,
	FLT_MAX * scale.y,
	hm_y * scale.z / 2);
    break;
  case CS_BBOX_ACCURATE: {
    float max = FLT_MIN;
    float min = FLT_MAX;
    for (int i = 0; i < hm_x; i ++)
      for (int j = 0; j < hm_y; j ++) {
        if (max < datamap[i + j * hm_x].pos.y) 
	  max = datamap[i + j * hm_x].pos.y;
	if (min > datamap[i + j * hm_x].pos.y)
	  min = datamap[i + j * hm_x].pos.y;
      }
    bbox.AddBoundingVertex (
	- (hm_x * scale.x / 2),
	0,
	- (hm_y * scale.z / 2));
    bbox.AddBoundingVertex (
	hm_x * scale.x / 2,
	max * scale.y,
	hm_y * scale.z / 2);
    break;
  }
  case CS_BBOX_MAX:
    bbox.AddBoundingVertex (-FLT_MAX, -FLT_MAX, -FLT_MAX);
    bbox.AddBoundingVertex (FLT_MAX, FLT_MAX, FLT_MAX);
    break;
  }
}

void csChunkLodTerrainFactory::GetRadius (csVector3& rad, csVector3& c)
{
  c = csVector3 (0,0,0);
  rad = scale * csVector3 (hm_x, FLT_MAX, hm_y);
}

void csChunkLodTerrainFactory::SetScale (const csVector3& s)
{
  scale = s;
}

csVector3 csChunkLodTerrainFactory::GetScale ()
{
  return scale;
}

void csChunkLodTerrainFactory::ComputeError (int i, int j, int di, int dj, int n, int w)
{
  Data *b = &datamap[i + j * w];
  Data *l = &datamap[i - di + (j - dj) * w];
  Data *r = &datamap[i + di + (j + dj) * w];
  b->error = (float)fabs(b->pos.y - (l->pos.y + r->pos.y) / 2.0);
  if (n)
  {
    dj = (di + dj) / 2;
    di -= dj;
    for (int k = 0; k < 4; k ++)
    {
      if ((i > 0 || di >= 0) && (i < (w-1) || di <= 0) &&
          (j > 0 || dj >= 0) && (j < (w-1) || dj <= 0))
      {
        Data *cp = &datamap[i + di + (j + dj) * w];
        b->error = (b->error > cp->error) ? b->error : cp->error;
      }
      dj += di;
      di -= dj;
      dj += di;
    }
  }
}

bool csChunkLodTerrainFactory::SetHeightMap (const csArray<float>& data, int w, int h)
{
  CS_ASSERT (w == h);
  CS_ASSERT (w >= MIN_TERRAIN);
  datamap.SetLength (w * h);
  hm_x = w; hm_y = h;
  int i, j;
  for (j = 0; j < h; j ++) {
    for (i = 0; i < w; i ++) {
      int pos = i + j * w;

      datamap[pos].pos.x = (i - (w>>1)) * scale.x;
      datamap[pos].pos.y = data[pos] * scale.y;
      datamap[pos].pos.z = ((w>>1) - j) * scale.z;

      float up = (j-1 < 0) ? data[pos] : data[pos-w];
      float dn = (j+1 >= w) ? data[pos] : data[pos+w];
      float lt = (i-1 < 0) ? data[pos] : data[pos-1];
      float rt = (i+1 >= h) ? data[pos] : data[pos+1];

      datamap[pos].norm = csVector3 (lt - rt, 2.0/(float)w + 2.0/(float)h, dn - up);
      datamap[pos].norm.Normalize ();
      datamap[pos].tan = csVector3 (0,0,1) % datamap[pos].norm;
      datamap[pos].tan.Normalize ();
      datamap[pos].bin = datamap[pos].norm % datamap[pos].tan;
      datamap[pos].bin.Normalize ();

      datamap[pos].tex = csVector2 (i, j);

      datamap[pos].col = csColor (1, 1, 1);
    }
  }
  int a, b, c, s;
  for (a = c = 1, b = 2, s = 0; a != w-1; a = c = b, b *= 2, s = w)
  {
    for (j = a; j < w-1; j += b)
    {
      for (i = 0; i < w; i += b)
      {
        ComputeError (i, j, 0, a, s, w);
        ComputeError (j, i, a, 0, s, w);
      }
    }

    for (j = a; j < w-1; c = -c, j += b)
    {
      for (i = a; i < w-1; c = -c, i += b)
      {
        ComputeError (i, j, a, c, w, w);
      }
    }
  }

  int max_error = csLog2 (w - 1) - csLog2 (MIN_TERRAIN);
  float error = pow (2.0, max_error);

  root = new MeshTreeNode (this, 0, 0, w, h, error);
  return true;
}

bool csChunkLodTerrainFactory::SetHeightMap (iImage* map)
{
  csArray<float> image_data;
  image_data.SetLength (map->GetSize());
  if (map->GetFormat () & CS_IMGFMT_PALETTED8)
  {
    csRGBpixel *palette = map->GetPalette ();
    uint8 *data = (uint8 *)map->GetImageData ();
    for (int i = 0; i < map->GetSize (); i ++)
    {
      image_data[i] = ((float)palette[data[i]].Intensity()) / 255.0;
    }
  }
  else
  {
    csRGBpixel *data = (csRGBpixel *)map->GetImageData ();
    for (int i = 0; i < map->GetSize (); i ++)
    {
      image_data[i] = ((float)data[i].Intensity()) / 255.0;
    }
  }
  return SetHeightMap (image_data, map->GetWidth(), map->GetHeight());
}

csArray<float> csChunkLodTerrainFactory::GetHeightMap ()
{
  csArray<float> hm;
  return hm;
}

bool csChunkLodTerrainFactory::SaveState (const char *filename)
{
  return false;
}

bool csChunkLodTerrainFactory::RestoreState (const char *filename)
{
  return false;
}

SCF_IMPLEMENT_IBASE (csChunkLodTerrainFactory::MeshTreeNode)
  SCF_IMPLEMENTS_INTERFACE (iRenderBufferSource)
SCF_IMPLEMENT_IBASE_END

csChunkLodTerrainFactory::MeshTreeNode::MeshTreeNode (
	csChunkLodTerrainFactory* p, int x, int y, int w, int h, float e)
{
  SCF_CONSTRUCT_IBASE (p)

  pFactory = p;
  vertex_buffer = 0;
  normal_buffer = 0;
  texcors_buffer = 0;
  color_buffer = 0;
  index_buffer = 0;

  error = (e < 1) ? 0.0 : e;
  int mid_w = w>>1, mid_h = h>>1;
  radius = sqrt (mid_w*mid_w + mid_h*mid_h);
  int c = x+mid_w + (y+mid_w) * p->hm_x;
  center = p->datamap[c].pos;
  int sw = x + (y+h-1) * p->hm_x;
  int se = x+w-1 + (y+h-1) * p->hm_x;
  int ne = x+w-1 + y * p->hm_x;
  int nw = x + y * p->hm_x;

  max_levels = 2 * csLog2(w) - 1;
  InitBuffer (p->datamap[sw], 0);
  ProcessMap (0, c, sw, se);
  AddVertex (p->datamap[se], 1);
  ProcessMap (0, c, se, ne);
  AddVertex (p->datamap[ne], 1);
  ProcessMap (0, c, ne, nw);
  AddVertex (p->datamap[nw], 1);
  ProcessMap (0, c, nw, sw);
  AddVertex (p->datamap[sw], 1);

  Data south, east, north, west;
  south.pos = csVector3 (0,-error-1,0);
  south.norm = csVector3 (0,0,-1);
  south.tan = csVector3 (1,0,0);
  south.bin = csVector3 (0,-1,0);
  south.tex = csVector2 (0,error+1);

  east.pos = csVector3 (0,-error-1,0);
  east.norm = csVector3 (1,0,0);
  east.tan = csVector3 (0,0,1);
  east.bin = csVector3 (0,-1,0);
  east.tex = csVector2 (error+1,0);

  north.pos = csVector3 (0,-error-1,0);
  north.norm = csVector3 (0,0,-1);
  north.tan = csVector3 (-1,0,0);
  north.bin = csVector3 (0,-1,0);
  north.tex = csVector2 (0,-error-1);

  west.pos = csVector3 (0,-error-1,0);
  west.norm = csVector3 (-1,0,0);
  west.tan = csVector3 (0,0,-1);
  west.bin = csVector3 (0,-1,0);
  west.tex = csVector2 (-error-1,0);

  Data southwest;
  southwest.pos = csVector3 (0,-error-1,0);
  southwest.norm = csVector3 (-.707,0,-.707);
  southwest.tan = csVector3 (.707,0,-.707);
  southwest.bin = csVector3 (0,-1,0);
  southwest.tex = csVector2 (-error-1,error+1);
  AddEdgeVertex (p->datamap[sw]);
  AddSkirtVertex (p->datamap[sw], southwest);

  // bottom edge
  ProcessEdge (sw+1, se-1, 1, south);

  Data southeast;
  southeast.pos = csVector3 (0,-error-1,0);
  southeast.norm = csVector3 (.707,0,-.707);
  southeast.tan = csVector3 (.707,0,.707);
  southeast.bin = csVector3 (0,-1,0);
  southeast.tex = csVector2 (error+1,error+1);
  AddEdgeVertex (p->datamap[se]);
  AddSkirtVertex (p->datamap[se], southeast);

  // right edge
  ProcessEdge (se-p->hm_x, ne+p->hm_x, -p->hm_x, east);

  Data northeast;
  northeast.pos = csVector3 (0,-error-1,0);
  northeast.norm = csVector3 (.707,0,.707);
  northeast.tan = csVector3 (-.707,0,.707);
  northeast.bin = csVector3 (0,-1,0);
  northeast.tex = csVector2 (error+1,-error-1);
  AddEdgeVertex (p->datamap[ne]);
  AddSkirtVertex (p->datamap[ne], northeast);

  // top edge
  ProcessEdge (ne-1, nw+1, -1, north);

  Data northwest;
  northwest.pos = csVector3 (0,-error-1,0);
  northwest.norm = csVector3 (-.707,0,.707);
  northwest.tan = csVector3 (-.707,0,-.707);
  northwest.bin = csVector3 (0,-1,0);
  northwest.tex = csVector2 (-error-1,-error-1);
  AddEdgeVertex (p->datamap[nw]);
  AddSkirtVertex (p->datamap[nw], northwest);

  // left edge
  ProcessEdge (nw+p->hm_x, sw-p->hm_x, p->hm_x, west);

  AddEdgeVertex (p->datamap[sw]);
  AddSkirtVertex (p->datamap[sw], southwest);

  if (error > 0.0) {
    float new_error = error / 2.0;
    int new_w = mid_w + 1;
    int new_h = mid_h + 1;
    children[0] = new MeshTreeNode (p, x, y, new_w, new_h, new_error);
    children[1] = new MeshTreeNode (p, x+new_w-1, y, new_w, new_h, new_error);
    children[2] = new MeshTreeNode (p, x, y+new_h-1, new_w, new_h, new_error);
    children[3] = new MeshTreeNode (p, x+new_w-1, y+new_h-1, new_w, new_h, new_error);
  } else {
    children[0] = 0;
    children[1] = 0;
    children[2] = 0;
    children[3] = 0;
  }

}

csChunkLodTerrainFactory::MeshTreeNode::~MeshTreeNode ()
{
  if (error > 0) {
    delete children[0];
    delete children[1];
    delete children[2];
    delete children[3];
  }
}

iRenderBuffer *csChunkLodTerrainFactory::MeshTreeNode::GetRenderBuffer (
	csStringID name)
{
  if (name == pFactory->vertex_name) 
  {
    if (!vertex_buffer) 
    {
      int len = vertices.Length();
      vertex_buffer = pFactory->r3d->CreateRenderBuffer (
	sizeof (csVector3) * len, CS_BUF_STATIC, 
	CS_BUFCOMP_FLOAT, 3, false);
      csVector3 *vbuf = (csVector3*)vertex_buffer->Lock (CS_BUF_LOCK_NORMAL);
      memcpy (vbuf, &vertices[0], len * sizeof (csVector3));
      vertex_buffer->Release ();
    }
    return vertex_buffer;
  }
  if (name == pFactory->normal_name) 
  {
    if (!normal_buffer)
    {
      int len = normals.Length();
      normal_buffer = pFactory->r3d->CreateRenderBuffer (
	sizeof (csVector3) * len, CS_BUF_STATIC, 
	CS_BUFCOMP_FLOAT, 3, false);
   
      csVector3 *nbuf = (csVector3*)normal_buffer->Lock (CS_BUF_LOCK_NORMAL);
      memcpy (nbuf, &normals[0], len * sizeof (csVector3));
      normal_buffer->Release ();
    }
    return normal_buffer;
  }
  if (name == pFactory->tangent_name) 
  {
    if (!tangent_buffer)
    {
      int len = tangents.Length();
      tangent_buffer = pFactory->r3d->CreateRenderBuffer (
	sizeof (csVector3) * len, CS_BUF_STATIC, 
	CS_BUFCOMP_FLOAT, 3, false);
   
      csVector3 *tbuf = (csVector3*)tangent_buffer->Lock (CS_BUF_LOCK_NORMAL);
      memcpy (tbuf, &tangents[0], len * sizeof (csVector3));
      tangent_buffer->Release ();
    }
    return tangent_buffer;
  }
  if (name == pFactory->binormal_name) 
  {
    if (!binormal_buffer)
    {
      int len = binormals.Length();
      binormal_buffer = pFactory->r3d->CreateRenderBuffer (
	sizeof (csVector3) * len, CS_BUF_STATIC, 
	CS_BUFCOMP_FLOAT, 3, false);
   
      csVector3 *bbuf = (csVector3*)binormal_buffer->Lock (CS_BUF_LOCK_NORMAL);
      memcpy (bbuf, &binormals[0], len * sizeof (csVector3));
      binormal_buffer->Release ();
    }
    return binormal_buffer;
  }
  if (name == pFactory->texcors_name) 
  {
    if (!texcors_buffer)
    {
      int len = texcors.Length();
      texcors_buffer = pFactory->r3d->CreateRenderBuffer (
	sizeof (csVector2) * len, CS_BUF_STATIC, 
	CS_BUFCOMP_FLOAT, 2, false);

      csVector2 *tbuf = (csVector2*)texcors_buffer->Lock (CS_BUF_LOCK_NORMAL);
      memcpy (tbuf, &texcors[0], len * sizeof (csVector2));
      texcors_buffer->Release ();

    }
    return texcors_buffer;
  }
  if (name == pFactory->color_name) 
  {
    if (!color_buffer)
    {
      int len = colors.Length();
      color_buffer = pFactory->r3d->CreateRenderBuffer (
	sizeof (csColor) * len, CS_BUF_STATIC, 
	CS_BUFCOMP_FLOAT, 3, false);

      csColor *cbuf = (csColor*)color_buffer->Lock (CS_BUF_LOCK_NORMAL);
      memcpy (cbuf, &colors[0], len * sizeof (csColor));
      color_buffer->Release ();
    }
    return color_buffer;
  }
  if (name == pFactory->index_name) 
  {
    if (!index_buffer)
    {
      int len = vertices.Length();
      index_buffer = pFactory->r3d->CreateRenderBuffer (
	sizeof (unsigned int) * len, CS_BUF_STATIC, 
	CS_BUFCOMP_UNSIGNED_INT, 1, true);

      unsigned int *ibuf = (unsigned int *)index_buffer->Lock (CS_BUF_LOCK_NORMAL);
      for (int i = 0; i < len; i ++) {
        ibuf[i] = i;
      }
      index_buffer->Release ();
    }
    return index_buffer;
  }
  return 0;
}

void csChunkLodTerrainFactory::MeshTreeNode::InitBuffer (const Data& d, int p)
{
  vertices.Push(d.pos); vertices.Push(d.pos);
  normals.Push(d.norm); normals.Push(d.norm);
  tangents.Push(d.tan); tangents.Push(d.tan);
  binormals.Push(d.bin); binormals.Push(d.bin);
  texcors.Push(d.tex); texcors.Push(d.tex);
  colors.Push(d.col); colors.Push(d.col);
  parity = p;
}

void csChunkLodTerrainFactory::MeshTreeNode::AddVertex (const Data& d, int p)
{
  int len = vertices.Length ();
  if (d.pos == vertices[len - 1] || d.pos == vertices[len - 2])
    return;
  if (p == parity)
  {
    csVector3 v = vertices[len - 2];
    vertices.Push(v);
    v = normals[len - 2];
    normals.Push(v);
    v = tangents[len - 2];
    tangents.Push(v);
    v = binormals[len - 2];
    binormals.Push(v);
    csVector2 t = texcors[len - 2];
    texcors.Push(t);
    csColor c = colors[len - 2];
    colors.Push(c);
  }
  vertices.Push(d.pos);
  normals.Push(d.norm);
  tangents.Push(d.tan);
  binormals.Push(d.bin);
  texcors.Push(d.tex);
  colors.Push(d.col);
  parity = p;
}

void csChunkLodTerrainFactory::MeshTreeNode::AddEdgeVertex (const Data& d)
{
  vertices.Push(d.pos);
  normals.Push(d.norm);
  tangents.Push(d.tan);
  binormals.Push(d.bin);
  texcors.Push(d.tex);
  colors.Push(d.col);
}

void csChunkLodTerrainFactory::MeshTreeNode::AddSkirtVertex (const Data& d, const Data& m)
{
  vertices.Push(d.pos+m.pos);
  normals.Push(d.norm);
  tangents.Push(d.tan);
  binormals.Push(d.bin);
  texcors.Push(d.tex+m.tex);
  colors.Push(d.col);
}


void csChunkLodTerrainFactory::MeshTreeNode::ProcessMap (int l, int i, int j, 
	int k)
{
  int child = (j+k)>>1;
  if (l == 0 || pFactory->datamap[i].error > error)
  {
    if (l < max_levels)
      ProcessMap (l+1, child, j, i);
    AddVertex (pFactory->datamap[i], l & 1);
    if (l < max_levels)
      ProcessMap (l+1, child, i, k);
  }
}

void csChunkLodTerrainFactory::MeshTreeNode::ProcessEdge (int start, int end, 
	int move, const Data& mod)
{
  while (start != end) 
  {
    if (pFactory->datamap[start].error > error)
    {
      AddEdgeVertex (pFactory->datamap[start]);
      AddSkirtVertex (pFactory->datamap[start], mod);
    }
    start += move;
  }
}

SCF_IMPLEMENT_IBASE (csChunkLodTerrainObject)
  SCF_IMPLEMENTS_INTERFACE (iMeshObject)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iTerrainObjectState)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iObjectModel)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csChunkLodTerrainObject::eiTerrainObjectState)
  SCF_IMPLEMENTS_INTERFACE (iTerrainObjectState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csChunkLodTerrainObject::eiObjectModel)
  SCF_IMPLEMENTS_INTERFACE (iObjectModel)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csChunkLodTerrainObject::csChunkLodTerrainObject (csChunkLodTerrainFactory* p)
	: logparent (p), pFactory (p)
{
  SCF_CONSTRUCT_IBASE (p)
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiTerrainObjectState)
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiObjectModel)

  meshpp = 0;
  meshppsize = 0;
}

csChunkLodTerrainObject::~csChunkLodTerrainObject ()
{
  if (meshpp)
    delete [] meshpp;
}

bool csChunkLodTerrainObject::DrawTestQuad (iRenderView* rv, 
	csChunkLodTerrainFactory::MeshTreeNode* node, float kappa) 
{
  int clip_portal, clip_plane, clip_z_plane;
  csSphere s(node->Center (), node->Radius ());
  if (!rv->ClipBSphere (tr_o2c, s, clip_portal, clip_plane, clip_z_plane))
    return false;
  float sq_dist = (tr_o2c * node->Center()).SquaredNorm ();
  float error_projection = node->Error() / kappa + node->Radius();
  error_projection *= error_projection;
  if (error_projection > sq_dist && node->GetChild(0) != 0)
  {
    DrawTestQuad (rv, node->GetChild (0), kappa);
    DrawTestQuad (rv, node->GetChild (1), kappa);
    DrawTestQuad (rv, node->GetChild (2), kappa);
    DrawTestQuad (rv, node->GetChild (3), kappa);
  } 
  else 
  {
    int len = meshes.Length();
    meshes.GetExtend(len).object2camera = tr_o2c;
    meshes[len].clip_portal = clip_portal;
    meshes[len].clip_plane = clip_plane;
    meshes[len].clip_z_plane = clip_z_plane;
    meshes[len].do_mirror = rv->GetCamera()->IsMirrored();
    matwrap->Visit ();
    meshes[len].material = matwrap;
    meshes[len].z_buf_mode = CS_ZBUF_TEST;
    meshes[len].mixmode = CS_FX_COPY;
    meshes[len].buffersource = node;
    meshes[len].indexstart = 0;
    meshes[len].indexend = node->Count ();
    meshes[len].meshtype = CS_MESHTYPE_TRIANGLESTRIP;
    // meshes[len].meshtype = CS_MESHTYPE_LINESTRIP;
    for (int i = 0; i < palette.Length(); i ++)
    {
      int len = palette_meshes[i].Length();
      palette_meshes[i].GetExtend(len).object2camera = tr_o2c;
      palette_meshes[i][len].clip_portal = clip_portal;
      palette_meshes[i][len].clip_plane = clip_plane;
      palette_meshes[i][len].clip_z_plane = clip_z_plane;
      palette_meshes[i][len].do_mirror = rv->GetCamera()->IsMirrored();
      palette[i]->Visit ();
      palette_meshes[i][len].material = palette[i];
      palette_meshes[i][len].z_buf_mode = CS_ZBUF_TEST;
      palette_meshes[i][len].mixmode = CS_FX_COPY;
      palette_meshes[i][len].buffersource = node;
      palette_meshes[i][len].indexstart = 0;
      palette_meshes[i][len].indexend = node->Count ();
      palette_meshes[i][len].meshtype = CS_MESHTYPE_TRIANGLESTRIP;
      // palette_meshes[i][len].meshtype = CS_MESHTYPE_LINESTRIP;
    }
    tricount += node->Count () - 2;
  }
  return true;
}

bool csChunkLodTerrainObject::DrawTest (iRenderView* rview, iMovable* movable)
{
  iCamera* cam = rview->GetCamera ();
  tr_o2c = cam->GetTransform ();
  if (!movable->IsFullTransformIdentity ())
    tr_o2c /= movable->GetFullTransform ();

  float kappa = error_tolerance * cam->GetInvFOV() * 
	2*tan(cam->GetFOVAngle()/180.0*PI/2);
  meshes.SetLength (0);
  for (int i = 0; i < palette_meshes.Length (); i ++) 
  {
    palette_meshes[i].SetLength(0);
  }
  tricount = 0;
  if (!DrawTestQuad (rview, pFactory->root, kappa)) 
    return false;
  if (meshes.Length () == 0)
    return false;
  scfiObjectModel.ShapeChanged ();
  return true;
}

void csChunkLodTerrainObject::UpdateLighting (iLight**, int, iMovable*)
{
}

csRenderMesh** csChunkLodTerrainObject::GetRenderMeshes (int &n)
{
  int i;
  n = meshes.Length();
  for (i = 0; i < palette_meshes.Length(); i ++)
  {
    n += palette_meshes[i].Length();
  }
  if (n == 0) 
  {
    // pass back root node as default always
    meshes.GetExtend(0).z_buf_mode = CS_ZBUF_TEST;
    meshes[0].mixmode = CS_FX_COPY;
    meshes[0].buffersource = pFactory->root;
    meshes[0].indexstart = 0;
    meshes[0].indexend = pFactory->root->Count ();
    meshes[0].meshtype = CS_MESHTYPE_TRIANGLESTRIP;
    n ++;
  }

  if (n > meshppsize)
  {
    if (meshpp)
      delete [] meshpp;
    meshpp = new csRenderMesh*[n];
    meshppsize = n;
  }

  int index = 0;
  for (i = 0; i < meshes.Length(); i ++) 
  {
    meshpp[index++] = &meshes[i];
  }
  for (i = 0; i < palette_meshes.Length(); i ++)
  {
    for (int j = 0; j < palette_meshes[i].Length(); j++)
    {
      meshpp[index++] = &palette_meshes[i][j];
    }
  }
  return meshpp;
}

bool csChunkLodTerrainObject::HitBeamOutline (const csVector3& start, 
	const csVector3& end, csVector3& isect, float* pr)
{
printf ("ChunkLOD: HitBeamOutline called, but not implemented\n");
  return false;
}

bool csChunkLodTerrainObject::HitBeamObject (const csVector3& start,
	const csVector3& end, csVector3& isect, float* pr,
	int* polygon_idx)
{
  if (polygon_idx) *polygon_idx = -1;
printf ("ChunkLOD: HitBeamObject called, but not implemented\n");
  return false;
}

bool csChunkLodTerrainObject::SetMaterialPalette (
	const csArray<iMaterialWrapper*>& pal)
{
  palette.SetLength (pal.Length());
  for (int i = 0; i < pal.Length(); i++) {
    palette[i] = pal[i];
  }
  palette_meshes.SetLength (pal.Length());
  return true;
}

csArray<iMaterialWrapper*> csChunkLodTerrainObject::GetMaterialPalette ()
{
  return palette;
}

bool csChunkLodTerrainObject::SetMaterialMap (csArray<char> data, int w, int h)
{
  csRef<iStringSet> strings = 
	CS_QUERY_REGISTRY_TAG_INTERFACE (pFactory->object_reg,
	"crystalspace.renderer.stringset", iStringSet);
  for (int i = 0; i < palette.Length(); i ++) 
  {
    csRef<iImage> alpha = csPtr<iImage> (new csImageMemory (w, h, 
	CS_IMGFMT_ALPHA | CS_IMGFMT_TRUECOLOR));

    csRGBpixel *map = (csRGBpixel *)alpha->GetImageData ();
    for (int y = 0; y < h; y ++) 
    {
      for (int x = 0; x < w; x ++) 
      {
        map[x + y * w].red = (data[x + y * w] == i) ? 255 : 0;
        map[x + y * w].green = (data[x + y * w] == i) ? 255 : 0;
        map[x + y * w].blue = (data[x + y * w] == i) ? 255 : 0;
        map[x + y * w].alpha = (data[x + y * w] == i) ? 255 : 0;
      }
    }
    csRef<iTextureManager> mgr = pFactory->r3d->GetTextureManager ();
    csRef<iTextureHandle> hdl = mgr->RegisterTexture (alpha, CS_TEXTURE_2D);
    csRef<csShaderVariable> var = pFactory->shmgr->CreateVariable (
	strings->Request ("splat alpha map"));
    var->SetType (csShaderVariable::TEXTURE);
    var->SetValue (hdl);
    palette[i]->GetMaterial()->AddVariable (var);
    var = pFactory->shmgr->CreateVariable (strings->Request ("splat map scale"));
    var->SetType (csShaderVariable::VECTOR2);
    var->SetValue (csVector2 (1.0 / (float)pFactory->hm_x, 1.0 / (float)pFactory->hm_y));
    palette[i]->GetMaterial()->AddVariable (var);
  }
  return true;
}

bool csChunkLodTerrainObject::SetMaterialMap (iImage* map)
{
  csArray<char> image_data;
  image_data.SetLength (map->GetSize());
  if (map->GetFormat () & CS_IMGFMT_PALETTED8)
  {
    uint8 *data = (uint8 *)map->GetImageData ();
    for (int i = 0; i < map->GetSize (); i ++)
    {
      image_data[i] = data[i];
    }
  }
  else
  {
    csRGBpixel *data = (csRGBpixel *)map->GetImageData ();
    for (int i = 0; i < map->GetSize (); i ++)
    {
      image_data[i] = data[i].Intensity();
    }
  }
  return SetMaterialMap (image_data, map->GetWidth(), map->GetHeight());
}

csArray<char> csChunkLodTerrainObject::GetMaterialMap ()
{
  return 0;
}

bool csChunkLodTerrainObject::SaveState (const char *filename)
{
  return false;
}

bool csChunkLodTerrainObject::RestoreState (const char *filename)
{
  return false;
}
