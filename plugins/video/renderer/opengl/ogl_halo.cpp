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

/* This file is more or less a ripoff of Andrew Zabolotny's software halo renderer */

#include "cssysdef.h"
#ifdef OS_WIN32
#include <windows.h>
#endif
#include "qint.h"
#include "csgeom/math2d.h"
#include "csutil/util.h"
#include "ogl_g3dcom.h"
#include "ihalo.h"

class csOpenGLHalo:public iHalo
{
  float R,G,B;        //This halos color
  unsigned char *Alpha;   //The original map
  int Width,Height,Size;    //Take a wild guess
  int x_scale,y_scale;    //For scaling between actual size and OpenGL size
                //This is needed because we want 2^n maps for OpenGL,
                //alphamap size is arbitrary.
  GLuint halohandle;      //Our OpenGL texture handle
  csGraphics3DOGLCommon *G3D;  //
public:
  DECLARE_IBASE;

  csOpenGLHalo (float iR,float iG,float iB, unsigned char *iAlpha, 
		int iWidth,int iHeight, csGraphics3DOGLCommon *iG3D);

  virtual ~csOpenGLHalo();

  virtual int GetWidth(){return Width;};
  virtual int GetHeight(){return Height;};

  virtual void SetColor(float &iR,float &iG,float &iB)
  {R=iR;G=iG;B=iB;}

  virtual void GetColor(float &oR,float &oG,float &oB)
  { oR=R;oG=G;oB=B;}

  virtual void Draw(float x,float y,float w,float h,float iIntensity,csVector2 *iVertices,int iVertCount);
};

IMPLEMENT_IBASE(csOpenGLHalo)
  IMPLEMENTS_INTERFACE(iHalo)
IMPLEMENT_IBASE_END

csOpenGLHalo::csOpenGLHalo(float iR,float iG,float iB,unsigned char *iAlpha,
               int iWidth,int iHeight,csGraphics3DOGLCommon *iG3D)
{
  CONSTRUCT_IBASE(NULL);

  //Initialization
  R=iR;G=iG;B=iB;
  Width=FindNearestPowerOf2(iWidth);  //OpenGL can only use 2^n sized textures
  Height=FindNearestPowerOf2(iHeight);
  Size=Width*Height;

  Alpha=new unsigned char [Width*Height];

  memset(Alpha,0,Width*Height); //Clear both buffers (no garbage)

  for(int i = 0;i<iHeight;i++)
    memcpy(Alpha+(i*Width),iAlpha+(i*iWidth),iWidth); //Copy the supplied alphamap

  x_scale=int((1.0*iWidth)/Width);   //Calculate the scale between halo size and our texture size
  y_scale=int((1.0*iHeight)/Height); // (used for drawing)

  glGenTextures(1,&halohandle); //Create handle
  glBindTexture(GL_TEXTURE_2D,halohandle);  //Activate handle


  //Jaddajaddajadda
  glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D,0,GL_ALPHA,Width,Height,0,GL_ALPHA,
    GL_UNSIGNED_BYTE,Alpha);
	delete []Alpha;
  (G3D=iG3D)->IncRef();
}

csOpenGLHalo::~csOpenGLHalo()
{
  //Kill, crush and destroy
  glDeleteTextures(1,&halohandle);  //Delete generated OpenGL handle
  G3D->DecRef();
}

//Draw the halo. Wasn't that a suprise
void csOpenGLHalo::Draw(float x,float y,float w,float h,float iIntensity,csVector2 *iVertices,int iVertCount)
{
  (void) w;
  (void) h;
  int height=G3D->GetHeight();
  int i;
  csVector2 *texcoords=new csVector2[iVertCount];

  //Saw that this was used somewhere else, so it has to be good
  glPushAttrib (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_TEXTURE_BIT);
  glDisable(GL_DEPTH_TEST);

  //Simple 'clippping' (more like adjustments) of texture coords
  for(i=0;i<iVertCount;i++)
  {
    texcoords[i].x=(iVertices[i].x-x)/Width;
    texcoords[i].y=(iVertices[i].y-y)/Height;
  }

  glBindTexture (GL_TEXTURE_2D, halohandle);    
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glShadeModel (GL_FLAT);

  //Our usual blending
  glBlendFunc (GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glColor4f(R,G,B, iIntensity);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  glBegin (GL_POLYGON);
  {
    for(i=0;i<iVertCount;i++)
    {
      glTexCoord2f(texcoords[i].x,texcoords[i].y);
      glVertex2f(iVertices[i].x,height-iVertices[i].y-1);
    }
  }
  glEnd ();
  glDisable(GL_BLEND);
  glDisable(GL_TEXTURE_2D);
  glPopAttrib();

  delete []texcoords; //Ooops. Almost forgot.
}

iHalo *csGraphics3DOGLCommon::CreateHalo (float iR, float iG, float iB,
			      unsigned char *iAlpha,int iWidth,int iHeight)
{
  csOpenGLHalo *h=new csOpenGLHalo(iR,iG,iB,iAlpha,iWidth,iHeight,this);
  return h;
}
