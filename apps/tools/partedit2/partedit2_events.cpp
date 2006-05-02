#include "partedit2_app.h"


/** Callback for particle fountain events like Start/Stop, etc. */
BEGIN_AWS2_EVENT(onParticleFountainEvent)
{
	switch(info->GetIntArg(0))
	{
		default:
			pe2App()->ReportError("Unknown particle fountain command received!");	
	}
}	
END_AWS2_EVENT


void 
PartEdit2::initializeScriptEvents()
{
	iAws2ScriptObject *so;
		
	so = aws->CreateScriptObject("particleFountain", new onParticleFountainEvent);
	
}