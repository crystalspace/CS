#include "cssysdef.h"
#include "iutil/string.h"
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
   for(int i=0; i<children.Length(); ++i)  
   {
     void *p=children[i];
     awsKey *key = STATIC_CAST(awsKey*,p);

     if (key && key->Name() == idname) 
       return key;
         
   }

  return NULL;
}
