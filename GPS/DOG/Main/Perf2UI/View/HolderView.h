#pragma once

// CHolderView view

//used to hold a window for splitting with a splitter
class CHolderView : public CView
{
	DECLARE_DYNCREATE(CHolderView)
public:
	void AttachWnd(CWnd* pWnd, CWnd* pOwner);	
	void SetOwner(CWnd* pOwner) 
	{
		ASSERT(pOwner);
		m_pOwner = pOwner;
	}

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

private:
	CWnd* m_pAttachedWnd; //the widow that is being holded
	CWnd* m_pOwner; //the owner of this control

protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnDraw(CDC*) {}

protected:
	CHolderView();           // protected constructor used by dynamic creation
	virtual ~CHolderView();	
};
