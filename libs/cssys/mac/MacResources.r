#include <Types.r>

resource 'ALRT' (1024, purgeable) {
	{332,326,476,670},		/* 344 wide by 144 high */
	1024,					/* DITL id */
	{
		OK, visible, sound1;	/* Stage 4 */
		OK, visible, sound1;	/* Stage 3 */
		OK, visible, sound1;	/* Stage 2 */
		OK, visible, sound1 	/* Stage 1 */
	},
	alertPositionMainScreen
};

resource 'DITL' (1024, purgeable) {		/* 3 items */
  {
	/* 1*/ {109,261,129,329},
			Button { enabled , "OK" },
	/* 2*/ {17,61,33,329},
			StaticText { disabled , "^0" },
	/* 3*/ {49,17,97,329},
			StaticText { disabled , "^1" }
  }
};

data 'actb' (1024) {
	$"0000 0000 0000 0004 0000 DDDD DDDD DDDD"
	$"0001 0000 0000 0000 0002 0000 0000 0000"
	$"0003 0000 0000 0000 0004 FFFF FFFF FFFF"
};

data 'ictb' (1024) {
	$"0000 0000 0007 000C 0007 0020 0001 0100"
	$"000C 0000 0000 0000 FFFF FFFF FFFF 0000"
	$"0001 0000 0009 0000 0000 0000 FFFF FFFF"
	$"FFFF 0000"
};

resource 'DLOG' (1025, purgeable) {
	{259,337,423,693},		/* 356 wide by 164 high */
	movableDBoxProc,
	invisible, noGoAway,
	0x0,
	1025,
	"",
	centerMainScreen
};

resource 'DITL' (1025, purgeable) {		/* 4 items */
  {
	/* 1*/ {129,273,149,341},
			Button { enabled , "OK" },
	/* 2*/ {129,193,149,257},
			Button { enabled , "Quit" },
	/* 3*/ {36,16,108,340},
			EditText { enabled , "" },
	/* 4*/ {13,13,29,341},
			StaticText { disabled , "Command Line Entry:" }
  }
};

data 'dctb' (1025) {
	$"0000 0000 0000 0004 0000 DDDD DDDD DDDD"
	$"0001 0000 0000 0000 0002 0000 0000 0000"
	$"0003 0000 0000 0000 0004 FFFF FFFF FFFF"
};

data 'ictb' (1025) {
	$"0000 0000 0000 0000 200F 0010 0007 0024"
	$"0001 0000 000C 0000 0000 0000 FFFF FFFF"
	$"FFFF 0000 0001 0100 000C 0000 0000 0000"
	$"FFFF FFFF FFFF 0000"
};

resource 'ALRT' (1026, purgeable) {
	{332,326,480,698},		/* 372 wide by 148 high */
	1026,					/* DITL id */
	{
		OK, visible, sound1;	/* Stage 4 */
		OK, visible, sound1;	/* Stage 3 */
		OK, visible, sound1;	/* Stage 2 */
		OK, visible, sound1 	/* Stage 1 */
	},
	alertPositionMainScreen
};

resource 'DITL' (1026, purgeable) {		/* 2 items */
  {
	/* 1*/ {113,293,133,361},
			Button { enabled , "OK" },
	/* 2*/ {17,65,101,361},
			StaticText { disabled , "^0" }
  }
};

data 'actb' (1026) {
	$"0000 0000 0000 0004 0000 DDDD DDDD DDDD"
	$"0001 0000 0000 0000 0002 0000 0000 0000"
	$"0003 0000 0000 0000 0004 FFFF FFFF FFFF"
};

data 'ictb' (1026) {
	$"0000 0000 0000 0000"
};

resource 'ALRT' (1027, purgeable) {
	{332,326,444,654},		/* 328 wide by 112 high */
	1027,					/* DITL id */
	{
		OK, visible, sound1;	/* Stage 4 */
		OK, visible, sound1;	/* Stage 3 */
		OK, visible, sound1;	/* Stage 2 */
		OK, visible, sound1 	/* Stage 1 */
	},
	alertPositionMainScreen
};

