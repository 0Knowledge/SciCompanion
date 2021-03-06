#pragma once

#include "Compile\ScriptOM.h"
#include "Vocab99x.h"
class SCIClassBrowserNode;

//
// SCIClassBrowser
//
// 
//
// Usage: AddScript's to it.
//
class SCIClassBrowser : public ICompileLog
{
public:
    SCIClassBrowser();
    ~SCIClassBrowser();

    void SetClassBrowserEvents(IClassBrowserEvents *pEvents);

    // See my comments in the implementations
    // These functions do not require using the lock.
    sci::MethodVector *CreateMethodArray(const std::string &strObject, sci::Script *pScript = NULL) const;
    sci::ClassPropertyVector *CreatePropertyArray(const std::string &strObject, sci::Script *pScript = NULL, PCTSTR pszSuper = NULL) const;
    std::vector<std::string> *CreateSubSpeciesArray(PCTSTR pszSpecies);

    //
    // This operators similarly to GlobalLock/Unlock. Before calling any functions here,
    // call Lock or TryLock, and make sure to call Unlock at the end (and don't hold on to
    // any internal references you got back from the class browser!).  This isn't very
    // robust, but it is more performant than making copies of everything we hand out.
    //
    void Lock() const; // Blocks until it gets a lock.
    bool TryLock() const; // Tries to get a lock for this thread
    void Unlock() const; // Releases lock.
    bool HasLock() const; 

    bool ReLoadFromSources();
    void ReLoadFromCompiled();
    void ReLoadScript(PCTSTR pszFullPath);

    // The remaining public functions should only be called if within a lock, as they return
    // information internal to this class.
    const SCIClassBrowserNode *GetRoot(size_t i) const;
    size_t GetNumRoots() const;
    const sci::Script *GetLKGScript(WORD wScriptNumber);
    const sci::Script *GetLKGScript(PCTSTR pszScriptFullPath);
    const sci::VariableDeclVector *GetMainGlobals() const;
    const std::vector<std::string> &GetKernelNames() const;
    const sci::ProcedureVector &GetPublicProcedures();
    const sci::ClassVector &GetAllClasses();
    const std::vector<sci::Script*> &GetHeaders();
    const SelectorTable &GetSelectorNames();
    bool IsSubClassOf(PCTSTR pszClass, PCTSTR pszSuper);
    void ResolveValue(WORD wScript, const sci::PropertyValue &In, sci::PropertyValue &Out);
    bool ResolveValue(const sci::Script *pScript, const std::string &strValue, sci::PropertyValue &Out) const;
    WORD GetScriptNumberHelper(sci::Script *pScript) const;
    WORD GetScriptNumberHelperConst(const sci::Script *pScript) const;
    bool GetPropertyValue(PCTSTR pszName, const sci::ClassDefinition *pClass, WORD *pw);
    bool GetProperty(PCTSTR pszName, const sci::ClassDefinition *pClass, sci::PropertyValue &Out);
    bool GetPropertyValue(PCTSTR pszName, ISCIPropertyBag *pBag, const sci::ClassDefinition *pClass, WORD *pw);
    sci::Script *_LoadScript(PCTSTR pszPath);
    
    // Error reporting.
    void ReportResult(const CompileResult &result);
    void ClearErrors();
    std::vector<CompileResult> GetErrors();
    int HasErrors(); // 0: no erors,  1: errors,  -1: unknown

    void Reset();

private:
    typedef stdext::hash_map<std::string, SCIClassBrowserNode*> class_map;
    typedef stdext::hash_map<WORD, sci::ClassVector*> instance_map;
    typedef stdext::hash_map<std::string, sci::Script*> script_map;
    typedef stdext::hash_map<std::string, WORD> define_map;
    typedef stdext::hash_map<std::string, WORD> word_map;

    bool _CreateClassTree();
    void _AddToClassTree(sci::Script& script);
    bool _AddFileName(PCTSTR pszFullPath, PCTSTR pszFileName, BOOL fReplace = FALSE);
    void _RemoveAllRelatedData(sci::Script *pScript);
    void _AddHeaders();
    void _AddHeader(PCTSTR pszHeaderPath);
    void _CacheHeaderDefines();
    void _AddInstanceToMap(sci::Script& script, sci::ClassDefinition *pClass);
    void _AddSubclassesToArray(std::vector<std::string> *pArray, SCIClassBrowserNode *pBrowserInfo);

    // This maps strings to SCIClassBrowserNode.  e.g. gEgo to it's node in the tree
    class_map _classMap;

    // This maps script numbers to arrays of instances within them.
    instance_map _instanceMap;

    // This is an array of all instances, for use in the hierarchy tree.
    std::vector<SCIClassBrowserNode*> _instances;

    // This is a list of script OMs
    std::vector<sci::Script*> _scripts;
    std::vector<sci::Script*> _headers;   // Note: _headers's pointers are owned by _headerMap
    script_map _headerMap;
    define_map _headerDefines;  // Note: defines are owned by the _headerMap.

    // This maps filenames to scriptnumbers.
    word_map _filenameToScriptNumber;

    // Cache;
    const sci::Script *_pLKGScript;
    WORD _wLKG;

    bool _fPublicProceduresValid;
    sci::ProcedureVector _publicProcedures; // These store the pointers, but don't own them (perf)
    bool _fPublicClassesValid;
    sci::ClassVector _allClasses;

    // These are the files we look at
    bool fFoundRoot;
    std::vector<std::string> strRootNames;  // We can have multiple roots, apparently.

    const std::vector<std::string> &_kernelNames;
    KernelTable _kernelNamesResource;
    SelectorTable _selectorNames;

    // Error reporting - protected by g_csErrorReport
    std::vector<CompileResult> _errors;

    mutable CRITICAL_SECTION _csClassBrowser;
    mutable volatile int _fCBLocked;
    mutable CRITICAL_SECTION _csErrorReport;
    bool _fAbortBrowseInfoGeneration;

    IClassBrowserEvents *_pEvents;

    HANDLE _hAborted;
};

//
// Some helpers.
//

//
// Looks for the first Rm class in pScript, and tries to figure out a pic number.
//
WORD GetPicResourceFromScript(SCIClassBrowser *pBrowser, const sci::Script *pScript);

//
// Quick helper for parsing a script (move this to a better place)
//
class CCrystalTextBuffer; 
sci::Script *ParseScript(ScriptId script, CCrystalTextBuffer *pBuffer);

class ClassBrowserLock
{
public:
    ClassBrowserLock(SCIClassBrowser &browser) : _browser(browser), _fLock(false), _fDidSomething(false)  {}
    ~ClassBrowserLock()
    {
        ASSERT(_fDidSomething);
        if (_fLock)
        {
            _browser.Unlock();
        }
    }
    bool TryLock()
    {
        _fDidSomething = true;
        _fLock = _browser.TryLock();
        return _fLock;
    }
    void Lock()
    {
        _fDidSomething = true;
        _browser.Lock();
        _fLock = true;
    }

private:
    SCIClassBrowser &_browser;
    bool _fLock;
    bool _fDidSomething;
};
