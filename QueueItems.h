
#pragma once

//
// This template implements a worker thread that can be used to perform background tasks.
//
// TITEM is a class that represents the data to work with.
// TRESULT is a class that represents the results you get back.
// TRESULT needs a static function of the form:
// static TRESULT *CreateFromWorkItem(TITEM *pWorkItem);
//

//
// Don't delete QueueItems.  Use Release.  The proper sequence for cleaning up QueueItems on
// the UI thread is:
//  _pQueue->Abort();
//  _pQueue->Release();
//

template <class TITEM, class TRESULT>
class QueueItems
{
public:
    void AddRef()
    {
        InterlockedIncrement(&_cRef);
    }

    void Release()
    {
        LONG cRef = InterlockedDecrement(&_cRef);
        if (cRef == 0)
        {
            delete this;
        }
    }

    //
    // uMessage is posted to hwndView when there are (potentially multiple) results ready.
    //
    QueueItems(HWND hwndView, UINT uMessage)
    {
        _hwndView = hwndView;
        _uResultReadyMessage = uMessage;
        _cRef = 1;
        _fAbort = FALSE;
        _hEventWorkAvailable = NULL;
        InitializeCriticalSection(&_cs);
    }

    ~QueueItems()
    {
        Abort();

        // Go through any items in workItems and delete them.
        // Go through any items in workResults and delete them
        {
            CGuard guard(&this->_cs);
            for_each(_workItems.begin(), _workItems.end(), delete_object());
            for_each(_workResults.begin(), _workResults.end(), delete_object());
        }

        DeleteCriticalSection(&_cs);
        CloseHandle(_hEventWorkAvailable);
    }

    bool Init()
    {
        bool fRet = false;
        _hEventWorkAvailable = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (_hEventWorkAvailable)
        {
            AddRef(); // For the worker thread.
            _pThread = AfxBeginThread(s_ThreadWorker, this, 0, 0, 0, NULL);
            if (_pThread)
            {
                fRet = true;
            }
            else
            {
                Release(); // (for worker thread)
            }
        }
        return fRet;
    }

    void GiveWorkItem(TITEM *pWorkItem)
    {
        // Insert things at the beginning of the list.
        CGuard guard(&this->_cs);
        _workItems.push_front(pWorkItem);
        SetEvent(_hEventWorkAvailable);
    }

    bool TakeWorkResult(TRESULT **ppWorkResult)
    {
        bool fRet = false;
        CGuard guard(&this->_cs);
        if (!_workResults.empty())
        {
            fRet = true;
            *ppWorkResult = _workResults.front();
            _workResults.pop_front();
        }
        return fRet;
    }

    void Abort()
    {
        CGuard guard(&this->_cs);
        _fAbort = TRUE;
        SetEvent(_hEventWorkAvailable);
    }

    bool HasAborted()
    {
        CGuard guard(&this->_cs);
        return _fAbort;
    }

private:
    bool _TakeWorkItem(TITEM **ppWorkItem)
    {
        bool fRet = false;
        CGuard guard(&this->_cs);
        if (!_workItems.empty())
        {
            fRet = true;
            *ppWorkItem = _workItems.back();
            _workItems.pop_back();
        }
        else
        {
            // No work items left.
            ResetEvent(_hEventWorkAvailable);
        }
        return fRet;
    }

    void _GiveWorkResult(TRESULT *pWorkResult)
    {
        {
            CGuard guard(&this->_cs);
            _workResults.push_back(pWorkResult);
        }
        PostMessage(_hwndView, _uResultReadyMessage, 0, 0);
    }

    void _ThreadWorker()
    {
        while (WAIT_OBJECT_0 == WaitForSingleObject(_hEventWorkAvailable, INFINITE))
        {
            if (_fAbort)
            {
                break;
            }

            TITEM *pWorkItem;
            while (!_fAbort && _TakeWorkItem(&pWorkItem))
            {
                TRESULT *pResult = TRESULT::CreateFromWorkItem(pWorkItem);
                if (pResult)
                {
                    _GiveWorkResult(pResult);
                }
                delete pWorkItem;
            }

            if (_fAbort)
            {
                break;
            }
        }
    }

    static UINT s_ThreadWorker(void *pParam)
    {
        QueueItems *pQueueItems = (QueueItems*)pParam;
        pQueueItems->_ThreadWorker();
        pQueueItems->Release();
        return 0;
    }

    LONG _cRef;
    HWND _hwndView;
    UINT _uResultReadyMessage;
    CRITICAL_SECTION _cs;
    HANDLE _hEventWorkAvailable;

    CWinThread *_pThread;
    bool _fAbort;

    std::list<TITEM*> _workItems;
    std::list<TRESULT*> _workResults;
};



