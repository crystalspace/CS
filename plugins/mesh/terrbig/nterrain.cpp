#define CS_SYSDEF_PROVIDE_HARDWARE_MMIO 1

#include "cssysdef.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/polyclip.h"
#include "csgeom/poly2d.h"
#include "csgeom/vector3.h"
#include "csutil/garray.h"
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

void nTerrain::SetVariance(nBlock &b)
{
  float low=b.ne;
  float high=b.ne;

  if (b.nw<low)  low=b.nw;
  if (b.nw>high) high=b.nw;

  if (b.se<low)  low=b.se;
  if (b.se>high) high=b.se;

  if (b.sw<low)  low=b.sw;
  if (b.sw>high) high=b.sw;

  if (b.center<low)  low=b.center;
  if (b.center>high) high=b.center;

  // Store variance
  b.variance = high-low;
  b.midh = low;
}

float nTerrain::BuildTreeNode(FILE *f, unsigned int level, 
	unsigned int parent_index, unsigned int child_num, nRect bounds, 
	float *heightmap, csVector3 *norms, unsigned int w)
{
  unsigned int my_index = (parent_index<<2) + child_num + NTERRAIN_QUADTREE_ROOT;
  unsigned int mid = bounds.w>>1;
  unsigned int cw = mid+1;
  nBlock b;

  // Get heights.
  b.ne = heightmap[bounds.x +              (bounds.y * w)];
  b.nw = heightmap[bounds.x + bounds.w-1 + (bounds.y * w)];
  b.se = heightmap[bounds.x +              ((bounds.y + bounds.h-1) * w)];
  b.sw = heightmap[bounds.x + bounds.w-1 + ((bounds.y + bounds.h-1) * w)];
  b.center = heightmap[bounds.x +  mid  +  ((bounds.y + mid) * w)];

  b.ne_norm = norms[bounds.x +              (bounds.y * w)];
  b.nw_norm = norms[bounds.x + bounds.w-1 + (bounds.y * w)];
  b.se_norm = norms[bounds.x +              ((bounds.y + bounds.h-1) * w)];
  b.sw_norm = norms[bounds.x + bounds.w-1 + ((bounds.y + bounds.h-1) * w)];
  b.ce_norm = norms[bounds.x +  mid  +  ((bounds.y + mid) * w)];

  // Set the variance for this block (the difference between the highest and lowest points.)
  SetVariance(b);

  // Expand the quadtree until we get to the max resolution.
  if (level<max_levels)
  {
    float var1 = BuildTreeNode(f, level+1, my_index, 0, nRect(bounds.x,bounds.y,cw,cw), heightmap, norms, w);
    float var2 = BuildTreeNode(f, level+1, my_index, 1, nRect(bounds.x+mid,bounds.y,cw,cw), heightmap, norms, w);
    float var3 = BuildTreeNode(f, level+1, my_index, 2, nRect(bounds.x,bounds.y+mid,cw,cw), heightmap, norms, w);
    float var4 = BuildTreeNode(f, level+1, my_index, 3, nRect(bounds.x+mid,bounds.y+mid,cw,cw), heightmap, norms, w);

    b.variance = (var1 > b.variance) ? var1 : b.variance;
    b.variance = (var2 > b.variance) ? var2 : b.variance;
    b.variance = (var3 > b.variance) ? var3 : b.variance;
    b.variance = (var4 > b.variance) ? var4 : b.variance;
  }
  /* midh was set to low */
  b.midh += b.variance/2.0;

  // Store the block in the file.
  fseek(f, my_index*nBlockLen, SEEK_SET);
  fwrite(&b, nBlockLen, 1, f);

  return b.variance;
}

