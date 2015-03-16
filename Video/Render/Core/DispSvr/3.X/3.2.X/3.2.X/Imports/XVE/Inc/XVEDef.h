#pragma once
//////////////////////////////////////////////////////////////////////////
/// Video Effect Attribute GUID
//////////////////////////////////////////////////////////////////////////

// {500B4C20-ADF6-4727-B153-86E21A6F917A}
// PROPVARIANT type: VT_CLSID
// PROPVARIANT member : puuid
DEFINE_GUID(XVE_ATTR_RESOURCE_ID, 
            0x500b4c20, 0xadf6, 0x4727, 0xb1, 0x53, 0x86, 0xe2, 0x1a, 0x6f, 0x91, 0x7a);

// {5052DF2B-F8D3-473e-A3EF-92E705764A87}
// PROPVARIANT type: VT_UI4
// PROPVARIANT member : ulVal
DEFINE_GUID(XVE_ATTR_SUPPORTED_DEVICE, 
            0x5052df2b, 0xf8d3, 0x473e, 0xa3, 0xef, 0x92, 0xe7, 0x5, 0x76, 0x4a, 0x87);

// {5034F159-FFF0-488d-B76C-455011BA9F57}
// PROPVARIANT type: VT_UI4
// PROPVARIANT member : ulVal
DEFINE_GUID(XVE_ATTR_OUTPUT_BOUND, 
            0x5034f159, 0xfff0, 0x488d, 0xb7, 0x6c, 0x45, 0x50, 0x11, 0xba, 0x9f, 0x57);

// {50604A88-A9EF-4854-9AD6-2886A0A4B186}
// PROPVARIANT type: VT_UI4
// PROPVARIANT member : ulVal
DEFINE_GUID(XVE_ATTR_ENABLED, 
            0x50604a88, 0xa9ef, 0x4854, 0x9a, 0xd6, 0x28, 0x86, 0xa0, 0xa4, 0xb1, 0x86);

// {5082101C-7213-47e4-8249-AC083971696B}
// PROPVARIANT type: VT_UI4
// PROPVARIANT member : ulVal
// For Direct3D, use D3DVS_VERSION macro
DEFINE_GUID(XVE_ATTR_VERTEX_SHADER_VERSION, 
            0x5082101c, 0x7213, 0x47e4, 0x82, 0x49, 0xac, 0x8, 0x39, 0x71, 0x69, 0x6b);

// {5084C333-0D20-4312-BB4B-568A5AB5A63A}
// PROPVARIANT type: VT_UI4
// PROPVARIANT member : ulVal
// For Direct3D, use D3DPS_VERSION macro
DEFINE_GUID(XVE_ATTR_PIXEL_SHADER_VERSION, 
            0x5084c333, 0xd20, 0x4312, 0xbb, 0x4b, 0x56, 0x8a, 0x5a, 0xb5, 0xa6, 0x3a);

// {A7DD81B4-9333-41c8-85EF-3D876542564E}
// Get characteristics of multiple output effect.
// PROPVARIANT type: VT_UI4
// PROPVARIANT member: ulVal (DWORD OR'ed XVE_MULTIPLEOUTPUT_TYPE enumeration)
DEFINE_GUID(XVE_ATTR_MULTIPLE_OUTPUT, 
    0xa7dd81b4, 0x9333, 0x41c8, 0x85, 0xef, 0x3d, 0x87, 0x65, 0x42, 0x56, 0x4e);

//////////////////////////////////////////////////////////////////////////
/// Input Frame Data GUID
//////////////////////////////////////////////////////////////////////////

// {50D8B301-F793-4e39-8046-E70ABC86054D}
// PROPVARIANT type: VT_UNKNOWN
// PROPVARIANT member : punkVal
DEFINE_GUID(XVE_ATTR_FRAME, 
            0x50d8b301, 0xf793, 0x4e39, 0x80, 0x46, 0xe7, 0xa, 0xbc, 0x86, 0x5, 0x4d);

// {51926BBB-F3EA-46d9-A188-472B42527458}
// PROPVARIANT type: VT_UI8
// PROPVARIANT member : uhVal ((Width << 32) + Height)
DEFINE_GUID(XVE_ATTR_FRAME_SIZE, 
            0x51926bbb, 0xf3ea, 0x46d9, 0xa1, 0x88, 0x47, 0x2b, 0x42, 0x52, 0x74, 0x58);

