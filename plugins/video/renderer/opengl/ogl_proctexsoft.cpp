/*
    Copyright (C) 2001 by Jorrit Tyberghein
    Copyright (C) 2000 by Samuel Humphreys
  
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
#include "csutil/scf.h"
#include "ogl_txtcache.h"
#include "ogl_txtmgr.h"
#include "ogl_proctexsoft.h"
#include "ogl_g3dcom.h"
#include "isys/system.h"
#include "ivideo/sproctxt.h"
#include "ivideo/txtmgr.h"
#include "imesh/thing/polygon.h"
#include "isys/event.h"
#include "isys/plugin.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"

#if defined(CS_OPENGL_PATH)
#include CS_HEADER_GLOBAL(CS_OPENGL_PATH,gl.h)
#else
#include <GL/gl.h>
#endif

SCF_IMPLEMENT_IBASE (csOpenGLProcSoftware)
  SCF_IMPLEMENTS_INTERFACE (iGraphics3D)
SCF_IMPLEMENT_IBASE_END;

SCF_IMPLEMENT_IBASE (csOpenGLProcSoftware2D)
  SCF_IMPLEMENTS_INTERFACE (iGraphics2D)
SCF_IMPLEMENT_IBASE_END;

//----------------------------------------------------------------------------
// Keep a private vector of soft textures generated from ogl textures
//----------------------------------------------------------------------------
struct txt_handles
{
  txt_handles (iTextureHandle *soft, iTextureHandle *ogl)
  { 
    soft_txt = soft;
    ogl_txt = ogl;
  }

  iTextureHandle *soft_txt;
  iTextureHandle *ogl_txt;
};

// A new material that is used to replace textures.
// @@@ Samuel: Look if this is ok!
class dummyMaterial : public iMaterialHandle
{
public:
  iTextureHandle* handle;
  dummyMaterial ();
  SCF_DECLARE_IBASE;
  virtual iTextureHandle* GetTexture ()
  { return handle; }
  virtual void GetFlatColor (csRGBpixel &oColor)
  { oColor.Set (0,0,0); }
  virtual void GetReflection (float &oDiffuse, float &oAmbient, float &oReflection)
  { oDiffuse = oAmbient = oReflection = 0; }
  virtual void Prepare ()
  { }
};

SCF_IMPLEMENT_IBASE (dummyMaterial)
  SCF_IMPLEMENTS_INTERFACE (iMaterialHandle)
SCF_IMPLEMENT_IBASE_END

dummyMaterial::dummyMaterial ()
{
  SCF_CONSTRUCT_IBASE (NULL);
}

class TxtHandleVector : public csVector
{
  iObjectRegistry *object_reg;
  iPluginManager* plugin_mgr;
  iTextureManager* soft_man;
public:
  // Constructor
  TxtHandleVector (iObjectRegistry *objreg, iTextureManager *stm) 
    : csVector (8, 8), object_reg (objreg), soft_man (stm)
  {
    plugin_mgr = CS_QUERY_REGISTRY (objreg, iPluginManager);
  }; 
  // Destructor
  virtual ~TxtHandleVector () { DeleteAll (); }
  // Free an item from array
  virtual bool FreeItem (csSome Item)
  { 
    ((txt_handles *)Item)->soft_txt->DecRef(); 
    ((txt_handles *)Item)->ogl_txt->DecRef();
    delete (txt_handles *)Item; return true; }
  // Find a state by referenced OpenGL texture handle
  virtual int CompareKey (csSome Item, csConstSome Key, int /*Mode*/) const
  { return ((txt_handles *)Item)->ogl_txt == (iTextureHandle *)Key ? 0 : -1; }
  // Get world state according to index
  inline txt_handles *Get (int n) const
  { return (txt_handles*)csVector::Get (n); }

  iTextureHandle *RegisterAndPrepare (iTextureHandle *ogltxt);
  void AddTextureHandles (iTextureHandle *soft, iTextureHandle *ogl);
};

//----------------------------------------------------------------------------

