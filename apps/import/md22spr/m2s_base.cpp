/*
  Crystal Space Quake MDL/MD2 convertor
  Copyright (C) 1998 by Nathaniel Saint Martin <noote@bigfoot.com>
  Significant overhaul by Eric Sunshine <sunshine@sunshineco.com> in Feb 2000

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
#include "m2s_base.h"

bool QModel::CheckMagic(const char* mdlfile, const char* magic)
{
  bool ok = false;
  FILE* file = fopen(mdlfile, "rb");
  if (file != 0)
  {
    char buff[4];
    if (fread(buff, 4, 1, file) == 1)
      ok = (strncmp(buff, magic, 4) == 0);
    fclose(file);
  }
  return ok;
}

QModel::QModel() : sError(0)
{
  clearError();
}

QModel::~QModel()
{
  if (sError != 0)
    free(sError);
}

bool QModel::setError(const char* errorstring, FILE* closethis)
{
  if (closethis != 0)
    fclose(closethis);

  if (sError != 0)
    free(sError);

  if (errorstring == 0)
    sError = strdup("Unknown error");
  else
    sError = strdup(errorstring);

  bError = true;
  return false;
}

void QModel::clearError()
{
  setError("No error");
  bError = false;
}
