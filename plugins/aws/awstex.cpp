
#include "cssysdef.h"
#include "isys/plugin.h"
#include "isys/vfs.h"
#include "ivaria/reporter.h"
#include "igraphic/imageio.h"
#include "igraphic/image.h"
#include "ivideo/txtmgr.h"


#include "awstex.h"
#include <string.h>

awsTextureManager::awsTextureManager():loader(NULL)
{
 // empty
}
  
awsTextureManager::~awsTextureManager()
{
  if (loader) loader->DecRef();
}

void
awsTextureManager::Initialize(iObjectRegistry* obj_reg)
{
  object_reg=obj_reg;

  iPluginManager* plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  
  loader = CS_QUERY_PLUGIN_ID(plugin_mgr, CS_FUNCID_IMGLOADER, iImageIO);
  vfs    = CS_QUERY_PLUGIN_ID(plugin_mgr, CS_FUNCID_VFS, iVFS);
  
  //loader = CS_QUERY_REGISTRY (object_reg, iImageIO);

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
}

iTextureHandle * 
awsTextureManager::GetTexture(char *name, char *filename)
{
 /*  Perform a lookup on the texture list.  We may consider doing this a little more
  * optimally in the future, like subclassing csVector and overriding CompareKey for
  * the QuickSort algorithm */

  for(int i=0; i<textures.Length(); ++i)
  {
    awsTexture *awstxt = (awsTexture *)textures[i];

    if (strcmp(name, awstxt->name)) return awstxt->tex;
  }
  
 /*  If we have arrived here, then we know that the texture does not exist in the cache.
  * Therefore, we will now attempt to load it from the disk, register it, and pass back a handle.
  * If this fails, we'll return NULL.
  */
  
  int Format = txtmgr->GetTextureFormat();

  iImage *ifile = NULL;
  iDataBuffer *buf = vfs->ReadFile (filename);

  if (!buf || !buf->GetSize ())
  {
    if (buf) buf->DecRef ();

    //csReportV(object_reg, CS_REPORTER_SEVERITY_ERROR, "crystalspace.awsprefs",
    //	      "Could not open image file '%s' on VFS!", filename);

    return NULL;
  }

  ifile = loader->Load (buf->GetUint8 (), buf->GetSize (), Format);
  buf->DecRef ();

  if (!ifile)
  {
    //csReportV(object_reg, CS_REPORTER_SEVERITY_ERROR, "crystalspace.awsprefs",
    //          "Could not load image '%s'. Unknown format or wrong extension!", filename);

    return NULL;
  }

  ifile->SetName(name);


 /*  At this point, we have loaded the file from the disk, and all we're doing now is creating
  * a texture handle to the image.  The texture handle is necessary to draw a pixmap with 
  * iGraphics3D::DrawPixmap()
  */
  
  awsTexture *awstxt = new awsTexture;

  awstxt->img = ifile;
  awstxt->tex = txtmgr->RegisterTexture(ifile, CS_TEXTURE_2D);
  awstxt->name = strdup(name);

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
