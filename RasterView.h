#pragma once

#include "ScrollingThing.h"

// fwd decl
class IEditRasterResource;
class IEditRasterResourceDoc;
class IRasterResource;

// CRasterView view

class CRasterView : public CScrollingThing<CView>
{
	DECLARE_DYNCREATE(CRasterView)

protected:
	CRasterView();           // protected constructor used by dynamic creation
	virtual ~CRasterView();

public:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
    virtual void OnRButtonDown(UINT nFlags, CPoint point);
    virtual void OnLButtonDown(UINT nFlags, CPoint point);
    virtual void OnRButtonUp(UINT nFlags, CPoint point);
    virtual void OnLButtonUp(UINT nFlags, CPoint point);
    virtual void OnLButtonDblClk(UINT nFlags, CPoint point);
    virtual void OnRButtonDblClk(UINT nFlags, CPoint point);
    virtual void OnMouseMove(UINT nFlags, CPoint point);
    virtual void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    virtual void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
    virtual BOOL OnEraseBkgnd(CDC *pDC);
    virtual int OnCreate(LPCREATESTRUCT lpCreateStruct);
    virtual void OnTimer(UINT_PTR nIDEvent);
    virtual void OnKillFocus(CWnd *pNewWnd);
    virtual BOOL OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT message);

    virtual void OnInitialUpdate();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
    virtual void OnUpdate(CView *pSender, LPARAM lHint, CObject *pHint);

