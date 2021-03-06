// PicCommandDialogBar.cpp : implementation file
//

#include "stdafx.h"
#include "SCIPicEditor.h"
#include "PicCommandDialogBar.h"
#include "PicDoc.h"
#include "PicResource.h"

// PicCommandDialogBar dialog

PicCommandDialogBar::PicCommandDialogBar(CWnd* pParent /*=NULL*/)
	: CExtDialogFwdCmd(PicCommandDialogBar::IDD, pParent)
{
    _fAttached = FALSE;
    _iUserSelectedPos = LB_ERR;
    _iCurrentPicPos = LB_ERR;

    // Load our accelerators
	HINSTANCE hInst = AfxFindResourceHandle(MAKEINTRESOURCE(IDR_ACCELERATORPICCOMMANDS), RT_ACCELERATOR);
	_hAccel = ::LoadAccelerators(hInst, MAKEINTRESOURCE(IDR_ACCELERATORPICCOMMANDS));
}

PicCommandDialogBar::~PicCommandDialogBar()
{
}

#define STR_NOSCRIPT "no script"

void PicCommandDialogBar::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

    DDX_Control(pDX, ID_EDIT_COPY, m_wndCopy);
    DDX_Control(pDX, ID_CROPCOMMANDS, m_wndCrop);
    DDX_Control(pDX, ID_EDIT_DELETE, m_wndDelete);
    DDX_Control(pDX, IDC_GOTOSCRIPT, m_wndGotoScript);
    m_wndGotoScript.m_bUseStdCommandNotification = true;
    m_wndGotoScript.SetWindowText(STR_NOSCRIPT);
    m_wndGotoScript.EnableWindow(FALSE);

    DDX_Control(pDX, IDC_LISTCOMMANDS, m_wndList);
}


BEGIN_MESSAGE_MAP(PicCommandDialogBar, CExtDialogFwdCmd)
    ON_WM_DRAWITEM()
    ON_LBN_SELCHANGE(IDC_LISTCOMMANDS, OnSelChange)
    ON_COMMAND(ID_CROPCOMMANDS, OnCropCommands)
    // The pic document's ID_EDIT_COPY copies the pic background to the clipboard,
    // so we handle our own.
    // However, the pic document's ID_EDIT_PASTE handles commands on the clipboard.
    ON_COMMAND(ID_EDIT_COPY, OnCopyCommands)
    ON_COMMAND(ID_EDIT_DELETE, OnDeleteCommands)
    ON_COMMAND(ID_EDIT_CUT, OnCutCommands)
    ON_WM_CREATE()
    ON_COMMAND(IDC_GOTOSCRIPT, OnGotoScript)
END_MESSAGE_MAP()


// PicCommandDialogBar message handlers

void PicCommandDialogBar::OnCropCommands()
{
    // Delete from the current position to the end.
    GetDocument()->RemoveCommandRange(_iCurrentPicPos - 1, -1);
}

void PicCommandDialogBar::OnDeleteCommands()
{
    // Cut, don't copy
    _OnDelete(TRUE, FALSE);
}

void PicCommandDialogBar::OnCutCommands()
{
    // Cut, and copy
    _OnDelete(TRUE, TRUE);
}

void PicCommandDialogBar::OnCopyCommands()
{
    // Copy, don't cut
    _OnDelete(FALSE, TRUE);
}

void PicCommandDialogBar::_OnUpdateCommands()
{
    BOOL fItemsSelected = m_wndList.SendMessage(LB_GETSELCOUNT, 0, 0) > 0;
    m_wndDelete.EnableWindow(fItemsSelected);
    m_wndCopy.EnableWindow(fItemsSelected);
}

