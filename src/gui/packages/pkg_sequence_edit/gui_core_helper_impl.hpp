#ifndef __GUI_CORE_HELPER_IMPL__HPP
#define __GUI_CORE_HELPER_IMPL__HPP

/*  $Id: gui_core_helper_impl.hpp 42902 2019-04-25 15:23:56Z filippov $
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
 * Authors: Igor Filippov
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>
#include <gui/framework/workbench_impl.hpp>
#include <gui/core/selection_service_impl.hpp>
#include <gui/core/project_service.hpp>
#include <gui/objects/GBWorkspace.hpp>
#include <gui/objects/WorkspaceFolder.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/widgets/edit/gui_core_helper.hpp>

BEGIN_NCBI_SCOPE

class CGuiCoreHelper : public IGuiCoreHelper
{

public:
    CGuiCoreHelper(IWorkbench*  wb) : IGuiCoreHelper(wb) {}
    virtual ~CGuiCoreHelper() {}
    virtual void Sync() 
    {
        IGuiCoreHelper::Sync();
        TConstScopedObjects objects;
        CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
        if (!sel_srv) 
            return;
        sel_srv->GetActiveObjects(objects);
        if (objects.empty()) 
        {
            GetViewObjects(m_Workbench, objects);
        }

        if (objects.empty()) 
            return;

        NON_CONST_ITERATE (TConstScopedObjects, it, objects) 
        {
            const objects::CSeq_submit* sub = dynamic_cast<const objects::CSeq_submit*>((*it).object.GetPointer());
            if (sub) 
            {
                m_SeqSubmit.Reset(sub);
            }
            if (!m_TopSeqEntry) 
            {
                objects::CSeq_entry_Handle seh = GetTopSeqEntryFromScopedObject(*it);
                if (seh) 
                {
                    m_TopSeqEntry = seh;
                    if (m_SeqSubmit) 
                    {
                        break;
                    }
                }
            }
        }
        if (!m_TopSeqEntry) 
            return;

        CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
        if (!srv)
            return;

        CRef<objects::CGBWorkspace> ws = srv->GetGBWorkspace();
        if (!ws) 
            return;

        CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(m_TopSeqEntry.GetScope()));
        if (!doc)
            return;
        m_CmdProcessor = &doc->GetUndoManager(); 
    }       
};


END_NCBI_SCOPE

#endif  // __GUI_CORE_HELPER_IMPL__HPP
