/**
*  Copyright (C) 2000 - DivXNetworks
 *
 * Adam Lee
 * Andrea Graziani
 * < etabeta@projectmayo.com >
**/
// decore.h //

// This is the header file describing 
// the entrance function of the encoder core
// or the encore ...

#ifdef __cplusplus
extern "C" {
#endif 

#ifndef _DECORE_H_
#define _DECORE_H_

// decore options
#define DEC_OPT_INIT		32768				// 0x00008000
#define DEC_OPT_RELEASE 65536				// 0x00010000
#define DEC_OPT_SETPP		0x00020000	// 0x00020000

// decore return values
#define DEC_OK					0
#define DEC_MEMORY			1
#define DEC_BAD_FORMAT	2

typedef struct _DEC_PARAM_ 
{
	int x_dim; // x dimension of the frames to be decoded
	int y_dim; // y dimension of the frames to be decoded
	unsigned long color_depth;
} DEC_PARAM;

typedef struct _DEC_FRAME_
{
	void *bmp; // the 24-bit decoded bitmap 
	void *bitstream; // the decoder buffer
	long length; // the lenght of the decoder stream
	int render_flag;
} DEC_FRAME;

typedef struct _DEC_SET_
{
	int postproc_level; // valid interval are [0..100]
} DEC_SET;

// the prototype of the decore() - main decore engine entrance
int decore(
			unsigned long handle,	// handle	- the handle of the calling entity, must be unique
			unsigned long dec_opt, // dec_opt - the option for docoding, see below
			void *param1,	// param1	- the parameter 1 (it's actually meaning depends on dec_opt
			void *param2);	// param2	- the parameter 2 (it's actually meaning depends on dec_opt

#endif // _DECORE_H_
#ifdef __cplusplus
}
#endif 
