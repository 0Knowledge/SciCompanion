#pragma once

//
// Manages the type system for Cpp syntax
//

// Fwd decl
namespace sci
{
    class PropertyValueBase;
    class SingleStatement;
}
class ICompileContext;

// These are hard coded data type values that exist in the same space as the game's species values.
// Species values begin from 0 and go up (a unique one for every class in the game).
// These values begin from 0xffff and go down.  The only one that has anything to do with the
// interpreter (as opposed to being strictly a compile thing) is 0xffff, which the interpeter
// uses as the base class for classes that have no base class.
// All the other values are used by the c-style compiler.

// Standard data types
static const WORD DataTypeNone                  = 0xffff; // Used by the interpreter as "empty" base class
static const WORD DataTypeAny                   = 0xfffe; // e.g. "var"
static const WORD DataTypeVoid                  = 0xfffd; // e.g. "void"
static const WORD DataTypeInt                   = 0xfffc;
static const WORD DataTypeUInt                  = 0xfffb;
static const WORD DataTypeBool                  = 0xfffa;
static const WORD DataTypeChar                  = 0xfff9;
static const WORD DataTypeString                = 0xfff8;
static const WORD DataTypeSaidString            = 0xfff7;
static const WORD DataTypePointer               = 0xfff6;
static const WORD DataTypeSelector              = 0xfff5;

// These are some other data types used in kernel functions.
static const WORD DataTypeKWindow               = 0xffdf;
static const WORD DataTypeKList                 = 0xffde;
static const WORD DataTypeKNode                 = 0xffdd;

// Special token for resource strings (two values)
static const WORD DataTypeResourceString        = 0xffce;

static const WORD DataTypeInvalid               = 0xffbf; // Used as an uninitialized SpeciesIndex.

static const WORD UserDefinedDataTypeBoundary   = 0xf000; // Everything below this number is an object.

//
// 
//
class SpeciesIndex
{
public:
    SpeciesIndex() : _wType(DataTypeInvalid) {}
    SpeciesIndex(WORD w) : _wType(w) {}
    WORD Type() const { return _wType; }
    bool operator==(SpeciesIndex &other) const
    {
        return _wType == other._wType;
    }
    bool operator==(WORD w) const
    {
        return _wType == w;
    }
    bool operator<(WORD w) const
    {
        return _wType < w;
    }
    bool operator<=(WORD w) const
    {
        return _wType <= w;
    }
    bool operator>(WORD w) const
    {
        return _wType > w;
    }
    bool operator>=(WORD w) const
    {
        return _wType >= w;
    }
    bool operator!=(SpeciesIndex &other) const
    {
        return _wType != other._wType;
    }
    SpeciesIndex &operator=(const SpeciesIndex &src)
    {
        if (this != &src)
        {
            _wType = src._wType;
        }
        return *this;
    }
    operator WORD() const
    {
        return _wType;
    }
private:
    WORD _wType;
};

// Returns true (and the species index) if type is a built in data type
bool GetBuiltInDataTypeSpecies(const std::string &type, SpeciesIndex &wSpecies);

// Is type a built in data type?
bool IsPODType(const std::string &type);

// Is si a built in data type?
bool IsPODType(SpeciesIndex si);

bool IsPointerType(SpeciesIndex type);

// Is this an unsigned type?
bool IsUnsignedType(SpeciesIndex si);

std::string GetBuiltInDataTypeString(SpeciesIndex wSpecies);

//
// The principal function to call to determine if two types match.  There are different rules
// depending on which is the source and which is the destination type.
//
// Additional rules may apply if an operator is supplied (e.g. '<' vs "==")
//
// A statement may be supplied.  This will be used for places where there are some default conversions
// (e.g. int can be converted to string if the int is a zero value)
//
bool DoesTypeMatch(ICompileContext &context, SpeciesIndex destType, const SpeciesIndex sourceType, const std::string *pOperator = NULL, const sci::SingleStatement *pStatement = NULL);



