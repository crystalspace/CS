
#include "csutil/scf.h"

#define WINDOWCLASSNAME "Crystal"

SCF_INTERFACE(iWin32SystemDriver, 0, 0, 1) : public iBase
{
	/// Returns the HINSTANCE of the program
	STDMETHOD(GetInstance)(HINSTANCE* retval) = 0;
	/// Returns S_OK if the program is 'active', S_FALSE otherwise.
	STDMETHOD(GetIsActive)() = 0;
	/// Gets the nCmdShow of the WinMain().
	STDMETHOD(GetCmdShow)(int* retval) = 0;
};
