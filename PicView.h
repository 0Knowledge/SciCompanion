// PicView.h : interface of the CPicView class
//


#pragma once

#include "PicResource.h"
#include "PicDoc.h"
#include "ScrollingThing.h"
#include "PaletteDefinitionCallback.h"

class PicDrawManager;

class CPicView : public CScrollingThing<CView>, public IPaletteDefinitionCallback
{
private: // create from serialization only
    CPicView();
    DECLARE_DYNCREATE(CPicView)

    enum ToolType
    {
        None         = 0x00000000,
        Command      = 0x00000001, // We're using a CommandType
        History      = 0x00000002,
        Zoom         = 0x00000003,
        Pasting      = 0x00000004, // We're currently pasting commands
    };
    const static key_value_pair<ToolType, UINT> c_toolToID [];
    static ToolType _IDToTool(UINT nID);


// Attributes
public:
    CPicDoc* GetDocument() const;

// Operations
public:
    void SetOpacity(int iOpacity);
    int GetOpacity() { return _iTraceAlpha; }


// Overrides
    public:
    virtual void OnDraw(CDC* pDC);  // overridden to draw this view
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual void OnInitialUpdate();

protected:
    virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
    virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
    virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
    virtual BOOL OnEraseBkgnd(CDC *pDC);
    virtual void OnRButtonDown(UINT nFlags, CPoint point);
    virtual void OnLButtonDown(UINT nFlags, CPoint point);
    virtual void OnLButtonDblClk(UINT nFlags, CPoint point);
    virtual void OnRButtonDblClk(UINT nFlags, CPoint point);
    virtual void OnLButtonUp(UINT nFlags, CPoint point);
    virtual void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

// IPaletteDefinitionCallback
    virtual void OnSomethingChanged(BOOL fWriteEntire, EGACOLOR *pPalettes, int iPalette);
    virtual void OnPreviewOff();

// Implementation
public:
    virtual ~CPicView();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
    virtual void OnUpdate(CView *pSender, LPARAM lHint, CObject *pHint);

protected:
    const PicResource *_GetEditPic();
    void _OnHistoryLClick(CPoint point);
    void _OnHistoryRClick(CPoint point);
    void _OnPatternLClick(CPoint point);
    void _OnPatternRClick();
    void _OnLineLClick(CPoint point);
    void _OnLineRClick(CPoint point);
    void _OnFillLClick(CPoint point);
    void _OnCircleLClick(CPoint point);
    void _OnCircleRClick(CPoint point);
    void _OnZoomLClick(CPoint *ppt = NULL);
    void _OnZoomRClick(CPoint *ppt = NULL);
    void _OnZoomClick(CPoint *ppt, int iMultiplier);
    UINT _GetCurrentToolOrCommand();
    void _UpdateCursor();
    void _OnCommandChanged();
    void _OnShowPalette(BYTE b);
    static void CALLBACK s_CloseCoordCallback(void *pv, CDC *pDC, int x1, int y1, int x2, int y2);

    CPoint _MapScreenPointToPic(CPoint ptScreenAdjusted);
    CPoint _MapClientPointToPic(CPoint ptScreen);
    CPoint _MapPicPointToClient(CPoint ptPic);
    void _MapPicRectToScreen(RECT *prcPic, RECT *prcScreen);
    void _EnsureDoubleBuffer(CDC *pDC);
    void _GenerateTraceImage(CDC *pDC);
    void _OnDraw(CDC* pDC);
    void _DrawInitPicData(PicData *pData);
    void _DrawShowingEgo(PicDrawManager &pdm);
    void _DrawPriorityLines();
    void _DrawLineDraw(BOOL *fDrawInvisibleLine);
    void _DrawCircleDraw();
    void _DrawPenPreview();
    void _DrawGuideLines();
    void _InitCommandAdjust(PICCOMMAND_ADJUST *pAdjust);
    void _DrawPasteCommands();
    void _DrawEgoCoordinates(CDC *pDC);
    void _MakeRandomNR();
    void _InsertPaletteCommands(EGACOLOR *pPaletteOrig, EGACOLOR *pPaletteNew, BOOL fWriteEntire);
    void _MakeNewMasterTraceImage(PCTSTR pszFileName, BITMAPINFO *pbmi, void *pBits);
    void _InsertPastedCommands();
    void _OnPasteCommands(HGLOBAL hMem);
    void _GetPasteRect(CRect &rect);
    void _CleanUpPaste();
    void _OnMouseWheel(UINT nFlags, BOOL fForward, CPoint pt, short nNotches);
    BOOL _HitTestFakeEgo(CPoint pt);
    CPoint _FindCenterOfFakeEgo();
    bool _NearPasteCorners();
    bool _EvaluateCanBeHere(CPoint pt);
    PicDrawManager &_GetDrawManager();

