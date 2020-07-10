#ifndef GUI_WIDGETS_EDIT___GUI_CORE_HELPER__HPP
#define GUI_WIDGETS_EDIT___GUI_CORE_HELPER__HPP

/*  $Id: gui_core_helper.hpp 42902 2019-04-25 15:23:56Z filippov $
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
#include <gui/gui_export.h>
#include <objects/submit/Seq_submit.hpp>
#include <gui/framework/workbench.hpp>


BEGIN_NCBI_SCOPE

class ICommandProccessor;

class NCBI_GUIWIDGETS_EDIT_EXPORT IGuiCoreHelper : public CObject
{
public:   
    IGuiCoreHelper(IWorkbench*  wb) : m_Workbench(wb), m_CmdProcessor(NULL) {}
    virtual ~IGuiCoreHelper() {}
    virtual void Sync() 
    {
        m_TopSeqEntry.Reset();
        m_SeqSubmit.Reset();
        m_CmdProcessor = NULL;
    }
 
    objects::CSeq_entry_Handle GetTopSeqEntry() {return m_TopSeqEntry;}
    CConstRef< objects::CSeq_submit> GetSeqSubmit() {return m_SeqSubmit;}
    ICommandProccessor* GetCmdProcessor() {return m_CmdProcessor;}
protected:
    IWorkbench* m_Workbench;
    objects::CSeq_entry_Handle m_TopSeqEntry;
    CConstRef< objects::CSeq_submit> m_SeqSubmit;
    ICommandProccessor* m_CmdProcessor;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___GUI_CORE_HELPER__HPP
