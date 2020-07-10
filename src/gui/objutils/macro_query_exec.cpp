/*  $Id: macro_query_exec.cpp 39115 2017-08-01 18:58:09Z asztalos $
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
 * Authors:  Bob Falk
 *
 * File Description: 
 *
 */

#include <ncbi_pch.hpp>
#include <gui/objutils/macro_query_exec.hpp>


BEGIN_NCBI_SCOPE

CRef<CQueryNodeValue> CMacroQueryExec::GetOrCreateRTVar(const string& name)
{
    if (name.empty())
        return CRef<CQueryNodeValue>();

    auto rt_var = x_LocateRTVar( name );
    if (rt_var) {
        return rt_var;
    }

    CRef<CQueryNodeValue> node(new CQueryNodeValue());
    m_RTVars[name] = node;
    return node;
}

bool CMacroQueryExec::ResolveRTVar(const string& identifier, CQueryNodeValue& v) //, const CQueryParseTree::TNode* parent)
{
    CRef<CQueryNodeValue> rt_var = x_LocateRTVar(identifier);
    if (rt_var) {
        v.SetRef(rt_var);
        return true;
    }
    return false;
}

CRef<CQueryNodeValue> CMacroQueryExec::x_LocateRTVar(const string& identifier)
{
    auto it = m_RTVars.find(identifier);
    if (it != m_RTVars.end()) {
        return (*it).second;
    }

    return CRef<CQueryNodeValue>();
}

END_NCBI_SCOPE

