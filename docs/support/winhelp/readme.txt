Creating the documentation in HTML Help format:
===============================================

required: HTML Help Workshop 1.3 (available at www.microsoft.com), 
	  Perl (www.activestate.com, for example).

1. do a 'make htmldoc' and 'make pubapi'. Or, if you're lazy, copy the 
   directories docs/html and docs/pubapi to out/docs/html and out/docs/pubapi.
   
2. run 'make chmsupp'.

3. in the directory out/docs you can find two html help project files, 
   csapi.hhp and csdocs.hhp. Compile those with HTML Help Workshop.
   
4. Enjoy.