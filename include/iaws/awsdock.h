
#include "csutil/scf.h"


struct iAwsDockSite;
struct iAwsDockableWindow;

SCF_VERSION (iAwsDockSite, 0, 0, 1);

	/// Dock site types, used to determine which bars
	/// can dock in which places
	const int AWS_DOCK_EAST = 0x1;
	const int AWS_DOCK_WEST = 0x2;
	const int AWS_DOCK_NORTH = 0x4;
	const int AWS_DOCK_SOUTH = 0x8;
	const int AWS_DOCK_HORZ = AWS_DOCK_SOUTH | AWS_DOCK_NORTH;
	const int AWS_DOCK_VERT = AWS_DOCK_EAST | AWS_DOCK_WEST;
	const int AWS_DOCK_ALL = AWS_DOCK_VERT | AWS_DOCK_HORZ;


struct iAwsDockSite : iBase
{
public:

	virtual int GetType() =0;

	/// Insert a window into the dock site
	virtual void AddDockWindow(iAwsDockableWindow* win)=0;

	/// Remove a window from a dock site
	virtual void RemoveDockWindow(iAwsDockableWindow* win)=0;

	/// Gets the docked window frame where the dockable window
	/// should draw itself
	virtual csRect GetDockedWindowFrame(iAwsDockableWindow* win)=0;
};
	
SCF_VERSION (iAwsDockableWindow, 0, 0, 1);

struct iAwsDockableWindow : iBase
{
public:
	/// Returns the current dock site, or NULL if the window is floating
	virtual iAwsDockSite* GetDockSite()=0;

};