/*
    adler32.h - Compute the Adler-32 checksum of a data stream.
    
    Copyright (C) 1995-1998 Mark Adler
    
    For conditions of distribution and use, see copyright notice in zlib.h
*/

#ifndef __CS_AWS_ADLER_H__
#define __CS_AWS_ADLER_H__

unsigned long aws_adler32 (
  unsigned long adler,
  const unsigned char *buf,
  unsigned int len);
  
#endif // __CS_AWS_ADLER_H__
