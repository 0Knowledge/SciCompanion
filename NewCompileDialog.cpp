// CompileDialog.cpp : implementation file
//

#include "stdafx.h"
#include "SCIPicEditor.h"
#include "NewCompileDialog.h"
#include "TextResource.h"
#include "ScriptDocument.h"

#define UWM_STARTCOMPILE (WM_APP + 0)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CNewCompileDialog dialog

CNewCompileDialog::CNewCompileDialog(CWnd* pParent /*=NULL*/)
    : CExtResizableDialog(CNewCompileDialog::IDD, pParent), _headers(theApp.GetResourceMap())
{
    _fResult = FALSE;
    _fAbort = FALSE;
    _fDone = FALSE;
}

CNewCompileDialog::~CNewCompileDialog()
{
}


LRESULT CNewCompileDialog::CompileAll(WPARAM wParam, LPARAM lParam)
{
    ShowWindow(SW_SHOW);

    m_wndProgress.SetPos(_nScript);

    int nSuccessfulResults = 0;

    // Clear out the results from previous compiles.
    _log.Clear();
    int scriptsSize = static_cast<int>(_scripts.size());
    ASSERT(_nScript < scriptsSize);

    ScriptId &scriptId = _scripts[_nScript];
    // Update the edit control with the current scripts name.
    m_wndDisplay.SetWindowText(scriptId.GetTitle().c_str());

    // Now pump some messages, so the display updates
    // Read all of the messages in this next loop, 
    // removing each message as we read it.
    MSG msg; 
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE | PM_QS_PAINT | PM_QS_INPUT)) 
    { 
        // If it is a quit message, exit.
        ASSERT(msg.message != WM_QUIT);

        //  return 1; 
        // Otherwise, dispatch the message.
        DispatchMessage(&msg); 
    } // End of PeekMessage while loop.
    if (_fAbort)
    {
        _fDone = true;
        OnCancel();
    }
    else
    {
        // Do a compile
        NewCompileScript(_log, _tables, _headers, scriptId);

        // The compile is done.  Post the results.
        theApp.OutputAddBatch(_log.Results());
    }

    if (!_fAbort)
    {
        _nScript++;
        if (_nScript < (((int)_scripts.size()) - 1))
        {
            PostMessage(UWM_STARTCOMPILE, 0, 0); // Start another compile
        }
        else
        {
            // Change the text to close:
            SetDlgItemText(IDCANCEL, "Close");
            _fDone = true;
            // Actually, just close ourselves
            PostMessage(WM_CLOSE, 0, 0);
        }
    }
    return 0;
}

void CNewCompileDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_PROGRESS, m_wndProgress);
    DDX_Control(pDX, IDC_EDIT, m_wndDisplay);

    // Visuals
    DDX_Control(pDX, IDCANCEL, m_wndCancel);
}

BOOL CNewCompileDialog::OnInitDialog()
{
    BOOL fRet = __super::OnInitDialog();
    ShowSizeGrip(FALSE);
    _tables.Load(); // REVIEW: clean up
    theApp.GetResourceMap().GetAllScripts(_scripts);
    // Set the range of the progress control.
    m_wndProgress.SetRange32(0, (int)_scripts.size());
    _nScript = 0;
    PostMessage(UWM_STARTCOMPILE, 0, 0);
    return fRet;
}

void CNewCompileDialog::OnDestroy()
{
    // Do some reporting.
    std::stringstream str;
    str << _nScript << " scripts compiled.";
    _log.ReportResult(str.str());
    theApp.OutputAddBatch(_log.Results());

    // Save any tables...
    _tables.Save();

    __super::OnDestroy();
}


void CNewCompileDialog::OnCancel()
{
    if (!_fDone)
    {
        // We're still doing stuff.  Signal ourself to close.
        _fAbort = TRUE;
    }
    else
    {
        __super::OnCancel();
    }
}

BEGIN_MESSAGE_MAP(CNewCompileDialog, CExtResizableDialog)
    ON_MESSAGE(UWM_STARTCOMPILE, CompileAll)
    ON_WM_DESTROY()
END_MESSAGE_MAP()


// CNewCompileDialog message handlers
