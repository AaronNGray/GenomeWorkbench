#ifndef GUI_WIDGETS_SEQ___FASTA_RETRIEVE_JOB__HPP
#define GUI_WIDGETS_SEQ___FASTA_RETRIEVE_JOB__HPP

/*  $Id: fasta_retrieve_job.hpp 44629 2020-02-10 18:30:10Z asztalos $
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

#include <corelib/ncbiobj.hpp>
#include <corelib/ncbimtx.hpp>
#include <gui/gui_export.h>

#include <objmgr/seq_entry_handle.hpp>

#include <gui/widgets/seq/text_retrieve_job.hpp>

BEGIN_NCBI_SCOPE

class ITextItem;

BEGIN_SCOPE(objects)
    class CBioseq_Handle;
END_SCOPE(objects)

class NCBI_GUIWIDGETS_SEQ_EXPORT CFastaRetrieveJob :
        public CTextRetrieveJob
{
public:
    CFastaRetrieveJob(CTextPanelContext& context,
                      const vector<pair<objects::CBioseq_Handle, string> >& handles);
    virtual ~CFastaRetrieveJob();

    /// @name IAppJob implementation
    /// @{
    virtual string GetDescr() const { return "Retrieve FastA data for text view"; }
    /// @}

protected:
    virtual string x_GetJobName() const { return "FastaRetrieveJob"; }
    virtual EJobState x_Run();

private:
    void x_DumpBioseq(const objects::CBioseq_Handle& h);

    vector<pair<objects::CBioseq_Handle, string> > m_Handles;
};

class NCBI_GUIWIDGETS_SEQ_EXPORT CFastaRetrieveLocJob :
        public CTextRetrieveJob
{
public:
    CFastaRetrieveLocJob(CTextPanelContext& context, const objects::CSeq_loc& loc, objects::CScope& scope);
    virtual ~CFastaRetrieveLocJob();

    /// @name IAppJob implementation
    /// @{
    virtual string GetDescr() const { return "Retrieve FastA data for text view"; }
    /// @}

protected:
    virtual string x_GetJobName() const { return "FastaRetrieveLocJob"; }
    virtual EJobState x_Run();

private:
    CConstRef<objects::CSeq_loc> m_Loc;
    CRef<objects::CScope> m_Scope;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ___FASTA_RETRIEVE_JOB__HPP
