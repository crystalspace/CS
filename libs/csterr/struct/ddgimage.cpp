/*
    Copyright (C) 1997, 1998, 1999, 2000 by Alex Pfaffe
    (Digital Dawn Graphics Inc)
  
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
// ----------------------------------------------------------------------
// General includes
#include <stdio.h>
#include <string.h>

#ifdef WIN32
#include <io.h>
#include <fcntl.h>
#endif

#ifdef DDGPNG
#include <png.h>
#endif

#include "util/ddgerror.h"
#include "struct/ddgimage.h"

#ifdef DDGPNG
static int check_if_png(FILE *fp);
#endif

unsigned char*
read_rgb_texture(const char *name, int *width, int *height, int *components);
unsigned char*
read_alpha_texture(const char *name, int *width, int *height);

unsigned int ddgImage::_maxval[5] = {0,0xFF,0xFFFF,0xFFFFF,0xFFFFFFFF};
// ----------------------------------------------------------------------

ddgImage::~ddgImage(void)
{
	ddgMemoryFree(unsigned char,_cols*_rows*_channels*(_pixbuffer?1:0)); 
    delete [] _pixbuffer;
}

bool ddgImage::readFile(ddgStr *filename)
{
    if (readBW(filename->s) 
     && readRGB(filename->s)
     && readTGA(filename->s)
     && readPNG(filename->s)
     && _readBWold(filename->s))
        return ddgFailure;
    return ddgSuccess;
}

bool ddgImage::allocate( unsigned short r, unsigned short  c, unsigned short ch )
{
    unsigned char *newbuf = new unsigned char[c*r*(ch?ch:_channels)];
	ddgAsserts(newbuf,"Failed to Allocate memory");
    ddgMemorySet(unsigned char,c*r*(ch?ch:_channels));
    if (!newbuf)
        return ddgFailure;

	ddgMemoryFree(unsigned char,_cols*_rows*_channels*(_pixbuffer?1:0)); 
    if (ch) _channels = ch;
    _cols = c;
    _rows = r;
    delete [] _pixbuffer;
    _pixbuffer = newbuf;

    return ddgSuccess;
}

// ----------------------------------------------------------------------
// load:
bool ddgImage::readMemory( unsigned char *buf )
{
    int i;
    for (i=0; i < _cols*_rows*_channels; i++)
        _pixbuffer[i] = buf[i];
    return ddgSuccess;
}


bool ddgImage::mapColorToAlpha( unsigned char *color )
{
	// Convert 3 channel color Alpha to true 4 channel alpha.
	if (channels() != 3)
		return ddgFailure;

	// If no color has been specified,
	// we assume that the top left corner pixel will always
	// contain the color which maps to transparent.
	unsigned char *pixel;
	unsigned char alpha[3];
	if (color)
		pixel = color;
	else
		get(0,0,&pixel);
	alpha[0] = pixel[0];
	alpha[1] = pixel[1];
	alpha[2] = pixel[2];
	ddgImage *cImgOut = new ddgImage();
	cImgOut->allocate(rows(),cols(),4);
	int r = 0, c = 0;
	for (r = 0; r < rows(); r++)
	{
		for (c = 0; c < cols(); c++)
		{
			get(r,c,&pixel);
			if (pixel[0] == alpha[0] && pixel[1] == alpha[1] && pixel[2] == alpha[2])
				cImgOut->set(r,c,255,255,255,0);
			else
				cImgOut->set(r,c,pixel[0],pixel[1],pixel[2],255);
		}
	}
	delete [] _pixbuffer;
	_pixbuffer = cImgOut->_pixbuffer;
	_channels = 4;
	cImgOut->_pixbuffer = NULL;
	delete cImgOut;

	return ddgSuccess;
}

bool ddgImage::mapLuminanceToRGBA( void )
{
	// Convert 1 channel color Luminance to true 4 channel alpha.
	if (channels() != 1)
		return ddgFailure;

	unsigned char pixel = 0;

	ddgImage *cImgOut = new ddgImage();
	cImgOut->allocate(rows(),cols(),4);
	int r = 0, c = 0;
	for (r = 0; r < rows(); r++)
	{
		for (c = 0; c < cols(); c++)
		{
			get(r,c,&pixel);
			cImgOut->set(r,c,pixel,pixel,pixel,pixel);
		}
	}
	delete [] _pixbuffer;
	_pixbuffer = cImgOut->_pixbuffer;
	_channels = 4;
	cImgOut->_pixbuffer = NULL;
	delete cImgOut;

	return ddgSuccess;
}
bool ddgImage::mapLuminanceToRGB( void )
{
	// Convert 1 channel color Luminance to true 3 channel.
	if (channels() != 1)
		return ddgFailure;

	unsigned char pixel = 0;

	ddgImage *cImgOut = new ddgImage();
	cImgOut->allocate(rows(),cols(),3);
	int r = 0, c = 0;
	for (r = 0; r < rows(); r++)
	{
		for (c = 0; c < cols(); c++)
		{
			get(r,c,&pixel);
			cImgOut->set(r,c,pixel,pixel,pixel);
		}
	}
	delete [] _pixbuffer;
	_pixbuffer = cImgOut->_pixbuffer;
	_channels = 3;
	cImgOut->_pixbuffer = NULL;
	delete cImgOut;

	return ddgSuccess;
}
// ----------------------------------------------------------------------
// load:
bool ddgImage::readRGB( const char *filename  )
{
	char *fullName = ddgFileHelper::getFullFileName(filename);
    // Load texture from file.
    int w,h,d;
    if (!(_pixbuffer = read_rgb_texture( fullName, &w, &h, &d)))
        return ddgFailure;

    _cols = w;
    _rows = h;
    _channels = d;
#ifdef _DEBUG
#ifdef DDGSTREAM
    cerr <<"Loaded RGB texture " << (char*)fullName << " (" << _cols << 'x' << _rows << ")\n";
#endif
#endif
    if (!_pixbuffer)
        return ddgFailure;
    return ddgSuccess;
}

// ----------------------------------------------------------------------
// load:
bool ddgImage::readBW( const char *filename  )
{
	char *fullName = ddgFileHelper::getFullFileName(filename);
    // Load texture from file.
    int w,h;
    if (!(_pixbuffer = read_alpha_texture( fullName, &w, &h)))
	{
        return ddgFailure;
	}

    _cols = w;
    _rows = h;
    _channels = 1;
#ifdef _DEBUG
#ifdef DDGSTREAM
    cerr <<"Loaded BW texture " << (char*)fullName << " (" << _cols << 'x' << _rows << ")\n";
#endif
#endif
    if (!_pixbuffer)
        return ddgFailure;
    return ddgSuccess;
}
// HACK!
bool ddgImage::_readBWold( char *filename  )
{
    // Load texture from file.
    int w,h;
    FILE            *fptr;

	char *fullName = ddgFileHelper::getFullFileName(filename);
    int l = strlen(fullName);
    if ( (fptr = fopen(fullName, "rb")) == NULL) { 
        ddgErrorSet(FileRead,(char *) (fullName ? fullName : "(null)"));
        goto error;
    }


    if (strncmp("terrain.bw", &(fullName[l-10]), 10) == 0) {
        w = 256;
        h = 256;
    } else if (strncmp("clouds.bw", &(fullName[l-9]), 9) == 0) {
        w = 128;
        h = 128;
    }
    else
	{
        ddgErrorSet(FileContent,"This data file is not supported.");
		goto error;
	}

    _pixbuffer = new unsigned char[ w * h];
	if (!_pixbuffer)
	{
        ddgErrorSet(Memory,"Memory allocation failure");
		goto error;
	}

    fread(_pixbuffer, sizeof _pixbuffer[0], w * h, fptr);
    fclose(fptr);    

    _cols = w;
    _rows = h;
    _channels = 1;
#ifdef _DEBUG
#ifdef DDGSTREAM
    cerr <<"Loaded BW old texture " << (char*)fullName << " (" << _cols << 'x' << _rows << ")\n";
#endif
#endif
	return ddgSuccess;

error:
	if (fptr)
		fclose(fptr);
    return ddgFailure;

}
// Read data from a tga file.
// return true if an error occured.
bool ddgImage::readTGA(const char *filename)
{
	char *fullName = ddgFileHelper::getFullFileName(filename);
    FILE *fptr = fullName && fullName[0] ? fopen(fullName,"rb") : 0;
    if (!fptr) {
        ddgErrorSet(FileRead,(char *) (fullName ? fullName : "(null)"));
        goto error;
    }
    unsigned char ch, ch2,cm,descr;
    unsigned char imageIdLength;

    // Read header.
    imageIdLength = fgetc(fptr);
	
    cm = fgetc(fptr); if(cm!=0 && cm!=1)
	{
        ddgErrorSet(FileContent,"Color mapped images are not supported!");
		return ddgFailure;// Color map provided.
	}
    ch = fgetc(fptr); if(ch!=2 && ch!=1)
	{
        ddgErrorSet(FileContent,"Compressed images are not supported!");
		return ddgFailure;// 1 = Uncompressed color mapped image.
	}
	int origx, origy;
                                            // 2 =    "         true color.
    ch = fgetc(fptr);                       // High: Index of 1st entry 0
    ch = fgetc(fptr);                       // Low:
    ch = fgetc(fptr);                       // High: Number of entries 256
    ch = fgetc(fptr);                       // Low:
    ch = fgetc(fptr);                       // Color map entry size 16,24,32
    ch = fgetc(fptr);					    // X Orig
    ch2 = fgetc(fptr);
	origx = ch2*256+ch;
    ch = fgetc(fptr);						// Y Orig
    ch2 = fgetc(fptr);
	origy = ch2*256+ch;
    
	
	ch = fgetc(fptr);                       // Width
    ch2 = fgetc(fptr);
    _cols = ch2*256+ch;
    ch = fgetc(fptr);                       // Height
    ch2 = fgetc(fptr);
    _rows = ch2*256+ch;
    ch = fgetc(fptr); ddgAssert(ch==32 ||ch==24 ||ch==16 || ch==8); // Pixel depth
    _channels = ch/8;
    descr = fgetc(fptr); //ddgAssert(ch==0);        // Descriptor.
    
    if (imageIdLength > 0)
    {
        /* just read it and ignore */
