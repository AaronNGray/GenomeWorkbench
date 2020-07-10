/* $Id: seq_grid_table_navigator.cpp 38175 2017-04-06 14:52:27Z filippov $
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
 * Authors:  Igor Filippov
 */
#include <ncbi_pch.hpp>
#include <gui/widgets/seq/flat_file_ctrl.hpp>
#include <gui/core/project_service.hpp>
#include <gui/core/simple_project_view.hpp>
#include <gui/packages/pkg_sequence_edit/seq_grid_table_navigator.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects); 


void CSeqGridTableNav::NavigateToTextViewPosition(TConstScopedObjects &objects, CBioseq_Handle bsh, const CObject *obj,  IWorkbench* workbench)
{
    if (!workbench)
        return;
    CIRef<IFlatFileCtrl> FlatFileCtrl;
    CIRef<CProjectService> prjSrv = workbench->GetServiceByType<CProjectService>();
    CIRef<IProjectView> pTextView(prjSrv->FindView(*(objects.front().object), "Text View"));
    FlatFileCtrl.Reset(dynamic_cast<IFlatFileCtrl*>(pTextView.GetPointerOrNull()));
    if(FlatFileCtrl.NotEmpty()) {
        FlatFileCtrl->SetPosition(bsh, obj);        
    }
}

void CSeqGridTableNav::JumpToTextView(int row)
{
    TConstScopedObjects objects;
    CBioseq_Handle bsh;
    const CObject* obj =  RowToScopedObjects(row, objects, bsh);
    NavigateToTextViewPosition(objects, bsh, obj, this->m_Workbench);
}

END_NCBI_SCOPE

