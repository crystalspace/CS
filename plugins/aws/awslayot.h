#ifndef __AWS_LAYOUT__
#define __AWS_LAYOUT__

#include "iaws/aws.h"
#include "csutil/scanstr.h"
#include "csgeom/csrect.h"


/// Base class for layouts
class awsLayoutManager
{


public:
	awsLayoutManager() {};
	virtual ~awsLayoutManager() {};

	/** Adds a component to the layout, returning it's actual rect.  This function needs
	 *  to get some more info from the key node, so that is passed in as well as the
	 *  preference manager to decode the information.
	 */
	virtual csRect AddComponent(iAwsPrefManager *pm, awsComponentNode *settings, iAwsComponent *cmp)=0;
};




#endif