iTextureHandle *TxtHandleVector::RegisterAndPrepare (iTextureHandle *ogl_txt)
{
  csTextureHandle *txtmm = (csTextureHandle*)ogl_txt->GetPrivateObject();
  iImage *image = ((csTextureOpenGL*) txtmm->get_texture (0))->get_image();

  int flags = ((csTextureHandleOpenGL*)txtmm)->GetFlags ();
#ifdef CS_DEBUG
  if ((flags & CS_TEXTURE_PROC) == CS_TEXTURE_PROC) 
  {
    printf ("Big Error, attempting to register and prepare a procedural texture with the software Opengl texture vector\n");
  }
#endif
  // image gets a DecRef() in the software texture manager if procedural texture
  // flags are not set. 
  image->IncRef ();
  iTextureHandle *hstxt = soft_man->RegisterTexture (image, flags);
  // deal with key colours..
  if (ogl_txt->GetKeyColor ())
  {
    UByte r, g, b;
    ogl_txt->GetKeyColor (r, g, b);
    hstxt->SetKeyColor (true);
    hstxt->SetKeyColor (r, g, b);
  }

  Push (new txt_handles (hstxt, ogl_txt));
  hstxt->Prepare ();
  return hstxt;
}

void TxtHandleVector::AddTextureHandles (iTextureHandle *soft, 
					 iTextureHandle *ogl)
{
  Push (new txt_handles (soft, ogl));
}

//-----------------------------------------------------------------------------

csOpenGLProcSoftware::csOpenGLProcSoftware (iBase * pParent)
{ 
  SCF_CONSTRUCT_IBASE (pParent);
  tex = NULL; 
  g3d = NULL; 
  parent_g3d = NULL;
  alone_mode = true;
  head_soft_tex = NULL;
  next_soft_tex = NULL;
}

csOpenGLProcSoftware::~csOpenGLProcSoftware ()
{
  if (g3d) g3d->DecRef ();
  // remove ourselves from the linked list
  if (!head_soft_tex)
  {
    if (!next_soft_tex)
      delete txts_vector;
    else
    {
      next_soft_tex->head_soft_tex = NULL;
      ((csTextureManagerOpenGL*)
       (parent_g3d->GetTextureManager()))->head_soft_proc_tex = next_soft_tex;
    }
  }
  else
  {
    csOpenGLProcSoftware *last = head_soft_tex;
    while (last->next_soft_tex != this)
      last = last->next_soft_tex;
    last->next_soft_tex = next_soft_tex;
  }
  iSystem* sys = CS_GET_SYSTEM (object_reg);	//@@@
  sys->GetSystemEventOutlet ()->Broadcast (cscmdContextClose, (void*)dummy_g2d);
  dummy_g2d->DecRef ();
}

void csOpenGLProcSoftware::ConvertAloneMode ()
{
  alone_mode = false;
  isoft_proc->ConvertMode ();
}

