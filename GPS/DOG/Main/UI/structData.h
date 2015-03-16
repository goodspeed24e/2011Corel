#ifndef structData_h__
#define structData_h__

namespace UI
{
struct DogEvent
	{
		int dogId;
		TCHAR* pEventName;	//VIDEO_BITRATE_UPDATE_EVENT, DEMUX_VIDEO_EB_EVENT,...
		
		DogEvent()
		{
			dogId = 0;
			pEventName = NULL;
		}
	};
}

struct structData
{
// 	enum DataType
// 	{
// 		STRING_MESSAGE_EVENT_TYPE,
// 		BUFFER_EVENT_TYPE,
// 		VARIANT_DATA_EVENT_TYPE
// 	};
// 	DataType dataType;
	LONGLONG timeStamp;
	double maxValue;
	double value;
};

#endif // structData_h__
