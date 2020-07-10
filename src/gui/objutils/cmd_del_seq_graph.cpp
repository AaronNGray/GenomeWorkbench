/*  $Id: cmd_del_seq_graph.cpp 29927 2014-03-13 20:16:12Z filippov $
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
 * Authors:  Roman Katargin, Igor Filippov
 */


#include <ncbi_pch.hpp>

#include <gui/objutils/cmd_del_seq_graph.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

void CCmdDelSeq_graph::Execute()
{
    m_Parent_handle =  m_Orig.GetAnnot();
    m_Saved.Reset();
    m_Saved = m_Orig.GetSeq_graph();
    m_Orig.Remove();
}

void CCmdDelSeq_graph::Unexecute()
{
    if (m_Saved && m_Parent_handle) 
    {
        CSeq_annot_EditHandle feh(m_Parent_handle);
        m_Orig = feh.AddGraph(*m_Saved);
        m_Saved.Reset();
    }
}

string CCmdDelSeq_graph::GetLabel()
{
    return "Delete graph";
}

END_NCBI_SCOPE
