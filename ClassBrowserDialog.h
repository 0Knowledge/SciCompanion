#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "Compile\ScriptOM.h"
#include "ClassBrowser.h"


// CClassBrowserDialog dialog

class CClassBrowserDialog : public CDialog
{
	DECLARE_DYNAMIC(CClassBrowserDialog)

public:
	CClassBrowserDialog(const SCIClassBrowser *pBrowser, CWnd* pParent = NULL);   // standard constructor
	virtual ~CClassBrowserDialog();

// Dialog Data
	enum { IDD = IDD_CLASSBROWSERDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
    void OnSelChange();
    void OnInstanceSelChange();

private:
    void _PopulateTree();
    void _PopulateTree(HTREEITEM hItemParent, const SCIClassBrowserNode *pBrowserInfo);
    void _ShowFunction(const sci::FunctionBase *pFunc);
    void _ShowClass(const sci::ClassDefinition *pClass);
    void _ShowHTML(const std::stringstream &html);
    void _OnClassOrInstanceSelected(const SCIClassBrowserNode *pBrowserInfo);
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnTvnSelchangedTreeClass(NMHDR *pNMHDR, LRESULT *pResult);
protected:
    CListBox m_wndProperties;
    CListBox m_wndMethods;
    CTreeCtrl m_wndTree;
    CStatic m_wndClassName;

    CImageList _imagelist;

    // Keep this around, as it owns the lifetimes of the objects we inserted into the tree.
    const SCIClassBrowser *_pBrowser;
    CFont m_font;
    int _iIndexPropSC;
    int _iIndexMethodSC;
    CListBox m_wndInstances;
    CWnd m_wndBrowser;
    bool _fInitialized;

public:
    afx_msg void OnBnClickedButtonprint();
};
