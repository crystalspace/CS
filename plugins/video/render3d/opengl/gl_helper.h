/*
    Copyright (C) 2002 by Mårten Svanfeldt
                          Anders Stenberg

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

#ifndef __CS_GL_HELPER_H__
#define __CS_GL_HELPER_H__

class csGLExtensionManager;

class csGLVertexArrayHelper
{
public:
  csGLExtensionManager* ext;

  bool IsATI;

  csGLVertexArrayHelper() : IsATI (false) {}
  
  inline void VertexPointer(GLint size, GLenum type, GLsizei stride,
                            GLvoid* pointer)
  {
    if(IsATI)
      ext->glArrayObjectATI(GL_VERTEX_ARRAY, size, type, stride,(int)pointer, 0);
    else
      glVertexPointer(size, type, stride, pointer);
  }

  inline void TexCoordPointer(GLint size, GLenum type, GLsizei stride,
                            GLvoid* pointer)
  {
    if(IsATI)
      ext->glArrayObjectATI(GL_TEXTURE_COORD_ARRAY, size, type, stride,(int)pointer, 0);
    else
      glTexCoordPointer(size, type, stride, pointer);
  }

  inline void NormalPointer(GLenum type, GLsizei stride,
                            GLvoid* pointer)
  {
    if(IsATI)
      ext->glArrayObjectATI(GL_NORMAL_ARRAY, 3, type, stride,(int)pointer, 0);
    else
      glNormalPointer(type, stride, pointer);
  }

  inline void ColorPointer(GLint size, GLenum type, GLsizei stride,
                            GLvoid* pointer)
  {
    if(IsATI)
      ext->glArrayObjectATI(GL_COLOR_ARRAY, size, type, stride,(int)pointer, 0);
    else
      glColorPointer(size, type, stride, pointer);
  }

  inline void VertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized,
                            GLsizei stride, GLvoid* pointer)
  {
    if(IsATI)
      ext->glVertexAttribArrayObjectATI(index, size, type, normalized, stride, (int)pointer, 0);
    else
      ext->glVertexAttribPointerARB(index, size, type, normalized, stride, pointer);
  }

};

#endif