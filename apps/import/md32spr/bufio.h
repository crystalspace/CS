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
  int position;
  int buffSize;
  int maxline;
 public:
  DataBuffer(char*, int);
  //DataBuffer(csString);
  ~DataBuffer();
  void SetPosition(int p) { position = p; }
  int GetPosition() { return position; }
  void ResetPosition() {  position = 0; }
  int GetSize() { return buffSize; }
  void SetData(char *d, int bSize) { data = d; buffSize = bSize; position = 0; }
  bool GetData(char *,int);
  bool GetLine(char *);
  int GetMaxLineLength() { return maxline; }
  bool eof() { return ((position >= buffSize) ? true : false); }
};


#endif
