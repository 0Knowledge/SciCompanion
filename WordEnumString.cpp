
#include "stdafx.h"

#include "WordEnumString.h"
#include "Vocab000.h"
#include "ResourceMap.h"
#include "SCIPicEditor.h"

using namespace std;

class CWordEnumString : public IEnumString
{
public:
    CWordEnumString();
    ~CWordEnumString();

    // IUnknown
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv);
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

    // IEnumString
    HRESULT STDMETHODCALLTYPE Next(ULONG celt, LPOLESTR *rgelt, ULONG *pceltFetched);
    HRESULT STDMETHODCALLTYPE Skip(ULONG celt);
    HRESULT STDMETHODCALLTYPE Reset();
    HRESULT STDMETHODCALLTYPE Clone(IEnumString **ppenum);

    // CWordEnumString
    HRESULT Init();

private:
    LONG _cRef;
    Vocab000 _vocab;

    size_t _vocabIndex;
};


CWordEnumString::CWordEnumString()
{
    _cRef = 1;
    _vocabIndex = 0;
}

CWordEnumString::~CWordEnumString()
{
}

ULONG CWordEnumString::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CWordEnumString::Release()
{
    LONG cRef = InterlockedDecrement(&_cRef);
    if (cRef == 0)
    {
        delete this;
    }
    return cRef;
}

HRESULT CWordEnumString::QueryInterface(REFIID riid, void **ppv)
{
    *ppv = NULL;
    HRESULT hr = E_NOINTERFACE;
    if (IsEqualIID(IID_IUnknown, riid))
    {
        *ppv = (IUnknown*)this;
        hr = S_OK;
    }
    else if (IsEqualIID(IID_IEnumString, riid))
    {
        *ppv = (IEnumString*)this;
        hr = S_OK;
    }

    if (SUCCEEDED(hr))
    {
        AddRef();
    }

    return hr;
}

HRESULT CWordEnumString::Next(ULONG celt, LPOLESTR *rgelt, ULONG *pceltFetched)
{
    HRESULT hr = S_FALSE;
    *pceltFetched = 0;
    vector<string> &words = _vocab.GetWords();
    while ((celt > 0) && SUCCEEDED(hr) && (_vocabIndex < words.size()))
    {
        string &strWord = words[_vocabIndex];
        int cch = (int)strWord.length() + 1;
        rgelt[*pceltFetched] = (LPOLESTR)CoTaskMemAlloc(cch * sizeof(WCHAR));
        hr = rgelt[*pceltFetched] ? S_OK : E_OUTOFMEMORY;
        if (SUCCEEDED(hr))
        {
            MultiByteToWideChar(CP_ACP, 0, strWord.c_str(), -1, rgelt[*pceltFetched], cch);
            *pceltFetched++;
            celt--;
        }
        _vocabIndex++;
    }
    return hr;
}

HRESULT CWordEnumString::Skip(ULONG celt)
{
    _vocabIndex += celt;
    return S_OK;
}

HRESULT CWordEnumString::Reset()
{
    _vocabIndex = 0;
    return S_OK;
}

HRESULT CWordEnumString::Clone(IEnumString **ppenum)
{
    return CWordEnumString_CreateInstance(IID_IEnumString, (void**)ppenum);
}

HRESULT CWordEnumString::Init()
{
    ResourceEnumerator *pEnum;
    HRESULT hr = theApp.GetResourceMap().CreateEnumerator(NULL, ResourceTypeFlagVocab, &pEnum);
    if (SUCCEEDED(hr))
    {
        ResourceBlob *pData;
        BOOL fDone = FALSE;
        while(SUCCEEDED(hr) && !fDone && (S_OK == pEnum->Next(&pData)))
        {
            if (0 == pData->GetNumber())
            {
                hr = _vocab.InitFromResource(pData);
                fDone = TRUE;
            }
            delete pData;
        }
        delete pEnum;
    }
    return hr;
}

HRESULT CWordEnumString_CreateInstance(REFIID riid, void **ppv)
{
    *ppv = NULL;
    HRESULT hr = E_OUTOFMEMORY;
    CWordEnumString *pEnumString = new CWordEnumString();
    if (pEnumString)
    {
        hr = pEnumString->Init();
        if (SUCCEEDED(hr))
        {
            hr = pEnumString->QueryInterface(riid, ppv);
        }
        pEnumString->Release();
    }
    return hr;

}