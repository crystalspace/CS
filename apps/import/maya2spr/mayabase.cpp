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

#include "cssysdef.h"
#include "mayabase.h"

bool MayaModel::CheckMagic(const char* mdlfile, const char* magic)
{
  bool ok = false;
  FILE* file = fopen(mdlfile, "rb");
  if (file != 0)
  {
    char buff[512];
    if (fread(buff, strlen(magic), 1, file) == 1)
      ok = (strncmp(buff, magic, strlen(magic)) == 0);
    fclose(file);
  }
  return ok;
}

MayaModel::MayaModel() : sError("")
{
  clearError();
}

MayaModel::~MayaModel()
{
}

bool MayaModel::setError(const char* errorstring, FILE* closethis)
{
  if (closethis != 0)
    fclose(closethis);

  if (errorstring == 0)
    sError = "Unknown error";
  else
    sError = errorstring;

  bError = true;
  return false;
}

void MayaModel::clearError()
{
  setError("No error");
  bError = false;
}
