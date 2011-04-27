/*
    Copyright (C) 2011 by Frank Richter

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

#include "textureclass.h"

#include "iutil/cfgfile.h"
#include "csutil/csstring.h"

void TextureClass::Parse (iConfigFile* config, const char* prefix)
{
  csString iterPrefix (prefix);
  iterPrefix.Append ('.');

  csRef<iConfigIterator> keys (config->Enumerate (iterPrefix));
  while (keys->HasNext())
  {
    keys->Next ();
    
    const char* keyName = keys->GetKey (true);
    if (strcasecmp (keyName, "SharpenPrecomputedMipmaps") == 0)
    {
      sharpenPrecomputed = keys->GetBool ();
    } 
    else if (strcasecmp (keyName, "AllowMipSharpen") == 0)
    {
      sharpenMips = keys->GetBool ();
    } 
  }
}

//---------------------------------------------------------------------------

void TextureClassManager::Parse (iConfigFile* config)
{
  csString extractedClass;
  csRef<iConfigIterator> keys (config->Enumerate ("Video.OpenGL.TextureClass."));
  while (keys->HasNext())
  {
    keys->Next ();
    
    const char* keyName = keys->GetKey (true);
    const char* dot = strchr (keyName, '.');
    if (dot != 0)
    {
      extractedClass.Replace (keyName, dot - keyName);
      if (!classes.Contains (extractedClass))
      {
	csString fullPrefix (keys->GetSubsection ());
	fullPrefix.Append (extractedClass);
	classes.GetOrCreate (extractedClass).Parse (config, fullPrefix);
      }
    }
  }
}

const TextureClass& TextureClassManager::GetTextureClass (const char* className)
{
  if (!className) className = "default";
  return classes.Get (className, unknownClass);
}
