//
// RasterResourceListView.cpp : implementation of the RasterResourceListView class
//
// This is the listview that shows all the pics in a game.
//

#include "stdafx.h"
#include "ViewResource.h"
#include "SCIPicEditor.h"
#include "ResourceListDoc.h"
#include "ResourceListView.h"
#include "RasterResourceListView.h"
#include "ResourceManagerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define VIEW_IMAGE_SIZE 40

// CResourceListListView

IMPLEMENT_DYNCREATE(CRasterResourceListCtrl, CResourceListCtrl)

BEGIN_MESSAGE_MAP(CRasterResourceListCtrl, CResourceListCtrl)
    ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
    ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnItemChanged)
    ON_NOTIFY_REFLECT(NM_DBLCLK, OnItemDoubleClick)
END_MESSAGE_MAP()

// CResourceListListView construction/destruction

CRasterResourceListCtrl::CRasterResourceListCtrl()
{
    _himlPics = NULL;
    _iCorruptBitmapIndex = 0;
}

CRasterResourceListCtrl::~CRasterResourceListCtrl()
{
}

void CRasterResourceListCtrl::OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
    NMLISTVIEW *pnmlv = (NMLISTVIEW*)pNMHDR;
    if (pnmlv->uChanged & LVIF_STATE)
    {
        if (pnmlv->uNewState & LVIS_SELECTED)
        {
            // Set the "global selected view"
            // (use for InsertObject dialog, or the fake ego in the pic editor)
            theApp._iView = _GetResourceForItem(pnmlv->iItem)->GetNumber();
        }
    }
    __super::OnItemClick(pNMHDR, pResult); // Our base class does some processing...
}

void CRasterResourceListCtrl::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
    LV_ITEM *pItem = &((LV_DISPINFO*)pNMHDR)->item; 

    if( pItem->mask & LVIF_IMAGE ) 
    { 
        if (_iView == LVS_ICON)
        {
            // Just in case we can't get the view:
            pItem->iImage = _iCorruptBitmapIndex;
            pItem->mask |= LVIF_DI_SETITEM; // So we don't ask for it again.

            ResourceBlob *pData = (ResourceBlob*)pItem->lParam;

            IResourceBase *pResource;
            if (SUCCEEDED(CreateResourceFromResourceData(pData, &pResource)))
            {
                IRasterResource *prr;
                if (pResource->QueryCapability(IID_PPV_ARGS(&prr)))
                {
                    HBITMAP hbm = prr->GetBitmap(prr->GetPreviewIndex(), VIEW_IMAGE_SIZE, VIEW_IMAGE_SIZE);
                    if (hbm)
                    {
                        int iIndex = ImageList_Add(_himlPics, hbm, NULL);
                        if (iIndex != -1)
                        {
                            pItem->iImage = iIndex; // Done!
                        }
                        DeleteObject(hbm);
                    }
                }
                delete pResource;
            }
        }
        // Otherwise, in report view, we don't provide it.
    } 
    
    *pResult = 0; 
}



void CRasterResourceListCtrl::_OnInitListView(int cItems)
{
    // Put an imagelist in the view, for our pic previews.

    // Destroy any old list
    if (_himlPics)
    {
        ImageList_Destroy(_himlPics);
        _himlPics = NULL;
    }

    CSize sizeImages(VIEW_IMAGE_SIZE, VIEW_IMAGE_SIZE);
    // Note: this color depth must match that which we get back from the ViewResource
    // Also: ILC_MASK must be specified, because of the overlay
    _himlPics = ImageList_Create(sizeImages.cx, sizeImages.cy, ILC_COLOR8 | ILC_MASK, cItems, 30);
    
    if (_himlPics)
    {
        CImageList *pTemp = CImageList::FromHandle(_himlPics);
        if (pTemp)
        {
            HIMAGELIST himlOld = (HIMAGELIST)SetImageList(pTemp, LVSIL_NORMAL);
        }
        // pTemp is a temporary object that we don't need to delete.

        // Load an overlay image to use for "unused" items
        HBITMAP hbmOverlay = LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_OVERLAYOLD40));
        if (hbmOverlay)
        {
            int iOverlayImageIndex = ImageList_AddMasked(_himlPics, hbmOverlay, RGB(255, 0, 255));
            ImageList_SetOverlayImage(_himlPics, iOverlayImageIndex, OVERLAYINDEX_UNUSED);
            DeleteObject(hbmOverlay);
        }

        HBITMAP hbmCorrupt = LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_CORRUPTRESOURCE));
        if (hbmCorrupt)
        {
            _iCorruptBitmapIndex = ImageList_Add(_himlPics, hbmCorrupt, NULL);
            DeleteObject(hbmCorrupt);
        }
    }

    // Adjust the icon spacing so things don't look too spread out.
    CSize sizeSpacing(sizeImages.cx + 20, sizeImages.cy + 30);
    SetIconSpacing(sizeSpacing);

    // LVS_EX_BORDERSELECT, supported on IE 4.0 or later. -> Removed, as it causes problems in details view
    // LVS_EX_DOUBLEBUFFER, supported on XP or later.
    SetExtendedStyle(GetExtendedStyle() | 0x00010000 | LVS_EX_INFOTIP);

}

void CRasterResourceListCtrl::_PrepareLVITEM(LVITEM *pItem)
{
    __super::_PrepareLVITEM(pItem);

    // Say that we want to be called back for images.
    pItem->mask |= LVIF_IMAGE;
    pItem->iImage = I_IMAGECALLBACK;

    const ResourceBlob *pData = (const ResourceBlob*)pItem->lParam;
    if (pData)
    {
        // Add an overlay that indicates it is not the most recent.
        if (!theApp._resourceRecency.IsResourceMostRecent(pData))
        {
            pItem->mask |= LVIF_STATE;
            pItem->state = INDEXTOOVERLAYMASK(OVERLAYINDEX_UNUSED);
            pItem->stateMask = LVIS_OVERLAYMASK;
        }
    }
}

// CResourceListListView diagnostics

#ifdef _DEBUG
void CRasterResourceListCtrl::AssertValid() const
{
    __super::AssertValid();
}

void CRasterResourceListCtrl::Dump(CDumpContext& dc) const
{
    __super::Dump(dc);
}

#endif //_DEBUG

// CResourceListListView message handlers


 
 

