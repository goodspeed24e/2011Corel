// stdafx.cpp : source file that includes just the standard includes
//	视频编解码器.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

//picture specific data (picture header)
  int vbv_delay; //video buffering verifier delay (1/90000 seconds)
//图像数据 
//reconstructed frames  指针数组
  unsigned char *newrefframe[3], *oldrefframe[3];
//original frames
  unsigned char *neworgframe[3], *oldorgframe[3];
// prediction of current frame
  unsigned char *predframe[3];
// 8*8 block data
  short (*blocks)[64];//指向数组得指针

//宏块信息
  struct mbinfo *mbinfor;
//解码缓冲信息
  ldecode *ld,base;

//图象尺寸
  int block_count;  //宏块中得块数
  double frame_rate;   //frames per second 
  double bit_rate;     //bits per second
  int vbv_buffer_size; //size of VBV buffer (* 16 kbit)


