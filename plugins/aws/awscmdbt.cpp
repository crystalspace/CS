#include "cssysdef.h"
#include "awscmdbt.h"

SCF_IMPLEMENT_IBASE(awsCmdButton)
  SCF_IMPLEMENTS_INTERFACE(awsComponent)
SCF_IMPLEMENT_IBASE_END

const unsigned int awsCmdButton::fsNormal =0x0;
const unsigned int awsCmdButton::fsToolbar=0x1;
const unsigned int awsCmdButton::fsBitmap =0x2;

awsCmdButton::awsCmdButton():is_down(false), mouse_is_over(false)
{
  tex[0]=tex[1]=tex[2]=NULL;
}

awsCmdButton::~awsCmdButton()
{
}

bool
awsCmdButton::Setup(iAws *wmgr, awsComponentNode *settings)
{
 

 return true;
}

void 
awsCmdButton::OnDraw(csRect clip)
{

}

bool 
awsCmdButton::OnMouseDown(int button, int x, int y)
{
  return false;
}
    
bool 
awsCmdButton::OnMouseUp(int button, int x, int y)
{
  return false;
}
    
bool
awsCmdButton::OnMouseMove(int button, int x, int y)
{
  return false;
}

bool
awsCmdButton::OnMouseClick(int button, int x, int y)
{
  return false;
}

bool
awsCmdButton::OnMouseDoubleClick(int button, int x, int y)
{
  return false;
}

bool 
awsCmdButton::OnMouseExit()
{
  mouse_is_over=false;
  return true;
}

bool
awsCmdButton::OnMouseEnter()
{
  mouse_is_over=true;
  return true;
}

bool
awsCmdButton::OnKeypress(int key, int modifiers)
{
  return false;
}
    
bool
awsCmdButton::OnLostFocus()
{
  return false;
}

bool 
awsCmdButton::OnGainFocus()
{
  return false;
}

/************************************* Command Button Factory ****************/
SCF_IMPLEMENT_IBASE(awsCmdButtonFactory)
  SCF_IMPLEMENTS_INTERFACE(iAwsComponentFactory)
SCF_IMPLEMENT_IBASE_END

awsCmdButtonFactory::awsCmdButtonFactory(iAws *wmgr):awsComponentFactory(wmgr)
{
  Register("Command Button");
  RegisterConstant("bfsNormal",  0x0);
  RegisterConstant("bfsToolbar", 0x1);
  RegisterConstant("bfsBitmap",  0x2);
}

awsCmdButtonFactory::~awsCmdButtonFactory()
{
 // empty
}

awsCmdButton *
awsCmdButtonFactory::Create()
{
 return new awsCmdButton; 
}

