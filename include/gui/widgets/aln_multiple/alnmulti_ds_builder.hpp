#ifndef GUI_WIDGETS_ALN_MULTIPLE___ALNMULTI_DS_BUILDER__HPP
#define GUI_WIDGETS_ALN_MULTIPLE___ALNMULTI_DS_BUILDER__HPP

/*  $Id: alnmulti_ds_builder.hpp 44958 2020-04-28 18:04:11Z shkeda $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <gui/widgets/aln_multiple/align_ds.hpp>
#include <gui/widgets/aln_multiple/alnmulti_ds.hpp>

#include <gui/objutils/ui_data_source_notif.hpp>

#include <objects/seqalign/Seq_align.hpp>
#include <objects/seqalign/Std_seg.hpp>

#include <objtools/alnmgr/aln_tests.hpp>
#include <objtools/alnmgr/aln_user_options.hpp>
#include <objtools/alnmgr/aln_stats.hpp>
#include <objtools/alnmgr/seqids_extractor.hpp>


BEGIN_SCOPE(objects)
    class CScope;
    class CSeq_annot;
    class CSeq_align;
    class CBioseq_handle;
END_SCOPE(objects)

BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
///
class NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT CAlnMultiDSBuilder
{
public:
    CAlnSeqId* aaa;

    typedef vector< CConstRef<objects::CSeq_annot> >  TAnnotVector;
    typedef vector< CConstRef<objects::CSeq_align> >  TAlignVector;
    typedef vector<const objects::CSeq_align*> TAlnVector;
    typedef CAlnUserOptions TOptions;

    CAlnMultiDSBuilder();
    virtual ~CAlnMultiDSBuilder();

    /// initial data set from which an alignment will be build
    void    Init(objects::CScope& scope, const objects::CSeq_align& align);
    void    Init(objects::CScope& scope, const objects::CSeq_annot& annot);
    void    Init(objects::CScope& scope, const objects::CBioseq_Handle& handle);
    void    Init(objects::CScope& scope, const objects::CBioseq& bioseq);
    void    Init(objects::CScope& scope, const objects::CSeq_entry& seq_entry);
    void    Init(objects::CScope& scope, TAlignVector& aligns);
    void    Init(objects::CScope& scope, TAnnotVector& annots);

    void    PreCreateDataSource(bool sparse);
    CRef<IAlnMultiDataSource>   CreateDataSource();

    TOptions&    SetOptions() {   return m_Options;    }
    void    GetBioseqHandles(vector<CBioseq_Handle>& handles);

    bool GetSyncCreate() { return m_SyncCreate; }
    void SetSyncCreate( bool sync ) { m_SyncCreate = sync; }

    void SetSelectAnchor( bool select_anchor ) { m_SelectAnchor = select_anchor; }

    /// Subscribe a new listener for data source.
    /// No ownership transfer for pListener
    virtual void    SetListener(CDataChangeNotifier::IListener* pListener)
        { m_Listener = pListener; }
    virtual CDataChangeNotifier::IListener*  GetListener()
        { return m_Listener; }

protected:

    void    x_Clear();
    void    x_TestAlignments();

    void    x_GetLinearAlignments(TAlignVector& aligns);
    bool    x_IsLinear(const objects::CStd_seg& seg);

    void    x_PreCreateSparseDataSource();
    CRef<IAlnMultiDataSource> x_CreateSparseDataSource();
    CRef<IAlnMultiDataSource> x_CreateAlnVecDataSource();

protected:
    CAlnSeqId* d;

    typedef CAlnSeqIdsExtract<CAlnSeqId> TIdExtract;
    typedef vector<CRef<CAnchoredAln> > TAnchoredAlnVector;
    typedef CAlnIdMap<TAlnVector, TIdExtract> TAlnIdMap;
    typedef CAlnStats<TAlnIdMap> TAlnStats;

    CRef<objects::CScope> m_Scope;
    TAlignVector m_OrigAligns;
    CRef<TAlnStats> m_AlnStats;
    TAnchoredAlnVector m_AnchoredAlns;

    CConstRef<objects::CSeq_id> m_MasterId;
    TOptions m_Options; /// control alignment building
    bool m_CreateSparse;
    bool m_SyncCreate;
    bool m_SelectAnchor = false; /// Try to set anchor automatically
    CDataChangeNotifier::IListener *m_Listener;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_ALN_MULTIPLE___ALNMULTI_DS_BUILDER__HPP
