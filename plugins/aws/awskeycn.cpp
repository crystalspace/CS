#include "cssysdef.h"
#include "iutil/string.h"
#include "csgeom/csrect.h"
#include "csutil/csdllist.h"
#include "ivaria/aws.h"
#include "awsprefs.h"

awsKey *
awsKeyContainer::Find(iString *name)
{
   void *p = children.GetFirstItem();

   while(p) 
   {
     awsKey *key = STATIC_CAST(awsKey*,p);

     if (key) 
     {
       if (key->Name() == name)
       {
          return key;
       }
     }

     p = children.GetNextItem();

   } // end while traversing the children

  return NULL;
}
