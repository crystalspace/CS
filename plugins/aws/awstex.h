#ifndef __AWS_TEXTURE_MANAGER__
# define __AWS_TEXTURE_MANAGER__

# include "csutil/csvector.h"

struct iString;
struct iObjectRegistry;
struct iReporter;
struct iTextureHandle;
struct iImageIO;
struct iVFS;
struct iImage;

/**
 *
 *  This class embeds a normal texture manager, and keeps track of all the textures currently
 * in use by the windowing system.  This includes bitmaps for buttons, etc.  When the skin
 * changes, it unloads all the skin textures currently being used.  Then it is ready to demand-load
 * new ones.
 *
 */
class awsTextureManager
{
  /// this contains a reference to our loader.
  iImageIO *loader;

  /// this contains a reference to our texture manager
  iTextureManager *txtmgr;

  /// this contains a reference to the VFS plugin
  iVFS *vfs;

  /// contains a reference to the object registry
  iObjectRegistry *object_reg;

  /// list of textures loaded
  csBasicVector textures;

  struct awsTexture
  {
    ~awsTexture();
    iImage *img;
    iTextureHandle *tex;
    unsigned long id;
  };
private:
  /// registers all currently loaded textures with the texture manager
  void RegisterTextures ();

  /// unregisters all currently loaded textures with the texture manager
  void UnregisterTextures ();
public:
  /// empty constructor
  awsTextureManager ();

  /// de-inits
  ~awsTextureManager ();

  /** Get's a reference to and iLoader. */
  void Initialize (iObjectRegistry *object_reg);

  /** Get's a texture.  If the texture is already cached, it returns the cached texture.
   * If the texture has not been cached, and a filename is specified, the file is loaded.
   * If the file cannot be found, or no file was specified, NULL is returned. */
  iTextureHandle *GetTexture (
                    char *name,
                    char *filename = NULL,
                    bool replace = false,
                    unsigned char key_r = 255,
                    unsigned char key_g = 0,
                    unsigned char key_r = 255
                    );

  /** Get's a texture.  If the texture is already cached, it returns the cached texture.
  * If the texture has not been cached, and a filename is specified, the file is loaded.
  * If the file cannot be found, or no file was specified, NULL is returned. This variety
    uses the id directly, in case you have it.  Mostly used internally by AWSPrefManager. */
  iTextureHandle *GetTexturebyID (
                    unsigned long id,
                    char *filename = NULL,
                    bool replace = false,
                    unsigned char key_r = 255,
                    unsigned char key_g = 0,
                    unsigned char key_r = 255
                    );

  /** Changes the texture manager: unregisters all current textures, and then re-registers them
   * with the new manager */
  void SetTextureManager (iTextureManager *txtmgr);

  /** Retrieves the texture manager that we are currently using */
  iTextureManager *GetTextureManager () { return txtmgr; }
};
#endif
