//void set_dot(char *,char Color);
#pragma aux set_dot parm [esi][eax]=\
"mov [esi],al";

//void bout(short,unsigned char);
#pragma aux bout parm [dx][al]=\
"out dx,al"\

//void bout(short,short);
#pragma aux wout parm [dx][ax]=\
"out dx,ax"\

//unsigned char bin(short);
#pragma aux bin parm [dx]=\
"xor eax,eax"\
"in al,dx"\
value [eax];

//void stosb(char *adr,int lenght);
#pragma aux stos parm [edi][edx]=\
"xor eax,eax     "\
"mov ecx,edx     "\
"shr ecx,2       "\
"repne stosd     "\
"mov cl,dl       "\
"and cl,3        "\
"repne stosb     "\
modify [edi eax ecx];

//void stosb(char *adr,int lenght,int byte_);
#pragma aux stosb parm [edi][ecx][eax]=\
"repne stosb"\
modify [edi ecx];

//void move(char *dst,char *src,int lenght);
#pragma aux move parm [edi][esi][eax]=\
"mov ecx,eax"\
"shr ecx,2  "\
"repne movsd"\
"mov cl,al  "\
"and cl,3   "\
"repne movsb"\
modify [esi edi ecx];

//void movsb(char *dst,char *src,int lenght);
#pragma aux movsb parm [edi][esi][ecx]=\
"repne movsb"\
modify [esi edi ecx];

//void mover(char *dst,char *src,int num_dots);
//void mover(register int dst,register int src,register int num_dots);
#pragma aux mover parm [edi][esi][eax]=\
"mov ecx,eax"\
"shr ecx,5"\
"mov edx,3"\
"jecxz low_01f"\
"next_dot:"\
"movsb"\
"add esi,edx"\
"movsb"\
"add esi,edx"\
"movsb"\
"add esi,edx"\
"movsb"\
"add esi,edx"\
"movsb"\
"add esi,edx"\
"movsb"\
"add esi,edx"\
"movsb"\
"add esi,edx"\
"movsb"\
"add esi,edx"\
"movsb"\
"add esi,edx"\
"movsb"\
"add esi,edx"\
"movsb"\
"add esi,edx"\
"movsb"\
"add esi,edx"\
"movsb"\
"add esi,edx"\
"movsb"\
"add esi,edx"\
"movsb"\
"add esi,edx"\
"movsb"\
"add esi,edx"\
"movsb"\
"add esi,edx"\
"movsb"\
"add esi,edx"\
"movsb"\
"add esi,edx"\
"movsb"\
"add esi,edx"\
"movsb"\
"add esi,edx"\
"movsb"\
"add esi,edx"\
"movsb"\
"add esi,edx"\
"movsb"\
"add esi,edx"\
"movsb"\
"add esi,edx"\
"movsb"\
"add esi,edx"\
"movsb"\
"add esi,edx"\
"movsb"\
"add esi,edx"\
"movsb"\
"add esi,edx"\
"movsb"\
"add esi,edx"\
"movsb"\
"add esi,edx"\
"movsb"\
"add esi,edx"\
"loop next_dot"\
"low_01f:"\
"mov cl,al"\
"and cl,01fh"\
"jecxz zero"\
"next_dot1:"\
"movsb      "\
"add esi,edx"\
"loop next_dot1"\
"zero:"\
modify [eax ecx edx edi esi];

void movecol(register char *dst,register char *src,register int hiest,register int screen_width);
#pragma aux movecol parm [edi][esi][eax][edx]=\
"mov ecx,eax"\
"shr ecx,5"\
"dec edx"\
"jecxz low_01fh"\
"next_dotr:"\
"movsb"\
"add edi,edx"\
"movsb"\
"add edi,edx"\
"movsb"\
"add edi,edx"\
"movsb"\
"add edi,edx"\
"movsb"\
"add edi,edx"\
"movsb"\
"add edi,edx"\
"movsb"\
"add edi,edx"\
"movsb"\
"add edi,edx"\
"movsb"\
"add edi,edx"\
"movsb"\
"add edi,edx"\
"movsb"\
"add edi,edx"\
"movsb"\
"add edi,edx"\
"movsb"\
"add edi,edx"\
"movsb"\
"add edi,edx"\
"movsb"\
"add edi,edx"\
"movsb"\
"add edi,edx"\
"movsb"\
"add edi,edx"\
"movsb"\
"add edi,edx"\
"movsb"\
"add edi,edx"\
"movsb"\
"add edi,edx"\
"movsb"\
"add edi,edx"\
"movsb"\
"add edi,edx"\
"movsb"\
"add edi,edx"\
"movsb"\
"add edi,edx"\
"movsb"\
"add edi,edx"\
"movsb"\
"add edi,edx"\
"movsb"\
"add edi,edx"\
"movsb"\
"add edi,edx"\
"movsb"\
"add edi,edx"\
"movsb"\
"add edi,edx"\
"movsb"\
"add edi,edx"\
"movsb"\
"add edi,edx"\
"loop next_dotr"\
"low_01fh:"\
"mov cl,al"\
"and cl,01fh"\
"jecxz zero"\
"next_dotr1:"\
"movsb      "\
"add edi,edx"\
"loop next_dotr1"\
"zero:"\
modify [eax edi esi ecx edx];


