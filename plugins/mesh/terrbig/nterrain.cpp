#define CS_SYSDEF_PROVIDE_HARDWARE_MMIO 1

#include "cssysdef.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/polyclip.h"
#include "csgeom/poly2d.h"
#include "csgeom/vector3.h"
#include "csutil/garray.h"
#include "csutil/util.h"
#include "iengine/camera.h"
#include "iengine/engine.h"
#include "ivideo/graph3d.h"
#include "ivideo/material.h"
#include "ivideo/vbufmgr.h"
#include "iengine/material.h"
#include "iengine/rview.h"
#include "ivideo/txtmgr.h"
#include "igraphic/image.h"
#include "igraphic/imageio.h"
#include "iutil/objreg.h"
#include "csgfx/rgbpixel.h"
#include "imesh/object.h"
#include "csutil/mmapio.h"
#include "ivaria/reporter.h"
#include "iengine/light.h"
#include "iengine/movable.h"
#include "qsqrt.h"
#include "nterrain.h"


///////////////////////////// SCF stuff ///////////////////////////////////////////////////////////////////////
CS_IMPLEMENT_PLUGIN

nTerrainInfo::nTerrainInfo (iObjectRegistry *obj_reg)
{
  mesh = new G3DTriangleMesh;
  mesh->vertex_fog = 0;
  mG3D = CS_QUERY_REGISTRY (obj_reg, iGraphics3D);
  if (mG3D)
  {
  // @@@ priority should be a parameter.
    vbufmgr = mG3D->GetVertexBufferManager ();
    vbuf = vbufmgr->CreateBuffer (1);
  }
  bufcount = 0;
  triangles = 0;
  triangle_count = triangle_size = 0;
  triangle_parity = true;
  vertices = 0;
  texels = 0;
  colors = 0;
  vertex_count = vertex_size = 0;
  parity = 0;
}

nTerrainInfo::~nTerrainInfo ()
{
  delete[] mesh->vertex_fog;
  delete mesh;
  delete [] triangles;
  delete [] vertices;
  delete [] texels;
  delete [] colors;
}

void nTerrainInfo::InitBuffer (const csVector3 &v, const csVector2 &t, const csColor &c, int p)
{
  if (vertex_size == 0)
  {
    vertices = new csVector3[2];
    texels = new csVector2[2];
    colors = new csColor[2];
    vertex_size = 2;
  }
  else if (mG3D)
  {
    delete [] mesh->vertex_fog;
    vbufmgr->UnlockBuffer(vbuf);
	bufcount ++;
  }
  triangle_count = 0;
  triangle_parity = false;
  vertex_count = 0;
  vertices[0] = vertices[1] = v;
  texels[0] = texels[1] = t;
  colors[0] = colors[1] = c;
  vertex_count = 2;
  parity = p;
}

void nTerrainInfo::AddVertex (const csVector3 &v, const csVector2 &t, const csColor &c, int p)
{
  CS_ASSERT (vertex_size >= 2);
  if (v == vertices[vertex_count - 1] || v == vertices[vertex_count - 2])
  {
    return;
  }
  if (vertex_count+1 >= vertex_size)
  {
    ResizeVertices ();
  }
  if (p == parity)
  {
    vertices[vertex_count] = vertices[vertex_count - 2];
    texels[vertex_count] = texels[vertex_count - 2];
    colors[vertex_count] = colors[vertex_count - 2];
    vertex_count ++;
    AddTriangle ();
  }
  vertices[vertex_count] = v;
  texels[vertex_count] = t;
  colors[vertex_count] = c;
  parity = p;
  vertex_count ++;
  AddTriangle ();
}

void nTerrainInfo::EndBuffer (const csVector3 &v, const csVector2 &t, const csColor &c, iRenderView *rview, const csBox3& bbox)
{
  if (vertex_count >= vertex_size)
  {
    ResizeVertices ();
  }
  vertices[vertex_count] = v;
  texels[vertex_count] = t;
  colors[vertex_count] = c;
  vertex_count ++;
  AddTriangle ();

  if (mG3D)
  {
    vbufmgr->LockBuffer(vbuf, vertices, texels, colors, vertex_count, bufcount,
      bbox);
    mesh->triangles = triangles;
    mesh->vertex_fog = new G3DFogInfo[vertex_count]; 
    mesh->buffers[0]=vbuf;
    mesh->num_triangles = triangle_count;
    rview->CalculateFogMesh(mG3D->GetObjectToCamera(), *mesh);
  }
}

