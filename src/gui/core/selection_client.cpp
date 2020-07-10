/*  $Id: selection_client.cpp 38835 2017-06-23 18:15:53Z katargir $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/core/selection_client.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/objutils/obj_event.hpp>

BEGIN_NCBI_SCOPE

void CSelectionClient::SetSelectionService(ISelectionService* service)
{
    m_SelectionService = service;
}

void CSelectionClient::SetSelectedObjects(TConstScopedObjects& objs)
{
    m_Selection = objs;

    if (m_SelectionService)
        m_SelectionService->OnSelectionChanged(this);
}

void CSelectionClient::GetSelection(TConstScopedObjects& objs) const
{
    objs = m_Selection;
}

void CSelectionClient::GetSelection(CSelectionEvent& evt) const
{
    TConstObjects objs;
    for (const auto& o : m_Selection)
        objs.push_back(o.object);

    if (!objs.empty())
        evt.AddObjectSelection(objs);
}

void  CSelectionClient::BroadcastSelection()
{
    if (m_Selection.empty() || !m_SelectionService)
        return;

    objects::CScope* scope = m_Selection.front().scope;
    if (scope) {
        CSelectionEvent sel_evt(*scope);
        GetSelection(sel_evt);
        m_SelectionService->Broadcast(sel_evt, this);
    }
}


END_NCBI_SCOPE
