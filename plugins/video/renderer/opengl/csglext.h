#ifdef _CSGLEXT_
#undef _CSGLEXT_

/*
    Copyright (C) 2001 by Norman Krämer
  
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

/**********************************************************************
 * Begin system-specific stuff.
 */
#if defined(__BEOS__)
#include <stdlib.h>     /* to get some BeOS-isms */
#endif

#if !defined(OPENSTEP) && (defined(NeXT) || defined(NeXT_PDO))
#define OPENSTEP
#endif

#if defined(_WIN32) && !defined(__WIN32__) && !defined(__CYGWIN__)
#define __WIN32__
#endif

#if !defined(OPENSTEP) && (defined(__WIN32__) && !defined(__CYGWIN__))
#  if defined(_MSC_VER) && defined(BUILD_GL32) /* tag specify we're building mesa as a DLL */
#    define GLAPI __declspec(dllexport)
#  elif defined(_MSC_VER) && defined(_DLL) /* tag specifying we're building for DLL runtime support */
#    define GLAPI __declspec(dllimport)
#  else /* for use with static link lib build of Win32 edition only */
#    define GLAPI extern
#  endif /* _STATIC_MESA support */
#  define GLAPIENTRY __stdcall
#else
/* non-Windows compilation */
#  define GLAPI extern
#  define GLAPIENTRY
#endif /* WIN32 / CYGWIN bracket */

#if defined(_WIN32) && !defined(_WINGDI_) && !defined(__CYGWIN__) && !defined(_GNU_H_WINDOWS32_DEFINES) && !defined(OPENSTEP)
#include <gl/mesa_wgl.h>
#endif

#if defined(macintosh) && PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

#if defined(_WIN32) && !defined(APIENTRY) && !defined(__CYGWIN__)
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#endif

#ifndef APIENTRY
#define APIENTRY
#endif

/*
 * End system-specific stuff.
 **********************************************************************/

/*
#ifdef CENTERLINE_CLPP
#define signed
#endif
typedef unsigned int	GLenum;
typedef unsigned char	GLboolean;
typedef unsigned int	GLbitfield;
typedef void		GLvoid;
typedef signed char	GLbyte;	       
typedef short		GLshort;	
typedef int		GLint;		
typedef unsigned char	GLubyte;	
typedef unsigned short	GLushort;	
typedef unsigned int	GLuint;		
typedef int		GLsizei;	
typedef float		GLfloat;	
typedef float		GLclampf;	
typedef double		GLdouble;	
typedef double		GLclampd;	
*/

