// PicEditorView.cpp : implementation of the CPicView class
//

#include "stdafx.h"
#include "SCIPicEditor.h"
#include "PicChildFrame.h"
#include "PicDoc.h"
#include "PicView.h"
#include "PicResource.h"
#include "ChooseColorDialog.h"
#include "PaletteDefinitionDialog.h"
#include "ChoosePenStyleDialog.h"
#include "DontShowAgainDialog.h"
#include "BitmapToPicDialog.h"
#include "PicDrawManager.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const key_value_pair<CPicView::ToolType, UINT> CPicView::c_toolToID [] =
{
    { Command, ID_PENTOOL },
    { Command, ID_FILL },
    { Command, ID_CIRCLE },
    { Command, ID_LINE },
    { History, ID_HISTORY },
    { Zoom, ID_ZOOM },
    { None, ID_DRAWOFF },
    // Add more here...
};


CExtAlphaSlider *g_pPicAlphaSlider = NULL;

// CPicView

IMPLEMENT_DYNCREATE(CPicView, CView)

BEGIN_MESSAGE_MAP(CPicView, CScrollingThing<CView>)
    // Standard printing commands
    ON_COMMAND(ID_PENTOOL, CPicView::OnPenCommand)
    ON_COMMAND(ID_ZOOM, CPicView::OnZoomTool)
    ON_COMMAND(ID_HISTORY, CPicView::OnHistoryTool)
    ON_COMMAND(ID_LINE, CPicView::OnLineCommand)
    ON_COMMAND(ID_FILL, CPicView::OnFillCommand)
    ON_COMMAND(ID_CIRCLE, CPicView::OnCircleCommand)
    ON_COMMAND(ID_DRAWOFF, CPicView::OnDrawOff)
    ON_COMMAND(ID_TRACEBITMAP, CPicView::OnTraceBitmap)
    ON_COMMAND(ID_SHOWTRACEIMAGE, CPicView::OnShowTraceImage)
    ON_COMMAND(IDC_TOGGLEVISUAL, CPicView::OnToggleVisual)
    ON_COMMAND(IDC_TOGGLEPRIORITY, CPicView::OnTogglePriority)
    ON_COMMAND(IDC_TOGGLECONTROL, CPicView::OnToggleControl)
    ON_COMMAND(ID_SHOWVISUALSCREEN, CPicView::OnShowVisual)
    ON_COMMAND(ID_SHOWPRIORITYSCREEN, CPicView::OnShowPriority)
    ON_COMMAND(ID_SHOWCONTROLSCREEN, CPicView::OnShowControl)
    ON_COMMAND(ID_SHOWPALETTE0, CPicView::OnShowPalette0)
    ON_COMMAND(ID_SHOWPALETTE1, CPicView::OnShowPalette1)
    ON_COMMAND(ID_SHOWPALETTE2, CPicView::OnShowPalette2)
    ON_COMMAND(ID_SHOWPALETTE3, CPicView::OnShowPalette3)
    ON_COMMAND(ID_TOGGLEPRIORITYLINES, CPicView::OnTogglePriorityLines)
    ON_COMMAND(ID_TOGGLEEGO, CPicView::OnToggleEgo)
    ON_COMMAND(ID_LIGHTUPCOORDS, CPicView::OnLightUpCoords)
    ON_COMMAND(ID_DEFINEPALETTES, CPicView::OnSetPalette)
    ON_COMMAND(ID_EDIT_COPY, CPicView::OnCopyPic)
    ON_COMMAND(ID_EDIT_PASTE, CPicView::OnPaste)
    ON_COMMAND(ID_EDIT_PASTEINTOPIC, CPicView::OnPasteIntoPic)
    ON_COMMAND(ID_VIEW_ZOOMIN, CPicView::OnZoomIn)
    ON_COMMAND(ID_VIEW_ZOOMOUT, CPicView::OnZoomOut)
    ON_COMMAND(ID_OBSERVECONTROLLINES, CPicView::OnObserveControlLines)
    ON_COMMAND_RANGE(ID_DEFAULTPRIORITY, ID_MAIN_PRI15, CPicView::OnSetEgoPriority)
    ON_UPDATE_COMMAND_UI(ID_PENTOOL, CPicView::OnUpdateAllPicCommands)
    ON_UPDATE_COMMAND_UI(ID_LINE, CPicView::OnUpdateAllPicCommands)
    ON_UPDATE_COMMAND_UI(ID_FILL, CPicView::OnUpdateAllPicCommands)
    ON_UPDATE_COMMAND_UI(ID_CIRCLE, CPicView::OnUpdateAllPicCommands)
    ON_UPDATE_COMMAND_UI(ID_DRAWOFF, CPicView::OnUpdateAllPicCommands)
    ON_UPDATE_COMMAND_UI(ID_ZOOM, CPicView::OnUpdateAllPicCommands)
    ON_UPDATE_COMMAND_UI(ID_HISTORY, CPicView::OnUpdateAllPicCommands)
    ON_UPDATE_COMMAND_UI(ID_TRACEBITMAP, CPicView::OnUpdateIsGDIPAvailable)
    ON_UPDATE_COMMAND_UI(ID_SHOWTRACEIMAGE, CPicView::OnUpdateShowTraceImage)
    ON_UPDATE_COMMAND_UI(ID_ALPHASLIDER, OnUpdateAlphaSlider)
    ON_UPDATE_COMMAND_UI(ID_SETPRIORITY, CPicView::OnCommandUIAlwaysValid)
    ON_UPDATE_COMMAND_UI(ID_INDICATOR_COORDS, CPicView::OnCommandUIStatus)
    ON_UPDATE_COMMAND_UI(ID_INDICATOR_PRI, CPicView::OnCommandUIStatus)
    ON_UPDATE_COMMAND_UI(ID_INDICATOR_WORD, CPicView::OnCommandUIStatus)
    ON_UPDATE_COMMAND_UI(ID_SHOWVISUALSCREEN, CPicView::OnUpdateShowScreenControl)
    ON_UPDATE_COMMAND_UI(ID_SHOWPRIORITYSCREEN, CPicView::OnUpdateShowScreenControl)
    ON_UPDATE_COMMAND_UI(ID_SHOWCONTROLSCREEN, CPicView::OnUpdateShowScreenControl)
    ON_UPDATE_COMMAND_UI(ID_TOGGLEPRIORITYLINES, CPicView::OnUpdateTogglePriorityLines)
    ON_UPDATE_COMMAND_UI(ID_SHOWPALETTE0, CPicView::OnUpdateShowPaletteControl)
    ON_UPDATE_COMMAND_UI(ID_SHOWPALETTE1, CPicView::OnUpdateShowPaletteControl)
    ON_UPDATE_COMMAND_UI(ID_SHOWPALETTE2, CPicView::OnUpdateShowPaletteControl)
    ON_UPDATE_COMMAND_UI(ID_SHOWPALETTE3, CPicView::OnUpdateShowPaletteControl)
    ON_UPDATE_COMMAND_UI(ID_TOGGLEPRIORITYLINES, CPicView::OnUpdateShowPaletteControl)
    ON_UPDATE_COMMAND_UI(ID_TOGGLEEGO, CPicView::OnUpdateToggleEgo)
    ON_UPDATE_COMMAND_UI(ID_LIGHTUPCOORDS, CPicView::OnUpdateLightUpCoords)
    ON_UPDATE_COMMAND_UI(IDC_TOGGLEVISUAL, CPicView::OnUpdateToggleScreen)
    ON_UPDATE_COMMAND_UI(IDC_TOGGLEPRIORITY, CPicView::OnUpdateToggleScreen)
    ON_UPDATE_COMMAND_UI(IDC_TOGGLECONTROL, CPicView::OnUpdateToggleScreen)
    ON_UPDATE_COMMAND_UI(ID_DEFINEPALETTES, CPicView::OnCommandUIAlwaysValid)
    ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, CPicView::OnCommandUIAlwaysValid)
    ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, CPicView::OnCommandUpdatePastePic)
    ON_UPDATE_COMMAND_UI(ID_EDIT_PASTEINTOPIC, CPicView::OnUpdateIsGDIPAvailable)
    ON_UPDATE_COMMAND_UI(ID_DEFAULTPRIORITY, CPicView::OnCommandUIAlwaysValid)
    ON_UPDATE_COMMAND_UI_RANGE(ID_DEFAULTPRIORITY, ID_MAIN_PRI15, CPicView::OnCommandUIAlwaysValid)
    ON_UPDATE_COMMAND_UI(ID_ZOOMSLIDER, CPicView::OnCommandUIAlwaysValid)
    ON_UPDATE_COMMAND_UI(ID_OBSERVECONTROLLINES, CPicView::OnUpdateObserveControlLines)
    ON_WM_ERASEBKGND()
    ON_WM_RBUTTONDOWN()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONDBLCLK()
    ON_WM_RBUTTONDBLCLK()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_WM_CREATE()
    ON_WM_VSCROLL()
    ON_WM_HSCROLL()
    ON_WM_SIZE()
    ON_WM_MOUSEWHEEL()
    ON_WM_KEYDOWN()
    ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
END_MESSAGE_MAP()

// CPicView construction/destruction

CPicView::CPicView()
{
    _currentCommand = PicCommand::Pattern; // just something
    _currentTool = None;
    _fShowTraceImage = false;
    _fHaveTraceImage = false;
    _fShowPriorityLines = FALSE;
    _fShowingEgo = FALSE;
    _nFakeCel = 0;
    _nFakeLoop = 0;
    _nFakePri = -1;
    _fCapturing = FALSE;
    _pointCapture.x = 0;
    _pointCapture.y = 0;
    _pointEgo.x = 140; // Good starting values.
    _pointEgo.y = 80;
    _fCanBeHere = false;

    _fDrawingLine = FALSE;
    _fDrawingCircle = FALSE;
    _fPreviewPen = FALSE;
    _fMouseWithin = FALSE;
    _xOld = -1;
    _yOld = -1;
    _ptCurrentHover = CPoint(0, 0);

    // Scrolling:
    _yOrigin = 0;
    _xOrigin = 0;

    // Default size of a pic.
    _cyPic = sPIC_HEIGHT * 2;
    _cxPic = sPIC_WIDTH * 2;

    _pdataDisplay.reset(new BYTE[BMPSIZE]);
    _pdataAux.reset(new BYTE[BMPSIZE]);

    // Start with the visual screen.
    _dwDrawEnable = DRAW_ENABLE_VISUAL;

    _fRegenerateTrace = false;
    _bRandomNR = 0;
    _fLightUpCoords = 0;
    _iTraceAlpha = 50;
}

PicDrawManager &CPicView::_GetDrawManager()
{
    if (_pPreview.get())
    {
        return _pPreview->GetDrawManager();
    }
    else
    {
        return GetDocument()->GetDrawManager();
    }
}

CPicView::~CPicView()
{
    _CleanUpPaste();
}