resource 'DITL' (1027, purgeable) {		/* 3 items */
  {
	/* 1*/ {77,245,97,313},
			Button { enabled , "Yes" },
	/* 2*/ {77,165,97,229},
			Button { enabled , "Quit" },
	/* 3*/ {17,65,57,317},
			StaticText { disabled , "This game requires ^0 colors.  Can it be changed?" }
  }
};

data 'actb' (1027) {
	$"0000 0000 0000 0004 0000 DDDD DDDD DDDD"
	$"0001 0000 0000 0000 0002 0000 0000 0000"
	$"0003 0000 0000 0000 0004 FFFF FFFF FFFF"
};

data 'ictb' (1027) {
	$"0000 0000 0000 0000 0000 0000"
};

resource 'ALRT' (1028, purgeable) {
	{332,326,520,658},		/* 332 wide by 188 high */
	1028,					/* DITL id */
	{
		OK, visible, sound1;	/* Stage 4 */
		OK, visible, sound1;	/* Stage 3 */
		OK, visible, sound1;	/* Stage 2 */
		OK, visible, sound1 	/* Stage 1 */
	},
	alertPositionMainScreen
};

resource 'DITL' (1028, purgeable) {		/* 5 items */
  {
	/* 1*/ {153,249,173,317},
			Button { enabled , "OK" },
	/* 2*/ {13,13,29,317},
			StaticText { disabled , "About ^0. " },
	/* 3*/ {41,13,77,317},
			StaticText { disabled , "Crystal Space Copyright (c) 1998 by Jorrit Tyberghein" },
	/* 4*/ {81,13,97,317},
			StaticText { disabled , "Mac Port Copyright (c) 1998 by K. Robert Bate" },
	/* 5*/ {101,13,137,317},
			StaticText { disabled , "Some libraries Copyright (c) 1998 Metrowerks Inc." }
  }
};

data 'actb' (1028) {
	$"0000 0000 0000 0004 0000 DDDD DDDD DDDD"
	$"0001 0000 0000 0000 0002 0000 0000 0000"
	$"0003 0000 0000 0000 0004 FFFF FFFF FFFF"
};

data 'ictb' (1028) {
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 0000"
};


data 'cicn' (0, purgeable) {
	$"0000 0000 8010 0000 0000 0020 0020 0000"
	$"0000 0000 0000 0048 0000 0048 0000 0000"
	$"0004 0001 0004 0000 0000 0000 0352 0000"
	$"0000 0000 0000 0004 0000 0000 0020 0020"
	$"0000 0000 0004 0000 0000 0020 0020 0000"
	$"0000 01FF FF80 03FF FFC0 07FF FFE0 0FFF"
	$"FFF0 1FFF FFF8 3FFF FFFC 7FFF FFFE FFFF"
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
	$"FFFF FFFF FFFF FFFF FFFF 7FFF FFFE 3FFF"
	$"FFFC 1FFF FFF8 0FFF FFF0 07FF FFE0 03FF"
	$"FFC0 00FF FF00 01FF FF80 03FF FFC0 07FE"
	$"FFE0 0FFC 7FF0 1FEC 67F8 3FC4 47FC 7FC4"
	$"47FE FF44 47FF FE44 47FF FE44 47FF FE44"
	$"47FF FE44 47FF FE44 47FF FE40 071F FE00"
	$"061F FE00 061F FE00 043F FE00 003F FE00"
	$"007F FE00 007F FE00 00FF FE00 00FF FE00"
	$"01FF FE00 01FF 7E04 03FE 3F02 07FC 1FFF"
	$"FFF8 0FFF FFF0 07FF FFE0 03FF FFC0 01FF"
	$"FF80 0000 8C02 0000 000A 0000 FFFF FFFF"
	$"FFFF 0001 FFFF CCCC 9999 0002 FFFF 0000"
	$"0000 0003 CCCC 9999 6666 0004 9999 6666"
	$"3333 0005 9999 0000 0000 0006 DDDD 0000"
	$"0000 0007 AAAA 0000 0000 0008 5555 0000"
	$"0000 0009 AAAA AAAA AAAA 000F 0000 0000"
	$"0000 0000 0009 FFFF FFFF FFFF FFFF 9000"
	$"0000 0000 009F 2222 2222 2222 2222 F900"
	$"0000 0000 09F2 6666 666F 6666 6666 6F90"
	$"0000 0000 9F26 6666 66F3 F666 6666 66F9"
	$"0000 0009 F266 666F 6F11 4F6F F666 666F"
	$"9000 009F 2666 66F3 FF11 4FF3 4F66 6666"
	$"F900 09F2 6666 6F11 4F11 4F11 4F66 6666"
	$"6F90 9F26 6666 FF11 4F11 4F11 4F66 6666"
	$"66F9 F266 666F 4F11 4F11 4F11 4F66 6666"
	$"665F F266 66F1 4F11 4F11 4F11 4F66 6666"
	$"665F F266 66F1 4F11 4F11 4F11 4F66 6666"
	$"665F F266 66F1 4F11 4F11 4F11 4F66 6666"
	$"665F F266 66F1 4F11 3F11 3F11 4F66 6666"
	$"665F F266 66F1 3F11 1F11 1F11 4F66 8FF6"
	$"665F F266 66F1 1F11 1111 1111 4F68 433F"
	$"665F F266 66F1 1111 1111 1111 4F74 114F"
	$"665F F266 66F1 1111 1111 1111 3FF1 114F"
	$"665F F266 66F1 1111 1111 1111 1F11 14F6"
	$"665F F266 66F1 1111 1111 1111 1111 14F6"
	$"665F F266 66F1 1111 1111 1111 1111 4F66"
	$"665F F266 66F1 1111 1111 1111 1113 4F66"
	$"665F F266 66F1 1111 1111 1111 1114 F666"
	$"665F F266 66F1 1111 1111 1111 1134 F666"
	$"665F F266 66F1 1111 1111 1111 114F 6666"
	$"665F F266 6674 1111 1111 1111 134F 6666"
	$"665F 9F66 666F 1111 1F31 1113 34F6 6666"
	$"65F9 09F6 6666 F344 44F4 4444 4F66 6666"
	$"5F90 009F 6666 6FFF FF6F FFFF F666 6665"
	$"F900 0009 F666 6666 6666 6666 6666 665F"
	$"9000 0000 9F66 6666 6666 6666 6666 65F9"
	$"0000 0000 09F5 5555 5555 5555 5555 5F90"
	$"0000 0000 009F FFFF FFFF FFFF FFFF F900"
	$"0000"
	};

