#include "ChecksumRegions.h"
/*
//Region 29: LibHD2DVD::HDDiscMgr.cpp
int TR_RegionBegin_29() 
{
	static int a=0;
	for (int i = 0; i < 29; i ++)
		a++; 
	return a;
}
int TR_RegionEnd_29() 
{
	static int a=0;
	for (int i = 29; i > 0; i --); 
		a++; 
	return a;
}
volatile DWORD	TR_ScrambleEnable_29	    = TR_SCRAMBLE_ENABLE_29^1;
DWORD	TR_RelocateSize_29	        = 10*1024;
WORD	TR_RelocateTable_29[10*1024] = {0xffff};
volatile DWORD	TR_ChecksumEnable_29 = TR_CHECKSUM_ENABLE_29^1;
volatile DWORD	TR_ChecksumValue_29  = TR_CHECKSUM_VALUE_29;

//Region 52: LibHD2DVD::CHDPlaylistMgr.cpp
int TR_RegionBegin_52()
{ 
	static int play; 
	for( int i=368; i<98754; i++) play|=play; 
	return play; 
}
int TR_RegionEnd_52() 
{
	static int a = 0; 
	for (int i = 52; i > 0; i --) 
		a++;
	return a;
}
volatile DWORD	TR_ScrambleEnable_52	    = TR_SCRAMBLE_ENABLE_52^1;
DWORD	TR_RelocateSize_52	        = 10*1024;
WORD	TR_RelocateTable_52[10*1024] = {0xffff};
volatile DWORD	TR_ChecksumEnable_52 = TR_CHECKSUM_ENABLE_52^1;
volatile DWORD	TR_ChecksumValue_52  = TR_CHECKSUM_VALUE_52;

//Region 56: LibBDAV::CBDAVCtrl/CBDAVSubPathCtrl.cpp
int  TR_RegionBegin_56()
{
	int Region56;
	for (int i=0; i<100;i++)
		Region56 |= i;
	return Region56;
}

int  TR_RegionEnd_56()
{
	int Region56 = 56;
	for (int i=0; i<99; i++)
		Region56 = Region56*(i+1); 
	return Region56;
}

volatile DWORD	TR_ChecksumEnable_56 = TR_CHECKSUM_ENABLE_56^1;
WORD	TR_RelocateTable_56[4000] = {0xffff};
DWORD	TR_RelocateSize_56 = sizeof(TR_RelocateTable_56)/sizeof(WORD);
volatile DWORD	TR_ChecksumValue_56 = TR_CHECKSUM_VALUE_56;

//Region 57: LibBDROM::BDROMCtrl.cpp/BDROMSubPathCtrl.cpp
int  TR_RegionBegin_57()
{
	int Region57;
	for (int i=0; i<111;i++)
		Region57 ^= i;
	return Region57;
}

int  TR_RegionEnd_57()
{
	int Region57 = 57;
	for (int i=0; i<99; i++)
		Region57 = Region57/(i+1); 
	return Region57;
}

volatile DWORD   TR_ScrambleEnable_57 = TR_SCRAMBLE_ENABLE_57^1;
volatile DWORD	TR_ChecksumEnable_57 = TR_CHECKSUM_ENABLE_57^1;
WORD	TR_RelocateTable_57[4000] = {0xffff};
DWORD	TR_RelocateSize_57 = sizeof(TR_RelocateTable_57)/sizeof(WORD);
volatile DWORD	TR_ChecksumValue_57 = TR_CHECKSUM_VALUE_57;

//Region 61: LibHD2DVD::hd2dvdctrl.cpp/HDDiscFileData.cpp
int  TR_RegionBegin_61()
{
	int Region61;
	for (int i=0; i<111;i++)
		Region61 ^= i*i;
	return Region61;
}

int  TR_RegionEnd_61()
{
	int Region61;
	for (int i=0; i<115;i++)
		Region61 ^= i;
	return Region61;
}

volatile DWORD	TR_ChecksumEnable_61 = TR_CHECKSUM_ENABLE_61^1;
WORD	TR_RelocateTable_61[4000] = {0xffff};
DWORD	TR_RelocateSize_61 = sizeof(TR_RelocateTable_61)/sizeof(WORD);
volatile DWORD	TR_ChecksumValue_61 = TR_CHECKSUM_VALUE_61;


//Region 62: LibHDDVD::hddvdctrl.cpp
int TR_RegionBegin_62()
{
	int Region_62;
	for (int i=0; i<62;i++)
		Region_62 ^= i;
	return Region_62;
}

int TR_RegionEnd_62()
{
	int Region_62;
	for (int i=62; i>0; i--)
		Region_62 ^= i;
	return Region_62;
}
volatile DWORD	TR_ChecksumEnable_62 = TR_CHECKSUM_ENABLE_62^1;
WORD	TR_RelocateTable_62[4000] = {0xffff};
DWORD	TR_RelocateSize_62 = sizeof(TR_RelocateTable_62)/sizeof(WORD);
volatile DWORD	TR_ChecksumValue_62 = TR_CHECKSUM_VALUE_62;


//Region 63: LibHDVR::HDVRCtrl.cpp

int  TR_RegionBegin_63()
{
	volatile int Region63;
	char *a = 0;
	for (int i=0; i<111;i++)
		Region63 ^= i;
	for (int i=0; i<111;i++)
		Region63 |= *a;
	return Region63;
}

int  TR_RegionEnd_63()
{
	int Region63 = 63;
	volatile char *a = 0;
	for (int i=0; i<99; i++)
		Region63 = Region63/(i+1) + *a; 
	return Region63;
}
volatile DWORD	TR_ChecksumEnable_63 = TR_CHECKSUM_ENABLE_63^1;
WORD	TR_RelocateTable_63[4000] = {0xffff};
DWORD	TR_RelocateSize_63 = sizeof(TR_RelocateTable_63)/sizeof(WORD);
volatile DWORD	TR_ChecksumValue_63 = TR_CHECKSUM_VALUE_63;

//**************************AACS REGIONS********************
volatile DWORD	TR_ChecksumEnable_91  = TR_CHECKSUM_ENABLE_91^1;	// set nonzero if auth integrity check is to be performed.
volatile DWORD	TR_ChecksumValue_91	  = TR_CHECKSUM_VALUE_91;		// default magic value
volatile DWORD	TR_ScrambleEnable_91  = TR_SCRAMBLE_ENABLE_91^1;   // Represents whether this section is currently scrambled.
DWORD	TR_RelocateSize_91	  = 2000;                      // Only necessary for DLL. Not needed for EXE
WORD	TR_RelocateTable_91[2000] = {0xffff};             // Only necessary for DLL. Not needed for EXE
int TR_RegionBegin_91()
{
	volatile int i,j;
	for(i=22,j=44;i<66;i++)
		i += j+7;
	return i;
}

int TR_RegionEnd_91()
{
	volatile int i,j;
	for(i=33,j=55;i<77;i++)
		i += j-9;
	return i;
}

volatile DWORD	TR_ChecksumEnable_85  = TR_CHECKSUM_ENABLE_85^1;	// set nonzero if auth integrity check is to be performed.
volatile DWORD	TR_ChecksumValue_85	  = TR_CHECKSUM_VALUE_85;		// default magic value
volatile DWORD	TR_ScrambleEnable_85  = TR_SCRAMBLE_ENABLE_85^1;   // Represents whether this section is currently scrambled.
DWORD	TR_RelocateSize_85	  = 2000;                      // Only necessary for DLL. Not needed for EXE
WORD	TR_RelocateTable_85[2000] = {0xffff};             // Only necessary for DLL. Not needed for EXE

int TR_RegionBegin_85()
{
	volatile int i,j;
	for(i=0,j=4;i<9;i++)
		i += j+2;
	return i;
}

int TR_RegionEnd_85()
{
	volatile int i,j;
	for(i=1,j=3;i<7;i++)
		i += j-2;
	return i;
}

volatile DWORD	TR_ChecksumEnable_86  = TR_CHECKSUM_ENABLE_86^1;	// set nonzero if auth integrity check is to be performed.
volatile DWORD	TR_ChecksumValue_86	  = TR_CHECKSUM_VALUE_86;		// default magic value
volatile DWORD	TR_ScrambleEnable_86  = TR_SCRAMBLE_ENABLE_86^1;   // Represents whether this section is currently scrambled.
DWORD	TR_RelocateSize_86	  = 2000;                      // Only necessary for DLL. Not needed for EXE
WORD	TR_RelocateTable_86[2000] = {0xffff};             // Only necessary for DLL. Not needed for EXE

int TR_RegionBegin_86()
{
	volatile int i,j;
	for(i=333,j=666;i<999;i++)
		i += j+777;
	return i;
}

int TR_RegionEnd_86()
{
	volatile int i,j;
	for(i=222,j=444;i<888;i++)
		i += j-999;
	return i;
}

volatile DWORD	TR_ChecksumEnable_93  = TR_CHECKSUM_ENABLE_93^1;	// set nonzero if auth integrity check is to be performed.
volatile DWORD	TR_ChecksumValue_93	  = TR_CHECKSUM_VALUE_93;		// default magic value
volatile DWORD	TR_ScrambleEnable_93  = TR_SCRAMBLE_ENABLE_93^1;   // Represents whether this section is currently scrambled.
DWORD	TR_RelocateSize_93	  = 2000;                      // Only necessary for DLL. Not needed for EXE
WORD	TR_RelocateTable_93[2000] = {0xffff};             // Only necessary for DLL. Not needed for EXE

int TR_RegionBegin_93()
{
	volatile int i,j;
	for(i=38726,j=1153;i<9876;i--)
		i += j*7;
	return i;
}

int TR_RegionEnd_93()
{
	volatile int i,j;
	for(i=1121,j=5665;i<7897;i--)
		i += j+9;
	return i;
}

volatile DWORD TR_ScrambleEnable_98  = TR_SCRAMBLE_ENABLE_98 ^ 1;
WORD TR_RelocateTable_98[4000] = {0xffff};
DWORD TR_RelocateSize_98 = sizeof(TR_RelocateTable_98) / sizeof(WORD);
volatile DWORD TR_ChecksumEnable_98 = TR_CHECKSUM_ENABLE_98 ^ 1;
volatile DWORD TR_ChecksumValue_98 = TR_CHECKSUM_VALUE_98;

int TR_RegionBegin_98()
{
	volatile int i,j;
	for(i=4346,j=1510;i<7881;i++)
		i += j+403;
	return i;
}

int TR_RegionEnd_98()
{
	volatile int i,j;
	for(i=298,j=-838;i<8990;i++)
		i -= j-1337;
	return i;
}

volatile DWORD TR_ChecksumEnable_81 = TR_CHECKSUM_ENABLE_81 ^ 1;
volatile DWORD TR_ChecksumValue_81 = TR_CHECKSUM_VALUE_81;
volatile DWORD TR_ScrambleEnable_81 = TR_SCRAMBLE_ENABLE_81 ^ 1;
DWORD TR_RelocateSize_81 = 2000;
WORD TR_RelocateTable_81[2000] = {0xffff};

int TR_RegionBegin_81()
{
	volatile int i, j;
	for(i = 81, j = 118; i < 818; i++)
		i += j + 18;
	return i;
}

int TR_RegionEnd_81()
{
	volatile int i, j;
	for(i = 81, j = 18; i < 181; i++)
		i += j - 881;
	return i;
}

volatile DWORD	TR_ChecksumEnable_92  = TR_CHECKSUM_ENABLE_92^1;	// set nonzero if auth integrity check is to be performed.
volatile DWORD	TR_ChecksumValue_92	  = TR_CHECKSUM_VALUE_92;		// default magic value
volatile DWORD	TR_ScrambleEnable_92  = TR_SCRAMBLE_ENABLE_92^1;   // Represents whether this section is currently scrambled.
DWORD	TR_RelocateSize_92	  = 2000;                      // Only necessary for DLL. Not needed for EXE
WORD	TR_RelocateTable_92[2000] = {0xffff};             // Only necessary for DLL. Not needed for EXE

int TR_RegionBegin_92()
{
	volatile int i,j;
	for(i=135,j=246;i<379;i++)
		i += j+7;
	return i;
}

int TR_RegionEnd_92()
{
	volatile int i,j;
	for(i=987,j=654;i<321;i++)
		i += j-9;
	return i;
}

volatile DWORD	TR_ChecksumEnable_87  = TR_CHECKSUM_ENABLE_87^1;	// set nonzero if auth integrity check is to be performed.
volatile DWORD	TR_ChecksumValue_87	  = TR_CHECKSUM_VALUE_87;		// default magic value
volatile DWORD	TR_ScrambleEnable_87  = TR_SCRAMBLE_ENABLE_87^1;   // Represents whether this section is currently scrambled.
DWORD	TR_RelocateSize_87	  = 2000;                      // Only necessary for DLL. Not needed for EXE
WORD	TR_RelocateTable_87[2000] = {0xffff};             // Only necessary for DLL. Not needed for EXE

int TR_RegionBegin_87()
{
	volatile int i,j;
	for(i=4545,j=2323;i<8989;i++)
		i += j+7;
	return i;
}

int TR_RegionEnd_87()
{
	volatile int i,j;
	for(i=949,j=838;i<414;i++)
		i += j-9;
	return i;
}

volatile DWORD	TR_ChecksumEnable_88  = TR_CHECKSUM_ENABLE_88^1;	// set nonzero if auth integrity check is to be performed.
volatile DWORD	TR_ChecksumValue_88	  = TR_CHECKSUM_VALUE_88;		// default magic value
volatile DWORD	TR_ScrambleEnable_88  = TR_SCRAMBLE_ENABLE_88^1;   // Represents whether this section is currently scrambled.
DWORD	TR_RelocateSize_88	  = 2000;                      // Only necessary for DLL. Not needed for EXE
WORD	TR_RelocateTable_88[2000] = {0xffff};             // Only necessary for DLL. Not needed for EXE

int TR_RegionBegin_88()
{
	volatile int i,j;
	for(i=100,j=200;i<300;i++)
		i += j+7;
	return i;
}

int TR_RegionEnd_88()
{
	volatile int i,j;
	for(i=400,j=500;i<600;i++)
		i += j-9;
	return i;
}

volatile DWORD	TR_ChecksumEnable_96  = TR_CHECKSUM_ENABLE_96^1;	// set nonzero if auth integrity check is to be performed.
volatile DWORD	TR_ChecksumValue_96	  = TR_CHECKSUM_VALUE_96;		// default magic value
volatile DWORD	TR_ScrambleEnable_96  = TR_SCRAMBLE_ENABLE_96^1;   // Represents whether this section is currently scrambled.
DWORD	TR_RelocateSize_96	  = 2000;                      // Only necessary for DLL. Not needed for EXE
WORD	TR_RelocateTable_96[2000] = {0xffff};             // Only necessary for DLL. Not needed for EXE

int TR_RegionBegin_96()
{
	volatile int i,j;
	for(i=13579,j=97531;i<22446;i++)
		i += j-789;
	return i;
}

int TR_RegionEnd_96()
{
	volatile int i,j;
	for(i=123123,j=456456;i<789789;i++)
		i += j+54321;
	return i;
}

volatile DWORD	TR_ChecksumEnable_82  = TR_CHECKSUM_ENABLE_82^1;	// set nonzero if auth integrity check is to be performed.
volatile DWORD	TR_ChecksumValue_82	  = TR_CHECKSUM_VALUE_82;		// default magic value
volatile DWORD	TR_ScrambleEnable_82  = TR_SCRAMBLE_ENABLE_82^1;   // Represents whether this section is currently scrambled.
DWORD	TR_RelocateSize_82	  = 2000;                      // Only necessary for DLL. Not needed for EXE
WORD	TR_RelocateTable_82[2000] = {0xffff};             // Only necessary for DLL. Not needed for EXE

int TR_RegionBegin_82()
{
	volatile int i;
	for(i=0; i<82; i++)
		i+=2;
	return i;
}

int TR_RegionEnd_82()
{
	volatile int i;
	for(i=82; i>0; i--)
		i-=3;
	return i;
}

volatile DWORD	TR_ChecksumEnable_83  = TR_CHECKSUM_ENABLE_83^1;	// set nonzero if auth integrity check is to be performed.
volatile DWORD	TR_ChecksumValue_83	  = TR_CHECKSUM_VALUE_83;		// default magic value
volatile DWORD	TR_ScrambleEnable_83  = TR_SCRAMBLE_ENABLE_83^1;   // Represents whether this section is currently scrambled.
DWORD	TR_RelocateSize_83	  = 2000;                      // Only necessary for DLL. Not needed for EXE
WORD	TR_RelocateTable_83[2000] = {0xffff};             // Only necessary for DLL. Not needed for EXE

int TR_RegionBegin_83()
{
	volatile int i;
	for(i=0; i<83; i++)
		i++;
	return i;
}

int TR_RegionEnd_83()
{
	volatile int i;
	for(i=83; i>0; i--)
		i-=2;
	return i;
}

volatile DWORD	TR_ChecksumEnable_89  = TR_CHECKSUM_ENABLE_89^1;	// set nonzero if auth integrity check is to be performed.
volatile DWORD	TR_ChecksumValue_89	  = TR_CHECKSUM_VALUE_89;		// default magic value
volatile DWORD	TR_ScrambleEnable_89  = TR_SCRAMBLE_ENABLE_89^1;   // Represents whether this section is currently scrambled.
DWORD	TR_RelocateSize_89	  = 2000;                      // Only necessary for DLL. Not needed for EXE
WORD	TR_RelocateTable_89[2000] = {0xffff};             // Only necessary for DLL. Not needed for EXE

int TR_RegionBegin_89()
{
	volatile int i,j;
	for(i=90,j=80;i<70;i++)
		i += j+7;
	return i;
}

int TR_RegionEnd_89()
{
	volatile int i,j;
	for(i=60,j=50;i<40;i++)
		i += j-9;
	return i;
}

volatile DWORD	TR_ChecksumEnable_90  = TR_CHECKSUM_ENABLE_90^1;	// set nonzero if auth integrity check is to be performed.
volatile DWORD	TR_ChecksumValue_90	  = TR_CHECKSUM_VALUE_90;		// default magic value
volatile DWORD	TR_ScrambleEnable_90  = TR_SCRAMBLE_ENABLE_90^1;   // Represents whether this section is currently scrambled.
DWORD	TR_RelocateSize_90	  = 2000;                      // Only necessary for DLL. Not needed for EXE
WORD	TR_RelocateTable_90[2000] = {0xffff};             // Only necessary for DLL. Not needed for EXE

int TR_RegionBegin_90()
{
	volatile int i,j;
	for(i=57,j=38;i<49;i++)
		i += j+7;
	return i;
}

int TR_RegionEnd_90()
{
	volatile int i,j;
	for(i=26,j=73;i<88;i++)
		i += j-9;
	return i;
}
volatile DWORD	TR_ChecksumEnable_78  = TR_CHECKSUM_ENABLE_78^1;	// set nonzero if auth integrity check is to be performed.
volatile DWORD	TR_ChecksumValue_78	  = TR_CHECKSUM_VALUE_78;		// default magic value
volatile DWORD	TR_ScrambleEnable_78  = TR_SCRAMBLE_ENABLE_78^1;   // Represents whether this section is currently scrambled.
DWORD	TR_RelocateSize_78	  = 2000;                      // Only necessary for DLL. Not needed for EXE
WORD	TR_RelocateTable_78[2000] = {0xffff};             // Only necessary for DLL. Not needed for EXE

int TR_RegionBegin_78()
{
	volatile int i;
	for(i=0; i<78; i++)
		i += 2;
	return i;
}

int TR_RegionEnd_78()
{
	volatile int i;
	for(i=78; i>0; i--)
		i -= 2;
	return i;
}

volatile DWORD	TR_ChecksumEnable_79  = TR_CHECKSUM_ENABLE_79^1;	// set nonzero if auth integrity check is to be performed.
volatile DWORD	TR_ChecksumValue_79	  = TR_CHECKSUM_VALUE_79;		// default magic value
volatile DWORD	TR_ScrambleEnable_79  = TR_SCRAMBLE_ENABLE_79^1;   // Represents whether this section is currently scrambled.
DWORD	TR_RelocateSize_79	  = 2000;                      // Only necessary for DLL. Not needed for EXE
WORD	TR_RelocateTable_79[2000] = {0xffff};             // Only necessary for DLL. Not needed for EXE

int TR_RegionBegin_79()
{
	volatile int i;
	for(i=0; i<79; i++)
		i += 3;
	return i;
}

int TR_RegionEnd_79()
{
	volatile int i;
	for(i=79; i>0; i--)
		i -= 3;
	return i;
}


int TR_RegionBegin_84()
{
	volatile int i,j;
	for(i=22,j=44;i<84;i++)
		i += j+7;
	return i;
}

int TR_RegionEnd_84()
{
	volatile int i,j;
	for(i=333,j=555;i<8844;i++)
		i += j-99;
	return i;
}

volatile DWORD	TR_ChecksumEnable_84  = TR_CHECKSUM_ENABLE_84^1;	// set nonzero if auth integrity check is to be performed.
volatile DWORD	TR_ChecksumValue_84	  = TR_CHECKSUM_VALUE_84;		// default magic value
volatile DWORD	TR_ScrambleEnable_84  = TR_SCRAMBLE_ENABLE_84^1;   // Represents whether this section is currently scrambled.
DWORD	TR_RelocateSize_84	  = 4000;                      // Only necessary for DLL. Not needed for EXE
WORD	TR_RelocateTable_84[4000] = {0xffff};             // Only necessary for DLL. Not needed for EXE

volatile DWORD	TR_ChecksumEnable_80  = TR_CHECKSUM_ENABLE_80^1;	// set nonzero if auth integrity check is to be performed.
volatile DWORD	TR_ChecksumValue_80	  = TR_CHECKSUM_VALUE_80;		// default magic value
volatile DWORD	TR_ScrambleEnable_80  = TR_SCRAMBLE_ENABLE_80^1;   // Represents whether this section is currently scrambled.
DWORD	TR_RelocateSize_80	  = 2000;                      // Only necessary for DLL. Not needed for EXE
WORD	TR_RelocateTable_80[2000] = {0xffff};             // Only necessary for DLL. Not needed for EXE

int TR_RegionBegin_80()
{
	volatile int i;
	for(i=0; i< 80; i++)
		i++;
	return i;
}

int TR_RegionEnd_80()
{
	volatile int i;
	for(i=80; i>0; i--)
		i--;
	return i;
}

volatile DWORD	TR_ChecksumEnable_94  = TR_CHECKSUM_ENABLE_94^1;	// set nonzero if auth integrity check is to be performed.
volatile DWORD	TR_ChecksumValue_94	  = TR_CHECKSUM_VALUE_94;		// default magic value
volatile DWORD	TR_ScrambleEnable_94  = TR_SCRAMBLE_ENABLE_94^1;   // Represents whether this section is currently scrambled.
DWORD	TR_RelocateSize_94	  = 2000;                      // Only necessary for DLL. Not needed for EXE
WORD	TR_RelocateTable_94[2000] = {0xffff};             // Only necessary for DLL. Not needed for EXE

int TR_RegionBegin_94()
{
	volatile int i,j;
	for(i=2552,j=4554;i<6556;i++)
		i += j+755;
	return i;
}

int TR_RegionEnd_94()
{
	volatile int i,j;
	for(i=3113,j=5115;i<7117;i++)
		i += j-911;
	return i;
}

volatile DWORD	TR_ChecksumEnable_58  = TR_CHECKSUM_ENABLE_58^1;	// set nonzero if auth integrity check is to be performed.
volatile DWORD	TR_ChecksumValue_58	  = TR_CHECKSUM_VALUE_58;		// default magic value
volatile DWORD	TR_ScrambleEnable_58  = TR_SCRAMBLE_ENABLE_58^1;   // Represents whether this section is currently scrambled.
DWORD	TR_RelocateSize_58	  = 2000;                      // Only necessary for DLL. Not needed for EXE
WORD	TR_RelocateTable_58[2000] = {0xffff};             // Only necessary for DLL. Not needed for EXE

int TR_RegionBegin_58()
{
	volatile int i,j=0;
	for(i=0;i<58;i++)
		j++;
	return j;
}

int TR_RegionEnd_58()
{
	volatile int i,j=58;
	for(i=58; i>0; i--)
		j--;
	return j;
}

volatile DWORD	TR_ChecksumEnable_59  = TR_CHECKSUM_ENABLE_59^1;	// set nonzero if auth integrity check is to be performed.
volatile DWORD	TR_ChecksumValue_59	  = TR_CHECKSUM_VALUE_59;		// default magic value
volatile DWORD	TR_ScrambleEnable_59  = TR_SCRAMBLE_ENABLE_59^1;   // Represents whether this section is currently scrambled.
DWORD	TR_RelocateSize_59	  = 2000;                      // Only necessary for DLL. Not needed for EXE
WORD	TR_RelocateTable_59[2000] = {0xffff};             // Only necessary for DLL. Not needed for EXE

int TR_RegionBegin_59()
{
	volatile int i,j;
	for(i=4,j=459;i<59;i++)
		i += j+9;
	return i;
}

int TR_RegionEnd_59()
{
	volatile int i,j;
	for(i=59,j=599;i<777;i++)
		i += j-29;
	return i;
}
volatile DWORD	TR_ChecksumEnable_77  = TR_CHECKSUM_ENABLE_77^1;	// set nonzero if auth integrity check is to be performed.
volatile DWORD	TR_ChecksumValue_77	  = TR_CHECKSUM_VALUE_77;		// default magic value
volatile DWORD	TR_ScrambleEnable_77  = TR_SCRAMBLE_ENABLE_77^1;   // Represents whether this section is currently scrambled.
DWORD	TR_RelocateSize_77	  = 2000;                      // Only necessary for DLL. Not needed for EXE
WORD	TR_RelocateTable_77[2000] = {0xffff};             // Only necessary for DLL. Not needed for EXE

int TR_RegionBegin_77()
{
	volatile int i,j=0;
	for(i=0;i<77;i++)
		j++;
	return j;
}

int TR_RegionEnd_77()
{
	volatile int i,j=77;
	for(i=77; i>0; i--)
		j--;
	return j;
}
*/