//
// Given a screen coordinate, returns the coordinate in the pic.
// It assumes origins have been taken into account, so ptScreen
// is not the physical viewport point, but the point within the whole thing.
//
CPoint CPicView::_MapScreenPointToPic(CPoint ptScreen)
{
    return CPoint(ptScreen.x * sPIC_WIDTH / _cxPic,
                  ptScreen.y * sPIC_HEIGHT / _cyPic);
}

//
// This one takes into account origins
//
CPoint CPicView::_MapClientPointToPic(CPoint ptScreen)
{
    return CPoint((ptScreen.x + _xOrigin) * sPIC_WIDTH / _cxPic,
                  (ptScreen.y + _yOrigin) * sPIC_HEIGHT / _cyPic);
}

//
// From pic coordinates to window client coordinates
// (taking into account origins)
//
CPoint CPicView::_MapPicPointToClient(CPoint ptPic)
{
    return CPoint((ptPic.x * _cxPic / sPIC_WIDTH) - _xOrigin,
                  (ptPic.y * _cyPic / sPIC_HEIGHT) - _yOrigin);
}

void CPicView::_MapPicRectToScreen(RECT *prcPic, RECT *prcScreen)
{
    int cxScreen = RECTWIDTH(*prcPic) * _cxPic / sPIC_WIDTH;
    int cyScreen = RECTHEIGHT(*prcPic) * _cyPic / sPIC_HEIGHT;
    int x = prcPic->left *  _cxPic / sPIC_WIDTH - _xOrigin;
    int y = prcPic->top *  _cyPic / sPIC_HEIGHT - _yOrigin;
    prcScreen->left = x;
    prcScreen->top = y;
    prcScreen->right = x + cxScreen;
    prcScreen->bottom = y + cyScreen;
}



void CPicView::OnPenCommand()
{
    // When this is pressed, choose it.
    _currentTool = Command;
    _currentCommand = PicCommand::Pattern;
    _UpdateCursor();
    _OnCommandChanged();
    _MakeRandomNR();
}

void CPicView::OnLineCommand()
{
    // When this is pressed, choose it.
    _currentTool = Command;
    _currentCommand = PicCommand::Line;
    _UpdateCursor();
    _OnCommandChanged();
}

void CPicView::OnFillCommand()
{
    // When this is pressed, choose it.
    _currentTool = Command;
    _currentCommand = PicCommand::Fill;
    _UpdateCursor();
    _OnCommandChanged();
}

void CPicView::OnCircleCommand()
{
    // When this is pressed, choose it.
    _currentTool = Command;
    _currentCommand = PicCommand::Circle;
    _UpdateCursor();
    _OnCommandChanged();
}

void CPicView::OnDrawOff()
{
    _currentTool = None;
    _UpdateCursor();
    _OnCommandChanged();
}

void CPicView::OnZoomTool()
{
    // When this is pressed, choose it.
    _currentTool = Zoom;
    _UpdateCursor();
    _OnCommandChanged();
}

void CPicView::OnHistoryTool()
{
    // When this is pressed, choose it.
    _currentTool = History;
    _UpdateCursor();
    _OnCommandChanged();
}

void CPicView::OnSetEgoPriority(UINT nID)
{
    if (nID == ID_DEFAULTPRIORITY)
    {
        _nFakePri = -1;
    }
    else
    {
        _nFakePri = (nID - ID_MAIN_PRI0);
    }
}

void CPicView::OnObserveControlLines()
{
    theApp._fObserveControlLines = !theApp._fObserveControlLines;
}

void CPicView::OnUpdateObserveControlLines(CCmdUI *pCmdUI)
{
    pCmdUI->Enable();
    pCmdUI->SetCheck(theApp._fObserveControlLines);
}



//
// Takes either a pszFileName, or a combination of pbmi and pBits
//
void CPicView::_MakeNewMasterTraceImage(PCTSTR pszFileName, BITMAPINFO *pbmi, void *pBits)
{
    // Remove any old one.
    _fShowTraceImage = false;
    _fHaveTraceImage = false;

    if (pszFileName)
    {
#ifdef UNICODE
        _pgdiplusTrace.reset(Bitmap::FromFile(pszFileName));
#else
        // GDI+ only deals with unicode.
        int a = lstrlenA(pszFileName);
        BSTR unicodestr = SysAllocStringLen(NULL, a);
        MultiByteToWideChar(CP_ACP, 0, pszFileName, a, unicodestr, a);
        _pgdiplusTrace.reset(Bitmap::FromFile(unicodestr));
        //... when done, free the BSTR
        SysFreeString(unicodestr);
#endif    
    }
    else if (pbmi && pBits)
    {
        _pgdiplusTrace.reset(Bitmap::FromBITMAPINFO(pbmi, pBits));
    }

    // If we got a new one, then set bits indicating we have one, and that
    // we need to regenerate the 320x190 bitmap on the next draw cycle.
    if (_pgdiplusTrace.get())
    {
        if ((_pgdiplusTrace->GetWidth() != sPIC_WIDTH) || (_pgdiplusTrace->GetHeight() != sPIC_HEIGHT))
        {
            if (!theApp._fDontShowTraceScaleWarning)
            {
                TCHAR szPath[MAX_PATH];
                StringCchPrintf(szPath, ARRAYSIZE(szPath), TEXT("The image is not 320x190, and will be %s.\nTo change these settings, go to the preferences dialog."), theApp._fScaleTracingImages ? TEXT("scaled") : TEXT("cropped"));
                CDontShowAgainDialog dialog(szPath, theApp._fDontShowTraceScaleWarning);
                dialog.DoModal(); // Don't care about the thing.
            }
        }

        _fRegenerateTrace = true;
        _fShowTraceImage = true;
        _fHaveTraceImage = true;
        Invalidate(FALSE);
    }
}

void CPicView::_CleanUpPaste()
{
    // Bail on the paste.
    _pastedCommands.clear();
}


// These are the formats that GDI+ uses.
extern TCHAR g_szGdiplusFilter[] = "Image Files|*.jpg;*.jpeg;*.bmp;*.gif;*.png;*.tif;*.tiff;*.emf|All Files|*.*|";


void CPicView::OnTraceBitmap()
{
    // Create a file dialog.
    CFileDialog fileDialog(TRUE, NULL, NULL, OFN_ENABLESIZING | OFN_EXPLORER | OFN_HIDEREADONLY, g_szGdiplusFilter, this);
    fileDialog.m_ofn.lpstrTitle = TEXT("Loading tracing image");
    if (IDOK == fileDialog.DoModal())
    {
        // Tell EditPic about the image.
        CString fileName = fileDialog.GetFileName();
        PCTSTR pszFileName = (PCTSTR)fileName;
        _MakeNewMasterTraceImage(pszFileName, NULL, NULL);
    }
}

void CPicView::OnShowTraceImage()
{
    if (_fHaveTraceImage)
    {
        _fShowTraceImage = !_fShowTraceImage;
        Invalidate(FALSE);
    }
}


void CPicView::OnToggleVisual()
{
    const SCIPicState *pstate = _GetDrawManager().GetPicState();
    PicCommand command;
    if (pstate->dwDrawEnable & DRAW_ENABLE_VISUAL)
    {
        command = PicCommand::CreateDisableVisual();
    }
    else
    {
        // When turning back on, use the last set values in the pic state:
        command = PicCommand::CreateSetVisual(pstate->bPaletteNumber, pstate->bPaletteOffset);
    }
    GetDocument()->InsertCommand(&command);
}

void CPicView::OnTogglePriority()
{
    const SCIPicState *pstate = _GetDrawManager().GetPicState();
    PicCommand command;
    if (pstate->dwDrawEnable & DRAW_ENABLE_PRIORITY)
    {
        command = PicCommand::CreateDisablePriority();
    }
    else
    {
        // When turning back on, use the last set values in the pic state:
        command = PicCommand::CreateSetPriority(pstate->bPriorityValue);
    }
    GetDocument()->InsertCommand(&command);
}

void CPicView::OnToggleControl()
{
    const SCIPicState *pstate = _GetDrawManager().GetPicState();
    PicCommand command;
    if (pstate->dwDrawEnable & DRAW_ENABLE_CONTROL)
    {
        command = PicCommand::CreateDisableControl();
    }
    else
    {
        // When turning back on, use the last set values in the pic state:
        command = PicCommand::CreateSetControl(pstate->bControlValue);
    }
    GetDocument()->InsertCommand(&command);
}


void CPicView::OnShowVisual()
{
    if (_dwDrawEnable != DRAW_ENABLE_VISUAL)
    {
        _dwDrawEnable = DRAW_ENABLE_VISUAL;
        Invalidate(FALSE);
    }
}

void CPicView::OnShowPriority()
{
    if (_dwDrawEnable != DRAW_ENABLE_PRIORITY)
    {
        _dwDrawEnable = DRAW_ENABLE_PRIORITY;
        Invalidate(FALSE);
    }
}

void CPicView::OnShowControl()
{
    if (_dwDrawEnable != DRAW_ENABLE_CONTROL)
    {
        _dwDrawEnable = DRAW_ENABLE_CONTROL;
        Invalidate(FALSE);
    }
}

void CPicView::_OnShowPalette(BYTE b)
{
    GetDocument()->SetPalette(b);
}

void CPicView::OnShowPalette0() { _OnShowPalette(0); }
void CPicView::OnShowPalette1() { _OnShowPalette(1); }
void CPicView::OnShowPalette2() { _OnShowPalette(2); }
void CPicView::OnShowPalette3() { _OnShowPalette(3); }


void CPicView::OnTogglePriorityLines()
{
    _fShowPriorityLines = !_fShowPriorityLines;
    Invalidate(FALSE);
}

void CPicView::OnToggleEgo()
{
    _fShowingEgo = !_fShowingEgo;
    if (_fShowingEgo)
    {
        // Generally, when showing the ego, we'll assume the user no longer wants
        // to use a drawing tool.  This behaviour is a little strange.
        // The alternative is to have draw commands not work when the mouse is over the
        // ego (and change the cursor).  But the user might want to draw in and around the ego.
        OnDrawOff();
    }
    Invalidate(FALSE);
}

void CPicView::OnLightUpCoords()
{
    _fLightUpCoords = !_fLightUpCoords;
    Invalidate(FALSE);
}

void CPicView::OnSomethingChanged(BOOL fWriteEntire, EGACOLOR *pPalettes, int iPalette)
{
    _pPreview.reset(); // Reset to NULL before making a new one, since we rely on
                       // getting the "real" draw manager and pic resource.
    INT_PTR iRealPos = _GetDrawManager().GetPos();
    _pPreview.reset(new PaletteChangePreview(_GetEditPic(), _GetDrawManager()));
    _pPreview->GetDrawManager().SetPalette(static_cast<BYTE>(iPalette));
    // Insert the commands at the current spot:
    _pPreview->GetDrawManager().SeekToPos(iRealPos);
    // Grab the palette state from the actual pic:
    const SCIPicState *pstate = _GetDrawManager().GetPicState();
    InsertPaletteCommands(&_pPreview->GetPicResource(), _pPreview->GetDrawManager().GetPos(), pstate->pPalettes, pPalettes, fWriteEntire);
    _pPreview->GetDrawManager().SeekToPos(-1); // Move to the end for the preview...
    Invalidate(FALSE);
}

