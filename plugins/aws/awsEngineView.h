// awsEngineView.h: interface for the awsEngineView class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AWSENGINEVIEW_H__6451E6E6_A559_4018_B48F_EBF7ECD35B84__INCLUDED_)
#define AFX_AWSENGINEVIEW_H__6451E6E6_A559_4018_B48F_EBF7ECD35B84__INCLUDED_

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

#endif // !defined(AFX_AWSENGINEVIEW_H__6451E6E6_A559_4018_B48F_EBF7ECD35B84__INCLUDED_)
