#pragma once

#include "scii.h"
#include "..\TextResource.h"
#include "SCO.h"
#include "..\Vocab000.h"
#include "..\Vocab99x.h"
#include <boost/foreach.hpp>

//
// Indicates where code should puts its value (stack or accumulator)
//
enum OutputContext
{
    OC_Stack        = 0,
    OC_Accumulator  = 1,
    OC_Unknown      = 2,
};

enum VariableModifier
{
    VM_None,
    VM_PreIncrement,
    VM_PreDecrement,
};

// fwd decl's
class sci::SyntaxNode;
class sci::Define;
class sci::ClassDefinition;
class sci::Script;
class CResourceMap;


//
// Object used to hold state for compilation.
//
class ICompileContext : public ICompileLog
{
    // NOTE: this implementation needs to take into account USES and INCLUDES
public:
    virtual LangSyntax GetLanguage() = 0;

    // Frame that indicate if a value should be returned in the accumulator or on the stack
    virtual OutputContext GetOutputContext() = 0;
    virtual void PushOutputContext(OutputContext outputContext) = 0;
    virtual void PopOutputContext() = 0;

    // Frames that indicate if something is using the value generated by the code
    virtual bool HasMeaning() = 0;
    virtual void PushMeaning(bool fMeaning) = 0;
    virtual void PopMeaning() = 0;

    // Frames that indicate if && and || should branch, or return a result in the accumulator/stack
    virtual bool InConditional() = 0;
    virtual void PushConditional(bool fConditional) = 0;
    virtual void PopConditional() = 0;

    virtual bool SupportTypeChecking() = 0;

    virtual WORD GetScriptNumber() = 0;
    virtual WORD AddStringResourceTuple(const std::string &str) = 0; // Can return InvalidResourceNumber on failure
    virtual WORD LookupSelectorAndAdd(const std::string &str) = 0;
    virtual bool LookupSelector(const std::string &str, WORD &wIndex) = 0;
    virtual void DefineNewSelector(const std::string &str, WORD &wIndex) = 0;
    virtual TOKENTYPEFOO LookupToken(const sci::SyntaxNode *pNode, const std::string &str, WORD &wIndex, SpeciesIndex &dataType, WORD *pwScript = NULL) = 0;
    virtual ProcedureType LookupProc(const std::string &str, WORD &wScript, WORD &wIndex, std::string &classOwner, std::vector<CSCOFunctionSignature> *pSignatures = NULL) = 0;
    virtual bool LookupSpeciesMethodOrProperty(SpeciesIndex wCallee, WORD wSelector, std::vector<CSCOFunctionSignature> &signatures, SpeciesIndex &propertyType, bool &fMethod) = 0;
    virtual ProcedureType LookupProc(const std::string &str) = 0;
    virtual bool LookupWord(const std::string, WORD &wWordGroup) = 0;
    virtual std::vector<species_property> GetSpeciesProperties(const std::string&) = 0;
    virtual bool LookupSpeciesIndex(const std::string&, SpeciesIndex &wSpeciesIndex) = 0;
    virtual bool LookupTypeSpeciesIndex(const std::string&, SpeciesIndex &wSpeciesIndex) = 0;
    virtual SpeciesIndex LookupTypeSpeciesIndex(const std::string &str, const ISourceCodePosition *pPos) = 0;
    virtual std::string SpeciesIndexToDataTypeString(SpeciesIndex wSpeciesIndex) = 0;
    virtual SpeciesIndex GetSpeciesSuperClass(SpeciesIndex wSpeciesIndex) = 0;
    virtual void AddDefine(sci::Define *pDefine) = 0;
    virtual bool LookupDefine(const std::string&, WORD &wValue) = 0;

    virtual void SetClassContext(const std::string &superClass, const std::string &className, bool fInstance) = 0;
    virtual std::string GetClassName() = 0;
    virtual std::string GetSuperClassName() = 0;
    virtual bool IsInstance() = 0;
    virtual const sci::ClassDefinition *LookupClassDefinition(const std::string &str) = 0;