void CPicView::OnPreviewOff()
{
    _pPreview.reset();
    Invalidate(FALSE);
}

void CPicView::OnSetPalette()
{
    PicDrawManager &pdm = _GetDrawManager();
    CPaletteDefinitionDialog dialog;
    const SCIPicState *pstate = pdm.GetPicState();

    // For now, just do palette 0
    dialog.SetCallback(this);
    dialog.InitPalettes(pstate->pPalettes);
    INT_PTR iDialogResult = dialog.DoModal();
    _pPreview.reset();
    if (IDOK == iDialogResult)
    {
        EGACOLOR paletteNew[160];
        dialog.RetrievePalettes(paletteNew);
        InsertPaletteCommands(GetDocument(), pdm.GetPos(), pstate->pPalettes, paletteNew, dialog.GetWriteEntirePalette());
    }
    Invalidate(FALSE); // In case we didn't insert anything (to refresh from possible preview)
}

void CPicView::OnCopyPic()
{
    if (OpenClipboard())
    {
        if (EmptyClipboard())
        {
            ASSERT((HBITMAP)_bitmapDoubleBuf);
            // Quick and dirty: use the bitmap we generated for double-buffering.
            // We'll lose it to the clipboard, so we detach it.
            SetClipboardData(CF_BITMAP, (HBITMAP)_bitmapDoubleBuf.Detach());
        }
        CloseClipboard();
    }
}

BOOL _HaveCompatibleClipboardFormat(HWND hwnd, UINT *puFormat)
{
    BOOL bRet = FALSE;
    if (OpenClipboard(hwnd))
    {
        // See if we have a DIB, or a pic command object
        UINT rgFormats[] = { CF_DIB, theApp.GetCommandClipboardFormat() };
        *puFormat = GetPriorityClipboardFormat(rgFormats, ARRAYSIZE(rgFormats));
        // This returns 0 if the clipboard is empty, and -1 if the format isn't available.
        bRet = *puFormat && (*puFormat != -1);
        CloseClipboard();
    }
    return bRet;
}

void CPicView::_InsertPastedCommands()
{
    // Perform the paste
    if (!_pastedCommands.empty())
    {
        // Adjust the commands:
        PICCOMMAND_ADJUST adjust = { 0 };
        _InitCommandAdjust(&adjust);
        PastedCommands_Adjust(_pastedCommands, &adjust);

        // And then insert them.
        GetDocument()->InsertCommands(_pastedCommands.size(), &_pastedCommands[0]);
        _CleanUpPaste();
    }
}

//
// Returns the current paste rect, constrained.
//
void CPicView::_GetPasteRect(CRect &rect)
{
    rect.left = _xPasted + _xPasteOffset;
    rect.left = max(0, rect.left);
    rect.right = _xPasted + _xPasteOffset + _cxPastedScale;
    rect.right = min(sPIC_WIDTH, rect.right);
    rect.top = _yPasted + _yPasteOffset;
    rect.top = max(0, rect.top);
    rect.bottom = _yPasted + _yPasteOffset + _cyPastedScale;
    rect.bottom = min(sPIC_HEIGHT, rect.bottom);
}

void CPicView::_OnPasteCommands(HGLOBAL hMem)
{
    if (_currentTool != Pasting) // If we're already "pasting", we can't do it again.
    {
        // Remove any old ones
        _pastedCommands.clear();

        size_t cb = GlobalSize(hMem);
        BYTE *pBits = (BYTE*)GlobalLock(hMem);
        if (pBits)
        {
            // Create a byte stream with this data.
            sci::istream stream;
            if (stream.WriteBytes(pBits, (int)cb))
            {
                BOOL fOk = stream.Seek(0); // Go back to beginning.
                while (fOk)
                {
                    PicCommand command;
                    fOk = command.Initialize(stream);
                    if (fOk)
                    {
                        _pastedCommands.push_back(command);
                    }
                }

                sRECT rcPasted;
                PastedCommands_GetBounds(&_pastedCommands[0], _pastedCommands.size(), &rcPasted);
                _cxPasted = rcPasted.right - rcPasted.left;
                _cyPasted = rcPasted.bottom - rcPasted.top;
                _xPasteOffset = 0;
                _yPasteOffset = 0;
                _xPasted = rcPasted.left;
                _yPasted = rcPasted.top;
                _fHFlip = false;
                _fVFlip = false;
                _iAngle = 0;
                _cxPastedScale = _cxPasted;
                _cyPastedScale = _cyPasted;

                if (PastedCommands_ContainDrawCommands(&_pastedCommands[0], _pastedCommands.size()))
                {
                    // Go into a mode where they can drag around the commands
                    _currentTool = Pasting;
                    _currentCommand = PicCommand::None;
                    _UpdateCursor();
                    _OnCommandChanged();
                    Invalidate(FALSE);
                }
                else
                {
                    // Otherwise, just paste right now.
                    _InsertPastedCommands();
                }
            }
            GlobalUnlock(hMem);
        }
    }
}

void *_GetBitsPtrFromBITMAPINFO(BITMAPINFO *pbmi)
{
    void *pBits = NULL;
    if (pbmi->bmiHeader.biCompression == BI_RGB)
    {
        // The bits start right after the BITMAPINFO, plus (maybe?) any colour table.
        pBits = ((BYTE*)pbmi + (WORD)(pbmi->bmiHeader.biSize) + pbmi->bmiHeader.biClrUsed * sizeof(RGBQUAD));
    }
    else if (pbmi->bmiHeader.biCompression == BI_BITFIELDS)
    {
        ASSERT((pbmi->bmiHeader.biBitCount == 16) || (pbmi->bmiHeader.biBitCount == 32));
        // In this case, there are 3 DWORDS before the bits actually start.
        pBits = ((BYTE*)pbmi + (WORD)(pbmi->bmiHeader.biSize) + (3 * sizeof(DWORD)));
    }
    return pBits;
}



void CPicView::OnPasteIntoPic()
{
    CBitmapToPicDialog dialog;

    // Initialize the dialog with the current palette 0.
    const SCIPicState *pstate = _GetDrawManager().GetPicState();
    dialog.SetPicPalette(&pstate->pPalettes[0]);

    if (IDOK == dialog.DoModal())
    {
        // Insert the commands in to the pic if successful.
        dialog.InsertIntoPic(GetDocument());
    }
}

void CPicView::OnPaste()
{
    UINT uFormat;
    if (_HaveCompatibleClipboardFormat(m_hWnd, &uFormat))
    {
        if (OpenClipboard())
        {
            HGLOBAL hMem = GetClipboardData(uFormat);

            if (hMem)
            {
                switch (uFormat)
                {
                case CF_DIB:
                    {
                        BITMAPINFO *pbmi = (BITMAPINFO*)GlobalLock(hMem);
                        if (pbmi)
                        {
                            void *pDIBBits = _GetBitsPtrFromBITMAPINFO(pbmi);
                            if (pDIBBits)
                            {
                                _MakeNewMasterTraceImage(NULL, pbmi, pDIBBits);
                            }
                        }
                    }
                    break;

                default:
                    {
                        _OnPasteCommands(hMem);
                        break;
                    }
                }
            }
            CloseClipboard();
        }
    }
}


void CPicView::OnUpdateSetVisual(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(TRUE);
    pCmdUI->SetCheck(TRUE);
}

void CPicView::OnCommandUpdatePastePic(CCmdUI *pCmdUI)
{
    UINT uFormat;
    pCmdUI->Enable((_currentTool != Pasting) && !theApp._fNoGdiPlus && _HaveCompatibleClipboardFormat(m_hWnd, &uFormat));
}

void CPicView::OnUpdateIsGDIPAvailable(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(!theApp._fNoGdiPlus);
}

LRESULT CPicView::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
    _fMouseWithin = FALSE;
    if (_fPreviewPen || _fDrawingLine || _fDrawingCircle)
    {
        Invalidate(FALSE);
    }
    return 0;
}

void CPicView::OnUpdateShowTraceImage(CCmdUI *pCmdUI)
{
    // The show/hide trace image button is disabled if we don't have a trace image.
    ASSERT(pCmdUI->m_nID == ID_SHOWTRACEIMAGE);
    pCmdUI->SetCheck(_fShowTraceImage);
    pCmdUI->Enable(_fHaveTraceImage);
}

void CPicView::OnUpdateAlphaSlider(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(_fHaveTraceImage);
    g_pPicAlphaSlider->SetView(this);
}

void CPicView::SetOpacity(int iOpacity)
{
    ASSERT(iOpacity >= 0 && iOpacity <= 100);
    if (_iTraceAlpha != iOpacity)
    {
        _iTraceAlpha = iOpacity;
        Invalidate(FALSE); // Redraw
    }
}


//
// Update the coordinate display
//
void CPicView::OnCommandUIStatus(CCmdUI *pCmdUI)
{
    if (pCmdUI->m_nID == ID_INDICATOR_COORDS)
    {
        int x = min(_ptCurrentHover.x, 319);
        int y = min(_ptCurrentHover.y, 189);
        pCmdUI->Enable(); 
        TCHAR szText[16];
        StringCchPrintf(szText, ARRAYSIZE(szText), "%3d,%3d", x, y);
        pCmdUI->SetText(szText);
    }
    else if (pCmdUI->m_nID == ID_INDICATOR_PRI)
    {
        int y = min(_ptCurrentHover.y, 189);
        pCmdUI->Enable();
        TCHAR szText[16];
        StringCchPrintf(szText, ARRAYSIZE(szText), "Priority: %2d", PriorityFromY((WORD)y));
        pCmdUI->SetText(szText);
    }
    else if (pCmdUI->m_nID == ID_INDICATOR_WORD) // Overload meaning of this.
    {
        if (_dwDrawEnable & DRAW_ENABLE_CONTROL)
        {
            if ((_ptCurrentHover.x < sPIC_WIDTH) && (_ptCurrentHover.y < sPIC_HEIGHT))
            {
                BYTE bColor = *(_pdataDisplay.get() + BUFFEROFFSET(_ptCurrentHover.x, _ptCurrentHover.y));
                ASSERT(bColor < 16);
                pCmdUI->SetText(c_rgControlColourNames[bColor]);
            }
        }
        else
        {
            pCmdUI->SetText("");
        }
    }
}

void CPicView::OnUpdateShowScreenControl(CCmdUI *pCmdUI)
{
    BOOL fChecked = FALSE;
    switch (pCmdUI->m_nID)
    {
    case ID_SHOWVISUALSCREEN:
        fChecked = (_dwDrawEnable == DRAW_ENABLE_VISUAL);
        break;
    case ID_SHOWPRIORITYSCREEN:
        fChecked = (_dwDrawEnable == DRAW_ENABLE_PRIORITY);
        break;
    case ID_SHOWCONTROLSCREEN:
        fChecked = (_dwDrawEnable == DRAW_ENABLE_CONTROL);
        break;
    }

    pCmdUI->SetCheck(fChecked ? 1 : 0);

    // Always enabled
    pCmdUI->Enable();
}

