/*
 * NuppelWriter.cpp - A simple class for writing compressed NuppelVideo files.
 *                    NuppelVideo is a simple video format based on RTjpeg and LZO
 *                    that is designed for recording with minimal CPU. The
 *                    VideoRecorder object uses this to record real-time, assuming that
 *                    you'll recompress to Divx or something similar for redistribution.
 *
 * Copyright (C) 2002-2003 Micah Dowty <micah@navi.picogui.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 */

#include "cssysdef.h"

#include "nuppelwriter.h"
#include "minilzo.h"
#include "rtjpeg.h"
#include "nuppelvideo.h"
//#include <string.h>

/* Colorspace conversion routines from rgb2yuv420.cpp */
void InitLookupTable();
int RGB2YUV420 (int x_dim, int y_dim, 
		unsigned char *bmp, 
		unsigned char *yuv,
		unsigned char *tmp1, unsigned char *tmp2);


NuppelWriter::NuppelWriter(int width, int height, 
			   outputCallback_t outputCallback, void *callbackExtra,
			   float frameRate, float rtjQuality, bool rtjpeg,
			   bool lzo, bool rgb, int keyframeFreq) {
  rtfileheader nuvh;
  rtframeheader frameh;
  static unsigned long int tbls[128];
  int Q;

  this->outputCallback = outputCallback;
  this->callbackExtra = callbackExtra;
  this->width = width;
  this->height = height;
  this->frameRate = frameRate;
  this->keyframeFreq = keyframeFreq;
  this->lzo = lzo;
  this->rgb = rgb;
  this->rtjpeg = rtjpeg;
  frameofgop = 0;
  frameNumber = 0;
  
  /* Initialize a nuppelvideo header */
  memset(&nuvh, 0, sizeof(nuvh));
  strcpy(nuvh.finfo,"NuppelVideo");
  strcpy(nuvh.version,"0.05");
  nuvh.width = width;
  nuvh.height = height;
  nuvh.pimode = 'P';
  nuvh.aspect = 1.0;
  nuvh.fps = frameRate;
  nuvh.videoblocks = -1;   /* unknown */
  
  bufferSize = width * height * 3;

  /* Allocate several temporary buffers */
  compressBuffer = new unsigned char [width*height+(width*height)/2];
  yuvBuffer = new unsigned char [width*height+(width*height)/2];
  yuvTmp1 = new unsigned char [width*height];
  yuvTmp2 = new unsigned char [width*height];
  lzoTmp = new unsigned char [LZO1X_MEM_COMPRESS];
  InitLookupTable();

  /* Write the header */ 
  outputCallback(&nuvh, sizeof(nuvh), callbackExtra);
 
  /* According to the RTJpeg docs, 255 is the full scale for Q,
   * being equivalent to a JPEG with 70% quality.
   */
  Q = (int) (rtjQuality*255);

  /* Initialize the compression tables */
  RTjpeg_init_compress(tbls,width,height, Q);
  RTjpeg_init_mcompress();

  /* Write the compression configuration frame */
  memset(&frameh, 0, sizeof(frameh));
  frameh.frametype = 'D';
  frameh.comptype = 'R';
  frameh.packetlength = sizeof(tbls);
  outputCallback(&frameh, sizeof(frameh), callbackExtra);
  outputCallback(&tbls, sizeof(tbls), callbackExtra);
}

NuppelWriter::~NuppelWriter() {
  delete lzoTmp;
  delete yuvTmp1;
  delete yuvTmp2;
  delete compressBuffer;
  delete yuvBuffer;
}

void NuppelWriter::writeFrame(unsigned char *frameBuffer) {
  rtframeheader frameh;
  lzo_uint lzoSize;
  unsigned char *currentBuffer;
  unsigned int currentBufferSize;

  /* Do we need to write a keyframe? */
  if ((frameofgop % keyframeFreq) == 0) {
    memset(&frameh, 'j', sizeof(frameh));
    frameh.frametype = 'R';
    frameh.comptype = 'T';
    outputCallback(&frameh, sizeof(frameh), callbackExtra);
    memset(&frameh, 0, sizeof(frameh));
    frameofgop = 0;
    frameh.frametype = 'S';
    frameh.comptype = 'V';
    frameh.timecode = frameNumber;
    outputCallback(&frameh, sizeof(frameh), callbackExtra);
  }
  
  /* Set up a video frame with RTJpeg and LZO compression */
  memset(&frameh, 0, sizeof(frameh));
  frameh.frametype = 'V';
  frameh.keyframe = frameofgop;
  frameh.timecode = (int) (frameNumber * frameRate * 1000.0);

  if (rgb) {
    /* Nonstandard: uncompressed bottom-up RGB24 */
    frameh.comptype = 'R';
    currentBuffer = frameBuffer;
    currentBufferSize = bufferSize;
  }
  else {
    /* Convert from RGB to YUV420. This routine has also
     * been modified to flip the video vertically.
     */
    RGB2YUV420(width, height, frameBuffer, yuvBuffer, yuvTmp1, yuvTmp2);
    currentBuffer = yuvBuffer;
    currentBufferSize = width*height+(width*height/2);
    frameh.comptype = '0';
  }

  if (rtjpeg) {
    /* Compress the frame using RTJpeg (lossy) */
    currentBufferSize = RTjpeg_mcompressYUV420((int8*) compressBuffer, yuvBuffer, 1, 1);
    currentBuffer = compressBuffer;
    frameh.comptype = '1';
  }

  if (lzo) {
    /* Compress it again using LZO (lossless) */
    lzo1x_1_compress(currentBuffer, currentBufferSize, frameBuffer, &lzoSize, lzoTmp);
    currentBufferSize = lzoSize;
    currentBuffer = frameBuffer;
    if (rtjpeg)
      frameh.comptype = '2';
    else {
      if (rgb)
	frameh.comptype = 'r';   /* Nonstandard: LZO'ed bottom-up RGB24 */
      else
	frameh.comptype = '3';
    }
  }

  /* Write the frame */
  frameh.packetlength = currentBufferSize;
  outputCallback(&frameh, sizeof(frameh), callbackExtra);
  outputCallback(currentBuffer, currentBufferSize, callbackExtra);

  frameNumber++;
  frameofgop++;
}
