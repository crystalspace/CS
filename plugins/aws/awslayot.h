#ifndef __CS_AWS_LAYOUT_H__
#define __CS_AWS_LAYOUT_H__

# include "iaws/aws.h"
# include "csutil/scanstr.h"
# include "csgeom/csrect.h"

/// Base class for layouts
class awsLayoutManager : public iAwsLayoutManager
{
protected:
  iAwsComponent *owner;
  iAwsPrefManager* pm;
public:
  awsLayoutManager (iAwsComponent *_owner, iAwsComponentNode* , iAwsPrefManager* _pm)
    : owner(_owner), pm(_pm)
  { 
    SCF_CONSTRUCT_IBASE(NULL);
  }
  virtual ~awsLayoutManager ()
  { }

  SCF_DECLARE_IBASE;

  /**  Sets the owner.  Normally the owner should never change, but in some rare
    * cases (like in the Window class) the owner is set improperly by the setup
    * code and must be fixed by the embedder.  This should ALWAYS be used by widgets
    * which embed the component and use delegate wrappers (i.e. awsecomponent)
    */
  virtual void SetOwner (iAwsComponent *_owner) { owner = _owner; }

  /** Adds a component to the layout, returning it's actual rect. 
    */
  virtual csRect AddComponent (iAwsComponent *cmp, iAwsComponentNode* settings) = 0;

  /// Removes a component from the layout
  virtual void RemoveComponent(iAwsComponent* )
  { }

  /// Lays out components properly
  virtual void LayoutComponents () = 0;
};
#endif // __CS_AWS_LAYOUT_H__