void CPicView::OnUpdateShowPaletteControl(CCmdUI *pCmdUI)
{
    pCmdUI->Enable();
    BYTE bPaletteButton = 0;
    switch (pCmdUI->m_nID)
    {
    case ID_SHOWPALETTE0:
        bPaletteButton = 0;
        break;
    case ID_SHOWPALETTE1:
        bPaletteButton = 1;
        break;
    case ID_SHOWPALETTE2:
        bPaletteButton = 2;
        break;
    case ID_SHOWPALETTE3:
        bPaletteButton = 3;
        break;
    default:
        ASSERT(FALSE);
    }

    pCmdUI->SetCheck((_GetDrawManager().GetPalette() == bPaletteButton) ? 1 : 0);
}

void CPicView::OnUpdateTogglePriorityLines(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck(_fShowPriorityLines);
    pCmdUI->Enable();
}

void CPicView::OnUpdateToggleEgo(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck(_fShowingEgo);
    pCmdUI->Enable();
}

void CPicView::OnUpdateLightUpCoords(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck(_fLightUpCoords);
    pCmdUI->Enable();
}


void CPicView::OnUpdateToggleScreen(CCmdUI *pCmdUI)
{
    const SCIPicState *pstate = _GetDrawManager().GetPicState();
    DWORD dwDrawEnableMask = 0;
    switch (pCmdUI->m_nID)
    {
    case IDC_TOGGLEVISUAL:
        dwDrawEnableMask |= DRAW_ENABLE_VISUAL;        
        break;

    case IDC_TOGGLEPRIORITY:
        dwDrawEnableMask |= DRAW_ENABLE_PRIORITY;
        break;

    case IDC_TOGGLECONTROL:
        dwDrawEnableMask |= DRAW_ENABLE_CONTROL;
        break;
    }

    pCmdUI->SetCheck((dwDrawEnableMask & pstate->dwDrawEnable) ? 1 : 0);
    pCmdUI->Enable();
}

const key_value_pair<PicCommand::CommandType, UINT> c_commandToID[] =
{
    { PicCommand::Pattern, ID_PENTOOL },
    { PicCommand::Line, ID_LINE },
    { PicCommand::Fill, ID_FILL },
    { PicCommand::Circle, ID_CIRCLE },
    // Add more here...
};
PicCommand::CommandType _IDToCommand(UINT nID)
{
    return LookupKey(c_commandToID, ARRAYSIZE(c_commandToID), nID, PicCommand::None);
}



CPicView::ToolType CPicView::_IDToTool(UINT nID)
{
    return LookupKey(c_toolToID, ARRAYSIZE(c_toolToID), nID, None);
}


const key_value_pair<UINT, int> c_IDToCursor [] =
{
    { ID_PENTOOL, IDC_CURSORPEN },
    { ID_HISTORY, IDC_CURSORHISTORY },
    { ID_FILL, IDC_CURSORFILL },
    { ID_CIRCLE, IDC_CURSORCIRCLE },
    { ID_LINE, IDC_CURSORLINE },
    { ID_ZOOM, IDC_CURSORZOOM },
};
int _IDToCursor(UINT nID)
{
    return LookupKeyValue(c_IDToCursor, ARRAYSIZE(c_IDToCursor), nID, -1);
}

int CalcAngle(CPoint pt1, CPoint pt2)
{
    if (pt2.y == pt1.y) // avoid / 0
    {
        if (pt2.x > pt2.y)
        {
            return 270;
        }
        else
        {
            return 90;
        }
    }
    else
    {
        int i = (int)(360 * atan(((double)(pt2.x - pt1.x)) / (double)(pt2.y - pt1.y)) / 6.28);
        if (pt2.y > pt1.y)
        {
            i += 180;
        }
        return i;
    }
}

void CPicView::OnMouseMove(UINT nFlags, CPoint point)
{
    // Adjust to pic coords.
    _ptCurrentHover = _MapClientPointToPic(point);

    if (_fDrawingLine || _fPreviewPen || _fLightUpCoords || _fDrawingCircle)
    {
        // FEATURE: we could be more selective with this too
        // Just invalidate the region that covers our from-to
        // CPU spikes to 100% when moving around here.
        Invalidate(FALSE);
        // Note: we're not invalidating the bitmap here...
    }

    if ((_currentTool == Command) && (theApp._fGridLines))
    {
        // Invalidate on every mouse move if we're showing gridlines.
        Invalidate(FALSE);
    }

    BOOL fUpdateCursor = FALSE;

    if (_fCapturing)
    {
        int dx = _ptCurrentHover.x - _pointCapture.x;
        int dy = _ptCurrentHover.y - _pointCapture.y;

        if (_currentTool == Pasting)
        {
            if (_fRotating)
            {
                // Rotate
                // Figure out the angle for the original (pointCapture to center rect)
                CRect rc(_xPasted, _yPasted, _xPasted + _cxPasted, _yPasted + _cyPasted);
                rc.OffsetRect(_xPasteOffset, _yPasteOffset);
                int iAngleOrig = CalcAngle(rc.CenterPoint(), _pointCapture);
                int iAngleNow = CalcAngle(rc.CenterPoint(), _ptCurrentHover);
                _iAngle = _iAngleCapture + iAngleOrig - iAngleNow;
            }
            else
            {
                // Adjust our delta
                _xPasteOffset = dx + _xPasteOffsetFromCapture;
                _yPasteOffset = dy + _yPasteOffsetFromCapture;
            }

            Invalidate(FALSE);
        }
        else if (_fShowingEgo)
        {
            // Update the ego position, if the mouse button is down.  But only if we're not "pasting"
            // And only if the ego can be at this position
            CPoint ptNew = _pointEgoOrig + CPoint(dx, dy);
            bool fCanBeHere = _EvaluateCanBeHere(ptNew);
            if (fCanBeHere)
            {
                _fCanBeHere = fCanBeHere;
                _pointEgo = ptNew;
                theApp._ptFakeEgo = _pointEgo;
                Invalidate(FALSE);
            }
            else if (fCanBeHere != _fCanBeHere)
            {
                _fCanBeHere = fCanBeHere;
                Invalidate(FALSE);
            }
            
        }
    }
    else
    {
        if ((_currentTool == Pasting) || (_fShowingEgo))
        {
            fUpdateCursor = TRUE;
            // If we're not capturing, but hovering, and we're pasting or showing the ego
            // then change the cursor when the user is over the paste box (or over the ego)
            BOOL fInPasteRect;
            if (_currentTool == Pasting)
            {
                CRect rectPasted;
                _GetPasteRect(rectPasted);
                fInPasteRect = PtInRect(&rectPasted, CPoint(_ptCurrentHover.x, _ptCurrentHover.y));
            }
            else
            {
                ASSERT(_fShowingEgo);
                fInPasteRect = _HitTestFakeEgo(_ptCurrentHover);
            }
            
            if (fInPasteRect)
            {
                if (!_fInPasteRect)
                {
                    _fInPasteRect = true;
                    if (_fShowingEgo)
                    {
                        // Since we show coordinates when hovered.
                        Invalidate(FALSE);
                    }
                }
            }
            else
            {
                if (_fInPasteRect)
                {
                    _fInPasteRect = false;
                    if (_fShowingEgo)
                    {
                        // Since we show coordinates when hovered.
                        Invalidate(FALSE);
                    }
                }
            }
        }
    }

    if (!_fMouseWithin)
    {
        // Track mouseleaves.
        TRACKMOUSEEVENT eventTrack;
        eventTrack.cbSize = sizeof(eventTrack);
        eventTrack.dwFlags = TME_LEAVE;
        eventTrack.hwndTrack = m_hWnd;
        _fMouseWithin = TrackMouseEvent(&eventTrack);
        fUpdateCursor = TRUE;
    }

    if (fUpdateCursor)
    {
        _UpdateCursor();
    }
}

void CPicView::OnUpdateAllPicCommands(CCmdUI *pCmdUI)
{
    // All commands are enabled when we have focus:
    pCmdUI->Enable(TRUE);

    // Which one is checked though?
    PicCommand::CommandType command = _IDToCommand(pCmdUI->m_nID);
    ToolType tool = _IDToTool(pCmdUI->m_nID);
    int iCheck = (tool == _currentTool);
    if (iCheck && (tool == Command))
    {
        // Even though the tools are equal, the commands might not.
        iCheck = (command == _currentCommand);
    }

    pCmdUI->SetCheck(iCheck);
}

void CPicView::OnInitialUpdate()
{
    __super::OnInitialUpdate();
    CRect rect;
    GetClientRect(&rect);
    // Set our initial zoom level to the biggest we can fit (or at least 2)
    int iZoom = min(rect.Width() / sPIC_WIDTH, rect.Height() / sPIC_HEIGHT);
    iZoom = max(iZoom, 2);
    GetDocument()->SetZoom(iZoom);
}

int CPicView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (__super::OnCreate(lpCreateStruct) == -1)
        return -1;

    // Remove any cursor associated with this window class, so we can change the cursor as we
    // change tools
    SetClassLongPtr(m_hWnd, GCL_HCURSOR, NULL);

    return 0;
}

BOOL CPicView::PreCreateWindow(CREATESTRUCT& cs)
{
    return __super::PreCreateWindow(cs);
}

// CPicView drawing


void CPicView::_EnsureDoubleBuffer(CDC *pDC)
{
    if ((HBITMAP)_bitmapDoubleBuf == NULL)
    {
        _bitmapDoubleBuf.CreateCompatibleBitmap(pDC, sPIC_WIDTH, sPIC_HEIGHT);
    }
}

void CPicView::_GenerateTraceImage(CDC *pDC)
{
    _bitmapTrace.DeleteObject();
    
    // Take our master copy, and generate a 320x190 version of it.
    if (_pgdiplusTrace.get())
    {
        _bitmapTrace.CreateCompatibleBitmap(pDC, sPIC_WIDTH, sPIC_HEIGHT);

        // Now copy the trace image into it.
        CDC dcDest;
        if (dcDest.CreateCompatibleDC(pDC))
        {
            HGDIOBJ hgdiObj = dcDest.SelectObject(_bitmapTrace);
            Graphics graphics((HDC)dcDest);
            if (theApp._fScaleTracingImages)
            {
                graphics.DrawImage(_pgdiplusTrace.get(), 0, 0, sPIC_WIDTH, sPIC_HEIGHT);
            }
            else
            {
                int cx = min(sPIC_WIDTH, _pgdiplusTrace->GetWidth());
                int cy = min(sPIC_HEIGHT, _pgdiplusTrace->GetHeight());
                graphics.DrawImage(_pgdiplusTrace.get(), 0, 0, 0, 0, cx, cy, Gdiplus::UnitPixel);
            }
            dcDest.SelectObject(hgdiObj);

            _fRegenerateTrace = false;
        }
    }
}

