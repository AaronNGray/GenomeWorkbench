#ifndef GUI_WIDGETS_SEQ___FLATFILE_SEQUENCE_LIST_JOB__HPP
#define GUI_WIDGETS_SEQ___FLATFILE_SEQUENCE_LIST_JOB__HPP

/*  $Id: flat_file_sequence_list_job.hpp 44629 2020-02-10 18:30:10Z asztalos $
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

#include <gui/utils/app_job_impl.hpp>

#include <objtools/format/flat_file_config.hpp>
#include <objtools/format/context.hpp>
#include <objtools/format/item_ostream.hpp>
#include <objtools/format/items/item.hpp>

#include <gui/widgets/seq/text_panel.hpp>

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
    class CSeq_loc;
    class CSubmit_block;
END_SCOPE(objects)

class CTextViewSequenceListResult : public CObject
{
public:
    CTextViewSequenceListResult(const vector<CFlatFileSeq>& sequenceList)
        : m_SequenceList(sequenceList) {}

    virtual ~CTextViewSequenceListResult() {}

    const vector<CFlatFileSeq>& GetData()
    {
        return m_SequenceList;
    }

private:
    CTextViewSequenceListResult(const CTextViewSequenceListResult&);
    CTextViewSequenceListResult& operator=(const CTextViewSequenceListResult&);

    vector<CFlatFileSeq> m_SequenceList;
};


class CFlatFileSequenceListJob : public CJobCancelable
{
public:
    CFlatFileSequenceListJob(
        objects::CSeq_entry_Handle& h,
        const objects::CSubmit_block* submitBlock,
        const objects::CSeq_loc* seq_loc,
        objects::CFlatFileConfig::EStyle style);

    virtual ~CFlatFileSequenceListJob();

    /// @name IAppJob implementation
    /// @{
    virtual EJobState                   Run();
    virtual CConstIRef<IAppJobProgress> GetProgress();
    virtual CRef<CObject>               GetResult();
    virtual CConstIRef<IAppJobError>    GetError();
    /// @}


    /// @name IAppJob implementation
    /// @{
    virtual string GetDescr() const { return "Retrieve list of sequences for text view"; }
    /// @}

private:
    virtual string x_GetJobName() const { return "FlatFileSequenceListJob"; }
    EJobState x_Run();

    objects::CFlatFileConfig        m_Config;
    CRef<objects::CFlatFileContext> m_Context;

    vector<CFlatFileSeq>    m_Seqs;

    // for job results/status
    CRef<CTextViewSequenceListResult> m_Result;
    CRef<CAppJobError>                m_Error;

    class CCancelException
    {
    };

    class CFlatFileSeqBuilder : public objects::CFlatItemOStream
    {
    public:
        CFlatFileSeqBuilder(vector<CFlatFileSeq>& seqs, ICanceled& canceled)
            : m_Seqs(seqs), m_Canceled(canceled) {}
        ~CFlatFileSeqBuilder() {}

    private:
        virtual void AddItem(CConstRef<objects::IFlatItem> item);

        vector<CFlatFileSeq>& m_Seqs;
        ICanceled&            m_Canceled;
    };
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ___FLATFILE_SEQUENCE_LIST_JOB__HPP
