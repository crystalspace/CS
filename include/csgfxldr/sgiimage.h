#ifndef SGIIMAGE_H
#define SGIIMAGE_H

#include "csgfxldr/csimage.h"

/**
 * The SGI Image Loader.
 */
class csSGIImageLoader : public csImageLoader
{
protected:
  /// Try to load the image
  virtual csImageFile* LoadImage (UByte* iBuffer, ULong iSize, int iFormat);
};

class ImageSGIFile : public csImageFile
{
  friend class csSGIImageLoader;
  // Read and validate SGI header
  bool readHeader(UByte *buf,UInt numplanes);
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
