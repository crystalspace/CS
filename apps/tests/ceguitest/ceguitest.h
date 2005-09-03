#ifndef __CEGUITEST_H__
#define __CEGUITEST_H__

#include <crystalspace.h>
#include "CEGUI.h"
#include "ivaria/cegui.h"

struct iSector;

class CEGUITest : public csApplicationFramework, public csBaseEventHandler
{
private:
  iSector *room;
  float rotX, rotY;

  csRef<iEngine> engine;
  csRef<iLoader> loader;
  csRef<iGraphics3D> g3d;
  csRef<iKeyboardDriver> kbd;
  csRef<iVirtualClock> vc;
  csRef<iView> view;
  csRef<iVFS> vfs;
  csRef<iCEGUI> cegui;

  void ProcessFrame();
  void FinishFrame();

  bool OnKeyboard(iEvent&);
  void CreateRoom(); 

public:
  CEGUITest();
  ~CEGUITest();

  void OnExit();
  bool OnInitialize(int argc, char* argv[]);

  bool Application();
};

#endif // __CEGUITEST_H__
