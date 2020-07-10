/*  $Id: cmd_composite.cpp 44876 2020-04-02 20:05:56Z asztalos $
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
 * Authors:  Roman Katargin
 */


#include <ncbi_pch.hpp>

#include <gui/objutils/cmd_composite.hpp>

BEGIN_NCBI_SCOPE

void CCmdComposite::AddCommand(IEditCommand& command)
{
    m_CmdList.push_back(CIRef<IEditCommand>(&command));
}

bool CCmdComposite::IsEmpty()
{
    return m_CmdList.empty();
}

void CCmdComposite::Execute()
{
    TCmdList::iterator it;
    for (it = m_CmdList.begin(); it != m_CmdList.end(); ++it) {
        (*it)->Execute();
    }
}

void CCmdComposite::Unexecute()
{
    TCmdList::reverse_iterator it;
    for (it = m_CmdList.rbegin(); it != m_CmdList.rend(); ++it) {
        (*it)->Unexecute();
    }
}


END_NCBI_SCOPE
