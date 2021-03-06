//
// SCISyntaxParser implementation
//
#include "stdafx.h"
#include "ScriptOMAll.h"
#include "SCISyntaxParser.h"
#include "CPPSyntaxParser.h"
#include <boost/foreach.hpp>

/*
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif*/

using namespace sci;
using namespace std;

boost::pool<> ASTNode::_ASTPool(sizeof(ASTNode));
boost::pool<> CPPSyntaxContext::ParseLevel::_ParseLevelPool(sizeof(ParseLevel));


//
// Common parsing primitives
//

bool BasicMatch(const TokenParser *pParser, CPPSyntaxContext *pContext, CrystalScriptTokenStream::const_iterator &stream)
{
    bool fRet = ((*stream).type == pParser->_token);
    if (fRet)
    {
        ++stream;
    }
    return fRet;
}

bool MatchOperator(const TokenParser *pParser, CPPSyntaxContext *pContext, CrystalScriptTokenStream::const_iterator &stream)
{
    bool fRet = ((*stream).type == pParser->_token);
    if (fRet)
    {
        if (pParser->_token < 128)
        {
            pContext->ScratchString() = static_cast<char>(pParser->_token);
        }
        else
        {
            pContext->ScratchString() = StringFromSyntaxToken(pParser->_token);
        }
        ++stream;
    }
    return fRet;
}

bool MatchString(const TokenParser *pParser, CPPSyntaxContext *pContext, CrystalScriptTokenStream::const_iterator &stream)
{
    bool fRet = ((*stream).type == pParser->_token);
    if (fRet)
    {
        pContext->ScratchString() = (*stream).string;
        ++stream;
    }
    return fRet;
}
bool MatchNumber(const TokenParser *pParser, CPPSyntaxContext *pContext, CrystalScriptTokenStream::const_iterator &stream)
{
    bool fRet = ((*stream).type == pParser->_token);
    if (fRet)
    {
        const TokenInfo &info = (*stream);
        pContext->Integer = info.integer;
        pContext->IntFlags = info.intFlags; 
        ++stream;
    }
    return fRet;
}
bool AlwaysMatch(const TokenParser *pParser, CPPSyntaxContext *pContext, CrystalScriptTokenStream::const_iterator &stream)
{
    return true;
}




//
// Parser operators
//


bool TSequenceP(const TokenParser *pParser, CPPSyntaxContext *pContext, CrystalScriptTokenStream::const_iterator &stream)
{
    std::vector<TokenParser*>::const_iterator start = pParser->_parsers.begin();
    std::vector<TokenParser*>::const_iterator end = pParser->_parsers.end();
    while (start != end)
    {
        if (!(*start)->Match(pContext, stream).Result())
        {
            return false;
        }
        ++start;
    }
    return true;
}

bool TAlternativeP(const TokenParser *pParser, CPPSyntaxContext *pContext, CrystalScriptTokenStream::const_iterator &stream)
{
    std::vector<TokenParser *>::const_iterator start = pParser->_parsers.begin();
    std::vector<TokenParser *>::const_iterator end = pParser->_parsers.end();
    while (start != end)
    {
        if ((*start)->Match(pContext, stream).Result())
        {
            return true;
        }
        ++start;
    }
    return false;
}


bool TKleeneP(const TokenParser *pParser, CPPSyntaxContext *pContext, CrystalScriptTokenStream::const_iterator &stream)
{
    while (pParser->_pa->Match(pContext, stream).Result())
    {
    }
    return true; // Always matches
}

bool TOneOrMoreP(const TokenParser *pParser, CPPSyntaxContext *pContext, CrystalScriptTokenStream::const_iterator &stream)
{
    bool fResult = false;
    while (pParser->_pa->Match(pContext, stream).Result())
    {
        fResult = true;
    }
    return fResult;
}

bool TZeroOrOnceP(const TokenParser *pParser, CPPSyntaxContext *pContext, CrystalScriptTokenStream::const_iterator &stream)
{
    pParser->_pa->Match(pContext, stream);
    return true; // Always matches
}

// The difference between token_p and operator_p is that operator_p provides the string to
// the parser context (since binary operations need to know which operation it was)
TokenParser token_p(SyntaxToken token)
{
    return TokenParser(BasicMatch, token);
}
TokenParser token_p(const char *psz)
{
    SyntaxToken token = SyntaxTokenFromString(psz);
    if (token == TokenInvalid)
    {
        // Not a known token.  Might be a single letter.
        ASSERT((lstrlen(psz) == 1) && (*psz < 128));
        return TokenParser(BasicMatch, static_cast<SyntaxToken>(*psz));
    }
    else
    {
        return TokenParser(BasicMatch, token);
    }
}
TokenParser operator_p(const char *psz)
{
    SyntaxToken token = SyntaxTokenFromString(psz);
    if (token == TokenInvalid)
    {
        // Not a known token.  Might be a single letter.
        ASSERT((lstrlen(psz) == 1) && (*psz < 128));
        return TokenParser(MatchOperator, static_cast<SyntaxToken>(*psz));
    }
    else
    {
        return TokenParser(MatchOperator, token);
    }
}
TokenParser alpha_p = TokenParser(MatchString, TokenAlphanum);
TokenParser said_p = TokenParser(MatchString, TokenSingleQuotedString);
TokenParser string_p = TokenParser(MatchString, TokenQuotedString);
TokenParser number_p = TokenParser(MatchNumber, TokenInteger);
TokenParser alwaysmatch_p = TokenParser(AlwaysMatch);
TokenParser oppar = token_p("(");
TokenParser clpar = token_p(")");
TokenParser opcurly = token_p("{");
TokenParser clcurly = token_p("}");
TokenParser opbracket = token_p("[");
TokenParser clbracket = token_p("]");
TokenParser semi = token_p(";");
TokenParser dot = token_p(".");
TokenParser comma = token_p(",");
TokenParser colon = token_p(":");
TokenParser equalSign = token_p("=");


//
// Common error primitives
//
void GeneralE2(TokenParser::MatchResult &match, const TokenParser *pParser, CPPSyntaxContext *pContext, const CrystalScriptTokenStream::const_iterator &stream)
{
    ASSERT(pParser->_token < 128); // Not valid to call this if there isn't something we can say was expected.
    if (!match.Result())
    {
        pContext->ReportExpectedChar((char)pParser->_token, stream);
        // REVIEW: we could support this, but we don't need it yet:
        //error += StringFromSyntaxToken(pParser->_token);
    }
}

void EndTokenE2(TokenParser::MatchResult &match, const TokenParser *pParser, CPPSyntaxContext *pContext, const CrystalScriptTokenStream::const_iterator &stream)
{
    ASSERT(pParser->_token < 128); // Not valid to call this if there isn't something we can say was expected.
    if (match.Result())
    {
        pContext->LevelPtr()->AddNode(new ASTNode(ASTNode::End, stream.GetPosition(), stream.GetEndPosition()));        
    }
    else
    {
        GeneralE2(match, pParser, pContext, stream);
    }
}


template<char const* error>
void Error2A(TokenParser::MatchResult &match, const TokenParser *pParser, CPPSyntaxContext *pContext, const CrystalScriptTokenStream::const_iterator &stream)
{
    // PERF: Instead of reporting errors (since we may report errors, but it actually succeeds), we
    // can just keep track of the char const*'s so we don't needlessly create strings.
    if (!match.Result())
    {
        pContext->ReportError(error, stream);
    }
}

template<typename _It>
void IdentifierE2(TokenParser::MatchResult &match, const TokenParser *pParser, CPPSyntaxContext *pContext, const _It &stream)
{
    if (!match.Result())
    {
        pContext->ReportError("Expected keyword.", stream);
    }
}

extern char const errInteger2[] = "Expected integer value.";
extern char const errPropertyValue[] = "Expected property value.";
extern char const errArgument2[] = "Expected argument.";
extern char const errExpression[] = "Expected expression.";
extern char const errExpressionUnary[] = "Expected expression after unary operator.";
extern char const errMethodProp[] = "Expected method or property name.";
extern char const errClassName[] = "Expected class name.";
extern char const errPropertyValueType[] = "Expected both property type and name.";
extern char const errInstanceName[] = "Expected instance name.";
extern char const errFunctionParameter[] = "Function parameter is missing type."; // Kind of an assumption
extern char const errFunctionParameterType[] = "Expected parameter type.";
extern char const errDefineName[] = "Expected name.";
extern char const errDefineValue[] = "Expected label or numeric value.";
extern char const errArrayValue[] = "Expected array value.";
extern char const errSelectorValue[] = "Expected selector.";
extern char const errPointerValue[] = "Expected argument.";
extern char const errResourceString[] = "Expected resource string.";
extern char const errVariableName[] = "Expected variable name.";

#ifdef DEBUG
size_t g_biggestNodeBay = 0;
#endif

//
// Actions to generate the AST (Abstract syntax tree)
//
template<ASTNode::Type type>
void RootWithStringA(TokenParser::MatchResult &match, const TokenParser *pParser, CPPSyntaxContext *pContext, const CrystalScriptTokenStream::const_iterator &stream)
{
    if (match.Result())
    {
        pContext->LevelPtr()->AssignRoot(new ASTNode(pContext->ScratchString(), type, stream.GetPosition(), stream.GetEndPosition()));
    }
}

template<ASTNode::Type type, const char *error>
void ErrorRootWithStringA(TokenParser::MatchResult &match, const TokenParser *pParser, CPPSyntaxContext *pContext, const CrystalScriptTokenStream::const_iterator &stream)
{
    if (match.Result())
    {
        pContext->LevelPtr()->AssignRoot(new ASTNode(pContext->ScratchString(), type, stream.GetPosition(), stream.GetEndPosition()));
    }
    else
    {
        pContext->ReportError(error, stream);
    }
}

template<ASTNode::Type type>
void TransitiveRootWithStringA(TokenParser::MatchResult &match, const TokenParser *pParser, CPPSyntaxContext *pContext, const CrystalScriptTokenStream::const_iterator &stream)
{
    if (match.Result())
    {
        pContext->LevelPtr()->InsertRoot(new ASTNode(pContext->ScratchString(), type, stream.GetPosition(), stream.GetEndPosition()));
    }
}

template<ASTNode::Type type>
void RootA(TokenParser::MatchResult &match, const TokenParser *pParser, CPPSyntaxContext *pContext, const CrystalScriptTokenStream::const_iterator &stream)
{
    if (match.Result())
    {
        pContext->LevelPtr()->AssignRoot(new ASTNode(type, stream.GetPosition(), stream.GetEndPosition()));
    }
}

template<ASTNode::Type type, const char *error>
void ErrorRootA(TokenParser::MatchResult &match, const TokenParser *pParser, CPPSyntaxContext *pContext, const CrystalScriptTokenStream::const_iterator &stream)
{
    if (match.Result())
    {
        pContext->LevelPtr()->AssignRoot(new ASTNode(type, stream.GetPosition(), stream.GetEndPosition()));
    }
    else
    {
        pContext->ReportError(error, stream);
    }
}

