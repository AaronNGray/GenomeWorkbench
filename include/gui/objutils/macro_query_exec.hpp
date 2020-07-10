#ifndef GUI_OBJUTILS___MACRO_QUERY_EXEC__HPP
#define GUI_OBJUTILS___MACRO_QUERY_EXEC__HPP

/*  $Id: macro_query_exec.hpp 39115 2017-08-01 18:58:09Z asztalos $
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
* Authors: Bob Falk
*
* File Description:
* Header file for classes needed to implement query execution.
*
*/

#include <objmgr/scope.hpp>

#include <util/qparse/query_exec.hpp>
#include <gui/objutils/macro_rep.hpp>
#include <gui/objutils/macro_parse.hpp>
#include <gui/objutils/query_node_value.hpp>

BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
/// class CMacroQueryExec
///
///
///
class NCBI_GUIOBJUTILS_EXPORT CMacroQueryExec : public CQueryExec
{
public:
    CMacroQueryExec() {}
    virtual ~CMacroQueryExec() {}

    virtual bool EvalNext(macro::CMacroRep& /* macro */) { return false; }
    virtual bool EvalNext(CQueryParseTree& /* qtree */) { return false; }

    // Add the names of the 'do' and 'where' functions to parser so that
    // it will recognize supported functions when parsing
    virtual void GetFunctionNames(macro::CMacroParser& parser) const {}

    void SetMacroRep(macro::CMacroRep* mr) { m_MacroRep = mr; }
    macro::CMacroRep* GetMacroRep() { return m_MacroRep; }

    /// Gets or creates run-time vars (used in assignment in Do clause)
    CRef<CQueryNodeValue> GetOrCreateRTVar(const string& name);

    /// Return the value of RT variable in node "v"
    bool ResolveRTVar(const string& identifier, CQueryNodeValue& v);

protected:
    /// Gets the pointer to RT variable
    CRef<CQueryNodeValue> x_LocateRTVar(const string& identifier);

    macro::CMacroRep* m_MacroRep;

    typedef map< string, CRef<CQueryNodeValue> > TRTVarsMap;
    TRTVarsMap m_RTVars;
};



END_NCBI_SCOPE


#endif  // GUI_OBJUTILS___MACRO_QUERY_EXEC__HPP

