#ifndef __CS_SGIIMAGE_H__
#define __CS_SGIIMAGE_H__

#include "csgfx/memimage.h"
#include "igraphic/imageio.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/databuff.h"

/**
 * The SGI image file format loader.
 */
class csSGIImageIO : public iImageIO
{
 protected:
  csImageIOFileFormatDescriptions formats;

 public:
  SCF_DECLARE_IBASE;

  csSGIImageIO (iBase *pParent);
  virtual ~csSGIImageIO ();

  virtual const csImageIOFileFormatDescriptions& GetDescription ();
  virtual csPtr<iImage> Load (iDataBuffer* buf, int iFormat);
  virtual void SetDithering (bool iEnable);
  virtual csPtr<iDataBuffer> Save (iImage* image, const char* mime,
    const char* extraoptions);
  virtual csPtr<iDataBuffer> Save (iImage *image,
  	iImageIO::FileFormatDescription* format,
    	const char* extraoptions);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSGIImageIO);
    virtual bool Initialize (iObjectRegistry*) { return true; }
  } scfiComponent;
};

class ImageSGIFile : public csImageMemory
{
  friend class csSGIImageIO;
  // Read SGI header and get the number of planes (only 3 or 4 is supported)
  uint readHeader(uint8 *buf);
  // Read table with offsets
  void loadSGITables(uint8 *in,uint32 *out,int size);
  // Decode an RLE encoded line
  int decode_rle (uint8 *src, uint32 length, uint8 *dst);

private:
  /// Initialize the image object
  ImageSGIFile (int iFormat) : csImageMemory (iFormat) { };
  /// Try to read the SGI file from the buffer and return success status
  bool Load (uint8* iBuffer, size_t iSize);
};

#endif // __CS_SGIIMAGE_H__
