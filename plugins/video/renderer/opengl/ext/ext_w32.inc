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

//I took these values from Mesas gl.h, and it seem to work

#define GL_TEXTURE0_ARB			0x84C0
#define GL_TEXTURE1_ARB			0x84C1
#define GL_TEXTURE2_ARB			0x84C2
#define GL_TEXTURE3_ARB			0x84C3
#define GL_TEXTURE4_ARB			0x84C4
#define GL_TEXTURE5_ARB			0x84C5
#define GL_TEXTURE6_ARB			0x84C6
#define GL_TEXTURE7_ARB			0x84C7
#define GL_TEXTURE8_ARB			0x84C8
#define GL_TEXTURE9_ARB			0x84C9
#define GL_TEXTURE10_ARB		0x84CA
#define GL_TEXTURE11_ARB		0x84CB
#define GL_TEXTURE12_ARB		0x84CC
#define GL_TEXTURE13_ARB		0x84CD
#define GL_TEXTURE14_ARB		0x84CE
#define GL_TEXTURE15_ARB		0x84CF
#define GL_TEXTURE16_ARB		0x84D0
#define GL_TEXTURE17_ARB		0x84D1
#define GL_TEXTURE18_ARB		0x84D2
#define GL_TEXTURE19_ARB		0x84D3
#define GL_TEXTURE20_ARB		0x84D4
#define GL_TEXTURE21_ARB		0x84D5
#define GL_TEXTURE22_ARB		0x84D6
#define GL_TEXTURE23_ARB		0x84D7
#define GL_TEXTURE24_ARB		0x84D8
#define GL_TEXTURE25_ARB		0x84D9
#define GL_TEXTURE26_ARB		0x84DA
#define GL_TEXTURE27_ARB		0x84DB
#define GL_TEXTURE28_ARB		0x84DC
#define GL_TEXTURE29_ARB		0x84DD
#define GL_TEXTURE30_ARB		0x84DE
#define GL_TEXTURE31_ARB		0x84DF
#define GL_ACTIVE_TEXTURE_ARB		0x84E0
#define GL_CLIENT_ACTIVE_TEXTURE_ARB	0x84E1
#define GL_MAX_TEXTURE_UNITS_ARB	0x84E2

#include <windows.h>
#include "../ogl_g3d.h"

/*********************************************************************
*
* Big, extensive multitexture-function-list coming. Think I've got it
* all covered.  Tell me otherwise.
*
***********************************************************************/

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

/*****************************************************************
*
* End of Multitexture declarations.
*
******************************************************************/

//Strings of the known extensions
static char const* const knownExtensions[] = {"GL_ARB_multitexture"};

//Number of know extensions
#define NUM_EXTENSIONS (sizeof(knownExtensions) / sizeof(knownExtensions[0]))