data 'cicn' (1, purgeable) {
	$"0000 0000 8010 0000 0000 0020 0020 0000"
	$"0000 0000 0000 0048 0000 0048 0000 0000"
	$"0004 0001 0004 0000 0000 0000 0352 0000"
	$"0000 0000 0000 0004 0000 0000 0020 0020"
	$"0000 0000 0004 0000 0000 0020 0020 0000"
	$"0000 FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
	$"FFFF FFFF FFFF 807F FFFF 807F FFFF 807F"
	$"FFFF 807F FFFF 807F C0FF 887F 003F 887E"
	$"001F 887C 000F 8078 0007 8078 0007 8070"
	$"0003 8071 DDC3 8070 0003 8070 0003 8071"
	$"DD43 8070 0003 8070 0003 8071 D703 8070"
	$"0003 87F0 0003 81F1 EEC3 81F0 0007 81F0"
	$"0007 81F0 000F 81E0 001F 8F80 007F 81FF"
	$"FFFF 81FF FFFF 81FF FFFF 81FF FFFF FFFF"
	$"FFFF 0000 8CAE 0000 0006 0000 FFFF FFFF"
	$"FFFF 0001 FFFF CCCC 9999 0002 CCCC CCCC"
	$"FFFF 0003 CCCC 9999 6666 0004 9999 6666"
	$"3333 0005 8888 8888 8888 0006 1111 1111"
	$"1111 6666 6666 6666 6666 6666 6666 6666"
	$"6666 6111 1113 4666 6666 6666 6666 6666"
	$"6666 6111 1111 4666 6666 6666 6666 6666"
	$"6666 6111 1111 4666 6666 6666 6666 6666"
	$"6666 6111 1111 4666 6666 6666 6666 6666"
	$"6666 6113 3111 4666 6666 6622 2222 6666"
	$"6666 6113 6111 4666 6666 2220 0002 2566"
	$"6666 6113 6111 4666 6662 2000 0000 2256"
	$"6666 6111 6111 4666 6622 0000 0000 0225"
	$"6666 6111 1111 4666 6220 0000 0000 0022"
	$"5666 6111 1111 4666 6200 0000 0000 0002"
	$"5666 6111 1111 4666 2200 0000 0000 0002"
	$"2566 6111 1111 4666 2006 6606 6606 6600"
	$"2566 6111 1111 4666 2000 0000 0000 0000"
	$"2566 6111 1111 4666 2000 0000 0000 0000"
	$"2566 6111 1111 4666 2006 6606 6606 0600"
	$"2566 6111 1111 4666 2000 0000 0000 0000"
	$"2566 6111 1111 4666 2000 0000 0000 0000"
	$"2566 6111 1111 4666 2006 6606 0666 0000"
	$"2566 6111 1133 4666 2000 0000 0000 0000"
	$"2566 6111 1666 6666 2000 0000 0000 0002"
	$"2566 6111 1346 6666 2006 6660 6660 6602"
	$"5566 6111 1146 6666 2000 0000 0000 0022"
	$"5666 6111 1146 6666 2000 0000 0000 0225"
	$"5666 6111 1146 6666 2000 0000 0002 2255"
	$"6666 6113 3336 6662 2222 2222 2222 5556"
	$"6666 6111 6666 6222 5555 5555 5555 5666"
	$"6666 6111 1116 6666 6666 6666 6666 6666"
	$"6666 6111 1146 6666 6666 6666 6666 6666"
	$"6666 6111 1146 6666 6666 6666 6666 6666"
	$"6666 6344 4446 6666 6666 6666 6666 6666"
	$"6666 6666 6666 6666 6666 6666 6666 6666"
	$"6666"
	};

