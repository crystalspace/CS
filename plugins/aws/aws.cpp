#include "cssysdef.h"
#include "aws.h"
#include "awsprefs.h"

IMPLEMENT_IBASE (awsPrefManager)
  IMPLEMENTS_INTERFACE (iAwsPrefs)
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (awsManager)
  IMPLEMENTS_INTERFACE (iAws)
  IMPLEMENTS_EMBEDDED_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_EMBEDDED_IBASE (awsManager::eiPlugIn)                                                               
   IMPLEMENTS_INTERFACE (iPlugIn)                                                                              
IMPLEMENT_EMBEDDED_IBASE_END                                                                                  
                                                                                                               
IMPLEMENT_FACTORY (awsManager)                                                                                
IMPLEMENT_FACTORY (awsPrefManager)                                                                                

EXPORT_CLASS_TABLE (aws)                                                                                      
  EXPORT_CLASS (awsManager, "crystalspace.window.alternatemanager", "Crystal Space alternate window manager") 
  EXPORT_CLASS (awsPrefManager, "crystalspace.window.preferencemanager", "Crystal Space window preference manager") 
EXPORT_CLASS_TABLE_END                                                                                        
