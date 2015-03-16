#include "stdafx.h"
#include "CChartData.h"
#include <stdlib.h>
#include <crtdbg.h>
#include <math.h>

CChartData::CChartData()
{
	m_DogId = -1;
	m_DogEventName = _T("");
	m_ChartType = ChartType::eModeCount;
	m_DataType = DataType::UNDEFINED;
	InitializeCriticalSection(&m_cs);
}

void CChartData::Add(const structData& newElement)
{
	EnterCriticalSection(&m_cs);
	m_ChartData.Add(const_cast<structData&>(newElement));
	LeaveCriticalSection(&m_cs);
}

int CChartData::RemoveAll()
{
	int count = 0;
	EnterCriticalSection(&m_cs);
	count = m_ChartData.GetSize();
	m_ChartData.RemoveAt(0, count);
	LeaveCriticalSection(&m_cs);
	return count;
}

int CChartData::GetCount() const
{
	int count = 0;
	EnterCriticalSection(&m_cs);
	count = m_ChartData.GetSize();
	LeaveCriticalSection(&m_cs);
	return count;
}

LONGLONG CChartData::FirstTimestamp() const
{
	LONGLONG ret;
	EnterCriticalSection(&m_cs);
	ret = (m_ChartData.GetSize() > 0) ? m_ChartData[0].timeStamp : -1;
	LeaveCriticalSection(&m_cs);
    return ret;
}

LONGLONG CChartData::LastTimestamp() const
{
	LONGLONG ret;
	EnterCriticalSection(&m_cs);
	ret = (m_ChartData.GetSize() > 0) ? m_ChartData[m_ChartData.GetSize()-1].timeStamp : -1;
	LeaveCriticalSection(&m_cs);
	return ret;
}

POSITION CChartData::GetHeadPosition() const
{
	POSITION ret;
	EnterCriticalSection(&m_cs);
	ret = (m_ChartData.GetSize() > 0) ? (POSITION)1 : NULL;
	LeaveCriticalSection(&m_cs);
	return ret;
}

structData& CChartData::GetAt(int index)
{
	EnterCriticalSection(&m_cs);
    ASSERT(index < m_ChartData.GetSize());
	structData tmpData = m_ChartData[(int)index];
	LeaveCriticalSection(&m_cs);
    return tmpData;
}

void CChartData::RemoveHead()
{
	EnterCriticalSection(&m_cs);
	m_ChartData.RemoveAt(0);
	LeaveCriticalSection(&m_cs);
}

int CChartData::RemoveOverdue(LONGLONG timeStamp)	//current timeStamp also removed
{
	ASSERT(timeStamp > 0);
	int leftIndex = 0;

	//m_ChartData size may be increased after m_ChartData.GetCount()
	//it can only affected by "Add" function. So that's ok.
	int rightIndex = m_ChartData.GetCount()-1;
	int currentIndex = (leftIndex+rightIndex)/2;
	int removeCount = 0;

	if(timeStamp < m_ChartData[leftIndex].timeStamp)
		return 0;

	if(m_ChartData[rightIndex].timeStamp <= timeStamp)
	{
		removeCount = rightIndex+1;
		EnterCriticalSection(&m_cs);
		m_ChartData.RemoveAt(0, removeCount);	//removeCount: number of elements to remove
		LeaveCriticalSection(&m_cs);
		return removeCount;
	}

	while (leftIndex < rightIndex)
	{
		if(timeStamp < m_ChartData[currentIndex].timeStamp)
		{
			rightIndex = currentIndex-1;

			if(m_ChartData[rightIndex].timeStamp <= timeStamp)
			{
				removeCount = rightIndex+1;
				EnterCriticalSection(&m_cs);
				m_ChartData.RemoveAt(0, removeCount);	//removeCount: number of elements to remove
				LeaveCriticalSection(&m_cs);
				return removeCount;
			}
		}
		else //(m_ChartData[currentIndex].timeStamp <= timeStamp)
			leftIndex = currentIndex+1;

		currentIndex = (leftIndex+rightIndex)/2;
	}

	if (rightIndex < leftIndex)
		currentIndex = leftIndex;

	removeCount = currentIndex;

	//if rightIndex == leftIndex == currentIndex, should remove all elements before currentIndex(not included) 
	EnterCriticalSection(&m_cs);
	m_ChartData.RemoveAt(0, removeCount);	//rightIndex: number of elements to remove
	LeaveCriticalSection(&m_cs);
	
	return removeCount;
}