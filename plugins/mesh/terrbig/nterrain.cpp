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
	float *heightmap, unsigned int w)
{
  unsigned int my_index = (parent_index<<2) + child_num + NTERRAIN_QUADTREE_ROOT;
  unsigned int mid = bounds.w>>1;
  unsigned int cw = mid+1;
  nBlock b;

  // Get heights.
  b.ne = 10 * heightmap[bounds.x +              (bounds.y * w)];
  b.nw = 10 * heightmap[bounds.x + bounds.w-1 + (bounds.y * w)];
  b.se = 10 * heightmap[bounds.x +              ((bounds.y + bounds.h-1) * w)];
  b.sw = 10 * heightmap[bounds.x + bounds.w-1 + ((bounds.y + bounds.h-1) * w)];
  b.center = 10 * heightmap[bounds.x +  mid  +  ((bounds.y + mid) * w)];

  // Set the variance for this block (the difference between the highest and lowest points.)
  SetVariance(b);

  // Expand the quadtree until we get to the max resolution.
  if (level<max_levels)
  {
    float var1 = BuildTreeNode(f, level+1, my_index, 0, nRect(bounds.x,bounds.y,cw,cw), heightmap, w);
    float var2 = BuildTreeNode(f, level+1, my_index, 1, nRect(bounds.x+mid,bounds.y,cw,cw), heightmap, w);
    float var3 = BuildTreeNode(f, level+1, my_index, 2, nRect(bounds.x,bounds.y+mid,cw,cw), heightmap, w);
    float var4 = BuildTreeNode(f, level+1, my_index, 3, nRect(bounds.x+mid,bounds.y+mid,cw,cw), heightmap, w);

    b.variance = (var1 > b.variance) ? var1 : b.variance;
    b.variance = (var2 > b.variance) ? var2 : b.variance;
    b.variance = (var3 > b.variance) ? var3 : b.variance;
    b.variance = (var4 > b.variance) ? var4 : b.variance;
  }
  /* midh was set to low */
  b.midh += b.variance/2.0;
  b.radius = qsqrt(b.variance*b.variance + bounds.w*bounds.w/4.0);

  // Store the block in the file.
  fseek(f, my_index*nBlockLen, SEEK_SET);
  fwrite(&b, nBlockLen, 1, f);

  return b.variance;
}

void nTerrain::BufferTreeNode(nBlock *b, nRect bounds)
{
  int mid = terrain_w >> 1;
  int ne = info->vertices.Push(csVector3(bounds.x-mid, b->ne, bounds.y-mid)),
      nw = info->vertices.Push(csVector3(bounds.x+bounds.w-1-mid, b->nw, bounds.y-mid)),
      se = info->vertices.Push(csVector3(bounds.x-mid, b->se, bounds.y+bounds.h-1-mid)),
      sw = info->vertices.Push(csVector3(bounds.x+bounds.w-1-mid, b->sw, bounds.y+bounds.h-1-mid)),
      center = info->vertices.Push(csVector3(bounds.x+bounds.h/2.0-mid, b->center, bounds.y+bounds.h/2.0-mid));
// printf ("ne %d %f %d\n", bounds.x-mid, b->ne, bounds.y-mid);
// printf ("nw %d %f %d\n", bounds.x+bounds.w-1-mid, b->nw, bounds.y-mid);
// printf ("se %d %f %d\n", bounds.x-mid, b->se, bounds.y+bounds.h-1-mid);
// printf ("sw %d %f %d\n", bounds.x+bounds.w-1-mid, b->sw, bounds.y+bounds.h-1-mid);
// printf ("center %f %f %f\n", bounds.x+bounds.h/2.0-mid, b->center, bounds.y+bounds.h/2.0-mid);

  float wid = (float)terrain_w;
  info->texels.Push(csVector2(bounds.x/wid, bounds.y/wid));
  info->texels.Push(csVector2((bounds.x+bounds.w-1)/wid, bounds.y/wid));
  info->texels.Push(csVector2(bounds.x/wid, (bounds.y+bounds.h-1)/wid));
  info->texels.Push(csVector2((bounds.x+bounds.w-1)/wid, (bounds.y+bounds.h-1)/wid));
  info->texels.Push(csVector2((bounds.x+bounds.h/2.0)/wid, (bounds.y+bounds.h/2.0)/wid));

  info->colors.Push(csColor(0.8,0.5,0.2));
  info->colors.Push(csColor(0.2,0.5,0.8));
  info->colors.Push(csColor(0.2,0.5,0.2));
  info->colors.Push(csColor(0.8,0.5,0.8));
  info->colors.Push(csColor(0.5,0.5,0.5));

  /** Build triangle stacks, for each block output four triangles.
   * No need to merge and split because each block is at full resolution,
   * we have no partial resolution blocks.
   */
  info->triq[0/*b->ti*/].triangles.Push(csTriangle(sw, center, se));
  info->triq[0/*b->ti*/].triangles.Push(csTriangle(nw, center, sw));
  info->triq[0/*b->ti*/].triangles.Push(csTriangle(ne, center, nw));
  info->triq[0/*b->ti*/].triangles.Push(csTriangle(se, center, ne));
}

