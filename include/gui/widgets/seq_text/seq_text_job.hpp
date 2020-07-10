#ifndef GUI_WIDGETS_SEQ_TEXT___SEQ_TEXT_JOB__HPP
#define GUI_WIDGETS_SEQ_TEXT___SEQ_TEXT_JOB__HPP

/*  $Id: seq_text_job.hpp 26649 2012-10-18 20:31:07Z katargir $
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
 * Authors:  Colleen Bollin, based on a file by Vlad Lebedev
 *
 * File Description:
 * This file contains the class declaration for the CSeqTextJob class,
 * which will be used to load and format data for the Sequence Text View widget
 * in a background thread.
 *
 */



#include <gui/gui.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objmgr/seq_vector.hpp>
#include <gui/gui_export.h>

#include <gui/utils/app_job.hpp>
#include <gui/utils/app_job_impl.hpp>
#include <gui/widgets/seq_text/seq_text_defs.hpp>

BEGIN_NCBI_SCOPE


class CSeqTextJobResult;

/// CSeqTextJobResult
class NCBI_GUIWIDGETS_SEQTEXT_EXPORT CSeqTextJobResult : public CObject
{
public:

    CSeqTextJobResult() {}
    typedef vector<bool> TAvailableSubtypeVector;

    virtual ~CSeqTextJobResult()  {};

    size_t size() { return m_AvailableSubtypes.size(); }

    objects::SAnnotSelector m_Sel;

    TAvailableSubtypeVector m_AvailableSubtypes;
};


///////////////////////////////////////////////////////////////////////////////
/// CSeqTextJob
// Job will load data from ObjectManager for a specific layer in the layout
//
class NCBI_GUIWIDGETS_SEQTEXT_EXPORT CSeqTextJob : public CJobCancelable
{
public:
    enum EJobTask {  // Data loading tasks
        eLoadFeatureTypesTask
    };

    typedef EJobTask  TJobTask;
    typedef int       TSelectorResolveDepth;

    CSeqTextJob (const objects::CSeq_loc& loc,
                 objects::CScope& scope,
                 TJobTask task);

    /// @name IAppJob implementation
    /// @{
    virtual EJobState                   Run();
    virtual CConstIRef<IAppJobProgress> GetProgress();
    virtual CRef<CObject>               GetResult();
    virtual CConstIRef<IAppJobError>    GetError();
    virtual string                      GetDescr() const;
    /// @}

protected:
    // levels for data retrieval
    void x_LoadFeatureTypes();

    const objects::CSeq_loc& m_Loc;
    objects::CScope&         m_Scope;
    objects::SAnnotSelector  m_Sel;            // our selector
    TJobTask                 m_TaskID;         // what is this job for

    // for job results/status
    CRef<CAppJobProgress>   m_Progress;
    CRef<CAppJobError>      m_Error;
    CRef<CSeqTextJobResult> m_Result;


};



END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_TEXT___SEQ_TEXT_JOB__HPP
