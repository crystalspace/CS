#include "cssysdef.h"
#include "iaws/aws.h"
#include "awsscr.h"
#include "awsfparm.h"
#include "awsadler.h"

#include <string.h>

void awsActionDispatcher::Register (const char *name,
  void (Action) (void *owner, iAwsParmList* parmlist))
{
  awsActionMap *map = new awsActionMap ();

  map->name = aws_adler32 (
      aws_adler32 (0, 0, 0),
      (unsigned char *)name,
      strlen (name));
  map->Action = Action;

  actions.Push (map);
}

void awsActionDispatcher::Execute (
  const char *action,
  void *owner,
  iAwsParmList* parmlist)
{
  unsigned long name = aws_adler32 (
      aws_adler32 (0, 0, 0),
      (unsigned char *)action,
      strlen (action));

  size_t i;
  for (i = 0; i < actions.Length (); ++i)
  {
    awsActionMap *map = actions[i];

    if (name == map->name) map->Action (owner, parmlist);
  }
}