void nTerrain::ProcessTreeNode(iRenderView *rv, unsigned int level, unsigned int parent_index, unsigned int child_num, nRect bounds)
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
  csSphere bs(csVector3 (bounds.x+mid-c, b->midh, bounds.y+mid-c), b->radius);

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
    float error_metric = b->variance / distance;

    if (error_metric<=error_metric_tolerance) {
// printf ("%d early cull\n", level);
// printf ("distance = %f\n", distance);
// printf ("error = %f\n", error_metric);
      render_this=true;
    } 
  }

  // Don't render this block, resolve to the next level.
  if (!render_this)
  {
    ProcessTreeNode(rv, level+1, my_index, 0, nRect(bounds.x,bounds.y,cw,cw));
    ProcessTreeNode(rv, level+1, my_index, 1, nRect(bounds.x+mid,bounds.y,cw,cw));
    ProcessTreeNode(rv, level+1, my_index, 2, nRect(bounds.x,bounds.y+mid,cw,cw));
    ProcessTreeNode(rv, level+1, my_index, 3, nRect(bounds.x+mid,bounds.y+mid,cw,cw));
  }
  // Render this block to the buffer for later drawing.
  else
    BufferTreeNode(b, bounds);

}

void nTerrain::BuildTree(FILE *f, float *heightmap, unsigned int w)
{
  terrain_w = w-1;
  max_levels = ilogb(w-1) - 1;

  unsigned int mid = w>>1;
  unsigned int cw = mid+1;
  unsigned int x=0, y=0;
  nBlock b;

  b.ne = 10 * heightmap[0];
  b.nw = 10 * heightmap[w-1];
  b.se = 10 * heightmap[(w-1)*w];
  b.sw = 10 * heightmap[w-1 + (w-1) * w];
  b.center = 10 * heightmap[mid + mid * w];

  SetVariance(b);

  b.midh = w; /* for this first block the w is encoded into midh */

  float var1 = BuildTreeNode(f, 1, 0, 0, nRect(x,y,cw,cw), heightmap, w);
  float var2 = BuildTreeNode(f, 1, 0, 1, nRect(x+mid,y,cw,cw), heightmap, w);
  float var3 = BuildTreeNode(f, 1, 0, 2, nRect(x,y+mid,cw,cw), heightmap, w);
  float var4 = BuildTreeNode(f, 1, 0, 3, nRect(x+mid,y+mid,cw,cw), heightmap, w);

  b.variance = (var1 > b.variance) ? var1 : b.variance;
  b.variance = (var2 > b.variance) ? var2 : b.variance;
  b.variance = (var3 > b.variance) ? var3 : b.variance;
  b.variance = (var4 > b.variance) ? var4 : b.variance;

  b.radius = qsqrt(b.variance*b.variance + w*w/4.0);

  fseek (f, 0, SEEK_SET); 
  fwrite (&b, nBlockLen, 1, f);
}

void nTerrain::AssembleTerrain(iRenderView *rv, nTerrainInfo *terrinfo)
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
  
  csSphere bs (csVector3 (0, b->variance/2.0, 0), b->radius);
  if (!rv->TestBSphere(obj2cam, bs)) return;

  float distance = ((cam-bs.GetCenter()).Norm())+0.0001;

  float error_metric = b->variance / distance;

  /* TODO: LOD doesn't currently work */
  if (error_metric <= error_metric_tolerance) {
    BufferTreeNode (b, nRect (0, 0, terrain_w+1, terrain_w+1));
  } else {

  // Buffer entire viewable terrain by first doing view culling on the block, then checking for the
  // error metric.  If the error metric fails, then we need to drop down another level. Begin that
  // process here.
  
  ProcessTreeNode(rv, 1, 0, 0, nRect(x,y,cw,cw));
  ProcessTreeNode(rv, 1, 0, 1, nRect(x+mid,y,cw,cw));
  ProcessTreeNode(rv, 1, 0, 2, nRect(x,y+mid,cw,cw));
  ProcessTreeNode(rv, 1, 0, 3, nRect(x+mid,y+mid,cw,cw));

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

void nTerrain::CreateMaterialMap(iFile *matmap, iImage *terrtex)
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

csBigTerrainObject::csBigTerrainObject(iObjectRegistry* _obj_reg, iMeshObjectFactory *_pFactory):vbufmgr(0), vbuf(0), pFactory(_pFactory), object_reg(_obj_reg), terrain(NULL), nTextures(0) 
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
  iImage *image = imageio->Load (raw, input->GetSize(), CS_IMGFMT_ANY);
  if (!image) {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR, 
    	"crystalspace.mesh.object.terrbig", 
	"Unable to load image file\n");
    return false;
  }
