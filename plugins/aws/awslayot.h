#ifndef __AWS_LAYOUT__
# define __AWS_LAYOUT__

# include "iaws/aws.h"
# include "csutil/scanstr.h"
# include "csgeom/csrect.h"

/// Base class for layouts
class awsLayoutManager
{
protected:
  iAwsComponent *owner;
public:
  awsLayoutManager (iAwsComponent *_owner) :
  owner(_owner)
  {
  };
  virtual ~awsLayoutManager ()  { };

  /**  Sets the owner.  Normally the owner should never change, but in some rare
    * cases (like in the Window class) the owner is set improperly by the setup
    * code and must be fixed by the embedder.  This should ALWAYS be used by widgets
    * which embed the component and use delegate wrappers (i.e. awsecomponent)
    */
  virtual void SetOwner (iAwsComponent *_owner) { owner = _owner; }

  /** Adds a component to the layout, returning it's actual rect.  This function needs
    *  to get some more info from the key node, so that is passed in as well as the
    *  preference manager to decode the information.
    */
  virtual csRect AddComponent (
                  iAwsPrefManager *pm,
                  awsComponentNode *settings,
                  iAwsComponent *cmp) = 0;

  /// Lays out components properly
  virtual void LayoutComponents () = 0;
};
#endif
