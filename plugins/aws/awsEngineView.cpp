// awsEngineView.cpp: implementation of the awsEngineView class.
//
//////////////////////////////////////////////////////////////////////

#include "cssysdef.h"
#include "awsEngineView.h"
#include "ivaria/view.h"
#include "iengine/engine.h"
#include "iengine/camera.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

awsEngineView::awsEngineView()
  : view(NULL)
{
}

awsEngineView::~awsEngineView()
{
  if(view) view->DecRef();
}

bool awsEngineView::SetProperty (const char* name, void* parm)
{
  int i = strcmp(name, "view");
  printf("%d", i);
  if(strcmp(name, "view")==0)
  {
    if(view)
      view->DecRef();
    view = (iView*) parm;
    if(view)
      view->IncRef();
    return true;
  }
  else
    return awsComponent::SetProperty(name, parm);
}

bool awsEngineView::GetProperty (const char* name, void** parm)
{
  if(strcmp(name, "view")==0)
  {
    *parm = (void*) view;
    return true;
  }
  else
    return awsComponent::GetProperty(name, parm);
}

const char* awsEngineView::Type()
{
  return "Engine View";
}

void awsEngineView::OnDraw(csRect )
{
  if (view)
  {
    iGraphics3D* g3d = WindowManager ()->G3D ();
    iGraphics3D* og3d = view->GetContext ();
    view->SetContext (g3d);

    view->SetRectangle (
        Frame ().xmin,
        g3d->GetHeight () - Frame ().ymax,
        Frame ().Width (),
        Frame ().Height ());
    view->GetCamera ()->SetPerspectiveCenter (
        Frame ().xmin + (Frame ().Width () >> 1),
        (g3d->GetHeight () - Frame ().Height () - Frame ().ymin) +
          (Frame ().Height () >> 1));
    view->GetCamera ()->SetFOV (
        view->GetCamera ()->GetFOV (),
        Frame ().Width ());

	g3d->BeginDraw (
              view->GetEngine ()->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS);
    view->Draw ();
    g3d->BeginDraw (CSDRAW_2DGRAPHICS);
	view->SetContext(og3d);
  }
}

/* ---------------------------- Window Factory ----------------------------- */

awsEngineViewFactory::awsEngineViewFactory(iAws* wmgr) :
    awsComponentFactory(wmgr)
{
  Register ("Engine View");
}

awsEngineViewFactory::~awsEngineViewFactory()
{
}

iAwsComponent* awsEngineViewFactory::Create()
{
  return (new awsEngineView())->GetComponent();
}

