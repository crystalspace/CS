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

// Implementation of File Class
// Implementation of InputFile Class
// Implementation of OutputFile Class

#include "cssysdef.h"
#include "mayafile.h"
#include <ctype.h>
#include <errno.h>
#include <string.h>



File::File(const char *str)
{
    Filename = str;
    f = 0;
}

File::~File()
{
    if (f)
        fclose(f);
}

void File::Close()
{
    if (f)
    {
        fclose(f);
        f = 0;
    }
}


//
// Implementation of InputFile


InputFile::InputFile(const char *pStr)
  : File(pStr)
{
    f = fopen(pStr,"rb");
    stBuff.SetCapacity(1000);
}


int InputFile::ReadRawBytes(int iBytes,void *pBuff)
{
    if (!IsValid())
        return 0;
    return (int)fread(pBuff,iBytes,1,f);
}

csString& InputFile::ReadBytes(int iBytes)
{
    if (!IsValid())
    {
        stBuff = "";
        return stBuff;
    }
    for (int x=0; x<iBytes; x++)
    {
        int ch;

        if (!fread(&ch,1,1,f))
            break;

        stBuff.SetAt(x, ch);
    }
    return stBuff;
}

int InputFile::Read(void *pPtr,int iNum)
{
    if (IsValid())
        return (int)fread(pPtr,1,iNum,f);
    else
        return 0;
}

int InputFile::ReadLine(csString &str)
{
    int ch=0;

    str = "";

    if (!IsValid())
        return -1;
    int x;
    for (x=0; ch!='\n'; x++)
    {
        ch = fgetc(f);
        if (ch == '\r' || ch == '\n')
        {
            x--;
            continue;
        }

        if (ch == EOF)
            break;
        else
            str.Append((char)ch);
    }

    str.Append((char)0);

    if (ch == EOF && x==0)
        return -1;
    else
        return x;
}

csString& InputFile::ReadWord(char *pCharList,int iWantTrailingSpace)
{
    int ch='a';

    stBuff="";
    if (!IsValid())
    {
        return stBuff;
    }
    int x;
    for (x=0; (isalnum(ch) || strchr(pCharList,ch)); x++)
    {
        ch = fgetc(f);
        if (ch == EOF)
            break;
        else
            stBuff.Append((char)ch);
    }
    if (x>1)
    {
        if (ch == ' ' && iWantTrailingSpace)
            stBuff.Append((char)0);
        else
        {
            stBuff.SetAt(x-1,(char)0);
            ungetc(ch,f);
        }
    }
    else
        stBuff.Append((char)0);

    return stBuff;
}


bool MayaInputFile::GetToken(csString& tok)
{
    if (pushtoken.Length())
    {
        tok = pushtoken;
        pushtoken = "";
        return true;
    }

    csString word;
    bool stop=false;
    bool quoted=false;

    tok = "";
    while (!stop)
    {
        word = ReadWord("_-./\"", false);    // . and " are allowed within the word
        if (!word.Length())
            break;

        if (!tok.Length() && word.GetAt(0) == '\"')
            quoted = true;

        if (quoted)
        {
            tok.Append(word);
            if (word.GetAt(word.Length()-1) == '\"') // trailing quote
                stop = true;
        }
        else
        {
            if (isspace(word.GetAt(0)))
            {
                if (word == "\n")
                    line++;

                continue;
            }
            if (!strncmp(word,"//",2)) // comment line
            {
                ReadLine(word);         // skip entire line and get next
                line++;
                if (tok.Length() == 0)
                {
                    continue;
                }
                else
                {
                    break;      // existing token is good enough
                }
            }
            tok.Append(word);
            stop = true;
        }
    }

    // take off delimiting quotes, if any
    if (tok.Length() > 1 && quoted)
    {
        tok.DeleteAt(0);
        tok.DeleteAt(tok.Length()-1);
    }

    return (tok.Length() > 0);
}


int MayaInputFile::GetType()
{
    csString tok;

    GetToken(tok);
    if (tok == "float2")
        return 1;
    else if (tok == "double3")
        return 2;
    else
        return 0;
}

bool MayaInputFile::GetFloat(int type,float& fl)
{
    csString token;

    switch(type)
    {
    case 1:
        GetToken(token);
        sscanf(token,"%f",&fl);
        return true;

    case 2:
        double d;
        GetToken(token);
        sscanf(token,"%lf",&d);
        fl = (float)d;
        return true;
    }
    return false;
}

void MayaInputFile::SetError(const char *str)
{ 
    error.Format("%s  (%s:%d)",str,(const char *)Filename,line);
}
