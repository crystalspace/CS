#include "cssysdef.h"
#include "awskcfct.h"

awsKeyFactory::awsKeyFactory ()
{
    SCF_CONSTRUCT_IBASE(NULL);
}

awsKeyFactory::~awsKeyFactory ()
{
}

void awsKeyFactory::Initialize (const char* name, const char* component_type)
{
  awsComponentNode* n = new awsComponentNode (name, component_type);
  base = SCF_QUERY_INTERFACE(n, iAwsComponentNode);
  CS_ASSERT(base);


  // we have a ref for n and one for base we don't want the one for n though
  n->DecRef();
}

void awsKeyFactory::AddToWindowList (iAwsPrefManager *pm)
{
  if (pm && base)
  {
    // following the changes that let us instantiate non-window top level components
    // this check seems un-needed
    //if (strcmp (base->ComponentTypeName ()->GetData (), "Window") == 0)
    ((awsPrefManager *)pm)->AddWindowDef (base);
  }
}

void awsKeyFactory::AddFactory (iAwsKeyFactory *factory)
{
  if (base && factory)
  {
    base->Add (((awsKeyFactory *)factory)->base);
  }
}

void awsKeyFactory::AddIntKey (const char* name, int v)
{
  if (base)
  {
    awsIntKey* temp = new awsIntKey(name, v);
    csRef<iAwsIntKey> key (SCF_QUERY_INTERFACE(temp, iAwsIntKey));
    CS_ASSERT(key);

    base->Add (key);

    temp->DecRef();
  }
}

void awsKeyFactory::AddStringKey (const char* name, const char* v)
{
  if (base)
  {
    awsStringKey* temp = new awsStringKey(name, v);
    csRef<iAwsStringKey> key (SCF_QUERY_INTERFACE(temp, iAwsStringKey));
    CS_ASSERT(key);

    base->Add (key);

    temp->DecRef();
  }
}

void awsKeyFactory::AddRectKey (const char* name, csRect v)
{
  if (base)
  {
    awsRectKey* temp = new awsRectKey(name, v);
    csRef<iAwsRectKey> key (SCF_QUERY_INTERFACE(temp, iAwsRectKey));
    CS_ASSERT(key);

    base->Add (key);

    temp->DecRef();
  }
}

void awsKeyFactory::AddRGBKey (
  const char* name,
  unsigned char r,
  unsigned char g,
  unsigned char b)
{
  if (base)
  {
    awsRGBKey* temp = new awsRGBKey(name, r, g, b);
    csRef<iAwsRGBKey> key (SCF_QUERY_INTERFACE(temp, iAwsRGBKey));
    CS_ASSERT(key);

    base->Add (key);

    temp->DecRef();
  }
}

void awsKeyFactory::AddPointKey (const char* name, csPoint v)
{
  if (base)
  {
    awsPointKey* temp = new awsPointKey(name, v);
    csRef<iAwsPointKey> key (SCF_QUERY_INTERFACE(temp, iAwsPointKey));
    CS_ASSERT(key);

    base->Add (key);

    temp->DecRef();
  }
}

void awsKeyFactory::AddConnectionKey (
  const char* name,
  iAwsSink *s,
  unsigned long t,
  unsigned long sig)
{
  if (base)
  {
    awsConnectionKey* temp = new awsConnectionKey(name, s, t, sig);
    csRef<iAwsConnectionKey> key (SCF_QUERY_INTERFACE(temp, iAwsConnectionKey));
    CS_ASSERT(key);

    base->Add (key);

    temp->DecRef();
  }
}

void awsKeyFactory::AddConnectionNode (iAwsConnectionNodeFactory *node)
{
  if(base && node && node->GetThisNode())
  {
    base->Add ((awsKey*)node->GetThisNode());
    ((awsConnectionNodeFactory *)node)->base_in_use = true;
  }
}

iAwsComponentNode *awsKeyFactory::GetThisNode ()
{
  return base;
}

awsConnectionNodeFactory::awsConnectionNodeFactory ()
{
  base=0;
  base_in_use=false;
}

awsConnectionNodeFactory::~awsConnectionNodeFactory ()
{
  // THIS LEAKS!! Key containers do not yet clean up after themselves!
  if (base && !base_in_use) delete base;
}

void awsConnectionNodeFactory::Initialize()
{
  base=new awsConnectionNode();
}

void awsConnectionNodeFactory::AddConnectionKey (
  const char* name,
  iAwsSink *s,
  unsigned long t,
  unsigned long sig)
{
  if (base)
  {
    base->Add ((awsKey*)(new awsConnectionKey (name, s, t, sig)));
  }
}

awsConnectionNode *awsConnectionNodeFactory::GetThisNode ()
{
  return base;
}