void nTerrainInfo::AddTriangle ()
{
  if (triangle_count == triangle_size)
  {
    if (triangle_size == 0)
    {
      triangle_size = 1;
      triangles = new csTriangle[triangle_size];
    }
    else
    {
      triangle_size <<= 1;
      csTriangle *ttmp = triangles;
      triangles = new csTriangle[triangle_size];
      for (int i = 0; i < triangle_count; i++)
      {
        triangles[i] = ttmp[i];
      }
      delete [] ttmp;
    }
  }
  if (triangle_parity)
  { // go counter-clockwise
    triangles[triangle_count++] = csTriangle (vertex_count - 1,
      vertex_count - 2, vertex_count - 3);
  }
  else
  { // go clockwise
    triangles[triangle_count++] = csTriangle (vertex_count - 3,
      vertex_count - 2, vertex_count - 1);
  }
  triangle_parity = !triangle_parity;
}

void nTerrainInfo::ResizeVertices ()
{
  CS_ASSERT (vertex_size >= 2);
  vertex_size <<= 1;
  csVector3 *vtmp = vertices;
  csVector2 *ttmp = texels;
  csColor *ctmp = colors;
  vertices = new csVector3[vertex_size];
  texels = new csVector2[vertex_size];
  colors = new csColor[vertex_size];
  for (int i = 0; i < vertex_count; i ++) {
    vertices[i] = vtmp[i];
    texels[i] = ttmp[i];
    colors[i] = ctmp[i];
  }
  delete [] vtmp;
  delete [] ttmp;
  delete [] ctmp;
}

void nTerrain::VerifyTreeNode(FILE *f, unsigned int level,
	unsigned int parent, unsigned int my, 
	unsigned int i, unsigned int j, unsigned int k,
	nBlock *heightmap)
{
  unsigned int my_ind = (j+k)>>1;
  nBlock *b = &heightmap[my_ind];

  if (level < 2 * max_levels - 1)
  {
    unsigned int right_ind = my * 2 + 1;
    VerifyTreeNode (f, level+1, parent, right_ind, my_ind, i, k, heightmap);
    if (b->error < heightmap[(i+k)>>1].error)
    {
      printf ("INVALID error between heightmap[%d].error = %f and hm[%d].error = %f\n", my_ind, b->error, right_ind, heightmap[right_ind].error);
    }
    if (b->radius < heightmap[(i+k)>>1].radius)
    {
      printf ("INVALID radius between heightmap[%d].radius = %f and hm[%d].radius = %f\n", my_ind, b->radius, right_ind, heightmap[right_ind].radius);
    }
    unsigned int left_ind = my * 2 + 0;
    VerifyTreeNode(f, level+1, parent, left_ind, my_ind, j, i, heightmap);
    if (b->error < heightmap[(j+i)>>1].error)
    {
      printf ("INVALID error between heightmap[%d].error = %f and hm[%d].error = %f\n", my_ind, b->error, left_ind, heightmap[left_ind].error);
    }
    if (b->radius < heightmap[(j+i)>>1].radius)
    {
      printf ("INVALID radius between heightmap[%d].radius = %f and hm[%d].radius = %f\n", my_ind, b->radius, left_ind, heightmap[left_ind].radius);
    }
  }
}

void nTerrain::WriteTreeNode(FILE *f, unsigned int level, unsigned int my, 
	unsigned int i, unsigned int j, unsigned int k,
	nBlock *heightmap, nBlock **storage)
{
  unsigned int my_ind = (j+k)>>1;
  storage[my] = &heightmap[my_ind];

  // Store the block in the file.
  // fseek(f, (parent + my)*sizeof(nBlock), SEEK_SET);
  // fwrite(b, sizeof(nBlock), 1, f);

  if (level < 2 * max_levels - 1)
  {
    unsigned int left_ind = my * 2 + 0;
    WriteTreeNode(f, level+1, left_ind, my_ind, j, i, heightmap, storage);
    unsigned int right_ind = my * 2 + 1;
    WriteTreeNode (f, level+1, right_ind, my_ind, i, k, heightmap, storage);
  }
}

