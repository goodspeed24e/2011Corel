/************************************************************************/
/* Copyright (C) 2008 AMD Inc. All Rights Reserved. */
/* Filename: amddecext.h */
/* Content : header for AMD DXVA Decode Extension */
/************************************************************************/
#ifndef __AMD_DEC_EXT__
#define __AMD_DEC_EXT__
#define AMD_DEC_EXT_VERSION 10
/* h264 */
typedef enum
{
	BASELINE = 0,
	MAIN,
	HIGH,
	_TOO_BIG_
}AMD_H264_DEC_PROFILE;
#define amddecext_h264setprofile(x,ptr) ((ptr)->Reserved8BitsA |= ((x) << 6))
#define amddecext_h264setlevel(x,ptr) ((ptr)->Reserved8BitsA |= (x))
#define amddecext_h264setseek(x,ptr) ((ptr)->Reserved8BitsB |= ((x) << 1))
#define amddecext_h264setgap(x,ptr) ((ptr)->Reserved8BitsB |= (x))
#define amddecext_h264setextsupport(x,ptr) ((ptr)->Reserved8BitsB |= ((x) << 7))
#endif /* __AMD_DEC_EXT__ */
