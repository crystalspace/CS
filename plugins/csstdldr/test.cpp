#include "sysdef.h"
#include "stdldr.h"
#include <malloc.h>
#include <stdio.h>

int main ()
{
  csStandardLoader ldr (NULL);

  FILE *f = fopen ("plugins/csstdldr/test/world.test", "rb");
  if (!f)
  {
    fprintf (stderr, "cant open input file\n");
    return -1;
  }
  fseek (f, 0, SEEK_END);
  size_t size = ftell (f);
  fseek (f, 0, SEEK_SET);
  char *data = (char *)malloc (size);
  fread (data, size, 1, f);
  fclose (f);

  printf ("success: %d\n", ldr.Parse (data));

  return 0;
}