template<ASTNode::Type type>
void NodeWithStringA(TokenParser::MatchResult &match, const TokenParser *pParser, CPPSyntaxContext *pContext, const CrystalScriptTokenStream::const_iterator &stream)
{
    if (match.Result())
    {
        pContext->LevelPtr()->AddNode(new ASTNode(pContext->ScratchString(), type, stream.GetPosition(), stream.GetEndPosition()));
    }
}

// Like NodeWithStringA, but allows specification of an error.
template<ASTNode::Type type, char const* error>
void ErrorNodeWithStringA(TokenParser::MatchResult &match, const TokenParser *pParser, CPPSyntaxContext *pContext, const CrystalScriptTokenStream::const_iterator &stream)
{
    if (match.Result())
    {
        pContext->LevelPtr()->AddNode(new ASTNode(pContext->ScratchString(), type, stream.GetPosition(), stream.GetEndPosition()));
    }
    else
    {
        pContext->ReportError(error, stream);
    }
}


template<ASTNode::Type type>
void NodeWithNumberA(TokenParser::MatchResult &match, const TokenParser *pParser, CPPSyntaxContext *pContext, const CrystalScriptTokenStream::const_iterator &stream)
{
    if (match.Result())
    {
        pContext->LevelPtr()->AddNode(new ASTNode(pContext->Integer, pContext->IntFlags, type, stream.GetPosition(), stream.GetEndPosition()));
    }
}

template<ASTNode::Type type, const char *pszError>
void ErrorNodeWithNumberA(TokenParser::MatchResult &match, const TokenParser *pParser, CPPSyntaxContext *pContext, const CrystalScriptTokenStream::const_iterator &stream)
{
    if (match.Result())
    {
        pContext->LevelPtr()->AddNode(new ASTNode(pContext->Integer, pContext->IntFlags, type, stream.GetPosition(), stream.GetEndPosition()));
    }
    else
    {
        pContext->ReportError(pszError, stream);
    }
}

template<ASTNode::Type type>
void NodeA(TokenParser::MatchResult &match, const TokenParser *pParser, CPPSyntaxContext *pContext, const CrystalScriptTokenStream::const_iterator &stream)
{
    if (match.Result())
    {
        pContext->LevelPtr()->AddNode(new ASTNode(type, stream.GetPosition(), stream.GetEndPosition()));
    }
}

template<ASTNode::Type type>
void ErrorNodeA(TokenParser::MatchResult &match, const TokenParser *pParser, CPPSyntaxContext *pContext, const CrystalScriptTokenStream::const_iterator &stream)
{
    if (match.Result())
    {
        pContext->LevelPtr()->AddNode(new ASTNode(type, stream.GetPosition(), stream.GetEndPosition()));
    }
    else
    {
        ASSERT(pParser->_token < 128); // Not valid to call this if there isn't something we can say was expected.
        pContext->ReportExpectedChar((char)pParser->_token, stream);
        // REVIEW: we could support this, but we don't need it yet:
        //error += StringFromSyntaxToken(pParser->_token);
    }
}


template<ASTNode::Type type>
void NodeAPhil(TokenParser::MatchResult &match, const TokenParser *pParser, CPPSyntaxContext *pContext, const CrystalScriptTokenStream::const_iterator &stream)
{
    if (match.Result())
    {
        pContext->LevelPtr()->AddNode(new ASTNode(type, stream.GetPosition(), stream.GetEndPosition()));
    }
}





//
// This does the parsing.
//
// 2007/07/28
// GenerateScriptResource 14.5%
//	            generating code		7.5%
// SyntaxParser::parser   30%
// Tokensize              6.5%
// --------------
// total: 51%
bool CPPSyntaxParser::Parse(Script &script, CrystalScriptTokenStream::const_iterator &stream, ICompileLog *pError)
{
    ScriptId scriptId(script.GetPath().c_str());
    CPPSyntaxContext context(stream, script, *pError, scriptId);
    bool fRet = false;
    if (entire_script.Match(&context, stream).Result() && ((*stream).type == TokenEnd)) // Needs a full match
    {
        fRet = true;
        // Now evaluate the AST tree.
        context.EvaluateAST(); 
    }
    else
    {
        // With regards to syntax errors - there can really only be one, because we can't
        // recover afterwards.
        std::string strError = "  [Error]: ";
        strError += context.GetErrorText();
        CrystalScriptTokenStream::const_iterator errorPos = context.GetErrorPosition();
        if (pError)
        {
            // Add one to line#, since editor lines are 1-based
            pError->ReportResult(CompileResult(strError, scriptId, errorPos.GetLineNumber() + 1, errorPos.GetColumnNumber(), CompileResult::CRT_Error));
        }
    }
    return fRet;
}

bool CPPSyntaxParser::ParseHeader(Script &script, CrystalScriptTokenStream::const_iterator &stream, ICompileLog *pError)
{
    ScriptId scriptId(script.GetPath().c_str());
    CPPSyntaxContext context(stream, script, *pError, scriptId);
    bool fRet = entire_header.Match(&context, stream).Result() && ((*stream).type == TokenEnd); // Needs a full match
    if (fRet)
    {
        context.EvaluateAST(); 
    }
    else
    {
        std::stringstream ss;
        ss << " [Error] (" << script.GetTitle() << ") " << context.GetErrorText();
        CrystalScriptTokenStream::const_iterator errorPos = context.GetErrorPosition();
        if (pError)
        {
            pError->ReportResult(CompileResult(ss.str(), scriptId, errorPos.GetLineNumber() + 1, errorPos.GetColumnNumber(), CompileResult::CRT_Error));
        }
    }
    return fRet;
}

