
#include "stdafx.h"
#include <math.h>

bool _IsNumber(TCHAR ch, BOOL fHex)
{
    return isdigit(ch) || (fHex && isxdigit(ch));
}

namespace sci
{

    bool istream::_Grow(DWORD cbGrowMin)
    {
        bool fRet = FALSE;
        // Grow by 50% or cbGrowMin
        DWORD cbSizeNew = max(_cbSize + (_cbSize / 3 * 2), _cbSize + cbGrowMin);
        BYTE *pDataNew = new BYTE[cbSizeNew];
        if (pDataNew)
        {
            CopyMemory(pDataNew, _pData, _cbSize);
            delete [] _pData;
            _pData = pDataNew;
            _cbSize = cbSizeNew;
            fRet = TRUE;
        }
        return fRet;
    }

    bool istream::WriteByte(BYTE b)
    {
        bool fRet = TRUE;
        if (_iIndex >= _cbSize)
        {
            fRet = _Grow(1);
        }

        if (fRet)
        {
            _pData[_iIndex] = b;
            _iIndex++;
            _cbSizeValid = _iIndex;
        }

        return fRet;
    }

    bool istream::WriteWord(WORD w)
    {
        bool fRet = WriteByte((BYTE)(w & 0xff));
        if (fRet)
        {
            fRet = WriteByte((BYTE)(w >> 8));
        }
        return fRet;
    }


    bool istream::WriteBytes(const BYTE *pDataIn, int cCount)
    {
        bool fRet = true;
        if ((_iIndex + cCount) >= _cbSize)
        {
            fRet = _Grow((DWORD)cCount);
        }

        if (fRet)
        {
            CopyMemory(&_pData[_iIndex], pDataIn, (size_t)cCount);
            _iIndex += (DWORD)cCount;
            _cbSizeValid = _iIndex;
        }
        return fRet;
    }

    bool istream::_Read(BYTE *pDataDest, DWORD cCount)
    {
        bool fRet = false;
        if ((_cbSizeValid > _iIndex) && ((_cbSizeValid - _iIndex) >= cCount))
        {
            // we're good.
            fRet = _pData || _pDataReadOnly;
            if (_pData)
            {
                CopyMemory(pDataDest, _pData + _iIndex, cCount);
            }
            else
            {
                CopyMemory(pDataDest, _pDataReadOnly + _iIndex, cCount);
            }
            _iIndex += cCount;
        }
        return fRet;
    }

    bool istream::Seek(DWORD iIndexRequested)
    {
        bool fRet = FALSE;
        if (iIndexRequested < _cbSizeValid)
        {
            fRet = TRUE;
            _iIndex = iIndexRequested;
        }
        return fRet;
    }

    istream::istream(HANDLE hFile)
    {
        _state = std::ios_base::goodbit;
        _pData = NULL;
        _cbSize = 0;
        _iIndex = 0;
        _cbSizeValid = 0;
        _pDataReadOnly = NULL;
        DWORD dwSizeHigh;
        DWORD dwSize = GetFileSize(hFile, &dwSizeHigh);
        if (dwSize != INVALID_FILE_SIZE)
        {
            // Limit ourselves a little.
            if ((dwSizeHigh == 0) && (dwSize < 67108864))  // 2 ^ 26
            {
                _pData = new BYTE [dwSize];
                if (_pData)
                {
                    DWORD dwSizeRead;
                    if (ReadFile(hFile, _pData, dwSize, &dwSizeRead, NULL) && (dwSizeRead == dwSize))
                    {
                        _cbSize = dwSize;
                        _iIndex = 0;
                        _cbSizeValid = dwSize;
                    }
                }
            }
        }
    }

