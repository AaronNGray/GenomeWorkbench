#ifndef GUI_WIDGETS_SEQ___FASTA_DS__HPP
#define GUI_WIDGETS_SEQ___FASTA_DS__HPP

/*  $Id: text_ds.hpp 39645 2017-10-23 19:42:27Z katargir $
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
#include <gui/gui_export.h>

#include <gui/utils/app_job_dispatcher.hpp>

#include <gui/widgets/text_widget/root_text_item.hpp>
#include <gui/widgets/seq/text_panel.hpp>

BEGIN_NCBI_SCOPE

struct SFlatFileParams;
class CJobCancelable;

BEGIN_SCOPE(objects)

class CSeq_feat;
    class CSeq_loc;
	class CSubmit_block;
END_SCOPE(objects)

/// Class to deliver partial results as part of progress report of text viewer
///
class CTextViewRetrieveResult : public CObject
{
public:
    CTextViewRetrieveResult() {}
    virtual ~CTextViewRetrieveResult() {}

    void SetData(auto_ptr<CRootTextItem> mainItem, auto_ptr<CTextPanelContext> context)
    {
        m_MainItem = mainItem;
        m_Context = context;
    }

    void GetData(auto_ptr<CRootTextItem>& mainItem, auto_ptr<CTextPanelContext>& context)
    {
        mainItem = m_MainItem;
        context = m_Context;
    }

private:
    CTextViewRetrieveResult(const CTextViewRetrieveResult&);
    CTextViewRetrieveResult& operator=(const CTextViewRetrieveResult&);

    auto_ptr<CRootTextItem>     m_MainItem;
    auto_ptr<CTextPanelContext> m_Context;
};

class NCBI_GUIWIDGETS_SEQ_EXPORT CTextDataSource :
        public CObjectEx, 
        public CEventHandler
{
    DECLARE_EVENT_MAP();
public:
    CTextDataSource(CTextPanel& textPanel);
    virtual ~CTextDataSource();

    void LoadFasta(CTextPanelContext* context,
                   const vector<pair<objects::CBioseq_Handle, string> >& handles);
    void LoadFasta(CTextPanelContext* context, const objects::CSeq_loc& loc, objects::CScope& scope);
    void LoadFlatFile(
        CTextPanelContext* context,
        const vector<pair<objects::CBioseq_Handle, string> >& handles,
        const SFlatFileParams& params);

    void GetFlatFileSeqs(
        objects::CSeq_entry_Handle& h,
        const objects::CSubmit_block* submitBlock,
        const objects::CSeq_loc* seq_loc,
        objects::CFlatFileConfig::EStyle style);

    void LoadAsn(CTextPanelContext* context, const CSerialObject& so, objects::CScope& scope);
    void LoadXml(CTextPanelContext* context, const CSerialObject& so, objects::CScope& scope);

    void LoadFlatFile(CTextPanelContext* context,
                      objects::CScope& scope,
                      const objects::CSeq_feat& seq_feat,
                      bool getGeneModel,
                      objects::CFlatFileConfig::EMode mode,
                      objects::CFlatFileConfig::EStyle style);

    bool IsLoading() const;

    /// CAppJobDispatcher notification handler
    void OnJobNotification(CEvent* evt);

private:
    void x_CancelJob();
    void x_SetDataLocker(CJobCancelable& job);

    CTextPanel& m_TextPanel;

    CAppJobDispatcher::TJobID m_JobId;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ___FASTA_DS__HPP
