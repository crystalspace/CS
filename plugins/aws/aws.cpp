#include "cssysdef.h"
#include "aws.h"
#include "awsprefs.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (awsPrefManager)
  SCF_IMPLEMENTS_INTERFACE (iAwsPrefs)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (awsManager)
  SCF_IMPLEMENTS_INTERFACE (iAws)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (awsManager::eiPlugin)                                                               
   SCF_IMPLEMENTS_INTERFACE (iPlugin)                                                                              
SCF_IMPLEMENT_EMBEDDED_IBASE_END                                                                                  
                                                                                                               
SCF_IMPLEMENT_FACTORY (awsManager)                                                                                
SCF_IMPLEMENT_FACTORY (awsPrefManager)                                                                                

SCF_EXPORT_CLASS_TABLE (aws)                                                                                      
  SCF_EXPORT_CLASS (awsManager, "crystalspace.window.alternatemanager", "Crystal Space alternate window manager") 
  SCF_EXPORT_CLASS (awsPrefManager, "crystalspace.window.preferencemanager", "Crystal Space window preference manager") 
SCF_EXPORT_CLASS_TABLE_END                                                                                        
