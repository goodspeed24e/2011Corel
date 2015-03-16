#pragma once

#include "EventCollection.h"
#include "DataSource.h"
#include "EventView.h"
#include <string>
#include <map>

// CChildView window

class CMessageView : public CWnd, public CEventView
{
	DECLARE_DYNCREATE(CMessageView)
	// Construction
public:
	CMessageView();
	virtual ~CMessageView();

public:
	virtual void SetDataSource(CEventCollection* pEventCollection);
	virtual void AddEvent(DWORD id, std::basic_string<TCHAR> friendlyName);
	virtual void RemoveEvent(DWORD id);
	virtual std::vector<DWORD> GetEventList();
	virtual void Clear();
	virtual void Update();

public:
	void  SetRefreshRate(DWORD milliseconds);
	DWORD GetRefreshRate();

protected:
	// Overrides
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual int  OnCreate(LPCREATESTRUCT lpCreateStruct);

	// Generated message map functions
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()

	CEventCollection* m_pEventCollection;

	CListCtrl m_MessageList;
	DWORD m_RefreshRate;

	struct DATA
	{
		DWORD id;
		DWORD eventType;
		std::basic_string<TCHAR> name;
		CDataSource* pDataSource;
	};
	std::map<DWORD, DATA> m_DataMap;

	inline void UpdateDataSource(DWORD id, CDataSource* pDataSource);
	inline void UpdateMessage();
	inline void AppendMessage(LONGLONG time, std::basic_string<TCHAR> message);

	enum 
	{
		TimerID = 1,
		DefaultRefreshRate = 200,
	};
};