//
// The constructor sets up the parse tree
//
void CPPSyntaxParser::Load()
{
    if (_fLoaded)
    {
        return;
    }
    _fLoaded = true;
    assignment_operator = operator_p("=") | operator_p("+=") | operator_p("-=") | operator_p("*=") |
            operator_p("/=") | operator_p("%=") | operator_p("&=") |
            operator_p("|=") | operator_p("^=") | operator_p(">>=") |
            operator_p("<<=");
    _DEBUG_PARSER(assignment_operator);

    general_token = alpha_p;
    _DEBUG_PARSER(general_token);

    // alphanum, integer, string, said
    simple_value =
        alpha_p[NodeWithStringA<ASTNode::AlphaNumeric>]
        | number_p[NodeWithNumberA<ASTNode::Integer>]
        | string_p[NodeWithStringA<ASTNode::String>]
        | said_p[NodeWithStringA<ASTNode::Said>]
        | (token_p("#") >> alpha_p[ErrorNodeWithStringA<ASTNode::Selector, errSelectorValue>])
        | (token_p("@") >> alpha_p[ErrorNodeWithStringA<ASTNode::Pointer, errPointerValue>])
        | (token_p("res") >> string_p[ErrorNodeWithStringA<ASTNode::Res, errResourceString>]);
    _DEBUG_PARSER(simple_value)


    // alphanum, integer
    immediateValue = number_p[NodeWithNumberA<ASTNode::Integer>] | alpha_p[NodeWithStringA<ASTNode::AlphaNumeric>];
    _DEBUG_PARSER(immediateValue);

    code_block =
            opcurly[RootA<ASTNode::Block>]
        >>  *statement
        >>  clcurly;
    _DEBUG_PARSER(code_block)

    // simple  |  (expression)
    primary_expression =
	    simple_value
      | (oppar >> expression[RootA<ASTNode::Block>] >> clpar[GeneralE2])
	;
    _DEBUG_PARSER(primary_expression)

    array_index =
        opbracket >> expression[ErrorRootWithStringA<ASTNode::IndexedVariable, errExpression>] >> clbracket[GeneralE2];
    _DEBUG_PARSER(array_index)

    function_call =
        oppar[RootA<ASTNode::ProcedureCall>] >> *argument_expression_list >> clpar[GeneralE2];
    _DEBUG_PARSER(function_call)

    property_access =
        dot[RootA<ASTNode::PropertyAccess>] >> general_token[ErrorNodeWithStringA<ASTNode::Name, errMethodProp>];
    _DEBUG_PARSER(property_access)

    method_call =
        dot[RootA<ASTNode::MethodCall>] >> general_token[ErrorNodeWithStringA<ASTNode::Name, errMethodProp>] >> function_call
    ;
    _DEBUG_PARSER(method_call)

    cast_operator =
            dot
        >> token_p("cast")[RootA<ASTNode::Cast>]
        >> oppar[GeneralE2]
        >> alpha_p[NodeWithStringA<ASTNode::TypeSpecifier>]
        >> clpar[GeneralE2];
    _DEBUG_PARSER(cast_operator)

    postfix_expression =
         primary_expression[RootA<ASTNode::ExpressionRoot>]
      >> *(
             array_index      // array[3]                 // ok
	       | function_call    // function(1, 2, 3)
           | method_call      // object.method(1, 2, 3)
	       | property_access  // object.prop              // ok
           | cast_operator    // object.cast(int)
           )
    ;
    _DEBUG_PARSER(postfix_expression)

    unary_operator = operator_p("!") | operator_p("~") | operator_p("++") | operator_p("--") | operator_p("-");// | keyword_p("@");
    _DEBUG_PARSER(unary_operator)

    // Need to separate this out because of the composition of unary_expression (nodes could leak into
    // unary_expression if the "core" part succeeded then failed...
    unary_expression_core = unary_operator[RootWithStringA<ASTNode::Unary>]
         >> unary_expression[Error2A<errExpressionUnary>]
    ;
    _DEBUG_PARSER(unary_expression_core)

    // !pfexp  | pfexp
    unary_expression =
     (unary_expression_core | postfix_expression)
	;
    _DEBUG_PARSER(unary_expression)

    // unexp [* unexp]
    multiplicative_expression =
        unary_expression[Error2A<errExpression>] % ((operator_p("*") | operator_p("/") | operator_p("%"))[TransitiveRootWithStringA<ASTNode::Binary>])
	;
    _DEBUG_PARSER(multiplicative_expression)

    additive_expression =
           multiplicative_expression[Error2A<errExpression>] % ((operator_p("+") | operator_p("-"))[TransitiveRootWithStringA<ASTNode::Binary>])
    ;
    _DEBUG_PARSER(additive_expression)

    shift_expression =
        additive_expression[Error2A<errExpression>] % ((operator_p("<<") | operator_p(">>"))[TransitiveRootWithStringA<ASTNode::Binary>]);
	;
    _DEBUG_PARSER(shift_expression)

    relational_expression = 
	    shift_expression[Error2A<errExpression>] % ((operator_p("<") | operator_p(">") | operator_p("<=") | operator_p(">="))[TransitiveRootWithStringA<ASTNode::Binary>])
	;
    _DEBUG_PARSER(relational_expression)

    equality_expression =
	    relational_expression[Error2A<errExpression>] % ((operator_p("==") | operator_p("!="))[TransitiveRootWithStringA<ASTNode::Binary>])
	;
    _DEBUG_PARSER(equality_expression)

    and_expression =
	    equality_expression[Error2A<errExpression>] % operator_p("&")[TransitiveRootWithStringA<ASTNode::Binary>]
	;
    _DEBUG_PARSER(and_expression)

    exclusive_or_expression =
	    and_expression[Error2A<errExpression>] % operator_p("^")[TransitiveRootWithStringA<ASTNode::Binary>]
	;
    _DEBUG_PARSER(exclusive_or_expression)

    inclusive_or_expression =
	    exclusive_or_expression[Error2A<errExpression>] % operator_p("|")[TransitiveRootWithStringA<ASTNode::Binary>]
	;
    _DEBUG_PARSER(inclusive_or_expression)

    logical_and_expression =
	    inclusive_or_expression[Error2A<errExpression>] % operator_p("&&")[TransitiveRootWithStringA<ASTNode::Binary>]
    ;
    _DEBUG_PARSER(logical_and_expression)

    logical_or_expression =
	    logical_and_expression[Error2A<errExpression>] % operator_p("||")[TransitiveRootWithStringA<ASTNode::Binary>]
	;    
    _DEBUG_PARSER(logical_or_expression)

    /*
    conditional_expression = logical_or_expression;
    _DEBUG_PARSER(conditional_expression)*/

    expression_ternary = expression[RootA<ASTNode::Statement>];
    conditional_expression_ternary = conditional_expression[RootA<ASTNode::Statement>];

    conditional_expression = logical_or_expression
        >> *( // Handle ternary operation:
             operator_p("?")[RootA<ASTNode::Ternary>]
          >> expression_ternary              // statement: compatibility with _EvaluateIf
          >> operator_p(":")[ErrorNodeA<ASTNode::Else>]
          >> conditional_expression_ternary  // statement: compatibility with _EvaluateIf
            );
    _DEBUG_PARSER(conditional_expression)

    lvalue_expression = 
        unary_expression[RootA<ASTNode::LValue>]
    ;
    _DEBUG_PARSER(lvalue_expression)
    rvalue_expression = 
        assignment_expression[RootA<ASTNode::RValue>]
    ;
    _DEBUG_PARSER(rvalue_expression)

    assignment_expression_core =
        (lvalue_expression >> assignment_operator[RootWithStringA<ASTNode::Assignment>] >> rvalue_expression);

    assignment_expression =
        assignment_expression_core
      | conditional_expression
	  ;
    _DEBUG_PARSER(assignment_expression);

    expression =
        assignment_expression % comma[GeneralE2];
      ;
    _DEBUG_PARSER(expression);

    expression_statement =
        semi[GeneralE2]
      | (expression >> semi[GeneralE2]);
    _DEBUG_PARSER(expression_statement)

    argument_expression_list =
	    (rest_statement | assignment_expression)[Error2A<errArgument2>] % comma[GeneralE2];
	;
    _DEBUG_PARSER(argument_expression_list)

    loop_condition = expression[ErrorRootA<ASTNode::Condition, errExpression>];
    _DEBUG_PARSER(loop_condition)

    if_statement =
         token_p("if")[RootA<ASTNode::If>]
      >> oppar[GeneralE2] >> loop_condition >> clpar 
      >> statement
      >> !(token_p("else")[NodeA<ASTNode::Else>] >> statement);
    ;
    _DEBUG_PARSER(if_statement)

    // Just so a bunch of statements can have its own root
    statement_list = (*statement)[RootA<ASTNode::Block>];

    case_statement =
        token_p("case")[RootA<ASTNode::Case>] >> expression >> colon >> statement_list;
    ;
    default_statement =
        token_p("default")[RootA<ASTNode::Default>] >> colon >> statement_list;
    ;

    // Treat switch conditions like a block - they can be separated by commas
    // Only different from loop_condition in that we generate a CodeBlock object and
    // not a ConditionalExpression.
    switch_condition = expression[ErrorRootA<ASTNode::Block, errExpression>];
    _DEBUG_PARSER(switch_condition);

    // We differ from standard c syntax here.
    switch_statement =
        token_p("switch")[RootA<ASTNode::Switch>]
      >> oppar >> switch_condition >> clpar
      >> opcurly
      >> *(case_statement | default_statement)
      >> clcurly
    ;
    _DEBUG_PARSER(switch_statement)

    selection_statement =
        if_statement | switch_statement
    ;
    _DEBUG_PARSER(selection_statement)

    do_loop =
           token_p("do")[RootA<ASTNode::Do>]
        >> statement
        >> token_p("while")
        >> oppar[GeneralE2] >> loop_condition >> clpar[GeneralE2] >> semi[GeneralE2]
    ;
    _DEBUG_PARSER(do_loop)

    while_loop =
           token_p("while")[RootA<ASTNode::While>]
        >> oppar[GeneralE2] >> loop_condition >> clpar[GeneralE2]
        >> statement
    ;
    _DEBUG_PARSER(while_loop)

    for_loop_initializer = expression_statement[ErrorRootA<ASTNode::Initializer, errExpression>];
    for_loop_condition = expression_statement[ErrorRootA<ASTNode::Condition, errExpression>];
    for_loop_looper = expression[ErrorRootA<ASTNode::Looper, errExpression>];

    for_loop =
           token_p("for")[RootA<ASTNode::For>]
        >> oppar[GeneralE2]
        >> for_loop_initializer
        >> for_loop_condition
        >> !for_loop_looper  // ! because it might not exist.
        >> clpar[GeneralE2]
        >> statement;
    ;
    _DEBUG_PARSER(for_loop)

    iteration_statement =
        do_loop | while_loop | for_loop
    ;
    _DEBUG_PARSER(iteration_statement)

    break_statement = token_p("break")[NodeA<ASTNode::Break>]
     >> semi[GeneralE2];
    _DEBUG_PARSER(break_statement)

    return_statement =
        token_p("return")[RootA<ASTNode::Return>]
     >> !expression
     >> semi[GeneralE2];
    _DEBUG_PARSER(return_statement)

    rest_statement =
            token_p("rest")
        >>  alpha_p[RootWithStringA<ASTNode::Rest>];

    jump_statement =
        break_statement
      | return_statement
    ;
    _DEBUG_PARSER(jump_statement)

    // Building block
    statement = alwaysmatch_p[RootA<ASTNode::Statement>] >>
        code_block  // "compound_statement"
      | jump_statement
      | expression_statement
      | selection_statement
      | iteration_statement
      ;
    // REVIEW: we may want to allow variable declarations anywhere.
    // That would allow for loops to work for (int i = 0)
    // We would need to be careful about context-sensitivity in the parser though...
    // And we would probably need full scoping rules.
    _DEBUG_PARSER(statement)

    // foo[4]
    var_decl =
        alpha_p[ErrorRootWithStringA<ASTNode::Name, errVariableName>]
    >> !(opbracket >> number_p[NodeWithNumberA<ASTNode::VariableSize>] >> clbracket);
    _DEBUG_PARSER(var_decl)

    // A little different than script-level array_init, since it allows code to execute.
    function_array_init = opcurly[GeneralE2]
        >> (assignment_expression[Error2A<errExpression>] % comma[GeneralE2])
        >> clcurly[GeneralE2];  // {0 0 $4c VIEW_EGO}

    // var i = 4, j, x = 3;
    function_var_decl = alpha_p[RootWithStringA<ASTNode::VariableDeclarationsOfType>]
        >>
         ((var_decl >> !(token_p("=") >> ((assignment_expression[Error2A<errExpression>] | function_array_init)) )) % comma)
        >> semi[GeneralE2];
    _DEBUG_PARSER(function_var_decl)


    parameter_base = alpha_p[ErrorNodeWithStringA<ASTNode::TypeSpecifier, errFunctionParameterType>] 
        >> alpha_p[ErrorRootWithStringA<ASTNode::Parameter, errFunctionParameter>];
    _DEBUG_PARSER(parameter_base)

    parameter = parameter_base | token_p("...")[RootA<ASTNode::MoreParameters>];
    _DEBUG_PARSER(parameter)

    // Methods
    method_decl = !token_p("private")[NodeA<ASTNode::Private>]
           >> procedure_base[RootA<ASTNode::Method>];
    _DEBUG_PARSER(method_decl)

    // Classes
    property_value =
        simple_value
        ;
    _DEBUG_PARSER(property_value)

    instance_property_decl =
           alpha_p[ErrorRootWithStringA<ASTNode::Property, errPropertyValue>]
        >> equalSign[GeneralE2]
        >> property_value[Error2A<errPropertyValue>]
        >> semi[GeneralE2];
    _DEBUG_PARSER(instance_property_decl);

    instance_properties_decl = *instance_property_decl;
    _DEBUG_PARSER(instance_properties_decl);

    // Like instance properties, but with a type.
    property_decl =
           alpha_p[ErrorRootWithStringA<ASTNode::Property, errPropertyValue>]
        >> !alpha_p[NodeWithStringA<ASTNode::TypeSpecifier>] // This is a lie! Type and property name are switched.
        >> equalSign[GeneralE2]
        >> property_value[Error2A<errPropertyValue>]
        >> semi[GeneralE2];
    _DEBUG_PARSER(property_decl);

    properties_decl = *property_decl;
    _DEBUG_PARSER(properties_decl);

    classbase_decl = !token_p("public")[NodeA<ASTNode::Public>]
        >> general_token[ErrorRootWithStringA<ASTNode::Class, errClassName>]
        >> !(colon[GeneralE2] >> alpha_p[ErrorNodeWithStringA<ASTNode::SuperClass, errClassName>])
        >> opcurly[GeneralE2]
        >> !properties_decl
        >> *method_decl
        >> clcurly[EndTokenE2];
    _DEBUG_PARSER(classbase_decl)

    instancebase_decl = !token_p("public")[NodeA<ASTNode::Public>]
        >> token_p("instance")
        >> general_token[ErrorRootWithStringA<ASTNode::Instance, errInstanceName>]
        >> colon[GeneralE2]
        >> alpha_p[ErrorNodeWithStringA<ASTNode::SuperClass, errClassName>]
        >> opcurly[GeneralE2]
        >> !instance_properties_decl
        >> *method_decl
        >> clcurly[EndTokenE2];
    _DEBUG_PARSER(instancebase_decl)

    // Main script pieces.
    instance_decl = instancebase_decl;
    _DEBUG_PARSER(instance_decl)

    class_decl = token_p("class") >> classbase_decl;
    _DEBUG_PARSER(class_decl)

    parameter_sequence = (parameter % comma)
                      >> !(opbracket[NodeA<ASTNode::Optional>] >> comma[GeneralE2] >> (parameter % comma) >> clbracket[GeneralE2]); // Optional params
    _DEBUG_PARSER(parameter_sequence)

    initial_optional_parameter_sequence = (opbracket[NodeA<ASTNode::Optional>] >> (parameter % comma) >> clbracket[GeneralE2]); // Optional params
    _DEBUG_PARSER(initial_optional_parameter_sequence)

    function_signature =
           alpha_p[NodeWithStringA<ASTNode::TypeSpecifier>]
        >> alpha_p[NodeWithStringA<ASTNode::Name>]
        >> oppar[RootA<ASTNode::FunctionSignature>]
        >> !(parameter_sequence | initial_optional_parameter_sequence)
        >> clpar[GeneralE2];

    // init(blah balh) { code }
    procedure_base = 
            function_signature
        >> *function_signature // Yes, we allow for more than one function signature.
        >> !(colon >> alpha_p[NodeWithStringA<ASTNode::Class>])
        >> opcurly[GeneralE2]
        >> *function_var_decl
        >> statement_list
        >> clcurly[EndTokenE2];
    _DEBUG_PARSER(procedure_base)

    include = token_p("include") >> string_p[RootWithStringA<ASTNode::Include>] >> semi[GeneralE2];
    _DEBUG_PARSER(include)

    use = token_p("use") >> string_p[RootWithStringA<ASTNode::Use>] >> semi[GeneralE2];
    _DEBUG_PARSER(use)

    define = token_p("define") 
        >> alpha_p[ErrorRootWithStringA<ASTNode::Define, errDefineName>] 
        >> immediateValue[Error2A<errDefineValue>] 
        >> semi[GeneralE2];
    _DEBUG_PARSER(define)

    // An array initializer
    array_init = opcurly[GeneralE2]
        >> (immediateValue[Error2A<errArrayValue>] % comma[GeneralE2])
        >> clcurly[GeneralE2];  // {0 0 $4c VIEW_EGO}

    script_var = alpha_p[RootWithStringA<ASTNode::VariableDeclarationsOfType>]
        >> 
          ((var_decl >> !(token_p("=")[GeneralE2] >> (immediateValue | array_init))) % comma)
        >> semi[GeneralE2];
    _DEBUG_PARSER(script_var)

    synonyms = token_p("synonym")
        >> said_p[RootWithStringA<ASTNode::Synonym>]
        >> equalSign[GeneralE2]
        >> said_p[NodeWithStringA<ASTNode::Name>]
        >> semi[GeneralE2];
    _DEBUG_PARSER(synonyms)

    procedure_decl = !token_p("public")[NodeA<ASTNode::Public>]
        >> procedure_base[RootA<ASTNode::Procedure>];
    _DEBUG_PARSER(procedure_decl)

    entire_script = alwaysmatch_p[RootA<ASTNode::Script>] >>
          *(
            (include
            | use
            | define
            | synonyms
            | script_var
            | instance_decl
            | class_decl
            | procedure_decl      // comes last, since it isn't preceded by a keyword
            )[IdentifierE2]);
    _DEBUG_PARSER(entire_script)

    // And for headers, only defines and includes are allowed.
    entire_header = alwaysmatch_p[RootA<ASTNode::Script>] >> *((include | define)[IdentifierE2]);
    _DEBUG_PARSER(entire_header)
}

