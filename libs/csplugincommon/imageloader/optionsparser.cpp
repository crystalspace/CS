/*
    Copyright (C) 2005 by Jorrit Tyberghein
		  2005 by Frank Richter

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

#include "csplugincommon/imageloader/optionsparser.h"

csImageLoaderOptionsParser::csImageLoaderOptionsParser (const char* options)
{
  const char *current_opt = options;
  while (current_opt && *current_opt)
  {
    if (*current_opt == ',') current_opt++;

    const char *opt_end = strchr (current_opt, ',');
    if (!opt_end) opt_end = current_opt + strlen (current_opt);

    csString opt_key;
    opt_key.Append (current_opt, opt_end - current_opt);
    current_opt = opt_end;

    csString opt_value;
    size_t opt_value_pos = opt_key.FindFirst ('=');
    if (opt_value_pos != (size_t)-1)
    {
      opt_key.SubString (opt_value, opt_value_pos + 1, 
        opt_key.Length() - opt_value_pos);
      opt_key.Truncate (opt_value_pos);
    }
    optValues.PutUnique (opt_key, opt_value);
  }
}

bool csImageLoaderOptionsParser::GetInt (const char* key, int& v) const
{
  const csString* val = optValues.GetElementPointer (key);
  if (!val) return false;

  char dummy;
  return (sscanf ("%d%c", *val, &v, &dummy) == 1);
}

bool csImageLoaderOptionsParser::GetBool (const char* key, bool& v) const
{
  const csString* val = optValues.GetElementPointer (key);
  if (!val) return false;

  if (val->IsEmpty())
  {
    v = true;
  }
  else
  {
    v = (*val == "yes") || (*val == "true") || (*val == "1") || (*val == "on");
  }
  return true;
}

bool csImageLoaderOptionsParser::GetFloat (const char* key, float& v) const
{
  const csString* val = optValues.GetElementPointer (key);
  if (!val) return false;

  char dummy;
  return (sscanf ("%f%c", *val, &v, &dummy) == 1);
}

bool csImageLoaderOptionsParser::GetString (const char* key, csString& v) const
{
  const csString* val = optValues.GetElementPointer (key);
  if (!val) return false;
  v = *val;
  return true;
}