private:
    // Timer constants
    static const UINT_PTR RubberBandTimer = 1002;
    static const UINT RubberBandMs = 200;
    static const UINT_PTR HotSpotTimer = 1003;
    static const UINT HotSpotMs = 200;

    // The drawing tools
    enum ViewToolType
    {
        Line =           0,
        Pen =            1,
        Fill =           2,
        RectNormal =     3,
        RoundedRect =    4,
        RectSolid =      5,
        RoundedRectSolid = 6,
        Oval =           7,
        OvalSolid =      8,
        Zoom =           9,
        Select =         10,
        EyeDrop =        11,
        Replace =        12,
        SetPlacement =   13,
        Off =            0xffffffff, // Just for convenience
    };
    const static key_value_pair<UINT, ViewToolType> c_viewIDToTool[];
    static ViewToolType _IDToViewTool(UINT nID);
    const static key_value_pair<ViewToolType, int> c_viewToolToCursor[];
    static int _ViewIDToCursor(ViewToolType vt);


    enum OnSizerType
    {
        None        = 0,
        Horizontal  = 1,
        Diagonal    = 2,
        Vertical    = 3,
    };


    //
    // Encapsulates the data for each cel
    //
    class CelData
    {
    public:
        CelData()
        {
            _dwIndex = -1;
        }
        CelData(const CelData &data)
        {
            this->_point1 = data._point1;
            this->_point2 = data._point2;
            size_t cbSize = CX_ACTUAL(data._size.cx) & _size.cy;
            if (data._pData.get())
            {
                this->_pData.reset(new BYTE[cbSize]);
                memcpy(this->_pData.get(), data._pData.get(), cbSize);
            }
            this->_size = data._size;
            this->_ptPlacement = data._ptPlacement;
            this->_bTransparent = data._bTransparent;
            this->_dwIndex = data._dwIndex;
        }
        CelData& CelData::operator=(const CelData& data)
        {
            if (this != &data)
            {
                this->_point1 = data._point1;
                this->_point2 = data._point2;
                size_t cbSize = CX_ACTUAL(data._size.cx) & _size.cy;
                if (data._pData.get())
                {
                    this->_pData.reset(new BYTE[cbSize]);
                    memcpy(this->_pData.get(), data._pData.get(), cbSize);
                }
                this->_size = data._size;
                this->_ptPlacement = data._ptPlacement;
                this->_bTransparent = data._bTransparent;
                this->_dwIndex = data._dwIndex;
            }
            return (*this);
        }

        void SetSize(DWORD dwIndex, CPoint ptPlacement, CSize size, BYTE bTransparent)
        {
            _dwIndex = dwIndex;
            _ptPlacement = ptPlacement;
            _size = size;
            _bTransparent = bTransparent;
            _pData.reset(new BYTE[CX_ACTUAL(size.cx) * size.cy]);
        }
        void CalcOffset(CPoint ptPlacementMain, CSize sizeMain, CPoint pt1, CPoint pt2)
        {
            // NOTE: this assumes ORIGINSTYLE_BOTTOMCENTER.  The origin is type-specific.
            // However, for fonts and views, it is centered at the bottom center, and cursors
            // only ever have one cel, so in the end it won't matter.
            CPoint ptOriginMain = CPoint(sizeMain.cx / 2, sizeMain.cy - 1) - ptPlacementMain;
            // Now adjust the point
            CPoint ptOriginThis = CPoint(_size.cx / 2, _size.cy - 1) - _ptPlacement;
            _point1 = ptOriginThis - (ptOriginMain - pt1);
            _point2 = ptOriginThis - (ptOriginMain - pt2);
        }
        const CSize &GetSize() const
        {
            return _size;
        }
        DWORD GetIndex() const
        {
            return _dwIndex;
        }
        BYTE *GetDataPtr()
        {
            return _pData.get();
        }
        const CPoint &GetPlacement() const
        {
            return _ptPlacement;
        }
        BYTE GetTransparentColor() const
        {
            return _bTransparent;
        }
        CPoint _point1;     // temporary point buffer
        CPoint _point2;     // temporary point buffer

    private:
        scoped_array<BYTE> _pData;       // The pixels
        CSize _size;        // The size of this cel
        DWORD _dwIndex;     // In index in the resource object of this cel
        CPoint _ptPlacement;
        BYTE _bTransparent; // Transparent color
    };

    //
    // Helper class for managing rectangular selections in cels
    //
    class SelectionManager
    {
    public:
        SelectionManager();
        ~SelectionManager();

        bool HasSelection();

        void LiftSelection(CRect rcMain, int iMain, int cCels, std::vector<CRasterView::CelData> &celData, BOOL fClear, BOOL fGrabBits);
        BYTE *GetMainSelection(CSize &sizeOut);
        void DrawSelection(CRect rectSelection, int iIndex, BOOL fTransparent, CelData &celData);
        void ClearSelection() { _GenerateSelectionBits(0, CSize(0, 0)); }
        CRect PasteBitmap(BITMAPINFO *pbmi, int cCels, CSize sizeMain);

    private:
        CRect _GetBottomOriginRect(CSize sizeCel, const RECT *prc);
        void _GenerateSelectionBits(int cCels, CSize size);

        bool _fSelection;
        std::vector<BYTE*> _selectionBits;
        CSize _sizeSelectionBits;
        int _iMain;
    };




	DECLARE_MESSAGE_MAP()
    afx_msg BOOL OnDrawCommand(UINT nID);

    afx_msg void OnCopyPic();
    afx_msg void OnPaste();
    afx_msg void OnPasteTransparent();
    afx_msg void OnUpdateAllViewCommands(CCmdUI *pCmdUI);
    afx_msg void OnCut();
    afx_msg void OnDelete();
    afx_msg void OnSelectAll();
    afx_msg void OnUpdateSelectAll(CCmdUI *pCmdUI);
    afx_msg void OnDither();
    afx_msg void OnUpdateDither(CCmdUI *pCmdUI);
    afx_msg void OnUpdateDelete(CCmdUI *pCmdUI);
    afx_msg void OnIndicatorCoords(CCmdUI *pCmdUI);
    afx_msg void OnFlipHorz();
    afx_msg void OnFlipVert();
    afx_msg void OnInvert();
    afx_msg void OnGreyScale();

    void _OnDrawCommand(ViewToolType type);
    void _OnDraw(CDC* pDC);
    void _OnBltResized(CDC *pDCDest, CDC *pDCSrc);
    void _OnBltResized2(CDC *pDCDest, HBITMAP hbmp);
    void _OnDrawSizers(CDC *pDC, CPoint &ptWhatsLeft);
    void _OnDrawSelectionRect(RECT *prc);
    void _OnDrawSelectionRectI(RECT *prc);
    void _OnDrawSelectionBits(const RECT *prcSelection, BOOL fTransparent, int iIndex);
    void _DrawHotSpot(CDC *pDC);
    void _LiftSelection();
    void _DrawCaptureTool(CDC *pDC);
    void _ApplyCaptureTool();
    void _DrawCaptureToolHelper(CDC *pDC, CPoint ptStart, CPoint ptEnd);
    bool _IsCaptureTool();
    void _GenerateDoubleBuffer(CDC *pDC);
    void _RefreshViewData();
    void _DestroyDoubleBuffer();
    void _OnZoomLClick();
    void _OnZoomRClick();
    void _OnFillLClick(CPoint pt, bool fUseForeground);
    void _OnPenClick(CPoint pt, bool fUseForeground);
    void _OnSetPlacementLClick(CPoint pt);
    void _OnCaptureToolButtonDown(CPoint pt, bool fAux);
    void _OnCaptureToolButtonUp();
    void _OnChangeTool(ViewToolType newTool);
    void _UpdateCursor();
    CPoint _MapClientPointToPic(CPoint ptScreen);
    const IRasterResource *_GetRR();
    IEditRasterResourceDoc *_GetERRD();
    IEditRasterResource *_GetERR();
    void _LockDoc(bool fLock);
    void _GrabSourceData();
    void _CommitSourceData();
    OnSizerType _OnSizer(CPoint &ptClient);
    BYTE _GetCurrentTransparentColor();
    BYTE _GetTransparentColor(DWORD dwIndex);
    CRect _GetBottomOriginRect(const RECT *prc);
    void _EnsurePointWithinBounds(CPoint &pointView);
    void _EnsureRectWithinBounds(CRect &rect);
    void _CommitSelectionBits();
    void _GetRidOfSelection();
    void _OnCutOrDeleteSelection(BOOL fCut);
    void _OnCopyBitsToClipboard(const BYTE *pBitsSelection, CSize size);
    BYTE *_CreateSelectionByteArray(BOOL fCleanOutView, BOOL fGrabBits, BOOL fReturnSingle);
    void _StartRubberBandTimer() { SetTimer(RubberBandTimer, RubberBandMs, NULL); }
    void _KillRubberBandTimer() { KillTimer(RubberBandTimer); }
    void _StartHotSpotTimer() { SetTimer(HotSpotTimer, HotSpotMs, NULL); }
    void _KillHotSpotTimer() { KillTimer(HotSpotTimer); }
    void _InvalidateViewArea();
    void _OnMouseWheel(UINT nFlags, BOOL fForward, CPoint pt, short nNotches);
    void _EnsureScratchBuffer1(CSize size);
    void _DrawPen(CDC *pDC, CPoint point, EGACOLOR color, BOOL fUseForeground);
    BOOL _IsPenTool() { return ((_currentTool == Pen) || (_currentTool == Replace)); }
    void _OnPaste(bool fTransparent);
    void _DrawDitheredPen(CDC *pDC, CPoint point);
    BYTE *_ViewOffset(int y) { return (_iMainIndex == -1) ? NULL : _ViewOffset(_iMainIndex, y); }
    BYTE *_ViewOffset(int iIndex, int y) { return _celData[iIndex].GetDataPtr() + CX_ACTUAL(_celData[iIndex].GetSize().cx) * y; }
    BYTE *_GetMainViewData() { return _ViewOffset(0); }
    BOOL _PointInView(CPoint point) { CRect rect(0, 0, _sizeView.cx, _sizeView.cy); return rect.PtInRect(point); }
    void _InitPatternBrush();
    BOOL _CanResize();
    void _PrepareCelOffsets(const CPoint &ptOriginal1, const CPoint &ptOriginal2);
    void _CleanUpViewData();
    void _CheckControlShift(UINT nChar);
    void _OnReplaceTool(CPoint pt, BOOL fReplaceAll = FALSE, bool fRightClick = false);
    CRect _GetEffectArea(int nCel) const;

    // Scrolling
    virtual int _GetViewWidth() { return _cxViewZoom; }
    virtual int _GetViewHeight() { return _cyViewZoom; }

    // Size of current cel:
    CSize _sizeView;

    // Current bitmap cel selection
    DWORD _dwIndex;

    // Actual size of cel on screen:
    int _cxViewZoom;
    int _cyViewZoom;
    int _iZoomFactor;

    // Bitmap for double-buffering
    CBitmap *_pbitmapDoubleBuf;
    bool _fDoubleBuf;

    ViewToolType _currentTool;
    bool _fCapturing;
    CPoint _ptStartCapture; // Where the capture started.

    CPoint _ptCurrentHover;         // Current mouse pos in view coordinates
    CPoint _ptCurrentHoverClient;   // In client coordinates.

    EGACOLOR _color;

    // Used for drawing tools that require capturing the mouse
    bool _fAux; // This means the user is holding down the right mouse button.

    // Pens
    bool _fPreviewPen;              // Is the user dragging a pen.  When we're in this mode, we don't keep
                                    // updating our data from the view resource, since pen's are additive.
    int _nPenWidth;                 // The width of the pen.
    scoped_array<BYTE> _pBitsScratch1;
    CSize _sizeScratch1;

    // Sizers
    CPoint _ptStartSizerCapture;    // In view coordinates
    CSize _sizeOrig;                // In view coordinates
    CSize _sizeNew;                 // In view coords, updated as the user drags
    bool _fSizerCapturing;
    OnSizerType _onSizerType;       // Contains the sizertype when resizing, or the kind you are hovered over when not resizing.

    // REVIEW: maybe move rectselection to _selectionManager
    CRect _rectSelection;           // Current selection rect
    bool _fDraggingSelection;       // Is the user dragging the selection? (_fCapturing will also be true).
    bool _fSelectionLifted;         // When the user moves the selection rect, should we lift selection?
    int _iRubberBandFrameNumber;    // Animating frame of selection rubber band
    bool _fSelectionTransparent;    // Is the selection "transparent" (only applies to pasted ones)

    bool _fHotSpot;

    CBrush _brushPattern;
    CBitmap _bitmapBrush;
    bool _fDithered;

    COLORREF _colorEditor;

    // remove this:
    DWORD _rgdwGroups[128];

    // The number of cels we are currently working on
    // (general just 1, unless the user tells us to apply operation to all cels)
    int _cWorkingCels;
    // These are the cels we affect.  The are allocated to a size of _cWorkingCels
    std::vector<CelData> _celData;
    // This is the index in _celData of the main cel we're looking at in the view
    int _iMainIndex;

    // Manages selection. Go figure.
    SelectionManager _selectionManager;
};


