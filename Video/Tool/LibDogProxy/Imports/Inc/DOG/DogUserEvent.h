#ifndef _DIAGNOSIS_OF_GPS_USER_EVENT_CLASS_H_
#define _DIAGNOSIS_OF_GPS_USER_EVENT_CLASS_H_ 

#include <windows.h>
#include "DOG.h"
#include "DogVariant.h"
#include "DogEventId.h"

#include <algorithm>

// namespace Diagnosis-of-GPS
namespace dog
{

enum DogEventTypeEnum
{
	BASIC_EVENT_TYPE = 0,       // DogEvent
	STRING_MESSAGE_EVENT_TYPE,  // StringEvent
	VARIANT_DATA_EVENT_TYPE,    // VariantDataEvent

	DATA_PROCESS_EVENT_TYPE = 100, // DataProcessEvent
	BUFFER_EVENT_TYPE,          // BufferEvent
	VIDEO_FRAME_EVENT_TYPE,     // VideoFrameEvent

	AUDIO_STREAM_INFO_UPDATE_EVENT_TYPE,// AudioStreamInfoUpdateEvent
	AUDIO_OUTPUT_UPDATE_EVENT_TYPE,     // AudioOutputUpdateEvent
	AUDIO_DEVICE_CAPABILITY_UPDATE_EVENT_TYPE, // AudioDevCapUpdateEvent
};

typedef LONGLONG REFERENCE_TIME;


class StringEvent : public DogEvent
{
public:
	static const size_t ReservedSize = 64;
	DWORD nBytesOfChar;
	DWORD nLength;
	union 
	{
		char szMessage[512-sizeof(DogEvent)-ReservedSize];
		wchar_t szMessageW[(512-sizeof(DogEvent)-ReservedSize)/2];
	};

	StringEvent(DWORD cat, DWORD subcat, DWORD id, const char* msg) : DogEvent(cat, subcat, id, STRING_MESSAGE_EVENT_TYPE, sizeof(StringEvent)), nBytesOfChar(1) 
	{
		nLength = (DWORD)strlen(msg);
		nLength = (std::min<DWORD>)(nLength, sizeof(szMessage)-1);
		strncpy_s(szMessage, sizeof(szMessage), msg, nLength);

		const size_t nSizeOfStringEvent = sizeof(DogEvent) + sizeof(DWORD)*2 + sizeof(char)*(nLength+1);
		Size = (std::min<DWORD>)((DWORD)nSizeOfStringEvent, sizeof(StringEvent));
	}
	StringEvent(DWORD cat, DWORD subcat, DWORD id, const wchar_t* msg) : DogEvent(cat, subcat, id, STRING_MESSAGE_EVENT_TYPE, sizeof(StringEvent)), nBytesOfChar(2)  
	{
		nLength = (DWORD)wcslen(msg);
		nLength = (std::min<DWORD>)(nLength, sizeof(szMessageW)/sizeof(wchar_t) -1);
		wcsncpy_s(szMessageW, sizeof(szMessageW)/sizeof(wchar_t), msg, nLength);

		const size_t nSizeOfStringEvent = sizeof(DogEvent) + sizeof(DWORD)*2 + sizeof(wchar_t)*(nLength+1);
		Size = (std::min<DWORD>)((DWORD)nSizeOfStringEvent, sizeof(StringEvent));
	}
};

class DebugMessageEvent : public StringEvent
{
public:
	DebugMessageEvent(DWORD cat, DWORD subcat, const char* msg) : StringEvent(cat, subcat, DEBUG_MESSAGE_EVENT, msg) {}
	DebugMessageEvent(DWORD cat, DWORD subcat, const wchar_t* msg) : StringEvent(cat, subcat, DEBUG_MESSAGE_EVENT, msg) {}
};

class ErrorMessageEvent : public StringEvent
{
public:
	ErrorMessageEvent(DWORD cat, DWORD subcat, const char* msg) : StringEvent(cat, subcat, ERROR_MESSAGE_EVENT, msg) {}
	ErrorMessageEvent(DWORD cat, DWORD subcat, const wchar_t* msg) : StringEvent(cat, subcat, ERROR_MESSAGE_EVENT, msg) {}
};

class FatalMessageEvent : public StringEvent
{
public:
	FatalMessageEvent(DWORD cat, DWORD subcat, const char* msg) : StringEvent(cat, subcat, FATAL_MESSAGE_EVENT, msg) {}
	FatalMessageEvent(DWORD cat, DWORD subcat, const wchar_t* msg) : StringEvent(cat, subcat, FATAL_MESSAGE_EVENT, msg) {}
};


class VariantDataEvent : public DogEvent
{
public:
	Variant Data;
	VariantDataEvent(DWORD cat, DWORD subcat, DWORD id, Variant data) 
		: DogEvent(cat, subcat, id, VARIANT_DATA_EVENT_TYPE, sizeof(VariantDataEvent)), Data(data) {}
};

class DataProcessEvent : public DogEvent
{
public:
	DWORD BufferSize; //current buffer data size in the process unit
	DWORD Capacity;   //capacity of the buffer of the process unit

	enum OperationEnum
	{
		UPDATE_STATUS = 0,
		DATA_IN,
		DATA_OUT,
	};
	DWORD Operation; //update, in, out

	REFERENCE_TIME DataPTS;
	REFERENCE_TIME DataDuration;
	DWORD DataSize;