//
// To properly handle commands we do two things here:
// 1) Process our own accelerators
//      - The main frame's accelerator table doesn't get used in dialogs in control bars,
//        so we need to do our own.  So, e.g. Ctrl-Z becomes ID_EDIT_UNDO
// 2) Fwd any commands we receive to our frame
//      if we don't handle them ourselves.
//      - they don't bubble up naturally in dialogs in control bars.
//      - we do this by inheriting from CExtDialogFwdCmd
//
BOOL PicCommandDialogBar::PreTranslateMessage(MSG* pMsg)
{
    BOOL fRet = FALSE;
    if (_hAccel && (pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST))
    {
        fRet = ::TranslateAccelerator(GetSafeHwnd(), _hAccel, pMsg);
    }
    if (!fRet)
    {
        fRet = __super::PreTranslateMessage(pMsg);
    }
    return fRet;
}



BOOL PicCommandDialogBar::OnInitDialog()
{
    BOOL fRet = __super::OnInitDialog();

    // Set up anchoring for resize
    AddAnchor(IDC_LISTCOMMANDS, CPoint(0, 0), CPoint(100, 100));
    // Hide the sizing grip
    ShowSizeGrip(FALSE);
    return fRet;
}


void PicCommandDialogBar::OnSelChange()
{
    // Selection changed.  Time to move the position.
    const PicResource *pepic = _GetEditPic();
    if (pepic)
    {
        m_wndList.Invalidate(FALSE);
        _iUserSelectedPos = (int)m_wndList.SendMessage(LB_GETCURSEL, 0, 0);
        if (_iUserSelectedPos != LB_ERR)
        {
            if (_iUserSelectedPos > pepic->GetCommandCount())
            {
                _iUserSelectedPos = -1;
            }
            _pDoc->SeekToPos(_iUserSelectedPos);
        }
    }
}

//
// If fCut is true, the commands are deleted from the pic
// If fCopy is true, the commands are copied to the clipboard
//
void PicCommandDialogBar::_OnDelete(BOOL fCut, BOOL fCopy)
{
    const PicResource *pepic = _GetEditPic();
    if (pepic)
    {
        int cItems = (int)m_wndList.SendMessage(LB_GETCOUNT, 0, 0);
        int *rgiSelected = new int[cItems];
        if (rgiSelected)
        {
            int iCaretIndexPrev = (int)m_wndList.SendMessage(LB_GETCARETINDEX, 0, 0);
            int iCaretAdjustment = 0;

            int cSelectedItems = (int)m_wndList.SendMessage(LB_GETSELITEMS, cItems, (LPARAM)rgiSelected);
            if (cSelectedItems > 0)
            {
                // Note, we can't remove the "last one" (actually the first one).
                // Subtract 1 from the item index for every call

                // REVIEW: I don't think this will work too well if there are 
                // multiple ranges, since we'll keep getting selection changes, etc...
                // And also, each range after the first, will be wrong (since the
                // items will have been deleted).
                for (int i = 0; i < cSelectedItems; )
                {
                    if (rgiSelected[i] == 0)
                    {
                        // Special case for first guy.  If it's item 0, it doesn't count.
                        // Hopefully item 0 would only be included at the beginning.
                        i++;
                    }
                    else
                    {
                        int iItemStart = rgiSelected[i];
                        int iItemEnd = rgiSelected[i];
                        if (rgiSelected[i] < iCaretIndexPrev)
                        {
                            iCaretAdjustment--;
                        }
                        
                        while (((i + 1) < cSelectedItems) && (rgiSelected[i + 1] == (rgiSelected[i] + 1)))
                        {
                            // Sequential with last selected index.
                            i++;
                            iItemEnd = rgiSelected[i];

                            if (rgiSelected[i] < iCaretIndexPrev)
                            {
                                iCaretAdjustment--;
                            }
                        }

                        if (fCopy)
                        {
                            pepic->CopyRangeToClipboard(iItemStart - 1, iItemEnd - 1);
                        }

                        if (fCut)
                        {
                            GetDocument()->RemoveCommandRange(iItemStart - 1, iItemEnd - 1);
                        }

                        i++;
                    }
                }
            }

            if (fCut)
            {
                // Adjust the caret if we removed commands
                if (iCaretIndexPrev != LB_ERR)
                {
                    // Set selection to where the caret index was.
                    m_wndList.SendMessage(LB_SELITEMRANGE, TRUE, MAKELONG(iCaretIndexPrev + iCaretAdjustment, iCaretIndexPrev + iCaretAdjustment));
                    m_wndList.SendMessage(LB_SETCARETINDEX, iCaretIndexPrev + iCaretAdjustment, 0);
                }
            }
            delete [] rgiSelected;
        }
    }
}