csColor nTerrain::CalculateLightIntensity (iLight *li, iMovable *m, csVector3 v, csVector3 n)
{
  csColor color (0.0, 0.0, 0.0);
  csVector3 li_center = m->GetTransform().Other2This (li->GetCenter());
  csVector3 light_dir = v - li_center;
  float sq_dist = light_dir.SquaredNorm();
  if (sq_dist < li->GetSquaredRadius ()) {
    
    csColor light_color = li->GetColor () * (256.0 / CS_NORMAL_LIGHT_LEVEL)
      * li->GetBrightnessAtDistance (qsqrt (sq_dist));

    float cosinus;
    if (sq_dist < SMALL_EPSILON) 
      cosinus = 1.0;
    else
      cosinus = light_dir * n;

    if (cosinus > 0) {
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

void nTerrain::BufferTreeNode(iMovable *m, nBlock *b, nRect bounds)
{
  int mid = terrain_w >> 1;
  csReversibleTransform t = m->GetTransform();
  int ne = info->vertex_count ++;
  int nw = info->vertex_count ++;
  int se = info->vertex_count ++;
  int sw = info->vertex_count ++;
  int ce = info->vertex_count ++;
  info->vertices[ne] = csVector3((bounds.x-mid)*scale.x, 
                                 b->ne*scale.y, 
							     (bounds.y-mid)*scale.z) / t;
  info->vertices[nw] = csVector3((bounds.x+bounds.w-1-mid)*scale.x, 
                                 b->nw*scale.y, 
                                 (bounds.y-mid)*scale.z) / t;
  info->vertices[se] = csVector3((bounds.x-mid)*scale.x,
                                 b->se*scale.y, 
                                 (bounds.y+bounds.h-1-mid)*scale.z) / t;
  info->vertices[sw] = csVector3((bounds.x+bounds.w-1-mid)*scale.x,
                                 b->sw*scale.y, 
                                 (bounds.y+bounds.h-1-mid)*scale.z) / t;
  info->vertices[ce] = csVector3((bounds.x+bounds.h/2.0-mid)*scale.x,
                                 b->center*scale.y, 
                                 (bounds.y+bounds.h/2.0-mid)*scale.z) / t; 

  float wid = (float)terrain_w;
  info->texels[ne] = csVector2(bounds.x/wid, bounds.y/wid);
  info->texels[nw] = csVector2((bounds.x+bounds.w-1)/wid, bounds.y/wid);
  info->texels[se] = csVector2(bounds.x/wid, (bounds.y+bounds.h-1)/wid);
  info->texels[sw] = csVector2((bounds.x+bounds.w-1)/wid, (bounds.y+bounds.h-1)/wid);
  info->texels[ce] = csVector2((bounds.x+bounds.h/2.0)/wid, (bounds.y+bounds.h/2.0)/wid);

  csColor nec(0,0,0), nwc(0,0,0), sec(0,0,0), swc(0,0,0), centerc(0,0,0);

  if (info->num_lights > 0) {
    for (int i = 0; i < info->num_lights; i ++) {
      iLight* li = info->light_list[i];

      nec += CalculateLightIntensity (li, m, info->vertices[ne], b->ne_norm);
      nwc += CalculateLightIntensity (li, m, info->vertices[nw], b->nw_norm);
      sec += CalculateLightIntensity (li, m, info->vertices[se], b->se_norm);
      swc += CalculateLightIntensity (li, m, info->vertices[sw], b->sw_norm);
      centerc += CalculateLightIntensity (li, m, info->vertices[ce], b->ce_norm);
    }
  } else {
    nec = csColor (1.0, 1.0, 1.0);
    nwc = csColor (1.0, 1.0, 1.0);
    sec = csColor (1.0, 1.0, 1.0);
    swc = csColor (1.0, 1.0, 1.0);
    centerc = csColor (1.0, 1.0, 1.0);
  }

  nec.Clamp (2., 2., 2.);
  nwc.Clamp (2., 2., 2.);
  sec.Clamp (2., 2., 2.);
  swc.Clamp (2., 2., 2.);
  centerc.Clamp (2., 2., 2.);

  info->colors[ne] = (nec);
  info->colors[nw] = (nwc);
  info->colors[se] = (sec);
  info->colors[sw] = (swc);
  info->colors[ce] = (centerc);

  /** Build triangle stacks, for each block output four triangles.
   * No need to merge and split because each block is at full resolution,
   * we have no partial resolution blocks.
   */
  info->triq[0/*b->ti*/].triangles[info->triangle_count++] = csTriangle(sw, ce, se);
  info->triq[0/*b->ti*/].triangles[info->triangle_count++] = csTriangle(nw, ce, sw);
  info->triq[0/*b->ti*/].triangles[info->triangle_count++] = csTriangle(ne, ce, nw);
  info->triq[0/*b->ti*/].triangles[info->triangle_count++] = csTriangle(se, ce, ne);
}

void nTerrain::ProcessTreeNode(iRenderView *rv, iMovable *m, unsigned int level, unsigned int parent_index, unsigned int child_num, nRect bounds)
{

  unsigned int my_index = (parent_index<<2) + child_num + NTERRAIN_QUADTREE_ROOT;
  int mid = bounds.w>>1;
  unsigned int cw = mid+1;
  bool render_this=false;
  nBlock *b;
  
  // Get the block we're currently checking.
  b=(nBlock *)hm->GetPointer(my_index);

  if (!b) {
  	return;
  }

  // Create a bounding sphere.
  int c = terrain_w >> 1;
  csVector3 center (bounds.x+mid-c*scale.x, b->midh*scale.y, bounds.y+mid-c*scale.z);
  csVector3 radius (bounds.w * scale.x, b->variance * scale.y, bounds.h * scale.z);
  csSphere bs(m->GetTransform().This2Other(center), radius.Norm());

  // Test it for culling, return if it's not visible.
  if (!rv->TestBSphere(obj2cam, bs)) return;

  // If we are at the bottom level, we HAVE to render this block, so don't bother with the calcs.
  if (level>=max_levels)
    render_this=true;

  else
  {
    // Get distance from center of block to camera, plus a small epsilon to avoid division by zero.
    float distance = ((cam-bs.GetCenter()).Norm())+1e-10;
    // Get the error metric, in this case it's the ratio between variance and distance.
    float error_metric = b->variance * scale.y / distance;

    if (error_metric<=error_metric_tolerance) {
      render_this=true;
    } 
  }

  // Don't render this block, resolve to the next level.
  if (!render_this)
  {
    ProcessTreeNode(rv, m, level+1, my_index, 0, nRect(bounds.x,bounds.y,cw,cw));
    ProcessTreeNode(rv, m, level+1, my_index, 1, nRect(bounds.x+mid,bounds.y,cw,cw));
    ProcessTreeNode(rv, m, level+1, my_index, 2, nRect(bounds.x,bounds.y+mid,cw,cw));
    ProcessTreeNode(rv, m, level+1, my_index, 3, nRect(bounds.x+mid,bounds.y+mid,cw,cw));
  }
  // Render this block to the buffer for later drawing.
  else
    BufferTreeNode(m, b, bounds);

}

void nTerrain::BuildTree(FILE *f, float *heightmap, csVector3 *norms, unsigned int w)
{
  terrain_w = w-1;
  max_levels = ilogb(w-1) - 1;

  unsigned int mid = w>>1;
  unsigned int cw = mid+1;
  unsigned int x=0, y=0;
  nBlock b;

  b.ne = heightmap[0];
  b.nw = heightmap[w-1];
  b.se = heightmap[(w-1)*w];
  b.sw = heightmap[w-1 + (w-1) * w];
  b.center = heightmap[mid + mid * w];

  b.ne_norm = norms[0];
  b.nw_norm = norms[w-1];
  b.se_norm = norms[(w-1)*w];
  b.sw_norm = norms[w-1 + (w-1) * w];
  b.ce_norm = norms[mid + mid * w];

  SetVariance(b);

  b.midh = w; /* for this first block the w is encoded into midh */

  float var1 = BuildTreeNode(f, 1, 0, 0, nRect(x,y,cw,cw), heightmap, norms, w);
  float var2 = BuildTreeNode(f, 1, 0, 1, nRect(x+mid,y,cw,cw), heightmap, norms, w);
  float var3 = BuildTreeNode(f, 1, 0, 2, nRect(x,y+mid,cw,cw), heightmap, norms, w);
  float var4 = BuildTreeNode(f, 1, 0, 3, nRect(x+mid,y+mid,cw,cw), heightmap, norms, w);

  b.variance = (var1 > b.variance) ? var1 : b.variance;
  b.variance = (var2 > b.variance) ? var2 : b.variance;
  b.variance = (var3 > b.variance) ? var3 : b.variance;
  b.variance = (var4 > b.variance) ? var4 : b.variance;

  fseek (f, 0, SEEK_SET); 
  fwrite (&b, nBlockLen, 1, f);
}

void nTerrain::AssembleTerrain(iRenderView *rv, iMovable *m, nTerrainInfo *terrinfo)
{
  // Clear mesh lists
  info = terrinfo;   

  nBlock *b = (nBlock *)hm->GetPointer(0);
  if (!b) { return; }

  terrain_w = (unsigned int)b->midh;
  max_levels = ilogb(terrain_w) - 1;

  unsigned int mid = terrain_w>>1;
  unsigned int cw = mid+1;
  unsigned int x=0, y=0;
  
  csVector3 center (0, b->variance / 2.0 * scale.y, 0);
  csVector3 radius (terrain_w * scale.x, b->variance * scale.y, terrain_w * scale.z);
  csSphere bs (m->GetTransform().This2Other(center), radius.Norm());
  if (!rv->TestBSphere(obj2cam, bs)) return;

  float distance = ((cam-bs.GetCenter()).Norm())+0.0001;
  float error_metric = b->variance * scale.y / distance;

  if (error_metric <= error_metric_tolerance) 
  BufferTreeNode (m, b, nRect (0, 0, terrain_w+1, terrain_w+1));

  else {

  // Buffer entire viewable terrain by first doing view culling on the block, then checking for the
  // error metric.  If the error metric fails, then we need to drop down another level. Begin that
  // process here.
  
  ProcessTreeNode(rv, m, 1, 0, 0, nRect(x,y,cw,cw));
  ProcessTreeNode(rv, m, 1, 0, 1, nRect(x+mid,y,cw,cw));
  ProcessTreeNode(rv, m, 1, 0, 2, nRect(x,y+mid,cw,cw));
  ProcessTreeNode(rv, m, 1, 0, 3, nRect(x+mid,y+mid,cw,cw));
  
  }

}

void nTerrain::SetMaterialsList(iMaterialWrapper **matlist, unsigned int nMaterials)
{
  unsigned int i;
  if (materials) delete [] materials;

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
	
	pal_colors.Push((void *)r);
    }
  }

  // Now sort the list
  if (map_mode==MAP_MODE_RGB) rgb_colors.QuickSort();
  else pal_colors.QuickSort();
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

csBigTerrainObject::csBigTerrainObject(iObjectRegistry* _obj_reg, iMeshObjectFactory *_pFactory):vbufmgr(0), pFactory(_pFactory), object_reg(_obj_reg), terrain(NULL), nTextures(0) 
{
  SCF_CONSTRUCT_IBASE (_pFactory)
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiTerrBigState);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiObjectModel);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiVertexBufferManagerClient);

  info = new nTerrainInfo();
	
	terrain = new nTerrain;
}

