/*

	ImageDXTC.h

*/

#ifndef IMAGEDXTC_H_
#define IMAGEDXTC_H_

#include "Image.h"

namespace ImageLib 
{

class CodeBook;

typedef enum
{
	DC_None,
	DC_DXT1,
	DC_DXT3,
	//DC_DXT5,
} DXTCMethod;


class ImageDXTC
{
private:
	long		XSize, YSize;
	WORD		*pBlocks;
	DXTCMethod	Method;
	BYTE		AlphaValue;

	void		DXT1to32(Image32 *pDest);
	void		DXT3to32(Image32 *pDest);

	void		EmitTransparentBlock(WORD *pDest);
	void		Emit1ColorBlock(WORD *pDest, Color c);
	void		Emit1ColorBlockTrans(WORD *pDest, Color c, Color *pSrc);
	void		Emit2ColorBlock(WORD *pDest, Color c1, Color c2, Color *pSrc);
	void		Emit2ColorBlockTrans(WORD *pDest, Color c1, Color c2, Color *pSrc);
	void		EmitMultiColorBlock3(WORD *pDest, CodeBook &cb, Color *pSrc);
	void		EmitMultiColorBlock4(WORD *pDest, CodeBook &cb, Color *pSrc);
	void		EmitMultiColorBlockTrans(WORD *pDest, CodeBook &cb, Color *pSrc);
	void		Emit4BitAlphaBlock(WORD *pDest, Color *pSrc);


public:
	ImageDXTC();
	~ImageDXTC();

	void	ReleaseAll(void);

	void	SetMethod(DXTCMethod NewMethod);	// MUST be called before setsize
	void	SetSize(long x, long y);

	long		GetXSize(void) {return XSize;}
	long		GetYSize(void) {return YSize;}
	DXTCMethod	GetMethod(void) {return Method;}
	WORD		*GetBlocks(void) {return pBlocks;}

	void	FromImage32(Image32 *pSrc, DXTCMethod = DC_None);
	void	ToImage32(Image32 *pDest);

	void	CompressDXT1(Image32 *pSrcImg);		// Potentially called by FromImage32
	void	CompressDXT3(Image32 *pSrcImg);		// Potentially called by FromImage32
};

} // end of namespace ImageLib

#endif
