#include "cssysdef.h"
#include "aws/awsfparm.h"
#include "aws/awsadler.h"

#include <string.h>

const int awsParmList::INT=0;    
const int awsParmList::FLOAT=1;
const int awsParmList::STRING=2;   
const int awsParmList::BASICVECTOR=3;
const int awsParmList::STRINGVECTOR=4;
const int awsParmList::RECT=5;
const int awsParmList::POINT=6;
const int awsParmList::BOOL=7;

static unsigned long
NameToID(char *name)
{
 return aws_adler32(aws_adler32(0, NULL, 0), (unsigned char *)name, strlen(name));
}

awsParmList::parmItem *
awsParmList::FindParm(char *_name, int type)
{
  unsigned long name = NameToID(_name);
  int i;

  for(i=0; i<parms.Length(); ++i)
  {
    parmItem *item = (parmItem *)parms[i];
    
    if (item->name == name && item->type==type)
      return item;
  }

  return NULL;
}

void 
awsParmList::AddInt(char *name, int value)
{
  parmItem *pi = new parmItem;

  pi->name = NameToID(name);
  pi->type = INT;
  pi->parm.i = value;

  parms.Push(pi);
}

void 
awsParmList::AddFloat(char *name, float value)
{
  parmItem *pi = new parmItem;

  pi->name = NameToID(name);
  pi->type = FLOAT;
  pi->parm.f = value;

  parms.Push(pi);
}

void 
awsParmList::AddBool(char *name, bool value)
{
  parmItem *pi = new parmItem;

  pi->name = NameToID(name);
  pi->type = BOOL;
  pi->parm.b = value;

  parms.Push(pi);
}

void 
awsParmList::AddString(char *name, iString* value)
{
  parmItem *pi = new parmItem;

  pi->name = NameToID(name);
  pi->type = STRING;
  pi->parm.s = value;
  
  parms.Push(pi);
}

void 
awsParmList::AddBasicVector(char *name, csBasicVector* value)
{
  parmItem *pi = new parmItem;

  pi->name = NameToID(name);
  pi->type = BASICVECTOR;
  pi->parm.bv = value;

  parms.Push(pi);
}

void 
awsParmList::AddStringVector(char *name, csStrVector* value)
{
  parmItem *pi = new parmItem;

  pi->name = NameToID(name);
  pi->type = STRINGVECTOR;
  pi->parm.sv = value;

  parms.Push(pi);
}

void 
awsParmList::AddRect(char *name, csRect *value)
{
  parmItem *pi = new parmItem;

  pi->name = NameToID(name);
  pi->type = RECT;
  pi->parm.r = value;

  parms.Push(pi);
}

void 
awsParmList::AddPoint(char *name, csPoint *value)
{
  parmItem *pi = new parmItem;

  pi->name = NameToID(name);
  pi->type = POINT;
  pi->parm.p = value;

  parms.Push(pi);
}

bool 
awsParmList::GetInt(char *name, int *value)
{
  parmItem *pi=FindParm(name, INT);

  if (pi) 
  {
    *value = pi->parm.i;
    return true;
  }

  return false;
}

bool
awsParmList::GetFloat(char *name, float *value)
{
  parmItem *pi=FindParm(name, FLOAT);

  if (pi) 
  {
    *value = pi->parm.f;
    return true;
  }

  return false;
}

bool
awsParmList::GetBool(char *name, bool *value)
{
  parmItem *pi=FindParm(name, BOOL);

  if (pi) 
  {
    *value = pi->parm.b;
    return true;
  }

  return false;
}
  
bool 
awsParmList::GetString(char *name, iString **value)
{
  parmItem *pi=FindParm(name, STRING);

  if (pi) 
  {
    *value = pi->parm.s;
    return true;
  }

  return false;
}

  
bool 
awsParmList::GetBasicVector(char *name, csBasicVector **value)
{
  parmItem *pi=FindParm(name, BASICVECTOR);

  if (pi) 
  {
    *value = pi->parm.bv;
    return true;
  }

  return false;
}
  
bool 
awsParmList::GetStringVector(char *name, csStrVector **value)
{
  parmItem *pi=FindParm(name, STRINGVECTOR);

  if (pi) 
  {
    *value = pi->parm.sv;
    return true;
  }

  return false;
}
  
bool 
awsParmList::GetRect(char *name, csRect **value)
{
  parmItem *pi=FindParm(name, RECT);

  if (pi) 
  {
    *value = pi->parm.r;
    return true;
  }

  return false;
}
  
bool 
awsParmList::GetPoint(char *name, csPoint **value)
{
  parmItem *pi=FindParm(name, FLOAT);

  if (pi) 
  {
    *value = pi->parm.p;
    return true;
  }

  return false;
}