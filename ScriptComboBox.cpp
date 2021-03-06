// ScriptComboBox.cpp : implementation file
//

#include "stdafx.h"
#include "SCIPicEditor.h"
#include "ScriptComboBox.h"
#include "ScriptDocument.h"
#include "ClassBrowser.h"
#include "Compile\ScriptOM.h"
#include "ScriptView.h"

using namespace sci;
using namespace std;

// CScriptComboBox

#define TIMER_UPDATECOMBO 1234

CScriptComboBox::CScriptComboBox()
{
    _pDoc = NULL;
    _fDroppedDown = false;
    _fIgnorePosChanged = false;

    // Load an image list.
    _imageList.Create(IDB_CLASSBROWSER, 16, 0, RGB(255, 0, 255));
}

CScriptComboBox::~CScriptComboBox()
{
}

void CScriptComboBox::UpdateNonView(LPARAM lHint)
{
    if (_pDoc && (lHint & VIEWUPDATEHINT_SCRIPTPOSCHANGED) && !_fIgnorePosChanged)
    {
        SetTimer(TIMER_UPDATECOMBO, 500, NULL);
    }
}

void CScriptComboBox::SetDocument(CDocument *pDoc)
{
    ASSERT(pDoc == NULL || pDoc->IsKindOf(RUNTIME_CLASS(CScriptDocument)));
    if (pDoc)
    {
        _pDoc = static_cast<CScriptDocument*>(pDoc);
        _pDoc->AddNonViewClient(this); // (assumes dupes are filtered out)

        // Don't update ourselves synchronously...
        SetTimer(TIMER_UPDATECOMBO, 500, NULL);
    }
    else
    {
        // Clear combobox
        ResetContent();
    }
}


BEGIN_MESSAGE_MAP(CScriptComboBox, CExtComboBox)
    ON_WM_TIMER()
    ON_CONTROL_REFLECT(CBN_SELCHANGE, OnSelChange)
    ON_CONTROL_REFLECT(CBN_CLOSEUP, OnCloseUp)
    ON_CONTROL_REFLECT(CBN_DROPDOWN, OnDropDown)
END_MESSAGE_MAP()


sci::Script *CScriptComboBox::_CreateScript(CPoint &pt)
{
    sci::Script *pScript = NULL;

    if (theApp._fBrowseInfo)
    {
        // NOTE: the document could have been closed, and the CDocument deleted.
        // Ensure that the active document is still us.
        CFrameWnd *pFrame = static_cast<CFrameWnd*>(AfxGetMainWnd());
        CFrameWnd *pMDIFrame = pFrame->GetActiveFrame();
        if (pMDIFrame)
        {
            CDocument *pDocActive = pMDIFrame->GetActiveDocument();
            if (_pDoc && (pDocActive == _pDoc))
            {
                CView *pView = pMDIFrame->GetActiveView();
                if (pView && pView->IsKindOf(RUNTIME_CLASS(CScriptView)))
                {
                    CScriptView *pScriptView = static_cast<CScriptView*>(pView);
                    pt = pScriptView->GetCursorPos();

                    SCIClassBrowser &browser = theApp.GetResourceMap().GetClassBrowser();
                    ClassBrowserLock lock(browser);
                    if (lock.TryLock()) // Don't block
                    {
                        // Do a full parse (e.g. don't ask for LKG)
                        // REVIEW: this causes a short pause when switching to a script.
                        pScript = ParseScript(_pDoc->GetScriptId(), _pDoc->GetTextBuffer());
                    }
                }
            }
        }
    }
    return pScript;
}

void CScriptComboBox::_ParseAndUpdate(bool fForce)
{
    if (_fDroppedDown)
    {
        // Don't do it while the user is looking at the combobox
        SetTimer(TIMER_UPDATECOMBO, 1000, NULL); // Try again in a second
    }
    else
    {
        CPoint pt;
        auto_ptr<sci::Script> script(_CreateScript(pt)); 
        // Don't update if we failed the parse - unless we're forced too.
        // The user may just be typing, and the parse will be failing if the thing isn't complete.
        // REVIEW: maybe we can change things so that we can still get a partially completed syntax
        // tree.
        if (script.get() || fForce)
        {
            _script = script;
            _OnUpdateFromScript(_script.get(), pt); // REVIEW: exception safety.
        }
    }
}

// CScriptComboBox message handlers
void CScriptComboBox::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == TIMER_UPDATECOMBO)
    {
        KillTimer(nIDEvent);
        // phil - disabled for now, crashes when typing a "
        //_ParseAndUpdate();
    }
    else
    {
        __super::OnTimer(nIDEvent);
    }
}

int CScriptComboBox::_AddItem(const std::string &str, sci::SyntaxNode *pPos)
{
    int iIndex = InsertString(-1, str.c_str());
    SetItemData(iIndex, reinterpret_cast<DWORD_PTR>(pPos));
    return iIndex;
}