    // Scrolling
    virtual int _GetViewWidth() { return _cxPic; }
    virtual int _GetViewHeight() { return _cyPic; }

// Generated message map functions
protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnPenCommand();
    afx_msg void OnLineCommand();
    afx_msg void OnFillCommand();
    afx_msg void OnCircleCommand();
    afx_msg void OnDrawOff();
    afx_msg void OnZoomTool();
    afx_msg void OnZoomIn() { _OnZoomLClick(&_ptCurrentHover); }
    afx_msg void OnZoomOut() { _OnZoomRClick(&_ptCurrentHover); }
    afx_msg void OnSetEgoPriority(UINT nID);
    afx_msg void OnHistoryTool();
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnTraceBitmap();
    afx_msg void OnShowTraceImage();
    afx_msg void OnToggleVisual();
    afx_msg void OnTogglePriority();
    afx_msg void OnToggleControl();
    afx_msg void OnShowVisual();
    afx_msg void OnShowPriority();
    afx_msg void OnShowControl();
    afx_msg void OnShowPalette0();
    afx_msg void OnShowPalette1();
    afx_msg void OnShowPalette2();
    afx_msg void OnShowPalette3();
    afx_msg void OnTogglePriorityLines();
    afx_msg void OnToggleEgo();
    afx_msg void OnLightUpCoords();
    afx_msg void OnSetPalette();
    afx_msg void OnCopyPic();
    afx_msg void OnPaste();
    afx_msg void OnPasteIntoPic();
    afx_msg void OnObserveControlLines();
    afx_msg void OnUpdateAllPicCommands(CCmdUI *pCmdUI);
    afx_msg void OnUpdateShowTraceImage(CCmdUI *pCmdUI);
    afx_msg void OnUpdateAlphaSlider(CCmdUI *pCmdUI);
    afx_msg void OnCommandUIAlwaysValid(CCmdUI *pCmdUI) { pCmdUI->Enable(TRUE); }
    afx_msg void OnCommandUIStatus(CCmdUI *pCmdUI);
    afx_msg void OnUpdateShowScreenControl(CCmdUI *pCmdUI);
    afx_msg void OnUpdateShowPaletteControl(CCmdUI *pCmdUI);
    afx_msg void OnUpdateTogglePriorityLines(CCmdUI *pCmdUI);
    afx_msg void OnUpdateToggleEgo(CCmdUI *pCmdUI);
    afx_msg void OnUpdateLightUpCoords(CCmdUI *pCmdUI);
    afx_msg void OnUpdateToggleScreen(CCmdUI *pCmdUI);
    afx_msg void OnUpdateSetVisual(CCmdUI *pCmdUI);
    afx_msg void OnCommandUpdatePastePic(CCmdUI *pCmdUI);
    afx_msg void OnUpdateIsGDIPAvailable(CCmdUI *pCmdUI);
    afx_msg void OnUpdateObserveControlLines(CCmdUI *pCmdUI);
    LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);

    DECLARE_MESSAGE_MAP()