    istream &istream::operator>> (WORD &w)
    {
        DWORD dwSave = tellg();
        if (!_ReadWord(&w))
        {
            seekg(dwSave);
            w = 0;
            _state = std::ios_base::eofbit | std::ios_base::failbit;
        }
        return *this;
    }
    istream &istream::operator>> (BYTE &b)
    {
        DWORD dwSave = tellg();
        if (!_ReadByte(&b))
        {
            seekg(dwSave);
            b = 0;
            _state = std::ios_base::eofbit | std::ios_base::failbit;
        }
        return *this;
    }
    istream &istream::operator>> (std::string &str)
    {
        DWORD dwSave = tellg();
        str.clear();
        // Read a null terminated string.
        bool fDone = false;
        while (_iIndex < _cbSizeValid)
        {
            char x = (char)_pDataReadOnly[_iIndex];
            ++_iIndex;
            if (x)
            {
                str += x;
            }
            else
            {
                fDone = true;
                break;
            }
        }
        if (!fDone && (_iIndex == _cbSizeValid))
        {
            // Failure
            str.clear();
            seekg(dwSave);
            _state = std::ios_base::eofbit | std::ios_base::failbit;
        }
        return *this;
    }

    void istream::skip(DWORD cBytes)
    {
        if ((_iIndex + cBytes) < _cbSizeValid)
        {
            _iIndex += cBytes;
        }
        else
        {
            _state = std::ios_base::eofbit | std::ios_base::failbit;
        }
    }

    bool istream::peek(BYTE &b)
    {
        if (_iIndex < _cbSizeValid)
        {
            b = _pDataReadOnly[_iIndex];
            return true;
        }
        return false;
    }

    void istream::getRLE(std::string &str)
    {
        // Vocab files strings are run-length encoded.
        WORD wSize;
        DWORD dwSave = tellg();
        *this >> wSize;
        if (wSize == 0)
        {
            str.clear();
        }
        else
        {
            while (good() && wSize)
            {
                BYTE b;
                *this >> b;
                if (b)
                {
                    str += (char)b;
                    --wSize;
                }
                else
                {
                    // Be very careful here... in addition to being RLE, these strings
                    // are null-terminated.  We don't want to go adding a null character to
                    // str, because it will not compare == when it should.
                    break;
                }
            }
        }
        if (!good())
        {
            str.clear();
            seekg(dwSave);
        }
    }
} // namespace sci

BOOL IsValidResourceType(int iResourceType)
{
    return (iResourceType >= 0) && (iResourceType < NumResourceTypes);
}

BOOL IsValidResourceNumber(int iResourceNum)
{
    return (iResourceNum >= 0) && (iResourceNum < 1000);
}


CGuard::CGuard(CRITICAL_SECTION *pcs)
{
    _pcs = pcs;
    EnterCriticalSection(_pcs);
}

CGuard::~CGuard()
{
    LeaveCriticalSection(_pcs);
}




typedef struct
{
    double h;
    double s;
    double l;
} HSL;

typedef struct
{
    double r;
    double g;
    double b;
} COLOUR;


HSL RGB2HSL(COLORREF color)
{
   double themin,themax,delta;
   HSL c2;
   COLOUR c1 = { ((double)GetRValue(color)) / 255.0, ((double)GetGValue(color)) / 255.0, ((double)GetBValue(color)) / 255.0 };

   themin = min(c1.r,min(c1.g,c1.b));
   themax = max(c1.r,max(c1.g,c1.b));
   delta = themax - themin;
   c2.l = (themin + themax) / 2;
   c2.s = 0;
   if (c2.l > 0 && c2.l < 1)
      c2.s = delta / (c2.l < 0.5 ? (2*c2.l) : (2-2*c2.l));
   c2.h = 0;
   if (delta > 0) {
      if (themax == c1.r && themax != c1.g)
         c2.h += (c1.g - c1.b) / delta;
      if (themax == c1.g && themax != c1.b)
         c2.h += (2 + (c1.b - c1.r) / delta);
      if (themax == c1.b && themax != c1.r)
         c2.h += (4 + (c1.r - c1.g) / delta);
      c2.h /= 6.0;
   }
   return(c2);
}

COLORREF g_rgColorCombos[256];  // EGACOLORS to COLORREFs - can be indexed by EGACOLOR_TO_BYTE
HSL g_rgColorCombosHSL[256];

// Palettes:
EGACOLOR g_rg136ToByte[136];    // The 136 unique EGACOLORS
EGACOLOR g_rg16ToByte[16];      // The 16 pure colors
EGACOLOR g_rgSmoothToByte[136]; // The smooth dithered EGACOLORS
int g_cSmoothEntries = 0;