data 'cicn' (2, purgeable) {
	$"0000 0000 8010 0000 0000 0020 0020 0000"
	$"0000 0000 0000 0048 0000 0048 0000 0000"
	$"0004 0001 0004 0000 0000 0000 0352 0000"
	$"0000 0000 0000 0004 0000 0000 0020 0020"
	$"0000 0000 0004 0000 0000 0020 0020 0000"
	$"0000 0001 8000 0003 C000 0007 E000 0007"
	$"E000 000F F000 000F F000 001F F800 001F"
	$"F800 003F FC00 003F FC00 007F FE00 007F"
	$"FE00 00FF FF00 00FF FF00 01FF FF80 01FF"
	$"FF80 03FF FFC0 03FF FFC0 07FF FFE0 07FF"
	$"FFE0 0FFF FFF0 0FFF FFF0 1FFF FFF8 1FFF"
	$"FFF8 3FFF FFFC 3FFF FFFC 7FFF FFFE 7FFF"
	$"FFFE FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
	$"FFFF 0001 8000 0003 C000 0003 C000 0006"
	$"6000 0006 6000 000C 3000 000C 3000 0018"
	$"1800 0019 9800 0033 CC00 0033 CC00 0063"
	$"C600 0063 C600 00C3 C300 00C3 C300 0183"
	$"C180 0183 C180 0303 C0C0 0303 C0C0 0603"
	$"C060 0601 8060 0C01 8030 0C00 0030 1800"
	$"0018 1801 8018 3003 C00C 3003 C00C 6001"
	$"8006 6000 0006 C000 0003 FFFF FFFF 7FFF"
	$"FFFE 0000 8D26 0000 0006 0000 FFFF FFFF"
	$"FFFF 0001 FFFF CCCC 3333 0002 CCCC 9999"
	$"0000 0003 9999 6666 0000 0004 3333 3333"
	$"3333 0005 BBBB BBBB BBBB 000F 0000 0000"
	$"0000 0000 0000 0000 000F F000 0000 0000"
	$"0000 0000 0000 0000 004F F400 0000 0000"
	$"0000 0000 0000 0000 05FF FF50 0000 0000"
	$"0000 0000 0000 0000 04F3 3F40 0000 0000"
	$"0000 0000 0000 0000 5FF1 1FF5 0000 0000"
	$"0000 0000 0000 0000 4F31 13F4 0000 0000"
	$"0000 0000 0000 0005 FF11 11FF 5000 0000"
	$"0000 0000 0000 0004 F311 113F 4000 0000"
	$"0000 0000 0000 005F F12F F21F F500 0000"
	$"0000 0000 0000 004F 314F F413 F400 0000"
	$"0000 0000 0000 05FF 11FF FF11 FF50 0000"
	$"0000 0000 0000 04F3 11FF FF11 3F40 0000"
	$"0000 0000 0000 5FF1 11FF FF11 1FF5 0000"
	$"0000 0000 0000 4F31 11FF FF11 13F4 0000"
	$"0000 0000 0005 FF11 11FF FF11 11FF 5000"
	$"0000 0000 0004 F311 11FF FF11 113F 4000"
	$"0000 0000 005F F111 11FF FF11 111F F500"
	$"0000 0000 004F 3111 11FF FF11 1113 F400"
	$"0000 0000 05FF 1111 11FF FF11 1111 FF50"
	$"0000 0000 04F3 1111 114F F411 1111 3F40"
	$"0000 0000 5FF1 1111 112F F211 1111 1FF5"
	$"0000 0000 4F31 1111 111F F111 1111 13F4"
	$"0000 0005 FF11 1111 1112 2111 1111 11FF"
	$"5000 0004 F311 1111 1111 1111 1111 113F"
	$"4000 005F F111 1111 112F F211 1111 111F"
	$"F500 004F 3111 1111 11FF FF11 1111 1113"
	$"F400 05FF 1111 1111 11FF FF11 1111 1111"
	$"FF50 04F3 1111 1111 112F F211 1111 1111"
	$"3F40 5FF1 1111 1111 1111 1111 1111 1111"
	$"1FF5 FF31 1111 1111 1111 1111 1111 1111"
	$"13FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
	$"FFFF 5FFF FFFF FFFF FFFF FFFF FFFF FFFF"
	$"FFF5"
	};

