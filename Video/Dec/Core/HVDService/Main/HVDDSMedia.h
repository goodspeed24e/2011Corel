#pragma once

namespace HVDService
{
	class CHVDDSMedia
	{
	public:
		//static void WINAPI PrintGuid(const GUID &guid);
		//static void WINAPI DumpGuidToFile(char *pFName);
		//static void WINAPI PrintGuid(const GUID &guid, IVPrintStruct *pPrint);
		//static void WINAPI PrintMediaType(const AM_MEDIA_TYPE *pmt);
		//static void WINAPI PrintBitMapInfoHeader(const BITMAPINFOHEADER *bmiHeader);
		//static void WINAPI PrintVideoInfoHeader1(const VIDEOINFOHEADER *hdr);
		//static void WINAPI PrintVideoInfoHeader2(const VIDEOINFOHEADER2 *hdr);
		//static void WINAPI PrintMpeg1VideoInfo(const MPEG1VIDEOINFO *pvi);
		//static void WINAPI PrintMpeg2VideoInfo(const MPEG2VIDEOINFO *pvi);
		static int WINAPI GetMediaPtrs(const AM_MEDIA_TYPE *pmt, LPBITMAPINFOHEADER *lpbi, RECT **pSrc, RECT **pTgt);
		//static int WINAPI GetMPEGDimension(int *pwidth, int *pheight, const AM_MEDIA_TYPE *pmt);
		//static int WINAPI GetMediaDimension(int *pwidth, int *pheight, const AM_MEDIA_TYPE *pmt, PIN_DIRECTION pPinDir = PINDIR_INPUT);
		//static int WINAPI GetAspectRatio(int *px, int *py, const AM_MEDIA_TYPE *pmt);
		//static BOOL WINAPI IsAMControlUsed(const AM_MEDIA_TYPE *pmt);
		//static BOOL WINAPI IsOffsetTgt(const AM_MEDIA_TYPE *pmt);
		//static BOOL WINAPI IsCroppedSrc(const AM_MEDIA_TYPE *pmt);
		//static BOOL WINAPI IsSizeCompatible(const AM_MEDIA_TYPE *pNewMT, const AM_MEDIA_TYPE *pConnectedMT, PIN_DIRECTION pPinDir = PINDIR_INPUT);
		static BOOL WINAPI IsFourCC(const GUID &guid);
		static BOOL WINAPI IsRGB(const GUID &guid);
		static BOOL WINAPI IsYUV(const GUID &guid);
		static BOOL WINAPI IsDXVA(const GUID &guid);
		static BOOL WINAPI IsDXVA2Exclusive(const GUID &guid); // use only in DXVA2 mode.
		static BOOL WINAPI IsDXVAH264(const GUID &guid);
		static BOOL WINAPI IsDXVAVC1(const GUID &guid);
		//static BOOL WINAPI IsHVA(const GUID &guid);
		static BOOL WINAPI IsMPEG2(const GUID &guid);
		static BOOL WINAPI IsMPEG4(const GUID &guid);
		static BOOL WINAPI IsVC1(const GUID &guid);
		static BOOL WINAPI IsH264(const GUID &guid);
		//static FILTER_TYPE WINAPI FilterNameToFilterType(const WCHAR *wszFilterName);
	};
}