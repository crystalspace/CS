#ifndef __AWSTEST_H__
#define __AWSTEST_H__

#include <stdarg.h>
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "iaws/aws2.h"

struct iEngine;
struct iSector;
struct iView;
struct iFont;
struct iFile;
struct iImageLoader;
struct iLoaderPlugin;
struct iMeshWrapper;
struct iLoader;
struct iConsoleOutput;
struct iObjectRegistry;
struct iVirtualClock;
struct iEvent;
struct iGraphics3D;
struct iGraphics2D;
struct iVFS;
class csTransform;

class awsTest
{
public:
  iObjectRegistry* object_reg;

private:
  csRef<iEngine> engine;
  csRef<iAws> aws;

  //csRef<iAwsCanvas> awsCanvas;
  iSector* room;
  csRef<iView> view, wview;
  csRef<iGraphics3D> myG3D;
  csRef<iGraphics2D> myG2D;
  csRef<iVFS> myVFS;
  csRef<iConsoleOutput> myConsole;
  csRef<iLoader> loader;
  csRef<iVirtualClock> vc;

  csRef<iFont> font;
  int col_red, col_blue, col_white, col_black;
  int col_yellow, col_cyan, col_green, col_gray;
  csString message;
  csTicks message_timer;
  bool message_error;


public:
  awsTest();
  virtual ~awsTest();

  bool Initialize(int argc, const char* const argv[], const char *iConfigName);

  void Report (int severity, const char* msg, ...);

  void SetupFrame();
  void FinishFrame();
  bool HandleEvent (iEvent &Event);
};

#endif // __AWSTEST_H__