private:

    // For scrolling (screen coordinates)
    int _cyPic;
    int _cxPic;

    ToolType _currentTool;
    PicCommand::CommandType _currentCommand;    // Only valid if _currentTool == Command

    BOOL _fShowTraceImage;
    BOOL _fHaveTraceImage;

    BOOL _fShowPriorityLines;

    BOOL _fShowingEgo;
    int _nFakeCel;
    int _nFakeLoop;
    int _nFakePri;
    BOOL _fCapturing;
    CPoint _pointEgo;
    CPoint _pointEgoOrig;
    CPoint _pointCapture;
    bool _fCanBeHere;   // Can the fake ego be here?

    // When we're drawing temporary stuff on top (e.g. lines):
    // Everything here is screen coordinates (not pic coordinates)
    BOOL _fDrawingLine;
    BOOL _fDrawingCircle;
    BOOL _fPreviewPen;
    BOOL _fMouseWithin;

    // For previewing palette changes.
    class PaletteChangePreview
    {
    public:
        PaletteChangePreview(const PicResource *pResource, PicDrawManager &pdm) : _pic(*pResource), _pdm(NULL)
        {
            _pdm.SetPic(&_pic);
        }
        PicResource &GetPicResource() { return _pic; }
        PicDrawManager &GetDrawManager() { return _pdm; }
    private:
        PicResource _pic;
        PicDrawManager _pdm;
    };
    std::auto_ptr<PaletteChangePreview> _pPreview;

    // These are all in pic coordinates
    int _xOld;  // -1
    int _yOld;  // -1
    CPoint _ptCurrentHover;

    // We store our own 320 x 190 bitmaps
    scoped_array<BYTE> _pdataDisplay;
    scoped_array<BYTE> _pdataAux;

    // This is our screen bitmap for double-buffering
    CBitmap _bitmapDoubleBuf;
    bool _fDoubleBuf;

    // Trace image.
    CBitmap _bitmapTrace;
    std::auto_ptr<Bitmap> _pgdiplusTrace;
    BOOL _fRegenerateTrace;

    // Which screen are we drawing?
    DWORD _dwDrawEnable;

    BYTE _bRandomNR;

    BOOL _fLightUpCoords;

    // For command pasting.
    std::vector<PicCommand> _pastedCommands;
    // The x, y, width and height of the commands that were pasted (doesn't change)
    __int16 _xPasted;
    __int16 _yPasted;
    __int16 _cxPasted;
    __int16 _cyPasted;
    // Offset from the *original* position of the commands (not from the current drag)
    __int16 _xPasteOffset;
    __int16 _yPasteOffset;
    // Offset from where we began a drag (e.g. zero at first)
    __int16 _xPasteOffsetFromCapture;
    __int16 _yPasteOffsetFromCapture;
    // Have the commands been flipped?
    bool _fHFlip;
    bool _fVFlip;
    // Pasted command rotation
    int _iAngle;
    int _iAngleCapture;
    bool _fRotating;
    // Pasted command scaling:
    __int16 _cxPastedScale;
    __int16 _cyPastedScale;
    bool _fInPasteRect;

    int _iTraceAlpha;
};

#ifndef _DEBUG  // debug version in PicEditorView.cpp
inline CPicDoc* CPicView::GetDocument() const
   { return reinterpret_cast<CPicDoc*>(m_pDocument); }
#endif

//
// Slider for trace image alpha
//
class CExtAlphaSlider : public CExtBarSliderButton
{
public:
    CExtAlphaSlider::CExtAlphaSlider(CExtToolControlBar *pBar, UINT nCmdId) :
        CExtBarSliderButton(pBar, nCmdId, 0,
    		// scroll total/pos/page
            100, 50, 0,
		    // button extent horz(left/right)/vert(up/down) in pixels
		    // (if zeros - use slider-like layout instead of scrollbar-like)
		    0, 0,
		    // total slider control extent horz/vert in pixels
		    100, 100)
    {
        _pView = NULL;
    }
    void SetView(CPicView *pView)
    {
        if (pView != _pView)
        {
            _pView = pView;
            // Update from view
            ScrollPosSet((ULONG)_pView->GetOpacity());
        }
    }
    ULONG ScrollPosSet(ULONG nScrollPos)
    {
        // Push to view.
        ULONG nOldPos = __super::ScrollPosSet(nScrollPos);
        if (_pView)
        {
            // Note: It's possible we hold a ref to a _pView that gets
            // deleted, but this should be ok, since we'll never get here again unless
            // we are assigned a new view (since the scroll pos should only change if
            // 1) we are assigned a new view, or
            // 2) the user moves it, which means our view is still valid.
            _pView->SetOpacity((int)nScrollPos);
        }
        return nOldPos;
    }
private:
    CPicView *_pView;
};

extern CExtAlphaSlider *g_pPicAlphaSlider;
