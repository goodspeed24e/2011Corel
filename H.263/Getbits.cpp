// Getbits.cpp: implementation of the CGetbits class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "codedecoder.h"
#include "Getbits.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


extern CFile h263file;
extern ldecode *ld;
CGetbits::CGetbits()
{

}

CGetbits::~CGetbits()
{

}

static unsigned int msk[33] =
{
  0x00000000,0x00000001,0x00000003,0x00000007,
  0x0000000f,0x0000001f,0x0000003f,0x0000007f,
  0x000000ff,0x000001ff,0x000003ff,0x000007ff,
  0x00000fff,0x00001fff,0x00003fff,0x00007fff,
  0x0000ffff,0x0001ffff,0x0003ffff,0x0007ffff,
  0x000fffff,0x001fffff,0x003fffff,0x007fffff,
  0x00ffffff,0x01ffffff,0x03ffffff,0x07ffffff,
  0x0fffffff,0x1fffffff,0x3fffffff,0x7fffffff,
  0xffffffff
};  //最多可以取出32位比特数

void CGetbits::initbits()
{
  ld->incnt = 0;  //比特数为零
  ld->rdptr = ld->rdbfr + 2048;//把缓冲区的头地址赋给指针
  ld->bitcnt = 0;

}

void CGetbits::fillbfr()
{
 int l;

  ld->inbfr[0] = ld->inbfr[8];
  ld->inbfr[1] = ld->inbfr[9];
  ld->inbfr[2] = ld->inbfr[10];
  ld->inbfr[3] = ld->inbfr[11];

  if (ld->rdptr>=ld->rdbfr+2048)
  {
    l = h263file.Read(ld->rdbfr,2048);//装入字节
    ld->rdptr = ld->rdbfr;
    if (l<2048)
    {
      if (l<0)
        l = 0;
      while (l<2048)   /* Add recognizable sequence end code */
      {
        ld->rdbfr[l++] = 0;
        ld->rdbfr[l++] = 0;
        ld->rdbfr[l++] = (1<<7) | (SEC<<2);
      }
    }
  }

  for (l=0; l<8; l++)
    ld->inbfr[l+4] = ld->rdptr[l];

  ld->rdptr+= 8;
  ld->incnt+= 64;

}

unsigned int CGetbits::showbits(int n)
{
  unsigned char *v;
  unsigned int b;
  int c;

  if (ld->incnt<n)//缓存中的比特数(incnt)小于n
     fillbfr();  //载入比特数

  v = ld->inbfr + ((96 - ld->incnt)>>3);
  b = (v[0]<<24) | (v[1]<<16) | (v[2]<<8) | v[3];//取4个字节
  c = ((ld->incnt-1) & 7) + 25;
  return (b>>(c-n)) & msk[n];

}

unsigned int CGetbits::getbits(int n)
{
  unsigned int l;
  l = showbits(n);
  flushbits(n);
  return l;
}

unsigned int CGetbits::getbits1()
{
  return getbits(1);
}

void CGetbits::flushbits(int n)
{
  ld->bitcnt+= n;
  ld->incnt-= n;
  if (ld->incnt < 0)
    fillbfr();
}


