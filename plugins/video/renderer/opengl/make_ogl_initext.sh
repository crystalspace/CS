#!/bin/sh

#
# make_ogl_initext.h
#
# small helper script to generate initialization code for the supported 
# GL extensions.
#

temp1=glexthlp/1.tmp 	
temp2=glexthlp/2.tmp 	

initextgen=glexthlp/geninitext.sh
initextsrc=ogl_initext.cpp

cat glexthlp/initexthead > ${initextsrc}

echo > ${temp1}
echo '#define USE_OGL_EXT(ext)' ${initextgen} ext ${initextsrc} >> ${temp1}
echo '#include "../ogl_suppext.h"' >> ${temp1}

cpp ${temp1} -o ${temp2}

sh ${temp2}

cat glexthlp/initextfoot >> ${initextsrc}

rm -f ${temp1}
rm -f ${temp2}