// Here I get all function names for Windows (I don't like the platform,
// yet I go through all this trouble. Go figure.). 
bool setupARBmultitexture()
{
  if(!(glActiveTextureARB=(PFNGLACTIVETEXTUREARB)wglGetProcAddress("glActiveTextureARB")))return false;
  if(!(glClientActiveTextureARB=(PFNGLCLIENTACTIVETEXTUREARB)wglGetProcAddress("glClientActiveTextureARB")))return false;

  if(!(glMultiTexCoord1sARB=(PFNGLMULTITEXCOORD1SARB)wglGetProcAddress("glMultiTexCoord1sARB")))return false;
  if(!(glMultiTexCoord1iARB=(PFNGLMULTITEXCOORD1IARB)wglGetProcAddress("glMultiTexCoord1iARB")))return false;
  if(!(glMultiTexCoord1fARB=(PFNGLMULTITEXCOORD1FARB)wglGetProcAddress("glMultiTexCoord1fARB")))return false;
  if(!(glMultiTexCoord1dARB=(PFNGLMULTITEXCOORD1DARB)wglGetProcAddress("glMultiTexCoord1dARB")))return false;
  if(!(glMultiTexCoord2sARB=(PFNGLMULTITEXCOORD2SARB)wglGetProcAddress("glMultiTexCoord2sARB")))return false;
  if(!(glMultiTexCoord2iARB=(PFNGLMULTITEXCOORD2IARB)wglGetProcAddress("glMultiTexCoord2iARB")))return false;
  if(!(glMultiTexCoord2fARB=(PFNGLMULTITEXCOORD2FARB)wglGetProcAddress("glMultiTexCoord2fARB")))return false;
  if(!(glMultiTexCoord2dARB=(PFNGLMULTITEXCOORD2DARB)wglGetProcAddress("glMultiTexCoord2dARB")))return false;
  if(!(glMultiTexCoord3sARB=(PFNGLMULTITEXCOORD3SARB)wglGetProcAddress("glMultiTexCoord3sARB")))return false;
  if(!(glMultiTexCoord3iARB=(PFNGLMULTITEXCOORD3IARB)wglGetProcAddress("glMultiTexCoord3iARB")))return false;
  if(!(glMultiTexCoord3fARB=(PFNGLMULTITEXCOORD3FARB)wglGetProcAddress("glMultiTexCoord3fARB")))return false;
  if(!(glMultiTexCoord3dARB=(PFNGLMULTITEXCOORD3DARB)wglGetProcAddress("glMultiTexCoord3dARB")))return false;
  if(!(glMultiTexCoord4sARB=(PFNGLMULTITEXCOORD4SARB)wglGetProcAddress("glMultiTexCoord4sARB")))return false;
  if(!(glMultiTexCoord4iARB=(PFNGLMULTITEXCOORD4IARB)wglGetProcAddress("glMultiTexCoord4iARB")))return false;
  if(!(glMultiTexCoord4fARB=(PFNGLMULTITEXCOORD4FARB)wglGetProcAddress("glMultiTexCoord4fARB")))return false;
  if(!(glMultiTexCoord4dARB=(PFNGLMULTITEXCOORD4DARB)wglGetProcAddress("glMultiTexCoord4dARB")))return false;

  if(!(glMultiTexCoord1svARB=(PFNGLMULTITEXCOORD1SVARB)wglGetProcAddress("glMultiTexCoord1svARB")))return false;
  if(!(glMultiTexCoord1ivARB=(PFNGLMULTITEXCOORD1IVARB)wglGetProcAddress("glMultiTexCoord1ivARB")))return false;
  if(!(glMultiTexCoord1fvARB=(PFNGLMULTITEXCOORD1FVARB)wglGetProcAddress("glMultiTexCoord1fvARB")))return false;
  if(!(glMultiTexCoord1dvARB=(PFNGLMULTITEXCOORD1DVARB)wglGetProcAddress("glMultiTexCoord1dvARB")))return false;
  if(!(glMultiTexCoord2svARB=(PFNGLMULTITEXCOORD2SVARB)wglGetProcAddress("glMultiTexCoord2svARB")))return false;
  if(!(glMultiTexCoord2ivARB=(PFNGLMULTITEXCOORD2IVARB)wglGetProcAddress("glMultiTexCoord2ivARB")))return false;
  if(!(glMultiTexCoord2fvARB=(PFNGLMULTITEXCOORD2FVARB)wglGetProcAddress("glMultiTexCoord2fvARB")))return false;
  if(!(glMultiTexCoord2dvARB=(PFNGLMULTITEXCOORD2DVARB)wglGetProcAddress("glMultiTexCoord2dvARB")))return false;
  if(!(glMultiTexCoord3svARB=(PFNGLMULTITEXCOORD3SVARB)wglGetProcAddress("glMultiTexCoord3svARB")))return false;
  if(!(glMultiTexCoord3ivARB=(PFNGLMULTITEXCOORD3IVARB)wglGetProcAddress("glMultiTexCoord3ivARB")))return false;
  if(!(glMultiTexCoord3fvARB=(PFNGLMULTITEXCOORD3FVARB)wglGetProcAddress("glMultiTexCoord3fvARB")))return false;
  if(!(glMultiTexCoord3dvARB=(PFNGLMULTITEXCOORD3DVARB)wglGetProcAddress("glMultiTexCoord3dvARB")))return false;
  if(!(glMultiTexCoord4svARB=(PFNGLMULTITEXCOORD4SVARB)wglGetProcAddress("glMultiTexCoord4svARB")))return false;
  if(!(glMultiTexCoord4ivARB=(PFNGLMULTITEXCOORD4IVARB)wglGetProcAddress("glMultiTexCoord4ivARB")))return false;
  if(!(glMultiTexCoord4fvARB=(PFNGLMULTITEXCOORD4FVARB)wglGetProcAddress("glMultiTexCoord4fvARB")))return false;
  if(!(glMultiTexCoord4dvARB=(PFNGLMULTITEXCOORD4DVARB)wglGetProcAddress("glMultiTexCoord4dvARB")))return false;

  return true;
}

// This function detects which extensions that is available on this platform. This function 
// should be implemented on all OpenGL platforms (or maybe we should share this, move it
// into ogl_g3d.cpp prehaps)
void csGraphics3DOGLCommon::DetectExtensions()
{
  //The SysPrintf is just for debugging, but it writes which extensions you have into
  //debug.txt. Made me jealous when I ran it on a TnT2 :-(
  printf ("Detecting Win32 OpenGL extensions.\n");
  const unsigned char *extensions;
  char *currentextension;

  extensions=glGetString(GL_EXTENSIONS);
  if(!extensions) return;	//No luck, no extensions on this machine.

  currentextension=strtok((char*)extensions," "); //Find the first substring
  if(!currentextension) return; //Nope, just an empty string
  while(1)
  {
    //Debugging
    printf ("Found extension: %s\n",currentextension);

    //Go through all the extension we know, and see if we find this one
    for(int i=0;i<NUM_EXTENSIONS;i++)
    {
      if(!strcmp(currentextension,knownExtensions[i]))
        break;
    }

    // i is now an index, so use it
    switch (i)
    {
    case 0:
      if(setupARBmultitexture())
        printf("All procs found\n");
      else
        printf("Not all procs found\n");
        ARB_multitexture=true;	//Flag that should be checked in renderer
      break;
    default:
      break;
    }
    if(!(currentextension=strtok(0," "))) return;
  }
}

// this tells the main ogl_g3d.cpp module not to define a 'default'
#define _DEFINED_DETECTION_METHOD

#endif _WIN32_OPENGL_EXTENSIONS