bool csOpenGLProcSoftware::Prepare(
  csGraphics3DOGLCommon *parent_g3d,
  csOpenGLProcSoftware *head_soft_tex,
  csTextureHandleOpenGL *tex,
  csPixelFormat *ipfmt,
  void *buffer,bool alone_hint)
{ 
  // We generate a 32 bit pfmt taking into account endianness and whether
  // frame buffer is RGB or BGR...there must be a better way...

#if defined (CS_BIG_ENDIAN)
  if (ipfmt->RedMask > ipfmt->BlueMask)
  {
    pfmt.RedMask   = 0xff000000;
    pfmt.GreenMask = 0x00ff0000;
    pfmt.BlueMask  = 0x0000ff00;
  }
  else
  {
    pfmt.RedMask   = 0x0000ff00;
    pfmt.GreenMask = 0x00ff0000;
    pfmt.BlueMask  = 0xff000000;
  }
#else
  if (ipfmt->RedMask > ipfmt->BlueMask)
  {
    pfmt.RedMask   = 0x00ff0000;
    pfmt.GreenMask = 0x0000ff00;
    pfmt.BlueMask  = 0x000000ff;
  }
  else
  {
    pfmt.RedMask   = 0x000000ff;
    pfmt.GreenMask = 0x0000ff00;
    pfmt.BlueMask  = 0x00ff0000;
  }
#endif
  pfmt.PixelBytes = 4;
  pfmt.PalEntries = 0;
  pfmt.complete ();

  object_reg = parent_g3d->object_reg;
  CS_ASSERT (object_reg != NULL);
  iPluginManager* plugin_mgr = parent_g3d->plugin_mgr;
  this->buffer = (char*) buffer;
  this->tex = tex;
  this->parent_g3d = parent_g3d;

  tex->GetMipMapDimensions (0, width, height);

  this->head_soft_tex = head_soft_tex;
  if (!alone_hint && head_soft_tex && head_soft_tex->alone_mode)
  {
    // here we need to convert all procedural textures in the system to not
    // alone mode.
    csOpenGLProcSoftware *last = head_soft_tex;
    while (last)
    {
      last->ConvertAloneMode ();
      last = last->next_soft_tex;
    }
  }
  alone_mode = alone_hint;

  // Get an instance of the software procedural texture renderer
  iGraphics3D *soft_proc_g3d = CS_LOAD_PLUGIN (plugin_mgr, 
    "crystalspace.graphics3d.software.offscreen", NULL, iGraphics3D);
  if (!soft_proc_g3d)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.graphics3d.opengl",
	"Error creating offscreen software renderer");
    return false;
  }

  isoft_proc =(iSoftProcTexture*)SCF_QUERY_INTERFACE(soft_proc_g3d, 
						 iSoftProcTexture);
  isoft_proc->DecRef ();

  if (!isoft_proc)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
	"crystalspace.graphics3d.opengl",
    	"Error creating offscreen software renderer");
    return false;
  }
  // temporarily assign parent_g3d as g3d so that the software 3d driver
  // can get the 2d software procedural texture driver from the opengl 2d 
  // driver.
//    g3d = parent_g3d;

  iTextureHandle *soft_proc_tex = isoft_proc->CreateOffScreenRenderer 
    ((iGraphics3D*) parent_g3d, head_soft_tex ? head_soft_tex->g3d : NULL, 
     width, height, buffer, &pfmt, tex->GetFlags());

  if (!soft_proc_tex)
  {
    soft_proc_g3d->DecRef ();
    return false;
  }
  // set to correct value.
  g3d = soft_proc_g3d;

  // Get the main renderers pixel format as this is not necessarily 32bit
  csPixelFormat *main_pfmt = parent_g3d->GetDriver2D()->GetPixelFormat ();
  dummy_g2d = (iGraphics2D*) new csOpenGLProcSoftware2D (g3d, main_pfmt);

  CS_ASSERT (object_reg != NULL);
  if (!head_soft_tex)
    txts_vector = new TxtHandleVector (object_reg, g3d->GetTextureManager ());
  else
    txts_vector = head_soft_tex->txts_vector;

  // Register ourselves.
  txts_vector->AddTextureHandles (soft_proc_tex, (iTextureHandle*)tex);

  // Here we add ourselves to the end of the linked list of software textures
  if (head_soft_tex)
  {
    csOpenGLProcSoftware *last = head_soft_tex;
    while (last->next_soft_tex)
      last = last->next_soft_tex;
    last->next_soft_tex = this;
  }

#if CS_DEBUG
   printf ("GL software procedural texture\n");
#endif
  return true;
}

void csOpenGLProcSoftware::Print (csRect *area)
{
  glEnable (GL_TEXTURE_2D);
  csGraphics3DOGLCommon::SetGLZBufferFlags (CS_ZBUF_NONE);
  csGraphics3DOGLCommon::SetupBlend (CS_FX_COPY, 0, false);
  glDisable (GL_ALPHA_TEST);

  csTxtCacheData *tex_data = (csTxtCacheData*) tex->GetCacheData();
  if (tex_data)
  {
    // Texture is in tha cache, update texture directly.
    //glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glBindTexture (GL_TEXTURE_2D, tex_data->Handle);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
		    width, height,
		    GL_RGBA, GL_UNSIGNED_BYTE, buffer);
  }
  else
  {
    // Not in cache. Sharing buffer with texture so do nothing
  }
  g3d->Print (area);
}

iGraphics2D *csOpenGLProcSoftware::GetDriver2D ()
{ 
  return dummy_g2d; 
}

bool csOpenGLProcSoftware::BeginDraw (int DrawFlags)
{
  return g3d->BeginDraw (DrawFlags);
}