//Strings for the enum
#undef ENUM_OR_STRING
#define ENUM_OR_STRING(x) #x
char * c_szASTNodeTypes[] =
{
    AST_NODE_TYPES
}; 

//
// AST Debugging
//
struct ASTDebug
{
    ASTDebug(std::stringstream &out, int cIndent) : _out(out), _cIndent(cIndent) {}
    std::stringstream &_out;
    int _cIndent;
};

void _DebugOutputAST(ASTDebug &debug, ASTNode *pNode)
{
    if (pNode)
    {
        for (int i = 0; i < debug._cIndent; i++)
        {
            debug._out << "  ";
        }
        bool fNoText = pNode->GetText().empty();
        debug._out << setw(4) << setfill('0') << c_szASTNodeTypes[pNode->GetType()] << ": ";
        if (pNode->GetText().empty())
        {
            debug._out << pNode->GetNumber();
        }
        else
        {
            debug._out << pNode->GetText();
        }
        debug._out << "\n";

        ASTDebug debugChild(debug._out, debug._cIndent + 1);
        std::vector<ASTNode*>::iterator it = pNode->Children().begin();
        while (it != pNode->Children().end())
        {
            _DebugOutputAST(debugChild, *it);
            ++it;
        }
    }
}

void CPPSyntaxContext::DebugOutputAST()
{
    if (_pTree)
    {
        OutputDebugString("Abstract Syntax Tree\n");
        OutputDebugString("---------------------\n");
        std::stringstream out;
        int cIndent = 0;
        ASTDebug debug(out, cIndent);
        _DebugOutputAST(debug, _pTree);
        OutputDebugString(out.str().c_str());
    }
}


//
// Turn the AST into a completed Script object.
//

// FWD decl
SingleStatement *_EvaluateStatement(CPPSyntaxContext &context, Script &script, ASTNode *pActual);
SingleStatement _EvaluateStatement2(CPPSyntaxContext &context, Script &script, ASTNode *pActual);
SyntaxNode *_EvaluateExpressionRoot(CPPSyntaxContext &context, Script &script, ASTNode *pParent, size_t endIndex);
ComplexPropertyValue *_EvaluateIndexedVariable(CPPSyntaxContext &context, Script &script, ASTNode *pParent, size_t endIndex);

bool _IsSimplePropertyValue(ASTNode *pNode)
{
    switch (pNode->GetType())
    {
    case ASTNode::AlphaNumeric:
    case ASTNode::Said:
    case ASTNode::String:
    case ASTNode::Integer:
        return true;
    }
    return false;
}

void _ReportIntegerOverflow(CPPSyntaxContext &context, ASTNode *pNode)
{
    if ((pNode->GetNumber() > 65535) || (pNode->GetNumber() < -32768))
    {
        std::stringstream ss;
        ss << "Number is too large:" << pNode->GetNumber() << ".";
        context.ReportASTError(ss.str(), pNode->GetPosition());
    }
}

//
// Transfer position information from the AST node to the syntax node.
// In general, whomever news the syntax node should call this.
//
void _SyncPosition(SyntaxNode *pSyntaxNode, const ASTNode *pASTNode)
{
    pSyntaxNode->SetPosition(pASTNode->GetPosition());
    pSyntaxNode->SetEndPosition(pASTNode->GetPosition());
}

PropertyValue _GetPropertyValue(CPPSyntaxContext &context, ASTNode *pNode)
{
    PropertyValue value;
    switch (pNode->GetType())
    {
    case ASTNode::AlphaNumeric:
    case ASTNode::Said:
    case ASTNode::String:
    case ASTNode::Selector:
    case ASTNode::Pointer:
        ValueType type;
        switch (pNode->GetType())
        {
            case ASTNode::AlphaNumeric:
                type = ValueTypeToken;
                break;
            case ASTNode::Said:
                type = ValueTypeSaid;
                break;
            case ASTNode::String:
                type = ValueTypeString;
                break;
            case ASTNode::Selector:
                type = ValueTypeSelector;
                break;
            case ASTNode::Pointer:
                type = ValueTypePointer;
                break;
        }
        value.SetValue(pNode->GetText(), type);
        break;
    case ASTNode::Integer:
        _ReportIntegerOverflow(context, pNode);
        value.SetValue(pNode->GetNumber(), pNode->GetIntegerFlags());
        break;
    default:
        ASSERT(FALSE);
    }
    _SyncPosition(&value, pNode);
    return value;
}

void _EvaluateParameter(CPPSyntaxContext &context, Script &script, FunctionSignature *pSignature, ASTNode *pNode, bool fOptional)
{
    ASSERT(pNode->Children().size() == 1);
    auto_ptr<FunctionParameter> pParam(new FunctionParameter(pNode->GetText()));
    pParam->SetDataType(pNode->Children()[0]->GetText());
    _SyncPosition(pParam.get(), pNode);
    pSignature->AddParam(pParam, fOptional);
    // Once we've had an optional parameter, we can have no more non optional ones... this is enforced in
    // the grammar, and ASSERTed in FunctionSignature::AddParam.
}

template<typename _T>
void _EvaluateVariableDeclarations(CPPSyntaxContext &context, Script &script, _T *pFunction, ASTNode *pParent)
{
    // Children have, in this order:
    // Name
    //      Size        -> what about array initializers????  Let's allow them
    // then Integer     -> could just have a series of these for array initializers.
    ASSERT(pParent->GetType() == ASTNode::VariableDeclarationsOfType);
    std::vector<ASTNode*>::iterator &nodeIt = pParent->Children().begin();
    std::vector<ASTNode*>::iterator &nodeEnd = pParent->Children().end(); 
    while (nodeIt != nodeEnd)
    {
        ASTNode *pNode = (*nodeIt);
        ASSERT(pNode->GetType() == ASTNode::Name);
        auto_ptr<VariableDecl> pVar(new VariableDecl());
        _SyncPosition(pVar.get(), pNode);
        pVar->SetDataType(pParent->GetText());
        pVar->SetName(pNode->GetText());
        ASSERT(pNode->Children().size() <= 1);
        if (pNode->Children().size() == 1)
        {
            // We could have one child, which is the size.
            _ReportIntegerOverflow(context, pNode->Children()[0]);
            pVar->SetSize(static_cast<WORD>(pNode->Children()[0]->GetNumber()));
        }
        // Now we could have an initializer list.
        PropertyValue value;
        value.Zero(); // By default...
        ++nodeIt;
        while (nodeIt != nodeEnd)
        {
            if ((*nodeIt)->GetType() == ASTNode::Name) // Name signals the next variable...
            {
                break;
            }
            if (_IsSimplePropertyValue(*nodeIt))
            {
                value = _GetPropertyValue(context, *nodeIt);
                // Add it
                pVar->AddSimpleInitializer(value);
            }
            else
            {
                pVar->AddStatement(_EvaluateStatement(context, script, *nodeIt));
            }
            ++nodeIt;
        }
        // Add it
        pFunction->AddVariable(pVar);
    }
}


bool _IsSendCall(ASTNode *pNode)
{
    return (pNode->GetType() == ASTNode::MethodCall) ||
           (pNode->GetType() == ASTNode::PropertyAccess);
}

