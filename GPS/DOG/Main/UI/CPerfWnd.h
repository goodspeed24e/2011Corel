#ifndef CPerfWnd_h__
#define CPerfWnd_h__


class CPerfWnd : public CFrameWnd
{
public:
	static const POINT m_WindowMinSize;
	static const POINT m_WindowMaxSize;
    void OnDestroy();
	void OnGetMinMaxInfo(MINMAXINFO* lpMMI);

protected:
	// Hide ctor that client code should not create CPerfWnd directly
    CPerfWnd();
    CPerfWnd(const CPerfWnd& rhs);
	DECLARE_MESSAGE_MAP()

private:

};

#endif // CPerfWnd_h__
