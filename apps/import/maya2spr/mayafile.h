/*
  Crystal Space Maya .ma Convertor
  Copyright (C) 2002 by Keith Fulton <keith@paqrat.com>
    (loosely based on "mdl2spr" by Nathaniel Saint Martin <noote@bigfoot.com>
                     and Eric Sunshine <sunshine@sunshineco.com>)

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

// Generic file class
// InputFile class
// MayaInputFile class

#ifndef MAYAFILE_Z
#define MAYAFILE_Z

#include "csutil/csstring.h"

class File
{
protected:
    csString Filename;
    FILE *f;

public:
    File(const char *str);
    ~File();

    void Close(void);

    int IsValid(void) { return (f!=0); };

    long Length(void)
    {
        if (IsValid())
        {
            long x;
            fseek(f,0,SEEK_END);
            x = ftell(f);
            fseek(f,0,SEEK_SET); //reset to beginning of file
            return x;
        }
        else
            return 0;
    };
    
    void Reset(void)
    {
        if (IsValid())
            fseek(f,0,SEEK_SET);
    };
    
    int EndOfFile(void)
    {
        if (IsValid())
            return (feof(f));
        else
            return 1; //always EOF if invalid file ptr
    };
    
    long GetCurrPosition(void)
    {
        if (IsValid())
            return ftell(f);
        else
            return 0;
    };
};

class InputFile : public File
{
protected:
    csString stBuff;

public:
    InputFile(const char *pStr);

    csString& ReadBytes(int iBytes);
    int  ReadRawBytes(int iBytes,void *pBuff);

    int ReadLine(csString& str);
    csString& ReadWord(char *pCharList="",int iWantTrailingSpace=0);
    int Read(void *pPtr,int iNum);
};

class MayaInputFile : public InputFile
{
    csString pushtoken;
    csString error;
    int line;

public:
    MayaInputFile(const char *pStr) : InputFile(pStr) { line = 0; };

    bool GetToken(csString& tok);
    void PushToken(csString& tok) { pushtoken = tok; };

    int GetType();
    bool GetFloat(int type,float& fl);

    void SetError(const char *str);
    const char *GetError() { return error; };
};

#endif
