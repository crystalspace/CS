/* ====================================================================
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 2000 The Apache Software Foundation.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The end-user documentation included with the redistribution,
 *    if any, must include the following acknowledgment:
 *       "This product includes software developed by the
 *        Apache Software Foundation (http://www.apache.org/)."
 *    Alternately, this acknowledgment may appear in the software itself,
 *    if and wherever such third-party acknowledgments normally appear.
 *
 * 4. The names "Apache" and "Apache Software Foundation" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact apache@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache",
 *    nor may "Apache" appear in their name, without prior written
 *    permission of the Apache Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE APACHE SOFTWARE FOUNDATION OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Software Foundation.  For more
 * information on the Apache Software Foundation, please see
 * <http://www.apache.org/>.
 *
 * Portions of this software are based upon public domain software
 * originally written at the National Center for Supercomputing Applications,
 * University of Illinois, Urbana-Champaign.
 */

#ifndef __CS_CSCTYPE_H__
#define __CS_CSCTYPE_H__

/**\file
 */

#include <ctype.h>

#ifdef __cplusplus
extern "C" {
#endif

/* These macros allow correct support of 8-bit characters on systems which
 * support 8-bit characters.  Pretty dumb how the cast is required, but
 * that's legacy libc for ya.  These new macros do not support EOF like
 * the standard macros do.  Tough.
 */
/** returns true if c is an alphanumeric character. */
#define cs_isalnum(c) (isalnum(((unsigned char)(c))))
/** returns true if c is a letter. */
#define cs_isalpha(c) (isalpha(((unsigned char)(c))))
/** returns true if c is a control character. */
#define cs_iscntrl(c) (iscntrl(((unsigned char)(c))))
/** returns true if c is a digit. */
#define cs_isdigit(c) (isdigit(((unsigned char)(c))))
/** returns true if c is a graphic character. */
#define cs_isgraph(c) (isgraph(((unsigned char)(c))))
/** returns true if c is a lower-case letter. */
#define cs_islower(c) (islower(((unsigned char)(c))))
/** returns true if c is a printable character. */
#define cs_isprint(c) (isprint(((unsigned char)(c))))
/** returns true if c is a punctuation character. */
#define cs_ispunct(c) (ispunct(((unsigned char)(c))))
/** returns true if c is a space character. */
#define cs_isspace(c) (isspace(((unsigned char)(c))))
/** returns true if c is a upper-case letter. */
#define cs_isupper(c) (isupper(((unsigned char)(c))))
/** returns true if c is a hex digit. */
#define cs_isxdigit(c) (isxdigit(((unsigned char)(c))))
/** converts c to lower-case. */
#define cs_tolower(c) (tolower(((unsigned char)(c))))
/** converts c to upper-case. */
#define cs_toupper(c) (toupper(((unsigned char)(c))))

#ifdef __cplusplus
}
#endif

#endif	// __CS_CSCTYPE_H__
