@echo off

  rem ** Usage: win32link deffile "description" [exename] ...

  echo DESCRIPTION %2>%1
  if /%3/ == // goto exports

exit

:exports
  echo EXPORTS>>%1
  echo 	DllMain			PRIVATE>>%1
  echo 	DllCanUnloadNow		PRIVATE>>%1
  echo 	DllGetClassObject	PRIVATE>>%1
  echo 	DllRegisterServer	PRIVATE>>%1
  echo 	DllUnregisterServer	PRIVATE>>%1
exit
