/*!
 **************************************************************************************
 * \file
 *    output.h
 * \brief
 *    Picture Information, Stream title rip, Time latency , writting routine headers.
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details) 
 *      - Delly Chen      <delly@intervideo.com.tw>
 ***************************************************************************************
 */
#ifndef _VC1DEBUG_H_
#define _VC1DEBUG_H_
#include <string>
#include "VC1TS.h"
#include "VC1VDecDef.h"
using namespace std ;

struct VC1VDec_Frame_;
struct VC1VDecGMO_Frame_;
struct _SEQHEADER;
struct _VC1_FrameProp;
class CVC1TS;
class CVC1MediaBuffer;

class VC1StreamDebug
{
public:
	VC1StreamDebug() {};
	~VC1StreamDebug() {};

	void open_recordFileName();
	void close_recordFile();

	void write_bitstream_frame(CVC1MediaBuffer*	pCVC1MediaBuffer, char* p_out);
	void write_bitstream_frame(const unsigned char *pucbitstreamframe, long lbitstreamframelength, char* p_out);
	void write_frame(const unsigned char *pucbitstreamframe, long lbitstreamframelength, char* p_out);

private:
	string m_sBitstreamWriteFile;
};

class VC1DecDebugUtility
{
public:
	static void DumpDecFrameInformation(long lFrameNum, _VC1_FrameProp* pVC1FrameProp);
	static void DumpOutFrameInformation(long lFrameNum, const VC1VDecParam::VC1VDec_Frame_* pFrame);
	static void DumpGMOFrameInformation(long lFrameNum, const VC1VDecParam::VC1VDecGMO_Frame_* pFrame);
	static void DumpSequenceInformation(const _SEQHEADER csSeqHeader);
	static void DumpFrameBufferInformation(long lFrameNum, CVC1MediaBuffer*	pCVC1MediaBuffer);
	static void DumpDemuxingBufferInformation(long lFrameNum, const unsigned char* pucFrameBuffer, long lFrameBufferLength);
	static void DumpPTSInformation(long lFrameNum, int iUpdatePTS, int iPreFramePTS, int iPreUpdatedPTS);
	static void DP(char *pcFormat, ...);

private:
	static CVC1TS  m_sVC1PrePTS;
	static CVC1TS  m_sVC1CurPTS;

};	

class VC1DecTimeDebugUtility
{
public:
	VC1DecTimeDebugUtility();
	~VC1DecTimeDebugUtility() {};

	void resetDecoderTime();
	void setRecordTime();
	void setDecoderDurTime(DWORD *dwpRecTime=NULL);	// real time latency for Decoder
	void setFrameBufferingTime(DWORD *dwpRecTime=NULL);	// time latency for frame buffering
	void setLine21FrameBufferingTime(DWORD *dwpRecTime=NULL);	// time latency for line21 frame buffering
	void setLine21FramePlayTime(DWORD *dwpRecTime=NULL);	// time latency for line21 frame playing
	inline void setSyncTime(int iRecTime=0) { m_iSyncTime=iRecTime;};

	inline DWORD getVC1DecoderBeginTime() { return m_dwVC1DecoderBeginTime; };
	inline DWORD getVC1DisplayBeginTime() { return m_dwVC1DisplayBeginTime; };
	inline DWORD getDecoderDurTime() { return m_dwDecoderDurTime; };
	inline DWORD getFrameBufferingTime() { return m_dwFrameBufferingTime; };
	inline DWORD getLine21FrameBufferingTime() { return m_dwLine21FrameBufferingTime; };
	inline DWORD getLine21FramePlayTime() { return m_dwLine21FramePlayTime; };
	inline int getSyncTime() { return m_iSyncTime; };


private:
	void getCurrentStartTime();
	void getCurrentEndTime();

private:
	DWORD m_dwRecordTime;
	DWORD m_dwVC1DecoderBeginTime;
	DWORD m_dwVC1DisplayBeginTime;

	DWORD m_dwDecoderDurTime;
	DWORD m_dwFrameBufferingTime;
	DWORD m_dwLine21FrameBufferingTime;
	DWORD m_dwLine21FramePlayTime;
	int		m_iSyncTime;


	LARGE_INTEGER m_sliStartFreq;
	LARGE_INTEGER m_sliEndFreq;
};	

#endif //_VC1DEBUG_H_