BOOL g_bFilledCOLORREFArray = FALSE;

int GetNormalizedColorDistance(COLORREF color1, COLORREF color2);

//
// Fills an array with all possible colour combinations of the dithered EGA colors
// rgcolors must be 256 entries long
//
void FillCOLORREFArray()
{
    int iIndex = 0;
    int iIndex16 = 0;
    g_cSmoothEntries = 0;
    for (int i = 0; i < 16; i++)
    {
        for (int j = i; j < 16; j++)
        {
            // Calculate the 136 unique colours
            ASSERT(iIndex < ARRAYSIZE(g_rg136ToByte));
            RGBQUAD rgbq = _Combine(g_egaColors[i], g_egaColors[j]);
            COLORREF color = RGB(rgbq.rgbRed, rgbq.rgbGreen, rgbq.rgbBlue);
            // And the BYTE/EGACOLOR it maps to:
            g_rg136ToByte[iIndex].color1 = i;
            g_rg136ToByte[iIndex].color2 = j;
            if (i == j)
            {
                g_rg16ToByte[iIndex16].color1 = i;
                g_rg16ToByte[iIndex16].color2 = j;
                iIndex16++;
            }
            // See if this is a "smooth" colour
            if (200 > GetNormalizedColorDistance(g_egaColorsCR[i], g_egaColorsCR[j]))
            {
                g_rgSmoothToByte[g_cSmoothEntries].color1 = i;
                g_rgSmoothToByte[g_cSmoothEntries].color2 = j;
                g_cSmoothEntries++;
            }
            iIndex++;
        }
        for (int j = 0; j < 16; j++)
        {
            // This is a mapping of EGACOLORs to COLORREFs
            RGBQUAD rgbq = _Combine(g_egaColors[i], g_egaColors[j]);
            COLORREF color = RGB(rgbq.rgbRed, rgbq.rgbGreen, rgbq.rgbBlue);
            EGACOLOR egaColor = { i, j };
            g_rgColorCombos[EGACOLOR_TO_BYTE(egaColor)] = color; // the COLORREF color
            g_rgColorCombosHSL[EGACOLOR_TO_BYTE(egaColor)] = RGB2HSL(color);
        }
    }
    g_bFilledCOLORREFArray = TRUE;
}


//
// Returns the distance between 2 colours
//
int GetColorDistance(COLORREF color1, COLORREF color2)
{
    int redsq = ((int)GetRValue(color1)) - (int)GetRValue(color2);
    redsq *= redsq;
    int bluesq = ((int)GetBValue(color1)) - (int)GetBValue(color2);
    bluesq *= bluesq;
    int greensq = ((int)GetGValue(color1)) - (int)GetGValue(color2);
    greensq *= greensq;

    return (redsq + bluesq + greensq);
}

// (normalized euclidian RGB distance)
int GetNormalizedColorDistance(COLORREF color1, COLORREF color2)
{
    int iDistance = GetColorDistance(color1, color2);
    return (int)sqrt((double)iDistance);
}


//
// Differences between two colours, using HSL and euclidian distance
//
int GetColorDistance2(HSL one, COLORREF color2)
{
    HSL two = (RGB2HSL(color2));
    double hsq = (one.h - two.h);
    hsq *= hsq;
    double lsq = one.l - two.l;
    lsq *= lsq;
    double ssq = one.s - two.s;
    ssq *= ssq;

    return (int)(255.0 * (hsq + lsq + ssq));
}

//
// from http://www.compuphase.com/cmetric.htm
// 
int GetColorDistance3(COLORREF e1, COLORREF e2)
{
  int r,g,b;
  int rmean;

  rmean = ( (int)GetRValue(e1) + (int)GetRValue(e2)) / 2;
  r = (int)GetRValue(e1) - (int)GetRValue(e2);
  g = (int)GetGValue(e1) - (int)GetGValue(e2);
  b = (int)GetBValue(e1) - (int)GetBValue(e2);
  return (((512+rmean)*r*r)>>8) + 4*g*g + (((767-rmean)*b*b)>>8);
}

