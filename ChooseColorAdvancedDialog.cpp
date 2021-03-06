// ChooseColorDialog.cpp : implementation file
//

#include "stdafx.h"
#include "SCIPicEditor.h"
#include "ChooseColorAdvancedDialog.h"


// CChooseColorDialog dialog

CChooseColorAdvancedDialog::CChooseColorAdvancedDialog(CWnd* pParent /*=NULL*/)
	: CExtResizableDialog(CChooseColorAdvancedDialog::IDD, pParent)
{
    _bChoice = 0;

    // 256 things here.
    m_wndStatic.SetRowsAndColumns(16, 16);
    m_wndStatic.ShowSelection(TRUE);

    // Fill palette.
    for (int color1 = 0; color1 < 16; color1++)
    {
        for (int color2 = 0; color2 < 16; color2++)
        {
            _allColorsPalette[color1 * 16 + color2].color1 = color1;
            _allColorsPalette[color1 * 16 + color2].color2 = color2;
        }
    }
    m_wndStatic.SetPalette(_allColorsPalette);
}

CChooseColorAdvancedDialog::~CChooseColorAdvancedDialog()
{
}

EGACOLOR CChooseColorAdvancedDialog::GetColor()
{
    EGACOLOR color;
    color.color1 = _bChoice >> 4;
    color.color2 = _bChoice & 0x0f;
    return color;
}

void CChooseColorAdvancedDialog::SetColor(EGACOLOR color)
{
    _bChoice = (color.color1 << 4) | color.color2;
    m_wndStatic.SetSelection(_bChoice);
}

void CChooseColorAdvancedDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
    ShowSizeGrip(FALSE);
    DDX_Control(pDX, IDC_CHOOSECOLORSTATIC, m_wndStatic);
    m_wndStatic.SetCallback(this);

    DDX_Control(pDX, IDC_DIALOGDESCRIPTION, m_wndDesc);
    DDX_Control(pDX, IDCANCEL, m_wndCancel);
}


BEGIN_MESSAGE_MAP(CChooseColorAdvancedDialog, CExtResizableDialog)
END_MESSAGE_MAP()


void CChooseColorAdvancedDialog::OnColorClick(BYTE bIndex, int nID, BOOL fLeftClick)
{
    if (fLeftClick)
    {
        // We only have one child - it must have been it.
        this->_bChoice = bIndex;
        EndDialog(IDOK);
    }
}