//        unsigned char imageId[imageIdLength];
		unsigned char *imageId;
        int i;

		imageId = (unsigned char *) malloc(imageIdLength);

        for(i = 0; i < imageIdLength; i++)
        {
            imageId[i] = fgetc(fptr);
        }
    }

    int cnt;
    // Get colour table and ignore it.
    if (cm==1)
    {
        char buf[256*3];
        if ((cnt = fread(buf,3,256,fptr)) < 256)
        {
            ddgErrorSet(FileRead,(char *) (fullName ? fullName : "(null)"));
            if (feof(fptr))
            {
                ddgErrorSet(FileAccess,"Reached end of file!");
                goto error;
            }
            goto error;
        }
    }
    // Read data
    allocate(_rows,_cols);
    int r;
	r = 0;
    while (r < _rows)
    {
        if ((cnt = fread(&(_pixbuffer[r*_cols*_channels]),_channels,_cols,fptr)) < (int)_cols)
        {
            ddgErrorSet(FileRead,(char *) (fullName ? fullName : "(null)"));
            if (feof(fptr))
            {
                ddgErrorSet(FileAccess,"Reached end of file!");
                goto error;
            }
            goto error;
        }
		if (_rows > 300)
			ddgConsole::progress("Texture loading",r,_rows);
        r++;
    }
	if (_rows > 300)
	{
		ddgConsole::progress("Texture loading",_rows,_rows);
		ddgConsole::end();
	}
	if (_channels > 2)
	{
		// Reverse RED and BLUE.  TGA stores BGR instead of RGB.
		int i = 0, offset = 0;
		unsigned char tmp;
		for (i = 0; i < _rows * _cols; i++)
		{
			tmp = _pixbuffer[offset];
			_pixbuffer[offset] = _pixbuffer[offset+2];
			_pixbuffer[offset+2] = tmp;
			offset += _channels;
		}
	}

    fclose(fptr);
