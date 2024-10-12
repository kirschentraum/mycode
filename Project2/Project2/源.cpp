#include<afxwin.h>
#include<tchar.h>
#include<afxext.h>
#include"resource.h"

class CMyDoc : public CDocument
{
	
};

class CMyView : public CView
{
	DECLARE_DYNCREATE(CMyView)
public:
	virtual void OnDraw(CDC*);
};

IMPLEMENT_DYNCREATE(CMyView, CView)

void CMyView::OnDraw(CDC* pDC)
{
}



class CMyFrameWnd : public CFrameWnd
{

};

class CMyWinApp :public CWinApp
{
public:
	CMyWinApp(){}
	virtual BOOL InitInstance();
};

BOOL CMyWinApp::InitInstance()
{
	CMyFrameWnd* pFrame = new CMyFrameWnd;
	CMyDoc* pDoc = new CMyDoc;

	CCreateContext cct;
	cct.m_pCurrentDoc = pDoc;
	cct.m_pNewViewClass = RUNTIME_CLASS(CMyView);

	pFrame->LoadFrame(IDR_MENU1, WS_OVERLAPPEDWINDOW, NULL, &cct);
	m_pMainWnd = pFrame;
	pFrame->ShowWindow(SW_SHOW);
	return true;
}
CMyWinApp theApp;