// {5193B027-69E1-4e4c-9801-CA1C056E6546}
// PROPVARIANT type: VT_UI8
// PROPVARIANT member : uhVal ((X Offset << 32) + Y Offset)
DEFINE_GUID(XVE_ATTR_FRAME_OFFSET, 
            0x5193b027, 0x69e1, 0x4e4c, 0x98, 0x1, 0xca, 0x1c, 0x5, 0x6e, 0x65, 0x46);

// {519B088F-A898-44fc-84BF-A35EDB6BA84A}
// PROPVARIANT type: VT_UI8
// PROPVARIANT member : uhVal ((Numerator << 32) + Denominator)
DEFINE_GUID(XVE_ATTR_FRAME_ASPECT_RATIO, 
            0x519b088f, 0xa898, 0x44fc, 0x84, 0xbf, 0xa3, 0x5e, 0xdb, 0x6b, 0xa8, 0x4a);

// {51AA99B3-75A0-4a03-88A3-17360D95BF6A}
// PROPVARIANT type: VT_UI4
// PROPVARIANT member : ulVal
DEFINE_GUID(XVE_ATTR_FRAME_FORMAT, 
            0x51aa99b3, 0x75a0, 0x4a03, 0x88, 0xa3, 0x17, 0x36, 0xd, 0x95, 0xbf, 0x6a);

// {52A7C6BD-682E-4b57-9DB6-01B96BFBAD9F}
// PROPVARIANT type: VT_I8
// PROPVARIANT member : hVal
DEFINE_GUID(XVE_ATTR_FRAME_DURATION, 
            0x52a7c6bd, 0x682e, 0x4b57, 0x9d, 0xb6, 0x1, 0xb9, 0x6b, 0xfb, 0xad, 0x9f);

// {54953872-27AF-472a-92BD-B163168DF97B}
// PROPVARIANT type: VT_I8
// PROPVARIANT member : hVal
DEFINE_GUID(XVE_ATTR_FRAME_TIMESTAMP, 
            0x54953872, 0x27af, 0x472a, 0x92, 0xbd, 0xb1, 0x63, 0x16, 0x8d, 0xf9, 0x7b);

// {54A2C285-ECF1-4c25-9429-AD5963D8AB44}
// PROPVARIANT type: VT_UI4
// PROPVARIANT member : ulVal
DEFINE_GUID(XVE_ATTR_FRAME_INTERLACE_MODE, 
            0x54a2c285, 0xecf1, 0x4c25, 0x94, 0x29, 0xad, 0x59, 0x63, 0xd8, 0xab, 0x44);

// {54E2F125-DFBB-41e9-B165-103A229516EE}
// PROPVARIANT type: VT_UI4
// PROPVARIANT member : ulVal
DEFINE_GUID(XVE_ATTR_FRAME_FIELD_SELECT_MODE, 
            0x54e2f125, 0xdfbb, 0x41e9, 0xb1, 0x65, 0x10, 0x3a, 0x22, 0x95, 0x16, 0xee);

// {54F9FA32-16F5-4349-896D-01241951FBEF}
// returns XVE_FORMAT_TYPE
// PROPVARIANT type: VT_UI4
// PROPVARIANT member : ulVal
DEFINE_GUID(XVE_ATTR_FRAME_TYPE, 
            0x54f9fa32, 0x16f5, 0x4349, 0x89, 0x6d, 0x1, 0x24, 0x19, 0x51, 0xfb, 0xef);

// {550250CA-9CA7-461b-ACA8-22BDEB1F4B4E}
// PROPVARIANT type: VT_BYREF | VT_UNKNOWN
// PROPVARIANT member : ppunkVal
DEFINE_GUID(XVE_ATTR_PAST_FRAME, 
            0x550250ca, 0x9ca7, 0x461b, 0xac, 0xa8, 0x22, 0xbd, 0xeb, 0x1f, 0x4b, 0x4e);

// {550FAB7A-FF83-4240-A0C6-0B39CD7E0528}
// PROPVARIANT type: VT_UI4
// PROPVARIANT member : ulVal
DEFINE_GUID(XVE_ATTR_PAST_FRAME_COUNT, 
            0x550fab7a, 0xff83, 0x4240, 0xa0, 0xc6, 0xb, 0x39, 0xcd, 0x7e, 0x5, 0x28);

