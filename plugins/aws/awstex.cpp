
#include "cssysdef.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/vfs.h"
#include "iutil/plugin.h"
#include "ivaria/reporter.h"
#include "igraphic/imageio.h"
#include "igraphic/image.h"
#include "ivideo/txtmgr.h"
#include "ivideo/texture.h"
#include "aws/awstex.h"
#include "aws/awsadler.h"
#include <string.h>
#include <stdio.h>


const bool DEBUG_GETTEX = false;

static unsigned long
NameToId(char *txt)
{
  return aws_adler32(aws_adler32(0, NULL, 0), (unsigned char *)txt, strlen(txt));
}

awsTextureManager::awsTexture::~awsTexture ()
{
  img->DecRef ();
  tex->DecRef ();
}

awsTextureManager::awsTextureManager():loader(NULL), txtmgr(NULL), vfs(NULL), object_reg(NULL)
{
 // empty
}
  
awsTextureManager::~awsTextureManager()
{
  for (int i=0; i < textures.Length (); i++)
    delete (awsTexture*)textures.Get (i);

  SCF_DEC_REF (loader);
  SCF_DEC_REF (vfs);
  SCF_DEC_REF (txtmgr);
}

void
awsTextureManager::Initialize(iObjectRegistry* obj_reg)
{
  object_reg=obj_reg;

  if (!obj_reg) printf("aws-debug:  bad obj_reg (%s)\n", __FILE__);
  if (!object_reg) printf("aws-debug:  bad object_reg (%s)\n", __FILE__);

  loader = CS_QUERY_REGISTRY (object_reg, iImageIO);
  vfs    = CS_QUERY_REGISTRY (object_reg, iVFS);
  
  if (!loader)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR, "crystalspace.awsprefs",
      	"could not load the image loader plugin. This is a fatal error.");
  }

  if (!vfs)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR, "crystalspace.awsprefs",
      	"could not load the VFS plugin. This is a fatal error.");
  }

  if (!vfs->Mount("/aws", "./data/awsdef.zip"))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR, "crystalspace.awsprefs",
      	"could not mount the default aws skin (awsdef.zip)aws.");
  }
}

iTextureHandle * 
awsTextureManager::GetTexture(char *name, char *filename, bool replace)
{
  unsigned long id = NameToId(name);
  return GetTexturebyID(id, filename, replace);
}

iTextureHandle * 
awsTextureManager::GetTexturebyID(unsigned long id, char *filename, bool replace)
{
  awsTexture *awstxt=NULL;
  bool        txtfound=false;
  

 /*  Perform a lookup on the texture list.  We may consider doing this a little more
  * optimally in the future, like subclassing csVector and overriding CompareKey for
  * the QuickSort algorithm */

  if (DEBUG_GETTEX) printf("aws-debug: (%s) texture count is: %d\n", __FILE__, textures.Length());

  int i;
  for(i=0; i<textures.Length() && txtfound==false; ++i)
  {
    awsTexture *awstxt = (awsTexture *)textures[i];

    if (DEBUG_GETTEX) printf("aws-debug: (%s) texture is: %p\n", __FILE__, awstxt->tex);

    if (awstxt && id == awstxt->id) 
    {
      if (replace && filename !=NULL) 
        txtfound=true;
      else
        return awstxt->tex;
    }
  }
  
  if (!txtfound && filename == NULL) return NULL;
  if (!txtfound) awstxt=NULL;

 /*  If we have arrived here, then we know that the texture does not exist in the cache.
  * Therefore, we will now attempt to load it from the disk, register it, and pass back a handle.
  * If this fails, we'll return NULL.
  */

  if (DEBUG_GETTEX)
  {
    if (txtmgr==NULL) printf("aws-debug: GetTexturebyID (%s) no texture manager.\n", __FILE__);
    if (vfs==NULL)    printf("aws-debug: GetTexturebyID (%s) no vfs.\n", __FILE__);
    if (loader==NULL) printf("aws-debug: GetTexturebyID (%s) no loader.\n", __FILE__);

    if (!txtmgr || !vfs || !loader) return NULL;
  }

  int Format = txtmgr->GetTextureFormat();
  
  iImage *ifile = NULL;
  iDataBuffer *buf = vfs->ReadFile (filename);

  if (buf==NULL || buf->GetSize() == 0)
  {
    if (buf) buf->DecRef ();

    csReport(object_reg, CS_REPORTER_SEVERITY_ERROR, "crystalspace.awsprefs",
    	      "Could not open image file '%s' on VFS!", filename);

    return NULL;
  }


  ifile = loader->Load (buf->GetUint8 (), buf->GetSize (), Format);
  buf->DecRef ();

  if (!ifile)
  {
    csReport(object_reg, CS_REPORTER_SEVERITY_ERROR, "crystalspace.awsprefs",
              "Could not load image '%s'. Unknown format or wrong extension!", filename);

    return NULL;
  }

 /*  At this point, we have loaded the file from the disk, and all we're doing now is creating
  * a texture handle to the image.  The texture handle is necessary to draw a pixmap with 
  * iGraphics3D::DrawPixmap()
  */
  
  if (awstxt==NULL)
  {
    awstxt = new awsTexture;
    memset(awstxt, 0, sizeof(awsTexture));
  }
  else
  {
    txtmgr->UnregisterTexture(awstxt->tex);

    awstxt->img->DecRef();
    awstxt->tex->DecRef();
  }

  awstxt->img = ifile;
  awstxt->tex = txtmgr->RegisterTexture(ifile, CS_TEXTURE_2D | CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS);
  awstxt->id = id;

  // Post load work...
  awstxt->tex->SetKeyColor(255,0,255);
  awstxt->tex->Prepare();

  textures.Push(awstxt);

  return awstxt->tex;
}

void 
awsTextureManager::SetTextureManager(iTextureManager *newtxtmgr)
{
  if (txtmgr && newtxtmgr)
  {
    UnregisterTextures();
    txtmgr->DecRef();
  }
  
  if (newtxtmgr)
  {
    txtmgr = newtxtmgr;
    txtmgr->IncRef();
    RegisterTextures();
  }
}

void
awsTextureManager::RegisterTextures()
{

}

void
awsTextureManager::UnregisterTextures()
{

}
