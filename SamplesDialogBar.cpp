// QuickScripts.cpp : implementation file
//

#include "stdafx.h"
#include "SCIPicEditor.h"
#include "SamplesDialogBar.h"
#include "ScriptDocument.h"
#include "PicResource.h"
#include "ResourceListDoc.h"

using namespace sci;
using namespace std;

#define MAX_SAMPLES (ID_GOTOVIEW28 - ID_GOTOVIEW1 + 1)

// CSamplesDialogBar dialog
CSamplesDialogBar::CSamplesDialogBar(CWnd* pParent /*=NULL*/)
	: CExtResizableDialog(CSamplesDialogBar::IDD, pParent)
{
    _bFirstTime = true;
    _pDoc = NULL;
    _samples.resize(MAX_SAMPLES);
}

CSamplesDialogBar::~CSamplesDialogBar()
{
}


void CSamplesDialogBar::_PrepareViewMenu(int cItems)
{
    if (m_wndButtonViews.m_menu == NULL)
    {
        CMenu menu;
        if (menu.LoadMenu(IDR_MENUVIEWS))
        {
            // We only need to do this once...
            m_wndButtonViews.m_menu.Attach(menu.Detach());
        }
    }
    if (m_wndButtonViews.m_menu)
    {
        UINT nID;
        if (_GetMenuItem("placeholder", &m_wndButtonViews.m_menu, &nID))
        {
            CMenu *pMenuSub = m_wndButtonViews.m_menu.GetSubMenu(nID);

            // Clear the menu out first.
            UINT cItemsInMenu = pMenuSub->GetMenuItemCount();
            while (cItemsInMenu > 0)
            {
                pMenuSub->RemoveMenu(0, MF_BYPOSITION);
                cItemsInMenu--;
            }

            // And rebuild
            for (int iIndex = 0; iIndex < cItems; iIndex++)
            {
                MENUITEMINFO mii = { 0 };
                mii.cbSize = sizeof(mii);
                mii.fMask = MIIM_ID | MIIM_STRING;
                mii.wID = ID_GOTOVIEW1 + iIndex;
                mii.dwTypeData = "foo";
                pMenuSub->InsertMenuItem(ID_GOTOVIEW1 + iIndex, &mii, FALSE);
            }
        }
    }
}


//
void CSamplesDialogBar::_PrepareSamples()
{
    _samples.clear();

    if (_pDoc)
    {
        ResourceType type = _pDoc->GetShownResourceType();
        if (type != RS_NONE)
        {
            // Find all the samples in a particular folder.
            std::string samplesFolder = theApp.GetResourceMap().GetSamplesFolder() + "\\" + g_resourceInfo[type].pszFileNameDefault;
            std::string findFirstString = samplesFolder + "\\*.bin";
            WIN32_FIND_DATA findData;
            HANDLE hFolder = FindFirstFile(findFirstString.c_str(), &findData);
            if (hFolder != INVALID_HANDLE_VALUE)
            {
                BOOL fOk = TRUE;
                while (fOk)
                {
                    std::string fileName = samplesFolder + "\\" + findData.cFileName;
                    ResourceBlob blob;
                    // Use the filename minus the .bin:
                    TCHAR *pszExt = PathFindExtension(findData.cFileName);
                    *pszExt = 0; // Get rid of it.
                    if (SUCCEEDED(blob.CreateFromFile(findData.cFileName, fileName.c_str(), -1, -1)))
                    {
                        if (blob.GetType() == type)
                        {
                            _samples.push_back(blob);
                        }
                    }
                    fOk = FindNextFile(hFolder, &findData);
                }
                FindClose(hFolder);
            }

            // Now go through all the samples we got.
            for (size_t iIndex = 0; iIndex < _samples.size() && iIndex < MAX_SAMPLES; ++iIndex)
            {
                UINT nID = ID_GOTOVIEW1 + (UINT)iIndex;
                // Ensure we have a command entry for this.
                CExtCmdItem *pCmdItem;
	            pCmdItem = g_CmdManager->CmdGetPtr(theApp._pszCommandProfile, nID);
                if (pCmdItem == NULL)
                {
                    pCmdItem = g_CmdManager->CmdAllocPtr(theApp._pszCommandProfile, nID);
                }
                // Update the command entry with an icon and text
                if (pCmdItem)
                {
                    // Try to get a bitmap.
                    CBitmap bitmap;
                    IResourceBase *pResource;
                    if (SUCCEEDED(CreateResourceFromResourceData(&_samples[iIndex], &pResource)))
                    {
                        auto_ptr<IResourceBase> resource(pResource);
                        ASSERT(resource.get()); // Since we succeeded CreateResource
                        DWORD dwIndex = MAKE_INDEX(0, 0); // Normally we'll use cel/loop 0
                        if (pResource->GetType() == RS_FONT)
                        {
                            // But not for fonts...
                            dwIndex = 65; // 'a'
                        }
                        IRasterResource *prr;
                        if (pResource->QueryCapability(IID_PPV_ARGS(&prr)))
                        {
                            // Note: if the index is out of bounds, it will return a NULL HBITMAP
                            bitmap.Attach(prr->GetBitmap(dwIndex, 24, 24));
                        }
                        else if (type == RS_PIC)
                        {
                            // Special handler for pics.
                            PicResource *pPic = static_cast<PicResource*>(pResource);
                            bitmap.Attach(pPic->GetVisualBitmap(40, 24));
                        }
                        if ((HBITMAP)bitmap)
                        {
                            CExtBitmap extBitmap;
                            extBitmap.FromBitmap((HBITMAP)bitmap);
                            g_CmdManager->CmdSetIcon(theApp._pszCommandProfile, nID, extBitmap, RGB(255, 255, 255), CRect(0, 0, 24, 24));
                        }
                    }

                    // The text:
                    pCmdItem->m_sMenuText = _samples[iIndex].GetName().c_str();
                    pCmdItem->m_sTipTool = pCmdItem->m_sMenuText;
                }
            }
        }
    }
}

