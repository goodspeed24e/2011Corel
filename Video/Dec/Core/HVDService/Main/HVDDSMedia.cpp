#include "stdafx.h"
#include <streams.h>
#include <dvdmedia.h>
#include <dxva.h>
#include "HVDGuids.h"
#include "HVDDSMedia.h"

using namespace HVDService;

BOOL CHVDDSMedia::GetMediaPtrs(const AM_MEDIA_TYPE *pmt, LPBITMAPINFOHEADER *lpbi, RECT **pSrc, RECT **pTgt)
{
	if(pmt->formattype==FORMAT_MPEG2Video)
	{
		MPEG2VIDEOINFO *pvi = (MPEG2VIDEOINFO *)pmt->pbFormat;
		if(pvi)
		{
			if(lpbi)
				*lpbi = &pvi->hdr.bmiHeader;
			if(pSrc)
				*pSrc = &pvi->hdr.rcSource;
			if(pTgt)
				*pTgt = &pvi->hdr.rcTarget;
			return TRUE;
		}
	}
	else if(pmt->formattype==FORMAT_MPEGVideo)
	{
		MPEG1VIDEOINFO *pvi = (MPEG1VIDEOINFO *)pmt->pbFormat;
		if(pvi)
		{
			if(lpbi)
				*lpbi = &pvi->hdr.bmiHeader;
			if(pSrc)
				*pSrc = &pvi->hdr.rcSource;
			if(pTgt)
				*pTgt = &pvi->hdr.rcTarget;
			return TRUE;
		}
	}
	else if(pmt->formattype==FORMAT_VideoInfo)
	{
		VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *)pmt->pbFormat;
		if(pvi)
		{
			if(lpbi)
				*lpbi = &pvi->bmiHeader;
			if(pSrc)
				*pSrc = &pvi->rcSource;
			if(pTgt)
				*pTgt = &pvi->rcTarget;
			return TRUE;
		}
	}
	else if(pmt->formattype==FORMAT_VideoInfo2)
	{
		VIDEOINFOHEADER2 *pvi = (VIDEOINFOHEADER2 *)pmt->pbFormat;
		if(pvi)
		{
			if(lpbi)
				*lpbi = &pvi->bmiHeader;
			if(pSrc)
				*pSrc = &pvi->rcSource;
			if(pTgt)
				*pTgt = &pvi->rcTarget;
			return TRUE;
		}
	}
	if(lpbi)
		*lpbi = 0;
	if(pSrc)
		*pSrc = 0;
	if(pTgt)
		*pTgt = 0;
	return FALSE;
}

BOOL CHVDDSMedia::IsFourCC(const GUID &guid)
{
	return memcmp((char *)&guid+4, (char *)&MEDIASUBTYPE_YUY2+4,12)==0;
}

BOOL CHVDDSMedia::IsRGB(const GUID &guid)
{
	return guid==MEDIASUBTYPE_RGB8   ||
		guid==MEDIASUBTYPE_RGB555 ||
		guid==MEDIASUBTYPE_RGB565 ||
		guid==MEDIASUBTYPE_RGB24  ||
		guid==MEDIASUBTYPE_RGB32;
}

BOOL CHVDDSMedia::IsYUV(const GUID &guid)
{
	return guid==MEDIASUBTYPE_YUY2   ||
		guid==MEDIASUBTYPE_YV12 || 
		guid==MEDIASUBTYPE_NV12;
}

BOOL CHVDDSMedia::IsDXVA(const GUID &guid)
{
	return  guid==DXVA_ModeMPEG1_A ||
		guid==DXVA_ModeMPEG2_A ||
		guid==DXVA_ModeMPEG2_B ||
		guid==DXVA_ModeMPEG2_C ||
		guid==DXVA_ModeMPEG2_D ||
		guid==DXVA_ModeMPEG2_VLD;
}

BOOL CHVDDSMedia::IsDXVA2Exclusive(const GUID &guid)
{
	return  guid==MEDIASUBTYPE_NV24 ||
		guid==MEDIASUBTYPE_IMC3 ||
		guid==MEDIASUBTYPE_IMC4;
}

BOOL CHVDDSMedia::IsDXVAH264(const GUID &guid)
{
	return guid==DXVA_ModeH264_VP1 ||
		guid==DXVA_ModeH264_ATI_A ||
		guid==DXVA_ModeH264_ATI_B ||
		guid==DXVA_ATI_BA_H264 ||
		guid==DXVA_ModeH264_E ||
		guid==DXVA_ModeH264_F;
}

BOOL CHVDDSMedia::IsDXVAVC1(const GUID &guid)
{
	return	guid==DXVA_ModeVC1_A ||
		guid==DXVA_ModeVC1_B ||
		guid==DXVA_ModeVC1_C ||
		guid==DXVA_ModeVC1_D;
}

//BOOL CHVDDSMedia::IsHVA(const GUID &guid)
//{
//	return IsDXVA(guid) || 
//		guid==MEDIASUBTYPE_TDMC;
//}

BOOL CHVDDSMedia::IsMPEG2(const GUID &guid)
{
	return guid==MEDIASUBTYPE_MPEG2_VIDEO;
}

BOOL CHVDDSMedia::IsMPEG4(const GUID &guid)
{
	return guid==MEDIASUBTYPE_DIVX;
}

BOOL CHVDDSMedia::IsVC1(const GUID &guid)
{
	return guid==MEDIASUBTYPE_VC1;
}

BOOL CHVDDSMedia::IsH264(const GUID &guid)
{
	return guid == MEDIASUBTYPE_BLH264 ||
		guid == MEDIASUBTYPE_MLH264 ||
		guid == MEDIASUBTYPE_HLH264 ||
		guid == MEDIASUBTYPE_ELH264; 
}