    virtual sci::Script *SetErrorContext(sci::Script *pScript) = 0;
    virtual void ReportError(const ISourceCodePosition *pPos, const char *pszFormat, ...) = 0;
    virtual void ReportWarning(const ISourceCodePosition *pPos, const char *pszFormat, ...) = 0;
    virtual void ReportTypeError(const ISourceCodePosition *pPos, SpeciesIndex w1, SpeciesIndex w2, const char *pszFormat) = 0;
    virtual std::string ScanForIdentifiersScriptName(const std::string &identifier) = 0;

    virtual scicode &code() = 0;

    // Tell someone about where we have 1) exports, 2) saids, and 3) internal strings
    virtual void TrackExport(code_pos where) = 0;
    virtual void TrackCallOffsetInstruction(WORD wProcIndex) = 0;
    virtual void PreScanSaid(const std::string &theSaid, const ISourceCodePosition *pPos) = 0;
    virtual WORD GetSaidTempOffset(const std::string &theSaid) = 0;
    virtual void PreScanStringAndUnescape(std::string &theString, const ISourceCodePosition *pPos) = 0;
    virtual WORD GetStringTempOffset(const std::string &theString) = 0;
    virtual void SpecifyOffsetIndexResolvedValue(WORD wOffsetIndex, WORD wOffset) = 0;

    // Tell the context the name of a local procedure
    virtual bool PreScanLocalProc(const std::string &name, const std::string &ownerClass) = 0;
    // Tell the context the name and position of a class method
    virtual bool TrackMethod(const std::string &name, code_pos where) = 0;
    // Tell the context the code_pos of the local procedure
    virtual void TrackLocalProc(const std::string &name, code_pos where) = 0;
    // Tell the context about a piece of code that has a reference to an instance
    virtual void TrackInstanceReference(const std::string &name) = 0;
    // Tell the context about a piece of code that refers to an internal string
    virtual void TrackStringReference(const std::string &name) = 0;
    // Tell the context about a piece of code that refers to a said string
    virtual void TrackSaidReference(const std::string &name) = 0;
    // Tell the context about a piece of code that needs to point to a local proc
    virtual void TrackLocalProcCall(const std::string &name) = 0;
    // Get the pos of a local proc or method
    virtual code_pos GetLocalProcPos(const std::string &name) = 0;

    virtual void TrackRelocation(WORD wOffset) = 0;

    virtual void PushVariableLookupContext(const IVariableLookupContext *pVarContext) = 0;
    virtual void PopVariableLookupContext() = 0;
    virtual void SetClassPropertyLookupContext(const IVariableLookupContext *pVarContext) = 0;
    virtual void SetReturnValueTypes(const std::vector<SpeciesIndex> &types) = 0;
    virtual std::vector<SpeciesIndex> GetReturnValueTypes() = 0;

    virtual void PushSendCallType(SpeciesIndex wType) = 0;
    virtual void PopSendCallType() = 0;
    virtual SpeciesIndex GetSendCalleeType() = 0;

    virtual void SetVariableModifier(VariableModifier modifier) = 0;
    virtual VariableModifier GetVariableModifier() = 0;
};


class GenericOutputByteCode : public std::unary_function<IOutputByteCode*, void>
{
public:
    GenericOutputByteCode(ICompileContext &context) : _context(context) {}
    void operator()(const IOutputByteCode* proc)
    {
        //_wBytes += proc->OutputByteCode(_context).GetBytes();
        _results.push_back(proc->OutputByteCode(_context));
    }
    WORD GetByteCount() const
    {
        WORD wBytes = 0;
        BOOST_FOREACH(const CodeResult &result, _results)
        {
            wBytes += result.GetBytes();
        }
        return wBytes;
    }
    std::vector<SpeciesIndex> GetTypes() const
    {
        std::vector<SpeciesIndex> types;
        BOOST_FOREACH(const CodeResult &result, _results)
        {
            types.push_back(result.GetType());
        }
        return types;
    }
    SpeciesIndex GetLastType() const
    {
        SpeciesIndex type = DataTypeVoid;
        if (!_results.empty())
        {
            type = _results[_results.size() - 1].GetType(); // Return last type.
        }
        return type;
    }
private:
    std::vector<CodeResult> _results;
    ICompileContext &_context;
};