//
// Weighted RGB
// 
int GetColorDistance4(COLORREF color1, COLORREF color2)
{
    int redsq = ((int)GetRValue(color1)) - (int)GetRValue(color2);
    BOOL fMoreThan128 =  (abs(redsq) >= 128);
    redsq *= redsq;
    int bluesq = ((int)GetBValue(color1)) - (int)GetBValue(color2);
    bluesq *= bluesq;
    int greensq = ((int)GetGValue(color1)) - (int)GetGValue(color2);
    greensq *= greensq;

    if (fMoreThan128)
    {
        return (2 * redsq + 4 * greensq + 3 * bluesq);
    }
    else
    {
        return (3 * redsq + 4 * greensq + 2 * bluesq);
    }
}

//
// Combo of RGB and HSL
//
int GetColorDistance5(HSL one, COLORREF color1, COLORREF color2)
{
    return GetColorDistance(color1, color2) + GetColorDistance2(one, color2);
}



//
// Returns the closest matching EGACOLOR to color.
// The EGACOLORs are chosen from rgColors
//
EGACOLOR GetClosestEGAColorFromSet(int iAlgorithm, COLORREF color, EGACOLOR *rgColors, int cColors)
{
    if (!g_bFilledCOLORREFArray)
    {
        FillCOLORREFArray();
        ASSERT(g_bFilledCOLORREFArray);
    }

    int closest = INT_MAX;
    int leastdithered = INT_MAX;
    EGACOLOR egacolor = { 0 };
    for (int iIndex = 0; iIndex < cColors; iIndex++)
    {
        BYTE egaIndex = EGACOLOR_TO_BYTE(rgColors[iIndex]);
        int closeness;
        switch (iAlgorithm)
        {
        case 1:
            closeness = GetColorDistance(g_rgColorCombos[egaIndex], color);
            break;
        case 2:
            closeness = GetColorDistance2(g_rgColorCombosHSL[egaIndex], color);
            break;
        case 3:
            closeness = GetColorDistance3(g_rgColorCombos[egaIndex], color);
            break;
        case 4:
            closeness = GetColorDistance4(g_rgColorCombos[egaIndex], color);
            break;
        case 5:
            closeness = GetColorDistance5(g_rgColorCombosHSL[egaIndex], g_rgColorCombos[egaIndex], color);
            break;
        default:
            ASSERT(FALSE);
        }

        if (closeness <= closest)
        {
            BOOL set = FALSE;
            if (closeness == closest)
            {
                // Before going again and setting it, we should check the ega ditherness.
                // We want as close to zero as possible. e.g. we prefer grey/grey to black/white
                // So check the difference between the two components of the dithered EGA color
                int ditherness = GetColorDistance(g_egaColorsCR[rgColors[iIndex].color1], g_egaColorsCR[rgColors[iIndex].color2]);
                if (ditherness < leastdithered)
                {
                    leastdithered = ditherness;
                    set = TRUE;
                }
            }
            else
            {
                set = TRUE;
            }

            if (set)
            {
                closest = closeness;
                egacolor = rgColors[iIndex];
            }
        }
    }
    return egacolor;
}

EGACOLOR GetClosestEGAColor(int iAlgorithm, int iPalette, COLORREF color)
{
    EGACOLOR *pPalette = NULL;
    int cEntries = 0;
    switch (iPalette)
    {
    case 1:
        pPalette = g_rg136ToByte;
        cEntries = ARRAYSIZE(g_rg136ToByte);
        break;
    case 2:
        pPalette = g_rg16ToByte;
        cEntries = ARRAYSIZE(g_rg16ToByte);
        break;
    case 3:
        pPalette = g_rgSmoothToByte;
        cEntries = g_cSmoothEntries;
        break;
    default:
        ASSERT(FALSE);
    }
    return GetClosestEGAColorFromSet(iAlgorithm, color, pPalette, cEntries);
}

EGACOLOR EGAColorFromByte(BYTE b)
{
    EGACOLOR color = { b & 0xf, (b & 0xf0) >> 4 };
    return color;
}

