#include "cssysdef.h"
#include "iutil/string.h"
#include "csutil/scfstr.h"
#include "csgeom/csrect.h"
#include "csutil/csdllist.h"
#include "ivaria/aws.h"
#include "awsprefs.h"


extern unsigned long aws_adler32(unsigned long adler,  const unsigned char *buf,  unsigned int len);


awsKey *
awsKeyContainer::Find(iString *n)
{
 return Find(aws_adler32(aws_adler32(0, NULL, 0), (unsigned char *)n->GetData(), n->Length()));
}

awsKey *
awsKeyContainer::Find(unsigned long idname)
{
   if (aws_debug)
     printf("aws-debug: searching for %lu (%i items)\n", idname, children.Length());
   
   int i;
   for(i=0; i<children.Length(); ++i)  
   {
              
       awsKey *key = STATIC_CAST(awsKey*,children[i]);
	
       if (aws_debug)
         printf("aws-debug: item %d=%lu ? %lu\n", i, key->Name(), idname);
       
       if (key && key->Name() == idname) 
         return key;
     
   }
  
  if (aws_debug)
    printf("aws-debug: search failed.\n");
    
  return NULL;
}

void 
awsKeyContainer::Consume(awsKeyContainer *c)
  {
     if (aws_debug)
       printf("aws-debug: Consuming %d items (%d items currently).\n", c->children.Length(), children.Length());
     
     int i;
     for(i=0; i<c->children.Length(); ++i) 
     {
        void *p = c->children[i];
	
        children.Push(p);
     }
     
     c->children.SetLength(0);
     
     if (aws_debug)
       printf("aws-debug: Now contains %d items.\n", children.Length());
     
     //Do NOT delete awsKeyContainer!  This is NOT a memory leak!  The caller is
     //  responsible for cleaning up the container!
  }



// Connection node ///////////////////////////////////////////////////////////////////////////////
awsConnectionNode::awsConnectionNode():awsKey(new scfString("Connect"))
{}

awsConnectionNode::~awsConnectionNode()
{};



