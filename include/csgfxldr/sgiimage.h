#ifndef SGIIMAGE_H
#define SGIIMAGE_H

#include "csgfxldr/csimage.h"

class SGIImageLoader;
class ImageSGIFile : public ImageFile
{
  ///
  friend class ImageFile;	// For constructor
  friend class SGIImageLoader;

private:
  // Read the SGI file from the buffer.
  ImageSGIFile (UByte* buf, long size);
  //Load an RLE encoded 3 bpp picture
  bool loadRGBRLE(UByte *in,RGBPixel *image);
public:
  ///
  virtual ~ImageSGIFile ();
protected:	//Routines that can be used by AlphaMapSGIFile
	ULong getLong(UByte *buf);	//Get a long value
	UShort getShort(UByte *buf);	//Get a short value
	bool readHeader(UByte *buf,UInt numplanes);	//Read and validate SGI header
	void loadSGITables(UByte *in,ULong *out,int size);	//Read table with offsets

	//Decode an RLE encoded line
	UInt decode_rle(UByte *buf,ULong offset,ULong length,UByte *out,UByte *tmp);
};

/**
 * The SGI Image Loader.
 */

class SGIImageLoader : public ImageLoader
{
protected:
  ///
  virtual ImageFile* LoadImage (UByte* buf, ULong size);
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
