#ifndef _UTIL_DBG_H_
#define _UTIL_DBG_H_
//
// utildbg.hpp :
// DP() outputs debug printfs with the same format as printf().
// This function should be called to dump messages to
// an attached debugger or to the Perf monitor program.
//
#include <windows.h>
#include <stdio.h>

#pragma warning(disable: 4996)

#define MAX_VAR 1024
#if defined (_DEBUG)
#define DEBUG_SHOW_ERROR_INFO     DP	// For error debugger information
#define DEBUG_INFO                DP	// For gerneral module
#define DEBUG_SHOW_SW_INFO        DP	// For software module
#define DEBUG_SHOW_HW_INFO        DP	// For hardware module
#else
#define DEBUG_SHOW_ERROR_INFO     //DP	// For error debugger information
#define DEBUG_INFO                //DP	// For gerneral module
#define DEBUG_SHOW_SW_INFO        //DP	// For software module
#define DEBUG_SHOW_HW_INFO        //DP	// For hardware module
#endif
#define DEBUG_SHOW_SW_INFO_DETAIL //DP	// For hardware module  detail information 	
#define DP_QUEUE                  //DP  // For DXVA module debug
#define DEBUG_SHOW_HW_INFO_INTEL	//DP	// For hardware module, Intel debug information


#define OPEN_RECORD_FILE     //OpenRecordFile()  // For bitsteam ripper file open
#define WRITE_BITSTREAM      //WriteBistream     // For bitsteam ripper file writter
#define CLOSE_RECORD_FILE    //CloseRecordFile() // For Bitsteam ripper
#define DUMP_NVIDIA          //Dumpbuf_NVIDIA    // For dump DXVA buffer

#ifdef GLOBAL_INSTANCE
FILE* g_fpOutBitstream = NULL;
#else
extern FILE* g_fpOutBitstream;
#endif

static void DP(char *pcFormat, ...)
{
	char t_cBuffer[MAX_VAR]; 
	char* t_pcVal=NULL;
	int t_nWritten;

	va_start(t_pcVal, pcFormat);
	t_nWritten = _vsnprintf(t_cBuffer, MAX_VAR-1, pcFormat, t_pcVal);
	t_nWritten = t_nWritten>=0 ? t_nWritten : MAX_VAR-2;
	t_cBuffer[t_nWritten]='\n';
	t_cBuffer[t_nWritten+1]='\0';

	OutputDebugString(t_cBuffer);
}

static void OpenRecordFile()
{
	char *t_pcFileName = '\0';
	t_pcFileName = new char [128];
	SYSTEMTIME t_sLocalTime; 
	GetLocalTime(&t_sLocalTime);
	sprintf(t_pcFileName, "C:\\bitstream\\H264\\%.2d%.2d-%.2d%.2d%.2d.avc", t_sLocalTime.wMonth, t_sLocalTime.wDay, t_sLocalTime.wHour, t_sLocalTime.wMinute, t_sLocalTime.wSecond);
	
	if(g_fpOutBitstream==NULL)
		g_fpOutBitstream = fopen(t_pcFileName, "wb");

	delete [] t_pcFileName;
	t_pcFileName =NULL;
}

static void CloseRecordFile()
{
	if(g_fpOutBitstream)
		fclose(g_fpOutBitstream);

	g_fpOutBitstream=NULL;
}

static void WriteBistream(const unsigned char *pucbitstreamframe, long sizeofBistream, bool bStartCode)
{
	if(g_fpOutBitstream)
	{
		if(bStartCode)
			fprintf(g_fpOutBitstream, "%c%c%c%c", 0, 0, 0, 1);
		else
			fwrite(pucbitstreamframe, sizeof(unsigned char), sizeofBistream, g_fpOutBitstream);
	}
}

static void Dumpbuf_NVIDIA(BYTE*p, int size, int iframeCounter, char *name)
{
	char t_cFileName[256]; 
	FILE *fp;

	if(iframeCounter < 10)
		sprintf(t_cFileName, "c:\\dump\\00%d-%s",iframeCounter,name);
	else if(iframeCounter < 100)
		sprintf(t_cFileName, "c:\\dump\\0%d-%s",iframeCounter,name);
	else
		sprintf(t_cFileName, "c:\\dump\\%d-%s",iframeCounter,name);
	
	fp = fopen(t_cFileName,"ab");
	fwrite(p,1,size,fp);
	fclose(fp);
}
#endif