//
// Contains some special logic for coaslescing sends.
//
void _EvaluateStatementList(CPPSyntaxContext &context, Script &script, StatementsNode *pStatementsHost, ASTNode *pParent, bool fAllowCoalesce = true)
{
    std::vector<ASTNode*>::iterator &nodeIt = pParent->Children().begin();
    std::vector<ASTNode*>::iterator &nodeEnd = pParent->Children().end();
    sci::SendCall *pSend = NULL;
    while (nodeIt != nodeEnd)
    {
        auto_ptr<SingleStatement> pStatement(_EvaluateStatement(context, script, *nodeIt));
        // Possibly coalesce a send.
        bool fCoalesce = false;
        if (pSend && (pStatement->GetType() == NodeTypeSendCall) && fAllowCoalesce)
        {
            // The last one was a send and so is this
            sci::SendCall *pNewSend = static_cast<sci::SendCall*>(pStatement->GetSyntaxNode());
            // Are they sending to the same thing?  Note: we need to check that name isn't empty,
            // since there are other ways to express send objects.
            // REVIEW: we could expand this a little
            if (!pNewSend->GetName().empty() && (pNewSend->GetName() == pSend->GetName()))
            {
                // Transfer will remove the send params from pNewSend.
                pSend->TransferParamsFrom(pNewSend);
                // When we set fCoalesce to true, we won't add the statement containing the send
                // to the pStatementsHost, so it will get deleted.
                fCoalesce = true;
            }
        }
        if (!fCoalesce)
        {
            // For next time:
            pSend = (pStatement->GetType() == NodeTypeSendCall) ? static_cast<sci::SendCall*>(pStatement->GetSyntaxNode()) : NULL;
            pStatementsHost->AddStatement(pStatement);
        }

        ++nodeIt;
    }
}

ConditionalExpression _EvaluateTernaryConditionalExpression(CPPSyntaxContext &context, Script &script, ASTNode *pParent)
{
    ConditionalExpression condExp;
    _SyncPosition(&condExp, pParent);
    auto_ptr<SingleStatement> pConditionalStatement(_EvaluateStatement(context, script, pParent));
    condExp.AddStatement(pConditionalStatement);
    return condExp;
}

ConditionalExpression _EvaluateConditionalExpression(CPPSyntaxContext &context, Script &script, ASTNode *pParent)
{
    ConditionalExpression condExp;
    // It is possible we'll have multiple conditions.  They are all evaluated, even though we throw away
    // all but the last one's return value.
    // while (a, b, c)
    // { /* do stuff */ }
    _SyncPosition(&condExp, pParent);
    ASSERT(pParent->GetType() == ASTNode::Condition);
    BOOST_FOREACH(ASTNode *pConditionChild, pParent->Children())
    {
        auto_ptr<SingleStatement> pConditionalStatement(_EvaluateStatement(context, script, pConditionChild));
        condExp.AddStatement(pConditionalStatement);
    }
    return condExp;
}

void _EvaluateCase(CPPSyntaxContext &context, Script &script, SwitchStatement *pSwitch, ASTNode *pCaseNode)
{
    ASSERT((pCaseNode->GetType() == ASTNode::Case) || (pCaseNode->GetType() == ASTNode::Default));
    auto_ptr<CaseStatement> pCase(new CaseStatement);
    _SyncPosition(pCase.get(), pCaseNode);
    if (pCaseNode->GetType() == ASTNode::Case)
    {
        ASSERT(pCaseNode->Children().size() == 2);
        pCase->SetStatement1(_EvaluateStatement2(context, script, pCaseNode->Children()[0]));
        _EvaluateStatementList(context, script, pCase.get(), pCaseNode->Children()[1]);
    }
    else if (pCaseNode->GetType() == ASTNode::Default)
    {
        ASSERT(pCaseNode->Children().size() == 1);
        pCase->SetDefault(true);
        _EvaluateStatementList(context, script, pCase.get(), pCaseNode->Children()[0]);
    }
    pSwitch->AddCase(pCase);
}

SyntaxNode *_EvaluateSwitch(CPPSyntaxContext &context, Script &script, ASTNode *pParent)
{
    ASSERT(pParent->GetType() == ASTNode::Switch);
    auto_ptr<SwitchStatement> pSwitch(new SwitchStatement);
    _SyncPosition(pSwitch.get(), pParent);
    // Switch
    //   Expression
    //   Case
    //       Expression
    //       Block
    //          foo...
    //   Case
    ASSERT(pParent->Children().size() >= 1);
    std::vector<ASTNode*>::iterator &nodeIt = pParent->Children().begin();
    std::vector<ASTNode*>::iterator &nodeEnd = pParent->Children().end();
    if (nodeIt != nodeEnd)
    {
        pSwitch->SetStatement1(_EvaluateStatement2(context, script, *nodeIt));
        ++nodeIt;
    }
    while (nodeIt != nodeEnd)
    {
        _EvaluateCase(context, script, pSwitch.get(), *nodeIt);
        ++nodeIt;
    }

    return pSwitch.release();
}

SyntaxNode *_EvaluateIf(CPPSyntaxContext &context, Script &script, ASTNode *pParent)
{
    // If
    //   Expression Source
    //     < stuff>
    //   Statement
    //   Else           - opt
    //   Statement      - opt
    ASSERT((pParent->GetType() == ASTNode::If) || ((pParent->GetType() == ASTNode::Ternary)));
    auto_ptr<CppIfStatement> pIf(new CppIfStatement);
    _SyncPosition(pIf.get(), pParent);
    //ASSERT(pParent->Children().size() >= 2);    
    ASSERT((pParent->Children().size() == 2) || (pParent->Children().size() == 4));
    std::vector<ASTNode*>::iterator &nodeIt = pParent->Children().begin();
    std::vector<ASTNode*>::iterator &nodeEnd = pParent->Children().end();
    if (nodeIt != nodeEnd)
    {
        if (pParent->GetType() == ASTNode::Ternary)
        {
            pIf->MakeTernary();
            pIf->SetCondition(_EvaluateTernaryConditionalExpression(context, script, *nodeIt));
        }
        else
        {
            pIf->SetCondition(_EvaluateConditionalExpression(context, script, *nodeIt));
        }
        ++nodeIt;
    }
    if (nodeIt != nodeEnd)
    {
        ASTNode *pStatement = *nodeIt;
        ASSERT(pStatement->GetType() == ASTNode::Statement);
        pIf->SetStatement1(_EvaluateStatement2(context, script, pStatement));
        // We only ever have one... it may be a code block...
        ++nodeIt;
    }
    if (nodeIt != nodeEnd)
    {
        ASSERT((*nodeIt)->GetType() == ASTNode::Else);
        ++nodeIt;
    }
    if (nodeIt != nodeEnd)
    {
        ASTNode *pStatement = *nodeIt;
        ASSERT(pStatement->GetType() == ASTNode::Statement);
        pIf->SetStatement2(_EvaluateStatement2(context, script, pStatement));
    }
    return pIf.release();
}

SyntaxNode *_EvaluateDoLoop(CPPSyntaxContext &context, Script &script, ASTNode *pParent)
{
    // Do
    //   Statement
    //   <something>  -> condition
    //
    ASSERT(pParent->GetType() == ASTNode::Do);
    auto_ptr<DoLoop> pLoop(new DoLoop);
    _SyncPosition(pLoop.get(), pParent);
    ASSERT(pParent->Children().size() == 2);
    std::vector<ASTNode*>::iterator &nodeIt = pParent->Children().begin();
    std::vector<ASTNode*>::iterator &nodeEnd = pParent->Children().end();
    if (nodeIt != nodeEnd)
    {
        ASTNode *pStatement = *nodeIt;
        ASSERT(pStatement->GetType() == ASTNode::Statement);
        auto_ptr<SingleStatement> pSafe(_EvaluateStatement(context, script, pStatement));
        // We only ever have one... it may be a code block...
        pLoop->AddStatement(pSafe);
        ++nodeIt;       
    }
    if (nodeIt != nodeEnd)
    {
        pLoop->SetCondition(_EvaluateConditionalExpression(context, script, *nodeIt));
    }
    return pLoop.release();
}


SyntaxNode *_EvaluateWhileLoop(CPPSyntaxContext &context, Script &script, ASTNode *pParent)
{
    // While
    //   <condition> (could be anything)
    //   Statement
    ASSERT(pParent->GetType() == ASTNode::While);
    auto_ptr<WhileLoop> pLoop(new WhileLoop);
    _SyncPosition(pLoop.get(), pParent);
    ASSERT(pParent->Children().size() == 2);
    std::vector<ASTNode*>::iterator &nodeIt = pParent->Children().begin();
    std::vector<ASTNode*>::iterator &nodeEnd = pParent->Children().end();
    if (nodeIt != nodeEnd)
    {
        pLoop->SetCondition(_EvaluateConditionalExpression(context, script, *nodeIt));
        ++nodeIt;
    }
    if (nodeIt != nodeEnd)
    {
        ASTNode *pStatement = *nodeIt;
        ASSERT(pStatement->GetType() == ASTNode::Statement);
        auto_ptr<SingleStatement> pSafe(_EvaluateStatement(context, script, pStatement));
        // We only ever have one... it may be a code block...
        pLoop->AddStatement(pSafe);
    }
    return pLoop.release();
}

SyntaxNode *_EvaluateBreak(CPPSyntaxContext &context, Script &script, ASTNode *pParent)
{
    ASSERT(pParent->GetType() == ASTNode::Break);
    return new BreakStatement;
}

SyntaxNode *_EvaluateReturn(CPPSyntaxContext &context, Script &script, ASTNode *pParent)
{
    ASSERT(pParent->GetType() == ASTNode::Return);
    auto_ptr<ReturnStatement> pReturn(new ReturnStatement);
    _SyncPosition(pReturn.get(), pParent);

    if (!pParent->Children().empty())
    {
        // There's a value to return.
        ASSERT(pParent->Children().size() == 1);
        pReturn->SetStatement1(_EvaluateStatement2(context, script, pParent->Children()[0]));
    }
    return pReturn.release();
}

SyntaxNode *_EvaluateRest(CPPSyntaxContext &context, Script &script, ASTNode *pParent)
{
    ASSERT(pParent->GetType() == ASTNode::Rest);
    auto_ptr<RestStatement> pRest(new RestStatement);
    _SyncPosition(pRest.get(), pParent);
    pRest->SetName(pParent->GetText());
    ASSERT(pParent->Children().empty());
    return pRest.release();
}

