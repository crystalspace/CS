// Depending on the Colorformat, we define different masks and shiftvalues
// to get the accrding values for rgb; R,G,B stands for Red, Green and Blue,
// and M, S, B stands for Mask, Shift and Bits correspondingly.
#undef PI_PIXTYPE
#undef PI_RM
#undef PI_RS
#undef PI_RB
#undef PI_GM
#undef PI_GS
#undef PI_GB
#undef PI_BM
#undef PI_BS
#undef PI_BB

#if defined (PI_R5G5B5)
#  define PI_PIXTYPE	UShort
#  define PI_RM		0x00007C00
#  define PI_RS		10
#  define PI_RB		5
#  define PI_GM		0x000003E0
#  define PI_GS		5
#  define PI_GB		5
#  define PI_BM		0x0000001F
#  define PI_BS		0
#  define PI_BB		5
#elif defined (PI_R5G6B5)
#  define PI_PIXTYPE	UShort
#  define PI_RM		0x0000F800
#  define PI_RS		11
#  define PI_RB		5
#  define PI_GM		0x000007E0
#  define PI_GS		5
#  define PI_GB		6
#  define PI_BM		0x0000001F
#  define PI_BS		0
#  define PI_BB		5
#elif defined (PI_R8G8B8)
#  define PI_PIXTYPE	ULong
#  define PI_RM		0x00FF0000
#  define PI_RS		16
#  define PI_RB		8
#  define PI_GM		0x0000FF00
#  define PI_GS		8
#  define PI_GB		8
#  define PI_BM		0x000000FF
#  define PI_BS		0
#  define PI_BB		8
#elif defined (PI_B8G8R8)
#  define PI_PIXTYPE	ULong
#  define PI_RM		0x000000FF
#  define PI_RS		0
#  define PI_RB		8
#  define PI_GM		0x0000FF00
#  define PI_GS		8
#  define PI_GB		8
#  define PI_BM		0x00FF0000
#  define PI_BS		16
#  define PI_BB		8
#else
#  error "ERROR: Pixel format not defined before including pixtype.inc!"
#endif
