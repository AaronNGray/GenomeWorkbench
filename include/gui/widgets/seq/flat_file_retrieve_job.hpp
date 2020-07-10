#ifndef GUI_WIDGETS_SEQ___FLATFILE_RETRIEVE_JOB__HPP
#define GUI_WIDGETS_SEQ___FLATFILE_RETRIEVE_JOB__HPP

/*  $Id: flat_file_retrieve_job.hpp 44629 2020-02-10 18:30:10Z asztalos $
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

#include <objtools/format/flat_file_config.hpp>

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
    class CFlatGatherer;
    class CFlatFileContext;
END_SCOPE(objects)

class ITextItem;

struct SFlatFileParams
{
    SFlatFileParams() : m_HideVariations(false), m_HideSTS(false) {}
    SFlatFileParams(bool hideVariations, bool hideSTS) : 
        m_Mode(objects::CFlatFileConfig::eMode_GBench),
        m_Style(objects::CFlatFileConfig::eStyle_Normal),
        m_HideVariations(hideVariations),
        m_HideSTS(hideSTS),
        m_ShowComponentFeats(false) {}

    CConstRef<objects::CSubmit_block> m_SeqSubmit;
    CConstRef<objects::CSeq_loc> m_SeqLoc;
    objects::CFlatFileConfig::EMode m_Mode;
    objects::CFlatFileConfig::EStyle m_Style;

    bool m_HideVariations;
    bool m_HideSTS;
    bool m_ShowComponentFeats;
};

class NCBI_GUIWIDGETS_SEQ_EXPORT CFlatFileRetrieveJob :
        public CTextRetrieveJob
{
public:
    CFlatFileRetrieveJob(
        CTextPanelContext& context,
        const vector<pair<objects::CBioseq_Handle, string> >& handles,
        const SFlatFileParams& params);

    virtual ~CFlatFileRetrieveJob();

    /// @name IAppJob implementation
    /// @{
    virtual string GetDescr() const { return "Retrieve FlatFile data for text view"; }
    /// @}

protected:
    virtual string x_GetJobName() const { return "FlatFileRetrieveJob"; }
    virtual EJobState x_Run();

private:
friend class CFlatFileFactoryBuilder;

    vector<pair<objects::CBioseq_Handle, string> > m_Handles;
    SFlatFileParams m_Params;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ___FLATFILE_RETRIEVE_JOB__HPP