void CPicView::_DrawEgoCoordinates(CDC *pDC)
{
    if ((theApp._fUseBoxEgo && (_nFakePri != -1)) || (!theApp._fUseBoxEgo && theApp.GetSelectedViewResource()))
    {
        // The cursor is over the ego.  Draw the coordinates.
        // First we need to convert the ego coordinates to "client" coordinates
        CPoint ptEgoClient = _MapPicPointToClient(_pointEgo);
        TCHAR szCoords[20];
        if (theApp._fUseBoxEgo)
        {
            ASSERT(_nFakePri != -1);
            // The only time we draw anything for "box" egos is if the user set a pri.
            StringCchPrintf(szCoords, ARRAYSIZE(szCoords), TEXT("pri %d"), _nFakePri);
        }
        else
        {
            if (_nFakePri == -1)
            {
                StringCchPrintf(szCoords, ARRAYSIZE(szCoords), TEXT("(%d,%d)"), _pointEgo.x, _pointEgo.y);
            }
            else
            {
                StringCchPrintf(szCoords, ARRAYSIZE(szCoords), TEXT("(%d,%d) - pri %d"), _pointEgo.x, _pointEgo.y, _nFakePri);
            }
        }
        CRect rectTextSize;
        int cyText = pDC->DrawText(szCoords, &rectTextSize, DT_SINGLELINE | DT_CALCRECT);

        // Center the text:
        CPoint ptEgoCenter = _MapPicPointToClient(_FindCenterOfFakeEgo());
        int cxHalf = rectTextSize.Width() / 2;
        cyText /= 2;
        CRect rectFinal(ptEgoCenter.x - cxHalf, ptEgoCenter.y - cyText, ptEgoCenter.x + cxHalf, ptEgoCenter.y + cyText);

        // darken the area under the letters so we can see them better.
        HBITMAP hbmOverlay = LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_ALPHABLEND));
        if (hbmOverlay)
        {
            CDC dcMem;
            if (dcMem.CreateCompatibleDC(pDC))
            {
                HGDIOBJ hOld = dcMem.SelectObject(hbmOverlay);
                BLENDFUNCTION blend = { 0 };
                blend.AlphaFormat = 0;
                blend.SourceConstantAlpha = 128;
                blend.BlendFlags = 0;
                blend.BlendOp = AC_SRC_OVER;
                pDC->AlphaBlend(rectFinal.left, rectFinal.top, rectFinal.Width(), rectFinal.Height(),
                    &dcMem, 0, 0, 32, 4, blend);
                dcMem.SelectObject(hOld);
            }
            DeleteObject(hbmOverlay);
        }

        // Draw it
        int iOldMode = pDC->SetBkMode(TRANSPARENT);
        COLORREF crOld = pDC->SetTextColor(RGB(0, 0, 0));
        pDC->DrawText(szCoords, &rectFinal, DT_SINGLELINE);
        // Offset 1x1, and draw in white (or red, if we can't be here)
        rectFinal.OffsetRect(-1, -1);
        pDC->SetTextColor(_fCanBeHere ? RGB(255, 255, 255) : RGB(250, 0, 0));
        pDC->DrawText(szCoords, &rectFinal, DT_SINGLELINE);

        // Restore stuff
        pDC->SetTextColor(crOld);
        pDC->SetBkMode(iOldMode);
    }
}

void CPicView::OnDraw(CDC *pDC)
{
    RECT rcClient;
    GetClientRect(&rcClient);
    CDC dcMem;
    if (dcMem.CreateCompatibleDC(pDC))
    {
        int cxVisible = min(_cxPic, RECTWIDTH(rcClient));
        int cyVisible = min(_cyPic, RECTHEIGHT(rcClient));
        _EnsureDoubleBuffer(pDC);

        HGDIOBJ hgdiObj = dcMem.SelectObject(_bitmapDoubleBuf);

        // Draw the picture
        _OnDraw(&dcMem);

        if (_fShowTraceImage && _fHaveTraceImage && _fRegenerateTrace)
        {
            // Blit this is one time - it will be there from then on.
            _GenerateTraceImage(pDC);
        }

        if (_fShowTraceImage)
        {
            CPicDoc *pDoc = GetDocument();

            BLENDFUNCTION blendFunc;
            blendFunc.BlendOp = AC_SRC_OVER;
            blendFunc.BlendFlags = 0;
            blendFunc.SourceConstantAlpha = (BYTE)(_iTraceAlpha * 255 / 100); // 0 to 255
            blendFunc.AlphaFormat = 0;//AC_SRC_ALPHA;

            CDC dcMem2;
            if (dcMem2.CreateCompatibleDC(pDC))
            {
                HGDIOBJ hgdiObj2 = dcMem2.SelectObject(_bitmapTrace);
                // Draw this.
                dcMem.AlphaBlend(0, 0, sPIC_WIDTH, sPIC_HEIGHT, &dcMem2, 0, 0, sPIC_WIDTH, sPIC_HEIGHT, blendFunc); 
                dcMem2.SelectObject(hgdiObj2);
            }
        }


        // Now blt back to the real DC.
        // Now we want to copy it back to the real dc.
        pDC->StretchBlt(-_xOrigin, -_yOrigin, _cxPic, _cyPic, &dcMem, 0, 0, sPIC_WIDTH, sPIC_HEIGHT, SRCCOPY); 

        dcMem.SelectObject(hgdiObj);
    }

    // Finish off by drawing a navy background around the pic.
    CBrush brush(RGB(0, 0, 128));
    int cyBottom = RECTHEIGHT(rcClient) - _cyPic;
    if (cyBottom > 0)
    {
        CRect rect1(CPoint(0, _cyPic), CSize(RECTWIDTH(rcClient), cyBottom));
        pDC->FillRect(&rect1, &brush);
    }
    int cxLeft = RECTWIDTH(rcClient) - _cxPic;
    if (cxLeft > 0)
    {
        CRect rect2(CPoint(_cxPic, 0), CSize(cxLeft, RECTHEIGHT(rcClient)));
        pDC->FillRect(&rect2, &brush);
    }

    // This will flicker here, but oh well.
    if (_fLightUpCoords)
    {
        const SCIPicState *pState = _GetDrawManager().GetPicState();
        CPen pen(PS_SOLID, 1, RGB(255, 192, 128));
        HGDIOBJ hObj = pDC->SelectObject(&pen);
        pState->tracker.DrawCoords(_ptCurrentHover, pDC, s_CloseCoordCallback, this);
        pDC->SelectObject(hObj);
    }

    // A final touch.  This will cause some flicker, but it shouldn't be too bad.
    if (_fShowingEgo && _HitTestFakeEgo(_ptCurrentHover))
    {
        _DrawEgoCoordinates(pDC);
    }
    __super::OnDraw(pDC);
}


void CPicView::_DrawShowingEgo(PicDrawManager &pdm)
{
    // Here, we need to figure out the priority of the ego
    if (_pointEgo.y < sPIC_HEIGHT)
    {
        BYTE bEgoPriority = (_nFakePri) == -1 ? PriorityFromY((WORD)_pointEgo.y) : (BYTE)_nFakePri;
        // Now we need to draw a box (or whatever), but only where
        // the bEgoPriority is equal to or greater than the priority
        // of the priority screen.

        const BYTE *pdataPriority = pdm.GetPriorityBits();
        if (pdataPriority)
        {
            // Alright, we're part way there.
            if (!theApp._fUseBoxEgo && theApp.GetSelectedViewResource())
            {
                // Draw a view.
                DrawViewWithPriority(_pdataDisplay.get(), pdataPriority, bEgoPriority, (WORD)_pointEgo.x, (WORD)_pointEgo.y, theApp.GetSelectedViewResource(), _nFakeLoop, _nFakeCel, _HitTestFakeEgo(_ptCurrentHover));
            }
            else
            {
                // Just draw a box
                DrawBoxWithPriority(_pdataDisplay.get(), pdataPriority, bEgoPriority, (WORD)_pointEgo.x, (WORD)_pointEgo.y, theApp._cxFakeEgo, theApp._cyFakeEgo);
            }
        }
    }
}

//
// Init a special PicData for drawing, where we substitute the display screen
// for different screens.
//
void CPicView::_DrawInitPicData(PicData *pData)
{
    ZeroMemory(pData, sizeof(*pData));
    pData->dwMapsToRedraw = _dwDrawEnable;
    pData->pdataAux = _pdataAux.get();
    switch (_dwDrawEnable)
    {
    case DRAW_ENABLE_VISUAL:
        pData->pdataVisual = _pdataDisplay.get();
        break;
    case DRAW_ENABLE_PRIORITY:
        pData->pdataPriority = _pdataDisplay.get();
        break;
    case DRAW_ENABLE_CONTROL:
        pData->pdataControl = _pdataDisplay.get();
        break;
    }
}

void CPicView::_DrawPriorityLines()
{
    // Draw a series of 14 lines on top of the image (from 1 to 14)
    WORD yOld = 0;
    for (BYTE bPriorityValue = 1; bPriorityValue <= 15; bPriorityValue++)
    {
        WORD y = g_iPriorityBands[bPriorityValue - 1];

        PicData data = { 0 };
        // Pretend this is priority screen.
        data.dwMapsToRedraw = DRAW_ENABLE_PRIORITY;
        // Aux needs to be supplied, and it can't hurt - this is just our display buffer,
        // and lines don't check it.
        data.pdataAux = _pdataAux.get();
        data.pdataPriority = _pdataDisplay.get(); // this is the pretend part
        EGACOLOR colorDummy = { 0, 0};

        // Across
        PicCommand command = PicCommand::CreateLine(0, y, sPIC_WIDTH - 1, y);
        LineCommand_DrawOverride(&command, &data, colorDummy, bPriorityValue, 0, DRAW_ENABLE_PRIORITY);

        // And up
        PicCommand commandUp = PicCommand::CreateLine(sPIC_WIDTH - 1, y, sPIC_WIDTH - 1, yOld + 1);
        LineCommand_DrawOverride(&commandUp, &data, colorDummy, bPriorityValue, 0, DRAW_ENABLE_PRIORITY);

        yOld = y;
    }
}

void CPicView::_DrawLineDraw(BOOL *pfDrawInvisibleLine)
{
    // The user is drawing a line.  We'll draw it right into the bits, by asking
    // a line command to draw.
    ASSERT(_xOld != -1);
    ASSERT(_yOld != -1);

    PicCommand command = PicCommand::CreateLine((WORD)_xOld, (WORD)_yOld, (WORD)_ptCurrentHover.x, (WORD)_ptCurrentHover.y);

    // Set up the pic data.
    PicData data;
    _DrawInitPicData(&data);

    // Get the current state from the pic
    const SCIPicState *pState = _GetDrawManager().GetPicState();

    if ((pState->dwDrawEnable & _dwDrawEnable) == 0)
    {
        // We need to draw this some other way.  We'll wait until we've copied
        // our buffer into the memory dc.
        *pfDrawInvisibleLine = TRUE;
    }
    else
    {
        // Finally draw (whew!)
        LineCommand_DrawOnly(&command, &data, pState);
    }
}