void nTerrain::BuildTree(FILE *f, nBlock *heightmap, unsigned int w)
{
  terrain_w = w;
  max_levels = ilogb(w-1);
  fseek (f, 0, SEEK_SET);
  unsigned int SW = (w-1)*w;
  unsigned int SE = (w-1)+(w-1)*w;
  unsigned int NE = w-1;
  unsigned int NW = 0;
  unsigned int x = w >> 1;
  unsigned int C = x + x*w;

  fwrite (&heightmap[SW], sizeof (nBlock), 1, f);
  fwrite (&heightmap[SE], sizeof (nBlock), 1, f);
  fwrite (&heightmap[NE], sizeof (nBlock), 1, f);
  fwrite (&heightmap[NW], sizeof (nBlock), 1, f);
  fwrite (&heightmap[C], sizeof (nBlock), 1, f);
  unsigned int size = 0;
  unsigned int i, inc = 1;
  for (i = 0; i < 2 * max_levels - 1; i ++)
  {
    size += inc;
    inc *= 2;
  }
  size ++;

  nBlock **storage = new nBlock*[size];
  WriteTreeNode(f, 1, 1, C, SW, SE, heightmap, storage); 
  for (i = 1; i < size; i ++) {
    fwrite (storage[i], sizeof (nBlock), 1, f);
  }
  WriteTreeNode(f, 1, 1, C, SE, NE, heightmap, storage); 
  for (i = 1; i < size; i ++) {
    fwrite (storage[i], sizeof (nBlock), 1, f);
  }
  WriteTreeNode(f, 1, 1, C, NE, NW, heightmap, storage); 
  for (i = 1; i < size; i ++) {
    fwrite (storage[i], sizeof (nBlock), 1, f);
  }
  WriteTreeNode(f, 1, 1, C, NW, SW, heightmap, storage); 
  for (i = 1; i < size; i ++) {
    fwrite (storage[i], sizeof (nBlock), 1, f);
  }
  delete [] storage;

  /*
  VerifyTreeNode(f, 1, 0 * size + 4, 1, C, SW, SE, heightmap); 
  VerifyTreeNode(f, 1, 1 * size + 4, 1, C, SE, NE, heightmap); 
  VerifyTreeNode(f, 1, 2 * size + 4, 1, C, NE, NW, heightmap); 
  VerifyTreeNode(f, 1, 3 * size + 4, 1, C, NW, SW, heightmap); 
  */
}

csColor nTerrain::CalculateLightIntensity (iLight *li, csVector3 v, csVector3 n)
{
  csColor color (0.0, 0.0, 0.0);
  csVector3 li_center = movable->GetTransform().Other2This (li->GetCenter());
  csVector3 light_dir = v - li_center;
  float sq_dist = light_dir.SquaredNorm();
  if (sq_dist < li->GetInfluenceRadiusSq ())
  {
    
    csColor light_color = li->GetColor () * (256.0 / CS_NORMAL_LIGHT_LEVEL);
      // * li->GetBrightnessAtDistance (qsqrt (sq_dist));

    float cosinus;
    if (sq_dist < SMALL_EPSILON) 
      cosinus = 1.0;
    else
      cosinus = light_dir * n;

    if (cosinus > 0)
    {
      if (sq_dist >= SMALL_EPSILON) 
        cosinus *= qisqrt (sq_dist);
      if (cosinus < 1)
        color += light_color * cosinus;
      else
        color += light_color;
    }
  } 
  return color;
}

