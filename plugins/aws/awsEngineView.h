// awsEngineView.h: interface for the awsEngineView class.
//
//////////////////////////////////////////////////////////////////////
#ifndef __AWSENGINEVIEW_H__
#define __AWSENGINEVIEW_H__

#include "awscomp.h"

struct iView;

class awsEngineView : public awsComponent  
{
public:
	awsEngineView();
	virtual ~awsEngineView();

	virtual void OnDraw(csRect clip);

	/// Gets properties
    bool GetProperty (char *name, void **parm);

    /// Sets properties
    bool SetProperty (char *name, void *parm);

    /// Returns the named TYPE of the component, like "Radio Button", etc.
    virtual char *Type ();

private:
	iView* view;

};

class awsEngineViewFactory : public awsComponentFactory
{
public:
	awsEngineViewFactory(iAws* mgr);
	~awsEngineViewFactory();

	iAwsComponent* Create();
};

#endif

