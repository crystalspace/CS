#include "partedit2_app.h"

/** Callback for particle fountain events like Start/Stop, etc. */
static void onParticleFountainEvent(iAws2ScriptObject *info)
{
	switch(info->GetIntArg(0))
	{
		default:
			pe2App()->ReportError("Unknown particle fountain command received!");	
	}
}


void initializeScriptEvents(csRef<iAws2> aws)
{
	iAws2ScriptObject *so;
		
	so = aws->CreateScriptObject("particleFountain", onParticleFountainEvent);
	
}