#ifdef _DEBUG
#ifdef DDGSTREAM
    cerr <<"Loaded TGA texture " << (char*)fullName << " (" << _cols << 'x' << _rows << ":" << _channels <<")\n";
#endif
#endif
    return ddgSuccess;
error:
	if (fptr)
		fclose(fptr);
	return ddgFailure;

}

// Write data to a tga file.
bool ddgImage::writeTGA( const char *filename)
{
	char *fullName = ddgFileHelper::getFullFileName(filename);
    FILE *fptr = fullName && fullName[0] ? fopen(fullName,"wb") : 0;
    if (!fptr) {
        ddgErrorSet(FileWrite,(char *) (fullName ? fullName : "(null)"));
        goto error;
    }

    // Write header.
    fputc(0,fptr);					// Image ID.
	// Absence or presence of a color map.
    fputc((_channels<3?1:0),fptr);
	// Image Type.
	// 0 - No Image Data
	// 1 - Uncompressed - Colormapped
	// 2 - Uncompressed - True color
	// 3 - Uncompressed - Black and white
	// 9 - RLE Colormapped
	// 10 - RLE True color
	// 11 - RLE B&W
    fputc((_channels<3?1:2),fptr);// 1 = color mapped image, 2 = true color
	// Color map
    fputc(0,fptr);                  // 1st entry
    fputc(0,fptr);                  
    fputc(0,fptr);                  // Num entries
    fputc((_channels<3?1:0),fptr);
    fputc((_channels<3?24:0),fptr);	// Depth per colormap entry.
	// Image specification
    fputc(0,fptr);                  // X origin
    fputc(0,fptr);
    fputc(0,fptr);                  // Y Origin
    fputc(0,fptr);
    fputc(_cols%256,fptr);          // Width
    fputc(_cols/256,fptr);
    fputc(_rows%256,fptr);          // Height
    fputc(_rows/256,fptr);
    fputc(8*_channels,fptr);            // Depth
	// Image Descriptor.
	// bits 5 & 4 indicate data transfer direction.
	// 0 0 bottom left.
	// 0 1 bottom right
	// 1 0 top left
	// 1 1 top right
	// Bits 7 & 6 must be zero.
    fputc(32,fptr);                  // Dest of 1st pixel

    int cnt;
    if (_channels<3)
    {
        // put colour table and ignore it.
        for(int i=0; i<256; i++) {
            fputc((unsigned char) i,fptr);
            fputc((unsigned char) i,fptr);
            fputc((unsigned char) i,fptr);
        }
    }

	// temporarily swap RED and BLUE
	if (_channels > 2)
	{
		// Reverse RED and BLUE.  TGA stores ABGR instead of RGBA.
		int i = 0, offset = 0;
		unsigned char tmp;
		for (i = 0; i < _rows * _cols; i++)
		{
			tmp = _pixbuffer[offset];
			_pixbuffer[offset] = _pixbuffer[offset+2];
			_pixbuffer[offset+2] = tmp;
			offset += _channels;
		}
	}

    if ((cnt = fwrite(_pixbuffer,_channels,_rows*_cols,fptr)) < _rows*_cols)
    {
        ddgErrorSet(FileWrite,(char *) (fullName ? fullName : "(null)"));
        goto error;
    }

	// Swap it back.
	if (_channels > 2)
	{
		// Reverse RED and BLUE.  TGA stores ABGR instead of RGBA.
		int i = 0, offset = 0;
		unsigned char tmp;
		for (i = 0; i < _rows * _cols; i++)
		{
			tmp = _pixbuffer[offset];
			_pixbuffer[offset] = _pixbuffer[offset+2];
			_pixbuffer[offset+2] = tmp;
			offset += _channels;
		}
	}

    fclose(fptr);
    return ddgSuccess;

error:
	if (fptr)
		fclose(fptr);
	return ddgFailure;
}



