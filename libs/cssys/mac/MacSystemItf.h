
#include "csutil/scf.h"

extern const IID IID_IMacSystemDriver;

interface IMacSystemDriver : public iBase
{
	/// Returns the HINSTANCE of the program
	STDMETHOD(GetInstance)(HINSTANCE* retval) = 0;
	/// Returns S_OK if the program is 'active', S_FALSE otherwise.
	STDMETHOD(GetIsActive)() = 0;
	/// Gets the nCmdShow of the WinMain().
	STDMETHOD(GetCmdShow)(int* retval) = 0;
};
