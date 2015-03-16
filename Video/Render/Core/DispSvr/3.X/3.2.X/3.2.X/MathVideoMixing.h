#ifndef _DISPSVR_MATH_VIDEOMIXING_H_
#define _DISPSVR_MATH_VIDEOMIXING_H_

namespace DispSvr
{

template<typename T> T clamp(T input, T min, T max)
{
	if (input <= min)
		return min;
	if (input >= max)
		return max;
	return input;
}

#define ValidateInputNRect(x)
#ifndef ValidateInputNRect
static inline void ValidateInputNRect(const NORMALIZEDRECT& NRect)
{
	ASSERT(NRect.left >= 0.f && NRect.left <= 1.f);
	ASSERT(NRect.right >= 0.f && NRect.right <= 1.f);
	ASSERT(NRect.top >= 0.f && NRect.top <= 1.f);
	ASSERT(NRect.bottom >= 0.f && NRect.bottom <= 1.f);
	ASSERT(NRect.right >= NRect.left && NRect.bottom >= NRect.top);
}
#endif

static inline bool __FloatEqual(float a, float b, int epsilon = 8)
{
	if (sizeof(float) == sizeof(int))
	{
		// use a == b to detect negative zero.
		return (a == b) || abs(*(int *)&a - *(int *)&b) <= epsilon;
	}
	else
	{
		return abs(a - b) < (0.00001f);
	}
}

static inline bool operator== (const NORMALIZEDRECT &a, const NORMALIZEDRECT &b)
{
	return __FloatEqual(a.top, b.top) && __FloatEqual(a.bottom, b.bottom)
		&& __FloatEqual(a.left, b.left) && __FloatEqual(a.right, b.right);
}

static inline bool operator!= (const NORMALIZEDRECT &a, const NORMALIZEDRECT &b)
{
	return !operator== (a, b);
}

static inline void IntersectNRect(NORMALIZEDRECT &out, const NORMALIZEDRECT &a, const NORMALIZEDRECT &b)
{
	bool bIntersected = !(a.left > b.right || a.right < b.left || a.top > b.bottom || a.bottom < b.top); 

	if (bIntersected)
	{
		out.left = max(a.left, b.left);
		out.top = max(a.top, b.top);
		out.right = min(a.right, b.right);
		out.bottom = min(a.bottom, b.bottom);
	}
	else
	{
		memset(&out, 0, sizeof(NORMALIZEDRECT));
	}
}

static inline void LoadDefaultMixerCoordinate(NORMALIZEDRECT &r)
{
	r.left = r.top = 0.f;
	r.right = r.bottom = 1.f;
}

static inline void LoadDefaultD3DCoordinate(NORMALIZEDRECT &r)
{
	r.left = r.bottom = -1.f;
	r.right = r.top = 1.f;
}

/// Convert D3D coordinate to mixer coordinate.
/// @param nrcMixer output
/// @param nrcD3D input D3D normalized rect, ranged from left top[-1, 1] to right bottom [1, -1].
static inline void ConvertD3DCoordToMixer(NORMALIZEDRECT &nrcMixer, const NORMALIZEDRECT &nrcD3D)
{
	nrcMixer.left = (1.0f + nrcD3D.left) / 2;
	nrcMixer.right = (1.0f + nrcD3D.right) / 2;
	nrcMixer.top = (1.0f - nrcD3D.top) / 2;
	nrcMixer.bottom = (1.0f - nrcD3D.bottom) / 2;
}

static inline void CalTexturePowerOf2Size(UINT &uWidth, UINT &uHeight)
{
	UINT w = 2;
	while (w < uWidth)
		w <<= 1;

	UINT h = 2;
	while (h < uHeight)
		h <<= 1;

	uWidth = w;
	uHeight = h;
}

/// Calculate normalized rect based on input rectangle and input clipping rectangle.
static inline void NormalizeRect(NORMALIZEDRECT &nrcOutput, const RECT &rc, const RECT &rcClip)
{
	FLOAT w = static_cast<FLOAT> (rcClip.right - rcClip.left);
	FLOAT h = static_cast<FLOAT> (rcClip.bottom - rcClip.top);

	nrcOutput.left = (rc.left - rcClip.left) / w;
	nrcOutput.right = 1 - (rcClip.right - rc.right) / w;
	nrcOutput.top = (rc.top - rcClip.top) / h;
	nrcOutput.bottom = 1 - (rcClip.bottom - rc.bottom) / h;
}

static inline void RoundUpTo2(LONG &v)
{
	v = (v + 1) & (-2);
}

/// Round up all components in a given rectangle to nearest even number.
/// @param rc input rectangle.
static inline void RoundUpTo2Rect(RECT &rc)
{
	RoundUpTo2(rc.left);
	RoundUpTo2(rc.right);
	RoundUpTo2(rc.top);
	RoundUpTo2(rc.bottom);
}

/// Convert NRect to according rectangle given.
/// @param rect input and output rectangle
/// @param NRect normalized rectangle ranged from 0 - 1.0
static inline void NRectToRect(RECT &rect, const NORMALIZEDRECT &NRect)
{
	ValidateInputNRect(NRect);

	LONG w = rect.right - rect.left;
	LONG h = rect.bottom - rect.top;

	rect.right = rect.left + static_cast<LONG> (ceil(NRect.right * w));
	rect.left = rect.left + static_cast<LONG> (NRect.left * w);
	rect.bottom = rect.top + static_cast<LONG> (ceil(NRect.bottom * h));
	rect.top = rect.top + static_cast<LONG> (NRect.top * h);
}

static inline void CropRect(NORMALIZEDRECT &rect, const NORMALIZEDRECT &CropRect)
{
	ValidateInputNRect(CropRect);

	float w = rect.right - rect.left;
	float h = rect.top - rect.bottom;

	rect.right = rect.left + CropRect.right * w;
	rect.left += CropRect.left * w;
	rect.bottom = rect.top - CropRect.bottom * h;
	rect.top -= CropRect.top * h;
}

static inline void CropRect(RECT &rect, const NORMALIZEDRECT &CropRect)
{
	NRectToRect(rect, CropRect);
}

/// Apply aspect ratio correction according to video aspect ratio and view port aspect ratio.
/// @param nrDst input destination rectangle and output corrected destination.
/// @param fVideoARByViewPortAR video aspect ratio / view port aspect ratio, video aspect ratio
///		is defined as width/height, view port aspect ratio is defined as video window width/height.
static inline void CorrectAspectRatio(NORMALIZEDRECT &nrcDst, float fVideoARByViewPortAR)
{
	ValidateInputNRect(nrcDst);

	if (fVideoARByViewPortAR <= 0 || __FloatEqual(fVideoARByViewPortAR, 1.0f))
		return;

	NORMALIZEDRECT nrcBox;
	LoadDefaultMixerCoordinate(nrcBox);

	// pillarboxing cases
	if (fVideoARByViewPortAR < 1.0f)
	{
		float dx = 0.5f * fVideoARByViewPortAR;
		nrcBox.left = 0.5f - dx;
		nrcBox.right = 0.5f + dx;
	}
	// letterboxing cases
	else
	{
		float dy = 0.5f / fVideoARByViewPortAR;
		nrcBox.top = 0.5f - dy;
		nrcBox.bottom = 0.5f + dy;
	}

	CropRect(nrcDst, nrcBox);
}

/// Clip destination and source rectangles whenever destination rectangle is
/// outside of destination clipping rectangle.
/// @param rcDst destination rectangle, must be unclipped.
/// @param rcSrc source rectangle.
/// @param rcDstClip destination clipping rectangle.
static inline void ClipRect(RECT &rcDst, RECT &rcSrc, const RECT &rcDstClip)
{
	LONG dstWidth = rcDst.right - rcDst.left;
	LONG dstHeight = rcDst.bottom - rcDst.top;
	LONG srcWidth = rcSrc.right - rcSrc.left;
	LONG srcHeight = rcSrc.bottom - rcSrc.top;

	ASSERT(dstWidth >= 0 && dstHeight >= 0 && srcWidth >= 0 && srcHeight >= 0);
	if (rcDstClip.left > rcDst.left)
	{
		rcSrc.left += LONG(float(rcDstClip.left - rcDst.left) / dstWidth * srcWidth);
		rcDst.left = rcDstClip.left;
	}

	if (rcDstClip.right < rcDst.right)
	{
		rcSrc.right -= LONG(float(rcDst.right - rcDstClip.right) / dstWidth * srcWidth + 0.5f);
		rcDst.right = rcDstClip.right;
	}

	if (rcDstClip.top > rcDst.top)
	{
		rcSrc.top += LONG(float(rcDstClip.top - rcDst.top) / dstHeight * srcHeight);
		rcDst.top = rcDstClip.top;
	}

	if (rcDstClip.bottom < rcDst.bottom)
	{
		rcSrc.bottom -= LONG(float(rcDst.bottom - rcDstClip.bottom) / dstHeight * srcHeight + 0.5f);
		rcDst.bottom = rcDstClip.bottom;
	}
}

}	// namespace DispSvr

#endif	// _DISPSVR_MATH_VIDEOMIXING_H_