void nTerrain::BufferTreeNode(int p, nBlock *b)
{
  csVector3 v = b->pos;
  float mid = terrain_w / 2.0;
  // csVector2 t = csVector2 ((b->pos.x + mid) / (float)terrain_w, 1.0 - (b->pos.z + mid) / (float)terrain_w);
  csVector2 t = csVector2 ((b->pos.x + mid)/4, (float)terrain_w - (b->pos.z + mid)/4);
  csColor c = csColor (1.0, 1.0, 1.0);
  if (info->num_lights > 0)
  {
    c = csColor (0.0, 0.0, 0.0);
    for (int i = 0; i < info->num_lights; i ++)
    {
      iLight *li = info->light_list[i];
      c += CalculateLightIntensity (li, b->pos, b->norm);
    }
    c.Clamp (1.0, 1.0, 1.0);
  }
  info->AddVertex (v, t, c, p);
}

void nTerrain::ProcessTreeNode(iRenderView *rv, float kappa, unsigned int level, unsigned int parent, unsigned int child, unsigned int branch)
{
  nBlock b = (nBlock *)hm->GetPointer(child + parent);
  if (level < 2 * max_levels - 1) {
    csSphere bs(b.pos, b.radius);
    if (rv->TestBSphere (obj2cam, bs))
    {
      float distance = (obj2cam * b.pos).SquaredNorm();
      float error_projection = (b.error / kappa + b.radius);
      error_projection *= error_projection;
      if (error_projection > distance)
      {
        ProcessTreeNode (rv, kappa, level + 1, parent, branch, branch * 2 + 0);
        BufferTreeNode (level & 1, &b);
        ProcessTreeNode (rv, kappa, level + 1, parent, branch, branch * 2 + 1);
      } 
    }
  }
  else
  {
    float distance = (obj2cam * b.pos).SquaredNorm();
    float error_projection = (b.error / kappa + b.radius);
    error_projection *= error_projection;
    if (error_projection > distance)
    {
      BufferTreeNode (level & 1, &b);
    } 
  }
}

void nTerrain::AssembleTerrain(iRenderView *rv, iMovable*m, nTerrainInfo *terrinfo, const csBox3& bbox)
{
  info = terrinfo;   
  movable = m;

  nBlock sw = (nBlock *)hm->GetPointer(0);
  nBlock se = (nBlock *)hm->GetPointer(1);
  nBlock ne = (nBlock *)hm->GetPointer(2);
  nBlock nw = (nBlock *)hm->GetPointer(3);
  nBlock c = (nBlock *)hm->GetPointer(4);

  terrain_w = (unsigned int)sw.radius;
  max_levels = ilogb(terrain_w);

  unsigned int size = 0;
  for (unsigned int i = 0, inc = 1; i < 2 * max_levels - 1; i ++, inc *= 2)
  {
    size += inc;
  }
  float kappa = error_metric_tolerance * 
    rv->GetCamera()->GetInvFOV() * 2 * tan (rv->GetCamera()->GetFOVAngle() / 180.0 * PI / 2);
    // rv->GetCamera()->GetInvFOV() * rv->GetCamera()->GetFOVAngle() / 180.0 * PI;

  float mid = terrain_w/2.0;
  info->InitBuffer (sw.pos, 
    csVector2 ((sw.pos.x + mid)/(float)terrain_w, 1.0 - (sw.pos.z + mid)/(float)terrain_w),
    csColor (1.0, 1.0, 1.0), 0);
  ProcessTreeNode (rv, kappa, 1, 0 * size + 4, 1, 2);
  BufferTreeNode (0, &c);
  ProcessTreeNode (rv, kappa, 1, 0 * size + 4, 1, 3);

  BufferTreeNode (1, &se);
  ProcessTreeNode (rv, kappa, 1, 1 * size + 4, 1, 2);
  BufferTreeNode (0, &c);
  ProcessTreeNode (rv, kappa, 1, 1 * size + 4, 1, 3);

  BufferTreeNode (1, &ne);
  ProcessTreeNode (rv, kappa, 1, 2 * size + 4, 1, 2);
  BufferTreeNode (0, &c);
  ProcessTreeNode (rv, kappa, 1, 2 * size + 4, 1, 3);

  BufferTreeNode (1, &nw);
  ProcessTreeNode (rv, kappa, 1, 3 * size + 4, 1, 2);
  BufferTreeNode (0, &c);
  ProcessTreeNode (rv, kappa, 1, 3 * size + 4, 1, 3);

  info->EndBuffer (sw.pos, 
    csVector2 ((sw.pos.x + mid)/(float)terrain_w, 1.0 - (sw.pos.z + mid)/(float)terrain_w),
    csColor (1.0, 1.0, 1.0), rv, bbox);
}

