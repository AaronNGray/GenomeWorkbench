#ifndef GUI_OBJUTILS___MACRO_EDIT_FN_BASE__HPP
#define GUI_OBJUTILS___MACRO_EDIT_FN_BASE__HPP
/*  $Id: macro_edit_fn_base.hpp 44444 2019-12-19 15:54:41Z asztalos $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software/database is freely available
 *  to the public for use. The National Library of Medicine and the U.S.
 *  Government have not placed any restriction on its use or reproduction.
 *
 *  Although all reasonable efforts have been taken to ensure the accuracy
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors: Anatoly Osipov, Andrea Asztalos
 *
 * File Description: Interface class for macro function implementation
 *
 */

/// @file macro_fn_base.hpp
/// Interface class for macro function implementation

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>
#include <gui/objutils/macro_fn_base.hpp>
#include <gui/objutils/macro_exec.hpp>
#include <gui/objutils/macro_biodata_iter.hpp>

/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(macro)


//////////////////////////////////////////////////////////////////
/// Base class for any user function that performs editing operations on ASN.1 data 
///
class NCBI_GUIOBJUTILS_EXPORT IEditMacroFunction : public IMacroFunction<CIRef<IMacroBioDataIter>>
{
public:
    using TParent = IMacroFunction<CIRef<IMacroBioDataIter>>;
    using TNodeValuesVec = vector<CRef<CMQueryNodeValue>>;

    /// Virtual destructor
    virtual ~IEditMacroFunction() {}

    /// Functions make the action and collect statistics
    ///@param[in,out] oi objectinfo that is being set
    ///@param[in] value string value that will be set to the objectinfo
    bool SetQualStringValue(CObjectInfo& oi, const string& value);

protected:
    IEditMacroFunction(EScopeEnum func_scope)
        : IMacroFunction<CIRef<IMacroBioDataIter>>(func_scope), m_QualsChangedCount(0){}

    virtual void x_InitCall(CIRef<IMacroBioDataIter>& data);

    /// Extracts the function arguments and initializes m_Result member
    virtual void x_SetUserObjects(CQueryParseTree::TNode& qnode);

    /// Tests the number and the type of function arguments
    virtual bool x_ValidArguments() const = 0;
    /// Reset the state of member variables that are set directly/indirectly from function arguments
    virtual void x_ResetState() {}

    virtual bool x_IsNestedFunctionReturnValid() const { return true; }

    /// Assigns value to m_Result. It is mostly useful for functions used in the WHERE clause
    void x_AssignReturnValue(const CObjectInfo& oi, const string& field_name);
    void x_AssignReturnValueFromContainer(const CObjectInfo& oi, const string& container, const string& field_name);

    void x_GetObjectsFromRef(CMQueryNodeValue::TObs& objects, const size_t& index);

    void x_GetOptionalArgs(string& delimiter, bool& remove_field, size_t& index);

    CIRef<IMacroBioDataIter>  m_DataIter; 
    TNodeValuesVec m_Args;
    CRef<CMQueryNodeValue> m_Result;

    Int4 m_QualsChangedCount;  ///< Number of changed qualifiers during the function call
    string m_FuncName;         ///< Name of the function as it's been used in the script
};

struct NCBI_GUIOBJUTILS_EXPORT SArgMetaData
{
    SArgMetaData(const string& name, CMQueryNodeValue::EType type, bool opt)
        : m_Name(name), m_Type(type), m_Optional(opt) {}

    string m_Name;
    CMQueryNodeValue::EType m_Type;
    bool m_Optional;
};

///////////////////////////////////////////////////////////////////////////////
/// Built-in functions
/// 
/// To add the function the following steps need to be done:
/// 1)  Add a class declaration in one of the macro_fn_*.hpp files
/// 2)  Add function definition in the corresponding macro_fn_*.cpp file
/// 3)  Register function name in macro_engine.cpp. This is the name which is expected to 
///     be seen inside macro. To do this just add a corresponding line 
///     in CMacroEngine::x_InitSetOfBuiltInFunctions.
///
///////////////////////////////////////////////////////////////////////////////
/// Macro for defining the user function
#define DECLARE_FUNC_CLASS(CL_NAME)                                           \
class NCBI_GUIOBJUTILS_EXPORT CL_NAME : public IEditMacroFunction             \
{                                                                             \
public:                                                                       \
    CL_NAME(EScopeEnum func_scope) : IEditMacroFunction(func_scope) {}        \
    virtual void TheFunction();                                               \
    static const char* sm_FunctionName;                                       \
protected:                                                                    \
    virtual bool x_ValidArguments() const;                                    \
};                                                                            \


#define DECLARE_FUNC_CLASS_WITH_ARGS(CL_NAME)                                 \
class NCBI_GUIOBJUTILS_EXPORT CL_NAME : public IEditMacroFunction             \
{                                                                             \
public:                                                                       \
    CL_NAME(EScopeEnum func_scope) : IEditMacroFunction(func_scope) {}        \
    virtual void TheFunction();                                               \
    static CTempString GetFuncName();                                         \
    static const vector<SArgMetaData> sm_Arguments;                           \
protected:                                                                    \
    virtual bool x_ValidArguments() const;                                    \
};                                                                            \

// Use these two macros below when declaring new macro function classes
#define DECLARE_FUNC_CLASS_WITH_FNCNAME(CL_NAME)                              \
class NCBI_GUIOBJUTILS_EXPORT CL_NAME : public IEditMacroFunction             \
{                                                                             \
public:                                                                       \
    CL_NAME(EScopeEnum func_scope) : IEditMacroFunction(func_scope) {}        \
    virtual void TheFunction();                                               \
    static CTempString GetFuncName();                                         \
protected:                                                                    \
    virtual bool x_ValidArguments() const;                                    \
};                                                                            \

#define DEFINE_MACRO_FUNCNAME(CL_NAME, FN_NAME)                               \
CTempString CL_NAME::GetFuncName()                                            \
{                                                                             \
    static const char* s_funcname = FN_NAME;                                  \
    return s_funcname;                                                        \
}                                                                             \

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_TopLevel)

END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */

#endif  
    // GUI_OBJUTILS___MACRO_EDIT_FN_BASE__HPP
