#pragma once

#include "NoFlickerStatic.h"

// fwd decl
class CPicDoc;

// CBitmapToPicDialog dialog

class CBitmapToPicDialog : public CExtNCW<CExtResizableDialog>
{
public:
	CBitmapToPicDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CBitmapToPicDialog();

    void SetPicPalette(const EGACOLOR *pPalette) { CopyMemory(&_picPalette, pPalette, sizeof(_picPalette)); }

    // After DoModal returns IDOK, call this:
    void InsertIntoPic(CPicDoc *pepic);

// Dialog Data
	enum { IDD = IDD_BITMAPTOPIC };

private:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual void OnCancel();
    virtual void OnOK();
    static BOOL s_ConvertToPic(HWND hwnd, HANDLE hEvent, CArray<PicCommand, PicCommand> *pcommands, EGACOLOR *pegaTemp, CSize &size, BOOL fIgnoreWhite, EGACOLOR *rgColors, int cColors, BOOL fDontSetPalette);
    void _AddToEdit(PCTSTR pszText);
    void _SetBitmap(HBITMAP hbmp);
    static UINT s_ThreadWorker(void *pParam);
    afx_msg void OnConvert();
    afx_msg void OnBrowse();
    afx_msg void OnPasteFromClipboard();
    void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pWnd);
    void OnTimer(UINT_PTR nIDEvent);
    LRESULT _OnConvertDone(WPARAM wParam, LPARAM lParam);
    LRESULT _OnConvertStatus(WPARAM wParam, LPARAM lParam);
    LRESULT _OnConvertProgress(WPARAM wParam, LPARAM lParam);
    void _UpdateOrigBitmap();
    void _PrepareBitmapForConversion();
    BOOL _ReallocateCRBitmap(CSize size);
    BOOL _Init(Gdiplus::Image *pImage);
    void _ApplySettingsToCurrentBitmap();
    void _CalculateContrastCenter(Gdiplus::Bitmap *pBitmap, BYTE *pbContrastCenter);
    void _DeleteCommands();
    virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()


    CExtEdit m_wndEditStatus;
    CExtProgressWnd m_wndProgress;
    int _iAlgorithm;
    int _iPalette;
    int _iScaleImage;
    int _iIgnoreWhite;
    int _nColors;
    CExtNoFlickerStatic m_wndPic;
    CExtButton m_wndFromClipboard;

    CExtNoFlickerStatic m_wndOrig;

    // just for prof-uis
    CExtSliderWnd m_wndSlider1;
    CExtSliderWnd m_wndSlider2;
    CExtSliderWnd m_wndSlider3;
    CExtSliderWnd m_wndSlider4;
    CExtButton m_wndButton1;
    CExtButton m_wndButton2;
    CExtButton m_wndButton3;
    CExtButton m_wndButton4;
    CExtRadioButton m_wndRadio1;
    CExtRadioButton m_wndRadio2;
    CExtRadioButton m_wndRadio3;
    CExtRadioButton m_wndRadio4;
    CExtRadioButton m_wndRadio5;
    CExtRadioButton m_wndRadio6;
    CExtRadioButton m_wndRadio7;
    CExtRadioButton m_wndRadio8;
    CExtRadioButton m_wndRadio9;
    CExtGroupBox m_wndGroup1;
    CExtGroupBox m_wndGroup2;
    CExtGroupBox m_wndGroup3;
    CExtGroupBox m_wndGroup4;
    CExtCheckBox m_wndCheck1;
    CExtCheckBox m_wndCheck2;
    CExtLabel m_wndLabel1;
    CExtLabel m_wndLabel2;
    CExtLabel m_wndLabel3;

    // Straight through
    COLORREF *_pCRBitmap;
    CSize _size;

    CWinThread *_pThread;
    BOOL _fConverting;
    HANDLE _hEvent;

    Gdiplus::Bitmap *_pbmpNormal;
    int _nContrastCenter;
    Gdiplus::Bitmap *_pbmpScaled;
    Gdiplus::Bitmap *_pbmpCurrent;
    BYTE _bContrastCenterNormal;
    BYTE _bContrastCenterScaled;

    int _nBrightness;
    int _nContrast;
    int _nSaturation;

    CArray<PicCommand, PicCommand> *_pcommands;
    BOOL _fTooBig;

    // Optional palette from current pic.
    EGACOLOR _picPalette[40];

    bool _fInitializedControls;

public:
    afx_msg void OnBnClickedRadio4();
    afx_msg void OnBnClickedRadio1();
    afx_msg void OnBnClickedRadio2();
    afx_msg void OnBnClickedRadio3();
    afx_msg void OnBnClickedRadio5();
    afx_msg void OnBnClickedRadiohalftone();
    afx_msg void OnBnClickedRadiosolid();
    afx_msg void OnBnClickedRadiohalftonesmooth();

    afx_msg void OnScaleClicked();
    afx_msg void OnBnClickedRadiopicpalette();
    afx_msg void OnBnClickedCheckignorewhite();
};
