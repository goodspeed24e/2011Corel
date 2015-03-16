#ifndef __GMOHELPER_H
#define __GMOHELPER_H

#include	"GMO_i.h"
#include	<exception>
namespace GMediaObjectHelper{
//
// Mediatype helpers.  MoInitMediaType() goes with MoFreeMediaType(), and
// MoCreateMediaType() goes with MoDeleteMediaType().  Don't mix them!
//

//
// Takes a pointer to an already allocated GMO_MEDIA_TYPE structure, allocates
// a format block of cbFormat bytes, and sets appropriate members of
// GMO_MEDIA_TYPE to point to the newly allocated format block.  Also
// initializes the IUnknown pointer inside GMO_MEDIA_TYPE to NULL.
//
// The format block allocated by MoInitMediaType must be freed by calling
// MoFreeMediaType().
//

HRESULT MoInitMediaType(GMO_MEDIA_TYPE *pmt, DWORD cbFormat);

//
// Frees the format block and releases any IUnknown, but does not free the
// GMO_MEDIA_TYPE structure itself.  Input parameter must point to an
// GMO_MEDIA_TYPE structure previously initialized by MoInitMediaType().
//
HRESULT MoFreeMediaType(GMO_MEDIA_TYPE *pmt);

//
// Copies the GMO_MEDIA_TYPE members.  Also duplicates the format block and
// the IUnknown pointer.  Both parameters must point to valid GMO_MEDIA_TYPE
// structures.  Target structure must be later freed using MoFreeMediaType().
//
HRESULT MoCopyMediaType(GMO_MEDIA_TYPE *pmtDest, const GMO_MEDIA_TYPE *pmtSrc);



//
// Allocates a new GMO_MEDIA_TYPE structure and initializes it just like
// MoInitMediaType.  I.e., this function allocates both the format block
// and the GMO_MEDIA_TYPE structure itself.  Pointer to GMO_MEDIA_TYPE is
// returned as *ppmt.
//
// GMO_MEDIA_TYPE structures allocated by MoCreateMediaType() must be freed
// by calling MoDeleteMediaType().
//
HRESULT MoCreateMediaType(GMO_MEDIA_TYPE **ppmt, DWORD cbFormat = 0);

//
// Frees any format block, releases any IUnknown, and deletes the
// GMO_MEDIA_TYPE structure itself.  The input parameter must point to an
// GMO_MEDIA_TYPE structure previously allocated by MoCreateMediaType().
//
HRESULT MoDeleteMediaType(GMO_MEDIA_TYPE *pmt);

//
// Allocates a new GMO_MEDIA_TYPE structure and copies pmtSrc into it like
// MoCopyMediaType.  I.e., this function allocates a new GMO_MEDIA_TYPE struct
// as well as a new format block for the target mediatype.  Trager mediatype
// must later be freed using MoDeleteMediaType().
//
HRESULT MoDuplicateMediaType(GMO_MEDIA_TYPE **ppmtDest, const GMO_MEDIA_TYPE *pmtSrc);

inline bool operator==(const GMO_MEDIA_TYPE& lhs, const GMO_MEDIA_TYPE& rhs);

inline HRESULT GMediaObjectHelper::MoInitMediaType(GMO_MEDIA_TYPE *pmt, DWORD cbFormat)
{
	if(!pmt)
		return E_POINTER;

	memset(pmt, 0, sizeof(*pmt));
	pmt->cbFormat = cbFormat;
	if ( cbFormat )
	{
		try
		{
			pmt->pbFormat = new BYTE[cbFormat];
		}
		catch (std::bad_alloc&)
		{
			pmt->cbFormat = 0;
			pmt->pbFormat = 0;
			return E_OUTOFMEMORY;
		}
	}
	else
		pmt->pbFormat = 0;
	return S_OK;
}
inline HRESULT GMediaObjectHelper::MoFreeMediaType(GMO_MEDIA_TYPE *pmt)
{
	if(!pmt)
		return E_POINTER;
	if(pmt->pbFormat)
	{
		delete [] pmt->pbFormat;
		pmt->pbFormat= 0;
	}
	if(pmt->pUnk)
		pmt->pUnk->Release();
	return S_OK;
}
inline HRESULT GMediaObjectHelper::MoCopyMediaType(GMO_MEDIA_TYPE *pmtDest, const GMO_MEDIA_TYPE *pmtSrc)
{
    //  We'll leak if we copy onto one that already exists - there's one
    //  case we can check like that - copying to itself.
    _ASSERT(pmtSrc != pmtDest);
// 	if ( pmtDest->pbFormat )
// 		delete [] pmtDest->pbFormat;
// 	if ( pmtDest->pUnk )
// 		pmtDest->pUnk->Release();

    *pmtDest = *pmtSrc;
    if (pmtSrc->cbFormat != 0)
	{
        _ASSERT(pmtSrc->pbFormat != NULL);
		try
		{
			pmtDest->pbFormat = new BYTE[pmtSrc->cbFormat];
		}
		catch (std::bad_alloc&)
		{
			pmtDest->cbFormat = 0;
			pmtDest->pbFormat = 0;
			pmtDest->pUnk = 0;
			return E_OUTOFMEMORY;
		}
		CopyMemory((PVOID)pmtDest->pbFormat, (PVOID)pmtSrc->pbFormat, pmtDest->cbFormat);
    }
	else
		pmtDest->pbFormat= 0;

    if (pmtDest->pUnk != NULL) {
        pmtDest->pUnk->AddRef();
    }

    return S_OK;
}
inline HRESULT GMediaObjectHelper::MoCreateMediaType(GMO_MEDIA_TYPE **ppmt, DWORD cbFormat)
{
	if ( !ppmt )
		return E_POINTER;

	GMO_MEDIA_TYPE *pmt = NULL;
	try
	{
		pmt = new GMO_MEDIA_TYPE;
	}
	catch (std::bad_alloc&)
	{
		return E_OUTOFMEMORY;
	}

	if ( FAILED( MoInitMediaType(pmt, cbFormat) ) )
	{
		delete pmt;
		return E_OUTOFMEMORY;
	}
	*ppmt = pmt;
	return S_OK;
}
inline HRESULT GMediaObjectHelper::MoDeleteMediaType(GMO_MEDIA_TYPE *pmt)
{
    if (pmt == NULL) {
        return E_POINTER;
    }
    return MoFreeMediaType(pmt);
}
inline HRESULT GMediaObjectHelper::MoDuplicateMediaType(GMO_MEDIA_TYPE **ppmtDest, const GMO_MEDIA_TYPE *pmtSrc)
{
	GMO_MEDIA_TYPE *pmt = NULL;
	HRESULT hr;
	hr = MoCreateMediaType(&pmt);
	if ( FAILED( hr ) )
		return hr;
	hr = MoCopyMediaType(pmt, pmtSrc);
	if ( FAILED( hr ) )
		return hr;

	*ppmtDest = pmt;
	return S_OK;
}

inline bool operator == (const GMO_MEDIA_TYPE& lhs, const GMO_MEDIA_TYPE& rhs)
{
	return (
		memcmp(&lhs, &rhs, sizeof(GMO_MEDIA_TYPE)-sizeof(BYTE*)) == 0 &&
		( (lhs.cbFormat == 0) || (memcmp(lhs.pbFormat, rhs.pbFormat,lhs. cbFormat) == 0))
		);
}

class CGMO_MEDIA_TYPE: public GMO_MEDIA_TYPE
{
public:
	CGMO_MEDIA_TYPE()
	{
		majortype= GUID_NULL;
		subtype= GUID_NULL;
		bFixedSizeSamples= false;
		bTemporalCompression= false;
		formattype= GUID_NULL;
		lSampleSize= 0;
		pbFormat= 0;
		cbFormat= 0;
		pUnk= 0;
	}
	~CGMO_MEDIA_TYPE()
	{
		if(pbFormat)
			delete [] pbFormat;
		if(pUnk)
			pUnk->Release();
	}
};
}; //namespace GMediaObjectHelper
#endif