csBigTerrainObject::~csBigTerrainObject()
{
  if (terrain) delete terrain;
  if (info)    
  {
    delete [] info->mesh;
    delete [] info->triq;
    delete info;
  }
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

void csBigTerrainObject::SetScaleFactor (const csVector3 &scale)
{
  if (terrain)
    terrain->SetScaleFactor (scale);
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
  if (input->Read ((char *)raw, input->GetSize()) != input->GetSize ()) {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR, 
    	"crystalspace.mesh.object.terrbig", 
	"Unable to read from input file\n");
    return false;
  }
  csRef<iImage> image (imageio->Load (raw, input->GetSize(), CS_IMGFMT_ANY));
  delete raw;
  if (!image) {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR, 
    	"crystalspace.mesh.object.terrbig", 
	"Unable to load image file\n");
    return false;
  }
  if (image->GetWidth () != image->GetHeight ()) {
    image->Rescale (image->GetWidth (), image->GetWidth ());
  }
  FILE *hmfp = fopen (hm, "w");
  if (!hmfp) {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR, 
    	"crystalspace.mesh.object.terrbig", 
	"Unable to open %s for writing\n", hm);
    return false;
  }
  int size = image->GetSize ();
  int width = image->GetWidth ();
  float *heightmap = new float[size];
  if (image->GetFormat () & CS_IMGFMT_PALETTED8) {
    csRGBpixel *palette = image->GetPalette ();
    uint8 *data = (uint8 *)image->GetImageData ();
    for (int i = 0; i < image->GetSize (); i ++) {
      heightmap[i] = ((float)palette[data[i]].Intensity()) / 255.0;
    }
  } else {
    csRGBpixel *data = (csRGBpixel *)image->GetImageData ();
    for (int i = 0; i < image->GetSize (); i ++) {
      heightmap[i] = ((float)data[i].Intensity()) / 255.0;
    }
  }
  csVector3 *norms = new csVector3[image->GetSize ()];
  for  (int i = 0; i < size; i ++) {
    int ind = i - width;
    csVector3 curr (0, heightmap[i], 0);
    csVector3 up = csVector3(0, (ind < 0) ? 0 : heightmap[ind], 1) - curr;
    ind = i + width;
    csVector3 dn = csVector3(0, (ind >= image->GetSize ()) ? 0 : heightmap[ind], -1) - curr;
    ind = i - 1;
    csVector3 lt = csVector3(-1, 
        (i%width - ind%width != 1) ? 0 : heightmap[ind], 0) - curr;
    ind = i + 1;
    csVector3 rt = csVector3(1, 
        (ind%width - i%width != 1) ? 0 : heightmap[ind], 0) - curr;

    norms[i] = (up + dn + lt + rt) / 4.0;
    norms[i].Normalize ();
  }
  if (!terrain) {
    terrain = new nTerrain;
  }
  terrain->BuildTree (hmfp, heightmap, norms, width);
  delete [] heightmap;
  delete [] norms;
  fclose (hmfp);

  terrain->SetHeightMapFile (hm);
  InitMesh (info);
  return true;
}

