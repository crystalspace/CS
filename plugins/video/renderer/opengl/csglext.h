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

//#if defined(_WIN32) && !defined(_WINGDI_) && !defined(__CYGWIN__) && !defined(_GNU_H_WINDOWS32_DEFINES) && !defined(OPENSTEP)
//#include <gl/mesa_wgl.h>
//#endif

#if defined(macintosh) && PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

#if defined(_WIN32) && !defined(APIENTRY) && !defined(__CYGWIN__)
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#endif

#ifndef csAPIENTRY
#ifndef APIENTRY
#define csAPIENTRY
#else
#define csAPIENTRY APIENTRY
#endif
#endif

/*
 * End system-specific stuff.
 **********************************************************************/


#ifndef _CSGL_FUNCTTYPES_
//#define _CSGL_FUNCTTYPES_

typedef void (csAPIENTRY * csPFNGLBLENDCOLORPROC) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef void (csAPIENTRY * csPFNGLBLENDEQUATIONPROC) (GLenum mode);
typedef void (csAPIENTRY * csPFNGLDRAWRANGEELEMENTSPROC) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);
typedef void (csAPIENTRY * csPFNGLCOLORTABLEPROC) (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table);
typedef void (csAPIENTRY * csPFNGLCOLORTABLEPARAMETERFVPROC) (GLenum target, GLenum pname, const GLfloat *params);
typedef void (csAPIENTRY * csPFNGLCOLORTABLEPARAMETERIVPROC) (GLenum target, GLenum pname, const GLint *params);
typedef void (csAPIENTRY * csPFNGLCOPYCOLORTABLEPROC) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
typedef void (csAPIENTRY * csPFNGLGETCOLORTABLEPROC) (GLenum target, GLenum format, GLenum type, GLvoid *table);
typedef void (csAPIENTRY * csPFNGLGETCOLORTABLEPARAMETERFVPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef void (csAPIENTRY * csPFNGLGETCOLORTABLEPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (csAPIENTRY * csPFNGLCOLORSUBTABLEPROC) (GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data);
typedef void (csAPIENTRY * csPFNGLCOPYCOLORSUBTABLEPROC) (GLenum target, GLsizei start, GLint x, GLint y, GLsizei width);
typedef void (csAPIENTRY * csPFNGLCONVOLUTIONFILTER1DPROC) (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *image);
typedef void (csAPIENTRY * csPFNGLCONVOLUTIONFILTER2DPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image);
typedef void (csAPIENTRY * csPFNGLCONVOLUTIONPARAMETERFPROC) (GLenum target, GLenum pname, GLfloat params);
typedef void (csAPIENTRY * csPFNGLCONVOLUTIONPARAMETERFVPROC) (GLenum target, GLenum pname, const GLfloat *params);
typedef void (csAPIENTRY * csPFNGLCONVOLUTIONPARAMETERIPROC) (GLenum target, GLenum pname, GLint params);
typedef void (csAPIENTRY * csPFNGLCONVOLUTIONPARAMETERIVPROC) (GLenum target, GLenum pname, const GLint *params);
typedef void (csAPIENTRY * csPFNGLCOPYCONVOLUTIONFILTER1DPROC) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
typedef void (csAPIENTRY * csPFNGLCOPYCONVOLUTIONFILTER2DPROC) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (csAPIENTRY * csPFNGLGETCONVOLUTIONFILTERPROC) (GLenum target, GLenum format, GLenum type, GLvoid *image);
typedef void (csAPIENTRY * csPFNGLGETCONVOLUTIONPARAMETERFVPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef void (csAPIENTRY * csPFNGLGETCONVOLUTIONPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (csAPIENTRY * csPFNGLGETSEPARABLEFILTERPROC) (GLenum target, GLenum format, GLenum type, GLvoid *row, GLvoid *column, GLvoid *span);
typedef void (csAPIENTRY * csPFNGLSEPARABLEFILTER2DPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *row, const GLvoid *column);
typedef void (csAPIENTRY * csPFNGLGETHISTOGRAMPROC) (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
typedef void (csAPIENTRY * csPFNGLGETHISTOGRAMPARAMETERFVPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef void (csAPIENTRY * csPFNGLGETHISTOGRAMPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (csAPIENTRY * csPFNGLGETMINMAXPROC) (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
typedef void (csAPIENTRY * csPFNGLGETMINMAXPARAMETERFVPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef void (csAPIENTRY * csPFNGLGETMINMAXPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (csAPIENTRY * csPFNGLHISTOGRAMPROC) (GLenum target, GLsizei width, GLenum internalformat, GLboolean sink);
typedef void (csAPIENTRY * csPFNGLMINMAXPROC) (GLenum target, GLenum internalformat, GLboolean sink);
typedef void (csAPIENTRY * csPFNGLRESETHISTOGRAMPROC) (GLenum target);
typedef void (csAPIENTRY * csPFNGLRESETMINMAXPROC) (GLenum target);
typedef void (csAPIENTRY * csPFNGLTEXIMAGE3DPROC) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (csAPIENTRY * csPFNGLTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (csAPIENTRY * csPFNGLCOPYTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (csAPIENTRY * csPFNGLACTIVETEXTUREARBPROC) (GLenum texture);
typedef void (csAPIENTRY * csPFNGLCLIENTACTIVETEXTUREARBPROC) (GLenum texture);
typedef void (csAPIENTRY * csPFNGLMULTITEXCOORD1DARBPROC) (GLenum target, GLdouble s);
typedef void (csAPIENTRY * csPFNGLMULTITEXCOORD1DVARBPROC) (GLenum target, const GLdouble *v);
typedef void (csAPIENTRY * csPFNGLMULTITEXCOORD1FARBPROC) (GLenum target, GLfloat s);
typedef void (csAPIENTRY * csPFNGLMULTITEXCOORD1FVARBPROC) (GLenum target, const GLfloat *v);
typedef void (csAPIENTRY * csPFNGLMULTITEXCOORD1IARBPROC) (GLenum target, GLint s);
typedef void (csAPIENTRY * csPFNGLMULTITEXCOORD1IVARBPROC) (GLenum target, const GLint *v);
typedef void (csAPIENTRY * csPFNGLMULTITEXCOORD1SARBPROC) (GLenum target, GLshort s);
typedef void (csAPIENTRY * csPFNGLMULTITEXCOORD1SVARBPROC) (GLenum target, const GLshort *v);
typedef void (csAPIENTRY * csPFNGLMULTITEXCOORD2DARBPROC) (GLenum target, GLdouble s, GLdouble t);
typedef void (csAPIENTRY * csPFNGLMULTITEXCOORD2DVARBPROC) (GLenum target, const GLdouble *v);
typedef void (csAPIENTRY * csPFNGLMULTITEXCOORD2FARBPROC) (GLenum target, GLfloat s, GLfloat t);
typedef void (csAPIENTRY * csPFNGLMULTITEXCOORD2FVARBPROC) (GLenum target, const GLfloat *v);
typedef void (csAPIENTRY * csPFNGLMULTITEXCOORD2IARBPROC) (GLenum target, GLint s, GLint t);
typedef void (csAPIENTRY * csPFNGLMULTITEXCOORD2IVARBPROC) (GLenum target, const GLint *v);
typedef void (csAPIENTRY * csPFNGLMULTITEXCOORD2SARBPROC) (GLenum target, GLshort s, GLshort t);
typedef void (csAPIENTRY * csPFNGLMULTITEXCOORD2SVARBPROC) (GLenum target, const GLshort *v);
typedef void (csAPIENTRY * csPFNGLMULTITEXCOORD3DARBPROC) (GLenum target, GLdouble s, GLdouble t, GLdouble r);
typedef void (csAPIENTRY * csPFNGLMULTITEXCOORD3DVARBPROC) (GLenum target, const GLdouble *v);
typedef void (csAPIENTRY * csPFNGLMULTITEXCOORD3FARBPROC) (GLenum target, GLfloat s, GLfloat t, GLfloat r);
typedef void (csAPIENTRY * csPFNGLMULTITEXCOORD3FVARBPROC) (GLenum target, const GLfloat *v);
typedef void (csAPIENTRY * csPFNGLMULTITEXCOORD3IARBPROC) (GLenum target, GLint s, GLint t, GLint r);
typedef void (csAPIENTRY * csPFNGLMULTITEXCOORD3IVARBPROC) (GLenum target, const GLint *v);
typedef void (csAPIENTRY * csPFNGLMULTITEXCOORD3SARBPROC) (GLenum target, GLshort s, GLshort t, GLshort r);
typedef void (csAPIENTRY * csPFNGLMULTITEXCOORD3SVARBPROC) (GLenum target, const GLshort *v);
typedef void (csAPIENTRY * csPFNGLMULTITEXCOORD4DARBPROC) (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
typedef void (csAPIENTRY * csPFNGLMULTITEXCOORD4DVARBPROC) (GLenum target, const GLdouble *v);
typedef void (csAPIENTRY * csPFNGLMULTITEXCOORD4FARBPROC) (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
typedef void (csAPIENTRY * csPFNGLMULTITEXCOORD4FVARBPROC) (GLenum target, const GLfloat *v);
typedef void (csAPIENTRY * csPFNGLMULTITEXCOORD4IARBPROC) (GLenum target, GLint s, GLint t, GLint r, GLint q);
typedef void (csAPIENTRY * csPFNGLMULTITEXCOORD4IVARBPROC) (GLenum target, const GLint *v);
typedef void (csAPIENTRY * csPFNGLMULTITEXCOORD4SARBPROC) (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
typedef void (csAPIENTRY * csPFNGLMULTITEXCOORD4SVARBPROC) (GLenum target, const GLshort *v);
typedef void (csAPIENTRY * csPFNGLLOADTRANSPOSEMATRIXFARBPROC) (const GLfloat *m);
typedef void (csAPIENTRY * csPFNGLLOADTRANSPOSEMATRIXDARBPROC) (const GLdouble *m);
typedef void (csAPIENTRY * csPFNGLMULTTRANSPOSEMATRIXFARBPROC) (const GLfloat *m);
typedef void (csAPIENTRY * csPFNGLMULTTRANSPOSEMATRIXDARBPROC) (const GLdouble *m);
typedef void (csAPIENTRY * csPFNGLSAMPLECOVERAGEARBPROC) (GLclampf value, GLboolean invert);
typedef void (csAPIENTRY * csPFNGLSAMPLEPASSARBPROC) (GLenum pass);
typedef void (csAPIENTRY * csPFNGLCOMPRESSEDTEXIMAGE3DARBPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (csAPIENTRY * csPFNGLCOMPRESSEDTEXIMAGE2DARBPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (csAPIENTRY * csPFNGLCOMPRESSEDTEXIMAGE1DARBPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (csAPIENTRY * csPFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (csAPIENTRY * csPFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (csAPIENTRY * csPFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (csAPIENTRY * csPFNGLGETCOMPRESSEDTEXIMAGEARBPROC) (GLenum target, GLint level, void *img);
typedef void (csAPIENTRY * csPFNGLBLENDCOLOREXTPROC) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef void (csAPIENTRY * csPFNGLPOLYGONOFFSETEXTPROC) (GLfloat factor, GLfloat bias);
typedef void (csAPIENTRY * csPFNGLTEXIMAGE3DEXTPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (csAPIENTRY * csPFNGLTEXSUBIMAGE3DEXTPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (csAPIENTRY * csPFNGLGETTEXFILTERFUNCSGISPROC) (GLenum target, GLenum filter, GLfloat *weights);
typedef void (csAPIENTRY * csPFNGLTEXFILTERFUNCSGISPROC) (GLenum target, GLenum filter, GLsizei n, const GLfloat *weights);
typedef void (csAPIENTRY * csPFNGLTEXSUBIMAGE1DEXTPROC) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (csAPIENTRY * csPFNGLTEXSUBIMAGE2DEXTPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (csAPIENTRY * csPFNGLCOPYTEXIMAGE1DEXTPROC) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
typedef void (csAPIENTRY * csPFNGLCOPYTEXIMAGE2DEXTPROC) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
typedef void (csAPIENTRY * csPFNGLCOPYTEXSUBIMAGE1DEXTPROC) (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
typedef void (csAPIENTRY * csPFNGLCOPYTEXSUBIMAGE2DEXTPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (csAPIENTRY * csPFNGLCOPYTEXSUBIMAGE3DEXTPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (csAPIENTRY * csPFNGLGETHISTOGRAMEXTPROC) (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
typedef void (csAPIENTRY * csPFNGLGETHISTOGRAMPARAMETERFVEXTPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef void (csAPIENTRY * csPFNGLGETHISTOGRAMPARAMETERIVEXTPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (csAPIENTRY * csPFNGLGETMINMAXEXTPROC) (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
typedef void (csAPIENTRY * csPFNGLGETMINMAXPARAMETERFVEXTPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef void (csAPIENTRY * csPFNGLGETMINMAXPARAMETERIVEXTPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (csAPIENTRY * csPFNGLHISTOGRAMEXTPROC) (GLenum target, GLsizei width, GLenum internalformat, GLboolean sink);
typedef void (csAPIENTRY * csPFNGLMINMAXEXTPROC) (GLenum target, GLenum internalformat, GLboolean sink);
typedef void (csAPIENTRY * csPFNGLRESETHISTOGRAMEXTPROC) (GLenum target);
typedef void (csAPIENTRY * csPFNGLRESETMINMAXEXTPROC) (GLenum target);
typedef void (csAPIENTRY * csPFNGLCONVOLUTIONFILTER1DEXTPROC) (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *image);
typedef void (csAPIENTRY * csPFNGLCONVOLUTIONFILTER2DEXTPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image);
typedef void (csAPIENTRY * csPFNGLCONVOLUTIONPARAMETERFEXTPROC) (GLenum target, GLenum pname, GLfloat params);
typedef void (csAPIENTRY * csPFNGLCONVOLUTIONPARAMETERFVEXTPROC) (GLenum target, GLenum pname, const GLfloat *params);
typedef void (csAPIENTRY * csPFNGLCONVOLUTIONPARAMETERIEXTPROC) (GLenum target, GLenum pname, GLint params);
typedef void (csAPIENTRY * csPFNGLCONVOLUTIONPARAMETERIVEXTPROC) (GLenum target, GLenum pname, const GLint *params);
typedef void (csAPIENTRY * csPFNGLCOPYCONVOLUTIONFILTER1DEXTPROC) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
typedef void (csAPIENTRY * csPFNGLCOPYCONVOLUTIONFILTER2DEXTPROC) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (csAPIENTRY * csPFNGLGETCONVOLUTIONFILTEREXTPROC) (GLenum target, GLenum format, GLenum type, GLvoid *image);
typedef void (csAPIENTRY * csPFNGLGETCONVOLUTIONPARAMETERFVEXTPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef void (csAPIENTRY * csPFNGLGETCONVOLUTIONPARAMETERIVEXTPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (csAPIENTRY * csPFNGLGETSEPARABLEFILTEREXTPROC) (GLenum target, GLenum format, GLenum type, GLvoid *row, GLvoid *column, GLvoid *span);
typedef void (csAPIENTRY * csPFNGLSEPARABLEFILTER2DEXTPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *row, const GLvoid *column);
typedef void (csAPIENTRY * csPFNGLCOLORTABLESGIPROC) (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table);
typedef void (csAPIENTRY * csPFNGLCOLORTABLEPARAMETERFVSGIPROC) (GLenum target, GLenum pname, const GLfloat *params);
typedef void (csAPIENTRY * csPFNGLCOLORTABLEPARAMETERIVSGIPROC) (GLenum target, GLenum pname, const GLint *params);
typedef void (csAPIENTRY * csPFNGLCOPYCOLORTABLESGIPROC) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
typedef void (csAPIENTRY * csPFNGLGETCOLORTABLESGIPROC) (GLenum target, GLenum format, GLenum type, GLvoid *table);
typedef void (csAPIENTRY * csPFNGLGETCOLORTABLEPARAMETERFVSGIPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef void (csAPIENTRY * csPFNGLGETCOLORTABLEPARAMETERIVSGIPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (csAPIENTRY * csPFNGLPIXELTEXGENSGIXPROC) (GLenum mode);
typedef void (csAPIENTRY * csPFNGLPIXELTEXGENPARAMETERISGISPROC) (GLenum pname, GLint param);
typedef void (csAPIENTRY * csPFNGLPIXELTEXGENPARAMETERIVSGISPROC) (GLenum pname, const GLint *params);
typedef void (csAPIENTRY * csPFNGLPIXELTEXGENPARAMETERFSGISPROC) (GLenum pname, GLfloat param);
typedef void (csAPIENTRY * csPFNGLPIXELTEXGENPARAMETERFVSGISPROC) (GLenum pname, const GLfloat *params);
typedef void (csAPIENTRY * csPFNGLGETPIXELTEXGENPARAMETERIVSGISPROC) (GLenum pname, GLint *params);
typedef void (csAPIENTRY * csPFNGLGETPIXELTEXGENPARAMETERFVSGISPROC) (GLenum pname, GLfloat *params);
typedef void (csAPIENTRY * csPFNGLTEXIMAGE4DSGISPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLsizei size4d, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (csAPIENTRY * csPFNGLTEXSUBIMAGE4DSGISPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint woffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei size4d, GLenum format, GLenum type, const GLvoid *pixels);
typedef GLboolean (csAPIENTRY * csPFNGLARETEXTURESRESIDENTEXTPROC) (GLsizei n, const GLuint *textures, GLboolean *residences);
typedef void (csAPIENTRY * csPFNGLBINDTEXTUREEXTPROC) (GLenum target, GLuint texture);
typedef void (csAPIENTRY * csPFNGLDELETETEXTURESEXTPROC) (GLsizei n, const GLuint *textures);
typedef void (csAPIENTRY * csPFNGLGENTEXTURESEXTPROC) (GLsizei n, GLuint *textures);
typedef GLboolean (csAPIENTRY * csPFNGLISTEXTUREEXTPROC) (GLuint texture);
typedef void (csAPIENTRY * csPFNGLPRIORITIZETEXTURESEXTPROC) (GLsizei n, const GLuint *textures, const GLclampf *priorities);
typedef void (csAPIENTRY * csPFNGLDETAILTEXFUNCSGISPROC) (GLenum target, GLsizei n, const GLfloat *points);
typedef void (csAPIENTRY * csPFNGLGETDETAILTEXFUNCSGISPROC) (GLenum target, GLfloat *points);
typedef void (csAPIENTRY * csPFNGLSHARPENTEXFUNCSGISPROC) (GLenum target, GLsizei n, const GLfloat *points);
typedef void (csAPIENTRY * csPFNGLGETSHARPENTEXFUNCSGISPROC) (GLenum target, GLfloat *points);
typedef void (csAPIENTRY * csPFNGLSAMPLEMASKSGISPROC) (GLclampf value, GLboolean invert);
typedef void (csAPIENTRY * csPFNGLSAMPLEPATTERNSGISPROC) (GLenum pattern);
typedef void (csAPIENTRY * csPFNGLARRAYELEMENTEXTPROC) (GLint i);
typedef void (csAPIENTRY * csPFNGLCOLORPOINTEREXTPROC) (GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid *pointer);
typedef void (csAPIENTRY * csPFNGLDRAWARRAYSEXTPROC) (GLenum mode, GLint first, GLsizei count);
typedef void (csAPIENTRY * csPFNGLEDGEFLAGPOINTEREXTPROC) (GLsizei stride, GLsizei count, const GLboolean *pointer);
typedef void (csAPIENTRY * csPFNGLGETPOINTERVEXTPROC) (GLenum pname, GLvoid* *params);
typedef void (csAPIENTRY * csPFNGLINDEXPOINTEREXTPROC) (GLenum type, GLsizei stride, GLsizei count, const GLvoid *pointer);
typedef void (csAPIENTRY * csPFNGLNORMALPOINTEREXTPROC) (GLenum type, GLsizei stride, GLsizei count, const GLvoid *pointer);
typedef void (csAPIENTRY * csPFNGLTEXCOORDPOINTEREXTPROC) (GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid *pointer);
typedef void (csAPIENTRY * csPFNGLVERTEXPOINTEREXTPROC) (GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid *pointer);
typedef void (csAPIENTRY * csPFNGLBLENDEQUATIONEXTPROC) (GLenum mode);
typedef void (csAPIENTRY * csPFNGLSPRITEPARAMETERFSGIXPROC) (GLenum pname, GLfloat param);
typedef void (csAPIENTRY * csPFNGLSPRITEPARAMETERFVSGIXPROC) (GLenum pname, const GLfloat *params);
typedef void (csAPIENTRY * csPFNGLSPRITEPARAMETERISGIXPROC) (GLenum pname, GLint param);
typedef void (csAPIENTRY * csPFNGLSPRITEPARAMETERIVSGIXPROC) (GLenum pname, const GLint *params);
typedef void (csAPIENTRY * csPFNGLPOINTPARAMETERFEXTPROC) (GLenum pname, GLfloat param);
typedef void (csAPIENTRY * csPFNGLPOINTPARAMETERFVEXTPROC) (GLenum pname, const GLfloat *params);
typedef void (csAPIENTRY * csPFNGLPOINTPARAMETERFSGISPROC) (GLenum pname, GLfloat param);
typedef void (csAPIENTRY * csPFNGLPOINTPARAMETERFVSGISPROC) (GLenum pname, const GLfloat *params);
typedef GLint (csAPIENTRY * csPFNGLGETINSTRUMENTSSGIXPROC) (void);
typedef void (csAPIENTRY * csPFNGLINSTRUMENTSBUFFERSGIXPROC) (GLsizei size, GLint *buffer);
typedef GLint (csAPIENTRY * csPFNGLPOLLINSTRUMENTSSGIXPROC) (GLint *marker_p);
typedef void (csAPIENTRY * csPFNGLREADINSTRUMENTSSGIXPROC) (GLint marker);
typedef void (csAPIENTRY * csPFNGLSTARTINSTRUMENTSSGIXPROC) (void);
typedef void (csAPIENTRY * csPFNGLSTOPINSTRUMENTSSGIXPROC) (GLint marker);
typedef void (csAPIENTRY * csPFNGLFRAMEZOOMSGIXPROC) (GLint factor);
typedef void (csAPIENTRY * csPFNGLTAGSAMPLEBUFFERSGIXPROC) (void);
typedef void (csAPIENTRY * csPFNGLREFERENCEPLANESGIXPROC) (const GLdouble *equation);
typedef void (csAPIENTRY * csPFNGLFLUSHRASTERSGIXPROC) (void);
typedef void (csAPIENTRY * csPFNGLFOGFUNCSGISPROC) (GLsizei n, const GLfloat *points);
typedef void (csAPIENTRY * csPFNGLGETFOGFUNCSGISPROC) (const GLfloat *points);
typedef void (csAPIENTRY * csPFNGLIMAGETRANSFORMPARAMETERIHPPROC) (GLenum target, GLenum pname, GLint param);
typedef void (csAPIENTRY * csPFNGLIMAGETRANSFORMPARAMETERFHPPROC) (GLenum target, GLenum pname, GLfloat param);
typedef void (csAPIENTRY * csPFNGLIMAGETRANSFORMPARAMETERIVHPPROC) (GLenum target, GLenum pname, const GLint *params);
typedef void (csAPIENTRY * csPFNGLIMAGETRANSFORMPARAMETERFVHPPROC) (GLenum target, GLenum pname, const GLfloat *params);
typedef void (csAPIENTRY * csPFNGLGETIMAGETRANSFORMPARAMETERIVHPPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (csAPIENTRY * csPFNGLGETIMAGETRANSFORMPARAMETERFVHPPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef void (csAPIENTRY * csPFNGLCOLORSUBTABLEEXTPROC) (GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data);
typedef void (csAPIENTRY * csPFNGLCOPYCOLORSUBTABLEEXTPROC) (GLenum target, GLsizei start, GLint x, GLint y, GLsizei width);
typedef void (csAPIENTRY * csPFNGLHINTPGIPROC) (GLenum target, GLint mode);
typedef void (csAPIENTRY * csPFNGLCOLORTABLEEXTPROC) (GLenum target, GLenum internalFormat, GLsizei width, GLenum format, GLenum type, const GLvoid *table);
typedef void (csAPIENTRY * csPFNGLGETCOLORTABLEEXTPROC) (GLenum target, GLenum format, GLenum type, GLvoid *data);
typedef void (csAPIENTRY * csPFNGLGETCOLORTABLEPARAMETERIVEXTPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (csAPIENTRY * csPFNGLGETCOLORTABLEPARAMETERFVEXTPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef void (csAPIENTRY * csPFNGLGETLISTPARAMETERFVSGIXPROC) (GLuint list, GLenum pname, GLfloat *params);
typedef void (csAPIENTRY * csPFNGLGETLISTPARAMETERIVSGIXPROC) (GLuint list, GLenum pname, GLint *params);
typedef void (csAPIENTRY * csPFNGLLISTPARAMETERFSGIXPROC) (GLuint list, GLenum pname, GLfloat param);
typedef void (csAPIENTRY * csPFNGLLISTPARAMETERFVSGIXPROC) (GLuint list, GLenum pname, const GLfloat *params);
typedef void (csAPIENTRY * csPFNGLLISTPARAMETERISGIXPROC) (GLuint list, GLenum pname, GLint param);
typedef void (csAPIENTRY * csPFNGLLISTPARAMETERIVSGIXPROC) (GLuint list, GLenum pname, const GLint *params);
typedef void (csAPIENTRY * csPFNGLINDEXMATERIALEXTPROC) (GLenum face, GLenum mode);
typedef void (csAPIENTRY * csPFNGLINDEXFUNCEXTPROC) (GLenum func, GLclampf ref);
typedef void (csAPIENTRY * csPFNGLLOCKARRAYSEXTPROC) (GLint first, GLsizei count);
typedef void (csAPIENTRY * csPFNGLUNLOCKARRAYSEXTPROC) (void);
typedef void (csAPIENTRY * csPFNGLCULLPARAMETERDVEXTPROC) (GLenum pname, GLdouble *params);
typedef void (csAPIENTRY * csPFNGLCULLPARAMETERFVEXTPROC) (GLenum pname, GLfloat *params);
typedef void (csAPIENTRY * csPFNGLFRAGMENTCOLORMATERIALSGIXPROC) (GLenum face, GLenum mode);
typedef void (csAPIENTRY * csPFNGLFRAGMENTLIGHTFSGIXPROC) (GLenum light, GLenum pname, GLfloat param);
typedef void (csAPIENTRY * csPFNGLFRAGMENTLIGHTFVSGIXPROC) (GLenum light, GLenum pname, const GLfloat *params);
typedef void (csAPIENTRY * csPFNGLFRAGMENTLIGHTISGIXPROC) (GLenum light, GLenum pname, GLint param);
typedef void (csAPIENTRY * csPFNGLFRAGMENTLIGHTIVSGIXPROC) (GLenum light, GLenum pname, const GLint *params);
typedef void (csAPIENTRY * csPFNGLFRAGMENTLIGHTMODELFSGIXPROC) (GLenum pname, GLfloat param);
typedef void (csAPIENTRY * csPFNGLFRAGMENTLIGHTMODELFVSGIXPROC) (GLenum pname, const GLfloat *params);
typedef void (csAPIENTRY * csPFNGLFRAGMENTLIGHTMODELISGIXPROC) (GLenum pname, GLint param);
typedef void (csAPIENTRY * csPFNGLFRAGMENTLIGHTMODELIVSGIXPROC) (GLenum pname, const GLint *params);
typedef void (csAPIENTRY * csPFNGLFRAGMENTMATERIALFSGIXPROC) (GLenum face, GLenum pname, GLfloat param);
typedef void (csAPIENTRY * csPFNGLFRAGMENTMATERIALFVSGIXPROC) (GLenum face, GLenum pname, const GLfloat *params);
typedef void (csAPIENTRY * csPFNGLFRAGMENTMATERIALISGIXPROC) (GLenum face, GLenum pname, GLint param);
typedef void (csAPIENTRY * csPFNGLFRAGMENTMATERIALIVSGIXPROC) (GLenum face, GLenum pname, const GLint *params);
typedef void (csAPIENTRY * csPFNGLGETFRAGMENTLIGHTFVSGIXPROC) (GLenum light, GLenum pname, GLfloat *params);
typedef void (csAPIENTRY * csPFNGLGETFRAGMENTLIGHTIVSGIXPROC) (GLenum light, GLenum pname, GLint *params);
typedef void (csAPIENTRY * csPFNGLGETFRAGMENTMATERIALFVSGIXPROC) (GLenum face, GLenum pname, GLfloat *params);
typedef void (csAPIENTRY * csPFNGLGETFRAGMENTMATERIALIVSGIXPROC) (GLenum face, GLenum pname, GLint *params);
typedef void (csAPIENTRY * csPFNGLLIGHTENVISGIXPROC) (GLenum pname, GLint param);
typedef void (csAPIENTRY * csPFNGLDRAWRANGEELEMENTSEXTPROC) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);
typedef void (csAPIENTRY * csPFNGLAPPLYTEXTUREEXTPROC) (GLenum mode);
typedef void (csAPIENTRY * csPFNGLTEXTURELIGHTEXTPROC) (GLenum pname);
typedef void (csAPIENTRY * csPFNGLTEXTUREMATERIALEXTPROC) (GLenum face, GLenum mode);
typedef void (csAPIENTRY * csPFNGLVERTEXPOINTERVINTELPROC) (GLint size, GLenum type, const GLvoid* *pointer);
typedef void (csAPIENTRY * csPFNGLNORMALPOINTERVINTELPROC) (GLenum type, const GLvoid* *pointer);
typedef void (csAPIENTRY * csPFNGLCOLORPOINTERVINTELPROC) (GLint size, GLenum type, const GLvoid* *pointer);
typedef void (csAPIENTRY * csPFNGLTEXCOORDPOINTERVINTELPROC) (GLint size, GLenum type, const GLvoid* *pointer);
typedef void (csAPIENTRY * csPFNGLPIXELTRANSFORMPARAMETERIEXTPROC) (GLenum target, GLenum pname, GLint param);
typedef void (csAPIENTRY * csPFNGLPIXELTRANSFORMPARAMETERFEXTPROC) (GLenum target, GLenum pname, GLfloat param);
typedef void (csAPIENTRY * csPFNGLPIXELTRANSFORMPARAMETERIVEXTPROC) (GLenum target, GLenum pname, const GLint *params);
typedef void (csAPIENTRY * csPFNGLPIXELTRANSFORMPARAMETERFVEXTPROC) (GLenum target, GLenum pname, const GLfloat *params);
typedef void (csAPIENTRY * csPFNGLSECONDARYCOLOR3BEXTPROC) (GLbyte red, GLbyte green, GLbyte blue);
typedef void (csAPIENTRY * csPFNGLSECONDARYCOLOR3BVEXTPROC) (const GLbyte *v);
typedef void (csAPIENTRY * csPFNGLSECONDARYCOLOR3DEXTPROC) (GLdouble red, GLdouble green, GLdouble blue);
typedef void (csAPIENTRY * csPFNGLSECONDARYCOLOR3DVEXTPROC) (const GLdouble *v);
typedef void (csAPIENTRY * csPFNGLSECONDARYCOLOR3FEXTPROC) (GLfloat red, GLfloat green, GLfloat blue);
typedef void (csAPIENTRY * csPFNGLSECONDARYCOLOR3FVEXTPROC) (const GLfloat *v);
typedef void (csAPIENTRY * csPFNGLSECONDARYCOLOR3IEXTPROC) (GLint red, GLint green, GLint blue);
typedef void (csAPIENTRY * csPFNGLSECONDARYCOLOR3IVEXTPROC) (const GLint *v);
typedef void (csAPIENTRY * csPFNGLSECONDARYCOLOR3SEXTPROC) (GLshort red, GLshort green, GLshort blue);
typedef void (csAPIENTRY * csPFNGLSECONDARYCOLOR3SVEXTPROC) (const GLshort *v);
typedef void (csAPIENTRY * csPFNGLSECONDARYCOLOR3UBEXTPROC) (GLubyte red, GLubyte green, GLubyte blue);
typedef void (csAPIENTRY * csPFNGLSECONDARYCOLOR3UBVEXTPROC) (const GLubyte *v);
typedef void (csAPIENTRY * csPFNGLSECONDARYCOLOR3UIEXTPROC) (GLuint red, GLuint green, GLuint blue);
typedef void (csAPIENTRY * csPFNGLSECONDARYCOLOR3UIVEXTPROC) (const GLuint *v);
typedef void (csAPIENTRY * csPFNGLSECONDARYCOLOR3USEXTPROC) (GLushort red, GLushort green, GLushort blue);
typedef void (csAPIENTRY * csPFNGLSECONDARYCOLOR3USVEXTPROC) (const GLushort *v);
typedef void (csAPIENTRY * csPFNGLSECONDARYCOLORPOINTEREXTPROC) (GLint size, GLenum type, GLsizei stride, GLvoid *pointer);
typedef void (csAPIENTRY * csPFNGLTEXTURENORMALEXTPROC) (GLenum mode);
typedef void (csAPIENTRY * csPFNGLMULTIDRAWARRAYSEXTPROC) (GLenum mode, GLint *first, GLsizei *count, GLsizei primcount);
typedef void (csAPIENTRY * csPFNGLMULTIDRAWELEMENTSEXTPROC) (GLenum mode, const GLsizei *count, GLenum type, const GLvoid* *indices, GLsizei primcount);
typedef void (csAPIENTRY * csPFNGLFOGCOORDFEXTPROC) (GLfloat coord);
typedef void (csAPIENTRY * csPFNGLFOGCOORDFVEXTPROC) (const GLfloat *coord);
typedef void (csAPIENTRY * csPFNGLFOGCOORDDEXTPROC) (GLdouble coord);
typedef void (csAPIENTRY * csPFNGLFOGCOORDDVEXTPROC) (const GLdouble *coord);
typedef void (csAPIENTRY * csPFNGLFOGCOORDPOINTEREXTPROC) (GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void (csAPIENTRY * csPFNGLTANGENT3BEXTPROC) (GLbyte tx, GLbyte ty, GLbyte tz);
typedef void (csAPIENTRY * csPFNGLTANGENT3BVEXTPROC) (const GLbyte *v);
typedef void (csAPIENTRY * csPFNGLTANGENT3DEXTPROC) (GLdouble tx, GLdouble ty, GLdouble tz);
typedef void (csAPIENTRY * csPFNGLTANGENT3DVEXTPROC) (const GLdouble *v);
typedef void (csAPIENTRY * csPFNGLTANGENT3FEXTPROC) (GLfloat tx, GLfloat ty, GLfloat tz);
typedef void (csAPIENTRY * csPFNGLTANGENT3FVEXTPROC) (const GLfloat *v);
typedef void (csAPIENTRY * csPFNGLTANGENT3IEXTPROC) (GLint tx, GLint ty, GLint tz);
typedef void (csAPIENTRY * csPFNGLTANGENT3IVEXTPROC) (const GLint *v);
typedef void (csAPIENTRY * csPFNGLTANGENT3SEXTPROC) (GLshort tx, GLshort ty, GLshort tz);
typedef void (csAPIENTRY * csPFNGLTANGENT3SVEXTPROC) (const GLshort *v);
typedef void (csAPIENTRY * csPFNGLBINORMAL3BEXTPROC) (GLbyte bx, GLbyte by, GLbyte bz);
typedef void (csAPIENTRY * csPFNGLBINORMAL3BVEXTPROC) (const GLbyte *v);
typedef void (csAPIENTRY * csPFNGLBINORMAL3DEXTPROC) (GLdouble bx, GLdouble by, GLdouble bz);
typedef void (csAPIENTRY * csPFNGLBINORMAL3DVEXTPROC) (const GLdouble *v);
typedef void (csAPIENTRY * csPFNGLBINORMAL3FEXTPROC) (GLfloat bx, GLfloat by, GLfloat bz);
typedef void (csAPIENTRY * csPFNGLBINORMAL3FVEXTPROC) (const GLfloat *v);
typedef void (csAPIENTRY * csPFNGLBINORMAL3IEXTPROC) (GLint bx, GLint by, GLint bz);
typedef void (csAPIENTRY * csPFNGLBINORMAL3IVEXTPROC) (const GLint *v);
typedef void (csAPIENTRY * csPFNGLBINORMAL3SEXTPROC) (GLshort bx, GLshort by, GLshort bz);
typedef void (csAPIENTRY * csPFNGLBINORMAL3SVEXTPROC) (const GLshort *v);
typedef void (csAPIENTRY * csPFNGLTANGENTPOINTEREXTPROC) (GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void (csAPIENTRY * csPFNGLBINORMALPOINTEREXTPROC) (GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void (csAPIENTRY * csPFNGLFINISHTEXTURESUNXPROC) (void);
typedef void (csAPIENTRY * csPFNGLGLOBALALPHAFACTORBSUNPROC) (GLbyte factor);
typedef void (csAPIENTRY * csPFNGLGLOBALALPHAFACTORSSUNPROC) (GLshort factor);
typedef void (csAPIENTRY * csPFNGLGLOBALALPHAFACTORISUNPROC) (GLint factor);
typedef void (csAPIENTRY * csPFNGLGLOBALALPHAFACTORFSUNPROC) (GLfloat factor);
typedef void (csAPIENTRY * csPFNGLGLOBALALPHAFACTORDSUNPROC) (GLdouble factor);
typedef void (csAPIENTRY * csPFNGLGLOBALALPHAFACTORUBSUNPROC) (GLubyte factor);
typedef void (csAPIENTRY * csPFNGLGLOBALALPHAFACTORUSSUNPROC) (GLushort factor);
typedef void (csAPIENTRY * csPFNGLGLOBALALPHAFACTORUISUNPROC) (GLuint factor);
typedef void (csAPIENTRY * csPFNGLREPLACEMENTCODEUISUNPROC) (GLuint code);
typedef void (csAPIENTRY * csPFNGLREPLACEMENTCODEUSSUNPROC) (GLushort code);
typedef void (csAPIENTRY * csPFNGLREPLACEMENTCODEUBSUNPROC) (GLubyte code);
typedef void (csAPIENTRY * csPFNGLREPLACEMENTCODEUIVSUNPROC) (const GLuint *code);
typedef void (csAPIENTRY * csPFNGLREPLACEMENTCODEUSVSUNPROC) (const GLushort *code);
typedef void (csAPIENTRY * csPFNGLREPLACEMENTCODEUBVSUNPROC) (const GLubyte *code);
typedef void (csAPIENTRY * csPFNGLREPLACEMENTCODEPOINTERSUNPROC) (GLenum type, GLsizei stride, const GLvoid* *pointer);
typedef void (csAPIENTRY * csPFNGLCOLOR4UBVERTEX2FSUNPROC) (GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y);
typedef void (csAPIENTRY * csPFNGLCOLOR4UBVERTEX2FVSUNPROC) (const GLubyte *c, const GLfloat *v);
typedef void (csAPIENTRY * csPFNGLCOLOR4UBVERTEX3FSUNPROC) (GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z);
typedef void (csAPIENTRY * csPFNGLCOLOR4UBVERTEX3FVSUNPROC) (const GLubyte *c, const GLfloat *v);
typedef void (csAPIENTRY * csPFNGLCOLOR3FVERTEX3FSUNPROC) (GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z);
typedef void (csAPIENTRY * csPFNGLCOLOR3FVERTEX3FVSUNPROC) (const GLfloat *c, const GLfloat *v);
typedef void (csAPIENTRY * csPFNGLNORMAL3FVERTEX3FSUNPROC) (GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
typedef void (csAPIENTRY * csPFNGLNORMAL3FVERTEX3FVSUNPROC) (const GLfloat *n, const GLfloat *v);
typedef void (csAPIENTRY * csPFNGLCOLOR4FNORMAL3FVERTEX3FSUNPROC) (GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
typedef void (csAPIENTRY * csPFNGLCOLOR4FNORMAL3FVERTEX3FVSUNPROC) (const GLfloat *c, const GLfloat *n, const GLfloat *v);
typedef void (csAPIENTRY * csPFNGLTEXCOORD2FVERTEX3FSUNPROC) (GLfloat s, GLfloat t, GLfloat x, GLfloat y, GLfloat z);
typedef void (csAPIENTRY * csPFNGLTEXCOORD2FVERTEX3FVSUNPROC) (const GLfloat *tc, const GLfloat *v);
typedef void (csAPIENTRY * csPFNGLTEXCOORD4FVERTEX4FSUNPROC) (GLfloat s, GLfloat t, GLfloat p, GLfloat q, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (csAPIENTRY * csPFNGLTEXCOORD4FVERTEX4FVSUNPROC) (const GLfloat *tc, const GLfloat *v);
typedef void (csAPIENTRY * csPFNGLTEXCOORD2FCOLOR4UBVERTEX3FSUNPROC) (GLfloat s, GLfloat t, GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z);
typedef void (csAPIENTRY * csPFNGLTEXCOORD2FCOLOR4UBVERTEX3FVSUNPROC) (const GLfloat *tc, const GLubyte *c, const GLfloat *v);
typedef void (csAPIENTRY * csPFNGLTEXCOORD2FCOLOR3FVERTEX3FSUNPROC) (GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z);
typedef void (csAPIENTRY * csPFNGLTEXCOORD2FCOLOR3FVERTEX3FVSUNPROC) (const GLfloat *tc, const GLfloat *c, const GLfloat *v);
typedef void (csAPIENTRY * csPFNGLTEXCOORD2FNORMAL3FVERTEX3FSUNPROC) (GLfloat s, GLfloat t, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
typedef void (csAPIENTRY * csPFNGLTEXCOORD2FNORMAL3FVERTEX3FVSUNPROC) (const GLfloat *tc, const GLfloat *n, const GLfloat *v);
typedef void (csAPIENTRY * csPFNGLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUNPROC) (GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
typedef void (csAPIENTRY * csPFNGLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUNPROC) (const GLfloat *tc, const GLfloat *c, const GLfloat *n, const GLfloat *v);
typedef void (csAPIENTRY * csPFNGLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FSUNPROC) (GLfloat s, GLfloat t, GLfloat p, GLfloat q, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (csAPIENTRY * csPFNGLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FVSUNPROC) (const GLfloat *tc, const GLfloat *c, const GLfloat *n, const GLfloat *v);
typedef void (csAPIENTRY * csPFNGLREPLACEMENTCODEUIVERTEX3FSUNPROC) (GLenum rc, GLfloat x, GLfloat y, GLfloat z);
typedef void (csAPIENTRY * csPFNGLREPLACEMENTCODEUIVERTEX3FVSUNPROC) (const GLenum *rc, const GLfloat *v);
typedef void (csAPIENTRY * csPFNGLREPLACEMENTCODEUICOLOR4UBVERTEX3FSUNPROC) (GLenum rc, GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z);
typedef void (csAPIENTRY * csPFNGLREPLACEMENTCODEUICOLOR4UBVERTEX3FVSUNPROC) (const GLenum *rc, const GLubyte *c, const GLfloat *v);
typedef void (csAPIENTRY * csPFNGLREPLACEMENTCODEUICOLOR3FVERTEX3FSUNPROC) (GLenum rc, GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z);
typedef void (csAPIENTRY * csPFNGLREPLACEMENTCODEUICOLOR3FVERTEX3FVSUNPROC) (const GLenum *rc, const GLfloat *c, const GLfloat *v);
typedef void (csAPIENTRY * csPFNGLREPLACEMENTCODEUINORMAL3FVERTEX3FSUNPROC) (GLenum rc, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
typedef void (csAPIENTRY * csPFNGLREPLACEMENTCODEUINORMAL3FVERTEX3FVSUNPROC) (const GLenum *rc, const GLfloat *n, const GLfloat *v);
typedef void (csAPIENTRY * csPFNGLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FSUNPROC) (GLenum rc, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
typedef void (csAPIENTRY * csPFNGLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FVSUNPROC) (const GLenum *rc, const GLfloat *c, const GLfloat *n, const GLfloat *v);
typedef void (csAPIENTRY * csPFNGLREPLACEMENTCODEUITEXCOORD2FVERTEX3FSUNPROC) (GLenum rc, GLfloat s, GLfloat t, GLfloat x, GLfloat y, GLfloat z);
typedef void (csAPIENTRY * csPFNGLREPLACEMENTCODEUITEXCOORD2FVERTEX3FVSUNPROC) (const GLenum *rc, const GLfloat *tc, const GLfloat *v);
typedef void (csAPIENTRY * csPFNGLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FSUNPROC) (GLenum rc, GLfloat s, GLfloat t, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
typedef void (csAPIENTRY * csPFNGLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FVSUNPROC) (const GLenum *rc, const GLfloat *tc, const GLfloat *n, const GLfloat *v);
typedef void (csAPIENTRY * csPFNGLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUNPROC) (GLenum rc, GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
typedef void (csAPIENTRY * csPFNGLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUNPROC) (const GLenum *rc, const GLfloat *tc, const GLfloat *c, const GLfloat *n, const GLfloat *v);
typedef void (csAPIENTRY * csPFNGLBLENDFUNCSEPARATEEXTPROC) (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
typedef void (csAPIENTRY * csPFNGLVERTEXWEIGHTFEXTPROC) (GLfloat weight);
typedef void (csAPIENTRY * csPFNGLVERTEXWEIGHTFVEXTPROC) (const GLfloat *weight);
typedef void (csAPIENTRY * csPFNGLVERTEXWEIGHTPOINTEREXTPROC) (GLsizei size, GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void (csAPIENTRY * csPFNGLFLUSHVERTEXARRAYRANGENVPROC) (void);
typedef void (csAPIENTRY * csPFNGLVERTEXARRAYRANGENVPROC) (GLsizei size, const GLvoid *pointer);
typedef void (csAPIENTRY * csPFNGLCOMBINERPARAMETERFVNVPROC) (GLenum pname, const GLfloat *params);
typedef void (csAPIENTRY * csPFNGLCOMBINERPARAMETERFNVPROC) (GLenum pname, GLfloat param);
typedef void (csAPIENTRY * csPFNGLCOMBINERPARAMETERIVNVPROC) (GLenum pname, const GLint *params);
typedef void (csAPIENTRY * csPFNGLCOMBINERPARAMETERINVPROC) (GLenum pname, GLint param);
typedef void (csAPIENTRY * csPFNGLCOMBINERINPUTNVPROC) (GLenum stage, GLenum portion, GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage);
typedef void (csAPIENTRY * csPFNGLCOMBINEROUTPUTNVPROC) (GLenum stage, GLenum portion, GLenum abOutput, GLenum cdOutput, GLenum sumOutput, GLenum scale, GLenum bias, GLboolean abDotProduct, GLboolean cdDotProduct, GLboolean muxSum);
typedef void (csAPIENTRY * csPFNGLFINALCOMBINERINPUTNVPROC) (GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage);
typedef void (csAPIENTRY * csPFNGLGETCOMBINERINPUTPARAMETERFVNVPROC) (GLenum stage, GLenum portion, GLenum variable, GLenum pname, GLfloat *params);
typedef void (csAPIENTRY * csPFNGLGETCOMBINERINPUTPARAMETERIVNVPROC) (GLenum stage, GLenum portion, GLenum variable, GLenum pname, GLint *params);
typedef void (csAPIENTRY * csPFNGLGETCOMBINEROUTPUTPARAMETERFVNVPROC) (GLenum stage, GLenum portion, GLenum pname, GLfloat *params);
typedef void (csAPIENTRY * csPFNGLGETCOMBINEROUTPUTPARAMETERIVNVPROC) (GLenum stage, GLenum portion, GLenum pname, GLint *params);
typedef void (csAPIENTRY * csPFNGLGETFINALCOMBINERINPUTPARAMETERFVNVPROC) (GLenum variable, GLenum pname, GLfloat *params);
typedef void (csAPIENTRY * csPFNGLGETFINALCOMBINERINPUTPARAMETERIVNVPROC) (GLenum variable, GLenum pname, GLint *params);
typedef void (csAPIENTRY * csPFNGLRESIZEBUFFERSMESAPROC) (void);
typedef void (csAPIENTRY * csPFNGLWINDOWPOS2DMESAPROC) (GLdouble x, GLdouble y);
typedef void (csAPIENTRY * csPFNGLWINDOWPOS2DVMESAPROC) (const GLdouble *v);
typedef void (csAPIENTRY * csPFNGLWINDOWPOS2FMESAPROC) (GLfloat x, GLfloat y);
typedef void (csAPIENTRY * csPFNGLWINDOWPOS2FVMESAPROC) (const GLfloat *v);
typedef void (csAPIENTRY * csPFNGLWINDOWPOS2IMESAPROC) (GLint x, GLint y);
typedef void (csAPIENTRY * csPFNGLWINDOWPOS2IVMESAPROC) (const GLint *v);
typedef void (csAPIENTRY * csPFNGLWINDOWPOS2SMESAPROC) (GLshort x, GLshort y);
typedef void (csAPIENTRY * csPFNGLWINDOWPOS2SVMESAPROC) (const GLshort *v);
typedef void (csAPIENTRY * csPFNGLWINDOWPOS3DMESAPROC) (GLdouble x, GLdouble y, GLdouble z);
typedef void (csAPIENTRY * csPFNGLWINDOWPOS3DVMESAPROC) (const GLdouble *v);
typedef void (csAPIENTRY * csPFNGLWINDOWPOS3FMESAPROC) (GLfloat x, GLfloat y, GLfloat z);
typedef void (csAPIENTRY * csPFNGLWINDOWPOS3FVMESAPROC) (const GLfloat *v);
typedef void (csAPIENTRY * csPFNGLWINDOWPOS3IMESAPROC) (GLint x, GLint y, GLint z);
typedef void (csAPIENTRY * csPFNGLWINDOWPOS3IVMESAPROC) (const GLint *v);
typedef void (csAPIENTRY * csPFNGLWINDOWPOS3SMESAPROC) (GLshort x, GLshort y, GLshort z);
typedef void (csAPIENTRY * csPFNGLWINDOWPOS3SVMESAPROC) (const GLshort *v);
typedef void (csAPIENTRY * csPFNGLWINDOWPOS4DMESAPROC) (GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (csAPIENTRY * csPFNGLWINDOWPOS4DVMESAPROC) (const GLdouble *v);
typedef void (csAPIENTRY * csPFNGLWINDOWPOS4FMESAPROC) (GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (csAPIENTRY * csPFNGLWINDOWPOS4FVMESAPROC) (const GLfloat *v);
typedef void (csAPIENTRY * csPFNGLWINDOWPOS4IMESAPROC) (GLint x, GLint y, GLint z, GLint w);
typedef void (csAPIENTRY * csPFNGLWINDOWPOS4IVMESAPROC) (const GLint *v);
typedef void (csAPIENTRY * csPFNGLWINDOWPOS4SMESAPROC) (GLshort x, GLshort y, GLshort z, GLshort w);
typedef void (csAPIENTRY * csPFNGLWINDOWPOS4SVMESAPROC) (const GLshort *v);
typedef void (csAPIENTRY * csPFNGLMULTIMODEDRAWARRAYSIBMPROC) (GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount, GLint modestride);
typedef void (csAPIENTRY * csPFNGLMULTIMODEDRAWELEMENTSIBMPROC) (const GLenum *mode, const GLsizei *count, GLenum type, const GLvoid* *indices, GLsizei primcount, GLint modestride);
typedef void (csAPIENTRY * csPFNGLCOLORPOINTERLISTIBMPROC) (GLint size, GLenum type, GLint stride, const GLvoid* *pointer, GLint ptrstride);
typedef void (csAPIENTRY * csPFNGLSECONDARYCOLORPOINTERLISTIBMPROC) (GLint size, GLenum type, GLint stride, const GLvoid* *pointer, GLint ptrstride);
typedef void (csAPIENTRY * csPFNGLEDGEFLAGPOINTERLISTIBMPROC) (GLint stride, const GLboolean* *pointer, GLint ptrstride);
typedef void (csAPIENTRY * csPFNGLFOGCOORDPOINTERLISTIBMPROC) (GLenum type, GLint stride, const GLvoid* *pointer, GLint ptrstride);
typedef void (csAPIENTRY * csPFNGLINDEXPOINTERLISTIBMPROC) (GLenum type, GLint stride, const GLvoid* *pointer, GLint ptrstride);
typedef void (csAPIENTRY * csPFNGLNORMALPOINTERLISTIBMPROC) (GLenum type, GLint stride, const GLvoid* *pointer, GLint ptrstride);
typedef void (csAPIENTRY * csPFNGLTEXCOORDPOINTERLISTIBMPROC) (GLint size, GLenum type, GLint stride, const GLvoid* *pointer, GLint ptrstride);
typedef void (csAPIENTRY * csPFNGLVERTEXPOINTERLISTIBMPROC) (GLint size, GLenum type, GLint stride, const GLvoid* *pointer, GLint ptrstride);
typedef void (csAPIENTRY * csPFNGLTBUFFERMASK3DFXPROC) (GLuint mask);
typedef void (csAPIENTRY * csPFNGLSAMPLEMASKEXTPROC) (GLclampf value, GLboolean invert);
typedef void (csAPIENTRY * csPFNGLSAMPLEPATTERNEXTPROC) (GLenum pattern);
typedef void (csAPIENTRY * csPFNGLTEXTURECOLORMASKSGISPROC) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
#endif

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
#define CSGL_ARB_texture_env_combine
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
CS_PREP_GL_FUNCTION (csPFNGLBLENDCOLORPROC, glBlendColor);
CS_PREP_GL_FUNCTION (csPFNGLBLENDEQUATIONPROC, glBlendEquation);
CS_PREP_GL_FUNCTION (csPFNGLDRAWRANGEELEMENTSPROC, glDrawRangeElements);
CS_PREP_GL_FUNCTION (csPFNGLCOLORTABLEPROC, glColorTable);
CS_PREP_GL_FUNCTION (csPFNGLCOLORTABLEPARAMETERFVPROC, glColorTableParameterfv);
CS_PREP_GL_FUNCTION (csPFNGLCOLORTABLEPARAMETERIVPROC, glColorTableParameteriv);
CS_PREP_GL_FUNCTION (csPFNGLCOPYCOLORTABLEPROC, glCopyColorTable);
CS_PREP_GL_FUNCTION (csPFNGLGETCOLORTABLEPROC, glGetColorTable);
CS_PREP_GL_FUNCTION (csPFNGLGETCOLORTABLEPARAMETERFVPROC, glGetColorTableParameterfv);
CS_PREP_GL_FUNCTION (csPFNGLGETCOLORTABLEPARAMETERIVPROC, glGetColorTableParameteriv);
CS_PREP_GL_FUNCTION (csPFNGLCOLORSUBTABLEPROC, glColorSubTable);
CS_PREP_GL_FUNCTION (csPFNGLCOPYCOLORSUBTABLEPROC, glCopyColorSubTable);
CS_PREP_GL_FUNCTION (csPFNGLCONVOLUTIONFILTER1DPROC, glConvolutionFilter1D);
CS_PREP_GL_FUNCTION (csPFNGLCONVOLUTIONFILTER2DPROC, glConvolutionFilter2D);
CS_PREP_GL_FUNCTION (csPFNGLCONVOLUTIONPARAMETERFPROC, glConvolutionParameterf);
CS_PREP_GL_FUNCTION (csPFNGLCONVOLUTIONPARAMETERFVPROC, glConvolutionParameterfv);
CS_PREP_GL_FUNCTION (csPFNGLCONVOLUTIONPARAMETERIPROC, glConvolutionParameteri);
CS_PREP_GL_FUNCTION (csPFNGLCONVOLUTIONPARAMETERIVPROC, glConvolutionParameteriv);
CS_PREP_GL_FUNCTION (csPFNGLCOPYCONVOLUTIONFILTER1DPROC, glCopyConvolutionFilter1D);
CS_PREP_GL_FUNCTION (csPFNGLCOPYCONVOLUTIONFILTER2DPROC, glCopyConvolutionFilter2D);
CS_PREP_GL_FUNCTION (csPFNGLGETCONVOLUTIONFILTERPROC, glGetConvolutionFilter);
CS_PREP_GL_FUNCTION (csPFNGLGETCONVOLUTIONPARAMETERFVPROC, glGetConvolutionParameterfv);
CS_PREP_GL_FUNCTION (csPFNGLGETCONVOLUTIONPARAMETERIVPROC, glGetConvolutionParameteriv);
CS_PREP_GL_FUNCTION (csPFNGLGETSEPARABLEFILTERPROC, glGetSeparableFilter);
CS_PREP_GL_FUNCTION (csPFNGLSEPARABLEFILTER2DPROC, glSeparableFilter2D);
CS_PREP_GL_FUNCTION (csPFNGLGETHISTOGRAMPROC, glGetHistogram);
CS_PREP_GL_FUNCTION (csPFNGLGETHISTOGRAMPARAMETERFVPROC, glGetHistogramParameterfv);
CS_PREP_GL_FUNCTION (csPFNGLGETHISTOGRAMPARAMETERIVPROC, glGetHistogramParameteriv);
CS_PREP_GL_FUNCTION (csPFNGLGETMINMAXPROC, glGetMinmax);
CS_PREP_GL_FUNCTION (csPFNGLGETMINMAXPARAMETERFVPROC, glGetMinmaxParameterfv);
CS_PREP_GL_FUNCTION (csPFNGLGETMINMAXPARAMETERIVPROC, glGetMinmaxParameteriv);
CS_PREP_GL_FUNCTION (csPFNGLHISTOGRAMPROC, glHistogram);
CS_PREP_GL_FUNCTION (csPFNGLMINMAXPROC, glMinmax);
CS_PREP_GL_FUNCTION (csPFNGLRESETHISTOGRAMPROC, glResetHistogram);
CS_PREP_GL_FUNCTION (csPFNGLRESETMINMAXPROC, glResetMinmax);
CS_PREP_GL_FUNCTION (csPFNGLTEXIMAGE3DPROC, glTexImage3D);
CS_PREP_GL_FUNCTION (csPFNGLTEXSUBIMAGE3DPROC, glTexSubImage3D);
CS_PREP_GL_FUNCTION (csPFNGLCOPYTEXSUBIMAGE3DPROC, glCopyTexSubImage3D);
#endif

#ifdef CSGL_ARB_multitexture
CS_PREP_GL_FUNCTION (csPFNGLACTIVETEXTUREARBPROC, glActiveTextureARB);
CS_PREP_GL_FUNCTION (csPFNGLCLIENTACTIVETEXTUREARBPROC, glClientActiveTextureARB);
CS_PREP_GL_FUNCTION (csPFNGLMULTITEXCOORD1DARBPROC, glMultiTexCoord1dARB);
CS_PREP_GL_FUNCTION (csPFNGLMULTITEXCOORD1DVARBPROC, glMultiTexCoord1dvARB);
CS_PREP_GL_FUNCTION (csPFNGLMULTITEXCOORD1FARBPROC, glMultiTexCoord1fARB);
CS_PREP_GL_FUNCTION (csPFNGLMULTITEXCOORD1FVARBPROC, glMultiTexCoord1fvARB);
CS_PREP_GL_FUNCTION (csPFNGLMULTITEXCOORD1IARBPROC, glMultiTexCoord1iARB);
CS_PREP_GL_FUNCTION (csPFNGLMULTITEXCOORD1IVARBPROC, glMultiTexCoord1ivARB);
CS_PREP_GL_FUNCTION (csPFNGLMULTITEXCOORD1SARBPROC, glMultiTexCoord1sARB);
CS_PREP_GL_FUNCTION (csPFNGLMULTITEXCOORD1SVARBPROC, glMultiTexCoord1svARB);
CS_PREP_GL_FUNCTION (csPFNGLMULTITEXCOORD2DARBPROC, glMultiTexCoord2dARB);
CS_PREP_GL_FUNCTION (csPFNGLMULTITEXCOORD2DVARBPROC, glMultiTexCoord2dvARB);
CS_PREP_GL_FUNCTION (csPFNGLMULTITEXCOORD2FARBPROC, glMultiTexCoord2fARB);
CS_PREP_GL_FUNCTION (csPFNGLMULTITEXCOORD2FVARBPROC, glMultiTexCoord2fvARB);
CS_PREP_GL_FUNCTION (csPFNGLMULTITEXCOORD2IARBPROC, glMultiTexCoord2iARB);
CS_PREP_GL_FUNCTION (csPFNGLMULTITEXCOORD2IVARBPROC, glMultiTexCoord2ivARB);
CS_PREP_GL_FUNCTION (csPFNGLMULTITEXCOORD2SARBPROC, glMultiTexCoord2sARB);
CS_PREP_GL_FUNCTION (csPFNGLMULTITEXCOORD2SVARBPROC, glMultiTexCoord2svARB);
CS_PREP_GL_FUNCTION (csPFNGLMULTITEXCOORD3DARBPROC, glMultiTexCoord3dARB);
CS_PREP_GL_FUNCTION (csPFNGLMULTITEXCOORD3DVARBPROC, glMultiTexCoord3dvARB);
CS_PREP_GL_FUNCTION (csPFNGLMULTITEXCOORD3FARBPROC, glMultiTexCoord3fARB);
CS_PREP_GL_FUNCTION (csPFNGLMULTITEXCOORD3FVARBPROC, glMultiTexCoord3fvARB);
CS_PREP_GL_FUNCTION (csPFNGLMULTITEXCOORD3IARBPROC, glMultiTexCoord3iARB);
CS_PREP_GL_FUNCTION (csPFNGLMULTITEXCOORD3IVARBPROC, glMultiTexCoord3ivARB);
CS_PREP_GL_FUNCTION (csPFNGLMULTITEXCOORD3SARBPROC, glMultiTexCoord3sARB);
CS_PREP_GL_FUNCTION (csPFNGLMULTITEXCOORD3SVARBPROC, glMultiTexCoord3svARB);
CS_PREP_GL_FUNCTION (csPFNGLMULTITEXCOORD4DARBPROC, glMultiTexCoord4dARB);
CS_PREP_GL_FUNCTION (csPFNGLMULTITEXCOORD4DVARBPROC, glMultiTexCoord4dvARB);
CS_PREP_GL_FUNCTION (csPFNGLMULTITEXCOORD4FARBPROC, glMultiTexCoord4fARB);
CS_PREP_GL_FUNCTION (csPFNGLMULTITEXCOORD4FVARBPROC, glMultiTexCoord4fvARB);
CS_PREP_GL_FUNCTION (csPFNGLMULTITEXCOORD4IARBPROC, glMultiTexCoord4iARB);
CS_PREP_GL_FUNCTION (csPFNGLMULTITEXCOORD4IVARBPROC, glMultiTexCoord4ivARB);
CS_PREP_GL_FUNCTION (csPFNGLMULTITEXCOORD4SARBPROC, glMultiTexCoord4sARB);
CS_PREP_GL_FUNCTION (csPFNGLMULTITEXCOORD4SVARBPROC, glMultiTexCoord4svARB);
#endif

#ifdef CSGL_ARB_transpose_matrix
CS_PREP_GL_FUNCTION (csPFNGLLOADTRANSPOSEMATRIXFARBPROC, glLoadTransposeMatrixfARB);
CS_PREP_GL_FUNCTION (csPFNGLLOADTRANSPOSEMATRIXDARBPROC, glLoadTransposeMatrixdARB);
CS_PREP_GL_FUNCTION (csPFNGLMULTTRANSPOSEMATRIXFARBPROC, glMultTransposeMatrixfARB);
CS_PREP_GL_FUNCTION (csPFNGLMULTTRANSPOSEMATRIXDARBPROC, glMultTransposeMatrixdARB);
#endif

#ifdef CSGL_ARB_multisample
CS_PREP_GL_FUNCTION (csPFNGLSAMPLECOVERAGEARBPROC, glSampleCoverageARB);
CS_PREP_GL_FUNCTION (csPFNGLSAMPLEPASSARBPROC, glSamplePassARB);
#endif

#ifdef CSGL_ARB_texture_env_add
#endif

#ifdef CSGL_ARB_texture_cube_map
#endif

#ifdef CSGL_ARB_texture_compression
CS_PREP_GL_FUNCTION (csPFNGLCOMPRESSEDTEXIMAGE3DARBPROC, glCompressedTexImage3DARB);
CS_PREP_GL_FUNCTION (csPFNGLCOMPRESSEDTEXIMAGE2DARBPROC, glCompressedTexImage2DARB);
CS_PREP_GL_FUNCTION (csPFNGLCOMPRESSEDTEXIMAGE1DARBPROC, glCompressedTexImage1DARB);
CS_PREP_GL_FUNCTION (csPFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC, glCompressedTexSubImage3DARB);
CS_PREP_GL_FUNCTION (csPFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC, glCompressedTexSubImage2DARB);
CS_PREP_GL_FUNCTION (csPFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC, glCompressedTexSubImage1DARB);
CS_PREP_GL_FUNCTION (csPFNGLGETCOMPRESSEDTEXIMAGEARBPROC, glGetCompressedTexImageARB);
#endif

#ifdef CSGL_EXT_abgr
#endif

#ifdef CSGL_EXT_blend_color
CS_PREP_GL_FUNCTION (csPFNGLBLENDCOLOREXTPROC, glBlendColorEXT);
#endif

#ifdef CSGL_EXT_polygon_offset
CS_PREP_GL_FUNCTION (csPFNGLPOLYGONOFFSETEXTPROC, glPolygonOffsetEXT);
#endif

#ifdef CSGL_EXT_texture
#endif

#ifdef CSGL_EXT_texture3D
CS_PREP_GL_FUNCTION (csPFNGLTEXIMAGE3DEXTPROC, glTexImage3DEXT);
#endif

#ifdef CSGL_EXT_subtexture
CS_PREP_GL_FUNCTION (csPFNGLTEXSUBIMAGE3DEXTPROC, glTexSubImage3DEXT);
#endif

#ifdef CSGL_SGIS_texture_filter4
CS_PREP_GL_FUNCTION (csPFNGLGETTEXFILTERFUNCSGISPROC, glGetTexFilterFuncSGIS);
CS_PREP_GL_FUNCTION (csPFNGLTEXFILTERFUNCSGISPROC, glTexFilterFuncSGIS);
#endif

#ifdef CSGL_EXT_subtexture
CS_PREP_GL_FUNCTION (csPFNGLTEXSUBIMAGE1DEXTPROC, glTexSubImage1DEXT);
CS_PREP_GL_FUNCTION (csPFNGLTEXSUBIMAGE2DEXTPROC, glTexSubImage2DEXT);
#endif

#ifdef CSGL_EXT_copy_texture
CS_PREP_GL_FUNCTION (csPFNGLCOPYTEXIMAGE1DEXTPROC, glCopyTexImage1DEXT);
CS_PREP_GL_FUNCTION (csPFNGLCOPYTEXIMAGE2DEXTPROC, glCopyTexImage2DEXT);
CS_PREP_GL_FUNCTION (csPFNGLCOPYTEXSUBIMAGE1DEXTPROC, glCopyTexSubImage1DEXT);
CS_PREP_GL_FUNCTION (csPFNGLCOPYTEXSUBIMAGE2DEXTPROC, glCopyTexSubImage2DEXT);
CS_PREP_GL_FUNCTION (csPFNGLCOPYTEXSUBIMAGE3DEXTPROC, glCopyTexSubImage3DEXT);
#endif

#ifdef CSGL_EXT_histogram
CS_PREP_GL_FUNCTION (csPFNGLGETHISTOGRAMEXTPROC, glGetHistogramEXT);
CS_PREP_GL_FUNCTION (csPFNGLGETHISTOGRAMPARAMETERFVEXTPROC, glGetHistogramParameterfvEXT);
CS_PREP_GL_FUNCTION (csPFNGLGETHISTOGRAMPARAMETERIVEXTPROC, glGetHistogramParameterivEXT);
CS_PREP_GL_FUNCTION (csPFNGLGETMINMAXEXTPROC, glGetMinmaxEXT);
CS_PREP_GL_FUNCTION (csPFNGLGETMINMAXPARAMETERFVEXTPROC, glGetMinmaxParameterfvEXT);
CS_PREP_GL_FUNCTION (csPFNGLGETMINMAXPARAMETERIVEXTPROC, glGetMinmaxParameterivEXT);
CS_PREP_GL_FUNCTION (csPFNGLHISTOGRAMEXTPROC, glHistogramEXT);
CS_PREP_GL_FUNCTION (csPFNGLMINMAXEXTPROC, glMinmaxEXT);
CS_PREP_GL_FUNCTION (csPFNGLRESETHISTOGRAMEXTPROC, glResetHistogramEXT);
CS_PREP_GL_FUNCTION (csPFNGLRESETMINMAXEXTPROC, glResetMinmaxEXT);
#endif

#ifdef CSGL_EXT_convolution
CS_PREP_GL_FUNCTION (csPFNGLCONVOLUTIONFILTER1DEXTPROC, glConvolutionFilter1DEXT);
CS_PREP_GL_FUNCTION (csPFNGLCONVOLUTIONFILTER2DEXTPROC, glConvolutionFilter2DEXT);
CS_PREP_GL_FUNCTION (csPFNGLCONVOLUTIONPARAMETERFEXTPROC, glConvolutionParameterfEXT);
CS_PREP_GL_FUNCTION (csPFNGLCONVOLUTIONPARAMETERFVEXTPROC, glConvolutionParameterfvEXT);
CS_PREP_GL_FUNCTION (csPFNGLCONVOLUTIONPARAMETERIEXTPROC, glConvolutionParameteriEXT);
CS_PREP_GL_FUNCTION (csPFNGLCONVOLUTIONPARAMETERIVEXTPROC, glConvolutionParameterivEXT);
CS_PREP_GL_FUNCTION (csPFNGLCOPYCONVOLUTIONFILTER1DEXTPROC, glCopyConvolutionFilter1DEXT);
CS_PREP_GL_FUNCTION (csPFNGLCOPYCONVOLUTIONFILTER2DEXTPROC, glCopyConvolutionFilter2DEXT);
CS_PREP_GL_FUNCTION (csPFNGLGETCONVOLUTIONFILTEREXTPROC, glGetConvolutionFilterEXT);
CS_PREP_GL_FUNCTION (csPFNGLGETCONVOLUTIONPARAMETERFVEXTPROC, glGetConvolutionParameterfvEXT);
CS_PREP_GL_FUNCTION (csPFNGLGETCONVOLUTIONPARAMETERIVEXTPROC, glGetConvolutionParameterivEXT);
CS_PREP_GL_FUNCTION (csPFNGLGETSEPARABLEFILTEREXTPROC, glGetSeparableFilterEXT);
CS_PREP_GL_FUNCTION (csPFNGLSEPARABLEFILTER2DEXTPROC, glSeparableFilter2DEXT);
#endif

#ifdef CSGL_EXT_color_matrix
#endif

#ifdef CSGL_SGI_color_table
CS_PREP_GL_FUNCTION (csPFNGLCOLORTABLESGIPROC, glColorTableSGI);
CS_PREP_GL_FUNCTION (csPFNGLCOLORTABLEPARAMETERFVSGIPROC, glColorTableParameterfvSGI);
CS_PREP_GL_FUNCTION (csPFNGLCOLORTABLEPARAMETERIVSGIPROC, glColorTableParameterivSGI);
CS_PREP_GL_FUNCTION (csPFNGLCOPYCOLORTABLESGIPROC, glCopyColorTableSGI);
CS_PREP_GL_FUNCTION (csPFNGLGETCOLORTABLESGIPROC, glGetColorTableSGI);
CS_PREP_GL_FUNCTION (csPFNGLGETCOLORTABLEPARAMETERFVSGIPROC, glGetColorTableParameterfvSGI);
CS_PREP_GL_FUNCTION (csPFNGLGETCOLORTABLEPARAMETERIVSGIPROC, glGetColorTableParameterivSGI);
#endif

#ifdef CSGL_SGIX_pixel_texture
CS_PREP_GL_FUNCTION (csPFNGLPIXELTEXGENSGIXPROC, glPixelTexGenSGIX);
#endif

#ifdef CSGL_SGIS_pixel_texture
CS_PREP_GL_FUNCTION (csPFNGLPIXELTEXGENPARAMETERISGISPROC, glPixelTexGenParameteriSGIS);
CS_PREP_GL_FUNCTION (csPFNGLPIXELTEXGENPARAMETERIVSGISPROC, glPixelTexGenParameterivSGIS);
CS_PREP_GL_FUNCTION (csPFNGLPIXELTEXGENPARAMETERFSGISPROC, glPixelTexGenParameterfSGIS);
CS_PREP_GL_FUNCTION (csPFNGLPIXELTEXGENPARAMETERFVSGISPROC, glPixelTexGenParameterfvSGIS);
CS_PREP_GL_FUNCTION (csPFNGLGETPIXELTEXGENPARAMETERIVSGISPROC, glGetPixelTexGenParameterivSGIS);
CS_PREP_GL_FUNCTION (csPFNGLGETPIXELTEXGENPARAMETERFVSGISPROC, glGetPixelTexGenParameterfvSGIS);
#endif

#ifdef CSGL_SGIS_texture4D
CS_PREP_GL_FUNCTION (csPFNGLTEXIMAGE4DSGISPROC, glTexImage4DSGIS);
CS_PREP_GL_FUNCTION (csPFNGLTEXSUBIMAGE4DSGISPROC, glTexSubImage4DSGIS);
#endif

#ifdef CSGL_SGI_texture_color_table
#endif

#ifdef CSGL_EXT_cmyka
#endif

#ifdef CSGL_EXT_texture_object
CS_PREP_GL_FUNCTION (csPFNGLARETEXTURESRESIDENTEXTPROC, glAreTexturesResidentEXT);
CS_PREP_GL_FUNCTION (csPFNGLBINDTEXTUREEXTPROC, glBindTextureEXT);
CS_PREP_GL_FUNCTION (csPFNGLDELETETEXTURESEXTPROC, glDeleteTexturesEXT);
CS_PREP_GL_FUNCTION (csPFNGLGENTEXTURESEXTPROC, glGenTexturesEXT);
CS_PREP_GL_FUNCTION (csPFNGLISTEXTUREEXTPROC, glIsTextureEXT);
CS_PREP_GL_FUNCTION (csPFNGLPRIORITIZETEXTURESEXTPROC, glPrioritizeTexturesEXT);
#endif

#ifdef CSGL_SGIS_detail_texture
CS_PREP_GL_FUNCTION (csPFNGLDETAILTEXFUNCSGISPROC, glDetailTexFuncSGIS);
CS_PREP_GL_FUNCTION (csPFNGLGETDETAILTEXFUNCSGISPROC, glGetDetailTexFuncSGIS);
#endif

#ifdef CSGL_SGIS_sharpen_texture
CS_PREP_GL_FUNCTION (csPFNGLSHARPENTEXFUNCSGISPROC, glSharpenTexFuncSGIS);
CS_PREP_GL_FUNCTION (csPFNGLGETSHARPENTEXFUNCSGISPROC, glGetSharpenTexFuncSGIS);
#endif

#ifdef CSGL_EXT_packed_pixels
#endif

#ifdef CSGL_SGIS_texture_lod
#endif

#ifdef CSGL_SGIS_multisample
CS_PREP_GL_FUNCTION (csPFNGLSAMPLEMASKSGISPROC, glSampleMaskSGIS);
CS_PREP_GL_FUNCTION (csPFNGLSAMPLEPATTERNSGISPROC, glSamplePatternSGIS);
#endif

#ifdef CSGL_EXT_rescale_normal
#endif

#ifdef CSGL_EXT_vertex_array
CS_PREP_GL_FUNCTION (csPFNGLARRAYELEMENTEXTPROC, glArrayElementEXT);
CS_PREP_GL_FUNCTION (csPFNGLCOLORPOINTEREXTPROC, glColorPointerEXT);
CS_PREP_GL_FUNCTION (csPFNGLDRAWARRAYSEXTPROC, glDrawArraysEXT);
CS_PREP_GL_FUNCTION (csPFNGLEDGEFLAGPOINTEREXTPROC, glEdgeFlagPointerEXT);
CS_PREP_GL_FUNCTION (csPFNGLGETPOINTERVEXTPROC, glGetPointervEXT);
CS_PREP_GL_FUNCTION (csPFNGLINDEXPOINTEREXTPROC, glIndexPointerEXT);
CS_PREP_GL_FUNCTION (csPFNGLNORMALPOINTEREXTPROC, glNormalPointerEXT);
CS_PREP_GL_FUNCTION (csPFNGLTEXCOORDPOINTEREXTPROC, glTexCoordPointerEXT);
CS_PREP_GL_FUNCTION (csPFNGLVERTEXPOINTEREXTPROC, glVertexPointerEXT);
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
CS_PREP_GL_FUNCTION (csPFNGLBLENDEQUATIONEXTPROC, glBlendEquationEXT);
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
CS_PREP_GL_FUNCTION (csPFNGLSPRITEPARAMETERFSGIXPROC, glSpriteParameterfSGIX);
CS_PREP_GL_FUNCTION (csPFNGLSPRITEPARAMETERFVSGIXPROC, glSpriteParameterfvSGIX);
CS_PREP_GL_FUNCTION (csPFNGLSPRITEPARAMETERISGIXPROC, glSpriteParameteriSGIX);
CS_PREP_GL_FUNCTION (csPFNGLSPRITEPARAMETERIVSGIXPROC, glSpriteParameterivSGIX);
#endif

#ifdef CSGL_SGIX_texture_multi_buffer
#endif

#ifdef CSGL_EXT_point_parameters
CS_PREP_GL_FUNCTION (csPFNGLPOINTPARAMETERFEXTPROC, glPointParameterfEXT);
CS_PREP_GL_FUNCTION (csPFNGLPOINTPARAMETERFVEXTPROC, glPointParameterfvEXT);
CS_PREP_GL_FUNCTION (csPFNGLPOINTPARAMETERFSGISPROC, glPointParameterfSGIS);
CS_PREP_GL_FUNCTION (csPFNGLPOINTPARAMETERFVSGISPROC, glPointParameterfvSGIS);
#endif

#ifdef CSGL_SGIX_instruments
CS_PREP_GL_FUNCTION (csPFNGLGETINSTRUMENTSSGIXPROC, glGetInstrumentsSGIX);
CS_PREP_GL_FUNCTION (csPFNGLINSTRUMENTSBUFFERSGIXPROC, glInstrumentsBufferSGIX);
CS_PREP_GL_FUNCTION (csPFNGLPOLLINSTRUMENTSSGIXPROC, glPollInstrumentsSGIX);
CS_PREP_GL_FUNCTION (csPFNGLREADINSTRUMENTSSGIXPROC, glReadInstrumentsSGIX);
CS_PREP_GL_FUNCTION (csPFNGLSTARTINSTRUMENTSSGIXPROC, glStartInstrumentsSGIX);
CS_PREP_GL_FUNCTION (csPFNGLSTOPINSTRUMENTSSGIXPROC, glStopInstrumentsSGIX);
#endif

#ifdef CSGL_SGIX_texture_scale_bias
#endif

#ifdef CSGL_SGIX_framezoom
CS_PREP_GL_FUNCTION (csPFNGLFRAMEZOOMSGIXPROC, glFrameZoomSGIX);
#endif

#ifdef CSGL_SGIX_tag_sample_buffer
CS_PREP_GL_FUNCTION (csPFNGLTAGSAMPLEBUFFERSGIXPROC, glTagSampleBufferSGIX);
#endif

#ifdef CSGL_SGIX_reference_plane
CS_PREP_GL_FUNCTION (csPFNGLREFERENCEPLANESGIXPROC, glReferencePlaneSGIX);
#endif

#ifdef CSGL_SGIX_flush_raster
CS_PREP_GL_FUNCTION (csPFNGLFLUSHRASTERSGIXPROC, glFlushRasterSGIX);
#endif

#ifdef CSGL_SGIX_depth_texture
#endif

#ifdef CSGL_SGIS_fog_function
CS_PREP_GL_FUNCTION (csPFNGLFOGFUNCSGISPROC, glFogFuncSGIS);
CS_PREP_GL_FUNCTION (csPFNGLGETFOGFUNCSGISPROC, glGetFogFuncSGIS);
#endif

#ifdef CSGL_SGIX_fog_offset
#endif

#ifdef CSGL_HP_image_transform
CS_PREP_GL_FUNCTION (csPFNGLIMAGETRANSFORMPARAMETERIHPPROC, glImageTransformParameteriHP);
CS_PREP_GL_FUNCTION (csPFNGLIMAGETRANSFORMPARAMETERFHPPROC, glImageTransformParameterfHP);
CS_PREP_GL_FUNCTION (csPFNGLIMAGETRANSFORMPARAMETERIVHPPROC, glImageTransformParameterivHP);
CS_PREP_GL_FUNCTION (csPFNGLIMAGETRANSFORMPARAMETERFVHPPROC, glImageTransformParameterfvHP);
CS_PREP_GL_FUNCTION (csPFNGLGETIMAGETRANSFORMPARAMETERIVHPPROC, glGetImageTransformParameterivHP);
CS_PREP_GL_FUNCTION (csPFNGLGETIMAGETRANSFORMPARAMETERFVHPPROC, glGetImageTransformParameterfvHP);
#endif

#ifdef CSGL_HP_convolution_border_modes
#endif

#ifdef CSGL_SGIX_texture_add_env
#endif

#ifdef CSGL_EXT_color_subtable
CS_PREP_GL_FUNCTION (csPFNGLCOLORSUBTABLEEXTPROC, glColorSubTableEXT);
CS_PREP_GL_FUNCTION (csPFNGLCOPYCOLORSUBTABLEEXTPROC, glCopyColorSubTableEXT);
#endif

#ifdef CSGL_PGI_vertex_hints
#endif

#ifdef CSGL_PGI_misc_hints
CS_PREP_GL_FUNCTION (csPFNGLHINTPGIPROC, glHintPGI);
#endif

#ifdef CSGL_EXT_paletted_texture
CS_PREP_GL_FUNCTION (csPFNGLCOLORTABLEEXTPROC, glColorTableEXT);
CS_PREP_GL_FUNCTION (csPFNGLGETCOLORTABLEEXTPROC, glGetColorTableEXT);
CS_PREP_GL_FUNCTION (csPFNGLGETCOLORTABLEPARAMETERIVEXTPROC, glGetColorTableParameterivEXT);
CS_PREP_GL_FUNCTION (csPFNGLGETCOLORTABLEPARAMETERFVEXTPROC, glGetColorTableParameterfvEXT);
#endif

#ifdef CSGL_EXT_clip_volume_hint
#endif

#ifdef CSGL_SGIX_list_priority
CS_PREP_GL_FUNCTION (csPFNGLGETLISTPARAMETERFVSGIXPROC, glGetListParameterfvSGIX);
CS_PREP_GL_FUNCTION (csPFNGLGETLISTPARAMETERIVSGIXPROC, glGetListParameterivSGIX);
CS_PREP_GL_FUNCTION (csPFNGLLISTPARAMETERFSGIXPROC, glListParameterfSGIX);
CS_PREP_GL_FUNCTION (csPFNGLLISTPARAMETERFVSGIXPROC, glListParameterfvSGIX);
CS_PREP_GL_FUNCTION (csPFNGLLISTPARAMETERISGIXPROC, glListParameteriSGIX);
CS_PREP_GL_FUNCTION (csPFNGLLISTPARAMETERIVSGIXPROC, glListParameterivSGIX);
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
CS_PREP_GL_FUNCTION (csPFNGLINDEXMATERIALEXTPROC, glIndexMaterialEXT);
#endif

#ifdef CSGL_EXT_index_func
CS_PREP_GL_FUNCTION (csPFNGLINDEXFUNCEXTPROC, glIndexFuncEXT);
#endif

#ifdef CSGL_EXT_index_array_formats
#endif

#ifdef CSGL_EXT_compiled_vertex_array
CS_PREP_GL_FUNCTION (csPFNGLLOCKARRAYSEXTPROC, glLockArraysEXT);
CS_PREP_GL_FUNCTION (csPFNGLUNLOCKARRAYSEXTPROC, glUnlockArraysEXT);
#endif

#ifdef CSGL_EXT_cull_vertex
CS_PREP_GL_FUNCTION (csPFNGLCULLPARAMETERDVEXTPROC, glCullParameterdvEXT);
CS_PREP_GL_FUNCTION (csPFNGLCULLPARAMETERFVEXTPROC, glCullParameterfvEXT);
#endif

#ifdef CSGL_SGIX_ycrcb
#endif

#ifdef CSGL_SGIX_fragment_lighting
CS_PREP_GL_FUNCTION (csPFNGLFRAGMENTCOLORMATERIALSGIXPROC, glFragmentColorMaterialSGIX);
CS_PREP_GL_FUNCTION (csPFNGLFRAGMENTLIGHTFSGIXPROC, glFragmentLightfSGIX);
CS_PREP_GL_FUNCTION (csPFNGLFRAGMENTLIGHTFVSGIXPROC, glFragmentLightfvSGIX);
CS_PREP_GL_FUNCTION (csPFNGLFRAGMENTLIGHTISGIXPROC, glFragmentLightiSGIX);
CS_PREP_GL_FUNCTION (csPFNGLFRAGMENTLIGHTIVSGIXPROC, glFragmentLightivSGIX);
CS_PREP_GL_FUNCTION (csPFNGLFRAGMENTLIGHTMODELFSGIXPROC, glFragmentLightModelfSGIX);
CS_PREP_GL_FUNCTION (csPFNGLFRAGMENTLIGHTMODELFVSGIXPROC, glFragmentLightModelfvSGIX);
CS_PREP_GL_FUNCTION (csPFNGLFRAGMENTLIGHTMODELISGIXPROC, glFragmentLightModeliSGIX);
CS_PREP_GL_FUNCTION (csPFNGLFRAGMENTLIGHTMODELIVSGIXPROC, glFragmentLightModelivSGIX);
CS_PREP_GL_FUNCTION (csPFNGLFRAGMENTMATERIALFSGIXPROC, glFragmentMaterialfSGIX);
CS_PREP_GL_FUNCTION (csPFNGLFRAGMENTMATERIALFVSGIXPROC, glFragmentMaterialfvSGIX);
CS_PREP_GL_FUNCTION (csPFNGLFRAGMENTMATERIALISGIXPROC, glFragmentMaterialiSGIX);
CS_PREP_GL_FUNCTION (csPFNGLFRAGMENTMATERIALIVSGIXPROC, glFragmentMaterialivSGIX);
CS_PREP_GL_FUNCTION (csPFNGLGETFRAGMENTLIGHTFVSGIXPROC, glGetFragmentLightfvSGIX);
CS_PREP_GL_FUNCTION (csPFNGLGETFRAGMENTLIGHTIVSGIXPROC, glGetFragmentLightivSGIX);
CS_PREP_GL_FUNCTION (csPFNGLGETFRAGMENTMATERIALFVSGIXPROC, glGetFragmentMaterialfvSGIX);
CS_PREP_GL_FUNCTION (csPFNGLGETFRAGMENTMATERIALIVSGIXPROC, glGetFragmentMaterialivSGIX);
CS_PREP_GL_FUNCTION (csPFNGLLIGHTENVISGIXPROC, glLightEnviSGIX);
#endif

#ifdef CSGL_IBM_rasterpos_clip
#endif

#ifdef CSGL_HP_texture_lighting
#endif

#ifdef CSGL_EXT_draw_range_elements
CS_PREP_GL_FUNCTION (csPFNGLDRAWRANGEELEMENTSEXTPROC, glDrawRangeElementsEXT);
#endif

#ifdef CSGL_WIN_phong_shading
#endif

#ifdef CSGL_WIN_specular_fog
#endif

#ifdef CSGL_EXT_light_texture
CS_PREP_GL_FUNCTION (csPFNGLAPPLYTEXTUREEXTPROC, glApplyTextureEXT);
CS_PREP_GL_FUNCTION (csPFNGLTEXTURELIGHTEXTPROC, glTextureLightEXT);
CS_PREP_GL_FUNCTION (csPFNGLTEXTUREMATERIALEXTPROC, glTextureMaterialEXT);
#endif

#ifdef CSGL_SGIX_blend_alpha_minmax
#endif

#ifdef CSGL_EXT_bgra
#endif

#ifdef CSGL_INTEL_parallel_arrays
CS_PREP_GL_FUNCTION (csPFNGLVERTEXPOINTERVINTELPROC, glVertexPointervINTEL);
CS_PREP_GL_FUNCTION (csPFNGLNORMALPOINTERVINTELPROC, glNormalPointervINTEL);
CS_PREP_GL_FUNCTION (csPFNGLCOLORPOINTERVINTELPROC, glColorPointervINTEL);
CS_PREP_GL_FUNCTION (csPFNGLTEXCOORDPOINTERVINTELPROC, glTexCoordPointervINTEL);
#endif

#ifdef CSGL_HP_occlusion_test
#endif

#ifdef CSGL_EXT_pixel_transform
CS_PREP_GL_FUNCTION (csPFNGLPIXELTRANSFORMPARAMETERIEXTPROC, glPixelTransformParameteriEXT);
CS_PREP_GL_FUNCTION (csPFNGLPIXELTRANSFORMPARAMETERFEXTPROC, glPixelTransformParameterfEXT);
CS_PREP_GL_FUNCTION (csPFNGLPIXELTRANSFORMPARAMETERIVEXTPROC, glPixelTransformParameterivEXT);
CS_PREP_GL_FUNCTION (csPFNGLPIXELTRANSFORMPARAMETERFVEXTPROC, glPixelTransformParameterfvEXT);
#endif

#ifdef CSGL_EXT_pixel_transform_color_table
#endif

#ifdef CSGL_EXT_shared_texture_palette
#endif

#ifdef CSGL_EXT_separate_specular_color
#endif

#ifdef CSGL_EXT_secondary_color
CS_PREP_GL_FUNCTION (csPFNGLSECONDARYCOLOR3BEXTPROC, glSecondaryColor3bEXT);
CS_PREP_GL_FUNCTION (csPFNGLSECONDARYCOLOR3BVEXTPROC, glSecondaryColor3bvEXT);
CS_PREP_GL_FUNCTION (csPFNGLSECONDARYCOLOR3DEXTPROC, glSecondaryColor3dEXT);
CS_PREP_GL_FUNCTION (csPFNGLSECONDARYCOLOR3DVEXTPROC, glSecondaryColor3dvEXT);
CS_PREP_GL_FUNCTION (csPFNGLSECONDARYCOLOR3FEXTPROC, glSecondaryColor3fEXT);
CS_PREP_GL_FUNCTION (csPFNGLSECONDARYCOLOR3FVEXTPROC, glSecondaryColor3fvEXT);
CS_PREP_GL_FUNCTION (csPFNGLSECONDARYCOLOR3IEXTPROC, glSecondaryColor3iEXT);
CS_PREP_GL_FUNCTION (csPFNGLSECONDARYCOLOR3IVEXTPROC, glSecondaryColor3ivEXT);
CS_PREP_GL_FUNCTION (csPFNGLSECONDARYCOLOR3SEXTPROC, glSecondaryColor3sEXT);
CS_PREP_GL_FUNCTION (csPFNGLSECONDARYCOLOR3SVEXTPROC, glSecondaryColor3svEXT);
CS_PREP_GL_FUNCTION (csPFNGLSECONDARYCOLOR3UBEXTPROC, glSecondaryColor3ubEXT);
CS_PREP_GL_FUNCTION (csPFNGLSECONDARYCOLOR3UBVEXTPROC, glSecondaryColor3ubvEXT);
CS_PREP_GL_FUNCTION (csPFNGLSECONDARYCOLOR3UIEXTPROC, glSecondaryColor3uiEXT);
CS_PREP_GL_FUNCTION (csPFNGLSECONDARYCOLOR3UIVEXTPROC, glSecondaryColor3uivEXT);
CS_PREP_GL_FUNCTION (csPFNGLSECONDARYCOLOR3USEXTPROC, glSecondaryColor3usEXT);
CS_PREP_GL_FUNCTION (csPFNGLSECONDARYCOLOR3USVEXTPROC, glSecondaryColor3usvEXT);
CS_PREP_GL_FUNCTION (csPFNGLSECONDARYCOLORPOINTEREXTPROC, glSecondaryColorPointerEXT);
#endif

#ifdef CSGL_EXT_texture_perturb_normal
CS_PREP_GL_FUNCTION (csPFNGLTEXTURENORMALEXTPROC, glTextureNormalEXT);
#endif

#ifdef CSGL_EXT_multi_draw_arrays
CS_PREP_GL_FUNCTION (csPFNGLMULTIDRAWARRAYSEXTPROC, glMultiDrawArraysEXT);
CS_PREP_GL_FUNCTION (csPFNGLMULTIDRAWELEMENTSEXTPROC, glMultiDrawElementsEXT);
#endif

#ifdef CSGL_EXT_fog_coord
CS_PREP_GL_FUNCTION (csPFNGLFOGCOORDFEXTPROC, glFogCoordfEXT);
CS_PREP_GL_FUNCTION (csPFNGLFOGCOORDFVEXTPROC, glFogCoordfvEXT);
CS_PREP_GL_FUNCTION (csPFNGLFOGCOORDDEXTPROC, glFogCoorddEXT);
CS_PREP_GL_FUNCTION (csPFNGLFOGCOORDDVEXTPROC, glFogCoorddvEXT);
CS_PREP_GL_FUNCTION (csPFNGLFOGCOORDPOINTEREXTPROC, glFogCoordPointerEXT);
#endif

#ifdef CSGL_REND_screen_coordinates
#endif

#ifdef CSGL_EXT_coordinate_frame
CS_PREP_GL_FUNCTION (csPFNGLTANGENT3BEXTPROC, glTangent3bEXT);
CS_PREP_GL_FUNCTION (csPFNGLTANGENT3BVEXTPROC, glTangent3bvEXT);
CS_PREP_GL_FUNCTION (csPFNGLTANGENT3DEXTPROC, glTangent3dEXT);
CS_PREP_GL_FUNCTION (csPFNGLTANGENT3DVEXTPROC, glTangent3dvEXT);
CS_PREP_GL_FUNCTION (csPFNGLTANGENT3FEXTPROC, glTangent3fEXT);
CS_PREP_GL_FUNCTION (csPFNGLTANGENT3FVEXTPROC, glTangent3fvEXT);
CS_PREP_GL_FUNCTION (csPFNGLTANGENT3IEXTPROC, glTangent3iEXT);
CS_PREP_GL_FUNCTION (csPFNGLTANGENT3IVEXTPROC, glTangent3ivEXT);
CS_PREP_GL_FUNCTION (csPFNGLTANGENT3SEXTPROC, glTangent3sEXT);
CS_PREP_GL_FUNCTION (csPFNGLTANGENT3SVEXTPROC, glTangent3svEXT);
CS_PREP_GL_FUNCTION (csPFNGLBINORMAL3BEXTPROC, glBinormal3bEXT);
CS_PREP_GL_FUNCTION (csPFNGLBINORMAL3BVEXTPROC, glBinormal3bvEXT);
CS_PREP_GL_FUNCTION (csPFNGLBINORMAL3DEXTPROC, glBinormal3dEXT);
CS_PREP_GL_FUNCTION (csPFNGLBINORMAL3DVEXTPROC, glBinormal3dvEXT);
CS_PREP_GL_FUNCTION (csPFNGLBINORMAL3FEXTPROC, glBinormal3fEXT);
CS_PREP_GL_FUNCTION (csPFNGLBINORMAL3FVEXTPROC, glBinormal3fvEXT);
CS_PREP_GL_FUNCTION (csPFNGLBINORMAL3IEXTPROC, glBinormal3iEXT);
CS_PREP_GL_FUNCTION (csPFNGLBINORMAL3IVEXTPROC, glBinormal3ivEXT);
CS_PREP_GL_FUNCTION (csPFNGLBINORMAL3SEXTPROC, glBinormal3sEXT);
CS_PREP_GL_FUNCTION (csPFNGLBINORMAL3SVEXTPROC, glBinormal3svEXT);
CS_PREP_GL_FUNCTION (csPFNGLTANGENTPOINTEREXTPROC, glTangentPointerEXT);
CS_PREP_GL_FUNCTION (csPFNGLBINORMALPOINTEREXTPROC, glBinormalPointerEXT);
#endif

#ifdef CSGL_EXT_texture_env_combine
#endif

#ifdef CSGL_ARB_texture_env_combine
#endif

#ifdef CSGL_APPLE_specular_vector
#endif

#ifdef CSGL_APPLE_transform_hint
#endif

#ifdef CSGL_SGIX_fog_scale
#endif

#ifdef CSGL_SUNX_constant_data
CS_PREP_GL_FUNCTION (csPFNGLFINISHTEXTURESUNXPROC, glFinishTextureSUNX);
#endif

#ifdef CSGL_SUN_global_alpha
CS_PREP_GL_FUNCTION (csPFNGLGLOBALALPHAFACTORBSUNPROC, glGlobalAlphaFactorbSUN);
CS_PREP_GL_FUNCTION (csPFNGLGLOBALALPHAFACTORSSUNPROC, glGlobalAlphaFactorsSUN);
CS_PREP_GL_FUNCTION (csPFNGLGLOBALALPHAFACTORISUNPROC, glGlobalAlphaFactoriSUN);
CS_PREP_GL_FUNCTION (csPFNGLGLOBALALPHAFACTORFSUNPROC, glGlobalAlphaFactorfSUN);
CS_PREP_GL_FUNCTION (csPFNGLGLOBALALPHAFACTORDSUNPROC, glGlobalAlphaFactordSUN);
CS_PREP_GL_FUNCTION (csPFNGLGLOBALALPHAFACTORUBSUNPROC, glGlobalAlphaFactorubSUN);
CS_PREP_GL_FUNCTION (csPFNGLGLOBALALPHAFACTORUSSUNPROC, glGlobalAlphaFactorusSUN);
CS_PREP_GL_FUNCTION (csPFNGLGLOBALALPHAFACTORUISUNPROC, glGlobalAlphaFactoruiSUN);
#endif

#ifdef CSGL_SUN_triangle_list
CS_PREP_GL_FUNCTION (csPFNGLREPLACEMENTCODEUISUNPROC, glReplacementCodeuiSUN);
CS_PREP_GL_FUNCTION (csPFNGLREPLACEMENTCODEUSSUNPROC, glReplacementCodeusSUN);
CS_PREP_GL_FUNCTION (csPFNGLREPLACEMENTCODEUBSUNPROC, glReplacementCodeubSUN);
CS_PREP_GL_FUNCTION (csPFNGLREPLACEMENTCODEUIVSUNPROC, glReplacementCodeuivSUN);
CS_PREP_GL_FUNCTION (csPFNGLREPLACEMENTCODEUSVSUNPROC, glReplacementCodeusvSUN);
CS_PREP_GL_FUNCTION (csPFNGLREPLACEMENTCODEUBVSUNPROC, glReplacementCodeubvSUN);
CS_PREP_GL_FUNCTION (csPFNGLREPLACEMENTCODEPOINTERSUNPROC, glReplacementCodePointerSUN);
#endif

#ifdef CSGL_SUN_vertex
CS_PREP_GL_FUNCTION (csPFNGLCOLOR4UBVERTEX2FSUNPROC, glColor4ubVertex2fSUN);
CS_PREP_GL_FUNCTION (csPFNGLCOLOR4UBVERTEX2FVSUNPROC, glColor4ubVertex2fvSUN);
CS_PREP_GL_FUNCTION (csPFNGLCOLOR4UBVERTEX3FSUNPROC, glColor4ubVertex3fSUN);
CS_PREP_GL_FUNCTION (csPFNGLCOLOR4UBVERTEX3FVSUNPROC, glColor4ubVertex3fvSUN);
CS_PREP_GL_FUNCTION (csPFNGLCOLOR3FVERTEX3FSUNPROC, glColor3fVertex3fSUN);
CS_PREP_GL_FUNCTION (csPFNGLCOLOR3FVERTEX3FVSUNPROC, glColor3fVertex3fvSUN);
CS_PREP_GL_FUNCTION (csPFNGLNORMAL3FVERTEX3FSUNPROC, glNormal3fVertex3fSUN);
CS_PREP_GL_FUNCTION (csPFNGLNORMAL3FVERTEX3FVSUNPROC, glNormal3fVertex3fvSUN);
CS_PREP_GL_FUNCTION (csPFNGLCOLOR4FNORMAL3FVERTEX3FSUNPROC, glColor4fNormal3fVertex3fSUN);
CS_PREP_GL_FUNCTION (csPFNGLCOLOR4FNORMAL3FVERTEX3FVSUNPROC, glColor4fNormal3fVertex3fvSUN);
CS_PREP_GL_FUNCTION (csPFNGLTEXCOORD2FVERTEX3FSUNPROC, glTexCoord2fVertex3fSUN);
CS_PREP_GL_FUNCTION (csPFNGLTEXCOORD2FVERTEX3FVSUNPROC, glTexCoord2fVertex3fvSUN);
CS_PREP_GL_FUNCTION (csPFNGLTEXCOORD4FVERTEX4FSUNPROC, glTexCoord4fVertex4fSUN);
CS_PREP_GL_FUNCTION (csPFNGLTEXCOORD4FVERTEX4FVSUNPROC, glTexCoord4fVertex4fvSUN);
CS_PREP_GL_FUNCTION (csPFNGLTEXCOORD2FCOLOR4UBVERTEX3FSUNPROC, glTexCoord2fColor4ubVertex3fSUN);
CS_PREP_GL_FUNCTION (csPFNGLTEXCOORD2FCOLOR4UBVERTEX3FVSUNPROC, glTexCoord2fColor4ubVertex3fvSUN);
CS_PREP_GL_FUNCTION (csPFNGLTEXCOORD2FCOLOR3FVERTEX3FSUNPROC, glTexCoord2fColor3fVertex3fSUN);
CS_PREP_GL_FUNCTION (csPFNGLTEXCOORD2FCOLOR3FVERTEX3FVSUNPROC, glTexCoord2fColor3fVertex3fvSUN);
CS_PREP_GL_FUNCTION (csPFNGLTEXCOORD2FNORMAL3FVERTEX3FSUNPROC, glTexCoord2fNormal3fVertex3fSUN);
CS_PREP_GL_FUNCTION (csPFNGLTEXCOORD2FNORMAL3FVERTEX3FVSUNPROC, glTexCoord2fNormal3fVertex3fvSUN);
CS_PREP_GL_FUNCTION (csPFNGLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUNPROC, glTexCoord2fColor4fNormal3fVertex3fSUN);
CS_PREP_GL_FUNCTION (csPFNGLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUNPROC, glTexCoord2fColor4fNormal3fVertex3fvSUN);
CS_PREP_GL_FUNCTION (csPFNGLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FSUNPROC, glTexCoord4fColor4fNormal3fVertex4fSUN);
CS_PREP_GL_FUNCTION (csPFNGLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FVSUNPROC, glTexCoord4fColor4fNormal3fVertex4fvSUN);
CS_PREP_GL_FUNCTION (csPFNGLREPLACEMENTCODEUIVERTEX3FSUNPROC, glReplacementCodeuiVertex3fSUN);
CS_PREP_GL_FUNCTION (csPFNGLREPLACEMENTCODEUIVERTEX3FVSUNPROC, glReplacementCodeuiVertex3fvSUN);
CS_PREP_GL_FUNCTION (csPFNGLREPLACEMENTCODEUICOLOR4UBVERTEX3FSUNPROC, glReplacementCodeuiColor4ubVertex3fSUN);
CS_PREP_GL_FUNCTION (csPFNGLREPLACEMENTCODEUICOLOR4UBVERTEX3FVSUNPROC, glReplacementCodeuiColor4ubVertex3fvSUN);
CS_PREP_GL_FUNCTION (csPFNGLREPLACEMENTCODEUICOLOR3FVERTEX3FSUNPROC, glReplacementCodeuiColor3fVertex3fSUN);
CS_PREP_GL_FUNCTION (csPFNGLREPLACEMENTCODEUICOLOR3FVERTEX3FVSUNPROC, glReplacementCodeuiColor3fVertex3fvSUN);
CS_PREP_GL_FUNCTION (csPFNGLREPLACEMENTCODEUINORMAL3FVERTEX3FSUNPROC, glReplacementCodeuiNormal3fVertex3fSUN);
CS_PREP_GL_FUNCTION (csPFNGLREPLACEMENTCODEUINORMAL3FVERTEX3FVSUNPROC, glReplacementCodeuiNormal3fVertex3fvSUN);
CS_PREP_GL_FUNCTION (csPFNGLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FSUNPROC, glReplacementCodeuiColor4fNormal3fVertex3fSUN);
CS_PREP_GL_FUNCTION (csPFNGLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FVSUNPROC, glReplacementCodeuiColor4fNormal3fVertex3fvSUN);
CS_PREP_GL_FUNCTION (csPFNGLREPLACEMENTCODEUITEXCOORD2FVERTEX3FSUNPROC, glReplacementCodeuiTexCoord2fVertex3fSUN);
CS_PREP_GL_FUNCTION (csPFNGLREPLACEMENTCODEUITEXCOORD2FVERTEX3FVSUNPROC, glReplacementCodeuiTexCoord2fVertex3fvSUN);
CS_PREP_GL_FUNCTION (csPFNGLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FSUNPROC, glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN);
CS_PREP_GL_FUNCTION (csPFNGLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FVSUNPROC, glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN);
CS_PREP_GL_FUNCTION (csPFNGLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUNPROC, glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN);
CS_PREP_GL_FUNCTION (csPFNGLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUNPROC, glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN);
#endif

#ifdef CSGL_EXT_blend_func_separate
CS_PREP_GL_FUNCTION (csPFNGLBLENDFUNCSEPARATEEXTPROC, glBlendFuncSeparateEXT);
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
CS_PREP_GL_FUNCTION (csPFNGLVERTEXWEIGHTFEXTPROC, glVertexWeightfEXT);
CS_PREP_GL_FUNCTION (csPFNGLVERTEXWEIGHTFVEXTPROC, glVertexWeightfvEXT);
CS_PREP_GL_FUNCTION (csPFNGLVERTEXWEIGHTPOINTEREXTPROC, glVertexWeightPointerEXT);
#endif

#ifdef CSGL_NV_light_max_exponent
#endif

#ifdef CSGL_NV_vertex_array_range
CS_PREP_GL_FUNCTION (csPFNGLFLUSHVERTEXARRAYRANGENVPROC, glFlushVertexArrayRangeNV);
CS_PREP_GL_FUNCTION (csPFNGLVERTEXARRAYRANGENVPROC, glVertexArrayRangeNV);
#endif

#ifdef CSGL_NV_register_combiners
CS_PREP_GL_FUNCTION (csPFNGLCOMBINERPARAMETERFVNVPROC, glCombinerParameterfvNV);
CS_PREP_GL_FUNCTION (csPFNGLCOMBINERPARAMETERFNVPROC, glCombinerParameterfNV);
CS_PREP_GL_FUNCTION (csPFNGLCOMBINERPARAMETERIVNVPROC, glCombinerParameterivNV);
CS_PREP_GL_FUNCTION (csPFNGLCOMBINERPARAMETERINVPROC, glCombinerParameteriNV);
CS_PREP_GL_FUNCTION (csPFNGLCOMBINERINPUTNVPROC, glCombinerInputNV);
CS_PREP_GL_FUNCTION (csPFNGLCOMBINEROUTPUTNVPROC, glCombinerOutputNV);
CS_PREP_GL_FUNCTION (csPFNGLFINALCOMBINERINPUTNVPROC, glFinalCombinerInputNV);
CS_PREP_GL_FUNCTION (csPFNGLGETCOMBINERINPUTPARAMETERFVNVPROC, glGetCombinerInputParameterfvNV);
CS_PREP_GL_FUNCTION (csPFNGLGETCOMBINERINPUTPARAMETERIVNVPROC, glGetCombinerInputParameterivNV);
CS_PREP_GL_FUNCTION (csPFNGLGETCOMBINEROUTPUTPARAMETERFVNVPROC, glGetCombinerOutputParameterfvNV);
CS_PREP_GL_FUNCTION (csPFNGLGETCOMBINEROUTPUTPARAMETERIVNVPROC, glGetCombinerOutputParameterivNV);
CS_PREP_GL_FUNCTION (csPFNGLGETFINALCOMBINERINPUTPARAMETERFVNVPROC, glGetFinalCombinerInputParameterfvNV);
CS_PREP_GL_FUNCTION (csPFNGLGETFINALCOMBINERINPUTPARAMETERIVNVPROC, glGetFinalCombinerInputParameterivNV);
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
CS_PREP_GL_FUNCTION (csPFNGLRESIZEBUFFERSMESAPROC, glResizeBuffersMESA);
#endif

#ifdef CSGL_MESA_window_pos
CS_PREP_GL_FUNCTION (csPFNGLWINDOWPOS2DMESAPROC, glWindowPos2dMESA);
CS_PREP_GL_FUNCTION (csPFNGLWINDOWPOS2DVMESAPROC, glWindowPos2dvMESA);
CS_PREP_GL_FUNCTION (csPFNGLWINDOWPOS2FMESAPROC, glWindowPos2fMESA);
CS_PREP_GL_FUNCTION (csPFNGLWINDOWPOS2FVMESAPROC, glWindowPos2fvMESA);
CS_PREP_GL_FUNCTION (csPFNGLWINDOWPOS2IMESAPROC, glWindowPos2iMESA);
CS_PREP_GL_FUNCTION (csPFNGLWINDOWPOS2IVMESAPROC, glWindowPos2ivMESA);
CS_PREP_GL_FUNCTION (csPFNGLWINDOWPOS2SMESAPROC, glWindowPos2sMESA);
CS_PREP_GL_FUNCTION (csPFNGLWINDOWPOS2SVMESAPROC, glWindowPos2svMESA);
CS_PREP_GL_FUNCTION (csPFNGLWINDOWPOS3DMESAPROC, glWindowPos3dMESA);
CS_PREP_GL_FUNCTION (csPFNGLWINDOWPOS3DVMESAPROC, glWindowPos3dvMESA);
CS_PREP_GL_FUNCTION (csPFNGLWINDOWPOS3FMESAPROC, glWindowPos3fMESA);
CS_PREP_GL_FUNCTION (csPFNGLWINDOWPOS3FVMESAPROC, glWindowPos3fvMESA);
CS_PREP_GL_FUNCTION (csPFNGLWINDOWPOS3IMESAPROC, glWindowPos3iMESA);
CS_PREP_GL_FUNCTION (csPFNGLWINDOWPOS3IVMESAPROC, glWindowPos3ivMESA);
CS_PREP_GL_FUNCTION (csPFNGLWINDOWPOS3SMESAPROC, glWindowPos3sMESA);
CS_PREP_GL_FUNCTION (csPFNGLWINDOWPOS3SVMESAPROC, glWindowPos3svMESA);
CS_PREP_GL_FUNCTION (csPFNGLWINDOWPOS4DMESAPROC, glWindowPos4dMESA);
CS_PREP_GL_FUNCTION (csPFNGLWINDOWPOS4DVMESAPROC, glWindowPos4dvMESA);
CS_PREP_GL_FUNCTION (csPFNGLWINDOWPOS4FMESAPROC, glWindowPos4fMESA);
CS_PREP_GL_FUNCTION (csPFNGLWINDOWPOS4FVMESAPROC, glWindowPos4fvMESA);
CS_PREP_GL_FUNCTION (csPFNGLWINDOWPOS4IMESAPROC, glWindowPos4iMESA);
CS_PREP_GL_FUNCTION (csPFNGLWINDOWPOS4IVMESAPROC, glWindowPos4ivMESA);
CS_PREP_GL_FUNCTION (csPFNGLWINDOWPOS4SMESAPROC, glWindowPos4sMESA);
CS_PREP_GL_FUNCTION (csPFNGLWINDOWPOS4SVMESAPROC, glWindowPos4svMESA);
#endif

#ifdef CSGL_IBM_cull_vertex
#endif

#ifdef CSGL_IBM_multimode_draw_arrays
CS_PREP_GL_FUNCTION (csPFNGLMULTIMODEDRAWARRAYSIBMPROC, glMultiModeDrawArraysIBM);
CS_PREP_GL_FUNCTION (csPFNGLMULTIMODEDRAWELEMENTSIBMPROC, glMultiModeDrawElementsIBM);
#endif

#ifdef CSGL_IBM_vertex_array_lists
CS_PREP_GL_FUNCTION (csPFNGLCOLORPOINTERLISTIBMPROC, glColorPointerListIBM);
CS_PREP_GL_FUNCTION (csPFNGLSECONDARYCOLORPOINTERLISTIBMPROC, glSecondaryColorPointerListIBM);
CS_PREP_GL_FUNCTION (csPFNGLEDGEFLAGPOINTERLISTIBMPROC, glEdgeFlagPointerListIBM);
CS_PREP_GL_FUNCTION (csPFNGLFOGCOORDPOINTERLISTIBMPROC, glFogCoordPointerListIBM);
CS_PREP_GL_FUNCTION (csPFNGLINDEXPOINTERLISTIBMPROC, glIndexPointerListIBM);
CS_PREP_GL_FUNCTION (csPFNGLNORMALPOINTERLISTIBMPROC, glNormalPointerListIBM);
CS_PREP_GL_FUNCTION (csPFNGLTEXCOORDPOINTERLISTIBMPROC, glTexCoordPointerListIBM);
CS_PREP_GL_FUNCTION (csPFNGLVERTEXPOINTERLISTIBMPROC, glVertexPointerListIBM);
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
CS_PREP_GL_FUNCTION (csPFNGLTBUFFERMASK3DFXPROC, glTbufferMask3DFX);
#endif

#ifdef CSGL_EXT_multisample
CS_PREP_GL_FUNCTION (csPFNGLSAMPLEMASKEXTPROC, glSampleMaskEXT);
CS_PREP_GL_FUNCTION (csPFNGLSAMPLEPATTERNEXTPROC, glSamplePatternEXT);
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
CS_PREP_GL_FUNCTION (csPFNGLTEXTURECOLORMASKSGISPROC, glTextureColorMaskSGIS);
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
#undef CSGL_ARB_texture_env_combine
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

#ifndef GL_CONSTANT_COLOR                 
#define GL_CONSTANT_COLOR                 (GLenum)0x8001
#endif
#ifndef GL_ONE_MINUS_CONSTANT_COLOR       
#define GL_ONE_MINUS_CONSTANT_COLOR       (GLenum)0x8002
#endif
#ifndef GL_CONSTANT_ALPHA                 
#define GL_CONSTANT_ALPHA                 (GLenum)0x8003
#endif
#ifndef GL_ONE_MINUS_CONSTANT_ALPHA       
#define GL_ONE_MINUS_CONSTANT_ALPHA       (GLenum)0x8004
#endif
#ifndef GL_BLEND_COLOR                    
#define GL_BLEND_COLOR                    (GLenum)0x8005
#endif
#ifndef GL_FUNC_ADD                       
#define GL_FUNC_ADD                       (GLenum)0x8006
#endif
#ifndef GL_MIN                            
#define GL_MIN                            (GLenum)0x8007
#endif
#ifndef GL_MAX                            
#define GL_MAX                            (GLenum)0x8008
#endif
#ifndef GL_BLEND_EQUATION                 
#define GL_BLEND_EQUATION                 (GLenum)0x8009
#endif
#ifndef GL_FUNC_SUBTRACT                  
#define GL_FUNC_SUBTRACT                  (GLenum)0x800A
#endif
#ifndef GL_FUNC_REVERSE_SUBTRACT          
#define GL_FUNC_REVERSE_SUBTRACT          (GLenum)0x800B
#endif
#ifndef GL_CONVOLUTION_1D                 
#define GL_CONVOLUTION_1D                 (GLenum)0x8010
#endif
#ifndef GL_CONVOLUTION_2D                 
#define GL_CONVOLUTION_2D                 (GLenum)0x8011
#endif
#ifndef GL_SEPARABLE_2D                   
#define GL_SEPARABLE_2D                   (GLenum)0x8012
#endif
#ifndef GL_CONVOLUTION_BORDER_MODE        
#define GL_CONVOLUTION_BORDER_MODE        (GLenum)0x8013
#endif
#ifndef GL_CONVOLUTION_FILTER_SCALE       
#define GL_CONVOLUTION_FILTER_SCALE       (GLenum)0x8014
#endif
#ifndef GL_CONVOLUTION_FILTER_BIAS        
#define GL_CONVOLUTION_FILTER_BIAS        (GLenum)0x8015
#endif
#ifndef GL_REDUCE                         
#define GL_REDUCE                         (GLenum)0x8016
#endif
#ifndef GL_CONVOLUTION_FORMAT             
#define GL_CONVOLUTION_FORMAT             (GLenum)0x8017
#endif
#ifndef GL_CONVOLUTION_WIDTH              
#define GL_CONVOLUTION_WIDTH              (GLenum)0x8018
#endif
#ifndef GL_CONVOLUTION_HEIGHT             
#define GL_CONVOLUTION_HEIGHT             (GLenum)0x8019
#endif
#ifndef GL_MAX_CONVOLUTION_WIDTH          
#define GL_MAX_CONVOLUTION_WIDTH          (GLenum)0x801A
#endif
#ifndef GL_MAX_CONVOLUTION_HEIGHT         
#define GL_MAX_CONVOLUTION_HEIGHT         (GLenum)0x801B
#endif
#ifndef GL_POST_CONVOLUTION_RED_SCALE     
#define GL_POST_CONVOLUTION_RED_SCALE     (GLenum)0x801C
#endif
#ifndef GL_POST_CONVOLUTION_GREEN_SCALE   
#define GL_POST_CONVOLUTION_GREEN_SCALE   (GLenum)0x801D
#endif
#ifndef GL_POST_CONVOLUTION_BLUE_SCALE    
#define GL_POST_CONVOLUTION_BLUE_SCALE    (GLenum)0x801E
#endif
#ifndef GL_POST_CONVOLUTION_ALPHA_SCALE   
#define GL_POST_CONVOLUTION_ALPHA_SCALE   (GLenum)0x801F
#endif
#ifndef GL_POST_CONVOLUTION_RED_BIAS      
#define GL_POST_CONVOLUTION_RED_BIAS      (GLenum)0x8020
#endif
#ifndef GL_POST_CONVOLUTION_GREEN_BIAS    
#define GL_POST_CONVOLUTION_GREEN_BIAS    (GLenum)0x8021
#endif
#ifndef GL_POST_CONVOLUTION_BLUE_BIAS     
#define GL_POST_CONVOLUTION_BLUE_BIAS     (GLenum)0x8022
#endif
#ifndef GL_POST_CONVOLUTION_ALPHA_BIAS    
#define GL_POST_CONVOLUTION_ALPHA_BIAS    (GLenum)0x8023
#endif
#ifndef GL_HISTOGRAM                      
#define GL_HISTOGRAM                      (GLenum)0x8024
#endif
#ifndef GL_PROXY_HISTOGRAM                
#define GL_PROXY_HISTOGRAM                (GLenum)0x8025
#endif
#ifndef GL_HISTOGRAM_WIDTH                
#define GL_HISTOGRAM_WIDTH                (GLenum)0x8026
#endif
#ifndef GL_HISTOGRAM_FORMAT               
#define GL_HISTOGRAM_FORMAT               (GLenum)0x8027
#endif
#ifndef GL_HISTOGRAM_RED_SIZE             
#define GL_HISTOGRAM_RED_SIZE             (GLenum)0x8028
#endif
#ifndef GL_HISTOGRAM_GREEN_SIZE           
#define GL_HISTOGRAM_GREEN_SIZE           (GLenum)0x8029
#endif
#ifndef GL_HISTOGRAM_BLUE_SIZE            
#define GL_HISTOGRAM_BLUE_SIZE            (GLenum)0x802A
#endif
#ifndef GL_HISTOGRAM_ALPHA_SIZE           
#define GL_HISTOGRAM_ALPHA_SIZE           (GLenum)0x802B
#endif
#ifndef GL_HISTOGRAM_LUMINANCE_SIZE       
#define GL_HISTOGRAM_LUMINANCE_SIZE       (GLenum)0x802C
#endif
#ifndef GL_HISTOGRAM_SINK                 
#define GL_HISTOGRAM_SINK                 (GLenum)0x802D
#endif
#ifndef GL_MINMAX                         
#define GL_MINMAX                         (GLenum)0x802E
#endif
#ifndef GL_MINMAX_FORMAT                  
#define GL_MINMAX_FORMAT                  (GLenum)0x802F
#endif
#ifndef GL_MINMAX_SINK                    
#define GL_MINMAX_SINK                    (GLenum)0x8030
#endif
#ifndef GL_TABLE_TOO_LARGE                
#define GL_TABLE_TOO_LARGE                (GLenum)0x8031
#endif
#ifndef GL_UNSIGNED_BYTE_3_3_2            
#define GL_UNSIGNED_BYTE_3_3_2            (GLenum)0x8032
#endif
#ifndef GL_UNSIGNED_SHORT_4_4_4_4         
#define GL_UNSIGNED_SHORT_4_4_4_4         (GLenum)0x8033
#endif
#ifndef GL_UNSIGNED_SHORT_5_5_5_1         
#define GL_UNSIGNED_SHORT_5_5_5_1         (GLenum)0x8034
#endif
#ifndef GL_UNSIGNED_INT_8_8_8_8           
#define GL_UNSIGNED_INT_8_8_8_8           (GLenum)0x8035
#endif
#ifndef GL_UNSIGNED_INT_10_10_10_2        
#define GL_UNSIGNED_INT_10_10_10_2        (GLenum)0x8036
#endif
#ifndef GL_RESCALE_NORMAL                 
#define GL_RESCALE_NORMAL                 (GLenum)0x803A
#endif
#ifndef GL_UNSIGNED_BYTE_2_3_3_REV        
#define GL_UNSIGNED_BYTE_2_3_3_REV        (GLenum)0x8362
#endif
#ifndef GL_UNSIGNED_SHORT_5_6_5           
#define GL_UNSIGNED_SHORT_5_6_5           (GLenum)0x8363
#endif
#ifndef GL_UNSIGNED_SHORT_5_6_5_REV       
#define GL_UNSIGNED_SHORT_5_6_5_REV       (GLenum)0x8364
#endif
#ifndef GL_UNSIGNED_SHORT_4_4_4_4_REV     
#define GL_UNSIGNED_SHORT_4_4_4_4_REV     (GLenum)0x8365
#endif
#ifndef GL_UNSIGNED_SHORT_1_5_5_5_REV     
#define GL_UNSIGNED_SHORT_1_5_5_5_REV     (GLenum)0x8366
#endif
#ifndef GL_UNSIGNED_INT_8_8_8_8_REV       
#define GL_UNSIGNED_INT_8_8_8_8_REV       (GLenum)0x8367
#endif
#ifndef GL_UNSIGNED_INT_2_10_10_10_REV    
#define GL_UNSIGNED_INT_2_10_10_10_REV    (GLenum)0x8368
#endif
#ifndef GL_COLOR_MATRIX                   
#define GL_COLOR_MATRIX                   (GLenum)0x80B1
#endif
#ifndef GL_COLOR_MATRIX_STACK_DEPTH       
#define GL_COLOR_MATRIX_STACK_DEPTH       (GLenum)0x80B2
#endif
#ifndef GL_MAX_COLOR_MATRIX_STACK_DEPTH   
#define GL_MAX_COLOR_MATRIX_STACK_DEPTH   (GLenum)0x80B3
#endif
#ifndef GL_POST_COLOR_MATRIX_RED_SCALE    
#define GL_POST_COLOR_MATRIX_RED_SCALE    (GLenum)0x80B4
#endif
#ifndef GL_POST_COLOR_MATRIX_GREEN_SCALE  
#define GL_POST_COLOR_MATRIX_GREEN_SCALE  (GLenum)0x80B5
#endif
#ifndef GL_POST_COLOR_MATRIX_BLUE_SCALE   
#define GL_POST_COLOR_MATRIX_BLUE_SCALE   (GLenum)0x80B6
#endif
#ifndef GL_POST_COLOR_MATRIX_ALPHA_SCALE  
#define GL_POST_COLOR_MATRIX_ALPHA_SCALE  (GLenum)0x80B7
#endif
#ifndef GL_POST_COLOR_MATRIX_RED_BIAS     
#define GL_POST_COLOR_MATRIX_RED_BIAS     (GLenum)0x80B8
#endif
#ifndef GL_POST_COLOR_MATRIX_GREEN_BIAS   
#define GL_POST_COLOR_MATRIX_GREEN_BIAS   (GLenum)0x80B9
#endif
#ifndef GL_POST_COLOR_MATRIX_BLUE_BIAS    
#define GL_POST_COLOR_MATRIX_BLUE_BIAS    (GLenum)0x80BA
#endif
#ifndef GL_COLOR_TABLE                    
#define GL_COLOR_TABLE                    (GLenum)0x80D0
#endif
#ifndef GL_POST_CONVOLUTION_COLOR_TABLE   
#define GL_POST_CONVOLUTION_COLOR_TABLE   (GLenum)0x80D1
#endif
#ifndef GL_POST_COLOR_MATRIX_COLOR_TABLE  
#define GL_POST_COLOR_MATRIX_COLOR_TABLE  (GLenum)0x80D2
#endif
#ifndef GL_PROXY_COLOR_TABLE              
#define GL_PROXY_COLOR_TABLE              (GLenum)0x80D3
#endif
#ifndef GL_PROXY_POST_CONVOLUTION_COLOR_TABLE 
#define GL_PROXY_POST_CONVOLUTION_COLOR_TABLE (GLenum)0x80D4
#endif
#ifndef GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE 
#define GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE (GLenum)0x80D5
#endif
#ifndef GL_COLOR_TABLE_SCALE              
#define GL_COLOR_TABLE_SCALE              (GLenum)0x80D6
#endif
#ifndef GL_COLOR_TABLE_BIAS               
#define GL_COLOR_TABLE_BIAS               (GLenum)0x80D7
#endif
#ifndef GL_COLOR_TABLE_FORMAT             
#define GL_COLOR_TABLE_FORMAT             (GLenum)0x80D8
#endif
#ifndef GL_COLOR_TABLE_WIDTH              
#define GL_COLOR_TABLE_WIDTH              (GLenum)0x80D9
#endif
#ifndef GL_COLOR_TABLE_RED_SIZE           
#define GL_COLOR_TABLE_RED_SIZE           (GLenum)0x80DA
#endif
#ifndef GL_COLOR_TABLE_GREEN_SIZE         
#define GL_COLOR_TABLE_GREEN_SIZE         (GLenum)0x80DB
#endif
#ifndef GL_COLOR_TABLE_BLUE_SIZE          
#define GL_COLOR_TABLE_BLUE_SIZE          (GLenum)0x80DC
#endif
#ifndef GL_COLOR_TABLE_ALPHA_SIZE         
#define GL_COLOR_TABLE_ALPHA_SIZE         (GLenum)0x80DD
#endif
#ifndef GL_COLOR_TABLE_LUMINANCE_SIZE     
#define GL_COLOR_TABLE_LUMINANCE_SIZE     (GLenum)0x80DE
#endif
#ifndef GL_COLOR_TABLE_INTENSITY_SIZE     
#define GL_COLOR_TABLE_INTENSITY_SIZE     (GLenum)0x80DF
#endif
#ifndef GL_CLAMP_TO_EDGE                  
#define GL_CLAMP_TO_EDGE                  (GLenum)0x812F
#endif
#ifndef GL_TEXTURE_MIN_LOD                
#define GL_TEXTURE_MIN_LOD                (GLenum)0x813A
#endif
#ifndef GL_TEXTURE_MAX_LOD                
#define GL_TEXTURE_MAX_LOD                (GLenum)0x813B
#endif
#ifndef GL_TEXTURE_BASE_LEVEL             
#define GL_TEXTURE_BASE_LEVEL             (GLenum)0x813C
#endif
#ifndef GL_TEXTURE_MAX_LEVEL              
#define GL_TEXTURE_MAX_LEVEL              (GLenum)0x813D
#endif
#ifndef GL_TEXTURE0_ARB                   
#define GL_TEXTURE0_ARB                   (GLenum)0x84C0
#endif
#ifndef GL_TEXTURE1_ARB                   
#define GL_TEXTURE1_ARB                   (GLenum)0x84C1
#endif
#ifndef GL_TEXTURE2_ARB                   
#define GL_TEXTURE2_ARB                   (GLenum)0x84C2
#endif
#ifndef GL_TEXTURE3_ARB                   
#define GL_TEXTURE3_ARB                   (GLenum)0x84C3
#endif
#ifndef GL_TEXTURE4_ARB                   
#define GL_TEXTURE4_ARB                   (GLenum)0x84C4
#endif
#ifndef GL_TEXTURE5_ARB                   
#define GL_TEXTURE5_ARB                   (GLenum)0x84C5
#endif
#ifndef GL_TEXTURE6_ARB                   
#define GL_TEXTURE6_ARB                   (GLenum)0x84C6
#endif
#ifndef GL_TEXTURE7_ARB                   
#define GL_TEXTURE7_ARB                   (GLenum)0x84C7
#endif
#ifndef GL_TEXTURE8_ARB                   
#define GL_TEXTURE8_ARB                   (GLenum)0x84C8
#endif
#ifndef GL_TEXTURE9_ARB                   
#define GL_TEXTURE9_ARB                   (GLenum)0x84C9
#endif
#ifndef GL_TEXTURE10_ARB                  
#define GL_TEXTURE10_ARB                  (GLenum)0x84CA
#endif
#ifndef GL_TEXTURE11_ARB                  
#define GL_TEXTURE11_ARB                  (GLenum)0x84CB
#endif
#ifndef GL_TEXTURE12_ARB                  
#define GL_TEXTURE12_ARB                  (GLenum)0x84CC
#endif
#ifndef GL_TEXTURE13_ARB                  
#define GL_TEXTURE13_ARB                  (GLenum)0x84CD
#endif
#ifndef GL_TEXTURE14_ARB                  
#define GL_TEXTURE14_ARB                  (GLenum)0x84CE
#endif
#ifndef GL_TEXTURE15_ARB                  
#define GL_TEXTURE15_ARB                  (GLenum)0x84CF
#endif
#ifndef GL_TEXTURE16_ARB                  
#define GL_TEXTURE16_ARB                  (GLenum)0x84D0
#endif
#ifndef GL_TEXTURE17_ARB                  
#define GL_TEXTURE17_ARB                  (GLenum)0x84D1
#endif
#ifndef GL_TEXTURE18_ARB                  
#define GL_TEXTURE18_ARB                  (GLenum)0x84D2
#endif
#ifndef GL_TEXTURE19_ARB                  
#define GL_TEXTURE19_ARB                  (GLenum)0x84D3
#endif
#ifndef GL_TEXTURE20_ARB                  
#define GL_TEXTURE20_ARB                  (GLenum)0x84D4
#endif
#ifndef GL_TEXTURE21_ARB                  
#define GL_TEXTURE21_ARB                  (GLenum)0x84D5
#endif
#ifndef GL_TEXTURE22_ARB                  
#define GL_TEXTURE22_ARB                  (GLenum)0x84D6
#endif
#ifndef GL_TEXTURE23_ARB                  
#define GL_TEXTURE23_ARB                  (GLenum)0x84D7
#endif
#ifndef GL_TEXTURE24_ARB                  
#define GL_TEXTURE24_ARB                  (GLenum)0x84D8
#endif
#ifndef GL_TEXTURE25_ARB                  
#define GL_TEXTURE25_ARB                  (GLenum)0x84D9
#endif
#ifndef GL_TEXTURE26_ARB                  
#define GL_TEXTURE26_ARB                  (GLenum)0x84DA
#endif
#ifndef GL_TEXTURE27_ARB                  
#define GL_TEXTURE27_ARB                  (GLenum)0x84DB
#endif
#ifndef GL_TEXTURE28_ARB                  
#define GL_TEXTURE28_ARB                  (GLenum)0x84DC
#endif
#ifndef GL_TEXTURE29_ARB                  
#define GL_TEXTURE29_ARB                  (GLenum)0x84DD
#endif
#ifndef GL_TEXTURE30_ARB                  
#define GL_TEXTURE30_ARB                  (GLenum)0x84DE
#endif
#ifndef GL_TEXTURE31_ARB                  
#define GL_TEXTURE31_ARB                  (GLenum)0x84DF
#endif
#ifndef GL_ACTIVE_TEXTURE_ARB             
#define GL_ACTIVE_TEXTURE_ARB             (GLenum)0x84E0
#endif
#ifndef GL_CLIENT_ACTIVE_TEXTURE_ARB      
#define GL_CLIENT_ACTIVE_TEXTURE_ARB      (GLenum)0x84E1
#endif
#ifndef GL_MAX_TEXTURE_UNITS_ARB          
#define GL_MAX_TEXTURE_UNITS_ARB          (GLenum)0x84E2
#endif
#ifndef GL_TRANSPOSE_MODELVIEW_MATRIX_ARB 
#define GL_TRANSPOSE_MODELVIEW_MATRIX_ARB (GLenum)0x84E3
#endif
#ifndef GL_TRANSPOSE_PROJECTION_MATRIX_ARB 
#define GL_TRANSPOSE_PROJECTION_MATRIX_ARB (GLenum)0x84E4
#endif
#ifndef GL_TRANSPOSE_TEXTURE_MATRIX_ARB   
#define GL_TRANSPOSE_TEXTURE_MATRIX_ARB   (GLenum)0x84E5
#endif
#ifndef GL_TRANSPOSE_COLOR_MATRIX_ARB     
#define GL_TRANSPOSE_COLOR_MATRIX_ARB     (GLenum)0x84E6
#endif
#ifndef GL_MULTISAMPLE_ARB                
#define GL_MULTISAMPLE_ARB                (GLenum)0x809D
#endif
#ifndef GL_SAMPLE_ALPHA_TO_COVERAGE_ARB   
#define GL_SAMPLE_ALPHA_TO_COVERAGE_ARB   (GLenum)0x809E
#endif
#ifndef GL_SAMPLE_ALPHA_TO_ONE_ARB        
#define GL_SAMPLE_ALPHA_TO_ONE_ARB        (GLenum)0x809F
#endif
#ifndef GL_SAMPLE_COVERAGE_ARB            
#define GL_SAMPLE_COVERAGE_ARB            (GLenum)0x80A0
#endif
#ifndef GL_SAMPLE_BUFFERS_ARB             
#define GL_SAMPLE_BUFFERS_ARB             (GLenum)0x80A8
#endif
#ifndef GL_SAMPLES_ARB                    
#define GL_SAMPLES_ARB                    (GLenum)0x80A9
#endif
#ifndef GL_SAMPLE_COVERAGE_VALUE_ARB      
#define GL_SAMPLE_COVERAGE_VALUE_ARB      (GLenum)0x80AA
#endif
#ifndef GL_SAMPLE_COVERAGE_INVERT_ARB     
#define GL_SAMPLE_COVERAGE_INVERT_ARB     (GLenum)0x80AB
#endif
#ifndef GL_MULTISAMPLE_BIT_ARB            
#define GL_MULTISAMPLE_BIT_ARB            (GLenum)0x20000000
#endif
#ifndef GL_NORMAL_MAP_ARB                 
#define GL_NORMAL_MAP_ARB                 (GLenum)0x8511
#endif
#ifndef GL_REFLECTION_MAP_ARB             
#define GL_REFLECTION_MAP_ARB             (GLenum)0x8512
#endif
#ifndef GL_TEXTURE_CUBE_MAP_ARB           
#define GL_TEXTURE_CUBE_MAP_ARB           (GLenum)0x8513
#endif
#ifndef GL_TEXTURE_BINDING_CUBE_MAP_ARB   
#define GL_TEXTURE_BINDING_CUBE_MAP_ARB   (GLenum)0x8514
#endif
#ifndef GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB 
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB (GLenum)0x8515
#endif
#ifndef GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB 
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB (GLenum)0x8516
#endif
#ifndef GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB 
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB (GLenum)0x8517
#endif
#ifndef GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB 
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB (GLenum)0x8518
#endif
#ifndef GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB 
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB (GLenum)0x8519
#endif
#ifndef GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB 
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB (GLenum)0x851A
#endif
#ifndef GL_PROXY_TEXTURE_CUBE_MAP_ARB     
#define GL_PROXY_TEXTURE_CUBE_MAP_ARB     (GLenum)0x851B
#endif
#ifndef GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB  
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB  (GLenum)0x851C
#endif
#ifndef GL_COMPRESSED_ALPHA_ARB           
#define GL_COMPRESSED_ALPHA_ARB           (GLenum)0x84E9
#endif
#ifndef GL_COMPRESSED_LUMINANCE_ARB       
#define GL_COMPRESSED_LUMINANCE_ARB       (GLenum)0x84EA
#endif
#ifndef GL_COMPRESSED_LUMINANCE_ALPHA_ARB 
#define GL_COMPRESSED_LUMINANCE_ALPHA_ARB (GLenum)0x84EB
#endif
#ifndef GL_COMPRESSED_INTENSITY_ARB       
#define GL_COMPRESSED_INTENSITY_ARB       (GLenum)0x84EC
#endif
#ifndef GL_COMPRESSED_RGB_ARB             
#define GL_COMPRESSED_RGB_ARB             (GLenum)0x84ED
#endif
#ifndef GL_COMPRESSED_RGBA_ARB            
#define GL_COMPRESSED_RGBA_ARB            (GLenum)0x84EE
#endif
#ifndef GL_TEXTURE_COMPRESSION_HINT_ARB   
#define GL_TEXTURE_COMPRESSION_HINT_ARB   (GLenum)0x84EF
#endif
#ifndef GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB         
#define GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB         (GLenum)0x86A0
#endif
#ifndef GL_TEXTURE_COMPRESSED_ARB         
#define GL_TEXTURE_COMPRESSED_ARB         (GLenum)0x86A1
#endif
#ifndef GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB 
#define GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB (GLenum)0x86A2
#endif
#ifndef GL_COMPRESSED_TEXTURE_FORMATS_ARB 
#define GL_COMPRESSED_TEXTURE_FORMATS_ARB (GLenum)0x86A3
#endif
#ifndef GL_ABGR_EXT                       
#define GL_ABGR_EXT                       (GLenum)0x8000
#endif
#ifndef GL_CONSTANT_COLOR_EXT             
#define GL_CONSTANT_COLOR_EXT             (GLenum)0x8001
#endif
#ifndef GL_ONE_MINUS_CONSTANT_COLOR_EXT   
#define GL_ONE_MINUS_CONSTANT_COLOR_EXT   (GLenum)0x8002
#endif
#ifndef GL_CONSTANT_ALPHA_EXT             
#define GL_CONSTANT_ALPHA_EXT             (GLenum)0x8003
#endif
#ifndef GL_ONE_MINUS_CONSTANT_ALPHA_EXT   
#define GL_ONE_MINUS_CONSTANT_ALPHA_EXT   (GLenum)0x8004
#endif
#ifndef GL_BLEND_COLOR_EXT                
#define GL_BLEND_COLOR_EXT                (GLenum)0x8005
#endif
#ifndef GL_POLYGON_OFFSET_EXT             
#define GL_POLYGON_OFFSET_EXT             (GLenum)0x8037
#endif
#ifndef GL_POLYGON_OFFSET_FACTOR_EXT      
#define GL_POLYGON_OFFSET_FACTOR_EXT      (GLenum)0x8038
#endif
#ifndef GL_POLYGON_OFFSET_BIAS_EXT        
#define GL_POLYGON_OFFSET_BIAS_EXT        (GLenum)0x8039
#endif
#ifndef GL_ALPHA4_EXT                     
#define GL_ALPHA4_EXT                     (GLenum)0x803B
#endif
#ifndef GL_ALPHA8_EXT                     
#define GL_ALPHA8_EXT                     (GLenum)0x803C
#endif
#ifndef GL_ALPHA12_EXT                    
#define GL_ALPHA12_EXT                    (GLenum)0x803D
#endif
#ifndef GL_ALPHA16_EXT                    
#define GL_ALPHA16_EXT                    (GLenum)0x803E
#endif
#ifndef GL_LUMINANCE4_EXT                 
#define GL_LUMINANCE4_EXT                 (GLenum)0x803F
#endif
#ifndef GL_LUMINANCE8_EXT                 
#define GL_LUMINANCE8_EXT                 (GLenum)0x8040
#endif
#ifndef GL_LUMINANCE12_EXT                
#define GL_LUMINANCE12_EXT                (GLenum)0x8041
#endif
#ifndef GL_LUMINANCE16_EXT                
#define GL_LUMINANCE16_EXT                (GLenum)0x8042
#endif
#ifndef GL_LUMINANCE4_ALPHA4_EXT          
#define GL_LUMINANCE4_ALPHA4_EXT          (GLenum)0x8043
#endif
#ifndef GL_LUMINANCE6_ALPHA2_EXT          
#define GL_LUMINANCE6_ALPHA2_EXT          (GLenum)0x8044
#endif
#ifndef GL_LUMINANCE8_ALPHA8_EXT          
#define GL_LUMINANCE8_ALPHA8_EXT          (GLenum)0x8045
#endif
#ifndef GL_LUMINANCE12_ALPHA4_EXT         
#define GL_LUMINANCE12_ALPHA4_EXT         (GLenum)0x8046
#endif
#ifndef GL_LUMINANCE12_ALPHA12_EXT        
#define GL_LUMINANCE12_ALPHA12_EXT        (GLenum)0x8047
#endif
#ifndef GL_LUMINANCE16_ALPHA16_EXT        
#define GL_LUMINANCE16_ALPHA16_EXT        (GLenum)0x8048
#endif
#ifndef GL_INTENSITY_EXT                  
#define GL_INTENSITY_EXT                  (GLenum)0x8049
#endif
#ifndef GL_INTENSITY4_EXT                 
#define GL_INTENSITY4_EXT                 (GLenum)0x804A
#endif
#ifndef GL_INTENSITY8_EXT                 
#define GL_INTENSITY8_EXT                 (GLenum)0x804B
#endif
#ifndef GL_INTENSITY12_EXT                
#define GL_INTENSITY12_EXT                (GLenum)0x804C
#endif
#ifndef GL_INTENSITY16_EXT                
#define GL_INTENSITY16_EXT                (GLenum)0x804D
#endif
#ifndef GL_RGB2_EXT                       
#define GL_RGB2_EXT                       (GLenum)0x804E
#endif
#ifndef GL_RGB4_EXT                       
#define GL_RGB4_EXT                       (GLenum)0x804F
#endif
#ifndef GL_RGB5_EXT                       
#define GL_RGB5_EXT                       (GLenum)0x8050
#endif
#ifndef GL_RGB8_EXT                       
#define GL_RGB8_EXT                       (GLenum)0x8051
#endif
#ifndef GL_RGB10_EXT                      
#define GL_RGB10_EXT                      (GLenum)0x8052
#endif
#ifndef GL_RGB12_EXT                      
#define GL_RGB12_EXT                      (GLenum)0x8053
#endif
#ifndef GL_RGB16_EXT                      
#define GL_RGB16_EXT                      (GLenum)0x8054
#endif
#ifndef GL_RGBA2_EXT                      
#define GL_RGBA2_EXT                      (GLenum)0x8055
#endif
#ifndef GL_RGBA4_EXT                      
#define GL_RGBA4_EXT                      (GLenum)0x8056
#endif
#ifndef GL_RGB5_A1_EXT                    
#define GL_RGB5_A1_EXT                    (GLenum)0x8057
#endif
#ifndef GL_RGBA8_EXT                      
#define GL_RGBA8_EXT                      (GLenum)0x8058
#endif
#ifndef GL_RGB10_A2_EXT                   
#define GL_RGB10_A2_EXT                   (GLenum)0x8059
#endif
#ifndef GL_RGBA12_EXT                     
#define GL_RGBA12_EXT                     (GLenum)0x805A
#endif
#ifndef GL_RGBA16_EXT                     
#define GL_RGBA16_EXT                     (GLenum)0x805B
#endif
#ifndef GL_TEXTURE_RED_SIZE_EXT           
#define GL_TEXTURE_RED_SIZE_EXT           (GLenum)0x805C
#endif
#ifndef GL_TEXTURE_GREEN_SIZE_EXT         
#define GL_TEXTURE_GREEN_SIZE_EXT         (GLenum)0x805D
#endif
#ifndef GL_TEXTURE_BLUE_SIZE_EXT          
#define GL_TEXTURE_BLUE_SIZE_EXT          (GLenum)0x805E
#endif
#ifndef GL_TEXTURE_ALPHA_SIZE_EXT         
#define GL_TEXTURE_ALPHA_SIZE_EXT         (GLenum)0x805F
#endif
#ifndef GL_TEXTURE_LUMINANCE_SIZE_EXT     
#define GL_TEXTURE_LUMINANCE_SIZE_EXT     (GLenum)0x8060
#endif
#ifndef GL_TEXTURE_INTENSITY_SIZE_EXT     
#define GL_TEXTURE_INTENSITY_SIZE_EXT     (GLenum)0x8061
#endif
#ifndef GL_REPLACE_EXT                    
#define GL_REPLACE_EXT                    (GLenum)0x8062
#endif
#ifndef GL_PROXY_TEXTURE_1D_EXT           
#define GL_PROXY_TEXTURE_1D_EXT           (GLenum)0x8063
#endif
#ifndef GL_PROXY_TEXTURE_2D_EXT           
#define GL_PROXY_TEXTURE_2D_EXT           (GLenum)0x8064
#endif
#ifndef GL_TEXTURE_TOO_LARGE_EXT          
#define GL_TEXTURE_TOO_LARGE_EXT          (GLenum)0x8065
#endif
#ifndef GL_PACK_SKIP_IMAGES               
#define GL_PACK_SKIP_IMAGES               (GLenum)0x806B
#endif
#ifndef GL_PACK_SKIP_IMAGES_EXT           
#define GL_PACK_SKIP_IMAGES_EXT           (GLenum)0x806B
#endif
#ifndef GL_PACK_IMAGE_HEIGHT              
#define GL_PACK_IMAGE_HEIGHT              (GLenum)0x806C
#endif
#ifndef GL_PACK_IMAGE_HEIGHT_EXT          
#define GL_PACK_IMAGE_HEIGHT_EXT          (GLenum)0x806C
#endif
#ifndef GL_UNPACK_SKIP_IMAGES             
#define GL_UNPACK_SKIP_IMAGES             (GLenum)0x806D
#endif
#ifndef GL_UNPACK_SKIP_IMAGES_EXT         
#define GL_UNPACK_SKIP_IMAGES_EXT         (GLenum)0x806D
#endif
#ifndef GL_UNPACK_IMAGE_HEIGHT            
#define GL_UNPACK_IMAGE_HEIGHT            (GLenum)0x806E
#endif
#ifndef GL_UNPACK_IMAGE_HEIGHT_EXT        
#define GL_UNPACK_IMAGE_HEIGHT_EXT        (GLenum)0x806E
#endif
#ifndef GL_TEXTURE_3D                     
#define GL_TEXTURE_3D                     (GLenum)0x806F
#endif
#ifndef GL_TEXTURE_3D_EXT                 
#define GL_TEXTURE_3D_EXT                 (GLenum)0x806F
#endif
#ifndef GL_PROXY_TEXTURE_3D               
#define GL_PROXY_TEXTURE_3D               (GLenum)0x8070
#endif
#ifndef GL_PROXY_TEXTURE_3D_EXT           
#define GL_PROXY_TEXTURE_3D_EXT           (GLenum)0x8070
#endif
#ifndef GL_TEXTURE_DEPTH                  
#define GL_TEXTURE_DEPTH                  (GLenum)0x8071
#endif
#ifndef GL_TEXTURE_DEPTH_EXT              
#define GL_TEXTURE_DEPTH_EXT              (GLenum)0x8071
#endif
#ifndef GL_TEXTURE_WRAP_R                 
#define GL_TEXTURE_WRAP_R                 (GLenum)0x8072
#endif
#ifndef GL_TEXTURE_WRAP_R_EXT             
#define GL_TEXTURE_WRAP_R_EXT             (GLenum)0x8072
#endif
#ifndef GL_MAX_3D_TEXTURE_SIZE            
#define GL_MAX_3D_TEXTURE_SIZE            (GLenum)0x8073
#endif
#ifndef GL_MAX_3D_TEXTURE_SIZE_EXT        
#define GL_MAX_3D_TEXTURE_SIZE_EXT        (GLenum)0x8073
#endif
#ifndef GL_FILTER4_SGIS                   
#define GL_FILTER4_SGIS                   (GLenum)0x8146
#endif
#ifndef GL_TEXTURE_FILTER4_SIZE_SGIS      
#define GL_TEXTURE_FILTER4_SIZE_SGIS      (GLenum)0x8147
#endif
#ifndef GL_HISTOGRAM_EXT                  
#define GL_HISTOGRAM_EXT                  (GLenum)0x8024
#endif
#ifndef GL_PROXY_HISTOGRAM_EXT            
#define GL_PROXY_HISTOGRAM_EXT            (GLenum)0x8025
#endif
#ifndef GL_HISTOGRAM_WIDTH_EXT            
#define GL_HISTOGRAM_WIDTH_EXT            (GLenum)0x8026
#endif
#ifndef GL_HISTOGRAM_FORMAT_EXT           
#define GL_HISTOGRAM_FORMAT_EXT           (GLenum)0x8027
#endif
#ifndef GL_HISTOGRAM_RED_SIZE_EXT         
#define GL_HISTOGRAM_RED_SIZE_EXT         (GLenum)0x8028
#endif
#ifndef GL_HISTOGRAM_GREEN_SIZE_EXT       
#define GL_HISTOGRAM_GREEN_SIZE_EXT       (GLenum)0x8029
#endif
#ifndef GL_HISTOGRAM_BLUE_SIZE_EXT        
#define GL_HISTOGRAM_BLUE_SIZE_EXT        (GLenum)0x802A
#endif
#ifndef GL_HISTOGRAM_ALPHA_SIZE_EXT       
#define GL_HISTOGRAM_ALPHA_SIZE_EXT       (GLenum)0x802B
#endif
#ifndef GL_HISTOGRAM_LUMINANCE_SIZE_EXT   
#define GL_HISTOGRAM_LUMINANCE_SIZE_EXT   (GLenum)0x802C
#endif
#ifndef GL_HISTOGRAM_SINK_EXT             
#define GL_HISTOGRAM_SINK_EXT             (GLenum)0x802D
#endif
#ifndef GL_MINMAX_EXT                     
#define GL_MINMAX_EXT                     (GLenum)0x802E
#endif
#ifndef GL_MINMAX_FORMAT_EXT              
#define GL_MINMAX_FORMAT_EXT              (GLenum)0x802F
#endif
#ifndef GL_MINMAX_SINK_EXT                
#define GL_MINMAX_SINK_EXT                (GLenum)0x8030
#endif
#ifndef GL_TABLE_TOO_LARGE_EXT            
#define GL_TABLE_TOO_LARGE_EXT            (GLenum)0x8031
#endif
#ifndef GL_CONVOLUTION_1D_EXT             
#define GL_CONVOLUTION_1D_EXT             (GLenum)0x8010
#endif
#ifndef GL_CONVOLUTION_2D_EXT             
#define GL_CONVOLUTION_2D_EXT             (GLenum)0x8011
#endif
#ifndef GL_SEPARABLE_2D_EXT               
#define GL_SEPARABLE_2D_EXT               (GLenum)0x8012
#endif
#ifndef GL_CONVOLUTION_BORDER_MODE_EXT    
#define GL_CONVOLUTION_BORDER_MODE_EXT    (GLenum)0x8013
#endif
#ifndef GL_CONVOLUTION_FILTER_SCALE_EXT   
#define GL_CONVOLUTION_FILTER_SCALE_EXT   (GLenum)0x8014
#endif
#ifndef GL_CONVOLUTION_FILTER_BIAS_EXT    
#define GL_CONVOLUTION_FILTER_BIAS_EXT    (GLenum)0x8015
#endif
#ifndef GL_REDUCE_EXT                     
#define GL_REDUCE_EXT                     (GLenum)0x8016
#endif
#ifndef GL_CONVOLUTION_FORMAT_EXT         
#define GL_CONVOLUTION_FORMAT_EXT         (GLenum)0x8017
#endif
#ifndef GL_CONVOLUTION_WIDTH_EXT          
#define GL_CONVOLUTION_WIDTH_EXT          (GLenum)0x8018
#endif
#ifndef GL_CONVOLUTION_HEIGHT_EXT         
#define GL_CONVOLUTION_HEIGHT_EXT         (GLenum)0x8019
#endif
#ifndef GL_MAX_CONVOLUTION_WIDTH_EXT      
#define GL_MAX_CONVOLUTION_WIDTH_EXT      (GLenum)0x801A
#endif
#ifndef GL_MAX_CONVOLUTION_HEIGHT_EXT     
#define GL_MAX_CONVOLUTION_HEIGHT_EXT     (GLenum)0x801B
#endif
#ifndef GL_POST_CONVOLUTION_RED_SCALE_EXT 
#define GL_POST_CONVOLUTION_RED_SCALE_EXT (GLenum)0x801C
#endif
#ifndef GL_POST_CONVOLUTION_GREEN_SCALE_EXT 
#define GL_POST_CONVOLUTION_GREEN_SCALE_EXT (GLenum)0x801D
#endif
#ifndef GL_POST_CONVOLUTION_BLUE_SCALE_EXT 
#define GL_POST_CONVOLUTION_BLUE_SCALE_EXT (GLenum)0x801E
#endif
#ifndef GL_POST_CONVOLUTION_ALPHA_SCALE_EXT 
#define GL_POST_CONVOLUTION_ALPHA_SCALE_EXT (GLenum)0x801F
#endif
#ifndef GL_POST_CONVOLUTION_RED_BIAS_EXT  
#define GL_POST_CONVOLUTION_RED_BIAS_EXT  (GLenum)0x8020
#endif
#ifndef GL_POST_CONVOLUTION_GREEN_BIAS_EXT 
#define GL_POST_CONVOLUTION_GREEN_BIAS_EXT (GLenum)0x8021
#endif
#ifndef GL_POST_CONVOLUTION_BLUE_BIAS_EXT 
#define GL_POST_CONVOLUTION_BLUE_BIAS_EXT (GLenum)0x8022
#endif
#ifndef GL_POST_CONVOLUTION_ALPHA_BIAS_EXT 
#define GL_POST_CONVOLUTION_ALPHA_BIAS_EXT (GLenum)0x8023
#endif
#ifndef GL_COLOR_MATRIX_SGI               
#define GL_COLOR_MATRIX_SGI               (GLenum)0x80B1
#endif
#ifndef GL_COLOR_MATRIX_STACK_DEPTH_SGI   
#define GL_COLOR_MATRIX_STACK_DEPTH_SGI   (GLenum)0x80B2
#endif
#ifndef GL_MAX_COLOR_MATRIX_STACK_DEPTH_SGI 
#define GL_MAX_COLOR_MATRIX_STACK_DEPTH_SGI (GLenum)0x80B3
#endif
#ifndef GL_POST_COLOR_MATRIX_RED_SCALE_SGI 
#define GL_POST_COLOR_MATRIX_RED_SCALE_SGI (GLenum)0x80B4
#endif
#ifndef GL_POST_COLOR_MATRIX_GREEN_SCALE_SGI 
#define GL_POST_COLOR_MATRIX_GREEN_SCALE_SGI (GLenum)0x80B5
#endif
#ifndef GL_POST_COLOR_MATRIX_BLUE_SCALE_SGI 
#define GL_POST_COLOR_MATRIX_BLUE_SCALE_SGI (GLenum)0x80B6
#endif
#ifndef GL_POST_COLOR_MATRIX_ALPHA_SCALE_SGI 
#define GL_POST_COLOR_MATRIX_ALPHA_SCALE_SGI (GLenum)0x80B7
#endif
#ifndef GL_POST_COLOR_MATRIX_RED_BIAS_SGI 
#define GL_POST_COLOR_MATRIX_RED_BIAS_SGI (GLenum)0x80B8
#endif
#ifndef GL_POST_COLOR_MATRIX_GREEN_BIAS_SGI 
#define GL_POST_COLOR_MATRIX_GREEN_BIAS_SGI (GLenum)0x80B9
#endif
#ifndef GL_POST_COLOR_MATRIX_BLUE_BIAS_SGI 
#define GL_POST_COLOR_MATRIX_BLUE_BIAS_SGI (GLenum)0x80BA
#endif
#ifndef GL_POST_COLOR_MATRIX_ALPHA_BIAS_SGI 
#define GL_POST_COLOR_MATRIX_ALPHA_BIAS_SGI (GLenum)0x80BB
#endif
#ifndef GL_COLOR_TABLE_SGI                
#define GL_COLOR_TABLE_SGI                (GLenum)0x80D0
#endif
#ifndef GL_POST_CONVOLUTION_COLOR_TABLE_SGI 
#define GL_POST_CONVOLUTION_COLOR_TABLE_SGI (GLenum)0x80D1
#endif
#ifndef GL_POST_COLOR_MATRIX_COLOR_TABLE_SGI 
#define GL_POST_COLOR_MATRIX_COLOR_TABLE_SGI (GLenum)0x80D2
#endif
#ifndef GL_PROXY_COLOR_TABLE_SGI          
#define GL_PROXY_COLOR_TABLE_SGI          (GLenum)0x80D3
#endif
#ifndef GL_PROXY_POST_CONVOLUTION_COLOR_TABLE_SGI 
#define GL_PROXY_POST_CONVOLUTION_COLOR_TABLE_SGI (GLenum)0x80D4
#endif
#ifndef GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE_SGI 
#define GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE_SGI (GLenum)0x80D5
#endif
#ifndef GL_COLOR_TABLE_SCALE_SGI          
#define GL_COLOR_TABLE_SCALE_SGI          (GLenum)0x80D6
#endif
#ifndef GL_COLOR_TABLE_BIAS_SGI           
#define GL_COLOR_TABLE_BIAS_SGI           (GLenum)0x80D7
#endif
#ifndef GL_COLOR_TABLE_FORMAT_SGI         
#define GL_COLOR_TABLE_FORMAT_SGI         (GLenum)0x80D8
#endif
#ifndef GL_COLOR_TABLE_WIDTH_SGI          
#define GL_COLOR_TABLE_WIDTH_SGI          (GLenum)0x80D9
#endif
#ifndef GL_COLOR_TABLE_RED_SIZE_SGI       
#define GL_COLOR_TABLE_RED_SIZE_SGI       (GLenum)0x80DA
#endif
#ifndef GL_COLOR_TABLE_GREEN_SIZE_SGI     
#define GL_COLOR_TABLE_GREEN_SIZE_SGI     (GLenum)0x80DB
#endif
#ifndef GL_COLOR_TABLE_BLUE_SIZE_SGI      
#define GL_COLOR_TABLE_BLUE_SIZE_SGI      (GLenum)0x80DC
#endif
#ifndef GL_COLOR_TABLE_ALPHA_SIZE_SGI     
#define GL_COLOR_TABLE_ALPHA_SIZE_SGI     (GLenum)0x80DD
#endif
#ifndef GL_COLOR_TABLE_LUMINANCE_SIZE_SGI 
#define GL_COLOR_TABLE_LUMINANCE_SIZE_SGI (GLenum)0x80DE
#endif
#ifndef GL_COLOR_TABLE_INTENSITY_SIZE_SGI 
#define GL_COLOR_TABLE_INTENSITY_SIZE_SGI (GLenum)0x80DF
#endif
#ifndef GL_PIXEL_TEXTURE_SGIS             
#define GL_PIXEL_TEXTURE_SGIS             (GLenum)0x8353
#endif
#ifndef GL_PIXEL_FRAGMENT_RGB_SOURCE_SGIS 
#define GL_PIXEL_FRAGMENT_RGB_SOURCE_SGIS (GLenum)0x8354
#endif
#ifndef GL_PIXEL_FRAGMENT_ALPHA_SOURCE_SGIS 
#define GL_PIXEL_FRAGMENT_ALPHA_SOURCE_SGIS (GLenum)0x8355
#endif
#ifndef GL_PIXEL_GROUP_COLOR_SGIS         
#define GL_PIXEL_GROUP_COLOR_SGIS         (GLenum)0x8356
#endif
#ifndef GL_PIXEL_TEX_GEN_SGIX             
#define GL_PIXEL_TEX_GEN_SGIX             (GLenum)0x8139
#endif
#ifndef GL_PIXEL_TEX_GEN_MODE_SGIX        
#define GL_PIXEL_TEX_GEN_MODE_SGIX        (GLenum)0x832B
#endif
#ifndef GL_PACK_SKIP_VOLUMES_SGIS         
#define GL_PACK_SKIP_VOLUMES_SGIS         (GLenum)0x8130
#endif
#ifndef GL_PACK_IMAGE_DEPTH_SGIS          
#define GL_PACK_IMAGE_DEPTH_SGIS          (GLenum)0x8131
#endif
#ifndef GL_UNPACK_SKIP_VOLUMES_SGIS       
#define GL_UNPACK_SKIP_VOLUMES_SGIS       (GLenum)0x8132
#endif
#ifndef GL_UNPACK_IMAGE_DEPTH_SGIS        
#define GL_UNPACK_IMAGE_DEPTH_SGIS        (GLenum)0x8133
#endif
#ifndef GL_TEXTURE_4D_SGIS                
#define GL_TEXTURE_4D_SGIS                (GLenum)0x8134
#endif
#ifndef GL_PROXY_TEXTURE_4D_SGIS          
#define GL_PROXY_TEXTURE_4D_SGIS          (GLenum)0x8135
#endif
#ifndef GL_TEXTURE_4DSIZE_SGIS            
#define GL_TEXTURE_4DSIZE_SGIS            (GLenum)0x8136
#endif
#ifndef GL_TEXTURE_WRAP_Q_SGIS            
#define GL_TEXTURE_WRAP_Q_SGIS            (GLenum)0x8137
#endif
#ifndef GL_MAX_4D_TEXTURE_SIZE_SGIS       
#define GL_MAX_4D_TEXTURE_SIZE_SGIS       (GLenum)0x8138
#endif
#ifndef GL_TEXTURE_4D_BINDING_SGIS        
#define GL_TEXTURE_4D_BINDING_SGIS        (GLenum)0x814F
#endif
#ifndef GL_TEXTURE_COLOR_TABLE_SGI        
#define GL_TEXTURE_COLOR_TABLE_SGI        (GLenum)0x80BC
#endif
#ifndef GL_PROXY_TEXTURE_COLOR_TABLE_SGI  
#define GL_PROXY_TEXTURE_COLOR_TABLE_SGI  (GLenum)0x80BD
#endif
#ifndef GL_CMYK_EXT                       
#define GL_CMYK_EXT                       (GLenum)0x800C
#endif
#ifndef GL_CMYKA_EXT                      
#define GL_CMYKA_EXT                      (GLenum)0x800D
#endif
#ifndef GL_PACK_CMYK_HINT_EXT             
#define GL_PACK_CMYK_HINT_EXT             (GLenum)0x800E
#endif
#ifndef GL_UNPACK_CMYK_HINT_EXT           
#define GL_UNPACK_CMYK_HINT_EXT           (GLenum)0x800F
#endif
#ifndef GL_TEXTURE_PRIORITY_EXT           
#define GL_TEXTURE_PRIORITY_EXT           (GLenum)0x8066
#endif
#ifndef GL_TEXTURE_RESIDENT_EXT           
#define GL_TEXTURE_RESIDENT_EXT           (GLenum)0x8067
#endif
#ifndef GL_TEXTURE_1D_BINDING_EXT         
#define GL_TEXTURE_1D_BINDING_EXT         (GLenum)0x8068
#endif
#ifndef GL_TEXTURE_2D_BINDING_EXT         
#define GL_TEXTURE_2D_BINDING_EXT         (GLenum)0x8069
#endif
#ifndef GL_TEXTURE_3D_BINDING_EXT         
#define GL_TEXTURE_3D_BINDING_EXT         (GLenum)0x806A
#endif
#ifndef GL_DETAIL_TEXTURE_2D_SGIS         
#define GL_DETAIL_TEXTURE_2D_SGIS         (GLenum)0x8095
#endif
#ifndef GL_DETAIL_TEXTURE_2D_BINDING_SGIS 
#define GL_DETAIL_TEXTURE_2D_BINDING_SGIS (GLenum)0x8096
#endif
#ifndef GL_LINEAR_DETAIL_SGIS             
#define GL_LINEAR_DETAIL_SGIS             (GLenum)0x8097
#endif
#ifndef GL_LINEAR_DETAIL_ALPHA_SGIS       
#define GL_LINEAR_DETAIL_ALPHA_SGIS       (GLenum)0x8098
#endif
#ifndef GL_LINEAR_DETAIL_COLOR_SGIS       
#define GL_LINEAR_DETAIL_COLOR_SGIS       (GLenum)0x8099
#endif
#ifndef GL_DETAIL_TEXTURE_LEVEL_SGIS      
#define GL_DETAIL_TEXTURE_LEVEL_SGIS      (GLenum)0x809A
#endif
#ifndef GL_DETAIL_TEXTURE_MODE_SGIS       
#define GL_DETAIL_TEXTURE_MODE_SGIS       (GLenum)0x809B
#endif
#ifndef GL_DETAIL_TEXTURE_FUNC_POINTS_SGIS 
#define GL_DETAIL_TEXTURE_FUNC_POINTS_SGIS (GLenum)0x809C
#endif
#ifndef GL_LINEAR_SHARPEN_SGIS            
#define GL_LINEAR_SHARPEN_SGIS            (GLenum)0x80AD
#endif
#ifndef GL_LINEAR_SHARPEN_ALPHA_SGIS      
#define GL_LINEAR_SHARPEN_ALPHA_SGIS      (GLenum)0x80AE
#endif
#ifndef GL_LINEAR_SHARPEN_COLOR_SGIS      
#define GL_LINEAR_SHARPEN_COLOR_SGIS      (GLenum)0x80AF
#endif
#ifndef GL_SHARPEN_TEXTURE_FUNC_POINTS_SGIS 
#define GL_SHARPEN_TEXTURE_FUNC_POINTS_SGIS (GLenum)0x80B0
#endif
#ifndef GL_UNSIGNED_BYTE_3_3_2_EXT        
#define GL_UNSIGNED_BYTE_3_3_2_EXT        (GLenum)0x8032
#endif
#ifndef GL_UNSIGNED_SHORT_4_4_4_4_EXT     
#define GL_UNSIGNED_SHORT_4_4_4_4_EXT     (GLenum)0x8033
#endif
#ifndef GL_UNSIGNED_SHORT_5_5_5_1_EXT     
#define GL_UNSIGNED_SHORT_5_5_5_1_EXT     (GLenum)0x8034
#endif
#ifndef GL_UNSIGNED_INT_8_8_8_8_EXT       
#define GL_UNSIGNED_INT_8_8_8_8_EXT       (GLenum)0x8035
#endif
#ifndef GL_UNSIGNED_INT_10_10_10_2_EXT    
#define GL_UNSIGNED_INT_10_10_10_2_EXT    (GLenum)0x8036
#endif
#ifndef GL_TEXTURE_MIN_LOD_SGIS           
#define GL_TEXTURE_MIN_LOD_SGIS           (GLenum)0x813A
#endif
#ifndef GL_TEXTURE_MAX_LOD_SGIS           
#define GL_TEXTURE_MAX_LOD_SGIS           (GLenum)0x813B
#endif
#ifndef GL_TEXTURE_BASE_LEVEL_SGIS        
#define GL_TEXTURE_BASE_LEVEL_SGIS        (GLenum)0x813C
#endif
#ifndef GL_TEXTURE_MAX_LEVEL_SGIS         
#define GL_TEXTURE_MAX_LEVEL_SGIS         (GLenum)0x813D
#endif
#ifndef GL_MULTISAMPLE_SGIS               
#define GL_MULTISAMPLE_SGIS               (GLenum)0x809D
#endif
#ifndef GL_SAMPLE_ALPHA_TO_MASK_SGIS      
#define GL_SAMPLE_ALPHA_TO_MASK_SGIS      (GLenum)0x809E
#endif
#ifndef GL_SAMPLE_ALPHA_TO_ONE_SGIS       
#define GL_SAMPLE_ALPHA_TO_ONE_SGIS       (GLenum)0x809F
#endif
#ifndef GL_SAMPLE_MASK_SGIS               
#define GL_SAMPLE_MASK_SGIS               (GLenum)0x80A0
#endif
#ifndef GL_1PASS_SGIS                     
#define GL_1PASS_SGIS                     (GLenum)0x80A1
#endif
#ifndef GL_2PASS_0_SGIS                   
#define GL_2PASS_0_SGIS                   (GLenum)0x80A2
#endif
#ifndef GL_2PASS_1_SGIS                   
#define GL_2PASS_1_SGIS                   (GLenum)0x80A3
#endif
#ifndef GL_4PASS_0_SGIS                   
#define GL_4PASS_0_SGIS                   (GLenum)0x80A4
#endif
#ifndef GL_4PASS_1_SGIS                   
#define GL_4PASS_1_SGIS                   (GLenum)0x80A5
#endif
#ifndef GL_4PASS_2_SGIS                   
#define GL_4PASS_2_SGIS                   (GLenum)0x80A6
#endif
#ifndef GL_4PASS_3_SGIS                   
#define GL_4PASS_3_SGIS                   (GLenum)0x80A7
#endif
#ifndef GL_SAMPLE_BUFFERS_SGIS            
#define GL_SAMPLE_BUFFERS_SGIS            (GLenum)0x80A8
#endif
#ifndef GL_SAMPLES_SGIS                   
#define GL_SAMPLES_SGIS                   (GLenum)0x80A9
#endif
#ifndef GL_SAMPLE_MASK_VALUE_SGIS         
#define GL_SAMPLE_MASK_VALUE_SGIS         (GLenum)0x80AA
#endif
#ifndef GL_SAMPLE_MASK_INVERT_SGIS        
#define GL_SAMPLE_MASK_INVERT_SGIS        (GLenum)0x80AB
#endif
#ifndef GL_SAMPLE_PATTERN_SGIS            
#define GL_SAMPLE_PATTERN_SGIS            (GLenum)0x80AC
#endif
#ifndef GL_RESCALE_NORMAL_EXT             
#define GL_RESCALE_NORMAL_EXT             (GLenum)0x803A
#endif
#ifndef GL_VERTEX_ARRAY_EXT               
#define GL_VERTEX_ARRAY_EXT               (GLenum)0x8074
#endif
#ifndef GL_NORMAL_ARRAY_EXT               
#define GL_NORMAL_ARRAY_EXT               (GLenum)0x8075
#endif
#ifndef GL_COLOR_ARRAY_EXT                
#define GL_COLOR_ARRAY_EXT                (GLenum)0x8076
#endif
#ifndef GL_INDEX_ARRAY_EXT                
#define GL_INDEX_ARRAY_EXT                (GLenum)0x8077
#endif
#ifndef GL_TEXTURE_COORD_ARRAY_EXT        
#define GL_TEXTURE_COORD_ARRAY_EXT        (GLenum)0x8078
#endif
#ifndef GL_EDGE_FLAG_ARRAY_EXT            
#define GL_EDGE_FLAG_ARRAY_EXT            (GLenum)0x8079
#endif
#ifndef GL_VERTEX_ARRAY_SIZE_EXT          
#define GL_VERTEX_ARRAY_SIZE_EXT          (GLenum)0x807A
#endif
#ifndef GL_VERTEX_ARRAY_TYPE_EXT          
#define GL_VERTEX_ARRAY_TYPE_EXT          (GLenum)0x807B
#endif
#ifndef GL_VERTEX_ARRAY_STRIDE_EXT        
#define GL_VERTEX_ARRAY_STRIDE_EXT        (GLenum)0x807C
#endif
#ifndef GL_VERTEX_ARRAY_COUNT_EXT         
#define GL_VERTEX_ARRAY_COUNT_EXT         (GLenum)0x807D
#endif
#ifndef GL_NORMAL_ARRAY_TYPE_EXT          
#define GL_NORMAL_ARRAY_TYPE_EXT          (GLenum)0x807E
#endif
#ifndef GL_NORMAL_ARRAY_STRIDE_EXT        
#define GL_NORMAL_ARRAY_STRIDE_EXT        (GLenum)0x807F
#endif
#ifndef GL_NORMAL_ARRAY_COUNT_EXT         
#define GL_NORMAL_ARRAY_COUNT_EXT         (GLenum)0x8080
#endif
#ifndef GL_COLOR_ARRAY_SIZE_EXT           
#define GL_COLOR_ARRAY_SIZE_EXT           (GLenum)0x8081
#endif
#ifndef GL_COLOR_ARRAY_TYPE_EXT           
#define GL_COLOR_ARRAY_TYPE_EXT           (GLenum)0x8082
#endif
#ifndef GL_COLOR_ARRAY_STRIDE_EXT         
#define GL_COLOR_ARRAY_STRIDE_EXT         (GLenum)0x8083
#endif
#ifndef GL_COLOR_ARRAY_COUNT_EXT          
#define GL_COLOR_ARRAY_COUNT_EXT          (GLenum)0x8084
#endif
#ifndef GL_INDEX_ARRAY_TYPE_EXT           
#define GL_INDEX_ARRAY_TYPE_EXT           (GLenum)0x8085
#endif
#ifndef GL_INDEX_ARRAY_STRIDE_EXT         
#define GL_INDEX_ARRAY_STRIDE_EXT         (GLenum)0x8086
#endif
#ifndef GL_INDEX_ARRAY_COUNT_EXT          
#define GL_INDEX_ARRAY_COUNT_EXT          (GLenum)0x8087
#endif
#ifndef GL_TEXTURE_COORD_ARRAY_SIZE_EXT   
#define GL_TEXTURE_COORD_ARRAY_SIZE_EXT   (GLenum)0x8088
#endif
#ifndef GL_TEXTURE_COORD_ARRAY_TYPE_EXT   
#define GL_TEXTURE_COORD_ARRAY_TYPE_EXT   (GLenum)0x8089
#endif
#ifndef GL_TEXTURE_COORD_ARRAY_STRIDE_EXT 
#define GL_TEXTURE_COORD_ARRAY_STRIDE_EXT (GLenum)0x808A
#endif
#ifndef GL_TEXTURE_COORD_ARRAY_COUNT_EXT  
#define GL_TEXTURE_COORD_ARRAY_COUNT_EXT  (GLenum)0x808B
#endif
#ifndef GL_EDGE_FLAG_ARRAY_STRIDE_EXT     
#define GL_EDGE_FLAG_ARRAY_STRIDE_EXT     (GLenum)0x808C
#endif
#ifndef GL_EDGE_FLAG_ARRAY_COUNT_EXT      
#define GL_EDGE_FLAG_ARRAY_COUNT_EXT      (GLenum)0x808D
#endif
#ifndef GL_VERTEX_ARRAY_POINTER_EXT       
#define GL_VERTEX_ARRAY_POINTER_EXT       (GLenum)0x808E
#endif
#ifndef GL_NORMAL_ARRAY_POINTER_EXT       
#define GL_NORMAL_ARRAY_POINTER_EXT       (GLenum)0x808F
#endif
#ifndef GL_COLOR_ARRAY_POINTER_EXT        
#define GL_COLOR_ARRAY_POINTER_EXT        (GLenum)0x8090
#endif
#ifndef GL_INDEX_ARRAY_POINTER_EXT        
#define GL_INDEX_ARRAY_POINTER_EXT        (GLenum)0x8091
#endif
#ifndef GL_TEXTURE_COORD_ARRAY_POINTER_EXT 
#define GL_TEXTURE_COORD_ARRAY_POINTER_EXT (GLenum)0x8092
#endif
#ifndef GL_EDGE_FLAG_ARRAY_POINTER_EXT    
#define GL_EDGE_FLAG_ARRAY_POINTER_EXT    (GLenum)0x8093
#endif
#ifndef GL_GENERATE_MIPMAP_SGIS           
#define GL_GENERATE_MIPMAP_SGIS           (GLenum)0x8191
#endif
#ifndef GL_GENERATE_MIPMAP_HINT_SGIS      
#define GL_GENERATE_MIPMAP_HINT_SGIS      (GLenum)0x8192
#endif
#ifndef GL_LINEAR_CLIPMAP_LINEAR_SGIX     
#define GL_LINEAR_CLIPMAP_LINEAR_SGIX     (GLenum)0x8170
#endif
#ifndef GL_TEXTURE_CLIPMAP_CENTER_SGIX    
#define GL_TEXTURE_CLIPMAP_CENTER_SGIX    (GLenum)0x8171
#endif
#ifndef GL_TEXTURE_CLIPMAP_FRAME_SGIX     
#define GL_TEXTURE_CLIPMAP_FRAME_SGIX     (GLenum)0x8172
#endif
#ifndef GL_TEXTURE_CLIPMAP_OFFSET_SGIX    
#define GL_TEXTURE_CLIPMAP_OFFSET_SGIX    (GLenum)0x8173
#endif
#ifndef GL_TEXTURE_CLIPMAP_VIRTUAL_DEPTH_SGIX 
#define GL_TEXTURE_CLIPMAP_VIRTUAL_DEPTH_SGIX (GLenum)0x8174
#endif
#ifndef GL_TEXTURE_CLIPMAP_LOD_OFFSET_SGIX 
#define GL_TEXTURE_CLIPMAP_LOD_OFFSET_SGIX (GLenum)0x8175
#endif
#ifndef GL_TEXTURE_CLIPMAP_DEPTH_SGIX     
#define GL_TEXTURE_CLIPMAP_DEPTH_SGIX     (GLenum)0x8176
#endif
#ifndef GL_MAX_CLIPMAP_DEPTH_SGIX         
#define GL_MAX_CLIPMAP_DEPTH_SGIX         (GLenum)0x8177
#endif
#ifndef GL_MAX_CLIPMAP_VIRTUAL_DEPTH_SGIX 
#define GL_MAX_CLIPMAP_VIRTUAL_DEPTH_SGIX (GLenum)0x8178
#endif
#ifndef GL_NEAREST_CLIPMAP_NEAREST_SGIX   
#define GL_NEAREST_CLIPMAP_NEAREST_SGIX   (GLenum)0x844D
#endif
#ifndef GL_NEAREST_CLIPMAP_LINEAR_SGIX    
#define GL_NEAREST_CLIPMAP_LINEAR_SGIX    (GLenum)0x844E
#endif
#ifndef GL_LINEAR_CLIPMAP_NEAREST_SGIX    
#define GL_LINEAR_CLIPMAP_NEAREST_SGIX    (GLenum)0x844F
#endif
#ifndef GL_TEXTURE_COMPARE_SGIX           
#define GL_TEXTURE_COMPARE_SGIX           (GLenum)0x819A
#endif
#ifndef GL_TEXTURE_COMPARE_OPERATOR_SGIX  
#define GL_TEXTURE_COMPARE_OPERATOR_SGIX  (GLenum)0x819B
#endif
#ifndef GL_TEXTURE_LEQUAL_R_SGIX          
#define GL_TEXTURE_LEQUAL_R_SGIX          (GLenum)0x819C
#endif
#ifndef GL_TEXTURE_GEQUAL_R_SGIX          
#define GL_TEXTURE_GEQUAL_R_SGIX          (GLenum)0x819D
#endif
#ifndef GL_CLAMP_TO_EDGE_SGIS             
#define GL_CLAMP_TO_EDGE_SGIS             (GLenum)0x812F
#endif
#ifndef GL_CLAMP_TO_BORDER_SGIS           
#define GL_CLAMP_TO_BORDER_SGIS           (GLenum)0x812D
#endif
#ifndef GL_FUNC_ADD_EXT                   
#define GL_FUNC_ADD_EXT                   (GLenum)0x8006
#endif
#ifndef GL_MIN_EXT                        
#define GL_MIN_EXT                        (GLenum)0x8007
#endif
#ifndef GL_MAX_EXT                        
#define GL_MAX_EXT                        (GLenum)0x8008
#endif
#ifndef GL_BLEND_EQUATION_EXT             
#define GL_BLEND_EQUATION_EXT             (GLenum)0x8009
#endif
#ifndef GL_FUNC_SUBTRACT_EXT              
#define GL_FUNC_SUBTRACT_EXT              (GLenum)0x800A
#endif
#ifndef GL_FUNC_REVERSE_SUBTRACT_EXT      
#define GL_FUNC_REVERSE_SUBTRACT_EXT      (GLenum)0x800B
#endif
#ifndef GL_INTERLACE_SGIX                 
#define GL_INTERLACE_SGIX                 (GLenum)0x8094
#endif
#ifndef GL_PIXEL_TILE_BEST_ALIGNMENT_SGIX 
#define GL_PIXEL_TILE_BEST_ALIGNMENT_SGIX (GLenum)0x813E
#endif
#ifndef GL_PIXEL_TILE_CACHE_INCREMENT_SGIX 
#define GL_PIXEL_TILE_CACHE_INCREMENT_SGIX (GLenum)0x813F
#endif
#ifndef GL_PIXEL_TILE_WIDTH_SGIX          
#define GL_PIXEL_TILE_WIDTH_SGIX          (GLenum)0x8140
#endif
#ifndef GL_PIXEL_TILE_HEIGHT_SGIX         
#define GL_PIXEL_TILE_HEIGHT_SGIX         (GLenum)0x8141
#endif
#ifndef GL_PIXEL_TILE_GRID_WIDTH_SGIX     
#define GL_PIXEL_TILE_GRID_WIDTH_SGIX     (GLenum)0x8142
#endif
#ifndef GL_PIXEL_TILE_GRID_HEIGHT_SGIX    
#define GL_PIXEL_TILE_GRID_HEIGHT_SGIX    (GLenum)0x8143
#endif
#ifndef GL_PIXEL_TILE_GRID_DEPTH_SGIX     
#define GL_PIXEL_TILE_GRID_DEPTH_SGIX     (GLenum)0x8144
#endif
#ifndef GL_PIXEL_TILE_CACHE_SIZE_SGIX     
#define GL_PIXEL_TILE_CACHE_SIZE_SGIX     (GLenum)0x8145
#endif
#ifndef GL_DUAL_ALPHA4_SGIS               
#define GL_DUAL_ALPHA4_SGIS               (GLenum)0x8110
#endif
#ifndef GL_DUAL_ALPHA8_SGIS               
#define GL_DUAL_ALPHA8_SGIS               (GLenum)0x8111
#endif
#ifndef GL_DUAL_ALPHA12_SGIS              
#define GL_DUAL_ALPHA12_SGIS              (GLenum)0x8112
#endif
#ifndef GL_DUAL_ALPHA16_SGIS              
#define GL_DUAL_ALPHA16_SGIS              (GLenum)0x8113
#endif
#ifndef GL_DUAL_LUMINANCE4_SGIS           
#define GL_DUAL_LUMINANCE4_SGIS           (GLenum)0x8114
#endif
#ifndef GL_DUAL_LUMINANCE8_SGIS           
#define GL_DUAL_LUMINANCE8_SGIS           (GLenum)0x8115
#endif
#ifndef GL_DUAL_LUMINANCE12_SGIS          
#define GL_DUAL_LUMINANCE12_SGIS          (GLenum)0x8116
#endif
#ifndef GL_DUAL_LUMINANCE16_SGIS          
#define GL_DUAL_LUMINANCE16_SGIS          (GLenum)0x8117
#endif
#ifndef GL_DUAL_INTENSITY4_SGIS           
#define GL_DUAL_INTENSITY4_SGIS           (GLenum)0x8118
#endif
#ifndef GL_DUAL_INTENSITY8_SGIS           
#define GL_DUAL_INTENSITY8_SGIS           (GLenum)0x8119
#endif
#ifndef GL_DUAL_INTENSITY12_SGIS          
#define GL_DUAL_INTENSITY12_SGIS          (GLenum)0x811A
#endif
#ifndef GL_DUAL_INTENSITY16_SGIS          
#define GL_DUAL_INTENSITY16_SGIS          (GLenum)0x811B
#endif
#ifndef GL_DUAL_LUMINANCE_ALPHA4_SGIS     
#define GL_DUAL_LUMINANCE_ALPHA4_SGIS     (GLenum)0x811C
#endif
#ifndef GL_DUAL_LUMINANCE_ALPHA8_SGIS     
#define GL_DUAL_LUMINANCE_ALPHA8_SGIS     (GLenum)0x811D
#endif
#ifndef GL_QUAD_ALPHA4_SGIS               
#define GL_QUAD_ALPHA4_SGIS               (GLenum)0x811E
#endif
#ifndef GL_QUAD_ALPHA8_SGIS               
#define GL_QUAD_ALPHA8_SGIS               (GLenum)0x811F
#endif
#ifndef GL_QUAD_LUMINANCE4_SGIS           
#define GL_QUAD_LUMINANCE4_SGIS           (GLenum)0x8120
#endif
#ifndef GL_QUAD_LUMINANCE8_SGIS           
#define GL_QUAD_LUMINANCE8_SGIS           (GLenum)0x8121
#endif
#ifndef GL_QUAD_INTENSITY4_SGIS           
#define GL_QUAD_INTENSITY4_SGIS           (GLenum)0x8122
#endif
#ifndef GL_QUAD_INTENSITY8_SGIS           
#define GL_QUAD_INTENSITY8_SGIS           (GLenum)0x8123
#endif
#ifndef GL_DUAL_TEXTURE_SELECT_SGIS       
#define GL_DUAL_TEXTURE_SELECT_SGIS       (GLenum)0x8124
#endif
#ifndef GL_QUAD_TEXTURE_SELECT_SGIS       
#define GL_QUAD_TEXTURE_SELECT_SGIS       (GLenum)0x8125
#endif
#ifndef GL_SPRITE_SGIX                    
#define GL_SPRITE_SGIX                    (GLenum)0x8148
#endif
#ifndef GL_SPRITE_MODE_SGIX               
#define GL_SPRITE_MODE_SGIX               (GLenum)0x8149
#endif
#ifndef GL_SPRITE_AXIS_SGIX               
#define GL_SPRITE_AXIS_SGIX               (GLenum)0x814A
#endif
#ifndef GL_SPRITE_TRANSLATION_SGIX        
#define GL_SPRITE_TRANSLATION_SGIX        (GLenum)0x814B
#endif
#ifndef GL_SPRITE_AXIAL_SGIX              
#define GL_SPRITE_AXIAL_SGIX              (GLenum)0x814C
#endif
#ifndef GL_SPRITE_OBJECT_ALIGNED_SGIX     
#define GL_SPRITE_OBJECT_ALIGNED_SGIX     (GLenum)0x814D
#endif
#ifndef GL_SPRITE_EYE_ALIGNED_SGIX        
#define GL_SPRITE_EYE_ALIGNED_SGIX        (GLenum)0x814E
#endif
#ifndef GL_TEXTURE_MULTI_BUFFER_HINT_SGIX 
#define GL_TEXTURE_MULTI_BUFFER_HINT_SGIX (GLenum)0x812E
#endif
#ifndef GL_POINT_SIZE_MIN_EXT             
#define GL_POINT_SIZE_MIN_EXT             (GLenum)0x8126
#endif
#ifndef GL_POINT_SIZE_MIN_SGIS            
#define GL_POINT_SIZE_MIN_SGIS            (GLenum)0x8126
#endif
#ifndef GL_POINT_SIZE_MAX_EXT             
#define GL_POINT_SIZE_MAX_EXT             (GLenum)0x8127
#endif
#ifndef GL_POINT_SIZE_MAX_SGIS            
#define GL_POINT_SIZE_MAX_SGIS            (GLenum)0x8127
#endif
#ifndef GL_POINT_FADE_THRESHOLD_SIZE_EXT  
#define GL_POINT_FADE_THRESHOLD_SIZE_EXT  (GLenum)0x8128
#endif
#ifndef GL_POINT_FADE_THRESHOLD_SIZE_SGIS 
#define GL_POINT_FADE_THRESHOLD_SIZE_SGIS (GLenum)0x8128
#endif
#ifndef GL_DISTANCE_ATTENUATION_EXT       
#define GL_DISTANCE_ATTENUATION_EXT       (GLenum)0x8129
#endif
#ifndef GL_DISTANCE_ATTENUATION_SGIS      
#define GL_DISTANCE_ATTENUATION_SGIS      (GLenum)0x8129
#endif
#ifndef GL_INSTRUMENT_BUFFER_POINTER_SGIX 
#define GL_INSTRUMENT_BUFFER_POINTER_SGIX (GLenum)0x8180
#endif
#ifndef GL_INSTRUMENT_MEASUREMENTS_SGIX   
#define GL_INSTRUMENT_MEASUREMENTS_SGIX   (GLenum)0x8181
#endif
#ifndef GL_POST_TEXTURE_FILTER_BIAS_SGIX  
#define GL_POST_TEXTURE_FILTER_BIAS_SGIX  (GLenum)0x8179
#endif
#ifndef GL_POST_TEXTURE_FILTER_SCALE_SGIX 
#define GL_POST_TEXTURE_FILTER_SCALE_SGIX (GLenum)0x817A
#endif
#ifndef GL_POST_TEXTURE_FILTER_BIAS_RANGE_SGIX 
#define GL_POST_TEXTURE_FILTER_BIAS_RANGE_SGIX (GLenum)0x817B
#endif
#ifndef GL_POST_TEXTURE_FILTER_SCALE_RANGE_SGIX 
#define GL_POST_TEXTURE_FILTER_SCALE_RANGE_SGIX (GLenum)0x817C
#endif
#ifndef GL_FRAMEZOOM_SGIX                 
#define GL_FRAMEZOOM_SGIX                 (GLenum)0x818B
#endif
#ifndef GL_FRAMEZOOM_FACTOR_SGIX          
#define GL_FRAMEZOOM_FACTOR_SGIX          (GLenum)0x818C
#endif
#ifndef GL_MAX_FRAMEZOOM_FACTOR_SGIX      
#define GL_MAX_FRAMEZOOM_FACTOR_SGIX      (GLenum)0x818D
#endif
#ifndef GL_REFERENCE_PLANE_SGIX           
#define GL_REFERENCE_PLANE_SGIX           (GLenum)0x817D
#endif
#ifndef GL_REFERENCE_PLANE_EQUATION_SGIX  
#define GL_REFERENCE_PLANE_EQUATION_SGIX  (GLenum)0x817E
#endif
#ifndef GL_DEPTH_COMPONENT16_SGIX         
#define GL_DEPTH_COMPONENT16_SGIX         (GLenum)0x81A5
#endif
#ifndef GL_DEPTH_COMPONENT24_SGIX         
#define GL_DEPTH_COMPONENT24_SGIX         (GLenum)0x81A6
#endif
#ifndef GL_DEPTH_COMPONENT32_SGIX         
#define GL_DEPTH_COMPONENT32_SGIX         (GLenum)0x81A7
#endif
#ifndef GL_FOG_FUNC_SGIS                  
#define GL_FOG_FUNC_SGIS                  (GLenum)0x812A
#endif
#ifndef GL_FOG_FUNC_POINTS_SGIS           
#define GL_FOG_FUNC_POINTS_SGIS           (GLenum)0x812B
#endif
#ifndef GL_MAX_FOG_FUNC_POINTS_SGIS       
#define GL_MAX_FOG_FUNC_POINTS_SGIS       (GLenum)0x812C
#endif
#ifndef GL_FOG_OFFSET_SGIX                
#define GL_FOG_OFFSET_SGIX                (GLenum)0x8198
#endif
#ifndef GL_FOG_OFFSET_VALUE_SGIX          
#define GL_FOG_OFFSET_VALUE_SGIX          (GLenum)0x8199
#endif
#ifndef GL_IMAGE_SCALE_X_HP               
#define GL_IMAGE_SCALE_X_HP               (GLenum)0x8155
#endif
#ifndef GL_IMAGE_SCALE_Y_HP               
#define GL_IMAGE_SCALE_Y_HP               (GLenum)0x8156
#endif
#ifndef GL_IMAGE_TRANSLATE_X_HP           
#define GL_IMAGE_TRANSLATE_X_HP           (GLenum)0x8157
#endif
#ifndef GL_IMAGE_TRANSLATE_Y_HP           
#define GL_IMAGE_TRANSLATE_Y_HP           (GLenum)0x8158
#endif
#ifndef GL_IMAGE_ROTATE_ANGLE_HP          
#define GL_IMAGE_ROTATE_ANGLE_HP          (GLenum)0x8159
#endif
#ifndef GL_IMAGE_ROTATE_ORIGIN_X_HP       
#define GL_IMAGE_ROTATE_ORIGIN_X_HP       (GLenum)0x815A
#endif
#ifndef GL_IMAGE_ROTATE_ORIGIN_Y_HP       
#define GL_IMAGE_ROTATE_ORIGIN_Y_HP       (GLenum)0x815B
#endif
#ifndef GL_IMAGE_MAG_FILTER_HP            
#define GL_IMAGE_MAG_FILTER_HP            (GLenum)0x815C
#endif
#ifndef GL_IMAGE_MIN_FILTER_HP            
#define GL_IMAGE_MIN_FILTER_HP            (GLenum)0x815D
#endif
#ifndef GL_IMAGE_CUBIC_WEIGHT_HP          
#define GL_IMAGE_CUBIC_WEIGHT_HP          (GLenum)0x815E
#endif
#ifndef GL_CUBIC_HP                       
#define GL_CUBIC_HP                       (GLenum)0x815F
#endif
#ifndef GL_AVERAGE_HP                     
#define GL_AVERAGE_HP                     (GLenum)0x8160
#endif
#ifndef GL_IMAGE_TRANSFORM_2D_HP          
#define GL_IMAGE_TRANSFORM_2D_HP          (GLenum)0x8161
#endif
#ifndef GL_POST_IMAGE_TRANSFORM_COLOR_TABLE_HP 
#define GL_POST_IMAGE_TRANSFORM_COLOR_TABLE_HP (GLenum)0x8162
#endif
#ifndef GL_PROXY_POST_IMAGE_TRANSFORM_COLOR_TABLE_HP 
#define GL_PROXY_POST_IMAGE_TRANSFORM_COLOR_TABLE_HP (GLenum)0x8163
#endif
#ifndef GL_IGNORE_BORDER_HP               
#define GL_IGNORE_BORDER_HP               (GLenum)0x8150
#endif
#ifndef GL_CONSTANT_BORDER_HP             
#define GL_CONSTANT_BORDER_HP             (GLenum)0x8151
#endif
#ifndef GL_REPLICATE_BORDER_HP            
#define GL_REPLICATE_BORDER_HP            (GLenum)0x8153
#endif
#ifndef GL_CONVOLUTION_BORDER_COLOR_HP    
#define GL_CONVOLUTION_BORDER_COLOR_HP    (GLenum)0x8154
#endif
#ifndef GL_TEXTURE_ENV_BIAS_SGIX          
#define GL_TEXTURE_ENV_BIAS_SGIX          (GLenum)0x80BE
#endif
#ifndef GL_VERTEX_DATA_HINT_PGI           
#define GL_VERTEX_DATA_HINT_PGI           (GLenum)0x1A22A
#endif
#ifndef GL_VERTEX_CONSISTENT_HINT_PGI     
#define GL_VERTEX_CONSISTENT_HINT_PGI     (GLenum)0x1A22B
#endif
#ifndef GL_MATERIAL_SIDE_HINT_PGI         
#define GL_MATERIAL_SIDE_HINT_PGI         (GLenum)0x1A22C
#endif
#ifndef GL_MAX_VERTEX_HINT_PGI            
#define GL_MAX_VERTEX_HINT_PGI            (GLenum)0x1A22D
#endif
#ifndef GL_COLOR3_BIT_PGI                 
#define GL_COLOR3_BIT_PGI                 (GLenum)0x00010000
#endif
#ifndef GL_COLOR4_BIT_PGI                 
#define GL_COLOR4_BIT_PGI                 (GLenum)0x00020000
#endif
#ifndef GL_EDGEFLAG_BIT_PGI               
#define GL_EDGEFLAG_BIT_PGI               (GLenum)0x00040000
#endif
#ifndef GL_INDEX_BIT_PGI                  
#define GL_INDEX_BIT_PGI                  (GLenum)0x00080000
#endif
#ifndef GL_MAT_AMBIENT_BIT_PGI            
#define GL_MAT_AMBIENT_BIT_PGI            (GLenum)0x00100000
#endif
#ifndef GL_MAT_AMBIENT_AND_DIFFUSE_BIT_PGI 
#define GL_MAT_AMBIENT_AND_DIFFUSE_BIT_PGI (GLenum)0x00200000
#endif
#ifndef GL_MAT_DIFFUSE_BIT_PGI            
#define GL_MAT_DIFFUSE_BIT_PGI            (GLenum)0x00400000
#endif
#ifndef GL_MAT_EMISSION_BIT_PGI           
#define GL_MAT_EMISSION_BIT_PGI           (GLenum)0x00800000
#endif
#ifndef GL_MAT_COLOR_INDEXES_BIT_PGI      
#define GL_MAT_COLOR_INDEXES_BIT_PGI      (GLenum)0x01000000
#endif
#ifndef GL_MAT_SHININESS_BIT_PGI          
#define GL_MAT_SHININESS_BIT_PGI          (GLenum)0x02000000
#endif
#ifndef GL_MAT_SPECULAR_BIT_PGI           
#define GL_MAT_SPECULAR_BIT_PGI           (GLenum)0x04000000
#endif
#ifndef GL_NORMAL_BIT_PGI                 
#define GL_NORMAL_BIT_PGI                 (GLenum)0x08000000
#endif
#ifndef GL_TEXCOORD1_BIT_PGI              
#define GL_TEXCOORD1_BIT_PGI              (GLenum)0x10000000
#endif
#ifndef GL_TEXCOORD2_BIT_PGI              
#define GL_TEXCOORD2_BIT_PGI              (GLenum)0x20000000
#endif
#ifndef GL_TEXCOORD3_BIT_PGI              
#define GL_TEXCOORD3_BIT_PGI              (GLenum)0x40000000
#endif
#ifndef GL_TEXCOORD4_BIT_PGI              
#define GL_TEXCOORD4_BIT_PGI              (GLenum)0x80000000
#endif
#ifndef GL_VERTEX23_BIT_PGI               
#define GL_VERTEX23_BIT_PGI               (GLenum)0x00000004
#endif
#ifndef GL_VERTEX4_BIT_PGI                
#define GL_VERTEX4_BIT_PGI                (GLenum)0x00000008
#endif
#ifndef GL_PREFER_DOUBLEBUFFER_HINT_PGI   
#define GL_PREFER_DOUBLEBUFFER_HINT_PGI   (GLenum)0x1A1F8
#endif
#ifndef GL_CONSERVE_MEMORY_HINT_PGI       
#define GL_CONSERVE_MEMORY_HINT_PGI       (GLenum)0x1A1FD
#endif
#ifndef GL_RECLAIM_MEMORY_HINT_PGI        
#define GL_RECLAIM_MEMORY_HINT_PGI        (GLenum)0x1A1FE
#endif
#ifndef GL_NATIVE_GRAPHICS_HANDLE_PGI     
#define GL_NATIVE_GRAPHICS_HANDLE_PGI     (GLenum)0x1A202
#endif
#ifndef GL_NATIVE_GRAPHICS_BEGIN_HINT_PGI 
#define GL_NATIVE_GRAPHICS_BEGIN_HINT_PGI (GLenum)0x1A203
#endif
#ifndef GL_NATIVE_GRAPHICS_END_HINT_PGI   
#define GL_NATIVE_GRAPHICS_END_HINT_PGI   (GLenum)0x1A204
#endif
#ifndef GL_ALWAYS_FAST_HINT_PGI           
#define GL_ALWAYS_FAST_HINT_PGI           (GLenum)0x1A20C
#endif
#ifndef GL_ALWAYS_SOFT_HINT_PGI           
#define GL_ALWAYS_SOFT_HINT_PGI           (GLenum)0x1A20D
#endif
#ifndef GL_ALLOW_DRAW_OBJ_HINT_PGI        
#define GL_ALLOW_DRAW_OBJ_HINT_PGI        (GLenum)0x1A20E
#endif
#ifndef GL_ALLOW_DRAW_WIN_HINT_PGI        
#define GL_ALLOW_DRAW_WIN_HINT_PGI        (GLenum)0x1A20F
#endif
#ifndef GL_ALLOW_DRAW_FRG_HINT_PGI        
#define GL_ALLOW_DRAW_FRG_HINT_PGI        (GLenum)0x1A210
#endif
#ifndef GL_ALLOW_DRAW_MEM_HINT_PGI        
#define GL_ALLOW_DRAW_MEM_HINT_PGI        (GLenum)0x1A211
#endif
#ifndef GL_STRICT_DEPTHFUNC_HINT_PGI      
#define GL_STRICT_DEPTHFUNC_HINT_PGI      (GLenum)0x1A216
#endif
#ifndef GL_STRICT_LIGHTING_HINT_PGI       
#define GL_STRICT_LIGHTING_HINT_PGI       (GLenum)0x1A217
#endif
#ifndef GL_STRICT_SCISSOR_HINT_PGI        
#define GL_STRICT_SCISSOR_HINT_PGI        (GLenum)0x1A218
#endif
#ifndef GL_FULL_STIPPLE_HINT_PGI          
#define GL_FULL_STIPPLE_HINT_PGI          (GLenum)0x1A219
#endif
#ifndef GL_CLIP_NEAR_HINT_PGI             
#define GL_CLIP_NEAR_HINT_PGI             (GLenum)0x1A220
#endif
#ifndef GL_CLIP_FAR_HINT_PGI              
#define GL_CLIP_FAR_HINT_PGI              (GLenum)0x1A221
#endif
#ifndef GL_WIDE_LINE_HINT_PGI             
#define GL_WIDE_LINE_HINT_PGI             (GLenum)0x1A222
#endif
#ifndef GL_BACK_NORMALS_HINT_PGI          
#define GL_BACK_NORMALS_HINT_PGI          (GLenum)0x1A223
#endif
#ifndef GL_COLOR_INDEX1_EXT               
#define GL_COLOR_INDEX1_EXT               (GLenum)0x80E2
#endif
#ifndef GL_COLOR_INDEX2_EXT               
#define GL_COLOR_INDEX2_EXT               (GLenum)0x80E3
#endif
#ifndef GL_COLOR_INDEX4_EXT               
#define GL_COLOR_INDEX4_EXT               (GLenum)0x80E4
#endif
#ifndef GL_COLOR_INDEX8_EXT               
#define GL_COLOR_INDEX8_EXT               (GLenum)0x80E5
#endif
#ifndef GL_COLOR_INDEX12_EXT              
#define GL_COLOR_INDEX12_EXT              (GLenum)0x80E6
#endif
#ifndef GL_COLOR_INDEX16_EXT              
#define GL_COLOR_INDEX16_EXT              (GLenum)0x80E7
#endif
#ifndef GL_TEXTURE_INDEX_SIZE_EXT         
#define GL_TEXTURE_INDEX_SIZE_EXT         (GLenum)0x80ED
#endif
#ifndef GL_CLIP_VOLUME_CLIPPING_HINT_EXT  
#define GL_CLIP_VOLUME_CLIPPING_HINT_EXT  (GLenum)0x80F0
#endif
#ifndef GL_LIST_PRIORITY_SGIX             
#define GL_LIST_PRIORITY_SGIX             (GLenum)0x8182
#endif
#ifndef GL_IR_INSTRUMENT1_SGIX            
#define GL_IR_INSTRUMENT1_SGIX            (GLenum)0x817F
#endif
#ifndef GL_CALLIGRAPHIC_FRAGMENT_SGIX     
#define GL_CALLIGRAPHIC_FRAGMENT_SGIX     (GLenum)0x8183
#endif
#ifndef GL_TEXTURE_LOD_BIAS_S_SGIX        
#define GL_TEXTURE_LOD_BIAS_S_SGIX        (GLenum)0x818E
#endif
#ifndef GL_TEXTURE_LOD_BIAS_T_SGIX        
#define GL_TEXTURE_LOD_BIAS_T_SGIX        (GLenum)0x818F
#endif
#ifndef GL_TEXTURE_LOD_BIAS_R_SGIX        
#define GL_TEXTURE_LOD_BIAS_R_SGIX        (GLenum)0x8190
#endif
#ifndef GL_SHADOW_AMBIENT_SGIX            
#define GL_SHADOW_AMBIENT_SGIX            (GLenum)0x80BF
#endif
#ifndef GL_INDEX_MATERIAL_EXT             
#define GL_INDEX_MATERIAL_EXT             (GLenum)0x81B8
#endif
#ifndef GL_INDEX_MATERIAL_PARAMETER_EXT   
#define GL_INDEX_MATERIAL_PARAMETER_EXT   (GLenum)0x81B9
#endif
#ifndef GL_INDEX_MATERIAL_FACE_EXT        
#define GL_INDEX_MATERIAL_FACE_EXT        (GLenum)0x81BA
#endif
#ifndef GL_INDEX_TEST_EXT                 
#define GL_INDEX_TEST_EXT                 (GLenum)0x81B5
#endif
#ifndef GL_INDEX_TEST_FUNC_EXT            
#define GL_INDEX_TEST_FUNC_EXT            (GLenum)0x81B6
#endif
#ifndef GL_INDEX_TEST_REF_EXT             
#define GL_INDEX_TEST_REF_EXT             (GLenum)0x81B7
#endif
#ifndef GL_IUI_V2F_EXT                    
#define GL_IUI_V2F_EXT                    (GLenum)0x81AD
#endif
#ifndef GL_IUI_V3F_EXT                    
#define GL_IUI_V3F_EXT                    (GLenum)0x81AE
#endif
#ifndef GL_IUI_N3F_V2F_EXT                
#define GL_IUI_N3F_V2F_EXT                (GLenum)0x81AF
#endif
#ifndef GL_IUI_N3F_V3F_EXT                
#define GL_IUI_N3F_V3F_EXT                (GLenum)0x81B0
#endif
#ifndef GL_T2F_IUI_V2F_EXT                
#define GL_T2F_IUI_V2F_EXT                (GLenum)0x81B1
#endif
#ifndef GL_T2F_IUI_V3F_EXT                
#define GL_T2F_IUI_V3F_EXT                (GLenum)0x81B2
#endif
#ifndef GL_T2F_IUI_N3F_V2F_EXT            
#define GL_T2F_IUI_N3F_V2F_EXT            (GLenum)0x81B3
#endif
#ifndef GL_T2F_IUI_N3F_V3F_EXT            
#define GL_T2F_IUI_N3F_V3F_EXT            (GLenum)0x81B4
#endif
#ifndef GL_ARRAY_ELEMENT_LOCK_FIRST_EXT   
#define GL_ARRAY_ELEMENT_LOCK_FIRST_EXT   (GLenum)0x81A8
#endif
#ifndef GL_ARRAY_ELEMENT_LOCK_COUNT_EXT   
#define GL_ARRAY_ELEMENT_LOCK_COUNT_EXT   (GLenum)0x81A9
#endif
#ifndef GL_CULL_VERTEX_EXT                
#define GL_CULL_VERTEX_EXT                (GLenum)0x81AA
#endif
#ifndef GL_CULL_VERTEX_EYE_POSITION_EXT   
#define GL_CULL_VERTEX_EYE_POSITION_EXT   (GLenum)0x81AB
#endif
#ifndef GL_CULL_VERTEX_OBJECT_POSITION_EXT 
#define GL_CULL_VERTEX_OBJECT_POSITION_EXT (GLenum)0x81AC
#endif
#ifndef GL_YCRCB_422_SGIX                 
#define GL_YCRCB_422_SGIX                 (GLenum)0x81BB
#endif
#ifndef GL_YCRCB_444_SGIX                 
#define GL_YCRCB_444_SGIX                 (GLenum)0x81BC
#endif
#ifndef GL_FRAGMENT_LIGHTING_SGIX         
#define GL_FRAGMENT_LIGHTING_SGIX         (GLenum)0x8400
#endif
#ifndef GL_FRAGMENT_COLOR_MATERIAL_SGIX   
#define GL_FRAGMENT_COLOR_MATERIAL_SGIX   (GLenum)0x8401
#endif
#ifndef GL_FRAGMENT_COLOR_MATERIAL_FACE_SGIX 
#define GL_FRAGMENT_COLOR_MATERIAL_FACE_SGIX (GLenum)0x8402
#endif
#ifndef GL_FRAGMENT_COLOR_MATERIAL_PARAMETER_SGIX 
#define GL_FRAGMENT_COLOR_MATERIAL_PARAMETER_SGIX (GLenum)0x8403
#endif
#ifndef GL_MAX_FRAGMENT_LIGHTS_SGIX       
#define GL_MAX_FRAGMENT_LIGHTS_SGIX       (GLenum)0x8404
#endif
#ifndef GL_MAX_ACTIVE_LIGHTS_SGIX         
#define GL_MAX_ACTIVE_LIGHTS_SGIX         (GLenum)0x8405
#endif
#ifndef GL_CURRENT_RASTER_NORMAL_SGIX     
#define GL_CURRENT_RASTER_NORMAL_SGIX     (GLenum)0x8406
#endif
#ifndef GL_LIGHT_ENV_MODE_SGIX            
#define GL_LIGHT_ENV_MODE_SGIX            (GLenum)0x8407
#endif
#ifndef GL_FRAGMENT_LIGHT_MODEL_LOCAL_VIEWER_SGIX 
#define GL_FRAGMENT_LIGHT_MODEL_LOCAL_VIEWER_SGIX (GLenum)0x8408
#endif
#ifndef GL_FRAGMENT_LIGHT_MODEL_TWO_SIDE_SGIX 
#define GL_FRAGMENT_LIGHT_MODEL_TWO_SIDE_SGIX (GLenum)0x8409
#endif
#ifndef GL_FRAGMENT_LIGHT_MODEL_AMBIENT_SGIX 
#define GL_FRAGMENT_LIGHT_MODEL_AMBIENT_SGIX (GLenum)0x840A
#endif
#ifndef GL_FRAGMENT_LIGHT_MODEL_NORMAL_INTERPOLATION_SGIX 
#define GL_FRAGMENT_LIGHT_MODEL_NORMAL_INTERPOLATION_SGIX (GLenum)0x840B
#endif
#ifndef GL_FRAGMENT_LIGHT0_SGIX           
#define GL_FRAGMENT_LIGHT0_SGIX           (GLenum)0x840C
#endif
#ifndef GL_FRAGMENT_LIGHT1_SGIX           
#define GL_FRAGMENT_LIGHT1_SGIX           (GLenum)0x840D
#endif
#ifndef GL_FRAGMENT_LIGHT2_SGIX           
#define GL_FRAGMENT_LIGHT2_SGIX           (GLenum)0x840E
#endif
#ifndef GL_FRAGMENT_LIGHT3_SGIX           
#define GL_FRAGMENT_LIGHT3_SGIX           (GLenum)0x840F
#endif
#ifndef GL_FRAGMENT_LIGHT4_SGIX           
#define GL_FRAGMENT_LIGHT4_SGIX           (GLenum)0x8410
#endif
#ifndef GL_FRAGMENT_LIGHT5_SGIX           
#define GL_FRAGMENT_LIGHT5_SGIX           (GLenum)0x8411
#endif
#ifndef GL_FRAGMENT_LIGHT6_SGIX           
#define GL_FRAGMENT_LIGHT6_SGIX           (GLenum)0x8412
#endif
#ifndef GL_FRAGMENT_LIGHT7_SGIX           
#define GL_FRAGMENT_LIGHT7_SGIX           (GLenum)0x8413
#endif
#ifndef GL_RASTER_POSITION_UNCLIPPED_IBM  
#define GL_RASTER_POSITION_UNCLIPPED_IBM  (GLenum)0x19262
#endif
#ifndef GL_TEXTURE_LIGHTING_MODE_HP       
#define GL_TEXTURE_LIGHTING_MODE_HP       (GLenum)0x8167
#endif
#ifndef GL_TEXTURE_POST_SPECULAR_HP       
#define GL_TEXTURE_POST_SPECULAR_HP       (GLenum)0x8168
#endif
#ifndef GL_TEXTURE_PRE_SPECULAR_HP        
#define GL_TEXTURE_PRE_SPECULAR_HP        (GLenum)0x8169
#endif
#ifndef GL_MAX_ELEMENTS_VERTICES_EXT      
#define GL_MAX_ELEMENTS_VERTICES_EXT      (GLenum)0x80E8
#endif
#ifndef GL_MAX_ELEMENTS_INDICES_EXT       
#define GL_MAX_ELEMENTS_INDICES_EXT       (GLenum)0x80E9
#endif
#ifndef GL_PHONG_WIN                      
#define GL_PHONG_WIN                      (GLenum)0x80EA
#endif
#ifndef GL_PHONG_HINT_WIN                 
#define GL_PHONG_HINT_WIN                 (GLenum)0x80EB
#endif
#ifndef GL_FOG_SPECULAR_TEXTURE_WIN       
#define GL_FOG_SPECULAR_TEXTURE_WIN       (GLenum)0x80EC
#endif
#ifndef GL_FRAGMENT_MATERIAL_EXT          
#define GL_FRAGMENT_MATERIAL_EXT          (GLenum)0x8349
#endif
#ifndef GL_FRAGMENT_NORMAL_EXT            
#define GL_FRAGMENT_NORMAL_EXT            (GLenum)0x834A
#endif
#ifndef GL_FRAGMENT_COLOR_EXT             
#define GL_FRAGMENT_COLOR_EXT             (GLenum)0x834C
#endif
#ifndef GL_ATTENUATION_EXT                
#define GL_ATTENUATION_EXT                (GLenum)0x834D
#endif
#ifndef GL_SHADOW_ATTENUATION_EXT         
#define GL_SHADOW_ATTENUATION_EXT         (GLenum)0x834E
#endif
#ifndef GL_TEXTURE_APPLICATION_MODE_EXT   
#define GL_TEXTURE_APPLICATION_MODE_EXT   (GLenum)0x834F
#endif
#ifndef GL_TEXTURE_LIGHT_EXT              
#define GL_TEXTURE_LIGHT_EXT              (GLenum)0x8350
#endif
#ifndef GL_TEXTURE_MATERIAL_FACE_EXT      
#define GL_TEXTURE_MATERIAL_FACE_EXT      (GLenum)0x8351
#endif
#ifndef GL_TEXTURE_MATERIAL_PARAMETER_EXT 
#define GL_TEXTURE_MATERIAL_PARAMETER_EXT (GLenum)0x8352
#endif
#ifndef GL_ALPHA_MIN_SGIX                 
#define GL_ALPHA_MIN_SGIX                 (GLenum)0x8320
#endif
#ifndef GL_ALPHA_MAX_SGIX                 
#define GL_ALPHA_MAX_SGIX                 (GLenum)0x8321
#endif
#ifndef GL_BGR_EXT                        
#define GL_BGR_EXT                        (GLenum)0x80E0
#endif
#ifndef GL_BGRA_EXT                       
#define GL_BGRA_EXT                       (GLenum)0x80E1
#endif
#ifndef GL_PARALLEL_ARRAYS_INTEL          
#define GL_PARALLEL_ARRAYS_INTEL          (GLenum)0x83F4
#endif
#ifndef GL_VERTEX_ARRAY_PARALLEL_POINTERS_INTEL 
#define GL_VERTEX_ARRAY_PARALLEL_POINTERS_INTEL (GLenum)0x83F5
#endif
#ifndef GL_NORMAL_ARRAY_PARALLEL_POINTERS_INTEL 
#define GL_NORMAL_ARRAY_PARALLEL_POINTERS_INTEL (GLenum)0x83F6
#endif
#ifndef GL_COLOR_ARRAY_PARALLEL_POINTERS_INTEL 
#define GL_COLOR_ARRAY_PARALLEL_POINTERS_INTEL (GLenum)0x83F7
#endif
#ifndef GL_TEXTURE_COORD_ARRAY_PARALLEL_POINTERS_INTEL 
#define GL_TEXTURE_COORD_ARRAY_PARALLEL_POINTERS_INTEL (GLenum)0x83F8
#endif
#ifndef GL_OCCLUSION_TEST_HP              
#define GL_OCCLUSION_TEST_HP              (GLenum)0x8165
#endif
#ifndef GL_OCCLUSION_TEST_RESULT_HP       
#define GL_OCCLUSION_TEST_RESULT_HP       (GLenum)0x8166
#endif
#ifndef GL_PIXEL_TRANSFORM_2D_EXT         
#define GL_PIXEL_TRANSFORM_2D_EXT         (GLenum)0x8330
#endif
#ifndef GL_PIXEL_MAG_FILTER_EXT           
#define GL_PIXEL_MAG_FILTER_EXT           (GLenum)0x8331
#endif
#ifndef GL_PIXEL_MIN_FILTER_EXT           
#define GL_PIXEL_MIN_FILTER_EXT           (GLenum)0x8332
#endif
#ifndef GL_PIXEL_CUBIC_WEIGHT_EXT         
#define GL_PIXEL_CUBIC_WEIGHT_EXT         (GLenum)0x8333
#endif
#ifndef GL_CUBIC_EXT                      
#define GL_CUBIC_EXT                      (GLenum)0x8334
#endif
#ifndef GL_AVERAGE_EXT                    
#define GL_AVERAGE_EXT                    (GLenum)0x8335
#endif
#ifndef GL_PIXEL_TRANSFORM_2D_STACK_DEPTH_EXT 
#define GL_PIXEL_TRANSFORM_2D_STACK_DEPTH_EXT (GLenum)0x8336
#endif
#ifndef GL_MAX_PIXEL_TRANSFORM_2D_STACK_DEPTH_EXT 
#define GL_MAX_PIXEL_TRANSFORM_2D_STACK_DEPTH_EXT (GLenum)0x8337
#endif
#ifndef GL_PIXEL_TRANSFORM_2D_MATRIX_EXT  
#define GL_PIXEL_TRANSFORM_2D_MATRIX_EXT  (GLenum)0x8338
#endif
#ifndef GL_SHARED_TEXTURE_PALETTE_EXT     
#define GL_SHARED_TEXTURE_PALETTE_EXT     (GLenum)0x81FB
#endif
#ifndef GL_LIGHT_MODEL_COLOR_CONTROL_EXT  
#define GL_LIGHT_MODEL_COLOR_CONTROL_EXT  (GLenum)0x81F8
#endif
#ifndef GL_SINGLE_COLOR_EXT               
#define GL_SINGLE_COLOR_EXT               (GLenum)0x81F9
#endif
#ifndef GL_SEPARATE_SPECULAR_COLOR_EXT    
#define GL_SEPARATE_SPECULAR_COLOR_EXT    (GLenum)0x81FA
#endif
#ifndef GL_COLOR_SUM_EXT                  
#define GL_COLOR_SUM_EXT                  (GLenum)0x8458
#endif
#ifndef GL_CURRENT_SECONDARY_COLOR_EXT    
#define GL_CURRENT_SECONDARY_COLOR_EXT    (GLenum)0x8459
#endif
#ifndef GL_SECONDARY_COLOR_ARRAY_SIZE_EXT 
#define GL_SECONDARY_COLOR_ARRAY_SIZE_EXT (GLenum)0x845A
#endif
#ifndef GL_SECONDARY_COLOR_ARRAY_TYPE_EXT 
#define GL_SECONDARY_COLOR_ARRAY_TYPE_EXT (GLenum)0x845B
#endif
#ifndef GL_SECONDARY_COLOR_ARRAY_STRIDE_EXT 
#define GL_SECONDARY_COLOR_ARRAY_STRIDE_EXT (GLenum)0x845C
#endif
#ifndef GL_SECONDARY_COLOR_ARRAY_POINTER_EXT 
#define GL_SECONDARY_COLOR_ARRAY_POINTER_EXT (GLenum)0x845D
#endif
#ifndef GL_SECONDARY_COLOR_ARRAY_EXT      
#define GL_SECONDARY_COLOR_ARRAY_EXT      (GLenum)0x845E
#endif
#ifndef GL_PERTURB_EXT                    
#define GL_PERTURB_EXT                    (GLenum)0x85AE
#endif
#ifndef GL_TEXTURE_NORMAL_EXT             
#define GL_TEXTURE_NORMAL_EXT             (GLenum)0x85AF
#endif
#ifndef GL_FOG_COORDINATE_SOURCE_EXT      
#define GL_FOG_COORDINATE_SOURCE_EXT      (GLenum)0x8450
#endif
#ifndef GL_FOG_COORDINATE_EXT             
#define GL_FOG_COORDINATE_EXT             (GLenum)0x8451
#endif
#ifndef GL_FRAGMENT_DEPTH_EXT             
#define GL_FRAGMENT_DEPTH_EXT             (GLenum)0x8452
#endif
#ifndef GL_CURRENT_FOG_COORDINATE_EXT     
#define GL_CURRENT_FOG_COORDINATE_EXT     (GLenum)0x8453
#endif
#ifndef GL_FOG_COORDINATE_ARRAY_TYPE_EXT  
#define GL_FOG_COORDINATE_ARRAY_TYPE_EXT  (GLenum)0x8454
#endif
#ifndef GL_FOG_COORDINATE_ARRAY_STRIDE_EXT 
#define GL_FOG_COORDINATE_ARRAY_STRIDE_EXT (GLenum)0x8455
#endif
#ifndef GL_FOG_COORDINATE_ARRAY_POINTER_EXT 
#define GL_FOG_COORDINATE_ARRAY_POINTER_EXT (GLenum)0x8456
#endif
#ifndef GL_FOG_COORDINATE_ARRAY_EXT       
#define GL_FOG_COORDINATE_ARRAY_EXT       (GLenum)0x8457
#endif
#ifndef GL_SCREEN_COORDINATES_REND        
#define GL_SCREEN_COORDINATES_REND        (GLenum)0x8490
#endif
#ifndef GL_INVERTED_SCREEN_W_REND         
#define GL_INVERTED_SCREEN_W_REND         (GLenum)0x8491
#endif
#ifndef GL_TANGENT_ARRAY_EXT              
#define GL_TANGENT_ARRAY_EXT              (GLenum)0x8439
#endif
#ifndef GL_BINORMAL_ARRAY_EXT             
#define GL_BINORMAL_ARRAY_EXT             (GLenum)0x843A
#endif
#ifndef GL_CURRENT_TANGENT_EXT            
#define GL_CURRENT_TANGENT_EXT            (GLenum)0x843B
#endif
#ifndef GL_CURRENT_BINORMAL_EXT           
#define GL_CURRENT_BINORMAL_EXT           (GLenum)0x843C
#endif
#ifndef GL_TANGENT_ARRAY_TYPE_EXT         
#define GL_TANGENT_ARRAY_TYPE_EXT         (GLenum)0x843E
#endif
#ifndef GL_TANGENT_ARRAY_STRIDE_EXT       
#define GL_TANGENT_ARRAY_STRIDE_EXT       (GLenum)0x843F
#endif
#ifndef GL_BINORMAL_ARRAY_TYPE_EXT        
#define GL_BINORMAL_ARRAY_TYPE_EXT        (GLenum)0x8440
#endif
#ifndef GL_BINORMAL_ARRAY_STRIDE_EXT      
#define GL_BINORMAL_ARRAY_STRIDE_EXT      (GLenum)0x8441
#endif
#ifndef GL_TANGENT_ARRAY_POINTER_EXT      
#define GL_TANGENT_ARRAY_POINTER_EXT      (GLenum)0x8442
#endif
#ifndef GL_BINORMAL_ARRAY_POINTER_EXT     
#define GL_BINORMAL_ARRAY_POINTER_EXT     (GLenum)0x8443
#endif
#ifndef GL_MAP1_TANGENT_EXT               
#define GL_MAP1_TANGENT_EXT               (GLenum)0x8444
#endif
#ifndef GL_MAP2_TANGENT_EXT               
#define GL_MAP2_TANGENT_EXT               (GLenum)0x8445
#endif
#ifndef GL_MAP1_BINORMAL_EXT              
#define GL_MAP1_BINORMAL_EXT              (GLenum)0x8446
#endif
#ifndef GL_MAP2_BINORMAL_EXT              
#define GL_MAP2_BINORMAL_EXT              (GLenum)0x8447
#endif
#ifndef GL_COMBINE_EXT                    
#define GL_COMBINE_EXT                    (GLenum)0x8570
#endif
#ifndef GL_COMBINE_ARB                    
#define GL_COMBINE_ARB                    GL_COMBINE_EXT
#endif
#ifndef GL_COMBINE_RGB_EXT                
#define GL_COMBINE_RGB_EXT                (GLenum)0x8571
#endif
#ifndef GL_COMBINE_RGB_ARB                
#define GL_COMBINE_RGB_ARB                GL_COMBINE_RGB_EXT
#endif
#ifndef GL_COMBINE_ALPHA_EXT              
#define GL_COMBINE_ALPHA_EXT              (GLenum)0x8572
#endif
#ifndef GL_COMBINE_ALPHA_ARB              
#define GL_COMBINE_ALPHA_ARB              GL_COMBINE_ALPHA_EXT
#endif
#ifndef GL_RGB_SCALE_EXT                  
#define GL_RGB_SCALE_EXT                  (GLenum)0x8573
#endif
#ifndef GL_RGB_SCALE_ARB                  
#define GL_RGB_SCALE_ARB                  GL_RGB_SCALE_EXT
#endif
#ifndef GL_ADD_SIGNED_EXT                 
#define GL_ADD_SIGNED_EXT                 (GLenum)0x8574
#endif
#ifndef GL_ADD_SIGNED_ARB                 
#define GL_ADD_SIGNED_ARB                 GL_ADD_SIGNED_EXT
#endif
#ifndef GL_INTERPOLATE_EXT                
#define GL_INTERPOLATE_EXT                (GLenum)0x8575
#endif
#ifndef GL_INTERPOLATE_ARB                
#define GL_INTERPOLATE_ARB                GL_INTERPOLATE_EXT
#endif
#ifndef GL_CONSTANT_EXT                   
#define GL_CONSTANT_EXT                   (GLenum)0x8576
#endif
#ifndef GL_CONSTANT_ARB                   
#define GL_CONSTANT_ARB                   GL_CONSTANT_EXT
#endif
#ifndef GL_PRIMARY_COLOR_EXT              
#define GL_PRIMARY_COLOR_EXT              (GLenum)0x8577
#endif
#ifndef GL_PRIMARY_COLOR_ARB              
#define GL_PRIMARY_COLOR_ARB              GL_PRIMARY_COLOR_EXT
#endif
#ifndef GL_PREVIOUS_EXT                   
#define GL_PREVIOUS_EXT                   (GLenum)0x8578
#endif
#ifndef GL_PREVIOUS_ARB                   
#define GL_PREVIOUS_ARB  GL_PREVIOUS_EXT                   
#endif
#ifndef GL_SOURCE0_RGB_EXT                
#define GL_SOURCE0_RGB_EXT                (GLenum)0x8580
#endif
#ifndef GL_SOURCE0_RGB_ARB                
#define GL_SOURCE0_RGB_ARB  GL_SOURCE0_RGB_EXT                
#endif
#ifndef GL_SOURCE1_RGB_EXT                
#define GL_SOURCE1_RGB_EXT                (GLenum)0x8581
#endif
#ifndef GL_SOURCE1_RGB_ARB                
#define GL_SOURCE1_RGB_ARB  GL_SOURCE1_RGB_EXT                
#endif
#ifndef GL_SOURCE2_RGB_EXT                
#define GL_SOURCE2_RGB_EXT                (GLenum)0x8582
#endif
#ifndef GL_SOURCE2_RGB_ARB                
#define GL_SOURCE2_RGB_ARB  GL_SOURCE2_RGB_EXT                
#endif
#ifndef GL_SOURCE3_RGB_EXT                
#define GL_SOURCE3_RGB_EXT                (GLenum)0x8583
#endif
#ifndef GL_SOURCE3_RGB_ARB                
#define GL_SOURCE3_RGB_ARB  GL_SOURCE3_RGB_EXT                
#endif
#ifndef GL_SOURCE4_RGB_EXT                
#define GL_SOURCE4_RGB_EXT                (GLenum)0x8584
#endif
#ifndef GL_SOURCE4_RGB_ARB                
#define GL_SOURCE4_RGB_ARB  GL_SOURCE4_RGB_EXT                
#endif
#ifndef GL_SOURCE5_RGB_EXT                
#define GL_SOURCE5_RGB_EXT                (GLenum)0x8585
#endif
#ifndef GL_SOURCE5_RGB_ARB                
#define GL_SOURCE5_RGB_ARB  GL_SOURCE5_RGB_EXT                
#endif
#ifndef GL_SOURCE6_RGB_EXT                
#define GL_SOURCE6_RGB_EXT                (GLenum)0x8586
#endif
#ifndef GL_SOURCE6_RGB_ARB                
#define GL_SOURCE6_RGB_ARB  GL_SOURCE6_RGB_EXT                
#endif
#ifndef GL_SOURCE7_RGB_EXT                
#define GL_SOURCE7_RGB_EXT                (GLenum)0x8587
#endif
#ifndef GL_SOURCE7_RGB_ARB                
#define GL_SOURCE7_RGB_ARB  GL_SOURCE7_RGB_EXT                
#endif
#ifndef GL_SOURCE0_ALPHA_EXT              
#define GL_SOURCE0_ALPHA_EXT              (GLenum)0x8588
#endif
#ifndef GL_SOURCE0_ALPHA_ARB              
#define GL_SOURCE0_ALPHA_ARB  GL_SOURCE0_ALPHA_EXT              
#endif
#ifndef GL_SOURCE1_ALPHA_EXT              
#define GL_SOURCE1_ALPHA_EXT              (GLenum)0x8589
#endif
#ifndef GL_SOURCE1_ALPHA_ARB              
#define GL_SOURCE1_ALPHA_ARB  GL_SOURCE1_ALPHA_EXT              
#endif
#ifndef GL_SOURCE2_ALPHA_EXT              
#define GL_SOURCE2_ALPHA_EXT              (GLenum)0x858A
#endif
#ifndef GL_SOURCE2_ALPHA_ARB              
#define GL_SOURCE2_ALPHA_ARB  GL_SOURCE2_ALPHA_EXT              
#endif
#ifndef GL_SOURCE3_ALPHA_EXT              
#define GL_SOURCE3_ALPHA_EXT              (GLenum)0x858B
#endif
#ifndef GL_SOURCE3_ALPHA_ARB              
#define GL_SOURCE3_ALPHA_ARB  GL_SOURCE3_ALPHA_EXT              
#endif
#ifndef GL_SOURCE4_ALPHA_EXT              
#define GL_SOURCE4_ALPHA_EXT              (GLenum)0x858C
#endif
#ifndef GL_SOURCE4_ALPHA_ARB              
#define GL_SOURCE4_ALPHA_ARB  GL_SOURCE4_ALPHA_EXT              
#endif
#ifndef GL_SOURCE5_ALPHA_EXT              
#define GL_SOURCE5_ALPHA_EXT              (GLenum)0x858D
#endif
#ifndef GL_SOURCE5_ALPHA_ARB              
#define GL_SOURCE5_ALPHA_ARB  GL_SOURCE5_ALPHA_EXT              
#endif
#ifndef GL_SOURCE6_ALPHA_EXT              
#define GL_SOURCE6_ALPHA_EXT              (GLenum)0x858E
#endif
#ifndef GL_SOURCE6_ALPHA_ARB              
#define GL_SOURCE6_ALPHA_ARB  GL_SOURCE6_ALPHA_EXT              
#endif
#ifndef GL_SOURCE7_ALPHA_EXT              
#define GL_SOURCE7_ALPHA_EXT              (GLenum)0x858F
#endif
#ifndef GL_SOURCE7_ALPHA_ARB              
#define GL_SOURCE7_ALPHA_ARB  GL_SOURCE7_ALPHA_EXT              
#endif
#ifndef GL_OPERAND0_RGB_EXT               
#define GL_OPERAND0_RGB_EXT               (GLenum)0x8590
#endif
#ifndef GL_OPERAND0_RGB_ARB               
#define GL_OPERAND0_RGB_ARB  GL_OPERAND0_RGB_EXT               
#endif
#ifndef GL_OPERAND1_RGB_EXT               
#define GL_OPERAND1_RGB_EXT               (GLenum)0x8591
#endif
#ifndef GL_OPERAND1_RGB_ARB               
#define GL_OPERAND1_RGB_ARB  GL_OPERAND1_RGB_EXT               
#endif
#ifndef GL_OPERAND2_RGB_EXT               
#define GL_OPERAND2_RGB_EXT               (GLenum)0x8592
#endif
#ifndef GL_OPERAND2_RGB_ARB               
#define GL_OPERAND2_RGB_ARB  GL_OPERAND2_RGB_EXT               
#endif
#ifndef GL_OPERAND3_RGB_EXT               
#define GL_OPERAND3_RGB_EXT               (GLenum)0x8593
#endif
#ifndef GL_OPERAND3_RGB_ARB               
#define GL_OPERAND3_RGB_ARB  GL_OPERAND3_RGB_EXT               
#endif
#ifndef GL_OPERAND4_RGB_EXT               
#define GL_OPERAND4_RGB_EXT               (GLenum)0x8594
#endif
#ifndef GL_OPERAND4_RGB_ARB               
#define GL_OPERAND4_RGB_ARB  GL_OPERAND4_RGB_EXT               
#endif
#ifndef GL_OPERAND5_RGB_EXT               
#define GL_OPERAND5_RGB_EXT               (GLenum)0x8595
#endif
#ifndef GL_OPERAND5_RGB_ARB               
#define GL_OPERAND5_RGB_ARB  GL_OPERAND5_RGB_EXT               
#endif
#ifndef GL_OPERAND6_RGB_EXT               
#define GL_OPERAND6_RGB_EXT               (GLenum)0x8596
#endif
#ifndef GL_OPERAND6_RGB_ARB               
#define GL_OPERAND6_RGB_ARB  GL_OPERAND6_RGB_EXT               
#endif
#ifndef GL_OPERAND7_RGB_EXT               
#define GL_OPERAND7_RGB_EXT               (GLenum)0x8597
#endif
#ifndef GL_OPERAND7_RGB_ARB               
#define GL_OPERAND7_RGB_ARB  GL_OPERAND7_RGB_EXT               
#endif
#ifndef GL_OPERAND0_ALPHA_EXT             
#define GL_OPERAND0_ALPHA_EXT             (GLenum)0x8598
#endif
#ifndef GL_OPERAND0_ALPHA_ARB             
#define GL_OPERAND0_ALPHA_ARB  GL_OPERAND0_ALPHA_EXT             
#endif
#ifndef GL_OPERAND1_ALPHA_EXT             
#define GL_OPERAND1_ALPHA_EXT             (GLenum)0x8599
#endif
#ifndef GL_OPERAND1_ALPHA_ARB             
#define GL_OPERAND1_ALPHA_ARB  GL_OPERAND1_ALPHA_EXT             
#endif
#ifndef GL_OPERAND2_ALPHA_EXT             
#define GL_OPERAND2_ALPHA_EXT             (GLenum)0x859A
#endif
#ifndef GL_OPERAND2_ALPHA_ARB             
#define GL_OPERAND2_ALPHA_ARB  GL_OPERAND2_ALPHA_EXT             
#endif
#ifndef GL_OPERAND3_ALPHA_EXT             
#define GL_OPERAND3_ALPHA_EXT             (GLenum)0x859B
#endif
#ifndef GL_OPERAND3_ALPHA_ARB             
#define GL_OPERAND3_ALPHA_ARB  GL_OPERAND3_ALPHA_EXT             
#endif
#ifndef GL_OPERAND4_ALPHA_EXT             
#define GL_OPERAND4_ALPHA_EXT             (GLenum)0x859C
#endif
#ifndef GL_OPERAND4_ALPHA_ARB             
#define GL_OPERAND4_ALPHA_ARB  GL_OPERAND4_ALPHA_EXT             
#endif
#ifndef GL_OPERAND5_ALPHA_EXT             
#define GL_OPERAND5_ALPHA_EXT             (GLenum)0x859D
#endif
#ifndef GL_OPERAND5_ALPHA_ARB             
#define GL_OPERAND5_ALPHA_ARB  GL_OPERAND5_ALPHA_EXT             
#endif
#ifndef GL_OPERAND6_ALPHA_EXT             
#define GL_OPERAND6_ALPHA_EXT             (GLenum)0x859E
#endif
#ifndef GL_OPERAND6_ALPHA_ARB             
#define GL_OPERAND6_ALPHA_ARB  GL_OPERAND6_ALPHA_EXT             
#endif
#ifndef GL_OPERAND7_ALPHA_EXT             
#define GL_OPERAND7_ALPHA_EXT             (GLenum)0x859F
#endif
#ifndef GL_OPERAND7_ALPHA_ARB             
#define GL_OPERAND7_ALPHA_ARB  GL_OPERAND7_ALPHA_EXT             
#endif
#ifndef GL_LIGHT_MODEL_SPECULAR_VECTOR_APPLE 
#define GL_LIGHT_MODEL_SPECULAR_VECTOR_APPLE (GLenum)0x85B0
#endif
#ifndef GL_TRANSFORM_HINT_APPLE           
#define GL_TRANSFORM_HINT_APPLE           (GLenum)0x85B1
#endif
#ifndef GL_FOG_SCALE_SGIX                 
#define GL_FOG_SCALE_SGIX                 (GLenum)0x81FC
#endif
#ifndef GL_FOG_SCALE_VALUE_SGIX           
#define GL_FOG_SCALE_VALUE_SGIX           (GLenum)0x81FD
#endif
#ifndef GL_UNPACK_CONSTANT_DATA_SUNX      
#define GL_UNPACK_CONSTANT_DATA_SUNX      (GLenum)0x81D5
#endif
#ifndef GL_TEXTURE_CONSTANT_DATA_SUNX     
#define GL_TEXTURE_CONSTANT_DATA_SUNX     (GLenum)0x81D6
#endif
#ifndef GL_GLOBAL_ALPHA_SUN               
#define GL_GLOBAL_ALPHA_SUN               (GLenum)0x81D9
#endif
#ifndef GL_GLOBAL_ALPHA_FACTOR_SUN        
#define GL_GLOBAL_ALPHA_FACTOR_SUN        (GLenum)0x81DA
#endif
#ifndef GL_RESTART_SUN                    
#define GL_RESTART_SUN                    (GLenum)0x01
#endif
#ifndef GL_REPLACE_MIDDLE_SUN             
#define GL_REPLACE_MIDDLE_SUN             (GLenum)0x02
#endif
#ifndef GL_REPLACE_OLDEST_SUN             
#define GL_REPLACE_OLDEST_SUN             (GLenum)0x03
#endif
#ifndef GL_TRIANGLE_LIST_SUN              
#define GL_TRIANGLE_LIST_SUN              (GLenum)0x81D7
#endif
#ifndef GL_REPLACEMENT_CODE_SUN           
#define GL_REPLACEMENT_CODE_SUN           (GLenum)0x81D8
#endif
#ifndef GL_REPLACEMENT_CODE_ARRAY_SUN     
#define GL_REPLACEMENT_CODE_ARRAY_SUN     (GLenum)0x85C0
#endif
#ifndef GL_REPLACEMENT_CODE_ARRAY_TYPE_SUN 
#define GL_REPLACEMENT_CODE_ARRAY_TYPE_SUN (GLenum)0x85C1
#endif
#ifndef GL_REPLACEMENT_CODE_ARRAY_STRIDE_SUN 
#define GL_REPLACEMENT_CODE_ARRAY_STRIDE_SUN (GLenum)0x85C2
#endif
#ifndef GL_REPLACEMENT_CODE_ARRAY_POINTER_SUN 
#define GL_REPLACEMENT_CODE_ARRAY_POINTER_SUN (GLenum)0x85C3
#endif
#ifndef GL_R1UI_V3F_SUN                   
#define GL_R1UI_V3F_SUN                   (GLenum)0x85C4
#endif
#ifndef GL_R1UI_C4UB_V3F_SUN              
#define GL_R1UI_C4UB_V3F_SUN              (GLenum)0x85C5
#endif
#ifndef GL_R1UI_C3F_V3F_SUN               
#define GL_R1UI_C3F_V3F_SUN               (GLenum)0x85C6
#endif
#ifndef GL_R1UI_N3F_V3F_SUN               
#define GL_R1UI_N3F_V3F_SUN               (GLenum)0x85C7
#endif
#ifndef GL_R1UI_C4F_N3F_V3F_SUN           
#define GL_R1UI_C4F_N3F_V3F_SUN           (GLenum)0x85C8
#endif
#ifndef GL_R1UI_T2F_V3F_SUN               
#define GL_R1UI_T2F_V3F_SUN               (GLenum)0x85C9
#endif
#ifndef GL_R1UI_T2F_N3F_V3F_SUN           
#define GL_R1UI_T2F_N3F_V3F_SUN           (GLenum)0x85CA
#endif
#ifndef GL_R1UI_T2F_C4F_N3F_V3F_SUN       
#define GL_R1UI_T2F_C4F_N3F_V3F_SUN       (GLenum)0x85CB
#endif
#ifndef GL_BLEND_DST_RGB_EXT              
#define GL_BLEND_DST_RGB_EXT              (GLenum)0x80C8
#endif
#ifndef GL_BLEND_SRC_RGB_EXT              
#define GL_BLEND_SRC_RGB_EXT              (GLenum)0x80C9
#endif
#ifndef GL_BLEND_DST_ALPHA_EXT            
#define GL_BLEND_DST_ALPHA_EXT            (GLenum)0x80CA
#endif
#ifndef GL_BLEND_SRC_ALPHA_EXT            
#define GL_BLEND_SRC_ALPHA_EXT            (GLenum)0x80CB
#endif
#ifndef GL_RED_MIN_CLAMP_INGR             
#define GL_RED_MIN_CLAMP_INGR             (GLenum)0x8560
#endif
#ifndef GL_GREEN_MIN_CLAMP_INGR           
#define GL_GREEN_MIN_CLAMP_INGR           (GLenum)0x8561
#endif
#ifndef GL_BLUE_MIN_CLAMP_INGR            
#define GL_BLUE_MIN_CLAMP_INGR            (GLenum)0x8562
#endif
#ifndef GL_ALPHA_MIN_CLAMP_INGR           
#define GL_ALPHA_MIN_CLAMP_INGR           (GLenum)0x8563
#endif
#ifndef GL_RED_MAX_CLAMP_INGR             
#define GL_RED_MAX_CLAMP_INGR             (GLenum)0x8564
#endif
#ifndef GL_GREEN_MAX_CLAMP_INGR           
#define GL_GREEN_MAX_CLAMP_INGR           (GLenum)0x8565
#endif
#ifndef GL_BLUE_MAX_CLAMP_INGR            
#define GL_BLUE_MAX_CLAMP_INGR            (GLenum)0x8566
#endif
#ifndef GL_ALPHA_MAX_CLAMP_INGR           
#define GL_ALPHA_MAX_CLAMP_INGR           (GLenum)0x8567
#endif
#ifndef GL_INTERLACE_READ_INGR            
#define GL_INTERLACE_READ_INGR            (GLenum)0x8568
#endif
#ifndef GL_INCR_WRAP_EXT                  
#define GL_INCR_WRAP_EXT                  (GLenum)0x8507
#endif
#ifndef GL_DECR_WRAP_EXT                  
#define GL_DECR_WRAP_EXT                  (GLenum)0x8508
#endif
#ifndef GL_422_EXT                        
#define GL_422_EXT                        (GLenum)0x80CC
#endif
#ifndef GL_422_REV_EXT                    
#define GL_422_REV_EXT                    (GLenum)0x80CD
#endif
#ifndef GL_422_AVERAGE_EXT                
#define GL_422_AVERAGE_EXT                (GLenum)0x80CE
#endif
#ifndef GL_422_REV_AVERAGE_EXT            
#define GL_422_REV_AVERAGE_EXT            (GLenum)0x80CF
#endif
#ifndef GL_NORMAL_MAP_NV                  
#define GL_NORMAL_MAP_NV                  (GLenum)0x8511
#endif
#ifndef GL_REFLECTION_MAP_NV              
#define GL_REFLECTION_MAP_NV              (GLenum)0x8512
#endif
#ifndef GL_NORMAL_MAP_EXT                 
#define GL_NORMAL_MAP_EXT                 (GLenum)0x8511
#endif
#ifndef GL_REFLECTION_MAP_EXT             
#define GL_REFLECTION_MAP_EXT             (GLenum)0x8512
#endif
#ifndef GL_TEXTURE_CUBE_MAP_EXT           
#define GL_TEXTURE_CUBE_MAP_EXT           (GLenum)0x8513
#endif
#ifndef GL_TEXTURE_BINDING_CUBE_MAP_EXT   
#define GL_TEXTURE_BINDING_CUBE_MAP_EXT   (GLenum)0x8514
#endif
#ifndef GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT 
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT (GLenum)0x8515
#endif
#ifndef GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT 
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT (GLenum)0x8516
#endif
#ifndef GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT 
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT (GLenum)0x8517
#endif
#ifndef GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT 
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT (GLenum)0x8518
#endif
#ifndef GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT 
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT (GLenum)0x8519
#endif
#ifndef GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT 
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT (GLenum)0x851A
#endif
#ifndef GL_PROXY_TEXTURE_CUBE_MAP_EXT     
#define GL_PROXY_TEXTURE_CUBE_MAP_EXT     (GLenum)0x851B
#endif
#ifndef GL_MAX_CUBE_MAP_TEXTURE_SIZE_EXT  
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE_EXT  (GLenum)0x851C
#endif
#ifndef GL_WRAP_BORDER_SUN                
#define GL_WRAP_BORDER_SUN                (GLenum)0x81D4
#endif
#ifndef GL_MAX_TEXTURE_LOD_BIAS_EXT       
#define GL_MAX_TEXTURE_LOD_BIAS_EXT       (GLenum)0x84FD
#endif
#ifndef GL_TEXTURE_FILTER_CONTROL_EXT     
#define GL_TEXTURE_FILTER_CONTROL_EXT     (GLenum)0x8500
#endif
#ifndef GL_TEXTURE_LOD_BIAS_EXT           
#define GL_TEXTURE_LOD_BIAS_EXT           (GLenum)0x8501
#endif
#ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT     
#define GL_TEXTURE_MAX_ANISOTROPY_EXT     (GLenum)0x84FE
#endif
#ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT (GLenum)0x84FF
#endif
#ifndef GL_MODELVIEW0_STACK_DEPTH_EXT     
#define GL_MODELVIEW0_STACK_DEPTH_EXT     GL_MODELVIEW_STACK_DEPTH
#endif
#ifndef GL_MODELVIEW1_STACK_DEPTH_EXT     
#define GL_MODELVIEW1_STACK_DEPTH_EXT     (GLenum)0x8502
#endif
#ifndef GL_MODELVIEW0_MATRIX_EXT          
#define GL_MODELVIEW0_MATRIX_EXT          GL_MODELVIEW_MATRIX
#endif
#ifndef GL_MODELVIEW_MATRIX1_EXT          
#define GL_MODELVIEW_MATRIX1_EXT          (GLenum)0x8506
#endif
#ifndef GL_VERTEX_WEIGHTING_EXT           
#define GL_VERTEX_WEIGHTING_EXT           (GLenum)0x8509
#endif
#ifndef GL_MODELVIEW0_EXT                 
#define GL_MODELVIEW0_EXT                 GL_MODELVIEW
#endif
#ifndef GL_MODELVIEW1_EXT                 
#define GL_MODELVIEW1_EXT                 (GLenum)0x850A
#endif
#ifndef GL_CURRENT_VERTEX_WEIGHT_EXT      
#define GL_CURRENT_VERTEX_WEIGHT_EXT      (GLenum)0x850B
#endif
#ifndef GL_VERTEX_WEIGHT_ARRAY_EXT        
#define GL_VERTEX_WEIGHT_ARRAY_EXT        (GLenum)0x850C
#endif
#ifndef GL_VERTEX_WEIGHT_ARRAY_SIZE_EXT   
#define GL_VERTEX_WEIGHT_ARRAY_SIZE_EXT   (GLenum)0x850D
#endif
#ifndef GL_VERTEX_WEIGHT_ARRAY_TYPE_EXT   
#define GL_VERTEX_WEIGHT_ARRAY_TYPE_EXT   (GLenum)0x850E
#endif
#ifndef GL_VERTEX_WEIGHT_ARRAY_STRIDE_EXT 
#define GL_VERTEX_WEIGHT_ARRAY_STRIDE_EXT (GLenum)0x850F
#endif
#ifndef GL_VERTEX_WEIGHT_ARRAY_POINTER_EXT 
#define GL_VERTEX_WEIGHT_ARRAY_POINTER_EXT (GLenum)0x8510
#endif
#ifndef GL_MAX_SHININESS_NV               
#define GL_MAX_SHININESS_NV               (GLenum)0x8504
#endif
#ifndef GL_MAX_SPOT_EXPONENT_NV           
#define GL_MAX_SPOT_EXPONENT_NV           (GLenum)0x8505
#endif
#ifndef GL_VERTEX_ARRAY_RANGE_NV          
#define GL_VERTEX_ARRAY_RANGE_NV          (GLenum)0x851D
#endif
#ifndef GL_VERTEX_ARRAY_RANGE_LENGTH_NV   
#define GL_VERTEX_ARRAY_RANGE_LENGTH_NV   (GLenum)0x851E
#endif
#ifndef GL_VERTEX_ARRAY_RANGE_VALID_NV    
#define GL_VERTEX_ARRAY_RANGE_VALID_NV    (GLenum)0x851F
#endif
#ifndef GL_MAX_VERTEX_ARRAY_RANGE_ELEMENT_NV 
#define GL_MAX_VERTEX_ARRAY_RANGE_ELEMENT_NV (GLenum)0x8520
#endif
#ifndef GL_VERTEX_ARRAY_RANGE_POINTER_NV  
#define GL_VERTEX_ARRAY_RANGE_POINTER_NV  (GLenum)0x8521
#endif
#ifndef GL_REGISTER_COMBINERS_NV          
#define GL_REGISTER_COMBINERS_NV          (GLenum)0x8522
#endif
#ifndef GL_VARIABLE_A_NV                  
#define GL_VARIABLE_A_NV                  (GLenum)0x8523
#endif
#ifndef GL_VARIABLE_B_NV                  
#define GL_VARIABLE_B_NV                  (GLenum)0x8524
#endif
#ifndef GL_VARIABLE_C_NV                  
#define GL_VARIABLE_C_NV                  (GLenum)0x8525
#endif
#ifndef GL_VARIABLE_D_NV                  
#define GL_VARIABLE_D_NV                  (GLenum)0x8526
#endif
#ifndef GL_VARIABLE_E_NV                  
#define GL_VARIABLE_E_NV                  (GLenum)0x8527
#endif
#ifndef GL_VARIABLE_F_NV                  
#define GL_VARIABLE_F_NV                  (GLenum)0x8528
#endif
#ifndef GL_VARIABLE_G_NV                  
#define GL_VARIABLE_G_NV                  (GLenum)0x8529
#endif
#ifndef GL_CONSTANT_COLOR0_NV             
#define GL_CONSTANT_COLOR0_NV             (GLenum)0x852A
#endif
#ifndef GL_CONSTANT_COLOR1_NV             
#define GL_CONSTANT_COLOR1_NV             (GLenum)0x852B
#endif
#ifndef GL_PRIMARY_COLOR_NV               
#define GL_PRIMARY_COLOR_NV               (GLenum)0x852C
#endif
#ifndef GL_SECONDARY_COLOR_NV             
#define GL_SECONDARY_COLOR_NV             (GLenum)0x852D
#endif
#ifndef GL_SPARE0_NV                      
#define GL_SPARE0_NV                      (GLenum)0x852E
#endif
#ifndef GL_SPARE1_NV                      
#define GL_SPARE1_NV                      (GLenum)0x852F
#endif
#ifndef GL_DISCARD_NV                     
#define GL_DISCARD_NV                     (GLenum)0x8530
#endif
#ifndef GL_E_TIMES_F_NV                   
#define GL_E_TIMES_F_NV                   (GLenum)0x8531
#endif
#ifndef GL_SPARE0_PLUS_SECONDARY_COLOR_NV 
#define GL_SPARE0_PLUS_SECONDARY_COLOR_NV (GLenum)0x8532
#endif
#ifndef GL_UNSIGNED_IDENTITY_NV           
#define GL_UNSIGNED_IDENTITY_NV           (GLenum)0x8536
#endif
#ifndef GL_UNSIGNED_INVERT_NV             
#define GL_UNSIGNED_INVERT_NV             (GLenum)0x8537
#endif
#ifndef GL_EXPAND_NORMAL_NV               
#define GL_EXPAND_NORMAL_NV               (GLenum)0x8538
#endif
#ifndef GL_EXPAND_NEGATE_NV               
#define GL_EXPAND_NEGATE_NV               (GLenum)0x8539
#endif
#ifndef GL_HALF_BIAS_NORMAL_NV            
#define GL_HALF_BIAS_NORMAL_NV            (GLenum)0x853A
#endif
#ifndef GL_HALF_BIAS_NEGATE_NV            
#define GL_HALF_BIAS_NEGATE_NV            (GLenum)0x853B
#endif
#ifndef GL_SIGNED_IDENTITY_NV             
#define GL_SIGNED_IDENTITY_NV             (GLenum)0x853C
#endif
#ifndef GL_SIGNED_NEGATE_NV               
#define GL_SIGNED_NEGATE_NV               (GLenum)0x853D
#endif
#ifndef GL_SCALE_BY_TWO_NV                
#define GL_SCALE_BY_TWO_NV                (GLenum)0x853E
#endif
#ifndef GL_SCALE_BY_FOUR_NV               
#define GL_SCALE_BY_FOUR_NV               (GLenum)0x853F
#endif
#ifndef GL_SCALE_BY_ONE_HALF_NV           
#define GL_SCALE_BY_ONE_HALF_NV           (GLenum)0x8540
#endif
#ifndef GL_BIAS_BY_NEGATIVE_ONE_HALF_NV   
#define GL_BIAS_BY_NEGATIVE_ONE_HALF_NV   (GLenum)0x8541
#endif
#ifndef GL_COMBINER_INPUT_NV              
#define GL_COMBINER_INPUT_NV              (GLenum)0x8542
#endif
#ifndef GL_COMBINER_MAPPING_NV            
#define GL_COMBINER_MAPPING_NV            (GLenum)0x8543
#endif
#ifndef GL_COMBINER_COMPONENT_USAGE_NV    
#define GL_COMBINER_COMPONENT_USAGE_NV    (GLenum)0x8544
#endif
#ifndef GL_COMBINER_AB_DOT_PRODUCT_NV     
#define GL_COMBINER_AB_DOT_PRODUCT_NV     (GLenum)0x8545
#endif
#ifndef GL_COMBINER_CD_DOT_PRODUCT_NV     
#define GL_COMBINER_CD_DOT_PRODUCT_NV     (GLenum)0x8546
#endif
#ifndef GL_COMBINER_MUX_SUM_NV            
#define GL_COMBINER_MUX_SUM_NV            (GLenum)0x8547
#endif
#ifndef GL_COMBINER_SCALE_NV              
#define GL_COMBINER_SCALE_NV              (GLenum)0x8548
#endif
#ifndef GL_COMBINER_BIAS_NV               
#define GL_COMBINER_BIAS_NV               (GLenum)0x8549
#endif
#ifndef GL_COMBINER_AB_OUTPUT_NV          
#define GL_COMBINER_AB_OUTPUT_NV          (GLenum)0x854A
#endif
#ifndef GL_COMBINER_CD_OUTPUT_NV          
#define GL_COMBINER_CD_OUTPUT_NV          (GLenum)0x854B
#endif
#ifndef GL_COMBINER_SUM_OUTPUT_NV         
#define GL_COMBINER_SUM_OUTPUT_NV         (GLenum)0x854C
#endif
#ifndef GL_MAX_GENERAL_COMBINERS_NV       
#define GL_MAX_GENERAL_COMBINERS_NV       (GLenum)0x854D
#endif
#ifndef GL_NUM_GENERAL_COMBINERS_NV       
#define GL_NUM_GENERAL_COMBINERS_NV       (GLenum)0x854E
#endif
#ifndef GL_COLOR_SUM_CLAMP_NV             
#define GL_COLOR_SUM_CLAMP_NV             (GLenum)0x854F
#endif
#ifndef GL_COMBINER0_NV                   
#define GL_COMBINER0_NV                   (GLenum)0x8550
#endif
#ifndef GL_COMBINER1_NV                   
#define GL_COMBINER1_NV                   (GLenum)0x8551
#endif
#ifndef GL_COMBINER2_NV                   
#define GL_COMBINER2_NV                   (GLenum)0x8552
#endif
#ifndef GL_COMBINER3_NV                   
#define GL_COMBINER3_NV                   (GLenum)0x8553
#endif
#ifndef GL_COMBINER4_NV                   
#define GL_COMBINER4_NV                   (GLenum)0x8554
#endif
#ifndef GL_COMBINER5_NV                   
#define GL_COMBINER5_NV                   (GLenum)0x8555
#endif
#ifndef GL_COMBINER6_NV                   
#define GL_COMBINER6_NV                   (GLenum)0x8556
#endif
#ifndef GL_COMBINER7_NV                   
#define GL_COMBINER7_NV                   (GLenum)0x8557
#endif
#ifndef GL_FOG_DISTANCE_MODE_NV           
#define GL_FOG_DISTANCE_MODE_NV           (GLenum)0x855A
#endif
#ifndef GL_EYE_RADIAL_NV                  
#define GL_EYE_RADIAL_NV                  (GLenum)0x855B
#endif
#ifndef GL_EYE_PLANE_ABSOLUTE_NV          
#define GL_EYE_PLANE_ABSOLUTE_NV          (GLenum)0x855C
#endif
#ifndef GL_EMBOSS_LIGHT_NV                
#define GL_EMBOSS_LIGHT_NV                (GLenum)0x855D
#endif
#ifndef GL_EMBOSS_CONSTANT_NV             
#define GL_EMBOSS_CONSTANT_NV             (GLenum)0x855E
#endif
#ifndef GL_EMBOSS_MAP_NV                  
#define GL_EMBOSS_MAP_NV                  (GLenum)0x855F
#endif
#ifndef GL_COMBINE4_NV                    
#define GL_COMBINE4_NV                    (GLenum)0x8503
#endif
#ifndef GL_SOURCE3_RGB_NV                 
#define GL_SOURCE3_RGB_NV                 (GLenum)0x8583
#endif
#ifndef GL_SOURCE3_ALPHA_NV               
#define GL_SOURCE3_ALPHA_NV               (GLenum)0x858B
#endif
#ifndef GL_OPERAND3_RGB_NV                
#define GL_OPERAND3_RGB_NV                (GLenum)0x8593
#endif
#ifndef GL_OPERAND3_ALPHA_NV              
#define GL_OPERAND3_ALPHA_NV              (GLenum)0x859B
#endif
#ifndef GL_COMPRESSED_RGB_S3TC_DXT1_EXT   
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT   (GLenum)0x83F0
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT  
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT  (GLenum)0x83F1
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT  
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT  (GLenum)0x83F2
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT  
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT  (GLenum)0x83F3
#endif
#ifndef GL_CULL_VERTEX_IBM                
#define GL_CULL_VERTEX_IBM                103050
#endif
#ifndef GL_VERTEX_ARRAY_LIST_IBM          
#define GL_VERTEX_ARRAY_LIST_IBM          103070
#endif
#ifndef GL_NORMAL_ARRAY_LIST_IBM          
#define GL_NORMAL_ARRAY_LIST_IBM          103071
#endif
#ifndef GL_COLOR_ARRAY_LIST_IBM           
#define GL_COLOR_ARRAY_LIST_IBM           103072
#endif
#ifndef GL_INDEX_ARRAY_LIST_IBM           
#define GL_INDEX_ARRAY_LIST_IBM           103073
#endif
#ifndef GL_TEXTURE_COORD_ARRAY_LIST_IBM   
#define GL_TEXTURE_COORD_ARRAY_LIST_IBM   103074
#endif
#ifndef GL_EDGE_FLAG_ARRAY_LIST_IBM       
#define GL_EDGE_FLAG_ARRAY_LIST_IBM       103075
#endif
#ifndef GL_FOG_COORDINATE_ARRAY_LIST_IBM  
#define GL_FOG_COORDINATE_ARRAY_LIST_IBM  103076
#endif
#ifndef GL_SECONDARY_COLOR_ARRAY_LIST_IBM 
#define GL_SECONDARY_COLOR_ARRAY_LIST_IBM 103077
#endif
#ifndef GL_VERTEX_ARRAY_LIST_STRIDE_IBM   
#define GL_VERTEX_ARRAY_LIST_STRIDE_IBM   103080
#endif
#ifndef GL_NORMAL_ARRAY_LIST_STRIDE_IBM   
#define GL_NORMAL_ARRAY_LIST_STRIDE_IBM   103081
#endif
#ifndef GL_COLOR_ARRAY_LIST_STRIDE_IBM    
#define GL_COLOR_ARRAY_LIST_STRIDE_IBM    103082
#endif
#ifndef GL_INDEX_ARRAY_LIST_STRIDE_IBM    
#define GL_INDEX_ARRAY_LIST_STRIDE_IBM    103083
#endif
#ifndef GL_TEXTURE_COORD_ARRAY_LIST_STRIDE_IBM 
#define GL_TEXTURE_COORD_ARRAY_LIST_STRIDE_IBM 103084
#endif
#ifndef GL_EDGE_FLAG_ARRAY_LIST_STRIDE_IBM 
#define GL_EDGE_FLAG_ARRAY_LIST_STRIDE_IBM 103085
#endif
#ifndef GL_FOG_COORDINATE_ARRAY_LIST_STRIDE_IBM 
#define GL_FOG_COORDINATE_ARRAY_LIST_STRIDE_IBM 103086
#endif
#ifndef GL_SECONDARY_COLOR_ARRAY_LIST_STRIDE_IBM 
#define GL_SECONDARY_COLOR_ARRAY_LIST_STRIDE_IBM 103087
#endif
#ifndef GL_PACK_SUBSAMPLE_RATE_SGIX       
#define GL_PACK_SUBSAMPLE_RATE_SGIX       (GLenum)0x85A0
#endif
#ifndef GL_UNPACK_SUBSAMPLE_RATE_SGIX     
#define GL_UNPACK_SUBSAMPLE_RATE_SGIX     (GLenum)0x85A1
#endif
#ifndef GL_PIXEL_SUBSAMPLE_4444_SGIX      
#define GL_PIXEL_SUBSAMPLE_4444_SGIX      (GLenum)0x85A2
#endif
#ifndef GL_PIXEL_SUBSAMPLE_2424_SGIX      
#define GL_PIXEL_SUBSAMPLE_2424_SGIX      (GLenum)0x85A3
#endif
#ifndef GL_PIXEL_SUBSAMPLE_4242_SGIX      
#define GL_PIXEL_SUBSAMPLE_4242_SGIX      (GLenum)0x85A4
#endif
#ifndef GL_YCRCB_SGIX                     
#define GL_YCRCB_SGIX                     (GLenum)0x8318
#endif
#ifndef GL_YCRCBA_SGIX                    
#define GL_YCRCBA_SGIX                    (GLenum)0x8319
#endif
#ifndef GL_DEPTH_PASS_INSTRUMENT_SGIX     
#define GL_DEPTH_PASS_INSTRUMENT_SGIX     (GLenum)0x8310
#endif
#ifndef GL_DEPTH_PASS_INSTRUMENT_COUNTERS_SGIX 
#define GL_DEPTH_PASS_INSTRUMENT_COUNTERS_SGIX (GLenum)0x8311
#endif
#ifndef GL_DEPTH_PASS_INSTRUMENT_MAX_SGIX 
#define GL_DEPTH_PASS_INSTRUMENT_MAX_SGIX (GLenum)0x8312
#endif
#ifndef GL_COMPRESSED_RGB_FXT1_3DFX       
#define GL_COMPRESSED_RGB_FXT1_3DFX       (GLenum)0x86B0
#endif
#ifndef GL_COMPRESSED_RGBA_FXT1_3DFX      
#define GL_COMPRESSED_RGBA_FXT1_3DFX      (GLenum)0x86B1
#endif
#ifndef GL_MULTISAMPLE_3DFX               
#define GL_MULTISAMPLE_3DFX               (GLenum)0x86B2
#endif
#ifndef GL_SAMPLE_BUFFERS_3DFX            
#define GL_SAMPLE_BUFFERS_3DFX            (GLenum)0x86B3
#endif
#ifndef GL_SAMPLES_3DFX                   
#define GL_SAMPLES_3DFX                   (GLenum)0x86B4
#endif
#ifndef GL_MULTISAMPLE_BIT_3DFX           
#define GL_MULTISAMPLE_BIT_3DFX           (GLenum)0x20000000
#endif
#ifndef GL_MULTISAMPLE_EXT                
#define GL_MULTISAMPLE_EXT                (GLenum)0x809D
#endif
#ifndef GL_SAMPLE_ALPHA_TO_MASK_EXT       
#define GL_SAMPLE_ALPHA_TO_MASK_EXT       (GLenum)0x809E
#endif
#ifndef GL_SAMPLE_ALPHA_TO_ONE_EXT        
#define GL_SAMPLE_ALPHA_TO_ONE_EXT        (GLenum)0x809F
#endif
#ifndef GL_SAMPLE_MASK_EXT                
#define GL_SAMPLE_MASK_EXT                (GLenum)0x80A0
#endif
#ifndef GL_1PASS_EXT                      
#define GL_1PASS_EXT                      (GLenum)0x80A1
#endif
#ifndef GL_2PASS_0_EXT                    
#define GL_2PASS_0_EXT                    (GLenum)0x80A2
#endif
#ifndef GL_2PASS_1_EXT                    
#define GL_2PASS_1_EXT                    (GLenum)0x80A3
#endif
#ifndef GL_4PASS_0_EXT                    
#define GL_4PASS_0_EXT                    (GLenum)0x80A4
#endif
#ifndef GL_4PASS_1_EXT                    
#define GL_4PASS_1_EXT                    (GLenum)0x80A5
#endif
#ifndef GL_4PASS_2_EXT                    
#define GL_4PASS_2_EXT                    (GLenum)0x80A6
#endif
#ifndef GL_4PASS_3_EXT                    
#define GL_4PASS_3_EXT                    (GLenum)0x80A7
#endif
#ifndef GL_SAMPLE_BUFFERS_EXT             
#define GL_SAMPLE_BUFFERS_EXT             (GLenum)0x80A8
#endif
#ifndef GL_SAMPLES_EXT                    
#define GL_SAMPLES_EXT                    (GLenum)0x80A9
#endif
#ifndef GL_SAMPLE_MASK_VALUE_EXT          
#define GL_SAMPLE_MASK_VALUE_EXT          (GLenum)0x80AA
#endif
#ifndef GL_SAMPLE_MASK_INVERT_EXT         
#define GL_SAMPLE_MASK_INVERT_EXT         (GLenum)0x80AB
#endif
#ifndef GL_SAMPLE_PATTERN_EXT             
#define GL_SAMPLE_PATTERN_EXT             (GLenum)0x80AC
#endif
#ifndef GL_VERTEX_PRECLIP_SGIX            
#define GL_VERTEX_PRECLIP_SGIX            (GLenum)0x83EE
#endif
#ifndef GL_VERTEX_PRECLIP_HINT_SGIX       
#define GL_VERTEX_PRECLIP_HINT_SGIX       (GLenum)0x83EF
#endif
#ifndef GL_CONVOLUTION_HINT_SGIX          
#define GL_CONVOLUTION_HINT_SGIX          (GLenum)0x8316
#endif
#ifndef GL_PACK_RESAMPLE_SGIX             
#define GL_PACK_RESAMPLE_SGIX             (GLenum)0x842C
#endif
#ifndef GL_UNPACK_RESAMPLE_SGIX           
#define GL_UNPACK_RESAMPLE_SGIX           (GLenum)0x842D
#endif
#ifndef GL_RESAMPLE_REPLICATE_SGIX        
#define GL_RESAMPLE_REPLICATE_SGIX        (GLenum)0x842E
#endif
#ifndef GL_RESAMPLE_ZERO_FILL_SGIX        
#define GL_RESAMPLE_ZERO_FILL_SGIX        (GLenum)0x842F
#endif
#ifndef GL_RESAMPLE_DECIMATE_SGIX         
#define GL_RESAMPLE_DECIMATE_SGIX         (GLenum)0x8430
#endif
#ifndef GL_EYE_DISTANCE_TO_POINT_SGIS     
#define GL_EYE_DISTANCE_TO_POINT_SGIS     (GLenum)0x81F0
#endif
#ifndef GL_OBJECT_DISTANCE_TO_POINT_SGIS  
#define GL_OBJECT_DISTANCE_TO_POINT_SGIS  (GLenum)0x81F1
#endif
#ifndef GL_EYE_DISTANCE_TO_LINE_SGIS      
#define GL_EYE_DISTANCE_TO_LINE_SGIS      (GLenum)0x81F2
#endif
#ifndef GL_OBJECT_DISTANCE_TO_LINE_SGIS   
#define GL_OBJECT_DISTANCE_TO_LINE_SGIS   (GLenum)0x81F3
#endif
#ifndef GL_EYE_POINT_SGIS                 
#define GL_EYE_POINT_SGIS                 (GLenum)0x81F4
#endif
#ifndef GL_OBJECT_POINT_SGIS              
#define GL_OBJECT_POINT_SGIS              (GLenum)0x81F5
#endif
#ifndef GL_EYE_LINE_SGIS                  
#define GL_EYE_LINE_SGIS                  (GLenum)0x81F6
#endif
#ifndef GL_OBJECT_LINE_SGIS               
#define GL_OBJECT_LINE_SGIS               (GLenum)0x81F7
#endif
#ifndef GL_TEXTURE_COLOR_WRITEMASK_SGIS   
#define GL_TEXTURE_COLOR_WRITEMASK_SGIS   (GLenum)0x81EF
#endif

#endif
