#ifndef SGIIMAGE_H
#define SGIIMAGE_H

#include "csgfx/csimage.h"
#include "igraphic/imageio.h"
#include "isys/plugin.h"
#include "iutil/databuff.h"
#include "csutil/csvector.h"

/**
 * The SGI image file format loader.
 */
class csSGIImageIO : public iImageIO
{
 protected:
  csVector formats;

 public:
  SCF_DECLARE_IBASE;

  csSGIImageIO (iBase *pParent);
  virtual ~csSGIImageIO () {}

  virtual const csVector& GetDescription ();
  virtual iImage *Load (UByte* iBuffer, ULong iSize, int iFormat);
  virtual void SetDithering (bool iEnable);
  virtual iDataBuffer *Save (iImage *image, const char *mime = NULL); 
  virtual iDataBuffer *Save (iImage *image, iImageIO::FileFormatDescription *format = NULL);

  struct eiPlugIn : public iPlugIn
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSGIImageIO);
    virtual bool Initialize (iSystem*) { return true; }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugIn;
};

class ImageSGIFile : public csImageFile
{
  friend class csSGIImageIO;
  // Read SGI header and get the number of planes (only 3 or 4 is supported)
  UInt readHeader(UByte *buf);
  // Read table with offsets
  void loadSGITables(UByte *in,ULong *out,int size);
  // Decode an RLE encoded line
  int decode_rle (UByte *src, ULong length, UByte *dst);

private:
  /// Initialize the image object
  ImageSGIFile (int iFormat) : csImageFile (iFormat) { };
  /// Try to read the SGI file from the buffer and return success status
  bool Load (UByte* iBuffer, ULong iSize);
};

#endif