void CPicView::_DrawCircleDraw()
{
    // The user is drawing a line.  We'll draw it right into the bits, by asking
    // a line command to draw.
    ASSERT(_xOld != -1);
    ASSERT(_yOld != -1);

    PicCommand command;
    command.CreateCircle((WORD)_xOld, (WORD)_yOld, (WORD)_ptCurrentHover.x, (WORD)_ptCurrentHover.y);

    // Set up the pic data.
    PicData data;
    _DrawInitPicData(&data);

    // Get the current state from the pic
    const SCIPicState *pState = _GetDrawManager().GetPicState();

    if ((pState->dwDrawEnable & _dwDrawEnable) == 0)
    {
        // We need to draw this some other way.  We'll wait until we've copied
        // our buffer into the memory dc.
        // TODO
    }
    else
    {
        // Finally draw (whew!)
        CircleCommand_DrawOnly(&command, &data, pState);
    }
}

void CPicView::_DrawPenPreview()
{
    // We're drawing with a pen.  Preview it.

    // Set up the pic data.
    PicData data;
    _DrawInitPicData(&data);

    // Get the current state from the pic
    const SCIPicState *pState = _GetDrawManager().GetPicState();

    // The user is using the pen tool.
    CPicDoc *pDoc = GetDocument();
    if (pDoc)
    {
        PenStyle penStyle;
        pDoc->GetPenStyle(&penStyle);
        PicCommand command = PicCommand::CreatePattern((WORD)_ptCurrentHover.x,
                                (WORD)_ptCurrentHover.y,
                                penStyle.bPatternSize,
                                _bRandomNR, // Always use _bRandomNR - if random change isn't set, we just don't cycle it.
                                penStyle.fPattern,
                                penStyle.fRectangle);

        PatternCommand_Draw_DrawOnly(&command, &data, pState);
    }
}


void CPicView::_DrawGuideLines()
{
    RECT rcAroundThis = { _ptCurrentHover.x - 10, _ptCurrentHover.y - 10, _ptCurrentHover.x + 10, _ptCurrentHover.y + 10 };

    // Get the current state from the pic
    const SCIPicState *pstate = _GetDrawManager().GetPicState();

    // Set up the pic data.
    PicData data;
    _DrawInitPicData(&data);

    if (rcAroundThis.left >= 0)
    {
        PicCommand command = PicCommand::CreateLine((WORD)0, (WORD)_ptCurrentHover.y, (WORD)rcAroundThis.left, (WORD)_ptCurrentHover.y);
        LineCommand_DrawOnly(&command, &data, pstate);
    }
    if (rcAroundThis.right < sPIC_WIDTH)
    {
        PicCommand command = PicCommand::CreateLine((WORD)rcAroundThis.right, (WORD)_ptCurrentHover.y, (sPIC_WIDTH - 1), (WORD)_ptCurrentHover.y);
        LineCommand_DrawOnly(&command, &data, pstate);
    }
    if (rcAroundThis.top >= 0)
    {
        PicCommand command = PicCommand::CreateLine((WORD)_ptCurrentHover.x, 0, (WORD)_ptCurrentHover.x, (WORD)rcAroundThis.top);
        LineCommand_DrawOnly(&command, &data, pstate);
    }
    if (rcAroundThis.bottom < sPIC_HEIGHT)
    {
        PicCommand command = PicCommand::CreateLine((WORD)_ptCurrentHover.x, (WORD)rcAroundThis.bottom, (WORD)_ptCurrentHover.x, sPIC_HEIGHT - 1);
        LineCommand_DrawOnly(&command, &data, pstate);
    }
}

void CPicView::_InitCommandAdjust(PICCOMMAND_ADJUST *pAdjust)
{
    pAdjust->rcBounds.left = _xPasted;
    pAdjust->rcBounds.top = _yPasted;
    pAdjust->rcBounds.right = _xPasted + _cxPasted;
    pAdjust->rcBounds.bottom = _yPasted + _cyPasted;
    pAdjust->rcNew.left = _xPasted + _xPasteOffset;
    pAdjust->rcNew.top = _yPasted + _yPasteOffset;
    pAdjust->rcNew.right = pAdjust->rcNew.left + _cxPastedScale;
    pAdjust->rcNew.bottom = pAdjust->rcNew.top + _cyPastedScale;

    pAdjust->fHFlip = _fHFlip;
    pAdjust->fVFlip = _fVFlip;

    pAdjust->iAngle = _iAngle;
}

void CPicView::_DrawPasteCommands()
{
    // Get the current state from the pic
    const SCIPicState *pState = _GetDrawManager().GetPicState();
    // We'll make our own copy to party on.
    SCIPicState stateFresh = *pState;

    // Set up the pic data.
    PicData data;
    _DrawInitPicData(&data);

    for (size_t i = 0; i < _pastedCommands.size(); i++)
    {
        PICCOMMAND_ADJUST adjust = { 0 };
        _InitCommandAdjust(&adjust);
        Command_DrawWithOffset(&_pastedCommands[i], &data, const_cast<SCIPicState*>(pState), &adjust);
    }
}


void CPicView::_OnDraw(CDC* pDC)
{
    BOOL fDrawInvisibleLine = FALSE;
    BOOL fDrawPasteBox = FALSE;

    DWORD dwMap = _dwDrawEnable; // Which screen to show
    BITMAPINFO *pbmi;
    _GetDrawManager().CopyBitmap(dwMap, _pdataDisplay.get(), _pdataAux.get(), &pbmi);

    if (_fShowingEgo)
    {
        _DrawShowingEgo(_GetDrawManager());
    }

    if (_fShowPriorityLines)
    {
        _DrawPriorityLines();
    }

    if (_fDrawingLine)
    {
        _DrawLineDraw(&fDrawInvisibleLine);
    }
    if (_fDrawingCircle)
    {
        _DrawCircleDraw();
    }

    if (_fPreviewPen && _fMouseWithin)
    {
        _DrawPenPreview();
    }

    if (_fMouseWithin &&  theApp._fGridLines && (_currentTool == Command))
    {
        // The mouse is within the window, and we are currently drawing.
        // If the user wants guide lines, give them to him.
        _DrawGuideLines();
    }

    // If we're currently pasting, draw those commands now.
    if (!_pastedCommands.empty())
    {
        _DrawPasteCommands();
        fDrawPasteBox = TRUE;
    }

    // Now copy to the memory dc.
    // FEATURE: this could be optimized (when zoomed, by limiting how much we copy)
    // - it involves palette conversion, so it could be expensive.
    if (pbmi)
    {
        StretchDIBits((HDC)*pDC, 0, 0, sPIC_WIDTH, sPIC_HEIGHT, 0, 0, sPIC_WIDTH, sPIC_HEIGHT, _pdataDisplay.get(), pbmi, DIB_RGB_COLORS, SRCCOPY);
        delete pbmi;

        // Draw a special line for when line-drawing is "invisible".  We can't do this directly
        // on the bits, because the line drawing functions don't support dotted lines.
        if (fDrawInvisibleLine)
        {
            // Use a black and white dotted line.
            CPen pen(PS_DOT, 1, RGB(255, 255, 255));
            COLORREF colorOld = pDC->SetBkColor(RGB(0, 0, 0));
            HGDIOBJ hObj = pDC->SelectObject(&pen);
            pDC->MoveTo(_xOld, _yOld);
            pDC->LineTo(_ptCurrentHover);
            pDC->SelectObject(hObj);
            pDC->SetBkColor(colorOld);
        }

        if (fDrawPasteBox)
        {
            // Use a black and white dotted line.
            CPen pen(PS_DOT, 1, RGB(255, 255, 255));
            COLORREF colorOld = pDC->SetBkColor(RGB(0, 0, 0));
            HGDIOBJ hObj = pDC->SelectObject(&pen);

            CRect rectPaste;
            _GetPasteRect(rectPaste);

            pDC->MoveTo(rectPaste.left, rectPaste.top);
            pDC->LineTo(rectPaste.right - 1, rectPaste.top);
            pDC->LineTo(rectPaste.right - 1, rectPaste.bottom - 1);
            pDC->LineTo(rectPaste.left, rectPaste.bottom - 1);
            pDC->LineTo(rectPaste.left, rectPaste.top);
            pDC->SelectObject(hObj);
            pDC->SetBkColor(colorOld);
        }
    }
}

//
// Draws lines that highlight the lines previously drawn in the editor.
//
void CPicView::s_CloseCoordCallback(void *pv, CDC *pDC, int x1, int y1, int x2, int y2)
{
    CPicView *ppev = static_cast<CPicView*>(pv);
    int iZoomOffset = (ppev->_cyPic / sPIC_HEIGHT) / 2;

    CPoint ptClient1 = ppev->_MapPicPointToClient(CPoint(x1, y1));
    ptClient1.Offset(iZoomOffset, iZoomOffset);
    pDC->MoveTo(ptClient1.x - 3, ptClient1.y - 3);
    pDC->LineTo(ptClient1.x + 4, ptClient1.y - 3);
    pDC->LineTo(ptClient1.x + 4, ptClient1.y + 4);
    pDC->LineTo(ptClient1.x - 3, ptClient1.y + 4);
    pDC->LineTo(ptClient1.x - 3, ptClient1.y - 3);

    CPoint ptClient2 = ppev->_MapPicPointToClient(CPoint(x2, y2));
    ptClient2.Offset(iZoomOffset, iZoomOffset);
    pDC->MoveTo(ptClient2.x - 3, ptClient2.y - 3);
    pDC->LineTo(ptClient2.x + 4, ptClient2.y + 4);
    pDC->MoveTo(ptClient2.x + 3, ptClient2.y - 3);
    pDC->LineTo(ptClient2.x - 4, ptClient2.y + 4);

    pDC->MoveTo(ptClient2.x, ptClient2.y);
    pDC->LineTo(ptClient1.x, ptClient1.y);
}

void CPicView::OnUpdate(CView *pSender, LPARAM lHint, CObject *pHint)
{
    // We have a new thing.
    __super::OnUpdate(pSender, lHint, pHint);

    if (lHint & VIEWUPDATEHINT_ZOOMLEVELCHANGED)
    {
        int iZoom = GetDocument()->GetZoom();
        if (iZoom)
        {
            int iOldZoom = _cxPic / sPIC_WIDTH;
            if (iOldZoom != iZoom)
            {
                int iMultiplier;
                if (iZoom > iOldZoom)
                {
                    iMultiplier = iZoom / iOldZoom;
                }
                else
                {
                    iMultiplier = -iOldZoom / iZoom;
                }
                // Figure out the center of the zoom
                CPoint ptZoom;
                CRect rcClient;
                GetClientRect(&rcClient);
                // Does the pic occupy less than the entire client area?
                if (_cxPic < rcClient.Width())
                {
                    // Yes... zoom around the center of the pic
                    ptZoom.SetPoint(sPIC_WIDTH / 2, sPIC_HEIGHT / 2);
                }
                else
                {
                    // Otherwise zoom around the center of the client area
                    ptZoom = _MapClientPointToPic(rcClient.CenterPoint());
                }
                _OnZoomClick(&ptZoom, iMultiplier);
            }
        }
    }

    if (lHint & (VIEWUPDATEHINT_NEWPIC | VIEWUPDATEHINT_EDITPICPOSCHANGE | VIEWUPDATEHINT_PALETTECHANGED))
    {
        // Need to redraw the pic, yup
        Invalidate(FALSE);
    }

    if (lHint & VIEWUPDATEHINT_PREFERENCESCHANGED)
    {
        // The scale/crop for trace images might have changed, so regenrate that
        _fRegenerateTrace = true;
        Invalidate(FALSE);
    }

    if (lHint & VIEWUPDATEHINT_PENSTYLECHANGE)
    {
        // Be smart and switch into pen mode.
        OnPenCommand();
    }
}

