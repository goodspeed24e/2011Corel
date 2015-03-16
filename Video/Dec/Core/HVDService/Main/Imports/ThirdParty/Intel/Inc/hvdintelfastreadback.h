//#define DELAYACCEL
#if defined DELAYACCEL	
#define DST_COUNT         12
#define SRC_COUNT         12
#define DelayPeriod       6
#else
#define DST_COUNT         1
#endif

const DXVA2_AYUVSample16 g_sBackground = { 0x8000, 0x8000, 0x0000, 0xffff };

CONST GUID DXVA2_Intel_FastCopy_Device = { 0x4b34d630, 0xd43c, 0x48a2, { 0x9a, 0xdd, 0xa3, 0xcf, 0x8f, 0xa2, 0xb4, 0x24 } };

// FastCopy Query Caps
typedef enum tagFASTCOPY_QUERYTYPE
{
	FASTCOPY_QUERY_REGISTRATION_HANDLE = 1,
} FASTCOPY_QUERYTYPE;

// FastCopy query caps structure
typedef struct tagFASTCOPY_QUERYCAPS
{
	FASTCOPY_QUERYTYPE Type;
	union
	{
		HANDLE                   hRegistration;
	};
} FASTCOPY_QUERYCAPS;

// Decoder extension Function Codes
typedef enum tagFASTCOPY_FUNCTION_ID
{
	FASTCOPY_BLT = 0x0100
} FASTCOPY_FUNCTION_ID;

// FastCopy Blt Parameters
typedef struct tagFASTCOPY_BLT_PARAMS
{
	void			*pSource;
	void			*pDest;
	BOOL			bPerformScaling;
	RECT			DstRect;	//Dest Rectangle
} FASTCOPY_BLT_PARAMS;
