/*  -*- Mode: C++; -*- */
/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Contributions made by Robert Bergkvist <fragdance@hotmail.com>

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

#ifndef _WIN32_OPENGL_EXTENSIONS
#define _WIN32_OPENGL_EXTENSIONS

//Define the looks of the procedures
typedef void (APIENTRY *PFNGLACTIVETEXTUREARB)(GLenum texture);
typedef void (APIENTRY *PFNGLCLIENTACTIVETEXTUREARB)(GLenum texture);

typedef void (APIENTRY *PFNGLMULTITEXCOORD1SARB)(GLenum target, GLshort s);
typedef void (APIENTRY *PFNGLMULTITEXCOORD1IARB)(GLenum target, GLint s);
typedef void (APIENTRY *PFNGLMULTITEXCOORD1FARB)(GLenum target, GLfloat s);
typedef void (APIENTRY *PFNGLMULTITEXCOORD1DARB)(GLenum target, GLdouble s);
typedef void (APIENTRY *PFNGLMULTITEXCOORD2SARB)(GLenum target, GLshort s, GLshort t);
typedef void (APIENTRY *PFNGLMULTITEXCOORD2IARB)(GLenum target, GLint s, GLint t);
typedef void (APIENTRY *PFNGLMULTITEXCOORD2FARB)(GLenum target, GLfloat s, GLfloat t);
typedef void (APIENTRY *PFNGLMULTITEXCOORD2DARB)(GLenum target, GLdouble s, GLdouble t); 
typedef void (APIENTRY *PFNGLMULTITEXCOORD3SARB)(GLenum target, GLshort s, GLshort t, GLshort r);
typedef void (APIENTRY *PFNGLMULTITEXCOORD3IARB)(GLenum target, GLint s, GLint t, GLint r);
typedef void (APIENTRY *PFNGLMULTITEXCOORD3FARB)(GLenum target, GLfloat s, GLfloat t, GLfloat r);
typedef void (APIENTRY *PFNGLMULTITEXCOORD3DARB)(GLenum target, GLdouble s, GLdouble t, GLdouble r); 
typedef void (APIENTRY *PFNGLMULTITEXCOORD4SARB)(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
typedef void (APIENTRY *PFNGLMULTITEXCOORD4IARB)(GLenum target, GLint s, GLint t, GLint r, GLint q);
typedef void (APIENTRY *PFNGLMULTITEXCOORD4FARB)(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
typedef void (APIENTRY *PFNGLMULTITEXCOORD4DARB)(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q); 

typedef void (APIENTRY *PFNGLMULTITEXCOORD1SVARB)(GLenum target, const GLshort *v);
typedef void (APIENTRY *PFNGLMULTITEXCOORD1IVARB)(GLenum target, const GLint *v);
typedef void (APIENTRY *PFNGLMULTITEXCOORD1FVARB)(GLenum target, const GLfloat *v);
typedef void (APIENTRY *PFNGLMULTITEXCOORD1DVARB)(GLenum target, const GLdouble *v);
typedef void (APIENTRY *PFNGLMULTITEXCOORD2SVARB)(GLenum target, const GLshort *v);
typedef void (APIENTRY *PFNGLMULTITEXCOORD2IVARB)(GLenum target, const GLint *v);
typedef void (APIENTRY *PFNGLMULTITEXCOORD2FVARB)(GLenum target, const GLfloat *v);
typedef void (APIENTRY *PFNGLMULTITEXCOORD2DVARB)(GLenum target, const GLdouble *v);
typedef void (APIENTRY *PFNGLMULTITEXCOORD3SVARB)(GLenum target, const GLshort *v); 
typedef void (APIENTRY *PFNGLMULTITEXCOORD3IVARB)(GLenum target, const GLint *v);
typedef void (APIENTRY *PFNGLMULTITEXCOORD3FVARB)(GLenum target, const GLfloat *v);
typedef void (APIENTRY *PFNGLMULTITEXCOORD3DVARB)(GLenum target, const GLdouble *v);
typedef void (APIENTRY *PFNGLMULTITEXCOORD4SVARB)(GLenum target, const GLshort *v);
typedef void (APIENTRY *PFNGLMULTITEXCOORD4IVARB)(GLenum target, const GLint *v);
typedef void (APIENTRY *PFNGLMULTITEXCOORD4FVARB)(GLenum target, const GLfloat *v);
typedef void (APIENTRY *PFNGLMULTITEXCOORD4DVARB)(GLenum target, const GLdouble *v);

//The exported names

extern PFNGLACTIVETEXTUREARB glActiveTextureARB;
extern PFNGLCLIENTACTIVETEXTUREARB glClientActiveTextureARB;

extern PFNGLMULTITEXCOORD1SARB glMultiTexCoord1sARB;
extern PFNGLMULTITEXCOORD1IARB glMultiTexCoord1iARB;
extern PFNGLMULTITEXCOORD1FARB glMultiTexCoord1fARB;
extern PFNGLMULTITEXCOORD1FARB glMultiTexCoord1fARB;
extern PFNGLMULTITEXCOORD1DARB glMultiTexCoord1dARB;
extern PFNGLMULTITEXCOORD2SARB glMultiTexCoord2sARB;
extern PFNGLMULTITEXCOORD2IARB glMultiTexCoord2iARB;
extern PFNGLMULTITEXCOORD2FARB glMultiTexCoord2fARB;
extern PFNGLMULTITEXCOORD2DARB glMultiTexCoord2dARB;
extern PFNGLMULTITEXCOORD3SARB glMultiTexCoord3sARB;
extern PFNGLMULTITEXCOORD3IARB glMultiTexCoord3iARB;
extern PFNGLMULTITEXCOORD3FARB glMultiTexCoord3fARB;
extern PFNGLMULTITEXCOORD3DARB glMultiTexCoord3dARB;
extern PFNGLMULTITEXCOORD4SARB glMultiTexCoord4sARB;
extern PFNGLMULTITEXCOORD4IARB glMultiTexCoord4iARB;
extern PFNGLMULTITEXCOORD4FARB glMultiTexCoord4fARB;
extern PFNGLMULTITEXCOORD4DARB glMultiTexCoord4dARB;

extern PFNGLMULTITEXCOORD1SVARB glMultiTexCoord1svARB;
extern PFNGLMULTITEXCOORD1IVARB glMultiTexCoord1ivARB;
extern PFNGLMULTITEXCOORD1FVARB glMultiTexCoord1fvARB;
extern PFNGLMULTITEXCOORD1DVARB glMultiTexCoord1dvARB;
extern PFNGLMULTITEXCOORD2SVARB glMultiTexCoord2svARB;
extern PFNGLMULTITEXCOORD2IVARB glMultiTexCoord2ivARB;
extern PFNGLMULTITEXCOORD2FVARB glMultiTexCoord2fvARB;
extern PFNGLMULTITEXCOORD2DVARB glMultiTexCoord2dvARB;
extern PFNGLMULTITEXCOORD3SVARB glMultiTexCoord3svARB;
extern PFNGLMULTITEXCOORD3IVARB glMultiTexCoord3ivARB;
extern PFNGLMULTITEXCOORD3FVARB glMultiTexCoord3fvARB;
extern PFNGLMULTITEXCOORD3DVARB glMultiTexCoord3dvARB;
extern PFNGLMULTITEXCOORD4SVARB glMultiTexCoord4svARB;
extern PFNGLMULTITEXCOORD4IVARB glMultiTexCoord4ivARB;
extern PFNGLMULTITEXCOORD4FVARB glMultiTexCoord4fvARB;
extern PFNGLMULTITEXCOORD4DVARB glMultiTexCoord4dvARB;

#include <windows.h>
#include <gl/glext.h>
#include "../ogl_g3d.h"

PFNGLACTIVETEXTUREARB glActiveTextureARB;
PFNGLCLIENTACTIVETEXTUREARB glClientActiveTextureARB;

PFNGLMULTITEXCOORD1SARB glMultiTexCoord1sARB;
PFNGLMULTITEXCOORD1IARB glMultiTexCoord1iARB;
PFNGLMULTITEXCOORD1FARB glMultiTexCoord1fARB;
PFNGLMULTITEXCOORD1DARB glMultiTexCoord1dARB;
PFNGLMULTITEXCOORD2SARB glMultiTexCoord2sARB;
PFNGLMULTITEXCOORD2IARB glMultiTexCoord2iARB;
PFNGLMULTITEXCOORD2FARB glMultiTexCoord2fARB;
PFNGLMULTITEXCOORD2DARB glMultiTexCoord2dARB;
PFNGLMULTITEXCOORD3SARB glMultiTexCoord3sARB;
PFNGLMULTITEXCOORD3IARB glMultiTexCoord3iARB;
PFNGLMULTITEXCOORD3FARB glMultiTexCoord3fARB;
PFNGLMULTITEXCOORD3DARB glMultiTexCoord3dARB;
PFNGLMULTITEXCOORD4SARB glMultiTexCoord4sARB;
PFNGLMULTITEXCOORD4IARB glMultiTexCoord4iARB;
PFNGLMULTITEXCOORD4FARB glMultiTexCoord4fARB;
PFNGLMULTITEXCOORD4DARB glMultiTexCoord4dARB;

PFNGLMULTITEXCOORD1SVARB glMultiTexCoord1svARB;
PFNGLMULTITEXCOORD1IVARB glMultiTexCoord1ivARB;
PFNGLMULTITEXCOORD1FVARB glMultiTexCoord1fvARB;
PFNGLMULTITEXCOORD1DVARB glMultiTexCoord1dvARB;
PFNGLMULTITEXCOORD2SVARB glMultiTexCoord2svARB;
PFNGLMULTITEXCOORD2IVARB glMultiTexCoord2ivARB;
PFNGLMULTITEXCOORD2FVARB glMultiTexCoord2fvARB;
PFNGLMULTITEXCOORD2DVARB glMultiTexCoord2dvARB;
PFNGLMULTITEXCOORD3SVARB glMultiTexCoord3svARB;
PFNGLMULTITEXCOORD3IVARB glMultiTexCoord3ivARB;
PFNGLMULTITEXCOORD3FVARB glMultiTexCoord3fvARB;
PFNGLMULTITEXCOORD3DVARB glMultiTexCoord3dvARB;
PFNGLMULTITEXCOORD4SVARB glMultiTexCoord4svARB;
PFNGLMULTITEXCOORD4IVARB glMultiTexCoord4ivARB;
PFNGLMULTITEXCOORD4FVARB glMultiTexCoord4fvARB;
PFNGLMULTITEXCOORD4DVARB glMultiTexCoord4dvARB;

//Strings of the known extensions
static char const* const knownExtensions[] =
{
  "GL_ARB_multitexture",
  "GL_EXT_compiled_vertex_array",
  "GLX_ARB_get_proc_address",
  "GL_ARB_texture_compression"
};

//Number of know extensions
#define NUM_EXTENSIONS (sizeof(knownExtensions) / sizeof(knownExtensions[0]))

void csGraphics3DOGLCommon::DetectExtensions()
{
  printf ("Detecting Mesa Driver extensions...\n");
  const unsigned char* extensions = glGetString(GL_EXTENSIONS);
  if (!extensions)
    return;	//No luck, no extensions on this machine.

  // check the extension string for each extension in turn
  for (unsigned int i = 0; i < NUM_EXTENSIONS; i++)
  {
    const char* searchresult =
      strstr((const char*)extensions, knownExtensions[i]);

    while (searchresult)
    {
      // make sure we didn't accidently catch the
      // substring of some other extension
      if (*(searchresult + strlen(knownExtensions[i])) == ' ' ||
	  *(searchresult + strlen(knownExtensions[i])) == '\0')
      {
        printf (
	  "Found extension: %s\n", knownExtensions[i]);
        switch (i)
        {
          case 0:
	    ARB_multitexture = true;
          case 3:
	    ARB_texture_compression = true;
          default:
	    break;
        }
      }
      // find next occurance -- we could have multiple matches if we match the
      // substring of another extension, but only one will trigger the if
      // statement above
      searchresult = strstr(searchresult + 1, knownExtensions[i]);
    }
  }
}

// this tells the main ogl_g3d.cpp module not to define a 'default'
#define _DEFINED_DETECTION_METHOD

#endif _WIN32_OPENGL_EXTENSIONS