SyntaxNode *_EvaluateForLoop(CPPSyntaxContext &context, Script &script, ASTNode *pParent)
{
    // For
    //   Initializer
    //     <could have many children>
    //   Condition
    //     <just one?>
    //   Looper
    //     <could have many children>
    //   Statement
    ASSERT(pParent->GetType() == ASTNode::For);
    auto_ptr<ForLoop> pLoop(new ForLoop);
    _SyncPosition(pLoop.get(), pParent);
    ASSERT(pParent->Children().size() >= 3); // Could have 3 or 4.
    std::vector<ASTNode*>::iterator &nodeIt = pParent->Children().begin();
    std::vector<ASTNode*>::iterator &nodeEnd = pParent->Children().end();
    if (nodeIt != nodeEnd)
    {
        ASTNode *pInit = *nodeIt;
        ASSERT(pInit->GetType() == ASTNode::Initializer);
        CodeBlock block;
        _EvaluateStatementList(context, script, &block, pInit);
        pLoop->SetCodeBlock(block); // Initializers.
        ++nodeIt;
    }
    if (nodeIt != nodeEnd)
    {
        pLoop->SetCondition(_EvaluateConditionalExpression(context, script, *nodeIt));
        ++nodeIt;
    }
    if (nodeIt != nodeEnd)
    {
        ASTNode *pLooper = *nodeIt;
        if (pLooper->GetType() == ASTNode::Looper)
        {
            CodeBlock block;
            _EvaluateStatementList(context, script, &block, pLooper);
            pLoop->SetLooper(block);
            ++nodeIt;
        }
    }
    if (nodeIt != nodeEnd)
    {
        ASTNode *pStatement = *nodeIt;
        ASSERT(pStatement->GetType() == ASTNode::Statement);
        auto_ptr<SingleStatement> pSafe(_EvaluateStatement(context, script, pStatement));
        // We only ever have one... it may be a code block...
        pLoop->AddStatement(pSafe);
    }
    return pLoop.release();
}


CodeBlock *_EvaluateBlock(CPPSyntaxContext &context, Script &script, ASTNode *pParent)
{
    ASSERT(pParent->GetType() == ASTNode::Block);
    auto_ptr<CodeBlock> pBlock(new CodeBlock);
    _SyncPosition(pBlock.get(), pParent);
    _EvaluateStatementList(context, script, pBlock.get(), pParent);
    return pBlock.release();
}

SyntaxNode *_EvaluateUnary(CPPSyntaxContext &context, Script &script, ASTNode *pParent)
{
    ASSERT(pParent->GetType() == ASTNode::Unary);
    auto_ptr<UnaryOp> pUnary(new UnaryOp);
    _SyncPosition(pUnary.get(), pParent);
    pUnary->SetName(pParent->GetText());
    ASSERT(pParent->Children().size() == 1);
    pUnary->SetStatement1(_EvaluateStatement2(context, script, pParent->Children()[0]));
    return pUnary.release();
}


SyntaxNode *_EvaluateBinary(CPPSyntaxContext &context, Script &script, ASTNode *pParent)
{
    ASSERT(pParent->GetType() == ASTNode::Binary);
    auto_ptr<BinaryOp> pBinary(new BinaryOp);
    _SyncPosition(pBinary.get(), pParent);
    pBinary->SetName(pParent->GetText());
    ASSERT(pParent->Children().size() == 2);
    pBinary->SetStatement1(_EvaluateStatement2(context, script, pParent->Children()[0]));
    pBinary->SetStatement2(_EvaluateStatement2(context, script, pParent->Children()[1]));
    return pBinary.release();
}

SyntaxNode *_EvaluateCast(CPPSyntaxContext &context, Script &script, ASTNode *pParent, size_t endIndex)
{
    //
    // ExpressionRoot
    //   Alphanumeric
    //   [anything]
    //   Cast
    //     TypeSpecifier
    //
    ASSERT(pParent->GetType() == ASTNode::ExpressionRoot);
    ASSERT(pParent->Children().size() >= 2);
    ASSERT(endIndex >= 2);
    ASTNode *pCastNode = pParent->Children()[endIndex - 1];
    ASTNode *pOriginalObject = pParent->Children()[endIndex - 2];
    ASSERT(pCastNode->GetType() == ASTNode::Cast);

    // Make a new cast object, with the specified type and source object.
    auto_ptr<Cast> pCast(new Cast);
    _SyncPosition(pCast.get(), pCastNode);

    // REVIEW: this might produce problems in code that checks to see if a SyntaxNode is a simple variable.
    // ++(fobar.cast(int)); -> this won't compile
    // Not a high priority scenario.  An alternate cast implementation might be to have castable support on
    // any SyntaxNode node.

    // Set the source object
    // REVIEW: This is slightly similar to _EvaluateSendCall, but not quite enough to re-use code.
    if (pOriginalObject->GetType() == ASTNode::AlphaNumeric)
    {
        // This functionality is a little different.  This is a special case for sends.
        // We need to treat it specially here too because the code we have that packages up
        // a property value into a statement doesn't recognize just a bare "alphanumeric".
        SingleStatement statement;
        auto_ptr<PropertyValue> newPV(new PropertyValue(_GetPropertyValue(context, pOriginalObject)));
        statement.SetSyntaxNode(newPV);
        pCast->SetStatement1(statement);
    }
    else
    {
        ASSERT(endIndex >= 2);
        // It must be sequence of things.  Recursively evaluate the ones before us (endIndex - 1).
        SyntaxNode *pNode = _EvaluateExpressionRoot(context, script, pParent, endIndex - 1);
        // And use this as our statement.
        SingleStatement statement;
        statement.SetSyntaxNode(pNode);
        pCast->SetStatement1(statement); // pNode will we tranfered from statement here.
        ASSERT(statement.GetSyntaxNode() == NULL);
    }

    // Set the type
    ASSERT(pCastNode->Children().size() == 1);
    pCast->SetDataType(pCastNode->Children()[0]->GetText());

    return pCast.release();
}

// Pseudo-policy class for the _EvaluateSendCall function.
class FinishOffMethod
{
    //
    // ExpressionRoot
    //   Alphanumeric
    //   [anything]
    //   [anything]
    //   MethodCall
    //     Name
    //     ParameterList
    //
public:
    static void FinishOff(CPPSyntaxContext &context, Script &script, SendCall *pSend, ASTNode *pMethodCall, SingleStatement *pAssignment = NULL)
    {
        // In c-syntax, we just have a single "send param"
        auto_ptr<SendParam> pSendParam(new SendParam);
        _SyncPosition(pSendParam.get(), pMethodCall);
        ASSERT(pMethodCall->Children().size() == 2);

        // Now the actual method name
        ASTNode *pNodeName = pMethodCall->Children()[0];
        ASSERT(pNodeName->GetType() == ASTNode::Name);
        pSendParam->SetName(pNodeName->GetText());

        // Now the parameters.
        ASTNode *pParameters = pMethodCall->Children()[1];
        ASSERT(pParameters->GetType() == ASTNode::ProcedureCall);
        _EvaluateStatementList(context, script, pSendParam.get(), pParameters, false);
        pSendParam->SetIsMethod(true);
        pSend->AddSendParam(pSendParam);
    }
};

class FinishOffProperty
{
    //
    // ExpressionRoot
    //   Alphanumeric
    //   [anything]
    //   [anything]
    //   PropertyAccess
    //     Name
    //     ParameterList
    //
public:
    static void FinishOff(CPPSyntaxContext &context, Script &script, SendCall *pSend, ASTNode *pProperty, SingleStatement *pAssignment = NULL)
    {
        // In c-syntax, we just have a single "send param"
        auto_ptr<SendParam> pSendParam(new SendParam);
        _SyncPosition(pSendParam.get(), pProperty);
        ASSERT(pProperty->Children().size() == 1);

        // Now the actual property name
        ASTNode *pNodeName = pProperty->Children()[0];
        ASSERT(pNodeName->GetType() == ASTNode::Name);
        pSendParam->SetName(pNodeName->GetText());

        // Now the parameter
        if (pAssignment)
        {
            // foo.x = 5;
            pSendParam->SetIsMethod(true); // Essentially a method call if we provide a statement here
            pSendParam->AddStatement(pAssignment);
        }
        else
        {
            // no parameters. e.g.:
            // return foo.x);
        }

        // Now add our completed send param to the send call..
        pSend->AddSendParam(pSendParam);
    }
};


template<typename _TFinisher>
SyntaxNode *_EvaluateSendCall(CPPSyntaxContext &context, Script &script, ASTNode *pParent, size_t endIndex, SingleStatement *pAssignment = NULL)
{
    ASSERT(pParent->GetType() == ASTNode::ExpressionRoot);
    ASSERT(pParent->Children().size() >= 2);
    ASSERT(endIndex >= 2);
    ASTNode *pSendCallNode = pParent->Children()[endIndex - 1];
    ASTNode *pSendObjectNode = pParent->Children()[endIndex - 2];
    ASSERT((pSendCallNode->GetType() == ASTNode::MethodCall) ||
           (pSendCallNode->GetType() == ASTNode::PropertyAccess));

    auto_ptr<SendCall> pSend(new SendCall);
    _SyncPosition(pSend.get(), pParent);

    // First, figure out what object we're calling.
    if (pSendObjectNode->GetType() == ASTNode::AlphaNumeric)
    {
        ASSERT(endIndex == 2); // This must be the beginning of the "expression"
        pSend->SetName(pSendObjectNode->GetText());
    }
    else if (pSendObjectNode->GetType() == ASTNode::Block)
    {
        ASSERT(endIndex == 2); // This must be the beginning of the "expression"
        auto_ptr<SingleStatement> pCallObject(_EvaluateStatement(context, script, pSendObjectNode));
        pSend->SetStatement1(*pCallObject); // Copied, so don't release pCallObject
    }
    else
    {
        ASSERT(endIndex > 2);
        // It must be sequence of things.  Recursively evaluate the ones before us (endIndex - 1).
        SyntaxNode *pNode = _EvaluateExpressionRoot(context, script, pParent, endIndex - 1);
        // And use this as our statement.
        SingleStatement statement;
        statement.SetSyntaxNode(pNode);
        pSend->SetStatement1(statement); // pNode will we tranfered from statement here.
        ASSERT(statement.GetSyntaxNode() == NULL);
    }

    _TFinisher::FinishOff(context, script, pSend.get(), pSendCallNode, pAssignment);

    return pSend.release();
}

SyntaxNode * _EvaluateProcedureCall(CPPSyntaxContext &context, Script &script, ASTNode *pParent)
{
    //
    // ExpressionRoot
    //   AlphaNumeric
    //   ProcedureCall
    //     ParameterList
    //       ...
    //
    ASSERT(pParent->GetType() == ASTNode::ExpressionRoot);
    auto_ptr<ProcedureCall> pProcedureCall(new ProcedureCall);
    _SyncPosition(pProcedureCall.get(), pParent);

    // First we need to find out what we're calling.  Procedures must be alphanumeric things - no expressions.
    // Since there is no way to get a procedure pointer.
    // FUTURE: I suppose you could theoretically pass a proc to a function.  Wouldn't work for kernels though.
    // e.g. FormatPrint("The address of the PrintNotCloseEnough function is %x", PrintNotCloseEnough);
    // Actually even that won't work I think... in SCI byte code we never have the procedure address, just its index.
    // So, we could enforce this rule in grammar, but it wrecks the rule a bit.  So enforce it here.
    if (pParent->Children().size() != 2)
    {
        context.ReportASTError("Not valid to have a procedure call here.", pParent->GetPosition());
    }
    else
    {
        // Set the name
        ASTNode *pNodeProcedureName = pParent->Children()[0];
        if (pNodeProcedureName->GetType() != ASTNode::AlphaNumeric)
        {
            context.ReportASTError("Not a valid procedure name.", pParent->GetPosition());
        }
        else
        {
            pProcedureCall->SetName(pNodeProcedureName->GetText());
        }

        // Now handle parameters.  Should be a single child with a parameter list
        ASTNode *pNodeProcedureCall = pParent->Children()[1];
        ASSERT(pNodeProcedureCall->GetType() == ASTNode::ProcedureCall);
        _EvaluateStatementList(context, script, pProcedureCall.get(), pNodeProcedureCall, false);
    }
    return pProcedureCall.release();
}


