#pragma once

#include "NoFlickerStatic.h"
#include "NonViewClient.h"
#include "ViewResource.h"

// CSamplesDialogBar dialog bar
class CResourceListDoc;
class CSamplesDialogBar;


//
// Used to delay the calculation of the sample menu contents until the
// user actually clicks on the button.
//
class CExtDelayedButton : public CExtButton
{
public:
    void SetSamplesDialogBar(CSamplesDialogBar *pDlgBar) { _pDlgBar = pDlgBar; }
    virtual bool _OnTrackPopup(bool bSelectAny);
private:
    CSamplesDialogBar *_pDlgBar;
};

class CSamplesDialogBar : public CExtResizableDialog, public INonViewClient
{
public:
	CSamplesDialogBar(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSamplesDialogBar();

    void SetDocument(CDocument *pDoc);

    // INonViewClient
    virtual void UpdateNonView(LPARAM lHint);

// Dialog Data
	enum { IDD = IDD_QUICKSCRIPTS };

    void Initialize();
    void TrackPopup();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    BOOL OnInitDialog();
    void OnItemDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()
private:
    afx_msg void OnGotoView(UINT nID);
    afx_msg void OnUpdateAlwaysOn(CCmdUI *pCmdUI) { pCmdUI->Enable(TRUE); }

    void _ResetUI();
    void _PrepareSamples();
    void _PrepareViewMenu(int cItems);

    CExtDelayedButton m_wndButtonViews;
    std::vector<ResourceBlob> _samples;

    bool _bFirstTime;
    CResourceListDoc *_pDoc;
};
