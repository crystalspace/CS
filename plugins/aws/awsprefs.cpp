#include "cssysdef.h"
#include "awsprefs.h"
#include <stdio.h>

extern int awsparse();
extern FILE *awsin;

awsPrefManager::awsPrefManager(iBase *iParent)
{
  CONSTRUCT_IBASE (iParent);
}

awsPrefManager::~awsPrefManager()
{
}

void 
awsPrefManager::Load(const char *def_file)
{
  printf("\tloading definitions file %s...\n", def_file);

  awsin = fopen( def_file, "r" );

  if(awsparse())
      printf("\tsyntax error in definition file, load failed.\n");
  else
      printf("\tload successful.\n");

  fclose(awsin);

}
