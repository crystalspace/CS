/*
    Copyright (C) 2000 by Norman Kraemer
  
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

#ifndef ALPHAMAP_H
#define ALPHAMAP_H

#include "csutil/scf.h"
#include "ivideo/texture.h"
#include "iengine/texture.h"
#include "cstypes.h"
#include "csutil/util.h"

/** 
 * A texture handle for alphamaps
 */
class csGlideAlphaMap : public iTextureHandle
{
protected:
  int width, height;
  int realwidth, realheight;
  unsigned char* alpha;
  UByte mean_alpha;
  void* pCacheData;

  void compute_mean_alpha ()
  { int i; long a, size;
    for ( i=0, a=0, size=width*height; i < size; i++, a += *alpha );
    mean_alpha = a / size;
  }
  
public:
  csGlideAlphaMap ( unsigned char* map, int width, int height )
  { 
    CONSTRUCT_IBASE( NULL );
    pCacheData = NULL;
    alpha = NULL;
    realwidth = width; realheight = height;
    this->width = csFindNearestPowerOf2( width );
    this->height = csFindNearestPowerOf2( height );
//    printf("nearest: %d %d\n", this->width, this->height);
//    if ( realwidth < this->width ) this->width >>= 1;
//    if ( realheight < this->height ) this->height >>= 1;
     alpha = new unsigned char[ this->width*this->height ] ;
    memset( alpha, this->width*this->height, 0 );
//    int xoff = (this->width - realwidth)/2;
//    int yoff = (this->height - realheight)/2;
    int i;
    for ( i=0; i < realheight; i++ )
      memcpy( alpha + i*this->width, map + i*realwidth, realwidth );
    compute_mean_alpha ();
//    savemap();
  }
  
  virtual ~csGlideAlphaMap ()
  { if ( alpha ) delete [] alpha; }
  
  void GetRealDimensions( int& w, int& h )
  { w = realwidth; h = realheight; }
    
  DECLARE_IBASE;
  
  /// The following doesnt matter since we have an alphamap anyway
  virtual void SetKeyColor (bool ){};
  virtual void SetKeyColor (UByte, UByte, UByte ){};
  virtual bool GetKeyColor (){ return true; }

  /// Get the transparent color
  virtual void GetKeyColor (UByte &red, UByte &green, UByte &blue)
  { GetMeanColor ( red, green, blue ); }
  /**
   * Get the dimensions for a given mipmap level (0 to 3).
   * If the texture was registered just for 2D usage, mipmap levels above
   * 0 will return false.
   */
  virtual bool GetMipMapDimensions (int mipmap, int &mw, int &mh)
  {
    if ( !mipmap )
    {
      mw = width;
      mh = height;
    }
    return !mipmap;
  }

  /// Get the alpha map data.
  void *GetAlphaMapData ()
  { return alpha; }

  bool GetAlphaMap ()
  { return true; }

  /// Get the mean color.
  virtual void GetMeanColor (UByte &red, UByte &green, UByte &blue)
  { red = green = blue = mean_alpha; }

  /// Get data associated internally with this texture by texture cache
  virtual void *GetCacheData ()
  { return pCacheData; }

  /// Set data associated internally with this texture by texture cache
  virtual void SetCacheData (void *d)
  { pCacheData = d ; }

  /**
   * Query the private object associated with this handle.
   * For internal usage by the 3D driver.
   */
  virtual void *GetPrivateObject ()
  { return (csGlideAlphaMap*)this; }

  virtual iGraphics3D* GetProcTextureInterface () { return NULL;}
  virtual void ProcTextureSync () {}    
  void savemap();
  int GetFlags () { return 0; }
  virtual void Prepare (){}
};

#endif //ALPHAMAP_H
