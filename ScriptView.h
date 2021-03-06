// SCICompilerView.h : interface of the CScriptView class
//


#pragma once

#include "crysedit/CCrystalEditView.h"
#include "Vocab000.h"
#include "IntellisenseListBox.h"
#include "ColoredToolTip.h"
#include "ToolTipResult.h"

class Script;
class CCrystalScriptStream;
class CScriptStreamLimiter;
class CMethodInfoTip;
class AutoCompleteThread;
class CScriptDocument;

class CScriptView : public CCrystalEditView, public IAutoCompleteClient
{
protected: // create from serialization only
	CScriptView();
	DECLARE_DYNCREATE(CScriptView)

// Attributes
public:
	CScriptDocument* GetDocument() const;
    void OnCompile();
    void HighlightLine(CPoint pt);
    int GetParenBalance(int nLineIndex);
    BOOL GetHeaderFile(CPoint pt, CString &strHeader);
    void GetSelectedText(CString &text);
    void SetAutoComplete(CIntellisenseListBox *pAutoComp, CColoredToolTip *pMethodToolTip, CColoredToolTip *pToolTip, AutoCompleteThread *pThread)
    {
        _pAutoComp = pAutoComp;
        _pACThread = pThread;
        _pMethodTip = pMethodToolTip;
        _pwndToolTip = pToolTip;
    }
// Operations
public:
// Overrides
	public:
    virtual void UpdateView(CCrystalTextView *pSource, CUpdateContext *pContext, DWORD dwFlags, int nLineIndex /*= -1*/);
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual void OnInitialUpdate();
    virtual CCrystalTextBuffer *LocateTextBuffer();

    // IAutoCompleteClient
    BOOL OnACDoubleClick();

    void AttachToAutoComp();
    void OnVisualScript();

protected:

// Implementation
public:
	virtual ~CScriptView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    virtual DWORD ParseLine(DWORD dwCookie, int nLineIndex, TEXTBLOCK *pBuf, int &nActualItems);
    virtual void UpdateCaret();
    virtual void ScrollPosChanged();

// Generated message map functions
    afx_msg void OnContextMenu(CWnd *pWnd, CPoint pos);
    afx_msg void OnActivate(UINT nState, CWnd *pWndOther, BOOL bMinimized);
    afx_msg void OnKillFocus(CWnd *pNewWnd);
    afx_msg void OnSetFocus(CWnd *pNewWnd);
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnInsertObject();
    afx_msg void OnAddAsNoun();
    afx_msg void OnAddAsImperativeVerb();
    afx_msg void OnAddAsPreposition();
    afx_msg void OnAddAsQualifyingAdjective();
    afx_msg void OnAddAsRelativePronoun();
    afx_msg void OnAddAsIndicativeVerb();
    afx_msg void OnAddAsAdverb();
    afx_msg void OnAddAsArticle();
    afx_msg void OnAddAsSynonymOf();
    afx_msg void OnGotoScriptHeader();
    afx_msg void OnGotoDefinition();
    afx_msg void OnIntellisense();
    afx_msg void OnGoto();
    afx_msg void OnUpdateAddAs(CCmdUI *pCmdUI);
    afx_msg void OnUpdateGotoScriptHeader(CCmdUI *pCmdUI);
    afx_msg void OnUpdateGotoDefinition(CCmdUI *pCmdUI);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnTimer(UINT nIDEvent);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);

protected:
	DECLARE_MESSAGE_MAP()

    void _OnAddAs(Vocab000::WordClass dwClass);
    BOOL _ScreenToWordRight(CPoint ptClient, CPoint &ptWordRight);
    ToolTipResult _DoToolTipParse(CPoint pt);
    void _BringUpToolTip(CPoint ptClient);
    BOOL _ClientToTextNoMargin(CPoint ptClient, CPoint &ptText);

    CString _contextMenuText;
    CString _gotoScriptText;

    CIntellisenseListBox *_pAutoComp;
    CColoredToolTip *_pMethodTip;
    AutoCompleteThread *_pACThread; // Not owned by us

    // Autocomplete
    CCrystalScriptStream *_pStream;
    CScriptStreamLimiter *_pLimiter;
    BOOL _fInOnChar;
    CPoint _ptAC;

    // We don't own these.
    HANDLE _hEventACDone;
    HANDLE _hEventDoAC;

    CColoredToolTip *_pwndToolTip;
    CPoint _ptLastMM;
    CPoint _ptToolTipWord;
    CFont m_Font;

    // For "goto definition"
    CString _gotoDefinitionText;
    ScriptId _gotoScript;
    int _gotoLineNumber;
};

#ifndef _DEBUG  // debug version in SCICompilerView.cpp
inline CScriptDocument* CScriptView::GetDocument() const
   { return reinterpret_cast<CScriptDocument*>(m_pDocument); }
#endif