void PicCommandListBox::DrawItem(DRAWITEMSTRUCT *pDrawItemStruct)
{
    PicCommandDialogBar *pParent = static_cast<PicCommandDialogBar*>(GetParent());
    CPicDoc* pDoc = pParent->GetDocument();
    ASSERT_VALID(pDoc);
    if (pDoc)
    {
        const PicResource *ppic = pDoc->GetPicResource();
        if (ppic)
        {
            ASSERT(pDrawItemStruct->CtlType == ODT_LISTBOX);
            CDC dc;
            dc.Attach(pDrawItemStruct->hDC);

            // REVIEW: We need to use this font when measuring too.
            HGDIOBJ hFontOld = dc.SelectObject(&g_PaintManager->m_FontNormalBC);

            // Save these values to restore them when done drawing.
            COLORREF crOldTextColor = dc.GetTextColor();
            COLORREF crOldBkColor = dc.GetBkColor();
            int iMode = dc.GetBkMode();

            // If this item is selected, set the background color 
            // and the text color to appropriate values. Also, erase
            // rect by filling it with the background color.
            BOOL bSelected = (pDrawItemStruct->itemAction | ODA_SELECT) && (pDrawItemStruct->itemState & ODS_SELECTED);
            if (bSelected)
            {
                dc.SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
                dc.SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));
                dc.FillSolidRect(&pDrawItemStruct->rcItem, ::GetSysColor(COLOR_HIGHLIGHT));
            }
            else
            {
                dc.FillSolidRect(&pDrawItemStruct->rcItem, crOldBkColor);
            }

            // If this item has the focus, draw a greenbackground around the
            // item's rect - unless it was selected.
            if (!bSelected && (pDrawItemStruct->itemAction | ODA_FOCUS) &&
                (pDrawItemStruct->itemState & ODS_FOCUS))
            {
                CBrush br(RGB(200, 255, 200));
                dc.FrameRect(&pDrawItemStruct->rcItem, &br);
            }

            // If this is the current pos, draw a caret.
            if (pDrawItemStruct->itemID == pParent->GetCurrentPosition())
            {
                // Draw the top of a "caret"
                int y = pDrawItemStruct->rcItem.bottom - 1;
                CPen br1(PS_SOLID, 1, RGB(0, 128, 0));
                HGDIOBJ hGdiObj = dc.SelectObject(&br1);
                dc.MoveTo(CPoint(pDrawItemStruct->rcItem.left, y));
                dc.LineTo(CPoint(pDrawItemStruct->rcItem.right - 1, y));

                y--;
                CPen br2(PS_SOLID, 1, RGB(0, 230, 0));
                dc.SelectObject(&br2);
                dc.MoveTo(CPoint(pDrawItemStruct->rcItem.left, y));
                dc.LineTo(CPoint(pDrawItemStruct->rcItem.right - 1, y));

                dc.SelectObject(hGdiObj);
            }
            else if (pDrawItemStruct->itemID == (pParent->GetCurrentPosition() + 1))
            {
                // FEATURE: Draw the bottom of a "caret"
            }


            if (pDrawItemStruct->itemID != -1)
            {
                TCHAR szName[256];
                if (pDrawItemStruct->itemID == 0)
                {
                    StringCchCopy(szName, ARRAYSIZE(szName), TEXT("Start"));
                }
                else
                {
                    // -1, since we have an additional entry at the top.
                    const PicCommand &command = ppic->GetCommandAt(pDrawItemStruct->itemID - 1);
                    ASSERT(command.type != PicCommand::None);
                    command.GetName(szName, ARRAYSIZE(szName));

                    if (!bSelected)
                    {
                        // For items that aren't selected, color them accordingly.
                        dc.SetTextColor(Command_GetColor(&command));
                    }
                }

                dc.SetBkMode(TRANSPARENT);

                // Draw the text.
                dc.DrawText(
                    szName,
                    -1,
                    &pDrawItemStruct->rcItem,
                    DT_CENTER | DT_SINGLELINE | DT_VCENTER);
            }

            // Reset the background color and the text color back to their
            // original values.
            dc.SelectObject(hFontOld);
            dc.SetTextColor(crOldTextColor);
            dc.SetBkColor(crOldBkColor);
            dc.SetBkMode(iMode);
            dc.Detach();
        }
    }
}

