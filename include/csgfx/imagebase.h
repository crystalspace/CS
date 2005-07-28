/*
    Copyright (C) 2005 by Jorrit Tyberghein
	      (C) 2005 by Frank Richter

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

/**\file
 * Base class for iImage implementations. Cannot be instantiated itself.
 */
 
/**\addtogroup gfx
 * @{ 
 */

#ifndef __CS_CSGFX_IMAGEBASE_H__
#define __CS_CSGFX_IMAGEBASE_H__

#include "csextern.h"

#include "csutil/databuf.h"

#include "igraphic/image.h"


/**
 * Base class for iImage implementations. Cannot be instantiated itself.
 * \remark SCF_CONSTRUCT_IBASE() / SCF_DESTRUCT_IBASE() invocation has to
 *  be done by descending classes!
 */
class csImageBase : public iImage
{
protected:
  /// Name of the image file.
  char* fName;
  /// Create new instance.
  csImageBase() : fName(0) { }
public:
  virtual ~csImageBase() { delete[] fName; }

  /* Commented out: should be implemented by all descendants.
  virtual const void *GetImageData () { return 0; }
  virtual int GetWidth () const { return 0; }
  virtual int GetHeight () const { return 0; }
  */
  // Most images are 2D, so provide a sensible default
  virtual int GetDepth () const { return 1; }

  virtual void SetName (const char *iName)
  {
    delete[] fName; fName = csStrNew (iName);
  }
  virtual const char *GetName () const { return fName; }

  /* Commented out: should be implemented by all descendants.
  virtual int GetFormat () const { return 0; }
  */
  virtual const csRGBpixel* GetPalette () { return 0; }
  virtual const uint8* GetAlpha () { return 0; }

  virtual bool HasKeyColor () const { return false; }
  CS_DEPRECATED_METHOD virtual bool HasKeycolor () const
  { return HasKeyColor(); }

  virtual void GetKeyColor (int & /*r*/, int & /*g*/, int & /*b*/) const { }
  CS_DEPRECATED_METHOD virtual void GetKeycolor (int &r, int &g, int &b) const
  { GetKeyColor (r, g, b); }

  virtual uint HasMipmaps () const { return 0; }
  virtual csRef<iImage> GetMipmap (uint num) 
  { return (num == 0) ? this : 0; }

  virtual const char* GetRawFormat() const { return 0; }
  virtual csRef<iDataBuffer> GetRawData() const { return 0; }
  virtual csImageType GetImageType() const { return csimg2D; }
  virtual uint HasSubImages() const { return 0; }
  virtual csRef<iImage> GetSubImage (uint num) 
  { return (num == 0) ? this : 0; }
};

/** @} */

#endif // __CS_CSGFX_IMAGEBASE_H__