void nTerrain::SetMaterialsList(iMaterialWrapper **matlist, unsigned int nMaterials)
{
  unsigned int i;
  delete [] materials;

  materials = new iMaterialWrapper *[nMaterials];
  for(i=0; i<nMaterials; ++i)
    materials[i]=matlist[i];

  // @@@ Should I incref each material?  Then decref on closing?
}

void nTerrain::CreateMaterialMap(iFile *matmap, iImage* /*terrtex*/)
{
  char mode[128];
  char matname[512];
  
  char *data = new char[matmap->GetSize()];
  matmap->Read(data, matmap->GetSize());

  unsigned int index=0;
  int read;
  int r,g,b;

  const int MAP_MODE_RGB = 0;
  const int MAP_MODE_8BIT = 1;
  
  while(index<matmap->GetSize())
  {
    // Get some settings
    if ((read=csScanStr(&data[index], "scale: %d", &map_scale))!=-1)
	index+=read;

    else if ((read=csScanStr(&data[index], "mode: %s", mode))!=-1)
    {
	index+=read;
	if (strcmp(mode, "RGB")==0)
	  map_mode=MAP_MODE_RGB;
	else
	  map_mode=MAP_MODE_8BIT;
    }
    else if (map_mode==MAP_MODE_RGB && (read=csScanStr(&data[index], "%d,%d,%d: %s", &r, &g, &b, matname))!=-1)
    {
	index+=read;
	csRGBcolor *color = new csRGBcolor;

	color->red=r;
	color->green=g;
	color->blue=b;

	rgb_colors.Push(color);
    }

    else if (map_mode==MAP_MODE_8BIT && (read=csScanStr(&data[index], "%d: %s", &r, matname))!=-1)
    {
	index+=read;
	
	// Disabled: not used: pal_colors.Push(r);
    }
  }

  // Now sort the list
  if (map_mode==MAP_MODE_RGB) rgb_colors.Sort(rgb_colors.Compare);
  // Disabled: not used: else pal_colors.Sort();
}

SCF_IMPLEMENT_IBASE (csBigTerrainObject)
  SCF_IMPLEMENTS_INTERFACE (iMeshObject)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iTerrBigState)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iObjectModel)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iVertexBufferManagerClient)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csBigTerrainObject::eiTerrBigState)
  SCF_IMPLEMENTS_INTERFACE (iTerrBigState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csBigTerrainObject::eiObjectModel)
  SCF_IMPLEMENTS_INTERFACE (iObjectModel)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csBigTerrainObject::eiVertexBufferManagerClient)
  SCF_IMPLEMENTS_INTERFACE (iVertexBufferManagerClient)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

csBigTerrainObject::csBigTerrainObject(iObjectRegistry* _obj_reg, iMeshObjectFactory *_pFactory):pFactory(_pFactory), object_reg(_obj_reg), terrain(0), nTextures(0), scale (1,1,1) 
{
  SCF_CONSTRUCT_IBASE (_pFactory)
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiTerrBigState);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiObjectModel);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiVertexBufferManagerClient);

  info = new nTerrainInfo(_obj_reg);
  terrain = new nTerrain;
  light_mgr = CS_QUERY_REGISTRY (object_reg, iLightManager);
}

csBigTerrainObject::~csBigTerrainObject()
{
  delete terrain;
  if (info->light_list)
  {
    delete [] info->light_list;
  }
  delete info;

  SCF_DESTRUCT_EMBEDDED_IBASE (scfiTerrBigState);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiObjectModel);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiVertexBufferManagerClient);
  SCF_DESTRUCT_IBASE ()
}

bool csBigTerrainObject::LoadHeightMapFile (const char *filename) 
{
  /* terrain will delete the memory map class */
  if (!terrain)
    terrain = new nTerrain;
  terrain->SetHeightMapFile (filename);
  InitMesh (info);
  return true;
}

