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
   void *p = children.GetFirstItem();

   while(p) 
   {
     awsKey *key = STATIC_CAST(awsKey*,p);

     if (key) 
     {
       if (key->Name() == idname)
       {
          return key;
       }
     }

     p = children.GetNextItem();

   } // end while traversing the children

  return NULL;
}
