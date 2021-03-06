// ChooseColorDialog.cpp : implementation file
//

#include "stdafx.h"
#include "SCIPicEditor.h"
#include "ChoosePenStyleDialog.h"


// CChoosePenStyleDialog dialog

CChoosePenStyleDialog::CChoosePenStyleDialog(CWnd* pParent /*=NULL*/)
	: CBaseColorDialog(CChoosePenStyleDialog::IDD, pParent)
{
    _pPalette = NULL;
    _bChoice = 0;

    // 32 things here.
    m_wndStatic.SetRowsAndColumns(4, 8);
    m_wndStatic.ShowSelection(TRUE);
}

CChoosePenStyleDialog::~CChoosePenStyleDialog()
{
}

void CChoosePenStyleDialog::SetPenStyle(const PenStyle *pPenStyle)
{
    _bChoice = IndexFromPatternInfo(pPenStyle);
    _iUseRandomNR = pPenStyle->fRandomNR ? 1 : 0; // This will be bound to the check box.
    m_wndStatic.SetSelection(_bChoice);
}

void CChoosePenStyleDialog::GetPenStyle(PenStyle *pPenStyle)
{
    PatternInfoFromIndex(_bChoice, pPenStyle);
    pPenStyle->fRandomNR = _iUseRandomNR;
}

void CChoosePenStyleDialog::DoDataExchange(CDataExchange* pDX)
{
	CBaseColorDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_CHOOSEBRUSHSTATIC, m_wndStatic);
    m_wndStatic.SetCallback(this);
    DDX_Check(pDX, IDC_USERANDOMPATTERNS, _iUseRandomNR);

    m_wndStatic.SetSelection(_bChoice);

    DDX_Control(pDX, IDCANCEL, m_wndCancel);
    DDX_Control(pDX, IDC_STATIC1, m_wndStatic1);
    DDX_Control(pDX, IDC_USERANDOMPATTERNS, m_wndCheck);
}


BEGIN_MESSAGE_MAP(CChoosePenStyleDialog, CBaseColorDialog)
    ON_WM_KEYDOWN()
    ON_WM_KILLFOCUS()
    ON_WM_ACTIVATE()
END_MESSAGE_MAP()


void CChoosePenStyleDialog::OnColorClick(BYTE bIndex, int nID, BOOL fLeftClick)
{
    if (fLeftClick)
    {
        // We only have one child - it must have been it.
        _bChoice = bIndex;
        _fEnded = TRUE;

        // We need to call this manually, since we don't have an ok button.
        UpdateData(TRUE);

        EndDialog(IDOK);
    }
}