//
// Some important tables used for compiling.  When doing multiple compiles at once, this
// is kept across individual compiles.
// Both the species and selector tables may be modified.  They will be updated in the game
// when Save is called.
//
class CompileTables
{
public:
    bool Load();
    void Save();
    const Vocab000 *Vocab() { return _pVocab; }
    const KernelTable &Kernels() { return _kernels; }
    SpeciesTable &Species() { return _species; }
    SelectorTable &Selectors() { return _selectors; }
private:
    const Vocab000 *_pVocab;
    KernelTable _kernels;
    SpeciesTable _species;
    SelectorTable _selectors;
};

//
// Encapsulates the result of a single compilation, including:
// 1) the generate script resource, as a vector of BYTEs
// 2) the sco file
// 3) the script number
// 4) a reference to an ICompileLog, for logging errors and warnings.
//
class CompileResults
{
public:
    CompileResults(ICompileLog &log);
    std::vector<BYTE> &GetScriptResource() { return _output; }
    CSCOFile &GetSCO() { return _sco; }
    WORD GetScriptNumber() const { return _wScriptNumber; }
    void SetScriptNumber(WORD wNum) { _wScriptNumber = wNum; }
    ICompileLog &GetLog() { return _log; }
    TextResource &GetTextResource() { return _text; }
    
private:
    std::vector<BYTE> _output;
    WORD _wScriptNumber;
    CSCOFile _sco;
    ICompileLog &_log;
    TextResource _text;
};


class CompileLog : public ICompileLog
{
public:
    CompileLog() { _cErrors = 0; _cWarnings = 0; }
    void ReportResult(const CompileResult &result)
    {
        _compileResults.push_back(result);
    }
    void SummarizeAndReportErrors();
    void Clear() { _compileResults.clear(); }
    std::vector<CompileResult> &Results() { return _compileResults; }
    void CalculateErrors();

private:
    std::vector<CompileResult> _compileResults;
    int _cErrors;
    int _cWarnings;
};


//
// Maintains a set of pre-compiled headers across compilations
// For each script that is comiled, call Update.  That will ensure the defines
// returned by GetDefines() are appropriate.
//
class PrecompiledHeaders
{
public:
    PrecompiledHeaders(CResourceMap &resourceMap);
    ~PrecompiledHeaders();
    // Call this each time you compile a new script
    void Update(ICompileContext &context, sci::Script &script);

    bool LookupDefine(const std::string &str, WORD &wValue);
private:
    typedef stdext::hash_map<std::string, sci::DefinePtr> defines_map;
    typedef std::map<std::string, sci::Script* > header_map;

    // Filename (not full path) which maps a header to its Script object.
    header_map _allHeaders; 

    // A set of the names of all the last script's header includes.
    std::set<std::string> _curHeaderList;   

    // The define hashtable which corresponds to the _curHeaderList
    defines_map _defines;

    // Whether or not the _defines table is valid
    bool _fValid;
    CResourceMap &_resourceMap;
};  

//
// The be-all end-all function for compiling a script.
// Returns true if there were no errors.
//
bool GenerateScriptResource(sci::Script &script, PrecompiledHeaders &headers, CompileTables &tables, CompileResults &results);

void ErrorHelper(ICompileContext &context, const ISourceCodePosition *pPos, const std::string &text, const std::string &identifier);

bool NewCompileScript(CompileLog &log, CompileTables &tables, PrecompiledHeaders &headers, ScriptId &script);
