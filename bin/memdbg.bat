@echo off

  rem This is a simplistic batch file that will create the memdbg.map file
  rem required by the memory debugger. It is not memdbg.sh counterpart, its
  rem rather a last resort if you dont have a Unix-like shell (or can't
  rem get djgpp's bash to run memdbg.sh, like me :-)

  if /%1/ == // goto help

  echo %2%3%4%5%6%7%8%9 >memdbg.map
  objdump --debugging %1 | awk -f bin/memdbg-d.awk >>memdbg.map

:help

  echo Usage: memdbg.bat [executable] {aslbLfdv}
  echo For a full description of options, see memdbg.sh
