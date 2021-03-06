#include "stdafx.h"
#include "ClassBrowser.h"
#include "Compile\ScriptOMAll.h"
#include "Compile\SCISyntaxParser.h"
#include "CodeAutoComplete.h"
#include "SCIPicEditor.h"

using namespace sci;
using namespace std;


AutoCompleteResult GetAutoCompleteResult(SyntaxContext *pContext)
{
    // TODO
    AutoCompleteResult result;
    return result;
}

UINT AutoCompleteThread::s_ThreadWorker(void *pParam)
{
    ((AutoCompleteThread*)pParam)->_DoWork();
    return 0;
}

AutoCompleteThread::AutoCompleteThread()
{
    _fDoingWork = false;
    _fCancel = false;
    _hStartWork = CreateEvent(NULL, FALSE, FALSE, NULL);
    _hEndWork = CreateEvent(NULL, FALSE, FALSE, NULL);
    _pContext = NULL;
    _pLimit = NULL;
    _pThread = NULL;
}

AutoCompleteThread::~AutoCompleteThread()
{
    if (!_fCancel)
    {
        Exit();
        // phil, this is incomplete... we need to wait until the thread exits, really
    }
    if (_hStartWork)
    {
        CloseHandle(_hStartWork);
    }
    if (_hEndWork)
    {
        CloseHandle(_hEndWork);
    }
}

void AutoCompleteThread::_DoWork()
{
    while ((WAIT_OBJECT_0 == WaitForSingleObject(_hStartWork, INFINITE)) && !_fCancel)
    {
        _it = _itOrig; // Restart from the beginning.
        _fDoingWork = true;
        sci::Script script;
        SyntaxContext context(_it, script);
        _pContext = &context;
        OutputDebugString("ACThread: PARSE START\n");
        //g_Parser.Parse(script, _it, NULL);
        //SCISyntaxParser::Instance(LangSyntaxUnknown, theApp._fAllowBraceSyntax).Parse(script, _it, context);    
        OutputDebugString("ACThread: PARSE END\n");
        SetEvent(_hEndWork);
        _pContext = NULL;
OutputDebugString("ACThread: waiting to start work...\n");
        _fDoingWork = false;
    }
    ASSERT(FALSE);
}

void AutoCompleteThread::Exit()
{
    _fCancel = true;
    SetEvent(_hStartWork);
}

CPoint AutoCompleteThread::GetCompletedPosition()
{
    LineCol dwPos = _it.GetPosition();
    return CPoint(dwPos.Column(), dwPos.Line());
}

void AutoCompleteThread::ResetPosition()
{
OutputDebugString("Resetting position...\n");
    _pLimit->Bail();
    LineCol dwPos = _it.GetPosition(); // phil, see what this is?
    int x = 0;
    // We need to wait for it to be done...
OutputDebugString("(RP) Waiting for AC thread to come around... getting stuck here.\n");
    if (WAIT_OBJECT_0 == WaitForSingleObject(_hEndWork, INFINITE))
    {
        // Done
    }
    else
    {
        ASSERT(FALSE);
    }
}

// Needs to be called when you switch to a new script
void AutoCompleteThread::InitAutoComplete(CCrystalScriptStream::const_iterator it, CScriptStreamLimiter *pLimit)
{
    _itOrig = it;
    _pLimit = pLimit;
}

AutoCompleteResult AutoCompleteThread::DoAutoComplete(CPoint pt)
{
    // Since this thread works directly off the text buffers, make sure all references to text
    // are invalidated (we're likely called here, because the user typed a character, which could
    // invalidate our pointers to text inside the document)
    _pLimit->InvalidateBuffer();

    // phil. if this is called, we need to sync here, to wait for it to be done?  Unwind to top, since
    // we're changing things.
    CPoint ptDone = GetCompletedPosition();
    if (_pThread && ((pt.y > ptDone.y) || ((pt.y == ptDone.y) && (pt.x > ptDone.x))))
    {
        OutputDebugString("DAC: Continue where we left off...\n");
        _pLimit->Limit(LineCol(pt.y, pt.x));
        if (!_fDoingWork)
        {
OutputDebugString("DAC: Telling the thread to start work\n");
            // Start the work...
            SetEvent(_hStartWork);
        }
        else
        {
OutputDebugString("DAC: Telling the limiter to continue..\n");
            // We can just continue from where we left off
            _pLimit->Continue();
        }
        // Wait until we hit the end...
        _pLimit->Wait();
    }
    else
    {
        if (_pThread == NULL)
        {
            // First time... create the thread... it will wait for the _hStartWork.
            _pThread = AfxBeginThread(s_ThreadWorker, this, 0, 0, 0, NULL);
        }
        else
        {
            // We need to reset.  Tell the parser to bail.
            ResetPosition();
        }

        // Ok, it's been reset, start it anew
        _pLimit->Limit(LineCol(pt.y, pt.x));
        SetEvent(_hStartWork);
        _pLimit->Wait();
    }

    // We have a result.
    AutoCompleteResult result = GetAutoCompleteResult(_pContext);
    char sz[100];
    StringCchPrintf(sz, ARRAYSIZE(sz), "Completed with %s\n", _pContext->ScratchString().c_str());
    OutputDebugString(sz);
    // phil, this should probably be an allocated thing...
    return result;
}