resource 'MENU' (128) {
	128,
	0,
	0x7FFFFFFD,
	enabled,
	apple, {
		"About ", noIcon, noKey, noMark, plain;
		"-", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (129) {
	129,
	0,
	0x7FFFFFFF,
	enabled,
	"File", {
		"Quit", noIcon, "Q", noMark, plain
	}
};

resource 'MENU' (130) {
	130,
	0,
	0x7FFFFFFD,
	enabled,
	"Edit", {
		"Undo", noIcon, "Z", noMark, plain;
		"-", noIcon, noKey, noMark, plain;
		"Cut", noIcon, "X", noMark, plain;
		"Copy", noIcon, "C", noMark, plain;
		"Paste", noIcon, "V", noMark, plain;
		"Clear", noIcon, noKey, noMark, plain
	}
};

resource 'MBAR' (128) {
	{
	128, 129, 130
	};
};

data 'CURS' (128) {
	/* Cursor */
	$"0000 4000 6000 7000 7800 7C00 7E00 7F00"
	$"7F80 7C00 6C00 4600 0600 0300 0300 0000"
	/* Mask */
	$"C000 E000 F000 F800 FC00 FE00 FF00 FF80"
	$"FFC0 FFE0 FE00 EF00 CF00 8780 0780 0380"
	/* Hotspot (v,h) */
	$"0001 0001"
};

data 'CURS' (129) {
	/* Cursor */
	$"0F00 30C0 4020 4020 8010 8010 8010 8010"
	$"4020 4020 30F0 0F38 001C 000E 0007 0002"
	/* Mask */
	$"0F00 30C0 4020 4020 8010 8010 8010 8010"
	$"4020 4020 30F0 0F38 001C 000E 0007 0002"
	/* Hotspot (v,h) */
	$"0006 0006"
};

data 'CURS' (130) {
	/* Cursor */
	$"0100 0100 0100 0100 0000 0100 0100 F7DE"
	$"0100 0100 0000 0100 0100 0100 0100 0000"
	/* Mask */
	$"0100 0100 0100 0100 0100 0100 0100 FEFE"
	$"0100 0100 0100 0100 0100 0100 0100 0000"
	/* Hotspot (v,h) */
	$"0007 0007"
};

data 'CURS' (131) {
	/* Cursor */
	$"0000 0800 0C00 0E00 0900 0880 0C40 0640"
	$"0620 0320 0310 0190 01B8 00C8 0088 0070"
	/* Mask */
	$"0000 0800 0C00 0E00 0F00 0F80 0FC0 07C0"
	$"07E0 03E0 03F0 01F0 01F8 00F8 00F8 0070"
	/* Hotspot (v,h) */
	$"0001 0004"
};

data 'CURS' (132) {
	/* Cursor */
	$"0000 0100 0380 07C0 0100 1110 3118 7FFC"
	$"3118 1110 0100 07C0 0380 0100 0000 0000"
	/* Mask */
	$"0100 0380 07C0 0FE0 1FF0 3BB8 7FFC FFFE"
	$"7FFC 3BB8 1FF0 0FE0 07C0 0380 0100 0000"
	/* Hotspot (v,h) */
	$"0007 0007"
};

data 'CURS' (133) {
	/* Cursor */
	$"0000 0000 3E00 3C00 3800 3400 2200 0100"
	$"0088 0058 0038 0078 00F8 0000 0000 0000"
	/* Mask */
	$"0000 7F00 7F00 7E00 7C00 7E00 7700 638C"
	$"01DC 00FC 007C 00FC 01FC 01FC 0000 0000"
	/* Hotspot (v,h) */
	$"0007 0007"
};

data 'CURS' (134) {
	/* Cursor */
	$"0000 0000 00F8 0078 0038 0058 0088 0100"
	$"2200 3400 3800 3C00 3E00 0000 0000 0000"
	/* Mask */
	$"0000 01FC 01FC 00FC 007C 00FC 01DC 638C"
	$"7700 7E00 7C00 7E00 7F00 7F00 0000 0000"
	/* Hotspot (v,h) */
	$"0007 0007"
};

data 'CURS' (135) {
	/* Cursor */
	$"0000 0100 0380 07C0 0FE0 0100 0100 0100"
	$"0100 0100 0100 0FE0 07C0 0380 0100 0000"
	/* Mask */
	$"0100 0380 07C0 0FE0 1FF0 1FF0 0380 0380"
	$"0380 0380 1FF0 1FF0 0FE0 07C0 0380 0100"
	/* Hotspot (v,h) */
	$"0007 0007"
};

data 'CURS' (136) {
	/* Cursor */
	$"0000 0000 0000 0000 0810 1818 381C 7FFE"
	$"381C 1818 0810 0000 0000 0000 0000 0000"
	/* Mask */
	$"0000 0000 0000 0C30 1C38 3C3C 7FFE FFFF"
	$"7FFE 3C3C 1C38 0C30 0000 0000 0000 0000"
	/* Hotspot (v,h) */
	$"0007 0008"
};

data 'CURS' (137) {
	/* Cursor */
	$"0000 0380 0C60 1010 2008 2008 5FF4 5FF4"
	$"5FF4 2008 2008 1010 0C60 0380 0000 0000"
	/* Mask */
	$"0380 0FE0 1FF0 3FF8 7FFC 7FFC FFFE FFFE"
	$"FFFE 7FFC 7FFC 3FF8 1FF0 0FE0 0380 0000"
	/* Hotspot (v,h) */
	$"0007 0007"
};

data 'CURS' (138) {
	/* Cursor */
	$"3F00 3F00 3F00 3F00 4080 8440 8440 8460"
	$"9C60 8040 8040 4080 3F00 3F00 3F00 3F00"
	/* Mask */
	$"3F00 3F00 3F00 3F00 7F80 FFC0 FFC0 FFC0"
	$"FFC0 FFC0 FFC0 7F80 3F00 3F00 3F00 3F00"
	/* Hotspot (v,h) */
	$"0008 0008"
};

/*
resource 'STR#' (1024, purgeable) {
	{
	"-recalc"
	}
};
*/

resource 'STR#' (1025, purgeable) {
	{ /* array StringArray: 4 elements */
	/* [1] */ "This Screen is unable to handle the required color depth.";
	/* [2] */ "No DrawSprocket context available.";
	/* [3] */ "Unable to open a DrawSprocket drawing context.";
	/* [4] */ "Unable to reserve a DrawSprocket drawing context.";
	/* [5] */ "Fatal Error in Glide2xRender.shlb";
	/* [6] */ "Fatal Error in OpenGL2D.shlb";
	/* [7] */ "Fatal Error in OpenGLRender.shlb";
	/* [8] */ "Out of Memory";
	/* [9] */ "Fatal Error in Driver2D.shlb"
	}
};