// {5637E1B4-CD74-4979-9EAB-0CE518CBF1AE}
// PROPVARIANT type: VT_BYREF | VT_UNKNOWN
// PROPVARIANT member : ppunkVal
DEFINE_GUID(XVE_ATTR_FUTURE_FRAME, 
            0x5637e1b4, 0xcd74, 0x4979, 0x9e, 0xab, 0xc, 0xe5, 0x18, 0xcb, 0xf1, 0xae);

// {56F979B0-EAEB-4676-BA36-7BFBC42EEB0D}
// PROPVARIANT type: VT_UI4
// PROPVARIANT member : ulVal
DEFINE_GUID(XVE_ATTR_FUTURE_FRAME_COUNT, 
            0x56f979b0, 0xeaeb, 0x4676, 0xba, 0x36, 0x7b, 0xfb, 0xc4, 0x2e, 0xeb, 0xd);

// {56FA3F1A-A640-4f9f-9CD1-4D3FE6A68C91}
// PROPVARIANT type: VT_UI8
// PROPVARIANT member : uhVal ((Width << 32) + Height)
DEFINE_GUID(XVE_ATTR_DISPLAY_SIZE, 
            0x56fa3f1a, 0xa640, 0x4f9f, 0x9c, 0xd1, 0x4d, 0x3f, 0xe6, 0xa6, 0x8c, 0x91);


//////////////////////////////////////////////////////////////////////////
/// Video Effect Type Extension GUID
//////////////////////////////////////////////////////////////////////////

// {5710F733-C5EE-41ad-8BAD-B023B48CD27C}
// PROPVARIANT type: VT_UI8
// PROPVARIANT member : uhVal ((Width << 32) + Height)
DEFINE_GUID(XVE_ATTR_EXTENSION_FRAME_SIZE, 
            0x5710f733, 0xc5ee, 0x41ad, 0x8b, 0xad, 0xb0, 0x23, 0xb4, 0x8c, 0xd2, 0x7c);

// {E6814CD9-CF9A-48ae-B38B-ED485BD8DDBF}
// Additional hint provided to video effect for view selection, especially in stereo case.
// There may not be the attribute of input sample and the effect may not respond accordingly.
// PROPVARIANT type: VT_UI4
// PROPVARIANT member : ulVal (DWORD view ID)
DEFINE_GUID(XVE_ATTR_EXTENSION_VIEW_ID,
	0xe6814cd9, 0xcf9a, 0x48ae, 0xb3, 0x8b, 0xed, 0x48, 0x5b, 0xd8, 0xdd, 0xbf);

// {9CDBAB40-4163-4ba0-A1AF-2C45A113061C}
// In normal case, the value should be identical to XVE_ATTR_FRAME_TIMESTAMP. In frame conversion case, the
// value is set to the target display timestamp which can be different from XVE_ATTR_FRAME_TIMESTAMP.
// The value can always be zero with older implementation.
// PROPVARIANT type: VT_UI8
// PROPVARIANT member : ulVal
DEFINE_GUID(XVE_ATTR_EXTENSION_DISPLAY_TIMESTAMP, 
    0x9cdbab40, 0x4163, 0x4ba0, 0xa1, 0xaf, 0x2c, 0x45, 0xa1, 0x13, 0x6, 0x1c);

// {E64C8C57-1206-4e23-A64B-2C6E883E23C0}
// An hint of the input sample contains a previously processed repeat frame.
// XVEMGR will skip intermediate effects if there is a generator effect or cache.
// PROPVARIANT type: VT_UI4
// PROPVARIANT member : ulVal (BOOL repeat frame)
DEFINE_GUID(XVE_ATTR_EXTENSION_REPEAT_FRAME, 
    0xe64c8c57, 0x1206, 0x4e23, 0xa6, 0x4b, 0x2c, 0x6e, 0x88, 0x3e, 0x23, 0xc0);

// {A186CAA3-4F27-4f38-8711-BCC10420E07A}
// The input resource is created on restricted content. If effect is to allocate internal resource manually,
// it should allocate any render target of the input accordingly as restricted content too.
// PROPVARIANT type: VT_UI4
// PROPVARIANT member : ulVal (BOOL repeat frame)
DEFINE_GUID(XVE_ATTR_EXTENSION_RESTRICTED_CONTENT, 
    0xa186caa3, 0x4f27, 0x4f38, 0x87, 0x11, 0xbc, 0xc1, 0x4, 0x20, 0xe0, 0x7a);