void csOpenGLProcSoftware::FinishDraw ()
{
  g3d->FinishDraw ();
}

void csOpenGLProcSoftware::DrawPolygon (G3DPolygonDP& poly)
{
  G3DPolygonDP cpoly;
  memcpy (&cpoly, &poly, sizeof (G3DPolygonDP));
  txt_handles *handles;
  dummyMaterial dmat;
  cpoly.mat_handle = &dmat;
  int idx = txts_vector->FindKey ((void*)poly.mat_handle->GetTexture ());
  if (idx == -1)
  {
    dmat.handle = txts_vector->RegisterAndPrepare (poly.mat_handle->GetTexture ());
    handles = txts_vector->Get(txts_vector->FindKey ((void*)poly.mat_handle->GetTexture ()));
  }
  else
  {
    handles = txts_vector->Get (idx);
    dmat.handle = (iTextureHandle*) handles->soft_txt;
  }
  g3d->DrawPolygon (cpoly);
}

void csOpenGLProcSoftware::DrawPolygonDebug (G3DPolygonDP& poly)
{
#if 0
  iMaterialHandle *ogl_mat_handle = poly.mat_handle;
  int idx = txts_vector->FindKey ((void*)ogl_mat_handle);
  if (idx == -1)
    poly.mat_handle = txts_vector->RegisterAndPrepare (ogl_mat_handle);
  else
    poly.mat_handle = (iMaterialHandle*) txts_vector->Get (idx)->soft_txt;
  g3d->DrawPolygonDebug (poly);
  poly.mat_handle = ogl_mat_handle;
#else
  (void) poly;
#endif
}

void csOpenGLProcSoftware::DrawLine (const csVector3& v1, const csVector3& v2, float fov, int color)
{ 
  g3d->DrawLine (v1, v2, fov, color); 
}

void csOpenGLProcSoftware::DrawPolygonFX (G3DPolygonDPFX& poly)
{ 
  dummyMaterial dmat;
  iMaterialHandle* old_handle = poly.mat_handle;
  poly.mat_handle = &dmat;
  int idx = txts_vector->FindKey ((void*)old_handle->GetTexture ());
  if (idx == -1)
    dmat.handle = txts_vector->RegisterAndPrepare (old_handle->GetTexture ());
  else
    dmat.handle = (iTextureHandle*) txts_vector->Get (idx)->soft_txt;
  g3d->DrawPolygonFX (poly);
  poly.mat_handle = old_handle;
}

void csOpenGLProcSoftware::DrawTriangleMesh (G3DTriangleMesh& mesh)
{
  G3DTriangleMesh cmesh;
  memcpy (&cmesh, &mesh, sizeof(G3DTriangleMesh));
  dummyMaterial dmat;
  cmesh.mat_handle = &dmat;
  int idx = txts_vector->FindKey ((void*)mesh.mat_handle->GetTexture ());
  if (idx == -1)
    dmat.handle = txts_vector->RegisterAndPrepare (
    	mesh.mat_handle->GetTexture ());
  else
    dmat.handle =(iTextureHandle*) txts_vector->Get (idx)->soft_txt;
  g3d->DrawTriangleMesh (cmesh);
}

void csOpenGLProcSoftware::DrawPolygonMesh (G3DPolygonMesh& mesh)
{ 
  G3DPolygonMesh cmesh;
  memcpy (&cmesh, &mesh, sizeof(G3DPolygonMesh));
  dummyMaterial* dmat = NULL;
  int idx;

  if (!mesh.master_mat_handle)
  {
    dmat = new dummyMaterial[mesh.num_polygons];
	int i;
    for (i = 0; i < mesh.num_polygons; i++)
    {
      cmesh.mat_handle[i] = &dmat[i];
      idx = txts_vector->FindKey ((void*)mesh.mat_handle[i]->GetTexture ());
      if (idx == -1)
	dmat[i].handle = txts_vector->RegisterAndPrepare(mesh.mat_handle[i]->GetTexture ());
      else
	dmat[i].handle = (iTextureHandle*) txts_vector->Get (idx)->soft_txt;
    }
  }
  else
  {
    dmat = new dummyMaterial[1];
    idx = txts_vector->FindKey ((void*)mesh.master_mat_handle->GetTexture ());
    cmesh.master_mat_handle = &dmat[0];
    if (idx == -1)
      dmat[0].handle = txts_vector->RegisterAndPrepare(mesh.master_mat_handle->GetTexture ());
    else
      dmat[0].handle = (iTextureHandle*) txts_vector->Get (idx)->soft_txt;
  }
  g3d->DrawPolygonMesh (cmesh);
  delete[] dmat;
}

