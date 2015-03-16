#ifndef _DISPSVR_COLOR_SPACE_VIDEO_MIXING_H_
#define _DISPSVR_COLOR_SPACE_VIDEO_MIXING_H_

#define GetRValue_D3DCOLOR_XRGB(rgb)      (LOBYTE((rgb)>>16))
#define GetGValue_D3DCOLOR_XRGB(rgb)      (LOBYTE(((WORD)(rgb)) >> 8))
#define GetBValue_D3DCOLOR_XRGB(rgb)      (LOBYTE(rgb))

namespace DispSvr
{
    static inline D3DCOLOR AYUVSample8ToD3DCOLOR(const AYUVSample8 &ayuv)
    {
	    if (ayuv.Alpha > 0)
	    {
		    int c, d, e, r, g, b;
		    c = ayuv.Y - 16;
		    d = ayuv.Cb - 128;
		    e = ayuv.Cr - 128;
		    r = clamp((298 * c + 409 * e + 128) >> 8, 0, 255);
		    g = clamp((298 * c - 100 * d - 208 * e + 128) >> 8, 0, 255);
		    b = clamp((298 * c + 516 * d + 128) >> 8, 0, 255);

		    return D3DCOLOR_ARGB(ayuv.Alpha, r, g, b);
	    }
	    return 0;
    }

    static inline void ConvertArgbToAyuv16(COLORREF color, DXVA2_AYUVSample16 &ayuv16)
    {
	    UCHAR R = GetRValue(color);
	    UCHAR G = GetGValue(color);
	    UCHAR B = GetBValue(color);

	    double Y = max(min(0.257 * R + 0.504 * G + 0.098 * B + 16, 255), 0);
	    double U = max(min(-0.148 * R - 0.291 * G + 0.439 * B + 128, 255), 0);
	    double V = max(min(0.439 * R - 0.368 * G - 0.071 * B + 128, 255), 0);

	    ayuv16.Alpha = 0xffff;
	    ayuv16.Y = USHORT(0xff * Y);
	    ayuv16.Cb = USHORT(0xff * U);	// U
	    ayuv16.Cr = USHORT(0xff * V);	// V
    }

}	// namespace DispSvr

#endif  // _DISPSVR_COLOR_SPACE_VIDEO_MIXING_H_