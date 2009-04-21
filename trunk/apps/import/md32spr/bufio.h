/*
    Copyright (C) 2002 by Manjunath Sripadarao

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

#ifndef __DATABUF_H
#define __DATABUF_H

class DataBuffer {
  char *data;
  size_t position;
  size_t buffSize;
  size_t maxline;
 public:
  DataBuffer(char*, size_t);
  //DataBuffer(csString);
  ~DataBuffer();
  void SetPosition(size_t p);
  size_t GetPosition();
  void ResetPosition();
  size_t GetSize();
  void SetData(char *d, size_t bSize);
  bool GetData(char *,size_t);
  bool GetLine(char *);
  size_t GetMaxLineLength() ;
  bool eof();
};


#endif
