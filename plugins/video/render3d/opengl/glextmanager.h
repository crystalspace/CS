/*
  Copyright (C) 2002 by Anders Stenberg

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



#ifndef __GLEXTENSIONMANAGER_H__
#define __GLEXTENSIONMANAGER_H__



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

#if !defined(GLAPI)
#  if !defined(OPENSTEP) && (defined(__WIN32__) && !defined(__CYGWIN__))
#    if defined(_MSC_VER) && defined(BUILD_GL32) /* tag specify we're building mesa as a DLL */
#      define GLAPI __declspec(dllexport)
#    elif defined(_MSC_VER) && defined(_DLL) /* tag specifying we're building for DLL runtime support */
#      define GLAPI __declspec(dllimport)
#    else /* for use with static link lib build of Win32 edition only */
#      define GLAPI extern
#    endif /* _STATIC_MESA support */
#    define GLAPIENTRY __stdcall
#  else
/* non-Windows compilation */
#    define GLAPI extern
#    define GLAPIENTRY
#  endif /* WIN32 / CYGWIN bracket */
#endif

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

#if defined(CS_OPENGL_PATH)
#include CS_HEADER_GLOBAL(CS_OPENGL_PATH,gl.h)
#else
#include <GL/gl.h>
#endif


#include "video/canvas/openglcommon/iogl.h"


// GL_version_1_2
#define GL_UNSIGNED_BYTE_3_3_2                                             32818
#define GL_UNSIGNED_SHORT_4_4_4_4                                          32819
#define GL_UNSIGNED_SHORT_5_5_5_1                                          32820
#define GL_UNSIGNED_INT_8_8_8_8                                            32821
#define GL_UNSIGNED_INT_10_10_10_2                                         32822
#define GL_RESCALE_NORMAL                                                  32826
#define GL_UNSIGNED_BYTE_2_3_3_REV                                         33634
#define GL_UNSIGNED_SHORT_5_6_5                                            33635
#define GL_UNSIGNED_SHORT_5_6_5_REV                                        33636
#define GL_UNSIGNED_SHORT_4_4_4_4_REV                                      33637
#define GL_UNSIGNED_SHORT_1_5_5_5_REV                                      33638
#define GL_UNSIGNED_INT_8_8_8_8_REV                                        33639
#define GL_UNSIGNED_INT_2_10_10_10_REV                                     33640
#define GL_BGR                                                             32992
#define GL_BGRA                                                            32993
#define GL_MAX_ELEMENTS_VERTICES                                           33000
#define GL_MAX_ELEMENTS_INDICES                                            33001
#define GL_CLAMP_TO_EDGE                                                   33071
#define GL_TEXTURE_MIN_LOD                                                 33082
#define GL_TEXTURE_MAX_LOD                                                 33083
#define GL_TEXTURE_BASE_LEVEL                                              33084
#define GL_TEXTURE_MAX_LEVEL                                               33085
#define GL_LIGHT_MODEL_COLOR_CONTROL                                       33272
#define GL_SINGLE_COLOR                                                    33273
#define GL_SEPARATE_SPECULAR_COLOR                                         33274
#define GL_SMOOTH_POINT_SIZE_RANGE                                         2834
#define GL_SMOOTH_POINT_SIZE_GRANULARITY                                   2835
#define GL_SMOOTH_LINE_WIDTH_RANGE                                         2850
#define GL_SMOOTH_LINE_WIDTH_GRANULARITY                                   2851
#define GL_ALIASED_POINT_SIZE_RANGE                                        33901
#define GL_ALIASED_LINE_WIDTH_RANGE                                        33902
#define GL_PACK_SKIP_IMAGES                                                32875
#define GL_PACK_IMAGE_HEIGHT                                               32876
#define GL_UNPACK_SKIP_IMAGES                                              32877
#define GL_UNPACK_IMAGE_HEIGHT                                             32878
#define GL_TEXTURE_3D                                                      32879
#define GL_PROXY_TEXTURE_3D                                                32880
#define GL_TEXTURE_DEPTH                                                   32881
#define GL_TEXTURE_WRAP_R                                                  32882
#define GL_MAX_3D_TEXTURE_SIZE                                             32883
typedef GLvoid (csAPIENTRY* csGLBLENDCOLOR) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef GLvoid (csAPIENTRY* csGLBLENDEQUATION) (GLenum mode);
typedef GLvoid (csAPIENTRY* csGLDRAWRANGEELEMENTS) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, GLvoid* indices);
typedef GLvoid (csAPIENTRY* csGLCOLORTABLE) (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, GLvoid* table);
typedef GLvoid (csAPIENTRY* csGLCOLORTABLEPARAMETERFV) (GLenum target, GLenum pname, GLfloat* params);
typedef GLvoid (csAPIENTRY* csGLCOLORTABLEPARAMETERIV) (GLenum target, GLenum pname, GLint* params);
typedef GLvoid (csAPIENTRY* csGLCOPYCOLORTABLE) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
typedef GLvoid (csAPIENTRY* csGLGETCOLORTABLE) (GLenum target, GLenum format, GLenum type, GLvoid* table);
typedef GLvoid (csAPIENTRY* csGLGETCOLORTABLEPARAMETERFV) (GLenum target, GLenum pname, GLfloat* params);
typedef GLvoid (csAPIENTRY* csGLGETCOLORTABLEPARAMETERIV) (GLenum target, GLenum pname, GLint* params);
typedef GLvoid (csAPIENTRY* csGLCOLORSUBTABLE) (GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, GLvoid* data);
typedef GLvoid (csAPIENTRY* csGLCOPYCOLORSUBTABLE) (GLenum target, GLsizei start, GLint x, GLint y, GLsizei width);
typedef GLvoid (csAPIENTRY* csGLCONVOLUTIONFILTER1D) (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, GLvoid* image);
typedef GLvoid (csAPIENTRY* csGLCONVOLUTIONFILTER2D) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* image);
typedef GLvoid (csAPIENTRY* csGLCONVOLUTIONPARAMETERF) (GLenum target, GLenum pname, GLfloat params);
typedef GLvoid (csAPIENTRY* csGLCONVOLUTIONPARAMETERFV) (GLenum target, GLenum pname, GLfloat* params);
typedef GLvoid (csAPIENTRY* csGLCONVOLUTIONPARAMETERI) (GLenum target, GLenum pname, GLint params);
typedef GLvoid (csAPIENTRY* csGLCONVOLUTIONPARAMETERIV) (GLenum target, GLenum pname, GLint* params);
typedef GLvoid (csAPIENTRY* csGLCOPYCONVOLUTIONFILTER1D) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
typedef GLvoid (csAPIENTRY* csGLCOPYCONVOLUTIONFILTER2D) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height);
typedef GLvoid (csAPIENTRY* csGLGETCONVOLUTIONFILTER) (GLenum target, GLenum format, GLenum type, GLvoid* image);
typedef GLvoid (csAPIENTRY* csGLGETCONVOLUTIONPARAMETERFV) (GLenum target, GLenum pname, GLfloat* params);
typedef GLvoid (csAPIENTRY* csGLGETCONVOLUTIONPARAMETERIV) (GLenum target, GLenum pname, GLint* params);
typedef GLvoid (csAPIENTRY* csGLGETSEPARABLEFILTER) (GLenum target, GLenum format, GLenum type, GLvoid* row, GLvoid* column, GLvoid* span);
typedef GLvoid (csAPIENTRY* csGLSEPARABLEFILTER2D) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* row, GLvoid* column);
typedef GLvoid (csAPIENTRY* csGLGETHISTOGRAM) (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid* values);
typedef GLvoid (csAPIENTRY* csGLGETHISTOGRAMPARAMETERFV) (GLenum target, GLenum pname, GLfloat* params);
typedef GLvoid (csAPIENTRY* csGLGETHISTOGRAMPARAMETERIV) (GLenum target, GLenum pname, GLint* params);
typedef GLvoid (csAPIENTRY* csGLGETMINMAX) (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid* values);
typedef GLvoid (csAPIENTRY* csGLGETMINMAXPARAMETERFV) (GLenum target, GLenum pname, GLfloat* params);
typedef GLvoid (csAPIENTRY* csGLGETMINMAXPARAMETERIV) (GLenum target, GLenum pname, GLint* params);
typedef GLvoid (csAPIENTRY* csGLHISTOGRAM) (GLenum target, GLsizei width, GLenum internalformat, GLboolean sink);
typedef GLvoid (csAPIENTRY* csGLMINMAX) (GLenum target, GLenum internalformat, GLboolean sink);
typedef GLvoid (csAPIENTRY* csGLRESETHISTOGRAM) (GLenum target);
typedef GLvoid (csAPIENTRY* csGLRESETMINMAX) (GLenum target);
typedef GLvoid (csAPIENTRY* csGLTEXIMAGE3D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, GLvoid* pixels);
typedef GLvoid (csAPIENTRY* csGLTEXSUBIMAGE3D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLvoid* pixels);
typedef GLvoid (csAPIENTRY* csGLCOPYTEXSUBIMAGE3D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);

// GL_ARB_imaging
#define GL_CONSTANT_COLOR                                                  32769
#define GL_ONE_MINUS_CONSTANT_COLOR                                        32770
#define GL_CONSTANT_ALPHA                                                  32771
#define GL_ONE_MINUS_CONSTANT_ALPHA                                        32772
#define GL_BLEND_COLOR                                                     32773
#define GL_FUNC_ADD                                                        32774
#define GL_MIN                                                             32775
#define GL_MAX                                                             32776
#define GL_BLEND_EQUATION                                                  32777
#define GL_FUNC_SUBTRACT                                                   32778
#define GL_FUNC_REVERSE_SUBTRACT                                           32779
#define GL_CONVOLUTION_1D                                                  32784
#define GL_CONVOLUTION_2D                                                  32785
#define GL_SEPARABLE_2D                                                    32786
#define GL_CONVOLUTION_BORDER_MODE                                         32787
#define GL_CONVOLUTION_FILTER_SCALE                                        32788
#define GL_CONVOLUTION_FILTER_BIAS                                         32789
#define GL_REDUCE                                                          32790
#define GL_CONVOLUTION_FORMAT                                              32791
#define GL_CONVOLUTION_WIDTH                                               32792
#define GL_CONVOLUTION_HEIGHT                                              32793
#define GL_MAX_CONVOLUTION_WIDTH                                           32794
#define GL_MAX_CONVOLUTION_HEIGHT                                          32795
#define GL_POST_CONVOLUTION_RED_SCALE                                      32796
#define GL_POST_CONVOLUTION_GREEN_SCALE                                    32797
#define GL_POST_CONVOLUTION_BLUE_SCALE                                     32798
#define GL_POST_CONVOLUTION_ALPHA_SCALE                                    32799
#define GL_POST_CONVOLUTION_RED_BIAS                                       32800
#define GL_POST_CONVOLUTION_GREEN_BIAS                                     32801
#define GL_POST_CONVOLUTION_BLUE_BIAS                                      32802
#define GL_POST_CONVOLUTION_ALPHA_BIAS                                     32803
#define GL_HISTOGRAM                                                       32804
#define GL_PROXY_HISTOGRAM                                                 32805
#define GL_HISTOGRAM_WIDTH                                                 32806
#define GL_HISTOGRAM_FORMAT                                                32807
#define GL_HISTOGRAM_RED_SIZE                                              32808
#define GL_HISTOGRAM_GREEN_SIZE                                            32809
#define GL_HISTOGRAM_BLUE_SIZE                                             32810
#define GL_HISTOGRAM_ALPHA_SIZE                                            32811
#define GL_HISTOGRAM_LUMINANCE_SIZE                                        32812
#define GL_HISTOGRAM_SINK                                                  32813
#define GL_MINMAX                                                          32814
#define GL_MINMAX_FORMAT                                                   32815
#define GL_MINMAX_SINK                                                     32816
#define GL_TABLE_TOO_LARGE                                                 32817
#define GL_COLOR_MATRIX                                                    32945
#define GL_COLOR_MATRIX_STACK_DEPTH                                        32946
#define GL_MAX_COLOR_MATRIX_STACK_DEPTH                                    32947
#define GL_POST_COLOR_MATRIX_RED_SCALE                                     32948
#define GL_POST_COLOR_MATRIX_GREEN_SCALE                                   32949
#define GL_POST_COLOR_MATRIX_BLUE_SCALE                                    32950
#define GL_POST_COLOR_MATRIX_ALPHA_SCALE                                   32951
#define GL_POST_COLOR_MATRIX_RED_BIAS                                      32952
#define GL_POST_COLOR_MATRIX_GREEN_BIAS                                    32953
#define GL_POST_COLOR_MATRIX_BLUE_BIAS                                     32954
#define GL_POST_COLOR_MATIX_ALPHA_BIAS                                     32955
#define GL_COLOR_TABLE                                                     32976
#define GL_POST_CONVOLUTION_COLOR_TABLE                                    32977
#define GL_POST_COLOR_MATRIX_COLOR_TABLE                                   32978
#define GL_PROXY_COLOR_TABLE                                               32979
#define GL_PROXY_POST_CONVOLUTION_COLOR_TABLE                              32980
#define GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE                             32981
#define GL_COLOR_TABLE_SCALE                                               32982
#define GL_COLOR_TABLE_BIAS                                                32983
#define GL_COLOR_TABLE_FORMAT                                              32984
#define GL_COLOR_TABLE_WIDTH                                               32985
#define GL_COLOR_TABLE_RED_SIZE                                            32986
#define GL_COLOR_TABLE_GREEN_SIZE                                          32987
#define GL_COLOR_TABLE_BLUE_SIZE                                           32988
#define GL_COLOR_TABLE_ALPHA_SIZE                                          32989
#define GL_COLOR_TABLE_LUMINANCE_SIZE                                      32990
#define GL_COLOR_TABLE_INTENSITY_SIZE                                      32991
#define GL_IGNORE_BORDER                                                   33104
#define GL_CONSTANT_BORDER                                                 33105
#define GL_WRAP_BORDER                                                     33106
#define GL_REPLICATE_BORDER                                                33107
#define GL_CONVOLUTION_BORDER_COLOR                                        33108

// GL_version_1_3
#define GL_TEXTURE0                                                        33984
#define GL_TEXTURE1                                                        33985
#define GL_TEXTURE2                                                        33986
#define GL_TEXTURE3                                                        33987
#define GL_TEXTURE4                                                        33988
#define GL_TEXTURE5                                                        33989
#define GL_TEXTURE6                                                        33990
#define GL_TEXTURE7                                                        33991
#define GL_TEXTURE8                                                        33992
#define GL_TEXTURE9                                                        33993
#define GL_TEXTURE10                                                       33994
#define GL_TEXTURE11                                                       33995
#define GL_TEXTURE12                                                       33996
#define GL_TEXTURE13                                                       33997
#define GL_TEXTURE14                                                       33998
#define GL_TEXTURE15                                                       33999
#define GL_TEXTURE16                                                       34000
#define GL_TEXTURE17                                                       34001
#define GL_TEXTURE18                                                       34002
#define GL_TEXTURE19                                                       34003
#define GL_TEXTURE20                                                       34004
#define GL_TEXTURE21                                                       34005
#define GL_TEXTURE22                                                       34006
#define GL_TEXTURE23                                                       34007
#define GL_TEXTURE24                                                       34008
#define GL_TEXTURE25                                                       34009
#define GL_TEXTURE26                                                       34010
#define GL_TEXTURE27                                                       34011
#define GL_TEXTURE28                                                       34012
#define GL_TEXTURE29                                                       34013
#define GL_TEXTURE30                                                       34014
#define GL_TEXTURE31                                                       34015
#define GL_ACTIVE_TEXTURE                                                  34016
#define GL_CLIENT_ACTIVE_TEXTURE                                           34017
#define GL_MAX_TEXTURE_UNITS                                               34018
#define GL_TRANSPOSE_MODELVIEW_MATRIX                                      34019
#define GL_TRANSPOSE_PROJECTION_MATRIX                                     34020
#define GL_TRANSPOSE_TEXTURE_MATRIX                                        34021
#define GL_TRANSPOSE_COLOR_MATRIX                                          34022
#define GL_MULTISAMPLE                                                     32925
#define GL_SAMPLE_ALPHA_TO_COVERAGE                                        32926
#define GL_SAMPLE_ALPHA_TO_ONE                                             32927
#define GL_SAMPLE_COVERAGE                                                 32928
#define GL_SAMPLE_BUFFERS                                                  32936
#define GL_SAMPLES                                                         32937
#define GL_SAMPLE_COVERAGE_VALUE                                           32938
#define GL_SAMPLE_COVERAGE_INVERT                                          32939
#define GL_MULTISAMPLE_BIT                                                 536870912
#define GL_NORMAL_MAP                                                      34065
#define GL_REFLECTION_MAP                                                  34066
#define GL_TEXTURE_CUBE_MAP                                                34067
#define GL_TEXTURE_BINDING_CUBE_MAP                                        34068
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X                                     34069
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X                                     34070
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y                                     34071
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y                                     34072
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z                                     34073
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z                                     34074
#define GL_PROXY_TEXTURE_CUBE_MAP                                          34075
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE                                       34076
#define GL_COMPRESSED_ALPHA                                                34025
#define GL_COMPRESSED_LUMINANCE                                            34026
#define GL_COMPRESSED_LUMINANCE_ALPHA                                      34027
#define GL_COMPRESSED_INTENSITY                                            34028
#define GL_COMPRESSED_RGB                                                  34029
#define GL_COMPRESSED_RGBA                                                 34030
#define GL_TEXTURE_COMPRESSION_HINT                                        34031
#define GL_TEXTURE_COMPRESSED_IMAGE_SIZE                                   34464
#define GL_TEXTURE_COMPRESSED                                              34465
#define GL_NUM_COMPRESSED_TEXTURE_FORMATS                                  34466
#define GL_COMPRESSED_TEXTURE_FORMATS                                      34467
#define GL_CLAMP_TO_BORDER                                                 33069
#define GL_CLAMP_TO_BORDER_SGIS                                            33069
#define GL_COMBINE                                                         34160
#define GL_COMBINE_RGB                                                     34161
#define GL_COMBINE_ALPHA                                                   34162
#define GL_SOURCE0_RGB                                                     34176
#define GL_SOURCE1_RGB                                                     34177
#define GL_SOURCE2_RGB                                                     34178
#define GL_SOURCE0_ALPHA                                                   34184
#define GL_SOURCE1_ALPHA                                                   34185
#define GL_SOURCE2_ALPHA                                                   34186
#define GL_OPERAND0_RGB                                                    34192
#define GL_OPERAND1_RGB                                                    34193
#define GL_OPERAND2_RGB                                                    34194
#define GL_OPERAND0_ALPHA                                                  34200
#define GL_OPERAND1_ALPHA                                                  34201
#define GL_OPERAND2_ALPHA                                                  34202
#define GL_RGB_SCALE                                                       34163
#define GL_ADD_SIGNED                                                      34164
#define GL_INTERPOLATE                                                     34165
#define GL_SUBTRACT                                                        34023
#define GL_CONSTANT                                                        34166
#define GL_PRIMARY_COLOR                                                   34167
#define GL_PREVIOUS                                                        34168
#define GL_DOT3_RGB                                                        34478
#define GL_DOT3_RGBA                                                       34479
typedef GLvoid (csAPIENTRY* csGLACTIVETEXTURE) (GLenum texture);
typedef GLvoid (csAPIENTRY* csGLCLIENTACTIVETEXTURE) (GLenum texture);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD1D) (GLenum target, GLdouble s);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD1DV) (GLenum target, GLdouble* v);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD1F) (GLenum target, GLfloat s);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD1FV) (GLenum target, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD1I) (GLenum target, GLint s);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD1IV) (GLenum target, GLint* v);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD1S) (GLenum target, GLshort s);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD1SV) (GLenum target, GLshort* v);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD2D) (GLenum target, GLdouble s, GLdouble t);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD2DV) (GLenum target, GLdouble* v);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD2F) (GLenum target, GLfloat s, GLfloat t);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD2FV) (GLenum target, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD2I) (GLenum target, GLint s, GLint t);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD2IV) (GLenum target, GLint* v);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD2S) (GLenum target, GLshort s, GLshort t);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD2SV) (GLenum target, GLshort* v);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD3D) (GLenum target, GLdouble s, GLdouble t, GLdouble r);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD3DV) (GLenum target, GLdouble* v);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD3F) (GLenum target, GLfloat s, GLfloat t, GLfloat r);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD3FV) (GLenum target, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD3I) (GLenum target, GLint s, GLint t, GLint r);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD3IV) (GLenum target, GLint* v);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD3S) (GLenum target, GLshort s, GLshort t, GLshort r);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD3SV) (GLenum target, GLshort* v);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD4D) (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD4DV) (GLenum target, GLdouble* v);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD4F) (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD4FV) (GLenum target, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD4I) (GLenum target, GLint s, GLint t, GLint r, GLint q);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD4IV) (GLenum target, GLint* v);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD4S) (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD4SV) (GLenum target, GLshort* v);
typedef GLvoid (csAPIENTRY* csGLLOADTRANSPOSEMATRIXF) (GLfloat* m);
typedef GLvoid (csAPIENTRY* csGLLOADTRANSPOSEMATRIXD) (GLdouble* m);
typedef GLvoid (csAPIENTRY* csGLMULTTRANSPOSEMATRIXF) (GLfloat* m);
typedef GLvoid (csAPIENTRY* csGLMULTTRANSPOSEMATRIXD) (GLdouble* m);
typedef GLvoid (csAPIENTRY* csGLSAMPLECOVERAGE) (GLclampf value, GLboolean invert);
typedef GLvoid (csAPIENTRY* csGLCOMPRESSEDTEXIMAGE3D) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, GLvoid* data);
typedef GLvoid (csAPIENTRY* csGLCOMPRESSEDTEXIMAGE2D) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, GLvoid* data);
typedef GLvoid (csAPIENTRY* csGLCOMPRESSEDTEXIMAGE1D) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, GLvoid* data);
typedef GLvoid (csAPIENTRY* csGLCOMPRESSEDTEXSUBIMAGE3D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, GLvoid* data);
typedef GLvoid (csAPIENTRY* csGLCOMPRESSEDTEXSUBIMAGE2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, GLvoid* data);
typedef GLvoid (csAPIENTRY* csGLCOMPRESSEDTEXSUBIMAGE1D) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, GLvoid* data);
typedef GLvoid (csAPIENTRY* csGLGETCOMPRESSEDTEXIMAGE) (GLenum target, GLint level, GLvoid* img);

// GL_ARB_multitexture
#define GL_TEXTURE0_ARB                                                    33984
#define GL_TEXTURE1_ARB                                                    33985
#define GL_TEXTURE2_ARB                                                    33986
#define GL_TEXTURE3_ARB                                                    33987
#define GL_TEXTURE4_ARB                                                    33988
#define GL_TEXTURE5_ARB                                                    33989
#define GL_TEXTURE6_ARB                                                    33990
#define GL_TEXTURE7_ARB                                                    33991
#define GL_TEXTURE8_ARB                                                    33992
#define GL_TEXTURE9_ARB                                                    33993
#define GL_TEXTURE10_ARB                                                   33994
#define GL_TEXTURE11_ARB                                                   33995
#define GL_TEXTURE12_ARB                                                   33996
#define GL_TEXTURE13_ARB                                                   33997
#define GL_TEXTURE14_ARB                                                   33998
#define GL_TEXTURE15_ARB                                                   33999
#define GL_TEXTURE16_ARB                                                   34000
#define GL_TEXTURE17_ARB                                                   34001
#define GL_TEXTURE18_ARB                                                   34002
#define GL_TEXTURE19_ARB                                                   34003
#define GL_TEXTURE20_ARB                                                   34004
#define GL_TEXTURE21_ARB                                                   34005
#define GL_TEXTURE22_ARB                                                   34006
#define GL_TEXTURE23_ARB                                                   34007
#define GL_TEXTURE24_ARB                                                   34008
#define GL_TEXTURE25_ARB                                                   34009
#define GL_TEXTURE26_ARB                                                   34010
#define GL_TEXTURE27_ARB                                                   34011
#define GL_TEXTURE28_ARB                                                   34012
#define GL_TEXTURE29_ARB                                                   34013
#define GL_TEXTURE30_ARB                                                   34014
#define GL_TEXTURE31_ARB                                                   34015
#define GL_ACTIVE_TEXTURE_ARB                                              34016
#define GL_CLIENT_ACTIVE_TEXTURE_ARB                                       34017
#define GL_MAX_TEXTURE_UNITS_ARB                                           34018
typedef GLvoid (csAPIENTRY* csGLACTIVETEXTUREARB) (GLenum texture);
typedef GLvoid (csAPIENTRY* csGLCLIENTACTIVETEXTUREARB) (GLenum texture);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD1DARB) (GLenum target, GLdouble s);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD1DVARB) (GLenum target, GLdouble* v);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD1FARB) (GLenum target, GLfloat s);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD1FVARB) (GLenum target, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD1IARB) (GLenum target, GLint s);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD1IVARB) (GLenum target, GLint* v);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD1SARB) (GLenum target, GLshort s);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD1SVARB) (GLenum target, GLshort* v);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD2DARB) (GLenum target, GLdouble s, GLdouble t);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD2DVARB) (GLenum target, GLdouble* v);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD2FARB) (GLenum target, GLfloat s, GLfloat t);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD2FVARB) (GLenum target, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD2IARB) (GLenum target, GLint s, GLint t);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD2IVARB) (GLenum target, GLint* v);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD2SARB) (GLenum target, GLshort s, GLshort t);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD2SVARB) (GLenum target, GLshort* v);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD3DARB) (GLenum target, GLdouble s, GLdouble t, GLdouble r);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD3DVARB) (GLenum target, GLdouble* v);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD3FARB) (GLenum target, GLfloat s, GLfloat t, GLfloat r);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD3FVARB) (GLenum target, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD3IARB) (GLenum target, GLint s, GLint t, GLint r);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD3IVARB) (GLenum target, GLint* v);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD3SARB) (GLenum target, GLshort s, GLshort t, GLshort r);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD3SVARB) (GLenum target, GLshort* v);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD4DARB) (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD4DVARB) (GLenum target, GLdouble* v);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD4FARB) (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD4FVARB) (GLenum target, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD4IARB) (GLenum target, GLint s, GLint t, GLint r, GLint q);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD4IVARB) (GLenum target, GLint* v);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD4SARB) (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
typedef GLvoid (csAPIENTRY* csGLMULTITEXCOORD4SVARB) (GLenum target, GLshort* v);

// GL_ARB_transpose_matrix
#define GL_TRANSPOSE_MODELVIEW_MATRIX_ARB                                  34019
#define GL_TRANSPOSE_PROJECTION_MATRIX_ARB                                 34020
#define GL_TRANSPOSE_TEXTURE_MATRIX_ARB                                    34021
#define GL_TRANSPOSE_COLOR_MATRIX_ARB                                      34022
typedef GLvoid (csAPIENTRY* csGLLOADTRANSPOSEMATRIXFARB) (GLfloat* m);
typedef GLvoid (csAPIENTRY* csGLLOADTRANSPOSEMATRIXDARB) (GLdouble* m);
typedef GLvoid (csAPIENTRY* csGLMULTTRANSPOSEMATRIXFARB) (GLfloat* m);
typedef GLvoid (csAPIENTRY* csGLMULTTRANSPOSEMATRIXDARB) (GLdouble* m);

// GL_ARB_multisample
#define WGL_SAMPLE_BUFFERS_ARB                                             8257
#define WGL_SAMPLES_ARB                                                    8258
#define GL_MULTISAMPLE_ARB                                                 32925
#define GL_SAMPLE_ALPHA_TO_COVERAGE_ARB                                    32926
#define GL_SAMPLE_ALPHA_TO_ONE_ARB                                         32927
#define GL_SAMPLE_COVERAGE_ARB                                             32928
#define GL_MULTISAMPLE_BIT_ARB                                             536870912
#define GL_SAMPLE_BUFFERS_ARB                                              32936
#define GL_SAMPLES_ARB                                                     32937
#define GL_SAMPLE_COVERAGE_VALUE_ARB                                       32938
#define GL_SAMPLE_COVERAGE_INVERT_ARB                                      32939
typedef GLvoid (csAPIENTRY* csGLSAMPLECOVERAGEARB) (GLclampf value, GLboolean invert);

// GL_ARB_texture_env_add

// WGL_ARB_extensions_string
typedef char* (csAPIENTRY* csWGLGETEXTENSIONSSTRINGARB) (HDC hdc);

// WGL_ARB_buffer_region
#define WGL_FRONT_COLOR_BUFFER_BIT_ARB                                     1
#define WGL_BACK_COLOR_BUFFER_BIT_ARB                                      2
#define WGL_DEPTH_BUFFER_BIT_ARB                                           4
#define WGL_STENCIL_BUFFER_BIT_ARB                                         8
typedef HANDLE (csAPIENTRY* csWGLCREATEBUFFERREGIONARB) (HDC hDC, GLint iLayerPlane, GLuint uType);
typedef GLvoid (csAPIENTRY* csWGLDELETEBUFFERREGIONARB) (HANDLE hRegion);
typedef BOOL (csAPIENTRY* csWGLSAVEBUFFERREGIONARB) (HANDLE hRegion, GLint x, GLint y, GLint width, GLint height);
typedef BOOL (csAPIENTRY* csWGLRESTOREBUFFERREGIONARB) (HANDLE hRegion, GLint x, GLint y, GLint width, GLint height, GLint xSrc, GLint ySrc);

// GL_ARB_texture_cube_map
#define GL_NORMAL_MAP_ARB                                                  34065
#define GL_REFLECTION_MAP_ARB                                              34066
#define GL_TEXTURE_CUBE_MAP_ARB                                            34067
#define GL_TEXTURE_BINDING_CUBE_MAP_ARB                                    34068
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB                                 34069
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB                                 34070
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB                                 34071
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB                                 34072
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB                                 34073
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB                                 34074
#define GL_PROXY_TEXTURE_CUBE_MAP_ARB                                      34075
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB                                   34076

// GL_ARB_depth_texture
#define GL_DEPTH_COMPONENT16_ARB                                           33189
#define GL_DEPTH_COMPONENT24_ARB                                           33190
#define GL_DEPTH_COMPONENT32_ARB                                           33191
#define GL_TEXTURE_DEPTH_SIZE_ARB                                          34890
#define GL_DEPTH_TEXTURE_MODE_ARB                                          34891

// GL_ARB_point_parameters
#define GL_POINT_SIZE_MIN_ARB                                              33062
#define GL_POINT_SIZE_MAX_ARB                                              33063
#define GL_POINT_FADE_THRESHOLD_SIZE_ARB                                   33064
#define GL_POINT_DISTANCE_ATTENUATION_ARB                                  33065
typedef GLvoid (csAPIENTRY* csGLPOINTPARAMETERFARB) (GLenum pname, GLfloat param);
typedef GLvoid (csAPIENTRY* csGLPOINTPARAMETERFVARB) (GLenum pname, GLfloat* params);

// GL_ARB_shadow
#define GL_TEXTURE_COMPARE_MODE_ARB                                        34892
#define GL_TEXTURE_COMPARE_FUNC_ARB                                        34893
#define GL_COMPARE_R_TO_TEXTURE_ARB                                        34894

// GL_ARB_shadow_ambient
#define GL_TEXTURE_COMPARE_FAIL_VALUE_ARB                                  32959

// GL_ARB_texture_border_clamp
#define GL_CLAMP_TO_BORDER_ARB                                             33069

// GL_ARB_texture_compression
#define GL_COMPRESSED_ALPHA_ARB                                            34025
#define GL_COMPRESSED_LUMINANCE_ARB                                        34026
#define GL_COMPRESSED_LUMINANCE_ALPHA_ARB                                  34027
#define GL_COMPRESSED_INTENSITY_ARB                                        34028
#define GL_COMPRESSED_RGB_ARB                                              34029
#define GL_COMPRESSED_RGBA_ARB                                             34030
#define GL_TEXTURE_COMPRESSION_HINT_ARB                                    34031
#define GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB                               34464
#define GL_TEXTURE_COMPRESSED_ARB                                          34465
#define GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB                              34466
#define GL_COMPRESSED_TEXTURE_FORMATS_ARB                                  34467
typedef GLvoid (csAPIENTRY* csGLCOMPRESSEDTEXIMAGE3DARB) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, GLvoid* data);
typedef GLvoid (csAPIENTRY* csGLCOMPRESSEDTEXIMAGE2DARB) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, GLvoid* data);
typedef GLvoid (csAPIENTRY* csGLCOMPRESSEDTEXIMAGE1DARB) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, GLvoid* data);
typedef GLvoid (csAPIENTRY* csGLCOMPRESSEDTEXSUBIMAGE3DARB) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, GLvoid* data);
typedef GLvoid (csAPIENTRY* csGLCOMPRESSEDTEXSUBIMAGE2DARB) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, GLvoid* data);
typedef GLvoid (csAPIENTRY* csGLCOMPRESSEDTEXSUBIMAGE1DARB) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, GLvoid* data);
typedef GLvoid (csAPIENTRY* csGLGETCOMPRESSEDTEXIMAGEARB) (GLenum target, GLint lod, GLvoid* img);

// GL_ARB_texture_env_combine
#define GL_COMBINE_ARB                                                     34160
#define GL_COMBINE_RGB_ARB                                                 34161
#define GL_COMBINE_ALPHA_ARB                                               34162
#define GL_SOURCE0_RGB_ARB                                                 34176
#define GL_SOURCE1_RGB_ARB                                                 34177
#define GL_SOURCE2_RGB_ARB                                                 34178
#define GL_SOURCE0_ALPHA_ARB                                               34184
#define GL_SOURCE1_ALPHA_ARB                                               34185
#define GL_SOURCE2_ALPHA_ARB                                               34186
#define GL_OPERAND0_RGB_ARB                                                34192
#define GL_OPERAND1_RGB_ARB                                                34193
#define GL_OPERAND2_RGB_ARB                                                34194
#define GL_OPERAND0_ALPHA_ARB                                              34200
#define GL_OPERAND1_ALPHA_ARB                                              34201
#define GL_OPERAND2_ALPHA_ARB                                              34202
#define GL_RGB_SCALE_ARB                                                   34163
#define GL_ADD_SIGNED_ARB                                                  34164
#define GL_INTERPOLATE_ARB                                                 34165
#define GL_SUBTRACT_ARB                                                    34023
#define GL_CONSTANT_ARB                                                    34166
#define GL_PRIMARY_COLOR_ARB                                               34167
#define GL_PREVIOUS_ARB                                                    34168

// GL_ARB_texture_env_crossbar

// GL_ARB_texture_env_dot3
#define GL_DOT3_RGB_ARB                                                    34478
#define GL_DOT3_RGBA_ARB                                                   34479

// GL_ARB_texture_mirrored_repeat
#define GL_MIRRORED_REPEAT_ARB                                             33648

// GL_ARB_vertex_blend
#define GL_MAX_VERTEX_UNITS_ARB                                            34468
#define GL_ACTIVE_VERTEX_UNITS_ARB                                         34469
#define GL_WEIGHT_SUM_UNITY_ARB                                            34470
#define GL_VERTEX_BLEND_ARB                                                34471
#define GL_MODELVIEW0_ARB                                                  5888
#define GL_MODELVIEW1_ARB                                                  34058
#define GL_MODELVIEW2_ARB                                                  34594
#define GL_MODELVIEW3_ARB                                                  34595
#define GL_MODELVIEW4_ARB                                                  34596
#define GL_MODELVIEW5_ARB                                                  34597
#define GL_MODELVIEW6_ARB                                                  34598
#define GL_MODELVIEW7_ARB                                                  34599
#define GL_MODELVIEW8_ARB                                                  34600
#define GL_MODELVIEW9_ARB                                                  34601
#define GL_MODELVIEW10_ARB                                                 34602
#define GL_MODELVIEW11_ARB                                                 34603
#define GL_MODELVIEW12_ARB                                                 34604
#define GL_MODELVIEW13_ARB                                                 34605
#define GL_MODELVIEW14_ARB                                                 34606
#define GL_MODELVIEW15_ARB                                                 34607
#define GL_MODELVIEW16_ARB                                                 34608
#define GL_MODELVIEW17_ARB                                                 34609
#define GL_MODELVIEW18_ARB                                                 34610
#define GL_MODELVIEW19_ARB                                                 34611
#define GL_MODELVIEW20_ARB                                                 34612
#define GL_MODELVIEW21_ARB                                                 34613
#define GL_MODELVIEW22_ARB                                                 34614
#define GL_MODELVIEW23_ARB                                                 34615
#define GL_MODELVIEW24_ARB                                                 34616
#define GL_MODELVIEW25_ARB                                                 34617
#define GL_MODELVIEW26_ARB                                                 34618
#define GL_MODELVIEW27_ARB                                                 34619
#define GL_MODELVIEW28_ARB                                                 34620
#define GL_MODELVIEW29_ARB                                                 34621
#define GL_MODELVIEW30_ARB                                                 34622
#define GL_MODELVIEW31_ARB                                                 34623
#define GL_CURRENT_WEIGHT_ARB                                              34472
#define GL_WEIGHT_ARRAY_TYPE_ARB                                           34473
#define GL_WEIGHT_ARRAY_STRIDE_ARB                                         34474
#define GL_WEIGHT_ARRAY_SIZE_ARB                                           34475
#define GL_WEIGHT_ARRAY_POINTER_ARB                                        34476
#define GL_WEIGHT_ARRAY_ARB                                                34477
typedef GLvoid (csAPIENTRY* csGLWEIGHTBVARB) (GLint size, GLbyte* weights);
typedef GLvoid (csAPIENTRY* csGLWEIGHTSVARB) (GLint size, GLshort* weights);
typedef GLvoid (csAPIENTRY* csGLWEIGHTIVARB) (GLint size, GLint* weights);
typedef GLvoid (csAPIENTRY* csGLWEIGHTFVARB) (GLint size, GLfloat* weights);
typedef GLvoid (csAPIENTRY* csGLWEIGHTDVARB) (GLint size, GLdouble* weights);
typedef GLvoid (csAPIENTRY* csGLWEIGHTVARB) (GLint size, GLdouble* weights);
typedef GLvoid (csAPIENTRY* csGLWEIGHTUBVARB) (GLint size, GLubyte* weights);
typedef GLvoid (csAPIENTRY* csGLWEIGHTUSVARB) (GLint size, GLushort* weights);
typedef GLvoid (csAPIENTRY* csGLWEIGHTUIVARB) (GLint size, GLuint* weights);
typedef GLvoid (csAPIENTRY* csGLWEIGHTPOINTERARB) (GLint size, GLenum type, GLsizei stride, GLvoid* pointer);
typedef GLvoid (csAPIENTRY* csGLVERTEXBLENDARB) (GLint count);

// GL_ARB_vertex_program
#define GL_VERTEX_PROGRAM_ARB                                              34336
#define GL_VERTEX_PROGRAM_POINT_SIZE_ARB                                   34370
#define GL_VERTEX_PROGRAM_TWO_SIDE_ARB                                     34371
#define GL_COLOR_SUM_ARB                                                   33880
#define GL_PROGRAM_FORMAT_ASCII_ARB                                        34933
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED_ARB                                 34338
#define GL_VERTEX_ATTRIB_ARRAY_SIZE_ARB                                    34339
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE_ARB                                  34340
#define GL_VERTEX_ATTRIB_ARRAY_TYPE_ARB                                    34341
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED_ARB                              34922
#define GL_CURRENT_VERTEX_ATTRIB_ARB                                       34342
#define GL_VERTEX_ATTRIB_ARRAY_POINTER_ARB                                 34373
#define GL_PROGRAM_LENGTH_ARB                                              34343
#define GL_PROGRAM_FORMAT_ARB                                              34934
#define GL_PROGRAM_BINDING_ARB                                             34423
#define GL_PROGRAM_INSTRUCTIONS_ARB                                        34976
#define GL_MAX_PROGRAM_INSTRUCTIONS_ARB                                    34977
#define GL_PROGRAM_NATIVE_INSTRUCTIONS_ARB                                 34978
#define GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB                             34979
#define GL_PROGRAM_TEMPORARIES_ARB                                         34980
#define GL_MAX_PROGRAM_TEMPORARIES_ARB                                     34981
#define GL_PROGRAM_NATIVE_TEMPORARIES_ARB                                  34982
#define GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB                              34983
#define GL_PROGRAM_PARAMETERS_ARB                                          34984
#define GL_MAX_PROGRAM_PARAMETERS_ARB                                      34985
#define GL_PROGRAM_NATIVE_PARAMETERS_ARB                                   34986
#define GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB                               34987
#define GL_PROGRAM_ATTRIBS_ARB                                             34988
#define GL_MAX_PROGRAM_ATTRIBS_ARB                                         34989
#define GL_PROGRAM_NATIVE_ATTRIBS_ARB                                      34990
#define GL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB                                  34991
#define GL_PROGRAM_ADDRESS_REGISTERS_ARB                                   34992
#define GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB                               34993
#define GL_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB                            34994
#define GL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB                        34995
#define GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB                                34996
#define GL_MAX_PROGRAM_ENV_PARAMETERS_ARB                                  34997
#define GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB                                 34998
#define GL_PROGRAM_STRING_ARB                                              34344
#define GL_PROGRAM_ERROR_POSITION_ARB                                      34379
#define GL_CURRENT_MATRIX_ARB                                              34369
#define GL_TRANSPOSE_CURRENT_MATRIX_ARB                                    34999
#define GL_CURRENT_MATRIX_STACK_DEPTH_ARB                                  34368
#define GL_MAX_VERTEX_ATTRIBS_ARB                                          34921
#define GL_MAX_PROGRAM_MATRICES_ARB                                        34351
#define GL_MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB                              34350
#define GL_PROGRAM_ERROR_STRING_ARB                                        34932
#define GL_MATRIX0_ARB                                                     35008
#define GL_MATRIX1_ARB                                                     35009
#define GL_MATRIX2_ARB                                                     35010
#define GL_MATRIX3_ARB                                                     35011
#define GL_MATRIX4_ARB                                                     35012
#define GL_MATRIX5_ARB                                                     35013
#define GL_MATRIX6_ARB                                                     35014
#define GL_MATRIX7_ARB                                                     35015
#define GL_MATRIX8_ARB                                                     35016
#define GL_MATRIX9_ARB                                                     35017
#define GL_MATRIX10_ARB                                                    35018
#define GL_MATRIX11_ARB                                                    35019
#define GL_MATRIX12_ARB                                                    35020
#define GL_MATRIX13_ARB                                                    35021
#define GL_MATRIX14_ARB                                                    35022
#define GL_MATRIX15_ARB                                                    35023
#define GL_MATRIX16_ARB                                                    35024
#define GL_MATRIX17_ARB                                                    35025
#define GL_MATRIX18_ARB                                                    35026
#define GL_MATRIX19_ARB                                                    35027
#define GL_MATRIX20_ARB                                                    35028
#define GL_MATRIX21_ARB                                                    35029
#define GL_MATRIX22_ARB                                                    35030
#define GL_MATRIX23_ARB                                                    35031
#define GL_MATRIX24_ARB                                                    35032
#define GL_MATRIX25_ARB                                                    35033
#define GL_MATRIX26_ARB                                                    35034
#define GL_MATRIX27_ARB                                                    35035
#define GL_MATRIX28_ARB                                                    35036
#define GL_MATRIX29_ARB                                                    35037
#define GL_MATRIX30_ARB                                                    35038
#define GL_MATRIX31_ARB                                                    35039
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB1SARB) (GLuint index, GLshort x);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB1FARB) (GLuint index, GLfloat x);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB1DARB) (GLuint index, GLdouble x);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB2SARB) (GLuint index, GLshort x, GLshort y);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB2FARB) (GLuint index, GLfloat x, GLfloat y);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB2DARB) (GLuint index, GLdouble x, GLdouble y);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB3SARB) (GLuint index, GLshort x, GLshort y, GLshort z);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB3FARB) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB3DARB) (GLuint index, GLdouble x, GLdouble y, GLdouble z);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB4SARB) (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB4FARB) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB4DARB) (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB4NUBARB) (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB1SVARB) (GLuint index, GLshort* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB1FVARB) (GLuint index, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB1DVARB) (GLuint index, GLdouble* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB2SVARB) (GLuint index, GLshort* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB2FVARB) (GLuint index, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB2DVARB) (GLuint index, GLdouble* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB3SVARB) (GLuint index, GLshort* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB3FVARB) (GLuint index, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB3DVARB) (GLuint index, GLdouble* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB4BVARB) (GLuint index, GLbyte* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB4SVARB) (GLuint index, GLshort* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB4IVARB) (GLuint index, GLint* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB4UBVARB) (GLuint index, GLubyte* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB4USVARB) (GLuint index, GLushort* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB4UIVARB) (GLuint index, GLuint* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB4FVARB) (GLuint index, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB4DVARB) (GLuint index, GLdouble* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB4NBVARB) (GLuint index, GLbyte* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB4NSVARB) (GLuint index, GLshort* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB4NIVARB) (GLuint index, GLint* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB4NUBVARB) (GLuint index, GLubyte* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB4NUSVARB) (GLuint index, GLushort* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB4NUIVARB) (GLuint index, GLuint* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIBPOINTERARB) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLvoid* pointer);
typedef GLvoid (csAPIENTRY* csGLENABLEVERTEXATTRIBARRAYARB) (GLuint index);
typedef GLvoid (csAPIENTRY* csGLDISABLEVERTEXATTRIBARRAYARB) (GLuint index);
typedef GLvoid (csAPIENTRY* csGLPROGRAMSTRINGARB) (GLenum target, GLenum format, GLsizei len, GLvoid* string);
typedef GLvoid (csAPIENTRY* csGLBINDPROGRAMARB) (GLenum target, GLuint program);
typedef GLvoid (csAPIENTRY* csGLDELETEPROGRAMSARB) (GLsizei n, GLuint* programs);
typedef GLvoid (csAPIENTRY* csGLGENPROGRAMSARB) (GLsizei n, GLuint* programs);
typedef GLvoid (csAPIENTRY* csGLPROGRAMENVPARAMETER4DARB) (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef GLvoid (csAPIENTRY* csGLPROGRAMENVPARAMETER4DVARB) (GLenum target, GLuint index, GLdouble* params);
typedef GLvoid (csAPIENTRY* csGLPROGRAMENVPARAMETER4FARB) (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef GLvoid (csAPIENTRY* csGLPROGRAMENVPARAMETER4FVARB) (GLenum target, GLuint index, GLfloat* params);
typedef GLvoid (csAPIENTRY* csGLPROGRAMLOCALPARAMETER4DARB) (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef GLvoid (csAPIENTRY* csGLPROGRAMLOCALPARAMETER4DVARB) (GLenum target, GLuint index, GLdouble* params);
typedef GLvoid (csAPIENTRY* csGLPROGRAMLOCALPARAMETER4FARB) (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef GLvoid (csAPIENTRY* csGLPROGRAMLOCALPARAMETER4FVARB) (GLenum target, GLuint index, GLfloat* params);
typedef GLvoid (csAPIENTRY* csGLGETPROGRAMENVPARAMETERDVARB) (GLenum target, GLuint index, GLdouble* params);
typedef GLvoid (csAPIENTRY* csGLGETPROGRAMENVPARAMETERFVARB) (GLenum target, GLuint index, GLfloat* params);
typedef GLvoid (csAPIENTRY* csGLGETPROGRAMLOCALPARAMETERDVARB) (GLenum target, GLuint index, GLdouble* params);
typedef GLvoid (csAPIENTRY* csGLGETPROGRAMLOCALPARAMETERFVARB) (GLenum target, GLuint index, GLfloat* params);
typedef GLvoid (csAPIENTRY* csGLGETPROGRAMIVARB) (GLenum target, GLenum pname, GLint* params);
typedef GLvoid (csAPIENTRY* csGLGETPROGRAMSTRINGARB) (GLenum target, GLenum pname, GLvoid* string);
typedef GLvoid (csAPIENTRY* csGLGETVERTEXATTRIBDVARB) (GLuint index, GLenum pname, GLdouble* params);
typedef GLvoid (csAPIENTRY* csGLGETVERTEXATTRIBFVARB) (GLuint index, GLenum pname, GLfloat* params);
typedef GLvoid (csAPIENTRY* csGLGETVERTEXATTRIBIVARB) (GLuint index, GLenum pname, GLint* params);
typedef GLvoid (csAPIENTRY* csGLGETVERTEXATTRIBPOINTERVARB) (GLuint index, GLenum pname, GLvoid* pointer);
typedef GLboolean (csAPIENTRY* csGLISPROGRAMARB) (GLuint program);

// GL_ARB_window_pos
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS2DARB) (GLdouble x, GLdouble y);
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS2FARB) (GLfloat x, GLfloat y);
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS2IARB) (GLint x, GLint y);
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS2SARB) (GLshort x, GLshort y);
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS2DVARB) (GLdouble* p);
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS2FVARB) (GLfloat* p);
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS2IVARB) (GLint* p);
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS2SVARB) (GLshort* p);
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS3DARB) (GLdouble x, GLdouble y, GLdouble z);
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS3FARB) (GLfloat x, GLfloat y, GLfloat z);
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS3IARB) (GLint x, GLint y, GLint z);
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS3SARB) (GLshort x, GLshort y, GLshort z);
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS3DVARB) (GLdouble* p);
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS3FVARB) (GLfloat* p);
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS3IVARB) (GLint* p);
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS3SVARB) (GLshort* p);

// GL_EXT_422_pixels
#define GL_422_EXT                                                         32972
#define GL_422_REV_EXT                                                     32973
#define GL_422_AVERAGE_EXT                                                 32974
#define GL_422_REV_AVERAGE_EXT                                             32975

// GL_EXT_abgr
#define GL_ABGR_EXT                                                        32768

// GL_EXT_bgra
#define GL_BGR_EXT                                                         32992
#define GL_BGRA_EXT                                                        32993

// GL_EXT_blend_color
#define GL_CONSTANT_COLOR_EXT                                              32769
#define GL_ONE_MINUS_CONSTANT_COLOR_EXT                                    32770
#define GL_CONSTANT_ALPHA_EXT                                              32771
#define GL_ONE_MINUS_CONSTANT_ALPHA_EXT                                    32772
#define GL_BLEND_COLOR_EXT                                                 32773
typedef GLvoid (csAPIENTRY* csGLBLENDCOLOREXT) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);

// GL_EXT_blend_func_separate
#define GL_BLEND_DST_RGB_EXT                                               32968
#define GL_BLEND_SRC_RGB_EXT                                               32969
#define GL_BLEND_DST_ALPHA_EXT                                             32970
#define GL_BLEND_SRC_ALPHA_EXT                                             32971
typedef GLvoid (csAPIENTRY* csGLBLENDFUNCSEPARATEEXT) (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);

// GL_EXT_blend_logic_op

// GL_EXT_blend_minmax
#define GL_FUNC_ADD_EXT                                                    32774
#define GL_MIN_EXT                                                         32775
#define GL_MAX_EXT                                                         32776
#define GL_BLEND_EQUATION_EXT                                              32777
typedef GLvoid (csAPIENTRY* csGLBLENDEQUATIONEXT) (GLenum mode);

// GL_EXT_blend_subtract
#define GL_FUNC_SUBTRACT_EXT                                               32778
#define GL_FUNC_REVERSE_SUBTRACT_EXT                                       32779

// GL_EXT_clip_volume_hint
#define GL_CLIP_VOLUME_CLIPPING_HINT_EXT                                   33008

// GL_EXT_color_subtable
typedef GLvoid (csAPIENTRY* csGLCOLORSUBTABLEEXT) (GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, GLvoid* data);
typedef GLvoid (csAPIENTRY* csGLCOPYCOLORSUBTABLEEXT) (GLenum target, GLsizei start, GLint x, GLint y, GLsizei width);

// GL_EXT_compiled_vertex_array
#define GL_ARRAY_ELEMENT_LOCK_FIRST_EXT                                    33192
#define GL_ARRAY_ELEMENT_LOCK_COUNT_EXT                                    33193
typedef GLvoid (csAPIENTRY* csGLLOCKARRAYSEXT) (GLint first, GLsizei count);
typedef GLvoid (csAPIENTRY* csGLUNLOCKARRAYSEXT) ();

// GL_EXT_convolution
#define GL_CONVOLUTION_1D_EXT                                              32784
#define GL_CONVOLUTION_2D_EXT                                              32785
#define GL_SEPARABLE_2D_EXT                                                32786
#define GL_CONVOLUTION_BORDER_MODE_EXT                                     32787
#define GL_CONVOLUTION_FILTER_SCALE_EXT                                    32788
#define GL_CONVOLUTION_FILTER_BIAS_EXT                                     32789
#define GL_REDUCE_EXT                                                      32790
#define GL_CONVOLUTION_FORMAT_EXT                                          32791
#define GL_CONVOLUTION_WIDTH_EXT                                           32792
#define GL_CONVOLUTION_HEIGHT_EXT                                          32793
#define GL_MAX_CONVOLUTION_WIDTH_EXT                                       32794
#define GL_MAX_CONVOLUTION_HEIGHT_EXT                                      32795
#define GL_POST_CONVOLUTION_RED_SCALE_EXT                                  32796
#define GL_POST_CONVOLUTION_GREEN_SCALE_EXT                                32797
#define GL_POST_CONVOLUTION_BLUE_SCALE_EXT                                 32798
#define GL_POST_CONVOLUTION_ALPHA_SCALE_EXT                                32799
#define GL_POST_CONVOLUTION_RED_BIAS_EXT                                   32800
#define GL_POST_CONVOLUTION_GREEN_BIAS_EXT                                 32801
#define GL_POST_CONVOLUTION_BLUE_BIAS_EXT                                  32802
#define GL_POST_CONVOLUTION_ALPHA_BIAS_EXT                                 32803
typedef GLvoid (csAPIENTRY* csGLCONVOLUTIONFILTER1DEXT) (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, GLvoid* image);
typedef GLvoid (csAPIENTRY* csGLCONVOLUTIONFILTER2DEXT) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* image);
typedef GLvoid (csAPIENTRY* csGLCOPYCONVOLUTIONFILTER1DEXT) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
typedef GLvoid (csAPIENTRY* csGLCOPYCONVOLUTIONFILTER2DEXT) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height);
typedef GLvoid (csAPIENTRY* csGLGETCONVOLUTIONFILTEREXT) (GLenum target, GLenum format, GLenum type, GLvoid* image);
typedef GLvoid (csAPIENTRY* csGLSEPARABLEFILTER2DEXT) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* row, GLvoid* column);
typedef GLvoid (csAPIENTRY* csGLGETSEPARABLEFILTEREXT) (GLenum target, GLenum format, GLenum type, GLvoid* row, GLvoid* column, GLvoid* span);
typedef GLvoid (csAPIENTRY* csGLCONVOLUTIONPARAMETERIEXT) (GLenum target, GLenum pname, GLint param);
typedef GLvoid (csAPIENTRY* csGLCONVOLUTIONPARAMETERIVEXT) (GLenum target, GLenum pname, GLint* params);
typedef GLvoid (csAPIENTRY* csGLCONVOLUTIONPARAMETERFEXT) (GLenum target, GLenum pname, GLfloat param);
typedef GLvoid (csAPIENTRY* csGLCONVOLUTIONPARAMETERFVEXT) (GLenum target, GLenum pname, GLfloat* params);
typedef GLvoid (csAPIENTRY* csGLGETCONVOLUTIONPARAMETERIVEXT) (GLenum target, GLenum pname, GLint* params);
typedef GLvoid (csAPIENTRY* csGLGETCONVOLUTIONPARAMETERFVEXT) (GLenum target, GLenum pname, GLfloat* params);

// GL_EXT_fog_coord
#define GL_FOG_COORDINATE_SOURCE_EXT                                       33872
#define GL_FOG_COORDINATE_EXT                                              33873
#define GL_FRAGMENT_DEPTH_EXT                                              33874
#define GL_CURRENT_FOG_COORDINATE_EXT                                      33875
#define GL_FOG_COORDINATE_ARRAY_TYPE_EXT                                   33876
#define GL_FOG_COORDINATE_ARRAY_STRIDE_EXT                                 33877
#define GL_FOG_COORDINATE_ARRAY_POINTER_EXT                                33878
#define GL_FOG_COORDINATE_ARRAY_EXT                                        33879
typedef GLvoid (csAPIENTRY* csGLFOGCOORDFEXFLOAT) (GLfloat coord);
typedef GLvoid (csAPIENTRY* csGLFOGCOORDDEXDOUBLE) (GLdouble coord);
typedef GLvoid (csAPIENTRY* csGLFOGCOORDFVEXFLOAT) (GLfloat coord);
typedef GLvoid (csAPIENTRY* csGLFOGCOORDDVEXDOUBLE) (GLdouble coord);
typedef GLvoid (csAPIENTRY* csGLFOGCOORDPOINTEREXT) (GLenum type, GLsizei stride, GLvoid* pointer);

// GL_EXT_histogram
#define GL_HISTOGRAM_EXT                                                   32804
#define GL_PROXY_HISTOGRAM_EXT                                             32805
#define GL_HISTOGRAM_WIDTH_EXT                                             32806
#define GL_HISTOGRAM_FORMAT_EXT                                            32807
#define GL_HISTOGRAM_RED_SIZE_EXT                                          32808
#define GL_HISTOGRAM_GREEN_SIZE_EXT                                        32809
#define GL_HISTOGRAM_BLUE_SIZE_EXT                                         32810
#define GL_HISTOGRAM_ALPHA_SIZE_EXT                                        32811
#define GL_HISTOGRAM_LUMINANCE_SIZE_EXT                                    32812
#define GL_HISTOGRAM_SINK_EXT                                              32813
#define GL_MINMAX_EXT                                                      32814
#define GL_MINMAX_FORMAT_EXT                                               32815
#define GL_MINMAX_SINK_EXT                                                 32816
typedef GLvoid (csAPIENTRY* csGLHISTOGRAMEXT) (GLenum target, GLsizei width, GLenum internalformat, GLboolean sink);
typedef GLvoid (csAPIENTRY* csGLRESETHISTOGRAMEXT) (GLenum target);
typedef GLvoid (csAPIENTRY* csGLGETHISTOGRAMEXT) (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid* values);
typedef GLvoid (csAPIENTRY* csGLGETHISTOGRAMPARAMETERIVEXT) (GLenum target, GLenum pname, GLint* params);
typedef GLvoid (csAPIENTRY* csGLGETHISTOGRAMPARAMETERFVEXT) (GLenum target, GLenum pname, GLfloat* params);
typedef GLvoid (csAPIENTRY* csGLMINMAXEXT) (GLenum target, GLenum internalformat, GLboolean sink);
typedef GLvoid (csAPIENTRY* csGLRESETMINMAXEXT) (GLenum target);
typedef GLvoid (csAPIENTRY* csGLGETMINMAXEXT) (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid* values);
typedef GLvoid (csAPIENTRY* csGLGETMINMAXPARAMETERIVEXT) (GLenum target, GLenum pname, GLint* params);
typedef GLvoid (csAPIENTRY* csGLGETMINMAXPARAMETERFVEXT) (GLenum target, GLenum pname, GLfloat* params);

// GL_EXT_multi_draw_arrays
typedef GLvoid (csAPIENTRY* csGLMULTIDRAWARRAYSEXT) (GLenum mode, GLint* first, GLsizei* count, GLsizei primcount);
typedef GLvoid (csAPIENTRY* csGLMULTIDRAWELEMENTSEXT) (GLenum mode, GLsizei* count, GLenum type, GLvoid* indices, GLsizei primcount);

// GL_EXT_packed_pixels
#define GL_UNSIGNED_BYTE_3_3_2_EXT                                         32818
#define GL_UNSIGNED_SHORT_4_4_4_4_EXT                                      32819
#define GL_UNSIGNED_SHORT_5_5_5_1_EXT                                      32820
#define GL_UNSIGNED_INT_8_8_8_8_EXT                                        32821
#define GL_UNSIGNED_INT_10_10_10_2_EXT                                     32822

// GL_EXT_paletted_texture
#define GL_COLOR_INDEX1_EXT                                                32994
#define GL_COLOR_INDEX2_EXT                                                32995
#define GL_COLOR_INDEX4_EXT                                                32996
#define GL_COLOR_INDEX8_EXT                                                32997
#define GL_COLOR_INDEX12_EXT                                               32998
#define GL_COLOR_INDEX16_EXT                                               32999
#define GL_COLOR_TABLE_FORMAT_EXT                                          32984
#define GL_COLOR_TABLE_WIDTH_EXT                                           32985
#define GL_COLOR_TABLE_RED_SIZE_EXT                                        32986
#define GL_COLOR_TABLE_GREEN_SIZE_EXT                                      32987
#define GL_COLOR_TABLE_BLUE_SIZE_EXT                                       32988
#define GL_COLOR_TABLE_ALPHA_SIZE_EXT                                      32989
#define GL_COLOR_TABLE_LUMINANCE_SIZE_EXT                                  32990
#define GL_COLOR_TABLE_INTENSITY_SIZE_EXT                                  32991
#define GL_TEXTURE_INDEX_SIZE_EXT                                          33005
#define GL_TEXTURE_1D                                                      3552
#define GL_TEXTURE_2D                                                      3553
#define GL_TEXTURE_3D_EXT                                                  32879
#define GL_TEXTURE_CUBE_MAP_ARB                                            34067
#define GL_PROXY_TEXTURE_1D                                                32867
#define GL_PROXY_TEXTURE_2D                                                32868
#define GL_PROXY_TEXTURE_3D_EXT                                            32880
#define GL_PROXY_TEXTURE_CUBE_MAP_ARB                                      34075
#define GL_TEXTURE_1D                                                      3552
#define GL_TEXTURE_2D                                                      3553
#define GL_TEXTURE_3D_EXT                                                  32879
#define GL_TEXTURE_CUBE_MAP_ARB                                            34067
typedef GLvoid (csAPIENTRY* csGLCOLORTABLEEXT) (GLenum target, GLenum internalFormat, GLsizei width, GLenum format, GLenum type, GLvoid* data);
typedef GLvoid (csAPIENTRY* csGLCOLORSUBTABLEEXT) (GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, GLvoid* data);
typedef GLvoid (csAPIENTRY* csGLGETCOLORTABLEEXT) (GLenum target, GLenum format, GLenum type, GLvoid* data);
typedef GLvoid (csAPIENTRY* csGLGETCOLORTABLEPARAMETERIVEXT) (GLenum target, GLenum pname, GLint* params);
typedef GLvoid (csAPIENTRY* csGLGETCOLORTABLEPARAMETERFVEXT) (GLenum target, GLenum pname, GLfloat* params);

// GL_EXT_point_parameters
#define GL_POINT_SIZE_MIN_EXT                                              33062
#define GL_POINT_SIZE_MAX_EXT                                              33063
#define GL_POINT_FADE_THRESHOLD_SIZE_EXT                                   33064
#define GL_DISTANCE_ATTENUATION_EXT                                        33065
typedef GLvoid (csAPIENTRY* csGLPOINTPARAMETERFEXT) (GLenum pname, GLfloat param);
typedef GLvoid (csAPIENTRY* csGLPOINTPARAMETERFVEXT) (GLenum pname, GLfloat* params);

// GL_EXT_polygon_offset
#define GL_POLYGON_OFFSET_EXT                                              32823
#define GL_POLYGON_OFFSET_FACTOR_EXT                                       32824
#define GL_POLYGON_OFFSET_BIAS_EXT                                         32825
typedef GLvoid (csAPIENTRY* csGLPOLYGONOFFSETEXT) (GLfloat factor, GLfloat bias);

// GL_EXT_secondary_color
#define GL_COLOR_SUM_EXT                                                   33880
#define GL_CURRENT_SECONDARY_COLOR_EXT                                     33881
#define GL_SECONDARY_COLOR_ARRAY_SIZE_EXT                                  33882
#define GL_SECONDARY_COLOR_ARRAY_TYPE_EXT                                  33883
#define GL_SECONDARY_COLOR_ARRAY_STRIDE_EXT                                33884
#define GL_SECONDARY_COLOR_ARRAY_POINTER_EXT                               33885
#define GL_SECONDARY_COLOR_ARRAY_EXT                                       33886
typedef GLvoid (csAPIENTRY* csGLSECONDARYCOLOR3BEXT) (GLbyte components);
typedef GLvoid (csAPIENTRY* csGLSECONDARYCOLOR3SEXT) (GLshort components);
typedef GLvoid (csAPIENTRY* csGLSECONDARYCOLOR3IEXT) (GLint components);
typedef GLvoid (csAPIENTRY* csGLSECONDARYCOLOR3FEXT) (GLfloat components);
typedef GLvoid (csAPIENTRY* csGLSECONDARYCOLOR3DEXT) (GLdouble components);
typedef GLvoid (csAPIENTRY* csGLSECONDARYCOLOR3UBEXT) (GLubyte components);
typedef GLvoid (csAPIENTRY* csGLSECONDARYCOLOR3USEXT) (GLushort components);
typedef GLvoid (csAPIENTRY* csGLSECONDARYCOLOR3UIEXT) (GLuint components);
typedef GLvoid (csAPIENTRY* csGLSECONDARYCOLOR3BVEXT) (GLbyte components);
typedef GLvoid (csAPIENTRY* csGLSECONDARYCOLOR3SVEXT) (GLshort components);
typedef GLvoid (csAPIENTRY* csGLSECONDARYCOLOR3IVEXT) (GLint components);
typedef GLvoid (csAPIENTRY* csGLSECONDARYCOLOR3FVEXT) (GLfloat components);
typedef GLvoid (csAPIENTRY* csGLSECONDARYCOLOR3DVEXT) (GLdouble components);
typedef GLvoid (csAPIENTRY* csGLSECONDARYCOLOR3UBVEXT) (GLubyte components);
typedef GLvoid (csAPIENTRY* csGLSECONDARYCOLOR3USVEXT) (GLushort components);
typedef GLvoid (csAPIENTRY* csGLSECONDARYCOLOR3UIVEXT) (GLuint components);
typedef GLvoid (csAPIENTRY* csGLSECONDARYCOLORPOINTEREXT) (GLint size, GLenum type, GLsizei stride, GLvoid* pointer);

// GL_EXT_separate_specular_color
#define GL_LIGHT_MODEL_COLOR_CONTROL_EXT                                   33272
#define GL_SINGLE_COLOR_EXT                                                33273
#define GL_SEPARATE_SPECULAR_COLOR_EXT                                     33274

// GL_EXT_shadow_funcs

// GL_EXT_shared_texture_palette
#define GL_SHARED_TEXTURE_PALETTE_EXT                                      33275

// GL_EXT_stencil_two_side
#define GL_STENCIL_TEST_TWO_SIDE_EXT                                       35088
#define GL_ACTIVE_STENCIL_FACE_EXT                                         35089
typedef GLvoid (csAPIENTRY* csGLACTIVESTENCILFACEEXT) (GLenum face);

// GL_EXT_stencil_wrap
#define GL_INCR_WRAP_EXT                                                   34055
#define GL_DECR_WRAP_EXT                                                   34056

// GL_EXT_subtexture
typedef GLvoid (csAPIENTRY* csGLTEXSUBIMAGE1DEXT) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, GLvoid* pixels);
typedef GLvoid (csAPIENTRY* csGLTEXSUBIMAGE2DEXT) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels);
typedef GLvoid (csAPIENTRY* csGLTEXSUBIMAGE3DEXT) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLvoid* pixels);

// GL_EXT_texture3D
#define GL_PACK_SKIP_IMAGES_EXT                                            32875
#define GL_PACK_IMAGE_HEIGHT_EXT                                           32876
#define GL_UNPACK_SKIP_IMAGES_EXT                                          32877
#define GL_UNPACK_IMAGE_HEIGHT_EXT                                         32878
#define GL_TEXTURE_3D_EXT                                                  32879
#define GL_PROXY_TEXTURE_3D_EXT                                            32880
#define GL_TEXTURE_DEPTH_EXT                                               32881
#define GL_TEXTURE_WRAP_R_EXT                                              32882
#define GL_MAX_3D_TEXTURE_SIZE_EXT                                         32883
typedef GLvoid (csAPIENTRY* csGLTEXIMAGE3DEXT) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, GLvoid* pixels);

// GL_EXT_texture_compression_s3tc
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT                                    33776
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT                                   33777
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT                                   33778
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT                                   33779

// GL_EXT_texture_env_add

// GL_EXT_texture_env_combine
#define GL_COMBINE_EXT                                                     34160
#define GL_COMBINE_RGB_EXT                                                 34161
#define GL_COMBINE_ALPHA_EXT                                               34162
#define GL_SOURCE0_RGB_EXT                                                 34176
#define GL_SOURCE1_RGB_EXT                                                 34177
#define GL_SOURCE2_RGB_EXT                                                 34178
#define GL_SOURCE0_ALPHA_EXT                                               34184
#define GL_SOURCE1_ALPHA_EXT                                               34185
#define GL_SOURCE2_ALPHA_EXT                                               34186
#define GL_OPERAND0_RGB_EXT                                                34192
#define GL_OPERAND1_RGB_EXT                                                34193
#define GL_OPERAND2_RGB_EXT                                                34194
#define GL_OPERAND0_ALPHA_EXT                                              34200
#define GL_OPERAND1_ALPHA_EXT                                              34201
#define GL_OPERAND2_ALPHA_EXT                                              34202
#define GL_RGB_SCALE_EXT                                                   34163
#define GL_ADD_SIGNED_EXT                                                  34164
#define GL_INTERPOLATE_EXT                                                 34165
#define GL_CONSTANT_EXT                                                    34166
#define GL_PRIMARY_COLOR_EXT                                               34167
#define GL_PREVIOUS_EXT                                                    34168

// GL_EXT_texture_env_dot3
#define GL_DOT3_RGB_EXT                                                    34624
#define GL_DOT3_RGBA_EXT                                                   34625

// GL_EXT_texture_filter_anisotropic
#define GL_TEXTURE_MAX_ANISOTROPY_EXT                                      34046
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT                                  34047

// GL_EXT_texture_lod_bias
#define GL_TEXTURE_FILTER_CONTROL_EXT                                      34048
#define GL_TEXTURE_LOD_BIAS_EXT                                            34049
#define GL_MAX_TEXTURE_LOD_BIAS_EXT                                        34045

// GL_EXT_texture_object
#define GL_TEXTURE_PRIORITY_EXT                                            32870
#define GL_TEXTURE_RESIDENT_EXT                                            32871
#define GL_TEXTURE_1D_BINDING_EXT                                          32872
#define GL_TEXTURE_2D_BINDING_EXT                                          32873
#define GL_TEXTURE_3D_BINDING_EXT                                          32874
typedef GLvoid (csAPIENTRY* csGLGENTEXTURESEXT) (GLsizei n, GLuint* textures);
typedef GLvoid (csAPIENTRY* csGLDELETETEXTURESEXT) (GLsizei n, GLuint* textures);
typedef GLvoid (csAPIENTRY* csGLBINDTEXTUREEXT) (GLenum target, GLuint texture);
typedef GLvoid (csAPIENTRY* csGLPRIORITIZETEXTURESEXT) (GLsizei n, GLuint* textures, GLclampf* priorities);
typedef GLboolean (csAPIENTRY* csGLARETEXTURESRESIDENTEXT) (GLsizei n, GLuint* textures, GLboolean* residences);
typedef GLboolean (csAPIENTRY* csGLISTEXTUREEXT) (GLuint texture);

// GL_EXT_vertex_array
#define GL_VERTEX_ARRAY_EXT                                                32884
#define GL_NORMAL_ARRAY_EXT                                                32885
#define GL_COLOR_ARRAY_EXT                                                 32886
#define GL_INDEX_ARRAY_EXT                                                 32887
#define GL_TEXTURE_COORD_ARRAY_EXT                                         32888
#define GL_EDGE_FLAG_ARRAY_EXT                                             32889
#define GL_DOUBLE_EXT                                                      5130
#define GL_VERTEX_ARRAY_SIZE_EXT                                           32890
#define GL_VERTEX_ARRAY_TYPE_EXT                                           32891
#define GL_VERTEX_ARRAY_STRIDE_EXT                                         32892
#define GL_VERTEX_ARRAY_COUNT_EXT                                          32893
#define GL_NORMAL_ARRAY_TYPE_EXT                                           32894
#define GL_NORMAL_ARRAY_STRIDE_EXT                                         32895
#define GL_NORMAL_ARRAY_COUNT_EXT                                          32896
#define GL_COLOR_ARRAY_SIZE_EXT                                            32897
#define GL_COLOR_ARRAY_TYPE_EXT                                            32898
#define GL_COLOR_ARRAY_STRIDE_EXT                                          32899
#define GL_COLOR_ARRAY_COUNT_EXT                                           32900
#define GL_INDEX_ARRAY_TYPE_EXT                                            32901
#define GL_INDEX_ARRAY_STRIDE_EXT                                          32902
#define GL_INDEX_ARRAY_COUNT_EXT                                           32903
#define GL_TEXTURE_COORD_ARRAY_SIZE_EXT                                    32904
#define GL_TEXTURE_COORD_ARRAY_TYPE_EXT                                    32905
#define GL_TEXTURE_COORD_ARRAY_STRIDE_EXT                                  32906
#define GL_TEXTURE_COORD_ARRAY_COUNT_EXT                                   32907
#define GL_EDGE_FLAG_ARRAY_STRIDE_EXT                                      32908
#define GL_EDGE_FLAG_ARRAY_COUNT_EXT                                       32909
#define GL_VERTEX_ARRAY_POINTER_EXT                                        32910
#define GL_NORMAL_ARRAY_POINTER_EXT                                        32911
#define GL_COLOR_ARRAY_POINTER_EXT                                         32912
#define GL_INDEX_ARRAY_POINTER_EXT                                         32913
#define GL_TEXTURE_COORD_ARRAY_POINTER_EXT                                 32914
#define GL_EDGE_FLAG_ARRAY_POINTER_EXT                                     32915
typedef GLvoid (csAPIENTRY* csGLARRAYELEMENTEXT) (GLint i);
typedef GLvoid (csAPIENTRY* csGLDRAWARRAYSEXT) (GLenum mode, GLint first, GLsizei count);
typedef GLvoid (csAPIENTRY* csGLVERTEXPOINTEREXT) (GLint size, GLenum type, GLsizei stride, GLsizei count, GLvoid* pointer);
typedef GLvoid (csAPIENTRY* csGLNORMALPOINTEREXT) (GLenum type, GLsizei stride, GLsizei count, GLvoid* pointer);
typedef GLvoid (csAPIENTRY* csGLCOLORPOINTEREXT) (GLint size, GLenum type, GLsizei stride, GLsizei count, GLvoid* pointer);
typedef GLvoid (csAPIENTRY* csGLINDEXPOINTEREXT) (GLenum type, GLsizei stride, GLsizei count, GLvoid* pointer);
typedef GLvoid (csAPIENTRY* csGLTEXCOORDPOINTEREXT) (GLint size, GLenum type, GLsizei stride, GLsizei count, GLvoid* pointer);
typedef GLvoid (csAPIENTRY* csGLEDGEFLAGPOINTEREXT) (GLsizei stride, GLsizei count, GLboolean* pointer);
typedef GLvoid (csAPIENTRY* csGLGETPOINTERVEXT) (GLenum pname, GLvoid* params);

// GL_EXT_vertex_shader
#define GL_VERTEX_SHADER_EXT                                               34688
#define GL_VARIANT_VALUE_EXT                                               34788
#define GL_VARIANT_DATATYPE_EXT                                            34789
#define GL_VARIANT_ARRAY_STRIDE_EXT                                        34790
#define GL_VARIANT_ARRAY_TYPE_EXT                                          34791
#define GL_VARIANT_ARRAY_EXT                                               34792
#define GL_VARIANT_ARRAY_POINTER_EXT                                       34793
#define GL_INVARIANT_VALUE_EXT                                             34794
#define GL_INVARIANT_DATATYPE_EXT                                          34795
#define GL_LOCAL_CONSTANT_VALUE_EXT                                        34796
#define GL_LOCAL_CONSTANT_DATATYPE_EXT                                     34797
#define GL_OP_INDEX_EXT                                                    34690
#define GL_OP_NEGATE_EXT                                                   34691
#define GL_OP_DOT3_EXT                                                     34692
#define GL_OP_DOT4_EXT                                                     34693
#define GL_OP_MUL_EXT                                                      34694
#define GL_OP_ADD_EXT                                                      34695
#define GL_OP_MADD_EXT                                                     34696
#define GL_OP_FRAC_EXT                                                     34697
#define GL_OP_MAX_EXT                                                      34698
#define GL_OP_MIN_EXT                                                      34699
#define GL_OP_SET_GE_EXT                                                   34700
#define GL_OP_SET_LT_EXT                                                   34701
#define GL_OP_CLAMP_EXT                                                    34702
#define GL_OP_FLOOR_EXT                                                    34703
#define GL_OP_ROUND_EXT                                                    34704
#define GL_OP_EXP_BASE_2_EXT                                               34705
#define GL_OP_LOG_BASE_2_EXT                                               34706
#define GL_OP_POWER_EXT                                                    34707
#define GL_OP_RECIP_EXT                                                    34708
#define GL_OP_RECIP_SQRT_EXT                                               34709
#define GL_OP_SUB_EXT                                                      34710
#define GL_OP_CROSS_PRODUCT_EXT                                            34711
#define GL_OP_MULTIPLY_MATRIX_EXT                                          34712
#define GL_OP_MOV_EXT                                                      34713
#define GL_OUTPUT_VERTEX_EXT                                               34714
#define GL_OUTPUT_COLOR0_EXT                                               34715
#define GL_OUTPUT_COLOR1_EXT                                               34716
#define GL_OUTPUT_TEXTURE_COORD0_EXT                                       34717
#define GL_OUTPUT_TEXTURE_COORD1_EXT                                       34718
#define GL_OUTPUT_TEXTURE_COORD2_EXT                                       34719
#define GL_OUTPUT_TEXTURE_COORD3_EXT                                       34720
#define GL_OUTPUT_TEXTURE_COORD4_EXT                                       34721
#define GL_OUTPUT_TEXTURE_COORD5_EXT                                       34722
#define GL_OUTPUT_TEXTURE_COORD6_EXT                                       34723
#define GL_OUTPUT_TEXTURE_COORD7_EXT                                       34724
#define GL_OUTPUT_TEXTURE_COORD8_EXT                                       34725
#define GL_OUTPUT_TEXTURE_COORD9_EXT                                       34726
#define GL_OUTPUT_TEXTURE_COORD10_EXT                                      34727
#define GL_OUTPUT_TEXTURE_COORD11_EXT                                      34728
#define GL_OUTPUT_TEXTURE_COORD12_EXT                                      34729
#define GL_OUTPUT_TEXTURE_COORD13_EXT                                      34730
#define GL_OUTPUT_TEXTURE_COORD14_EXT                                      34731
#define GL_OUTPUT_TEXTURE_COORD15_EXT                                      34732
#define GL_OUTPUT_TEXTURE_COORD16_EXT                                      34733
#define GL_OUTPUT_TEXTURE_COORD17_EXT                                      34734
#define GL_OUTPUT_TEXTURE_COORD18_EXT                                      34735
#define GL_OUTPUT_TEXTURE_COORD19_EXT                                      34736
#define GL_OUTPUT_TEXTURE_COORD20_EXT                                      34737
#define GL_OUTPUT_TEXTURE_COORD21_EXT                                      34738
#define GL_OUTPUT_TEXTURE_COORD22_EXT                                      34739
#define GL_OUTPUT_TEXTURE_COORD23_EXT                                      34740
#define GL_OUTPUT_TEXTURE_COORD24_EXT                                      34741
#define GL_OUTPUT_TEXTURE_COORD25_EXT                                      34742
#define GL_OUTPUT_TEXTURE_COORD26_EXT                                      34743
#define GL_OUTPUT_TEXTURE_COORD27_EXT                                      34744
#define GL_OUTPUT_TEXTURE_COORD28_EXT                                      34745
#define GL_OUTPUT_TEXTURE_COORD29_EXT                                      34746
#define GL_OUTPUT_TEXTURE_COORD30_EXT                                      34747
#define GL_OUTPUT_TEXTURE_COORD31_EXT                                      34748
#define GL_OUTPUT_FOG_EXT                                                  34749
#define GL_SCALAR_EXT                                                      34750
#define GL_VECTOR_EXT                                                      34751
#define GL_MATRIX_EXT                                                      34752
#define GL_VARIANT_EXT                                                     34753
#define GL_INVARIANT_EXT                                                   34754
#define GL_LOCAL_CONSTANT_EXT                                              34755
#define GL_LOCAL_EXT                                                       34756
#define GL_MAX_VERTEX_SHADER_INSTRUCTIONS_EXT                              34757
#define GL_MAX_VERTEX_SHADER_VARIANTS_EXT                                  34758
#define GL_MAX_VERTEX_SHADER_INVARIANTS_EXT                                34759
#define GL_MAX_VERTEX_SHADER_LOCAL_CONSTANTS_EXT                           34760
#define GL_MAX_VERTEX_SHADER_LOCALS_EXT                                    34761
#define GL_MAX_OPTIMIZED_VERTEX_SHADER_INSTRUCTIONS_EXT                    34762
#define GL_MAX_OPTIMIZED_VERTEX_SHADER_VARIANTS_EXT                        34763
#define GL_MAX_OPTIMIZED_VERTEX_SHADER_LOCAL_CONSTANTS_EXT                 34764
#define GL_MAX_OPTIMIZED_VERTEX_SHADER_INVARIANTS_EXT                      34765
#define GL_MAX_OPTIMIZED_VERTEX_SHADER_LOCALS_EXT                          34766
#define GL_VERTEX_SHADER_INSTRUCTIONS_EXT                                  34767
#define GL_VERTEX_SHADER_VARIANTS_EXT                                      34768
#define GL_VERTEX_SHADER_INVARIANTS_EXT                                    34769
#define GL_VERTEX_SHADER_LOCAL_CONSTANTS_EXT                               34770
#define GL_VERTEX_SHADER_LOCALS_EXT                                        34771
#define GL_VERTEX_SHADER_BINDING_EXT                                       34689
#define GL_VERTEX_SHADER_OPTIMIZED_EXT                                     34772
#define GL_X_EXT                                                           34773
#define GL_Y_EXT                                                           34774
#define GL_Z_EXT                                                           34775
#define GL_W_EXT                                                           34776
#define GL_NEGATIVE_X_EXT                                                  34777
#define GL_NEGATIVE_Y_EXT                                                  34778
#define GL_NEGATIVE_Z_EXT                                                  34779
#define GL_NEGATIVE_W_EXT                                                  34780
#define GL_ZERO_EXT                                                        34781
#define GL_ONE_EXT                                                         34782
#define GL_NEGATIVE_ONE_EXT                                                34783
#define GL_NORMALIZED_RANGE_EXT                                            34784
#define GL_FULL_RANGE_EXT                                                  34785
#define GL_CURRENT_VERTEX_EXT                                              34786
#define GL_MVP_MATRIX_EXT                                                  34787
typedef GLvoid (csAPIENTRY* csGLBEGINVERTEXSHADEREXT) ();
typedef GLvoid (csAPIENTRY* csGLENDVERTEXSHADEREXT) ();
typedef GLvoid (csAPIENTRY* csGLBINDVERTEXSHADEREXT) (GLuint id);
typedef GLuint (csAPIENTRY* csGLGENVERTEXSHADERSEXT) (GLuint range);
typedef GLvoid (csAPIENTRY* csGLDELETEVERTEXSHADEREXT) (GLuint id);
typedef GLvoid (csAPIENTRY* csGLSHADEROP1EXT) (GLenum op, GLuint res, GLuint arg1);
typedef GLvoid (csAPIENTRY* csGLSHADEROP2EXT) (GLenum op, GLuint res, GLuint arg1, GLuint arg2);
typedef GLvoid (csAPIENTRY* csGLSHADEROP3EXT) (GLenum op, GLuint res, GLuint arg1, GLuint arg2, GLuint arg3);
typedef GLvoid (csAPIENTRY* csGLSWIZZLEEXT) (GLuint res, GLuint in, GLenum outX, GLenum outY, GLenum outZ, GLenum outW);
typedef GLvoid (csAPIENTRY* csGLWRITEMASKEXT) (GLuint res, GLuint in, GLenum outX, GLenum outY, GLenum outZ, GLenum outW);
typedef GLvoid (csAPIENTRY* csGLINSERTCOMPONENTEXT) (GLuint res, GLuint src, GLuint num);
typedef GLvoid (csAPIENTRY* csGLEXTRACTCOMPONENTEXT) (GLuint res, GLuint src, GLuint num);
typedef GLuint (csAPIENTRY* csGLGENSYMBOLSEXT) (GLenum datatype, GLenum storagetype, GLenum range, GLuint components);
typedef GLvoid (csAPIENTRY* csGLSETINVARIANTEXT) (GLuint id, GLenum type, GLvoid* addr);
typedef GLvoid (csAPIENTRY* csGLSETLOCALCONSTANTEXT) (GLuint id, GLenum type, GLvoid* addr);
typedef GLvoid (csAPIENTRY* csGLVARIANTBVEXT) (GLuint id, GLbyte* addr);
typedef GLvoid (csAPIENTRY* csGLVARIANTSVEXT) (GLuint id, GLshort* addr);
typedef GLvoid (csAPIENTRY* csGLVARIANTIVEXT) (GLuint id, GLint* addr);
typedef GLvoid (csAPIENTRY* csGLVARIANTFVEXT) (GLuint id, GLfloat* addr);
typedef GLvoid (csAPIENTRY* csGLVARIANTDVEXT) (GLuint id, GLdouble* addr);
typedef GLvoid (csAPIENTRY* csGLVARIANTUBVEXT) (GLuint id, GLubyte* addr);
typedef GLvoid (csAPIENTRY* csGLVARIANTUSVEXT) (GLuint id, GLushort* addr);
typedef GLvoid (csAPIENTRY* csGLVARIANTUIVEXT) (GLuint id, GLuint* addr);
typedef GLvoid (csAPIENTRY* csGLVARIANTPOINTEREXT) (GLuint id, GLenum type, GLuint stride, GLvoid* addr);
typedef GLvoid (csAPIENTRY* csGLENABLEVARIANTCLIENTSTATEEXT) (GLuint id);
typedef GLvoid (csAPIENTRY* csGLDISABLEVARIANTCLIENTSTATEEXT) (GLuint id);
typedef GLuint (csAPIENTRY* csGLBINDLIGHTPARAMETEREXT) (GLenum light, GLenum value);
typedef GLuint (csAPIENTRY* csGLBINDMATERIALPARAMETEREXT) (GLenum face, GLenum value);
typedef GLuint (csAPIENTRY* csGLBINDTEXGENPARAMETEREXT) (GLenum unit, GLenum coord, GLenum value);
typedef GLuint (csAPIENTRY* csGLBINDTEXTUREUNITPARAMETEREXT) (GLenum unit, GLenum value);
typedef GLuint (csAPIENTRY* csGLBINDPARAMETEREXT) (GLenum value);
typedef GLboolean (csAPIENTRY* csGLISVARIANTENABLEDEXT) (GLuint id, GLenum cap);
typedef GLvoid (csAPIENTRY* csGLGETVARIANTBOOLEANVEXT) (GLuint id, GLenum value, GLboolean* data);
typedef GLvoid (csAPIENTRY* csGLGETVARIANTINTEGERVEXT) (GLuint id, GLenum value, GLint* data);
typedef GLvoid (csAPIENTRY* csGLGETVARIANTFLOATVEXT) (GLuint id, GLenum value, GLfloat* data);
typedef GLvoid (csAPIENTRY* csGLGETVARIANTPOINTERVEXT) (GLuint id, GLenum value, GLvoid* data);
typedef GLvoid (csAPIENTRY* csGLGETINVARIANTBOOLEANVEXT) (GLuint id, GLenum value, GLboolean* data);
typedef GLvoid (csAPIENTRY* csGLGETINVARIANTINTEGERVEXT) (GLuint id, GLenum value, GLint* data);
typedef GLvoid (csAPIENTRY* csGLGETINVARIANTFLOATVEXT) (GLuint id, GLenum value, GLfloat* data);
typedef GLvoid (csAPIENTRY* csGLGETLOCALCONSTANTBOOLEANVEXT) (GLuint id, GLenum value, GLboolean* data);
typedef GLvoid (csAPIENTRY* csGLGETLOCALCONSTANTINTEGERVEXT) (GLuint id, GLenum value, GLint* data);
typedef GLvoid (csAPIENTRY* csGLGETLOCALCONSTANTFLOATVEXT) (GLuint id, GLenum value, GLfloat* data);

// GL_EXT_vertex_weighting
#define GL_VERTEX_WEIGHTING_EXT                                            34057
#define GL_MODELVIEW0_EXT                                                  5888
#define GL_MODELVIEW1_EXT                                                  34058
#define GL_MODELVIEW0_MATRIX_EXT                                           2982
#define GL_MODELVIEW1_MATRIX_EXT                                           34054
#define GL_CURRENT_VERTEX_WEIGHT_EXT                                       34059
#define GL_VERTEX_WEIGHT_ARRAY_EXT                                         34060
#define GL_VERTEX_WEIGHT_ARRAY_SIZE_EXT                                    34061
#define GL_VERTEX_WEIGHT_ARRAY_TYPE_EXT                                    34062
#define GL_VERTEX_WEIGHT_ARRAY_STRIDE_EXT                                  34063
#define GL_MODELVIEW0_STACK_DEPTH_EXT                                      2979
#define GL_MODELVIEW1_STACK_DEPTH_EXT                                      34050
#define GL_VERTEX_WEIGHT_ARRAY_POINTER_EXT                                 34064
typedef GLvoid (csAPIENTRY* csGLVERTEXWEIGHTFEXT) (GLfloat weight);
typedef GLvoid (csAPIENTRY* csGLVERTEXWEIGHTFVEXT) (GLfloat* weight);
typedef GLvoid (csAPIENTRY* csGLVERTEXWEIGHTPOINTEREXT) (GLint size, GLenum type, GLsizei stride, GLvoid* pointer);

// GL_HP_occlusion_test
#define GL_OCCLUSION_TEST_HP                                               33125
#define GL_OCCLUSION_TEST_RESULT_HP                                        33126

// GL_NV_blend_square

// GL_NV_copy_depth_to_color
#define GL_DEPTH_STENCIL_TO_RGBA_NV                                        34926
#define GL_DEPTH_STENCIL_TO_BGRA_NV                                        34927

// GL_NV_depth_clamp
#define GL_DEPTH_CLAMP_NV                                                  34383

// GL_NV_evaluators
#define GL_EVAL_2D_NV                                                      34496
#define GL_EVAL_TRIANGULAR_2D_NV                                           34497
#define GL_MAP_TESSELLATION_NV                                             34498
#define GL_MAP_ATTRIB_U_ORDER_NV                                           34499
#define GL_MAP_ATTRIB_V_ORDER_NV                                           34500
#define GL_EVAL_FRACTIONAL_TESSELLATION_NV                                 34501
#define GL_EVAL_VERTEX_ATTRIB0_NV                                          34502
#define GL_EVAL_VERTEX_ATTRIB1_NV                                          34503
#define GL_EVAL_VERTEX_ATTRIB2_NV                                          34504
#define GL_EVAL_VERTEX_ATTRIB3_NV                                          34505
#define GL_EVAL_VERTEX_ATTRIB4_NV                                          34506
#define GL_EVAL_VERTEX_ATTRIB5_NV                                          34507
#define GL_EVAL_VERTEX_ATTRIB6_NV                                          34508
#define GL_EVAL_VERTEX_ATTRIB7_NV                                          34509
#define GL_EVAL_VERTEX_ATTRIB8_NV                                          34510
#define GL_EVAL_VERTEX_ATTRIB9_NV                                          34511
#define GL_EVAL_VERTEX_ATTRIB10_NV                                         34512
#define GL_EVAL_VERTEX_ATTRIB11_NV                                         34513
#define GL_EVAL_VERTEX_ATTRIB12_NV                                         34514
#define GL_EVAL_VERTEX_ATTRIB13_NV                                         34515
#define GL_EVAL_VERTEX_ATTRIB14_NV                                         34516
#define GL_EVAL_VERTEX_ATTRIB15_NV                                         34517
#define GL_MAX_MAP_TESSELLATION_NV                                         34518
#define GL_MAX_RATIONAL_EVAL_ORDER_NV                                      34519
typedef GLvoid (csAPIENTRY* csGLMAPCONTROLPOINTSNV) (GLenum target, GLuint index, GLenum type, GLsizei ustride, GLsizei vstride, GLint uorder, GLint vorder, GLboolean packed, GLvoid* points);
typedef GLvoid (csAPIENTRY* csGLMAPPARAMETERIVNV) (GLenum target, GLenum pname, GLint* params);
typedef GLvoid (csAPIENTRY* csGLMAPPARAMETERFVNV) (GLenum target, GLenum pname, GLfloat* params);
typedef GLvoid (csAPIENTRY* csGLGETMAPCONTROLPOINTSNV) (GLenum target, GLuint index, GLenum type, GLsizei ustride, GLsizei vstride, GLboolean packed, GLvoid* points);
typedef GLvoid (csAPIENTRY* csGLGETMAPPARAMETERIVNV) (GLenum target, GLenum pname, GLint* params);
typedef GLvoid (csAPIENTRY* csGLGETMAPPARAMETERFVNV) (GLenum target, GLenum pname, GLfloat* params);
typedef GLvoid (csAPIENTRY* csGLGETMAPATTRIBPARAMETERIVNV) (GLenum target, GLuint index, GLenum pname, GLint* params);
typedef GLvoid (csAPIENTRY* csGLGETMAPATTRIBPARAMETERFVNV) (GLenum target, GLuint index, GLenum pname, GLfloat* params);
typedef GLvoid (csAPIENTRY* csGLEVALMAPSNV) (GLenum target, GLenum mode);

// GL_NV_fence
#define GL_ALL_COMPLETED_NV                                                34034
#define GL_FENCE_STATUS_NV                                                 34035
#define GL_FENCE_CONDITION_NV                                              34036
typedef GLvoid (csAPIENTRY* csGLGENFENCESNV) (GLsizei n, GLuint* fences);
typedef GLvoid (csAPIENTRY* csGLDELETEFENCESNV) (GLsizei n, GLuint* fences);
typedef GLvoid (csAPIENTRY* csGLSETFENCENV) (GLuint fence, GLenum condition);
typedef GLboolean (csAPIENTRY* csGLTESTFENCENV) (GLuint fence);
typedef GLvoid (csAPIENTRY* csGLFINISHFENCENV) (GLuint fence);
typedef GLboolean (csAPIENTRY* csGLISFENCENV) (GLuint fence);
typedef GLvoid (csAPIENTRY* csGLGETFENCEIVNV) (GLuint fence, GLenum pname, GLint* params);

// GL_NV_fog_distance
#define GL_FOG_DISTANCE_MODE_NV                                            34138
#define GL_EYE_RADIAL_NV                                                   34139
#define GL_EYE_PLANE_ABSOLUTE_NV                                           34140

// GL_NV_light_max_exponent
#define GL_MAX_SHININESS_NV                                                34052
#define GL_MAX_SPOT_EXPONENT_NV                                            34053

// GL_NV_multisample_filter_hint
#define GL_MULTISAMPLE_FILTER_HINT_NV                                      34100

// GL_NV_occlusion_query
#define GL_OCCLUSION_TEST_HP                                               33125
#define GL_OCCLUSION_TEST_RESULT_HP                                        33126
#define GL_PIXEL_COUNTER_BITS_NV                                           34916
#define GL_CURRENT_OCCLUSION_QUERY_ID_NV                                   34917
#define GL_PIXEL_COUNT_NV                                                  34918
#define GL_PIXEL_COUNT_AVAILABLE_NV                                        34919
typedef GLvoid (csAPIENTRY* csGLGENOCCLUSIONQUERIESNV) (GLsizei n, GLuint* ids);
typedef GLvoid (csAPIENTRY* csGLDELETEOCCLUSIONQUERIESNV) (GLsizei n, GLuint* ids);
typedef GLboolean (csAPIENTRY* csGLISOCCLUSIONQUERYNV) (GLuint id);
typedef GLvoid (csAPIENTRY* csGLBEGINOCCLUSIONQUERYNV) (GLuint id);
typedef GLvoid (csAPIENTRY* csGLENDOCCLUSIONQUERYNV) ();
typedef GLvoid (csAPIENTRY* csGLGETOCCLUSIONQUERYIVNV) (GLuint id, GLenum pname, GLint* params);
typedef GLvoid (csAPIENTRY* csGLGETOCCLUSIONQUERYUIVNV) (GLuint id, GLenum pname, GLuint* params);

// GL_NV_packed_depth_stencil
#define GL_DEPTH_STENCIL_NV                                                34041
#define GL_UNSIGNED_INT_24_8_NV                                            34042

// GL_NV_point_sprite
#define GL_POINT_SPRITE_NV                                                 34913
#define GL_COORD_REPLACE_NV                                                34914
#define GL_POINT_SPRITE_R_MODE_NV                                          34915
typedef GLvoid (csAPIENTRY* csGLPOINTPARAMETERINV) (GLenum pname, GLint param);
typedef GLvoid (csAPIENTRY* csGLPOINTPARAMETERIVNV) (GLenum pname, GLint* params);

// GL_NV_register_combiners
#define GL_REGISTER_COMBINERS_NV                                           34082
#define GL_COMBINER0_NV                                                    34128
#define GL_COMBINER1_NV                                                    34129
#define GL_COMBINER2_NV                                                    34130
#define GL_COMBINER3_NV                                                    34131
#define GL_COMBINER4_NV                                                    34132
#define GL_COMBINER5_NV                                                    34133
#define GL_COMBINER6_NV                                                    34134
#define GL_COMBINER7_NV                                                    34135
#define GL_VARIABLE_A_NV                                                   34083
#define GL_VARIABLE_B_NV                                                   34084
#define GL_VARIABLE_C_NV                                                   34085
#define GL_VARIABLE_D_NV                                                   34086
#define GL_VARIABLE_E_NV                                                   34087
#define GL_VARIABLE_F_NV                                                   34088
#define GL_VARIABLE_G_NV                                                   34089
#define GL_CONSTANT_COLOR0_NV                                              34090
#define GL_CONSTANT_COLOR1_NV                                              34091
#define GL_PRIMARY_COLOR_NV                                                34092
#define GL_SECONDARY_COLOR_NV                                              34093
#define GL_SPARE0_NV                                                       34094
#define GL_SPARE1_NV                                                       34095
#define GL_UNSIGNED_IDENTITY_NV                                            34102
#define GL_UNSIGNED_INVERT_NV                                              34103
#define GL_EXPAND_NORMAL_NV                                                34104
#define GL_EXPAND_NEGATE_NV                                                34105
#define GL_HALF_BIAS_NORMAL_NV                                             34106
#define GL_HALF_BIAS_NEGATE_NV                                             34107
#define GL_SIGNED_IDENTITY_NV                                              34108
#define GL_SIGNED_NEGATE_NV                                                34109
#define GL_E_TIMES_F_NV                                                    34097
#define GL_SPARE0_PLUS_SECONDARY_COLOR_NV                                  34098
#define GL_SCALE_BY_TWO_NV                                                 34110
#define GL_SCALE_BY_FOUR_NV                                                34111
#define GL_SCALE_BY_ONE_HALF_NV                                            34112
#define GL_BIAS_BY_NEGATIVE_ONE_HALF_NV                                    34113
#define GL_DISCARD_NV                                                      34096
#define GL_COMBINER_INPUT_NV                                               34114
#define GL_COMBINER_MAPPING_NV                                             34115
#define GL_COMBINER_COMPONENT_USAGE_NV                                     34116
#define GL_COMBINER_AB_DOT_PRODUCT_NV                                      34117
#define GL_COMBINER_CD_DOT_PRODUCT_NV                                      34118
#define GL_COMBINER_MUX_SUM_NV                                             34119
#define GL_COMBINER_SCALE_NV                                               34120
#define GL_COMBINER_BIAS_NV                                                34121
#define GL_COMBINER_AB_OUTPUT_NV                                           34122
#define GL_COMBINER_CD_OUTPUT_NV                                           34123
#define GL_COMBINER_SUM_OUTPUT_NV                                          34124
#define GL_NUM_GENERAL_COMBINERS_NV                                        34126
#define GL_COLOR_SUM_CLAMP_NV                                              34127
#define GL_MAX_GENERAL_COMBINERS_NV                                        34125
typedef GLvoid (csAPIENTRY* csGLCOMBINERPARAMETERFVNV) (GLenum pname, GLfloat* params);
typedef GLvoid (csAPIENTRY* csGLCOMBINERPARAMETERIVNV) (GLenum pname, GLint* params);
typedef GLvoid (csAPIENTRY* csGLCOMBINERPARAMETERFNV) (GLenum pname, GLfloat param);
typedef GLvoid (csAPIENTRY* csGLCOMBINERPARAMETERINV) (GLenum pname, GLint param);
typedef GLvoid (csAPIENTRY* csGLCOMBINERINPUTNV) (GLenum stage, GLenum portion, GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage);
typedef GLvoid (csAPIENTRY* csGLCOMBINEROUTPUTNV) (GLenum stage, GLenum portion, GLenum abOutput, GLenum cdOutput, GLenum sumOutput, GLenum scale, GLenum bias, GLboolean abDotProduct, GLboolean cdDotProduct, GLboolean muxSum);
typedef GLvoid (csAPIENTRY* csGLFINALCOMBINERINPUTNV) (GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage);
typedef GLvoid (csAPIENTRY* csGLGETCOMBINERINPUTPARAMETERFVNV) (GLenum stage, GLenum portion, GLenum variable, GLenum pname, GLfloat* params);
typedef GLvoid (csAPIENTRY* csGLGETCOMBINERINPUTPARAMETERIVNV) (GLenum stage, GLenum portion, GLenum variable, GLenum pname, GLint* params);
typedef GLvoid (csAPIENTRY* csGLGETCOMBINEROUTPUTPARAMETERFVNV) (GLenum stage, GLenum portion, GLenum pname, GLfloat* params);
typedef GLvoid (csAPIENTRY* csGLGETCOMBINEROUTPUTPARAMETERIVNV) (GLenum stage, GLenum portion, GLenum pname, GLint* params);
typedef GLvoid (csAPIENTRY* csGLGETFINALCOMBINERINPUTPARAMETERFVNV) (GLenum variable, GLenum pname, GLfloat* params);
typedef GLvoid (csAPIENTRY* csGLGETFINALCOMBINERINPUTPARAMETERIVNV) (GLenum variable, GLenum pname, GLint* params);

// GL_NV_register_combiners2
#define GL_PER_STAGE_CONSTANTS_NV                                          34101
typedef GLvoid (csAPIENTRY* csGLCOMBINERSTAGEPARAMETERFVNV) (GLenum stage, GLenum pname, GLfloat* params);
typedef GLvoid (csAPIENTRY* csGLGETCOMBINERSTAGEPARAMETERFVNV) (GLenum stage, GLenum pname, GLfloat* params);

// GL_NV_texgen_emboss
#define GL_EMBOSS_MAP_NV                                                   34143
#define GL_EMBOSS_LIGHT_NV                                                 34141
#define GL_EMBOSS_CONSTANT_NV                                              34142

// GL_NV_texgen_reflection
#define GL_NORMAL_MAP_NV                                                   34065
#define GL_REFLECTION_MAP_NV                                               34066

// GL_NV_texture_compression_vtc
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT                                    33776
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT                                   33777
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT                                   33778
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT                                   33779

// GL_NV_texture_env_combine4
#define GL_COMBINE4_NV                                                     34051
#define GL_SOURCE3_RGB_NV                                                  34179
#define GL_SOURCE3_ALPHA_NV                                                34187
#define GL_OPERAND3_RGB_NV                                                 34195
#define GL_OPERAND3_ALPHA_NV                                               34203

// GL_NV_texture_rectangle
#define GL_TEXTURE_RECTANGLE_NV                                            34037
#define GL_TEXTURE_BINDING_RECTANGLE_NV                                    34038
#define GL_PROXY_TEXTURE_RECTANGLE_NV                                      34039
#define GL_MAX_RECTANGLE_TEXTURE_SIZE_NV                                   34040

// GL_NV_texture_shader
#define GL_TEXTURE_SHADER_NV                                               34526
#define GL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV                            34521
#define GL_SHADER_OPERATION_NV                                             34527
#define GL_CULL_MODES_NV                                                   34528
#define GL_OFFSET_TEXTURE_MATRIX_NV                                        34529
#define GL_OFFSET_TEXTURE_SCALE_NV                                         34530
#define GL_OFFSET_TEXTURE_BIAS_NV                                          34531
#define GL_PREVIOUS_TEXTURE_INPUT_NV                                       34532
#define GL_CONST_EYE_NV                                                    34533
#define GL_SHADER_CONSISTENT_NV                                            34525
#define GL_PASS_THROUGH_NV                                                 34534
#define GL_CULL_FRAGMENT_NV                                                34535
#define GL_OFFSET_TEXTURE_2D_NV                                            34536
#define GL_OFFSET_TEXTURE_RECTANGLE_NV                                     34380
#define GL_OFFSET_TEXTURE_RECTANGLE_SCALE_NV                               34381
#define GL_DEPENDENT_AR_TEXTURE_2D_NV                                      34537
#define GL_DEPENDENT_GB_TEXTURE_2D_NV                                      34538
#define GL_DOT_PRODUCT_NV                                                  34540
#define GL_DOT_PRODUCT_DEPTH_REPLACE_NV                                    34541
#define GL_DOT_PRODUCT_TEXTURE_2D_NV                                       34542
#define GL_DOT_PRODUCT_TEXTURE_RECTANGLE_NV                                34382
#define GL_DOT_PRODUCT_TEXTURE_CUBE_MAP_NV                                 34544
#define GL_DOT_PRODUCT_DIFFUSE_CUBE_MAP_NV                                 34545
#define GL_DOT_PRODUCT_REFLECT_CUBE_MAP_NV                                 34546
#define GL_DOT_PRODUCT_CONST_EYE_REFLECT_CUBE_MAP_NV                       34547
#define GL_HILO_NV                                                         34548
#define GL_DSDT_NV                                                         34549
#define GL_DSDT_MAG_NV                                                     34550
#define GL_DSDT_MAG_VIB_NV                                                 34551
#define GL_UNSIGNED_INT_S8_S8_8_8_NV                                       34522
#define GL_UNSIGNED_INT_8_8_S8_S8_REV_NV                                   34523
#define GL_SIGNED_RGBA_NV                                                  34555
#define GL_SIGNED_RGBA8_NV                                                 34556
#define GL_SIGNED_RGB_NV                                                   34558
#define GL_SIGNED_RGB8_NV                                                  34559
#define GL_SIGNED_LUMINANCE_NV                                             34561
#define GL_SIGNED_LUMINANCE8_NV                                            34562
#define GL_SIGNED_LUMINANCE_ALPHA_NV                                       34563
#define GL_SIGNED_LUMINANCE8_ALPHA8_NV                                     34564
#define GL_SIGNED_ALPHA_NV                                                 34565
#define GL_SIGNED_ALPHA8_NV                                                34566
#define GL_SIGNED_INTENSITY_NV                                             34567
#define GL_SIGNED_INTENSITY8_NV                                            34568
#define GL_SIGNED_RGB_UNSIGNED_ALPHA_NV                                    34572
#define GL_SIGNED_RGB8_UNSIGNED_ALPHA8_NV                                  34573
#define GL_HILO16_NV                                                       34552
#define GL_SIGNED_HILO_NV                                                  34553
#define GL_SIGNED_HILO16_NV                                                34554
#define GL_DSDT8_NV                                                        34569
#define GL_DSDT8_MAG8_NV                                                   34570
#define GL_DSDT_MAG_INTENSITY_NV                                           34524
#define GL_DSDT8_MAG8_INTENSITY8_NV                                        34571
#define GL_HI_SCALE_NV                                                     34574
#define GL_LO_SCALE_NV                                                     34575
#define GL_DS_SCALE_NV                                                     34576
#define GL_DT_SCALE_NV                                                     34577
#define GL_MAGNITUDE_SCALE_NV                                              34578
#define GL_VIBRANCE_SCALE_NV                                               34579
#define GL_HI_BIAS_NV                                                      34580
#define GL_LO_BIAS_NV                                                      34581
#define GL_DS_BIAS_NV                                                      34582
#define GL_DT_BIAS_NV                                                      34583
#define GL_MAGNITUDE_BIAS_NV                                               34584
#define GL_VIBRANCE_BIAS_NV                                                34585
#define GL_TEXTURE_BORDER_VALUES_NV                                        34586
#define GL_TEXTURE_HI_SIZE_NV                                              34587
#define GL_TEXTURE_LO_SIZE_NV                                              34588
#define GL_TEXTURE_DS_SIZE_NV                                              34589
#define GL_TEXTURE_DT_SIZE_NV                                              34590
#define GL_TEXTURE_MAG_SIZE_NV                                             34591

// GL_NV_texture_shader2
#define GL_DOT_PRODUCT_TEXTURE_3D_NV                                       34543
#define GL_HILO_NV                                                         34548
#define GL_DSDT_NV                                                         34549
#define GL_DSDT_MAG_NV                                                     34550
#define GL_DSDT_MAG_VIB_NV                                                 34551
#define GL_UNSIGNED_INT_S8_S8_8_8_NV                                       34522
#define GL_UNSIGNED_INT_8_8_S8_S8_REV_NV                                   34523
#define GL_SIGNED_RGBA_NV                                                  34555
#define GL_SIGNED_RGBA8_NV                                                 34556
#define GL_SIGNED_RGB_NV                                                   34558
#define GL_SIGNED_RGB8_NV                                                  34559
#define GL_SIGNED_LUMINANCE_NV                                             34561
#define GL_SIGNED_LUMINANCE8_NV                                            34562
#define GL_SIGNED_LUMINANCE_ALPHA_NV                                       34563
#define GL_SIGNED_LUMINANCE8_ALPHA8_NV                                     34564
#define GL_SIGNED_ALPHA_NV                                                 34565
#define GL_SIGNED_ALPHA8_NV                                                34566
#define GL_SIGNED_INTENSITY_NV                                             34567
#define GL_SIGNED_INTENSITY8_NV                                            34568
#define GL_SIGNED_RGB_UNSIGNED_ALPHA_NV                                    34572
#define GL_SIGNED_RGB8_UNSIGNED_ALPHA8_NV                                  34573
#define GL_HILO16_NV                                                       34552
#define GL_SIGNED_HILO_NV                                                  34553
#define GL_SIGNED_HILO16_NV                                                34554
#define GL_DSDT8_NV                                                        34569
#define GL_DSDT8_MAG8_NV                                                   34570
#define GL_DSDT_MAG_INTENSITY_NV                                           34524
#define GL_DSDT8_MAG8_INTENSITY8_NV                                        34571

// GL_NV_texture_shader3
#define GL_OFFSET_PROJECTIVE_TEXTURE_2D_NV                                 34896
#define GL_OFFSET_PROJECTIVE_TEXTURE_2D_SCALE_NV                           34897
#define GL_OFFSET_PROJECTIVE_TEXTURE_RECTANGLE_NV                          34898
#define GL_OFFSET_PROJECTIVE_TEXTURE_RECTANGLE_SCALE_NV                    34899
#define GL_OFFSET_HILO_TEXTURE_2D_NV                                       34900
#define GL_OFFSET_HILO_TEXTURE_RECTANGLE_NV                                34901
#define GL_OFFSET_HILO_PROJECTIVE_TEXTURE_2D_NV                            34902
#define GL_OFFSET_HILO_PROJECTIVE_TEXTURE_RECTANGLE_NV                     34903
#define GL_DEPENDENT_HILO_TEXTURE_2D_NV                                    34904
#define GL_DEPENDENT_RGB_TEXTURE_3D_NV                                     34905
#define GL_DEPENDENT_RGB_TEXTURE_CUBE_MAP_NV                               34906
#define GL_DOT_PRODUCT_PASS_THROUGH_NV                                     34907
#define GL_DOT_PRODUCT_TEXTURE_1D_NV                                       34908
#define GL_DOT_PRODUCT_AFFINE_DEPTH_REPLACE_NV                             34909
#define GL_HILO8_NV                                                        34910
#define GL_SIGNED_HILO8_NV                                                 34911
#define GL_FORCE_BLUE_TO_ONE_NV                                            34912

// GL_NV_vertex_array_range
#define GL_VERTEX_ARRAY_RANGE_NV                                           34077
#define GL_VERTEX_ARRAY_RANGE_LENGTH_NV                                    34078
#define GL_VERTEX_ARRAY_RANGE_VALID_NV                                     34079
#define GL_MAX_VERTEX_ARRAY_RANGE_ELEMENT_NV                               34080
#define GL_VERTEX_ARRAY_RANGE_POINTER_NV                                   34081
typedef GLvoid (csAPIENTRY* csGLVERTEXARRAYRANGENV) (GLsizei length, GLvoid* pointer);
typedef GLvoid (csAPIENTRY* csGLFLUSHVERTEXARRAYRANGENV) ();
typedef GLvoid* (csAPIENTRY* csWGLALLOCATEMEMORYNV) (GLsizei size, GLfloat readFrequency, GLfloat writeFrequency, GLfloat priority);
typedef GLvoid (csAPIENTRY* csWGLFREEMEMORYNV) (GLvoid* pointer);

// GL_NV_vertex_array_range2
#define GL_VERTEX_ARRAY_RANGE_WITHOUT_FLUSH_NV                             34099

// GL_NV_vertex_program
#define GL_VERTEX_PROGRAM_NV                                               34336
#define GL_VERTEX_PROGRAM_POINT_SIZE_NV                                    34370
#define GL_VERTEX_PROGRAM_TWO_SIDE_NV                                      34371
#define GL_VERTEX_STATE_PROGRAM_NV                                         34337
#define GL_ATTRIB_ARRAY_SIZE_NV                                            34339
#define GL_ATTRIB_ARRAY_STRIDE_NV                                          34340
#define GL_ATTRIB_ARRAY_TYPE_NV                                            34341
#define GL_CURRENT_ATTRIB_NV                                               34342
#define GL_PROGRAM_PARAMETER_NV                                            34372
#define GL_ATTRIB_ARRAY_POINTER_NV                                         34373
#define GL_PROGRAM_TARGET_NV                                               34374
#define GL_PROGRAM_LENGTH_NV                                               34343
#define GL_PROGRAM_RESIDENT_NV                                             34375
#define GL_PROGRAM_STRING_NV                                               34344
#define GL_TRACK_MATRIX_NV                                                 34376
#define GL_TRACK_MATRIX_TRANSFORM_NV                                       34377
#define GL_MAX_TRACK_MATRIX_STACK_DEPTH_NV                                 34350
#define GL_MAX_TRACK_MATRICES_NV                                           34351
#define GL_CURRENT_MATRIX_STACK_DEPTH_NV                                   34368
#define GL_CURRENT_MATRIX_NV                                               34369
#define GL_VERTEX_PROGRAM_BINDING_NV                                       34378
#define GL_PROGRAM_ERROR_POSITION_NV                                       34379
#define GL_MODELVIEW_PROJECTION_NV                                         34345
#define GL_MATRIX0_NV                                                      34352
#define GL_MATRIX1_NV                                                      34353
#define GL_MATRIX2_NV                                                      34354
#define GL_MATRIX3_NV                                                      34355
#define GL_MATRIX4_NV                                                      34356
#define GL_MATRIX5_NV                                                      34357
#define GL_MATRIX6_NV                                                      34358
#define GL_MATRIX7_NV                                                      34359
#define GL_IDENTITY_NV                                                     34346
#define GL_INVERSE_NV                                                      34347
#define GL_TRANSPOSE_NV                                                    34348
#define GL_INVERSE_TRANSPOSE_NV                                            34349
#define GL_VERTEX_ATTRIB_ARRAY0_NV                                         34384
#define GL_VERTEX_ATTRIB_ARRAY1_NV                                         34385
#define GL_VERTEX_ATTRIB_ARRAY2_NV                                         34386
#define GL_VERTEX_ATTRIB_ARRAY3_NV                                         34387
#define GL_VERTEX_ATTRIB_ARRAY4_NV                                         34388
#define GL_VERTEX_ATTRIB_ARRAY5_NV                                         34389
#define GL_VERTEX_ATTRIB_ARRAY6_NV                                         34390
#define GL_VERTEX_ATTRIB_ARRAY7_NV                                         34391
#define GL_VERTEX_ATTRIB_ARRAY8_NV                                         34392
#define GL_VERTEX_ATTRIB_ARRAY9_NV                                         34393
#define GL_VERTEX_ATTRIB_ARRAY10_NV                                        34394
#define GL_VERTEX_ATTRIB_ARRAY11_NV                                        34395
#define GL_VERTEX_ATTRIB_ARRAY12_NV                                        34396
#define GL_VERTEX_ATTRIB_ARRAY13_NV                                        34397
#define GL_VERTEX_ATTRIB_ARRAY14_NV                                        34398
#define GL_VERTEX_ATTRIB_ARRAY15_NV                                        34399
#define GL_MAP1_VERTEX_ATTRIB0_4_NV                                        34400
#define GL_MAP1_VERTEX_ATTRIB1_4_NV                                        34401
#define GL_MAP1_VERTEX_ATTRIB2_4_NV                                        34402
#define GL_MAP1_VERTEX_ATTRIB3_4_NV                                        34403
#define GL_MAP1_VERTEX_ATTRIB4_4_NV                                        34404
#define GL_MAP1_VERTEX_ATTRIB5_4_NV                                        34405
#define GL_MAP1_VERTEX_ATTRIB6_4_NV                                        34406
#define GL_MAP1_VERTEX_ATTRIB7_4_NV                                        34407
#define GL_MAP1_VERTEX_ATTRIB8_4_NV                                        34408
#define GL_MAP1_VERTEX_ATTRIB9_4_NV                                        34409
#define GL_MAP1_VERTEX_ATTRIB10_4_NV                                       34410
#define GL_MAP1_VERTEX_ATTRIB11_4_NV                                       34411
#define GL_MAP1_VERTEX_ATTRIB12_4_NV                                       34412
#define GL_MAP1_VERTEX_ATTRIB13_4_NV                                       34413
#define GL_MAP1_VERTEX_ATTRIB14_4_NV                                       34414
#define GL_MAP1_VERTEX_ATTRIB15_4_NV                                       34415
#define GL_MAP2_VERTEX_ATTRIB0_4_NV                                        34416
#define GL_MAP2_VERTEX_ATTRIB1_4_NV                                        34417
#define GL_MAP2_VERTEX_ATTRIB2_4_NV                                        34418
#define GL_MAP2_VERTEX_ATTRIB3_4_NV                                        34419
#define GL_MAP2_VERTEX_ATTRIB4_4_NV                                        34420
#define GL_MAP2_VERTEX_ATTRIB5_4_NV                                        34421
#define GL_MAP2_VERTEX_ATTRIB6_4_NV                                        34422
#define GL_MAP2_VERTEX_ATTRIB7_4_NV                                        34423
#define GL_MAP2_VERTEX_ATTRIB8_4_NV                                        34424
#define GL_MAP2_VERTEX_ATTRIB9_4_NV                                        34425
#define GL_MAP2_VERTEX_ATTRIB10_4_NV                                       34426
#define GL_MAP2_VERTEX_ATTRIB11_4_NV                                       34427
#define GL_MAP2_VERTEX_ATTRIB12_4_NV                                       34428
#define GL_MAP2_VERTEX_ATTRIB13_4_NV                                       34429
#define GL_MAP2_VERTEX_ATTRIB14_4_NV                                       34430
#define GL_MAP2_VERTEX_ATTRIB15_4_NV                                       34431
typedef GLvoid (csAPIENTRY* csGLBINDPROGRAMNV) (GLenum target, GLuint id);
typedef GLvoid (csAPIENTRY* csGLDELETEPROGRAMSNV) (GLsizei n, GLuint* ids);
typedef GLvoid (csAPIENTRY* csGLEXECUTEPROGRAMNV) (GLenum target, GLuint id, GLfloat* params);
typedef GLvoid (csAPIENTRY* csGLGENPROGRAMSNV) (GLsizei n, GLuint* ids);
typedef GLboolean (csAPIENTRY* csGLAREPROGRAMSRESIDENTNV) (GLsizei n, GLuint* ids, GLboolean* residences);
typedef GLvoid (csAPIENTRY* csGLREQUESTRESIDENTPROGRAMSNV) (GLsizei n, GLuint* ids);
typedef GLvoid (csAPIENTRY* csGLGETPROGRAMPARAMETERFVNV) (GLenum target, GLuint index, GLenum pname, GLfloat* params);
typedef GLvoid (csAPIENTRY* csGLGETPROGRAMPARAMETERDVNV) (GLenum target, GLuint index, GLenum pname, GLdouble* params);
typedef GLvoid (csAPIENTRY* csGLGETPROGRAMIVNV) (GLuint id, GLenum pname, GLint* params);
typedef GLvoid (csAPIENTRY* csGLGETPROGRAMSTRINGNV) (GLuint id, GLenum pname, GLubyte* program);
typedef GLvoid (csAPIENTRY* csGLGETTRACKMATRIXIVNV) (GLenum target, GLuint address, GLenum pname, GLint* params);
typedef GLvoid (csAPIENTRY* csGLGETVERTEXATTRIBDVNV) (GLuint index, GLenum pname, GLdouble* params);
typedef GLvoid (csAPIENTRY* csGLGETVERTEXATTRIBFVNV) (GLuint index, GLenum pname, GLfloat* params);
typedef GLvoid (csAPIENTRY* csGLGETVERTEXATTRIBIVNV) (GLuint index, GLenum pname, GLint* params);
typedef GLvoid (csAPIENTRY* csGLGETVERTEXATTRIBPOINTERVNV) (GLuint index, GLenum pname, GLvoid* pointer);
typedef GLboolean (csAPIENTRY* csGLISPROGRAMNV) (GLuint id);
typedef GLvoid (csAPIENTRY* csGLLOADPROGRAMNV) (GLenum target, GLuint id, GLsizei len, GLubyte* program);
typedef GLvoid (csAPIENTRY* csGLPROGRAMPARAMETER4FNV) (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef GLvoid (csAPIENTRY* csGLPROGRAMPARAMETER4FVNV) (GLenum target, GLuint index, GLfloat* params);
typedef GLvoid (csAPIENTRY* csGLPROGRAMPARAMETERS4DVNV) (GLenum target, GLuint index, GLuint num, GLdouble* params);
typedef GLvoid (csAPIENTRY* csGLPROGRAMPARAMETERS4FVNV) (GLenum target, GLuint index, GLuint num, GLfloat* params);
typedef GLvoid (csAPIENTRY* csGLTRACKMATRIXNV) (GLenum target, GLuint address, GLenum matrix, GLenum transform);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIBPOINTERNV) (GLuint index, GLint size, GLenum type, GLsizei stride, GLvoid* pointer);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB1SNV) (GLuint index, GLshort x);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB1FNV) (GLuint index, GLfloat x);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB1DNV) (GLuint index, GLdouble x);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB2SNV) (GLuint index, GLshort x, GLshort y);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB2FNV) (GLuint index, GLfloat x, GLfloat y);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB2DNV) (GLuint index, GLdouble x, GLdouble y);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB3SNV) (GLuint index, GLshort x, GLshort y, GLshort z);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB3FNV) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB3DNV) (GLuint index, GLdouble x, GLdouble y, GLdouble z);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB4SNV) (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB4FNV) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB4DNV) (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB4UBNV) (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB1SVNV) (GLuint index, GLshort* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB1FVNV) (GLuint index, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB1DVNV) (GLuint index, GLdouble* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB2SVNV) (GLuint index, GLshort* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB2FVNV) (GLuint index, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB2DVNV) (GLuint index, GLdouble* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB3SVNV) (GLuint index, GLshort* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB3FVNV) (GLuint index, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB3DVNV) (GLuint index, GLdouble* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB4SVNV) (GLuint index, GLshort* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB4FVNV) (GLuint index, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB4DVNV) (GLuint index, GLdouble* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIB4UBVNV) (GLuint index, GLubyte* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIBS1SVNV) (GLuint index, GLsizei n, GLshort* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIBS1FVNV) (GLuint index, GLsizei n, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIBS1DVNV) (GLuint index, GLsizei n, GLdouble* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIBS2SVNV) (GLuint index, GLsizei n, GLshort* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIBS2FVNV) (GLuint index, GLsizei n, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIBS2DVNV) (GLuint index, GLsizei n, GLdouble* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIBS3SVNV) (GLuint index, GLsizei n, GLshort* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIBS3FVNV) (GLuint index, GLsizei n, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIBS3DVNV) (GLuint index, GLsizei n, GLdouble* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIBS4SVNV) (GLuint index, GLsizei n, GLshort* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIBS4FVNV) (GLuint index, GLsizei n, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIBS4DVNV) (GLuint index, GLsizei n, GLdouble* v);
typedef GLvoid (csAPIENTRY* csGLVERTEXATTRIBS4UBVNV) (GLuint index, GLsizei n, GLubyte* v);

// GL_NV_vertex_program1_1

// GL_ATI_element_array
#define GL_ELEMENT_ARRAY_ATI                                               34664
#define GL_ELEMENT_ARRAY_TYPE_ATI                                          34665
#define GL_ELEMENT_ARRAY_POINTER_ATI                                       34666
typedef GLvoid (csAPIENTRY* csGLELEMENTPOINTERATI) (GLenum type, GLvoid* pointer);
typedef GLvoid (csAPIENTRY* csGLDRAWELEMENTARRAYATI) (GLenum mode, GLsizei count);
typedef GLvoid (csAPIENTRY* csGLDRAWRANGEELEMENTARRAYATI) (GLenum mode, GLuint start, GLuint end, GLsizei count);

// GL_ATI_envmap_bumpmap
#define GL_BUMP_ROT_MATRIX_ATI                                             34677
#define GL_BUMP_ROT_MATRIX_SIZE_ATI                                        34678
#define GL_BUMP_NUM_TEX_UNITS_ATI                                          34679
#define GL_BUMP_TEX_UNITS_ATI                                              34680
#define GL_DUDV_ATI                                                        34681
#define GL_DU8DV8_ATI                                                      34682
#define GL_BUMP_ENVMAP_ATI                                                 34683
#define GL_BUMP_TARGET_ATI                                                 34684
typedef GLvoid (csAPIENTRY* csGLTEXBUMPPARAMETERIVATI) (GLenum pname, GLint* param);
typedef GLvoid (csAPIENTRY* csGLTEXBUMPPARAMETERFVATI) (GLenum pname, GLfloat* param);
typedef GLvoid (csAPIENTRY* csGLGETTEXBUMPPARAMETERIVATI) (GLenum pname, GLint* param);
typedef GLvoid (csAPIENTRY* csGLGETTEXBUMPPARAMETERFVATI) (GLenum pname, GLfloat* param);

// GL_ATI_fragment_shader
#define GL_FRAGMENT_SHADER_ATI                                             35104
#define GL_REG_0_ATI                                                       35105
#define GL_REG_1_ATI                                                       35106
#define GL_REG_2_ATI                                                       35107
#define GL_REG_3_ATI                                                       35108
#define GL_REG_4_ATI                                                       35109
#define GL_REG_5_ATI                                                       35110
#define GL_CON_0_ATI                                                       35137
#define GL_CON_1_ATI                                                       35138
#define GL_CON_2_ATI                                                       35139
#define GL_CON_3_ATI                                                       35140
#define GL_CON_4_ATI                                                       35141
#define GL_CON_5_ATI                                                       35142
#define GL_CON_6_ATI                                                       35143
#define GL_CON_7_ATI                                                       35144
#define GL_MOV_ATI                                                         35169
#define GL_ADD_ATI                                                         35171
#define GL_MUL_ATI                                                         35172
#define GL_SUB_ATI                                                         35173
#define GL_DOT3_ATI                                                        35174
#define GL_DOT4_ATI                                                        35175
#define GL_MAD_ATI                                                         35176
#define GL_LERP_ATI                                                        35177
#define GL_CND_ATI                                                         35178
#define GL_CND0_ATI                                                        35179
#define GL_DOT2_ADD_ATI                                                    35180
#define GL_SECONDARY_INTERPOLATOR_ATI                                      35181
#define GL_SWIZZLE_STR_ATI                                                 35190
#define GL_SWIZZLE_STQ_ATI                                                 35191
#define GL_SWIZZLE_STR_DR_ATI                                              35192
#define GL_SWIZZLE_STQ_DQ_ATI                                              35193
#define GL_RED_BIT_ATI                                                     1
#define GL_GREEN_BIT_ATI                                                   2
#define GL_BLUE_BIT_ATI                                                    4
#define GL_2X_BIT_ATI                                                      1
#define GL_4X_BIT_ATI                                                      2
#define GL_8X_BIT_ATI                                                      4
#define GL_HALF_BIT_ATI                                                    8
#define GL_QUARTER_BIT_ATI                                                 16
#define GL_EIGHTH_BIT_ATI                                                  32
#define GL_SATURATE_BIT_ATI                                                64
#define GL_2X_BIT_ATI                                                      1
#define GL_COMP_BIT_ATI                                                    2
#define GL_NEGATE_BIT_ATI                                                  4
#define GL_BIAS_BIT_ATI                                                    8
typedef GLuint (csAPIENTRY* csGLGENFRAGMENTSHADERSATI) (GLuint range);
typedef GLvoid (csAPIENTRY* csGLBINDFRAGMENTSHADERATI) (GLuint id);
typedef GLvoid (csAPIENTRY* csGLDELETEFRAGMENTSHADERATI) (GLuint id);
typedef GLvoid (csAPIENTRY* csGLBEGINFRAGMENTSHADERATI) ();
typedef GLvoid (csAPIENTRY* csGLENDFRAGMENTSHADERATI) ();
typedef GLvoid (csAPIENTRY* csGLPASSTEXCOORDATI) (GLuint dst, GLuint coord, GLenum swizzle);
typedef GLvoid (csAPIENTRY* csGLSAMPLEMAPATI) (GLuint dst, GLuint interp, GLenum swizzle);
typedef GLvoid (csAPIENTRY* csGLCOLORFRAGMENTOP1ATI) (GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod);
typedef GLvoid (csAPIENTRY* csGLCOLORFRAGMENTOP2ATI) (GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod);
typedef GLvoid (csAPIENTRY* csGLCOLORFRAGMENTOP3ATI) (GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod);
typedef GLvoid (csAPIENTRY* csGLALPHAFRAGMENTOP1ATI) (GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod);
typedef GLvoid (csAPIENTRY* csGLALPHAFRAGMENTOP2ATI) (GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod);
typedef GLvoid (csAPIENTRY* csGLALPHAFRAGMENTOP3ATI) (GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod);
typedef GLvoid (csAPIENTRY* csGLSETFRAGMENTSHADERCONSTANTATI) (GLuint dst, GLfloat* value);

// GL_ATI_pn_triangles
#define GL_PN_TRIANGLES_ATI                                                34800
#define GL_MAX_PN_TRIANGLES_TESSELATION_LEVEL_ATI                          34801
#define GL_PN_TRIANGLES_POINT_MODE_ATI                                     34802
#define GL_PN_TRIANGLES_NORMAL_MODE_ATI                                    34803
#define GL_PN_TRIANGLES_TESSELATION_LEVEL_ATI                              34804
#define GL_PN_TRIANGLES_POINT_MODE_LINEAR_ATI                              34805
#define GL_PN_TRIANGLES_POINT_MODE_CUBIC_ATI                               34806
#define GL_PN_TRIANGLES_NORMAL_MODE_LINEAR_ATI                             34807
#define GL_PN_TRIANGLES_NORMAL_MODE_QUADRATIC_ATI                          34808
typedef GLvoid (csAPIENTRY* csGLPNTRIANGLESIATI) (GLenum pname, GLint param);
typedef GLvoid (csAPIENTRY* csGLPNTRIANGLESFATI) (GLenum pname, GLfloat param);

// GL_ATI_texture_mirror_once
#define GL_MIRROR_CLAMP_ATI                                                34626
#define GL_MIRROR_CLAMP_TO_EDGE_ATI                                        34627

// GL_ATI_vertex_array_object
#define GL_STATIC_ATI                                                      34656
#define GL_DYNAMIC_ATI                                                     34657
#define GL_PRESERVE_ATI                                                    34658
#define GL_DISCARD_ATI                                                     34659
#define GL_OBJECT_BUFFER_SIZE_ATI                                          34660
#define GL_OBJECT_BUFFER_USAGE_ATI                                         34661
#define GL_ARRAY_OBJECT_BUFFER_ATI                                         34662
#define GL_ARRAY_OBJECT_OFFSET_ATI                                         34663
typedef GLuint (csAPIENTRY* csGLNEWOBJECTBUFFERATI) (GLsizei size, GLvoid* pointer, GLenum usage);
typedef GLboolean (csAPIENTRY* csGLISOBJECTBUFFERATI) (GLuint buffer);
typedef GLvoid (csAPIENTRY* csGLUPDATEOBJECTBUFFERATI) (GLuint buffer, GLuint offset, GLsizei size, GLvoid* pointer, GLenum preserve);
typedef GLvoid (csAPIENTRY* csGLGETOBJECTBUFFERFVATI) (GLuint buffer, GLenum pname, GLfloat* params);
typedef GLvoid (csAPIENTRY* csGLGETOBJECTBUFFERIVATI) (GLuint buffer, GLenum pname, GLint* params);
typedef GLvoid (csAPIENTRY* csGLDELETEOBJECTBUFFERATI) (GLuint buffer);
typedef GLvoid (csAPIENTRY* csGLARRAYOBJECTATI) (GLenum array, GLint size, GLenum type, GLsizei stride, GLuint buffer, GLuint offset);
typedef GLvoid (csAPIENTRY* csGLGETARRAYOBJECTFVATI) (GLenum array, GLenum pname, GLfloat* params);
typedef GLvoid (csAPIENTRY* csGLGETARRAYOBJECTIVATI) (GLenum array, GLenum pname, GLint* params);
typedef GLvoid (csAPIENTRY* csGLVARIANTARRAYOBJECTATI) (GLuint id, GLenum type, GLsizei stride, GLuint buffer, GLuint offset);
typedef GLvoid (csAPIENTRY* csGLGETVARIANTARRAYOBJECTFVATI) (GLuint id, GLenum pname, GLfloat* params);
typedef GLvoid (csAPIENTRY* csGLGETVARIANTARRAYOBJECTIVATI) (GLuint id, GLenum pname, GLint* params);

// GL_ATI_vertex_streams
#define GL_MAX_VERTEX_STREAMS_ATI                                          34667
#define GL_VERTEX_STREAM0_ATI                                              34668
#define GL_VERTEX_STREAM1_ATI                                              34669
#define GL_VERTEX_STREAM2_ATI                                              34670
#define GL_VERTEX_STREAM3_ATI                                              34671
#define GL_VERTEX_STREAM4_ATI                                              34672
#define GL_VERTEX_STREAM5_ATI                                              34673
#define GL_VERTEX_STREAM6_ATI                                              34674
#define GL_VERTEX_STREAM7_ATI                                              34675
#define GL_VERTEX_SOURCE_ATI                                               34676
typedef GLvoid (csAPIENTRY* csGLVERTEXSTREAM1S) (GLenum stream, GLshort coords);
typedef GLvoid (csAPIENTRY* csGLVERTEXSTREAM1I) (GLenum stream, GLint coords);
typedef GLvoid (csAPIENTRY* csGLVERTEXSTREAM1F) (GLenum stream, GLfloat coords);
typedef GLvoid (csAPIENTRY* csGLVERTEXSTREAM1D) (GLenum stream, GLdouble coords);
typedef GLvoid (csAPIENTRY* csGLVERTEXSTREAM1SV) (GLenum stream, GLshort coords);
typedef GLvoid (csAPIENTRY* csGLVERTEXSTREAM1IV) (GLenum stream, GLint coords);
typedef GLvoid (csAPIENTRY* csGLVERTEXSTREAM1FV) (GLenum stream, GLfloat coords);
typedef GLvoid (csAPIENTRY* csGLVERTEXSTREAM1DV) (GLenum stream, GLdouble coords);
typedef GLvoid (csAPIENTRY* csGLVERTEXSTREAM2S) (GLenum stream, GLshort coords);
typedef GLvoid (csAPIENTRY* csGLVERTEXSTREAM2I) (GLenum stream, GLint coords);
typedef GLvoid (csAPIENTRY* csGLVERTEXSTREAM2F) (GLenum stream, GLfloat coords);
typedef GLvoid (csAPIENTRY* csGLVERTEXSTREAM2D) (GLenum stream, GLdouble coords);
typedef GLvoid (csAPIENTRY* csGLVERTEXSTREAM2SV) (GLenum stream, GLshort coords);
typedef GLvoid (csAPIENTRY* csGLVERTEXSTREAM2IV) (GLenum stream, GLint coords);
typedef GLvoid (csAPIENTRY* csGLVERTEXSTREAM2FV) (GLenum stream, GLfloat coords);
typedef GLvoid (csAPIENTRY* csGLVERTEXSTREAM2DV) (GLenum stream, GLdouble coords);
typedef GLvoid (csAPIENTRY* csGLVERTEXSTREAM3S) (GLenum stream, GLshort coords);
typedef GLvoid (csAPIENTRY* csGLVERTEXSTREAM3I) (GLenum stream, GLint coords);
typedef GLvoid (csAPIENTRY* csGLVERTEXSTREAM3F) (GLenum stream, GLfloat coords);
typedef GLvoid (csAPIENTRY* csGLVERTEXSTREAM3D) (GLenum stream, GLdouble coords);
typedef GLvoid (csAPIENTRY* csGLVERTEXSTREAM3SV) (GLenum stream, GLshort coords);
typedef GLvoid (csAPIENTRY* csGLVERTEXSTREAM3IV) (GLenum stream, GLint coords);
typedef GLvoid (csAPIENTRY* csGLVERTEXSTREAM3FV) (GLenum stream, GLfloat coords);
typedef GLvoid (csAPIENTRY* csGLVERTEXSTREAM3DV) (GLenum stream, GLdouble coords);
typedef GLvoid (csAPIENTRY* csGLVERTEXSTREAM4S) (GLenum stream, GLshort coords);
typedef GLvoid (csAPIENTRY* csGLVERTEXSTREAM4I) (GLenum stream, GLint coords);
typedef GLvoid (csAPIENTRY* csGLVERTEXSTREAM4F) (GLenum stream, GLfloat coords);
typedef GLvoid (csAPIENTRY* csGLVERTEXSTREAM4D) (GLenum stream, GLdouble coords);
typedef GLvoid (csAPIENTRY* csGLVERTEXSTREAM4SV) (GLenum stream, GLshort coords);
typedef GLvoid (csAPIENTRY* csGLVERTEXSTREAM4IV) (GLenum stream, GLint coords);
typedef GLvoid (csAPIENTRY* csGLVERTEXSTREAM4FV) (GLenum stream, GLfloat coords);
typedef GLvoid (csAPIENTRY* csGLVERTEXSTREAM4DV) (GLenum stream, GLdouble coords);
typedef GLvoid (csAPIENTRY* csGLNORMALSTREAM3B) (GLenum stream, GLbyte coords);
typedef GLvoid (csAPIENTRY* csGLNORMALSTREAM3S) (GLenum stream, GLshort coords);
typedef GLvoid (csAPIENTRY* csGLNORMALSTREAM3I) (GLenum stream, GLint coords);
typedef GLvoid (csAPIENTRY* csGLNORMALSTREAM3F) (GLenum stream, GLfloat coords);
typedef GLvoid (csAPIENTRY* csGLNORMALSTREAM3D) (GLenum stream, GLdouble coords);
typedef GLvoid (csAPIENTRY* csGLNORMALSTREAM3BV) (GLenum stream, GLbyte coords);
typedef GLvoid (csAPIENTRY* csGLNORMALSTREAM3SV) (GLenum stream, GLshort coords);
typedef GLvoid (csAPIENTRY* csGLNORMALSTREAM3IV) (GLenum stream, GLint coords);
typedef GLvoid (csAPIENTRY* csGLNORMALSTREAM3FV) (GLenum stream, GLfloat coords);
typedef GLvoid (csAPIENTRY* csGLNORMALSTREAM3DV) (GLenum stream, GLdouble coords);
typedef GLvoid (csAPIENTRY* csGLCLIENTACTIVEVERTEXSTREAM) (GLenum stream);
typedef GLvoid (csAPIENTRY* csGLVERTEXBLENDENVI) (GLenum pname, GLint param);
typedef GLvoid (csAPIENTRY* csGLVERTEXBLENDENVF) (GLenum pname, GLfloat param);

// WGL_I3D_image_buffer
#define WGL_IMAGE_BUFFER_MIN_ACCESS_I3D                                    1
#define WGL_IMAGE_BUFFER_LOCK_I3D                                          2
typedef GLvoid* (csAPIENTRY* csWGLCREATEIMAGEBUFFERI3D) (HDC hDC, DWORD dwSize, UINT uFlags);
typedef BOOL (csAPIENTRY* csWGLDESTROYIMAGEBUFFERI3D) (HDC hDC, GLvoid* pAddress);
typedef BOOL (csAPIENTRY* csWGLASSOCIATEIMAGEBUFFEREVENTSI3D) (HDC hdc, HANDLE* pEvent, GLvoid* pAddress, DWORD* pSize, UINT count);
typedef BOOL (csAPIENTRY* csWGLRELEASEIMAGEBUFFEREVENTSI3D) (HDC hdc, GLvoid* pAddress, UINT count);

// WGL_I3D_swap_frame_lock
typedef BOOL (csAPIENTRY* csWGLENABLEFRAMELOCKI3D) ();
typedef BOOL (csAPIENTRY* csWGLDISABLEFRAMELOCKI3D) ();
typedef BOOL (csAPIENTRY* csWGLISENABLEDFRAMELOCKI3D) (BOOL* pFlag);
typedef BOOL (csAPIENTRY* csWGLQUERYFRAMELOCKMASTERI3D) (BOOL* pFlag);

// WGL_I3D_swap_frame_usage
typedef BOOL (csAPIENTRY* csWGLGETFRAMEUSAGEI3D) (GLfloat* pUsage);
typedef BOOL (csAPIENTRY* csWGLBEGINFRAMETRACKINGI3D) ();
typedef BOOL (csAPIENTRY* csWGLENDFRAMETRACKINGI3D) ();
typedef BOOL (csAPIENTRY* csWGLQUERYFRAMETRACKINGI3D) (DWORD* pFrameCount, DWORD* pMissedFrames, GLfloat* pLastMissedUsage);

// GL_3DFX_texture_compression_FXT1
#define GL_COMPRESSED_RGB_FXT1_3DFX                                        34480
#define GL_COMPRESSED_RGBA_FXT1_3DFX                                       34481

// GL_IBM_cull_vertex
#define GL_CULL_VERTEX_IBM                                                 103050

// GL_IBM_multimode_draw_arrays
typedef GLvoid (csAPIENTRY* csGLMULTIMODEDRAWARRAYSIBM) (GLenum* mode, GLint* first, GLsizei* count, GLsizei primcount, GLint modestride);
typedef GLvoid (csAPIENTRY* csGLMULTIMODEDRAWELEMENTSIBM) (GLenum* mode, GLsizei* count, GLenum type, GLvoid* indices, GLsizei primcount, GLint modestride);

// GL_IBM_raster_pos_clip
#define GL_RASTER_POSITION_UNCLIPPED_IBM                                   103010

// GL_IBM_texture_mirrored_repeat
#define GL_MIRRORED_REPEAT_IBM                                             33648

// GL_IBM_vertex_array_lists
#define GL_VERTEX_ARRAY_LIST_IBM                                           103070
#define GL_NORMAL_ARRAY_LIST_IBM                                           103071
#define GL_COLOR_ARRAY_LIST_IBM                                            103072
#define GL_INDEX_ARRAY_LIST_IBM                                            103073
#define GL_TEXTURE_COORD_ARRAY_LIST_IBM                                    103074
#define GL_EDGE_FLAG_ARRAY_LIST_IBM                                        103075
#define GL_FOG_COORDINATE_ARRAY_LIST_IBM                                   103076
#define GL_SECONDARY_COLOR_ARRAY_LIST_IBM                                  103077
#define GL_VERTEX_ARRAY_LIST_STRIDE_IBM                                    103080
#define GL_NORMAL_ARRAY_LIST_STRIDE_IBM                                    103081
#define GL_COLOR_ARRAY_LIST_STRIDE_IBM                                     103082
#define GL_INDEX_ARRAY_LIST_STRIDE_IBM                                     103083
#define GL_TEXTURE_COORD_ARRAY_LIST_STRIDE_IBM                             103084
#define GL_EDGE_FLAG_ARRAY_LIST_STRIDE_IBM                                 103085
#define GL_FOG_COORDINATE_ARRAY_LIST_STRIDE_IBM                            103086
#define GL_SECONDARY_COLOR_ARRAY_LIST_STRIDE_IBM                           103087
typedef GLvoid (csAPIENTRY* csGLCOLORPOINTERLISTIBM) (GLint size, GLenum type, GLint stride, GLvoid* pointer, GLint ptrstride);
typedef GLvoid (csAPIENTRY* csGLSECONDARYCOLORPOINTERLISTIBM) (GLint size, GLenum type, GLint stride, GLvoid* pointer, GLint ptrstride);
typedef GLvoid (csAPIENTRY* csGLEDGEFLAGPOINTERLISTIBM) (GLint stride, GLboolean* pointer, GLint ptrstride);
typedef GLvoid (csAPIENTRY* csGLFOGCOORDPOINTERLISTIBM) (GLenum type, GLint stride, GLvoid* pointer, GLint ptrstride);
typedef GLvoid (csAPIENTRY* csGLNORMALPOINTERLISTIBM) (GLenum type, GLint stride, GLvoid* pointer, GLint ptrstride);
typedef GLvoid (csAPIENTRY* csGLTEXCOORDPOINTERLISTIBM) (GLint size, GLenum type, GLint stride, GLvoid* pointer, GLint ptrstride);
typedef GLvoid (csAPIENTRY* csGLVERTEXPOINTERLISTIBM) (GLint size, GLenum type, GLint stride, GLvoid* pointer, GLint ptrstride);

// GL_MESA_resize_buffers
typedef GLvoid (csAPIENTRY* csGLRESIZEBUFFERSMESA) ();

// GL_MESA_window_pos
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS2DMESA) (GLdouble x, GLdouble y);
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS2FMESA) (GLfloat x, GLfloat y);
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS2IMESA) (GLint x, GLint y);
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS2SMESA) (GLshort x, GLshort y);
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS2IVMESA) (GLint* p);
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS2SVMESA) (GLshort* p);
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS2FVMESA) (GLfloat* p);
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS2DVMESA) (GLdouble* p);
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS3IMESA) (GLint x, GLint y, GLint z);
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS3SMESA) (GLshort x, GLshort y, GLshort z);
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS3FMESA) (GLfloat x, GLfloat y, GLfloat z);
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS3DMESA) (GLdouble x, GLdouble y, GLdouble z);
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS3IVMESA) (GLint* p);
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS3SVMESA) (GLshort* p);
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS3FVMESA) (GLfloat* p);
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS3DVMESA) (GLdouble* p);
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS4IMESA) (GLint x, GLint y, GLint z, GLint w);
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS4SMESA) (GLshort x, GLshort y, GLshort z, GLshort w);
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS4FMESA) (GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS4DMESA) (GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS4IVMESA) (GLint* p);
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS4SVMESA) (GLshort* p);
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS4FVMESA) (GLfloat* p);
typedef GLvoid (csAPIENTRY* csGLWINDOWPOS4DVMESA) (GLdouble* p);

// GL_OML_interlace
#define GL_INTERLACE_OML                                                   35200
#define GL_INTERLACE_READ_OML                                              35201

// GL_OML_resample
#define GL_PACK_RESAMPLE_OML                                               35204
#define GL_UNPACK_RESAMPLE_OML                                             35205
#define GL_RESAMPLE_REPLICATE_OML                                          35206
#define GL_RESAMPLE_ZERO_FILL_OML                                          35207
#define GL_RESAMPLE_AVERAGE_OML                                            35208
#define GL_RESAMPLE_DECIMATE_OML                                           35209
#define GL_RESAMPLE_AVERAGE_OML                                            35208

// GL_OML_subsample
#define GL_FORMAT_SUBSAMPLE_24_24_OML                                      35202
#define GL_FORMAT_SUBSAMPLE_244_244_OML                                    35203

// GL_SGIS_generate_mipmap
#define GL_GENERATE_MIPMAP_SGIS                                            33169
#define GL_GENERATE_MIPMAP_HINT_SGIS                                       33170

// GL_SGIS_multisample
#define GLX_SAMPLE_BUFFERS_SGIS                                            100000
#define GLX_SAMPLES_SGIS                                                   100001
#define GL_MULTISAMPLE_SGIS                                                32925
#define GL_SAMPLE_ALPHA_TO_MASK_SGIS                                       32926
#define GL_SAMPLE_ALPHA_TO_ONE_SGIS                                        32927
#define GL_SAMPLE_MASK_SGIS                                                32928
#define GL_MULTISAMPLE_BIT_EXT                                             536870912
#define GL_1PASS_SGIS                                                      32929
#define GL_2PASS_0_SGIS                                                    32930
#define GL_2PASS_1_SGIS                                                    32931
#define GL_4PASS_0_SGIS                                                    32932
#define GL_4PASS_1_SGIS                                                    32933
#define GL_4PASS_2_SGIS                                                    32934
#define GL_4PASS_3_SGIS                                                    32935
#define GL_SAMPLE_BUFFERS_SGIS                                             32936
#define GL_SAMPLES_SGIS                                                    32937
#define GL_SAMPLE_MASK_VALUE_SGIS                                          32938
#define GL_SAMPLE_MASK_INVERT_SGIS                                         32939
#define GL_SAMPLE_PATTERN_SGIS                                             32940
typedef GLvoid (csAPIENTRY* csGLSAMPLEMASKSGIS) (GLclampf value, GLboolean invert);
typedef GLvoid (csAPIENTRY* csGLSAMPLEPATTERNSGIS) (GLenum pattern);

// GL_SGIS_pixel_texture
#define GL_PIXEL_TEXTURE_SGIS                                              33619
#define GL_PIXEL_FRAGMENT_RGB_SOURCE_SGIS                                  33620
#define GL_PIXEL_FRAGMENT_ALPHA_SOURCE_SGIS                                33621
#define GL_PIXEL_GROUP_COLOR_SGIS                                          33622
typedef GLvoid (csAPIENTRY* csGLPIXELTEXGENPARAMETERISGIS) (GLenum pname, GLint param);
typedef GLvoid (csAPIENTRY* csGLPIXELTEXGENPARAMETERFSGIS) (GLenum pname, GLfloat param);
typedef GLvoid (csAPIENTRY* csGLGETPIXELTEXGENPARAMETERIVSGIS) (GLenum pname, GLint params);
typedef GLvoid (csAPIENTRY* csGLGETPIXELTEXGENPARAMETERFVSGIS) (GLenum pname, GLfloat params);

// GL_SGIS_texture_border_clamp
#define GL_CLAMP_TO_BORDER_SGIS                                            33069

// GL_SGIS_texture_color_mask
#define GL_TEXTURE_COLOR_WRITEMASK_SGIS                                    33263
typedef GLvoid (csAPIENTRY* csGLTEXTURECOLORMASKSGIS) (GLboolean r, GLboolean g, GLboolean b, GLboolean a);

// GL_SGIS_texture_edge_clamp
#define GL_CLAMP_TO_EDGE_SGIS                                              33071

// GL_SGIS_texture_lod
#define GL_TEXTURE_MIN_LOD_SGIS                                            33082
#define GL_TEXTURE_MAX_LOD_SGIS                                            33083
#define GL_TEXTURE_BASE_LEVEL_SGIS                                         33084
#define GL_TEXTURE_MAX_LEVEL_SGIS                                          33085

// GL_SGIS_depth_texture
#define GL_DEPTH_COMPONENT16_SGIX                                          33189
#define GL_DEPTH_COMPONENT24_SGIX                                          33190
#define GL_DEPTH_COMPONENT32_SGIX                                          33191

// GL_SGIX_fog_offset
#define GL_FOG_OFFSET_SGIX                                                 33176
#define GL_FOG_OFFSET_VALUE_SGIX                                           33177

// GL_SGIX_interlace
#define GL_INTERLACE_SGIX                                                  32916

// GL_SGIX_shadow_ambient
#define GL_SHADOW_AMBIENT_SGIX                                             32959

// GL_SGI_color_matrix
#define GL_COLOR_MATRIX_SGI                                                32945
#define GL_COLOR_MATRIX_STACK_DEPTH_SGI                                    32946
#define GL_MAX_COLOR_MATRIX_STACK_DEPTH_SGI                                32947
#define GL_POST_COLOR_MATRIX_RED_SCALE_SGI                                 32948
#define GL_POST_COLOR_MATRIX_GREEN_SCALE_SGI                               32949
#define GL_POST_COLOR_MATRIX_BLUE_SCALE_SGI                                32950
#define GL_POST_COLOR_MATRIX_ALPHA_SCALE_SGI                               32951
#define GL_POST_COLOR_MATRIX_RED_BIAS_SGI                                  32952
#define GL_POST_COLOR_MATRIX_GREEN_BIAS_SGI                                32953
#define GL_POST_COLOR_MATRIX_BLUE_BIAS_SGI                                 32954
#define GL_POST_COLOR_MATRIX_ALPHA_BIAS_SGI                                32955

// GL_SGI_color_table
#define GL_COLOR_TABLE_SGI                                                 32976
#define GL_POST_CONVOLUTION_COLOR_TABLE_SGI                                32977
#define GL_POST_COLOR_MATRIX_COLOR_TABLE_SGI                               32978
#define GL_PROXY_COLOR_TABLE_SGI                                           32979
#define GL_PROXY_POST_CONVOLUTION_COLOR_TABLE_SGI                          32980
#define GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE_SGI                         32981
#define GL_COLOR_TABLE_SCALE_SGI                                           32982
#define GL_COLOR_TABLE_BIAS_SGI                                            32983
#define GL_COLOR_TABLE_FORMAT_SGI                                          32984
#define GL_COLOR_TABLE_WIDTH_SGI                                           32985
#define GL_COLOR_TABLE_RED_SIZE_SGI                                        32986
#define GL_COLOR_TABLE_GREEN_SIZE_SGI                                      32987
#define GL_COLOR_TABLE_BLUE_SIZE_SGI                                       32988
#define GL_COLOR_TABLE_ALPHA_SIZE_SGI                                      32989
#define GL_COLOR_TABLE_LUMINANCE_SIZE_SGI                                  32990
#define GL_COLOR_TABLE_INTENSITY_SIZE_SGI                                  32991
typedef GLvoid (csAPIENTRY* csGLCOLORTABLESGI) (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, GLvoid* table);
typedef GLvoid (csAPIENTRY* csGLCOPYCOLORTABLESGI) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
typedef GLvoid (csAPIENTRY* csGLCOLORTABLEPARAMETERIVSGI) (GLenum target, GLenum pname, GLint* params);
typedef GLvoid (csAPIENTRY* csGLCOLORTABLEPARAMETERFVSGI) (GLenum target, GLenum pname, GLfloat* params);
typedef GLvoid (csAPIENTRY* csGLGETCOLORTABLESGI) (GLenum target, GLenum format, GLenum type, GLvoid* table);
typedef GLvoid (csAPIENTRY* csGLGETCOLORTABLEPARAMETERIVSGI) (GLenum target, GLenum pname, GLint* params);
typedef GLvoid (csAPIENTRY* csGLGETCOLORTABLEPARAMETERFVSGI) (GLenum target, GLenum pname, GLfloat* params);

// GL_SGI_texture_color_table
#define GL_TEXTURE_COLOR_TABLE_SGI                                         32956
#define GL_PROXY_TEXTURE_COLOR_TABLE_SGI                                   32957

// GL_SUN_vertex
typedef GLvoid (csAPIENTRY* csGLCOLOR4UBVERTEX2FSUN) (GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y);
typedef GLvoid (csAPIENTRY* csGLCOLOR4UBVERTEX2FVSUN) (GLubyte* c, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLCOLOR4UBVERTEX3FSUN) (GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z);
typedef GLvoid (csAPIENTRY* csGLCOLOR4UBVERTEX3FVSUN) (GLubyte* c, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLCOLOR3FVERTEX3FSUN) (GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z);
typedef GLvoid (csAPIENTRY* csGLCOLOR3FVERTEX3FVSUN) (GLfloat* c, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLNORMAL3FVERTEX3FSUN) (GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
typedef GLvoid (csAPIENTRY* csGLNORMAL3FVERTEX3FVSUN) (GLfloat* n, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLCOLOR4FNORMAL3FVERTEX3FSUN) (GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
typedef GLvoid (csAPIENTRY* csGLCOLOR4FNORMAL3FVERTEX3FVSUN) (GLfloat* c, GLfloat* n, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLTEXCOORD2FVERTEX3FSUN) (GLfloat s, GLfloat t, GLfloat x, GLfloat y, GLfloat z);
typedef GLvoid (csAPIENTRY* csGLTEXCOORD2FVERTEX3FVSUN) (GLfloat* tc, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLTEXCOORD4FVERTEX4FSUN) (GLfloat s, GLfloat t, GLfloat p, GLfloat q, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef GLvoid (csAPIENTRY* csGLTEXCOORD4FVERTEX4FVSUN) (GLfloat* tc, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLTEXCOORD2FCOLOR4UBVERTEX3FSUN) (GLfloat s, GLfloat t, GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z);
typedef GLvoid (csAPIENTRY* csGLTEXCOORD2FCOLOR4UBVERTEX3FVSUN) (GLfloat* tc, GLubyte* c, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLTEXCOORD2FCOLOR3FVERTEX3FSUN) (GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z);
typedef GLvoid (csAPIENTRY* csGLTEXCOORD2FCOLOR3FVERTEX3FVSUN) (GLfloat* tc, GLfloat* c, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLTEXCOORD2FNORMAL3FVERTEX3FSUN) (GLfloat s, GLfloat t, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
typedef GLvoid (csAPIENTRY* csGLTEXCOORD2FNORMAL3FVERTEX3FVSUN) (GLfloat* tc, GLfloat* n, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUN) (GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
typedef GLvoid (csAPIENTRY* csGLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUN) (GLfloat* tc, GLfloat* c, GLfloat* n, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FSUN) (GLfloat s, GLfloat t, GLfloat p, GLfloat q, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef GLvoid (csAPIENTRY* csGLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FVSUN) (GLfloat* tc, GLfloat* c, GLfloat* n, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLREPLACEMENTCODEUIVERTEX3FSUN) (GLuint rc, GLfloat x, GLfloat y, GLfloat z);
typedef GLvoid (csAPIENTRY* csGLREPLACEMENTCODEUIVERTEX3FVSUN) (GLuint* rc, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLREPLACEMENTCODEUICOLOR4UBVERTEX3FSUN) (GLuint rc, GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z);
typedef GLvoid (csAPIENTRY* csGLREPLACEMENTCODEUICOLOR4UBVERTEX3FVSUN) (GLuint* rc, GLubyte* c, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLREPLACEMENTCODEUICOLOR3FVERTEX3FSUN) (GLuint rc, GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z);
typedef GLvoid (csAPIENTRY* csGLREPLACEMENTCODEUICOLOR3FVERTEX3FVSUN) (GLuint* rc, GLfloat* c, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLREPLACEMENTCODEUINORMAL3FVERTEX3FSUN) (GLuint rc, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
typedef GLvoid (csAPIENTRY* csGLREPLACEMENTCODEUINORMAL3FVERTEX3FVSUN) (GLuint* rc, GLfloat* n, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FSUN) (GLuint rc, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
typedef GLvoid (csAPIENTRY* csGLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FVSUN) (GLuint* rc, GLfloat* c, GLfloat* n, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLREPLACEMENTCODEUITEXCOORD2FVERTEX3FSUN) (GLuint rc, GLfloat s, GLfloat t, GLfloat x, GLfloat y, GLfloat z);
typedef GLvoid (csAPIENTRY* csGLREPLACEMENTCODEUITEXCOORD2FVERTEX3FVSUN) (GLuint* rc, GLfloat* tc, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FSUN) (GLuint rc, GLfloat s, GLfloat t, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
typedef GLvoid (csAPIENTRY* csGLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FVSUN) (GLuint* rc, GLfloat* tc, GLfloat* n, GLfloat* v);
typedef GLvoid (csAPIENTRY* csGLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUN) (GLuint rc, GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
typedef GLvoid (csAPIENTRY* csGLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUN) (GLuint* rc, GLfloat* tc, GLfloat* c, GLfloat* n, GLfloat* v);

// GL_ARB_fragment_program
#define GL_FRAGMENT_PROGRAM_ARB                                            34820
#define GL_PROGRAM_FORMAT_ASCII_ARB                                        34933
#define GL_PROGRAM_LENGTH_ARB                                              34343
#define GL_PROGRAM_FORMAT_ARB                                              34934
#define GL_PROGRAM_BINDING_ARB                                             34423
#define GL_PROGRAM_INSTRUCTIONS_ARB                                        34976
#define GL_MAX_PROGRAM_INSTRUCTIONS_ARB                                    34977
#define GL_PROGRAM_NATIVE_INSTRUCTIONS_ARB                                 34978
#define GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB                             34979
#define GL_PROGRAM_TEMPORARIES_ARB                                         34980
#define GL_MAX_PROGRAM_TEMPORARIES_ARB                                     34981
#define GL_PROGRAM_NATIVE_TEMPORARIES_ARB                                  34982
#define GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB                              34983
#define GL_PROGRAM_PARAMETERS_ARB                                          34984
#define GL_MAX_PROGRAM_PARAMETERS_ARB                                      34985
#define GL_PROGRAM_NATIVE_PARAMETERS_ARB                                   34986
#define GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB                               34987
#define GL_PROGRAM_ATTRIBS_ARB                                             34988
#define GL_MAX_PROGRAM_ATTRIBS_ARB                                         34989
#define GL_PROGRAM_NATIVE_ATTRIBS_ARB                                      34990
#define GL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB                                  34991
#define GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB                                34996
#define GL_MAX_PROGRAM_ENV_PARAMETERS_ARB                                  34997
#define GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB                                 34998
#define GL_PROGRAM_ALU_INSTRUCTIONS_ARB                                    34821
#define GL_PROGRAM_TEX_INSTRUCTIONS_ARB                                    34822
#define GL_PROGRAM_TEX_INDIRECTIONS_ARB                                    34823
#define GL_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB                             34824
#define GL_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB                             34825
#define GL_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB                             34826
#define GL_MAX_PROGRAM_ALU_INSTRUCTIONS_ARB                                34827
#define GL_MAX_PROGRAM_TEX_INSTRUCTIONS_ARB                                34828
#define GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB                                34829
#define GL_MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB                         34830
#define GL_MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB                         34831
#define GL_MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB                         34832
#define GL_PROGRAM_STRING_ARB                                              34344
#define GL_PROGRAM_ERROR_POSITION_ARB                                      34379
#define GL_CURRENT_MATRIX_ARB                                              34369
#define GL_TRANSPOSE_CURRENT_MATRIX_ARB                                    34999
#define GL_CURRENT_MATRIX_STACK_DEPTH_ARB                                  34368
#define GL_MAX_PROGRAM_MATRICES_ARB                                        34351
#define GL_MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB                              34350
#define GL_MAX_TEXTURE_COORDS_ARB                                          34929
#define GL_MAX_TEXTURE_IMAGE_UNITS_ARB                                     34930
#define GL_PROGRAM_ERROR_STRING_ARB                                        34932
#define GL_MATRIX0_ARB                                                     35008
#define GL_MATRIX1_ARB                                                     35009
#define GL_MATRIX2_ARB                                                     35010
#define GL_MATRIX3_ARB                                                     35011
#define GL_MATRIX4_ARB                                                     35012
#define GL_MATRIX5_ARB                                                     35013
#define GL_MATRIX6_ARB                                                     35014
#define GL_MATRIX7_ARB                                                     35015
#define GL_MATRIX8_ARB                                                     35016
#define GL_MATRIX9_ARB                                                     35017
#define GL_MATRIX10_ARB                                                    35018
#define GL_MATRIX11_ARB                                                    35019
#define GL_MATRIX12_ARB                                                    35020
#define GL_MATRIX13_ARB                                                    35021
#define GL_MATRIX14_ARB                                                    35022
#define GL_MATRIX15_ARB                                                    35023
#define GL_MATRIX16_ARB                                                    35024
#define GL_MATRIX17_ARB                                                    35025
#define GL_MATRIX18_ARB                                                    35026
#define GL_MATRIX19_ARB                                                    35027
#define GL_MATRIX20_ARB                                                    35028
#define GL_MATRIX21_ARB                                                    35029
#define GL_MATRIX22_ARB                                                    35030
#define GL_MATRIX23_ARB                                                    35031
#define GL_MATRIX24_ARB                                                    35032
#define GL_MATRIX25_ARB                                                    35033
#define GL_MATRIX26_ARB                                                    35034
#define GL_MATRIX27_ARB                                                    35035
#define GL_MATRIX28_ARB                                                    35036
#define GL_MATRIX29_ARB                                                    35037
#define GL_MATRIX30_ARB                                                    35038
#define GL_MATRIX31_ARB                                                    35039
typedef GLvoid (csAPIENTRY* csGLPROGRAMSTRINGARB) (GLenum target, GLenum format, GLsizei len, GLvoid* string);
typedef GLvoid (csAPIENTRY* csGLBINDPROGRAMARB) (GLenum target, GLuint program);
typedef GLvoid (csAPIENTRY* csGLDELETEPROGRAMSARB) (GLsizei n, GLuint* programs);
typedef GLvoid (csAPIENTRY* csGLGENPROGRAMSARB) (GLsizei n, GLuint* programs);
typedef GLvoid (csAPIENTRY* csGLPROGRAMENVPARAMETER4DARB) (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef GLvoid (csAPIENTRY* csGLPROGRAMENVPARAMETER4DVARB) (GLenum target, GLuint index, GLdouble* params);
typedef GLvoid (csAPIENTRY* csGLPROGRAMENVPARAMETER4FARB) (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef GLvoid (csAPIENTRY* csGLPROGRAMENVPARAMETER4FVARB) (GLenum target, GLuint index, GLfloat* params);
typedef GLvoid (csAPIENTRY* csGLPROGRAMLOCALPARAMETER4DARB) (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef GLvoid (csAPIENTRY* csGLPROGRAMLOCALPARAMETER4DVARB) (GLenum target, GLuint index, GLdouble* params);
typedef GLvoid (csAPIENTRY* csGLPROGRAMLOCALPARAMETER4FARB) (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef GLvoid (csAPIENTRY* csGLPROGRAMLOCALPARAMETER4FVARB) (GLenum target, GLuint index, GLfloat* params);
typedef GLvoid (csAPIENTRY* csGLGETPROGRAMENVPARAMETERDVARB) (GLenum target, GLuint index, GLdouble* params);
typedef GLvoid (csAPIENTRY* csGLGETPROGRAMENVPARAMETERFVARB) (GLenum target, GLuint index, GLfloat* params);
typedef GLvoid (csAPIENTRY* csGLGETPROGRAMLOCALPARAMETERDVARB) (GLenum target, GLuint index, GLdouble* params);
typedef GLvoid (csAPIENTRY* csGLGETPROGRAMLOCALPARAMETERFVARB) (GLenum target, GLuint index, GLfloat* params);
typedef GLvoid (csAPIENTRY* csGLGETPROGRAMIVARB) (GLenum target, GLenum pname, GLint* params);
typedef GLvoid (csAPIENTRY* csGLGETPROGRAMSTRINGARB) (GLenum target, GLenum pname, GLvoid* string);
typedef GLboolean (csAPIENTRY* csGLISPROGRAMARB) (GLuint program);

// GL_ATI_text_fragment_shader
#define GL_TEXT_FRAGMENT_SHADER_ATI                                        33280

// GL_APPLE_client_storage
#define GL_UNPACK_CLIENT_STORAGE_APPLE                                     34226

// GL_APPLE_element_array
#define GL_ELEMENT_ARRAY_APPLE                                             34664
#define GL_ELEMENT_ARRAY_TYPE_APPLE                                        34665
#define GL_ELEMENT_ARRAY_POINTER_APPLE                                     34666
typedef GLvoid (csAPIENTRY* csGLELEMENTPOINTERAPPLE) (GLenum type, GLvoid* pointer);
typedef GLvoid (csAPIENTRY* csGLDRAWELEMENTARRAYAPPLE) (GLenum mode, GLint first, GLsizei count);
typedef GLvoid (csAPIENTRY* csGLDRAWRANGEELEMENTARRAYAPPLE) (GLenum mode, GLuint start, GLuint end, GLint first, GLsizei count);
typedef GLvoid (csAPIENTRY* csGLMULTIDRAWELEMENTARRAYAPPLE) (GLenum mode, GLint* first, GLsizei* count, GLsizei primcount);
typedef GLvoid (csAPIENTRY* csGLMULTIDRAWRANGEELEMENTARRAYAPPLE) (GLenum mode, GLuint start, GLuint end, GLint* first, GLsizei* count, GLsizei primcount);

// GL_APPLE_fence
#define GL_DRAW_PIXELS_APPLE                                               35338
#define GL_FENCE_APPLE                                                     35339
typedef GLvoid (csAPIENTRY* csGLGENFENCESAPPLE) (GLsizei n, GLuint* fences);
typedef GLvoid (csAPIENTRY* csGLDELETEFENCESAPPLE) (GLsizei n, GLuint* fences);
typedef GLvoid (csAPIENTRY* csGLSETFENCEAPPLE) (GLuint fence);
typedef GLboolean (csAPIENTRY* csGLISFENCEAPPLE) (GLuint fence);
typedef GLboolean (csAPIENTRY* csGLTESTFENCEAPPLE) (GLuint fence);
typedef GLvoid (csAPIENTRY* csGLFINISHFENCEAPPLE) (GLuint fence);
typedef GLboolean (csAPIENTRY* csGLTESTOBJECTAPPLE) (GLenum object, GLuint name);
typedef GLvoid (csAPIENTRY* csGLFINISHOBJECTAPPLE) (GLenum object, GLint name);

// GL_APPLE_vertex_array_object
#define GL_VERTEX_ARRAY_BINDING_APPLE                                      34229
typedef GLvoid (csAPIENTRY* csGLBINDVERTEXARRAYAPPLE) (GLuint array);
typedef GLvoid (csAPIENTRY* csGLDELETEVERTEXARRAYSAPPLE) (GLsizei n, GLuint* arrays);
typedef GLvoid (csAPIENTRY* csGLGENVERTEXARRAYSAPPLE) (GLsizei n, GLuint* arrays);
typedef GLboolean (csAPIENTRY* csGLISVERTEXARRAYAPPLE) (GLuint array);

// GL_APPLE_vertex_array_range
#define GL_VERTEX_ARRAY_RANGE_APPLE                                        34077
#define GL_VERTEX_ARRAY_RANGE_LENGTH_APPLE                                 34078
#define GL_MAX_VERTEX_ARRAY_RANGE_ELEMENT_APPLE                            34080
#define GL_VERTEX_ARRAY_RANGE_POINTER_APPLE                                34081
#define GL_VERTEX_ARRAY_STORAGE_HINT_APPLE                                 34079
#define GL_STORAGE_CACHED_APPLE                                            34238
#define GL_STORAGE_SHARED_APPLE                                            34239
typedef GLvoid (csAPIENTRY* csGLVERTEXARRAYRANGEAPPLE) (GLsizei length, GLvoid* pointer);
typedef GLvoid (csAPIENTRY* csGLFLUSHVERTEXARRAYRANGEAPPLE) (GLsizei length, GLvoid* pointer);
typedef GLvoid (csAPIENTRY* csGLVERTEXARRAYPARAMETERIAPPLE) (GLenum pname, GLint param);

// WGL_ARB_pixel_format
#define WGL_NUMBER_PIXEL_FORMATS_ARB                                       8192
#define WGL_DRAW_TO_WINDOW_ARB                                             8193
#define WGL_DRAW_TO_BITMAP_ARB                                             8194
#define WGL_ACCELERATION_ARB                                               8195
#define WGL_NEED_PALETTE_ARB                                               8196
#define WGL_NEED_SYSTEM_PALETTE_ARB                                        8197
#define WGL_SWAP_LAYER_BUFFERS_ARB                                         8198
#define WGL_SWAP_METHOD_ARB                                                8199
#define WGL_NUMBER_OVERLAYS_ARB                                            8200
#define WGL_NUMBER_UNDERLAYS_ARB                                           8201
#define WGL_TRANSPARENT_ARB                                                8202
#define WGL_TRANSPARENT_RED_VALUE_ARB                                      8247
#define WGL_TRANSPARENT_GREEN_VALUE_ARB                                    8248
#define WGL_TRANSPARENT_BLUE_VALUE_ARB                                     8249
#define WGL_TRANSPARENT_ALPHA_VALUE_ARB                                    8250
#define WGL_TRANSPARENT_INDEX_VALUE_ARB                                    8251
#define WGL_SHARE_DEPTH_ARB                                                8204
#define WGL_SHARE_STENCIL_ARB                                              8205
#define WGL_SHARE_ACCUM_ARB                                                8206
#define WGL_SUPPORT_GDI_ARB                                                8207
#define WGL_SUPPORT_OPENGL_ARB                                             8208
#define WGL_DOUBLE_BUFFER_ARB                                              8209
#define WGL_STEREO_ARB                                                     8210
#define WGL_PIXEL_TYPE_ARB                                                 8211
#define WGL_COLOR_BITS_ARB                                                 8212
#define WGL_RED_BITS_ARB                                                   8213
#define WGL_RED_SHIFT_ARB                                                  8214
#define WGL_GREEN_BITS_ARB                                                 8215
#define WGL_GREEN_SHIFT_ARB                                                8216
#define WGL_BLUE_BITS_ARB                                                  8217
#define WGL_BLUE_SHIFT_ARB                                                 8218
#define WGL_ALPHA_BITS_ARB                                                 8219
#define WGL_ALPHA_SHIFT_ARB                                                8220
#define WGL_ACCUM_BITS_ARB                                                 8221
#define WGL_ACCUM_RED_BITS_ARB                                             8222
#define WGL_ACCUM_GREEN_BITS_ARB                                           8223
#define WGL_ACCUM_BLUE_BITS_ARB                                            8224
#define WGL_ACCUM_ALPHA_BITS_ARB                                           8225
#define WGL_DEPTH_BITS_ARB                                                 8226
#define WGL_STENCIL_BITS_ARB                                               8227
#define WGL_AUX_BUFFERS_ARB                                                8228
#define WGL_NO_ACCELERATION_ARB                                            8229
#define WGL_GENERIC_ACCELERATION_ARB                                       8230
#define WGL_FULL_ACCELERATION_ARB                                          8231
#define WGL_SWAP_EXCHANGE_ARB                                              8232
#define WGL_SWAP_COPY_ARB                                                  8233
#define WGL_SWAP_UNDEFINED_ARB                                             8234
#define WGL_TYPE_RGBA_ARB                                                  8235
#define WGL_TYPE_COLORINDEX_ARB                                            8236
typedef BOOL (csAPIENTRY* csWGLGETPIXELFORMATATTRIBIVARB) (HDC hdc, GLint iPixelFormat, GLint iLayerPlane, GLuint nAttributes, GLint* piAttributes, GLint* piValues);
typedef BOOL (csAPIENTRY* csWGLGETPIXELFORMATATTRIBFVARB) (HDC hdc, GLint iPixelFormat, GLint iLayerPlane, GLuint nAttributes, GLint* piAttributes, GLfloat* pfValues);
typedef BOOL (csAPIENTRY* csWGLCHOOSEPIXELFORMATARB) (HDC hdc, GLint* piAttribIList, GLfloat* pfAttribFList, GLuint nMaxFormats, GLint* piFormats, GLuint* nNumFormats);

// WGL_ARB_make_current_read
#define WGL_ERROR_INVALID_PIXEL_TYPE_ARB                                   8259
#define WGL_ERROR_INCOMPATIBLE_DEVICE_CONTEXTS_ARB                         8276
typedef BOOL (csAPIENTRY* csWGLMAKECONTEXTCURRENTARB) (HDC hDrawDC, HDC hReadDC, HGLRC hglrc);
typedef HDC (csAPIENTRY* csWGLGETCURRENTREADDCARB) ();

// WGL_ARB_pbuffer
#define WGL_DRAW_TO_PBUFFER_ARB                                            8237
#define WGL_DRAW_TO_PBUFFER_ARB                                            8237
#define WGL_MAX_PBUFFER_PIXELS_ARB                                         8238
#define WGL_MAX_PBUFFER_WIDTH_ARB                                          8239
#define WGL_MAX_PBUFFER_HEIGHT_ARB                                         8240
#define WGL_PBUFFER_LARGEST_ARB                                            8243
#define WGL_PBUFFER_WIDTH_ARB                                              8244
#define WGL_PBUFFER_HEIGHT_ARB                                             8245
#define WGL_PBUFFER_LOST_ARB                                               8246
typedef HANDLE (csAPIENTRY* csWGLCREATEPBUFFERARB) (HDC hDC, GLint iPixelFormat, GLint iWidth, GLint iHeight, GLint* piAttribList);
typedef HDC (csAPIENTRY* csWGLGETPBUFFERDCARB) (HANDLE hPbuffer);
typedef GLint (csAPIENTRY* csWGLRELEASEPBUFFERDCARB) (HANDLE hPbuffer, HDC hDC);
typedef BOOL (csAPIENTRY* csWGLDESTROYPBUFFERARB) (HANDLE hPbuffer);
typedef BOOL (csAPIENTRY* csWGLQUERYPBUFFERARB) (HANDLE hPbuffer, GLint iAttribute, GLint* piValue);

// WGL_EXT_swap_control
typedef BOOL (csAPIENTRY* csWGLSWAPINTERVALEXT) (GLint interval);
typedef GLint (csAPIENTRY* csWGLGETSWAPINTERVALEXT) ();

// WGL_ARB_render_texture
#define WGL_BIND_TO_TEXTURE_RGB_ARB                                        8304
#define WGL_BIND_TO_TEXTURE_RGBA_ARB                                       8305
#define WGL_TEXTURE_FORMAT_ARB                                             8306
#define WGL_TEXTURE_TARGET_ARB                                             8307
#define WGL_MIPMAP_TEXTURE_ARB                                             8308
#define WGL_TEXTURE_RGB_ARB                                                8309
#define WGL_TEXTURE_RGBA_ARB                                               8310
#define WGL_NO_TEXTURE_ARB                                                 8311
#define WGL_TEXTURE_CUBE_MAP_ARB                                           8312
#define WGL_TEXTURE_1D_ARB                                                 8313
#define WGL_TEXTURE_2D_ARB                                                 8314
#define WGL_NO_TEXTURE_ARB                                                 8311
#define WGL_MIPMAP_LEVEL_ARB                                               8315
#define WGL_CUBE_MAP_FACE_ARB                                              8316
#define WGL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB                                8317
#define WGL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB                                8318
#define WGL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB                                8319
#define WGL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB                                8320
#define WGL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB                                8321
#define WGL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB                                8322
#define WGL_FRONT_LEFT_ARB                                                 8323
#define WGL_FRONT_RIGHT_ARB                                                8324
#define WGL_BACK_LEFT_ARB                                                  8325
#define WGL_BACK_RIGHT_ARB                                                 8326
#define WGL_AUX0_ARB                                                       8327
#define WGL_AUX1_ARB                                                       8328
#define WGL_AUX2_ARB                                                       8329
#define WGL_AUX3_ARB                                                       8330
#define WGL_AUX4_ARB                                                       8331
#define WGL_AUX5_ARB                                                       8332
#define WGL_AUX6_ARB                                                       8333
#define WGL_AUX7_ARB                                                       8334
#define WGL_AUX8_ARB                                                       8335
#define WGL_AUX9_ARB                                                       8336
typedef BOOL (csAPIENTRY* csWGLBINDTEXIMAGEARB) (HANDLE hPbuffer, GLint iBuffer);
typedef BOOL (csAPIENTRY* csWGLRELEASETEXIMAGEARB) (HANDLE hPbuffer, GLint iBuffer);
typedef BOOL (csAPIENTRY* csWGLSETPBUFFERATTRIBARB) (HANDLE hPbuffer, GLint* piAttribList);

// WGL_EXT_extensions_string
typedef char* (csAPIENTRY* csWGLGETEXTENSIONSSTRINGEXT) ();

// WGL_EXT_make_current_read
typedef BOOL (csAPIENTRY* csWGLMAKECONTEXTCURRENTEXT) (HDC hDrawDC, HDC hReadDC, HGLRC hglrc);
typedef HDC (csAPIENTRY* csWGLGETCURRENTREADDCEXT) ();

// WGL_EXT_pbuffer
#define WGL_DRAW_TO_PBUFFER_EXT                                            8237
#define WGL_MAX_PBUFFER_PIXELS_EXT                                         8238
#define WGL_MAX_PBUFFER_WIDTH_EXT                                          8239
#define WGL_MAX_PBUFFER_HEIGHT_EXT                                         8240
#define WGL_OPTIMAL_PBUFFER_WIDTH_EXT                                      8241
#define WGL_OPTIMAL_PBUFFER_HEIGHT_EXT                                     8242
#define WGL_PBUFFER_LARGEST_EXT                                            8243
#define WGL_PBUFFER_WIDTH_EXT                                              8244
#define WGL_PBUFFER_HEIGHT_EXT                                             8245
typedef HANDLE (csAPIENTRY* csWGLCREATEPBUFFEREXT) (HDC hDC, GLint iPixelFormat, GLint iWidth, GLint iHeight, GLint* piAttribList);
typedef HDC (csAPIENTRY* csWGLGETPBUFFERDCEXT) (HANDLE hPbuffer);
typedef GLint (csAPIENTRY* csWGLRELEASEPBUFFERDCEXT) (HANDLE hPbuffer, HDC hDC);
typedef BOOL (csAPIENTRY* csWGLDESTROYPBUFFEREXT) (HANDLE hPbuffer);
typedef BOOL (csAPIENTRY* csWGLQUERYPBUFFEREXT) (HANDLE hPbuffer, GLint iAttribute, GLint* piValue);

// WGL_EXT_pixel_format
#define WGL_NUMBER_PIXEL_FORMATS_EXT                                       8192
#define WGL_DRAW_TO_WINDOW_EXT                                             8193
#define WGL_DRAW_TO_BITMAP_EXT                                             8194
#define WGL_ACCELERATION_EXT                                               8195
#define WGL_NEED_PALETTE_EXT                                               8196
#define WGL_NEED_SYSTEM_PALETTE_EXT                                        8197
#define WGL_SWAP_LAYER_BUFFERS_EXT                                         8198
#define WGL_SWAP_METHOD_EXT                                                8199
#define WGL_NUMBER_OVERLAYS_EXT                                            8200
#define WGL_NUMBER_UNDERLAYS_EXT                                           8201
#define WGL_TRANSPARENT_EXT                                                8202
#define WGL_TRANSPARENT_VALUE_EXT                                          8203
#define WGL_SHARE_DEPTH_EXT                                                8204
#define WGL_SHARE_STENCIL_EXT                                              8205
#define WGL_SHARE_ACCUM_EXT                                                8206
#define WGL_SUPPORT_GDI_EXT                                                8207
#define WGL_SUPPORT_OPENGL_EXT                                             8208
#define WGL_DOUBLE_BUFFER_EXT                                              8209
#define WGL_STEREO_EXT                                                     8210
#define WGL_PIXEL_TYPE_EXT                                                 8211
#define WGL_COLOR_BITS_EXT                                                 8212
#define WGL_RED_BITS_EXT                                                   8213
#define WGL_RED_SHIFT_EXT                                                  8214
#define WGL_GREEN_BITS_EXT                                                 8215
#define WGL_GREEN_SHIFT_EXT                                                8216
#define WGL_BLUE_BITS_EXT                                                  8217
#define WGL_BLUE_SHIFT_EXT                                                 8218
#define WGL_ALPHA_BITS_EXT                                                 8219
#define WGL_ALPHA_SHIFT_EXT                                                8220
#define WGL_ACCUM_BITS_EXT                                                 8221
#define WGL_ACCUM_RED_BITS_EXT                                             8222
#define WGL_ACCUM_GREEN_BITS_EXT                                           8223
#define WGL_ACCUM_BLUE_BITS_EXT                                            8224
#define WGL_ACCUM_ALPHA_BITS_EXT                                           8225
#define WGL_DEPTH_BITS_EXT                                                 8226
#define WGL_STENCIL_BITS_EXT                                               8227
#define WGL_AUX_BUFFERS_EXT                                                8228
#define WGL_NO_ACCELERATION_EXT                                            8229
#define WGL_GENERIC_ACCELERATION_EXT                                       8230
#define WGL_FULL_ACCELERATION_EXT                                          8231
#define WGL_SWAP_EXCHANGE_EXT                                              8232
#define WGL_SWAP_COPY_EXT                                                  8233
#define WGL_SWAP_UNDEFINED_EXT                                             8234
#define WGL_TYPE_RGBA_EXT                                                  8235
#define WGL_TYPE_COLORINDEX_EXT                                            8236
typedef BOOL (csAPIENTRY* csWGLGETPIXELFORMATATTRIBIVEXT) (HDC hdc, GLint iPixelFormat, GLint iLayerPlane, GLuint nAttributes, GLint* piAttributes, GLint* piValues);
typedef BOOL (csAPIENTRY* csWGLGETPIXELFORMATATTRIBFVEXT) (HDC hdc, GLint iPixelFormat, GLint iLayerPlane, GLuint nAttributes, GLint* piAttributes, GLfloat* pfValues);
typedef BOOL (csAPIENTRY* csWGLCHOOSEPIXELFORMATEXT) (HDC hdc, GLint* piAttribIList, GLfloat* pfAttribFList, GLuint nMaxFormats, GLint* piFormats, GLuint* nNumFormats);

// WGL_I3D_digital_video_control
#define WGL_DIGITAL_VIDEO_CURSOR_ALPHA_FRAMEBUFFER_I3D                     8272
#define WGL_DIGITAL_VIDEO_CURSOR_ALPHA_VALUE_I3D                           8273
#define WGL_DIGITAL_VIDEO_CURSOR_INCLUDED_I3D                              8274
#define WGL_DIGITAL_VIDEO_GAMMA_CORRECTED_I3D                              8275
typedef BOOL (csAPIENTRY* csWGLGETDIGITALVIDEOPARAMETERSI3D) (HDC hDC, GLint iAttribute, GLint* piValue);
typedef BOOL (csAPIENTRY* csWGLSETDIGITALVIDEOPARAMETERSI3D) (HDC hDC, GLint iAttribute, GLint* piValue);

// WGL_I3D_gamma
#define WGL_GAMMA_TABLE_SIZE_I3D                                           8270
#define WGL_GAMMA_EXCLUDE_DESKTOP_I3D                                      8271
#define WGL_GAMMA_EXCLUDE_DESKTOP_I3D                                      8271
typedef BOOL (csAPIENTRY* csWGLGETGAMMATABLEPARAMETERSI3D) (HDC hDC, GLint iAttribute, GLint* piValue);
typedef BOOL (csAPIENTRY* csWGLSETGAMMATABLEPARAMETERSI3D) (HDC hDC, GLint iAttribute, GLint* piValue);
typedef BOOL (csAPIENTRY* csWGLGETGAMMATABLEI3D) (HDC hDC, GLint iEntries, GLushort* puRed, GLushort* puGreen, GLushort* puBlue);
typedef BOOL (csAPIENTRY* csWGLSETGAMMATABLEI3D) (HDC hDC, GLint iEntries, GLushort* puRed, GLushort* puGreen, GLushort* puBlue);

// WGL_I3D_genlock
#define WGL_GENLOCK_SOURCE_MULTIVIEW_I3D                                   8260
#define WGL_GENLOCK_SOURCE_EXTERNAL_SYNC_I3D                               8261
#define WGL_GENLOCK_SOURCE_EXTERNAL_FIELD_I3D                              8262
#define WGL_GENLOCK_SOURCE_EXTERNAL_TTL_I3D                                8263
#define WGL_GENLOCK_SOURCE_DIGITAL_SYNC_I3D                                8264
#define WGL_GENLOCK_SOURCE_DIGITAL_FIELD_I3D                               8265
#define WGL_GENLOCK_SOURCE_EDGE_FALLING_I3D                                8266
#define WGL_GENLOCK_SOURCE_EDGE_RISING_I3D                                 8267
#define WGL_GENLOCK_SOURCE_EDGE_BOTH_I3D                                   8268
typedef BOOL (csAPIENTRY* csWGLENABLEGENLOCKI3D) (HDC hDC);
typedef BOOL (csAPIENTRY* csWGLDISABLEGENLOCKI3D) (HDC hDC);
typedef BOOL (csAPIENTRY* csWGLISENABLEDGENLOCKI3D) (HDC hDC, BOOL* pFlag);
typedef BOOL (csAPIENTRY* csWGLGENLOCKSOURCEI3D) (HDC hDC, GLuint uSource);
typedef BOOL (csAPIENTRY* csWGLGETGENLOCKSOURCEI3D) (HDC hDC, GLuint* uSource);
typedef BOOL (csAPIENTRY* csWGLGENLOCKSOURCEEDGEI3D) (HDC hDC, GLuint uEdge);
typedef BOOL (csAPIENTRY* csWGLGETGENLOCKSOURCEEDGEI3D) (HDC hDC, GLuint* uEdge);
typedef BOOL (csAPIENTRY* csWGLGENLOCKSAMPLERATEI3D) (HDC hDC, GLuint uRate);
typedef BOOL (csAPIENTRY* csWGLGETGENLOCKSAMPLERATEI3D) (HDC hDC, GLuint* uRate);
typedef BOOL (csAPIENTRY* csWGLGENLOCKSOURCEDELAYI3D) (HDC hDC, GLuint uDelay);
typedef BOOL (csAPIENTRY* csWGLGETGENLOCKSOURCEDELAYI3D) (HDC hDC, GLuint* uDelay);
typedef BOOL (csAPIENTRY* csWGLQUERYGENLOCKMAXSOURCEDELAYI3D) (HDC hDC, GLuint* uMaxLineDelay, GLuint* uMaxPixelDelay);

// GL_ARB_matrix_palette
#define GL_MATRIX_PALETTE_ARB                                              34880
#define GL_MAX_MATRIX_PALETTE_STACK_DEPTH_ARB                              34881
#define GL_MAX_PALETTE_MATRICES_ARB                                        34882
#define GL_CURRENT_PALETTE_MATRIX_ARB                                      34883
#define GL_MATRIX_INDEX_ARRAY_ARB                                          34884
#define GL_CURRENT_MATRIX_INDEX_ARB                                        34885
#define GL_MATRIX_INDEX_ARRAY_SIZE_ARB                                     34886
#define GL_MATRIX_INDEX_ARRAY_TYPE_ARB                                     34887
#define GL_MATRIX_INDEX_ARRAY_STRIDE_ARB                                   34888
#define GL_MATRIX_INDEX_ARRAY_POINTER_ARB                                  34889
typedef GLvoid (csAPIENTRY* csGLCURRENTPALETTEMATRIXARB) (GLint index);
typedef GLvoid (csAPIENTRY* csGLMATRIXINDEXUBVARB) (GLint size, GLubyte* indices);
typedef GLvoid (csAPIENTRY* csGLMATRIXINDEXUSVARB) (GLint size, GLushort* indices);
typedef GLvoid (csAPIENTRY* csGLMATRIXINDEXUIVARB) (GLint size, GLuint* indices);
typedef GLvoid (csAPIENTRY* csGLMATRIXINDEXPOINTERARB) (GLint size, GLenum type, GLsizei stride, GLvoid* pointer);

// GL_NV_element_array
#define GL_ELEMENT_ARRAY_TYPE_NV                                           34665
#define GL_ELEMENT_ARRAY_POINTER_NV                                        34666
typedef GLvoid (csAPIENTRY* csGLELEMENTPOINTERNV) (GLenum type, GLvoid* pointer);
typedef GLvoid (csAPIENTRY* csGLDRAWELEMENTARRAYNV) (GLenum mode, GLint first, GLsizei count);
typedef GLvoid (csAPIENTRY* csGLDRAWRANGEELEMENTARRAYNV) (GLenum mode, GLuint start, GLuint end, GLint first, GLsizei count);
typedef GLvoid (csAPIENTRY* csGLMULTIDRAWELEMENTARRAYNV) (GLenum mode, GLint* first, GLsizei* count, GLsizei primcount);
typedef GLvoid (csAPIENTRY* csGLMULTIDRAWRANGEELEMENTARRAYNV) (GLenum mode, GLuint start, GLuint end, GLint* first, GLsizei* count, GLsizei primcount);

// GL_NV_float_buffer
#define GL_FLOAT_R_NV                                                      34944
#define GL_FLOAT_RG_NV                                                     34945
#define GL_FLOAT_RGB_NV                                                    34946
#define GL_FLOAT_RGBA_NV                                                   34947
#define GL_FLOAT_R16_NV                                                    34948
#define GL_FLOAT_R32_NV                                                    34949
#define GL_FLOAT_RG16_NV                                                   34950
#define GL_FLOAT_RG32_NV                                                   34951
#define GL_FLOAT_RGB16_NV                                                  34952
#define GL_FLOAT_RGB32_NV                                                  34953
#define GL_FLOAT_RGBA16_NV                                                 34954
#define GL_FLOAT_RGBA32_NV                                                 34955
#define GL_TEXTURE_FLOAT_COMPONENTS_NV                                     34956
#define GL_FLOAT_CLEAR_COLOR_VALUE_NV                                      34957
#define GL_FLOAT_RGBA_MODE_NV                                              34958
#define WGL_FLOAT_COMPONENTS_NV                                            8368
#define WGL_BIND_TO_TEXTURE_RECTANGLE_FLOAT_R_NV                           8369
#define WGL_BIND_TO_TEXTURE_RECTANGLE_FLOAT_RG_NV                          8370
#define WGL_BIND_TO_TEXTURE_RECTANGLE_FLOAT_RGB_NV                         8371
#define WGL_BIND_TO_TEXTURE_RECTANGLE_FLOAT_RGBA_NV                        8372
#define WGL_TEXTURE_FLOAT_R_NV                                             8373
#define WGL_TEXTURE_FLOAT_RG_NV                                            8374
#define WGL_TEXTURE_FLOAT_RGB_NV                                           8375
#define WGL_TEXTURE_FLOAT_RGBA_NV                                          8376

// GL_NV_fragment_program
#define GL_FRAGMENT_PROGRAM_NV                                             34928
#define GL_MAX_TEXTURE_COORDS_NV                                           34929
#define GL_MAX_TEXTURE_IMAGE_UNITS_NV                                      34930
#define GL_FRAGMENT_PROGRAM_BINDING_NV                                     34931
#define GL_MAX_FRAGMENT_PROGRAM_LOCAL_PARAMETERS_NV                        34920
#define GL_PROGRAM_ERROR_STRING_NV                                         34932
typedef GLvoid (csAPIENTRY* csGLPROGRAMNAMEDPARAMETER4FNV) (GLuint id, GLsizei len, GLubyte* name, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef GLvoid (csAPIENTRY* csGLPROGRAMNAMEDPARAMETER4DNV) (GLuint id, GLsizei len, GLubyte* name, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef GLvoid (csAPIENTRY* csGLGETPROGRAMNAMEDPARAMETERFVNV) (GLuint id, GLsizei len, GLubyte* name, GLfloat* params);
typedef GLvoid (csAPIENTRY* csGLGETPROGRAMNAMEDPARAMETERDVNV) (GLuint id, GLsizei len, GLubyte* name, GLdouble* params);
typedef GLvoid (csAPIENTRY* csGLPROGRAMLOCALPARAMETER4DARB) (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef GLvoid (csAPIENTRY* csGLPROGRAMLOCALPARAMETER4DVARB) (GLenum target, GLuint index, GLdouble* params);
typedef GLvoid (csAPIENTRY* csGLPROGRAMLOCALPARAMETER4FARB) (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef GLvoid (csAPIENTRY* csGLPROGRAMLOCALPARAMETER4FVARB) (GLenum target, GLuint index, GLfloat* params);
typedef GLvoid (csAPIENTRY* csGLGETPROGRAMLOCALPARAMETERDVARB) (GLenum target, GLuint index, GLdouble* params);
typedef GLvoid (csAPIENTRY* csGLGETPROGRAMLOCALPARAMETERFVARB) (GLenum target, GLuint index, GLfloat* params);

// GL_NV_primitive_restart
#define GL_PRIMITIVE_RESTART_NV                                            34136
#define GL_PRIMITIVE_RESTART_INDEX_NV                                      34137
typedef GLvoid (csAPIENTRY* csGLPRIMITIVERESTARTNV) ();
typedef GLvoid (csAPIENTRY* csGLPRIMITIVERESTARTINDEXNV) (GLuint index);

// GL_NV_vertex_program2




class csGLExtensionManager
{
public:
  bool CS_GL_version_1_2;
  bool CS_GL_ARB_imaging;
  bool CS_GL_version_1_3;
  bool CS_GL_ARB_multitexture;
  bool CS_GL_ARB_transpose_matrix;
  bool CS_GL_ARB_multisample;
  bool CS_GL_ARB_texture_env_add;
  bool CS_WGL_ARB_extensions_string;
  bool CS_WGL_ARB_buffer_region;
  bool CS_GL_ARB_texture_cube_map;
  bool CS_GL_ARB_depth_texture;
  bool CS_GL_ARB_point_parameters;
  bool CS_GL_ARB_shadow;
  bool CS_GL_ARB_shadow_ambient;
  bool CS_GL_ARB_texture_border_clamp;
  bool CS_GL_ARB_texture_compression;
  bool CS_GL_ARB_texture_env_combine;
  bool CS_GL_ARB_texture_env_crossbar;
  bool CS_GL_ARB_texture_env_dot3;
  bool CS_GL_ARB_texture_mirrored_repeat;
  bool CS_GL_ARB_vertex_blend;
  bool CS_GL_ARB_vertex_program;
  bool CS_GL_ARB_window_pos;
  bool CS_GL_EXT_422_pixels;
  bool CS_GL_EXT_abgr;
  bool CS_GL_EXT_bgra;
  bool CS_GL_EXT_blend_color;
  bool CS_GL_EXT_blend_func_separate;
  bool CS_GL_EXT_blend_logic_op;
  bool CS_GL_EXT_blend_minmax;
  bool CS_GL_EXT_blend_subtract;
  bool CS_GL_EXT_clip_volume_hint;
  bool CS_GL_EXT_color_subtable;
  bool CS_GL_EXT_compiled_vertex_array;
  bool CS_GL_EXT_convolution;
  bool CS_GL_EXT_fog_coord;
  bool CS_GL_EXT_histogram;
  bool CS_GL_EXT_multi_draw_arrays;
  bool CS_GL_EXT_packed_pixels;
  bool CS_GL_EXT_paletted_texture;
  bool CS_GL_EXT_point_parameters;
  bool CS_GL_EXT_polygon_offset;
  bool CS_GL_EXT_secondary_color;
  bool CS_GL_EXT_separate_specular_color;
  bool CS_GL_EXT_shadow_funcs;
  bool CS_GL_EXT_shared_texture_palette;
  bool CS_GL_EXT_stencil_two_side;
  bool CS_GL_EXT_stencil_wrap;
  bool CS_GL_EXT_subtexture;
  bool CS_GL_EXT_texture3D;
  bool CS_GL_EXT_texture_compression_s3tc;
  bool CS_GL_EXT_texture_env_add;
  bool CS_GL_EXT_texture_env_combine;
  bool CS_GL_EXT_texture_env_dot3;
  bool CS_GL_EXT_texture_filter_anisotropic;
  bool CS_GL_EXT_texture_lod_bias;
  bool CS_GL_EXT_texture_object;
  bool CS_GL_EXT_vertex_array;
  bool CS_GL_EXT_vertex_shader;
  bool CS_GL_EXT_vertex_weighting;
  bool CS_GL_HP_occlusion_test;
  bool CS_GL_NV_blend_square;
  bool CS_GL_NV_copy_depth_to_color;
  bool CS_GL_NV_depth_clamp;
  bool CS_GL_NV_evaluators;
  bool CS_GL_NV_fence;
  bool CS_GL_NV_fog_distance;
  bool CS_GL_NV_light_max_exponent;
  bool CS_GL_NV_multisample_filter_hint;
  bool CS_GL_NV_occlusion_query;
  bool CS_GL_NV_packed_depth_stencil;
  bool CS_GL_NV_point_sprite;
  bool CS_GL_NV_register_combiners;
  bool CS_GL_NV_register_combiners2;
  bool CS_GL_NV_texgen_emboss;
  bool CS_GL_NV_texgen_reflection;
  bool CS_GL_NV_texture_compression_vtc;
  bool CS_GL_NV_texture_env_combine4;
  bool CS_GL_NV_texture_rectangle;
  bool CS_GL_NV_texture_shader;
  bool CS_GL_NV_texture_shader2;
  bool CS_GL_NV_texture_shader3;
  bool CS_GL_NV_vertex_array_range;
  bool CS_GL_NV_vertex_array_range2;
  bool CS_GL_NV_vertex_program;
  bool CS_GL_NV_vertex_program1_1;
  bool CS_GL_ATI_element_array;
  bool CS_GL_ATI_envmap_bumpmap;
  bool CS_GL_ATI_fragment_shader;
  bool CS_GL_ATI_pn_triangles;
  bool CS_GL_ATI_texture_mirror_once;
  bool CS_GL_ATI_vertex_array_object;
  bool CS_GL_ATI_vertex_streams;
  bool CS_WGL_I3D_image_buffer;
  bool CS_WGL_I3D_swap_frame_lock;
  bool CS_WGL_I3D_swap_frame_usage;
  bool CS_GL_3DFX_texture_compression_FXT1;
  bool CS_GL_IBM_cull_vertex;
  bool CS_GL_IBM_multimode_draw_arrays;
  bool CS_GL_IBM_raster_pos_clip;
  bool CS_GL_IBM_texture_mirrored_repeat;
  bool CS_GL_IBM_vertex_array_lists;
  bool CS_GL_MESA_resize_buffers;
  bool CS_GL_MESA_window_pos;
  bool CS_GL_OML_interlace;
  bool CS_GL_OML_resample;
  bool CS_GL_OML_subsample;
  bool CS_GL_SGIS_generate_mipmap;
  bool CS_GL_SGIS_multisample;
  bool CS_GL_SGIS_pixel_texture;
  bool CS_GL_SGIS_texture_border_clamp;
  bool CS_GL_SGIS_texture_color_mask;
  bool CS_GL_SGIS_texture_edge_clamp;
  bool CS_GL_SGIS_texture_lod;
  bool CS_GL_SGIS_depth_texture;
  bool CS_GL_SGIX_fog_offset;
  bool CS_GL_SGIX_interlace;
  bool CS_GL_SGIX_shadow_ambient;
  bool CS_GL_SGI_color_matrix;
  bool CS_GL_SGI_color_table;
  bool CS_GL_SGI_texture_color_table;
  bool CS_GL_SUN_vertex;
  bool CS_GL_ARB_fragment_program;
  bool CS_GL_ATI_text_fragment_shader;
  bool CS_GL_APPLE_client_storage;
  bool CS_GL_APPLE_element_array;
  bool CS_GL_APPLE_fence;
  bool CS_GL_APPLE_vertex_array_object;
  bool CS_GL_APPLE_vertex_array_range;
  bool CS_WGL_ARB_pixel_format;
  bool CS_WGL_ARB_make_current_read;
  bool CS_WGL_ARB_pbuffer;
  bool CS_WGL_EXT_swap_control;
  bool CS_WGL_ARB_render_texture;
  bool CS_WGL_EXT_extensions_string;
  bool CS_WGL_EXT_make_current_read;
  bool CS_WGL_EXT_pbuffer;
  bool CS_WGL_EXT_pixel_format;
  bool CS_WGL_I3D_digital_video_control;
  bool CS_WGL_I3D_gamma;
  bool CS_WGL_I3D_genlock;
  bool CS_GL_ARB_matrix_palette;
  bool CS_GL_NV_element_array;
  bool CS_GL_NV_float_buffer;
  bool CS_GL_NV_fragment_program;
  bool CS_GL_NV_primitive_restart;
  bool CS_GL_NV_vertex_program2;

  // GL_version_1_2
  #ifndef GLBLENDCOLOR_DECL
  #define GLBLENDCOLOR_DECL
  csGLBLENDCOLOR glBlendColor;
  #endif

  #ifndef GLBLENDEQUATION_DECL
  #define GLBLENDEQUATION_DECL
  csGLBLENDEQUATION glBlendEquation;
  #endif

  #ifndef GLDRAWRANGEELEMENTS_DECL
  #define GLDRAWRANGEELEMENTS_DECL
  csGLDRAWRANGEELEMENTS glDrawRangeElements;
  #endif

  #ifndef GLCOLORTABLE_DECL
  #define GLCOLORTABLE_DECL
  csGLCOLORTABLE glColorTable;
  #endif

  #ifndef GLCOLORTABLEPARAMETERFV_DECL
  #define GLCOLORTABLEPARAMETERFV_DECL
  csGLCOLORTABLEPARAMETERFV glColorTableParameterfv;
  #endif

  #ifndef GLCOLORTABLEPARAMETERIV_DECL
  #define GLCOLORTABLEPARAMETERIV_DECL
  csGLCOLORTABLEPARAMETERIV glColorTableParameteriv;
  #endif

  #ifndef GLCOPYCOLORTABLE_DECL
  #define GLCOPYCOLORTABLE_DECL
  csGLCOPYCOLORTABLE glCopyColorTable;
  #endif

  #ifndef GLGETCOLORTABLE_DECL
  #define GLGETCOLORTABLE_DECL
  csGLGETCOLORTABLE glGetColorTable;
  #endif

  #ifndef GLGETCOLORTABLEPARAMETERFV_DECL
  #define GLGETCOLORTABLEPARAMETERFV_DECL
  csGLGETCOLORTABLEPARAMETERFV glGetColorTableParameterfv;
  #endif

  #ifndef GLGETCOLORTABLEPARAMETERIV_DECL
  #define GLGETCOLORTABLEPARAMETERIV_DECL
  csGLGETCOLORTABLEPARAMETERIV glGetColorTableParameteriv;
  #endif

  #ifndef GLCOLORSUBTABLE_DECL
  #define GLCOLORSUBTABLE_DECL
  csGLCOLORSUBTABLE glColorSubTable;
  #endif

  #ifndef GLCOPYCOLORSUBTABLE_DECL
  #define GLCOPYCOLORSUBTABLE_DECL
  csGLCOPYCOLORSUBTABLE glCopyColorSubTable;
  #endif

  #ifndef GLCONVOLUTIONFILTER1D_DECL
  #define GLCONVOLUTIONFILTER1D_DECL
  csGLCONVOLUTIONFILTER1D glConvolutionFilter1D;
  #endif

  #ifndef GLCONVOLUTIONFILTER2D_DECL
  #define GLCONVOLUTIONFILTER2D_DECL
  csGLCONVOLUTIONFILTER2D glConvolutionFilter2D;
  #endif

  #ifndef GLCONVOLUTIONPARAMETERF_DECL
  #define GLCONVOLUTIONPARAMETERF_DECL
  csGLCONVOLUTIONPARAMETERF glConvolutionParameterf;
  #endif

  #ifndef GLCONVOLUTIONPARAMETERFV_DECL
  #define GLCONVOLUTIONPARAMETERFV_DECL
  csGLCONVOLUTIONPARAMETERFV glConvolutionParameterfv;
  #endif

  #ifndef GLCONVOLUTIONPARAMETERI_DECL
  #define GLCONVOLUTIONPARAMETERI_DECL
  csGLCONVOLUTIONPARAMETERI glConvolutionParameteri;
  #endif

  #ifndef GLCONVOLUTIONPARAMETERIV_DECL
  #define GLCONVOLUTIONPARAMETERIV_DECL
  csGLCONVOLUTIONPARAMETERIV glConvolutionParameteriv;
  #endif

  #ifndef GLCOPYCONVOLUTIONFILTER1D_DECL
  #define GLCOPYCONVOLUTIONFILTER1D_DECL
  csGLCOPYCONVOLUTIONFILTER1D glCopyConvolutionFilter1D;
  #endif

  #ifndef GLCOPYCONVOLUTIONFILTER2D_DECL
  #define GLCOPYCONVOLUTIONFILTER2D_DECL
  csGLCOPYCONVOLUTIONFILTER2D glCopyConvolutionFilter2D;
  #endif

  #ifndef GLGETCONVOLUTIONFILTER_DECL
  #define GLGETCONVOLUTIONFILTER_DECL
  csGLGETCONVOLUTIONFILTER glGetConvolutionFilter;
  #endif

  #ifndef GLGETCONVOLUTIONPARAMETERFV_DECL
  #define GLGETCONVOLUTIONPARAMETERFV_DECL
  csGLGETCONVOLUTIONPARAMETERFV glGetConvolutionParameterfv;
  #endif

  #ifndef GLGETCONVOLUTIONPARAMETERIV_DECL
  #define GLGETCONVOLUTIONPARAMETERIV_DECL
  csGLGETCONVOLUTIONPARAMETERIV glGetConvolutionParameteriv;
  #endif

  #ifndef GLGETSEPARABLEFILTER_DECL
  #define GLGETSEPARABLEFILTER_DECL
  csGLGETSEPARABLEFILTER glGetSeparableFilter;
  #endif

  #ifndef GLSEPARABLEFILTER2D_DECL
  #define GLSEPARABLEFILTER2D_DECL
  csGLSEPARABLEFILTER2D glSeparableFilter2D;
  #endif

  #ifndef GLGETHISTOGRAM_DECL
  #define GLGETHISTOGRAM_DECL
  csGLGETHISTOGRAM glGetHistogram;
  #endif

  #ifndef GLGETHISTOGRAMPARAMETERFV_DECL
  #define GLGETHISTOGRAMPARAMETERFV_DECL
  csGLGETHISTOGRAMPARAMETERFV glGetHistogramParameterfv;
  #endif

  #ifndef GLGETHISTOGRAMPARAMETERIV_DECL
  #define GLGETHISTOGRAMPARAMETERIV_DECL
  csGLGETHISTOGRAMPARAMETERIV glGetHistogramParameteriv;
  #endif

  #ifndef GLGETMINMAX_DECL
  #define GLGETMINMAX_DECL
  csGLGETMINMAX glGetMinmax;
  #endif

  #ifndef GLGETMINMAXPARAMETERFV_DECL
  #define GLGETMINMAXPARAMETERFV_DECL
  csGLGETMINMAXPARAMETERFV glGetMinmaxParameterfv;
  #endif

  #ifndef GLGETMINMAXPARAMETERIV_DECL
  #define GLGETMINMAXPARAMETERIV_DECL
  csGLGETMINMAXPARAMETERIV glGetMinmaxParameteriv;
  #endif

  #ifndef GLHISTOGRAM_DECL
  #define GLHISTOGRAM_DECL
  csGLHISTOGRAM glHistogram;
  #endif

  #ifndef GLMINMAX_DECL
  #define GLMINMAX_DECL
  csGLMINMAX glMinmax;
  #endif

  #ifndef GLRESETHISTOGRAM_DECL
  #define GLRESETHISTOGRAM_DECL
  csGLRESETHISTOGRAM glResetHistogram;
  #endif

  #ifndef GLRESETMINMAX_DECL
  #define GLRESETMINMAX_DECL
  csGLRESETMINMAX glResetMinmax;
  #endif

  #ifndef GLTEXIMAGE3D_DECL
  #define GLTEXIMAGE3D_DECL
  csGLTEXIMAGE3D glTexImage3D;
  #endif

  #ifndef GLTEXSUBIMAGE3D_DECL
  #define GLTEXSUBIMAGE3D_DECL
  csGLTEXSUBIMAGE3D glTexSubImage3D;
  #endif

  #ifndef GLCOPYTEXSUBIMAGE3D_DECL
  #define GLCOPYTEXSUBIMAGE3D_DECL
  csGLCOPYTEXSUBIMAGE3D glCopyTexSubImage3D;
  #endif


  // GL_ARB_imaging

  // GL_version_1_3
  #ifndef GLACTIVETEXTURE_DECL
  #define GLACTIVETEXTURE_DECL
  csGLACTIVETEXTURE glActiveTexture;
  #endif

  #ifndef GLCLIENTACTIVETEXTURE_DECL
  #define GLCLIENTACTIVETEXTURE_DECL
  csGLCLIENTACTIVETEXTURE glClientActiveTexture;
  #endif

  #ifndef GLMULTITEXCOORD1D_DECL
  #define GLMULTITEXCOORD1D_DECL
  csGLMULTITEXCOORD1D glMultiTexCoord1d;
  #endif

  #ifndef GLMULTITEXCOORD1DV_DECL
  #define GLMULTITEXCOORD1DV_DECL
  csGLMULTITEXCOORD1DV glMultiTexCoord1dv;
  #endif

  #ifndef GLMULTITEXCOORD1F_DECL
  #define GLMULTITEXCOORD1F_DECL
  csGLMULTITEXCOORD1F glMultiTexCoord1f;
  #endif

  #ifndef GLMULTITEXCOORD1FV_DECL
  #define GLMULTITEXCOORD1FV_DECL
  csGLMULTITEXCOORD1FV glMultiTexCoord1fv;
  #endif

  #ifndef GLMULTITEXCOORD1I_DECL
  #define GLMULTITEXCOORD1I_DECL
  csGLMULTITEXCOORD1I glMultiTexCoord1i;
  #endif

  #ifndef GLMULTITEXCOORD1IV_DECL
  #define GLMULTITEXCOORD1IV_DECL
  csGLMULTITEXCOORD1IV glMultiTexCoord1iv;
  #endif

  #ifndef GLMULTITEXCOORD1S_DECL
  #define GLMULTITEXCOORD1S_DECL
  csGLMULTITEXCOORD1S glMultiTexCoord1s;
  #endif

  #ifndef GLMULTITEXCOORD1SV_DECL
  #define GLMULTITEXCOORD1SV_DECL
  csGLMULTITEXCOORD1SV glMultiTexCoord1sv;
  #endif

  #ifndef GLMULTITEXCOORD2D_DECL
  #define GLMULTITEXCOORD2D_DECL
  csGLMULTITEXCOORD2D glMultiTexCoord2d;
  #endif

  #ifndef GLMULTITEXCOORD2DV_DECL
  #define GLMULTITEXCOORD2DV_DECL
  csGLMULTITEXCOORD2DV glMultiTexCoord2dv;
  #endif

  #ifndef GLMULTITEXCOORD2F_DECL
  #define GLMULTITEXCOORD2F_DECL
  csGLMULTITEXCOORD2F glMultiTexCoord2f;
  #endif

  #ifndef GLMULTITEXCOORD2FV_DECL
  #define GLMULTITEXCOORD2FV_DECL
  csGLMULTITEXCOORD2FV glMultiTexCoord2fv;
  #endif

  #ifndef GLMULTITEXCOORD2I_DECL
  #define GLMULTITEXCOORD2I_DECL
  csGLMULTITEXCOORD2I glMultiTexCoord2i;
  #endif

  #ifndef GLMULTITEXCOORD2IV_DECL
  #define GLMULTITEXCOORD2IV_DECL
  csGLMULTITEXCOORD2IV glMultiTexCoord2iv;
  #endif

  #ifndef GLMULTITEXCOORD2S_DECL
  #define GLMULTITEXCOORD2S_DECL
  csGLMULTITEXCOORD2S glMultiTexCoord2s;
  #endif

  #ifndef GLMULTITEXCOORD2SV_DECL
  #define GLMULTITEXCOORD2SV_DECL
  csGLMULTITEXCOORD2SV glMultiTexCoord2sv;
  #endif

  #ifndef GLMULTITEXCOORD3D_DECL
  #define GLMULTITEXCOORD3D_DECL
  csGLMULTITEXCOORD3D glMultiTexCoord3d;
  #endif

  #ifndef GLMULTITEXCOORD3DV_DECL
  #define GLMULTITEXCOORD3DV_DECL
  csGLMULTITEXCOORD3DV glMultiTexCoord3dv;
  #endif

  #ifndef GLMULTITEXCOORD3F_DECL
  #define GLMULTITEXCOORD3F_DECL
  csGLMULTITEXCOORD3F glMultiTexCoord3f;
  #endif

  #ifndef GLMULTITEXCOORD3FV_DECL
  #define GLMULTITEXCOORD3FV_DECL
  csGLMULTITEXCOORD3FV glMultiTexCoord3fv;
  #endif

  #ifndef GLMULTITEXCOORD3I_DECL
  #define GLMULTITEXCOORD3I_DECL
  csGLMULTITEXCOORD3I glMultiTexCoord3i;
  #endif

  #ifndef GLMULTITEXCOORD3IV_DECL
  #define GLMULTITEXCOORD3IV_DECL
  csGLMULTITEXCOORD3IV glMultiTexCoord3iv;
  #endif

  #ifndef GLMULTITEXCOORD3S_DECL
  #define GLMULTITEXCOORD3S_DECL
  csGLMULTITEXCOORD3S glMultiTexCoord3s;
  #endif

  #ifndef GLMULTITEXCOORD3SV_DECL
  #define GLMULTITEXCOORD3SV_DECL
  csGLMULTITEXCOORD3SV glMultiTexCoord3sv;
  #endif

  #ifndef GLMULTITEXCOORD4D_DECL
  #define GLMULTITEXCOORD4D_DECL
  csGLMULTITEXCOORD4D glMultiTexCoord4d;
  #endif

  #ifndef GLMULTITEXCOORD4DV_DECL
  #define GLMULTITEXCOORD4DV_DECL
  csGLMULTITEXCOORD4DV glMultiTexCoord4dv;
  #endif

  #ifndef GLMULTITEXCOORD4F_DECL
  #define GLMULTITEXCOORD4F_DECL
  csGLMULTITEXCOORD4F glMultiTexCoord4f;
  #endif

  #ifndef GLMULTITEXCOORD4FV_DECL
  #define GLMULTITEXCOORD4FV_DECL
  csGLMULTITEXCOORD4FV glMultiTexCoord4fv;
  #endif

  #ifndef GLMULTITEXCOORD4I_DECL
  #define GLMULTITEXCOORD4I_DECL
  csGLMULTITEXCOORD4I glMultiTexCoord4i;
  #endif

  #ifndef GLMULTITEXCOORD4IV_DECL
  #define GLMULTITEXCOORD4IV_DECL
  csGLMULTITEXCOORD4IV glMultiTexCoord4iv;
  #endif

  #ifndef GLMULTITEXCOORD4S_DECL
  #define GLMULTITEXCOORD4S_DECL
  csGLMULTITEXCOORD4S glMultiTexCoord4s;
  #endif

  #ifndef GLMULTITEXCOORD4SV_DECL
  #define GLMULTITEXCOORD4SV_DECL
  csGLMULTITEXCOORD4SV glMultiTexCoord4sv;
  #endif

  #ifndef GLLOADTRANSPOSEMATRIXF_DECL
  #define GLLOADTRANSPOSEMATRIXF_DECL
  csGLLOADTRANSPOSEMATRIXF glLoadTransposeMatrixf;
  #endif

  #ifndef GLLOADTRANSPOSEMATRIXD_DECL
  #define GLLOADTRANSPOSEMATRIXD_DECL
  csGLLOADTRANSPOSEMATRIXD glLoadTransposeMatrixd;
  #endif

  #ifndef GLMULTTRANSPOSEMATRIXF_DECL
  #define GLMULTTRANSPOSEMATRIXF_DECL
  csGLMULTTRANSPOSEMATRIXF glMultTransposeMatrixf;
  #endif

  #ifndef GLMULTTRANSPOSEMATRIXD_DECL
  #define GLMULTTRANSPOSEMATRIXD_DECL
  csGLMULTTRANSPOSEMATRIXD glMultTransposeMatrixd;
  #endif

  #ifndef GLSAMPLECOVERAGE_DECL
  #define GLSAMPLECOVERAGE_DECL
  csGLSAMPLECOVERAGE glSampleCoverage;
  #endif

  #ifndef GLCOMPRESSEDTEXIMAGE3D_DECL
  #define GLCOMPRESSEDTEXIMAGE3D_DECL
  csGLCOMPRESSEDTEXIMAGE3D glCompressedTexImage3D;
  #endif

  #ifndef GLCOMPRESSEDTEXIMAGE2D_DECL
  #define GLCOMPRESSEDTEXIMAGE2D_DECL
  csGLCOMPRESSEDTEXIMAGE2D glCompressedTexImage2D;
  #endif

  #ifndef GLCOMPRESSEDTEXIMAGE1D_DECL
  #define GLCOMPRESSEDTEXIMAGE1D_DECL
  csGLCOMPRESSEDTEXIMAGE1D glCompressedTexImage1D;
  #endif

  #ifndef GLCOMPRESSEDTEXSUBIMAGE3D_DECL
  #define GLCOMPRESSEDTEXSUBIMAGE3D_DECL
  csGLCOMPRESSEDTEXSUBIMAGE3D glCompressedTexSubImage3D;
  #endif

  #ifndef GLCOMPRESSEDTEXSUBIMAGE2D_DECL
  #define GLCOMPRESSEDTEXSUBIMAGE2D_DECL
  csGLCOMPRESSEDTEXSUBIMAGE2D glCompressedTexSubImage2D;
  #endif

  #ifndef GLCOMPRESSEDTEXSUBIMAGE1D_DECL
  #define GLCOMPRESSEDTEXSUBIMAGE1D_DECL
  csGLCOMPRESSEDTEXSUBIMAGE1D glCompressedTexSubImage1D;
  #endif

  #ifndef GLGETCOMPRESSEDTEXIMAGE_DECL
  #define GLGETCOMPRESSEDTEXIMAGE_DECL
  csGLGETCOMPRESSEDTEXIMAGE glGetCompressedTexImage;
  #endif


  // GL_ARB_multitexture
  #ifndef GLACTIVETEXTUREARB_DECL
  #define GLACTIVETEXTUREARB_DECL
  csGLACTIVETEXTUREARB glActiveTextureARB;
  #endif

  #ifndef GLCLIENTACTIVETEXTUREARB_DECL
  #define GLCLIENTACTIVETEXTUREARB_DECL
  csGLCLIENTACTIVETEXTUREARB glClientActiveTextureARB;
  #endif

  #ifndef GLMULTITEXCOORD1DARB_DECL
  #define GLMULTITEXCOORD1DARB_DECL
  csGLMULTITEXCOORD1DARB glMultiTexCoord1dARB;
  #endif

  #ifndef GLMULTITEXCOORD1DVARB_DECL
  #define GLMULTITEXCOORD1DVARB_DECL
  csGLMULTITEXCOORD1DVARB glMultiTexCoord1dvARB;
  #endif

  #ifndef GLMULTITEXCOORD1FARB_DECL
  #define GLMULTITEXCOORD1FARB_DECL
  csGLMULTITEXCOORD1FARB glMultiTexCoord1fARB;
  #endif

  #ifndef GLMULTITEXCOORD1FVARB_DECL
  #define GLMULTITEXCOORD1FVARB_DECL
  csGLMULTITEXCOORD1FVARB glMultiTexCoord1fvARB;
  #endif

  #ifndef GLMULTITEXCOORD1IARB_DECL
  #define GLMULTITEXCOORD1IARB_DECL
  csGLMULTITEXCOORD1IARB glMultiTexCoord1iARB;
  #endif

  #ifndef GLMULTITEXCOORD1IVARB_DECL
  #define GLMULTITEXCOORD1IVARB_DECL
  csGLMULTITEXCOORD1IVARB glMultiTexCoord1ivARB;
  #endif

  #ifndef GLMULTITEXCOORD1SARB_DECL
  #define GLMULTITEXCOORD1SARB_DECL
  csGLMULTITEXCOORD1SARB glMultiTexCoord1sARB;
  #endif

  #ifndef GLMULTITEXCOORD1SVARB_DECL
  #define GLMULTITEXCOORD1SVARB_DECL
  csGLMULTITEXCOORD1SVARB glMultiTexCoord1svARB;
  #endif

  #ifndef GLMULTITEXCOORD2DARB_DECL
  #define GLMULTITEXCOORD2DARB_DECL
  csGLMULTITEXCOORD2DARB glMultiTexCoord2dARB;
  #endif

  #ifndef GLMULTITEXCOORD2DVARB_DECL
  #define GLMULTITEXCOORD2DVARB_DECL
  csGLMULTITEXCOORD2DVARB glMultiTexCoord2dvARB;
  #endif

  #ifndef GLMULTITEXCOORD2FARB_DECL
  #define GLMULTITEXCOORD2FARB_DECL
  csGLMULTITEXCOORD2FARB glMultiTexCoord2fARB;
  #endif

  #ifndef GLMULTITEXCOORD2FVARB_DECL
  #define GLMULTITEXCOORD2FVARB_DECL
  csGLMULTITEXCOORD2FVARB glMultiTexCoord2fvARB;
  #endif

  #ifndef GLMULTITEXCOORD2IARB_DECL
  #define GLMULTITEXCOORD2IARB_DECL
  csGLMULTITEXCOORD2IARB glMultiTexCoord2iARB;
  #endif

  #ifndef GLMULTITEXCOORD2IVARB_DECL
  #define GLMULTITEXCOORD2IVARB_DECL
  csGLMULTITEXCOORD2IVARB glMultiTexCoord2ivARB;
  #endif

  #ifndef GLMULTITEXCOORD2SARB_DECL
  #define GLMULTITEXCOORD2SARB_DECL
  csGLMULTITEXCOORD2SARB glMultiTexCoord2sARB;
  #endif

  #ifndef GLMULTITEXCOORD2SVARB_DECL
  #define GLMULTITEXCOORD2SVARB_DECL
  csGLMULTITEXCOORD2SVARB glMultiTexCoord2svARB;
  #endif

  #ifndef GLMULTITEXCOORD3DARB_DECL
  #define GLMULTITEXCOORD3DARB_DECL
  csGLMULTITEXCOORD3DARB glMultiTexCoord3dARB;
  #endif

  #ifndef GLMULTITEXCOORD3DVARB_DECL
  #define GLMULTITEXCOORD3DVARB_DECL
  csGLMULTITEXCOORD3DVARB glMultiTexCoord3dvARB;
  #endif

  #ifndef GLMULTITEXCOORD3FARB_DECL
  #define GLMULTITEXCOORD3FARB_DECL
  csGLMULTITEXCOORD3FARB glMultiTexCoord3fARB;
  #endif

  #ifndef GLMULTITEXCOORD3FVARB_DECL
  #define GLMULTITEXCOORD3FVARB_DECL
  csGLMULTITEXCOORD3FVARB glMultiTexCoord3fvARB;
  #endif

  #ifndef GLMULTITEXCOORD3IARB_DECL
  #define GLMULTITEXCOORD3IARB_DECL
  csGLMULTITEXCOORD3IARB glMultiTexCoord3iARB;
  #endif

  #ifndef GLMULTITEXCOORD3IVARB_DECL
  #define GLMULTITEXCOORD3IVARB_DECL
  csGLMULTITEXCOORD3IVARB glMultiTexCoord3ivARB;
  #endif

  #ifndef GLMULTITEXCOORD3SARB_DECL
  #define GLMULTITEXCOORD3SARB_DECL
  csGLMULTITEXCOORD3SARB glMultiTexCoord3sARB;
  #endif

  #ifndef GLMULTITEXCOORD3SVARB_DECL
  #define GLMULTITEXCOORD3SVARB_DECL
  csGLMULTITEXCOORD3SVARB glMultiTexCoord3svARB;
  #endif

  #ifndef GLMULTITEXCOORD4DARB_DECL
  #define GLMULTITEXCOORD4DARB_DECL
  csGLMULTITEXCOORD4DARB glMultiTexCoord4dARB;
  #endif

  #ifndef GLMULTITEXCOORD4DVARB_DECL
  #define GLMULTITEXCOORD4DVARB_DECL
  csGLMULTITEXCOORD4DVARB glMultiTexCoord4dvARB;
  #endif

  #ifndef GLMULTITEXCOORD4FARB_DECL
  #define GLMULTITEXCOORD4FARB_DECL
  csGLMULTITEXCOORD4FARB glMultiTexCoord4fARB;
  #endif

  #ifndef GLMULTITEXCOORD4FVARB_DECL
  #define GLMULTITEXCOORD4FVARB_DECL
  csGLMULTITEXCOORD4FVARB glMultiTexCoord4fvARB;
  #endif

  #ifndef GLMULTITEXCOORD4IARB_DECL
  #define GLMULTITEXCOORD4IARB_DECL
  csGLMULTITEXCOORD4IARB glMultiTexCoord4iARB;
  #endif

  #ifndef GLMULTITEXCOORD4IVARB_DECL
  #define GLMULTITEXCOORD4IVARB_DECL
  csGLMULTITEXCOORD4IVARB glMultiTexCoord4ivARB;
  #endif

  #ifndef GLMULTITEXCOORD4SARB_DECL
  #define GLMULTITEXCOORD4SARB_DECL
  csGLMULTITEXCOORD4SARB glMultiTexCoord4sARB;
  #endif

  #ifndef GLMULTITEXCOORD4SVARB_DECL
  #define GLMULTITEXCOORD4SVARB_DECL
  csGLMULTITEXCOORD4SVARB glMultiTexCoord4svARB;
  #endif


  // GL_ARB_transpose_matrix
  #ifndef GLLOADTRANSPOSEMATRIXFARB_DECL
  #define GLLOADTRANSPOSEMATRIXFARB_DECL
  csGLLOADTRANSPOSEMATRIXFARB glLoadTransposeMatrixfARB;
  #endif

  #ifndef GLLOADTRANSPOSEMATRIXDARB_DECL
  #define GLLOADTRANSPOSEMATRIXDARB_DECL
  csGLLOADTRANSPOSEMATRIXDARB glLoadTransposeMatrixdARB;
  #endif

  #ifndef GLMULTTRANSPOSEMATRIXFARB_DECL
  #define GLMULTTRANSPOSEMATRIXFARB_DECL
  csGLMULTTRANSPOSEMATRIXFARB glMultTransposeMatrixfARB;
  #endif

  #ifndef GLMULTTRANSPOSEMATRIXDARB_DECL
  #define GLMULTTRANSPOSEMATRIXDARB_DECL
  csGLMULTTRANSPOSEMATRIXDARB glMultTransposeMatrixdARB;
  #endif


  // GL_ARB_multisample
  #ifndef GLSAMPLECOVERAGEARB_DECL
  #define GLSAMPLECOVERAGEARB_DECL
  csGLSAMPLECOVERAGEARB glSampleCoverageARB;
  #endif


  // GL_ARB_texture_env_add

  // WGL_ARB_extensions_string
  #ifndef WGLGETEXTENSIONSSTRINGARB_DECL
  #define WGLGETEXTENSIONSSTRINGARB_DECL
  csWGLGETEXTENSIONSSTRINGARB wglGetExtensionsStringARB;
  #endif


  // WGL_ARB_buffer_region
  #ifndef WGLCREATEBUFFERREGIONARB_DECL
  #define WGLCREATEBUFFERREGIONARB_DECL
  csWGLCREATEBUFFERREGIONARB wglCreateBufferRegionARB;
  #endif

  #ifndef WGLDELETEBUFFERREGIONARB_DECL
  #define WGLDELETEBUFFERREGIONARB_DECL
  csWGLDELETEBUFFERREGIONARB wglDeleteBufferRegionARB;
  #endif

  #ifndef WGLSAVEBUFFERREGIONARB_DECL
  #define WGLSAVEBUFFERREGIONARB_DECL
  csWGLSAVEBUFFERREGIONARB wglSaveBufferRegionARB;
  #endif

  #ifndef WGLRESTOREBUFFERREGIONARB_DECL
  #define WGLRESTOREBUFFERREGIONARB_DECL
  csWGLRESTOREBUFFERREGIONARB wglRestoreBufferRegionARB;
  #endif


  // GL_ARB_texture_cube_map

  // GL_ARB_depth_texture

  // GL_ARB_point_parameters
  #ifndef GLPOINTPARAMETERFARB_DECL
  #define GLPOINTPARAMETERFARB_DECL
  csGLPOINTPARAMETERFARB glPointParameterfARB;
  #endif

  #ifndef GLPOINTPARAMETERFVARB_DECL
  #define GLPOINTPARAMETERFVARB_DECL
  csGLPOINTPARAMETERFVARB glPointParameterfvARB;
  #endif


  // GL_ARB_shadow

  // GL_ARB_shadow_ambient

  // GL_ARB_texture_border_clamp

  // GL_ARB_texture_compression
  #ifndef GLCOMPRESSEDTEXIMAGE3DARB_DECL
  #define GLCOMPRESSEDTEXIMAGE3DARB_DECL
  csGLCOMPRESSEDTEXIMAGE3DARB glCompressedTexImage3DARB;
  #endif

  #ifndef GLCOMPRESSEDTEXIMAGE2DARB_DECL
  #define GLCOMPRESSEDTEXIMAGE2DARB_DECL
  csGLCOMPRESSEDTEXIMAGE2DARB glCompressedTexImage2DARB;
  #endif

  #ifndef GLCOMPRESSEDTEXIMAGE1DARB_DECL
  #define GLCOMPRESSEDTEXIMAGE1DARB_DECL
  csGLCOMPRESSEDTEXIMAGE1DARB glCompressedTexImage1DARB;
  #endif

  #ifndef GLCOMPRESSEDTEXSUBIMAGE3DARB_DECL
  #define GLCOMPRESSEDTEXSUBIMAGE3DARB_DECL
  csGLCOMPRESSEDTEXSUBIMAGE3DARB glCompressedTexSubImage3DARB;
  #endif

  #ifndef GLCOMPRESSEDTEXSUBIMAGE2DARB_DECL
  #define GLCOMPRESSEDTEXSUBIMAGE2DARB_DECL
  csGLCOMPRESSEDTEXSUBIMAGE2DARB glCompressedTexSubImage2DARB;
  #endif

  #ifndef GLCOMPRESSEDTEXSUBIMAGE1DARB_DECL
  #define GLCOMPRESSEDTEXSUBIMAGE1DARB_DECL
  csGLCOMPRESSEDTEXSUBIMAGE1DARB glCompressedTexSubImage1DARB;
  #endif

  #ifndef GLGETCOMPRESSEDTEXIMAGEARB_DECL
  #define GLGETCOMPRESSEDTEXIMAGEARB_DECL
  csGLGETCOMPRESSEDTEXIMAGEARB glGetCompressedTexImageARB;
  #endif


  // GL_ARB_texture_env_combine

  // GL_ARB_texture_env_crossbar

  // GL_ARB_texture_env_dot3

  // GL_ARB_texture_mirrored_repeat

  // GL_ARB_vertex_blend
  #ifndef GLWEIGHTBVARB_DECL
  #define GLWEIGHTBVARB_DECL
  csGLWEIGHTBVARB glWeightbvARB;
  #endif

  #ifndef GLWEIGHTSVARB_DECL
  #define GLWEIGHTSVARB_DECL
  csGLWEIGHTSVARB glWeightsvARB;
  #endif

  #ifndef GLWEIGHTIVARB_DECL
  #define GLWEIGHTIVARB_DECL
  csGLWEIGHTIVARB glWeightivARB;
  #endif

  #ifndef GLWEIGHTFVARB_DECL
  #define GLWEIGHTFVARB_DECL
  csGLWEIGHTFVARB glWeightfvARB;
  #endif

  #ifndef GLWEIGHTDVARB_DECL
  #define GLWEIGHTDVARB_DECL
  csGLWEIGHTDVARB glWeightdvARB;
  #endif

  #ifndef GLWEIGHTVARB_DECL
  #define GLWEIGHTVARB_DECL
  csGLWEIGHTVARB glWeightvARB;
  #endif

  #ifndef GLWEIGHTUBVARB_DECL
  #define GLWEIGHTUBVARB_DECL
  csGLWEIGHTUBVARB glWeightubvARB;
  #endif

  #ifndef GLWEIGHTUSVARB_DECL
  #define GLWEIGHTUSVARB_DECL
  csGLWEIGHTUSVARB glWeightusvARB;
  #endif

  #ifndef GLWEIGHTUIVARB_DECL
  #define GLWEIGHTUIVARB_DECL
  csGLWEIGHTUIVARB glWeightuivARB;
  #endif

  #ifndef GLWEIGHTPOINTERARB_DECL
  #define GLWEIGHTPOINTERARB_DECL
  csGLWEIGHTPOINTERARB glWeightPointerARB;
  #endif

  #ifndef GLVERTEXBLENDARB_DECL
  #define GLVERTEXBLENDARB_DECL
  csGLVERTEXBLENDARB glVertexBlendARB;
  #endif


  // GL_ARB_vertex_program
  #ifndef GLVERTEXATTRIB1SARB_DECL
  #define GLVERTEXATTRIB1SARB_DECL
  csGLVERTEXATTRIB1SARB glVertexAttrib1sARB;
  #endif

  #ifndef GLVERTEXATTRIB1FARB_DECL
  #define GLVERTEXATTRIB1FARB_DECL
  csGLVERTEXATTRIB1FARB glVertexAttrib1fARB;
  #endif

  #ifndef GLVERTEXATTRIB1DARB_DECL
  #define GLVERTEXATTRIB1DARB_DECL
  csGLVERTEXATTRIB1DARB glVertexAttrib1dARB;
  #endif

  #ifndef GLVERTEXATTRIB2SARB_DECL
  #define GLVERTEXATTRIB2SARB_DECL
  csGLVERTEXATTRIB2SARB glVertexAttrib2sARB;
  #endif

  #ifndef GLVERTEXATTRIB2FARB_DECL
  #define GLVERTEXATTRIB2FARB_DECL
  csGLVERTEXATTRIB2FARB glVertexAttrib2fARB;
  #endif

  #ifndef GLVERTEXATTRIB2DARB_DECL
  #define GLVERTEXATTRIB2DARB_DECL
  csGLVERTEXATTRIB2DARB glVertexAttrib2dARB;
  #endif

  #ifndef GLVERTEXATTRIB3SARB_DECL
  #define GLVERTEXATTRIB3SARB_DECL
  csGLVERTEXATTRIB3SARB glVertexAttrib3sARB;
  #endif

  #ifndef GLVERTEXATTRIB3FARB_DECL
  #define GLVERTEXATTRIB3FARB_DECL
  csGLVERTEXATTRIB3FARB glVertexAttrib3fARB;
  #endif

  #ifndef GLVERTEXATTRIB3DARB_DECL
  #define GLVERTEXATTRIB3DARB_DECL
  csGLVERTEXATTRIB3DARB glVertexAttrib3dARB;
  #endif

  #ifndef GLVERTEXATTRIB4SARB_DECL
  #define GLVERTEXATTRIB4SARB_DECL
  csGLVERTEXATTRIB4SARB glVertexAttrib4sARB;
  #endif

  #ifndef GLVERTEXATTRIB4FARB_DECL
  #define GLVERTEXATTRIB4FARB_DECL
  csGLVERTEXATTRIB4FARB glVertexAttrib4fARB;
  #endif

  #ifndef GLVERTEXATTRIB4DARB_DECL
  #define GLVERTEXATTRIB4DARB_DECL
  csGLVERTEXATTRIB4DARB glVertexAttrib4dARB;
  #endif

  #ifndef GLVERTEXATTRIB4NUBARB_DECL
  #define GLVERTEXATTRIB4NUBARB_DECL
  csGLVERTEXATTRIB4NUBARB glVertexAttrib4NubARB;
  #endif

  #ifndef GLVERTEXATTRIB1SVARB_DECL
  #define GLVERTEXATTRIB1SVARB_DECL
  csGLVERTEXATTRIB1SVARB glVertexAttrib1svARB;
  #endif

  #ifndef GLVERTEXATTRIB1FVARB_DECL
  #define GLVERTEXATTRIB1FVARB_DECL
  csGLVERTEXATTRIB1FVARB glVertexAttrib1fvARB;
  #endif

  #ifndef GLVERTEXATTRIB1DVARB_DECL
  #define GLVERTEXATTRIB1DVARB_DECL
  csGLVERTEXATTRIB1DVARB glVertexAttrib1dvARB;
  #endif

  #ifndef GLVERTEXATTRIB2SVARB_DECL
  #define GLVERTEXATTRIB2SVARB_DECL
  csGLVERTEXATTRIB2SVARB glVertexAttrib2svARB;
  #endif

  #ifndef GLVERTEXATTRIB2FVARB_DECL
  #define GLVERTEXATTRIB2FVARB_DECL
  csGLVERTEXATTRIB2FVARB glVertexAttrib2fvARB;
  #endif

  #ifndef GLVERTEXATTRIB2DVARB_DECL
  #define GLVERTEXATTRIB2DVARB_DECL
  csGLVERTEXATTRIB2DVARB glVertexAttrib2dvARB;
  #endif

  #ifndef GLVERTEXATTRIB3SVARB_DECL
  #define GLVERTEXATTRIB3SVARB_DECL
  csGLVERTEXATTRIB3SVARB glVertexAttrib3svARB;
  #endif

  #ifndef GLVERTEXATTRIB3FVARB_DECL
  #define GLVERTEXATTRIB3FVARB_DECL
  csGLVERTEXATTRIB3FVARB glVertexAttrib3fvARB;
  #endif

  #ifndef GLVERTEXATTRIB3DVARB_DECL
  #define GLVERTEXATTRIB3DVARB_DECL
  csGLVERTEXATTRIB3DVARB glVertexAttrib3dvARB;
  #endif

  #ifndef GLVERTEXATTRIB4BVARB_DECL
  #define GLVERTEXATTRIB4BVARB_DECL
  csGLVERTEXATTRIB4BVARB glVertexAttrib4bvARB;
  #endif

  #ifndef GLVERTEXATTRIB4SVARB_DECL
  #define GLVERTEXATTRIB4SVARB_DECL
  csGLVERTEXATTRIB4SVARB glVertexAttrib4svARB;
  #endif

  #ifndef GLVERTEXATTRIB4IVARB_DECL
  #define GLVERTEXATTRIB4IVARB_DECL
  csGLVERTEXATTRIB4IVARB glVertexAttrib4ivARB;
  #endif

  #ifndef GLVERTEXATTRIB4UBVARB_DECL
  #define GLVERTEXATTRIB4UBVARB_DECL
  csGLVERTEXATTRIB4UBVARB glVertexAttrib4ubvARB;
  #endif

  #ifndef GLVERTEXATTRIB4USVARB_DECL
  #define GLVERTEXATTRIB4USVARB_DECL
  csGLVERTEXATTRIB4USVARB glVertexAttrib4usvARB;
  #endif

  #ifndef GLVERTEXATTRIB4UIVARB_DECL
  #define GLVERTEXATTRIB4UIVARB_DECL
  csGLVERTEXATTRIB4UIVARB glVertexAttrib4uivARB;
  #endif

  #ifndef GLVERTEXATTRIB4FVARB_DECL
  #define GLVERTEXATTRIB4FVARB_DECL
  csGLVERTEXATTRIB4FVARB glVertexAttrib4fvARB;
  #endif

  #ifndef GLVERTEXATTRIB4DVARB_DECL
  #define GLVERTEXATTRIB4DVARB_DECL
  csGLVERTEXATTRIB4DVARB glVertexAttrib4dvARB;
  #endif

  #ifndef GLVERTEXATTRIB4NBVARB_DECL
  #define GLVERTEXATTRIB4NBVARB_DECL
  csGLVERTEXATTRIB4NBVARB glVertexAttrib4NbvARB;
  #endif

  #ifndef GLVERTEXATTRIB4NSVARB_DECL
  #define GLVERTEXATTRIB4NSVARB_DECL
  csGLVERTEXATTRIB4NSVARB glVertexAttrib4NsvARB;
  #endif

  #ifndef GLVERTEXATTRIB4NIVARB_DECL
  #define GLVERTEXATTRIB4NIVARB_DECL
  csGLVERTEXATTRIB4NIVARB glVertexAttrib4NivARB;
  #endif

  #ifndef GLVERTEXATTRIB4NUBVARB_DECL
  #define GLVERTEXATTRIB4NUBVARB_DECL
  csGLVERTEXATTRIB4NUBVARB glVertexAttrib4NubvARB;
  #endif

  #ifndef GLVERTEXATTRIB4NUSVARB_DECL
  #define GLVERTEXATTRIB4NUSVARB_DECL
  csGLVERTEXATTRIB4NUSVARB glVertexAttrib4NusvARB;
  #endif

  #ifndef GLVERTEXATTRIB4NUIVARB_DECL
  #define GLVERTEXATTRIB4NUIVARB_DECL
  csGLVERTEXATTRIB4NUIVARB glVertexAttrib4NuivARB;
  #endif

  #ifndef GLVERTEXATTRIBPOINTERARB_DECL
  #define GLVERTEXATTRIBPOINTERARB_DECL
  csGLVERTEXATTRIBPOINTERARB glVertexAttribPointerARB;
  #endif

  #ifndef GLENABLEVERTEXATTRIBARRAYARB_DECL
  #define GLENABLEVERTEXATTRIBARRAYARB_DECL
  csGLENABLEVERTEXATTRIBARRAYARB glEnableVertexAttribArrayARB;
  #endif

  #ifndef GLDISABLEVERTEXATTRIBARRAYARB_DECL
  #define GLDISABLEVERTEXATTRIBARRAYARB_DECL
  csGLDISABLEVERTEXATTRIBARRAYARB glDisableVertexAttribArrayARB;
  #endif

  #ifndef GLPROGRAMSTRINGARB_DECL
  #define GLPROGRAMSTRINGARB_DECL
  csGLPROGRAMSTRINGARB glProgramStringARB;
  #endif

  #ifndef GLBINDPROGRAMARB_DECL
  #define GLBINDPROGRAMARB_DECL
  csGLBINDPROGRAMARB glBindProgramARB;
  #endif

  #ifndef GLDELETEPROGRAMSARB_DECL
  #define GLDELETEPROGRAMSARB_DECL
  csGLDELETEPROGRAMSARB glDeleteProgramsARB;
  #endif

  #ifndef GLGENPROGRAMSARB_DECL
  #define GLGENPROGRAMSARB_DECL
  csGLGENPROGRAMSARB glGenProgramsARB;
  #endif

  #ifndef GLPROGRAMENVPARAMETER4DARB_DECL
  #define GLPROGRAMENVPARAMETER4DARB_DECL
  csGLPROGRAMENVPARAMETER4DARB glProgramEnvParameter4dARB;
  #endif

  #ifndef GLPROGRAMENVPARAMETER4DVARB_DECL
  #define GLPROGRAMENVPARAMETER4DVARB_DECL
  csGLPROGRAMENVPARAMETER4DVARB glProgramEnvParameter4dvARB;
  #endif

  #ifndef GLPROGRAMENVPARAMETER4FARB_DECL
  #define GLPROGRAMENVPARAMETER4FARB_DECL
  csGLPROGRAMENVPARAMETER4FARB glProgramEnvParameter4fARB;
  #endif

  #ifndef GLPROGRAMENVPARAMETER4FVARB_DECL
  #define GLPROGRAMENVPARAMETER4FVARB_DECL
  csGLPROGRAMENVPARAMETER4FVARB glProgramEnvParameter4fvARB;
  #endif

  #ifndef GLPROGRAMLOCALPARAMETER4DARB_DECL
  #define GLPROGRAMLOCALPARAMETER4DARB_DECL
  csGLPROGRAMLOCALPARAMETER4DARB glProgramLocalParameter4dARB;
  #endif

  #ifndef GLPROGRAMLOCALPARAMETER4DVARB_DECL
  #define GLPROGRAMLOCALPARAMETER4DVARB_DECL
  csGLPROGRAMLOCALPARAMETER4DVARB glProgramLocalParameter4dvARB;
  #endif

  #ifndef GLPROGRAMLOCALPARAMETER4FARB_DECL
  #define GLPROGRAMLOCALPARAMETER4FARB_DECL
  csGLPROGRAMLOCALPARAMETER4FARB glProgramLocalParameter4fARB;
  #endif

  #ifndef GLPROGRAMLOCALPARAMETER4FVARB_DECL
  #define GLPROGRAMLOCALPARAMETER4FVARB_DECL
  csGLPROGRAMLOCALPARAMETER4FVARB glProgramLocalParameter4fvARB;
  #endif

  #ifndef GLGETPROGRAMENVPARAMETERDVARB_DECL
  #define GLGETPROGRAMENVPARAMETERDVARB_DECL
  csGLGETPROGRAMENVPARAMETERDVARB glGetProgramEnvParameterdvARB;
  #endif

  #ifndef GLGETPROGRAMENVPARAMETERFVARB_DECL
  #define GLGETPROGRAMENVPARAMETERFVARB_DECL
  csGLGETPROGRAMENVPARAMETERFVARB glGetProgramEnvParameterfvARB;
  #endif

  #ifndef GLGETPROGRAMLOCALPARAMETERDVARB_DECL
  #define GLGETPROGRAMLOCALPARAMETERDVARB_DECL
  csGLGETPROGRAMLOCALPARAMETERDVARB glGetProgramLocalParameterdvARB;
  #endif

  #ifndef GLGETPROGRAMLOCALPARAMETERFVARB_DECL
  #define GLGETPROGRAMLOCALPARAMETERFVARB_DECL
  csGLGETPROGRAMLOCALPARAMETERFVARB glGetProgramLocalParameterfvARB;
  #endif

  #ifndef GLGETPROGRAMIVARB_DECL
  #define GLGETPROGRAMIVARB_DECL
  csGLGETPROGRAMIVARB glGetProgramivARB;
  #endif

  #ifndef GLGETPROGRAMSTRINGARB_DECL
  #define GLGETPROGRAMSTRINGARB_DECL
  csGLGETPROGRAMSTRINGARB glGetProgramStringARB;
  #endif

  #ifndef GLGETVERTEXATTRIBDVARB_DECL
  #define GLGETVERTEXATTRIBDVARB_DECL
  csGLGETVERTEXATTRIBDVARB glGetVertexAttribdvARB;
  #endif

  #ifndef GLGETVERTEXATTRIBFVARB_DECL
  #define GLGETVERTEXATTRIBFVARB_DECL
  csGLGETVERTEXATTRIBFVARB glGetVertexAttribfvARB;
  #endif

  #ifndef GLGETVERTEXATTRIBIVARB_DECL
  #define GLGETVERTEXATTRIBIVARB_DECL
  csGLGETVERTEXATTRIBIVARB glGetVertexAttribivARB;
  #endif

  #ifndef GLGETVERTEXATTRIBPOINTERVARB_DECL
  #define GLGETVERTEXATTRIBPOINTERVARB_DECL
  csGLGETVERTEXATTRIBPOINTERVARB glGetVertexAttribPointervARB;
  #endif

  #ifndef GLISPROGRAMARB_DECL
  #define GLISPROGRAMARB_DECL
  csGLISPROGRAMARB glIsProgramARB;
  #endif


  // GL_ARB_window_pos
  #ifndef GLWINDOWPOS2DARB_DECL
  #define GLWINDOWPOS2DARB_DECL
  csGLWINDOWPOS2DARB glWindowPos2dARB;
  #endif

  #ifndef GLWINDOWPOS2FARB_DECL
  #define GLWINDOWPOS2FARB_DECL
  csGLWINDOWPOS2FARB glWindowPos2fARB;
  #endif

  #ifndef GLWINDOWPOS2IARB_DECL
  #define GLWINDOWPOS2IARB_DECL
  csGLWINDOWPOS2IARB glWindowPos2iARB;
  #endif

  #ifndef GLWINDOWPOS2SARB_DECL
  #define GLWINDOWPOS2SARB_DECL
  csGLWINDOWPOS2SARB glWindowPos2sARB;
  #endif

  #ifndef GLWINDOWPOS2DVARB_DECL
  #define GLWINDOWPOS2DVARB_DECL
  csGLWINDOWPOS2DVARB glWindowPos2dvARB;
  #endif

  #ifndef GLWINDOWPOS2FVARB_DECL
  #define GLWINDOWPOS2FVARB_DECL
  csGLWINDOWPOS2FVARB glWindowPos2fvARB;
  #endif

  #ifndef GLWINDOWPOS2IVARB_DECL
  #define GLWINDOWPOS2IVARB_DECL
  csGLWINDOWPOS2IVARB glWindowPos2ivARB;
  #endif

  #ifndef GLWINDOWPOS2SVARB_DECL
  #define GLWINDOWPOS2SVARB_DECL
  csGLWINDOWPOS2SVARB glWindowPos2svARB;
  #endif

  #ifndef GLWINDOWPOS3DARB_DECL
  #define GLWINDOWPOS3DARB_DECL
  csGLWINDOWPOS3DARB glWindowPos3dARB;
  #endif

  #ifndef GLWINDOWPOS3FARB_DECL
  #define GLWINDOWPOS3FARB_DECL
  csGLWINDOWPOS3FARB glWindowPos3fARB;
  #endif

  #ifndef GLWINDOWPOS3IARB_DECL
  #define GLWINDOWPOS3IARB_DECL
  csGLWINDOWPOS3IARB glWindowPos3iARB;
  #endif

  #ifndef GLWINDOWPOS3SARB_DECL
  #define GLWINDOWPOS3SARB_DECL
  csGLWINDOWPOS3SARB glWindowPos3sARB;
  #endif

  #ifndef GLWINDOWPOS3DVARB_DECL
  #define GLWINDOWPOS3DVARB_DECL
  csGLWINDOWPOS3DVARB glWindowPos3dvARB;
  #endif

  #ifndef GLWINDOWPOS3FVARB_DECL
  #define GLWINDOWPOS3FVARB_DECL
  csGLWINDOWPOS3FVARB glWindowPos3fvARB;
  #endif

  #ifndef GLWINDOWPOS3IVARB_DECL
  #define GLWINDOWPOS3IVARB_DECL
  csGLWINDOWPOS3IVARB glWindowPos3ivARB;
  #endif

  #ifndef GLWINDOWPOS3SVARB_DECL
  #define GLWINDOWPOS3SVARB_DECL
  csGLWINDOWPOS3SVARB glWindowPos3svARB;
  #endif


  // GL_EXT_422_pixels

  // GL_EXT_abgr

  // GL_EXT_bgra

  // GL_EXT_blend_color
  #ifndef GLBLENDCOLOREXT_DECL
  #define GLBLENDCOLOREXT_DECL
  csGLBLENDCOLOREXT glBlendColorEXT;
  #endif


  // GL_EXT_blend_func_separate
  #ifndef GLBLENDFUNCSEPARATEEXT_DECL
  #define GLBLENDFUNCSEPARATEEXT_DECL
  csGLBLENDFUNCSEPARATEEXT glBlendFuncSeparateEXT;
  #endif


  // GL_EXT_blend_logic_op

  // GL_EXT_blend_minmax
  #ifndef GLBLENDEQUATIONEXT_DECL
  #define GLBLENDEQUATIONEXT_DECL
  csGLBLENDEQUATIONEXT glBlendEquationEXT;
  #endif


  // GL_EXT_blend_subtract

  // GL_EXT_clip_volume_hint

  // GL_EXT_color_subtable
  #ifndef GLCOLORSUBTABLEEXT_DECL
  #define GLCOLORSUBTABLEEXT_DECL
  csGLCOLORSUBTABLEEXT glColorSubTableEXT;
  #endif

  #ifndef GLCOPYCOLORSUBTABLEEXT_DECL
  #define GLCOPYCOLORSUBTABLEEXT_DECL
  csGLCOPYCOLORSUBTABLEEXT glCopyColorSubTableEXT;
  #endif


  // GL_EXT_compiled_vertex_array
  #ifndef GLLOCKARRAYSEXT_DECL
  #define GLLOCKARRAYSEXT_DECL
  csGLLOCKARRAYSEXT glLockArraysEXT;
  #endif

  #ifndef GLUNLOCKARRAYSEXT_DECL
  #define GLUNLOCKARRAYSEXT_DECL
  csGLUNLOCKARRAYSEXT glUnlockArraysEXT;
  #endif


  // GL_EXT_convolution
  #ifndef GLCONVOLUTIONFILTER1DEXT_DECL
  #define GLCONVOLUTIONFILTER1DEXT_DECL
  csGLCONVOLUTIONFILTER1DEXT glConvolutionFilter1DEXT;
  #endif

  #ifndef GLCONVOLUTIONFILTER2DEXT_DECL
  #define GLCONVOLUTIONFILTER2DEXT_DECL
  csGLCONVOLUTIONFILTER2DEXT glConvolutionFilter2DEXT;
  #endif

  #ifndef GLCOPYCONVOLUTIONFILTER1DEXT_DECL
  #define GLCOPYCONVOLUTIONFILTER1DEXT_DECL
  csGLCOPYCONVOLUTIONFILTER1DEXT glCopyConvolutionFilter1DEXT;
  #endif

  #ifndef GLCOPYCONVOLUTIONFILTER2DEXT_DECL
  #define GLCOPYCONVOLUTIONFILTER2DEXT_DECL
  csGLCOPYCONVOLUTIONFILTER2DEXT glCopyConvolutionFilter2DEXT;
  #endif

  #ifndef GLGETCONVOLUTIONFILTEREXT_DECL
  #define GLGETCONVOLUTIONFILTEREXT_DECL
  csGLGETCONVOLUTIONFILTEREXT glGetConvolutionFilterEXT;
  #endif

  #ifndef GLSEPARABLEFILTER2DEXT_DECL
  #define GLSEPARABLEFILTER2DEXT_DECL
  csGLSEPARABLEFILTER2DEXT glSeparableFilter2DEXT;
  #endif

  #ifndef GLGETSEPARABLEFILTEREXT_DECL
  #define GLGETSEPARABLEFILTEREXT_DECL
  csGLGETSEPARABLEFILTEREXT glGetSeparableFilterEXT;
  #endif

  #ifndef GLCONVOLUTIONPARAMETERIEXT_DECL
  #define GLCONVOLUTIONPARAMETERIEXT_DECL
  csGLCONVOLUTIONPARAMETERIEXT glConvolutionParameteriEXT;
  #endif

  #ifndef GLCONVOLUTIONPARAMETERIVEXT_DECL
  #define GLCONVOLUTIONPARAMETERIVEXT_DECL
  csGLCONVOLUTIONPARAMETERIVEXT glConvolutionParameterivEXT;
  #endif

  #ifndef GLCONVOLUTIONPARAMETERFEXT_DECL
  #define GLCONVOLUTIONPARAMETERFEXT_DECL
  csGLCONVOLUTIONPARAMETERFEXT glConvolutionParameterfEXT;
  #endif

  #ifndef GLCONVOLUTIONPARAMETERFVEXT_DECL
  #define GLCONVOLUTIONPARAMETERFVEXT_DECL
  csGLCONVOLUTIONPARAMETERFVEXT glConvolutionParameterfvEXT;
  #endif

  #ifndef GLGETCONVOLUTIONPARAMETERIVEXT_DECL
  #define GLGETCONVOLUTIONPARAMETERIVEXT_DECL
  csGLGETCONVOLUTIONPARAMETERIVEXT glGetConvolutionParameterivEXT;
  #endif

  #ifndef GLGETCONVOLUTIONPARAMETERFVEXT_DECL
  #define GLGETCONVOLUTIONPARAMETERFVEXT_DECL
  csGLGETCONVOLUTIONPARAMETERFVEXT glGetConvolutionParameterfvEXT;
  #endif


  // GL_EXT_fog_coord
  #ifndef GLFOGCOORDFEXFLOAT_DECL
  #define GLFOGCOORDFEXFLOAT_DECL
  csGLFOGCOORDFEXFLOAT glFogCoordfEXfloat;
  #endif

  #ifndef GLFOGCOORDDEXDOUBLE_DECL
  #define GLFOGCOORDDEXDOUBLE_DECL
  csGLFOGCOORDDEXDOUBLE glFogCoorddEXdouble;
  #endif

  #ifndef GLFOGCOORDFVEXFLOAT_DECL
  #define GLFOGCOORDFVEXFLOAT_DECL
  csGLFOGCOORDFVEXFLOAT glFogCoordfvEXfloat;
  #endif

  #ifndef GLFOGCOORDDVEXDOUBLE_DECL
  #define GLFOGCOORDDVEXDOUBLE_DECL
  csGLFOGCOORDDVEXDOUBLE glFogCoorddvEXdouble;
  #endif

  #ifndef GLFOGCOORDPOINTEREXT_DECL
  #define GLFOGCOORDPOINTEREXT_DECL
  csGLFOGCOORDPOINTEREXT glFogCoordPointerEXT;
  #endif


  // GL_EXT_histogram
  #ifndef GLHISTOGRAMEXT_DECL
  #define GLHISTOGRAMEXT_DECL
  csGLHISTOGRAMEXT glHistogramEXT;
  #endif

  #ifndef GLRESETHISTOGRAMEXT_DECL
  #define GLRESETHISTOGRAMEXT_DECL
  csGLRESETHISTOGRAMEXT glResetHistogramEXT;
  #endif

  #ifndef GLGETHISTOGRAMEXT_DECL
  #define GLGETHISTOGRAMEXT_DECL
  csGLGETHISTOGRAMEXT glGetHistogramEXT;
  #endif

  #ifndef GLGETHISTOGRAMPARAMETERIVEXT_DECL
  #define GLGETHISTOGRAMPARAMETERIVEXT_DECL
  csGLGETHISTOGRAMPARAMETERIVEXT glGetHistogramParameterivEXT;
  #endif

  #ifndef GLGETHISTOGRAMPARAMETERFVEXT_DECL
  #define GLGETHISTOGRAMPARAMETERFVEXT_DECL
  csGLGETHISTOGRAMPARAMETERFVEXT glGetHistogramParameterfvEXT;
  #endif

  #ifndef GLMINMAXEXT_DECL
  #define GLMINMAXEXT_DECL
  csGLMINMAXEXT glMinmaxEXT;
  #endif

  #ifndef GLRESETMINMAXEXT_DECL
  #define GLRESETMINMAXEXT_DECL
  csGLRESETMINMAXEXT glResetMinmaxEXT;
  #endif

  #ifndef GLGETMINMAXEXT_DECL
  #define GLGETMINMAXEXT_DECL
  csGLGETMINMAXEXT glGetMinmaxEXT;
  #endif

  #ifndef GLGETMINMAXPARAMETERIVEXT_DECL
  #define GLGETMINMAXPARAMETERIVEXT_DECL
  csGLGETMINMAXPARAMETERIVEXT glGetMinmaxParameterivEXT;
  #endif

  #ifndef GLGETMINMAXPARAMETERFVEXT_DECL
  #define GLGETMINMAXPARAMETERFVEXT_DECL
  csGLGETMINMAXPARAMETERFVEXT glGetMinmaxParameterfvEXT;
  #endif


  // GL_EXT_multi_draw_arrays
  #ifndef GLMULTIDRAWARRAYSEXT_DECL
  #define GLMULTIDRAWARRAYSEXT_DECL
  csGLMULTIDRAWARRAYSEXT glMultiDrawArraysEXT;
  #endif

  #ifndef GLMULTIDRAWELEMENTSEXT_DECL
  #define GLMULTIDRAWELEMENTSEXT_DECL
  csGLMULTIDRAWELEMENTSEXT glMultiDrawElementsEXT;
  #endif


  // GL_EXT_packed_pixels

  // GL_EXT_paletted_texture
  #ifndef GLCOLORTABLEEXT_DECL
  #define GLCOLORTABLEEXT_DECL
  csGLCOLORTABLEEXT glColorTableEXT;
  #endif

  #ifndef GLCOLORSUBTABLEEXT_DECL
  #define GLCOLORSUBTABLEEXT_DECL
  csGLCOLORSUBTABLEEXT glColorSubTableEXT;
  #endif

  #ifndef GLGETCOLORTABLEEXT_DECL
  #define GLGETCOLORTABLEEXT_DECL
  csGLGETCOLORTABLEEXT glGetColorTableEXT;
  #endif

  #ifndef GLGETCOLORTABLEPARAMETERIVEXT_DECL
  #define GLGETCOLORTABLEPARAMETERIVEXT_DECL
  csGLGETCOLORTABLEPARAMETERIVEXT glGetColorTableParameterivEXT;
  #endif

  #ifndef GLGETCOLORTABLEPARAMETERFVEXT_DECL
  #define GLGETCOLORTABLEPARAMETERFVEXT_DECL
  csGLGETCOLORTABLEPARAMETERFVEXT glGetColorTableParameterfvEXT;
  #endif


  // GL_EXT_point_parameters
  #ifndef GLPOINTPARAMETERFEXT_DECL
  #define GLPOINTPARAMETERFEXT_DECL
  csGLPOINTPARAMETERFEXT glPointParameterfEXT;
  #endif

  #ifndef GLPOINTPARAMETERFVEXT_DECL
  #define GLPOINTPARAMETERFVEXT_DECL
  csGLPOINTPARAMETERFVEXT glPointParameterfvEXT;
  #endif


  // GL_EXT_polygon_offset
  #ifndef GLPOLYGONOFFSETEXT_DECL
  #define GLPOLYGONOFFSETEXT_DECL
  csGLPOLYGONOFFSETEXT glPolygonOffsetEXT;
  #endif


  // GL_EXT_secondary_color
  #ifndef GLSECONDARYCOLOR3BEXT_DECL
  #define GLSECONDARYCOLOR3BEXT_DECL
  csGLSECONDARYCOLOR3BEXT glSecondaryColor3bEXT;
  #endif

  #ifndef GLSECONDARYCOLOR3SEXT_DECL
  #define GLSECONDARYCOLOR3SEXT_DECL
  csGLSECONDARYCOLOR3SEXT glSecondaryColor3sEXT;
  #endif

  #ifndef GLSECONDARYCOLOR3IEXT_DECL
  #define GLSECONDARYCOLOR3IEXT_DECL
  csGLSECONDARYCOLOR3IEXT glSecondaryColor3iEXT;
  #endif

  #ifndef GLSECONDARYCOLOR3FEXT_DECL
  #define GLSECONDARYCOLOR3FEXT_DECL
  csGLSECONDARYCOLOR3FEXT glSecondaryColor3fEXT;
  #endif

  #ifndef GLSECONDARYCOLOR3DEXT_DECL
  #define GLSECONDARYCOLOR3DEXT_DECL
  csGLSECONDARYCOLOR3DEXT glSecondaryColor3dEXT;
  #endif

  #ifndef GLSECONDARYCOLOR3UBEXT_DECL
  #define GLSECONDARYCOLOR3UBEXT_DECL
  csGLSECONDARYCOLOR3UBEXT glSecondaryColor3ubEXT;
  #endif

  #ifndef GLSECONDARYCOLOR3USEXT_DECL
  #define GLSECONDARYCOLOR3USEXT_DECL
  csGLSECONDARYCOLOR3USEXT glSecondaryColor3usEXT;
  #endif

  #ifndef GLSECONDARYCOLOR3UIEXT_DECL
  #define GLSECONDARYCOLOR3UIEXT_DECL
  csGLSECONDARYCOLOR3UIEXT glSecondaryColor3uiEXT;
  #endif

  #ifndef GLSECONDARYCOLOR3BVEXT_DECL
  #define GLSECONDARYCOLOR3BVEXT_DECL
  csGLSECONDARYCOLOR3BVEXT glSecondaryColor3bvEXT;
  #endif

  #ifndef GLSECONDARYCOLOR3SVEXT_DECL
  #define GLSECONDARYCOLOR3SVEXT_DECL
  csGLSECONDARYCOLOR3SVEXT glSecondaryColor3svEXT;
  #endif

  #ifndef GLSECONDARYCOLOR3IVEXT_DECL
  #define GLSECONDARYCOLOR3IVEXT_DECL
  csGLSECONDARYCOLOR3IVEXT glSecondaryColor3ivEXT;
  #endif

  #ifndef GLSECONDARYCOLOR3FVEXT_DECL
  #define GLSECONDARYCOLOR3FVEXT_DECL
  csGLSECONDARYCOLOR3FVEXT glSecondaryColor3fvEXT;
  #endif

  #ifndef GLSECONDARYCOLOR3DVEXT_DECL
  #define GLSECONDARYCOLOR3DVEXT_DECL
  csGLSECONDARYCOLOR3DVEXT glSecondaryColor3dvEXT;
  #endif

  #ifndef GLSECONDARYCOLOR3UBVEXT_DECL
  #define GLSECONDARYCOLOR3UBVEXT_DECL
  csGLSECONDARYCOLOR3UBVEXT glSecondaryColor3ubvEXT;
  #endif

  #ifndef GLSECONDARYCOLOR3USVEXT_DECL
  #define GLSECONDARYCOLOR3USVEXT_DECL
  csGLSECONDARYCOLOR3USVEXT glSecondaryColor3usvEXT;
  #endif

  #ifndef GLSECONDARYCOLOR3UIVEXT_DECL
  #define GLSECONDARYCOLOR3UIVEXT_DECL
  csGLSECONDARYCOLOR3UIVEXT glSecondaryColor3uivEXT;
  #endif

  #ifndef GLSECONDARYCOLORPOINTEREXT_DECL
  #define GLSECONDARYCOLORPOINTEREXT_DECL
  csGLSECONDARYCOLORPOINTEREXT glSecondaryColorPointerEXT;
  #endif


  // GL_EXT_separate_specular_color

  // GL_EXT_shadow_funcs

  // GL_EXT_shared_texture_palette

  // GL_EXT_stencil_two_side
  #ifndef GLACTIVESTENCILFACEEXT_DECL
  #define GLACTIVESTENCILFACEEXT_DECL
  csGLACTIVESTENCILFACEEXT glActiveStencilFaceEXT;
  #endif


  // GL_EXT_stencil_wrap

  // GL_EXT_subtexture
  #ifndef GLTEXSUBIMAGE1DEXT_DECL
  #define GLTEXSUBIMAGE1DEXT_DECL
  csGLTEXSUBIMAGE1DEXT glTexSubImage1DEXT;
  #endif

  #ifndef GLTEXSUBIMAGE2DEXT_DECL
  #define GLTEXSUBIMAGE2DEXT_DECL
  csGLTEXSUBIMAGE2DEXT glTexSubImage2DEXT;
  #endif

  #ifndef GLTEXSUBIMAGE3DEXT_DECL
  #define GLTEXSUBIMAGE3DEXT_DECL
  csGLTEXSUBIMAGE3DEXT glTexSubImage3DEXT;
  #endif


  // GL_EXT_texture3D
  #ifndef GLTEXIMAGE3DEXT_DECL
  #define GLTEXIMAGE3DEXT_DECL
  csGLTEXIMAGE3DEXT glTexImage3DEXT;
  #endif


  // GL_EXT_texture_compression_s3tc

  // GL_EXT_texture_env_add

  // GL_EXT_texture_env_combine

  // GL_EXT_texture_env_dot3

  // GL_EXT_texture_filter_anisotropic

  // GL_EXT_texture_lod_bias

  // GL_EXT_texture_object
  #ifndef GLGENTEXTURESEXT_DECL
  #define GLGENTEXTURESEXT_DECL
  csGLGENTEXTURESEXT glGenTexturesEXT;
  #endif

  #ifndef GLDELETETEXTURESEXT_DECL
  #define GLDELETETEXTURESEXT_DECL
  csGLDELETETEXTURESEXT glDeleteTexturesEXT;
  #endif

  #ifndef GLBINDTEXTUREEXT_DECL
  #define GLBINDTEXTUREEXT_DECL
  csGLBINDTEXTUREEXT glBindTextureEXT;
  #endif

  #ifndef GLPRIORITIZETEXTURESEXT_DECL
  #define GLPRIORITIZETEXTURESEXT_DECL
  csGLPRIORITIZETEXTURESEXT glPrioritizeTexturesEXT;
  #endif

  #ifndef GLARETEXTURESRESIDENTEXT_DECL
  #define GLARETEXTURESRESIDENTEXT_DECL
  csGLARETEXTURESRESIDENTEXT glAreTexturesResidentEXT;
  #endif

  #ifndef GLISTEXTUREEXT_DECL
  #define GLISTEXTUREEXT_DECL
  csGLISTEXTUREEXT glIsTextureEXT;
  #endif


  // GL_EXT_vertex_array
  #ifndef GLARRAYELEMENTEXT_DECL
  #define GLARRAYELEMENTEXT_DECL
  csGLARRAYELEMENTEXT glArrayElementEXT;
  #endif

  #ifndef GLDRAWARRAYSEXT_DECL
  #define GLDRAWARRAYSEXT_DECL
  csGLDRAWARRAYSEXT glDrawArraysEXT;
  #endif

  #ifndef GLVERTEXPOINTEREXT_DECL
  #define GLVERTEXPOINTEREXT_DECL
  csGLVERTEXPOINTEREXT glVertexPointerEXT;
  #endif

  #ifndef GLNORMALPOINTEREXT_DECL
  #define GLNORMALPOINTEREXT_DECL
  csGLNORMALPOINTEREXT glNormalPointerEXT;
  #endif

  #ifndef GLCOLORPOINTEREXT_DECL
  #define GLCOLORPOINTEREXT_DECL
  csGLCOLORPOINTEREXT glColorPointerEXT;
  #endif

  #ifndef GLINDEXPOINTEREXT_DECL
  #define GLINDEXPOINTEREXT_DECL
  csGLINDEXPOINTEREXT glIndexPointerEXT;
  #endif

  #ifndef GLTEXCOORDPOINTEREXT_DECL
  #define GLTEXCOORDPOINTEREXT_DECL
  csGLTEXCOORDPOINTEREXT glTexCoordPointerEXT;
  #endif

  #ifndef GLEDGEFLAGPOINTEREXT_DECL
  #define GLEDGEFLAGPOINTEREXT_DECL
  csGLEDGEFLAGPOINTEREXT glEdgeFlagPointerEXT;
  #endif

  #ifndef GLGETPOINTERVEXT_DECL
  #define GLGETPOINTERVEXT_DECL
  csGLGETPOINTERVEXT glGetPointervEXT;
  #endif


  // GL_EXT_vertex_shader
  #ifndef GLBEGINVERTEXSHADEREXT_DECL
  #define GLBEGINVERTEXSHADEREXT_DECL
  csGLBEGINVERTEXSHADEREXT glBeginVertexShaderEXT;
  #endif

  #ifndef GLENDVERTEXSHADEREXT_DECL
  #define GLENDVERTEXSHADEREXT_DECL
  csGLENDVERTEXSHADEREXT glEndVertexShaderEXT;
  #endif

  #ifndef GLBINDVERTEXSHADEREXT_DECL
  #define GLBINDVERTEXSHADEREXT_DECL
  csGLBINDVERTEXSHADEREXT glBindVertexShaderEXT;
  #endif

  #ifndef GLGENVERTEXSHADERSEXT_DECL
  #define GLGENVERTEXSHADERSEXT_DECL
  csGLGENVERTEXSHADERSEXT glGenVertexShadersEXT;
  #endif

  #ifndef GLDELETEVERTEXSHADEREXT_DECL
  #define GLDELETEVERTEXSHADEREXT_DECL
  csGLDELETEVERTEXSHADEREXT glDeleteVertexShaderEXT;
  #endif

  #ifndef GLSHADEROP1EXT_DECL
  #define GLSHADEROP1EXT_DECL
  csGLSHADEROP1EXT glShaderOp1EXT;
  #endif

  #ifndef GLSHADEROP2EXT_DECL
  #define GLSHADEROP2EXT_DECL
  csGLSHADEROP2EXT glShaderOp2EXT;
  #endif

  #ifndef GLSHADEROP3EXT_DECL
  #define GLSHADEROP3EXT_DECL
  csGLSHADEROP3EXT glShaderOp3EXT;
  #endif

  #ifndef GLSWIZZLEEXT_DECL
  #define GLSWIZZLEEXT_DECL
  csGLSWIZZLEEXT glSwizzleEXT;
  #endif

  #ifndef GLWRITEMASKEXT_DECL
  #define GLWRITEMASKEXT_DECL
  csGLWRITEMASKEXT glWriteMaskEXT;
  #endif

  #ifndef GLINSERTCOMPONENTEXT_DECL
  #define GLINSERTCOMPONENTEXT_DECL
  csGLINSERTCOMPONENTEXT glInsertComponentEXT;
  #endif

  #ifndef GLEXTRACTCOMPONENTEXT_DECL
  #define GLEXTRACTCOMPONENTEXT_DECL
  csGLEXTRACTCOMPONENTEXT glExtractComponentEXT;
  #endif

  #ifndef GLGENSYMBOLSEXT_DECL
  #define GLGENSYMBOLSEXT_DECL
  csGLGENSYMBOLSEXT glGenSymbolsEXT;
  #endif

  #ifndef GLSETINVARIANTEXT_DECL
  #define GLSETINVARIANTEXT_DECL
  csGLSETINVARIANTEXT glSetInvariantEXT;
  #endif

  #ifndef GLSETLOCALCONSTANTEXT_DECL
  #define GLSETLOCALCONSTANTEXT_DECL
  csGLSETLOCALCONSTANTEXT glSetLocalConstantEXT;
  #endif

  #ifndef GLVARIANTBVEXT_DECL
  #define GLVARIANTBVEXT_DECL
  csGLVARIANTBVEXT glVariantbvEXT;
  #endif

  #ifndef GLVARIANTSVEXT_DECL
  #define GLVARIANTSVEXT_DECL
  csGLVARIANTSVEXT glVariantsvEXT;
  #endif

  #ifndef GLVARIANTIVEXT_DECL
  #define GLVARIANTIVEXT_DECL
  csGLVARIANTIVEXT glVariantivEXT;
  #endif

  #ifndef GLVARIANTFVEXT_DECL
  #define GLVARIANTFVEXT_DECL
  csGLVARIANTFVEXT glVariantfvEXT;
  #endif

  #ifndef GLVARIANTDVEXT_DECL
  #define GLVARIANTDVEXT_DECL
  csGLVARIANTDVEXT glVariantdvEXT;
  #endif

  #ifndef GLVARIANTUBVEXT_DECL
  #define GLVARIANTUBVEXT_DECL
  csGLVARIANTUBVEXT glVariantubvEXT;
  #endif

  #ifndef GLVARIANTUSVEXT_DECL
  #define GLVARIANTUSVEXT_DECL
  csGLVARIANTUSVEXT glVariantusvEXT;
  #endif

  #ifndef GLVARIANTUIVEXT_DECL
  #define GLVARIANTUIVEXT_DECL
  csGLVARIANTUIVEXT glVariantuivEXT;
  #endif

  #ifndef GLVARIANTPOINTEREXT_DECL
  #define GLVARIANTPOINTEREXT_DECL
  csGLVARIANTPOINTEREXT glVariantPointerEXT;
  #endif

  #ifndef GLENABLEVARIANTCLIENTSTATEEXT_DECL
  #define GLENABLEVARIANTCLIENTSTATEEXT_DECL
  csGLENABLEVARIANTCLIENTSTATEEXT glEnableVariantClientStateEXT;
  #endif

  #ifndef GLDISABLEVARIANTCLIENTSTATEEXT_DECL
  #define GLDISABLEVARIANTCLIENTSTATEEXT_DECL
  csGLDISABLEVARIANTCLIENTSTATEEXT glDisableVariantClientStateEXT;
  #endif

  #ifndef GLBINDLIGHTPARAMETEREXT_DECL
  #define GLBINDLIGHTPARAMETEREXT_DECL
  csGLBINDLIGHTPARAMETEREXT glBindLightParameterEXT;
  #endif

  #ifndef GLBINDMATERIALPARAMETEREXT_DECL
  #define GLBINDMATERIALPARAMETEREXT_DECL
  csGLBINDMATERIALPARAMETEREXT glBindMaterialParameterEXT;
  #endif

  #ifndef GLBINDTEXGENPARAMETEREXT_DECL
  #define GLBINDTEXGENPARAMETEREXT_DECL
  csGLBINDTEXGENPARAMETEREXT glBindTexGenParameterEXT;
  #endif

  #ifndef GLBINDTEXTUREUNITPARAMETEREXT_DECL
  #define GLBINDTEXTUREUNITPARAMETEREXT_DECL
  csGLBINDTEXTUREUNITPARAMETEREXT glBindTextureUnitParameterEXT;
  #endif

  #ifndef GLBINDPARAMETEREXT_DECL
  #define GLBINDPARAMETEREXT_DECL
  csGLBINDPARAMETEREXT glBindParameterEXT;
  #endif

  #ifndef GLISVARIANTENABLEDEXT_DECL
  #define GLISVARIANTENABLEDEXT_DECL
  csGLISVARIANTENABLEDEXT glIsVariantEnabledEXT;
  #endif

  #ifndef GLGETVARIANTBOOLEANVEXT_DECL
  #define GLGETVARIANTBOOLEANVEXT_DECL
  csGLGETVARIANTBOOLEANVEXT glGetVariantBooleanvEXT;
  #endif

  #ifndef GLGETVARIANTINTEGERVEXT_DECL
  #define GLGETVARIANTINTEGERVEXT_DECL
  csGLGETVARIANTINTEGERVEXT glGetVariantIntegervEXT;
  #endif

  #ifndef GLGETVARIANTFLOATVEXT_DECL
  #define GLGETVARIANTFLOATVEXT_DECL
  csGLGETVARIANTFLOATVEXT glGetVariantFloatvEXT;
  #endif

  #ifndef GLGETVARIANTPOINTERVEXT_DECL
  #define GLGETVARIANTPOINTERVEXT_DECL
  csGLGETVARIANTPOINTERVEXT glGetVariantPointervEXT;
  #endif

  #ifndef GLGETINVARIANTBOOLEANVEXT_DECL
  #define GLGETINVARIANTBOOLEANVEXT_DECL
  csGLGETINVARIANTBOOLEANVEXT glGetInvariantBooleanvEXT;
  #endif

  #ifndef GLGETINVARIANTINTEGERVEXT_DECL
  #define GLGETINVARIANTINTEGERVEXT_DECL
  csGLGETINVARIANTINTEGERVEXT glGetInvariantIntegervEXT;
  #endif

  #ifndef GLGETINVARIANTFLOATVEXT_DECL
  #define GLGETINVARIANTFLOATVEXT_DECL
  csGLGETINVARIANTFLOATVEXT glGetInvariantFloatvEXT;
  #endif

  #ifndef GLGETLOCALCONSTANTBOOLEANVEXT_DECL
  #define GLGETLOCALCONSTANTBOOLEANVEXT_DECL
  csGLGETLOCALCONSTANTBOOLEANVEXT glGetLocalConstantBooleanvEXT;
  #endif

  #ifndef GLGETLOCALCONSTANTINTEGERVEXT_DECL
  #define GLGETLOCALCONSTANTINTEGERVEXT_DECL
  csGLGETLOCALCONSTANTINTEGERVEXT glGetLocalConstantIntegervEXT;
  #endif

  #ifndef GLGETLOCALCONSTANTFLOATVEXT_DECL
  #define GLGETLOCALCONSTANTFLOATVEXT_DECL
  csGLGETLOCALCONSTANTFLOATVEXT glGetLocalConstantFloatvEXT;
  #endif


  // GL_EXT_vertex_weighting
  #ifndef GLVERTEXWEIGHTFEXT_DECL
  #define GLVERTEXWEIGHTFEXT_DECL
  csGLVERTEXWEIGHTFEXT glVertexWeightfEXT;
  #endif

  #ifndef GLVERTEXWEIGHTFVEXT_DECL
  #define GLVERTEXWEIGHTFVEXT_DECL
  csGLVERTEXWEIGHTFVEXT glVertexWeightfvEXT;
  #endif

  #ifndef GLVERTEXWEIGHTPOINTEREXT_DECL
  #define GLVERTEXWEIGHTPOINTEREXT_DECL
  csGLVERTEXWEIGHTPOINTEREXT glVertexWeightPointerEXT;
  #endif


  // GL_HP_occlusion_test

  // GL_NV_blend_square

  // GL_NV_copy_depth_to_color

  // GL_NV_depth_clamp

  // GL_NV_evaluators
  #ifndef GLMAPCONTROLPOINTSNV_DECL
  #define GLMAPCONTROLPOINTSNV_DECL
  csGLMAPCONTROLPOINTSNV glMapControlPointsNV;
  #endif

  #ifndef GLMAPPARAMETERIVNV_DECL
  #define GLMAPPARAMETERIVNV_DECL
  csGLMAPPARAMETERIVNV glMapParameterivNV;
  #endif

  #ifndef GLMAPPARAMETERFVNV_DECL
  #define GLMAPPARAMETERFVNV_DECL
  csGLMAPPARAMETERFVNV glMapParameterfvNV;
  #endif

  #ifndef GLGETMAPCONTROLPOINTSNV_DECL
  #define GLGETMAPCONTROLPOINTSNV_DECL
  csGLGETMAPCONTROLPOINTSNV glGetMapControlPointsNV;
  #endif

  #ifndef GLGETMAPPARAMETERIVNV_DECL
  #define GLGETMAPPARAMETERIVNV_DECL
  csGLGETMAPPARAMETERIVNV glGetMapParameterivNV;
  #endif

  #ifndef GLGETMAPPARAMETERFVNV_DECL
  #define GLGETMAPPARAMETERFVNV_DECL
  csGLGETMAPPARAMETERFVNV glGetMapParameterfvNV;
  #endif

  #ifndef GLGETMAPATTRIBPARAMETERIVNV_DECL
  #define GLGETMAPATTRIBPARAMETERIVNV_DECL
  csGLGETMAPATTRIBPARAMETERIVNV glGetMapAttribParameterivNV;
  #endif

  #ifndef GLGETMAPATTRIBPARAMETERFVNV_DECL
  #define GLGETMAPATTRIBPARAMETERFVNV_DECL
  csGLGETMAPATTRIBPARAMETERFVNV glGetMapAttribParameterfvNV;
  #endif

  #ifndef GLEVALMAPSNV_DECL
  #define GLEVALMAPSNV_DECL
  csGLEVALMAPSNV glEvalMapsNV;
  #endif


  // GL_NV_fence
  #ifndef GLGENFENCESNV_DECL
  #define GLGENFENCESNV_DECL
  csGLGENFENCESNV glGenFencesNV;
  #endif

  #ifndef GLDELETEFENCESNV_DECL
  #define GLDELETEFENCESNV_DECL
  csGLDELETEFENCESNV glDeleteFencesNV;
  #endif

  #ifndef GLSETFENCENV_DECL
  #define GLSETFENCENV_DECL
  csGLSETFENCENV glSetFenceNV;
  #endif

  #ifndef GLTESTFENCENV_DECL
  #define GLTESTFENCENV_DECL
  csGLTESTFENCENV glTestFenceNV;
  #endif

  #ifndef GLFINISHFENCENV_DECL
  #define GLFINISHFENCENV_DECL
  csGLFINISHFENCENV glFinishFenceNV;
  #endif

  #ifndef GLISFENCENV_DECL
  #define GLISFENCENV_DECL
  csGLISFENCENV glIsFenceNV;
  #endif

  #ifndef GLGETFENCEIVNV_DECL
  #define GLGETFENCEIVNV_DECL
  csGLGETFENCEIVNV glGetFenceivNV;
  #endif


  // GL_NV_fog_distance

  // GL_NV_light_max_exponent

  // GL_NV_multisample_filter_hint

  // GL_NV_occlusion_query
  #ifndef GLGENOCCLUSIONQUERIESNV_DECL
  #define GLGENOCCLUSIONQUERIESNV_DECL
  csGLGENOCCLUSIONQUERIESNV glGenOcclusionQueriesNV;
  #endif

  #ifndef GLDELETEOCCLUSIONQUERIESNV_DECL
  #define GLDELETEOCCLUSIONQUERIESNV_DECL
  csGLDELETEOCCLUSIONQUERIESNV glDeleteOcclusionQueriesNV;
  #endif

  #ifndef GLISOCCLUSIONQUERYNV_DECL
  #define GLISOCCLUSIONQUERYNV_DECL
  csGLISOCCLUSIONQUERYNV glIsOcclusionQueryNV;
  #endif

  #ifndef GLBEGINOCCLUSIONQUERYNV_DECL
  #define GLBEGINOCCLUSIONQUERYNV_DECL
  csGLBEGINOCCLUSIONQUERYNV glBeginOcclusionQueryNV;
  #endif

  #ifndef GLENDOCCLUSIONQUERYNV_DECL
  #define GLENDOCCLUSIONQUERYNV_DECL
  csGLENDOCCLUSIONQUERYNV glEndOcclusionQueryNV;
  #endif

  #ifndef GLGETOCCLUSIONQUERYIVNV_DECL
  #define GLGETOCCLUSIONQUERYIVNV_DECL
  csGLGETOCCLUSIONQUERYIVNV glGetOcclusionQueryivNV;
  #endif

  #ifndef GLGETOCCLUSIONQUERYUIVNV_DECL
  #define GLGETOCCLUSIONQUERYUIVNV_DECL
  csGLGETOCCLUSIONQUERYUIVNV glGetOcclusionQueryuivNV;
  #endif


  // GL_NV_packed_depth_stencil

  // GL_NV_point_sprite
  #ifndef GLPOINTPARAMETERINV_DECL
  #define GLPOINTPARAMETERINV_DECL
  csGLPOINTPARAMETERINV glPointParameteriNV;
  #endif

  #ifndef GLPOINTPARAMETERIVNV_DECL
  #define GLPOINTPARAMETERIVNV_DECL
  csGLPOINTPARAMETERIVNV glPointParameterivNV;
  #endif


  // GL_NV_register_combiners
  #ifndef GLCOMBINERPARAMETERFVNV_DECL
  #define GLCOMBINERPARAMETERFVNV_DECL
  csGLCOMBINERPARAMETERFVNV glCombinerParameterfvNV;
  #endif

  #ifndef GLCOMBINERPARAMETERIVNV_DECL
  #define GLCOMBINERPARAMETERIVNV_DECL
  csGLCOMBINERPARAMETERIVNV glCombinerParameterivNV;
  #endif

  #ifndef GLCOMBINERPARAMETERFNV_DECL
  #define GLCOMBINERPARAMETERFNV_DECL
  csGLCOMBINERPARAMETERFNV glCombinerParameterfNV;
  #endif

  #ifndef GLCOMBINERPARAMETERINV_DECL
  #define GLCOMBINERPARAMETERINV_DECL
  csGLCOMBINERPARAMETERINV glCombinerParameteriNV;
  #endif

  #ifndef GLCOMBINERINPUTNV_DECL
  #define GLCOMBINERINPUTNV_DECL
  csGLCOMBINERINPUTNV glCombinerInputNV;
  #endif

  #ifndef GLCOMBINEROUTPUTNV_DECL
  #define GLCOMBINEROUTPUTNV_DECL
  csGLCOMBINEROUTPUTNV glCombinerOutputNV;
  #endif

  #ifndef GLFINALCOMBINERINPUTNV_DECL
  #define GLFINALCOMBINERINPUTNV_DECL
  csGLFINALCOMBINERINPUTNV glFinalCombinerInputNV;
  #endif

  #ifndef GLGETCOMBINERINPUTPARAMETERFVNV_DECL
  #define GLGETCOMBINERINPUTPARAMETERFVNV_DECL
  csGLGETCOMBINERINPUTPARAMETERFVNV glGetCombinerInputParameterfvNV;
  #endif

  #ifndef GLGETCOMBINERINPUTPARAMETERIVNV_DECL
  #define GLGETCOMBINERINPUTPARAMETERIVNV_DECL
  csGLGETCOMBINERINPUTPARAMETERIVNV glGetCombinerInputParameterivNV;
  #endif

  #ifndef GLGETCOMBINEROUTPUTPARAMETERFVNV_DECL
  #define GLGETCOMBINEROUTPUTPARAMETERFVNV_DECL
  csGLGETCOMBINEROUTPUTPARAMETERFVNV glGetCombinerOutputParameterfvNV;
  #endif

  #ifndef GLGETCOMBINEROUTPUTPARAMETERIVNV_DECL
  #define GLGETCOMBINEROUTPUTPARAMETERIVNV_DECL
  csGLGETCOMBINEROUTPUTPARAMETERIVNV glGetCombinerOutputParameterivNV;
  #endif

  #ifndef GLGETFINALCOMBINERINPUTPARAMETERFVNV_DECL
  #define GLGETFINALCOMBINERINPUTPARAMETERFVNV_DECL
  csGLGETFINALCOMBINERINPUTPARAMETERFVNV glGetFinalCombinerInputParameterfvNV;
  #endif

  #ifndef GLGETFINALCOMBINERINPUTPARAMETERIVNV_DECL
  #define GLGETFINALCOMBINERINPUTPARAMETERIVNV_DECL
  csGLGETFINALCOMBINERINPUTPARAMETERIVNV glGetFinalCombinerInputParameterivNV;
  #endif


  // GL_NV_register_combiners2
  #ifndef GLCOMBINERSTAGEPARAMETERFVNV_DECL
  #define GLCOMBINERSTAGEPARAMETERFVNV_DECL
  csGLCOMBINERSTAGEPARAMETERFVNV glCombinerStageParameterfvNV;
  #endif

  #ifndef GLGETCOMBINERSTAGEPARAMETERFVNV_DECL
  #define GLGETCOMBINERSTAGEPARAMETERFVNV_DECL
  csGLGETCOMBINERSTAGEPARAMETERFVNV glGetCombinerStageParameterfvNV;
  #endif


  // GL_NV_texgen_emboss

  // GL_NV_texgen_reflection

  // GL_NV_texture_compression_vtc

  // GL_NV_texture_env_combine4

  // GL_NV_texture_rectangle

  // GL_NV_texture_shader

  // GL_NV_texture_shader2

  // GL_NV_texture_shader3

  // GL_NV_vertex_array_range
  #ifndef GLVERTEXARRAYRANGENV_DECL
  #define GLVERTEXARRAYRANGENV_DECL
  csGLVERTEXARRAYRANGENV glVertexArrayRangeNV;
  #endif

  #ifndef GLFLUSHVERTEXARRAYRANGENV_DECL
  #define GLFLUSHVERTEXARRAYRANGENV_DECL
  csGLFLUSHVERTEXARRAYRANGENV glFlushVertexArrayRangeNV;
  #endif

  #ifndef WGLALLOCATEMEMORYNV_DECL
  #define WGLALLOCATEMEMORYNV_DECL
  csWGLALLOCATEMEMORYNV wglAllocateMemoryNV;
  #endif

  #ifndef WGLFREEMEMORYNV_DECL
  #define WGLFREEMEMORYNV_DECL
  csWGLFREEMEMORYNV wglFreeMemoryNV;
  #endif


  // GL_NV_vertex_array_range2

  // GL_NV_vertex_program
  #ifndef GLBINDPROGRAMNV_DECL
  #define GLBINDPROGRAMNV_DECL
  csGLBINDPROGRAMNV glBindProgramNV;
  #endif

  #ifndef GLDELETEPROGRAMSNV_DECL
  #define GLDELETEPROGRAMSNV_DECL
  csGLDELETEPROGRAMSNV glDeleteProgramsNV;
  #endif

  #ifndef GLEXECUTEPROGRAMNV_DECL
  #define GLEXECUTEPROGRAMNV_DECL
  csGLEXECUTEPROGRAMNV glExecuteProgramNV;
  #endif

  #ifndef GLGENPROGRAMSNV_DECL
  #define GLGENPROGRAMSNV_DECL
  csGLGENPROGRAMSNV glGenProgramsNV;
  #endif

  #ifndef GLAREPROGRAMSRESIDENTNV_DECL
  #define GLAREPROGRAMSRESIDENTNV_DECL
  csGLAREPROGRAMSRESIDENTNV glAreProgramsResidentNV;
  #endif

  #ifndef GLREQUESTRESIDENTPROGRAMSNV_DECL
  #define GLREQUESTRESIDENTPROGRAMSNV_DECL
  csGLREQUESTRESIDENTPROGRAMSNV glRequestResidentProgramsNV;
  #endif

  #ifndef GLGETPROGRAMPARAMETERFVNV_DECL
  #define GLGETPROGRAMPARAMETERFVNV_DECL
  csGLGETPROGRAMPARAMETERFVNV glGetProgramParameterfvNV;
  #endif

  #ifndef GLGETPROGRAMPARAMETERDVNV_DECL
  #define GLGETPROGRAMPARAMETERDVNV_DECL
  csGLGETPROGRAMPARAMETERDVNV glGetProgramParameterdvNV;
  #endif

  #ifndef GLGETPROGRAMIVNV_DECL
  #define GLGETPROGRAMIVNV_DECL
  csGLGETPROGRAMIVNV glGetProgramivNV;
  #endif

  #ifndef GLGETPROGRAMSTRINGNV_DECL
  #define GLGETPROGRAMSTRINGNV_DECL
  csGLGETPROGRAMSTRINGNV glGetProgramStringNV;
  #endif

  #ifndef GLGETTRACKMATRIXIVNV_DECL
  #define GLGETTRACKMATRIXIVNV_DECL
  csGLGETTRACKMATRIXIVNV glGetTrackMatrixivNV;
  #endif

  #ifndef GLGETVERTEXATTRIBDVNV_DECL
  #define GLGETVERTEXATTRIBDVNV_DECL
  csGLGETVERTEXATTRIBDVNV glGetVertexAttribdvNV;
  #endif

  #ifndef GLGETVERTEXATTRIBFVNV_DECL
  #define GLGETVERTEXATTRIBFVNV_DECL
  csGLGETVERTEXATTRIBFVNV glGetVertexAttribfvNV;
  #endif

  #ifndef GLGETVERTEXATTRIBIVNV_DECL
  #define GLGETVERTEXATTRIBIVNV_DECL
  csGLGETVERTEXATTRIBIVNV glGetVertexAttribivNV;
  #endif

  #ifndef GLGETVERTEXATTRIBPOINTERVNV_DECL
  #define GLGETVERTEXATTRIBPOINTERVNV_DECL
  csGLGETVERTEXATTRIBPOINTERVNV glGetVertexAttribPointervNV;
  #endif

  #ifndef GLISPROGRAMNV_DECL
  #define GLISPROGRAMNV_DECL
  csGLISPROGRAMNV glIsProgramNV;
  #endif

  #ifndef GLLOADPROGRAMNV_DECL
  #define GLLOADPROGRAMNV_DECL
  csGLLOADPROGRAMNV glLoadProgramNV;
  #endif

  #ifndef GLPROGRAMPARAMETER4FNV_DECL
  #define GLPROGRAMPARAMETER4FNV_DECL
  csGLPROGRAMPARAMETER4FNV glProgramParameter4fNV;
  #endif

  #ifndef GLPROGRAMPARAMETER4FVNV_DECL
  #define GLPROGRAMPARAMETER4FVNV_DECL
  csGLPROGRAMPARAMETER4FVNV glProgramParameter4fvNV;
  #endif

  #ifndef GLPROGRAMPARAMETERS4DVNV_DECL
  #define GLPROGRAMPARAMETERS4DVNV_DECL
  csGLPROGRAMPARAMETERS4DVNV glProgramParameters4dvNV;
  #endif

  #ifndef GLPROGRAMPARAMETERS4FVNV_DECL
  #define GLPROGRAMPARAMETERS4FVNV_DECL
  csGLPROGRAMPARAMETERS4FVNV glProgramParameters4fvNV;
  #endif

  #ifndef GLTRACKMATRIXNV_DECL
  #define GLTRACKMATRIXNV_DECL
  csGLTRACKMATRIXNV glTrackMatrixNV;
  #endif

  #ifndef GLVERTEXATTRIBPOINTERNV_DECL
  #define GLVERTEXATTRIBPOINTERNV_DECL
  csGLVERTEXATTRIBPOINTERNV glVertexAttribPointerNV;
  #endif

  #ifndef GLVERTEXATTRIB1SNV_DECL
  #define GLVERTEXATTRIB1SNV_DECL
  csGLVERTEXATTRIB1SNV glVertexAttrib1sNV;
  #endif

  #ifndef GLVERTEXATTRIB1FNV_DECL
  #define GLVERTEXATTRIB1FNV_DECL
  csGLVERTEXATTRIB1FNV glVertexAttrib1fNV;
  #endif

  #ifndef GLVERTEXATTRIB1DNV_DECL
  #define GLVERTEXATTRIB1DNV_DECL
  csGLVERTEXATTRIB1DNV glVertexAttrib1dNV;
  #endif

  #ifndef GLVERTEXATTRIB2SNV_DECL
  #define GLVERTEXATTRIB2SNV_DECL
  csGLVERTEXATTRIB2SNV glVertexAttrib2sNV;
  #endif

  #ifndef GLVERTEXATTRIB2FNV_DECL
  #define GLVERTEXATTRIB2FNV_DECL
  csGLVERTEXATTRIB2FNV glVertexAttrib2fNV;
  #endif

  #ifndef GLVERTEXATTRIB2DNV_DECL
  #define GLVERTEXATTRIB2DNV_DECL
  csGLVERTEXATTRIB2DNV glVertexAttrib2dNV;
  #endif

  #ifndef GLVERTEXATTRIB3SNV_DECL
  #define GLVERTEXATTRIB3SNV_DECL
  csGLVERTEXATTRIB3SNV glVertexAttrib3sNV;
  #endif

  #ifndef GLVERTEXATTRIB3FNV_DECL
  #define GLVERTEXATTRIB3FNV_DECL
  csGLVERTEXATTRIB3FNV glVertexAttrib3fNV;
  #endif

  #ifndef GLVERTEXATTRIB3DNV_DECL
  #define GLVERTEXATTRIB3DNV_DECL
  csGLVERTEXATTRIB3DNV glVertexAttrib3dNV;
  #endif

  #ifndef GLVERTEXATTRIB4SNV_DECL
  #define GLVERTEXATTRIB4SNV_DECL
  csGLVERTEXATTRIB4SNV glVertexAttrib4sNV;
  #endif

  #ifndef GLVERTEXATTRIB4FNV_DECL
  #define GLVERTEXATTRIB4FNV_DECL
  csGLVERTEXATTRIB4FNV glVertexAttrib4fNV;
  #endif

  #ifndef GLVERTEXATTRIB4DNV_DECL
  #define GLVERTEXATTRIB4DNV_DECL
  csGLVERTEXATTRIB4DNV glVertexAttrib4dNV;
  #endif

  #ifndef GLVERTEXATTRIB4UBNV_DECL
  #define GLVERTEXATTRIB4UBNV_DECL
  csGLVERTEXATTRIB4UBNV glVertexAttrib4ubNV;
  #endif

  #ifndef GLVERTEXATTRIB1SVNV_DECL
  #define GLVERTEXATTRIB1SVNV_DECL
  csGLVERTEXATTRIB1SVNV glVertexAttrib1svNV;
  #endif

  #ifndef GLVERTEXATTRIB1FVNV_DECL
  #define GLVERTEXATTRIB1FVNV_DECL
  csGLVERTEXATTRIB1FVNV glVertexAttrib1fvNV;
  #endif

  #ifndef GLVERTEXATTRIB1DVNV_DECL
  #define GLVERTEXATTRIB1DVNV_DECL
  csGLVERTEXATTRIB1DVNV glVertexAttrib1dvNV;
  #endif

  #ifndef GLVERTEXATTRIB2SVNV_DECL
  #define GLVERTEXATTRIB2SVNV_DECL
  csGLVERTEXATTRIB2SVNV glVertexAttrib2svNV;
  #endif

  #ifndef GLVERTEXATTRIB2FVNV_DECL
  #define GLVERTEXATTRIB2FVNV_DECL
  csGLVERTEXATTRIB2FVNV glVertexAttrib2fvNV;
  #endif

  #ifndef GLVERTEXATTRIB2DVNV_DECL
  #define GLVERTEXATTRIB2DVNV_DECL
  csGLVERTEXATTRIB2DVNV glVertexAttrib2dvNV;
  #endif

  #ifndef GLVERTEXATTRIB3SVNV_DECL
  #define GLVERTEXATTRIB3SVNV_DECL
  csGLVERTEXATTRIB3SVNV glVertexAttrib3svNV;
  #endif

  #ifndef GLVERTEXATTRIB3FVNV_DECL
  #define GLVERTEXATTRIB3FVNV_DECL
  csGLVERTEXATTRIB3FVNV glVertexAttrib3fvNV;
  #endif

  #ifndef GLVERTEXATTRIB3DVNV_DECL
  #define GLVERTEXATTRIB3DVNV_DECL
  csGLVERTEXATTRIB3DVNV glVertexAttrib3dvNV;
  #endif

  #ifndef GLVERTEXATTRIB4SVNV_DECL
  #define GLVERTEXATTRIB4SVNV_DECL
  csGLVERTEXATTRIB4SVNV glVertexAttrib4svNV;
  #endif

  #ifndef GLVERTEXATTRIB4FVNV_DECL
  #define GLVERTEXATTRIB4FVNV_DECL
  csGLVERTEXATTRIB4FVNV glVertexAttrib4fvNV;
  #endif

  #ifndef GLVERTEXATTRIB4DVNV_DECL
  #define GLVERTEXATTRIB4DVNV_DECL
  csGLVERTEXATTRIB4DVNV glVertexAttrib4dvNV;
  #endif

  #ifndef GLVERTEXATTRIB4UBVNV_DECL
  #define GLVERTEXATTRIB4UBVNV_DECL
  csGLVERTEXATTRIB4UBVNV glVertexAttrib4ubvNV;
  #endif

  #ifndef GLVERTEXATTRIBS1SVNV_DECL
  #define GLVERTEXATTRIBS1SVNV_DECL
  csGLVERTEXATTRIBS1SVNV glVertexAttribs1svNV;
  #endif

  #ifndef GLVERTEXATTRIBS1FVNV_DECL
  #define GLVERTEXATTRIBS1FVNV_DECL
  csGLVERTEXATTRIBS1FVNV glVertexAttribs1fvNV;
  #endif

  #ifndef GLVERTEXATTRIBS1DVNV_DECL
  #define GLVERTEXATTRIBS1DVNV_DECL
  csGLVERTEXATTRIBS1DVNV glVertexAttribs1dvNV;
  #endif

  #ifndef GLVERTEXATTRIBS2SVNV_DECL
  #define GLVERTEXATTRIBS2SVNV_DECL
  csGLVERTEXATTRIBS2SVNV glVertexAttribs2svNV;
  #endif

  #ifndef GLVERTEXATTRIBS2FVNV_DECL
  #define GLVERTEXATTRIBS2FVNV_DECL
  csGLVERTEXATTRIBS2FVNV glVertexAttribs2fvNV;
  #endif

  #ifndef GLVERTEXATTRIBS2DVNV_DECL
  #define GLVERTEXATTRIBS2DVNV_DECL
  csGLVERTEXATTRIBS2DVNV glVertexAttribs2dvNV;
  #endif

  #ifndef GLVERTEXATTRIBS3SVNV_DECL
  #define GLVERTEXATTRIBS3SVNV_DECL
  csGLVERTEXATTRIBS3SVNV glVertexAttribs3svNV;
  #endif

  #ifndef GLVERTEXATTRIBS3FVNV_DECL
  #define GLVERTEXATTRIBS3FVNV_DECL
  csGLVERTEXATTRIBS3FVNV glVertexAttribs3fvNV;
  #endif

  #ifndef GLVERTEXATTRIBS3DVNV_DECL
  #define GLVERTEXATTRIBS3DVNV_DECL
  csGLVERTEXATTRIBS3DVNV glVertexAttribs3dvNV;
  #endif

  #ifndef GLVERTEXATTRIBS4SVNV_DECL
  #define GLVERTEXATTRIBS4SVNV_DECL
  csGLVERTEXATTRIBS4SVNV glVertexAttribs4svNV;
  #endif

  #ifndef GLVERTEXATTRIBS4FVNV_DECL
  #define GLVERTEXATTRIBS4FVNV_DECL
  csGLVERTEXATTRIBS4FVNV glVertexAttribs4fvNV;
  #endif

  #ifndef GLVERTEXATTRIBS4DVNV_DECL
  #define GLVERTEXATTRIBS4DVNV_DECL
  csGLVERTEXATTRIBS4DVNV glVertexAttribs4dvNV;
  #endif

  #ifndef GLVERTEXATTRIBS4UBVNV_DECL
  #define GLVERTEXATTRIBS4UBVNV_DECL
  csGLVERTEXATTRIBS4UBVNV glVertexAttribs4ubvNV;
  #endif


  // GL_NV_vertex_program1_1

  // GL_ATI_element_array
  #ifndef GLELEMENTPOINTERATI_DECL
  #define GLELEMENTPOINTERATI_DECL
  csGLELEMENTPOINTERATI glElementPointerATI;
  #endif

  #ifndef GLDRAWELEMENTARRAYATI_DECL
  #define GLDRAWELEMENTARRAYATI_DECL
  csGLDRAWELEMENTARRAYATI glDrawElementArrayATI;
  #endif

  #ifndef GLDRAWRANGEELEMENTARRAYATI_DECL
  #define GLDRAWRANGEELEMENTARRAYATI_DECL
  csGLDRAWRANGEELEMENTARRAYATI glDrawRangeElementArrayATI;
  #endif


  // GL_ATI_envmap_bumpmap
  #ifndef GLTEXBUMPPARAMETERIVATI_DECL
  #define GLTEXBUMPPARAMETERIVATI_DECL
  csGLTEXBUMPPARAMETERIVATI glTexBumpParameterivATI;
  #endif

  #ifndef GLTEXBUMPPARAMETERFVATI_DECL
  #define GLTEXBUMPPARAMETERFVATI_DECL
  csGLTEXBUMPPARAMETERFVATI glTexBumpParameterfvATI;
  #endif

  #ifndef GLGETTEXBUMPPARAMETERIVATI_DECL
  #define GLGETTEXBUMPPARAMETERIVATI_DECL
  csGLGETTEXBUMPPARAMETERIVATI glGetTexBumpParameterivATI;
  #endif

  #ifndef GLGETTEXBUMPPARAMETERFVATI_DECL
  #define GLGETTEXBUMPPARAMETERFVATI_DECL
  csGLGETTEXBUMPPARAMETERFVATI glGetTexBumpParameterfvATI;
  #endif


  // GL_ATI_fragment_shader
  #ifndef GLGENFRAGMENTSHADERSATI_DECL
  #define GLGENFRAGMENTSHADERSATI_DECL
  csGLGENFRAGMENTSHADERSATI glGenFragmentShadersATI;
  #endif

  #ifndef GLBINDFRAGMENTSHADERATI_DECL
  #define GLBINDFRAGMENTSHADERATI_DECL
  csGLBINDFRAGMENTSHADERATI glBindFragmentShaderATI;
  #endif

  #ifndef GLDELETEFRAGMENTSHADERATI_DECL
  #define GLDELETEFRAGMENTSHADERATI_DECL
  csGLDELETEFRAGMENTSHADERATI glDeleteFragmentShaderATI;
  #endif

  #ifndef GLBEGINFRAGMENTSHADERATI_DECL
  #define GLBEGINFRAGMENTSHADERATI_DECL
  csGLBEGINFRAGMENTSHADERATI glBeginFragmentShaderATI;
  #endif

  #ifndef GLENDFRAGMENTSHADERATI_DECL
  #define GLENDFRAGMENTSHADERATI_DECL
  csGLENDFRAGMENTSHADERATI glEndFragmentShaderATI;
  #endif

  #ifndef GLPASSTEXCOORDATI_DECL
  #define GLPASSTEXCOORDATI_DECL
  csGLPASSTEXCOORDATI glPassTexCoordATI;
  #endif

  #ifndef GLSAMPLEMAPATI_DECL
  #define GLSAMPLEMAPATI_DECL
  csGLSAMPLEMAPATI glSampleMapATI;
  #endif

  #ifndef GLCOLORFRAGMENTOP1ATI_DECL
  #define GLCOLORFRAGMENTOP1ATI_DECL
  csGLCOLORFRAGMENTOP1ATI glColorFragmentOp1ATI;
  #endif

  #ifndef GLCOLORFRAGMENTOP2ATI_DECL
  #define GLCOLORFRAGMENTOP2ATI_DECL
  csGLCOLORFRAGMENTOP2ATI glColorFragmentOp2ATI;
  #endif

  #ifndef GLCOLORFRAGMENTOP3ATI_DECL
  #define GLCOLORFRAGMENTOP3ATI_DECL
  csGLCOLORFRAGMENTOP3ATI glColorFragmentOp3ATI;
  #endif

  #ifndef GLALPHAFRAGMENTOP1ATI_DECL
  #define GLALPHAFRAGMENTOP1ATI_DECL
  csGLALPHAFRAGMENTOP1ATI glAlphaFragmentOp1ATI;
  #endif

  #ifndef GLALPHAFRAGMENTOP2ATI_DECL
  #define GLALPHAFRAGMENTOP2ATI_DECL
  csGLALPHAFRAGMENTOP2ATI glAlphaFragmentOp2ATI;
  #endif

  #ifndef GLALPHAFRAGMENTOP3ATI_DECL
  #define GLALPHAFRAGMENTOP3ATI_DECL
  csGLALPHAFRAGMENTOP3ATI glAlphaFragmentOp3ATI;
  #endif

  #ifndef GLSETFRAGMENTSHADERCONSTANTATI_DECL
  #define GLSETFRAGMENTSHADERCONSTANTATI_DECL
  csGLSETFRAGMENTSHADERCONSTANTATI glSetFragmentShaderConstantATI;
  #endif


  // GL_ATI_pn_triangles
  #ifndef GLPNTRIANGLESIATI_DECL
  #define GLPNTRIANGLESIATI_DECL
  csGLPNTRIANGLESIATI glPNTrianglesiATI;
  #endif

  #ifndef GLPNTRIANGLESFATI_DECL
  #define GLPNTRIANGLESFATI_DECL
  csGLPNTRIANGLESFATI glPNTrianglesfATI;
  #endif


  // GL_ATI_texture_mirror_once

  // GL_ATI_vertex_array_object
  #ifndef GLNEWOBJECTBUFFERATI_DECL
  #define GLNEWOBJECTBUFFERATI_DECL
  csGLNEWOBJECTBUFFERATI glNewObjectBufferATI;
  #endif

  #ifndef GLISOBJECTBUFFERATI_DECL
  #define GLISOBJECTBUFFERATI_DECL
  csGLISOBJECTBUFFERATI glIsObjectBufferATI;
  #endif

  #ifndef GLUPDATEOBJECTBUFFERATI_DECL
  #define GLUPDATEOBJECTBUFFERATI_DECL
  csGLUPDATEOBJECTBUFFERATI glUpdateObjectBufferATI;
  #endif

  #ifndef GLGETOBJECTBUFFERFVATI_DECL
  #define GLGETOBJECTBUFFERFVATI_DECL
  csGLGETOBJECTBUFFERFVATI glGetObjectBufferfvATI;
  #endif

  #ifndef GLGETOBJECTBUFFERIVATI_DECL
  #define GLGETOBJECTBUFFERIVATI_DECL
  csGLGETOBJECTBUFFERIVATI glGetObjectBufferivATI;
  #endif

  #ifndef GLDELETEOBJECTBUFFERATI_DECL
  #define GLDELETEOBJECTBUFFERATI_DECL
  csGLDELETEOBJECTBUFFERATI glDeleteObjectBufferATI;
  #endif

  #ifndef GLARRAYOBJECTATI_DECL
  #define GLARRAYOBJECTATI_DECL
  csGLARRAYOBJECTATI glArrayObjectATI;
  #endif

  #ifndef GLGETARRAYOBJECTFVATI_DECL
  #define GLGETARRAYOBJECTFVATI_DECL
  csGLGETARRAYOBJECTFVATI glGetArrayObjectfvATI;
  #endif

  #ifndef GLGETARRAYOBJECTIVATI_DECL
  #define GLGETARRAYOBJECTIVATI_DECL
  csGLGETARRAYOBJECTIVATI glGetArrayObjectivATI;
  #endif

  #ifndef GLVARIANTARRAYOBJECTATI_DECL
  #define GLVARIANTARRAYOBJECTATI_DECL
  csGLVARIANTARRAYOBJECTATI glVariantArrayObjectATI;
  #endif

  #ifndef GLGETVARIANTARRAYOBJECTFVATI_DECL
  #define GLGETVARIANTARRAYOBJECTFVATI_DECL
  csGLGETVARIANTARRAYOBJECTFVATI glGetVariantArrayObjectfvATI;
  #endif

  #ifndef GLGETVARIANTARRAYOBJECTIVATI_DECL
  #define GLGETVARIANTARRAYOBJECTIVATI_DECL
  csGLGETVARIANTARRAYOBJECTIVATI glGetVariantArrayObjectivATI;
  #endif


  // GL_ATI_vertex_streams
  #ifndef GLVERTEXSTREAM1S_DECL
  #define GLVERTEXSTREAM1S_DECL
  csGLVERTEXSTREAM1S glVertexStream1s;
  #endif

  #ifndef GLVERTEXSTREAM1I_DECL
  #define GLVERTEXSTREAM1I_DECL
  csGLVERTEXSTREAM1I glVertexStream1i;
  #endif

  #ifndef GLVERTEXSTREAM1F_DECL
  #define GLVERTEXSTREAM1F_DECL
  csGLVERTEXSTREAM1F glVertexStream1f;
  #endif

  #ifndef GLVERTEXSTREAM1D_DECL
  #define GLVERTEXSTREAM1D_DECL
  csGLVERTEXSTREAM1D glVertexStream1d;
  #endif

  #ifndef GLVERTEXSTREAM1SV_DECL
  #define GLVERTEXSTREAM1SV_DECL
  csGLVERTEXSTREAM1SV glVertexStream1sv;
  #endif

  #ifndef GLVERTEXSTREAM1IV_DECL
  #define GLVERTEXSTREAM1IV_DECL
  csGLVERTEXSTREAM1IV glVertexStream1iv;
  #endif

  #ifndef GLVERTEXSTREAM1FV_DECL
  #define GLVERTEXSTREAM1FV_DECL
  csGLVERTEXSTREAM1FV glVertexStream1fv;
  #endif

  #ifndef GLVERTEXSTREAM1DV_DECL
  #define GLVERTEXSTREAM1DV_DECL
  csGLVERTEXSTREAM1DV glVertexStream1dv;
  #endif

  #ifndef GLVERTEXSTREAM2S_DECL
  #define GLVERTEXSTREAM2S_DECL
  csGLVERTEXSTREAM2S glVertexStream2s;
  #endif

  #ifndef GLVERTEXSTREAM2I_DECL
  #define GLVERTEXSTREAM2I_DECL
  csGLVERTEXSTREAM2I glVertexStream2i;
  #endif

  #ifndef GLVERTEXSTREAM2F_DECL
  #define GLVERTEXSTREAM2F_DECL
  csGLVERTEXSTREAM2F glVertexStream2f;
  #endif

  #ifndef GLVERTEXSTREAM2D_DECL
  #define GLVERTEXSTREAM2D_DECL
  csGLVERTEXSTREAM2D glVertexStream2d;
  #endif

  #ifndef GLVERTEXSTREAM2SV_DECL
  #define GLVERTEXSTREAM2SV_DECL
  csGLVERTEXSTREAM2SV glVertexStream2sv;
  #endif

  #ifndef GLVERTEXSTREAM2IV_DECL
  #define GLVERTEXSTREAM2IV_DECL
  csGLVERTEXSTREAM2IV glVertexStream2iv;
  #endif

  #ifndef GLVERTEXSTREAM2FV_DECL
  #define GLVERTEXSTREAM2FV_DECL
  csGLVERTEXSTREAM2FV glVertexStream2fv;
  #endif

  #ifndef GLVERTEXSTREAM2DV_DECL
  #define GLVERTEXSTREAM2DV_DECL
  csGLVERTEXSTREAM2DV glVertexStream2dv;
  #endif

  #ifndef GLVERTEXSTREAM3S_DECL
  #define GLVERTEXSTREAM3S_DECL
  csGLVERTEXSTREAM3S glVertexStream3s;
  #endif

  #ifndef GLVERTEXSTREAM3I_DECL
  #define GLVERTEXSTREAM3I_DECL
  csGLVERTEXSTREAM3I glVertexStream3i;
  #endif

  #ifndef GLVERTEXSTREAM3F_DECL
  #define GLVERTEXSTREAM3F_DECL
  csGLVERTEXSTREAM3F glVertexStream3f;
  #endif

  #ifndef GLVERTEXSTREAM3D_DECL
  #define GLVERTEXSTREAM3D_DECL
  csGLVERTEXSTREAM3D glVertexStream3d;
  #endif

  #ifndef GLVERTEXSTREAM3SV_DECL
  #define GLVERTEXSTREAM3SV_DECL
  csGLVERTEXSTREAM3SV glVertexStream3sv;
  #endif

  #ifndef GLVERTEXSTREAM3IV_DECL
  #define GLVERTEXSTREAM3IV_DECL
  csGLVERTEXSTREAM3IV glVertexStream3iv;
  #endif

  #ifndef GLVERTEXSTREAM3FV_DECL
  #define GLVERTEXSTREAM3FV_DECL
  csGLVERTEXSTREAM3FV glVertexStream3fv;
  #endif

  #ifndef GLVERTEXSTREAM3DV_DECL
  #define GLVERTEXSTREAM3DV_DECL
  csGLVERTEXSTREAM3DV glVertexStream3dv;
  #endif

  #ifndef GLVERTEXSTREAM4S_DECL
  #define GLVERTEXSTREAM4S_DECL
  csGLVERTEXSTREAM4S glVertexStream4s;
  #endif

  #ifndef GLVERTEXSTREAM4I_DECL
  #define GLVERTEXSTREAM4I_DECL
  csGLVERTEXSTREAM4I glVertexStream4i;
  #endif

  #ifndef GLVERTEXSTREAM4F_DECL
  #define GLVERTEXSTREAM4F_DECL
  csGLVERTEXSTREAM4F glVertexStream4f;
  #endif

  #ifndef GLVERTEXSTREAM4D_DECL
  #define GLVERTEXSTREAM4D_DECL
  csGLVERTEXSTREAM4D glVertexStream4d;
  #endif

  #ifndef GLVERTEXSTREAM4SV_DECL
  #define GLVERTEXSTREAM4SV_DECL
  csGLVERTEXSTREAM4SV glVertexStream4sv;
  #endif

  #ifndef GLVERTEXSTREAM4IV_DECL
  #define GLVERTEXSTREAM4IV_DECL
  csGLVERTEXSTREAM4IV glVertexStream4iv;
  #endif

  #ifndef GLVERTEXSTREAM4FV_DECL
  #define GLVERTEXSTREAM4FV_DECL
  csGLVERTEXSTREAM4FV glVertexStream4fv;
  #endif

  #ifndef GLVERTEXSTREAM4DV_DECL
  #define GLVERTEXSTREAM4DV_DECL
  csGLVERTEXSTREAM4DV glVertexStream4dv;
  #endif

  #ifndef GLNORMALSTREAM3B_DECL
  #define GLNORMALSTREAM3B_DECL
  csGLNORMALSTREAM3B glNormalStream3b;
  #endif

  #ifndef GLNORMALSTREAM3S_DECL
  #define GLNORMALSTREAM3S_DECL
  csGLNORMALSTREAM3S glNormalStream3s;
  #endif

  #ifndef GLNORMALSTREAM3I_DECL
  #define GLNORMALSTREAM3I_DECL
  csGLNORMALSTREAM3I glNormalStream3i;
  #endif

  #ifndef GLNORMALSTREAM3F_DECL
  #define GLNORMALSTREAM3F_DECL
  csGLNORMALSTREAM3F glNormalStream3f;
  #endif

  #ifndef GLNORMALSTREAM3D_DECL
  #define GLNORMALSTREAM3D_DECL
  csGLNORMALSTREAM3D glNormalStream3d;
  #endif

  #ifndef GLNORMALSTREAM3BV_DECL
  #define GLNORMALSTREAM3BV_DECL
  csGLNORMALSTREAM3BV glNormalStream3bv;
  #endif

  #ifndef GLNORMALSTREAM3SV_DECL
  #define GLNORMALSTREAM3SV_DECL
  csGLNORMALSTREAM3SV glNormalStream3sv;
  #endif

  #ifndef GLNORMALSTREAM3IV_DECL
  #define GLNORMALSTREAM3IV_DECL
  csGLNORMALSTREAM3IV glNormalStream3iv;
  #endif

  #ifndef GLNORMALSTREAM3FV_DECL
  #define GLNORMALSTREAM3FV_DECL
  csGLNORMALSTREAM3FV glNormalStream3fv;
  #endif

  #ifndef GLNORMALSTREAM3DV_DECL
  #define GLNORMALSTREAM3DV_DECL
  csGLNORMALSTREAM3DV glNormalStream3dv;
  #endif

  #ifndef GLCLIENTACTIVEVERTEXSTREAM_DECL
  #define GLCLIENTACTIVEVERTEXSTREAM_DECL
  csGLCLIENTACTIVEVERTEXSTREAM glClientActiveVertexStream;
  #endif

  #ifndef GLVERTEXBLENDENVI_DECL
  #define GLVERTEXBLENDENVI_DECL
  csGLVERTEXBLENDENVI glVertexBlendEnvi;
  #endif

  #ifndef GLVERTEXBLENDENVF_DECL
  #define GLVERTEXBLENDENVF_DECL
  csGLVERTEXBLENDENVF glVertexBlendEnvf;
  #endif


  // WGL_I3D_image_buffer
  #ifndef WGLCREATEIMAGEBUFFERI3D_DECL
  #define WGLCREATEIMAGEBUFFERI3D_DECL
  csWGLCREATEIMAGEBUFFERI3D wglCreateImageBufferI3D;
  #endif

  #ifndef WGLDESTROYIMAGEBUFFERI3D_DECL
  #define WGLDESTROYIMAGEBUFFERI3D_DECL
  csWGLDESTROYIMAGEBUFFERI3D wglDestroyImageBufferI3D;
  #endif

  #ifndef WGLASSOCIATEIMAGEBUFFEREVENTSI3D_DECL
  #define WGLASSOCIATEIMAGEBUFFEREVENTSI3D_DECL
  csWGLASSOCIATEIMAGEBUFFEREVENTSI3D wglAssociateImageBufferEventsI3D;
  #endif

  #ifndef WGLRELEASEIMAGEBUFFEREVENTSI3D_DECL
  #define WGLRELEASEIMAGEBUFFEREVENTSI3D_DECL
  csWGLRELEASEIMAGEBUFFEREVENTSI3D wglReleaseImageBufferEventsI3D;
  #endif


  // WGL_I3D_swap_frame_lock
  #ifndef WGLENABLEFRAMELOCKI3D_DECL
  #define WGLENABLEFRAMELOCKI3D_DECL
  csWGLENABLEFRAMELOCKI3D wglEnableFrameLockI3D;
  #endif

  #ifndef WGLDISABLEFRAMELOCKI3D_DECL
  #define WGLDISABLEFRAMELOCKI3D_DECL
  csWGLDISABLEFRAMELOCKI3D wglDisableFrameLockI3D;
  #endif

  #ifndef WGLISENABLEDFRAMELOCKI3D_DECL
  #define WGLISENABLEDFRAMELOCKI3D_DECL
  csWGLISENABLEDFRAMELOCKI3D wglIsEnabledFrameLockI3D;
  #endif

  #ifndef WGLQUERYFRAMELOCKMASTERI3D_DECL
  #define WGLQUERYFRAMELOCKMASTERI3D_DECL
  csWGLQUERYFRAMELOCKMASTERI3D wglQueryFrameLockMasterI3D;
  #endif


  // WGL_I3D_swap_frame_usage
  #ifndef WGLGETFRAMEUSAGEI3D_DECL
  #define WGLGETFRAMEUSAGEI3D_DECL
  csWGLGETFRAMEUSAGEI3D wglGetFrameUsageI3D;
  #endif

  #ifndef WGLBEGINFRAMETRACKINGI3D_DECL
  #define WGLBEGINFRAMETRACKINGI3D_DECL
  csWGLBEGINFRAMETRACKINGI3D wglBeginFrameTrackingI3D;
  #endif

  #ifndef WGLENDFRAMETRACKINGI3D_DECL
  #define WGLENDFRAMETRACKINGI3D_DECL
  csWGLENDFRAMETRACKINGI3D wglEndFrameTrackingI3D;
  #endif

  #ifndef WGLQUERYFRAMETRACKINGI3D_DECL
  #define WGLQUERYFRAMETRACKINGI3D_DECL
  csWGLQUERYFRAMETRACKINGI3D wglQueryFrameTrackingI3D;
  #endif


  // GL_3DFX_texture_compression_FXT1

  // GL_IBM_cull_vertex

  // GL_IBM_multimode_draw_arrays
  #ifndef GLMULTIMODEDRAWARRAYSIBM_DECL
  #define GLMULTIMODEDRAWARRAYSIBM_DECL
  csGLMULTIMODEDRAWARRAYSIBM glMultiModeDrawArraysIBM;
  #endif

  #ifndef GLMULTIMODEDRAWELEMENTSIBM_DECL
  #define GLMULTIMODEDRAWELEMENTSIBM_DECL
  csGLMULTIMODEDRAWELEMENTSIBM glMultiModeDrawElementsIBM;
  #endif


  // GL_IBM_raster_pos_clip

  // GL_IBM_texture_mirrored_repeat

  // GL_IBM_vertex_array_lists
  #ifndef GLCOLORPOINTERLISTIBM_DECL
  #define GLCOLORPOINTERLISTIBM_DECL
  csGLCOLORPOINTERLISTIBM glColorPointerListIBM;
  #endif

  #ifndef GLSECONDARYCOLORPOINTERLISTIBM_DECL
  #define GLSECONDARYCOLORPOINTERLISTIBM_DECL
  csGLSECONDARYCOLORPOINTERLISTIBM glSecondaryColorPointerListIBM;
  #endif

  #ifndef GLEDGEFLAGPOINTERLISTIBM_DECL
  #define GLEDGEFLAGPOINTERLISTIBM_DECL
  csGLEDGEFLAGPOINTERLISTIBM glEdgeFlagPointerListIBM;
  #endif

  #ifndef GLFOGCOORDPOINTERLISTIBM_DECL
  #define GLFOGCOORDPOINTERLISTIBM_DECL
  csGLFOGCOORDPOINTERLISTIBM glFogCoordPointerListIBM;
  #endif

  #ifndef GLNORMALPOINTERLISTIBM_DECL
  #define GLNORMALPOINTERLISTIBM_DECL
  csGLNORMALPOINTERLISTIBM glNormalPointerListIBM;
  #endif

  #ifndef GLTEXCOORDPOINTERLISTIBM_DECL
  #define GLTEXCOORDPOINTERLISTIBM_DECL
  csGLTEXCOORDPOINTERLISTIBM glTexCoordPointerListIBM;
  #endif

  #ifndef GLVERTEXPOINTERLISTIBM_DECL
  #define GLVERTEXPOINTERLISTIBM_DECL
  csGLVERTEXPOINTERLISTIBM glVertexPointerListIBM;
  #endif


  // GL_MESA_resize_buffers
  #ifndef GLRESIZEBUFFERSMESA_DECL
  #define GLRESIZEBUFFERSMESA_DECL
  csGLRESIZEBUFFERSMESA glResizeBuffersMESA;
  #endif


  // GL_MESA_window_pos
  #ifndef GLWINDOWPOS2DMESA_DECL
  #define GLWINDOWPOS2DMESA_DECL
  csGLWINDOWPOS2DMESA glWindowPos2dMESA;
  #endif

  #ifndef GLWINDOWPOS2FMESA_DECL
  #define GLWINDOWPOS2FMESA_DECL
  csGLWINDOWPOS2FMESA glWindowPos2fMESA;
  #endif

  #ifndef GLWINDOWPOS2IMESA_DECL
  #define GLWINDOWPOS2IMESA_DECL
  csGLWINDOWPOS2IMESA glWindowPos2iMESA;
  #endif

  #ifndef GLWINDOWPOS2SMESA_DECL
  #define GLWINDOWPOS2SMESA_DECL
  csGLWINDOWPOS2SMESA glWindowPos2sMESA;
  #endif

  #ifndef GLWINDOWPOS2IVMESA_DECL
  #define GLWINDOWPOS2IVMESA_DECL
  csGLWINDOWPOS2IVMESA glWindowPos2ivMESA;
  #endif

  #ifndef GLWINDOWPOS2SVMESA_DECL
  #define GLWINDOWPOS2SVMESA_DECL
  csGLWINDOWPOS2SVMESA glWindowPos2svMESA;
  #endif

  #ifndef GLWINDOWPOS2FVMESA_DECL
  #define GLWINDOWPOS2FVMESA_DECL
  csGLWINDOWPOS2FVMESA glWindowPos2fvMESA;
  #endif

  #ifndef GLWINDOWPOS2DVMESA_DECL
  #define GLWINDOWPOS2DVMESA_DECL
  csGLWINDOWPOS2DVMESA glWindowPos2dvMESA;
  #endif

  #ifndef GLWINDOWPOS3IMESA_DECL
  #define GLWINDOWPOS3IMESA_DECL
  csGLWINDOWPOS3IMESA glWindowPos3iMESA;
  #endif

  #ifndef GLWINDOWPOS3SMESA_DECL
  #define GLWINDOWPOS3SMESA_DECL
  csGLWINDOWPOS3SMESA glWindowPos3sMESA;
  #endif

  #ifndef GLWINDOWPOS3FMESA_DECL
  #define GLWINDOWPOS3FMESA_DECL
  csGLWINDOWPOS3FMESA glWindowPos3fMESA;
  #endif

  #ifndef GLWINDOWPOS3DMESA_DECL
  #define GLWINDOWPOS3DMESA_DECL
  csGLWINDOWPOS3DMESA glWindowPos3dMESA;
  #endif

  #ifndef GLWINDOWPOS3IVMESA_DECL
  #define GLWINDOWPOS3IVMESA_DECL
  csGLWINDOWPOS3IVMESA glWindowPos3ivMESA;
  #endif

  #ifndef GLWINDOWPOS3SVMESA_DECL
  #define GLWINDOWPOS3SVMESA_DECL
  csGLWINDOWPOS3SVMESA glWindowPos3svMESA;
  #endif

  #ifndef GLWINDOWPOS3FVMESA_DECL
  #define GLWINDOWPOS3FVMESA_DECL
  csGLWINDOWPOS3FVMESA glWindowPos3fvMESA;
  #endif

  #ifndef GLWINDOWPOS3DVMESA_DECL
  #define GLWINDOWPOS3DVMESA_DECL
  csGLWINDOWPOS3DVMESA glWindowPos3dvMESA;
  #endif

  #ifndef GLWINDOWPOS4IMESA_DECL
  #define GLWINDOWPOS4IMESA_DECL
  csGLWINDOWPOS4IMESA glWindowPos4iMESA;
  #endif

  #ifndef GLWINDOWPOS4SMESA_DECL
  #define GLWINDOWPOS4SMESA_DECL
  csGLWINDOWPOS4SMESA glWindowPos4sMESA;
  #endif

  #ifndef GLWINDOWPOS4FMESA_DECL
  #define GLWINDOWPOS4FMESA_DECL
  csGLWINDOWPOS4FMESA glWindowPos4fMESA;
  #endif

  #ifndef GLWINDOWPOS4DMESA_DECL
  #define GLWINDOWPOS4DMESA_DECL
  csGLWINDOWPOS4DMESA glWindowPos4dMESA;
  #endif

  #ifndef GLWINDOWPOS4IVMESA_DECL
  #define GLWINDOWPOS4IVMESA_DECL
  csGLWINDOWPOS4IVMESA glWindowPos4ivMESA;
  #endif

  #ifndef GLWINDOWPOS4SVMESA_DECL
  #define GLWINDOWPOS4SVMESA_DECL
  csGLWINDOWPOS4SVMESA glWindowPos4svMESA;
  #endif

  #ifndef GLWINDOWPOS4FVMESA_DECL
  #define GLWINDOWPOS4FVMESA_DECL
  csGLWINDOWPOS4FVMESA glWindowPos4fvMESA;
  #endif

  #ifndef GLWINDOWPOS4DVMESA_DECL
  #define GLWINDOWPOS4DVMESA_DECL
  csGLWINDOWPOS4DVMESA glWindowPos4dvMESA;
  #endif


  // GL_OML_interlace

  // GL_OML_resample

  // GL_OML_subsample

  // GL_SGIS_generate_mipmap

  // GL_SGIS_multisample
  #ifndef GLSAMPLEMASKSGIS_DECL
  #define GLSAMPLEMASKSGIS_DECL
  csGLSAMPLEMASKSGIS glSampleMaskSGIS;
  #endif

  #ifndef GLSAMPLEPATTERNSGIS_DECL
  #define GLSAMPLEPATTERNSGIS_DECL
  csGLSAMPLEPATTERNSGIS glSamplePatternSGIS;
  #endif


  // GL_SGIS_pixel_texture
  #ifndef GLPIXELTEXGENPARAMETERISGIS_DECL
  #define GLPIXELTEXGENPARAMETERISGIS_DECL
  csGLPIXELTEXGENPARAMETERISGIS glPixelTexGenParameteriSGIS;
  #endif

  #ifndef GLPIXELTEXGENPARAMETERFSGIS_DECL
  #define GLPIXELTEXGENPARAMETERFSGIS_DECL
  csGLPIXELTEXGENPARAMETERFSGIS glPixelTexGenParameterfSGIS;
  #endif

  #ifndef GLGETPIXELTEXGENPARAMETERIVSGIS_DECL
  #define GLGETPIXELTEXGENPARAMETERIVSGIS_DECL
  csGLGETPIXELTEXGENPARAMETERIVSGIS glGetPixelTexGenParameterivSGIS;
  #endif

  #ifndef GLGETPIXELTEXGENPARAMETERFVSGIS_DECL
  #define GLGETPIXELTEXGENPARAMETERFVSGIS_DECL
  csGLGETPIXELTEXGENPARAMETERFVSGIS glGetPixelTexGenParameterfvSGIS;
  #endif


  // GL_SGIS_texture_border_clamp

  // GL_SGIS_texture_color_mask
  #ifndef GLTEXTURECOLORMASKSGIS_DECL
  #define GLTEXTURECOLORMASKSGIS_DECL
  csGLTEXTURECOLORMASKSGIS glTextureColorMaskSGIS;
  #endif


  // GL_SGIS_texture_edge_clamp

  // GL_SGIS_texture_lod

  // GL_SGIS_depth_texture

  // GL_SGIX_fog_offset

  // GL_SGIX_interlace

  // GL_SGIX_shadow_ambient

  // GL_SGI_color_matrix

  // GL_SGI_color_table
  #ifndef GLCOLORTABLESGI_DECL
  #define GLCOLORTABLESGI_DECL
  csGLCOLORTABLESGI glColorTableSGI;
  #endif

  #ifndef GLCOPYCOLORTABLESGI_DECL
  #define GLCOPYCOLORTABLESGI_DECL
  csGLCOPYCOLORTABLESGI glCopyColorTableSGI;
  #endif

  #ifndef GLCOLORTABLEPARAMETERIVSGI_DECL
  #define GLCOLORTABLEPARAMETERIVSGI_DECL
  csGLCOLORTABLEPARAMETERIVSGI glColorTableParameterivSGI;
  #endif

  #ifndef GLCOLORTABLEPARAMETERFVSGI_DECL
  #define GLCOLORTABLEPARAMETERFVSGI_DECL
  csGLCOLORTABLEPARAMETERFVSGI glColorTableParameterfvSGI;
  #endif

  #ifndef GLGETCOLORTABLESGI_DECL
  #define GLGETCOLORTABLESGI_DECL
  csGLGETCOLORTABLESGI glGetColorTableSGI;
  #endif

  #ifndef GLGETCOLORTABLEPARAMETERIVSGI_DECL
  #define GLGETCOLORTABLEPARAMETERIVSGI_DECL
  csGLGETCOLORTABLEPARAMETERIVSGI glGetColorTableParameterivSGI;
  #endif

  #ifndef GLGETCOLORTABLEPARAMETERFVSGI_DECL
  #define GLGETCOLORTABLEPARAMETERFVSGI_DECL
  csGLGETCOLORTABLEPARAMETERFVSGI glGetColorTableParameterfvSGI;
  #endif


  // GL_SGI_texture_color_table

  // GL_SUN_vertex
  #ifndef GLCOLOR4UBVERTEX2FSUN_DECL
  #define GLCOLOR4UBVERTEX2FSUN_DECL
  csGLCOLOR4UBVERTEX2FSUN glColor4ubVertex2fSUN;
  #endif

  #ifndef GLCOLOR4UBVERTEX2FVSUN_DECL
  #define GLCOLOR4UBVERTEX2FVSUN_DECL
  csGLCOLOR4UBVERTEX2FVSUN glColor4ubVertex2fvSUN;
  #endif

  #ifndef GLCOLOR4UBVERTEX3FSUN_DECL
  #define GLCOLOR4UBVERTEX3FSUN_DECL
  csGLCOLOR4UBVERTEX3FSUN glColor4ubVertex3fSUN;
  #endif

  #ifndef GLCOLOR4UBVERTEX3FVSUN_DECL
  #define GLCOLOR4UBVERTEX3FVSUN_DECL
  csGLCOLOR4UBVERTEX3FVSUN glColor4ubVertex3fvSUN;
  #endif

  #ifndef GLCOLOR3FVERTEX3FSUN_DECL
  #define GLCOLOR3FVERTEX3FSUN_DECL
  csGLCOLOR3FVERTEX3FSUN glColor3fVertex3fSUN;
  #endif

  #ifndef GLCOLOR3FVERTEX3FVSUN_DECL
  #define GLCOLOR3FVERTEX3FVSUN_DECL
  csGLCOLOR3FVERTEX3FVSUN glColor3fVertex3fvSUN;
  #endif

  #ifndef GLNORMAL3FVERTEX3FSUN_DECL
  #define GLNORMAL3FVERTEX3FSUN_DECL
  csGLNORMAL3FVERTEX3FSUN glNormal3fVertex3fSUN;
  #endif

  #ifndef GLNORMAL3FVERTEX3FVSUN_DECL
  #define GLNORMAL3FVERTEX3FVSUN_DECL
  csGLNORMAL3FVERTEX3FVSUN glNormal3fVertex3fvSUN;
  #endif

  #ifndef GLCOLOR4FNORMAL3FVERTEX3FSUN_DECL
  #define GLCOLOR4FNORMAL3FVERTEX3FSUN_DECL
  csGLCOLOR4FNORMAL3FVERTEX3FSUN glColor4fNormal3fVertex3fSUN;
  #endif

  #ifndef GLCOLOR4FNORMAL3FVERTEX3FVSUN_DECL
  #define GLCOLOR4FNORMAL3FVERTEX3FVSUN_DECL
  csGLCOLOR4FNORMAL3FVERTEX3FVSUN glColor4fNormal3fVertex3fvSUN;
  #endif

  #ifndef GLTEXCOORD2FVERTEX3FSUN_DECL
  #define GLTEXCOORD2FVERTEX3FSUN_DECL
  csGLTEXCOORD2FVERTEX3FSUN glTexCoord2fVertex3fSUN;
  #endif

  #ifndef GLTEXCOORD2FVERTEX3FVSUN_DECL
  #define GLTEXCOORD2FVERTEX3FVSUN_DECL
  csGLTEXCOORD2FVERTEX3FVSUN glTexCoord2fVertex3fvSUN;
  #endif

  #ifndef GLTEXCOORD4FVERTEX4FSUN_DECL
  #define GLTEXCOORD4FVERTEX4FSUN_DECL
  csGLTEXCOORD4FVERTEX4FSUN glTexCoord4fVertex4fSUN;
  #endif

  #ifndef GLTEXCOORD4FVERTEX4FVSUN_DECL
  #define GLTEXCOORD4FVERTEX4FVSUN_DECL
  csGLTEXCOORD4FVERTEX4FVSUN glTexCoord4fVertex4fvSUN;
  #endif

  #ifndef GLTEXCOORD2FCOLOR4UBVERTEX3FSUN_DECL
  #define GLTEXCOORD2FCOLOR4UBVERTEX3FSUN_DECL
  csGLTEXCOORD2FCOLOR4UBVERTEX3FSUN glTexCoord2fColor4ubVertex3fSUN;
  #endif

  #ifndef GLTEXCOORD2FCOLOR4UBVERTEX3FVSUN_DECL
  #define GLTEXCOORD2FCOLOR4UBVERTEX3FVSUN_DECL
  csGLTEXCOORD2FCOLOR4UBVERTEX3FVSUN glTexCoord2fColor4ubVertex3fvSUN;
  #endif

  #ifndef GLTEXCOORD2FCOLOR3FVERTEX3FSUN_DECL
  #define GLTEXCOORD2FCOLOR3FVERTEX3FSUN_DECL
  csGLTEXCOORD2FCOLOR3FVERTEX3FSUN glTexCoord2fColor3fVertex3fSUN;
  #endif

  #ifndef GLTEXCOORD2FCOLOR3FVERTEX3FVSUN_DECL
  #define GLTEXCOORD2FCOLOR3FVERTEX3FVSUN_DECL
  csGLTEXCOORD2FCOLOR3FVERTEX3FVSUN glTexCoord2fColor3fVertex3fvSUN;
  #endif

  #ifndef GLTEXCOORD2FNORMAL3FVERTEX3FSUN_DECL
  #define GLTEXCOORD2FNORMAL3FVERTEX3FSUN_DECL
  csGLTEXCOORD2FNORMAL3FVERTEX3FSUN glTexCoord2fNormal3fVertex3fSUN;
  #endif

  #ifndef GLTEXCOORD2FNORMAL3FVERTEX3FVSUN_DECL
  #define GLTEXCOORD2FNORMAL3FVERTEX3FVSUN_DECL
  csGLTEXCOORD2FNORMAL3FVERTEX3FVSUN glTexCoord2fNormal3fVertex3fvSUN;
  #endif

  #ifndef GLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUN_DECL
  #define GLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUN_DECL
  csGLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUN glTexCoord2fColor4fNormal3fVertex3fSUN;
  #endif

  #ifndef GLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUN_DECL
  #define GLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUN_DECL
  csGLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUN glTexCoord2fColor4fNormal3fVertex3fvSUN;
  #endif

  #ifndef GLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FSUN_DECL
  #define GLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FSUN_DECL
  csGLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FSUN glTexCoord4fColor4fNormal3fVertex4fSUN;
  #endif

  #ifndef GLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FVSUN_DECL
  #define GLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FVSUN_DECL
  csGLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FVSUN glTexCoord4fColor4fNormal3fVertex4fvSUN;
  #endif

  #ifndef GLREPLACEMENTCODEUIVERTEX3FSUN_DECL
  #define GLREPLACEMENTCODEUIVERTEX3FSUN_DECL
  csGLREPLACEMENTCODEUIVERTEX3FSUN glReplacementCodeuiVertex3fSUN;
  #endif

  #ifndef GLREPLACEMENTCODEUIVERTEX3FVSUN_DECL
  #define GLREPLACEMENTCODEUIVERTEX3FVSUN_DECL
  csGLREPLACEMENTCODEUIVERTEX3FVSUN glReplacementCodeuiVertex3fvSUN;
  #endif

  #ifndef GLREPLACEMENTCODEUICOLOR4UBVERTEX3FSUN_DECL
  #define GLREPLACEMENTCODEUICOLOR4UBVERTEX3FSUN_DECL
  csGLREPLACEMENTCODEUICOLOR4UBVERTEX3FSUN glReplacementCodeuiColor4ubVertex3fSUN;
  #endif

  #ifndef GLREPLACEMENTCODEUICOLOR4UBVERTEX3FVSUN_DECL
  #define GLREPLACEMENTCODEUICOLOR4UBVERTEX3FVSUN_DECL
  csGLREPLACEMENTCODEUICOLOR4UBVERTEX3FVSUN glReplacementCodeuiColor4ubVertex3fvSUN;
  #endif

  #ifndef GLREPLACEMENTCODEUICOLOR3FVERTEX3FSUN_DECL
  #define GLREPLACEMENTCODEUICOLOR3FVERTEX3FSUN_DECL
  csGLREPLACEMENTCODEUICOLOR3FVERTEX3FSUN glReplacementCodeuiColor3fVertex3fSUN;
  #endif

  #ifndef GLREPLACEMENTCODEUICOLOR3FVERTEX3FVSUN_DECL
  #define GLREPLACEMENTCODEUICOLOR3FVERTEX3FVSUN_DECL
  csGLREPLACEMENTCODEUICOLOR3FVERTEX3FVSUN glReplacementCodeuiColor3fVertex3fvSUN;
  #endif

  #ifndef GLREPLACEMENTCODEUINORMAL3FVERTEX3FSUN_DECL
  #define GLREPLACEMENTCODEUINORMAL3FVERTEX3FSUN_DECL
  csGLREPLACEMENTCODEUINORMAL3FVERTEX3FSUN glReplacementCodeuiNormal3fVertex3fSUN;
  #endif

  #ifndef GLREPLACEMENTCODEUINORMAL3FVERTEX3FVSUN_DECL
  #define GLREPLACEMENTCODEUINORMAL3FVERTEX3FVSUN_DECL
  csGLREPLACEMENTCODEUINORMAL3FVERTEX3FVSUN glReplacementCodeuiNormal3fVertex3fvSUN;
  #endif

  #ifndef GLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FSUN_DECL
  #define GLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FSUN_DECL
  csGLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FSUN glReplacementCodeuiColor4fNormal3fVertex3fSUN;
  #endif

  #ifndef GLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FVSUN_DECL
  #define GLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FVSUN_DECL
  csGLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FVSUN glReplacementCodeuiColor4fNormal3fVertex3fvSUN;
  #endif

  #ifndef GLREPLACEMENTCODEUITEXCOORD2FVERTEX3FSUN_DECL
  #define GLREPLACEMENTCODEUITEXCOORD2FVERTEX3FSUN_DECL
  csGLREPLACEMENTCODEUITEXCOORD2FVERTEX3FSUN glReplacementCodeuiTexCoord2fVertex3fSUN;
  #endif

  #ifndef GLREPLACEMENTCODEUITEXCOORD2FVERTEX3FVSUN_DECL
  #define GLREPLACEMENTCODEUITEXCOORD2FVERTEX3FVSUN_DECL
  csGLREPLACEMENTCODEUITEXCOORD2FVERTEX3FVSUN glReplacementCodeuiTexCoord2fVertex3fvSUN;
  #endif

  #ifndef GLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FSUN_DECL
  #define GLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FSUN_DECL
  csGLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FSUN glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN;
  #endif

  #ifndef GLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FVSUN_DECL
  #define GLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FVSUN_DECL
  csGLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FVSUN glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN;
  #endif

  #ifndef GLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUN_DECL
  #define GLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUN_DECL
  csGLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUN glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN;
  #endif

  #ifndef GLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUN_DECL
  #define GLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUN_DECL
  csGLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUN glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN;
  #endif


  // GL_ARB_fragment_program
  #ifndef GLPROGRAMSTRINGARB_DECL
  #define GLPROGRAMSTRINGARB_DECL
  csGLPROGRAMSTRINGARB glProgramStringARB;
  #endif

  #ifndef GLBINDPROGRAMARB_DECL
  #define GLBINDPROGRAMARB_DECL
  csGLBINDPROGRAMARB glBindProgramARB;
  #endif

  #ifndef GLDELETEPROGRAMSARB_DECL
  #define GLDELETEPROGRAMSARB_DECL
  csGLDELETEPROGRAMSARB glDeleteProgramsARB;
  #endif

  #ifndef GLGENPROGRAMSARB_DECL
  #define GLGENPROGRAMSARB_DECL
  csGLGENPROGRAMSARB glGenProgramsARB;
  #endif

  #ifndef GLPROGRAMENVPARAMETER4DARB_DECL
  #define GLPROGRAMENVPARAMETER4DARB_DECL
  csGLPROGRAMENVPARAMETER4DARB glProgramEnvParameter4dARB;
  #endif

  #ifndef GLPROGRAMENVPARAMETER4DVARB_DECL
  #define GLPROGRAMENVPARAMETER4DVARB_DECL
  csGLPROGRAMENVPARAMETER4DVARB glProgramEnvParameter4dvARB;
  #endif

  #ifndef GLPROGRAMENVPARAMETER4FARB_DECL
  #define GLPROGRAMENVPARAMETER4FARB_DECL
  csGLPROGRAMENVPARAMETER4FARB glProgramEnvParameter4fARB;
  #endif

  #ifndef GLPROGRAMENVPARAMETER4FVARB_DECL
  #define GLPROGRAMENVPARAMETER4FVARB_DECL
  csGLPROGRAMENVPARAMETER4FVARB glProgramEnvParameter4fvARB;
  #endif

  #ifndef GLPROGRAMLOCALPARAMETER4DARB_DECL
  #define GLPROGRAMLOCALPARAMETER4DARB_DECL
  csGLPROGRAMLOCALPARAMETER4DARB glProgramLocalParameter4dARB;
  #endif

  #ifndef GLPROGRAMLOCALPARAMETER4DVARB_DECL
  #define GLPROGRAMLOCALPARAMETER4DVARB_DECL
  csGLPROGRAMLOCALPARAMETER4DVARB glProgramLocalParameter4dvARB;
  #endif

  #ifndef GLPROGRAMLOCALPARAMETER4FARB_DECL
  #define GLPROGRAMLOCALPARAMETER4FARB_DECL
  csGLPROGRAMLOCALPARAMETER4FARB glProgramLocalParameter4fARB;
  #endif

  #ifndef GLPROGRAMLOCALPARAMETER4FVARB_DECL
  #define GLPROGRAMLOCALPARAMETER4FVARB_DECL
  csGLPROGRAMLOCALPARAMETER4FVARB glProgramLocalParameter4fvARB;
  #endif

  #ifndef GLGETPROGRAMENVPARAMETERDVARB_DECL
  #define GLGETPROGRAMENVPARAMETERDVARB_DECL
  csGLGETPROGRAMENVPARAMETERDVARB glGetProgramEnvParameterdvARB;
  #endif

  #ifndef GLGETPROGRAMENVPARAMETERFVARB_DECL
  #define GLGETPROGRAMENVPARAMETERFVARB_DECL
  csGLGETPROGRAMENVPARAMETERFVARB glGetProgramEnvParameterfvARB;
  #endif

  #ifndef GLGETPROGRAMLOCALPARAMETERDVARB_DECL
  #define GLGETPROGRAMLOCALPARAMETERDVARB_DECL
  csGLGETPROGRAMLOCALPARAMETERDVARB glGetProgramLocalParameterdvARB;
  #endif

  #ifndef GLGETPROGRAMLOCALPARAMETERFVARB_DECL
  #define GLGETPROGRAMLOCALPARAMETERFVARB_DECL
  csGLGETPROGRAMLOCALPARAMETERFVARB glGetProgramLocalParameterfvARB;
  #endif

  #ifndef GLGETPROGRAMIVARB_DECL
  #define GLGETPROGRAMIVARB_DECL
  csGLGETPROGRAMIVARB glGetProgramivARB;
  #endif

  #ifndef GLGETPROGRAMSTRINGARB_DECL
  #define GLGETPROGRAMSTRINGARB_DECL
  csGLGETPROGRAMSTRINGARB glGetProgramStringARB;
  #endif

  #ifndef GLISPROGRAMARB_DECL
  #define GLISPROGRAMARB_DECL
  csGLISPROGRAMARB glIsProgramARB;
  #endif


  // GL_ATI_text_fragment_shader

  // GL_APPLE_client_storage

  // GL_APPLE_element_array
  #ifndef GLELEMENTPOINTERAPPLE_DECL
  #define GLELEMENTPOINTERAPPLE_DECL
  csGLELEMENTPOINTERAPPLE glElementPointerAPPLE;
  #endif

  #ifndef GLDRAWELEMENTARRAYAPPLE_DECL
  #define GLDRAWELEMENTARRAYAPPLE_DECL
  csGLDRAWELEMENTARRAYAPPLE glDrawElementArrayAPPLE;
  #endif

  #ifndef GLDRAWRANGEELEMENTARRAYAPPLE_DECL
  #define GLDRAWRANGEELEMENTARRAYAPPLE_DECL
  csGLDRAWRANGEELEMENTARRAYAPPLE glDrawRangeElementArrayAPPLE;
  #endif

  #ifndef GLMULTIDRAWELEMENTARRAYAPPLE_DECL
  #define GLMULTIDRAWELEMENTARRAYAPPLE_DECL
  csGLMULTIDRAWELEMENTARRAYAPPLE glMultiDrawElementArrayAPPLE;
  #endif

  #ifndef GLMULTIDRAWRANGEELEMENTARRAYAPPLE_DECL
  #define GLMULTIDRAWRANGEELEMENTARRAYAPPLE_DECL
  csGLMULTIDRAWRANGEELEMENTARRAYAPPLE glMultiDrawRangeElementArrayAPPLE;
  #endif


  // GL_APPLE_fence
  #ifndef GLGENFENCESAPPLE_DECL
  #define GLGENFENCESAPPLE_DECL
  csGLGENFENCESAPPLE glGenFencesAPPLE;
  #endif

  #ifndef GLDELETEFENCESAPPLE_DECL
  #define GLDELETEFENCESAPPLE_DECL
  csGLDELETEFENCESAPPLE glDeleteFencesAPPLE;
  #endif

  #ifndef GLSETFENCEAPPLE_DECL
  #define GLSETFENCEAPPLE_DECL
  csGLSETFENCEAPPLE glSetFenceAPPLE;
  #endif

  #ifndef GLISFENCEAPPLE_DECL
  #define GLISFENCEAPPLE_DECL
  csGLISFENCEAPPLE glIsFenceAPPLE;
  #endif

  #ifndef GLTESTFENCEAPPLE_DECL
  #define GLTESTFENCEAPPLE_DECL
  csGLTESTFENCEAPPLE glTestFenceAPPLE;
  #endif

  #ifndef GLFINISHFENCEAPPLE_DECL
  #define GLFINISHFENCEAPPLE_DECL
  csGLFINISHFENCEAPPLE glFinishFenceAPPLE;
  #endif

  #ifndef GLTESTOBJECTAPPLE_DECL
  #define GLTESTOBJECTAPPLE_DECL
  csGLTESTOBJECTAPPLE glTestObjectAPPLE;
  #endif

  #ifndef GLFINISHOBJECTAPPLE_DECL
  #define GLFINISHOBJECTAPPLE_DECL
  csGLFINISHOBJECTAPPLE glFinishObjectAPPLE;
  #endif


  // GL_APPLE_vertex_array_object
  #ifndef GLBINDVERTEXARRAYAPPLE_DECL
  #define GLBINDVERTEXARRAYAPPLE_DECL
  csGLBINDVERTEXARRAYAPPLE glBindVertexArrayAPPLE;
  #endif

  #ifndef GLDELETEVERTEXARRAYSAPPLE_DECL
  #define GLDELETEVERTEXARRAYSAPPLE_DECL
  csGLDELETEVERTEXARRAYSAPPLE glDeleteVertexArraysAPPLE;
  #endif

  #ifndef GLGENVERTEXARRAYSAPPLE_DECL
  #define GLGENVERTEXARRAYSAPPLE_DECL
  csGLGENVERTEXARRAYSAPPLE glGenVertexArraysAPPLE;
  #endif

  #ifndef GLISVERTEXARRAYAPPLE_DECL
  #define GLISVERTEXARRAYAPPLE_DECL
  csGLISVERTEXARRAYAPPLE glIsVertexArrayAPPLE;
  #endif


  // GL_APPLE_vertex_array_range
  #ifndef GLVERTEXARRAYRANGEAPPLE_DECL
  #define GLVERTEXARRAYRANGEAPPLE_DECL
  csGLVERTEXARRAYRANGEAPPLE glVertexArrayRangeAPPLE;
  #endif

  #ifndef GLFLUSHVERTEXARRAYRANGEAPPLE_DECL
  #define GLFLUSHVERTEXARRAYRANGEAPPLE_DECL
  csGLFLUSHVERTEXARRAYRANGEAPPLE glFlushVertexArrayRangeAPPLE;
  #endif

  #ifndef GLVERTEXARRAYPARAMETERIAPPLE_DECL
  #define GLVERTEXARRAYPARAMETERIAPPLE_DECL
  csGLVERTEXARRAYPARAMETERIAPPLE glVertexArrayParameteriAPPLE;
  #endif


  // WGL_ARB_pixel_format
  #ifndef WGLGETPIXELFORMATATTRIBIVARB_DECL
  #define WGLGETPIXELFORMATATTRIBIVARB_DECL
  csWGLGETPIXELFORMATATTRIBIVARB wglGetPixelFormatAttribivARB;
  #endif

  #ifndef WGLGETPIXELFORMATATTRIBFVARB_DECL
  #define WGLGETPIXELFORMATATTRIBFVARB_DECL
  csWGLGETPIXELFORMATATTRIBFVARB wglGetPixelFormatAttribfvARB;
  #endif

  #ifndef WGLCHOOSEPIXELFORMATARB_DECL
  #define WGLCHOOSEPIXELFORMATARB_DECL
  csWGLCHOOSEPIXELFORMATARB wglChoosePixelFormatARB;
  #endif


  // WGL_ARB_make_current_read
  #ifndef WGLMAKECONTEXTCURRENTARB_DECL
  #define WGLMAKECONTEXTCURRENTARB_DECL
  csWGLMAKECONTEXTCURRENTARB wglMakeContextCurrentARB;
  #endif

  #ifndef WGLGETCURRENTREADDCARB_DECL
  #define WGLGETCURRENTREADDCARB_DECL
  csWGLGETCURRENTREADDCARB wglGetCurrentReadDCARB;
  #endif


  // WGL_ARB_pbuffer
  #ifndef WGLCREATEPBUFFERARB_DECL
  #define WGLCREATEPBUFFERARB_DECL
  csWGLCREATEPBUFFERARB wglCreatePbufferARB;
  #endif

  #ifndef WGLGETPBUFFERDCARB_DECL
  #define WGLGETPBUFFERDCARB_DECL
  csWGLGETPBUFFERDCARB wglGetPbufferDCARB;
  #endif

  #ifndef WGLRELEASEPBUFFERDCARB_DECL
  #define WGLRELEASEPBUFFERDCARB_DECL
  csWGLRELEASEPBUFFERDCARB wglReleasePbufferDCARB;
  #endif

  #ifndef WGLDESTROYPBUFFERARB_DECL
  #define WGLDESTROYPBUFFERARB_DECL
  csWGLDESTROYPBUFFERARB wglDestroyPbufferARB;
  #endif

  #ifndef WGLQUERYPBUFFERARB_DECL
  #define WGLQUERYPBUFFERARB_DECL
  csWGLQUERYPBUFFERARB wglQueryPbufferARB;
  #endif


  // WGL_EXT_swap_control
  #ifndef WGLSWAPINTERVALEXT_DECL
  #define WGLSWAPINTERVALEXT_DECL
  csWGLSWAPINTERVALEXT wglSwapIntervalEXT;
  #endif

  #ifndef WGLGETSWAPINTERVALEXT_DECL
  #define WGLGETSWAPINTERVALEXT_DECL
  csWGLGETSWAPINTERVALEXT wglGetSwapIntervalEXT;
  #endif


  // WGL_ARB_render_texture
  #ifndef WGLBINDTEXIMAGEARB_DECL
  #define WGLBINDTEXIMAGEARB_DECL
  csWGLBINDTEXIMAGEARB wglBindTexImageARB;
  #endif

  #ifndef WGLRELEASETEXIMAGEARB_DECL
  #define WGLRELEASETEXIMAGEARB_DECL
  csWGLRELEASETEXIMAGEARB wglReleaseTexImageARB;
  #endif

  #ifndef WGLSETPBUFFERATTRIBARB_DECL
  #define WGLSETPBUFFERATTRIBARB_DECL
  csWGLSETPBUFFERATTRIBARB wglSetPbufferAttribARB;
  #endif


  // WGL_EXT_extensions_string
  #ifndef WGLGETEXTENSIONSSTRINGEXT_DECL
  #define WGLGETEXTENSIONSSTRINGEXT_DECL
  csWGLGETEXTENSIONSSTRINGEXT wglGetExtensionsStringEXT;
  #endif


  // WGL_EXT_make_current_read
  #ifndef WGLMAKECONTEXTCURRENTEXT_DECL
  #define WGLMAKECONTEXTCURRENTEXT_DECL
  csWGLMAKECONTEXTCURRENTEXT wglMakeContextCurrentEXT;
  #endif

  #ifndef WGLGETCURRENTREADDCEXT_DECL
  #define WGLGETCURRENTREADDCEXT_DECL
  csWGLGETCURRENTREADDCEXT wglGetCurrentReadDCEXT;
  #endif


  // WGL_EXT_pbuffer
  #ifndef WGLCREATEPBUFFEREXT_DECL
  #define WGLCREATEPBUFFEREXT_DECL
  csWGLCREATEPBUFFEREXT wglCreatePbufferEXT;
  #endif

  #ifndef WGLGETPBUFFERDCEXT_DECL
  #define WGLGETPBUFFERDCEXT_DECL
  csWGLGETPBUFFERDCEXT wglGetPbufferDCEXT;
  #endif

  #ifndef WGLRELEASEPBUFFERDCEXT_DECL
  #define WGLRELEASEPBUFFERDCEXT_DECL
  csWGLRELEASEPBUFFERDCEXT wglReleasePbufferDCEXT;
  #endif

  #ifndef WGLDESTROYPBUFFEREXT_DECL
  #define WGLDESTROYPBUFFEREXT_DECL
  csWGLDESTROYPBUFFEREXT wglDestroyPbufferEXT;
  #endif

  #ifndef WGLQUERYPBUFFEREXT_DECL
  #define WGLQUERYPBUFFEREXT_DECL
  csWGLQUERYPBUFFEREXT wglQueryPbufferEXT;
  #endif


  // WGL_EXT_pixel_format
  #ifndef WGLGETPIXELFORMATATTRIBIVEXT_DECL
  #define WGLGETPIXELFORMATATTRIBIVEXT_DECL
  csWGLGETPIXELFORMATATTRIBIVEXT wglGetPixelFormatAttribivEXT;
  #endif

  #ifndef WGLGETPIXELFORMATATTRIBFVEXT_DECL
  #define WGLGETPIXELFORMATATTRIBFVEXT_DECL
  csWGLGETPIXELFORMATATTRIBFVEXT wglGetPixelFormatAttribfvEXT;
  #endif

  #ifndef WGLCHOOSEPIXELFORMATEXT_DECL
  #define WGLCHOOSEPIXELFORMATEXT_DECL
  csWGLCHOOSEPIXELFORMATEXT wglChoosePixelFormatEXT;
  #endif


  // WGL_I3D_digital_video_control
  #ifndef WGLGETDIGITALVIDEOPARAMETERSI3D_DECL
  #define WGLGETDIGITALVIDEOPARAMETERSI3D_DECL
  csWGLGETDIGITALVIDEOPARAMETERSI3D wglGetDigitalVideoParametersI3D;
  #endif

  #ifndef WGLSETDIGITALVIDEOPARAMETERSI3D_DECL
  #define WGLSETDIGITALVIDEOPARAMETERSI3D_DECL
  csWGLSETDIGITALVIDEOPARAMETERSI3D wglSetDigitalVideoParametersI3D;
  #endif


  // WGL_I3D_gamma
  #ifndef WGLGETGAMMATABLEPARAMETERSI3D_DECL
  #define WGLGETGAMMATABLEPARAMETERSI3D_DECL
  csWGLGETGAMMATABLEPARAMETERSI3D wglGetGammaTableParametersI3D;
  #endif

  #ifndef WGLSETGAMMATABLEPARAMETERSI3D_DECL
  #define WGLSETGAMMATABLEPARAMETERSI3D_DECL
  csWGLSETGAMMATABLEPARAMETERSI3D wglSetGammaTableParametersI3D;
  #endif

  #ifndef WGLGETGAMMATABLEI3D_DECL
  #define WGLGETGAMMATABLEI3D_DECL
  csWGLGETGAMMATABLEI3D wglGetGammaTableI3D;
  #endif

  #ifndef WGLSETGAMMATABLEI3D_DECL
  #define WGLSETGAMMATABLEI3D_DECL
  csWGLSETGAMMATABLEI3D wglSetGammaTableI3D;
  #endif


  // WGL_I3D_genlock
  #ifndef WGLENABLEGENLOCKI3D_DECL
  #define WGLENABLEGENLOCKI3D_DECL
  csWGLENABLEGENLOCKI3D wglEnableGenlockI3D;
  #endif

  #ifndef WGLDISABLEGENLOCKI3D_DECL
  #define WGLDISABLEGENLOCKI3D_DECL
  csWGLDISABLEGENLOCKI3D wglDisableGenlockI3D;
  #endif

  #ifndef WGLISENABLEDGENLOCKI3D_DECL
  #define WGLISENABLEDGENLOCKI3D_DECL
  csWGLISENABLEDGENLOCKI3D wglIsEnabledGenlockI3D;
  #endif

  #ifndef WGLGENLOCKSOURCEI3D_DECL
  #define WGLGENLOCKSOURCEI3D_DECL
  csWGLGENLOCKSOURCEI3D wglGenlockSourceI3D;
  #endif

  #ifndef WGLGETGENLOCKSOURCEI3D_DECL
  #define WGLGETGENLOCKSOURCEI3D_DECL
  csWGLGETGENLOCKSOURCEI3D wglGetGenlockSourceI3D;
  #endif

  #ifndef WGLGENLOCKSOURCEEDGEI3D_DECL
  #define WGLGENLOCKSOURCEEDGEI3D_DECL
  csWGLGENLOCKSOURCEEDGEI3D wglGenlockSourceEdgeI3D;
  #endif

  #ifndef WGLGETGENLOCKSOURCEEDGEI3D_DECL
  #define WGLGETGENLOCKSOURCEEDGEI3D_DECL
  csWGLGETGENLOCKSOURCEEDGEI3D wglGetGenlockSourceEdgeI3D;
  #endif

  #ifndef WGLGENLOCKSAMPLERATEI3D_DECL
  #define WGLGENLOCKSAMPLERATEI3D_DECL
  csWGLGENLOCKSAMPLERATEI3D wglGenlockSampleRateI3D;
  #endif

  #ifndef WGLGETGENLOCKSAMPLERATEI3D_DECL
  #define WGLGETGENLOCKSAMPLERATEI3D_DECL
  csWGLGETGENLOCKSAMPLERATEI3D wglGetGenlockSampleRateI3D;
  #endif

  #ifndef WGLGENLOCKSOURCEDELAYI3D_DECL
  #define WGLGENLOCKSOURCEDELAYI3D_DECL
  csWGLGENLOCKSOURCEDELAYI3D wglGenlockSourceDelayI3D;
  #endif

  #ifndef WGLGETGENLOCKSOURCEDELAYI3D_DECL
  #define WGLGETGENLOCKSOURCEDELAYI3D_DECL
  csWGLGETGENLOCKSOURCEDELAYI3D wglGetGenlockSourceDelayI3D;
  #endif

  #ifndef WGLQUERYGENLOCKMAXSOURCEDELAYI3D_DECL
  #define WGLQUERYGENLOCKMAXSOURCEDELAYI3D_DECL
  csWGLQUERYGENLOCKMAXSOURCEDELAYI3D wglQueryGenlockMaxSourceDelayI3D;
  #endif


  // GL_ARB_matrix_palette
  #ifndef GLCURRENTPALETTEMATRIXARB_DECL
  #define GLCURRENTPALETTEMATRIXARB_DECL
  csGLCURRENTPALETTEMATRIXARB glCurrentPaletteMatrixARB;
  #endif

  #ifndef GLMATRIXINDEXUBVARB_DECL
  #define GLMATRIXINDEXUBVARB_DECL
  csGLMATRIXINDEXUBVARB glMatrixIndexubvARB;
  #endif

  #ifndef GLMATRIXINDEXUSVARB_DECL
  #define GLMATRIXINDEXUSVARB_DECL
  csGLMATRIXINDEXUSVARB glMatrixIndexusvARB;
  #endif

  #ifndef GLMATRIXINDEXUIVARB_DECL
  #define GLMATRIXINDEXUIVARB_DECL
  csGLMATRIXINDEXUIVARB glMatrixIndexuivARB;
  #endif

  #ifndef GLMATRIXINDEXPOINTERARB_DECL
  #define GLMATRIXINDEXPOINTERARB_DECL
  csGLMATRIXINDEXPOINTERARB glMatrixIndexPointerARB;
  #endif


  // GL_NV_element_array
  #ifndef GLELEMENTPOINTERNV_DECL
  #define GLELEMENTPOINTERNV_DECL
  csGLELEMENTPOINTERNV glElementPointerNV;
  #endif

  #ifndef GLDRAWELEMENTARRAYNV_DECL
  #define GLDRAWELEMENTARRAYNV_DECL
  csGLDRAWELEMENTARRAYNV glDrawElementArrayNV;
  #endif

  #ifndef GLDRAWRANGEELEMENTARRAYNV_DECL
  #define GLDRAWRANGEELEMENTARRAYNV_DECL
  csGLDRAWRANGEELEMENTARRAYNV glDrawRangeElementArrayNV;
  #endif

  #ifndef GLMULTIDRAWELEMENTARRAYNV_DECL
  #define GLMULTIDRAWELEMENTARRAYNV_DECL
  csGLMULTIDRAWELEMENTARRAYNV glMultiDrawElementArrayNV;
  #endif

  #ifndef GLMULTIDRAWRANGEELEMENTARRAYNV_DECL
  #define GLMULTIDRAWRANGEELEMENTARRAYNV_DECL
  csGLMULTIDRAWRANGEELEMENTARRAYNV glMultiDrawRangeElementArrayNV;
  #endif


  // GL_NV_float_buffer

  // GL_NV_fragment_program
  #ifndef GLPROGRAMNAMEDPARAMETER4FNV_DECL
  #define GLPROGRAMNAMEDPARAMETER4FNV_DECL
  csGLPROGRAMNAMEDPARAMETER4FNV glProgramNamedParameter4fNV;
  #endif

  #ifndef GLPROGRAMNAMEDPARAMETER4DNV_DECL
  #define GLPROGRAMNAMEDPARAMETER4DNV_DECL
  csGLPROGRAMNAMEDPARAMETER4DNV glProgramNamedParameter4dNV;
  #endif

  #ifndef GLGETPROGRAMNAMEDPARAMETERFVNV_DECL
  #define GLGETPROGRAMNAMEDPARAMETERFVNV_DECL
  csGLGETPROGRAMNAMEDPARAMETERFVNV glGetProgramNamedParameterfvNV;
  #endif

  #ifndef GLGETPROGRAMNAMEDPARAMETERDVNV_DECL
  #define GLGETPROGRAMNAMEDPARAMETERDVNV_DECL
  csGLGETPROGRAMNAMEDPARAMETERDVNV glGetProgramNamedParameterdvNV;
  #endif

  #ifndef GLPROGRAMLOCALPARAMETER4DARB_DECL
  #define GLPROGRAMLOCALPARAMETER4DARB_DECL
  csGLPROGRAMLOCALPARAMETER4DARB glProgramLocalParameter4dARB;
  #endif

  #ifndef GLPROGRAMLOCALPARAMETER4DVARB_DECL
  #define GLPROGRAMLOCALPARAMETER4DVARB_DECL
  csGLPROGRAMLOCALPARAMETER4DVARB glProgramLocalParameter4dvARB;
  #endif

  #ifndef GLPROGRAMLOCALPARAMETER4FARB_DECL
  #define GLPROGRAMLOCALPARAMETER4FARB_DECL
  csGLPROGRAMLOCALPARAMETER4FARB glProgramLocalParameter4fARB;
  #endif

  #ifndef GLPROGRAMLOCALPARAMETER4FVARB_DECL
  #define GLPROGRAMLOCALPARAMETER4FVARB_DECL
  csGLPROGRAMLOCALPARAMETER4FVARB glProgramLocalParameter4fvARB;
  #endif

  #ifndef GLGETPROGRAMLOCALPARAMETERDVARB_DECL
  #define GLGETPROGRAMLOCALPARAMETERDVARB_DECL
  csGLGETPROGRAMLOCALPARAMETERDVARB glGetProgramLocalParameterdvARB;
  #endif

  #ifndef GLGETPROGRAMLOCALPARAMETERFVARB_DECL
  #define GLGETPROGRAMLOCALPARAMETERFVARB_DECL
  csGLGETPROGRAMLOCALPARAMETERFVARB glGetProgramLocalParameterfvARB;
  #endif


  // GL_NV_primitive_restart
  #ifndef GLPRIMITIVERESTARTNV_DECL
  #define GLPRIMITIVERESTARTNV_DECL
  csGLPRIMITIVERESTARTNV glPrimitiveRestartNV;
  #endif

  #ifndef GLPRIMITIVERESTARTINDEXNV_DECL
  #define GLPRIMITIVERESTARTINDEXNV_DECL
  csGLPRIMITIVERESTARTINDEXNV glPrimitiveRestartIndexNV;
  #endif


  // GL_NV_vertex_program2

  void InitExtensions (iOpenGLInterface* gl)
  {
    bool allclear;
    const char* extensions = (const char*)glGetString (GL_EXTENSIONS);
    // GL_version_1_2
    CS_GL_version_1_2 = (strstr (extensions, "GL_version_1_2") != NULL);
    if (CS_GL_version_1_2)
    {
      allclear = true;
      if (!(glBlendColor = (csGLBLENDCOLOR) gl->GetProcAddress ("glBlendColor"))) allclear = false;
      if (!(glBlendEquation = (csGLBLENDEQUATION) gl->GetProcAddress ("glBlendEquation"))) allclear = false;
      if (!(glDrawRangeElements = (csGLDRAWRANGEELEMENTS) gl->GetProcAddress ("glDrawRangeElements"))) allclear = false;
      if (!(glColorTable = (csGLCOLORTABLE) gl->GetProcAddress ("glColorTable"))) allclear = false;
      if (!(glColorTableParameterfv = (csGLCOLORTABLEPARAMETERFV) gl->GetProcAddress ("glColorTableParameterfv"))) allclear = false;
      if (!(glColorTableParameteriv = (csGLCOLORTABLEPARAMETERIV) gl->GetProcAddress ("glColorTableParameteriv"))) allclear = false;
      if (!(glCopyColorTable = (csGLCOPYCOLORTABLE) gl->GetProcAddress ("glCopyColorTable"))) allclear = false;
      if (!(glGetColorTable = (csGLGETCOLORTABLE) gl->GetProcAddress ("glGetColorTable"))) allclear = false;
      if (!(glGetColorTableParameterfv = (csGLGETCOLORTABLEPARAMETERFV) gl->GetProcAddress ("glGetColorTableParameterfv"))) allclear = false;
      if (!(glGetColorTableParameteriv = (csGLGETCOLORTABLEPARAMETERIV) gl->GetProcAddress ("glGetColorTableParameteriv"))) allclear = false;
      if (!(glColorSubTable = (csGLCOLORSUBTABLE) gl->GetProcAddress ("glColorSubTable"))) allclear = false;
      if (!(glCopyColorSubTable = (csGLCOPYCOLORSUBTABLE) gl->GetProcAddress ("glCopyColorSubTable"))) allclear = false;
      if (!(glConvolutionFilter1D = (csGLCONVOLUTIONFILTER1D) gl->GetProcAddress ("glConvolutionFilter1D"))) allclear = false;
      if (!(glConvolutionFilter2D = (csGLCONVOLUTIONFILTER2D) gl->GetProcAddress ("glConvolutionFilter2D"))) allclear = false;
      if (!(glConvolutionParameterf = (csGLCONVOLUTIONPARAMETERF) gl->GetProcAddress ("glConvolutionParameterf"))) allclear = false;
      if (!(glConvolutionParameterfv = (csGLCONVOLUTIONPARAMETERFV) gl->GetProcAddress ("glConvolutionParameterfv"))) allclear = false;
      if (!(glConvolutionParameteri = (csGLCONVOLUTIONPARAMETERI) gl->GetProcAddress ("glConvolutionParameteri"))) allclear = false;
      if (!(glConvolutionParameteriv = (csGLCONVOLUTIONPARAMETERIV) gl->GetProcAddress ("glConvolutionParameteriv"))) allclear = false;
      if (!(glCopyConvolutionFilter1D = (csGLCOPYCONVOLUTIONFILTER1D) gl->GetProcAddress ("glCopyConvolutionFilter1D"))) allclear = false;
      if (!(glCopyConvolutionFilter2D = (csGLCOPYCONVOLUTIONFILTER2D) gl->GetProcAddress ("glCopyConvolutionFilter2D"))) allclear = false;
      if (!(glGetConvolutionFilter = (csGLGETCONVOLUTIONFILTER) gl->GetProcAddress ("glGetConvolutionFilter"))) allclear = false;
      if (!(glGetConvolutionParameterfv = (csGLGETCONVOLUTIONPARAMETERFV) gl->GetProcAddress ("glGetConvolutionParameterfv"))) allclear = false;
      if (!(glGetConvolutionParameteriv = (csGLGETCONVOLUTIONPARAMETERIV) gl->GetProcAddress ("glGetConvolutionParameteriv"))) allclear = false;
      if (!(glGetSeparableFilter = (csGLGETSEPARABLEFILTER) gl->GetProcAddress ("glGetSeparableFilter"))) allclear = false;
      if (!(glSeparableFilter2D = (csGLSEPARABLEFILTER2D) gl->GetProcAddress ("glSeparableFilter2D"))) allclear = false;
      if (!(glGetHistogram = (csGLGETHISTOGRAM) gl->GetProcAddress ("glGetHistogram"))) allclear = false;
      if (!(glGetHistogramParameterfv = (csGLGETHISTOGRAMPARAMETERFV) gl->GetProcAddress ("glGetHistogramParameterfv"))) allclear = false;
      if (!(glGetHistogramParameteriv = (csGLGETHISTOGRAMPARAMETERIV) gl->GetProcAddress ("glGetHistogramParameteriv"))) allclear = false;
      if (!(glGetMinmax = (csGLGETMINMAX) gl->GetProcAddress ("glGetMinmax"))) allclear = false;
      if (!(glGetMinmaxParameterfv = (csGLGETMINMAXPARAMETERFV) gl->GetProcAddress ("glGetMinmaxParameterfv"))) allclear = false;
      if (!(glGetMinmaxParameteriv = (csGLGETMINMAXPARAMETERIV) gl->GetProcAddress ("glGetMinmaxParameteriv"))) allclear = false;
      if (!(glHistogram = (csGLHISTOGRAM) gl->GetProcAddress ("glHistogram"))) allclear = false;
      if (!(glMinmax = (csGLMINMAX) gl->GetProcAddress ("glMinmax"))) allclear = false;
      if (!(glResetHistogram = (csGLRESETHISTOGRAM) gl->GetProcAddress ("glResetHistogram"))) allclear = false;
      if (!(glResetMinmax = (csGLRESETMINMAX) gl->GetProcAddress ("glResetMinmax"))) allclear = false;
      if (!(glTexImage3D = (csGLTEXIMAGE3D) gl->GetProcAddress ("glTexImage3D"))) allclear = false;
      if (!(glTexSubImage3D = (csGLTEXSUBIMAGE3D) gl->GetProcAddress ("glTexSubImage3D"))) allclear = false;
      if (!(glCopyTexSubImage3D = (csGLCOPYTEXSUBIMAGE3D) gl->GetProcAddress ("glCopyTexSubImage3D"))) allclear = false;
      if (CS_GL_version_1_2 = allclear)
        printf ("GL Extension 'GL_version_1_2' found and used.\n");
    }

    // GL_ARB_imaging
    CS_GL_ARB_imaging = (strstr (extensions, "GL_ARB_imaging") != NULL);
    if (CS_GL_ARB_imaging)
    {
      allclear = true;
      if (CS_GL_ARB_imaging = allclear)
        printf ("GL Extension 'GL_ARB_imaging' found and used.\n");
    }

    // GL_version_1_3
    CS_GL_version_1_3 = (strstr (extensions, "GL_version_1_3") != NULL);
    if (CS_GL_version_1_3)
    {
      allclear = true;
      if (!(glActiveTexture = (csGLACTIVETEXTURE) gl->GetProcAddress ("glActiveTexture"))) allclear = false;
      if (!(glClientActiveTexture = (csGLCLIENTACTIVETEXTURE) gl->GetProcAddress ("glClientActiveTexture"))) allclear = false;
      if (!(glMultiTexCoord1d = (csGLMULTITEXCOORD1D) gl->GetProcAddress ("glMultiTexCoord1d"))) allclear = false;
      if (!(glMultiTexCoord1dv = (csGLMULTITEXCOORD1DV) gl->GetProcAddress ("glMultiTexCoord1dv"))) allclear = false;
      if (!(glMultiTexCoord1f = (csGLMULTITEXCOORD1F) gl->GetProcAddress ("glMultiTexCoord1f"))) allclear = false;
      if (!(glMultiTexCoord1fv = (csGLMULTITEXCOORD1FV) gl->GetProcAddress ("glMultiTexCoord1fv"))) allclear = false;
      if (!(glMultiTexCoord1i = (csGLMULTITEXCOORD1I) gl->GetProcAddress ("glMultiTexCoord1i"))) allclear = false;
      if (!(glMultiTexCoord1iv = (csGLMULTITEXCOORD1IV) gl->GetProcAddress ("glMultiTexCoord1iv"))) allclear = false;
      if (!(glMultiTexCoord1s = (csGLMULTITEXCOORD1S) gl->GetProcAddress ("glMultiTexCoord1s"))) allclear = false;
      if (!(glMultiTexCoord1sv = (csGLMULTITEXCOORD1SV) gl->GetProcAddress ("glMultiTexCoord1sv"))) allclear = false;
      if (!(glMultiTexCoord2d = (csGLMULTITEXCOORD2D) gl->GetProcAddress ("glMultiTexCoord2d"))) allclear = false;
      if (!(glMultiTexCoord2dv = (csGLMULTITEXCOORD2DV) gl->GetProcAddress ("glMultiTexCoord2dv"))) allclear = false;
      if (!(glMultiTexCoord2f = (csGLMULTITEXCOORD2F) gl->GetProcAddress ("glMultiTexCoord2f"))) allclear = false;
      if (!(glMultiTexCoord2fv = (csGLMULTITEXCOORD2FV) gl->GetProcAddress ("glMultiTexCoord2fv"))) allclear = false;
      if (!(glMultiTexCoord2i = (csGLMULTITEXCOORD2I) gl->GetProcAddress ("glMultiTexCoord2i"))) allclear = false;
      if (!(glMultiTexCoord2iv = (csGLMULTITEXCOORD2IV) gl->GetProcAddress ("glMultiTexCoord2iv"))) allclear = false;
      if (!(glMultiTexCoord2s = (csGLMULTITEXCOORD2S) gl->GetProcAddress ("glMultiTexCoord2s"))) allclear = false;
      if (!(glMultiTexCoord2sv = (csGLMULTITEXCOORD2SV) gl->GetProcAddress ("glMultiTexCoord2sv"))) allclear = false;
      if (!(glMultiTexCoord3d = (csGLMULTITEXCOORD3D) gl->GetProcAddress ("glMultiTexCoord3d"))) allclear = false;
      if (!(glMultiTexCoord3dv = (csGLMULTITEXCOORD3DV) gl->GetProcAddress ("glMultiTexCoord3dv"))) allclear = false;
      if (!(glMultiTexCoord3f = (csGLMULTITEXCOORD3F) gl->GetProcAddress ("glMultiTexCoord3f"))) allclear = false;
      if (!(glMultiTexCoord3fv = (csGLMULTITEXCOORD3FV) gl->GetProcAddress ("glMultiTexCoord3fv"))) allclear = false;
      if (!(glMultiTexCoord3i = (csGLMULTITEXCOORD3I) gl->GetProcAddress ("glMultiTexCoord3i"))) allclear = false;
      if (!(glMultiTexCoord3iv = (csGLMULTITEXCOORD3IV) gl->GetProcAddress ("glMultiTexCoord3iv"))) allclear = false;
      if (!(glMultiTexCoord3s = (csGLMULTITEXCOORD3S) gl->GetProcAddress ("glMultiTexCoord3s"))) allclear = false;
      if (!(glMultiTexCoord3sv = (csGLMULTITEXCOORD3SV) gl->GetProcAddress ("glMultiTexCoord3sv"))) allclear = false;
      if (!(glMultiTexCoord4d = (csGLMULTITEXCOORD4D) gl->GetProcAddress ("glMultiTexCoord4d"))) allclear = false;
      if (!(glMultiTexCoord4dv = (csGLMULTITEXCOORD4DV) gl->GetProcAddress ("glMultiTexCoord4dv"))) allclear = false;
      if (!(glMultiTexCoord4f = (csGLMULTITEXCOORD4F) gl->GetProcAddress ("glMultiTexCoord4f"))) allclear = false;
      if (!(glMultiTexCoord4fv = (csGLMULTITEXCOORD4FV) gl->GetProcAddress ("glMultiTexCoord4fv"))) allclear = false;
      if (!(glMultiTexCoord4i = (csGLMULTITEXCOORD4I) gl->GetProcAddress ("glMultiTexCoord4i"))) allclear = false;
      if (!(glMultiTexCoord4iv = (csGLMULTITEXCOORD4IV) gl->GetProcAddress ("glMultiTexCoord4iv"))) allclear = false;
      if (!(glMultiTexCoord4s = (csGLMULTITEXCOORD4S) gl->GetProcAddress ("glMultiTexCoord4s"))) allclear = false;
      if (!(glMultiTexCoord4sv = (csGLMULTITEXCOORD4SV) gl->GetProcAddress ("glMultiTexCoord4sv"))) allclear = false;
      if (!(glLoadTransposeMatrixf = (csGLLOADTRANSPOSEMATRIXF) gl->GetProcAddress ("glLoadTransposeMatrixf"))) allclear = false;
      if (!(glLoadTransposeMatrixd = (csGLLOADTRANSPOSEMATRIXD) gl->GetProcAddress ("glLoadTransposeMatrixd"))) allclear = false;
      if (!(glMultTransposeMatrixf = (csGLMULTTRANSPOSEMATRIXF) gl->GetProcAddress ("glMultTransposeMatrixf"))) allclear = false;
      if (!(glMultTransposeMatrixd = (csGLMULTTRANSPOSEMATRIXD) gl->GetProcAddress ("glMultTransposeMatrixd"))) allclear = false;
      if (!(glSampleCoverage = (csGLSAMPLECOVERAGE) gl->GetProcAddress ("glSampleCoverage"))) allclear = false;
      if (!(glCompressedTexImage3D = (csGLCOMPRESSEDTEXIMAGE3D) gl->GetProcAddress ("glCompressedTexImage3D"))) allclear = false;
      if (!(glCompressedTexImage2D = (csGLCOMPRESSEDTEXIMAGE2D) gl->GetProcAddress ("glCompressedTexImage2D"))) allclear = false;
      if (!(glCompressedTexImage1D = (csGLCOMPRESSEDTEXIMAGE1D) gl->GetProcAddress ("glCompressedTexImage1D"))) allclear = false;
      if (!(glCompressedTexSubImage3D = (csGLCOMPRESSEDTEXSUBIMAGE3D) gl->GetProcAddress ("glCompressedTexSubImage3D"))) allclear = false;
      if (!(glCompressedTexSubImage2D = (csGLCOMPRESSEDTEXSUBIMAGE2D) gl->GetProcAddress ("glCompressedTexSubImage2D"))) allclear = false;
      if (!(glCompressedTexSubImage1D = (csGLCOMPRESSEDTEXSUBIMAGE1D) gl->GetProcAddress ("glCompressedTexSubImage1D"))) allclear = false;
      if (!(glGetCompressedTexImage = (csGLGETCOMPRESSEDTEXIMAGE) gl->GetProcAddress ("glGetCompressedTexImage"))) allclear = false;
      if (CS_GL_version_1_3 = allclear)
        printf ("GL Extension 'GL_version_1_3' found and used.\n");
    }

    // GL_ARB_multitexture
    CS_GL_ARB_multitexture = (strstr (extensions, "GL_ARB_multitexture") != NULL);
    if (CS_GL_ARB_multitexture)
    {
      allclear = true;
      if (!(glActiveTextureARB = (csGLACTIVETEXTUREARB) gl->GetProcAddress ("glActiveTextureARB"))) allclear = false;
      if (!(glClientActiveTextureARB = (csGLCLIENTACTIVETEXTUREARB) gl->GetProcAddress ("glClientActiveTextureARB"))) allclear = false;
      if (!(glMultiTexCoord1dARB = (csGLMULTITEXCOORD1DARB) gl->GetProcAddress ("glMultiTexCoord1dARB"))) allclear = false;
      if (!(glMultiTexCoord1dvARB = (csGLMULTITEXCOORD1DVARB) gl->GetProcAddress ("glMultiTexCoord1dvARB"))) allclear = false;
      if (!(glMultiTexCoord1fARB = (csGLMULTITEXCOORD1FARB) gl->GetProcAddress ("glMultiTexCoord1fARB"))) allclear = false;
      if (!(glMultiTexCoord1fvARB = (csGLMULTITEXCOORD1FVARB) gl->GetProcAddress ("glMultiTexCoord1fvARB"))) allclear = false;
      if (!(glMultiTexCoord1iARB = (csGLMULTITEXCOORD1IARB) gl->GetProcAddress ("glMultiTexCoord1iARB"))) allclear = false;
      if (!(glMultiTexCoord1ivARB = (csGLMULTITEXCOORD1IVARB) gl->GetProcAddress ("glMultiTexCoord1ivARB"))) allclear = false;
      if (!(glMultiTexCoord1sARB = (csGLMULTITEXCOORD1SARB) gl->GetProcAddress ("glMultiTexCoord1sARB"))) allclear = false;
      if (!(glMultiTexCoord1svARB = (csGLMULTITEXCOORD1SVARB) gl->GetProcAddress ("glMultiTexCoord1svARB"))) allclear = false;
      if (!(glMultiTexCoord2dARB = (csGLMULTITEXCOORD2DARB) gl->GetProcAddress ("glMultiTexCoord2dARB"))) allclear = false;
      if (!(glMultiTexCoord2dvARB = (csGLMULTITEXCOORD2DVARB) gl->GetProcAddress ("glMultiTexCoord2dvARB"))) allclear = false;
      if (!(glMultiTexCoord2fARB = (csGLMULTITEXCOORD2FARB) gl->GetProcAddress ("glMultiTexCoord2fARB"))) allclear = false;
      if (!(glMultiTexCoord2fvARB = (csGLMULTITEXCOORD2FVARB) gl->GetProcAddress ("glMultiTexCoord2fvARB"))) allclear = false;
      if (!(glMultiTexCoord2iARB = (csGLMULTITEXCOORD2IARB) gl->GetProcAddress ("glMultiTexCoord2iARB"))) allclear = false;
      if (!(glMultiTexCoord2ivARB = (csGLMULTITEXCOORD2IVARB) gl->GetProcAddress ("glMultiTexCoord2ivARB"))) allclear = false;
      if (!(glMultiTexCoord2sARB = (csGLMULTITEXCOORD2SARB) gl->GetProcAddress ("glMultiTexCoord2sARB"))) allclear = false;
      if (!(glMultiTexCoord2svARB = (csGLMULTITEXCOORD2SVARB) gl->GetProcAddress ("glMultiTexCoord2svARB"))) allclear = false;
      if (!(glMultiTexCoord3dARB = (csGLMULTITEXCOORD3DARB) gl->GetProcAddress ("glMultiTexCoord3dARB"))) allclear = false;
      if (!(glMultiTexCoord3dvARB = (csGLMULTITEXCOORD3DVARB) gl->GetProcAddress ("glMultiTexCoord3dvARB"))) allclear = false;
      if (!(glMultiTexCoord3fARB = (csGLMULTITEXCOORD3FARB) gl->GetProcAddress ("glMultiTexCoord3fARB"))) allclear = false;
      if (!(glMultiTexCoord3fvARB = (csGLMULTITEXCOORD3FVARB) gl->GetProcAddress ("glMultiTexCoord3fvARB"))) allclear = false;
      if (!(glMultiTexCoord3iARB = (csGLMULTITEXCOORD3IARB) gl->GetProcAddress ("glMultiTexCoord3iARB"))) allclear = false;
      if (!(glMultiTexCoord3ivARB = (csGLMULTITEXCOORD3IVARB) gl->GetProcAddress ("glMultiTexCoord3ivARB"))) allclear = false;
      if (!(glMultiTexCoord3sARB = (csGLMULTITEXCOORD3SARB) gl->GetProcAddress ("glMultiTexCoord3sARB"))) allclear = false;
      if (!(glMultiTexCoord3svARB = (csGLMULTITEXCOORD3SVARB) gl->GetProcAddress ("glMultiTexCoord3svARB"))) allclear = false;
      if (!(glMultiTexCoord4dARB = (csGLMULTITEXCOORD4DARB) gl->GetProcAddress ("glMultiTexCoord4dARB"))) allclear = false;
      if (!(glMultiTexCoord4dvARB = (csGLMULTITEXCOORD4DVARB) gl->GetProcAddress ("glMultiTexCoord4dvARB"))) allclear = false;
      if (!(glMultiTexCoord4fARB = (csGLMULTITEXCOORD4FARB) gl->GetProcAddress ("glMultiTexCoord4fARB"))) allclear = false;
      if (!(glMultiTexCoord4fvARB = (csGLMULTITEXCOORD4FVARB) gl->GetProcAddress ("glMultiTexCoord4fvARB"))) allclear = false;
      if (!(glMultiTexCoord4iARB = (csGLMULTITEXCOORD4IARB) gl->GetProcAddress ("glMultiTexCoord4iARB"))) allclear = false;
      if (!(glMultiTexCoord4ivARB = (csGLMULTITEXCOORD4IVARB) gl->GetProcAddress ("glMultiTexCoord4ivARB"))) allclear = false;
      if (!(glMultiTexCoord4sARB = (csGLMULTITEXCOORD4SARB) gl->GetProcAddress ("glMultiTexCoord4sARB"))) allclear = false;
      if (!(glMultiTexCoord4svARB = (csGLMULTITEXCOORD4SVARB) gl->GetProcAddress ("glMultiTexCoord4svARB"))) allclear = false;
      if (CS_GL_ARB_multitexture = allclear)
        printf ("GL Extension 'GL_ARB_multitexture' found and used.\n");
    }

    // GL_ARB_transpose_matrix
    CS_GL_ARB_transpose_matrix = (strstr (extensions, "GL_ARB_transpose_matrix") != NULL);
    if (CS_GL_ARB_transpose_matrix)
    {
      allclear = true;
      if (!(glLoadTransposeMatrixfARB = (csGLLOADTRANSPOSEMATRIXFARB) gl->GetProcAddress ("glLoadTransposeMatrixfARB"))) allclear = false;
      if (!(glLoadTransposeMatrixdARB = (csGLLOADTRANSPOSEMATRIXDARB) gl->GetProcAddress ("glLoadTransposeMatrixdARB"))) allclear = false;
      if (!(glMultTransposeMatrixfARB = (csGLMULTTRANSPOSEMATRIXFARB) gl->GetProcAddress ("glMultTransposeMatrixfARB"))) allclear = false;
      if (!(glMultTransposeMatrixdARB = (csGLMULTTRANSPOSEMATRIXDARB) gl->GetProcAddress ("glMultTransposeMatrixdARB"))) allclear = false;
      if (CS_GL_ARB_transpose_matrix = allclear)
        printf ("GL Extension 'GL_ARB_transpose_matrix' found and used.\n");
    }

    // GL_ARB_multisample
    CS_GL_ARB_multisample = (strstr (extensions, "GL_ARB_multisample") != NULL);
    if (CS_GL_ARB_multisample)
    {
      allclear = true;
      if (!(glSampleCoverageARB = (csGLSAMPLECOVERAGEARB) gl->GetProcAddress ("glSampleCoverageARB"))) allclear = false;
      if (CS_GL_ARB_multisample = allclear)
        printf ("GL Extension 'GL_ARB_multisample' found and used.\n");
    }

    // GL_ARB_texture_env_add
    CS_GL_ARB_texture_env_add = (strstr (extensions, "GL_ARB_texture_env_add") != NULL);
    if (CS_GL_ARB_texture_env_add)
    {
      allclear = true;
      if (CS_GL_ARB_texture_env_add = allclear)
        printf ("GL Extension 'GL_ARB_texture_env_add' found and used.\n");
    }

    // WGL_ARB_extensions_string
    CS_WGL_ARB_extensions_string = (strstr (extensions, "WGL_ARB_extensions_string") != NULL);
    if (CS_WGL_ARB_extensions_string)
    {
      allclear = true;
      if (!(wglGetExtensionsStringARB = (csWGLGETEXTENSIONSSTRINGARB) gl->GetProcAddress ("wglGetExtensionsStringARB"))) allclear = false;
      if (CS_WGL_ARB_extensions_string = allclear)
        printf ("GL Extension 'WGL_ARB_extensions_string' found and used.\n");
    }

    // WGL_ARB_buffer_region
    CS_WGL_ARB_buffer_region = (strstr (extensions, "WGL_ARB_buffer_region") != NULL);
    if (CS_WGL_ARB_buffer_region)
    {
      allclear = true;
      if (!(wglCreateBufferRegionARB = (csWGLCREATEBUFFERREGIONARB) gl->GetProcAddress ("wglCreateBufferRegionARB"))) allclear = false;
      if (!(wglDeleteBufferRegionARB = (csWGLDELETEBUFFERREGIONARB) gl->GetProcAddress ("wglDeleteBufferRegionARB"))) allclear = false;
      if (!(wglSaveBufferRegionARB = (csWGLSAVEBUFFERREGIONARB) gl->GetProcAddress ("wglSaveBufferRegionARB"))) allclear = false;
      if (!(wglRestoreBufferRegionARB = (csWGLRESTOREBUFFERREGIONARB) gl->GetProcAddress ("wglRestoreBufferRegionARB"))) allclear = false;
      if (CS_WGL_ARB_buffer_region = allclear)
        printf ("GL Extension 'WGL_ARB_buffer_region' found and used.\n");
    }

    // GL_ARB_texture_cube_map
    CS_GL_ARB_texture_cube_map = (strstr (extensions, "GL_ARB_texture_cube_map") != NULL);
    if (CS_GL_ARB_texture_cube_map)
    {
      allclear = true;
      if (CS_GL_ARB_texture_cube_map = allclear)
        printf ("GL Extension 'GL_ARB_texture_cube_map' found and used.\n");
    }

    // GL_ARB_depth_texture
    CS_GL_ARB_depth_texture = (strstr (extensions, "GL_ARB_depth_texture") != NULL);
    if (CS_GL_ARB_depth_texture)
    {
      allclear = true;
      if (CS_GL_ARB_depth_texture = allclear)
        printf ("GL Extension 'GL_ARB_depth_texture' found and used.\n");
    }

    // GL_ARB_point_parameters
    CS_GL_ARB_point_parameters = (strstr (extensions, "GL_ARB_point_parameters") != NULL);
    if (CS_GL_ARB_point_parameters)
    {
      allclear = true;
      if (!(glPointParameterfARB = (csGLPOINTPARAMETERFARB) gl->GetProcAddress ("glPointParameterfARB"))) allclear = false;
      if (!(glPointParameterfvARB = (csGLPOINTPARAMETERFVARB) gl->GetProcAddress ("glPointParameterfvARB"))) allclear = false;
      if (CS_GL_ARB_point_parameters = allclear)
        printf ("GL Extension 'GL_ARB_point_parameters' found and used.\n");
    }

    // GL_ARB_shadow
    CS_GL_ARB_shadow = (strstr (extensions, "GL_ARB_shadow") != NULL);
    if (CS_GL_ARB_shadow)
    {
      allclear = true;
      if (CS_GL_ARB_shadow = allclear)
        printf ("GL Extension 'GL_ARB_shadow' found and used.\n");
    }

    // GL_ARB_shadow_ambient
    CS_GL_ARB_shadow_ambient = (strstr (extensions, "GL_ARB_shadow_ambient") != NULL);
    if (CS_GL_ARB_shadow_ambient)
    {
      allclear = true;
      if (CS_GL_ARB_shadow_ambient = allclear)
        printf ("GL Extension 'GL_ARB_shadow_ambient' found and used.\n");
    }

    // GL_ARB_texture_border_clamp
    CS_GL_ARB_texture_border_clamp = (strstr (extensions, "GL_ARB_texture_border_clamp") != NULL);
    if (CS_GL_ARB_texture_border_clamp)
    {
      allclear = true;
      if (CS_GL_ARB_texture_border_clamp = allclear)
        printf ("GL Extension 'GL_ARB_texture_border_clamp' found and used.\n");
    }

    // GL_ARB_texture_compression
    CS_GL_ARB_texture_compression = (strstr (extensions, "GL_ARB_texture_compression") != NULL);
    if (CS_GL_ARB_texture_compression)
    {
      allclear = true;
      if (!(glCompressedTexImage3DARB = (csGLCOMPRESSEDTEXIMAGE3DARB) gl->GetProcAddress ("glCompressedTexImage3DARB"))) allclear = false;
      if (!(glCompressedTexImage2DARB = (csGLCOMPRESSEDTEXIMAGE2DARB) gl->GetProcAddress ("glCompressedTexImage2DARB"))) allclear = false;
      if (!(glCompressedTexImage1DARB = (csGLCOMPRESSEDTEXIMAGE1DARB) gl->GetProcAddress ("glCompressedTexImage1DARB"))) allclear = false;
      if (!(glCompressedTexSubImage3DARB = (csGLCOMPRESSEDTEXSUBIMAGE3DARB) gl->GetProcAddress ("glCompressedTexSubImage3DARB"))) allclear = false;
      if (!(glCompressedTexSubImage2DARB = (csGLCOMPRESSEDTEXSUBIMAGE2DARB) gl->GetProcAddress ("glCompressedTexSubImage2DARB"))) allclear = false;
      if (!(glCompressedTexSubImage1DARB = (csGLCOMPRESSEDTEXSUBIMAGE1DARB) gl->GetProcAddress ("glCompressedTexSubImage1DARB"))) allclear = false;
      if (!(glGetCompressedTexImageARB = (csGLGETCOMPRESSEDTEXIMAGEARB) gl->GetProcAddress ("glGetCompressedTexImageARB"))) allclear = false;
      if (CS_GL_ARB_texture_compression = allclear)
        printf ("GL Extension 'GL_ARB_texture_compression' found and used.\n");
    }

    // GL_ARB_texture_env_combine
    CS_GL_ARB_texture_env_combine = (strstr (extensions, "GL_ARB_texture_env_combine") != NULL);
    if (CS_GL_ARB_texture_env_combine)
    {
      allclear = true;
      if (CS_GL_ARB_texture_env_combine = allclear)
        printf ("GL Extension 'GL_ARB_texture_env_combine' found and used.\n");
    }

    // GL_ARB_texture_env_crossbar
    CS_GL_ARB_texture_env_crossbar = (strstr (extensions, "GL_ARB_texture_env_crossbar") != NULL);
    if (CS_GL_ARB_texture_env_crossbar)
    {
      allclear = true;
      if (CS_GL_ARB_texture_env_crossbar = allclear)
        printf ("GL Extension 'GL_ARB_texture_env_crossbar' found and used.\n");
    }

    // GL_ARB_texture_env_dot3
    CS_GL_ARB_texture_env_dot3 = (strstr (extensions, "GL_ARB_texture_env_dot3") != NULL);
    if (CS_GL_ARB_texture_env_dot3)
    {
      allclear = true;
      if (CS_GL_ARB_texture_env_dot3 = allclear)
        printf ("GL Extension 'GL_ARB_texture_env_dot3' found and used.\n");
    }

    // GL_ARB_texture_mirrored_repeat
    CS_GL_ARB_texture_mirrored_repeat = (strstr (extensions, "GL_ARB_texture_mirrored_repeat") != NULL);
    if (CS_GL_ARB_texture_mirrored_repeat)
    {
      allclear = true;
      if (CS_GL_ARB_texture_mirrored_repeat = allclear)
        printf ("GL Extension 'GL_ARB_texture_mirrored_repeat' found and used.\n");
    }

    // GL_ARB_vertex_blend
    CS_GL_ARB_vertex_blend = (strstr (extensions, "GL_ARB_vertex_blend") != NULL);
    if (CS_GL_ARB_vertex_blend)
    {
      allclear = true;
      if (!(glWeightbvARB = (csGLWEIGHTBVARB) gl->GetProcAddress ("glWeightbvARB"))) allclear = false;
      if (!(glWeightsvARB = (csGLWEIGHTSVARB) gl->GetProcAddress ("glWeightsvARB"))) allclear = false;
      if (!(glWeightivARB = (csGLWEIGHTIVARB) gl->GetProcAddress ("glWeightivARB"))) allclear = false;
      if (!(glWeightfvARB = (csGLWEIGHTFVARB) gl->GetProcAddress ("glWeightfvARB"))) allclear = false;
      if (!(glWeightdvARB = (csGLWEIGHTDVARB) gl->GetProcAddress ("glWeightdvARB"))) allclear = false;
      if (!(glWeightvARB = (csGLWEIGHTVARB) gl->GetProcAddress ("glWeightvARB"))) allclear = false;
      if (!(glWeightubvARB = (csGLWEIGHTUBVARB) gl->GetProcAddress ("glWeightubvARB"))) allclear = false;
      if (!(glWeightusvARB = (csGLWEIGHTUSVARB) gl->GetProcAddress ("glWeightusvARB"))) allclear = false;
      if (!(glWeightuivARB = (csGLWEIGHTUIVARB) gl->GetProcAddress ("glWeightuivARB"))) allclear = false;
      if (!(glWeightPointerARB = (csGLWEIGHTPOINTERARB) gl->GetProcAddress ("glWeightPointerARB"))) allclear = false;
      if (!(glVertexBlendARB = (csGLVERTEXBLENDARB) gl->GetProcAddress ("glVertexBlendARB"))) allclear = false;
      if (CS_GL_ARB_vertex_blend = allclear)
        printf ("GL Extension 'GL_ARB_vertex_blend' found and used.\n");
    }

    // GL_ARB_vertex_program
    CS_GL_ARB_vertex_program = (strstr (extensions, "GL_ARB_vertex_program") != NULL);
    if (CS_GL_ARB_vertex_program)
    {
      allclear = true;
      if (!(glVertexAttrib1sARB = (csGLVERTEXATTRIB1SARB) gl->GetProcAddress ("glVertexAttrib1sARB"))) allclear = false;
      if (!(glVertexAttrib1fARB = (csGLVERTEXATTRIB1FARB) gl->GetProcAddress ("glVertexAttrib1fARB"))) allclear = false;
      if (!(glVertexAttrib1dARB = (csGLVERTEXATTRIB1DARB) gl->GetProcAddress ("glVertexAttrib1dARB"))) allclear = false;
      if (!(glVertexAttrib2sARB = (csGLVERTEXATTRIB2SARB) gl->GetProcAddress ("glVertexAttrib2sARB"))) allclear = false;
      if (!(glVertexAttrib2fARB = (csGLVERTEXATTRIB2FARB) gl->GetProcAddress ("glVertexAttrib2fARB"))) allclear = false;
      if (!(glVertexAttrib2dARB = (csGLVERTEXATTRIB2DARB) gl->GetProcAddress ("glVertexAttrib2dARB"))) allclear = false;
      if (!(glVertexAttrib3sARB = (csGLVERTEXATTRIB3SARB) gl->GetProcAddress ("glVertexAttrib3sARB"))) allclear = false;
      if (!(glVertexAttrib3fARB = (csGLVERTEXATTRIB3FARB) gl->GetProcAddress ("glVertexAttrib3fARB"))) allclear = false;
      if (!(glVertexAttrib3dARB = (csGLVERTEXATTRIB3DARB) gl->GetProcAddress ("glVertexAttrib3dARB"))) allclear = false;
      if (!(glVertexAttrib4sARB = (csGLVERTEXATTRIB4SARB) gl->GetProcAddress ("glVertexAttrib4sARB"))) allclear = false;
      if (!(glVertexAttrib4fARB = (csGLVERTEXATTRIB4FARB) gl->GetProcAddress ("glVertexAttrib4fARB"))) allclear = false;
      if (!(glVertexAttrib4dARB = (csGLVERTEXATTRIB4DARB) gl->GetProcAddress ("glVertexAttrib4dARB"))) allclear = false;
      if (!(glVertexAttrib4NubARB = (csGLVERTEXATTRIB4NUBARB) gl->GetProcAddress ("glVertexAttrib4NubARB"))) allclear = false;
      if (!(glVertexAttrib1svARB = (csGLVERTEXATTRIB1SVARB) gl->GetProcAddress ("glVertexAttrib1svARB"))) allclear = false;
      if (!(glVertexAttrib1fvARB = (csGLVERTEXATTRIB1FVARB) gl->GetProcAddress ("glVertexAttrib1fvARB"))) allclear = false;
      if (!(glVertexAttrib1dvARB = (csGLVERTEXATTRIB1DVARB) gl->GetProcAddress ("glVertexAttrib1dvARB"))) allclear = false;
      if (!(glVertexAttrib2svARB = (csGLVERTEXATTRIB2SVARB) gl->GetProcAddress ("glVertexAttrib2svARB"))) allclear = false;
      if (!(glVertexAttrib2fvARB = (csGLVERTEXATTRIB2FVARB) gl->GetProcAddress ("glVertexAttrib2fvARB"))) allclear = false;
      if (!(glVertexAttrib2dvARB = (csGLVERTEXATTRIB2DVARB) gl->GetProcAddress ("glVertexAttrib2dvARB"))) allclear = false;
      if (!(glVertexAttrib3svARB = (csGLVERTEXATTRIB3SVARB) gl->GetProcAddress ("glVertexAttrib3svARB"))) allclear = false;
      if (!(glVertexAttrib3fvARB = (csGLVERTEXATTRIB3FVARB) gl->GetProcAddress ("glVertexAttrib3fvARB"))) allclear = false;
      if (!(glVertexAttrib3dvARB = (csGLVERTEXATTRIB3DVARB) gl->GetProcAddress ("glVertexAttrib3dvARB"))) allclear = false;
      if (!(glVertexAttrib4bvARB = (csGLVERTEXATTRIB4BVARB) gl->GetProcAddress ("glVertexAttrib4bvARB"))) allclear = false;
      if (!(glVertexAttrib4svARB = (csGLVERTEXATTRIB4SVARB) gl->GetProcAddress ("glVertexAttrib4svARB"))) allclear = false;
      if (!(glVertexAttrib4ivARB = (csGLVERTEXATTRIB4IVARB) gl->GetProcAddress ("glVertexAttrib4ivARB"))) allclear = false;
      if (!(glVertexAttrib4ubvARB = (csGLVERTEXATTRIB4UBVARB) gl->GetProcAddress ("glVertexAttrib4ubvARB"))) allclear = false;
      if (!(glVertexAttrib4usvARB = (csGLVERTEXATTRIB4USVARB) gl->GetProcAddress ("glVertexAttrib4usvARB"))) allclear = false;
      if (!(glVertexAttrib4uivARB = (csGLVERTEXATTRIB4UIVARB) gl->GetProcAddress ("glVertexAttrib4uivARB"))) allclear = false;
      if (!(glVertexAttrib4fvARB = (csGLVERTEXATTRIB4FVARB) gl->GetProcAddress ("glVertexAttrib4fvARB"))) allclear = false;
      if (!(glVertexAttrib4dvARB = (csGLVERTEXATTRIB4DVARB) gl->GetProcAddress ("glVertexAttrib4dvARB"))) allclear = false;
      if (!(glVertexAttrib4NbvARB = (csGLVERTEXATTRIB4NBVARB) gl->GetProcAddress ("glVertexAttrib4NbvARB"))) allclear = false;
      if (!(glVertexAttrib4NsvARB = (csGLVERTEXATTRIB4NSVARB) gl->GetProcAddress ("glVertexAttrib4NsvARB"))) allclear = false;
      if (!(glVertexAttrib4NivARB = (csGLVERTEXATTRIB4NIVARB) gl->GetProcAddress ("glVertexAttrib4NivARB"))) allclear = false;
      if (!(glVertexAttrib4NubvARB = (csGLVERTEXATTRIB4NUBVARB) gl->GetProcAddress ("glVertexAttrib4NubvARB"))) allclear = false;
      if (!(glVertexAttrib4NusvARB = (csGLVERTEXATTRIB4NUSVARB) gl->GetProcAddress ("glVertexAttrib4NusvARB"))) allclear = false;
      if (!(glVertexAttrib4NuivARB = (csGLVERTEXATTRIB4NUIVARB) gl->GetProcAddress ("glVertexAttrib4NuivARB"))) allclear = false;
      if (!(glVertexAttribPointerARB = (csGLVERTEXATTRIBPOINTERARB) gl->GetProcAddress ("glVertexAttribPointerARB"))) allclear = false;
      if (!(glEnableVertexAttribArrayARB = (csGLENABLEVERTEXATTRIBARRAYARB) gl->GetProcAddress ("glEnableVertexAttribArrayARB"))) allclear = false;
      if (!(glDisableVertexAttribArrayARB = (csGLDISABLEVERTEXATTRIBARRAYARB) gl->GetProcAddress ("glDisableVertexAttribArrayARB"))) allclear = false;
      if (!(glProgramStringARB = (csGLPROGRAMSTRINGARB) gl->GetProcAddress ("glProgramStringARB"))) allclear = false;
      if (!(glBindProgramARB = (csGLBINDPROGRAMARB) gl->GetProcAddress ("glBindProgramARB"))) allclear = false;
      if (!(glDeleteProgramsARB = (csGLDELETEPROGRAMSARB) gl->GetProcAddress ("glDeleteProgramsARB"))) allclear = false;
      if (!(glGenProgramsARB = (csGLGENPROGRAMSARB) gl->GetProcAddress ("glGenProgramsARB"))) allclear = false;
      if (!(glProgramEnvParameter4dARB = (csGLPROGRAMENVPARAMETER4DARB) gl->GetProcAddress ("glProgramEnvParameter4dARB"))) allclear = false;
      if (!(glProgramEnvParameter4dvARB = (csGLPROGRAMENVPARAMETER4DVARB) gl->GetProcAddress ("glProgramEnvParameter4dvARB"))) allclear = false;
      if (!(glProgramEnvParameter4fARB = (csGLPROGRAMENVPARAMETER4FARB) gl->GetProcAddress ("glProgramEnvParameter4fARB"))) allclear = false;
      if (!(glProgramEnvParameter4fvARB = (csGLPROGRAMENVPARAMETER4FVARB) gl->GetProcAddress ("glProgramEnvParameter4fvARB"))) allclear = false;
      if (!(glProgramLocalParameter4dARB = (csGLPROGRAMLOCALPARAMETER4DARB) gl->GetProcAddress ("glProgramLocalParameter4dARB"))) allclear = false;
      if (!(glProgramLocalParameter4dvARB = (csGLPROGRAMLOCALPARAMETER4DVARB) gl->GetProcAddress ("glProgramLocalParameter4dvARB"))) allclear = false;
      if (!(glProgramLocalParameter4fARB = (csGLPROGRAMLOCALPARAMETER4FARB) gl->GetProcAddress ("glProgramLocalParameter4fARB"))) allclear = false;
      if (!(glProgramLocalParameter4fvARB = (csGLPROGRAMLOCALPARAMETER4FVARB) gl->GetProcAddress ("glProgramLocalParameter4fvARB"))) allclear = false;
      if (!(glGetProgramEnvParameterdvARB = (csGLGETPROGRAMENVPARAMETERDVARB) gl->GetProcAddress ("glGetProgramEnvParameterdvARB"))) allclear = false;
      if (!(glGetProgramEnvParameterfvARB = (csGLGETPROGRAMENVPARAMETERFVARB) gl->GetProcAddress ("glGetProgramEnvParameterfvARB"))) allclear = false;
      if (!(glGetProgramLocalParameterdvARB = (csGLGETPROGRAMLOCALPARAMETERDVARB) gl->GetProcAddress ("glGetProgramLocalParameterdvARB"))) allclear = false;
      if (!(glGetProgramLocalParameterfvARB = (csGLGETPROGRAMLOCALPARAMETERFVARB) gl->GetProcAddress ("glGetProgramLocalParameterfvARB"))) allclear = false;
      if (!(glGetProgramivARB = (csGLGETPROGRAMIVARB) gl->GetProcAddress ("glGetProgramivARB"))) allclear = false;
      if (!(glGetProgramStringARB = (csGLGETPROGRAMSTRINGARB) gl->GetProcAddress ("glGetProgramStringARB"))) allclear = false;
      if (!(glGetVertexAttribdvARB = (csGLGETVERTEXATTRIBDVARB) gl->GetProcAddress ("glGetVertexAttribdvARB"))) allclear = false;
      if (!(glGetVertexAttribfvARB = (csGLGETVERTEXATTRIBFVARB) gl->GetProcAddress ("glGetVertexAttribfvARB"))) allclear = false;
      if (!(glGetVertexAttribivARB = (csGLGETVERTEXATTRIBIVARB) gl->GetProcAddress ("glGetVertexAttribivARB"))) allclear = false;
      if (!(glGetVertexAttribPointervARB = (csGLGETVERTEXATTRIBPOINTERVARB) gl->GetProcAddress ("glGetVertexAttribPointervARB"))) allclear = false;
      if (!(glIsProgramARB = (csGLISPROGRAMARB) gl->GetProcAddress ("glIsProgramARB"))) allclear = false;
      if (CS_GL_ARB_vertex_program = allclear)
        printf ("GL Extension 'GL_ARB_vertex_program' found and used.\n");
    }

    // GL_ARB_window_pos
    CS_GL_ARB_window_pos = (strstr (extensions, "GL_ARB_window_pos") != NULL);
    if (CS_GL_ARB_window_pos)
    {
      allclear = true;
      if (!(glWindowPos2dARB = (csGLWINDOWPOS2DARB) gl->GetProcAddress ("glWindowPos2dARB"))) allclear = false;
      if (!(glWindowPos2fARB = (csGLWINDOWPOS2FARB) gl->GetProcAddress ("glWindowPos2fARB"))) allclear = false;
      if (!(glWindowPos2iARB = (csGLWINDOWPOS2IARB) gl->GetProcAddress ("glWindowPos2iARB"))) allclear = false;
      if (!(glWindowPos2sARB = (csGLWINDOWPOS2SARB) gl->GetProcAddress ("glWindowPos2sARB"))) allclear = false;
      if (!(glWindowPos2dvARB = (csGLWINDOWPOS2DVARB) gl->GetProcAddress ("glWindowPos2dvARB"))) allclear = false;
      if (!(glWindowPos2fvARB = (csGLWINDOWPOS2FVARB) gl->GetProcAddress ("glWindowPos2fvARB"))) allclear = false;
      if (!(glWindowPos2ivARB = (csGLWINDOWPOS2IVARB) gl->GetProcAddress ("glWindowPos2ivARB"))) allclear = false;
      if (!(glWindowPos2svARB = (csGLWINDOWPOS2SVARB) gl->GetProcAddress ("glWindowPos2svARB"))) allclear = false;
      if (!(glWindowPos3dARB = (csGLWINDOWPOS3DARB) gl->GetProcAddress ("glWindowPos3dARB"))) allclear = false;
      if (!(glWindowPos3fARB = (csGLWINDOWPOS3FARB) gl->GetProcAddress ("glWindowPos3fARB"))) allclear = false;
      if (!(glWindowPos3iARB = (csGLWINDOWPOS3IARB) gl->GetProcAddress ("glWindowPos3iARB"))) allclear = false;
      if (!(glWindowPos3sARB = (csGLWINDOWPOS3SARB) gl->GetProcAddress ("glWindowPos3sARB"))) allclear = false;
      if (!(glWindowPos3dvARB = (csGLWINDOWPOS3DVARB) gl->GetProcAddress ("glWindowPos3dvARB"))) allclear = false;
      if (!(glWindowPos3fvARB = (csGLWINDOWPOS3FVARB) gl->GetProcAddress ("glWindowPos3fvARB"))) allclear = false;
      if (!(glWindowPos3ivARB = (csGLWINDOWPOS3IVARB) gl->GetProcAddress ("glWindowPos3ivARB"))) allclear = false;
      if (!(glWindowPos3svARB = (csGLWINDOWPOS3SVARB) gl->GetProcAddress ("glWindowPos3svARB"))) allclear = false;
      if (CS_GL_ARB_window_pos = allclear)
        printf ("GL Extension 'GL_ARB_window_pos' found and used.\n");
    }

    // GL_EXT_422_pixels
    CS_GL_EXT_422_pixels = (strstr (extensions, "GL_EXT_422_pixels") != NULL);
    if (CS_GL_EXT_422_pixels)
    {
      allclear = true;
      if (CS_GL_EXT_422_pixels = allclear)
        printf ("GL Extension 'GL_EXT_422_pixels' found and used.\n");
    }

    // GL_EXT_abgr
    CS_GL_EXT_abgr = (strstr (extensions, "GL_EXT_abgr") != NULL);
    if (CS_GL_EXT_abgr)
    {
      allclear = true;
      if (CS_GL_EXT_abgr = allclear)
        printf ("GL Extension 'GL_EXT_abgr' found and used.\n");
    }

    // GL_EXT_bgra
    CS_GL_EXT_bgra = (strstr (extensions, "GL_EXT_bgra") != NULL);
    if (CS_GL_EXT_bgra)
    {
      allclear = true;
      if (CS_GL_EXT_bgra = allclear)
        printf ("GL Extension 'GL_EXT_bgra' found and used.\n");
    }

    // GL_EXT_blend_color
    CS_GL_EXT_blend_color = (strstr (extensions, "GL_EXT_blend_color") != NULL);
    if (CS_GL_EXT_blend_color)
    {
      allclear = true;
      if (!(glBlendColorEXT = (csGLBLENDCOLOREXT) gl->GetProcAddress ("glBlendColorEXT"))) allclear = false;
      if (CS_GL_EXT_blend_color = allclear)
        printf ("GL Extension 'GL_EXT_blend_color' found and used.\n");
    }

    // GL_EXT_blend_func_separate
    CS_GL_EXT_blend_func_separate = (strstr (extensions, "GL_EXT_blend_func_separate") != NULL);
    if (CS_GL_EXT_blend_func_separate)
    {
      allclear = true;
      if (!(glBlendFuncSeparateEXT = (csGLBLENDFUNCSEPARATEEXT) gl->GetProcAddress ("glBlendFuncSeparateEXT"))) allclear = false;
      if (CS_GL_EXT_blend_func_separate = allclear)
        printf ("GL Extension 'GL_EXT_blend_func_separate' found and used.\n");
    }

    // GL_EXT_blend_logic_op
    CS_GL_EXT_blend_logic_op = (strstr (extensions, "GL_EXT_blend_logic_op") != NULL);
    if (CS_GL_EXT_blend_logic_op)
    {
      allclear = true;
      if (CS_GL_EXT_blend_logic_op = allclear)
        printf ("GL Extension 'GL_EXT_blend_logic_op' found and used.\n");
    }

    // GL_EXT_blend_minmax
    CS_GL_EXT_blend_minmax = (strstr (extensions, "GL_EXT_blend_minmax") != NULL);
    if (CS_GL_EXT_blend_minmax)
    {
      allclear = true;
      if (!(glBlendEquationEXT = (csGLBLENDEQUATIONEXT) gl->GetProcAddress ("glBlendEquationEXT"))) allclear = false;
      if (CS_GL_EXT_blend_minmax = allclear)
        printf ("GL Extension 'GL_EXT_blend_minmax' found and used.\n");
    }

    // GL_EXT_blend_subtract
    CS_GL_EXT_blend_subtract = (strstr (extensions, "GL_EXT_blend_subtract") != NULL);
    if (CS_GL_EXT_blend_subtract)
    {
      allclear = true;
      if (CS_GL_EXT_blend_subtract = allclear)
        printf ("GL Extension 'GL_EXT_blend_subtract' found and used.\n");
    }

    // GL_EXT_clip_volume_hint
    CS_GL_EXT_clip_volume_hint = (strstr (extensions, "GL_EXT_clip_volume_hint") != NULL);
    if (CS_GL_EXT_clip_volume_hint)
    {
      allclear = true;
      if (CS_GL_EXT_clip_volume_hint = allclear)
        printf ("GL Extension 'GL_EXT_clip_volume_hint' found and used.\n");
    }

    // GL_EXT_color_subtable
    CS_GL_EXT_color_subtable = (strstr (extensions, "GL_EXT_color_subtable") != NULL);
    if (CS_GL_EXT_color_subtable)
    {
      allclear = true;
      if (!(glColorSubTableEXT = (csGLCOLORSUBTABLEEXT) gl->GetProcAddress ("glColorSubTableEXT"))) allclear = false;
      if (!(glCopyColorSubTableEXT = (csGLCOPYCOLORSUBTABLEEXT) gl->GetProcAddress ("glCopyColorSubTableEXT"))) allclear = false;
      if (CS_GL_EXT_color_subtable = allclear)
        printf ("GL Extension 'GL_EXT_color_subtable' found and used.\n");
    }

    // GL_EXT_compiled_vertex_array
    CS_GL_EXT_compiled_vertex_array = (strstr (extensions, "GL_EXT_compiled_vertex_array") != NULL);
    if (CS_GL_EXT_compiled_vertex_array)
    {
      allclear = true;
      if (!(glLockArraysEXT = (csGLLOCKARRAYSEXT) gl->GetProcAddress ("glLockArraysEXT"))) allclear = false;
      if (!(glUnlockArraysEXT = (csGLUNLOCKARRAYSEXT) gl->GetProcAddress ("glUnlockArraysEXT"))) allclear = false;
      if (CS_GL_EXT_compiled_vertex_array = allclear)
        printf ("GL Extension 'GL_EXT_compiled_vertex_array' found and used.\n");
    }

    // GL_EXT_convolution
    CS_GL_EXT_convolution = (strstr (extensions, "GL_EXT_convolution") != NULL);
    if (CS_GL_EXT_convolution)
    {
      allclear = true;
      if (!(glConvolutionFilter1DEXT = (csGLCONVOLUTIONFILTER1DEXT) gl->GetProcAddress ("glConvolutionFilter1DEXT"))) allclear = false;
      if (!(glConvolutionFilter2DEXT = (csGLCONVOLUTIONFILTER2DEXT) gl->GetProcAddress ("glConvolutionFilter2DEXT"))) allclear = false;
      if (!(glCopyConvolutionFilter1DEXT = (csGLCOPYCONVOLUTIONFILTER1DEXT) gl->GetProcAddress ("glCopyConvolutionFilter1DEXT"))) allclear = false;
      if (!(glCopyConvolutionFilter2DEXT = (csGLCOPYCONVOLUTIONFILTER2DEXT) gl->GetProcAddress ("glCopyConvolutionFilter2DEXT"))) allclear = false;
      if (!(glGetConvolutionFilterEXT = (csGLGETCONVOLUTIONFILTEREXT) gl->GetProcAddress ("glGetConvolutionFilterEXT"))) allclear = false;
      if (!(glSeparableFilter2DEXT = (csGLSEPARABLEFILTER2DEXT) gl->GetProcAddress ("glSeparableFilter2DEXT"))) allclear = false;
      if (!(glGetSeparableFilterEXT = (csGLGETSEPARABLEFILTEREXT) gl->GetProcAddress ("glGetSeparableFilterEXT"))) allclear = false;
      if (!(glConvolutionParameteriEXT = (csGLCONVOLUTIONPARAMETERIEXT) gl->GetProcAddress ("glConvolutionParameteriEXT"))) allclear = false;
      if (!(glConvolutionParameterivEXT = (csGLCONVOLUTIONPARAMETERIVEXT) gl->GetProcAddress ("glConvolutionParameterivEXT"))) allclear = false;
      if (!(glConvolutionParameterfEXT = (csGLCONVOLUTIONPARAMETERFEXT) gl->GetProcAddress ("glConvolutionParameterfEXT"))) allclear = false;
      if (!(glConvolutionParameterfvEXT = (csGLCONVOLUTIONPARAMETERFVEXT) gl->GetProcAddress ("glConvolutionParameterfvEXT"))) allclear = false;
      if (!(glGetConvolutionParameterivEXT = (csGLGETCONVOLUTIONPARAMETERIVEXT) gl->GetProcAddress ("glGetConvolutionParameterivEXT"))) allclear = false;
      if (!(glGetConvolutionParameterfvEXT = (csGLGETCONVOLUTIONPARAMETERFVEXT) gl->GetProcAddress ("glGetConvolutionParameterfvEXT"))) allclear = false;
      if (CS_GL_EXT_convolution = allclear)
        printf ("GL Extension 'GL_EXT_convolution' found and used.\n");
    }

    // GL_EXT_fog_coord
    CS_GL_EXT_fog_coord = (strstr (extensions, "GL_EXT_fog_coord") != NULL);
    if (CS_GL_EXT_fog_coord)
    {
      allclear = true;
      if (!(glFogCoordfEXfloat = (csGLFOGCOORDFEXFLOAT) gl->GetProcAddress ("glFogCoordfEXfloat"))) allclear = false;
      if (!(glFogCoorddEXdouble = (csGLFOGCOORDDEXDOUBLE) gl->GetProcAddress ("glFogCoorddEXdouble"))) allclear = false;
      if (!(glFogCoordfvEXfloat = (csGLFOGCOORDFVEXFLOAT) gl->GetProcAddress ("glFogCoordfvEXfloat"))) allclear = false;
      if (!(glFogCoorddvEXdouble = (csGLFOGCOORDDVEXDOUBLE) gl->GetProcAddress ("glFogCoorddvEXdouble"))) allclear = false;
      if (!(glFogCoordPointerEXT = (csGLFOGCOORDPOINTEREXT) gl->GetProcAddress ("glFogCoordPointerEXT"))) allclear = false;
      if (CS_GL_EXT_fog_coord = allclear)
        printf ("GL Extension 'GL_EXT_fog_coord' found and used.\n");
    }

    // GL_EXT_histogram
    CS_GL_EXT_histogram = (strstr (extensions, "GL_EXT_histogram") != NULL);
    if (CS_GL_EXT_histogram)
    {
      allclear = true;
      if (!(glHistogramEXT = (csGLHISTOGRAMEXT) gl->GetProcAddress ("glHistogramEXT"))) allclear = false;
      if (!(glResetHistogramEXT = (csGLRESETHISTOGRAMEXT) gl->GetProcAddress ("glResetHistogramEXT"))) allclear = false;
      if (!(glGetHistogramEXT = (csGLGETHISTOGRAMEXT) gl->GetProcAddress ("glGetHistogramEXT"))) allclear = false;
      if (!(glGetHistogramParameterivEXT = (csGLGETHISTOGRAMPARAMETERIVEXT) gl->GetProcAddress ("glGetHistogramParameterivEXT"))) allclear = false;
      if (!(glGetHistogramParameterfvEXT = (csGLGETHISTOGRAMPARAMETERFVEXT) gl->GetProcAddress ("glGetHistogramParameterfvEXT"))) allclear = false;
      if (!(glMinmaxEXT = (csGLMINMAXEXT) gl->GetProcAddress ("glMinmaxEXT"))) allclear = false;
      if (!(glResetMinmaxEXT = (csGLRESETMINMAXEXT) gl->GetProcAddress ("glResetMinmaxEXT"))) allclear = false;
      if (!(glGetMinmaxEXT = (csGLGETMINMAXEXT) gl->GetProcAddress ("glGetMinmaxEXT"))) allclear = false;
      if (!(glGetMinmaxParameterivEXT = (csGLGETMINMAXPARAMETERIVEXT) gl->GetProcAddress ("glGetMinmaxParameterivEXT"))) allclear = false;
      if (!(glGetMinmaxParameterfvEXT = (csGLGETMINMAXPARAMETERFVEXT) gl->GetProcAddress ("glGetMinmaxParameterfvEXT"))) allclear = false;
      if (CS_GL_EXT_histogram = allclear)
        printf ("GL Extension 'GL_EXT_histogram' found and used.\n");
    }

    // GL_EXT_multi_draw_arrays
    CS_GL_EXT_multi_draw_arrays = (strstr (extensions, "GL_EXT_multi_draw_arrays") != NULL);
    if (CS_GL_EXT_multi_draw_arrays)
    {
      allclear = true;
      if (!(glMultiDrawArraysEXT = (csGLMULTIDRAWARRAYSEXT) gl->GetProcAddress ("glMultiDrawArraysEXT"))) allclear = false;
      if (!(glMultiDrawElementsEXT = (csGLMULTIDRAWELEMENTSEXT) gl->GetProcAddress ("glMultiDrawElementsEXT"))) allclear = false;
      if (CS_GL_EXT_multi_draw_arrays = allclear)
        printf ("GL Extension 'GL_EXT_multi_draw_arrays' found and used.\n");
    }

    // GL_EXT_packed_pixels
    CS_GL_EXT_packed_pixels = (strstr (extensions, "GL_EXT_packed_pixels") != NULL);
    if (CS_GL_EXT_packed_pixels)
    {
      allclear = true;
      if (CS_GL_EXT_packed_pixels = allclear)
        printf ("GL Extension 'GL_EXT_packed_pixels' found and used.\n");
    }

    // GL_EXT_paletted_texture
    CS_GL_EXT_paletted_texture = (strstr (extensions, "GL_EXT_paletted_texture") != NULL);
    if (CS_GL_EXT_paletted_texture)
    {
      allclear = true;
      if (!(glColorTableEXT = (csGLCOLORTABLEEXT) gl->GetProcAddress ("glColorTableEXT"))) allclear = false;
      if (!(glColorSubTableEXT = (csGLCOLORSUBTABLEEXT) gl->GetProcAddress ("glColorSubTableEXT"))) allclear = false;
      if (!(glGetColorTableEXT = (csGLGETCOLORTABLEEXT) gl->GetProcAddress ("glGetColorTableEXT"))) allclear = false;
      if (!(glGetColorTableParameterivEXT = (csGLGETCOLORTABLEPARAMETERIVEXT) gl->GetProcAddress ("glGetColorTableParameterivEXT"))) allclear = false;
      if (!(glGetColorTableParameterfvEXT = (csGLGETCOLORTABLEPARAMETERFVEXT) gl->GetProcAddress ("glGetColorTableParameterfvEXT"))) allclear = false;
      if (CS_GL_EXT_paletted_texture = allclear)
        printf ("GL Extension 'GL_EXT_paletted_texture' found and used.\n");
    }

    // GL_EXT_point_parameters
    CS_GL_EXT_point_parameters = (strstr (extensions, "GL_EXT_point_parameters") != NULL);
    if (CS_GL_EXT_point_parameters)
    {
      allclear = true;
      if (!(glPointParameterfEXT = (csGLPOINTPARAMETERFEXT) gl->GetProcAddress ("glPointParameterfEXT"))) allclear = false;
      if (!(glPointParameterfvEXT = (csGLPOINTPARAMETERFVEXT) gl->GetProcAddress ("glPointParameterfvEXT"))) allclear = false;
      if (CS_GL_EXT_point_parameters = allclear)
        printf ("GL Extension 'GL_EXT_point_parameters' found and used.\n");
    }

    // GL_EXT_polygon_offset
    CS_GL_EXT_polygon_offset = (strstr (extensions, "GL_EXT_polygon_offset") != NULL);
    if (CS_GL_EXT_polygon_offset)
    {
      allclear = true;
      if (!(glPolygonOffsetEXT = (csGLPOLYGONOFFSETEXT) gl->GetProcAddress ("glPolygonOffsetEXT"))) allclear = false;
      if (CS_GL_EXT_polygon_offset = allclear)
        printf ("GL Extension 'GL_EXT_polygon_offset' found and used.\n");
    }

    // GL_EXT_secondary_color
    CS_GL_EXT_secondary_color = (strstr (extensions, "GL_EXT_secondary_color") != NULL);
    if (CS_GL_EXT_secondary_color)
    {
      allclear = true;
      if (!(glSecondaryColor3bEXT = (csGLSECONDARYCOLOR3BEXT) gl->GetProcAddress ("glSecondaryColor3bEXT"))) allclear = false;
      if (!(glSecondaryColor3sEXT = (csGLSECONDARYCOLOR3SEXT) gl->GetProcAddress ("glSecondaryColor3sEXT"))) allclear = false;
      if (!(glSecondaryColor3iEXT = (csGLSECONDARYCOLOR3IEXT) gl->GetProcAddress ("glSecondaryColor3iEXT"))) allclear = false;
      if (!(glSecondaryColor3fEXT = (csGLSECONDARYCOLOR3FEXT) gl->GetProcAddress ("glSecondaryColor3fEXT"))) allclear = false;
      if (!(glSecondaryColor3dEXT = (csGLSECONDARYCOLOR3DEXT) gl->GetProcAddress ("glSecondaryColor3dEXT"))) allclear = false;
      if (!(glSecondaryColor3ubEXT = (csGLSECONDARYCOLOR3UBEXT) gl->GetProcAddress ("glSecondaryColor3ubEXT"))) allclear = false;
      if (!(glSecondaryColor3usEXT = (csGLSECONDARYCOLOR3USEXT) gl->GetProcAddress ("glSecondaryColor3usEXT"))) allclear = false;
      if (!(glSecondaryColor3uiEXT = (csGLSECONDARYCOLOR3UIEXT) gl->GetProcAddress ("glSecondaryColor3uiEXT"))) allclear = false;
      if (!(glSecondaryColor3bvEXT = (csGLSECONDARYCOLOR3BVEXT) gl->GetProcAddress ("glSecondaryColor3bvEXT"))) allclear = false;
      if (!(glSecondaryColor3svEXT = (csGLSECONDARYCOLOR3SVEXT) gl->GetProcAddress ("glSecondaryColor3svEXT"))) allclear = false;
      if (!(glSecondaryColor3ivEXT = (csGLSECONDARYCOLOR3IVEXT) gl->GetProcAddress ("glSecondaryColor3ivEXT"))) allclear = false;
      if (!(glSecondaryColor3fvEXT = (csGLSECONDARYCOLOR3FVEXT) gl->GetProcAddress ("glSecondaryColor3fvEXT"))) allclear = false;
      if (!(glSecondaryColor3dvEXT = (csGLSECONDARYCOLOR3DVEXT) gl->GetProcAddress ("glSecondaryColor3dvEXT"))) allclear = false;
      if (!(glSecondaryColor3ubvEXT = (csGLSECONDARYCOLOR3UBVEXT) gl->GetProcAddress ("glSecondaryColor3ubvEXT"))) allclear = false;
      if (!(glSecondaryColor3usvEXT = (csGLSECONDARYCOLOR3USVEXT) gl->GetProcAddress ("glSecondaryColor3usvEXT"))) allclear = false;
      if (!(glSecondaryColor3uivEXT = (csGLSECONDARYCOLOR3UIVEXT) gl->GetProcAddress ("glSecondaryColor3uivEXT"))) allclear = false;
      if (!(glSecondaryColorPointerEXT = (csGLSECONDARYCOLORPOINTEREXT) gl->GetProcAddress ("glSecondaryColorPointerEXT"))) allclear = false;
      if (CS_GL_EXT_secondary_color = allclear)
        printf ("GL Extension 'GL_EXT_secondary_color' found and used.\n");
    }

    // GL_EXT_separate_specular_color
    CS_GL_EXT_separate_specular_color = (strstr (extensions, "GL_EXT_separate_specular_color") != NULL);
    if (CS_GL_EXT_separate_specular_color)
    {
      allclear = true;
      if (CS_GL_EXT_separate_specular_color = allclear)
        printf ("GL Extension 'GL_EXT_separate_specular_color' found and used.\n");
    }

    // GL_EXT_shadow_funcs
    CS_GL_EXT_shadow_funcs = (strstr (extensions, "GL_EXT_shadow_funcs") != NULL);
    if (CS_GL_EXT_shadow_funcs)
    {
      allclear = true;
      if (CS_GL_EXT_shadow_funcs = allclear)
        printf ("GL Extension 'GL_EXT_shadow_funcs' found and used.\n");
    }

    // GL_EXT_shared_texture_palette
    CS_GL_EXT_shared_texture_palette = (strstr (extensions, "GL_EXT_shared_texture_palette") != NULL);
    if (CS_GL_EXT_shared_texture_palette)
    {
      allclear = true;
      if (CS_GL_EXT_shared_texture_palette = allclear)
        printf ("GL Extension 'GL_EXT_shared_texture_palette' found and used.\n");
    }

    // GL_EXT_stencil_two_side
    CS_GL_EXT_stencil_two_side = (strstr (extensions, "GL_EXT_stencil_two_side") != NULL);
    if (CS_GL_EXT_stencil_two_side)
    {
      allclear = true;
      if (!(glActiveStencilFaceEXT = (csGLACTIVESTENCILFACEEXT) gl->GetProcAddress ("glActiveStencilFaceEXT"))) allclear = false;
      if (CS_GL_EXT_stencil_two_side = allclear)
        printf ("GL Extension 'GL_EXT_stencil_two_side' found and used.\n");
    }

    // GL_EXT_stencil_wrap
    CS_GL_EXT_stencil_wrap = (strstr (extensions, "GL_EXT_stencil_wrap") != NULL);
    if (CS_GL_EXT_stencil_wrap)
    {
      allclear = true;
      if (CS_GL_EXT_stencil_wrap = allclear)
        printf ("GL Extension 'GL_EXT_stencil_wrap' found and used.\n");
    }

    // GL_EXT_subtexture
    CS_GL_EXT_subtexture = (strstr (extensions, "GL_EXT_subtexture") != NULL);
    if (CS_GL_EXT_subtexture)
    {
      allclear = true;
      if (!(glTexSubImage1DEXT = (csGLTEXSUBIMAGE1DEXT) gl->GetProcAddress ("glTexSubImage1DEXT"))) allclear = false;
      if (!(glTexSubImage2DEXT = (csGLTEXSUBIMAGE2DEXT) gl->GetProcAddress ("glTexSubImage2DEXT"))) allclear = false;
      if (!(glTexSubImage3DEXT = (csGLTEXSUBIMAGE3DEXT) gl->GetProcAddress ("glTexSubImage3DEXT"))) allclear = false;
      if (CS_GL_EXT_subtexture = allclear)
        printf ("GL Extension 'GL_EXT_subtexture' found and used.\n");
    }

    // GL_EXT_texture3D
    CS_GL_EXT_texture3D = (strstr (extensions, "GL_EXT_texture3D") != NULL);
    if (CS_GL_EXT_texture3D)
    {
      allclear = true;
      if (!(glTexImage3DEXT = (csGLTEXIMAGE3DEXT) gl->GetProcAddress ("glTexImage3DEXT"))) allclear = false;
      if (CS_GL_EXT_texture3D = allclear)
        printf ("GL Extension 'GL_EXT_texture3D' found and used.\n");
    }

    // GL_EXT_texture_compression_s3tc
    CS_GL_EXT_texture_compression_s3tc = (strstr (extensions, "GL_EXT_texture_compression_s3tc") != NULL);
    if (CS_GL_EXT_texture_compression_s3tc)
    {
      allclear = true;
      if (CS_GL_EXT_texture_compression_s3tc = allclear)
        printf ("GL Extension 'GL_EXT_texture_compression_s3tc' found and used.\n");
    }

    // GL_EXT_texture_env_add
    CS_GL_EXT_texture_env_add = (strstr (extensions, "GL_EXT_texture_env_add") != NULL);
    if (CS_GL_EXT_texture_env_add)
    {
      allclear = true;
      if (CS_GL_EXT_texture_env_add = allclear)
        printf ("GL Extension 'GL_EXT_texture_env_add' found and used.\n");
    }

    // GL_EXT_texture_env_combine
    CS_GL_EXT_texture_env_combine = (strstr (extensions, "GL_EXT_texture_env_combine") != NULL);
    if (CS_GL_EXT_texture_env_combine)
    {
      allclear = true;
      if (CS_GL_EXT_texture_env_combine = allclear)
        printf ("GL Extension 'GL_EXT_texture_env_combine' found and used.\n");
    }

    // GL_EXT_texture_env_dot3
    CS_GL_EXT_texture_env_dot3 = (strstr (extensions, "GL_EXT_texture_env_dot3") != NULL);
    if (CS_GL_EXT_texture_env_dot3)
    {
      allclear = true;
      if (CS_GL_EXT_texture_env_dot3 = allclear)
        printf ("GL Extension 'GL_EXT_texture_env_dot3' found and used.\n");
    }

    // GL_EXT_texture_filter_anisotropic
    CS_GL_EXT_texture_filter_anisotropic = (strstr (extensions, "GL_EXT_texture_filter_anisotropic") != NULL);
    if (CS_GL_EXT_texture_filter_anisotropic)
    {
      allclear = true;
      if (CS_GL_EXT_texture_filter_anisotropic = allclear)
        printf ("GL Extension 'GL_EXT_texture_filter_anisotropic' found and used.\n");
    }

    // GL_EXT_texture_lod_bias
    CS_GL_EXT_texture_lod_bias = (strstr (extensions, "GL_EXT_texture_lod_bias") != NULL);
    if (CS_GL_EXT_texture_lod_bias)
    {
      allclear = true;
      if (CS_GL_EXT_texture_lod_bias = allclear)
        printf ("GL Extension 'GL_EXT_texture_lod_bias' found and used.\n");
    }

    // GL_EXT_texture_object
    CS_GL_EXT_texture_object = (strstr (extensions, "GL_EXT_texture_object") != NULL);
    if (CS_GL_EXT_texture_object)
    {
      allclear = true;
      if (!(glGenTexturesEXT = (csGLGENTEXTURESEXT) gl->GetProcAddress ("glGenTexturesEXT"))) allclear = false;
      if (!(glDeleteTexturesEXT = (csGLDELETETEXTURESEXT) gl->GetProcAddress ("glDeleteTexturesEXT"))) allclear = false;
      if (!(glBindTextureEXT = (csGLBINDTEXTUREEXT) gl->GetProcAddress ("glBindTextureEXT"))) allclear = false;
      if (!(glPrioritizeTexturesEXT = (csGLPRIORITIZETEXTURESEXT) gl->GetProcAddress ("glPrioritizeTexturesEXT"))) allclear = false;
      if (!(glAreTexturesResidentEXT = (csGLARETEXTURESRESIDENTEXT) gl->GetProcAddress ("glAreTexturesResidentEXT"))) allclear = false;
      if (!(glIsTextureEXT = (csGLISTEXTUREEXT) gl->GetProcAddress ("glIsTextureEXT"))) allclear = false;
      if (CS_GL_EXT_texture_object = allclear)
        printf ("GL Extension 'GL_EXT_texture_object' found and used.\n");
    }

    // GL_EXT_vertex_array
    CS_GL_EXT_vertex_array = (strstr (extensions, "GL_EXT_vertex_array") != NULL);
    if (CS_GL_EXT_vertex_array)
    {
      allclear = true;
      if (!(glArrayElementEXT = (csGLARRAYELEMENTEXT) gl->GetProcAddress ("glArrayElementEXT"))) allclear = false;
      if (!(glDrawArraysEXT = (csGLDRAWARRAYSEXT) gl->GetProcAddress ("glDrawArraysEXT"))) allclear = false;
      if (!(glVertexPointerEXT = (csGLVERTEXPOINTEREXT) gl->GetProcAddress ("glVertexPointerEXT"))) allclear = false;
      if (!(glNormalPointerEXT = (csGLNORMALPOINTEREXT) gl->GetProcAddress ("glNormalPointerEXT"))) allclear = false;
      if (!(glColorPointerEXT = (csGLCOLORPOINTEREXT) gl->GetProcAddress ("glColorPointerEXT"))) allclear = false;
      if (!(glIndexPointerEXT = (csGLINDEXPOINTEREXT) gl->GetProcAddress ("glIndexPointerEXT"))) allclear = false;
      if (!(glTexCoordPointerEXT = (csGLTEXCOORDPOINTEREXT) gl->GetProcAddress ("glTexCoordPointerEXT"))) allclear = false;
      if (!(glEdgeFlagPointerEXT = (csGLEDGEFLAGPOINTEREXT) gl->GetProcAddress ("glEdgeFlagPointerEXT"))) allclear = false;
      if (!(glGetPointervEXT = (csGLGETPOINTERVEXT) gl->GetProcAddress ("glGetPointervEXT"))) allclear = false;
      if (CS_GL_EXT_vertex_array = allclear)
        printf ("GL Extension 'GL_EXT_vertex_array' found and used.\n");
    }

    // GL_EXT_vertex_shader
    CS_GL_EXT_vertex_shader = (strstr (extensions, "GL_EXT_vertex_shader") != NULL);
    if (CS_GL_EXT_vertex_shader)
    {
      allclear = true;
      if (!(glBeginVertexShaderEXT = (csGLBEGINVERTEXSHADEREXT) gl->GetProcAddress ("glBeginVertexShaderEXT"))) allclear = false;
      if (!(glEndVertexShaderEXT = (csGLENDVERTEXSHADEREXT) gl->GetProcAddress ("glEndVertexShaderEXT"))) allclear = false;
      if (!(glBindVertexShaderEXT = (csGLBINDVERTEXSHADEREXT) gl->GetProcAddress ("glBindVertexShaderEXT"))) allclear = false;
      if (!(glGenVertexShadersEXT = (csGLGENVERTEXSHADERSEXT) gl->GetProcAddress ("glGenVertexShadersEXT"))) allclear = false;
      if (!(glDeleteVertexShaderEXT = (csGLDELETEVERTEXSHADEREXT) gl->GetProcAddress ("glDeleteVertexShaderEXT"))) allclear = false;
      if (!(glShaderOp1EXT = (csGLSHADEROP1EXT) gl->GetProcAddress ("glShaderOp1EXT"))) allclear = false;
      if (!(glShaderOp2EXT = (csGLSHADEROP2EXT) gl->GetProcAddress ("glShaderOp2EXT"))) allclear = false;
      if (!(glShaderOp3EXT = (csGLSHADEROP3EXT) gl->GetProcAddress ("glShaderOp3EXT"))) allclear = false;
      if (!(glSwizzleEXT = (csGLSWIZZLEEXT) gl->GetProcAddress ("glSwizzleEXT"))) allclear = false;
      if (!(glWriteMaskEXT = (csGLWRITEMASKEXT) gl->GetProcAddress ("glWriteMaskEXT"))) allclear = false;
      if (!(glInsertComponentEXT = (csGLINSERTCOMPONENTEXT) gl->GetProcAddress ("glInsertComponentEXT"))) allclear = false;
      if (!(glExtractComponentEXT = (csGLEXTRACTCOMPONENTEXT) gl->GetProcAddress ("glExtractComponentEXT"))) allclear = false;
      if (!(glGenSymbolsEXT = (csGLGENSYMBOLSEXT) gl->GetProcAddress ("glGenSymbolsEXT"))) allclear = false;
      if (!(glSetInvariantEXT = (csGLSETINVARIANTEXT) gl->GetProcAddress ("glSetInvariantEXT"))) allclear = false;
      if (!(glSetLocalConstantEXT = (csGLSETLOCALCONSTANTEXT) gl->GetProcAddress ("glSetLocalConstantEXT"))) allclear = false;
      if (!(glVariantbvEXT = (csGLVARIANTBVEXT) gl->GetProcAddress ("glVariantbvEXT"))) allclear = false;
      if (!(glVariantsvEXT = (csGLVARIANTSVEXT) gl->GetProcAddress ("glVariantsvEXT"))) allclear = false;
      if (!(glVariantivEXT = (csGLVARIANTIVEXT) gl->GetProcAddress ("glVariantivEXT"))) allclear = false;
      if (!(glVariantfvEXT = (csGLVARIANTFVEXT) gl->GetProcAddress ("glVariantfvEXT"))) allclear = false;
      if (!(glVariantdvEXT = (csGLVARIANTDVEXT) gl->GetProcAddress ("glVariantdvEXT"))) allclear = false;
      if (!(glVariantubvEXT = (csGLVARIANTUBVEXT) gl->GetProcAddress ("glVariantubvEXT"))) allclear = false;
      if (!(glVariantusvEXT = (csGLVARIANTUSVEXT) gl->GetProcAddress ("glVariantusvEXT"))) allclear = false;
      if (!(glVariantuivEXT = (csGLVARIANTUIVEXT) gl->GetProcAddress ("glVariantuivEXT"))) allclear = false;
      if (!(glVariantPointerEXT = (csGLVARIANTPOINTEREXT) gl->GetProcAddress ("glVariantPointerEXT"))) allclear = false;
      if (!(glEnableVariantClientStateEXT = (csGLENABLEVARIANTCLIENTSTATEEXT) gl->GetProcAddress ("glEnableVariantClientStateEXT"))) allclear = false;
      if (!(glDisableVariantClientStateEXT = (csGLDISABLEVARIANTCLIENTSTATEEXT) gl->GetProcAddress ("glDisableVariantClientStateEXT"))) allclear = false;
      if (!(glBindLightParameterEXT = (csGLBINDLIGHTPARAMETEREXT) gl->GetProcAddress ("glBindLightParameterEXT"))) allclear = false;
      if (!(glBindMaterialParameterEXT = (csGLBINDMATERIALPARAMETEREXT) gl->GetProcAddress ("glBindMaterialParameterEXT"))) allclear = false;
      if (!(glBindTexGenParameterEXT = (csGLBINDTEXGENPARAMETEREXT) gl->GetProcAddress ("glBindTexGenParameterEXT"))) allclear = false;
      if (!(glBindTextureUnitParameterEXT = (csGLBINDTEXTUREUNITPARAMETEREXT) gl->GetProcAddress ("glBindTextureUnitParameterEXT"))) allclear = false;
      if (!(glBindParameterEXT = (csGLBINDPARAMETEREXT) gl->GetProcAddress ("glBindParameterEXT"))) allclear = false;
      if (!(glIsVariantEnabledEXT = (csGLISVARIANTENABLEDEXT) gl->GetProcAddress ("glIsVariantEnabledEXT"))) allclear = false;
      if (!(glGetVariantBooleanvEXT = (csGLGETVARIANTBOOLEANVEXT) gl->GetProcAddress ("glGetVariantBooleanvEXT"))) allclear = false;
      if (!(glGetVariantIntegervEXT = (csGLGETVARIANTINTEGERVEXT) gl->GetProcAddress ("glGetVariantIntegervEXT"))) allclear = false;
      if (!(glGetVariantFloatvEXT = (csGLGETVARIANTFLOATVEXT) gl->GetProcAddress ("glGetVariantFloatvEXT"))) allclear = false;
      if (!(glGetVariantPointervEXT = (csGLGETVARIANTPOINTERVEXT) gl->GetProcAddress ("glGetVariantPointervEXT"))) allclear = false;
      if (!(glGetInvariantBooleanvEXT = (csGLGETINVARIANTBOOLEANVEXT) gl->GetProcAddress ("glGetInvariantBooleanvEXT"))) allclear = false;
      if (!(glGetInvariantIntegervEXT = (csGLGETINVARIANTINTEGERVEXT) gl->GetProcAddress ("glGetInvariantIntegervEXT"))) allclear = false;
      if (!(glGetInvariantFloatvEXT = (csGLGETINVARIANTFLOATVEXT) gl->GetProcAddress ("glGetInvariantFloatvEXT"))) allclear = false;
      if (!(glGetLocalConstantBooleanvEXT = (csGLGETLOCALCONSTANTBOOLEANVEXT) gl->GetProcAddress ("glGetLocalConstantBooleanvEXT"))) allclear = false;
      if (!(glGetLocalConstantIntegervEXT = (csGLGETLOCALCONSTANTINTEGERVEXT) gl->GetProcAddress ("glGetLocalConstantIntegervEXT"))) allclear = false;
      if (!(glGetLocalConstantFloatvEXT = (csGLGETLOCALCONSTANTFLOATVEXT) gl->GetProcAddress ("glGetLocalConstantFloatvEXT"))) allclear = false;
      if (CS_GL_EXT_vertex_shader = allclear)
        printf ("GL Extension 'GL_EXT_vertex_shader' found and used.\n");
    }

    // GL_EXT_vertex_weighting
    CS_GL_EXT_vertex_weighting = (strstr (extensions, "GL_EXT_vertex_weighting") != NULL);
    if (CS_GL_EXT_vertex_weighting)
    {
      allclear = true;
      if (!(glVertexWeightfEXT = (csGLVERTEXWEIGHTFEXT) gl->GetProcAddress ("glVertexWeightfEXT"))) allclear = false;
      if (!(glVertexWeightfvEXT = (csGLVERTEXWEIGHTFVEXT) gl->GetProcAddress ("glVertexWeightfvEXT"))) allclear = false;
      if (!(glVertexWeightPointerEXT = (csGLVERTEXWEIGHTPOINTEREXT) gl->GetProcAddress ("glVertexWeightPointerEXT"))) allclear = false;
      if (CS_GL_EXT_vertex_weighting = allclear)
        printf ("GL Extension 'GL_EXT_vertex_weighting' found and used.\n");
    }

    // GL_HP_occlusion_test
    CS_GL_HP_occlusion_test = (strstr (extensions, "GL_HP_occlusion_test") != NULL);
    if (CS_GL_HP_occlusion_test)
    {
      allclear = true;
      if (CS_GL_HP_occlusion_test = allclear)
        printf ("GL Extension 'GL_HP_occlusion_test' found and used.\n");
    }

    // GL_NV_blend_square
    CS_GL_NV_blend_square = (strstr (extensions, "GL_NV_blend_square") != NULL);
    if (CS_GL_NV_blend_square)
    {
      allclear = true;
      if (CS_GL_NV_blend_square = allclear)
        printf ("GL Extension 'GL_NV_blend_square' found and used.\n");
    }

    // GL_NV_copy_depth_to_color
    CS_GL_NV_copy_depth_to_color = (strstr (extensions, "GL_NV_copy_depth_to_color") != NULL);
    if (CS_GL_NV_copy_depth_to_color)
    {
      allclear = true;
      if (CS_GL_NV_copy_depth_to_color = allclear)
        printf ("GL Extension 'GL_NV_copy_depth_to_color' found and used.\n");
    }

    // GL_NV_depth_clamp
    CS_GL_NV_depth_clamp = (strstr (extensions, "GL_NV_depth_clamp") != NULL);
    if (CS_GL_NV_depth_clamp)
    {
      allclear = true;
      if (CS_GL_NV_depth_clamp = allclear)
        printf ("GL Extension 'GL_NV_depth_clamp' found and used.\n");
    }

    // GL_NV_evaluators
    CS_GL_NV_evaluators = (strstr (extensions, "GL_NV_evaluators") != NULL);
    if (CS_GL_NV_evaluators)
    {
      allclear = true;
      if (!(glMapControlPointsNV = (csGLMAPCONTROLPOINTSNV) gl->GetProcAddress ("glMapControlPointsNV"))) allclear = false;
      if (!(glMapParameterivNV = (csGLMAPPARAMETERIVNV) gl->GetProcAddress ("glMapParameterivNV"))) allclear = false;
      if (!(glMapParameterfvNV = (csGLMAPPARAMETERFVNV) gl->GetProcAddress ("glMapParameterfvNV"))) allclear = false;
      if (!(glGetMapControlPointsNV = (csGLGETMAPCONTROLPOINTSNV) gl->GetProcAddress ("glGetMapControlPointsNV"))) allclear = false;
      if (!(glGetMapParameterivNV = (csGLGETMAPPARAMETERIVNV) gl->GetProcAddress ("glGetMapParameterivNV"))) allclear = false;
      if (!(glGetMapParameterfvNV = (csGLGETMAPPARAMETERFVNV) gl->GetProcAddress ("glGetMapParameterfvNV"))) allclear = false;
      if (!(glGetMapAttribParameterivNV = (csGLGETMAPATTRIBPARAMETERIVNV) gl->GetProcAddress ("glGetMapAttribParameterivNV"))) allclear = false;
      if (!(glGetMapAttribParameterfvNV = (csGLGETMAPATTRIBPARAMETERFVNV) gl->GetProcAddress ("glGetMapAttribParameterfvNV"))) allclear = false;
      if (!(glEvalMapsNV = (csGLEVALMAPSNV) gl->GetProcAddress ("glEvalMapsNV"))) allclear = false;
      if (CS_GL_NV_evaluators = allclear)
        printf ("GL Extension 'GL_NV_evaluators' found and used.\n");
    }

    // GL_NV_fence
    CS_GL_NV_fence = (strstr (extensions, "GL_NV_fence") != NULL);
    if (CS_GL_NV_fence)
    {
      allclear = true;
      if (!(glGenFencesNV = (csGLGENFENCESNV) gl->GetProcAddress ("glGenFencesNV"))) allclear = false;
      if (!(glDeleteFencesNV = (csGLDELETEFENCESNV) gl->GetProcAddress ("glDeleteFencesNV"))) allclear = false;
      if (!(glSetFenceNV = (csGLSETFENCENV) gl->GetProcAddress ("glSetFenceNV"))) allclear = false;
      if (!(glTestFenceNV = (csGLTESTFENCENV) gl->GetProcAddress ("glTestFenceNV"))) allclear = false;
      if (!(glFinishFenceNV = (csGLFINISHFENCENV) gl->GetProcAddress ("glFinishFenceNV"))) allclear = false;
      if (!(glIsFenceNV = (csGLISFENCENV) gl->GetProcAddress ("glIsFenceNV"))) allclear = false;
      if (!(glGetFenceivNV = (csGLGETFENCEIVNV) gl->GetProcAddress ("glGetFenceivNV"))) allclear = false;
      if (CS_GL_NV_fence = allclear)
        printf ("GL Extension 'GL_NV_fence' found and used.\n");
    }

    // GL_NV_fog_distance
    CS_GL_NV_fog_distance = (strstr (extensions, "GL_NV_fog_distance") != NULL);
    if (CS_GL_NV_fog_distance)
    {
      allclear = true;
      if (CS_GL_NV_fog_distance = allclear)
        printf ("GL Extension 'GL_NV_fog_distance' found and used.\n");
    }

    // GL_NV_light_max_exponent
    CS_GL_NV_light_max_exponent = (strstr (extensions, "GL_NV_light_max_exponent") != NULL);
    if (CS_GL_NV_light_max_exponent)
    {
      allclear = true;
      if (CS_GL_NV_light_max_exponent = allclear)
        printf ("GL Extension 'GL_NV_light_max_exponent' found and used.\n");
    }

    // GL_NV_multisample_filter_hint
    CS_GL_NV_multisample_filter_hint = (strstr (extensions, "GL_NV_multisample_filter_hint") != NULL);
    if (CS_GL_NV_multisample_filter_hint)
    {
      allclear = true;
      if (CS_GL_NV_multisample_filter_hint = allclear)
        printf ("GL Extension 'GL_NV_multisample_filter_hint' found and used.\n");
    }

    // GL_NV_occlusion_query
    CS_GL_NV_occlusion_query = (strstr (extensions, "GL_NV_occlusion_query") != NULL);
    if (CS_GL_NV_occlusion_query)
    {
      allclear = true;
      if (!(glGenOcclusionQueriesNV = (csGLGENOCCLUSIONQUERIESNV) gl->GetProcAddress ("glGenOcclusionQueriesNV"))) allclear = false;
      if (!(glDeleteOcclusionQueriesNV = (csGLDELETEOCCLUSIONQUERIESNV) gl->GetProcAddress ("glDeleteOcclusionQueriesNV"))) allclear = false;
      if (!(glIsOcclusionQueryNV = (csGLISOCCLUSIONQUERYNV) gl->GetProcAddress ("glIsOcclusionQueryNV"))) allclear = false;
      if (!(glBeginOcclusionQueryNV = (csGLBEGINOCCLUSIONQUERYNV) gl->GetProcAddress ("glBeginOcclusionQueryNV"))) allclear = false;
      if (!(glEndOcclusionQueryNV = (csGLENDOCCLUSIONQUERYNV) gl->GetProcAddress ("glEndOcclusionQueryNV"))) allclear = false;
      if (!(glGetOcclusionQueryivNV = (csGLGETOCCLUSIONQUERYIVNV) gl->GetProcAddress ("glGetOcclusionQueryivNV"))) allclear = false;
      if (!(glGetOcclusionQueryuivNV = (csGLGETOCCLUSIONQUERYUIVNV) gl->GetProcAddress ("glGetOcclusionQueryuivNV"))) allclear = false;
      if (CS_GL_NV_occlusion_query = allclear)
        printf ("GL Extension 'GL_NV_occlusion_query' found and used.\n");
    }

    // GL_NV_packed_depth_stencil
    CS_GL_NV_packed_depth_stencil = (strstr (extensions, "GL_NV_packed_depth_stencil") != NULL);
    if (CS_GL_NV_packed_depth_stencil)
    {
      allclear = true;
      if (CS_GL_NV_packed_depth_stencil = allclear)
        printf ("GL Extension 'GL_NV_packed_depth_stencil' found and used.\n");
    }

    // GL_NV_point_sprite
    CS_GL_NV_point_sprite = (strstr (extensions, "GL_NV_point_sprite") != NULL);
    if (CS_GL_NV_point_sprite)
    {
      allclear = true;
      if (!(glPointParameteriNV = (csGLPOINTPARAMETERINV) gl->GetProcAddress ("glPointParameteriNV"))) allclear = false;
      if (!(glPointParameterivNV = (csGLPOINTPARAMETERIVNV) gl->GetProcAddress ("glPointParameterivNV"))) allclear = false;
      if (CS_GL_NV_point_sprite = allclear)
        printf ("GL Extension 'GL_NV_point_sprite' found and used.\n");
    }

    // GL_NV_register_combiners
    CS_GL_NV_register_combiners = (strstr (extensions, "GL_NV_register_combiners") != NULL);
    if (CS_GL_NV_register_combiners)
    {
      allclear = true;
      if (!(glCombinerParameterfvNV = (csGLCOMBINERPARAMETERFVNV) gl->GetProcAddress ("glCombinerParameterfvNV"))) allclear = false;
      if (!(glCombinerParameterivNV = (csGLCOMBINERPARAMETERIVNV) gl->GetProcAddress ("glCombinerParameterivNV"))) allclear = false;
      if (!(glCombinerParameterfNV = (csGLCOMBINERPARAMETERFNV) gl->GetProcAddress ("glCombinerParameterfNV"))) allclear = false;
      if (!(glCombinerParameteriNV = (csGLCOMBINERPARAMETERINV) gl->GetProcAddress ("glCombinerParameteriNV"))) allclear = false;
      if (!(glCombinerInputNV = (csGLCOMBINERINPUTNV) gl->GetProcAddress ("glCombinerInputNV"))) allclear = false;
      if (!(glCombinerOutputNV = (csGLCOMBINEROUTPUTNV) gl->GetProcAddress ("glCombinerOutputNV"))) allclear = false;
      if (!(glFinalCombinerInputNV = (csGLFINALCOMBINERINPUTNV) gl->GetProcAddress ("glFinalCombinerInputNV"))) allclear = false;
      if (!(glGetCombinerInputParameterfvNV = (csGLGETCOMBINERINPUTPARAMETERFVNV) gl->GetProcAddress ("glGetCombinerInputParameterfvNV"))) allclear = false;
      if (!(glGetCombinerInputParameterivNV = (csGLGETCOMBINERINPUTPARAMETERIVNV) gl->GetProcAddress ("glGetCombinerInputParameterivNV"))) allclear = false;
      if (!(glGetCombinerOutputParameterfvNV = (csGLGETCOMBINEROUTPUTPARAMETERFVNV) gl->GetProcAddress ("glGetCombinerOutputParameterfvNV"))) allclear = false;
      if (!(glGetCombinerOutputParameterivNV = (csGLGETCOMBINEROUTPUTPARAMETERIVNV) gl->GetProcAddress ("glGetCombinerOutputParameterivNV"))) allclear = false;
      if (!(glGetFinalCombinerInputParameterfvNV = (csGLGETFINALCOMBINERINPUTPARAMETERFVNV) gl->GetProcAddress ("glGetFinalCombinerInputParameterfvNV"))) allclear = false;
      if (!(glGetFinalCombinerInputParameterivNV = (csGLGETFINALCOMBINERINPUTPARAMETERIVNV) gl->GetProcAddress ("glGetFinalCombinerInputParameterivNV"))) allclear = false;
      if (CS_GL_NV_register_combiners = allclear)
        printf ("GL Extension 'GL_NV_register_combiners' found and used.\n");
    }

    // GL_NV_register_combiners2
    CS_GL_NV_register_combiners2 = (strstr (extensions, "GL_NV_register_combiners2") != NULL);
    if (CS_GL_NV_register_combiners2)
    {
      allclear = true;
      if (!(glCombinerStageParameterfvNV = (csGLCOMBINERSTAGEPARAMETERFVNV) gl->GetProcAddress ("glCombinerStageParameterfvNV"))) allclear = false;
      if (!(glGetCombinerStageParameterfvNV = (csGLGETCOMBINERSTAGEPARAMETERFVNV) gl->GetProcAddress ("glGetCombinerStageParameterfvNV"))) allclear = false;
      if (CS_GL_NV_register_combiners2 = allclear)
        printf ("GL Extension 'GL_NV_register_combiners2' found and used.\n");
    }

    // GL_NV_texgen_emboss
    CS_GL_NV_texgen_emboss = (strstr (extensions, "GL_NV_texgen_emboss") != NULL);
    if (CS_GL_NV_texgen_emboss)
    {
      allclear = true;
      if (CS_GL_NV_texgen_emboss = allclear)
        printf ("GL Extension 'GL_NV_texgen_emboss' found and used.\n");
    }

    // GL_NV_texgen_reflection
    CS_GL_NV_texgen_reflection = (strstr (extensions, "GL_NV_texgen_reflection") != NULL);
    if (CS_GL_NV_texgen_reflection)
    {
      allclear = true;
      if (CS_GL_NV_texgen_reflection = allclear)
        printf ("GL Extension 'GL_NV_texgen_reflection' found and used.\n");
    }

    // GL_NV_texture_compression_vtc
    CS_GL_NV_texture_compression_vtc = (strstr (extensions, "GL_NV_texture_compression_vtc") != NULL);
    if (CS_GL_NV_texture_compression_vtc)
    {
      allclear = true;
      if (CS_GL_NV_texture_compression_vtc = allclear)
        printf ("GL Extension 'GL_NV_texture_compression_vtc' found and used.\n");
    }

    // GL_NV_texture_env_combine4
    CS_GL_NV_texture_env_combine4 = (strstr (extensions, "GL_NV_texture_env_combine4") != NULL);
    if (CS_GL_NV_texture_env_combine4)
    {
      allclear = true;
      if (CS_GL_NV_texture_env_combine4 = allclear)
        printf ("GL Extension 'GL_NV_texture_env_combine4' found and used.\n");
    }

    // GL_NV_texture_rectangle
    CS_GL_NV_texture_rectangle = (strstr (extensions, "GL_NV_texture_rectangle") != NULL);
    if (CS_GL_NV_texture_rectangle)
    {
      allclear = true;
      if (CS_GL_NV_texture_rectangle = allclear)
        printf ("GL Extension 'GL_NV_texture_rectangle' found and used.\n");
    }

    // GL_NV_texture_shader
    CS_GL_NV_texture_shader = (strstr (extensions, "GL_NV_texture_shader") != NULL);
    if (CS_GL_NV_texture_shader)
    {
      allclear = true;
      if (CS_GL_NV_texture_shader = allclear)
        printf ("GL Extension 'GL_NV_texture_shader' found and used.\n");
    }

    // GL_NV_texture_shader2
    CS_GL_NV_texture_shader2 = (strstr (extensions, "GL_NV_texture_shader2") != NULL);
    if (CS_GL_NV_texture_shader2)
    {
      allclear = true;
      if (CS_GL_NV_texture_shader2 = allclear)
        printf ("GL Extension 'GL_NV_texture_shader2' found and used.\n");
    }

    // GL_NV_texture_shader3
    CS_GL_NV_texture_shader3 = (strstr (extensions, "GL_NV_texture_shader3") != NULL);
    if (CS_GL_NV_texture_shader3)
    {
      allclear = true;
      if (CS_GL_NV_texture_shader3 = allclear)
        printf ("GL Extension 'GL_NV_texture_shader3' found and used.\n");
    }

    // GL_NV_vertex_array_range
    CS_GL_NV_vertex_array_range = (strstr (extensions, "GL_NV_vertex_array_range") != NULL);
    if (CS_GL_NV_vertex_array_range)
    {
      allclear = true;
      if (!(glVertexArrayRangeNV = (csGLVERTEXARRAYRANGENV) gl->GetProcAddress ("glVertexArrayRangeNV"))) allclear = false;
      if (!(glFlushVertexArrayRangeNV = (csGLFLUSHVERTEXARRAYRANGENV) gl->GetProcAddress ("glFlushVertexArrayRangeNV"))) allclear = false;
      if (!(wglAllocateMemoryNV = (csWGLALLOCATEMEMORYNV) gl->GetProcAddress ("wglAllocateMemoryNV"))) allclear = false;
      if (!(wglFreeMemoryNV = (csWGLFREEMEMORYNV) gl->GetProcAddress ("wglFreeMemoryNV"))) allclear = false;
      if (CS_GL_NV_vertex_array_range = allclear)
        printf ("GL Extension 'GL_NV_vertex_array_range' found and used.\n");
    }

    // GL_NV_vertex_array_range2
    CS_GL_NV_vertex_array_range2 = (strstr (extensions, "GL_NV_vertex_array_range2") != NULL);
    if (CS_GL_NV_vertex_array_range2)
    {
      allclear = true;
      if (CS_GL_NV_vertex_array_range2 = allclear)
        printf ("GL Extension 'GL_NV_vertex_array_range2' found and used.\n");
    }

    // GL_NV_vertex_program
    CS_GL_NV_vertex_program = (strstr (extensions, "GL_NV_vertex_program") != NULL);
    if (CS_GL_NV_vertex_program)
    {
      allclear = true;
      if (!(glBindProgramNV = (csGLBINDPROGRAMNV) gl->GetProcAddress ("glBindProgramNV"))) allclear = false;
      if (!(glDeleteProgramsNV = (csGLDELETEPROGRAMSNV) gl->GetProcAddress ("glDeleteProgramsNV"))) allclear = false;
      if (!(glExecuteProgramNV = (csGLEXECUTEPROGRAMNV) gl->GetProcAddress ("glExecuteProgramNV"))) allclear = false;
      if (!(glGenProgramsNV = (csGLGENPROGRAMSNV) gl->GetProcAddress ("glGenProgramsNV"))) allclear = false;
      if (!(glAreProgramsResidentNV = (csGLAREPROGRAMSRESIDENTNV) gl->GetProcAddress ("glAreProgramsResidentNV"))) allclear = false;
      if (!(glRequestResidentProgramsNV = (csGLREQUESTRESIDENTPROGRAMSNV) gl->GetProcAddress ("glRequestResidentProgramsNV"))) allclear = false;
      if (!(glGetProgramParameterfvNV = (csGLGETPROGRAMPARAMETERFVNV) gl->GetProcAddress ("glGetProgramParameterfvNV"))) allclear = false;
      if (!(glGetProgramParameterdvNV = (csGLGETPROGRAMPARAMETERDVNV) gl->GetProcAddress ("glGetProgramParameterdvNV"))) allclear = false;
      if (!(glGetProgramivNV = (csGLGETPROGRAMIVNV) gl->GetProcAddress ("glGetProgramivNV"))) allclear = false;
      if (!(glGetProgramStringNV = (csGLGETPROGRAMSTRINGNV) gl->GetProcAddress ("glGetProgramStringNV"))) allclear = false;
      if (!(glGetTrackMatrixivNV = (csGLGETTRACKMATRIXIVNV) gl->GetProcAddress ("glGetTrackMatrixivNV"))) allclear = false;
      if (!(glGetVertexAttribdvNV = (csGLGETVERTEXATTRIBDVNV) gl->GetProcAddress ("glGetVertexAttribdvNV"))) allclear = false;
      if (!(glGetVertexAttribfvNV = (csGLGETVERTEXATTRIBFVNV) gl->GetProcAddress ("glGetVertexAttribfvNV"))) allclear = false;
      if (!(glGetVertexAttribivNV = (csGLGETVERTEXATTRIBIVNV) gl->GetProcAddress ("glGetVertexAttribivNV"))) allclear = false;
      if (!(glGetVertexAttribPointervNV = (csGLGETVERTEXATTRIBPOINTERVNV) gl->GetProcAddress ("glGetVertexAttribPointervNV"))) allclear = false;
      if (!(glIsProgramNV = (csGLISPROGRAMNV) gl->GetProcAddress ("glIsProgramNV"))) allclear = false;
      if (!(glLoadProgramNV = (csGLLOADPROGRAMNV) gl->GetProcAddress ("glLoadProgramNV"))) allclear = false;
      if (!(glProgramParameter4fNV = (csGLPROGRAMPARAMETER4FNV) gl->GetProcAddress ("glProgramParameter4fNV"))) allclear = false;
      if (!(glProgramParameter4fvNV = (csGLPROGRAMPARAMETER4FVNV) gl->GetProcAddress ("glProgramParameter4fvNV"))) allclear = false;
      if (!(glProgramParameters4dvNV = (csGLPROGRAMPARAMETERS4DVNV) gl->GetProcAddress ("glProgramParameters4dvNV"))) allclear = false;
      if (!(glProgramParameters4fvNV = (csGLPROGRAMPARAMETERS4FVNV) gl->GetProcAddress ("glProgramParameters4fvNV"))) allclear = false;
      if (!(glTrackMatrixNV = (csGLTRACKMATRIXNV) gl->GetProcAddress ("glTrackMatrixNV"))) allclear = false;
      if (!(glVertexAttribPointerNV = (csGLVERTEXATTRIBPOINTERNV) gl->GetProcAddress ("glVertexAttribPointerNV"))) allclear = false;
      if (!(glVertexAttrib1sNV = (csGLVERTEXATTRIB1SNV) gl->GetProcAddress ("glVertexAttrib1sNV"))) allclear = false;
      if (!(glVertexAttrib1fNV = (csGLVERTEXATTRIB1FNV) gl->GetProcAddress ("glVertexAttrib1fNV"))) allclear = false;
      if (!(glVertexAttrib1dNV = (csGLVERTEXATTRIB1DNV) gl->GetProcAddress ("glVertexAttrib1dNV"))) allclear = false;
      if (!(glVertexAttrib2sNV = (csGLVERTEXATTRIB2SNV) gl->GetProcAddress ("glVertexAttrib2sNV"))) allclear = false;
      if (!(glVertexAttrib2fNV = (csGLVERTEXATTRIB2FNV) gl->GetProcAddress ("glVertexAttrib2fNV"))) allclear = false;
      if (!(glVertexAttrib2dNV = (csGLVERTEXATTRIB2DNV) gl->GetProcAddress ("glVertexAttrib2dNV"))) allclear = false;
      if (!(glVertexAttrib3sNV = (csGLVERTEXATTRIB3SNV) gl->GetProcAddress ("glVertexAttrib3sNV"))) allclear = false;
      if (!(glVertexAttrib3fNV = (csGLVERTEXATTRIB3FNV) gl->GetProcAddress ("glVertexAttrib3fNV"))) allclear = false;
      if (!(glVertexAttrib3dNV = (csGLVERTEXATTRIB3DNV) gl->GetProcAddress ("glVertexAttrib3dNV"))) allclear = false;
      if (!(glVertexAttrib4sNV = (csGLVERTEXATTRIB4SNV) gl->GetProcAddress ("glVertexAttrib4sNV"))) allclear = false;
      if (!(glVertexAttrib4fNV = (csGLVERTEXATTRIB4FNV) gl->GetProcAddress ("glVertexAttrib4fNV"))) allclear = false;
      if (!(glVertexAttrib4dNV = (csGLVERTEXATTRIB4DNV) gl->GetProcAddress ("glVertexAttrib4dNV"))) allclear = false;
      if (!(glVertexAttrib4ubNV = (csGLVERTEXATTRIB4UBNV) gl->GetProcAddress ("glVertexAttrib4ubNV"))) allclear = false;
      if (!(glVertexAttrib1svNV = (csGLVERTEXATTRIB1SVNV) gl->GetProcAddress ("glVertexAttrib1svNV"))) allclear = false;
      if (!(glVertexAttrib1fvNV = (csGLVERTEXATTRIB1FVNV) gl->GetProcAddress ("glVertexAttrib1fvNV"))) allclear = false;
      if (!(glVertexAttrib1dvNV = (csGLVERTEXATTRIB1DVNV) gl->GetProcAddress ("glVertexAttrib1dvNV"))) allclear = false;
      if (!(glVertexAttrib2svNV = (csGLVERTEXATTRIB2SVNV) gl->GetProcAddress ("glVertexAttrib2svNV"))) allclear = false;
      if (!(glVertexAttrib2fvNV = (csGLVERTEXATTRIB2FVNV) gl->GetProcAddress ("glVertexAttrib2fvNV"))) allclear = false;
      if (!(glVertexAttrib2dvNV = (csGLVERTEXATTRIB2DVNV) gl->GetProcAddress ("glVertexAttrib2dvNV"))) allclear = false;
      if (!(glVertexAttrib3svNV = (csGLVERTEXATTRIB3SVNV) gl->GetProcAddress ("glVertexAttrib3svNV"))) allclear = false;
      if (!(glVertexAttrib3fvNV = (csGLVERTEXATTRIB3FVNV) gl->GetProcAddress ("glVertexAttrib3fvNV"))) allclear = false;
      if (!(glVertexAttrib3dvNV = (csGLVERTEXATTRIB3DVNV) gl->GetProcAddress ("glVertexAttrib3dvNV"))) allclear = false;
      if (!(glVertexAttrib4svNV = (csGLVERTEXATTRIB4SVNV) gl->GetProcAddress ("glVertexAttrib4svNV"))) allclear = false;
      if (!(glVertexAttrib4fvNV = (csGLVERTEXATTRIB4FVNV) gl->GetProcAddress ("glVertexAttrib4fvNV"))) allclear = false;
      if (!(glVertexAttrib4dvNV = (csGLVERTEXATTRIB4DVNV) gl->GetProcAddress ("glVertexAttrib4dvNV"))) allclear = false;
      if (!(glVertexAttrib4ubvNV = (csGLVERTEXATTRIB4UBVNV) gl->GetProcAddress ("glVertexAttrib4ubvNV"))) allclear = false;
      if (!(glVertexAttribs1svNV = (csGLVERTEXATTRIBS1SVNV) gl->GetProcAddress ("glVertexAttribs1svNV"))) allclear = false;
      if (!(glVertexAttribs1fvNV = (csGLVERTEXATTRIBS1FVNV) gl->GetProcAddress ("glVertexAttribs1fvNV"))) allclear = false;
      if (!(glVertexAttribs1dvNV = (csGLVERTEXATTRIBS1DVNV) gl->GetProcAddress ("glVertexAttribs1dvNV"))) allclear = false;
      if (!(glVertexAttribs2svNV = (csGLVERTEXATTRIBS2SVNV) gl->GetProcAddress ("glVertexAttribs2svNV"))) allclear = false;
      if (!(glVertexAttribs2fvNV = (csGLVERTEXATTRIBS2FVNV) gl->GetProcAddress ("glVertexAttribs2fvNV"))) allclear = false;
      if (!(glVertexAttribs2dvNV = (csGLVERTEXATTRIBS2DVNV) gl->GetProcAddress ("glVertexAttribs2dvNV"))) allclear = false;
      if (!(glVertexAttribs3svNV = (csGLVERTEXATTRIBS3SVNV) gl->GetProcAddress ("glVertexAttribs3svNV"))) allclear = false;
      if (!(glVertexAttribs3fvNV = (csGLVERTEXATTRIBS3FVNV) gl->GetProcAddress ("glVertexAttribs3fvNV"))) allclear = false;
      if (!(glVertexAttribs3dvNV = (csGLVERTEXATTRIBS3DVNV) gl->GetProcAddress ("glVertexAttribs3dvNV"))) allclear = false;
      if (!(glVertexAttribs4svNV = (csGLVERTEXATTRIBS4SVNV) gl->GetProcAddress ("glVertexAttribs4svNV"))) allclear = false;
      if (!(glVertexAttribs4fvNV = (csGLVERTEXATTRIBS4FVNV) gl->GetProcAddress ("glVertexAttribs4fvNV"))) allclear = false;
      if (!(glVertexAttribs4dvNV = (csGLVERTEXATTRIBS4DVNV) gl->GetProcAddress ("glVertexAttribs4dvNV"))) allclear = false;
      if (!(glVertexAttribs4ubvNV = (csGLVERTEXATTRIBS4UBVNV) gl->GetProcAddress ("glVertexAttribs4ubvNV"))) allclear = false;
      if (CS_GL_NV_vertex_program = allclear)
        printf ("GL Extension 'GL_NV_vertex_program' found and used.\n");
    }

    // GL_NV_vertex_program1_1
    CS_GL_NV_vertex_program1_1 = (strstr (extensions, "GL_NV_vertex_program1_1") != NULL);
    if (CS_GL_NV_vertex_program1_1)
    {
      allclear = true;
      if (CS_GL_NV_vertex_program1_1 = allclear)
        printf ("GL Extension 'GL_NV_vertex_program1_1' found and used.\n");
    }

    // GL_ATI_element_array
    CS_GL_ATI_element_array = (strstr (extensions, "GL_ATI_element_array") != NULL);
    if (CS_GL_ATI_element_array)
    {
      allclear = true;
      if (!(glElementPointerATI = (csGLELEMENTPOINTERATI) gl->GetProcAddress ("glElementPointerATI"))) allclear = false;
      if (!(glDrawElementArrayATI = (csGLDRAWELEMENTARRAYATI) gl->GetProcAddress ("glDrawElementArrayATI"))) allclear = false;
      if (!(glDrawRangeElementArrayATI = (csGLDRAWRANGEELEMENTARRAYATI) gl->GetProcAddress ("glDrawRangeElementArrayATI"))) allclear = false;
      if (CS_GL_ATI_element_array = allclear)
        printf ("GL Extension 'GL_ATI_element_array' found and used.\n");
    }

    // GL_ATI_envmap_bumpmap
    CS_GL_ATI_envmap_bumpmap = (strstr (extensions, "GL_ATI_envmap_bumpmap") != NULL);
    if (CS_GL_ATI_envmap_bumpmap)
    {
      allclear = true;
      if (!(glTexBumpParameterivATI = (csGLTEXBUMPPARAMETERIVATI) gl->GetProcAddress ("glTexBumpParameterivATI"))) allclear = false;
      if (!(glTexBumpParameterfvATI = (csGLTEXBUMPPARAMETERFVATI) gl->GetProcAddress ("glTexBumpParameterfvATI"))) allclear = false;
      if (!(glGetTexBumpParameterivATI = (csGLGETTEXBUMPPARAMETERIVATI) gl->GetProcAddress ("glGetTexBumpParameterivATI"))) allclear = false;
      if (!(glGetTexBumpParameterfvATI = (csGLGETTEXBUMPPARAMETERFVATI) gl->GetProcAddress ("glGetTexBumpParameterfvATI"))) allclear = false;
      if (CS_GL_ATI_envmap_bumpmap = allclear)
        printf ("GL Extension 'GL_ATI_envmap_bumpmap' found and used.\n");
    }

    // GL_ATI_fragment_shader
    CS_GL_ATI_fragment_shader = (strstr (extensions, "GL_ATI_fragment_shader") != NULL);
    if (CS_GL_ATI_fragment_shader)
    {
      allclear = true;
      if (!(glGenFragmentShadersATI = (csGLGENFRAGMENTSHADERSATI) gl->GetProcAddress ("glGenFragmentShadersATI"))) allclear = false;
      if (!(glBindFragmentShaderATI = (csGLBINDFRAGMENTSHADERATI) gl->GetProcAddress ("glBindFragmentShaderATI"))) allclear = false;
      if (!(glDeleteFragmentShaderATI = (csGLDELETEFRAGMENTSHADERATI) gl->GetProcAddress ("glDeleteFragmentShaderATI"))) allclear = false;
      if (!(glBeginFragmentShaderATI = (csGLBEGINFRAGMENTSHADERATI) gl->GetProcAddress ("glBeginFragmentShaderATI"))) allclear = false;
      if (!(glEndFragmentShaderATI = (csGLENDFRAGMENTSHADERATI) gl->GetProcAddress ("glEndFragmentShaderATI"))) allclear = false;
      if (!(glPassTexCoordATI = (csGLPASSTEXCOORDATI) gl->GetProcAddress ("glPassTexCoordATI"))) allclear = false;
      if (!(glSampleMapATI = (csGLSAMPLEMAPATI) gl->GetProcAddress ("glSampleMapATI"))) allclear = false;
      if (!(glColorFragmentOp1ATI = (csGLCOLORFRAGMENTOP1ATI) gl->GetProcAddress ("glColorFragmentOp1ATI"))) allclear = false;
      if (!(glColorFragmentOp2ATI = (csGLCOLORFRAGMENTOP2ATI) gl->GetProcAddress ("glColorFragmentOp2ATI"))) allclear = false;
      if (!(glColorFragmentOp3ATI = (csGLCOLORFRAGMENTOP3ATI) gl->GetProcAddress ("glColorFragmentOp3ATI"))) allclear = false;
      if (!(glAlphaFragmentOp1ATI = (csGLALPHAFRAGMENTOP1ATI) gl->GetProcAddress ("glAlphaFragmentOp1ATI"))) allclear = false;
      if (!(glAlphaFragmentOp2ATI = (csGLALPHAFRAGMENTOP2ATI) gl->GetProcAddress ("glAlphaFragmentOp2ATI"))) allclear = false;
      if (!(glAlphaFragmentOp3ATI = (csGLALPHAFRAGMENTOP3ATI) gl->GetProcAddress ("glAlphaFragmentOp3ATI"))) allclear = false;
      if (!(glSetFragmentShaderConstantATI = (csGLSETFRAGMENTSHADERCONSTANTATI) gl->GetProcAddress ("glSetFragmentShaderConstantATI"))) allclear = false;
      if (CS_GL_ATI_fragment_shader = allclear)
        printf ("GL Extension 'GL_ATI_fragment_shader' found and used.\n");
    }

    // GL_ATI_pn_triangles
    CS_GL_ATI_pn_triangles = (strstr (extensions, "GL_ATI_pn_triangles") != NULL);
    if (CS_GL_ATI_pn_triangles)
    {
      allclear = true;
      if (!(glPNTrianglesiATI = (csGLPNTRIANGLESIATI) gl->GetProcAddress ("glPNTrianglesiATI"))) allclear = false;
      if (!(glPNTrianglesfATI = (csGLPNTRIANGLESFATI) gl->GetProcAddress ("glPNTrianglesfATI"))) allclear = false;
      if (CS_GL_ATI_pn_triangles = allclear)
        printf ("GL Extension 'GL_ATI_pn_triangles' found and used.\n");
    }

    // GL_ATI_texture_mirror_once
    CS_GL_ATI_texture_mirror_once = (strstr (extensions, "GL_ATI_texture_mirror_once") != NULL);
    if (CS_GL_ATI_texture_mirror_once)
    {
      allclear = true;
      if (CS_GL_ATI_texture_mirror_once = allclear)
        printf ("GL Extension 'GL_ATI_texture_mirror_once' found and used.\n");
    }

    // GL_ATI_vertex_array_object
    CS_GL_ATI_vertex_array_object = (strstr (extensions, "GL_ATI_vertex_array_object") != NULL);
    if (CS_GL_ATI_vertex_array_object)
    {
      allclear = true;
      if (!(glNewObjectBufferATI = (csGLNEWOBJECTBUFFERATI) gl->GetProcAddress ("glNewObjectBufferATI"))) allclear = false;
      if (!(glIsObjectBufferATI = (csGLISOBJECTBUFFERATI) gl->GetProcAddress ("glIsObjectBufferATI"))) allclear = false;
      if (!(glUpdateObjectBufferATI = (csGLUPDATEOBJECTBUFFERATI) gl->GetProcAddress ("glUpdateObjectBufferATI"))) allclear = false;
      if (!(glGetObjectBufferfvATI = (csGLGETOBJECTBUFFERFVATI) gl->GetProcAddress ("glGetObjectBufferfvATI"))) allclear = false;
      if (!(glGetObjectBufferivATI = (csGLGETOBJECTBUFFERIVATI) gl->GetProcAddress ("glGetObjectBufferivATI"))) allclear = false;
      if (!(glDeleteObjectBufferATI = (csGLDELETEOBJECTBUFFERATI) gl->GetProcAddress ("glDeleteObjectBufferATI"))) allclear = false;
      if (!(glArrayObjectATI = (csGLARRAYOBJECTATI) gl->GetProcAddress ("glArrayObjectATI"))) allclear = false;
      if (!(glGetArrayObjectfvATI = (csGLGETARRAYOBJECTFVATI) gl->GetProcAddress ("glGetArrayObjectfvATI"))) allclear = false;
      if (!(glGetArrayObjectivATI = (csGLGETARRAYOBJECTIVATI) gl->GetProcAddress ("glGetArrayObjectivATI"))) allclear = false;
      if (!(glVariantArrayObjectATI = (csGLVARIANTARRAYOBJECTATI) gl->GetProcAddress ("glVariantArrayObjectATI"))) allclear = false;
      if (!(glGetVariantArrayObjectfvATI = (csGLGETVARIANTARRAYOBJECTFVATI) gl->GetProcAddress ("glGetVariantArrayObjectfvATI"))) allclear = false;
      if (!(glGetVariantArrayObjectivATI = (csGLGETVARIANTARRAYOBJECTIVATI) gl->GetProcAddress ("glGetVariantArrayObjectivATI"))) allclear = false;
      if (CS_GL_ATI_vertex_array_object = allclear)
        printf ("GL Extension 'GL_ATI_vertex_array_object' found and used.\n");
    }

    // GL_ATI_vertex_streams
    CS_GL_ATI_vertex_streams = (strstr (extensions, "GL_ATI_vertex_streams") != NULL);
    if (CS_GL_ATI_vertex_streams)
    {
      allclear = true;
      if (!(glVertexStream1s = (csGLVERTEXSTREAM1S) gl->GetProcAddress ("glVertexStream1s"))) allclear = false;
      if (!(glVertexStream1i = (csGLVERTEXSTREAM1I) gl->GetProcAddress ("glVertexStream1i"))) allclear = false;
      if (!(glVertexStream1f = (csGLVERTEXSTREAM1F) gl->GetProcAddress ("glVertexStream1f"))) allclear = false;
      if (!(glVertexStream1d = (csGLVERTEXSTREAM1D) gl->GetProcAddress ("glVertexStream1d"))) allclear = false;
      if (!(glVertexStream1sv = (csGLVERTEXSTREAM1SV) gl->GetProcAddress ("glVertexStream1sv"))) allclear = false;
      if (!(glVertexStream1iv = (csGLVERTEXSTREAM1IV) gl->GetProcAddress ("glVertexStream1iv"))) allclear = false;
      if (!(glVertexStream1fv = (csGLVERTEXSTREAM1FV) gl->GetProcAddress ("glVertexStream1fv"))) allclear = false;
      if (!(glVertexStream1dv = (csGLVERTEXSTREAM1DV) gl->GetProcAddress ("glVertexStream1dv"))) allclear = false;
      if (!(glVertexStream2s = (csGLVERTEXSTREAM2S) gl->GetProcAddress ("glVertexStream2s"))) allclear = false;
      if (!(glVertexStream2i = (csGLVERTEXSTREAM2I) gl->GetProcAddress ("glVertexStream2i"))) allclear = false;
      if (!(glVertexStream2f = (csGLVERTEXSTREAM2F) gl->GetProcAddress ("glVertexStream2f"))) allclear = false;
      if (!(glVertexStream2d = (csGLVERTEXSTREAM2D) gl->GetProcAddress ("glVertexStream2d"))) allclear = false;
      if (!(glVertexStream2sv = (csGLVERTEXSTREAM2SV) gl->GetProcAddress ("glVertexStream2sv"))) allclear = false;
      if (!(glVertexStream2iv = (csGLVERTEXSTREAM2IV) gl->GetProcAddress ("glVertexStream2iv"))) allclear = false;
      if (!(glVertexStream2fv = (csGLVERTEXSTREAM2FV) gl->GetProcAddress ("glVertexStream2fv"))) allclear = false;
      if (!(glVertexStream2dv = (csGLVERTEXSTREAM2DV) gl->GetProcAddress ("glVertexStream2dv"))) allclear = false;
      if (!(glVertexStream3s = (csGLVERTEXSTREAM3S) gl->GetProcAddress ("glVertexStream3s"))) allclear = false;
      if (!(glVertexStream3i = (csGLVERTEXSTREAM3I) gl->GetProcAddress ("glVertexStream3i"))) allclear = false;
      if (!(glVertexStream3f = (csGLVERTEXSTREAM3F) gl->GetProcAddress ("glVertexStream3f"))) allclear = false;
      if (!(glVertexStream3d = (csGLVERTEXSTREAM3D) gl->GetProcAddress ("glVertexStream3d"))) allclear = false;
      if (!(glVertexStream3sv = (csGLVERTEXSTREAM3SV) gl->GetProcAddress ("glVertexStream3sv"))) allclear = false;
      if (!(glVertexStream3iv = (csGLVERTEXSTREAM3IV) gl->GetProcAddress ("glVertexStream3iv"))) allclear = false;
      if (!(glVertexStream3fv = (csGLVERTEXSTREAM3FV) gl->GetProcAddress ("glVertexStream3fv"))) allclear = false;
      if (!(glVertexStream3dv = (csGLVERTEXSTREAM3DV) gl->GetProcAddress ("glVertexStream3dv"))) allclear = false;
      if (!(glVertexStream4s = (csGLVERTEXSTREAM4S) gl->GetProcAddress ("glVertexStream4s"))) allclear = false;
      if (!(glVertexStream4i = (csGLVERTEXSTREAM4I) gl->GetProcAddress ("glVertexStream4i"))) allclear = false;
      if (!(glVertexStream4f = (csGLVERTEXSTREAM4F) gl->GetProcAddress ("glVertexStream4f"))) allclear = false;
      if (!(glVertexStream4d = (csGLVERTEXSTREAM4D) gl->GetProcAddress ("glVertexStream4d"))) allclear = false;
      if (!(glVertexStream4sv = (csGLVERTEXSTREAM4SV) gl->GetProcAddress ("glVertexStream4sv"))) allclear = false;
      if (!(glVertexStream4iv = (csGLVERTEXSTREAM4IV) gl->GetProcAddress ("glVertexStream4iv"))) allclear = false;
      if (!(glVertexStream4fv = (csGLVERTEXSTREAM4FV) gl->GetProcAddress ("glVertexStream4fv"))) allclear = false;
      if (!(glVertexStream4dv = (csGLVERTEXSTREAM4DV) gl->GetProcAddress ("glVertexStream4dv"))) allclear = false;
      if (!(glNormalStream3b = (csGLNORMALSTREAM3B) gl->GetProcAddress ("glNormalStream3b"))) allclear = false;
      if (!(glNormalStream3s = (csGLNORMALSTREAM3S) gl->GetProcAddress ("glNormalStream3s"))) allclear = false;
      if (!(glNormalStream3i = (csGLNORMALSTREAM3I) gl->GetProcAddress ("glNormalStream3i"))) allclear = false;
      if (!(glNormalStream3f = (csGLNORMALSTREAM3F) gl->GetProcAddress ("glNormalStream3f"))) allclear = false;
      if (!(glNormalStream3d = (csGLNORMALSTREAM3D) gl->GetProcAddress ("glNormalStream3d"))) allclear = false;
      if (!(glNormalStream3bv = (csGLNORMALSTREAM3BV) gl->GetProcAddress ("glNormalStream3bv"))) allclear = false;
      if (!(glNormalStream3sv = (csGLNORMALSTREAM3SV) gl->GetProcAddress ("glNormalStream3sv"))) allclear = false;
      if (!(glNormalStream3iv = (csGLNORMALSTREAM3IV) gl->GetProcAddress ("glNormalStream3iv"))) allclear = false;
      if (!(glNormalStream3fv = (csGLNORMALSTREAM3FV) gl->GetProcAddress ("glNormalStream3fv"))) allclear = false;
      if (!(glNormalStream3dv = (csGLNORMALSTREAM3DV) gl->GetProcAddress ("glNormalStream3dv"))) allclear = false;
      if (!(glClientActiveVertexStream = (csGLCLIENTACTIVEVERTEXSTREAM) gl->GetProcAddress ("glClientActiveVertexStream"))) allclear = false;
      if (!(glVertexBlendEnvi = (csGLVERTEXBLENDENVI) gl->GetProcAddress ("glVertexBlendEnvi"))) allclear = false;
      if (!(glVertexBlendEnvf = (csGLVERTEXBLENDENVF) gl->GetProcAddress ("glVertexBlendEnvf"))) allclear = false;
      if (CS_GL_ATI_vertex_streams = allclear)
        printf ("GL Extension 'GL_ATI_vertex_streams' found and used.\n");
    }

    // WGL_I3D_image_buffer
    CS_WGL_I3D_image_buffer = (strstr (extensions, "WGL_I3D_image_buffer") != NULL);
    if (CS_WGL_I3D_image_buffer)
    {
      allclear = true;
      if (!(wglCreateImageBufferI3D = (csWGLCREATEIMAGEBUFFERI3D) gl->GetProcAddress ("wglCreateImageBufferI3D"))) allclear = false;
      if (!(wglDestroyImageBufferI3D = (csWGLDESTROYIMAGEBUFFERI3D) gl->GetProcAddress ("wglDestroyImageBufferI3D"))) allclear = false;
      if (!(wglAssociateImageBufferEventsI3D = (csWGLASSOCIATEIMAGEBUFFEREVENTSI3D) gl->GetProcAddress ("wglAssociateImageBufferEventsI3D"))) allclear = false;
      if (!(wglReleaseImageBufferEventsI3D = (csWGLRELEASEIMAGEBUFFEREVENTSI3D) gl->GetProcAddress ("wglReleaseImageBufferEventsI3D"))) allclear = false;
      if (CS_WGL_I3D_image_buffer = allclear)
        printf ("GL Extension 'WGL_I3D_image_buffer' found and used.\n");
    }

    // WGL_I3D_swap_frame_lock
    CS_WGL_I3D_swap_frame_lock = (strstr (extensions, "WGL_I3D_swap_frame_lock") != NULL);
    if (CS_WGL_I3D_swap_frame_lock)
    {
      allclear = true;
      if (!(wglEnableFrameLockI3D = (csWGLENABLEFRAMELOCKI3D) gl->GetProcAddress ("wglEnableFrameLockI3D"))) allclear = false;
      if (!(wglDisableFrameLockI3D = (csWGLDISABLEFRAMELOCKI3D) gl->GetProcAddress ("wglDisableFrameLockI3D"))) allclear = false;
      if (!(wglIsEnabledFrameLockI3D = (csWGLISENABLEDFRAMELOCKI3D) gl->GetProcAddress ("wglIsEnabledFrameLockI3D"))) allclear = false;
      if (!(wglQueryFrameLockMasterI3D = (csWGLQUERYFRAMELOCKMASTERI3D) gl->GetProcAddress ("wglQueryFrameLockMasterI3D"))) allclear = false;
      if (CS_WGL_I3D_swap_frame_lock = allclear)
        printf ("GL Extension 'WGL_I3D_swap_frame_lock' found and used.\n");
    }

    // WGL_I3D_swap_frame_usage
    CS_WGL_I3D_swap_frame_usage = (strstr (extensions, "WGL_I3D_swap_frame_usage") != NULL);
    if (CS_WGL_I3D_swap_frame_usage)
    {
      allclear = true;
      if (!(wglGetFrameUsageI3D = (csWGLGETFRAMEUSAGEI3D) gl->GetProcAddress ("wglGetFrameUsageI3D"))) allclear = false;
      if (!(wglBeginFrameTrackingI3D = (csWGLBEGINFRAMETRACKINGI3D) gl->GetProcAddress ("wglBeginFrameTrackingI3D"))) allclear = false;
      if (!(wglEndFrameTrackingI3D = (csWGLENDFRAMETRACKINGI3D) gl->GetProcAddress ("wglEndFrameTrackingI3D"))) allclear = false;
      if (!(wglQueryFrameTrackingI3D = (csWGLQUERYFRAMETRACKINGI3D) gl->GetProcAddress ("wglQueryFrameTrackingI3D"))) allclear = false;
      if (CS_WGL_I3D_swap_frame_usage = allclear)
        printf ("GL Extension 'WGL_I3D_swap_frame_usage' found and used.\n");
    }

    // GL_3DFX_texture_compression_FXT1
    CS_GL_3DFX_texture_compression_FXT1 = (strstr (extensions, "GL_3DFX_texture_compression_FXT1") != NULL);
    if (CS_GL_3DFX_texture_compression_FXT1)
    {
      allclear = true;
      if (CS_GL_3DFX_texture_compression_FXT1 = allclear)
        printf ("GL Extension 'GL_3DFX_texture_compression_FXT1' found and used.\n");
    }

    // GL_IBM_cull_vertex
    CS_GL_IBM_cull_vertex = (strstr (extensions, "GL_IBM_cull_vertex") != NULL);
    if (CS_GL_IBM_cull_vertex)
    {
      allclear = true;
      if (CS_GL_IBM_cull_vertex = allclear)
        printf ("GL Extension 'GL_IBM_cull_vertex' found and used.\n");
    }

    // GL_IBM_multimode_draw_arrays
    CS_GL_IBM_multimode_draw_arrays = (strstr (extensions, "GL_IBM_multimode_draw_arrays") != NULL);
    if (CS_GL_IBM_multimode_draw_arrays)
    {
      allclear = true;
      if (!(glMultiModeDrawArraysIBM = (csGLMULTIMODEDRAWARRAYSIBM) gl->GetProcAddress ("glMultiModeDrawArraysIBM"))) allclear = false;
      if (!(glMultiModeDrawElementsIBM = (csGLMULTIMODEDRAWELEMENTSIBM) gl->GetProcAddress ("glMultiModeDrawElementsIBM"))) allclear = false;
      if (CS_GL_IBM_multimode_draw_arrays = allclear)
        printf ("GL Extension 'GL_IBM_multimode_draw_arrays' found and used.\n");
    }

    // GL_IBM_raster_pos_clip
    CS_GL_IBM_raster_pos_clip = (strstr (extensions, "GL_IBM_raster_pos_clip") != NULL);
    if (CS_GL_IBM_raster_pos_clip)
    {
      allclear = true;
      if (CS_GL_IBM_raster_pos_clip = allclear)
        printf ("GL Extension 'GL_IBM_raster_pos_clip' found and used.\n");
    }

    // GL_IBM_texture_mirrored_repeat
    CS_GL_IBM_texture_mirrored_repeat = (strstr (extensions, "GL_IBM_texture_mirrored_repeat") != NULL);
    if (CS_GL_IBM_texture_mirrored_repeat)
    {
      allclear = true;
      if (CS_GL_IBM_texture_mirrored_repeat = allclear)
        printf ("GL Extension 'GL_IBM_texture_mirrored_repeat' found and used.\n");
    }

    // GL_IBM_vertex_array_lists
    CS_GL_IBM_vertex_array_lists = (strstr (extensions, "GL_IBM_vertex_array_lists") != NULL);
    if (CS_GL_IBM_vertex_array_lists)
    {
      allclear = true;
      if (!(glColorPointerListIBM = (csGLCOLORPOINTERLISTIBM) gl->GetProcAddress ("glColorPointerListIBM"))) allclear = false;
      if (!(glSecondaryColorPointerListIBM = (csGLSECONDARYCOLORPOINTERLISTIBM) gl->GetProcAddress ("glSecondaryColorPointerListIBM"))) allclear = false;
      if (!(glEdgeFlagPointerListIBM = (csGLEDGEFLAGPOINTERLISTIBM) gl->GetProcAddress ("glEdgeFlagPointerListIBM"))) allclear = false;
      if (!(glFogCoordPointerListIBM = (csGLFOGCOORDPOINTERLISTIBM) gl->GetProcAddress ("glFogCoordPointerListIBM"))) allclear = false;
      if (!(glNormalPointerListIBM = (csGLNORMALPOINTERLISTIBM) gl->GetProcAddress ("glNormalPointerListIBM"))) allclear = false;
      if (!(glTexCoordPointerListIBM = (csGLTEXCOORDPOINTERLISTIBM) gl->GetProcAddress ("glTexCoordPointerListIBM"))) allclear = false;
      if (!(glVertexPointerListIBM = (csGLVERTEXPOINTERLISTIBM) gl->GetProcAddress ("glVertexPointerListIBM"))) allclear = false;
      if (CS_GL_IBM_vertex_array_lists = allclear)
        printf ("GL Extension 'GL_IBM_vertex_array_lists' found and used.\n");
    }

    // GL_MESA_resize_buffers
    CS_GL_MESA_resize_buffers = (strstr (extensions, "GL_MESA_resize_buffers") != NULL);
    if (CS_GL_MESA_resize_buffers)
    {
      allclear = true;
      if (!(glResizeBuffersMESA = (csGLRESIZEBUFFERSMESA) gl->GetProcAddress ("glResizeBuffersMESA"))) allclear = false;
      if (CS_GL_MESA_resize_buffers = allclear)
        printf ("GL Extension 'GL_MESA_resize_buffers' found and used.\n");
    }

    // GL_MESA_window_pos
    CS_GL_MESA_window_pos = (strstr (extensions, "GL_MESA_window_pos") != NULL);
    if (CS_GL_MESA_window_pos)
    {
      allclear = true;
      if (!(glWindowPos2dMESA = (csGLWINDOWPOS2DMESA) gl->GetProcAddress ("glWindowPos2dMESA"))) allclear = false;
      if (!(glWindowPos2fMESA = (csGLWINDOWPOS2FMESA) gl->GetProcAddress ("glWindowPos2fMESA"))) allclear = false;
      if (!(glWindowPos2iMESA = (csGLWINDOWPOS2IMESA) gl->GetProcAddress ("glWindowPos2iMESA"))) allclear = false;
      if (!(glWindowPos2sMESA = (csGLWINDOWPOS2SMESA) gl->GetProcAddress ("glWindowPos2sMESA"))) allclear = false;
      if (!(glWindowPos2ivMESA = (csGLWINDOWPOS2IVMESA) gl->GetProcAddress ("glWindowPos2ivMESA"))) allclear = false;
      if (!(glWindowPos2svMESA = (csGLWINDOWPOS2SVMESA) gl->GetProcAddress ("glWindowPos2svMESA"))) allclear = false;
      if (!(glWindowPos2fvMESA = (csGLWINDOWPOS2FVMESA) gl->GetProcAddress ("glWindowPos2fvMESA"))) allclear = false;
      if (!(glWindowPos2dvMESA = (csGLWINDOWPOS2DVMESA) gl->GetProcAddress ("glWindowPos2dvMESA"))) allclear = false;
      if (!(glWindowPos3iMESA = (csGLWINDOWPOS3IMESA) gl->GetProcAddress ("glWindowPos3iMESA"))) allclear = false;
      if (!(glWindowPos3sMESA = (csGLWINDOWPOS3SMESA) gl->GetProcAddress ("glWindowPos3sMESA"))) allclear = false;
      if (!(glWindowPos3fMESA = (csGLWINDOWPOS3FMESA) gl->GetProcAddress ("glWindowPos3fMESA"))) allclear = false;
      if (!(glWindowPos3dMESA = (csGLWINDOWPOS3DMESA) gl->GetProcAddress ("glWindowPos3dMESA"))) allclear = false;
      if (!(glWindowPos3ivMESA = (csGLWINDOWPOS3IVMESA) gl->GetProcAddress ("glWindowPos3ivMESA"))) allclear = false;
      if (!(glWindowPos3svMESA = (csGLWINDOWPOS3SVMESA) gl->GetProcAddress ("glWindowPos3svMESA"))) allclear = false;
      if (!(glWindowPos3fvMESA = (csGLWINDOWPOS3FVMESA) gl->GetProcAddress ("glWindowPos3fvMESA"))) allclear = false;
      if (!(glWindowPos3dvMESA = (csGLWINDOWPOS3DVMESA) gl->GetProcAddress ("glWindowPos3dvMESA"))) allclear = false;
      if (!(glWindowPos4iMESA = (csGLWINDOWPOS4IMESA) gl->GetProcAddress ("glWindowPos4iMESA"))) allclear = false;
      if (!(glWindowPos4sMESA = (csGLWINDOWPOS4SMESA) gl->GetProcAddress ("glWindowPos4sMESA"))) allclear = false;
      if (!(glWindowPos4fMESA = (csGLWINDOWPOS4FMESA) gl->GetProcAddress ("glWindowPos4fMESA"))) allclear = false;
      if (!(glWindowPos4dMESA = (csGLWINDOWPOS4DMESA) gl->GetProcAddress ("glWindowPos4dMESA"))) allclear = false;
      if (!(glWindowPos4ivMESA = (csGLWINDOWPOS4IVMESA) gl->GetProcAddress ("glWindowPos4ivMESA"))) allclear = false;
      if (!(glWindowPos4svMESA = (csGLWINDOWPOS4SVMESA) gl->GetProcAddress ("glWindowPos4svMESA"))) allclear = false;
      if (!(glWindowPos4fvMESA = (csGLWINDOWPOS4FVMESA) gl->GetProcAddress ("glWindowPos4fvMESA"))) allclear = false;
      if (!(glWindowPos4dvMESA = (csGLWINDOWPOS4DVMESA) gl->GetProcAddress ("glWindowPos4dvMESA"))) allclear = false;
      if (CS_GL_MESA_window_pos = allclear)
        printf ("GL Extension 'GL_MESA_window_pos' found and used.\n");
    }

    // GL_OML_interlace
    CS_GL_OML_interlace = (strstr (extensions, "GL_OML_interlace") != NULL);
    if (CS_GL_OML_interlace)
    {
      allclear = true;
      if (CS_GL_OML_interlace = allclear)
        printf ("GL Extension 'GL_OML_interlace' found and used.\n");
    }

    // GL_OML_resample
    CS_GL_OML_resample = (strstr (extensions, "GL_OML_resample") != NULL);
    if (CS_GL_OML_resample)
    {
      allclear = true;
      if (CS_GL_OML_resample = allclear)
        printf ("GL Extension 'GL_OML_resample' found and used.\n");
    }

    // GL_OML_subsample
    CS_GL_OML_subsample = (strstr (extensions, "GL_OML_subsample") != NULL);
    if (CS_GL_OML_subsample)
    {
      allclear = true;
      if (CS_GL_OML_subsample = allclear)
        printf ("GL Extension 'GL_OML_subsample' found and used.\n");
    }

    // GL_SGIS_generate_mipmap
    CS_GL_SGIS_generate_mipmap = (strstr (extensions, "GL_SGIS_generate_mipmap") != NULL);
    if (CS_GL_SGIS_generate_mipmap)
    {
      allclear = true;
      if (CS_GL_SGIS_generate_mipmap = allclear)
        printf ("GL Extension 'GL_SGIS_generate_mipmap' found and used.\n");
    }

    // GL_SGIS_multisample
    CS_GL_SGIS_multisample = (strstr (extensions, "GL_SGIS_multisample") != NULL);
    if (CS_GL_SGIS_multisample)
    {
      allclear = true;
      if (!(glSampleMaskSGIS = (csGLSAMPLEMASKSGIS) gl->GetProcAddress ("glSampleMaskSGIS"))) allclear = false;
      if (!(glSamplePatternSGIS = (csGLSAMPLEPATTERNSGIS) gl->GetProcAddress ("glSamplePatternSGIS"))) allclear = false;
      if (CS_GL_SGIS_multisample = allclear)
        printf ("GL Extension 'GL_SGIS_multisample' found and used.\n");
    }

    // GL_SGIS_pixel_texture
    CS_GL_SGIS_pixel_texture = (strstr (extensions, "GL_SGIS_pixel_texture") != NULL);
    if (CS_GL_SGIS_pixel_texture)
    {
      allclear = true;
      if (!(glPixelTexGenParameteriSGIS = (csGLPIXELTEXGENPARAMETERISGIS) gl->GetProcAddress ("glPixelTexGenParameteriSGIS"))) allclear = false;
      if (!(glPixelTexGenParameterfSGIS = (csGLPIXELTEXGENPARAMETERFSGIS) gl->GetProcAddress ("glPixelTexGenParameterfSGIS"))) allclear = false;
      if (!(glGetPixelTexGenParameterivSGIS = (csGLGETPIXELTEXGENPARAMETERIVSGIS) gl->GetProcAddress ("glGetPixelTexGenParameterivSGIS"))) allclear = false;
      if (!(glGetPixelTexGenParameterfvSGIS = (csGLGETPIXELTEXGENPARAMETERFVSGIS) gl->GetProcAddress ("glGetPixelTexGenParameterfvSGIS"))) allclear = false;
      if (CS_GL_SGIS_pixel_texture = allclear)
        printf ("GL Extension 'GL_SGIS_pixel_texture' found and used.\n");
    }

    // GL_SGIS_texture_border_clamp
    CS_GL_SGIS_texture_border_clamp = (strstr (extensions, "GL_SGIS_texture_border_clamp") != NULL);
    if (CS_GL_SGIS_texture_border_clamp)
    {
      allclear = true;
      if (CS_GL_SGIS_texture_border_clamp = allclear)
        printf ("GL Extension 'GL_SGIS_texture_border_clamp' found and used.\n");
    }

    // GL_SGIS_texture_color_mask
    CS_GL_SGIS_texture_color_mask = (strstr (extensions, "GL_SGIS_texture_color_mask") != NULL);
    if (CS_GL_SGIS_texture_color_mask)
    {
      allclear = true;
      if (!(glTextureColorMaskSGIS = (csGLTEXTURECOLORMASKSGIS) gl->GetProcAddress ("glTextureColorMaskSGIS"))) allclear = false;
      if (CS_GL_SGIS_texture_color_mask = allclear)
        printf ("GL Extension 'GL_SGIS_texture_color_mask' found and used.\n");
    }

    // GL_SGIS_texture_edge_clamp
    CS_GL_SGIS_texture_edge_clamp = (strstr (extensions, "GL_SGIS_texture_edge_clamp") != NULL);
    if (CS_GL_SGIS_texture_edge_clamp)
    {
      allclear = true;
      if (CS_GL_SGIS_texture_edge_clamp = allclear)
        printf ("GL Extension 'GL_SGIS_texture_edge_clamp' found and used.\n");
    }

    // GL_SGIS_texture_lod
    CS_GL_SGIS_texture_lod = (strstr (extensions, "GL_SGIS_texture_lod") != NULL);
    if (CS_GL_SGIS_texture_lod)
    {
      allclear = true;
      if (CS_GL_SGIS_texture_lod = allclear)
        printf ("GL Extension 'GL_SGIS_texture_lod' found and used.\n");
    }

    // GL_SGIS_depth_texture
    CS_GL_SGIS_depth_texture = (strstr (extensions, "GL_SGIS_depth_texture") != NULL);
    if (CS_GL_SGIS_depth_texture)
    {
      allclear = true;
      if (CS_GL_SGIS_depth_texture = allclear)
        printf ("GL Extension 'GL_SGIS_depth_texture' found and used.\n");
    }

    // GL_SGIX_fog_offset
    CS_GL_SGIX_fog_offset = (strstr (extensions, "GL_SGIX_fog_offset") != NULL);
    if (CS_GL_SGIX_fog_offset)
    {
      allclear = true;
      if (CS_GL_SGIX_fog_offset = allclear)
        printf ("GL Extension 'GL_SGIX_fog_offset' found and used.\n");
    }

    // GL_SGIX_interlace
    CS_GL_SGIX_interlace = (strstr (extensions, "GL_SGIX_interlace") != NULL);
    if (CS_GL_SGIX_interlace)
    {
      allclear = true;
      if (CS_GL_SGIX_interlace = allclear)
        printf ("GL Extension 'GL_SGIX_interlace' found and used.\n");
    }

    // GL_SGIX_shadow_ambient
    CS_GL_SGIX_shadow_ambient = (strstr (extensions, "GL_SGIX_shadow_ambient") != NULL);
    if (CS_GL_SGIX_shadow_ambient)
    {
      allclear = true;
      if (CS_GL_SGIX_shadow_ambient = allclear)
        printf ("GL Extension 'GL_SGIX_shadow_ambient' found and used.\n");
    }

    // GL_SGI_color_matrix
    CS_GL_SGI_color_matrix = (strstr (extensions, "GL_SGI_color_matrix") != NULL);
    if (CS_GL_SGI_color_matrix)
    {
      allclear = true;
      if (CS_GL_SGI_color_matrix = allclear)
        printf ("GL Extension 'GL_SGI_color_matrix' found and used.\n");
    }

    // GL_SGI_color_table
    CS_GL_SGI_color_table = (strstr (extensions, "GL_SGI_color_table") != NULL);
    if (CS_GL_SGI_color_table)
    {
      allclear = true;
      if (!(glColorTableSGI = (csGLCOLORTABLESGI) gl->GetProcAddress ("glColorTableSGI"))) allclear = false;
      if (!(glCopyColorTableSGI = (csGLCOPYCOLORTABLESGI) gl->GetProcAddress ("glCopyColorTableSGI"))) allclear = false;
      if (!(glColorTableParameterivSGI = (csGLCOLORTABLEPARAMETERIVSGI) gl->GetProcAddress ("glColorTableParameterivSGI"))) allclear = false;
      if (!(glColorTableParameterfvSGI = (csGLCOLORTABLEPARAMETERFVSGI) gl->GetProcAddress ("glColorTableParameterfvSGI"))) allclear = false;
      if (!(glGetColorTableSGI = (csGLGETCOLORTABLESGI) gl->GetProcAddress ("glGetColorTableSGI"))) allclear = false;
      if (!(glGetColorTableParameterivSGI = (csGLGETCOLORTABLEPARAMETERIVSGI) gl->GetProcAddress ("glGetColorTableParameterivSGI"))) allclear = false;
      if (!(glGetColorTableParameterfvSGI = (csGLGETCOLORTABLEPARAMETERFVSGI) gl->GetProcAddress ("glGetColorTableParameterfvSGI"))) allclear = false;
      if (CS_GL_SGI_color_table = allclear)
        printf ("GL Extension 'GL_SGI_color_table' found and used.\n");
    }

    // GL_SGI_texture_color_table
    CS_GL_SGI_texture_color_table = (strstr (extensions, "GL_SGI_texture_color_table") != NULL);
    if (CS_GL_SGI_texture_color_table)
    {
      allclear = true;
      if (CS_GL_SGI_texture_color_table = allclear)
        printf ("GL Extension 'GL_SGI_texture_color_table' found and used.\n");
    }

    // GL_SUN_vertex
    CS_GL_SUN_vertex = (strstr (extensions, "GL_SUN_vertex") != NULL);
    if (CS_GL_SUN_vertex)
    {
      allclear = true;
      if (!(glColor4ubVertex2fSUN = (csGLCOLOR4UBVERTEX2FSUN) gl->GetProcAddress ("glColor4ubVertex2fSUN"))) allclear = false;
      if (!(glColor4ubVertex2fvSUN = (csGLCOLOR4UBVERTEX2FVSUN) gl->GetProcAddress ("glColor4ubVertex2fvSUN"))) allclear = false;
      if (!(glColor4ubVertex3fSUN = (csGLCOLOR4UBVERTEX3FSUN) gl->GetProcAddress ("glColor4ubVertex3fSUN"))) allclear = false;
      if (!(glColor4ubVertex3fvSUN = (csGLCOLOR4UBVERTEX3FVSUN) gl->GetProcAddress ("glColor4ubVertex3fvSUN"))) allclear = false;
      if (!(glColor3fVertex3fSUN = (csGLCOLOR3FVERTEX3FSUN) gl->GetProcAddress ("glColor3fVertex3fSUN"))) allclear = false;
      if (!(glColor3fVertex3fvSUN = (csGLCOLOR3FVERTEX3FVSUN) gl->GetProcAddress ("glColor3fVertex3fvSUN"))) allclear = false;
      if (!(glNormal3fVertex3fSUN = (csGLNORMAL3FVERTEX3FSUN) gl->GetProcAddress ("glNormal3fVertex3fSUN"))) allclear = false;
      if (!(glNormal3fVertex3fvSUN = (csGLNORMAL3FVERTEX3FVSUN) gl->GetProcAddress ("glNormal3fVertex3fvSUN"))) allclear = false;
      if (!(glColor4fNormal3fVertex3fSUN = (csGLCOLOR4FNORMAL3FVERTEX3FSUN) gl->GetProcAddress ("glColor4fNormal3fVertex3fSUN"))) allclear = false;
      if (!(glColor4fNormal3fVertex3fvSUN = (csGLCOLOR4FNORMAL3FVERTEX3FVSUN) gl->GetProcAddress ("glColor4fNormal3fVertex3fvSUN"))) allclear = false;
      if (!(glTexCoord2fVertex3fSUN = (csGLTEXCOORD2FVERTEX3FSUN) gl->GetProcAddress ("glTexCoord2fVertex3fSUN"))) allclear = false;
      if (!(glTexCoord2fVertex3fvSUN = (csGLTEXCOORD2FVERTEX3FVSUN) gl->GetProcAddress ("glTexCoord2fVertex3fvSUN"))) allclear = false;
      if (!(glTexCoord4fVertex4fSUN = (csGLTEXCOORD4FVERTEX4FSUN) gl->GetProcAddress ("glTexCoord4fVertex4fSUN"))) allclear = false;
      if (!(glTexCoord4fVertex4fvSUN = (csGLTEXCOORD4FVERTEX4FVSUN) gl->GetProcAddress ("glTexCoord4fVertex4fvSUN"))) allclear = false;
      if (!(glTexCoord2fColor4ubVertex3fSUN = (csGLTEXCOORD2FCOLOR4UBVERTEX3FSUN) gl->GetProcAddress ("glTexCoord2fColor4ubVertex3fSUN"))) allclear = false;
      if (!(glTexCoord2fColor4ubVertex3fvSUN = (csGLTEXCOORD2FCOLOR4UBVERTEX3FVSUN) gl->GetProcAddress ("glTexCoord2fColor4ubVertex3fvSUN"))) allclear = false;
      if (!(glTexCoord2fColor3fVertex3fSUN = (csGLTEXCOORD2FCOLOR3FVERTEX3FSUN) gl->GetProcAddress ("glTexCoord2fColor3fVertex3fSUN"))) allclear = false;
      if (!(glTexCoord2fColor3fVertex3fvSUN = (csGLTEXCOORD2FCOLOR3FVERTEX3FVSUN) gl->GetProcAddress ("glTexCoord2fColor3fVertex3fvSUN"))) allclear = false;
      if (!(glTexCoord2fNormal3fVertex3fSUN = (csGLTEXCOORD2FNORMAL3FVERTEX3FSUN) gl->GetProcAddress ("glTexCoord2fNormal3fVertex3fSUN"))) allclear = false;
      if (!(glTexCoord2fNormal3fVertex3fvSUN = (csGLTEXCOORD2FNORMAL3FVERTEX3FVSUN) gl->GetProcAddress ("glTexCoord2fNormal3fVertex3fvSUN"))) allclear = false;
      if (!(glTexCoord2fColor4fNormal3fVertex3fSUN = (csGLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUN) gl->GetProcAddress ("glTexCoord2fColor4fNormal3fVertex3fSUN"))) allclear = false;
      if (!(glTexCoord2fColor4fNormal3fVertex3fvSUN = (csGLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUN) gl->GetProcAddress ("glTexCoord2fColor4fNormal3fVertex3fvSUN"))) allclear = false;
      if (!(glTexCoord4fColor4fNormal3fVertex4fSUN = (csGLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FSUN) gl->GetProcAddress ("glTexCoord4fColor4fNormal3fVertex4fSUN"))) allclear = false;
      if (!(glTexCoord4fColor4fNormal3fVertex4fvSUN = (csGLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FVSUN) gl->GetProcAddress ("glTexCoord4fColor4fNormal3fVertex4fvSUN"))) allclear = false;
      if (!(glReplacementCodeuiVertex3fSUN = (csGLREPLACEMENTCODEUIVERTEX3FSUN) gl->GetProcAddress ("glReplacementCodeuiVertex3fSUN"))) allclear = false;
      if (!(glReplacementCodeuiVertex3fvSUN = (csGLREPLACEMENTCODEUIVERTEX3FVSUN) gl->GetProcAddress ("glReplacementCodeuiVertex3fvSUN"))) allclear = false;
      if (!(glReplacementCodeuiColor4ubVertex3fSUN = (csGLREPLACEMENTCODEUICOLOR4UBVERTEX3FSUN) gl->GetProcAddress ("glReplacementCodeuiColor4ubVertex3fSUN"))) allclear = false;
      if (!(glReplacementCodeuiColor4ubVertex3fvSUN = (csGLREPLACEMENTCODEUICOLOR4UBVERTEX3FVSUN) gl->GetProcAddress ("glReplacementCodeuiColor4ubVertex3fvSUN"))) allclear = false;
      if (!(glReplacementCodeuiColor3fVertex3fSUN = (csGLREPLACEMENTCODEUICOLOR3FVERTEX3FSUN) gl->GetProcAddress ("glReplacementCodeuiColor3fVertex3fSUN"))) allclear = false;
      if (!(glReplacementCodeuiColor3fVertex3fvSUN = (csGLREPLACEMENTCODEUICOLOR3FVERTEX3FVSUN) gl->GetProcAddress ("glReplacementCodeuiColor3fVertex3fvSUN"))) allclear = false;
      if (!(glReplacementCodeuiNormal3fVertex3fSUN = (csGLREPLACEMENTCODEUINORMAL3FVERTEX3FSUN) gl->GetProcAddress ("glReplacementCodeuiNormal3fVertex3fSUN"))) allclear = false;
      if (!(glReplacementCodeuiNormal3fVertex3fvSUN = (csGLREPLACEMENTCODEUINORMAL3FVERTEX3FVSUN) gl->GetProcAddress ("glReplacementCodeuiNormal3fVertex3fvSUN"))) allclear = false;
      if (!(glReplacementCodeuiColor4fNormal3fVertex3fSUN = (csGLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FSUN) gl->GetProcAddress ("glReplacementCodeuiColor4fNormal3fVertex3fSUN"))) allclear = false;
      if (!(glReplacementCodeuiColor4fNormal3fVertex3fvSUN = (csGLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FVSUN) gl->GetProcAddress ("glReplacementCodeuiColor4fNormal3fVertex3fvSUN"))) allclear = false;
      if (!(glReplacementCodeuiTexCoord2fVertex3fSUN = (csGLREPLACEMENTCODEUITEXCOORD2FVERTEX3FSUN) gl->GetProcAddress ("glReplacementCodeuiTexCoord2fVertex3fSUN"))) allclear = false;
      if (!(glReplacementCodeuiTexCoord2fVertex3fvSUN = (csGLREPLACEMENTCODEUITEXCOORD2FVERTEX3FVSUN) gl->GetProcAddress ("glReplacementCodeuiTexCoord2fVertex3fvSUN"))) allclear = false;
      if (!(glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN = (csGLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FSUN) gl->GetProcAddress ("glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN"))) allclear = false;
      if (!(glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN = (csGLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FVSUN) gl->GetProcAddress ("glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN"))) allclear = false;
      if (!(glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN = (csGLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUN) gl->GetProcAddress ("glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN"))) allclear = false;
      if (!(glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN = (csGLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUN) gl->GetProcAddress ("glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN"))) allclear = false;
      if (CS_GL_SUN_vertex = allclear)
        printf ("GL Extension 'GL_SUN_vertex' found and used.\n");
    }

    // GL_ARB_fragment_program
    CS_GL_ARB_fragment_program = (strstr (extensions, "GL_ARB_fragment_program") != NULL);
    if (CS_GL_ARB_fragment_program)
    {
      allclear = true;
      if (!(glProgramStringARB = (csGLPROGRAMSTRINGARB) gl->GetProcAddress ("glProgramStringARB"))) allclear = false;
      if (!(glBindProgramARB = (csGLBINDPROGRAMARB) gl->GetProcAddress ("glBindProgramARB"))) allclear = false;
      if (!(glDeleteProgramsARB = (csGLDELETEPROGRAMSARB) gl->GetProcAddress ("glDeleteProgramsARB"))) allclear = false;
      if (!(glGenProgramsARB = (csGLGENPROGRAMSARB) gl->GetProcAddress ("glGenProgramsARB"))) allclear = false;
      if (!(glProgramEnvParameter4dARB = (csGLPROGRAMENVPARAMETER4DARB) gl->GetProcAddress ("glProgramEnvParameter4dARB"))) allclear = false;
      if (!(glProgramEnvParameter4dvARB = (csGLPROGRAMENVPARAMETER4DVARB) gl->GetProcAddress ("glProgramEnvParameter4dvARB"))) allclear = false;
      if (!(glProgramEnvParameter4fARB = (csGLPROGRAMENVPARAMETER4FARB) gl->GetProcAddress ("glProgramEnvParameter4fARB"))) allclear = false;
      if (!(glProgramEnvParameter4fvARB = (csGLPROGRAMENVPARAMETER4FVARB) gl->GetProcAddress ("glProgramEnvParameter4fvARB"))) allclear = false;
      if (!(glProgramLocalParameter4dARB = (csGLPROGRAMLOCALPARAMETER4DARB) gl->GetProcAddress ("glProgramLocalParameter4dARB"))) allclear = false;
      if (!(glProgramLocalParameter4dvARB = (csGLPROGRAMLOCALPARAMETER4DVARB) gl->GetProcAddress ("glProgramLocalParameter4dvARB"))) allclear = false;
      if (!(glProgramLocalParameter4fARB = (csGLPROGRAMLOCALPARAMETER4FARB) gl->GetProcAddress ("glProgramLocalParameter4fARB"))) allclear = false;
      if (!(glProgramLocalParameter4fvARB = (csGLPROGRAMLOCALPARAMETER4FVARB) gl->GetProcAddress ("glProgramLocalParameter4fvARB"))) allclear = false;
      if (!(glGetProgramEnvParameterdvARB = (csGLGETPROGRAMENVPARAMETERDVARB) gl->GetProcAddress ("glGetProgramEnvParameterdvARB"))) allclear = false;
      if (!(glGetProgramEnvParameterfvARB = (csGLGETPROGRAMENVPARAMETERFVARB) gl->GetProcAddress ("glGetProgramEnvParameterfvARB"))) allclear = false;
      if (!(glGetProgramLocalParameterdvARB = (csGLGETPROGRAMLOCALPARAMETERDVARB) gl->GetProcAddress ("glGetProgramLocalParameterdvARB"))) allclear = false;
      if (!(glGetProgramLocalParameterfvARB = (csGLGETPROGRAMLOCALPARAMETERFVARB) gl->GetProcAddress ("glGetProgramLocalParameterfvARB"))) allclear = false;
      if (!(glGetProgramivARB = (csGLGETPROGRAMIVARB) gl->GetProcAddress ("glGetProgramivARB"))) allclear = false;
      if (!(glGetProgramStringARB = (csGLGETPROGRAMSTRINGARB) gl->GetProcAddress ("glGetProgramStringARB"))) allclear = false;
      if (!(glIsProgramARB = (csGLISPROGRAMARB) gl->GetProcAddress ("glIsProgramARB"))) allclear = false;
      if (CS_GL_ARB_fragment_program = allclear)
        printf ("GL Extension 'GL_ARB_fragment_program' found and used.\n");
    }

    // GL_ATI_text_fragment_shader
    CS_GL_ATI_text_fragment_shader = (strstr (extensions, "GL_ATI_text_fragment_shader") != NULL);
    if (CS_GL_ATI_text_fragment_shader)
    {
      allclear = true;
      if (CS_GL_ATI_text_fragment_shader = allclear)
        printf ("GL Extension 'GL_ATI_text_fragment_shader' found and used.\n");
    }

    // GL_APPLE_client_storage
    CS_GL_APPLE_client_storage = (strstr (extensions, "GL_APPLE_client_storage") != NULL);
    if (CS_GL_APPLE_client_storage)
    {
      allclear = true;
      if (CS_GL_APPLE_client_storage = allclear)
        printf ("GL Extension 'GL_APPLE_client_storage' found and used.\n");
    }

    // GL_APPLE_element_array
    CS_GL_APPLE_element_array = (strstr (extensions, "GL_APPLE_element_array") != NULL);
    if (CS_GL_APPLE_element_array)
    {
      allclear = true;
      if (!(glElementPointerAPPLE = (csGLELEMENTPOINTERAPPLE) gl->GetProcAddress ("glElementPointerAPPLE"))) allclear = false;
      if (!(glDrawElementArrayAPPLE = (csGLDRAWELEMENTARRAYAPPLE) gl->GetProcAddress ("glDrawElementArrayAPPLE"))) allclear = false;
      if (!(glDrawRangeElementArrayAPPLE = (csGLDRAWRANGEELEMENTARRAYAPPLE) gl->GetProcAddress ("glDrawRangeElementArrayAPPLE"))) allclear = false;
      if (!(glMultiDrawElementArrayAPPLE = (csGLMULTIDRAWELEMENTARRAYAPPLE) gl->GetProcAddress ("glMultiDrawElementArrayAPPLE"))) allclear = false;
      if (!(glMultiDrawRangeElementArrayAPPLE = (csGLMULTIDRAWRANGEELEMENTARRAYAPPLE) gl->GetProcAddress ("glMultiDrawRangeElementArrayAPPLE"))) allclear = false;
      if (CS_GL_APPLE_element_array = allclear)
        printf ("GL Extension 'GL_APPLE_element_array' found and used.\n");
    }

    // GL_APPLE_fence
    CS_GL_APPLE_fence = (strstr (extensions, "GL_APPLE_fence") != NULL);
    if (CS_GL_APPLE_fence)
    {
      allclear = true;
      if (!(glGenFencesAPPLE = (csGLGENFENCESAPPLE) gl->GetProcAddress ("glGenFencesAPPLE"))) allclear = false;
      if (!(glDeleteFencesAPPLE = (csGLDELETEFENCESAPPLE) gl->GetProcAddress ("glDeleteFencesAPPLE"))) allclear = false;
      if (!(glSetFenceAPPLE = (csGLSETFENCEAPPLE) gl->GetProcAddress ("glSetFenceAPPLE"))) allclear = false;
      if (!(glIsFenceAPPLE = (csGLISFENCEAPPLE) gl->GetProcAddress ("glIsFenceAPPLE"))) allclear = false;
      if (!(glTestFenceAPPLE = (csGLTESTFENCEAPPLE) gl->GetProcAddress ("glTestFenceAPPLE"))) allclear = false;
      if (!(glFinishFenceAPPLE = (csGLFINISHFENCEAPPLE) gl->GetProcAddress ("glFinishFenceAPPLE"))) allclear = false;
      if (!(glTestObjectAPPLE = (csGLTESTOBJECTAPPLE) gl->GetProcAddress ("glTestObjectAPPLE"))) allclear = false;
      if (!(glFinishObjectAPPLE = (csGLFINISHOBJECTAPPLE) gl->GetProcAddress ("glFinishObjectAPPLE"))) allclear = false;
      if (CS_GL_APPLE_fence = allclear)
        printf ("GL Extension 'GL_APPLE_fence' found and used.\n");
    }

    // GL_APPLE_vertex_array_object
    CS_GL_APPLE_vertex_array_object = (strstr (extensions, "GL_APPLE_vertex_array_object") != NULL);
    if (CS_GL_APPLE_vertex_array_object)
    {
      allclear = true;
      if (!(glBindVertexArrayAPPLE = (csGLBINDVERTEXARRAYAPPLE) gl->GetProcAddress ("glBindVertexArrayAPPLE"))) allclear = false;
      if (!(glDeleteVertexArraysAPPLE = (csGLDELETEVERTEXARRAYSAPPLE) gl->GetProcAddress ("glDeleteVertexArraysAPPLE"))) allclear = false;
      if (!(glGenVertexArraysAPPLE = (csGLGENVERTEXARRAYSAPPLE) gl->GetProcAddress ("glGenVertexArraysAPPLE"))) allclear = false;
      if (!(glIsVertexArrayAPPLE = (csGLISVERTEXARRAYAPPLE) gl->GetProcAddress ("glIsVertexArrayAPPLE"))) allclear = false;
      if (CS_GL_APPLE_vertex_array_object = allclear)
        printf ("GL Extension 'GL_APPLE_vertex_array_object' found and used.\n");
    }

    // GL_APPLE_vertex_array_range
    CS_GL_APPLE_vertex_array_range = (strstr (extensions, "GL_APPLE_vertex_array_range") != NULL);
    if (CS_GL_APPLE_vertex_array_range)
    {
      allclear = true;
      if (!(glVertexArrayRangeAPPLE = (csGLVERTEXARRAYRANGEAPPLE) gl->GetProcAddress ("glVertexArrayRangeAPPLE"))) allclear = false;
      if (!(glFlushVertexArrayRangeAPPLE = (csGLFLUSHVERTEXARRAYRANGEAPPLE) gl->GetProcAddress ("glFlushVertexArrayRangeAPPLE"))) allclear = false;
      if (!(glVertexArrayParameteriAPPLE = (csGLVERTEXARRAYPARAMETERIAPPLE) gl->GetProcAddress ("glVertexArrayParameteriAPPLE"))) allclear = false;
      if (CS_GL_APPLE_vertex_array_range = allclear)
        printf ("GL Extension 'GL_APPLE_vertex_array_range' found and used.\n");
    }

    // WGL_ARB_pixel_format
    CS_WGL_ARB_pixel_format = (strstr (extensions, "WGL_ARB_pixel_format") != NULL);
    if (CS_WGL_ARB_pixel_format)
    {
      allclear = true;
      if (!(wglGetPixelFormatAttribivARB = (csWGLGETPIXELFORMATATTRIBIVARB) gl->GetProcAddress ("wglGetPixelFormatAttribivARB"))) allclear = false;
      if (!(wglGetPixelFormatAttribfvARB = (csWGLGETPIXELFORMATATTRIBFVARB) gl->GetProcAddress ("wglGetPixelFormatAttribfvARB"))) allclear = false;
      if (!(wglChoosePixelFormatARB = (csWGLCHOOSEPIXELFORMATARB) gl->GetProcAddress ("wglChoosePixelFormatARB"))) allclear = false;
      if (CS_WGL_ARB_pixel_format = allclear)
        printf ("GL Extension 'WGL_ARB_pixel_format' found and used.\n");
    }

    // WGL_ARB_make_current_read
    CS_WGL_ARB_make_current_read = (strstr (extensions, "WGL_ARB_make_current_read") != NULL);
    if (CS_WGL_ARB_make_current_read)
    {
      allclear = true;
      if (!(wglMakeContextCurrentARB = (csWGLMAKECONTEXTCURRENTARB) gl->GetProcAddress ("wglMakeContextCurrentARB"))) allclear = false;
      if (!(wglGetCurrentReadDCARB = (csWGLGETCURRENTREADDCARB) gl->GetProcAddress ("wglGetCurrentReadDCARB"))) allclear = false;
      if (CS_WGL_ARB_make_current_read = allclear)
        printf ("GL Extension 'WGL_ARB_make_current_read' found and used.\n");
    }

    // WGL_ARB_pbuffer
    CS_WGL_ARB_pbuffer = (strstr (extensions, "WGL_ARB_pbuffer") != NULL);
    if (CS_WGL_ARB_pbuffer)
    {
      allclear = true;
      if (!(wglCreatePbufferARB = (csWGLCREATEPBUFFERARB) gl->GetProcAddress ("wglCreatePbufferARB"))) allclear = false;
      if (!(wglGetPbufferDCARB = (csWGLGETPBUFFERDCARB) gl->GetProcAddress ("wglGetPbufferDCARB"))) allclear = false;
      if (!(wglReleasePbufferDCARB = (csWGLRELEASEPBUFFERDCARB) gl->GetProcAddress ("wglReleasePbufferDCARB"))) allclear = false;
      if (!(wglDestroyPbufferARB = (csWGLDESTROYPBUFFERARB) gl->GetProcAddress ("wglDestroyPbufferARB"))) allclear = false;
      if (!(wglQueryPbufferARB = (csWGLQUERYPBUFFERARB) gl->GetProcAddress ("wglQueryPbufferARB"))) allclear = false;
      if (CS_WGL_ARB_pbuffer = allclear)
        printf ("GL Extension 'WGL_ARB_pbuffer' found and used.\n");
    }

    // WGL_EXT_swap_control
    CS_WGL_EXT_swap_control = (strstr (extensions, "WGL_EXT_swap_control") != NULL);
    if (CS_WGL_EXT_swap_control)
    {
      allclear = true;
      if (!(wglSwapIntervalEXT = (csWGLSWAPINTERVALEXT) gl->GetProcAddress ("wglSwapIntervalEXT"))) allclear = false;
      if (!(wglGetSwapIntervalEXT = (csWGLGETSWAPINTERVALEXT) gl->GetProcAddress ("wglGetSwapIntervalEXT"))) allclear = false;
      if (CS_WGL_EXT_swap_control = allclear)
        printf ("GL Extension 'WGL_EXT_swap_control' found and used.\n");
    }

    // WGL_ARB_render_texture
    CS_WGL_ARB_render_texture = (strstr (extensions, "WGL_ARB_render_texture") != NULL);
    if (CS_WGL_ARB_render_texture)
    {
      allclear = true;
      if (!(wglBindTexImageARB = (csWGLBINDTEXIMAGEARB) gl->GetProcAddress ("wglBindTexImageARB"))) allclear = false;
      if (!(wglReleaseTexImageARB = (csWGLRELEASETEXIMAGEARB) gl->GetProcAddress ("wglReleaseTexImageARB"))) allclear = false;
      if (!(wglSetPbufferAttribARB = (csWGLSETPBUFFERATTRIBARB) gl->GetProcAddress ("wglSetPbufferAttribARB"))) allclear = false;
      if (CS_WGL_ARB_render_texture = allclear)
        printf ("GL Extension 'WGL_ARB_render_texture' found and used.\n");
    }

    // WGL_EXT_extensions_string
    CS_WGL_EXT_extensions_string = (strstr (extensions, "WGL_EXT_extensions_string") != NULL);
    if (CS_WGL_EXT_extensions_string)
    {
      allclear = true;
      if (!(wglGetExtensionsStringEXT = (csWGLGETEXTENSIONSSTRINGEXT) gl->GetProcAddress ("wglGetExtensionsStringEXT"))) allclear = false;
      if (CS_WGL_EXT_extensions_string = allclear)
        printf ("GL Extension 'WGL_EXT_extensions_string' found and used.\n");
    }

    // WGL_EXT_make_current_read
    CS_WGL_EXT_make_current_read = (strstr (extensions, "WGL_EXT_make_current_read") != NULL);
    if (CS_WGL_EXT_make_current_read)
    {
      allclear = true;
      if (!(wglMakeContextCurrentEXT = (csWGLMAKECONTEXTCURRENTEXT) gl->GetProcAddress ("wglMakeContextCurrentEXT"))) allclear = false;
      if (!(wglGetCurrentReadDCEXT = (csWGLGETCURRENTREADDCEXT) gl->GetProcAddress ("wglGetCurrentReadDCEXT"))) allclear = false;
      if (CS_WGL_EXT_make_current_read = allclear)
        printf ("GL Extension 'WGL_EXT_make_current_read' found and used.\n");
    }

    // WGL_EXT_pbuffer
    CS_WGL_EXT_pbuffer = (strstr (extensions, "WGL_EXT_pbuffer") != NULL);
    if (CS_WGL_EXT_pbuffer)
    {
      allclear = true;
      if (!(wglCreatePbufferEXT = (csWGLCREATEPBUFFEREXT) gl->GetProcAddress ("wglCreatePbufferEXT"))) allclear = false;
      if (!(wglGetPbufferDCEXT = (csWGLGETPBUFFERDCEXT) gl->GetProcAddress ("wglGetPbufferDCEXT"))) allclear = false;
      if (!(wglReleasePbufferDCEXT = (csWGLRELEASEPBUFFERDCEXT) gl->GetProcAddress ("wglReleasePbufferDCEXT"))) allclear = false;
      if (!(wglDestroyPbufferEXT = (csWGLDESTROYPBUFFEREXT) gl->GetProcAddress ("wglDestroyPbufferEXT"))) allclear = false;
      if (!(wglQueryPbufferEXT = (csWGLQUERYPBUFFEREXT) gl->GetProcAddress ("wglQueryPbufferEXT"))) allclear = false;
      if (CS_WGL_EXT_pbuffer = allclear)
        printf ("GL Extension 'WGL_EXT_pbuffer' found and used.\n");
    }

    // WGL_EXT_pixel_format
    CS_WGL_EXT_pixel_format = (strstr (extensions, "WGL_EXT_pixel_format") != NULL);
    if (CS_WGL_EXT_pixel_format)
    {
      allclear = true;
      if (!(wglGetPixelFormatAttribivEXT = (csWGLGETPIXELFORMATATTRIBIVEXT) gl->GetProcAddress ("wglGetPixelFormatAttribivEXT"))) allclear = false;
      if (!(wglGetPixelFormatAttribfvEXT = (csWGLGETPIXELFORMATATTRIBFVEXT) gl->GetProcAddress ("wglGetPixelFormatAttribfvEXT"))) allclear = false;
      if (!(wglChoosePixelFormatEXT = (csWGLCHOOSEPIXELFORMATEXT) gl->GetProcAddress ("wglChoosePixelFormatEXT"))) allclear = false;
      if (CS_WGL_EXT_pixel_format = allclear)
        printf ("GL Extension 'WGL_EXT_pixel_format' found and used.\n");
    }

    // WGL_I3D_digital_video_control
    CS_WGL_I3D_digital_video_control = (strstr (extensions, "WGL_I3D_digital_video_control") != NULL);
    if (CS_WGL_I3D_digital_video_control)
    {
      allclear = true;
      if (!(wglGetDigitalVideoParametersI3D = (csWGLGETDIGITALVIDEOPARAMETERSI3D) gl->GetProcAddress ("wglGetDigitalVideoParametersI3D"))) allclear = false;
      if (!(wglSetDigitalVideoParametersI3D = (csWGLSETDIGITALVIDEOPARAMETERSI3D) gl->GetProcAddress ("wglSetDigitalVideoParametersI3D"))) allclear = false;
      if (CS_WGL_I3D_digital_video_control = allclear)
        printf ("GL Extension 'WGL_I3D_digital_video_control' found and used.\n");
    }

    // WGL_I3D_gamma
    CS_WGL_I3D_gamma = (strstr (extensions, "WGL_I3D_gamma") != NULL);
    if (CS_WGL_I3D_gamma)
    {
      allclear = true;
      if (!(wglGetGammaTableParametersI3D = (csWGLGETGAMMATABLEPARAMETERSI3D) gl->GetProcAddress ("wglGetGammaTableParametersI3D"))) allclear = false;
      if (!(wglSetGammaTableParametersI3D = (csWGLSETGAMMATABLEPARAMETERSI3D) gl->GetProcAddress ("wglSetGammaTableParametersI3D"))) allclear = false;
      if (!(wglGetGammaTableI3D = (csWGLGETGAMMATABLEI3D) gl->GetProcAddress ("wglGetGammaTableI3D"))) allclear = false;
      if (!(wglSetGammaTableI3D = (csWGLSETGAMMATABLEI3D) gl->GetProcAddress ("wglSetGammaTableI3D"))) allclear = false;
      if (CS_WGL_I3D_gamma = allclear)
        printf ("GL Extension 'WGL_I3D_gamma' found and used.\n");
    }

    // WGL_I3D_genlock
    CS_WGL_I3D_genlock = (strstr (extensions, "WGL_I3D_genlock") != NULL);
    if (CS_WGL_I3D_genlock)
    {
      allclear = true;
      if (!(wglEnableGenlockI3D = (csWGLENABLEGENLOCKI3D) gl->GetProcAddress ("wglEnableGenlockI3D"))) allclear = false;
      if (!(wglDisableGenlockI3D = (csWGLDISABLEGENLOCKI3D) gl->GetProcAddress ("wglDisableGenlockI3D"))) allclear = false;
      if (!(wglIsEnabledGenlockI3D = (csWGLISENABLEDGENLOCKI3D) gl->GetProcAddress ("wglIsEnabledGenlockI3D"))) allclear = false;
      if (!(wglGenlockSourceI3D = (csWGLGENLOCKSOURCEI3D) gl->GetProcAddress ("wglGenlockSourceI3D"))) allclear = false;
      if (!(wglGetGenlockSourceI3D = (csWGLGETGENLOCKSOURCEI3D) gl->GetProcAddress ("wglGetGenlockSourceI3D"))) allclear = false;
      if (!(wglGenlockSourceEdgeI3D = (csWGLGENLOCKSOURCEEDGEI3D) gl->GetProcAddress ("wglGenlockSourceEdgeI3D"))) allclear = false;
      if (!(wglGetGenlockSourceEdgeI3D = (csWGLGETGENLOCKSOURCEEDGEI3D) gl->GetProcAddress ("wglGetGenlockSourceEdgeI3D"))) allclear = false;
      if (!(wglGenlockSampleRateI3D = (csWGLGENLOCKSAMPLERATEI3D) gl->GetProcAddress ("wglGenlockSampleRateI3D"))) allclear = false;
      if (!(wglGetGenlockSampleRateI3D = (csWGLGETGENLOCKSAMPLERATEI3D) gl->GetProcAddress ("wglGetGenlockSampleRateI3D"))) allclear = false;
      if (!(wglGenlockSourceDelayI3D = (csWGLGENLOCKSOURCEDELAYI3D) gl->GetProcAddress ("wglGenlockSourceDelayI3D"))) allclear = false;
      if (!(wglGetGenlockSourceDelayI3D = (csWGLGETGENLOCKSOURCEDELAYI3D) gl->GetProcAddress ("wglGetGenlockSourceDelayI3D"))) allclear = false;
      if (!(wglQueryGenlockMaxSourceDelayI3D = (csWGLQUERYGENLOCKMAXSOURCEDELAYI3D) gl->GetProcAddress ("wglQueryGenlockMaxSourceDelayI3D"))) allclear = false;
      if (CS_WGL_I3D_genlock = allclear)
        printf ("GL Extension 'WGL_I3D_genlock' found and used.\n");
    }

    // GL_ARB_matrix_palette
    CS_GL_ARB_matrix_palette = (strstr (extensions, "GL_ARB_matrix_palette") != NULL);
    if (CS_GL_ARB_matrix_palette)
    {
      allclear = true;
      if (!(glCurrentPaletteMatrixARB = (csGLCURRENTPALETTEMATRIXARB) gl->GetProcAddress ("glCurrentPaletteMatrixARB"))) allclear = false;
      if (!(glMatrixIndexubvARB = (csGLMATRIXINDEXUBVARB) gl->GetProcAddress ("glMatrixIndexubvARB"))) allclear = false;
      if (!(glMatrixIndexusvARB = (csGLMATRIXINDEXUSVARB) gl->GetProcAddress ("glMatrixIndexusvARB"))) allclear = false;
      if (!(glMatrixIndexuivARB = (csGLMATRIXINDEXUIVARB) gl->GetProcAddress ("glMatrixIndexuivARB"))) allclear = false;
      if (!(glMatrixIndexPointerARB = (csGLMATRIXINDEXPOINTERARB) gl->GetProcAddress ("glMatrixIndexPointerARB"))) allclear = false;
      if (CS_GL_ARB_matrix_palette = allclear)
        printf ("GL Extension 'GL_ARB_matrix_palette' found and used.\n");
    }

    // GL_NV_element_array
    CS_GL_NV_element_array = (strstr (extensions, "GL_NV_element_array") != NULL);
    if (CS_GL_NV_element_array)
    {
      allclear = true;
      if (!(glElementPointerNV = (csGLELEMENTPOINTERNV) gl->GetProcAddress ("glElementPointerNV"))) allclear = false;
      if (!(glDrawElementArrayNV = (csGLDRAWELEMENTARRAYNV) gl->GetProcAddress ("glDrawElementArrayNV"))) allclear = false;
      if (!(glDrawRangeElementArrayNV = (csGLDRAWRANGEELEMENTARRAYNV) gl->GetProcAddress ("glDrawRangeElementArrayNV"))) allclear = false;
      if (!(glMultiDrawElementArrayNV = (csGLMULTIDRAWELEMENTARRAYNV) gl->GetProcAddress ("glMultiDrawElementArrayNV"))) allclear = false;
      if (!(glMultiDrawRangeElementArrayNV = (csGLMULTIDRAWRANGEELEMENTARRAYNV) gl->GetProcAddress ("glMultiDrawRangeElementArrayNV"))) allclear = false;
      if (CS_GL_NV_element_array = allclear)
        printf ("GL Extension 'GL_NV_element_array' found and used.\n");
    }

    // GL_NV_float_buffer
    CS_GL_NV_float_buffer = (strstr (extensions, "GL_NV_float_buffer") != NULL);
    if (CS_GL_NV_float_buffer)
    {
      allclear = true;
      if (CS_GL_NV_float_buffer = allclear)
        printf ("GL Extension 'GL_NV_float_buffer' found and used.\n");
    }

    // GL_NV_fragment_program
    CS_GL_NV_fragment_program = (strstr (extensions, "GL_NV_fragment_program") != NULL);
    if (CS_GL_NV_fragment_program)
    {
      allclear = true;
      if (!(glProgramNamedParameter4fNV = (csGLPROGRAMNAMEDPARAMETER4FNV) gl->GetProcAddress ("glProgramNamedParameter4fNV"))) allclear = false;
      if (!(glProgramNamedParameter4dNV = (csGLPROGRAMNAMEDPARAMETER4DNV) gl->GetProcAddress ("glProgramNamedParameter4dNV"))) allclear = false;
      if (!(glGetProgramNamedParameterfvNV = (csGLGETPROGRAMNAMEDPARAMETERFVNV) gl->GetProcAddress ("glGetProgramNamedParameterfvNV"))) allclear = false;
      if (!(glGetProgramNamedParameterdvNV = (csGLGETPROGRAMNAMEDPARAMETERDVNV) gl->GetProcAddress ("glGetProgramNamedParameterdvNV"))) allclear = false;
      if (!(glProgramLocalParameter4dARB = (csGLPROGRAMLOCALPARAMETER4DARB) gl->GetProcAddress ("glProgramLocalParameter4dARB"))) allclear = false;
      if (!(glProgramLocalParameter4dvARB = (csGLPROGRAMLOCALPARAMETER4DVARB) gl->GetProcAddress ("glProgramLocalParameter4dvARB"))) allclear = false;
      if (!(glProgramLocalParameter4fARB = (csGLPROGRAMLOCALPARAMETER4FARB) gl->GetProcAddress ("glProgramLocalParameter4fARB"))) allclear = false;
      if (!(glProgramLocalParameter4fvARB = (csGLPROGRAMLOCALPARAMETER4FVARB) gl->GetProcAddress ("glProgramLocalParameter4fvARB"))) allclear = false;
      if (!(glGetProgramLocalParameterdvARB = (csGLGETPROGRAMLOCALPARAMETERDVARB) gl->GetProcAddress ("glGetProgramLocalParameterdvARB"))) allclear = false;
      if (!(glGetProgramLocalParameterfvARB = (csGLGETPROGRAMLOCALPARAMETERFVARB) gl->GetProcAddress ("glGetProgramLocalParameterfvARB"))) allclear = false;
      if (CS_GL_NV_fragment_program = allclear)
        printf ("GL Extension 'GL_NV_fragment_program' found and used.\n");
    }

    // GL_NV_primitive_restart
    CS_GL_NV_primitive_restart = (strstr (extensions, "GL_NV_primitive_restart") != NULL);
    if (CS_GL_NV_primitive_restart)
    {
      allclear = true;
      if (!(glPrimitiveRestartNV = (csGLPRIMITIVERESTARTNV) gl->GetProcAddress ("glPrimitiveRestartNV"))) allclear = false;
      if (!(glPrimitiveRestartIndexNV = (csGLPRIMITIVERESTARTINDEXNV) gl->GetProcAddress ("glPrimitiveRestartIndexNV"))) allclear = false;
      if (CS_GL_NV_primitive_restart = allclear)
        printf ("GL Extension 'GL_NV_primitive_restart' found and used.\n");
    }

    // GL_NV_vertex_program2
    CS_GL_NV_vertex_program2 = (strstr (extensions, "GL_NV_vertex_program2") != NULL);
    if (CS_GL_NV_vertex_program2)
    {
      allclear = true;
      if (CS_GL_NV_vertex_program2 = allclear)
        printf ("GL Extension 'GL_NV_vertex_program2' found and used.\n");
    }

  }
};

#endif // __GLEXTENSIONMANAGER_H__