// ========================================================================

/*
 * Read an SGI .rgb image file.
 * This code was borrowed from GLuT toolkit.
 */

// ========================================================================
typedef struct _ImageRec {
    unsigned short imagic;
    unsigned short type;
    unsigned short dim;
    unsigned short xsize, ysize, zsize;
    unsigned int min, max;
    unsigned int wasteBytes;
    char name[80];
    unsigned long colorMap;
    FILE *file;
    unsigned char *tmp;
    unsigned long rleEnd;
    unsigned int *rowStart;
    int *rowSize;
} ImageRec;

void
rgbtorgb(unsigned char *r,unsigned char *g,unsigned char *b,unsigned char *l,int n) {
    while(n--) {
        l[0] = r[0];
        l[1] = g[0];
        l[2] = b[0];
        l += 3; r++; g++; b++;
    }
}

static void
ConvertShort(unsigned short *array, unsigned int length) {
    unsigned short b1, b2;
    unsigned char *ptr;

    ptr = (unsigned char *)array;
    while (length--) {
        b1 = *ptr++;
        b2 = *ptr++;
        *array++ = (b1 << 8) | (b2);
    }
}

static void
ConvertUint(unsigned *array, unsigned int length) {
    unsigned int b1, b2, b3, b4;
    unsigned char *ptr;

    ptr = (unsigned char *)array;
    while (length--) {
        b1 = *ptr++;
        b2 = *ptr++;
        b3 = *ptr++;
        b4 = *ptr++;
        *array++ = (b1 << 24) | (b2 << 16) | (b3 << 8) | (b4);
    }
}