void PicCommandDialogBar::_UpdateItemCount()
{
    const PicResource *ppic = _GetEditPic();
    if (ppic)
    {
        // Store off the top index
        int iTopIndex = (int)m_wndList.SendMessage(LB_GETTOPINDEX, 0, 0);
        // Add one to the command count, so we have an item that represents "the beginning"
        m_wndList.SendMessage(LB_SETCOUNT, (WPARAM)ppic->GetCommandCount() + 1, 0);
        // Restore the top index.
        m_wndList.SendMessage(LB_SETTOPINDEX, iTopIndex, 0);
    }
}

void PicCommandDialogBar::UpdateNonView(LPARAM lHint)
{
    if (lHint & VIEWUPDATEHINT_NEWPIC)
    {
        // Remove all items
        m_wndList.SetRedraw(FALSE);
        m_wndList.SendMessage(LB_RESETCONTENT, 0, 0);
        _UpdateItemCount();
        m_wndList.SetRedraw(TRUE);
        // Selection changed.  Show a new pic.

        lHint |= VIEWUPDATEHINT_EDITPICPOSCHANGE; // New pic, so we'll update pos changed too.
    }

    if (lHint & VIEWUPDATEHINT_EDITPICINVALID)
    {
        // Something about the editpic changed (i.e. someone added a command)
        // Redraw ourselves.
        // But first - we need to figure out how commands there are.
        _UpdateItemCount();
        Invalidate(FALSE);
    }

    if (lHint & VIEWUPDATEHINT_EDITPICPOSCHANGE)
    {
        // The position of the editpic changed.  We should set our selected item.
        // Selection changed.  Show a new pic.
        const PicResource *ppic = _GetEditPic();
        if (ppic)
        {
            INT_PTR cCommands = ppic->GetCommandCount();
            INT_PTR iPos = _pDoc->GetDrawManager().GetPos();
            if (iPos == -1)
            {
                iPos = cCommands;
            }
            _iCurrentPicPos = (int)iPos;
            m_wndList.SendMessage(LB_SETCARETINDEX, iPos, 0);
            // We need to redraw everything... when the focus rect changes, the control
            // doesn't seem to send out a drawitem message for the item it was removed from.
            m_wndList.Invalidate(FALSE);
        }
    }
    _OnUpdateCommands();
}



const PicResource *PicCommandDialogBar::_GetEditPic()
{
    const PicResource *pepic = NULL;
    // Selection changed.  Show a new pic.
    CPicDoc* pDoc = GetDocument();
    if (pDoc)
    {
        pepic = pDoc->GetPicResource();
    }
    return pepic;
}

void PicCommandDialogBar::SetDocument(CDocument *pDoc)
{
    _pDoc = static_cast<CPicDoc*>(pDoc);
    UpdateNonView(VIEWUPDATEHINT_NEWPIC);
    if (_pDoc)
    {
        _pDoc->AddNonViewClient(this);

        // Update the script link.
        std::string text = "script: ";
        if (_pDoc->GetNumber() != -1)
        {
            text += theApp.GetResourceMap().FigureOutName(RS_SCRIPT, _pDoc->GetNumber());
            m_wndGotoScript.SetWindowText(text.c_str());
            m_wndGotoScript.EnableWindow(TRUE);
        }
        else
        {
            m_wndGotoScript.EnableWindow(FALSE);
        }
    }
}

void PicCommandDialogBar::OnGotoScript()
{
    theApp.OpenScript(_pDoc->GetNumber());
}