void csOpenGLProcSoftware::OpenFogObject (CS_ID id, csFog* fog)
{ 
  g3d->OpenFogObject (id, fog); 
}

void csOpenGLProcSoftware::DrawFogPolygon (CS_ID id, G3DPolygonDFP& poly, int fogtype)
{ 
  g3d->DrawFogPolygon (id, poly, fogtype);
}

void csOpenGLProcSoftware::CloseFogObject (CS_ID id)
{ 
  g3d->CloseFogObject (id); 
}

bool csOpenGLProcSoftware::SetRenderState (G3D_RENDERSTATEOPTION op, long val)
{ 
  return g3d->SetRenderState (op, val); 
}

long csOpenGLProcSoftware::GetRenderState (G3D_RENDERSTATEOPTION op)
{ 
  return g3d->GetRenderState (op); 
}

csGraphics3DCaps *csOpenGLProcSoftware::GetCaps ()
{ 
  return g3d->GetCaps (); 
}

uint32 *csOpenGLProcSoftware::GetZBuffAt (int x, int y)
{ 
  return g3d->GetZBuffAt (x, y); 
}

float csOpenGLProcSoftware::GetZBuffValue (int x, int y)
{ 
  return g3d->GetZBuffValue (x, y); 
}

void csOpenGLProcSoftware::DumpCache ()
{ 
  g3d->DumpCache (); 
}

void csOpenGLProcSoftware::ClearCache ()
{ 
  g3d->ClearCache (); 
}

void csOpenGLProcSoftware::RemoveFromCache (iPolygonTexture* poly_texture)
{ 
  g3d->RemoveFromCache (poly_texture); 
}

void csOpenGLProcSoftware::SetPerspectiveCenter (int x, int y)
{ 
  g3d->SetPerspectiveCenter (x, y); 
}

void csOpenGLProcSoftware::GetPerspectiveCenter (int& x, int& y)
{ 
  g3d->GetPerspectiveCenter (x, y); 
}

void csOpenGLProcSoftware::SetPerspectiveAspect (float aspect)
{ 
  g3d->SetPerspectiveAspect (aspect); 
}

float csOpenGLProcSoftware::GetPerspectiveAspect ()
{ 
  return g3d->GetPerspectiveAspect ();
}

void csOpenGLProcSoftware::SetObjectToCamera (csReversibleTransform* o2c)
{ 
  g3d->SetObjectToCamera (o2c); 
}

const csReversibleTransform& csOpenGLProcSoftware::GetObjectToCamera ()
{ 
  return g3d->GetObjectToCamera (); 
}

// Here we return the main opengl texture manager as all textures are originally
// registered here.
iTextureManager *csOpenGLProcSoftware::GetTextureManager ()
{ 
  return parent_g3d->GetTextureManager (); 
}

iHalo *csOpenGLProcSoftware::CreateHalo (float iR, float iG, float iB, 
	                         unsigned char *iAlpha, int iWidth, int iHeight)
{ 
  return g3d->CreateHalo (iR, iG, iB, iAlpha, iWidth, iHeight); 
}

void csOpenGLProcSoftware::DrawPixmap (iTextureHandle *hTex, int sx, 
  int sy, int sw, int sh,int tx, int ty, int tw, int th, uint8 Alpha)
{ 
  iTextureHandle *soft_txt_handle;
  int idx = txts_vector->FindKey ((void*)hTex);
  if (idx == -1)
    soft_txt_handle = txts_vector->RegisterAndPrepare (hTex);
  else
    soft_txt_handle = (iTextureHandle*)txts_vector->Get (idx)->soft_txt;
  g3d->DrawPixmap (soft_txt_handle, sx, sy, sw, sh, tx, ty, tw, th, Alpha); 
}