typedef void (APIENTRY * PFNGLBLENDCOLORPROC) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef void (APIENTRY * PFNGLBLENDEQUATIONPROC) (GLenum mode);
typedef void (APIENTRY * PFNGLDRAWRANGEELEMENTSPROC) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);
typedef void (APIENTRY * PFNGLCOLORTABLEPROC) (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table);
typedef void (APIENTRY * PFNGLCOLORTABLEPARAMETERFVPROC) (GLenum target, GLenum pname, const GLfloat *params);
typedef void (APIENTRY * PFNGLCOLORTABLEPARAMETERIVPROC) (GLenum target, GLenum pname, const GLint *params);
typedef void (APIENTRY * PFNGLCOPYCOLORTABLEPROC) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
typedef void (APIENTRY * PFNGLGETCOLORTABLEPROC) (GLenum target, GLenum format, GLenum type, GLvoid *table);
typedef void (APIENTRY * PFNGLGETCOLORTABLEPARAMETERFVPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef void (APIENTRY * PFNGLGETCOLORTABLEPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLCOLORSUBTABLEPROC) (GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data);
typedef void (APIENTRY * PFNGLCOPYCOLORSUBTABLEPROC) (GLenum target, GLsizei start, GLint x, GLint y, GLsizei width);
typedef void (APIENTRY * PFNGLCONVOLUTIONFILTER1DPROC) (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *image);
typedef void (APIENTRY * PFNGLCONVOLUTIONFILTER2DPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image);
typedef void (APIENTRY * PFNGLCONVOLUTIONPARAMETERFPROC) (GLenum target, GLenum pname, GLfloat params);
typedef void (APIENTRY * PFNGLCONVOLUTIONPARAMETERFVPROC) (GLenum target, GLenum pname, const GLfloat *params);
typedef void (APIENTRY * PFNGLCONVOLUTIONPARAMETERIPROC) (GLenum target, GLenum pname, GLint params);
typedef void (APIENTRY * PFNGLCONVOLUTIONPARAMETERIVPROC) (GLenum target, GLenum pname, const GLint *params);
typedef void (APIENTRY * PFNGLCOPYCONVOLUTIONFILTER1DPROC) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
typedef void (APIENTRY * PFNGLCOPYCONVOLUTIONFILTER2DPROC) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (APIENTRY * PFNGLGETCONVOLUTIONFILTERPROC) (GLenum target, GLenum format, GLenum type, GLvoid *image);
typedef void (APIENTRY * PFNGLGETCONVOLUTIONPARAMETERFVPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef void (APIENTRY * PFNGLGETCONVOLUTIONPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLGETSEPARABLEFILTERPROC) (GLenum target, GLenum format, GLenum type, GLvoid *row, GLvoid *column, GLvoid *span);
typedef void (APIENTRY * PFNGLSEPARABLEFILTER2DPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *row, const GLvoid *column);
typedef void (APIENTRY * PFNGLGETHISTOGRAMPROC) (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
typedef void (APIENTRY * PFNGLGETHISTOGRAMPARAMETERFVPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef void (APIENTRY * PFNGLGETHISTOGRAMPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLGETMINMAXPROC) (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
typedef void (APIENTRY * PFNGLGETMINMAXPARAMETERFVPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef void (APIENTRY * PFNGLGETMINMAXPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLHISTOGRAMPROC) (GLenum target, GLsizei width, GLenum internalformat, GLboolean sink);
typedef void (APIENTRY * PFNGLMINMAXPROC) (GLenum target, GLenum internalformat, GLboolean sink);
typedef void (APIENTRY * PFNGLRESETHISTOGRAMPROC) (GLenum target);
typedef void (APIENTRY * PFNGLRESETMINMAXPROC) (GLenum target);
typedef void (APIENTRY * PFNGLTEXIMAGE3DPROC) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (APIENTRY * PFNGLTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (APIENTRY * PFNGLCOPYTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (APIENTRY * PFNGLACTIVETEXTUREARBPROC) (GLenum texture);
typedef void (APIENTRY * PFNGLCLIENTACTIVETEXTUREARBPROC) (GLenum texture);
typedef void (APIENTRY * PFNGLMULTITEXCOORD1DARBPROC) (GLenum target, GLdouble s);
typedef void (APIENTRY * PFNGLMULTITEXCOORD1DVARBPROC) (GLenum target, const GLdouble *v);
typedef void (APIENTRY * PFNGLMULTITEXCOORD1FARBPROC) (GLenum target, GLfloat s);
typedef void (APIENTRY * PFNGLMULTITEXCOORD1FVARBPROC) (GLenum target, const GLfloat *v);
typedef void (APIENTRY * PFNGLMULTITEXCOORD1IARBPROC) (GLenum target, GLint s);
typedef void (APIENTRY * PFNGLMULTITEXCOORD1IVARBPROC) (GLenum target, const GLint *v);
typedef void (APIENTRY * PFNGLMULTITEXCOORD1SARBPROC) (GLenum target, GLshort s);
typedef void (APIENTRY * PFNGLMULTITEXCOORD1SVARBPROC) (GLenum target, const GLshort *v);
typedef void (APIENTRY * PFNGLMULTITEXCOORD2DARBPROC) (GLenum target, GLdouble s, GLdouble t);
typedef void (APIENTRY * PFNGLMULTITEXCOORD2DVARBPROC) (GLenum target, const GLdouble *v);
typedef void (APIENTRY * PFNGLMULTITEXCOORD2FARBPROC) (GLenum target, GLfloat s, GLfloat t);
typedef void (APIENTRY * PFNGLMULTITEXCOORD2FVARBPROC) (GLenum target, const GLfloat *v);
typedef void (APIENTRY * PFNGLMULTITEXCOORD2IARBPROC) (GLenum target, GLint s, GLint t);
typedef void (APIENTRY * PFNGLMULTITEXCOORD2IVARBPROC) (GLenum target, const GLint *v);
typedef void (APIENTRY * PFNGLMULTITEXCOORD2SARBPROC) (GLenum target, GLshort s, GLshort t);
typedef void (APIENTRY * PFNGLMULTITEXCOORD2SVARBPROC) (GLenum target, const GLshort *v);
typedef void (APIENTRY * PFNGLMULTITEXCOORD3DARBPROC) (GLenum target, GLdouble s, GLdouble t, GLdouble r);
typedef void (APIENTRY * PFNGLMULTITEXCOORD3DVARBPROC) (GLenum target, const GLdouble *v);
typedef void (APIENTRY * PFNGLMULTITEXCOORD3FARBPROC) (GLenum target, GLfloat s, GLfloat t, GLfloat r);
typedef void (APIENTRY * PFNGLMULTITEXCOORD3FVARBPROC) (GLenum target, const GLfloat *v);
typedef void (APIENTRY * PFNGLMULTITEXCOORD3IARBPROC) (GLenum target, GLint s, GLint t, GLint r);
typedef void (APIENTRY * PFNGLMULTITEXCOORD3IVARBPROC) (GLenum target, const GLint *v);
typedef void (APIENTRY * PFNGLMULTITEXCOORD3SARBPROC) (GLenum target, GLshort s, GLshort t, GLshort r);
typedef void (APIENTRY * PFNGLMULTITEXCOORD3SVARBPROC) (GLenum target, const GLshort *v);
typedef void (APIENTRY * PFNGLMULTITEXCOORD4DARBPROC) (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
typedef void (APIENTRY * PFNGLMULTITEXCOORD4DVARBPROC) (GLenum target, const GLdouble *v);
typedef void (APIENTRY * PFNGLMULTITEXCOORD4FARBPROC) (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
typedef void (APIENTRY * PFNGLMULTITEXCOORD4FVARBPROC) (GLenum target, const GLfloat *v);
typedef void (APIENTRY * PFNGLMULTITEXCOORD4IARBPROC) (GLenum target, GLint s, GLint t, GLint r, GLint q);
typedef void (APIENTRY * PFNGLMULTITEXCOORD4IVARBPROC) (GLenum target, const GLint *v);
typedef void (APIENTRY * PFNGLMULTITEXCOORD4SARBPROC) (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
typedef void (APIENTRY * PFNGLMULTITEXCOORD4SVARBPROC) (GLenum target, const GLshort *v);
typedef void (APIENTRY * PFNGLLOADTRANSPOSEMATRIXFARBPROC) (const GLfloat *m);
typedef void (APIENTRY * PFNGLLOADTRANSPOSEMATRIXDARBPROC) (const GLdouble *m);
typedef void (APIENTRY * PFNGLMULTTRANSPOSEMATRIXFARBPROC) (const GLfloat *m);
typedef void (APIENTRY * PFNGLMULTTRANSPOSEMATRIXDARBPROC) (const GLdouble *m);
typedef void (APIENTRY * PFNGLSAMPLECOVERAGEARBPROC) (GLclampf value, GLboolean invert);
typedef void (APIENTRY * PFNGLSAMPLEPASSARBPROC) (GLenum pass);
typedef void (APIENTRY * PFNGLCOMPRESSEDTEXIMAGE3DARBPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * PFNGLCOMPRESSEDTEXIMAGE2DARBPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * PFNGLCOMPRESSEDTEXIMAGE1DARBPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * PFNGLGETCOMPRESSEDTEXIMAGEARBPROC) (GLenum target, GLint level, void *img);
typedef void (APIENTRY * PFNGLBLENDCOLOREXTPROC) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef void (APIENTRY * PFNGLPOLYGONOFFSETEXTPROC) (GLfloat factor, GLfloat bias);
typedef void (APIENTRY * PFNGLTEXIMAGE3DEXTPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (APIENTRY * PFNGLTEXSUBIMAGE3DEXTPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (APIENTRY * PFNGLGETTEXFILTERFUNCSGISPROC) (GLenum target, GLenum filter, GLfloat *weights);
typedef void (APIENTRY * PFNGLTEXFILTERFUNCSGISPROC) (GLenum target, GLenum filter, GLsizei n, const GLfloat *weights);
typedef void (APIENTRY * PFNGLTEXSUBIMAGE1DEXTPROC) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (APIENTRY * PFNGLTEXSUBIMAGE2DEXTPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (APIENTRY * PFNGLCOPYTEXIMAGE1DEXTPROC) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
typedef void (APIENTRY * PFNGLCOPYTEXIMAGE2DEXTPROC) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
typedef void (APIENTRY * PFNGLCOPYTEXSUBIMAGE1DEXTPROC) (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
typedef void (APIENTRY * PFNGLCOPYTEXSUBIMAGE2DEXTPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (APIENTRY * PFNGLCOPYTEXSUBIMAGE3DEXTPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (APIENTRY * PFNGLGETHISTOGRAMEXTPROC) (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
typedef void (APIENTRY * PFNGLGETHISTOGRAMPARAMETERFVEXTPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef void (APIENTRY * PFNGLGETHISTOGRAMPARAMETERIVEXTPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLGETMINMAXEXTPROC) (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
typedef void (APIENTRY * PFNGLGETMINMAXPARAMETERFVEXTPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef void (APIENTRY * PFNGLGETMINMAXPARAMETERIVEXTPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLHISTOGRAMEXTPROC) (GLenum target, GLsizei width, GLenum internalformat, GLboolean sink);
typedef void (APIENTRY * PFNGLMINMAXEXTPROC) (GLenum target, GLenum internalformat, GLboolean sink);
typedef void (APIENTRY * PFNGLRESETHISTOGRAMEXTPROC) (GLenum target);
typedef void (APIENTRY * PFNGLRESETMINMAXEXTPROC) (GLenum target);
typedef void (APIENTRY * PFNGLCONVOLUTIONFILTER1DEXTPROC) (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *image);
typedef void (APIENTRY * PFNGLCONVOLUTIONFILTER2DEXTPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image);
typedef void (APIENTRY * PFNGLCONVOLUTIONPARAMETERFEXTPROC) (GLenum target, GLenum pname, GLfloat params);
typedef void (APIENTRY * PFNGLCONVOLUTIONPARAMETERFVEXTPROC) (GLenum target, GLenum pname, const GLfloat *params);
typedef void (APIENTRY * PFNGLCONVOLUTIONPARAMETERIEXTPROC) (GLenum target, GLenum pname, GLint params);
typedef void (APIENTRY * PFNGLCONVOLUTIONPARAMETERIVEXTPROC) (GLenum target, GLenum pname, const GLint *params);
typedef void (APIENTRY * PFNGLCOPYCONVOLUTIONFILTER1DEXTPROC) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
typedef void (APIENTRY * PFNGLCOPYCONVOLUTIONFILTER2DEXTPROC) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (APIENTRY * PFNGLGETCONVOLUTIONFILTEREXTPROC) (GLenum target, GLenum format, GLenum type, GLvoid *image);
typedef void (APIENTRY * PFNGLGETCONVOLUTIONPARAMETERFVEXTPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef void (APIENTRY * PFNGLGETCONVOLUTIONPARAMETERIVEXTPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLGETSEPARABLEFILTEREXTPROC) (GLenum target, GLenum format, GLenum type, GLvoid *row, GLvoid *column, GLvoid *span);
typedef void (APIENTRY * PFNGLSEPARABLEFILTER2DEXTPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *row, const GLvoid *column);
typedef void (APIENTRY * PFNGLCOLORTABLESGIPROC) (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table);
typedef void (APIENTRY * PFNGLCOLORTABLEPARAMETERFVSGIPROC) (GLenum target, GLenum pname, const GLfloat *params);
typedef void (APIENTRY * PFNGLCOLORTABLEPARAMETERIVSGIPROC) (GLenum target, GLenum pname, const GLint *params);
typedef void (APIENTRY * PFNGLCOPYCOLORTABLESGIPROC) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
typedef void (APIENTRY * PFNGLGETCOLORTABLESGIPROC) (GLenum target, GLenum format, GLenum type, GLvoid *table);
typedef void (APIENTRY * PFNGLGETCOLORTABLEPARAMETERFVSGIPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef void (APIENTRY * PFNGLGETCOLORTABLEPARAMETERIVSGIPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLPIXELTEXGENSGIXPROC) (GLenum mode);
typedef void (APIENTRY * PFNGLPIXELTEXGENPARAMETERISGISPROC) (GLenum pname, GLint param);
typedef void (APIENTRY * PFNGLPIXELTEXGENPARAMETERIVSGISPROC) (GLenum pname, const GLint *params);
typedef void (APIENTRY * PFNGLPIXELTEXGENPARAMETERFSGISPROC) (GLenum pname, GLfloat param);
typedef void (APIENTRY * PFNGLPIXELTEXGENPARAMETERFVSGISPROC) (GLenum pname, const GLfloat *params);
typedef void (APIENTRY * PFNGLGETPIXELTEXGENPARAMETERIVSGISPROC) (GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLGETPIXELTEXGENPARAMETERFVSGISPROC) (GLenum pname, GLfloat *params);
typedef void (APIENTRY * PFNGLTEXIMAGE4DSGISPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLsizei size4d, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (APIENTRY * PFNGLTEXSUBIMAGE4DSGISPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint woffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei size4d, GLenum format, GLenum type, const GLvoid *pixels);
typedef GLboolean (APIENTRY * PFNGLARETEXTURESRESIDENTEXTPROC) (GLsizei n, const GLuint *textures, GLboolean *residences);
typedef void (APIENTRY * PFNGLBINDTEXTUREEXTPROC) (GLenum target, GLuint texture);
typedef void (APIENTRY * PFNGLDELETETEXTURESEXTPROC) (GLsizei n, const GLuint *textures);
typedef void (APIENTRY * PFNGLGENTEXTURESEXTPROC) (GLsizei n, GLuint *textures);
typedef GLboolean (APIENTRY * PFNGLISTEXTUREEXTPROC) (GLuint texture);
typedef void (APIENTRY * PFNGLPRIORITIZETEXTURESEXTPROC) (GLsizei n, const GLuint *textures, const GLclampf *priorities);
typedef void (APIENTRY * PFNGLDETAILTEXFUNCSGISPROC) (GLenum target, GLsizei n, const GLfloat *points);
typedef void (APIENTRY * PFNGLGETDETAILTEXFUNCSGISPROC) (GLenum target, GLfloat *points);
typedef void (APIENTRY * PFNGLSHARPENTEXFUNCSGISPROC) (GLenum target, GLsizei n, const GLfloat *points);
typedef void (APIENTRY * PFNGLGETSHARPENTEXFUNCSGISPROC) (GLenum target, GLfloat *points);
typedef void (APIENTRY * PFNGLSAMPLEMASKSGISPROC) (GLclampf value, GLboolean invert);
typedef void (APIENTRY * PFNGLSAMPLEPATTERNSGISPROC) (GLenum pattern);
typedef void (APIENTRY * PFNGLARRAYELEMENTEXTPROC) (GLint i);
typedef void (APIENTRY * PFNGLCOLORPOINTEREXTPROC) (GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid *pointer);
typedef void (APIENTRY * PFNGLDRAWARRAYSEXTPROC) (GLenum mode, GLint first, GLsizei count);
typedef void (APIENTRY * PFNGLEDGEFLAGPOINTEREXTPROC) (GLsizei stride, GLsizei count, const GLboolean *pointer);
typedef void (APIENTRY * PFNGLGETPOINTERVEXTPROC) (GLenum pname, GLvoid* *params);
typedef void (APIENTRY * PFNGLINDEXPOINTEREXTPROC) (GLenum type, GLsizei stride, GLsizei count, const GLvoid *pointer);
typedef void (APIENTRY * PFNGLNORMALPOINTEREXTPROC) (GLenum type, GLsizei stride, GLsizei count, const GLvoid *pointer);
typedef void (APIENTRY * PFNGLTEXCOORDPOINTEREXTPROC) (GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid *pointer);
typedef void (APIENTRY * PFNGLVERTEXPOINTEREXTPROC) (GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid *pointer);
typedef void (APIENTRY * PFNGLBLENDEQUATIONEXTPROC) (GLenum mode);
typedef void (APIENTRY * PFNGLSPRITEPARAMETERFSGIXPROC) (GLenum pname, GLfloat param);
typedef void (APIENTRY * PFNGLSPRITEPARAMETERFVSGIXPROC) (GLenum pname, const GLfloat *params);
typedef void (APIENTRY * PFNGLSPRITEPARAMETERISGIXPROC) (GLenum pname, GLint param);
typedef void (APIENTRY * PFNGLSPRITEPARAMETERIVSGIXPROC) (GLenum pname, const GLint *params);
typedef void (APIENTRY * PFNGLPOINTPARAMETERFEXTPROC) (GLenum pname, GLfloat param);
typedef void (APIENTRY * PFNGLPOINTPARAMETERFVEXTPROC) (GLenum pname, const GLfloat *params);
typedef void (APIENTRY * PFNGLPOINTPARAMETERFSGISPROC) (GLenum pname, GLfloat param);
typedef void (APIENTRY * PFNGLPOINTPARAMETERFVSGISPROC) (GLenum pname, const GLfloat *params);
typedef GLint (APIENTRY * PFNGLGETINSTRUMENTSSGIXPROC) (void);
typedef void (APIENTRY * PFNGLINSTRUMENTSBUFFERSGIXPROC) (GLsizei size, GLint *buffer);
typedef GLint (APIENTRY * PFNGLPOLLINSTRUMENTSSGIXPROC) (GLint *marker_p);
typedef void (APIENTRY * PFNGLREADINSTRUMENTSSGIXPROC) (GLint marker);
typedef void (APIENTRY * PFNGLSTARTINSTRUMENTSSGIXPROC) (void);
typedef void (APIENTRY * PFNGLSTOPINSTRUMENTSSGIXPROC) (GLint marker);
typedef void (APIENTRY * PFNGLFRAMEZOOMSGIXPROC) (GLint factor);
typedef void (APIENTRY * PFNGLTAGSAMPLEBUFFERSGIXPROC) (void);
typedef void (APIENTRY * PFNGLREFERENCEPLANESGIXPROC) (const GLdouble *equation);
typedef void (APIENTRY * PFNGLFLUSHRASTERSGIXPROC) (void);
typedef void (APIENTRY * PFNGLFOGFUNCSGISPROC) (GLsizei n, const GLfloat *points);
typedef void (APIENTRY * PFNGLGETFOGFUNCSGISPROC) (const GLfloat *points);
typedef void (APIENTRY * PFNGLIMAGETRANSFORMPARAMETERIHPPROC) (GLenum target, GLenum pname, GLint param);
typedef void (APIENTRY * PFNGLIMAGETRANSFORMPARAMETERFHPPROC) (GLenum target, GLenum pname, GLfloat param);
typedef void (APIENTRY * PFNGLIMAGETRANSFORMPARAMETERIVHPPROC) (GLenum target, GLenum pname, const GLint *params);
typedef void (APIENTRY * PFNGLIMAGETRANSFORMPARAMETERFVHPPROC) (GLenum target, GLenum pname, const GLfloat *params);
typedef void (APIENTRY * PFNGLGETIMAGETRANSFORMPARAMETERIVHPPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLGETIMAGETRANSFORMPARAMETERFVHPPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef void (APIENTRY * PFNGLCOLORSUBTABLEEXTPROC) (GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data);
typedef void (APIENTRY * PFNGLCOPYCOLORSUBTABLEEXTPROC) (GLenum target, GLsizei start, GLint x, GLint y, GLsizei width);
typedef void (APIENTRY * PFNGLHINTPGIPROC) (GLenum target, GLint mode);
typedef void (APIENTRY * PFNGLCOLORTABLEEXTPROC) (GLenum target, GLenum internalFormat, GLsizei width, GLenum format, GLenum type, const GLvoid *table);
typedef void (APIENTRY * PFNGLGETCOLORTABLEEXTPROC) (GLenum target, GLenum format, GLenum type, GLvoid *data);
typedef void (APIENTRY * PFNGLGETCOLORTABLEPARAMETERIVEXTPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLGETCOLORTABLEPARAMETERFVEXTPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef void (APIENTRY * PFNGLGETLISTPARAMETERFVSGIXPROC) (GLuint list, GLenum pname, GLfloat *params);
typedef void (APIENTRY * PFNGLGETLISTPARAMETERIVSGIXPROC) (GLuint list, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLLISTPARAMETERFSGIXPROC) (GLuint list, GLenum pname, GLfloat param);
typedef void (APIENTRY * PFNGLLISTPARAMETERFVSGIXPROC) (GLuint list, GLenum pname, const GLfloat *params);
typedef void (APIENTRY * PFNGLLISTPARAMETERISGIXPROC) (GLuint list, GLenum pname, GLint param);
typedef void (APIENTRY * PFNGLLISTPARAMETERIVSGIXPROC) (GLuint list, GLenum pname, const GLint *params);
typedef void (APIENTRY * PFNGLINDEXMATERIALEXTPROC) (GLenum face, GLenum mode);
typedef void (APIENTRY * PFNGLINDEXFUNCEXTPROC) (GLenum func, GLclampf ref);
typedef void (APIENTRY * PFNGLLOCKARRAYSEXTPROC) (GLint first, GLsizei count);
typedef void (APIENTRY * PFNGLUNLOCKARRAYSEXTPROC) (void);
typedef void (APIENTRY * PFNGLCULLPARAMETERDVEXTPROC) (GLenum pname, GLdouble *params);
typedef void (APIENTRY * PFNGLCULLPARAMETERFVEXTPROC) (GLenum pname, GLfloat *params);
typedef void (APIENTRY * PFNGLFRAGMENTCOLORMATERIALSGIXPROC) (GLenum face, GLenum mode);
typedef void (APIENTRY * PFNGLFRAGMENTLIGHTFSGIXPROC) (GLenum light, GLenum pname, GLfloat param);
typedef void (APIENTRY * PFNGLFRAGMENTLIGHTFVSGIXPROC) (GLenum light, GLenum pname, const GLfloat *params);
typedef void (APIENTRY * PFNGLFRAGMENTLIGHTISGIXPROC) (GLenum light, GLenum pname, GLint param);
typedef void (APIENTRY * PFNGLFRAGMENTLIGHTIVSGIXPROC) (GLenum light, GLenum pname, const GLint *params);
typedef void (APIENTRY * PFNGLFRAGMENTLIGHTMODELFSGIXPROC) (GLenum pname, GLfloat param);
typedef void (APIENTRY * PFNGLFRAGMENTLIGHTMODELFVSGIXPROC) (GLenum pname, const GLfloat *params);
typedef void (APIENTRY * PFNGLFRAGMENTLIGHTMODELISGIXPROC) (GLenum pname, GLint param);
typedef void (APIENTRY * PFNGLFRAGMENTLIGHTMODELIVSGIXPROC) (GLenum pname, const GLint *params);
typedef void (APIENTRY * PFNGLFRAGMENTMATERIALFSGIXPROC) (GLenum face, GLenum pname, GLfloat param);
typedef void (APIENTRY * PFNGLFRAGMENTMATERIALFVSGIXPROC) (GLenum face, GLenum pname, const GLfloat *params);
typedef void (APIENTRY * PFNGLFRAGMENTMATERIALISGIXPROC) (GLenum face, GLenum pname, GLint param);
typedef void (APIENTRY * PFNGLFRAGMENTMATERIALIVSGIXPROC) (GLenum face, GLenum pname, const GLint *params);
typedef void (APIENTRY * PFNGLGETFRAGMENTLIGHTFVSGIXPROC) (GLenum light, GLenum pname, GLfloat *params);
typedef void (APIENTRY * PFNGLGETFRAGMENTLIGHTIVSGIXPROC) (GLenum light, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLGETFRAGMENTMATERIALFVSGIXPROC) (GLenum face, GLenum pname, GLfloat *params);
typedef void (APIENTRY * PFNGLGETFRAGMENTMATERIALIVSGIXPROC) (GLenum face, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLLIGHTENVISGIXPROC) (GLenum pname, GLint param);
typedef void (APIENTRY * PFNGLDRAWRANGEELEMENTSEXTPROC) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);
typedef void (APIENTRY * PFNGLAPPLYTEXTUREEXTPROC) (GLenum mode);
typedef void (APIENTRY * PFNGLTEXTURELIGHTEXTPROC) (GLenum pname);
typedef void (APIENTRY * PFNGLTEXTUREMATERIALEXTPROC) (GLenum face, GLenum mode);
typedef void (APIENTRY * PFNGLVERTEXPOINTERVINTELPROC) (GLint size, GLenum type, const GLvoid* *pointer);
typedef void (APIENTRY * PFNGLNORMALPOINTERVINTELPROC) (GLenum type, const GLvoid* *pointer);
typedef void (APIENTRY * PFNGLCOLORPOINTERVINTELPROC) (GLint size, GLenum type, const GLvoid* *pointer);
typedef void (APIENTRY * PFNGLTEXCOORDPOINTERVINTELPROC) (GLint size, GLenum type, const GLvoid* *pointer);
typedef void (APIENTRY * PFNGLPIXELTRANSFORMPARAMETERIEXTPROC) (GLenum target, GLenum pname, GLint param);
typedef void (APIENTRY * PFNGLPIXELTRANSFORMPARAMETERFEXTPROC) (GLenum target, GLenum pname, GLfloat param);
typedef void (APIENTRY * PFNGLPIXELTRANSFORMPARAMETERIVEXTPROC) (GLenum target, GLenum pname, const GLint *params);
typedef void (APIENTRY * PFNGLPIXELTRANSFORMPARAMETERFVEXTPROC) (GLenum target, GLenum pname, const GLfloat *params);
typedef void (APIENTRY * PFNGLSECONDARYCOLOR3BEXTPROC) (GLbyte red, GLbyte green, GLbyte blue);
typedef void (APIENTRY * PFNGLSECONDARYCOLOR3BVEXTPROC) (const GLbyte *v);
typedef void (APIENTRY * PFNGLSECONDARYCOLOR3DEXTPROC) (GLdouble red, GLdouble green, GLdouble blue);
typedef void (APIENTRY * PFNGLSECONDARYCOLOR3DVEXTPROC) (const GLdouble *v);
typedef void (APIENTRY * PFNGLSECONDARYCOLOR3FEXTPROC) (GLfloat red, GLfloat green, GLfloat blue);
typedef void (APIENTRY * PFNGLSECONDARYCOLOR3FVEXTPROC) (const GLfloat *v);
typedef void (APIENTRY * PFNGLSECONDARYCOLOR3IEXTPROC) (GLint red, GLint green, GLint blue);
typedef void (APIENTRY * PFNGLSECONDARYCOLOR3IVEXTPROC) (const GLint *v);
typedef void (APIENTRY * PFNGLSECONDARYCOLOR3SEXTPROC) (GLshort red, GLshort green, GLshort blue);
typedef void (APIENTRY * PFNGLSECONDARYCOLOR3SVEXTPROC) (const GLshort *v);
typedef void (APIENTRY * PFNGLSECONDARYCOLOR3UBEXTPROC) (GLubyte red, GLubyte green, GLubyte blue);
typedef void (APIENTRY * PFNGLSECONDARYCOLOR3UBVEXTPROC) (const GLubyte *v);
typedef void (APIENTRY * PFNGLSECONDARYCOLOR3UIEXTPROC) (GLuint red, GLuint green, GLuint blue);
typedef void (APIENTRY * PFNGLSECONDARYCOLOR3UIVEXTPROC) (const GLuint *v);
typedef void (APIENTRY * PFNGLSECONDARYCOLOR3USEXTPROC) (GLushort red, GLushort green, GLushort blue);
typedef void (APIENTRY * PFNGLSECONDARYCOLOR3USVEXTPROC) (const GLushort *v);
typedef void (APIENTRY * PFNGLSECONDARYCOLORPOINTEREXTPROC) (GLint size, GLenum type, GLsizei stride, GLvoid *pointer);
typedef void (APIENTRY * PFNGLTEXTURENORMALEXTPROC) (GLenum mode);
typedef void (APIENTRY * PFNGLMULTIDRAWARRAYSEXTPROC) (GLenum mode, GLint *first, GLsizei *count, GLsizei primcount);
typedef void (APIENTRY * PFNGLMULTIDRAWELEMENTSEXTPROC) (GLenum mode, const GLsizei *count, GLenum type, const GLvoid* *indices, GLsizei primcount);
typedef void (APIENTRY * PFNGLFOGCOORDFEXTPROC) (GLfloat coord);
typedef void (APIENTRY * PFNGLFOGCOORDFVEXTPROC) (const GLfloat *coord);
typedef void (APIENTRY * PFNGLFOGCOORDDEXTPROC) (GLdouble coord);
typedef void (APIENTRY * PFNGLFOGCOORDDVEXTPROC) (const GLdouble *coord);
typedef void (APIENTRY * PFNGLFOGCOORDPOINTEREXTPROC) (GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void (APIENTRY * PFNGLTANGENT3BEXTPROC) (GLbyte tx, GLbyte ty, GLbyte tz);
typedef void (APIENTRY * PFNGLTANGENT3BVEXTPROC) (const GLbyte *v);
typedef void (APIENTRY * PFNGLTANGENT3DEXTPROC) (GLdouble tx, GLdouble ty, GLdouble tz);
typedef void (APIENTRY * PFNGLTANGENT3DVEXTPROC) (const GLdouble *v);
typedef void (APIENTRY * PFNGLTANGENT3FEXTPROC) (GLfloat tx, GLfloat ty, GLfloat tz);
typedef void (APIENTRY * PFNGLTANGENT3FVEXTPROC) (const GLfloat *v);
typedef void (APIENTRY * PFNGLTANGENT3IEXTPROC) (GLint tx, GLint ty, GLint tz);
typedef void (APIENTRY * PFNGLTANGENT3IVEXTPROC) (const GLint *v);
typedef void (APIENTRY * PFNGLTANGENT3SEXTPROC) (GLshort tx, GLshort ty, GLshort tz);
typedef void (APIENTRY * PFNGLTANGENT3SVEXTPROC) (const GLshort *v);
typedef void (APIENTRY * PFNGLBINORMAL3BEXTPROC) (GLbyte bx, GLbyte by, GLbyte bz);
typedef void (APIENTRY * PFNGLBINORMAL3BVEXTPROC) (const GLbyte *v);
typedef void (APIENTRY * PFNGLBINORMAL3DEXTPROC) (GLdouble bx, GLdouble by, GLdouble bz);
typedef void (APIENTRY * PFNGLBINORMAL3DVEXTPROC) (const GLdouble *v);
typedef void (APIENTRY * PFNGLBINORMAL3FEXTPROC) (GLfloat bx, GLfloat by, GLfloat bz);
typedef void (APIENTRY * PFNGLBINORMAL3FVEXTPROC) (const GLfloat *v);
typedef void (APIENTRY * PFNGLBINORMAL3IEXTPROC) (GLint bx, GLint by, GLint bz);
typedef void (APIENTRY * PFNGLBINORMAL3IVEXTPROC) (const GLint *v);
typedef void (APIENTRY * PFNGLBINORMAL3SEXTPROC) (GLshort bx, GLshort by, GLshort bz);
typedef void (APIENTRY * PFNGLBINORMAL3SVEXTPROC) (const GLshort *v);
typedef void (APIENTRY * PFNGLTANGENTPOINTEREXTPROC) (GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void (APIENTRY * PFNGLBINORMALPOINTEREXTPROC) (GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void (APIENTRY * PFNGLFINISHTEXTURESUNXPROC) (void);
typedef void (APIENTRY * PFNGLGLOBALALPHAFACTORBSUNPROC) (GLbyte factor);
typedef void (APIENTRY * PFNGLGLOBALALPHAFACTORSSUNPROC) (GLshort factor);
typedef void (APIENTRY * PFNGLGLOBALALPHAFACTORISUNPROC) (GLint factor);
typedef void (APIENTRY * PFNGLGLOBALALPHAFACTORFSUNPROC) (GLfloat factor);
typedef void (APIENTRY * PFNGLGLOBALALPHAFACTORDSUNPROC) (GLdouble factor);
typedef void (APIENTRY * PFNGLGLOBALALPHAFACTORUBSUNPROC) (GLubyte factor);
typedef void (APIENTRY * PFNGLGLOBALALPHAFACTORUSSUNPROC) (GLushort factor);
typedef void (APIENTRY * PFNGLGLOBALALPHAFACTORUISUNPROC) (GLuint factor);
typedef void (APIENTRY * PFNGLREPLACEMENTCODEUISUNPROC) (GLuint code);
typedef void (APIENTRY * PFNGLREPLACEMENTCODEUSSUNPROC) (GLushort code);
typedef void (APIENTRY * PFNGLREPLACEMENTCODEUBSUNPROC) (GLubyte code);
typedef void (APIENTRY * PFNGLREPLACEMENTCODEUIVSUNPROC) (const GLuint *code);
typedef void (APIENTRY * PFNGLREPLACEMENTCODEUSVSUNPROC) (const GLushort *code);
typedef void (APIENTRY * PFNGLREPLACEMENTCODEUBVSUNPROC) (const GLubyte *code);
typedef void (APIENTRY * PFNGLREPLACEMENTCODEPOINTERSUNPROC) (GLenum type, GLsizei stride, const GLvoid* *pointer);
typedef void (APIENTRY * PFNGLCOLOR4UBVERTEX2FSUNPROC) (GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y);
typedef void (APIENTRY * PFNGLCOLOR4UBVERTEX2FVSUNPROC) (const GLubyte *c, const GLfloat *v);
typedef void (APIENTRY * PFNGLCOLOR4UBVERTEX3FSUNPROC) (GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRY * PFNGLCOLOR4UBVERTEX3FVSUNPROC) (const GLubyte *c, const GLfloat *v);
typedef void (APIENTRY * PFNGLCOLOR3FVERTEX3FSUNPROC) (GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRY * PFNGLCOLOR3FVERTEX3FVSUNPROC) (const GLfloat *c, const GLfloat *v);
typedef void (APIENTRY * PFNGLNORMAL3FVERTEX3FSUNPROC) (GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRY * PFNGLNORMAL3FVERTEX3FVSUNPROC) (const GLfloat *n, const GLfloat *v);
typedef void (APIENTRY * PFNGLCOLOR4FNORMAL3FVERTEX3FSUNPROC) (GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRY * PFNGLCOLOR4FNORMAL3FVERTEX3FVSUNPROC) (const GLfloat *c, const GLfloat *n, const GLfloat *v);
typedef void (APIENTRY * PFNGLTEXCOORD2FVERTEX3FSUNPROC) (GLfloat s, GLfloat t, GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRY * PFNGLTEXCOORD2FVERTEX3FVSUNPROC) (const GLfloat *tc, const GLfloat *v);
typedef void (APIENTRY * PFNGLTEXCOORD4FVERTEX4FSUNPROC) (GLfloat s, GLfloat t, GLfloat p, GLfloat q, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (APIENTRY * PFNGLTEXCOORD4FVERTEX4FVSUNPROC) (const GLfloat *tc, const GLfloat *v);
typedef void (APIENTRY * PFNGLTEXCOORD2FCOLOR4UBVERTEX3FSUNPROC) (GLfloat s, GLfloat t, GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRY * PFNGLTEXCOORD2FCOLOR4UBVERTEX3FVSUNPROC) (const GLfloat *tc, const GLubyte *c, const GLfloat *v);
typedef void (APIENTRY * PFNGLTEXCOORD2FCOLOR3FVERTEX3FSUNPROC) (GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRY * PFNGLTEXCOORD2FCOLOR3FVERTEX3FVSUNPROC) (const GLfloat *tc, const GLfloat *c, const GLfloat *v);
typedef void (APIENTRY * PFNGLTEXCOORD2FNORMAL3FVERTEX3FSUNPROC) (GLfloat s, GLfloat t, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRY * PFNGLTEXCOORD2FNORMAL3FVERTEX3FVSUNPROC) (const GLfloat *tc, const GLfloat *n, const GLfloat *v);
typedef void (APIENTRY * PFNGLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUNPROC) (GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRY * PFNGLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUNPROC) (const GLfloat *tc, const GLfloat *c, const GLfloat *n, const GLfloat *v);
typedef void (APIENTRY * PFNGLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FSUNPROC) (GLfloat s, GLfloat t, GLfloat p, GLfloat q, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (APIENTRY * PFNGLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FVSUNPROC) (const GLfloat *tc, const GLfloat *c, const GLfloat *n, const GLfloat *v);
typedef void (APIENTRY * PFNGLREPLACEMENTCODEUIVERTEX3FSUNPROC) (GLenum rc, GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRY * PFNGLREPLACEMENTCODEUIVERTEX3FVSUNPROC) (const GLenum *rc, const GLfloat *v);
typedef void (APIENTRY * PFNGLREPLACEMENTCODEUICOLOR4UBVERTEX3FSUNPROC) (GLenum rc, GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRY * PFNGLREPLACEMENTCODEUICOLOR4UBVERTEX3FVSUNPROC) (const GLenum *rc, const GLubyte *c, const GLfloat *v);
typedef void (APIENTRY * PFNGLREPLACEMENTCODEUICOLOR3FVERTEX3FSUNPROC) (GLenum rc, GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRY * PFNGLREPLACEMENTCODEUICOLOR3FVERTEX3FVSUNPROC) (const GLenum *rc, const GLfloat *c, const GLfloat *v);
typedef void (APIENTRY * PFNGLREPLACEMENTCODEUINORMAL3FVERTEX3FSUNPROC) (GLenum rc, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRY * PFNGLREPLACEMENTCODEUINORMAL3FVERTEX3FVSUNPROC) (const GLenum *rc, const GLfloat *n, const GLfloat *v);
typedef void (APIENTRY * PFNGLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FSUNPROC) (GLenum rc, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRY * PFNGLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FVSUNPROC) (const GLenum *rc, const GLfloat *c, const GLfloat *n, const GLfloat *v);
typedef void (APIENTRY * PFNGLREPLACEMENTCODEUITEXCOORD2FVERTEX3FSUNPROC) (GLenum rc, GLfloat s, GLfloat t, GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRY * PFNGLREPLACEMENTCODEUITEXCOORD2FVERTEX3FVSUNPROC) (const GLenum *rc, const GLfloat *tc, const GLfloat *v);
typedef void (APIENTRY * PFNGLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FSUNPROC) (GLenum rc, GLfloat s, GLfloat t, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRY * PFNGLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FVSUNPROC) (const GLenum *rc, const GLfloat *tc, const GLfloat *n, const GLfloat *v);
typedef void (APIENTRY * PFNGLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUNPROC) (GLenum rc, GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRY * PFNGLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUNPROC) (const GLenum *rc, const GLfloat *tc, const GLfloat *c, const GLfloat *n, const GLfloat *v);
typedef void (APIENTRY * PFNGLBLENDFUNCSEPARATEEXTPROC) (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
typedef void (APIENTRY * PFNGLVERTEXWEIGHTFEXTPROC) (GLfloat weight);
typedef void (APIENTRY * PFNGLVERTEXWEIGHTFVEXTPROC) (const GLfloat *weight);
typedef void (APIENTRY * PFNGLVERTEXWEIGHTPOINTEREXTPROC) (GLsizei size, GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void (APIENTRY * PFNGLFLUSHVERTEXARRAYRANGENVPROC) (void);
typedef void (APIENTRY * PFNGLVERTEXARRAYRANGENVPROC) (GLsizei size, const GLvoid *pointer);
typedef void (APIENTRY * PFNGLCOMBINERPARAMETERFVNVPROC) (GLenum pname, const GLfloat *params);
typedef void (APIENTRY * PFNGLCOMBINERPARAMETERFNVPROC) (GLenum pname, GLfloat param);
typedef void (APIENTRY * PFNGLCOMBINERPARAMETERIVNVPROC) (GLenum pname, const GLint *params);
typedef void (APIENTRY * PFNGLCOMBINERPARAMETERINVPROC) (GLenum pname, GLint param);
typedef void (APIENTRY * PFNGLCOMBINERINPUTNVPROC) (GLenum stage, GLenum portion, GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage);
typedef void (APIENTRY * PFNGLCOMBINEROUTPUTNVPROC) (GLenum stage, GLenum portion, GLenum abOutput, GLenum cdOutput, GLenum sumOutput, GLenum scale, GLenum bias, GLboolean abDotProduct, GLboolean cdDotProduct, GLboolean muxSum);
typedef void (APIENTRY * PFNGLFINALCOMBINERINPUTNVPROC) (GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage);
typedef void (APIENTRY * PFNGLGETCOMBINERINPUTPARAMETERFVNVPROC) (GLenum stage, GLenum portion, GLenum variable, GLenum pname, GLfloat *params);
typedef void (APIENTRY * PFNGLGETCOMBINERINPUTPARAMETERIVNVPROC) (GLenum stage, GLenum portion, GLenum variable, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLGETCOMBINEROUTPUTPARAMETERFVNVPROC) (GLenum stage, GLenum portion, GLenum pname, GLfloat *params);
typedef void (APIENTRY * PFNGLGETCOMBINEROUTPUTPARAMETERIVNVPROC) (GLenum stage, GLenum portion, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLGETFINALCOMBINERINPUTPARAMETERFVNVPROC) (GLenum variable, GLenum pname, GLfloat *params);
typedef void (APIENTRY * PFNGLGETFINALCOMBINERINPUTPARAMETERIVNVPROC) (GLenum variable, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLRESIZEBUFFERSMESAPROC) (void);
typedef void (APIENTRY * PFNGLWINDOWPOS2DMESAPROC) (GLdouble x, GLdouble y);
typedef void (APIENTRY * PFNGLWINDOWPOS2DVMESAPROC) (const GLdouble *v);
typedef void (APIENTRY * PFNGLWINDOWPOS2FMESAPROC) (GLfloat x, GLfloat y);
typedef void (APIENTRY * PFNGLWINDOWPOS2FVMESAPROC) (const GLfloat *v);
typedef void (APIENTRY * PFNGLWINDOWPOS2IMESAPROC) (GLint x, GLint y);
typedef void (APIENTRY * PFNGLWINDOWPOS2IVMESAPROC) (const GLint *v);
typedef void (APIENTRY * PFNGLWINDOWPOS2SMESAPROC) (GLshort x, GLshort y);
typedef void (APIENTRY * PFNGLWINDOWPOS2SVMESAPROC) (const GLshort *v);
typedef void (APIENTRY * PFNGLWINDOWPOS3DMESAPROC) (GLdouble x, GLdouble y, GLdouble z);
typedef void (APIENTRY * PFNGLWINDOWPOS3DVMESAPROC) (const GLdouble *v);
typedef void (APIENTRY * PFNGLWINDOWPOS3FMESAPROC) (GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRY * PFNGLWINDOWPOS3FVMESAPROC) (const GLfloat *v);
typedef void (APIENTRY * PFNGLWINDOWPOS3IMESAPROC) (GLint x, GLint y, GLint z);
typedef void (APIENTRY * PFNGLWINDOWPOS3IVMESAPROC) (const GLint *v);
typedef void (APIENTRY * PFNGLWINDOWPOS3SMESAPROC) (GLshort x, GLshort y, GLshort z);
typedef void (APIENTRY * PFNGLWINDOWPOS3SVMESAPROC) (const GLshort *v);
typedef void (APIENTRY * PFNGLWINDOWPOS4DMESAPROC) (GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (APIENTRY * PFNGLWINDOWPOS4DVMESAPROC) (const GLdouble *v);
typedef void (APIENTRY * PFNGLWINDOWPOS4FMESAPROC) (GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (APIENTRY * PFNGLWINDOWPOS4FVMESAPROC) (const GLfloat *v);
typedef void (APIENTRY * PFNGLWINDOWPOS4IMESAPROC) (GLint x, GLint y, GLint z, GLint w);
typedef void (APIENTRY * PFNGLWINDOWPOS4IVMESAPROC) (const GLint *v);
typedef void (APIENTRY * PFNGLWINDOWPOS4SMESAPROC) (GLshort x, GLshort y, GLshort z, GLshort w);
typedef void (APIENTRY * PFNGLWINDOWPOS4SVMESAPROC) (const GLshort *v);
typedef void (APIENTRY * PFNGLMULTIMODEDRAWARRAYSIBMPROC) (GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount, GLint modestride);
typedef void (APIENTRY * PFNGLMULTIMODEDRAWELEMENTSIBMPROC) (const GLenum *mode, const GLsizei *count, GLenum type, const GLvoid* *indices, GLsizei primcount, GLint modestride);
typedef void (APIENTRY * PFNGLCOLORPOINTERLISTIBMPROC) (GLint size, GLenum type, GLint stride, const GLvoid* *pointer, GLint ptrstride);
typedef void (APIENTRY * PFNGLSECONDARYCOLORPOINTERLISTIBMPROC) (GLint size, GLenum type, GLint stride, const GLvoid* *pointer, GLint ptrstride);
typedef void (APIENTRY * PFNGLEDGEFLAGPOINTERLISTIBMPROC) (GLint stride, const GLboolean* *pointer, GLint ptrstride);
typedef void (APIENTRY * PFNGLFOGCOORDPOINTERLISTIBMPROC) (GLenum type, GLint stride, const GLvoid* *pointer, GLint ptrstride);
typedef void (APIENTRY * PFNGLINDEXPOINTERLISTIBMPROC) (GLenum type, GLint stride, const GLvoid* *pointer, GLint ptrstride);
typedef void (APIENTRY * PFNGLNORMALPOINTERLISTIBMPROC) (GLenum type, GLint stride, const GLvoid* *pointer, GLint ptrstride);
typedef void (APIENTRY * PFNGLTEXCOORDPOINTERLISTIBMPROC) (GLint size, GLenum type, GLint stride, const GLvoid* *pointer, GLint ptrstride);
typedef void (APIENTRY * PFNGLVERTEXPOINTERLISTIBMPROC) (GLint size, GLenum type, GLint stride, const GLvoid* *pointer, GLint ptrstride);
typedef void (APIENTRY * PFNGLTBUFFERMASK3DFXPROC) (GLuint mask);
typedef void (APIENTRY * PFNGLSAMPLEMASKEXTPROC) (GLclampf value, GLboolean invert);
typedef void (APIENTRY * PFNGLSAMPLEPATTERNEXTPROC) (GLenum pattern);
typedef void (APIENTRY * PFNGLTEXTURECOLORMASKSGISPROC) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);


#ifdef CSGL_FOR_ALL
#define CSGL_VERSION_1_2
#define CSGL_ARB_multitexture
#define CSGL_ARB_transpose_matrix
#define CSGL_ARB_multisample
#define CSGL_ARB_texture_env_add
#define CSGL_ARB_texture_cube_map
#define CSGL_ARB_texture_compression
#define CSGL_EXT_abgr
#define CSGL_EXT_blend_color
#define CSGL_EXT_polygon_offset
#define CSGL_EXT_texture
#define CSGL_EXT_texture3D
#define CSGL_SGIS_texture_filter4
#define CSGL_EXT_subtexture
#define CSGL_EXT_copy_texture
#define CSGL_EXT_histogram
#define CSGL_EXT_convolution
#define CSGL_EXT_color_matrix
#define CSGL_SGI_color_table
#define CSGL_SGIX_pixel_texture
#define CSGL_SGIS_pixel_texture
#define CSGL_SGIS_texture4D
#define CSGL_SGI_texture_color_table
#define CSGL_EXT_cmyka
#define CSGL_EXT_texture_object
#define CSGL_SGIS_detail_texture
#define CSGL_SGIS_sharpen_texture
#define CSGL_EXT_packed_pixels
#define CSGL_SGIS_texture_lod
#define CSGL_SGIS_multisample
#define CSGL_EXT_rescale_normal
#define CSGL_EXT_vertex_array
#define CSGL_EXT_misc_attribute
#define CSGL_SGIS_generate_mipmap
#define CSGL_SGIX_clipmap
#define CSGL_SGIX_shadow
#define CSGL_SGIS_texture_edge_clamp
#define CSGL_SGIS_texture_border_clamp
#define CSGL_EXT_blend_minmax
#define CSGL_EXT_blend_subtract
#define CSGL_EXT_blend_logic_op
#define CSGL_SGIX_interlace
#define CSGL_SGIX_pixel_tiles
#define CSGL_SGIX_texture_select
#define CSGL_SGIX_sprite
#define CSGL_SGIX_texture_multi_buffer
#define CSGL_EXT_point_parameters
#define CSGL_SGIX_instruments
#define CSGL_SGIX_texture_scale_bias
#define CSGL_SGIX_framezoom
#define CSGL_SGIX_tag_sample_buffer
#define CSGL_SGIX_reference_plane
#define CSGL_SGIX_flush_raster
#define CSGL_SGIX_depth_texture
#define CSGL_SGIS_fog_function
#define CSGL_SGIX_fog_offset
#define CSGL_HP_image_transform
#define CSGL_HP_convolution_border_modes
#define CSGL_SGIX_texture_add_env
#define CSGL_EXT_color_subtable
#define CSGL_PGI_vertex_hints
#define CSGL_PGI_misc_hints
#define CSGL_EXT_paletted_texture
#define CSGL_EXT_clip_volume_hint
#define CSGL_SGIX_list_priority
#define CSGL_SGIX_ir_instrument1
#define CSGL_SGIX_calligraphic_fragment
#define CSGL_SGIX_texture_lod_bias
#define CSGL_SGIX_shadow_ambient
#define CSGL_EXT_index_texture
#define CSGL_EXT_index_material
#define CSGL_EXT_index_func
#define CSGL_EXT_index_array_formats
#define CSGL_EXT_compiled_vertex_array
#define CSGL_EXT_cull_vertex
#define CSGL_SGIX_ycrcb
#define CSGL_SGIX_fragment_lighting
#define CSGL_IBM_rasterpos_clip
#define CSGL_HP_texture_lighting
#define CSGL_EXT_draw_range_elements
#define CSGL_WIN_phong_shading
#define CSGL_WIN_specular_fog
#define CSGL_EXT_light_texture
#define CSGL_SGIX_blend_alpha_minmax
#define CSGL_EXT_bgra
#define CSGL_INTEL_parallel_arrays
#define CSGL_HP_occlusion_test
#define CSGL_EXT_pixel_transform
#define CSGL_EXT_pixel_transform_color_table
#define CSGL_EXT_shared_texture_palette
#define CSGL_EXT_separate_specular_color
#define CSGL_EXT_secondary_color
#define CSGL_EXT_texture_perturb_normal
#define CSGL_EXT_multi_draw_arrays
#define CSGL_EXT_fog_coord
#define CSGL_REND_screen_coordinates
#define CSGL_EXT_coordinate_frame
#define CSGL_EXT_texture_env_combine
#define CSGL_APPLE_specular_vector
#define CSGL_APPLE_transform_hint
#define CSGL_SGIX_fog_scale
#define CSGL_SUNX_constant_data
#define CSGL_SUN_global_alpha
#define CSGL_SUN_triangle_list
#define CSGL_SUN_vertex
#define CSGL_EXT_blend_func_separate
#define CSGL_INGR_color_clamp
#define CSGL_INGR_interlace_read
#define CSGL_EXT_stencil_wrap
#define CSGL_EXT_422_pixels
#define CSGL_NV_texgen_reflection
#define CSGL_SUN_convolution_border_modes
#define CSGL_EXT_texture_env_add
#define CSGL_EXT_texture_lod_bias
#define CSGL_EXT_texture_filter_anisotropic
#define CSGL_EXT_vertex_weighting
#define CSGL_NV_light_max_exponent
#define CSGL_NV_vertex_array_range
#define CSGL_NV_register_combiners
#define CSGL_NV_fog_distance
#define CSGL_NV_texgen_emboss
#define CSGL_NV_blend_square
#define CSGL_NV_texture_env_combine4
#define CSGL_MESA_resize_buffers
#define CSGL_MESA_window_pos
#define CSGL_IBM_cull_vertex
#define CSGL_IBM_multimode_draw_arrays
#define CSGL_IBM_vertex_array_lists
#define CSGL_SGIX_subsample
#define CSGL_SGIX_ycrcba
#define CSGL_SGIX_ycrcb_subsample
#define CSGL_SGIX_depth_pass_instrument
#define CSGL_3DFX_texture_compression_FXT1
#define CSGL_3DFX_multisample
#define CSGL_3DFX_tbuffer
#define CSGL_EXT_multisample
#define CSGL_SGI_vertex_preclip
#define CSGL_SGIX_convolution_accuracy
#define CSGL_SGIX_resample
#define CSGL_SGIS_point_line_texgen
#define CSGL_SGIS_texture_color_mask
#endif

#define CS_PREP_GL_FUNCTION(fType,fName) \
CSGL_FUNCTION(fType,fName)

#ifdef CSGL_VERSION_1_2
CS_PREP_GL_FUNCTION (PFNGLBLENDCOLORPROC, glBlendColor);
CS_PREP_GL_FUNCTION (PFNGLBLENDEQUATIONPROC, glBlendEquation);
CS_PREP_GL_FUNCTION (PFNGLDRAWRANGEELEMENTSPROC, glDrawRangeElements);
CS_PREP_GL_FUNCTION (PFNGLCOLORTABLEPROC, glColorTable);
CS_PREP_GL_FUNCTION (PFNGLCOLORTABLEPARAMETERFVPROC, glColorTableParameterfv);
CS_PREP_GL_FUNCTION (PFNGLCOLORTABLEPARAMETERIVPROC, glColorTableParameteriv);
CS_PREP_GL_FUNCTION (PFNGLCOPYCOLORTABLEPROC, glCopyColorTable);
CS_PREP_GL_FUNCTION (PFNGLGETCOLORTABLEPROC, glGetColorTable);
CS_PREP_GL_FUNCTION (PFNGLGETCOLORTABLEPARAMETERFVPROC, glGetColorTableParameterfv);
CS_PREP_GL_FUNCTION (PFNGLGETCOLORTABLEPARAMETERIVPROC, glGetColorTableParameteriv);
CS_PREP_GL_FUNCTION (PFNGLCOLORSUBTABLEPROC, glColorSubTable);
CS_PREP_GL_FUNCTION (PFNGLCOPYCOLORSUBTABLEPROC, glCopyColorSubTable);
CS_PREP_GL_FUNCTION (PFNGLCONVOLUTIONFILTER1DPROC, glConvolutionFilter1D);
CS_PREP_GL_FUNCTION (PFNGLCONVOLUTIONFILTER2DPROC, glConvolutionFilter2D);
CS_PREP_GL_FUNCTION (PFNGLCONVOLUTIONPARAMETERFPROC, glConvolutionParameterf);
CS_PREP_GL_FUNCTION (PFNGLCONVOLUTIONPARAMETERFVPROC, glConvolutionParameterfv);
CS_PREP_GL_FUNCTION (PFNGLCONVOLUTIONPARAMETERIPROC, glConvolutionParameteri);
CS_PREP_GL_FUNCTION (PFNGLCONVOLUTIONPARAMETERIVPROC, glConvolutionParameteriv);
CS_PREP_GL_FUNCTION (PFNGLCOPYCONVOLUTIONFILTER1DPROC, glCopyConvolutionFilter1D);
CS_PREP_GL_FUNCTION (PFNGLCOPYCONVOLUTIONFILTER2DPROC, glCopyConvolutionFilter2D);
CS_PREP_GL_FUNCTION (PFNGLGETCONVOLUTIONFILTERPROC, glGetConvolutionFilter);
CS_PREP_GL_FUNCTION (PFNGLGETCONVOLUTIONPARAMETERFVPROC, glGetConvolutionParameterfv);
CS_PREP_GL_FUNCTION (PFNGLGETCONVOLUTIONPARAMETERIVPROC, glGetConvolutionParameteriv);
CS_PREP_GL_FUNCTION (PFNGLGETSEPARABLEFILTERPROC, glGetSeparableFilter);
CS_PREP_GL_FUNCTION (PFNGLSEPARABLEFILTER2DPROC, glSeparableFilter2D);
CS_PREP_GL_FUNCTION (PFNGLGETHISTOGRAMPROC, glGetHistogram);
CS_PREP_GL_FUNCTION (PFNGLGETHISTOGRAMPARAMETERFVPROC, glGetHistogramParameterfv);
CS_PREP_GL_FUNCTION (PFNGLGETHISTOGRAMPARAMETERIVPROC, glGetHistogramParameteriv);
CS_PREP_GL_FUNCTION (PFNGLGETMINMAXPROC, glGetMinmax);
CS_PREP_GL_FUNCTION (PFNGLGETMINMAXPARAMETERFVPROC, glGetMinmaxParameterfv);
CS_PREP_GL_FUNCTION (PFNGLGETMINMAXPARAMETERIVPROC, glGetMinmaxParameteriv);
CS_PREP_GL_FUNCTION (PFNGLHISTOGRAMPROC, glHistogram);
CS_PREP_GL_FUNCTION (PFNGLMINMAXPROC, glMinmax);
CS_PREP_GL_FUNCTION (PFNGLRESETHISTOGRAMPROC, glResetHistogram);
CS_PREP_GL_FUNCTION (PFNGLRESETMINMAXPROC, glResetMinmax);
CS_PREP_GL_FUNCTION (PFNGLTEXIMAGE3DPROC, glTexImage3D);
CS_PREP_GL_FUNCTION (PFNGLTEXSUBIMAGE3DPROC, glTexSubImage3D);
CS_PREP_GL_FUNCTION (PFNGLCOPYTEXSUBIMAGE3DPROC, glCopyTexSubImage3D);
#endif

#ifdef CSGL_ARB_multitexture
CS_PREP_GL_FUNCTION (PFNGLACTIVETEXTUREARBPROC, glActiveTextureARB);
CS_PREP_GL_FUNCTION (PFNGLCLIENTACTIVETEXTUREARBPROC, glClientActiveTextureARB);
CS_PREP_GL_FUNCTION (PFNGLMULTITEXCOORD1DARBPROC, glMultiTexCoord1dARB);
CS_PREP_GL_FUNCTION (PFNGLMULTITEXCOORD1DVARBPROC, glMultiTexCoord1dvARB);
CS_PREP_GL_FUNCTION (PFNGLMULTITEXCOORD1FARBPROC, glMultiTexCoord1fARB);
CS_PREP_GL_FUNCTION (PFNGLMULTITEXCOORD1FVARBPROC, glMultiTexCoord1fvARB);
CS_PREP_GL_FUNCTION (PFNGLMULTITEXCOORD1IARBPROC, glMultiTexCoord1iARB);
CS_PREP_GL_FUNCTION (PFNGLMULTITEXCOORD1IVARBPROC, glMultiTexCoord1ivARB);
CS_PREP_GL_FUNCTION (PFNGLMULTITEXCOORD1SARBPROC, glMultiTexCoord1sARB);
CS_PREP_GL_FUNCTION (PFNGLMULTITEXCOORD1SVARBPROC, glMultiTexCoord1svARB);
CS_PREP_GL_FUNCTION (PFNGLMULTITEXCOORD2DARBPROC, glMultiTexCoord2dARB);
CS_PREP_GL_FUNCTION (PFNGLMULTITEXCOORD2DVARBPROC, glMultiTexCoord2dvARB);
CS_PREP_GL_FUNCTION (PFNGLMULTITEXCOORD2FARBPROC, glMultiTexCoord2fARB);
CS_PREP_GL_FUNCTION (PFNGLMULTITEXCOORD2FVARBPROC, glMultiTexCoord2fvARB);
CS_PREP_GL_FUNCTION (PFNGLMULTITEXCOORD2IARBPROC, glMultiTexCoord2iARB);
CS_PREP_GL_FUNCTION (PFNGLMULTITEXCOORD2IVARBPROC, glMultiTexCoord2ivARB);
CS_PREP_GL_FUNCTION (PFNGLMULTITEXCOORD2SARBPROC, glMultiTexCoord2sARB);
CS_PREP_GL_FUNCTION (PFNGLMULTITEXCOORD2SVARBPROC, glMultiTexCoord2svARB);
CS_PREP_GL_FUNCTION (PFNGLMULTITEXCOORD3DARBPROC, glMultiTexCoord3dARB);
CS_PREP_GL_FUNCTION (PFNGLMULTITEXCOORD3DVARBPROC, glMultiTexCoord3dvARB);
CS_PREP_GL_FUNCTION (PFNGLMULTITEXCOORD3FARBPROC, glMultiTexCoord3fARB);
CS_PREP_GL_FUNCTION (PFNGLMULTITEXCOORD3FVARBPROC, glMultiTexCoord3fvARB);
CS_PREP_GL_FUNCTION (PFNGLMULTITEXCOORD3IARBPROC, glMultiTexCoord3iARB);
CS_PREP_GL_FUNCTION (PFNGLMULTITEXCOORD3IVARBPROC, glMultiTexCoord3ivARB);
CS_PREP_GL_FUNCTION (PFNGLMULTITEXCOORD3SARBPROC, glMultiTexCoord3sARB);
CS_PREP_GL_FUNCTION (PFNGLMULTITEXCOORD3SVARBPROC, glMultiTexCoord3svARB);
CS_PREP_GL_FUNCTION (PFNGLMULTITEXCOORD4DARBPROC, glMultiTexCoord4dARB);
CS_PREP_GL_FUNCTION (PFNGLMULTITEXCOORD4DVARBPROC, glMultiTexCoord4dvARB);
CS_PREP_GL_FUNCTION (PFNGLMULTITEXCOORD4FARBPROC, glMultiTexCoord4fARB);
CS_PREP_GL_FUNCTION (PFNGLMULTITEXCOORD4FVARBPROC, glMultiTexCoord4fvARB);
CS_PREP_GL_FUNCTION (PFNGLMULTITEXCOORD4IARBPROC, glMultiTexCoord4iARB);
CS_PREP_GL_FUNCTION (PFNGLMULTITEXCOORD4IVARBPROC, glMultiTexCoord4ivARB);
CS_PREP_GL_FUNCTION (PFNGLMULTITEXCOORD4SARBPROC, glMultiTexCoord4sARB);
CS_PREP_GL_FUNCTION (PFNGLMULTITEXCOORD4SVARBPROC, glMultiTexCoord4svARB);
#endif

#ifdef CSGL_ARB_transpose_matrix
CS_PREP_GL_FUNCTION (PFNGLLOADTRANSPOSEMATRIXFARBPROC, glLoadTransposeMatrixfARB);
CS_PREP_GL_FUNCTION (PFNGLLOADTRANSPOSEMATRIXDARBPROC, glLoadTransposeMatrixdARB);
CS_PREP_GL_FUNCTION (PFNGLMULTTRANSPOSEMATRIXFARBPROC, glMultTransposeMatrixfARB);
CS_PREP_GL_FUNCTION (PFNGLMULTTRANSPOSEMATRIXDARBPROC, glMultTransposeMatrixdARB);
#endif

#ifdef CSGL_ARB_multisample
CS_PREP_GL_FUNCTION (PFNGLSAMPLECOVERAGEARBPROC, glSampleCoverageARB);
CS_PREP_GL_FUNCTION (PFNGLSAMPLEPASSARBPROC, glSamplePassARB);
#endif

#ifdef CSGL_ARB_texture_env_add
#endif

#ifdef CSGL_ARB_texture_cube_map
#endif

#ifdef CSGL_ARB_texture_compression
CS_PREP_GL_FUNCTION (PFNGLCOMPRESSEDTEXIMAGE3DARBPROC, glCompressedTexImage3DARB);
CS_PREP_GL_FUNCTION (PFNGLCOMPRESSEDTEXIMAGE2DARBPROC, glCompressedTexImage2DARB);
CS_PREP_GL_FUNCTION (PFNGLCOMPRESSEDTEXIMAGE1DARBPROC, glCompressedTexImage1DARB);
CS_PREP_GL_FUNCTION (PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC, glCompressedTexSubImage3DARB);
CS_PREP_GL_FUNCTION (PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC, glCompressedTexSubImage2DARB);
CS_PREP_GL_FUNCTION (PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC, glCompressedTexSubImage1DARB);
CS_PREP_GL_FUNCTION (PFNGLGETCOMPRESSEDTEXIMAGEARBPROC, glGetCompressedTexImageARB);
#endif

#ifdef CSGL_EXT_abgr
#endif

#ifdef CSGL_EXT_blend_color
CS_PREP_GL_FUNCTION (PFNGLBLENDCOLOREXTPROC, glBlendColorEXT);
#endif

#ifdef CSGL_EXT_polygon_offset
CS_PREP_GL_FUNCTION (PFNGLPOLYGONOFFSETEXTPROC, glPolygonOffsetEXT);
#endif

#ifdef CSGL_EXT_texture
#endif

#ifdef CSGL_EXT_texture3D
CS_PREP_GL_FUNCTION (PFNGLTEXIMAGE3DEXTPROC, glTexImage3DEXT);
#endif

#ifdef CSGL_EXT_subtexture
CS_PREP_GL_FUNCTION (PFNGLTEXSUBIMAGE3DEXTPROC, glTexSubImage3DEXT);
#endif

#ifdef CSGL_SGIS_texture_filter4
CS_PREP_GL_FUNCTION (PFNGLGETTEXFILTERFUNCSGISPROC, glGetTexFilterFuncSGIS);
CS_PREP_GL_FUNCTION (PFNGLTEXFILTERFUNCSGISPROC, glTexFilterFuncSGIS);
#endif

#ifdef CSGL_EXT_subtexture
CS_PREP_GL_FUNCTION (PFNGLTEXSUBIMAGE1DEXTPROC, glTexSubImage1DEXT);
CS_PREP_GL_FUNCTION (PFNGLTEXSUBIMAGE2DEXTPROC, glTexSubImage2DEXT);
#endif

#ifdef CSGL_EXT_copy_texture
CS_PREP_GL_FUNCTION (PFNGLCOPYTEXIMAGE1DEXTPROC, glCopyTexImage1DEXT);
CS_PREP_GL_FUNCTION (PFNGLCOPYTEXIMAGE2DEXTPROC, glCopyTexImage2DEXT);
CS_PREP_GL_FUNCTION (PFNGLCOPYTEXSUBIMAGE1DEXTPROC, glCopyTexSubImage1DEXT);
CS_PREP_GL_FUNCTION (PFNGLCOPYTEXSUBIMAGE2DEXTPROC, glCopyTexSubImage2DEXT);
CS_PREP_GL_FUNCTION (PFNGLCOPYTEXSUBIMAGE3DEXTPROC, glCopyTexSubImage3DEXT);
#endif

#ifdef CSGL_EXT_histogram
CS_PREP_GL_FUNCTION (PFNGLGETHISTOGRAMEXTPROC, glGetHistogramEXT);
CS_PREP_GL_FUNCTION (PFNGLGETHISTOGRAMPARAMETERFVEXTPROC, glGetHistogramParameterfvEXT);
CS_PREP_GL_FUNCTION (PFNGLGETHISTOGRAMPARAMETERIVEXTPROC, glGetHistogramParameterivEXT);
CS_PREP_GL_FUNCTION (PFNGLGETMINMAXEXTPROC, glGetMinmaxEXT);
CS_PREP_GL_FUNCTION (PFNGLGETMINMAXPARAMETERFVEXTPROC, glGetMinmaxParameterfvEXT);
CS_PREP_GL_FUNCTION (PFNGLGETMINMAXPARAMETERIVEXTPROC, glGetMinmaxParameterivEXT);
CS_PREP_GL_FUNCTION (PFNGLHISTOGRAMEXTPROC, glHistogramEXT);
CS_PREP_GL_FUNCTION (PFNGLMINMAXEXTPROC, glMinmaxEXT);
CS_PREP_GL_FUNCTION (PFNGLRESETHISTOGRAMEXTPROC, glResetHistogramEXT);
CS_PREP_GL_FUNCTION (PFNGLRESETMINMAXEXTPROC, glResetMinmaxEXT);
#endif

#ifdef CSGL_EXT_convolution
CS_PREP_GL_FUNCTION (PFNGLCONVOLUTIONFILTER1DEXTPROC, glConvolutionFilter1DEXT);
CS_PREP_GL_FUNCTION (PFNGLCONVOLUTIONFILTER2DEXTPROC, glConvolutionFilter2DEXT);
CS_PREP_GL_FUNCTION (PFNGLCONVOLUTIONPARAMETERFEXTPROC, glConvolutionParameterfEXT);
CS_PREP_GL_FUNCTION (PFNGLCONVOLUTIONPARAMETERFVEXTPROC, glConvolutionParameterfvEXT);
CS_PREP_GL_FUNCTION (PFNGLCONVOLUTIONPARAMETERIEXTPROC, glConvolutionParameteriEXT);
CS_PREP_GL_FUNCTION (PFNGLCONVOLUTIONPARAMETERIVEXTPROC, glConvolutionParameterivEXT);
CS_PREP_GL_FUNCTION (PFNGLCOPYCONVOLUTIONFILTER1DEXTPROC, glCopyConvolutionFilter1DEXT);
CS_PREP_GL_FUNCTION (PFNGLCOPYCONVOLUTIONFILTER2DEXTPROC, glCopyConvolutionFilter2DEXT);
CS_PREP_GL_FUNCTION (PFNGLGETCONVOLUTIONFILTEREXTPROC, glGetConvolutionFilterEXT);
CS_PREP_GL_FUNCTION (PFNGLGETCONVOLUTIONPARAMETERFVEXTPROC, glGetConvolutionParameterfvEXT);
CS_PREP_GL_FUNCTION (PFNGLGETCONVOLUTIONPARAMETERIVEXTPROC, glGetConvolutionParameterivEXT);
CS_PREP_GL_FUNCTION (PFNGLGETSEPARABLEFILTEREXTPROC, glGetSeparableFilterEXT);
CS_PREP_GL_FUNCTION (PFNGLSEPARABLEFILTER2DEXTPROC, glSeparableFilter2DEXT);
#endif

#ifdef CSGL_EXT_color_matrix
#endif

#ifdef CSGL_SGI_color_table
CS_PREP_GL_FUNCTION (PFNGLCOLORTABLESGIPROC, glColorTableSGI);
CS_PREP_GL_FUNCTION (PFNGLCOLORTABLEPARAMETERFVSGIPROC, glColorTableParameterfvSGI);
CS_PREP_GL_FUNCTION (PFNGLCOLORTABLEPARAMETERIVSGIPROC, glColorTableParameterivSGI);
CS_PREP_GL_FUNCTION (PFNGLCOPYCOLORTABLESGIPROC, glCopyColorTableSGI);
CS_PREP_GL_FUNCTION (PFNGLGETCOLORTABLESGIPROC, glGetColorTableSGI);
CS_PREP_GL_FUNCTION (PFNGLGETCOLORTABLEPARAMETERFVSGIPROC, glGetColorTableParameterfvSGI);
CS_PREP_GL_FUNCTION (PFNGLGETCOLORTABLEPARAMETERIVSGIPROC, glGetColorTableParameterivSGI);
#endif

#ifdef CSGL_SGIX_pixel_texture
CS_PREP_GL_FUNCTION (PFNGLPIXELTEXGENSGIXPROC, glPixelTexGenSGIX);
#endif

#ifdef CSGL_SGIS_pixel_texture
CS_PREP_GL_FUNCTION (PFNGLPIXELTEXGENPARAMETERISGISPROC, glPixelTexGenParameteriSGIS);
CS_PREP_GL_FUNCTION (PFNGLPIXELTEXGENPARAMETERIVSGISPROC, glPixelTexGenParameterivSGIS);
CS_PREP_GL_FUNCTION (PFNGLPIXELTEXGENPARAMETERFSGISPROC, glPixelTexGenParameterfSGIS);
CS_PREP_GL_FUNCTION (PFNGLPIXELTEXGENPARAMETERFVSGISPROC, glPixelTexGenParameterfvSGIS);
CS_PREP_GL_FUNCTION (PFNGLGETPIXELTEXGENPARAMETERIVSGISPROC, glGetPixelTexGenParameterivSGIS);
CS_PREP_GL_FUNCTION (PFNGLGETPIXELTEXGENPARAMETERFVSGISPROC, glGetPixelTexGenParameterfvSGIS);
#endif

#ifdef CSGL_SGIS_texture4D
CS_PREP_GL_FUNCTION (PFNGLTEXIMAGE4DSGISPROC, glTexImage4DSGIS);
CS_PREP_GL_FUNCTION (PFNGLTEXSUBIMAGE4DSGISPROC, glTexSubImage4DSGIS);
#endif

#ifdef CSGL_SGI_texture_color_table
#endif

#ifdef CSGL_EXT_cmyka
#endif

#ifdef CSGL_EXT_texture_object
CS_PREP_GL_FUNCTION (PFNGLARETEXTURESRESIDENTEXTPROC, glAreTexturesResidentEXT);
CS_PREP_GL_FUNCTION (PFNGLBINDTEXTUREEXTPROC, glBindTextureEXT);
CS_PREP_GL_FUNCTION (PFNGLDELETETEXTURESEXTPROC, glDeleteTexturesEXT);
CS_PREP_GL_FUNCTION (PFNGLGENTEXTURESEXTPROC, glGenTexturesEXT);
CS_PREP_GL_FUNCTION (PFNGLISTEXTUREEXTPROC, glIsTextureEXT);
CS_PREP_GL_FUNCTION (PFNGLPRIORITIZETEXTURESEXTPROC, glPrioritizeTexturesEXT);
#endif

#ifdef CSGL_SGIS_detail_texture
CS_PREP_GL_FUNCTION (PFNGLDETAILTEXFUNCSGISPROC, glDetailTexFuncSGIS);
CS_PREP_GL_FUNCTION (PFNGLGETDETAILTEXFUNCSGISPROC, glGetDetailTexFuncSGIS);
#endif

#ifdef CSGL_SGIS_sharpen_texture
CS_PREP_GL_FUNCTION (PFNGLSHARPENTEXFUNCSGISPROC, glSharpenTexFuncSGIS);
CS_PREP_GL_FUNCTION (PFNGLGETSHARPENTEXFUNCSGISPROC, glGetSharpenTexFuncSGIS);
#endif

#ifdef CSGL_EXT_packed_pixels
#endif

#ifdef CSGL_SGIS_texture_lod
#endif

#ifdef CSGL_SGIS_multisample
CS_PREP_GL_FUNCTION (PFNGLSAMPLEMASKSGISPROC, glSampleMaskSGIS);
CS_PREP_GL_FUNCTION (PFNGLSAMPLEPATTERNSGISPROC, glSamplePatternSGIS);
#endif

#ifdef CSGL_EXT_rescale_normal
#endif

#ifdef CSGL_EXT_vertex_array
CS_PREP_GL_FUNCTION (PFNGLARRAYELEMENTEXTPROC, glArrayElementEXT);
CS_PREP_GL_FUNCTION (PFNGLCOLORPOINTEREXTPROC, glColorPointerEXT);
CS_PREP_GL_FUNCTION (PFNGLDRAWARRAYSEXTPROC, glDrawArraysEXT);
CS_PREP_GL_FUNCTION (PFNGLEDGEFLAGPOINTEREXTPROC, glEdgeFlagPointerEXT);
CS_PREP_GL_FUNCTION (PFNGLGETPOINTERVEXTPROC, glGetPointervEXT);
CS_PREP_GL_FUNCTION (PFNGLINDEXPOINTEREXTPROC, glIndexPointerEXT);
CS_PREP_GL_FUNCTION (PFNGLNORMALPOINTEREXTPROC, glNormalPointerEXT);
CS_PREP_GL_FUNCTION (PFNGLTEXCOORDPOINTEREXTPROC, glTexCoordPointerEXT);
CS_PREP_GL_FUNCTION (PFNGLVERTEXPOINTEREXTPROC, glVertexPointerEXT);
#endif

#ifdef CSGL_EXT_misc_attribute
#endif

#ifdef CSGL_SGIS_generate_mipmap
#endif

#ifdef CSGL_SGIX_clipmap
#endif

#ifdef CSGL_SGIX_shadow
#endif

#ifdef CSGL_SGIS_texture_edge_clamp
#endif

#ifdef CSGL_SGIS_texture_border_clamp
#endif

#ifdef CSGL_EXT_blend_minmax
CS_PREP_GL_FUNCTION (PFNGLBLENDEQUATIONEXTPROC, glBlendEquationEXT);
#endif

#ifdef CSGL_EXT_blend_subtract
#endif

#ifdef CSGL_EXT_blend_logic_op
#endif

#ifdef CSGL_SGIX_interlace
#endif

#ifdef CSGL_SGIX_pixel_tiles
#endif

#ifdef CSGL_SGIX_texture_select
#endif

#ifdef CSGL_SGIX_sprite
CS_PREP_GL_FUNCTION (PFNGLSPRITEPARAMETERFSGIXPROC, glSpriteParameterfSGIX);
CS_PREP_GL_FUNCTION (PFNGLSPRITEPARAMETERFVSGIXPROC, glSpriteParameterfvSGIX);
CS_PREP_GL_FUNCTION (PFNGLSPRITEPARAMETERISGIXPROC, glSpriteParameteriSGIX);
CS_PREP_GL_FUNCTION (PFNGLSPRITEPARAMETERIVSGIXPROC, glSpriteParameterivSGIX);
#endif

#ifdef CSGL_SGIX_texture_multi_buffer
#endif

#ifdef CSGL_EXT_point_parameters
CS_PREP_GL_FUNCTION (PFNGLPOINTPARAMETERFEXTPROC, glPointParameterfEXT);
CS_PREP_GL_FUNCTION (PFNGLPOINTPARAMETERFVEXTPROC, glPointParameterfvEXT);
CS_PREP_GL_FUNCTION (PFNGLPOINTPARAMETERFSGISPROC, glPointParameterfSGIS);
CS_PREP_GL_FUNCTION (PFNGLPOINTPARAMETERFVSGISPROC, glPointParameterfvSGIS);
#endif

#ifdef CSGL_SGIX_instruments
CS_PREP_GL_FUNCTION (PFNGLGETINSTRUMENTSSGIXPROC, glGetInstrumentsSGIX);
CS_PREP_GL_FUNCTION (PFNGLINSTRUMENTSBUFFERSGIXPROC, glInstrumentsBufferSGIX);
CS_PREP_GL_FUNCTION (PFNGLPOLLINSTRUMENTSSGIXPROC, glPollInstrumentsSGIX);
CS_PREP_GL_FUNCTION (PFNGLREADINSTRUMENTSSGIXPROC, glReadInstrumentsSGIX);
CS_PREP_GL_FUNCTION (PFNGLSTARTINSTRUMENTSSGIXPROC, glStartInstrumentsSGIX);
CS_PREP_GL_FUNCTION (PFNGLSTOPINSTRUMENTSSGIXPROC, glStopInstrumentsSGIX);
#endif

#ifdef CSGL_SGIX_texture_scale_bias
#endif

#ifdef CSGL_SGIX_framezoom
CS_PREP_GL_FUNCTION (PFNGLFRAMEZOOMSGIXPROC, glFrameZoomSGIX);
#endif

#ifdef CSGL_SGIX_tag_sample_buffer
CS_PREP_GL_FUNCTION (PFNGLTAGSAMPLEBUFFERSGIXPROC, glTagSampleBufferSGIX);
#endif

#ifdef CSGL_SGIX_reference_plane
CS_PREP_GL_FUNCTION (PFNGLREFERENCEPLANESGIXPROC, glReferencePlaneSGIX);
#endif

#ifdef CSGL_SGIX_flush_raster
CS_PREP_GL_FUNCTION (PFNGLFLUSHRASTERSGIXPROC, glFlushRasterSGIX);
#endif

#ifdef CSGL_SGIX_depth_texture
#endif

#ifdef CSGL_SGIS_fog_function
CS_PREP_GL_FUNCTION (PFNGLFOGFUNCSGISPROC, glFogFuncSGIS);
CS_PREP_GL_FUNCTION (PFNGLGETFOGFUNCSGISPROC, glGetFogFuncSGIS);
#endif

#ifdef CSGL_SGIX_fog_offset
#endif

#ifdef CSGL_HP_image_transform
CS_PREP_GL_FUNCTION (PFNGLIMAGETRANSFORMPARAMETERIHPPROC, glImageTransformParameteriHP);
CS_PREP_GL_FUNCTION (PFNGLIMAGETRANSFORMPARAMETERFHPPROC, glImageTransformParameterfHP);
CS_PREP_GL_FUNCTION (PFNGLIMAGETRANSFORMPARAMETERIVHPPROC, glImageTransformParameterivHP);
CS_PREP_GL_FUNCTION (PFNGLIMAGETRANSFORMPARAMETERFVHPPROC, glImageTransformParameterfvHP);
CS_PREP_GL_FUNCTION (PFNGLGETIMAGETRANSFORMPARAMETERIVHPPROC, glGetImageTransformParameterivHP);
CS_PREP_GL_FUNCTION (PFNGLGETIMAGETRANSFORMPARAMETERFVHPPROC, glGetImageTransformParameterfvHP);
#endif

#ifdef CSGL_HP_convolution_border_modes
#endif

#ifdef CSGL_SGIX_texture_add_env
#endif

#ifdef CSGL_EXT_color_subtable
CS_PREP_GL_FUNCTION (PFNGLCOLORSUBTABLEEXTPROC, glColorSubTableEXT);
CS_PREP_GL_FUNCTION (PFNGLCOPYCOLORSUBTABLEEXTPROC, glCopyColorSubTableEXT);
#endif

#ifdef CSGL_PGI_vertex_hints
#endif

#ifdef CSGL_PGI_misc_hints
CS_PREP_GL_FUNCTION (PFNGLHINTPGIPROC, glHintPGI);
#endif

#ifdef CSGL_EXT_paletted_texture
CS_PREP_GL_FUNCTION (PFNGLCOLORTABLEEXTPROC, glColorTableEXT);
CS_PREP_GL_FUNCTION (PFNGLGETCOLORTABLEEXTPROC, glGetColorTableEXT);
CS_PREP_GL_FUNCTION (PFNGLGETCOLORTABLEPARAMETERIVEXTPROC, glGetColorTableParameterivEXT);
CS_PREP_GL_FUNCTION (PFNGLGETCOLORTABLEPARAMETERFVEXTPROC, glGetColorTableParameterfvEXT);
#endif

#ifdef CSGL_EXT_clip_volume_hint
#endif

#ifdef CSGL_SGIX_list_priority
CS_PREP_GL_FUNCTION (PFNGLGETLISTPARAMETERFVSGIXPROC, glGetListParameterfvSGIX);
CS_PREP_GL_FUNCTION (PFNGLGETLISTPARAMETERIVSGIXPROC, glGetListParameterivSGIX);
CS_PREP_GL_FUNCTION (PFNGLLISTPARAMETERFSGIXPROC, glListParameterfSGIX);
CS_PREP_GL_FUNCTION (PFNGLLISTPARAMETERFVSGIXPROC, glListParameterfvSGIX);
CS_PREP_GL_FUNCTION (PFNGLLISTPARAMETERISGIXPROC, glListParameteriSGIX);
CS_PREP_GL_FUNCTION (PFNGLLISTPARAMETERIVSGIXPROC, glListParameterivSGIX);
#endif

#ifdef CSGL_SGIX_ir_instrument1
#endif

#ifdef CSGL_SGIX_calligraphic_fragment
#endif

#ifdef CSGL_SGIX_texture_lod_bias
#endif

#ifdef CSGL_SGIX_shadow_ambient
#endif

#ifdef CSGL_EXT_index_texture
#endif

#ifdef CSGL_EXT_index_material
CS_PREP_GL_FUNCTION (PFNGLINDEXMATERIALEXTPROC, glIndexMaterialEXT);
#endif

#ifdef CSGL_EXT_index_func
CS_PREP_GL_FUNCTION (PFNGLINDEXFUNCEXTPROC, glIndexFuncEXT);
#endif

#ifdef CSGL_EXT_index_array_formats
#endif

#ifdef CSGL_EXT_compiled_vertex_array
CS_PREP_GL_FUNCTION (PFNGLLOCKARRAYSEXTPROC, glLockArraysEXT);
CS_PREP_GL_FUNCTION (PFNGLUNLOCKARRAYSEXTPROC, glUnlockArraysEXT);
#endif

#ifdef CSGL_EXT_cull_vertex
CS_PREP_GL_FUNCTION (PFNGLCULLPARAMETERDVEXTPROC, glCullParameterdvEXT);
CS_PREP_GL_FUNCTION (PFNGLCULLPARAMETERFVEXTPROC, glCullParameterfvEXT);
#endif

#ifdef CSGL_SGIX_ycrcb
#endif

#ifdef CSGL_SGIX_fragment_lighting
CS_PREP_GL_FUNCTION (PFNGLFRAGMENTCOLORMATERIALSGIXPROC, glFragmentColorMaterialSGIX);
CS_PREP_GL_FUNCTION (PFNGLFRAGMENTLIGHTFSGIXPROC, glFragmentLightfSGIX);
CS_PREP_GL_FUNCTION (PFNGLFRAGMENTLIGHTFVSGIXPROC, glFragmentLightfvSGIX);
CS_PREP_GL_FUNCTION (PFNGLFRAGMENTLIGHTISGIXPROC, glFragmentLightiSGIX);
CS_PREP_GL_FUNCTION (PFNGLFRAGMENTLIGHTIVSGIXPROC, glFragmentLightivSGIX);
CS_PREP_GL_FUNCTION (PFNGLFRAGMENTLIGHTMODELFSGIXPROC, glFragmentLightModelfSGIX);
CS_PREP_GL_FUNCTION (PFNGLFRAGMENTLIGHTMODELFVSGIXPROC, glFragmentLightModelfvSGIX);
CS_PREP_GL_FUNCTION (PFNGLFRAGMENTLIGHTMODELISGIXPROC, glFragmentLightModeliSGIX);
CS_PREP_GL_FUNCTION (PFNGLFRAGMENTLIGHTMODELIVSGIXPROC, glFragmentLightModelivSGIX);
CS_PREP_GL_FUNCTION (PFNGLFRAGMENTMATERIALFSGIXPROC, glFragmentMaterialfSGIX);
CS_PREP_GL_FUNCTION (PFNGLFRAGMENTMATERIALFVSGIXPROC, glFragmentMaterialfvSGIX);
CS_PREP_GL_FUNCTION (PFNGLFRAGMENTMATERIALISGIXPROC, glFragmentMaterialiSGIX);
CS_PREP_GL_FUNCTION (PFNGLFRAGMENTMATERIALIVSGIXPROC, glFragmentMaterialivSGIX);
CS_PREP_GL_FUNCTION (PFNGLGETFRAGMENTLIGHTFVSGIXPROC, glGetFragmentLightfvSGIX);
CS_PREP_GL_FUNCTION (PFNGLGETFRAGMENTLIGHTIVSGIXPROC, glGetFragmentLightivSGIX);
CS_PREP_GL_FUNCTION (PFNGLGETFRAGMENTMATERIALFVSGIXPROC, glGetFragmentMaterialfvSGIX);
CS_PREP_GL_FUNCTION (PFNGLGETFRAGMENTMATERIALIVSGIXPROC, glGetFragmentMaterialivSGIX);
CS_PREP_GL_FUNCTION (PFNGLLIGHTENVISGIXPROC, glLightEnviSGIX);
#endif

#ifdef CSGL_IBM_rasterpos_clip
#endif

#ifdef CSGL_HP_texture_lighting
#endif

#ifdef CSGL_EXT_draw_range_elements
CS_PREP_GL_FUNCTION (PFNGLDRAWRANGEELEMENTSEXTPROC, glDrawRangeElementsEXT);
#endif

#ifdef CSGL_WIN_phong_shading
#endif

#ifdef CSGL_WIN_specular_fog
#endif

#ifdef CSGL_EXT_light_texture
CS_PREP_GL_FUNCTION (PFNGLAPPLYTEXTUREEXTPROC, glApplyTextureEXT);
CS_PREP_GL_FUNCTION (PFNGLTEXTURELIGHTEXTPROC, glTextureLightEXT);
CS_PREP_GL_FUNCTION (PFNGLTEXTUREMATERIALEXTPROC, glTextureMaterialEXT);
#endif

#ifdef CSGL_SGIX_blend_alpha_minmax
#endif

#ifdef CSGL_EXT_bgra
#endif

#ifdef CSGL_INTEL_parallel_arrays
CS_PREP_GL_FUNCTION (PFNGLVERTEXPOINTERVINTELPROC, glVertexPointervINTEL);
CS_PREP_GL_FUNCTION (PFNGLNORMALPOINTERVINTELPROC, glNormalPointervINTEL);
CS_PREP_GL_FUNCTION (PFNGLCOLORPOINTERVINTELPROC, glColorPointervINTEL);
CS_PREP_GL_FUNCTION (PFNGLTEXCOORDPOINTERVINTELPROC, glTexCoordPointervINTEL);
#endif

#ifdef CSGL_HP_occlusion_test
#endif

#ifdef CSGL_EXT_pixel_transform
CS_PREP_GL_FUNCTION (PFNGLPIXELTRANSFORMPARAMETERIEXTPROC, glPixelTransformParameteriEXT);
CS_PREP_GL_FUNCTION (PFNGLPIXELTRANSFORMPARAMETERFEXTPROC, glPixelTransformParameterfEXT);
CS_PREP_GL_FUNCTION (PFNGLPIXELTRANSFORMPARAMETERIVEXTPROC, glPixelTransformParameterivEXT);
CS_PREP_GL_FUNCTION (PFNGLPIXELTRANSFORMPARAMETERFVEXTPROC, glPixelTransformParameterfvEXT);
#endif

#ifdef CSGL_EXT_pixel_transform_color_table
#endif

#ifdef CSGL_EXT_shared_texture_palette
#endif

#ifdef CSGL_EXT_separate_specular_color
#endif

#ifdef CSGL_EXT_secondary_color
CS_PREP_GL_FUNCTION (PFNGLSECONDARYCOLOR3BEXTPROC, glSecondaryColor3bEXT);
CS_PREP_GL_FUNCTION (PFNGLSECONDARYCOLOR3BVEXTPROC, glSecondaryColor3bvEXT);
CS_PREP_GL_FUNCTION (PFNGLSECONDARYCOLOR3DEXTPROC, glSecondaryColor3dEXT);
CS_PREP_GL_FUNCTION (PFNGLSECONDARYCOLOR3DVEXTPROC, glSecondaryColor3dvEXT);
CS_PREP_GL_FUNCTION (PFNGLSECONDARYCOLOR3FEXTPROC, glSecondaryColor3fEXT);
CS_PREP_GL_FUNCTION (PFNGLSECONDARYCOLOR3FVEXTPROC, glSecondaryColor3fvEXT);
CS_PREP_GL_FUNCTION (PFNGLSECONDARYCOLOR3IEXTPROC, glSecondaryColor3iEXT);
CS_PREP_GL_FUNCTION (PFNGLSECONDARYCOLOR3IVEXTPROC, glSecondaryColor3ivEXT);
CS_PREP_GL_FUNCTION (PFNGLSECONDARYCOLOR3SEXTPROC, glSecondaryColor3sEXT);
CS_PREP_GL_FUNCTION (PFNGLSECONDARYCOLOR3SVEXTPROC, glSecondaryColor3svEXT);
CS_PREP_GL_FUNCTION (PFNGLSECONDARYCOLOR3UBEXTPROC, glSecondaryColor3ubEXT);
CS_PREP_GL_FUNCTION (PFNGLSECONDARYCOLOR3UBVEXTPROC, glSecondaryColor3ubvEXT);
CS_PREP_GL_FUNCTION (PFNGLSECONDARYCOLOR3UIEXTPROC, glSecondaryColor3uiEXT);
CS_PREP_GL_FUNCTION (PFNGLSECONDARYCOLOR3UIVEXTPROC, glSecondaryColor3uivEXT);
CS_PREP_GL_FUNCTION (PFNGLSECONDARYCOLOR3USEXTPROC, glSecondaryColor3usEXT);
CS_PREP_GL_FUNCTION (PFNGLSECONDARYCOLOR3USVEXTPROC, glSecondaryColor3usvEXT);
CS_PREP_GL_FUNCTION (PFNGLSECONDARYCOLORPOINTEREXTPROC, glSecondaryColorPointerEXT);
#endif

#ifdef CSGL_EXT_texture_perturb_normal
CS_PREP_GL_FUNCTION (PFNGLTEXTURENORMALEXTPROC, glTextureNormalEXT);
#endif

#ifdef CSGL_EXT_multi_draw_arrays
CS_PREP_GL_FUNCTION (PFNGLMULTIDRAWARRAYSEXTPROC, glMultiDrawArraysEXT);
CS_PREP_GL_FUNCTION (PFNGLMULTIDRAWELEMENTSEXTPROC, glMultiDrawElementsEXT);
#endif

#ifdef CSGL_EXT_fog_coord
CS_PREP_GL_FUNCTION (PFNGLFOGCOORDFEXTPROC, glFogCoordfEXT);
CS_PREP_GL_FUNCTION (PFNGLFOGCOORDFVEXTPROC, glFogCoordfvEXT);
CS_PREP_GL_FUNCTION (PFNGLFOGCOORDDEXTPROC, glFogCoorddEXT);
CS_PREP_GL_FUNCTION (PFNGLFOGCOORDDVEXTPROC, glFogCoorddvEXT);
CS_PREP_GL_FUNCTION (PFNGLFOGCOORDPOINTEREXTPROC, glFogCoordPointerEXT);
#endif

#ifdef CSGL_REND_screen_coordinates
#endif

#ifdef CSGL_EXT_coordinate_frame
CS_PREP_GL_FUNCTION (PFNGLTANGENT3BEXTPROC, glTangent3bEXT);
CS_PREP_GL_FUNCTION (PFNGLTANGENT3BVEXTPROC, glTangent3bvEXT);
CS_PREP_GL_FUNCTION (PFNGLTANGENT3DEXTPROC, glTangent3dEXT);
CS_PREP_GL_FUNCTION (PFNGLTANGENT3DVEXTPROC, glTangent3dvEXT);
CS_PREP_GL_FUNCTION (PFNGLTANGENT3FEXTPROC, glTangent3fEXT);
CS_PREP_GL_FUNCTION (PFNGLTANGENT3FVEXTPROC, glTangent3fvEXT);
CS_PREP_GL_FUNCTION (PFNGLTANGENT3IEXTPROC, glTangent3iEXT);
CS_PREP_GL_FUNCTION (PFNGLTANGENT3IVEXTPROC, glTangent3ivEXT);
CS_PREP_GL_FUNCTION (PFNGLTANGENT3SEXTPROC, glTangent3sEXT);
CS_PREP_GL_FUNCTION (PFNGLTANGENT3SVEXTPROC, glTangent3svEXT);
CS_PREP_GL_FUNCTION (PFNGLBINORMAL3BEXTPROC, glBinormal3bEXT);
CS_PREP_GL_FUNCTION (PFNGLBINORMAL3BVEXTPROC, glBinormal3bvEXT);
CS_PREP_GL_FUNCTION (PFNGLBINORMAL3DEXTPROC, glBinormal3dEXT);
CS_PREP_GL_FUNCTION (PFNGLBINORMAL3DVEXTPROC, glBinormal3dvEXT);
CS_PREP_GL_FUNCTION (PFNGLBINORMAL3FEXTPROC, glBinormal3fEXT);
CS_PREP_GL_FUNCTION (PFNGLBINORMAL3FVEXTPROC, glBinormal3fvEXT);
CS_PREP_GL_FUNCTION (PFNGLBINORMAL3IEXTPROC, glBinormal3iEXT);
CS_PREP_GL_FUNCTION (PFNGLBINORMAL3IVEXTPROC, glBinormal3ivEXT);
CS_PREP_GL_FUNCTION (PFNGLBINORMAL3SEXTPROC, glBinormal3sEXT);
CS_PREP_GL_FUNCTION (PFNGLBINORMAL3SVEXTPROC, glBinormal3svEXT);
CS_PREP_GL_FUNCTION (PFNGLTANGENTPOINTEREXTPROC, glTangentPointerEXT);
CS_PREP_GL_FUNCTION (PFNGLBINORMALPOINTEREXTPROC, glBinormalPointerEXT);
#endif

#ifdef CSGL_EXT_texture_env_combine
#endif

#ifdef CSGL_APPLE_specular_vector
#endif

#ifdef CSGL_APPLE_transform_hint
#endif

#ifdef CSGL_SGIX_fog_scale
#endif

#ifdef CSGL_SUNX_constant_data
CS_PREP_GL_FUNCTION (PFNGLFINISHTEXTURESUNXPROC, glFinishTextureSUNX);
#endif

#ifdef CSGL_SUN_global_alpha
CS_PREP_GL_FUNCTION (PFNGLGLOBALALPHAFACTORBSUNPROC, glGlobalAlphaFactorbSUN);
CS_PREP_GL_FUNCTION (PFNGLGLOBALALPHAFACTORSSUNPROC, glGlobalAlphaFactorsSUN);
CS_PREP_GL_FUNCTION (PFNGLGLOBALALPHAFACTORISUNPROC, glGlobalAlphaFactoriSUN);
CS_PREP_GL_FUNCTION (PFNGLGLOBALALPHAFACTORFSUNPROC, glGlobalAlphaFactorfSUN);
CS_PREP_GL_FUNCTION (PFNGLGLOBALALPHAFACTORDSUNPROC, glGlobalAlphaFactordSUN);
CS_PREP_GL_FUNCTION (PFNGLGLOBALALPHAFACTORUBSUNPROC, glGlobalAlphaFactorubSUN);
CS_PREP_GL_FUNCTION (PFNGLGLOBALALPHAFACTORUSSUNPROC, glGlobalAlphaFactorusSUN);
CS_PREP_GL_FUNCTION (PFNGLGLOBALALPHAFACTORUISUNPROC, glGlobalAlphaFactoruiSUN);
#endif

#ifdef CSGL_SUN_triangle_list
CS_PREP_GL_FUNCTION (PFNGLREPLACEMENTCODEUISUNPROC, glReplacementCodeuiSUN);
CS_PREP_GL_FUNCTION (PFNGLREPLACEMENTCODEUSSUNPROC, glReplacementCodeusSUN);
CS_PREP_GL_FUNCTION (PFNGLREPLACEMENTCODEUBSUNPROC, glReplacementCodeubSUN);
CS_PREP_GL_FUNCTION (PFNGLREPLACEMENTCODEUIVSUNPROC, glReplacementCodeuivSUN);
CS_PREP_GL_FUNCTION (PFNGLREPLACEMENTCODEUSVSUNPROC, glReplacementCodeusvSUN);
CS_PREP_GL_FUNCTION (PFNGLREPLACEMENTCODEUBVSUNPROC, glReplacementCodeubvSUN);
CS_PREP_GL_FUNCTION (PFNGLREPLACEMENTCODEPOINTERSUNPROC, glReplacementCodePointerSUN);
#endif

#ifdef CSGL_SUN_vertex
CS_PREP_GL_FUNCTION (PFNGLCOLOR4UBVERTEX2FSUNPROC, glColor4ubVertex2fSUN);
CS_PREP_GL_FUNCTION (PFNGLCOLOR4UBVERTEX2FVSUNPROC, glColor4ubVertex2fvSUN);
CS_PREP_GL_FUNCTION (PFNGLCOLOR4UBVERTEX3FSUNPROC, glColor4ubVertex3fSUN);
CS_PREP_GL_FUNCTION (PFNGLCOLOR4UBVERTEX3FVSUNPROC, glColor4ubVertex3fvSUN);
CS_PREP_GL_FUNCTION (PFNGLCOLOR3FVERTEX3FSUNPROC, glColor3fVertex3fSUN);
CS_PREP_GL_FUNCTION (PFNGLCOLOR3FVERTEX3FVSUNPROC, glColor3fVertex3fvSUN);
CS_PREP_GL_FUNCTION (PFNGLNORMAL3FVERTEX3FSUNPROC, glNormal3fVertex3fSUN);
CS_PREP_GL_FUNCTION (PFNGLNORMAL3FVERTEX3FVSUNPROC, glNormal3fVertex3fvSUN);
CS_PREP_GL_FUNCTION (PFNGLCOLOR4FNORMAL3FVERTEX3FSUNPROC, glColor4fNormal3fVertex3fSUN);
CS_PREP_GL_FUNCTION (PFNGLCOLOR4FNORMAL3FVERTEX3FVSUNPROC, glColor4fNormal3fVertex3fvSUN);
CS_PREP_GL_FUNCTION (PFNGLTEXCOORD2FVERTEX3FSUNPROC, glTexCoord2fVertex3fSUN);
CS_PREP_GL_FUNCTION (PFNGLTEXCOORD2FVERTEX3FVSUNPROC, glTexCoord2fVertex3fvSUN);
CS_PREP_GL_FUNCTION (PFNGLTEXCOORD4FVERTEX4FSUNPROC, glTexCoord4fVertex4fSUN);
CS_PREP_GL_FUNCTION (PFNGLTEXCOORD4FVERTEX4FVSUNPROC, glTexCoord4fVertex4fvSUN);
CS_PREP_GL_FUNCTION (PFNGLTEXCOORD2FCOLOR4UBVERTEX3FSUNPROC, glTexCoord2fColor4ubVertex3fSUN);
CS_PREP_GL_FUNCTION (PFNGLTEXCOORD2FCOLOR4UBVERTEX3FVSUNPROC, glTexCoord2fColor4ubVertex3fvSUN);
CS_PREP_GL_FUNCTION (PFNGLTEXCOORD2FCOLOR3FVERTEX3FSUNPROC, glTexCoord2fColor3fVertex3fSUN);
CS_PREP_GL_FUNCTION (PFNGLTEXCOORD2FCOLOR3FVERTEX3FVSUNPROC, glTexCoord2fColor3fVertex3fvSUN);
CS_PREP_GL_FUNCTION (PFNGLTEXCOORD2FNORMAL3FVERTEX3FSUNPROC, glTexCoord2fNormal3fVertex3fSUN);
CS_PREP_GL_FUNCTION (PFNGLTEXCOORD2FNORMAL3FVERTEX3FVSUNPROC, glTexCoord2fNormal3fVertex3fvSUN);
CS_PREP_GL_FUNCTION (PFNGLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUNPROC, glTexCoord2fColor4fNormal3fVertex3fSUN);
CS_PREP_GL_FUNCTION (PFNGLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUNPROC, glTexCoord2fColor4fNormal3fVertex3fvSUN);
CS_PREP_GL_FUNCTION (PFNGLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FSUNPROC, glTexCoord4fColor4fNormal3fVertex4fSUN);
CS_PREP_GL_FUNCTION (PFNGLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FVSUNPROC, glTexCoord4fColor4fNormal3fVertex4fvSUN);
CS_PREP_GL_FUNCTION (PFNGLREPLACEMENTCODEUIVERTEX3FSUNPROC, glReplacementCodeuiVertex3fSUN);
CS_PREP_GL_FUNCTION (PFNGLREPLACEMENTCODEUIVERTEX3FVSUNPROC, glReplacementCodeuiVertex3fvSUN);
CS_PREP_GL_FUNCTION (PFNGLREPLACEMENTCODEUICOLOR4UBVERTEX3FSUNPROC, glReplacementCodeuiColor4ubVertex3fSUN);
CS_PREP_GL_FUNCTION (PFNGLREPLACEMENTCODEUICOLOR4UBVERTEX3FVSUNPROC, glReplacementCodeuiColor4ubVertex3fvSUN);
CS_PREP_GL_FUNCTION (PFNGLREPLACEMENTCODEUICOLOR3FVERTEX3FSUNPROC, glReplacementCodeuiColor3fVertex3fSUN);
CS_PREP_GL_FUNCTION (PFNGLREPLACEMENTCODEUICOLOR3FVERTEX3FVSUNPROC, glReplacementCodeuiColor3fVertex3fvSUN);
CS_PREP_GL_FUNCTION (PFNGLREPLACEMENTCODEUINORMAL3FVERTEX3FSUNPROC, glReplacementCodeuiNormal3fVertex3fSUN);
CS_PREP_GL_FUNCTION (PFNGLREPLACEMENTCODEUINORMAL3FVERTEX3FVSUNPROC, glReplacementCodeuiNormal3fVertex3fvSUN);
CS_PREP_GL_FUNCTION (PFNGLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FSUNPROC, glReplacementCodeuiColor4fNormal3fVertex3fSUN);
CS_PREP_GL_FUNCTION (PFNGLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FVSUNPROC, glReplacementCodeuiColor4fNormal3fVertex3fvSUN);
CS_PREP_GL_FUNCTION (PFNGLREPLACEMENTCODEUITEXCOORD2FVERTEX3FSUNPROC, glReplacementCodeuiTexCoord2fVertex3fSUN);
CS_PREP_GL_FUNCTION (PFNGLREPLACEMENTCODEUITEXCOORD2FVERTEX3FVSUNPROC, glReplacementCodeuiTexCoord2fVertex3fvSUN);
CS_PREP_GL_FUNCTION (PFNGLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FSUNPROC, glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN);
CS_PREP_GL_FUNCTION (PFNGLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FVSUNPROC, glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN);
CS_PREP_GL_FUNCTION (PFNGLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUNPROC, glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN);
CS_PREP_GL_FUNCTION (PFNGLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUNPROC, glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN);
#endif

#ifdef CSGL_EXT_blend_func_separate
CS_PREP_GL_FUNCTION (PFNGLBLENDFUNCSEPARATEEXTPROC, glBlendFuncSeparateEXT);
#endif

#ifdef CSGL_INGR_color_clamp
#endif

#ifdef CSGL_INGR_interlace_read
#endif

#ifdef CSGL_EXT_stencil_wrap
#endif

#ifdef CSGL_EXT_422_pixels
#endif

#ifdef CSGL_NV_texgen_reflection
#endif

#ifdef CSGL_SUN_convolution_border_modes
#endif

#ifdef CSGL_EXT_texture_env_add
#endif

#ifdef CSGL_EXT_texture_lod_bias
#endif

#ifdef CSGL_EXT_texture_filter_anisotropic
#endif

#ifdef CSGL_EXT_vertex_weighting
CS_PREP_GL_FUNCTION (PFNGLVERTEXWEIGHTFEXTPROC, glVertexWeightfEXT);
CS_PREP_GL_FUNCTION (PFNGLVERTEXWEIGHTFVEXTPROC, glVertexWeightfvEXT);
CS_PREP_GL_FUNCTION (PFNGLVERTEXWEIGHTPOINTEREXTPROC, glVertexWeightPointerEXT);
#endif

#ifdef CSGL_NV_light_max_exponent
#endif

#ifdef CSGL_NV_vertex_array_range
CS_PREP_GL_FUNCTION (PFNGLFLUSHVERTEXARRAYRANGENVPROC, glFlushVertexArrayRangeNV);
CS_PREP_GL_FUNCTION (PFNGLVERTEXARRAYRANGENVPROC, glVertexArrayRangeNV);
#endif

#ifdef CSGL_NV_register_combiners
CS_PREP_GL_FUNCTION (PFNGLCOMBINERPARAMETERFVNVPROC, glCombinerParameterfvNV);
CS_PREP_GL_FUNCTION (PFNGLCOMBINERPARAMETERFNVPROC, glCombinerParameterfNV);
CS_PREP_GL_FUNCTION (PFNGLCOMBINERPARAMETERIVNVPROC, glCombinerParameterivNV);
CS_PREP_GL_FUNCTION (PFNGLCOMBINERPARAMETERINVPROC, glCombinerParameteriNV);
CS_PREP_GL_FUNCTION (PFNGLCOMBINERINPUTNVPROC, glCombinerInputNV);
CS_PREP_GL_FUNCTION (PFNGLCOMBINEROUTPUTNVPROC, glCombinerOutputNV);
CS_PREP_GL_FUNCTION (PFNGLFINALCOMBINERINPUTNVPROC, glFinalCombinerInputNV);
CS_PREP_GL_FUNCTION (PFNGLGETCOMBINERINPUTPARAMETERFVNVPROC, glGetCombinerInputParameterfvNV);
CS_PREP_GL_FUNCTION (PFNGLGETCOMBINERINPUTPARAMETERIVNVPROC, glGetCombinerInputParameterivNV);
CS_PREP_GL_FUNCTION (PFNGLGETCOMBINEROUTPUTPARAMETERFVNVPROC, glGetCombinerOutputParameterfvNV);
CS_PREP_GL_FUNCTION (PFNGLGETCOMBINEROUTPUTPARAMETERIVNVPROC, glGetCombinerOutputParameterivNV);
CS_PREP_GL_FUNCTION (PFNGLGETFINALCOMBINERINPUTPARAMETERFVNVPROC, glGetFinalCombinerInputParameterfvNV);
CS_PREP_GL_FUNCTION (PFNGLGETFINALCOMBINERINPUTPARAMETERIVNVPROC, glGetFinalCombinerInputParameterivNV);
#endif

#ifdef CSGL_NV_fog_distance
#endif

#ifdef CSGL_NV_texgen_emboss
#endif

#ifdef CSGL_NV_blend_square
#endif

#ifdef CSGL_NV_texture_env_combine4
#endif

#ifdef CSGL_MESA_resize_buffers
CS_PREP_GL_FUNCTION (PFNGLRESIZEBUFFERSMESAPROC, glResizeBuffersMESA);
#endif

#ifdef CSGL_MESA_window_pos
CS_PREP_GL_FUNCTION (PFNGLWINDOWPOS2DMESAPROC, glWindowPos2dMESA);
CS_PREP_GL_FUNCTION (PFNGLWINDOWPOS2DVMESAPROC, glWindowPos2dvMESA);
CS_PREP_GL_FUNCTION (PFNGLWINDOWPOS2FMESAPROC, glWindowPos2fMESA);
CS_PREP_GL_FUNCTION (PFNGLWINDOWPOS2FVMESAPROC, glWindowPos2fvMESA);
CS_PREP_GL_FUNCTION (PFNGLWINDOWPOS2IMESAPROC, glWindowPos2iMESA);
CS_PREP_GL_FUNCTION (PFNGLWINDOWPOS2IVMESAPROC, glWindowPos2ivMESA);
CS_PREP_GL_FUNCTION (PFNGLWINDOWPOS2SMESAPROC, glWindowPos2sMESA);
CS_PREP_GL_FUNCTION (PFNGLWINDOWPOS2SVMESAPROC, glWindowPos2svMESA);
CS_PREP_GL_FUNCTION (PFNGLWINDOWPOS3DMESAPROC, glWindowPos3dMESA);
CS_PREP_GL_FUNCTION (PFNGLWINDOWPOS3DVMESAPROC, glWindowPos3dvMESA);
CS_PREP_GL_FUNCTION (PFNGLWINDOWPOS3FMESAPROC, glWindowPos3fMESA);
CS_PREP_GL_FUNCTION (PFNGLWINDOWPOS3FVMESAPROC, glWindowPos3fvMESA);
CS_PREP_GL_FUNCTION (PFNGLWINDOWPOS3IMESAPROC, glWindowPos3iMESA);
CS_PREP_GL_FUNCTION (PFNGLWINDOWPOS3IVMESAPROC, glWindowPos3ivMESA);
CS_PREP_GL_FUNCTION (PFNGLWINDOWPOS3SMESAPROC, glWindowPos3sMESA);
CS_PREP_GL_FUNCTION (PFNGLWINDOWPOS3SVMESAPROC, glWindowPos3svMESA);
CS_PREP_GL_FUNCTION (PFNGLWINDOWPOS4DMESAPROC, glWindowPos4dMESA);
CS_PREP_GL_FUNCTION (PFNGLWINDOWPOS4DVMESAPROC, glWindowPos4dvMESA);
CS_PREP_GL_FUNCTION (PFNGLWINDOWPOS4FMESAPROC, glWindowPos4fMESA);
CS_PREP_GL_FUNCTION (PFNGLWINDOWPOS4FVMESAPROC, glWindowPos4fvMESA);
CS_PREP_GL_FUNCTION (PFNGLWINDOWPOS4IMESAPROC, glWindowPos4iMESA);
CS_PREP_GL_FUNCTION (PFNGLWINDOWPOS4IVMESAPROC, glWindowPos4ivMESA);
CS_PREP_GL_FUNCTION (PFNGLWINDOWPOS4SMESAPROC, glWindowPos4sMESA);
CS_PREP_GL_FUNCTION (PFNGLWINDOWPOS4SVMESAPROC, glWindowPos4svMESA);
#endif

#ifdef CSGL_IBM_cull_vertex
#endif

#ifdef CSGL_IBM_multimode_draw_arrays
CS_PREP_GL_FUNCTION (PFNGLMULTIMODEDRAWARRAYSIBMPROC, glMultiModeDrawArraysIBM);
CS_PREP_GL_FUNCTION (PFNGLMULTIMODEDRAWELEMENTSIBMPROC, glMultiModeDrawElementsIBM);
#endif

#ifdef CSGL_IBM_vertex_array_lists
CS_PREP_GL_FUNCTION (PFNGLCOLORPOINTERLISTIBMPROC, glColorPointerListIBM);
CS_PREP_GL_FUNCTION (PFNGLSECONDARYCOLORPOINTERLISTIBMPROC, glSecondaryColorPointerListIBM);
CS_PREP_GL_FUNCTION (PFNGLEDGEFLAGPOINTERLISTIBMPROC, glEdgeFlagPointerListIBM);
CS_PREP_GL_FUNCTION (PFNGLFOGCOORDPOINTERLISTIBMPROC, glFogCoordPointerListIBM);
CS_PREP_GL_FUNCTION (PFNGLINDEXPOINTERLISTIBMPROC, glIndexPointerListIBM);
CS_PREP_GL_FUNCTION (PFNGLNORMALPOINTERLISTIBMPROC, glNormalPointerListIBM);
CS_PREP_GL_FUNCTION (PFNGLTEXCOORDPOINTERLISTIBMPROC, glTexCoordPointerListIBM);
CS_PREP_GL_FUNCTION (PFNGLVERTEXPOINTERLISTIBMPROC, glVertexPointerListIBM);
#endif

#ifdef CSGL_SGIX_subsample
#endif

#ifdef CSGL_SGIX_ycrcba
#endif

#ifdef CSGL_SGIX_ycrcb_subsample
#endif

#ifdef CSGL_SGIX_depth_pass_instrument
#endif

#ifdef CSGL_3DFX_texture_compression_FXT1
#endif

#ifdef CSGL_3DFX_multisample
#endif

#ifdef CSGL_3DFX_tbuffer
CS_PREP_GL_FUNCTION (PFNGLTBUFFERMASK3DFXPROC, glTbufferMask3DFX);
#endif

#ifdef CSGL_EXT_multisample
CS_PREP_GL_FUNCTION (PFNGLSAMPLEMASKEXTPROC, glSampleMaskEXT);
CS_PREP_GL_FUNCTION (PFNGLSAMPLEPATTERNEXTPROC, glSamplePatternEXT);
#endif

#ifdef CSGL_SGI_vertex_preclip
#endif

#ifdef CSGL_SGIX_convolution_accuracy
#endif

#ifdef CSGL_SGIX_resample
#endif

#ifdef CSGL_SGIS_point_line_texgen
#endif

#ifdef CSGL_SGIS_texture_color_mask
CS_PREP_GL_FUNCTION (PFNGLTEXTURECOLORMASKSGISPROC, glTextureColorMaskSGIS);
#endif

#ifdef CSGL_FOR_ALL
#undef CSGL_VERSION_1_2
#undef CSGL_ARB_multitexture
#undef CSGL_ARB_transpose_matrix
#undef CSGL_ARB_multisample
#undef CSGL_ARB_texture_env_add
#undef CSGL_ARB_texture_cube_map
#undef CSGL_ARB_texture_compression
#undef CSGL_EXT_abgr
#undef CSGL_EXT_blend_color
#undef CSGL_EXT_polygon_offset
#undef CSGL_EXT_texture
#undef CSGL_EXT_texture3D
#undef CSGL_SGIS_texture_filter4
#undef CSGL_EXT_subtexture
#undef CSGL_EXT_copy_texture
#undef CSGL_EXT_histogram
#undef CSGL_EXT_convolution
#undef CSGL_EXT_color_matrix
#undef CSGL_SGI_color_table
#undef CSGL_SGIX_pixel_texture
#undef CSGL_SGIS_pixel_texture
#undef CSGL_SGIS_texture4D
#undef CSGL_SGI_texture_color_table
#undef CSGL_EXT_cmyka
#undef CSGL_EXT_texture_object
#undef CSGL_SGIS_detail_texture
#undef CSGL_SGIS_sharpen_texture
#undef CSGL_EXT_packed_pixels
#undef CSGL_SGIS_texture_lod
#undef CSGL_SGIS_multisample
#undef CSGL_EXT_rescale_normal
#undef CSGL_EXT_vertex_array
#undef CSGL_EXT_misc_attribute
#undef CSGL_SGIS_generate_mipmap
#undef CSGL_SGIX_clipmap
#undef CSGL_SGIX_shadow
#undef CSGL_SGIS_texture_edge_clamp
#undef CSGL_SGIS_texture_border_clamp
#undef CSGL_EXT_blend_minmax
#undef CSGL_EXT_blend_subtract
#undef CSGL_EXT_blend_logic_op
#undef CSGL_SGIX_interlace
#undef CSGL_SGIX_pixel_tiles
#undef CSGL_SGIX_texture_select
#undef CSGL_SGIX_sprite
#undef CSGL_SGIX_texture_multi_buffer
#undef CSGL_EXT_point_parameters
#undef CSGL_SGIX_instruments
#undef CSGL_SGIX_texture_scale_bias
#undef CSGL_SGIX_framezoom
#undef CSGL_SGIX_tag_sample_buffer
#undef CSGL_SGIX_reference_plane
#undef CSGL_SGIX_flush_raster
#undef CSGL_SGIX_depth_texture
#undef CSGL_SGIS_fog_function
#undef CSGL_SGIX_fog_offset
#undef CSGL_HP_image_transform
#undef CSGL_HP_convolution_border_modes
#undef CSGL_SGIX_texture_add_env
#undef CSGL_EXT_color_subtable
#undef CSGL_PGI_vertex_hints
#undef CSGL_PGI_misc_hints
#undef CSGL_EXT_paletted_texture
#undef CSGL_EXT_clip_volume_hint
#undef CSGL_SGIX_list_priority
#undef CSGL_SGIX_ir_instrument1
#undef CSGL_SGIX_calligraphic_fragment
#undef CSGL_SGIX_texture_lod_bias
#undef CSGL_SGIX_shadow_ambient
#undef CSGL_EXT_index_texture
#undef CSGL_EXT_index_material
#undef CSGL_EXT_index_func
#undef CSGL_EXT_index_array_formats
#undef CSGL_EXT_compiled_vertex_array
#undef CSGL_EXT_cull_vertex
#undef CSGL_SGIX_ycrcb
#undef CSGL_SGIX_fragment_lighting
#undef CSGL_IBM_rasterpos_clip
#undef CSGL_HP_texture_lighting
#undef CSGL_EXT_draw_range_elements
#undef CSGL_WIN_phong_shading
#undef CSGL_WIN_specular_fog
#undef CSGL_EXT_light_texture
#undef CSGL_SGIX_blend_alpha_minmax
#undef CSGL_EXT_bgra
#undef CSGL_INTEL_parallel_arrays
#undef CSGL_HP_occlusion_test
#undef CSGL_EXT_pixel_transform
#undef CSGL_EXT_pixel_transform_color_table
#undef CSGL_EXT_shared_texture_palette
#undef CSGL_EXT_separate_specular_color
#undef CSGL_EXT_secondary_color
#undef CSGL_EXT_texture_perturb_normal
#undef CSGL_EXT_multi_draw_arrays
#undef CSGL_EXT_fog_coord
#undef CSGL_REND_screen_coordinates
#undef CSGL_EXT_coordinate_frame
#undef CSGL_EXT_texture_env_combine
#undef CSGL_APPLE_specular_vector
#undef CSGL_APPLE_transform_hint
#undef CSGL_SGIX_fog_scale
#undef CSGL_SUNX_constant_data
#undef CSGL_SUN_global_alpha
#undef CSGL_SUN_triangle_list
#undef CSGL_SUN_vertex
#undef CSGL_EXT_blend_func_separate
#undef CSGL_INGR_color_clamp
#undef CSGL_INGR_interlace_read
#undef CSGL_EXT_stencil_wrap
#undef CSGL_EXT_422_pixels
#undef CSGL_NV_texgen_reflection
#undef CSGL_SUN_convolution_border_modes
#undef CSGL_EXT_texture_env_add
#undef CSGL_EXT_texture_lod_bias
#undef CSGL_EXT_texture_filter_anisotropic
#undef CSGL_EXT_vertex_weighting
#undef CSGL_NV_light_max_exponent
#undef CSGL_NV_vertex_array_range
#undef CSGL_NV_register_combiners
#undef CSGL_NV_fog_distance
#undef CSGL_NV_texgen_emboss
#undef CSGL_NV_blend_square
#undef CSGL_NV_texture_env_combine4
#undef CSGL_MESA_resize_buffers
#undef CSGL_MESA_window_pos
#undef CSGL_IBM_cull_vertex
#undef CSGL_IBM_multimode_draw_arrays
#undef CSGL_IBM_vertex_array_lists
#undef CSGL_SGIX_subsample
#undef CSGL_SGIX_ycrcba
#undef CSGL_SGIX_ycrcb_subsample
#undef CSGL_SGIX_depth_pass_instrument
#undef CSGL_3DFX_texture_compression_FXT1
#undef CSGL_3DFX_multisample
#undef CSGL_3DFX_tbuffer
#undef CSGL_EXT_multisample
#undef CSGL_SGI_vertex_preclip
#undef CSGL_SGIX_convolution_accuracy
#undef CSGL_SGIX_resample
#undef CSGL_SGIS_point_line_texgen
#undef CSGL_SGIS_texture_color_mask
#endif

#undef CSGL_FOR_ALL

#endif
