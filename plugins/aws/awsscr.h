#ifndef __AWS_SCRIPT_UTILITY__
# define __AWS_SCRIPT_UTILITY__

# include "csutil/csvector.h"

struct awsActionMap
{
  /// The name of the action reduced to a long
  unsigned long name;

  /// The action to execute
  void (*Action) (void *owner, iAwsParmList &parmlist);
};

class awsActionDispatcher
{
  /// List of actions to execute
  csBasicVector actions;
public:
  /// Register an action.
  void Register (
        char *name,
        void (Action) (void *owner, iAwsParmList &parmlist));

  /// Execute the corresponding action
  void Execute (char *action, void *owner, iAwsParmList &parmlist);
};
#endif