void 
csBigTerrainObject::SetupVertexBuffer (csRef<iVertexBuffer> &vbuf1)
{
 if (!vbuf1)
 {
   if (!vbufmgr)
   {
     iObjectRegistry* object_reg = ((csBigTerrainObjectFactory*)pFactory)
     	->object_reg;
     csRef<iGraphics3D> g3d (
     	CS_QUERY_REGISTRY (object_reg, iGraphics3D));

     // @@@ priority should be a parameter.
     vbufmgr = g3d->GetVertexBufferManager ();

     //vbufmgr->AddClient (&scfiVertexBufferManagerClient);
   }
   vbuf = vbufmgr->CreateBuffer (1);
 }
}

void csBigTerrainObject::InitMesh (nTerrainInfo *info)
{
  int i;
 
  if (nTextures) {
 
    info->mesh = new G3DTriangleMesh[nTextures];
    info->triq = new nTerrainInfo::triangle_queue[nTextures];
	if (terrain) {
	  unsigned int size = terrain->GetWidth () * terrain->GetWidth();
	  for (int i = 0; i < nTextures; i ++) {
	  	info->triq->triangles = new csTriangle[size];
	  }
	  info->vertices = new csVector3[size];
	  info->texels = new csVector2[size];
	  info->tindexes = new ti_type[size];
	  info->colors = new csColor[size];
	} else {
	  for (int i = 0; i < nTextures; i ++) {
	  	info->triq->triangles = NULL;
	  }
	  info->vertices = NULL;
	  info->texels = NULL;
	  info->tindexes = NULL;
	  info->colors = NULL;
	}
    info->num_lights = 0;
	info->light_list = NULL;

    for(i=0; i<nTextures; ++i)
    {
      info->mesh[i].morph_factor = 0;
      info->mesh[i].num_vertices_pool = 1;
      info->mesh[i].do_morph_texels = false;
      info->mesh[i].do_morph_colors = false;
      info->mesh[i].do_fog = false;
      info->mesh[i].vertex_mode = G3DTriangleMesh::VM_WORLDSPACE;
    }
  }
}

