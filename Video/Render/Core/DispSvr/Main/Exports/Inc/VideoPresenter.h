#ifndef _DISPSVR_VIDEO_PRESENTER_H_
#define _DISPSVR_VIDEO_PRESENTER_H_

MIDL_INTERFACE("63A7AC20-32EF-400c-9A5A-312B7C11C3E3") IDispSvrVideoPresenter;

namespace DispSvr
{
    enum PRESENTER_PROPERTY_FLAG
    {
        PRESENTER_PROPERTY_WAITUNTILPRESENTABLE = 1 << 0,
        /// Use/create additional swapchain when applicable.
        /// Note, when device is lost, the created swapchain is released and the flag is cancelled.
        PRESENTER_PROPERTY_USEADDITIONALSWAPCHAIN = 1 << 1,
        PRESENTER_PROPERTY_EXCLUSIVE_MODE = 1 << 2,
    };

    enum GAMUT_METADATA_FORMAT
    {
        GAMUT_METADATA_NONE = 0,
        GAMUT_METADATA_RANGE,
        GAMUT_METADATA_VERTICES
    };

    struct GamutMetadataRange
    {
        unsigned int Format_Flag : 1;
        unsigned int Rsvd : 2;
        unsigned int GBD_Color_Precision : 2;
        unsigned int GBD_Color_Space : 3;
        unsigned int Min_Red_Data : 12;
        unsigned int Max_Red_Data : 12;
        unsigned int Min_Green_Data : 12;
        unsigned int Max_Green_Data : 12;
        unsigned int Min_Blue_Data : 12;
        unsigned int Max_Blue_Data : 12;
        unsigned int reserved : 16;
    };

    struct GamutMetadataVertices
    {
        unsigned int Format_Flat : 1;
        unsigned int Facet_Mode : 1;
        unsigned int Rsvd : 1;
        unsigned int GBD_Color_Precision : 2;
        unsigned int GBD_Color_Space : 3;
        unsigned int Number_Vertices_H : 4;
        unsigned int Number_Vertices_L : 4;
        unsigned int Black_Y : 8;
        unsigned int Black_Cb : 8;
        unsigned int Black_Cr : 8;
        unsigned int Red_Y : 8;
        unsigned int Red_Cb : 8;
        unsigned int Red_Cr : 8;
        unsigned int Green_Y : 8;
        unsigned int Green_Cb : 8;
        unsigned int Green_Cr : 8;
        unsigned int Blue_Y : 8;
        unsigned int Blue_Cb : 8;
        unsigned int Blue_Cr : 8;
        unsigned int reserved : 16;
    };

    typedef enum CODEC_TYPE
    {
        CODEC_TYPE_NONE = 0,
        CODEC_TYPE_MPEG2,
        CODEC_TYPE_H264,
        CODEC_TYPE_VC1,
    } CODEC;

    enum VIDEO_DECODE_CAP
    {
        VIDEO_CAP_STREAM_SUB	= 1 << 0,	//< stream main or sub
        VIDEO_CAP_INTERLACE		= 1 << 1,	//< progressive or interlace content
        VIDEO_CAP_CODEC_MPEG2	= 1 << 2,	//< codec type : MPEG2
        VIDEO_CAP_CODEC_H264	= 1 << 3,	//< codec type : H264
        VIDEO_CAP_CODEC_VC1		= 1 << 4,	//< codec type : VC1
        VIDEO_CAP_FORMAT_480	= 1 << 5,	//< video format : 480
        VIDEO_CAP_FORMAT_576	= 1 << 6,	//< video format : 576
        VIDEO_CAP_FORMAT_720	= 1 << 7,	//< video format : 720
        VIDEO_CAP_FORMAT_1080	= 1 << 8	//< video format : 1080
    };

    struct PresenterProperty
    {
        DWORD dwSize;
        DWORD dwFlags;
        DWORD dwReserved[30];
    };

    struct PresenterCaps
    {
        DWORD dwSize;
        DWORD dwFPS;
        DWORD dwResPixels;
        DWORD dwBandWidth;
        BOOL bIsOverlay;
        BOOL bSupportXvYCC;
        BOOL bHwDecode;
        DWORD VideoDecodeCaps;
        BOOL bCanVirtualizeFromOrigin;
        BOOL bSupportFRUC;
        DWORD dwReserved[22];
    };

    enum PRESENT_VIDEO_FLAGS
    {
        VIDEO_PROGRESSIVE = 0,
        VIDEO_EVEN_FIELD_FIRST = 1 << 0,
        VIDEO_ODD_FIELD_FIRST = 1 << 1,
    };

    struct PresentHints
    {
        DWORD dwFrameRate;
        DWORD dwVideoFlags;
        BOOL bRepeatFirstField;
    };
}

interface IDispSvrVideoPresenter : public IUnknown
{
    STDMETHOD(SetDisplayRect)(const RECT *rcDst, const RECT *rcSrc) = 0;
    STDMETHOD(BeginRender)() = 0;
    STDMETHOD(EndRender)() = 0;
    STDMETHOD(Present)(const DispSvr::PresentHints *pHints) = 0;
    STDMETHOD(Clear)() = 0;
    STDMETHOD(SetProperty)(const DispSvr::PresenterProperty *pProperty) = 0;
    STDMETHOD(GetProperty)(DispSvr::PresenterProperty *pProperty) = 0;
    STDMETHOD(SetColorKey)(const DWORD dwColorKey) = 0;
    STDMETHOD(GetColorKey)(DWORD* pdwColorKey) = 0;
    STDMETHOD(QueryCaps)(DispSvr::PresenterCaps* pCaps) = 0;
    STDMETHOD(SetScreenCaptureDefense)(BOOL bEnable) = 0;
    STDMETHOD(SetGamutMetadata)(const DWORD dwFormat, void *pGamutMetadata) = 0;
};

#endif