void CClassComboBox::_OnUpdateFromScript(const sci::Script *pScript, CPoint pt)
{
    SetRedraw(FALSE);
    ResetContent();
    int iSelection = -1;
    if (pScript != NULL)
    {
        // Add all the classes, and select the one that the point is in.
        const sci::ClassVector &classes = pScript->GetClasses();
        sci::ClassVector::const_iterator classIt = classes.begin();
        while (classIt != classes.end())
        {
            int iIndex = _AddItem((*classIt)->GetName(), *classIt);
            if ((*classIt)->GetLineNumber() <= pt.y && (*classIt)->GetEndLineNumber() >= pt.y)
            {
                iSelection = iIndex;
            }

            // Put in the methods of this class too...
            const sci::MethodVector &methods = (*classIt)->GetMethods();
            sci::MethodVector::const_iterator methodIt = methods.begin();
            while (methodIt != methods.end())
            {
                std::string name = GetMethodTrackingName(*classIt, *methodIt);
                int iIndex = _AddItem(name, *methodIt);
                if ((*methodIt)->GetLineNumber() <= pt.y && (*methodIt)->GetEndLineNumber() >= pt.y)
                {
                    iSelection = iIndex; // This might override the class selection, but that's ok
                }
                ++methodIt;
            }

            ++classIt;
        }
        // Add the procedures too
        const sci::ProcedureVector &procs = pScript->GetProcedures();
        sci::ProcedureVector::const_iterator procIt = procs.begin();
        while (procIt != procs.end())
        {
            int iIndex = _AddItem((*procIt)->GetName(), *procIt);
            if ((*procIt)->GetLineNumber() <= pt.y && (*procIt)->GetEndLineNumber() >= pt.y)
            {
                iSelection = iIndex;
            }
            ++procIt;
        }
        if (iSelection != -1)
        {
            SetCurSel(iSelection);
        }
    }
    SetRedraw(TRUE);
    Invalidate();
}

void CClassComboBox::OnSelChange()//NMHDR *pnmhdr, LRESULT *plResult)
{
    int iSelected = GetCurSel();
    if (iSelected != CB_ERR)
    {
        CPoint pt;
        scoped_ptr<sci::Script> pScript(_CreateScript(pt));
        sci::SyntaxNode *pPos = reinterpret_cast<sci::SyntaxNode*>(GetItemData(iSelected));

        CFrameWnd *pFrame = static_cast<CFrameWnd*>(AfxGetMainWnd());
        CFrameWnd *pMDIFrame = pFrame->GetActiveFrame();
        if (pMDIFrame)
        {
            CDocument *pDocActive = pMDIFrame->GetActiveDocument();
            if (_pDoc && (pDocActive == _pDoc))
            {
                _fIgnorePosChanged = true; // We'll get a pos changed here, so ignore it, since we're the one who caused it.
                theApp.OpenScriptAtLine(_pDoc->GetScriptId(), pPos->GetLineNumber() + 1); // off by one
                _fIgnorePosChanged = false;
            }
        }
    }
}

void CClassComboBox::OnCloseUp()
{
    _fDroppedDown = false;
}

void CClassComboBox::OnDropDown()
{
    _fDroppedDown = true;
}

int _MapTypeToIndex(sci::SyntaxNode *pNode)
{
    switch (pNode->GetNodeType())
    {
    case sci::NodeTypeClassDefinition:
        return 0;
    case sci::NodeTypeFunction:
        {
            sci::FunctionBase *pFunction = static_cast<sci::FunctionBase*>(pNode);
            if (pFunction->GetOwnerClass())
            {
                return 2; // method
            }
            else
            {
                return 3; // procedure
            }
        }
    default:
        ASSERT(FALSE);
    }
    return 0;
}

void CScriptComboBox::DrawItem(LPDRAWITEMSTRUCT pDrawItemStruct)
{
    if (pDrawItemStruct->itemID != -1)
    {

        CDC dc;
        dc.Attach(pDrawItemStruct->hDC);    

        // Save these values to restore them when done drawing.
        COLORREF crOldTextColor = dc.GetTextColor();
        COLORREF crOldBkColor = dc.GetBkColor();
        int iMode = dc.GetBkMode();

        CString str;
        GetLBText(pDrawItemStruct->itemID, str);
        CRect rc = pDrawItemStruct->rcItem;

        sci::SyntaxNode *pNode = reinterpret_cast<sci::SyntaxNode*>(GetItemData(pDrawItemStruct->itemID));
        int iImageListIndex = _MapTypeToIndex(pNode);

        // Draw a little icon next to the guy.
        _imageList.Draw(&dc, iImageListIndex, CPoint(rc.left, rc.top), ILD_NORMAL);
        rc.left += 18;

        // Now draw the text
        if ((pDrawItemStruct->itemAction | ODA_SELECT) && (pDrawItemStruct->itemState & ODS_SELECTED))
        {
            // Show something for the selected it.
            dc.SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
            dc.SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));
            dc.FillSolidRect(&rc, ::GetSysColor(COLOR_HIGHLIGHT));
        }
        else
        {
            dc.FillSolidRect(&rc, crOldBkColor);
        }

        rc.left += 3; // Offset a little more.
        dc.DrawText(str, &rc, DT_SINGLELINE);

        dc.SetTextColor(crOldTextColor);
        dc.SetBkColor(crOldBkColor);
        dc.SetBkMode(iMode);
        dc.Detach();
    }
}

void CScriptComboBox::MeasureItem(LPMEASUREITEMSTRUCT lpMIStruct)
{
    //__super::MeasureItem(lpMIStruct);
    // CExtComboBoxBase::MeasureItem calls GetCount, which will crash on comctl32 v5, we get this
    // WM_MEASUREITEM message before WM_CREATE has finished (and the combo's listbox is not yet created)
    lpMIStruct->itemHeight = 15; // We're hard-coding this, but it makes the combobox look the best in prof-uis
}