bool 
csBigTerrainObject::DrawTest (iRenderView* rview, iMovable* movable)
{
  if (terrain)
  {
    iCamera* cam = rview->GetCamera ();

	info->triangle_count = 0;
	info->vertex_count = 0;
    terrain->SetObjectToCamera(cam->GetTransform());
    terrain->SetCameraOrigin(cam->GetTransform().GetOrigin());
    terrain->AssembleTerrain(rview, movable, info);

	int i;
    for(i=0; i<nTextures; ++i)
    {
	  info->mesh[i].do_mirror = rview->GetCamera()->IsMirrored();
      if (info->num_lights > 0) {
        info->mesh[i].use_vertex_color = 1;
        info->mesh[i].mixmode = CS_FX_COPY | CS_FX_GOURAUD;
      } else {
        info->mesh[i].use_vertex_color = 0;
        info->mesh[i].mixmode = 0;
      }
    }

    return true;
  }

  return false;
}

void 
csBigTerrainObject::UpdateLighting (iLight** lis, int num_lights, iMovable*)
{
  if (info->light_list) {
    delete [] info->light_list;
  }

  info->num_lights = num_lights;
  info->light_list = new iLight*[num_lights];
  memcpy (info->light_list, lis, sizeof (iLight *) * num_lights);
}