WORD _HexToWord(PCTSTR psz)
{
    WORD wNum = 0;
	for(int i = 0; _IsNumber(psz[i], TRUE); i++)
    {
		wNum <<= 4;
		if (psz[i] >= '0' && psz[i] <= '9')
        {
		    wNum += psz[i]-'0';
        }
		else if (((char)(psz[i] & 0xDF) >= 'A') &&
				((char)(psz[i] & 0xDF) <= 'F'))
        {
		    wNum += ((char)(psz[i] & 0xDF)-'A') + 10;
        }
	}
    return wNum;
}


DWORD GetDllVersion(LPCTSTR lpszDllName)
{
    HINSTANCE hinstDll;
    DWORD dwVersion = 0;

    /* For security purposes, LoadLibrary should be provided with a 
       fully-qualified path to the DLL. The lpszDllName variable should be
       tested to ensure that it is a fully qualified path before it is used. */
    hinstDll = LoadLibrary(lpszDllName);
	
    if(hinstDll)
    {
        DLLGETVERSIONPROC pDllGetVersion;
        pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstDll, 
                          "DllGetVersion");

        /* Because some DLLs might not implement this function, you
        must test for it explicitly. Depending on the particular 
        DLL, the lack of a DllGetVersion function can be a useful
        indicator of the version. */

        if(pDllGetVersion)
        {
            DLLVERSIONINFO dvi;
            HRESULT hr;

            ZeroMemory(&dvi, sizeof(dvi));
            dvi.cbSize = sizeof(dvi);

            hr = (*pDllGetVersion)(&dvi);

            if(SUCCEEDED(hr))
            {
               dwVersion = PACKVERSION(dvi.dwMajorVersion, dvi.dwMinorVersion);
            }
        }

        FreeLibrary(hinstDll);
    }
    return dwVersion;
}

const std::string MakeFile(const std::stringstream &out, const std::string &filename)
{
    std::string path;
    char szTempPath[MAX_PATH];
    if (GetTempPath(ARRAYSIZE(szTempPath), szTempPath))
    {
        strncat(szTempPath, filename.c_str(), ARRAYSIZE(szTempPath));
        FILE *pFile = fopen(szTempPath, "wt");
        if (pFile)
        {
            fputs(out.str().c_str(), pFile);
            fclose(pFile);
            path = szTempPath;
        }
    }
    return path;
}

std::string MakeTextFile(const std::stringstream &out, const std::string &filename)
{
    char szTempPath[MAX_PATH];
    bool fOk = false;
    if (PathIsRelative(filename.c_str()))
    {
        fOk = !!GetTempPath(ARRAYSIZE(szTempPath), szTempPath);
        strncat(szTempPath, filename.c_str(), ARRAYSIZE(szTempPath));
    }
    else
    {
        // Just use the path as is.
        strncpy(szTempPath, filename.c_str(), ARRAYSIZE(szTempPath));
        fOk = true;
    }
    if (fOk)
    {
        bool fError = true;
        FILE *pFile = fopen(szTempPath, "wt");
        if (pFile)
        {
            fputs(out.str().c_str(), pFile);
            fclose(pFile);
        }
        return szTempPath;
    }
    else
    {
        return "";
    }
}

//
// Given a stringstream, put it into a temporary text file and show it.
//
void ShowTextFile(const std::stringstream &out, const std::string &filename)
{
    std::string actualPath = MakeTextFile(out, filename);
    if (!actualPath.empty())
    {
        bool fError = true;
        if (((INT_PTR)ShellExecute(AfxGetMainWnd()->GetSafeHwnd(), "open", actualPath.c_str(), NULL, NULL, SW_SHOWNORMAL)) > 32)
        {
            fError = false;
        }

        if (fError)
        {
            char szMsg[200];
            DWORD_PTR arg = (DWORD_PTR)actualPath.c_str();
            FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY, 0, GetLastError(), 0, szMsg, ARRAYSIZE(szMsg), (va_list*)&arg);
            AfxMessageBox(szMsg, MB_OK | MB_APPLMODAL);
        }
    }
}


