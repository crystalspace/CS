#include "cssysdef.h"
#include "awskcfct.h"

#define KC(x) ((awsKeyContainer *)x)
        
awsKeyFactory::awsKeyFactory():base(0), base_in_use(false)
{
}

awsKeyFactory::~awsKeyFactory()
{
  // THIS LEAKS!! Key containers do not yet clean up after themselves!
  if (base && !base_in_use)
    delete base;
}

void 
awsKeyFactory::Initialize(iString *name, iString *component_type)
{              
  base = new awsComponentNode(name,component_type);
}

void 
awsKeyFactory::AddToWindowList(iAwsPrefManager *pm)
{
  if (pm && base)
  {
    if (strcmp(base->ComponentTypeName()->GetData(), "Window")==0)
    ((awsPrefManager *)pm)->AddWindowDef(base);
  }
}

void
awsKeyFactory::AddFactory(iAwsKeyFactory *factory)
{
  if (base && factory)
  {
    KC(base)->Add(((awsKeyFactory *)factory)->base);
    ((awsKeyFactory *)factory)->base_in_use=true;
  }
}

void 
awsKeyFactory::AddIntKey(iString *name, int v)
{
  if (base)
  {
    KC(base)->Add(new awsIntKey(name,v));
  }
} 

void 
awsKeyFactory::AddStringKey(iString *name, iString *v)
{
  if (base)
  {
    KC(base)->Add(new awsStringKey(name,v));
  }
}

void
awsKeyFactory::AddRectKey(iString *name, csRect v)
{
  if (base)
  {
    KC(base)->Add(new awsRectKey(name,v));
  }
}

void 
awsKeyFactory::AddRGBKey(iString *name, unsigned char r, unsigned char g, unsigned char b)
{
  if (base)
  {
    KC(base)->Add(new awsRGBKey(name, r, g, b));
  }
}

void 
awsKeyFactory::AddPointKey(iString *name, csPoint v)
{
  if (base)
  {
    KC(base)->Add(new awsPointKey(name,v));
  }
}

void 
awsKeyFactory::AddConnectionKey(iString *name, iAwsSink *s, unsigned long t, unsigned long sig)
{
  if (base)
  {
    KC(base)->Add(new awsConnectionKey(name,s,t,sig));
  }
}