//
// prevent flicker
//
BOOL CPicView::OnEraseBkgnd(CDC *pDC)
{
    return TRUE;
}

// FEATURE: Do the replace-colour tool
// PosFromPoint, then do a setvisual of the current visual colour
// then after the point (now 1 after) restore it.
// Before doing a setvisual, see if you can go before and remove a setvisual
// e.g. if not drawing commands were done before the last setvisual

void CPicView::_OnHistoryLClick(CPoint point)
{
    INT_PTR iPos = _GetDrawManager().PosFromPoint(point.x, point.y, -1);

    // We'll set the pos!
    GetDocument()->SeekToPos(iPos);
}

void CPicView::_OnHistoryRClick(CPoint point)
{
    INT_PTR iPos = _GetDrawManager().PosFromPoint(point.x, point.y, -1);
    iPos++;

    // We'll set the pos!
    GetDocument()->SeekToPos(iPos);
}

void CPicView::_MakeRandomNR()
{
    // If we're doing random bit patterns, make a new one now.
    CPicDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);        
    if (pDoc)
    {
        PenStyle penStyle;
        pDoc->GetPenStyle(&penStyle);
        if (penStyle.fRandomNR)
        {
            _bRandomNR = (rand() % 128);
        }
    }
}

void CPicView::_OnPatternLClick(CPoint point)
{
    CPicDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);        
    if (pDoc)
    {
        PenStyle penStyle;
        pDoc->GetPenStyle(&penStyle);
        PicCommand command = PicCommand::CreatePattern((WORD)point.x, (WORD)point.y,
                            penStyle.bPatternSize, _bRandomNR, // Use _bRandomNR even if we're not randomly cycling it.
                            penStyle.fPattern, penStyle.fRectangle);
        pDoc->InsertCommand(&command);

        _MakeRandomNR();
    }
}

void CPicView::_OnPatternRClick()
{
    // cycle patternNR.
    _bRandomNR++;
    _bRandomNR = _bRandomNR % 128;

    // FEATURE: we can really optimize the clip region here.
    Invalidate(FALSE);
}

void CPicView::_OnCircleLClick(CPoint point)
{
    if (!_fDrawingCircle)
    {
        _fDrawingCircle = TRUE;
        // Start the line here:
        _xOld = point.x;
        _yOld = point.y;
    }
    else
    {
        // Actually draw a line. (Insert a line command)
        ASSERT(_xOld != -1);
        ASSERT(_yOld != -1);

        PicCommand command;
        command.CreateCircle((WORD)_xOld, (WORD)_yOld, (WORD)point.x, (WORD)point.y);
        GetDocument()->InsertCommand(&command);
        _fDrawingCircle = FALSE; // Start anew
    }    
}

void CPicView::_OnLineLClick(CPoint point)
{
    if (!_fDrawingLine)
    {
        _fDrawingLine = TRUE;
        // Start the line here:
        _xOld = point.x;
        _yOld = point.y;
    }
    else
    {
        // Actually draw a line. (Insert a line command)
        ASSERT(_xOld != -1);
        ASSERT(_yOld != -1);

        PicCommand command = PicCommand::CreateLine((WORD)_xOld, (WORD)_yOld, (WORD)point.x, (WORD)point.y);
        GetDocument()->InsertCommand(&command);

        // And update our coords.
        _xOld = point.x;
        _yOld = point.y;
    }
}

void CPicView::_OnLineRClick(CPoint point)
{
    if (_fDrawingLine)
    {
        // To clean up the line drawing turd.
        Invalidate(FALSE);
        _fDrawingLine = FALSE;
    }
    _xOld = -1;
    _yOld = -1;
}
void CPicView::_OnCircleRClick(CPoint point)
{
    if (_fDrawingCircle)
    {
        // To clean up the line drawing turd.
        Invalidate(FALSE);
        _fDrawingCircle = FALSE;
    }
    _xOld = -1;
    _yOld = -1;
}

void CPicView::_OnFillLClick(CPoint point)
{
    PicCommand command = PicCommand::CreateFill((WORD)point.x, (WORD)point.y);
    GetDocument()->InsertCommand(&command);
    // FEATURE: Here, we could sample the pixel, and determine that if nothing happened (i.e. the pixel wasn't white)
    // then we don't need to add a command, and we could even put a little message in the status bar.
}

//
// iMultiplier is 1 or -1
//
void CPicView::_OnZoomClick(CPoint *ppt, int iMultiplier)
{
    // Assumes already validated.
    CPoint ptScreenBefore;
    if (ppt)
    {
        // Zoom around where the user clicked!
         ptScreenBefore = _MapPicPointToClient(*ppt);
    }

    _cxPic = _cxPic + (sPIC_WIDTH * iMultiplier);
    _cyPic = _cyPic + (sPIC_HEIGHT * iMultiplier);
    _cxPic = max(_cxPic, sPIC_WIDTH);
    _cyPic = max(_cyPic, sPIC_HEIGHT);

    CPicDoc* pDoc = GetDocument();
    // Push to the document (it only fires an event when it changes, so we shouldn't get re-entered).
    pDoc->SetZoom(_cxPic / sPIC_WIDTH);

    CPoint ptScreenAfter;
    if (ppt)
    {
        ptScreenAfter = _MapPicPointToClient(*ppt);
    }
    CPoint ptDiff = ptScreenAfter - ptScreenBefore;
    // Now adjust origin by this amount
    _ScrollDelta(SB_HORZ, this->m_hWnd, ptDiff.x);
    _ScrollDelta(SB_VERT, this->m_hWnd, ptDiff.y);
    // Don't need to do this:
    //_InvalidateScrollStuff();
    Invalidate(FALSE);
}

//
// Make big
//
void CPicView::_OnZoomLClick(CPoint *ppt)
{
    if (_cxPic < (sPIC_WIDTH * 16))
    {
        _OnZoomClick(ppt, 1);
    }
    // Otherwise, make it no bigger
}

//
// Make small
//
void CPicView::_OnZoomRClick(CPoint *ppt)
{
    if (_cxPic > sPIC_WIDTH)
    {
        _OnZoomClick(ppt, -1);
    }
    // Otherwise, make it no smaller
}



//
// Returns the ID of the current tool or command.
// (-1 if none)
//
UINT CPicView::_GetCurrentToolOrCommand()
{
    int nID = -1;

    if (_currentTool != Command)
    {
        // Do we have a tool
        nID = LookupKeyValue(c_toolToID, ARRAYSIZE(c_toolToID), _currentTool, (UINT)-1);
    }

    if (nID == -1)
    {
        // or a command
        nID = LookupKeyValue(c_commandToID, ARRAYSIZE(c_commandToID), _currentCommand, (UINT)-1);
    }
    
    return nID;
}

BOOL PointIsNear(CPoint &pt1, CPoint &pt2)
{
    // Within 5 pixels
    return ((abs(pt1.x - pt2.x) < 5) && (abs(pt1.y - pt2.y) < 5));
}

bool CPicView::_NearPasteCorners()
{
    bool fRet = false;
    // See if we're near the edges.
    // Near the
    CRect rc(_xPasted, _yPasted, _xPasted + _cxPasted, _yPasted + _cyPasted);
    rc.OffsetRect(_xPasteOffset, _yPasteOffset);
    if (PointIsNear(_ptCurrentHover, rc.TopLeft()) ||
        PointIsNear(_ptCurrentHover, rc.BottomRight()) ||
        PointIsNear(_ptCurrentHover, CPoint(rc.right, rc.top)) ||
        PointIsNear(_ptCurrentHover, CPoint(rc.left, rc.bottom)))
    {
        fRet = true;
    }
    return fRet;
}

void CPicView::_UpdateCursor()
{
    if (_fMouseWithin)
    {
        if (((_currentTool == Pasting) || ((_currentTool == None) && _fShowingEgo)) && _fInPasteRect)
        {
            // Show a special cursor while the mouse is hovered over the ego (and drawing tools are off) or paste rect
            int iResource = IDC_CURSORPASTE;
            if (_currentTool == Pasting)
            {
                if (_NearPasteCorners())
                {
                    iResource = IDC_CURSORROTATE;
                }
            }
            SetCursor(LoadCursor(AfxGetInstanceHandle(), MAKEINTRESOURCE(iResource)));
        }
        else
        {
            BOOL fUseDefault = TRUE;
            // Otherwise, show the cursor for the current tool.
            UINT nID = _GetCurrentToolOrCommand();
            if (nID != -1)
            {
                int idResource = _IDToCursor(nID);
                if (idResource != -1)
                {
                    fUseDefault = FALSE;
                    SetCursor(LoadCursor(AfxGetInstanceHandle(), MAKEINTRESOURCE(idResource)));
                }
            }

            if (fUseDefault)
            {
                SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
            }
        }
    }
}

//
// This must be called every time a tool or command is changed.
//
void CPicView::_OnCommandChanged()
{
    // Reset x and y
    _xOld = -1;
    _yOld = -1;

    // Special case for lines:
    if ((_currentTool != Command) || (_currentCommand != PicCommand::Line))
    {
        if (_fDrawingLine)
        {
            // To clean up the line drawing tool thing.
            Invalidate(FALSE);
            _fDrawingLine = FALSE;
        }
    }
    if ((_currentTool != Command) || (_currentCommand != PicCommand::Circle))
    {
        if (_fDrawingCircle)
        {
            // To clean up the line drawing tool thing.
            Invalidate(FALSE);
            _fDrawingCircle = FALSE;
        }
    }

    // Special case for pens:
    BOOL fPreviewPen = (_currentTool == Command) && (_currentCommand == PicCommand::Pattern);
    if (fPreviewPen != _fPreviewPen)
    {
        _fPreviewPen = fPreviewPen;
        Invalidate(FALSE);
    }

    if ((_currentTool != Pasting) && (!_pastedCommands.empty()))
    {
        _CleanUpPaste();
    }
}