LValue _EvaluateLValue(CPPSyntaxContext &context, Script &script, ASTNode *pParent)
{
    LValue value;
    if (pParent->GetType() == ASTNode::ExpressionRoot)
    {
        // ExpressionSource
        //   AlphaNumeric
        //   IndexedVariable

        // First the name.
        ASSERT(pParent->Children().size() >= 1);
        ASTNode *pSource = pParent->Children()[0];

        if (pSource->GetType() == ASTNode::AlphaNumeric)
        {
            value.SetName(pSource->GetText());
        }
        else
        {
            context.ReportASTError("Expected identifier before '[]'.", pSource->GetPosition());
        }

        // Now the indexer.
        if (pParent->Children().size() > 1)
        {
            ASSERT(pParent->Children().size() == 2); // No more, no less, or we have a bug.
            ASTNode *pIndexParent = pParent->Children()[1];
            ASSERT(pIndexParent->Children().size() == 1);
            ASTNode *pIndex = pIndexParent->Children()[0];
            value.SetIndexer(_EvaluateStatement2(context, script, pIndex));
        }
    }
    else if (pParent->GetType() == ASTNode::IndexedVariable)
    {
        ASSERT(FALSE); // I think we're covered by _EvaluateExpressionRoot...
    }
    else
    {
        if (pParent->GetType() == ASTNode::Unary)
        {
            // Our grammar rules allow if (!x = 5), but we don't want to allow that. (c++ does not).
            std::stringstream errorText;
            errorText << "'" << pParent->GetText() << "' is not allowed on an l-value";
            context.ReportASTError(errorText.str(), pParent->GetPosition());
        }
        else
        {
            ASSERT(FALSE); // compiler error
        }
    }

    return value;
}

bool IsPropertyAccess(ASTNode *pExpression)
{
   return (pExpression->GetType() == ASTNode::ExpressionRoot) &&
          (pExpression->Children().back()->GetType() == ASTNode::PropertyAccess);
}

SyntaxNode * _EvaluateAssignment(CPPSyntaxContext &context, Script &script, ASTNode *pParent)
{
    ASSERT(pParent->GetType() == ASTNode::Assignment);
    ASSERT(pParent->Children().size() == 2);

    // We'll do the right node first... you'll see why in a minute.
    ASTNode *pRNode = pParent->Children()[1];
    ASSERT(pRNode->Children().size() == 1);
    ASSERT(pRNode->GetType() == ASTNode::RValue);
    ASTNode *pRValue = pRNode->Children()[0];

    ASTNode *pLNode = pParent->Children()[0];
    ASSERT(pLNode->Children().size() == 1);
    ASSERT(pLNode->GetType() == ASTNode::LValue);
    ASTNode *pLValue = pLNode->Children()[0];
    // This could take a few forms...
    if (IsPropertyAccess(pLValue))
    {
        // This will actually up being a send call.
        return _EvaluateSendCall<FinishOffProperty>(context, script, pLValue, pLValue->Children().size(), _EvaluateStatement(context, script, pRValue));
    }
    else
    {
        auto_ptr<Assignment> pAssignment(new Assignment);
        _SyncPosition(pAssignment.get(), pParent);
        pAssignment->SetStatement1(_EvaluateStatement2(context, script, pRValue));
        pAssignment->SetVariable(_EvaluateLValue(context, script, pLValue));
        pAssignment->SetName(pParent->GetText());
        return pAssignment.release();
    }
}

SyntaxNode *_EvaluateExpressionRoot(CPPSyntaxContext &context, Script &script, ASTNode *pParent, size_t endIndex)
{
    ASSERT(pParent->GetType() == ASTNode::ExpressionRoot);
    // An expression root is of the form:
    // expression  *(arrayindex | propertyaccess | methodcall | functioncall)
    // We need to start from the end.
    // e.g.
    // a[5].foo
    // a arrayindex methodcall
    // (a arrayindex) methodcall

    ASTNode *pNode = pParent->Children()[endIndex - 1];
    if (endIndex == 1)
    {
        // REVIEW: does this need to be a complex property value?
        auto_ptr<ComplexPropertyValue> pValue(new ComplexPropertyValue);
        _SyncPosition(pValue.get(), pParent);

        // There is but a single child.
        switch (pNode->GetType())
        {
        case ASTNode::Integer:
            _ReportIntegerOverflow(context, pNode);
            pValue->SetValue(pNode->GetNumber(), pNode->GetIntegerFlags());
            break;
        case ASTNode::AlphaNumeric:
            pValue->SetValue(pNode->GetText(), ValueTypeToken);
            break;
        case ASTNode::String:
            pValue->SetValue(pNode->GetText(), ValueTypeString);
            break;
        case ASTNode::Said:
            pValue->SetValue(pNode->GetText(), ValueTypeSaid);
            break;
        case ASTNode::Selector:
            pValue->SetValue(pNode->GetText(), ValueTypeSelector);
            break;
        case ASTNode::Pointer:
            pValue->SetValue(pNode->GetText(), ValueTypePointer);
            break;
        case ASTNode::Res:
            pValue->SetValue(pNode->GetText(), ValueTypeResourceString);
            break;
        case ASTNode::Block:
            {
                // Special one.
                auto_ptr<CodeBlock> pBlock(_EvaluateBlock(context, script, pNode));
                pValue.reset(pBlock->ReduceBlock<ComplexPropertyValue>());
                if (!pValue.get())
                {
                    // It couldn't be reduced.  Just return the block.
                    return pBlock.release();
                }
            }
            break;
        default:
            ASSERT(FALSE); // Nothing else is allowed.
            break;
        }
        return pValue.release();
    }
    else
    {
        // These are things that can follow the root.  We start from the end and work in.
        switch (pNode->GetType())
        {
        case ASTNode::PropertyAccess:
            return _EvaluateSendCall<FinishOffProperty>(context, script, pParent, endIndex, NULL);
            break;
        case ASTNode::MethodCall:
            return _EvaluateSendCall<FinishOffMethod>(context, script, pParent, endIndex, NULL);
            break;
        case ASTNode::Cast:
            return _EvaluateCast(context, script, pParent, endIndex);
            break;
        case ASTNode::ProcedureCall:
            // Different than the others, because it can only be preceded by a single node and
            // followed by none (REVIEW: why not let it be followed by many, like IndexedVariable?).
            // (enforced in _EvaluateProcedureCall)
            return _EvaluateProcedureCall(context, script, pParent);
            break;
        case ASTNode::IndexedVariable:
            // Different than the others, because like ProcedureCall, it can only appear in slot two.
            // See the details in _EvaluateIndexedVariable.
            return _EvaluateIndexedVariable(context, script, pParent, endIndex);
            break;
        default:
            ASSERT(FALSE);
            break;
        }
    }
    ASSERT(FALSE);
    return NULL;
}

ComplexPropertyValue *_EvaluateIndexedVariable(CPPSyntaxContext &context, Script &script, ASTNode *pParent, size_t endIndex)
{
    auto_ptr<ComplexPropertyValue> pValue(new ComplexPropertyValue);
    //
    // ExpressionRoot
    //   AlphaNumeric
    //   IndexedVariable
    //
    ASSERT(pParent->GetType() == ASTNode::ExpressionRoot);
    if (endIndex != 2)
    {
        // In SCI, variable indexing doesn't dereference a pointer, it just advances the value.
        // e.g.  foo is the same as foo[0].
        // So foo needs to be a local, global, temp or parameter variable.  It can't be a property,
        // the result of a method, or anything like that.  Therefore, an IndexedVariable node must follow
        // only a single AlphaNumeric node.
        ASSERT(endIndex > 2);
        context.ReportASTError("Not a valid spot for an array indexer.", pParent->Children()[1]->GetPosition());
    }
    else
    {
        // The value...
        bool fApplyIndexer = true;
        ASTNode *pSource = pParent->Children()[0];
        switch (pSource->GetType())
        {
        case ASTNode::AlphaNumeric:
            pValue->SetValue(pSource->GetText(), ValueTypeToken);
            break;
        case ASTNode::Pointer:
            pValue->SetValue(pSource->GetText(), ValueTypePointer);
            break;
        default:
            fApplyIndexer = false; // If we apply it, the compiler will assert.
            context.ReportASTError("Indexers can only be applied to variables.", pParent->GetPosition());
            break;
        }

        // Now the indexer.
        if (fApplyIndexer)
        {
            ASTNode *pIndexParent = pParent->Children()[1];
            ASSERT(pIndexParent->Children().size() == 1);
            ASTNode *pIndex = pIndexParent->Children()[0];
            auto_ptr<SingleStatement> pIndexer(_EvaluateStatement(context, script, pIndex));
            pValue->SetIndexer(pIndexer);
        }
    }
    return pValue.release();
}

SingleStatement _EvaluateStatement2(CPPSyntaxContext &context, Script &script, ASTNode *pActual)
{
    auto_ptr<SingleStatement> pStatement(_EvaluateStatement(context, script, pActual));
    return *pStatement;
}

SingleStatement *_EvaluateStatement(CPPSyntaxContext &context, Script &script, ASTNode *pActual)
{
    auto_ptr<SingleStatement> pStatement(new SingleStatement);
    _SyncPosition(pStatement.get(), pActual);
    SyntaxNode *pSyntaxNode = NULL;

    // No sense in having a statement inside a statement.
    while (pActual->GetType() == ASTNode::Statement)
    {
        ASSERT(pActual->Children().size() == 1);
        pActual = pActual->Children()[0];
    }

    switch (pActual->GetType())
    {
    case ASTNode::Assignment:
        pSyntaxNode = _EvaluateAssignment(context, script, pActual);
        break;
    case ASTNode::Binary:
        pSyntaxNode = _EvaluateBinary(context, script, pActual);
        break;
    case ASTNode::Unary:
        pSyntaxNode = _EvaluateUnary(context, script, pActual);
        break;
    case ASTNode::Block:
        pSyntaxNode = _EvaluateBlock(context, script, pActual);
        break;
    case ASTNode::If:
        pSyntaxNode = _EvaluateIf(context, script, pActual);
        break;
    case ASTNode::Ternary:
        pSyntaxNode = _EvaluateIf(context, script, pActual);
        break;
    case ASTNode::Switch:
        pSyntaxNode = _EvaluateSwitch(context, script, pActual);
        break;
    case ASTNode::Do:
        pSyntaxNode = _EvaluateDoLoop(context, script, pActual);
        break;
    case ASTNode::While:
        pSyntaxNode = _EvaluateWhileLoop(context, script, pActual);
        break;
    case ASTNode::For:
        pSyntaxNode = _EvaluateForLoop(context, script, pActual);
        break;
    case ASTNode::Break:
        pSyntaxNode = _EvaluateBreak(context, script, pActual);
        break;
    case ASTNode::Rest:
        pSyntaxNode = _EvaluateRest(context, script, pActual);
        break;
    case ASTNode::Return:
        pSyntaxNode = _EvaluateReturn(context, script, pActual);
        break;
    case ASTNode::ExpressionRoot:
        pSyntaxNode = _EvaluateExpressionRoot(context, script, pActual, pActual->Children().size());
        break;
    default:
        ASSERT(FALSE);
        break;
    }
    if (pSyntaxNode)
    {
        auto_ptr<SyntaxNode> pSafe(pSyntaxNode);
        // REVIEW: we really need start and end position... this is end position as start position
        pSafe->SetPosition(pActual->GetPosition());
        pStatement->SetSyntaxNode(pSafe);
    }
    return pStatement.release();
}