static ImageRec *ImageOpen(const char *fileName)
{
    union {
        int testWord;
        char testByte[4];
    } endianTest;
    ImageRec *image;
    int swapFlag;
    int x;

    endianTest.testWord = 1;
    if (endianTest.testByte[0] == 1) {
        swapFlag = 1;
    } else {
        swapFlag = 0;
    }

    image = (ImageRec *)malloc(sizeof(ImageRec));
    if (image == NULL) {
        fprintf(stderr, "Out of memory!\n");
        exit(1);
    }
    if ((image->file = fopen(fileName, "rb")) == NULL) {
    return NULL;
    }

    fread(image, 1, 12, image->file);

    if (swapFlag) {
        ConvertShort(&image->imagic, 6);
    }

    image->tmp = (unsigned char *)malloc(image->xsize*256);
    if (image->tmp == NULL) {
        fprintf(stderr, "\nOut of memory!\n");
        exit(1);
    }

    if ((image->type & 0xFF00) == 0x0100) {
        x = image->ysize * image->zsize * (int) sizeof(unsigned);
        image->rowStart = (unsigned *)malloc(x);
        image->rowSize = (int *)malloc(x);
        if (image->rowStart == NULL || image->rowSize == NULL) {
            fprintf(stderr, "\nOut of memory!\n");
            exit(1);
        }
        image->rleEnd = 512 + (2 * x);
        fseek(image->file, 512, SEEK_SET);
        fread(image->rowStart, 1, x, image->file);
        fread(image->rowSize, 1, x, image->file);
        if (swapFlag) {
            ConvertUint(image->rowStart, x/(int) sizeof(unsigned));
            ConvertUint((unsigned *)image->rowSize, x/(int) sizeof(int));
        }
    }
    return image;
}

static void
ImageClose(ImageRec *image) {
    fclose(image->file);
    free(image->tmp);
    free(image);
}