bool 
csBigTerrainObject::Draw (iRenderView* rview, iMovable*, csZBufMode zbufMode)
{
  int i;
  static int bufcount=0;

  iGraphics3D* pG3D = rview->GetGraphics3D ();
  iCamera* pCamera = rview->GetCamera ();

  csReversibleTransform& camtrans = pCamera->GetTransform ();
  // const csVector3& origin = camtrans.GetOrigin ();

  pG3D->SetObjectToCamera (&camtrans);
  pG3D->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, zbufMode );

  SetupVertexBuffer(vbuf);

  bufcount++;

  vbufmgr->LockBuffer(vbuf, 
	 	      info->vertices, 
		      info->texels,
		      info->colors,
		      info->vertex_count,
		      bufcount);

  for(i=0; i<nTextures; i++)
  {
    info->mesh[i].mat_handle = terrain->GetMaterialsList()[i]->GetMaterialHandle();
	terrain->GetMaterialsList()[i]->Visit ();
    info->mesh[i].triangles = info->triq[i].triangles;
    info->mesh[i].vertex_fog = new G3DFogInfo[info->vertex_count]; 
    info->mesh[i].buffers[0]=vbuf;
    info->mesh[i].num_triangles = info->triangle_count;
    rview->CalculateFogMesh( pG3D->GetObjectToCamera(), info->mesh[i] );
    pG3D->DrawTriangleMesh(info->mesh[i]);
    delete[] info->mesh[i].vertex_fog;
  }

  vbufmgr->UnlockBuffer(vbuf);

  return true;
}

void 
csBigTerrainObject::GetObjectBoundingBox (csBox3& bbox, int /*type*/)
{
		bbox.StartBoundingBox( csVector3( -1000, -1000, -1000 ) );
		bbox.AddBoundingVertexSmart( csVector3( 1000,  1000,  1000 ) );
}

void 
csBigTerrainObject::GetRadius (csVector3& rad, csVector3& cent)
{
	rad = csVector3( 1000, 1000, 1000 );
	cent = csVector3( 0, 0, 0 );
}

bool 
csBigTerrainObject::HitBeamOutline (const csVector3&, const csVector3&, csVector3&, float*)
// csBigTerrainObject::HitBeamOutline (const csVector3& start, const csVector3& end, csVector3& isect, float* pr)
{
	return false;
}

bool 
csBigTerrainObject::HitBeamObject (const csVector3&, const csVector3&, csVector3&, float*)
// csBigTerrainObject::HitBeamObject (const csVector3& start, const csVector3& end, csVector3& isect, float* pr)
{
	return false;

}

///////////////////////////////////////////////////////////////////////////////////////////
void 
csBigTerrainObject::SetMaterialsList(iMaterialWrapper **matlist, unsigned int nMaterials)
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

SCF_EXPORT_CLASS_TABLE (terrbig)
  SCF_EXPORT_CLASS (csBigTerrainObjectType, "crystalspace.mesh.object.terrbig",
    "Crystal Space Big Terrain Type")
SCF_EXPORT_CLASS_TABLE_END

csBigTerrainObjectType::csBigTerrainObjectType (iBase *pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csBigTerrainObjectType::~csBigTerrainObjectType ()
{
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

