/* adler32.h -- compute the Adler-32 checksum of a data stream
 * Copyright (C) 1995-1998 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */
#ifndef AWS_ADLER
unsigned long aws_adler32 (
                unsigned long adler,
                const unsigned char *buf,
                unsigned int len);
#endif // AWS_ADLER
