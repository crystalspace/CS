/* REXX */

  call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
  call SysLoadFuncs

  parse arg fromfile tofile;

  if (tofile = '') then
  do
    say 'Usage: os2upd <srcfile> <dstfile>';
    exit 1;
  end;

  lcount = 0;
  fileseq = 1;
  do while lines(fromfile)
    lines.lcount = linein(fromfile);
    if (lines.lcount \= linein(tofile)) then
      fileseq = 0;
    lcount = lcount + 1;
  end

  call stream fromfile, 'C', 'CLOSE';
  call stream tofile, 'C', 'CLOSE';

  call SysFileDelete fromfile
  if (fileseq = 0) then
  do
    call SysFileDelete tofile
    do i = 0 to lcount - 1
      call lineout tofile, lines.i
    end;
  end;

  call stream tofile, 'C', 'CLOSE';

exit 0
