#ifndef GUI_OBJUTILS___MACRO_FN_BASE__HPP
#define GUI_OBJUTILS___MACRO_FN_BASE__HPP
/*  $Id: macro_fn_base.hpp 41447 2018-07-31 16:20:16Z asztalos $
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
 * Authors: Anatoly Osipov, Andrea Asztalos, Robert Falk
 *
 * File Description: Interface class for macro function implementation
 *
 */

/// @file macro_fn_base.hpp
/// Interface class for macro function implementation

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>
#include <util/qparse/query_parse.hpp>
#include <gui/objutils/macro_cmd_composite.hpp>

/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(macro)

//////////////////////////////////////////////////////////////////
/// Abstract base class for any user function which can be 
/// called from either the WHERE or DO clauses
///
template<typename TData>
class IMacroFunction
{
public:
    enum EScopeEnum {
        eWhere,
        eDo,
        eBoth
    };

    enum ENestedFunc {
        eNotNested,
        eNested
    };

    /// Constructor
    IMacroFunction(EScopeEnum func_scope) 
        : m_FuncScope (func_scope)
        , m_Nested(eNotNested) {}

    /// Virtual destructor
    virtual ~IMacroFunction() {};

    /// Function implementation
    virtual void TheFunction() = 0;

    /// Function operator 
    void operator()(CMacroCmdComposite& cmd_composite,
        TData& data,
        CQueryParseTree::TNode& qnode)
    {
        m_CmdComposite = Ref(&cmd_composite);
        m_Report.clear();

        x_SetUserObjects(qnode);
        x_InitCall(data);

        TheFunction();
    }
    
    EScopeEnum  GetFuncScope()   { return m_FuncScope; }
    ENestedFunc GetNestedState() { return m_Nested;  }

    /// Function extracts statistic from the object
    string GetFuncReport() { return m_Report; } 

    void SetNestedState(ENestedFunc type) { m_Nested = type; }

private:
    EScopeEnum m_FuncScope;

protected:
    virtual void x_InitCall(TData& data) = 0;

    virtual void x_SetUserObjects(CQueryParseTree::TNode& qnode) = 0;

    void x_LogFunction(CNcbiOstrstream& logstr)
    {
        if (!IsOssEmpty(logstr)) {
            logstr << "\n";
            m_Report.assign(CNcbiOstrstreamToString(logstr));
        }
    }

    CRef<CMacroCmdComposite> m_CmdComposite;

    /// function activity report
    string m_Report;
    /// flag indicating whether the function is nested within another function
    ENestedFunc m_Nested;
};


END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */

#endif  // GUI_OBJUTILS___MACRO_FN_BASE__HPP
