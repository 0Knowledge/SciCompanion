#pragma once

#include "crysedit/CCrystalTextBuffer.h"
#include "NonViewHostDocument.h"

class CompileTables;
class PrecompiledHeaders;
class ICompileLog;

// CScriptDocument document

//
// Script view updates
//
// A script was saved.  The CObject is the CScriptDocument
#define VIEWUPDATEHINT_SCRIPTSAVED          0x00000001
// The cursor position in the script changed.
#define VIEWUPDATEHINT_SCRIPTPOSCHANGED     0x00000002
// The script was converted!
#define VIEWUPDATEHINT_SCRIPTCONVERTED      0x00000004

class CScriptDocument : public CNonViewHostDocument
{
	DECLARE_DYNCREATE(CScriptDocument)

public:
	CScriptDocument();
	virtual ~CScriptDocument();
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
    virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
    virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);

    CCrystalTextBuffer *GetTextBuffer();
    
    void UpdateModified();

    // For new scripts:
    void SetNameAndContent(ScriptId scriptId, int iResourceNumber, std::string &text);

    ScriptId GetScriptId() { return _scriptId; }
    void SetScriptNumber(WORD wScriptNum) { _scriptId.SetResourceNumber(wScriptNum); }

protected:
	virtual BOOL OnNewDocument();
	DECLARE_MESSAGE_MAP()

private:

    class CSCITextBuffer : public CCrystalTextBuffer
    {
    private:
        CScriptDocument &_ownerDoc;
    public:
        CSCITextBuffer(CScriptDocument &doc) : _ownerDoc(doc) {}

        virtual void SetModified(BOOL bModified = TRUE)
        {
            __super::SetModified(bModified);
            _ownerDoc.UpdateModified();
        };
    };


    // Msg handlers
    afx_msg void OnCompile();
#ifdef DOCSUPPORT
    afx_msg void OnCompileDocs();
#endif
    afx_msg void OnDisassemble();
    afx_msg void OnDecompile();
    afx_msg void OnViewObjectFile();
    afx_msg void OnViewScriptResource();
    afx_msg void OnViewSyntaxTree();
    afx_msg void OnConvertScript();
    afx_msg void OnUpdateIsScript(CCmdUI *pCmdUI);
    afx_msg void OnUpdateConvertScript(CCmdUI *pCmdUI);
    afx_msg void OnUpdateLineCount(CCmdUI *pCmdUI);

    // Other...
    void OnFileSave();
    void OnFileSaveAs();
    void _DoErrorSummary(ICompileLog &log);
    void _ClearErrorCount();
    void _OnUpdateTitle();

    CSCITextBuffer _buffer;
    ScriptId _scriptId;
};

void DisassembleScript(WORD wScript);
void DecompileScript(WORD wScript);

