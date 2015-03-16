#include <Windows.h>

//coclass GUIDs
// {97453fbc-3325-4911-ae03-0f6242019eca}
DEFINE_GUID(CLSID_GPUCP_PAVP, 
	0x97453fbc, 0x3325, 0x4911, 0xae, 0x03, 0x0f, 0x62, 0x42, 0x01, 0x9e, 0xca);

//interface GUIDS
// {9ff8e1ba-ea5f-4d0d-ada5-3d7f66435700}
DEFINE_GUID(IID_IGPUCPService, 
	0x9ff8e1ba, 0xea5f, 0x4d0d, 0xad, 0xa5, 0x3d, 0x7f, 0x66, 0x43, 0x57, 0x00);

// {9ff8e1ba-ea5f-4d0d-ada5-3d7f66435701}
DEFINE_GUID(IID_IGPUCPGetParams, 
	0x9ff8e1ba, 0xea5f, 0x4d0d, 0xad, 0xa5, 0x3d, 0x7f, 0x66, 0x43, 0x57, 0x01);
