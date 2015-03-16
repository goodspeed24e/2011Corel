#pragma once


enum SAMPLE_FLAGS
{
	/// interlaced sample, cleared if video processed.
	SAMPLE_FLAG_INTERLACED				= 1 << 0,
	/// bottom field first sample.
	SAMPLE_FLAG_BOTTOMFIELDFIRST		= 1 << 1,
	/// the sample should repeat the first field.
	SAMPLE_FLAG_REPEATFIRSTFIELD		= 1 << 2,
	/// discontinity.
	SAMPLE_FLAG_DISCONTINUITY			= 1 << 3,
	/// field select
	SAMPLE_FLAG_SELECTSECONDFIELD		= 1 << 4,
    /// The frame is a repeat frame
    SAMPLE_FLAG_REPEAT_FRAME            = 1 << 5
};

enum DISPFRAMERATE_TYPE
{
	DISPFRAMERATE_NONE			= 1 << 0,           // init
	DISPFRAMERATE_24FPS         = 1 << 1,			// display 24 FPS
	DISPFRAMERATE_25FPS			= 1 << 2,			// display 25 FPS
	DISPFRAMERATE_30FPS			= 1 << 3,			// display 30 FPS
	DISPFRAMERATE_48FPS			= 1 << 4,			// display 48 FPS
	DISPFRAMERATE_50FPS			= 1 << 5,			// display 50 FPS
	DISPFRAMERATE_60FPS			= 1 << 6,			// display 60 FPS
	DISPFRAMERATE_120FPS        = 1 << 7,			// display 120 FPS
	DISPFRAMERATE_UNKNOWN		= 1 << 8			// Unknown display frame rate
};

/// VideoSource handles with sample containing video or processed image.
struct SampleProperty
{
	IUnknown *pSurface;
	UINT uWidth;
	UINT uHeight;
	FLOAT fAspectRatio;
	DWORD dwFlags;				//< bitwise OR of one or more SAMPLE_FLAGS
	REFERENCE_TIME rtStart;
	REFERENCE_TIME rtEnd;
	REFERENCE_TIME rtDisplayTarget;
	DWORD dwFrameRate1000;
};

struct FrameRateRatio
{
    DWORD Numerator;
    DWORD Denominator;
};

class CSampleProperty : public SampleProperty
{
public:
	CSampleProperty()
	{
		memset((SampleProperty *)this, 0, sizeof(SampleProperty));
	}
	~CSampleProperty()
	{
		if (pSurface)
		{
			pSurface->Release();
			pSurface = 0;
		}
	}
};

static inline void ReleaseSampleProperty(SampleProperty *pProperty)
{
    if (pProperty)
    {
        if (pProperty->pSurface)
        {
            pProperty->pSurface->Release();
            pProperty->pSurface = 0;
        }
    }
}

/// Internal interface to get more properties over original IDisplayVideoSource interface.
/// If using IDispSvrVideoMixer, we will need a way to retrieve video properties such as interlaced,
/// pts, duration, and so on. It should not be of interest by outside applications which use
/// IDisplayVideoSource interface.
MIDL_INTERFACE("860A3CC7-5BA5-4fae-BCC6-DC491FFAE391") IDisplayVideoSourcePropertyExtension : IUnknown
{
	/// A GetSampleProperty call to retrieve all information instead of GetTexture/GetVideoSize/etc multiple calls.
	/// Caller should either release the returned IUnknown interface or use ReleaseSampleProperty helper function
	/// to release returned property structure.
	STDMETHOD(GetSampleProperty)(SampleProperty *pProperty) = 0;
	STDMETHOD(SetDispFrameRate)(DWORD dwFrameRate) = 0;
};

MIDL_INTERFACE("867E1820-2E29-407a-A33C-C3E60B94368F") IDisplayVideoSourceFRCControl : IUnknown
{
	STDMETHOD(EnableFRC)(FrameRateRatio *pRatio) = 0;
};

/// Helper class for video source.
class CVideoSourceDisplayHelper
{
public:
	CVideoSourceDisplayHelper();
	~CVideoSourceDisplayHelper();

	HRESULT UpdateMaxDisplayFrameRate(UINT uImageWidth, UINT uImageHeight);
	BOOL IsOverDisplayFrameRate(LONGLONG hnsDuration) const;
	BOOL IsSkipFrameForVideoSync(LONGLONG hnsDuration, LONGLONG hnsDelta) const;
	static BOOL IsYUVFormat(DWORD dwFormat);

	void	SetDispFrameRateMode(DWORD dwDispFrameRate);
	HRESULT ApplyDispFrameRateMode(DWORD dwDispFrameRate);

	void SetDispFrameRate(DWORD dwFrameRate);
	void RestoreFrameRate();
private:
	DISPFRAMERATE_TYPE m_eCurrentDispFrameRateMode;
	DISPFRAMERATE_TYPE m_eDefaultDispFrameRateMode;

    DWORD m_dwMaxDisplayFrameRate;
    LONGLONG m_llMinDisplayDuration;

	DWORD m_dwDefaultMaxDisplayFrameRate;
};