void CSamplesDialogBar::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_BUTTONSAMPLES, m_wndButtonViews);
    m_wndButtonViews.SetSamplesDialogBar(this);
}

BOOL CSamplesDialogBar::OnInitDialog()
{
    BOOL fRet = __super::OnInitDialog();
    ShowSizeGrip(FALSE);
    AddAnchor(IDC_BUTTONSAMPLES, CPoint(100, 0), CPoint(100, 0));
    return fRet;
}

void CSamplesDialogBar::TrackPopup()
{
    _ResetUI();
}

BEGIN_MESSAGE_MAP(CSamplesDialogBar, CExtResizableDialog)
    ON_UPDATE_COMMAND_UI(ID_VIEWMENU, OnUpdateAlwaysOn)
    ON_COMMAND_RANGE(ID_GOTOVIEW1, ID_GOTOVIEW28, OnGotoView)
END_MESSAGE_MAP()


// CSamplesDialogBar message handlers



void CSamplesDialogBar::Initialize()
{
    if (_bFirstTime)
    {

    }
}


void CSamplesDialogBar::_ResetUI()
{
    _PrepareSamples();
    _PrepareViewMenu((int)_samples.size());
}


void CSamplesDialogBar::OnGotoView(UINT nID)
{
    ResourceBlob &blob = _samples[nID - ID_GOTOVIEW1];
    theApp.GetResourceMap().AppendResourceAskForNumber(blob);
}


void CSamplesDialogBar::SetDocument(CDocument *pDoc)
{
    _pDoc = static_cast<CResourceListDoc*>(pDoc);
    if (_pDoc)
    {
        ASSERT(pDoc->IsKindOf(RUNTIME_CLASS(CResourceListDoc)));
        _pDoc->AddNonViewClient(this);
    }
}

// INonViewClient
void CSamplesDialogBar::UpdateNonView(LPARAM lHint)
{
    if (_pDoc && (lHint & VIEWUPDATEHINT_SHOWRESOURCETYPE))
    {
        // m_wndButtonViews
        std::string message = "Insert ";
        
        ResourceType type = _pDoc->GetShownResourceType();
        if (type >= 0 && type < (int)g_celResourceInfo)
        {
            message += g_resourceInfo[type].pszFileNameDefault;
            message += " sample";
            m_wndButtonViews.SetWindowText(message.c_str());

            m_wndButtonViews.EnableWindow(type != RS_NONE && type != RS_SCRIPT && type != RS_TEXT && type != RS_VOCAB);
        }
    }
}

bool CExtDelayedButton::_OnTrackPopup(bool bSelectAny)
{
    _pDlgBar->TrackPopup();
    return __super::_OnTrackPopup(bSelectAny);
}
