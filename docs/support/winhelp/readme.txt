Creating the documentation in MS compressed HTML Help format
============================================================

Required: HTML Help Workshop 1.3 (available at www.microsoft.com), 
	  Perl (www.activestate.com, for example).

1. Invoke 'make manualhtml' and 'make apihtml'. Or, if you're lazy, copy the 
   directories docs/html/manual and docs/html/api to out/docs/html/manual and
   out/docs/manual/api.
   
2. Invoke 'make manualchm apichm'.

3. In the directory out/docs you can find two HTML help project files, 
   csapi.hhp and csmanual.hhp. Compile those with HTML Help Workshop.
   
4. Enjoy.