void _EvaluateFunctionSignature(CPPSyntaxContext &context, Script &script, FunctionBase *pFunc, ASTNode *pParent)
{
    auto_ptr<FunctionSignature> pSignature(new FunctionSignature);
    _SyncPosition(pSignature.get(), pParent);
    ASSERT(pParent->GetType() == ASTNode::FunctionSignature);
    bool fOptional = false;
    BOOST_FOREACH(ASTNode *pChild, pParent->Children())
    {
        switch(pChild->GetType())
        {
        case ASTNode::TypeSpecifier:
            pSignature->SetDataType(pChild->GetText());
            break;

        case ASTNode::Parameter:
            if (pSignature->GetMoreParametersAllowed())
            {
                context.ReportASTError("The '...' parameter must be the final one in a function signature.", pChild->GetPosition());
            }
            _EvaluateParameter(context, script, pSignature.get(), pChild, fOptional);
            break;

        case ASTNode::Optional:
            fOptional = true; // All future parameters are optional.
            break;

        case ASTNode::MoreParameters:
            if (pSignature->GetMoreParametersAllowed())
            {
                context.ReportASTError("Only one '...' parameter is allowed in a function signature.", pChild->GetPosition());
            }
            pSignature->SetMoreParametersAllowed(true);
            break;

        case ASTNode::Name:
            if (!pFunc->GetName().empty() && (pFunc->GetName() != pChild->GetText()))
            {
                std::stringstream ss;
                ss << "Alternate function signature for " << pFunc->GetName() << " has different name: " << pChild->GetText();
                context.ReportASTError(ss.str(), pChild->GetPosition());
            }
            else
            {
                pFunc->SetName(pChild->GetText());
            }
            break;
        }
    }
    pFunc->AddSignature(pSignature);
}

void _EvaluateFunctionBase(CPPSyntaxContext &context, Script &script, FunctionBase *pFunc, ASTNode *pParent, ProcedureDefinition *pProcedure = NULL, MethodDefinition *pMethod = NULL)
{
    ASSERT(pParent->GetText().empty()); // The name is not here.
    pFunc->SetScript(&script);
    std::vector<ASTNode*>::iterator &nodeIt = pParent->Children().begin();
    while (nodeIt != pParent->Children().end())
    {
        ASTNode *pNode = (*nodeIt);
        switch (pNode->GetType())
        {
        case ASTNode::FunctionSignature:
            _EvaluateFunctionSignature(context, script, pFunc, pNode);
            break;

        case ASTNode::VariableDeclarationsOfType:
            _EvaluateVariableDeclarations(context, script, pFunc, pNode);
            break;

        case ASTNode::Block:
            // This is the bulk of the function body.
            _EvaluateStatementList(context, script, pFunc, pNode, true);
            break;

        case ASTNode::Public:
            pProcedure->SetPublic(true);
            break;

        case ASTNode::Private:
            pMethod->SetPrivate(true);
            break;

        case ASTNode::Class:
            if (pProcedure)
            {
                pProcedure->SetClass(pNode->GetText());
            }
            else
            {
                // We use the same grammar rule for methods and procedures, but
                // methods can't have the ": class" syntax.
                std::stringstream ss;
                ss << "Didn't expect " << pNode->GetText() << " on a method.";
                context.ReportASTError(ss.str(), pNode->GetPosition());
            }
            break;

        case ASTNode::End:
            pFunc->SetEndPosition(pNode->GetPosition());
            break;

        default:
            ASSERT(FALSE);
            break;
        }
        ++nodeIt;
    }
}

void _EvaluateMethod(CPPSyntaxContext &context, Script &script, ClassDefinition *pClass, ASTNode *pParent)
{
    auto_ptr<MethodDefinition> pMethod(new MethodDefinition());
    _SyncPosition(pMethod.get(), pParent);
    pMethod->SetOwnerClass(pClass);
    _EvaluateFunctionBase(context, script, pMethod.get(), pParent, NULL, pMethod.get());
    pClass->AddMethod(pMethod);
}

void _EvaluateProcedure(CPPSyntaxContext &context, Script &script, ASTNode *pParent)
{
    auto_ptr<ProcedureDefinition> pProc(new ProcedureDefinition());
    _SyncPosition(pProc.get(), pParent);
    _EvaluateFunctionBase(context, script, pProc.get(), pParent, pProc.get());
    script.AddProcedure(pProc);
}

// Handles both classes and instances.
void _EvaluateInstance(CPPSyntaxContext &context, Script &script, ASTNode *pParent, bool fInstance)
{
    auto_ptr<ClassDefinition> pClass(new ClassDefinition());
    _SyncPosition(pClass.get(), pParent);
    pClass->SetName(pParent->GetText());
    pClass->SetInstance(fInstance);
    pClass->SetScript(&script);
    std::vector<ASTNode*>::iterator &nodeIt = pParent->Children().begin();
    while (nodeIt != pParent->Children().end())
    {
        ASTNode *pNode = (*nodeIt);
        switch (pNode->GetType())
        {
        case ASTNode::SuperClass:
            pClass->SetSuperClass(pNode->GetText());
            break;
        case ASTNode::Property:
            {
                auto_ptr<ClassProperty> pProperty(new ClassProperty());
                _SyncPosition(pProperty.get(), pNode);
                if (fInstance || (pNode->Children().size() == 1)) // Classes may have 2 nodes.
                {
                    // Straightforward
                    pProperty->SetName(pNode->GetText());
                    ASSERT(pNode->Children().size() == 1);
                    pProperty->SetValue(_GetPropertyValue(context, pNode->Children()[0]));
                }
                else
                {
                    // Property (actually specifies type)
                    //      TypeSpecifier (actually specifies property name)
                    //      - some kind of value -

                    // In this case we have a class, WITH
                    // datatype selector = value;  INSTEAD OF
                    // selector = value;
                    ASSERT(pNode->Children().size() == 2);
                    // Only thing is, the type and name are switched
                    pProperty->SetDataType(pNode->GetText()); // See?
                    // Now the name...
                    ASTNode *pNodeType = pNode->Children()[0];
                    ASSERT(pNodeType->GetType() == ASTNode::TypeSpecifier);
                    pProperty->SetName(pNodeType->GetText());
                    // Now the value
                    pProperty->SetValue(_GetPropertyValue(context, pNode->Children()[1]));
                }
                pClass->AddProperty(pProperty);
            }
            break;
        case ASTNode::Method:
            _EvaluateMethod(context, script, pClass.get(), pNode);
            break;

        case ASTNode::Public:
            pClass->SetPublic(true);
            break;

        case ASTNode::End:
            pClass->SetEndPosition(pNode->GetPosition());
            break;

        default:
            ASSERT(FALSE);
            break;
        }
        ++nodeIt;
    }
    script.AddClass(pClass);
}

void _EvaluateScriptVariable(CPPSyntaxContext &context, Script &script, ASTNode *pParent)
{
ASSERT(FALSE);
}

void _EvaluateSynonym(CPPSyntaxContext &context, Script &script, ASTNode *pParent)
{
    ASSERT(pParent->GetType() == ASTNode::Define);
    ASSERT(pParent->Children().size() == 1);
    ASSERT(pParent->Children()[0]->GetType() == ASTNode::Name);
    auto_ptr<Synonym> pSynonym(new Synonym);
    _SyncPosition(pSynonym.get(), pParent);
    pSynonym->MainWord = pParent->GetText();
    pSynonym->Replacement = pParent->Children()[0]->GetText();
    script.AddSynonym(pSynonym);
}

void _EvaluateDefine(CPPSyntaxContext &context, Script &script, ASTNode *pParent)
{
    ASSERT(pParent->GetType() == ASTNode::Define);
    ASSERT(pParent->Children().size() == 1);
    ASTNode *pChild = pParent->Children()[0];
    ASSERT((pChild->GetType() == ASTNode::Integer) || (pChild->GetType() == ASTNode::AlphaNumeric));
    auto_ptr<Define> pDefine(new Define);
    _SyncPosition(pDefine.get(), pParent);
    pDefine->SetLabel(pParent->GetText());
    if (pChild->GetType() == ASTNode::Integer)
    {
        _ReportIntegerOverflow(context, pChild);
        pDefine->SetValue(pChild->GetNumber(), pChild->GetIntegerFlags());
    }
    else
    {
        pDefine->SetValue(pChild->GetText());
    }
    pDefine->SetScript(&script);
    script.AddDefine(pDefine);
}

void CPPSyntaxContext::EvaluateAST()
{
    std::vector<ASTNode*>::iterator &nodeIt = _pTree->Children().begin();
    while (nodeIt != _pTree->Children().end())
    {
        ASTNode *pNode = (*nodeIt);
        switch (pNode->GetType())
        {
        case ASTNode::Include:
            _script.AddInclude(pNode->GetText());
            break;
        case ASTNode::Use:
            _script.AddUse(pNode->GetText());
            break;
        case ASTNode::Instance:
            _EvaluateInstance(*this, _script, pNode, true);
            break;
        case ASTNode::Class:
            _EvaluateInstance(*this, _script, pNode, false);
            break;
        case ASTNode::Define:
            _EvaluateDefine(*this, _script, pNode);
            break;
        case ASTNode::VariableDeclarationsOfType:
            _EvaluateVariableDeclarations(*this, _script, &_script, pNode);
            break;
        case ASTNode::Procedure:
            _EvaluateProcedure(*this, _script, pNode);
            break;
        case ASTNode::Synonym:
            _EvaluateSynonym(*this, _script, pNode);
            break;
        }
        ++nodeIt;
    }
}

#ifdef PARSE_DEBUG
int g_ParseIndent2 = 0;
#endif