printf ("image w,h (%d, %d)\n", image->GetWidth(), image->GetHeight());
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
  float *heightmap = new float[image->GetSize ()];
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
  if (!terrain) {
    terrain = new nTerrain;
  }
  terrain->BuildTree (hmfp, heightmap, image->GetWidth());
  image->DecRef ();

  delete [] heightmap;
  fclose (hmfp);

  terrain->SetHeightMapFile (hm);
  InitMesh (info);
  return true;
}

void 
csBigTerrainObject::SetupVertexBuffer (iVertexBuffer *&vbuf1)
{
 if (!vbuf1)
 {
   if (!vbufmgr)
   {
     iObjectRegistry* object_reg = ((csBigTerrainObjectFactory*)pFactory)->object_reg;
     iGraphics3D* g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);

     // @@@ priority should be a parameter.
     vbufmgr = g3d->GetVertexBufferManager ();
     g3d->DecRef ();

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

  for(i=0; i<nTextures; ++i)
  {
    info->mesh[i].morph_factor = 0;
    info->mesh[i].num_vertices_pool = 1;
    info->mesh[i].use_vertex_color = false;
	info->mesh[i].do_mirror = false;
    info->mesh[i].do_morph_texels = false;
    info->mesh[i].do_morph_colors = false;
    info->mesh[i].do_fog = false;
    info->mesh[i].vertex_mode = G3DTriangleMesh::VM_WORLDSPACE;
    info->mesh[i].mixmode = CS_FX_GOURAUD;
  }
}
}  

bool 
csBigTerrainObject::DrawTest (iRenderView* rview, iMovable* movable)
{
  if (terrain)
  {
    iCamera* cam = rview->GetCamera ();

	info->vertices.SetLength( 0 );
	info->colors.SetLength( 0 );
	info->texels.SetLength( 0 );
	info->triq[0].triangles.SetLength( 0 );
    terrain->SetObjectToCamera(cam->GetTransform());
    terrain->SetCameraOrigin(cam->GetTransform().GetOrigin());
    terrain->AssembleTerrain(rview, info);

    return true;
  }

  return false;
}

void 
csBigTerrainObject::UpdateLighting (iLight** lights, int num_lights, iMovable* movable)
{
  return;
}

bool 
csBigTerrainObject::Draw (iRenderView* rview, iMovable* movable, csZBufMode zbufMode)
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
	 	      info->vertices.GetArray(), 
		      info->texels.GetArray(),
		      info->colors.GetArray(),
		      info->vertices.Length(),
		      bufcount);

  for(i=0; i<nTextures; i++)
  {
    info->mesh[i].mat_handle = terrain->GetMaterialsList()[i]->GetMaterialHandle();
	terrain->GetMaterialsList()[i]->Visit ();
    info->mesh[i].triangles = info->triq[i].triangles.GetArray();
    info->mesh[i].vertex_fog = new G3DFogInfo[info->vertices.Length()]; 
    info->mesh[i].buffers[0]=vbuf;
    info->mesh[i].num_triangles = info->triq[i].triangles.Length();
    rview->CalculateFogMesh( pG3D->GetObjectToCamera(), info->mesh[i] );
    pG3D->DrawTriangleMesh(info->mesh[i]);
		delete[] info->mesh[i].vertex_fog;
  }

  vbufmgr->UnlockBuffer(vbuf);

  return true;
}

void 
csBigTerrainObject::GetObjectBoundingBox (csBox3& bbox, int type)
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

iMeshObject* csBigTerrainObjectFactory::NewInstance ()
{
  csBigTerrainObject* pTerrObj = new csBigTerrainObject (object_reg, this);
  return (iMeshObject*)pTerrObj;
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

iMeshObjectFactory* csBigTerrainObjectType::NewFactory ()
{
  csBigTerrainObjectFactory* btf = new csBigTerrainObjectFactory (this, 
  	object_reg);
  iMeshObjectFactory* ifact = SCF_QUERY_INTERFACE (btf, iMeshObjectFactory);
  ifact->DecRef ();
  return ifact;
}
