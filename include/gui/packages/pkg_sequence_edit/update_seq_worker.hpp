#ifndef _GUI_PACKAGES_UPDATE_SEQ_WORKER_HPP_
#define _GUI_PACKAGES_UPDATE_SEQ_WORKER_HPP_
/*  $Id: update_seq_worker.hpp 44179 2019-11-12 20:55:18Z asztalos $
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
 * Authors:  Andrea Asztalos
 *
 *  Worker class to setup update sequence dialog and launch the updater
 */

#include <corelib/ncbistd.hpp>
#include <gui/framework/workbench.hpp>
#include <gui/utils/command_processor.hpp>
#include <gui/gui_export.h>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
    class CSeq_entry_Handle;
    class ILineErrorListener;
END_SCOPE(objects);

class CUpdateSeq_Input;
class SUpdateSeqParams;

class NCBI_GUIPKG_SEQUENCE_EDIT_EXPORT CSeqUpdateWorker
{
public:
    enum ESeqUpdateType {
        eUpdateNotSet,
        eSingle_File,
        eSingle_Clipboard,
        eSingle_Accession,
        eMultiple_File,
        eMultiple_Clipboard
    };
    CSeqUpdateWorker();
    ~CSeqUpdateWorker() {}

    void SetSeqUpdateType(ESeqUpdateType update_type) { m_Type = update_type; }
    void UpdateSingleSequence(IWorkbench* workbench, const objects::CSeq_entry_Handle& tse, ICommandProccessor* cmdProcessor) const;
    void UpdateSingleSequence_Ext(IWorkbench* workbench, const objects::CSeq_entry_Handle& tse, ICommandProccessor* cmdProcessor) const;
    void UpdateMultipleSequences(IWorkbench* workbench, const objects::CSeq_entry_Handle& tse, ICommandProccessor* cmdProcessor);
private:
    void x_GetSeqsWithIgnoredResidues(objects::ILineErrorListener& msg_listener);
    void x_LaunchSingleUpdateSequence(CUpdateSeq_Input& updseq_in, SUpdateSeqParams& params, const objects::CSeq_entry_Handle& tse, ICommandProccessor* cmdProcessor) const;

    CSeqUpdateWorker(const CSeqUpdateWorker&);
    CSeqUpdateWorker& operator=(const CSeqUpdateWorker&);

    ESeqUpdateType m_Type;
};

END_NCBI_SCOPE

#endif
// _GUI_PACKAGES_UPDATE_SEQ_WORKER_HPP_
