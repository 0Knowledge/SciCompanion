#pragma once


// CResourceListCtrl view


// FWD decl
class CResourceListDoc;
class ResourceBlob;

typedef void(* PFNRESOURCEOPEN )(const ResourceBlob *pData);

#define OVERLAYINDEX_UNUSED 1

class CResourceListCtrl : public CListCtrl
{
	DECLARE_DYNCREATE(CResourceListCtrl)

public:
	CResourceListCtrl();
	virtual ~CResourceListCtrl();
    CResourceListDoc* GetDocument() const; // non-debug version is inline
    void SetDocument(CResourceListDoc *pDocument) { m_pDocument = pDocument; }
    void SetType(DWORD dwType) { _dwType = dwType; }
    // subclass overrides to specify the type of resource they show
    virtual DWORD GetType() { return _dwType; }
    virtual DWORD GetDefaultViewMode() { return LVS_REPORT; }
    void UpdateView();
    void ChangeViewMode(int iView);
    int GetViewMode() { return _iView; }
    void OnUpdate(LPARAM lHint, CObject *pHint);
    void SetOpenFunction(PFNRESOURCEOPEN pfnOpen) { _pfnOpen = pfnOpen; }
    void OnDelete();
    int GetSelectedItem();
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    BOOL InLabelEdit() { return _bInLabelEdit; }
    void OnRename();
    void OnExtractResources();
    void OnExtractAsBitmap();
    BOOL PreTranslateMessage(MSG *pMsg);

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    void OnItemClick(NMHDR* pNMHDR, LRESULT* pResult);
    void OnItemDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
    void OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
    void OnColumnClicked(NMHDR* pNMHDR, LRESULT* pResult);
    void OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
    void OnNotifyDelete(NMHDR* pNMHDR, LRESULT* pResult);
    void OnGetInfoTip(NMHDR* pNMHDR, LRESULT* pResult);
    void OnBeginScroll(NMHDR* pNMHDR, LRESULT* pResult);
    void OnEndScroll(NMHDR* pNMHDR, LRESULT* pResult);
    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    void OnCustomDraw(NMHDR *pNMHDR, LRESULT *pResult);
    void OnContextMenu(CWnd *pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()

    void _InitColumns();
    void _InsertItem(ResourceBlob *pData);
    void _DeleteItem(const ResourceBlob *pData);
    virtual void _PrepareLVITEM(LVITEM *pItem);
    virtual void _OnItemDoubleClick(const ResourceBlob *pData);
    virtual void _OnInitListView(int cItems);
    void _ReevaluateRecency(const ResourceBlob *pData);
    HRESULT _UpdateEntries(CFile *pFile);
    ResourceBlob *_GetResourceForItem(int nItem) { return (ResourceBlob*)GetItemData(nItem); }
    int _GetItemForResource(const ResourceBlob *pData);

    int _iView;
    BOOL _bFirstTime;
    BOOL _bDidInitialUpdate;
    BOOL _bInLabelEdit;
    BOOL _bScrolling;

    CResourceListDoc *m_pDocument;
    DWORD _dwType;
    PFNRESOURCEOPEN _pfnOpen;

    int _iSortColumn;
};


#ifndef _DEBUG  // debug version in VocabView.cpp
inline CResourceListDoc* CResourceListCtrl::GetDocument() const
   { return reinterpret_cast<CResourceListDoc*>(m_pDocument); }
#endif