	DataProcessEvent(DWORD cat, DWORD subcat, DWORD id, DWORD op, REFERENCE_TIME PTS, REFERENCE_TIME duration, DWORD datSize, DWORD bufSize, DWORD cap) 
		: DogEvent(cat, subcat, id, DATA_PROCESS_EVENT_TYPE, sizeof(DataProcessEvent)),
		Capacity(cap), BufferSize(bufSize), Operation(op), DataPTS(PTS), DataDuration(duration), DataSize(datSize) {}
};

class BufferEvent : public DataProcessEvent
{
public:
	BufferEvent(DWORD cat, DWORD subcat, DWORD id, DWORD op, REFERENCE_TIME PTS, REFERENCE_TIME duration, DWORD datSize, DWORD bufSize, DWORD cap) 
		: DataProcessEvent(cat, subcat, id, op, PTS, duration, datSize, bufSize, cap)
	{
		DataType = BUFFER_EVENT_TYPE;
	}
};

/// Video frame manager - display queue status
class VideoFrameEvent : public DogEvent
{
public:
	DWORD BufferIndex;
	DWORD VideoQueueSize;

	REFERENCE_TIME FramePTS;
	DWORD FrameIndex;

	enum FrameTypeEnum
	{
		I_FRAME = 0,
		P_FRAME,
		B_FRAME,
	};
	DWORD FrameType;  ///< I-frame, P-frame, B-frame

	enum FrameGroupEnum
	{
		FREE_FRAME = 0,
		DECODE_FRAME,
		DISPLAY_FRAME,
	};
	DWORD FrameGroup; ///< free, decode, display

	enum FrameOperationEnum
	{
		OP_NONE = 0,
		DECODING,
		DECODING_FORWARD_REF,
		DECODING_BACKWARD_REF,
		DISPLAYING,
		DISPLAY_DEINTERLACE_FORWARD_REF,
		DISPLAY_DEINTERLACE_BACKWARD_REF,
	};
	DWORD FrameOperation; ///< none, decoding, decoding reference, displaying, de-interlace reference

	VideoFrameEvent(DWORD cat, DWORD subcat, DWORD idx, DWORD qsize, REFERENCE_TIME PTS, DWORD frmIdx, DWORD type, DWORD grp, DWORD op ) 
		: DogEvent(cat, subcat, VIDEO_FRAME_EVENT, VIDEO_FRAME_EVENT_TYPE, sizeof(VideoFrameEvent)),
		BufferIndex(idx), VideoQueueSize(qsize), FramePTS(PTS), FrameIndex(frmIdx), FrameType(type), FrameGroup(grp), FrameOperation(op) {}		
};

class AudioStreamInfoUpdateEvent : public DogEvent
{
public:
	AudioStreamInfoUpdateEvent(DWORD cat, DWORD subcat, DWORD id, GUID streamType, DWORD ch, DWORD rate, DWORD bits, DWORD samplesPerFrm, DWORD bitRate)
		: DogEvent(cat, subcat, id, AUDIO_STREAM_INFO_UPDATE_EVENT_TYPE, sizeof(AudioStreamInfoUpdateEvent)),
		StreamType(streamType), Channels(ch), SamplesPerSec(rate), BitsPerSample(bits), SamplesPerFrame(samplesPerFrm), BitRate(bitRate) {}

	GUID  StreamType;
	DWORD Channels;
	DWORD SamplesPerSec;
	DWORD BitsPerSample;
	DWORD SamplesPerFrame;
	DWORD BitRate;
};

class AudioOutputUpdateEvent : public DogEvent
{
public:
	AudioOutputUpdateEvent(DWORD cat, DWORD subcat, GUID outType, DWORD ch, DWORD rate, DWORD bits, const char* info)
	  : DogEvent(cat, subcat, AUDIO_OUTPUT_UPDATE_EVENT, AUDIO_OUTPUT_UPDATE_EVENT_TYPE, sizeof(AudioOutputUpdateEvent)),
		  OutputType(outType), Channels(ch), SamplesPerSec(rate), BitsPerSample(bits) 
	{
		size_t nLength = strlen(info);
		nLength = (std::min)(nLength, sizeof(RendererInfo));
		strcpy_s(RendererInfo, sizeof(RendererInfo), info);
		RendererInfo[sizeof(RendererInfo)-1] = 0;
	}

	GUID  OutputType;
	DWORD Channels;
	DWORD SamplesPerSec;
	DWORD BitsPerSample;
	char RendererInfo[128];
};

class AudioDevCapUpdateEvent : public DogEvent
{
public:
	AudioDevCapUpdateEvent(DWORD cat, DWORD subcat, DWORD devType, DWORD ch, DWORD rate, DWORD bits, DWORD sprkConfig, DWORD encFormats)
		: DogEvent(cat, subcat, AUDIO_DEVICE_CAPABILITY_UPDATE_EVENT, AUDIO_DEVICE_CAPABILITY_UPDATE_EVENT_TYPE, sizeof(AudioDevCapUpdateEvent)),
		IviAdoDevType(devType), Channels(ch), SamplesPerSec(rate), BitsPerSample(bits), SpeakerConfig(sprkConfig), SupportedEncodedFormats(encFormats) {}

	DWORD IviAdoDevType;
	DWORD Channels;
	DWORD SamplesPerSec;
	DWORD BitsPerSample;
	DWORD SpeakerConfig;

	enum EncodedFormatEnum
	{
		ENCFORMAT_NONE = 0,
		ENCFORMAT_AC3 = 0x01,
		ENCFORMAT_DTS = 0x02,
		ENCFORMAT_AAC = 0x04,
		ENCFORMAT_DDPLUS = 0x08,
		ENCFORMAT_DTSHD  = 0x10,
		ENCFORMAT_TRUEHD = 0x20,
	};
	DWORD SupportedEncodedFormats;
};


} // namespace dog

#endif // _DIAGNOSIS_OF_GPS_USER_EVENT_CLASS_H_