void csBigTerrainObject::SetScaleFactor (const csVector3 &s)
{
  scale = s;
}

void csBigTerrainObject::SetErrorTolerance (float tolerance)
{
  if (terrain)
    terrain->SetErrorTolerance (tolerance);
}

bool csBigTerrainObject::ConvertImageToMapFile (iFile *input, 
	iImageIO *imageio, const char *hm) 
{
  uint8 *raw;

  raw = new uint8[input->GetSize()];
  if (input->Read ((char *)raw, input->GetSize()) != input->GetSize ())
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR, 
    	"crystalspace.mesh.object.terrbig", 
	"Unable to read from input file\n");
    return false;
  }
  csRef<iImage> image (imageio->Load (raw, input->GetSize(), CS_IMGFMT_ANY));
  delete raw;
  if (!image)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR, 
    	"crystalspace.mesh.object.terrbig", 
	"Unable to load image file\n");
    return false;
  }
  if (image->GetWidth () != image->GetHeight ())
  {
    image->Rescale (image->GetWidth (), image->GetWidth ());
  }
  if (image->GetWidth () != ((1 << (csLog2 (image->GetWidth()))) + 1))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
	    "crystalspace.mesh.object.terrbig",
		"Unable to process image, must square and width 2^n+1");
    return false;
  }

  float *image_data = new float [image->GetSize()];

  if (image->GetFormat () & CS_IMGFMT_PALETTED8)
  {
    csRGBpixel *palette = image->GetPalette ();
    uint8 *data = (uint8 *)image->GetImageData ();
    for (int i = 0; i < image->GetSize (); i ++)
    {
      image_data[i] = ((float)palette[data[i]].Intensity()) / 255.0;
    }
  }
  else
  {
    csRGBpixel *data = (csRGBpixel *)image->GetImageData ();
    for (int i = 0; i < image->GetSize (); i ++)
    {
      image_data[i] = ((float)data[i].Intensity()) / 255.0 * scale.y;
    }
  }

  bool retval = ConvertArrayToMapFile (image_data, image->GetWidth(), hm);
  delete [] image_data;
  return retval;
}

bool csBigTerrainObject::ConvertArrayToMapFile (float *data, int w, const char *hm)
{
  if (w != ((1 << csLog2 (w)) + 1))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
	"crystalspace.mesh.object.terrbig",
	"Unable to process image, must square and width 2^n+1");
    return false;
  }
  FILE *hmfp = fopen (hm, "wb");
  if (!hmfp)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR, 
	"crystalspace.mesh.object.terrbig", 
	"Unable to open %s for writing\n", hm);
    return false;
  }
  int size = w * w;

  nBlock *heightmap = new nBlock[size];
  int k;
  for (k = 0; k < size; k ++)
  {
    heightmap[k].pos.x = (k % w - w/2) * scale.x;
    heightmap[k].pos.y = data[k] * scale.y;
    heightmap[k].pos.z = (w/2 - k/w) * scale.z;
    heightmap[k].error = 0.0;
    heightmap[k].radius = 0.0;
  }

  for (k = 0; k < size; k ++)
  {
    const float up = (k-w < 0) ? 0.0 : heightmap[k-w].pos.y;
    const float dn = (k+w >= size) ?  0.0 : heightmap[k+w].pos.y;
    const float lt = (k%w == 0) ?  0.0 : heightmap[k-1].pos.y;
    const float rt = ((k+1)%w == 0) ?  0.0 : heightmap[k+1].pos.y;

    heightmap[k].norm.x = (up - dn);
    heightmap[k].norm.y = (lt - rt);
    heightmap[k].norm.z = 4 / w;
    heightmap[k].norm.Normalize ();
  }

  int a, b, c, s, i, j;
  for (a = c = 1, b = 2, s = 0; a != w-1; a = c = b, b *= 2, s = w)
  {
    for (j = a; j < w-1; j += b)
    {
      for (i = 0; i < w; i += b)
      {
        ComputeLod (heightmap, i, j, 0, a, s, w);
        ComputeLod (heightmap, j, i, a, 0, s, w);
      }
    }

    for (j = a; j < w-1; c = -c, j += b)
    {
      for (i = a; i < w-1; c = -c, i += b)
      {
        ComputeLod (heightmap, i, j, a, c, w, w);
      }
    }
  }

  heightmap[(w-1)*w].error = 0.0;
  heightmap[(w-1)*w].radius = w;
  heightmap[w-1+(w-1)*w].error = 0.0;
  heightmap[w-1+(w-1)*w].radius = w;
  heightmap[w-1].error = 0.0;
  heightmap[w-1].radius = w;
  heightmap[0].error = 0.0;
  heightmap[0].radius = w;

  if (!terrain)
  {
    terrain = new nTerrain;
  }
  terrain->BuildTree (hmfp, heightmap, w);
  delete [] heightmap;
  fclose (hmfp);

  terrain->SetHeightMapFile (hm);
  InitMesh (info);

  return true;
}