static void
ImageGetRow(ImageRec *image, unsigned char *buf, int y, int z) {
    unsigned char *iPtr, *oPtr, pixel;
    int count;

    if ((image->type & 0xFF00) == 0x0100) {
        fseek(image->file, (long) image->rowStart[y+z*image->ysize], SEEK_SET);
        fread(image->tmp, 1, (unsigned int)image->rowSize[y+z*image->ysize],
              image->file);

        iPtr = image->tmp;
        oPtr = buf;
        for (;;) {
            pixel = *iPtr++;
            count = (int)(pixel & 0x7F);
            if (!count) {
                return;
            }
            if (pixel & 0x80) {
                while (count--) {
                    *oPtr++ = *iPtr++;
                }
            } else {
                pixel = *iPtr++;
                while (count--) {
                    *oPtr++ = pixel;
                }
            }
        }
    } else {
        fseek(image->file, 512+(y*image->xsize)+(z*image->xsize*image->ysize),
              SEEK_SET);
        fread(buf, 1, image->xsize, image->file);
    }
}

unsigned char *
read_alpha_texture(const char *name, int *width, int *height)
{
    unsigned char *base, *lptr;
    ImageRec *image;
    int y;

    image = ImageOpen(name);
    if(!image) {
        return NULL;
    }

    (*width)=image->xsize;
    (*height)=image->ysize;
    if (image->zsize != 1) {
      ImageClose(image);
      return NULL;
    }

    base = (unsigned char *)malloc(image->xsize*image->ysize*sizeof(unsigned char));
    lptr = base;
    for(y=0; y<image->ysize; y++) {
        ImageGetRow(image,lptr,y,0);
        lptr += image->xsize;
    }
    ImageClose(image);

    return (unsigned char *) base;
}

unsigned char *
read_rgb_texture(const char *name, int *width, int *height, int *depth)
{
    unsigned char *base, *ptr;
    unsigned char *rbuf, *gbuf, *bbuf, *abuf;
    ImageRec *image;
    int y;

    image = ImageOpen(name);
    
    if(!image)
        return NULL;
    (*width)=image->xsize;
    (*height)=image->ysize;
    (*depth)=image->zsize;
    if (image->zsize != 3 && image->zsize != 4) {
      ImageClose(image);
      return NULL;
    }

    base = (unsigned char*)malloc(image->xsize*image->ysize*sizeof(unsigned int)*3);
    rbuf = (unsigned char *)malloc(image->xsize*sizeof(unsigned char));
    gbuf = (unsigned char *)malloc(image->xsize*sizeof(unsigned char));
    bbuf = (unsigned char *)malloc(image->xsize*sizeof(unsigned char));
    abuf = (unsigned char *)malloc(image->xsize*sizeof(unsigned char));
    if(!base || !rbuf || !gbuf || !bbuf || !abuf) {
      if (base) free(base);
      if (rbuf) free(rbuf);
      if (gbuf) free(gbuf);
      if (bbuf) free(bbuf);
      if (abuf) free(abuf);
      return NULL;
    }
    ptr = base;
    for(y=0; y<image->ysize; y++) {
        if(image->zsize == 4) {
            ImageGetRow(image,rbuf,y,0);
            ImageGetRow(image,gbuf,y,1);
            ImageGetRow(image,bbuf,y,2);
            ImageGetRow(image,abuf,y,3);  /* Discard. */
            rgbtorgb(rbuf,gbuf,bbuf,ptr,image->xsize);
            ptr += (image->xsize * 3);
        } else {
            ImageGetRow(image,rbuf,y,0);
            ImageGetRow(image,gbuf,y,1);
            ImageGetRow(image,bbuf,y,2);
            rgbtorgb(rbuf,gbuf,bbuf,ptr,image->xsize);
            ptr += (image->xsize * 3);
        }
    }
    ImageClose(image);
    free(rbuf);
    free(gbuf);
    free(bbuf);
    free(abuf);

    return (unsigned char *) base;
}


