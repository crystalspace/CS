/*
    Copyright (C) 2000 by Jerry A. Segler, Jr.
    Based on csFont
    Copyright (C) 2000 by Norman Kramer
    original unplugged code and fonts by Andrew Zabolotny

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

#include <stdlib.h>
#include "cssysdef.h"
#include "isystem.h"
#include "csutil/csvector.h"
#include "fontplex.h"

IMPLEMENT_IBASE (csDefaultFontManager)
  IMPLEMENTS_INTERFACE (iFontServer)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csDefaultFontManager)

EXPORT_CLASS_TABLE (fontplex)
  EXPORT_CLASS (csDefaultFontManager, "crystalspace.font.manager", 
		"Crystal Space font server multiplexor" )
EXPORT_CLASS_TABLE_END

csDefaultFontManager::csDefaultFontManager (iBase *pParent)
{
  CONSTRUCT_IBASE (pParent);
}

bool csDefaultFontManager::Initialize (iSystem* SysDriver)
{
  FontList = new csVectorFonts();
  FontServers = (iFontServer **) malloc(sizeof(iFontServer *) * 10);
  FontServerCount=0;

  FontServers[FontServerCount] = LOAD_PLUGIN(SysDriver,"crystalspace.font.server.csfont","csfont",iFontServer);
  if (FontServers[FontServerCount] != NULL) FontServerCount++;

  FontServers[FontServerCount] = LOAD_PLUGIN(SysDriver,"crystalspace.font.server.freetype","freefont",iFontServer);
  if (FontServers[FontServerCount] != NULL) FontServerCount++;

  for(int i=0;i<FontServerCount;i++)
  {
    iFontServer * pServer = FontServers[i];
    int j = pServer->GetFontCount();
    for(int k=0;k<j;k++)
    {
      csManagedFontDef * font = new csManagedFontDef();
      font->pFontServer = pServer;
      font->FontID=k;
      FontList->Push(font);
    }
  }

  return true;
}

csDefaultFontManager::~csDefaultFontManager()
{
  for(int i=0;i<FontServerCount;i++)
  {
    iFontServer * pServer = FontServers[i];
    pServer->DecRef();
  }
  delete FontList;
}

int csDefaultFontManager::LoadFont (const char* name, const char* filename)
{
  for(int i=0;i<FontServerCount;i++)
  {
    iFontServer * pServer = FontServers[i];
    int FontID = pServer->LoadFont(name,filename);
    if (FontID != -1)
    {
      csManagedFontDef * font = new csManagedFontDef();
      font->pFontServer = pServer;
      font->FontID = FontID;
      int fontI = FontList->Find(font);
      if (fontI != -1)
      {
        delete font;
        return fontI;
      }
      else
      {
        return FontList->Push(font);
      }
    }
  }
  return -1;
}

bool csDefaultFontManager::SetFontProperty (int fontId,
  CS_FONTPROPERTY propertyId, long& property, bool autoApply)
{
  csManagedFontDef * font = (csManagedFontDef *) FontList->Get(fontId);
  if (font != NULL)
    return font->pFontServer->SetFontProperty(font->FontID,propertyId,property,autoApply);
  return false;
}

bool csDefaultFontManager::GetFontProperty (int fontId,
  CS_FONTPROPERTY propertyId, long& property)
{
  csManagedFontDef * font = (csManagedFontDef *) FontList->Get(fontId);
  if (font != NULL)
    return font->pFontServer->GetFontProperty(font->FontID,propertyId,property);
  return false;
}

unsigned char *csDefaultFontManager::GetGlyphBitmap (int fontId, unsigned char c,
  int &oW, int &oH)
{ 
  csManagedFontDef * font = (csManagedFontDef *) FontList->Get(fontId);
  if (font != NULL)
    return font->pFontServer->GetGlyphBitmap(font->FontID,c,oW,oH);
  return NULL;
}

bool csDefaultFontManager::GetGlyphSize (int fontId, unsigned char c, int &oW, int &oH)
{
  csManagedFontDef * font = (csManagedFontDef *) FontList->Get(fontId);
  if (font != NULL)
    return font->pFontServer->GetGlyphSize(font->FontID,c,oW,oH);
  return false;
}

int csDefaultFontManager::GetMaximumHeight (int fontId)
{
  csManagedFontDef * font = (csManagedFontDef *) FontList->Get(fontId);
  if (font != NULL)
    return font->pFontServer->GetMaximumHeight(font->FontID);
  return -1;
}

void csDefaultFontManager::GetTextDimensions (int fontId, const char* text,
  int& width, int& height)
{
  csManagedFontDef * font = (csManagedFontDef *) FontList->Get(fontId);
  if (font != NULL)
    font->pFontServer->GetTextDimensions(font->FontID,text,width,height);
}