void csBigTerrainObject::ComputeLod (nBlock *heightmap, int i, int j, int di, int dj, int n, int width)
{
  nBlock *b = &heightmap[i + j * width];
  nBlock *l = &heightmap[i - di + (j - dj) * width];
  nBlock *r = &heightmap[i + di + (j + dj) * width];
  b->error = (float)fabs(b->pos.y - (l->pos.y + r->pos.y) / 2.0);
  if (n)
  {
    dj = (di + dj) / 2;
    di -= dj;
    for (int k = 0; k < 4; k ++)
    {
      if ((i > 0 || di >= 0) && (i < (width-1) || di <= 0) &&
          (j > 0 || dj >= 0) && (j < (width-1) || dj <= 0))
      {
        nBlock *cp = &heightmap[i + di + (j + dj) * width];
        b->error = (b->error > cp->error) ? b->error : cp->error;
        float r = (b->pos - cp->pos).Norm() + cp->radius;
        b->radius = (b->radius > r) ? b->radius : r;
      }
      dj += di;
      di -= dj;
      dj += di;
    }
  }
}

void csBigTerrainObject::InitMesh (nTerrainInfo *info)
{
  info->num_lights = 0;
  info->light_list = 0;

  info->GetMesh()->morph_factor = 0;
  info->GetMesh()->num_vertices_pool = 1;
  info->GetMesh()->do_morph_texels = false;
  info->GetMesh()->do_morph_colors = false;
  info->GetMesh()->do_fog = false;
  info->GetMesh()->vertex_mode = G3DTriangleMesh::VM_WORLDSPACE;
}

bool csBigTerrainObject::DrawTest (iRenderView* rview, iMovable* movable,
	uint32 frustum_mask)
{
  if (terrain)
  {
    iCamera* cam = rview->GetCamera ();

    csReversibleTransform tr_o2c = cam->GetTransform ();
    tr_o2c /= movable->GetFullTransform ();
    terrain->SetObjectToCamera (tr_o2c);
    csBox3 bbox;
    GetObjectBoundingBox (bbox);
    terrain->AssembleTerrain(rview, movable, info, bbox);

    info->GetMesh()->do_mirror = rview->GetCamera()->IsMirrored();
    if (info->num_lights > 0)
    {
      info->GetMesh()->use_vertex_color = 1;
      info->GetMesh()->mixmode = CS_FX_COPY;
    }
    else
    {
      info->GetMesh()->use_vertex_color = 0;
      info->GetMesh()->mixmode = 0;
    }

    int clip_portal, clip_plane, clip_z_plane;
    rview->CalculateClipSettings (frustum_mask, clip_portal, clip_plane,
	  clip_z_plane);
    info->GetMesh()->clip_portal = clip_portal;
    info->GetMesh()->clip_plane = clip_plane;
    info->GetMesh()->clip_z_plane = clip_z_plane;
    if (light_mgr)
    {
      const csArray<iLight*>& relevant_lights = light_mgr
    	  ->GetRelevantLights (logparent, -1, false);
      UpdateLighting (relevant_lights, movable);
    }

    return true;
  }

  return false;
}