/*
**********************************************************************
** Read PNG file (Portable Network Graphics) 
**
**********************************************************************
*/
#ifdef DDGPNG
#define PNG_BYTES_TO_CHECK 4
static int check_if_png(FILE *fp)
{
   char buf[PNG_BYTES_TO_CHECK];

   /* Read in some of the signature bytes */
   if (fread(buf, 1, PNG_BYTES_TO_CHECK, fp) != PNG_BYTES_TO_CHECK)
      return 0;

   /* Compare the first PNG_BYTES_TO_CHECK bytes of the signature.
      Return nonzero (true) if they match */

   return(!png_sig_cmp((unsigned char *)buf, (png_size_t)0, PNG_BYTES_TO_CHECK));
}
#endif

bool ddgImage::readPNG(const char *filename)
{
#ifdef DDGPNG
	char *fullName = ddgFileHelper::getFullFileName(filename);
    png_structp png_ptr;
    png_infop info_ptr;
    png_uint_32 width, height, channels, rowbytes;
    int bit_depth, color_type, interlace_type;
    int row;
	png_bytepp row_pointers;

    FILE *fptr = fullName && fullName[0] ? fopen(fullName,"rb") : 0;
    if (!fptr) 
    {
        ddgErrorSet(FileRead,(char *) (fullName ? fullName : "(null)"));
		goto error;
    }

    if (!check_if_png(fptr))
    {
        goto error;
    }

    /*
    *******************************************
    ** use default error handlers for now
    *******************************************
    */
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
            NULL, NULL, NULL);
    if (png_ptr == NULL)
    {
        ddgErrorSet(FileContent,"PNG lib version incorrect");

        goto error;
    }

    /* Allocate/initialize the memory for image information.  REQUIRED. */
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL)
    {
        ddgErrorSet(Memory,"unable to alloc/init image info struct");
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);

        goto error;
    }

    /* Set error handling if you are using the setjmp/longjmp method (this is
     * the normal method of doing things with libpng).  REQUIRED unless you
     * set up your own error handlers in the png_create_read_struct() earlier.
     */
    if (setjmp(png_ptr->jmpbuf))
    {
        /* Free all of the memory associated with the png_ptr and info_ptr */
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
        /* If we get here, we had a problem reading the file */
        ddgErrorSet(FileRead,"unable to read file");

        goto error;
    }

    png_init_io(png_ptr, fptr);

    /*
    *************************************************
    ** because we already read some of the signature
    ** to figure out if this file has the correct
    ** format
    *************************************************
    */
    png_set_sig_bytes(png_ptr, PNG_BYTES_TO_CHECK);

    /*
    ************************************************
    ** get all the info from the PNG file before
    ** we get the first image data chunk
    ************************************************
    */
    png_read_info(png_ptr, info_ptr);

    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, 
            &interlace_type, NULL, NULL);
	
    rowbytes = png_get_rowbytes(png_ptr, info_ptr);

	row_pointers = (png_bytepp) malloc(height*sizeof(png_bytep));

    for (row = 0; (png_uint_32) row < height; row++)
    {
        row_pointers[row] = (png_bytep) malloc(rowbytes);
    }

    png_read_image(png_ptr, row_pointers);

    channels = png_get_channels(png_ptr, info_ptr);

    if(ddgImage::allocate(height, width, channels))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
        ddgErrorSet(Memory,"failed to allocate memory for image");
        if (fptr) fclose(fptr);
    }

    for (row = 0; (png_uint_32) row < height; row++)
    {
        memcpy(_pixbuffer+(row*rowbytes), row_pointers[row], rowbytes);
    }

    png_read_end(png_ptr, info_ptr);

    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
    fclose(fptr);

    /*
    ***********
    ** success
    ***********
    */
    return ddgSuccess;
error:
    if (fptr) fclose(fptr);
#else
	(void)filename;
#endif
	return ddgFailure;
}

bool ddgImage::writePNG(const char *filename)
{
#ifdef DDGPNG
    return ddgSuccess;
#else
    (void)filename;
#endif
    return ddgFailure;
}
