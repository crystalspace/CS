#ifndef SGIIMAGE_H
#define SGIIMAGE_H

#include "csgfxldr/csimage.h"

class csSGIImageLoader;
class ImageSGIFile : public csImageFile
{
  ///
  friend class csImageFile;	// For constructor
  friend class csSGIImageLoader;

private:
  // Read the SGI file from the buffer.
  ImageSGIFile (UByte* buf, long size);
  // Load an RLE encoded 3 bpp picture
  bool loadRGBRLE(UByte *in,RGBPixel *image);

public:
  ///
  virtual ~ImageSGIFile ();

protected:
  // Routines that can be used by AlphaMapSGIFile
  // Get a long value
  ULong getLong(UByte *buf);
  // Get a short value
  UShort getShort(UByte *buf);
  // Read and validate SGI header
  bool readHeader(UByte *buf,UInt numplanes);
  // Read table with offsets
  void loadSGITables(UByte *in,ULong *out,int size);

  // Decode an RLE encoded line
  UInt decode_rle(UByte *buf,ULong offset,ULong length,UByte *out,UByte *tmp);
};

/**
 * The SGI Image Loader.
 */

class csSGIImageLoader : public csImageLoader
{
protected:
  ///
  virtual csImageFile* LoadImage (UByte* buf, ULong size);
  virtual AlphaMapFile* LoadAlphaMap(UByte* buf,ULong size);

public:
  ///
  virtual const char* GetName() const
  { return "SGI"; }
  ///
  virtual const char* GetDescription() const 
  { return "SGI Image Format"; }
};

#endif
