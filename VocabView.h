#pragma once

#include "Vocab000.h"

// CVocabView view
class CVocabDoc;
class CFindWordDialog;

class CVocabView : public CListView
{
	DECLARE_DYNCREATE(CVocabView)

protected:
	CVocabView();           // protected constructor used by dynamic creation
	virtual ~CVocabView();

public:
    CVocabDoc* GetDocument() const;
    Vocab000* GetVocab() const;

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
    virtual void OnUpdate(CView *pSender, LPARAM lHint, CObject *pHint);
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual void OnInitialUpdate();
    BOOL PreTranslateMessage(MSG *pMsg);

protected:
    void OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
    void OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
    void OnItemDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    void OnTimer(UINT_PTR nIDEvent);
    void OnUpdateWordIndicator(CCmdUI *pCmdUI);
    void OnContextMenu(CWnd *pWnd, CPoint point);
    void OnVocabCommand(UINT nID);
    void OnDelete();
    void OnUpdateVocabCommand(CCmdUI *pCmdID);
    void OnUpdateIfSelection(CCmdUI *pCmdUI);
    void OnUpdateAlwaysOn(CCmdUI *pCmdUI) { pCmdUI->Enable(); }
    void OnNewWord();
    void OnRename();
    void OnFind();
    afx_msg LRESULT OnFindDialogMessage(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

    static int CALLBACK s_SortListByGroup(LPARAM lpOne, LPARAM lpTwo, LPARAM lpArg);

    void _InitColumns();
    void _ChangeView();
    void _InsertItem(int iItem, Vocab000::WordGroup dwGroup, Vocab000::WordClass dwClass, const std::string &strWords);
    void _UpdateItemInfo(int nItem, DWORD dwInfo);
    int _GetSelectedItem();
    void _SelectGroup(DWORD dwInfo);
    HRESULT _ParseViewString(PCTSTR pszString, CArray<CString, PCTSTR> &words);
    void _OnEndEditingNewItem(PCTSTR pszNewWords, NMLVDISPINFO *plvdi);

    int _iView;
    bool _bInLabelEdit;
    bool _bEditingNewItem;
    CString _strMatchText;
    CArray<CString, PCTSTR> _preEditWords;

    CFindWordDialog *_pFindDialog;
};



#ifndef _DEBUG  // debug version in VocabView.cpp
inline CVocabDoc* CVocabView::GetDocument() const
   { return reinterpret_cast<CVocabDoc*>(m_pDocument); }
#endif
