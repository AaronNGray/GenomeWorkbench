/* $Id: cmd_change_graph.cpp 27476 2013-02-22 11:02:38Z choi $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Roman Katargin, Yoon Choi
 */


#include <ncbi_pch.hpp>
#include <gui/objutils/cmd_change_graph.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CCmdChangeGraph::CCmdChangeGraph(const CSeq_graph_Handle& gh, const CSeq_graph& new_graph)
    : m_GH(gh)
{ 
    m_Graph.Reset(new CSeq_graph());
    m_Graph->Assign(new_graph);
}

void CCmdChangeGraph::Execute()
{
    CRef<CSeq_graph> saveGraph(new CSeq_graph());
    saveGraph->Assign(*m_GH.GetSeq_graph());

    m_GH.Replace(*m_Graph);
    m_Graph = saveGraph;
}

void CCmdChangeGraph::Unexecute()
{
    Execute();
}

string CCmdChangeGraph::GetLabel()
{
    return "Modify Seq-graph";
}

END_NCBI_SCOPE