void csBigTerrainObject::UpdateLighting (const csArray<iLight*>& lis, iMovable*)
{
  if (info->light_list)
  {
    delete [] info->light_list;
  }
  int num_lights = lis.Length ();
  info->num_lights = num_lights;
  info->light_list = new iLight*[num_lights];
  int i;
  for (i = 0 ; i < num_lights ; i++)
    info->light_list[i] = lis[i];
}

bool csBigTerrainObject::Draw (iRenderView* rview, iMovable* m, csZBufMode zbufMode)
{
  iGraphics3D* pG3D = rview->GetGraphics3D ();
  iCamera* pCamera = rview->GetCamera ();

  csReversibleTransform tr_o2c = pCamera->GetTransform ();
  tr_o2c /= m->GetFullTransform ();
  pG3D->SetObjectToCamera (&tr_o2c);
  pG3D->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, zbufMode );
  info->GetMesh()->mat_handle = terrain->GetMaterialsList()[0]->GetMaterialHandle();
  terrain->GetMaterialsList()[0]->Visit ();
  rview->CalculateFogMesh (tr_o2c, *info->GetMesh());
  pG3D->DrawTriangleMesh(*info->GetMesh());
  return true;
}

void csBigTerrainObject::GetObjectBoundingBox (csBox3& bbox, int /*type*/)
{
  /* use the radius from the center */
  bbox.StartBoundingBox( csVector3( -1000, -1000, -1000 ) );
  bbox.AddBoundingVertexSmart( csVector3( 1000,  1000,  1000 ) );
}

void csBigTerrainObject::GetRadius (csVector3& rad, csVector3& cent)
{
  /* use the radius from the center */
  rad = csVector3( 1000, 1000, 1000 );
  cent = csVector3( 0, 0, 0 );
}

bool csBigTerrainObject::HitBeamOutline (const csVector3&, const csVector3&, csVector3&, float*)
// csBigTerrainObject::HitBeamOutline (const csVector3& start, const csVector3& end, csVector3& isect, float* pr)
{
	return false;
}

bool csBigTerrainObject::HitBeamObject (const csVector3&, const csVector3&, csVector3&, float*, int*)
// csBigTerrainObject::HitBeamObject (const csVector3& start, const csVector3& end, csVector3& isect, float* pr)
{
	return false;

}

void csBigTerrainObject::SetMaterialsList(iMaterialWrapper **matlist, unsigned int nMaterials)
{
  if (terrain) terrain->SetMaterialsList(matlist, nMaterials);
	nTextures = nMaterials;
}


///////////////////////////////////////////////////////////////////////////////////////////

SCF_IMPLEMENT_IBASE (csBigTerrainObjectFactory)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectFactory)
SCF_IMPLEMENT_IBASE_END

csBigTerrainObjectFactory::csBigTerrainObjectFactory (iBase *pParent, iObjectRegistry* object_reg)
{
  SCF_CONSTRUCT_IBASE (pParent);
  csBigTerrainObjectFactory::object_reg = object_reg;
  logparent = pParent;
}

csBigTerrainObjectFactory::~csBigTerrainObjectFactory ()
{
  SCF_DESTRUCT_IBASE ();
}

csPtr<iMeshObject> csBigTerrainObjectFactory::NewInstance ()
{
  csBigTerrainObject* pTerrObj = new csBigTerrainObject (object_reg, this);
  return csPtr<iMeshObject> (pTerrObj);
}

///////////////////////////////////////////////////////////////////////////////////////////
SCF_IMPLEMENT_IBASE (csBigTerrainObjectType)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectType)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csBigTerrainObjectType::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csBigTerrainObjectType)


csBigTerrainObjectType::csBigTerrainObjectType (iBase *pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csBigTerrainObjectType::~csBigTerrainObjectType ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

csPtr<iMeshObjectFactory> csBigTerrainObjectType::NewFactory ()
{
  csBigTerrainObjectFactory* btf = new csBigTerrainObjectFactory (this, 
  	object_reg);
  csRef<iMeshObjectFactory> ifact (
  	SCF_QUERY_INTERFACE (btf, iMeshObjectFactory));
  btf->DecRef ();
  return csPtr<iMeshObjectFactory> (ifact);
}

