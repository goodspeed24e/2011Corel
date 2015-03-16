#ifndef CChartData_h__
#define CChartData_h__

#include "structData.h"
#include "CPerfWnd.h"

class CChartData
{
public:
	enum ChartType
	{
		eCurveType,
		eDotType,
		//eStatusType,
		//eScrollTextType,
		eModeCount
	};

	enum DataType
	{
		STRING_MESSAGE_EVENT_TYPE,
		BUFFER_EVENT_TYPE,
		VARIANT_DATA_EVENT_TYPE,
		UNDEFINED
	};

	//Parent Wnd (PerfPainterWnd)
 	CPerfWnd* GetParentWnd() const {return m_parentWnd;}
 	void SetParentWnd(CPerfWnd* value){m_parentWnd = value;}

	//DogId
	const int& GetDogId() const {return m_DogId;}
	void SetDogId(int value){m_DogId = value;}

	//DogEventName
	const CString& GetDogEventName() const {return m_DogEventName;}
	void SetDogEventName(CString& value){m_DogEventName = value;}

	//ChartType
	const ChartType& GetChartType() const {return m_ChartType;}
	void SetChartType(ChartType value){m_ChartType = value;}

	//DataType ex:STRING, BUFFER, VARIANT, UNDEFINED
	const DataType& GetDataType() const {return m_DataType;}
	void SetDataType(DataType value){m_DataType = value;}
	
	CChartData();
	void Add(const structData& newElement);
	int RemoveAll();
	
	//return: total point count
	int GetCount() const;

	LONGLONG FirstTimestamp() const;
	LONGLONG LastTimestamp() const;
	POSITION GetHeadPosition() const;
    
    structData& GetAt(int index);
	void RemoveHead();

	//return: number of removed data
    int RemoveOverdue(LONGLONG timeStamp);
	
protected:
    CChartData(const CChartData& rhs);

private:
	int m_DogId;
	CString m_DogEventName;	//VIDEO_BITRATE_UPDATE_EVENT, DEMUX_VIDEO_EB_EVENT,...
	ChartType m_ChartType;	//curve, dot
	DataType m_DataType;	//STRING_MESSAGE_EVENT_TYPE, BUFFER_EVENT_TYPE, VARIANT_DATA_EVENT_TYPE
	CPerfWnd* m_parentWnd;

	CArray<structData, structData&> m_ChartData;
	mutable CRITICAL_SECTION m_cs;
};

#endif // CChartData_h__