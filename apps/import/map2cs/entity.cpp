/*
    Map2cs: a convertor to convert the frequently used MAP format, into
    something, that can be directly understood by Crystal Space.

    Copyright (C) 1999 Thomas Hieber (thieber@gmx.net)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"
#include "mapstd.h"
#include "entity.h"
#include "mparser.h"
#include "brush.h"
#include "mcurve.h"

CMapKeyValuePair::CMapKeyValuePair(const char* key, const char* value)
{
  m_Key   = new char[strlen(key)+1];
  m_Value = new char[strlen(value)+1];
  strcpy(m_Key,   key);
  strcpy(m_Value, value);
}

CMapKeyValuePair::~CMapKeyValuePair()
{
  delete [] m_Key;
  delete [] m_Value;
}

CMapEntity::CMapEntity()
{
  static int EntityCount = 1;
  char Buffer[100];
  sprintf(Buffer, "e%d", EntityCount++);
  m_UniqueName = Buffer;
}

CMapEntity::~CMapEntity()
{
  DELETE_VECTOR_MEMBERS(m_Curves);
  DELETE_VECTOR_MEMBERS(m_Brushes);
  DELETE_VECTOR_MEMBERS(m_Keymap);
}

bool CMapEntity::Read(CMapParser* pParser, CMapFile* pMap)
{
  csString Buffer;
  csString Key;
  bool finished = false;
  int bufferLine, bufferLine2;

  while (!finished)
  {
    bufferLine = pParser->GetCurrentLine();
    if (!pParser->GetSafeToken(Buffer)) return false;
    bufferLine2 = pParser->GetCurrentLine();
    if (strcmp(Buffer, "{") == 0)
    {
      //This entity contains some brushes or curves!
      //We need to decide now, what to read:
      if (!pParser->PeekNextToken(Buffer)) return false;
      if (strcmp(Buffer, "(") == 0)
      {
        //Brush
        CMapBrush* pBrush = new CMapBrush(this);
        if (!pBrush->Read(pParser, pMap)) return false;
        m_Brushes.Push(pBrush);
      }
      else if (strcmp(Buffer, "patchDef2") == 0)
      {
        //Curve
        CMapCurve* pCurve = new CMapCurve;
        if (!pCurve->Read(pParser, pMap)) return false;
        m_Curves.Push(pCurve);
      }
      else
      {
        pParser->ReportError("Format error! Expected either \"(\" or \"patchDef2\""
                             ", Found\"%s\"", Buffer.GetData());
        return false;
      }
    }
    else if (strcmp(Buffer, "}") == 0)
    {
      //OK, we are done with this entity and it looks like
      //everything is ok
      finished = true;
    }
    else
    {
      //Now this seems to be a key/ value pair
      Key.Replace (Buffer);
      if ((bufferLine != bufferLine2) || !pParser->GetNextToken(Buffer))
      {
        pParser->ReportError("Format error. Keys and values for entities must"
                             "always come in pairs. Found no match for key \"%s\"",
                             Key.GetData());
        return false;
      }
      AddKeyValuePair(Key, Buffer);
    }
  }
  return true;
}

void CMapEntity::AddKeyValuePair(const char* Key, const char* Value)
{
  m_Keymap.Push(new CMapKeyValuePair(Key, Value));
}

const char* CMapEntity::GetValueOfKey (const char* key, const char* defaultvalue)
{
  size_t i, NumKeys = m_Keymap.Length();
  for (i=0; i<NumKeys; i++)
  {
    if (strcmp(key, m_Keymap[i]->GetKey())==0)
    {
      return m_Keymap[i]->GetValue();
    }
  }
  return defaultvalue;
}

double CMapEntity::GetNumValueOfKey(const char* key, double defaultvalue)
{
  const char* ValueStr = GetValueOfKey(key, 0);
  if (!ValueStr) return defaultvalue;

  char   dummy;
  double Value = 0.0;

  if (!(sscanf(ValueStr, "%lf%c", &Value, &dummy)==1))
  {
    return defaultvalue;
  }

  return Value;
}

bool CMapEntity::GetBoolValueOfKey(const char* key, bool defaultvalue)
{
  const char* ValueStr = GetValueOfKey(key, 0);
  if (!ValueStr) return defaultvalue;

  if (strcasecmp (ValueStr, "yes")   == 0) return true;
  if (strcasecmp (ValueStr, "true")  == 0) return true;
  if (strcasecmp (ValueStr, "1")     == 0) return true;
  if (strcasecmp (ValueStr, "no")    == 0) return false;
  if (strcasecmp (ValueStr, "false") == 0) return false;
  if (strcasecmp (ValueStr, "0")     == 0) return false;

  return defaultvalue;
}

bool CMapEntity::GetTripleNumValueOfKey(const char* key, double& v1, double& v2, double& v3)
{
  const char* ValueStr = GetValueOfKey(key, 0);
  if (!ValueStr) return false;

  char   dummy;
  double temp_v1, temp_v2, temp_v3;
  if (sscanf(ValueStr, "%lf %lf %lf%c", &temp_v1, &temp_v2, &temp_v3, &dummy)==3)
  {
    v1 = temp_v1;
    v2 = temp_v2;
    v3 = temp_v3;
    return true;
  }

  return false;
}

const char* CMapEntity::GetClassname()
{
  const char* classname = GetValueOfKey("classname");
  if (classname)
  {
    return classname;
  }
  else
  {
    return "";
  }
}

const char* CMapEntity::GetName()
{
  const char* name = GetValueOfKey("cs_name");
  if (name) return name;
  return m_UniqueName;
}

bool CMapEntity::GetOrigin(CdVector3& v)
{
  const char* origin = GetValueOfKey("origin");
  if (origin)
  {
    double x, y, z;
    char dummy;
    if (sscanf(origin, "%lf %lf %lf%c", &x, &y, &z, &dummy)==3)
    {
      v.x = x;
      v.y = y;
      v.z = z;
      return true;
    }
  }
  return false;
}

void CMapEntity::CreatePolygons()
{
  size_t i;
  for (i=0; i<m_Brushes.Length(); i++)
  {
    m_Brushes[i]->CreatePolygons();
  }
}

