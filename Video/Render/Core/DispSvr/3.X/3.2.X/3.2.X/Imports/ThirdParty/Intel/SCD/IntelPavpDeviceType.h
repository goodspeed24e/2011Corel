#ifndef INTEL_PAVP_DEVICE_TYPE
#define INTEL_PAVP_DEVICE_TYPE

// {7460004-7533-4E1A-B8E3-FF206BF5CE47} PAVP Service
DEFINE_GUID(DXVA2_Intel_Pavp, 0x7460004, 0x7533, 0x4e1a, 0xbd, 0xe3, 0xff, 0x20, 0x6b, 0xf5, 0xce, 0x47);

typedef enum
{
	PAVP_MEMORY_PROTECTION_NONE     = 0,
	PAVP_MEMORY_PROTECTION_LITE     = 1,
	PAVP_MEMORY_PROTECTION_STATIC   = 2,
	PAVP_MEMORY_PROTECTION_DYNAMIC_IMPLICIT  = 4,
	PAVP_MEMORY_PROTECTION_DYNAMIC_EXPLICIT  = 8
} PAVP_MEMORY_PROTECTION_MASK;

typedef enum
{
	PAVP_STATIC_MEMORY_0M       = 0,
	PAVP_STATIC_MEMORY_96M      = 1,
	PAVP_STATIC_MEMORY_128M     = 2
} PAVP_STATIC_MEMORY_SIZE_MASK;

typedef enum
{
	PAVP_KEY_EXCHANGE_CANTIGA     = 1,
	PAVP_KEY_EXCHANGE_EAGLELAKE	  = 2,
	PAVP_KEY_EXCHANGE_LARRABEE    = 4,
	PAVP_KEY_EXCHANGE_IRONLAKE    = 8,
	PAVP_KEY_EXCHANGE_DAA		  = 16,
} PAVP_KEY_EXCHANGE_MASK;

typedef struct tagPAVP_QUERY_CAPS
{
	UINT AvailableMemoryProtection;     // Indicates supported protection modes
	UINT AvailableKeyExchangeProtocols; // Indicates supported key exchange method
	UINT AvailableStaticMemorySizes;    // Indicates supported allocation sizes
	PAVP_MEMORY_PROTECTION_MASK     eCurrentMemoryMode; // only 1 bit set
	PAVP_STATIC_MEMORY_SIZE_MASK    eCurrentMemorySize; // only 1 bit set
} PAVP_QUERY_CAPS;

// Decoder extension Function Codes
typedef enum tagPAVP_FUNCTION_ID
{
	PAVP_KEY_EXCHANGE          = 0x200,
	PAVP_USE_PROTECTED_MEMORY  = 0x201,
	PAVP_GET_CONNECTION_STATE  = 0x202,
	PAVP_GET_FRESHNESS         = 0x203,
	PAVP_SET_FRESHNESS         = 0x204,
	PAVP_SET_WINDOW_POSITION   = 0x205		//< refer to SCD_SET_WINDOW_POSITION.
} PAVP_FUNCTION_ID;

// Device creation parameters
typedef struct tagPAVP_CREATE_DEVICE
{
	PAVP_KEY_EXCHANGE_MASK	eDesiredKeyExchangeMode;  // Only one bit should be set
} PAVP_CREATE_DEVICE, *PPAVP_CREATE_DEVICE;

// Fixed key exchange parameters
typedef struct tagPAVP_FIXED_EXCHANGE_PARAMS
{
	DWORD   FixedKey[4];
	DWORD   SessionKey[4];
} PAVP_FIXED_EXCHANGE_PARAMS;

// Use PAVP protected memory during allocation struct
typedef struct tagPAVP_USE_PROTECTED_MEMORY_PARAMS
{
	BOOL    bUseProtectedMemory;
} PAVP_USE_PROTECTED_MEMORY_PARAMS;

// PAVP get connection state struct
typedef struct tagPAVP_GET_CONNECTION_STATE_PARAMS
{
	DWORD   ProtectedMemoryStatus[4];
	DWORD   PortStatusType;
	DWORD   DisplayPortStatus[4];
} PAVP_GET_CONNECTION_STATE_PARAMS;

// PAVP get freshness struct
typedef struct tagPAVP_GET_FRESHNESS_PARAMS
{
	DWORD   Freshness[4];
} PAVP_GET_FRESHNESS_PARAMS;

#endif // INTEL_PAVP_DEVICE_TYPE