void CPicView::OnLButtonDown(UINT nFlags, CPoint point)
{
    // Don't do anything if this is off the picture.
    CPoint ptPic = _MapClientPointToPic(point);
    if ((ptPic.x >= sPIC_WIDTH) || (ptPic.y >= sPIC_HEIGHT))
    {
        return;
    }

    switch (_currentTool)
    {
    case History:
        {
            _OnHistoryLClick(ptPic);
            break;
        }

    case Zoom:
        {
            _OnZoomLClick(&ptPic);
            break;
        }

    case Command:
        {
            switch(_currentCommand)
            {
            case PicCommand::Pattern:
                {
                    _OnPatternLClick(ptPic);
                    break;
                }
            case PicCommand::Line:
                {
                    _OnLineLClick(ptPic);
                    break;
                }
            case PicCommand::Fill:
                {
                    _OnFillLClick(ptPic);
                    break;
                }
            case PicCommand::Circle:
                {
                    _OnCircleLClick(ptPic);
                    break;
                }
            }
            break;
        }

    case Pasting:
        {
            CRect rectPasted;
            _GetPasteRect(rectPasted);
            if (!_pastedCommands.empty() && PtInRect(&rectPasted, CPoint(ptPic.x, ptPic.y)))
            {
                _fCapturing = TRUE;
                _pointCapture = ptPic;

                _xPasteOffsetFromCapture = _xPasteOffset;
                _yPasteOffsetFromCapture = _yPasteOffset;

                // Angle where user clicked (in case they're rotating)
                CRect rc(_xPasted, _yPasted, _xPasted + _cxPasted, _yPasted + _cyPasted);
                rc.OffsetRect(_xPasteOffset, _yPasteOffset);
                _iAngleCapture = _iAngle;

                _fRotating = _NearPasteCorners();

                SetCapture();
                Invalidate(FALSE);
            }
            break;
        }

    case None:
        {
            if (_fShowingEgo && _HitTestFakeEgo(ptPic))
            {
                // The ego is showing.  Start capture.
                _fCapturing = TRUE;
                _pointCapture = ptPic;
                _pointEgoOrig = _pointEgo;
                _fCanBeHere = _EvaluateCanBeHere(_pointEgoOrig);
                SetCapture();
                Invalidate(FALSE);
            }
            break;
        }
    }
}

bool CPicView::_EvaluateCanBeHere(CPoint pt)
{
    const BYTE *pdataControl = _GetDrawManager().GetControlBits();
    if (theApp._fObserveControlLines)
    {
        return CanBeHere(pdataControl, GetViewBoundsRect((WORD)pt.x, (WORD)pt.y, theApp.GetSelectedViewResource(), _nFakeLoop, _nFakeCel));
    }
    else
    {
        return true;
    }
}

BOOL CPicView::_HitTestFakeEgo(CPoint ptPic)
{
    BOOL fHitTest = FALSE;
    if (!theApp._fUseBoxEgo && theApp.GetSelectedViewResource())
    {
        fHitTest = HitTestView((WORD)ptPic.x, (WORD)ptPic.y, (WORD)_pointEgo.x, (WORD)_pointEgo.y, theApp.GetSelectedViewResource(), _nFakeLoop, _nFakeCel);
    }
    else
    {
        fHitTest = HitTestEgoBox((WORD)ptPic.x, (WORD)ptPic.y, (WORD)_pointEgo.x, (WORD)_pointEgo.y, theApp._cxFakeEgo, theApp._cyFakeEgo);
    }
    return fHitTest;
}

//
// Center of ego in pic coords.
//
CPoint CPicView::_FindCenterOfFakeEgo()
{
    if (theApp._fUseBoxEgo)
    {
        return CPoint(_pointEgo.x, _pointEgo.y - theApp._cyFakeEgo / 2);
    }
    else
    {
        return FindCenterOfView((WORD)_pointEgo.x, (WORD)_pointEgo.y, theApp.GetSelectedViewResource(), _nFakeLoop, _nFakeCel);
    }
}

void CPicView::OnLButtonUp(UINT nFlags, CPoint point)
{
    if (_fCapturing)
    {
        ASSERT((_fShowingEgo && (_currentTool == None)) || (_currentTool == Pasting));
        _fCapturing = FALSE;
        ReleaseCapture();
    }
}

void CPicView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    CPoint ptPic = _MapClientPointToPic(point);

    // Don't do anything if this is off the picture.
    if ((ptPic.x >= sPIC_WIDTH) || (ptPic.y >= sPIC_HEIGHT))
    {
        return;
    }

    switch (_currentTool)
    {
    case Zoom:
        {
            // Zoom in on the 2nd click too
            _OnZoomLClick(&ptPic);
            break;
        }

    case Pasting:
        {
            _InsertPastedCommands();
            // Get out of pasting mode:
            _currentTool = None;
            _UpdateCursor();
            _OnCommandChanged();
            Invalidate(FALSE);
        }

    case Command:
        {
            // Perform pattern and line commands on the 2nd click too
            switch(_currentCommand)
            {
            case PicCommand::Pattern:
                {
                    _OnPatternLClick(ptPic);
                    break;
                }
            case PicCommand::Line:
                {
                    _OnLineLClick(ptPic);
                    break;
                }
            }
            break;
        }

    case None:
        {
            if (_fShowingEgo)
            {
                // The ego is showing - double click centers it around the mouse again.
                _pointEgo = ptPic;
                _pointEgo.y += theApp._cyFakeEgo / 2;
                _fCanBeHere = _EvaluateCanBeHere(_pointEgo);
                theApp._ptFakeEgo = _pointEgo;
                Invalidate(FALSE);
            }
            break;
        }

    }
}


void CPicView::OnRButtonDown(UINT nFlags, CPoint point)
{
    CPoint ptPic = _MapClientPointToPic(point);

    // Don't do anything if this is off the picture.
    if ((ptPic.x >= sPIC_WIDTH) || (ptPic.y >= sPIC_HEIGHT))
    {
        return;
    }

    switch (_currentTool)
    {
    case History:
        {
            _OnHistoryRClick(ptPic);
            break;
        }
    case Zoom:
        {
            _OnZoomRClick(&ptPic);
            break;
        }

    case Command:
        {
            switch(_currentCommand)
            {
            case PicCommand::Line:
                {
                    _OnLineRClick(ptPic);
                    break;
                }
            case PicCommand::Pattern:
                {
                    _OnPatternRClick();
                    break;
                }
            case PicCommand::Circle:
                {
                    _OnCircleRClick(ptPic);
                    break;
                }
            }
            break;
        }

    case Pasting:
        {
            // On right click, stop pasting.
            _CleanUpPaste();
            _currentTool = None;
            _UpdateCursor();
            _OnCommandChanged();
            Invalidate(FALSE);
            break;
        }

    case None:
        {
            if (_fShowingEgo && _HitTestFakeEgo(ptPic))
            {
                // Bring up a menu that let's us choose the ego's priority.
                CMenu contextMenu; 
                contextMenu.LoadMenu(IDR_MENUFAKEEGO); 
                CMenu* pTracker; 
                pTracker = contextMenu.GetSubMenu(0); 
                CPoint ptScreen = point;
                ClientToScreen(&ptScreen);
                if (pTracker)
                {
                    BOOL fRet = pTracker->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, 
                                                         ptScreen.x, ptScreen.y, AfxGetMainWnd()); 
                    Invalidate(FALSE);
                }
            }
            break;
        }
    }
}


void CPicView::OnRButtonDblClk(UINT nFlags, CPoint point)
{
    CPoint ptPic = _MapClientPointToPic(point);

    // Don't do anything if this is off the picture.
    if ((ptPic.x >= sPIC_WIDTH) || (ptPic.y >= sPIC_HEIGHT))
    {
        return;
    }

    switch (_currentTool)
    {
    case Zoom:
        {
            // Zoom in on the 2nd click too
            _OnZoomRClick(&ptPic);
            break;
        }

    case Command:
        {
            switch(_currentCommand)
            {
            case PicCommand::Pattern:
                {
                    // Do the same as R-click
                    _OnPatternRClick();
                }
                break;
            }
            break;
        }
    }
}


void CPicView::_OnMouseWheel(UINT nFlags, BOOL fForward, CPoint pt, short nNotches)
{
    // This is cool.
    if (nFlags & MK_SHIFT)
    {
        // Shift held down.  Do a zoom:
        if (fForward)
        {
            _OnZoomLClick();
        }
        else
        {
            _OnZoomRClick();
        }
    }

    // REVIEW: Disabled horizontal for now.
    if (nFlags == 0)
    {
        // No keys held down.  Do a scroll by 5 pics coordinates.
        int iAmount = 5 * (_cxPic / sPIC_WIDTH) * nNotches;

        // Which scroll bar?  Which one are we closest to?
        RECT rc;
        GetClientRect(&rc);
        _ScrollDelta(SB_VERT, m_hWnd, fForward ? -iAmount : iAmount);
    }
}

//
// This just controls the cel/loop of the fake ego.
//
void CPicView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    BOOL fHandled = TRUE;
    if (_currentTool == Pasting)
    {
        // TODO: temporary, until there is UI
        switch (nChar)
        {
        case VK_RIGHT:
            {
                _cxPastedScale++;
                break;
            }
        case VK_LEFT:
            {
                _cxPastedScale--;
                break;
            }
        case VK_UP:
            {
                _cyPastedScale--;
                break;
            }
        case VK_DOWN:
            {
                _cyPastedScale++;
                break;
            }
        case VK_NUMPAD4:
        case VK_NUMPAD6:
            _fVFlip = !_fVFlip;
            break;
        case VK_NUMPAD8:
        case VK_NUMPAD2:
            _fHFlip = !_fHFlip;
            break;

        case VK_NUMPAD7:
            _iAngle -= 5;
            break;
        case VK_NUMPAD9:
            _iAngle += 5;
            break;

        default:
            {
                fHandled = FALSE;
                break;
            }
        }
        if (fHandled)
        {
            Invalidate(FALSE);
        }
    }
    else
    {
        switch (nChar)
        {
        case VK_RIGHT:
            {
                _nFakeCel++;
                break;
            }
        case VK_LEFT:
            {
                _nFakeCel--;
                break;
            }
        case VK_UP:
            {
                _nFakeLoop--;
                break;
            }
        case VK_DOWN:
            {
                _nFakeLoop++;
                break;
            }
        default:
            {
                fHandled = FALSE;
                break;
            }
        }
        if (fHandled && _fShowingEgo)
        {
            // We changed cel/loop
            Invalidate(FALSE);
        }
    }

    if (!fHandled)
    {
        __super::OnKeyDown(nChar, nRepCnt, nFlags);
    }
}


// CPicView printing

BOOL CPicView::OnPreparePrinting(CPrintInfo* pInfo)
{
    // default preparation
    return DoPreparePrinting(pInfo);
}

void CPicView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

void CPicView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

const PicResource *CPicView::_GetEditPic()
{
    const PicResource *ppic = NULL;
    if (_pPreview.get())
    {
        ppic = &_pPreview->GetPicResource();
    }
    else
    {
        CPicDoc* pDoc = GetDocument();
        ASSERT_VALID(pDoc);
        if (pDoc)
        {
            ppic = pDoc->GetPicResource();
        }
    }
    return ppic;
}


// CPicView diagnostics

#ifdef _DEBUG
void CPicView::AssertValid() const
{
    __super::AssertValid();
}

void CPicView::Dump(CDumpContext& dc) const
{
    __super::Dump(dc);
}

CPicDoc* CPicView::GetDocument() const // non-debug version is inline
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPicDoc)));
    return (CPicDoc*)m_pDocument;
}
#endif //_DEBUG





// CPicView message handlers