bool Save8BitBmp(const std::string &filename, const BITMAPINFO &info, BYTE *pBits, DWORD id)
{
    std::ofstream bmpFile(filename.c_str(), std::ios::out | std::ios::binary);
    if (info.bmiHeader.biBitCount != 8)
    {
        return false; // We don't support that right now.
    }
    if (bmpFile.is_open())
    {
        // Calc the image size
        DWORD dwSize = info.bmiHeader.biSizeImage;
        if (dwSize == 0)
        {
            // Calculate it.
            ASSERT(info.bmiHeader.biCompression == BI_RGB);
            dwSize = (info.bmiHeader.biBitCount / 8) * CX_ACTUAL(info.bmiHeader.biWidth) * info.bmiHeader.biHeight;
        }

        DWORD cbPalette = sizeof(RGBQUAD) * 256;

        BITMAPFILEHEADER bmfh;
        bmfh.bfType = 'B'+ ('M' << 8);
        int nBitsOffset = info.bmiHeader.biSize + cbPalette;
        LONG lImageSize = dwSize;
        LONG lFileSize = nBitsOffset + lImageSize;
        bmfh.bfOffBits = nBitsOffset + sizeof(bmfh);
        bmfh.bfSize = lFileSize;
        bmfh.bfReserved1 = (WORD)(id & 0x0000FFFF);
        bmfh.bfReserved2 = (WORD)((id & 0xFFFF0000) >> 16);

        // Write the file header.
        bmpFile.write((const char *)&bmfh, sizeof(bmfh)); // 14

        // Write the bitmapinfo
        bmpFile.write((const char *)&info.bmiHeader, info.bmiHeader.biSize);

        // Write the palette
        bmpFile.write((const char *)info.bmiColors, cbPalette); // 2 ^ 8

        // Write the bits.
        bmpFile.write((const char *)pBits, dwSize);
        return true;
    }
    return false;
}


//
// ScriptId implementation
// (unique identifier for a script)
//
std::string ScriptId::GetTitle() const
{
    TCHAR szFileName[MAX_PATH];
    StringCchCopy(szFileName, ARRAYSIZE(szFileName), _strFileName.c_str());
    PTSTR pszExt = PathFindExtension(szFileName);
    *pszExt = 0; // Chop off file extension
    return szFileName;
}

std::string ScriptId::GetFullPath() const
{
    std::string fullPath = _strFolder;
    fullPath += "\\";
    fullPath += _strFileName;
    return fullPath;
}

bool ScriptId::IsHeader() const
{
    return (strcmp(TEXT(".sh"), PathFindExtension(_strFileName.c_str())) == 0) ||
           (strcmp(TEXT(".shp"), PathFindExtension(_strFileName.c_str())) == 0);
}
LangSyntax ScriptId::Language() const
{
    if ((0 == strcmp(".scp", PathFindExtension(_strFileName.c_str()))) ||
        (0 == strcmp(".shp", PathFindExtension(_strFileName.c_str()))))
    {
        return LangSyntaxCpp;
    }
    else
    {
        return LangSyntaxSCIStudio;
    }
}

void ScriptId::_MakeLower()
{
    transform(_strFolder.begin(), _strFolder.end(), _strFolder.begin(), tolower);
    transform(_strFileName.begin(), _strFileName.end(), _strFileName.begin(), tolower);
}

void ScriptId::SetResourceNumber(WORD wScriptNum)
{
    ASSERT((_wScriptNum == InvalidResourceNumber) || (_wScriptNum == wScriptNum));
    _wScriptNum = wScriptNum;
}

bool operator==(const ScriptId& script1, const ScriptId& script2)
{
    bool fRet;
    fRet = (script1.GetFileName() == script2.GetFileName());
    if (fRet)
    {
        // Don't think we need to check _wScriptNum, in case it isn't set?
        fRet = (script1.GetFolder() == script2.GetFolder());
    }
    return fRet;
}

bool operator<(const ScriptId& script1, const ScriptId& script2)
{
    bool fRet;
    fRet = (script1.GetFileName() < script2.GetFileName());
    if (fRet)
    {
        // Don't think we need to check _wScriptNum, in case it isn't set?
        fRet = (script1.GetFolder() < script2.GetFolder());
    }
    return fRet;
}



