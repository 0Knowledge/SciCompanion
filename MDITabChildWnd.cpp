// MDITabChildWnd.cpp : implementation file
//

#include "stdafx.h"
#include "SCIPicEditor.h"
#include "MDITabChildWnd.h"
#include "MainFrm.h"


// CMDITabChildWnd

IMPLEMENT_DYNCREATE(CMDITabChildWnd, CMDIChildWnd)

CMDITabChildWnd::CMDITabChildWnd()
{
    _fModifiedCache = false;
    _fFirstTime = true;
    _fRemovedSelf = false;
}

CMDITabChildWnd::~CMDITabChildWnd()
{
    _RemoveSelf();
}


BEGIN_MESSAGE_MAP(CMDITabChildWnd, CMDIChildWnd)
    ON_MESSAGE_VOID(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
    ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_WM_NCDESTROY()
    ON_WM_MDIACTIVATE()
END_MESSAGE_MAP()

BOOL CMDITabChildWnd::PreCreateWindow(CREATESTRUCT &cs)
{
    cs.style |= WS_VISIBLE | WS_MAXIMIZE;
    return __super::PreCreateWindow(cs);
}

// CMDITabChildWnd message handlers

int CMDITabChildWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (__super::OnCreate(lpCreateStruct) == -1)
        return -1;

    // Unfortunately, we can't update our title until we have a document -
    // which we do not have yet.  So there will be a little visual glitch as
    // the tab is added, and *then* gets it title.
    CMainFrame *pFrame = static_cast<CMainFrame*>(AfxGetMainWnd());
    if (pFrame)
    {
        pFrame->AddTab(this, GetTabType());
    }
    return 0;
}


void CMDITabChildWnd::_RemoveSelf()
{
    if (!_fRemovedSelf)
    {
        CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
        if (pFrame)
        {
            pFrame->RemoveTab(this);
            _fRemovedSelf = true;
        }
    }
}

void CMDITabChildWnd::OnDestroy()
{
    _RemoveSelf();
    __super::OnDestroy();
}

void CMDITabChildWnd::OnMDIActivate(BOOL fActivate, CWnd *pActivate, CWnd *pDeactivate)
{
    __super::OnMDIActivate(fActivate, pActivate, pDeactivate);
    if (fActivate)
    {
        CMainFrame *pFrame = static_cast<CMainFrame*>(AfxGetMainWnd());
        if (pFrame)
        {
            ASSERT(pActivate->IsKindOf(RUNTIME_CLASS(CFrameWnd)));
            //pFrame->Tabs().OnActivateTab(static_cast<CFrameWnd*>(pActivate));
            pFrame->OnActivateTab(this);
        }
    }
}

// For title updating.
void CMDITabChildWnd::OnIdleUpdateCmdUI()
{
     OnUpdateFrameTitle(TRUE);
    __super::OnIdleUpdateCmdUI();
}

// Append an asterisk to the title if the document is modified.
void CMDITabChildWnd::_UpdateFrameTitle(CDocument *pDocument)
{
    if (pDocument)
    {
        std::string title = (PCSTR)pDocument->GetTitle();
        if (pDocument->IsModified())
        {
            title += " *";
        }
        CString currentText;
        GetWindowText(currentText);
        if (title != (PCSTR)currentText)
        {
            // set title if changed, but don't remove completely
            SetWindowText(title.c_str());
            // Prof-UIS bug - window title does not repaint immediately when using CExtNCW<CMDIFrameWnd>
            AfxGetMainWnd()->SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_DRAWFRAME);
        }
    }
}

void CMDITabChildWnd::OnUpdateFrameTitle(BOOL bAddToTitle)
{
    // Don't call super - all it does it take the document title and put it there.
    CDocument* pDocument = GetActiveDocument();
    if (bAddToTitle && pDocument)
    {
        bool fModified = (pDocument->IsModified() == TRUE);
        if (_fFirstTime || (_fModifiedCache != fModified))
        {
            _fModifiedCache = (pDocument->IsModified() == TRUE);
            _UpdateFrameTitle(pDocument);
            _fFirstTime = true;
        }
    }
}


