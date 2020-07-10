#ifndef GUI_WIDGETS_ALN_TABLE__ANNOT_COMPARE_DS__HPP
#define GUI_WIDGETS_ALN_TABLE__ANNOT_COMPARE_DS__HPP

/*  $Id: annot_compare_ds.hpp 27635 2013-03-15 19:33:00Z katargir@NCBI.NLM.NIH.GOV $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *
 */

#include <gui/gui_export.h>
#include <objmgr/scope.hpp>
#include <objects/seqalign/Seq_align.hpp>
#include <objects/seqalign/Score.hpp>
#include <gui/widgets/wx/table_model.hpp>

#include <gui/utils/app_job_dispatcher.hpp>
//#include <gui/utils/thread_pool_engine.hpp>
//#include <gui/objutils/object_manager_engine.hpp>

// #include <gui/objutils/app_job_dispatcher.hpp>
#include <algo/sequence/annot_compare.hpp>

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
    class CSeq_annot;
    class CSeq_align;
    class CSeq_align_set;
END_SCOPE(objects)


///////////////////////////////////////////////////////////////////////////////
/// CAnnotCompareDS
class NCBI_GUIWIDGETS_ALNTABLE_EXPORT CAnnotCompareDS
    : public CObjectEx,
      public CEventHandler,
      public CwxAbstractTableModel
{
public:
    struct SRow
    {
        size_t row_idx;
        mutable CRef<objects::CScope> scope;

        /// we store a pair of features and their locations
        /// the location is stored separately because if a feature is
        /// missing, we can at least show its presumptive mapped location
        CConstRef<objects::CSeq_feat> feat1;
        CConstRef<objects::CSeq_feat> feat2;

        CConstRef<objects::CSeq_loc> feat1_loc;
        CConstRef<objects::CSeq_loc> feat2_loc;

        objects::CSeq_id_Handle feat1_seq;
        objects::CSeq_id_Handle feat2_seq;

        /// labels - stored and provided for future reference
        mutable string feat1_label;
        mutable string feat2_label;
        mutable string feat1_loc_label;
        mutable string feat2_loc_label;
        mutable string feat1_seq_label;
        mutable string feat2_seq_label;
        mutable string feat_type_label;

        objects::CAnnotCompare::TCompareFlags loc_state;
    };

    typedef vector<SRow> TRows;

public:
    CAnnotCompareDS(){}
    CAnnotCompareDS(objects::CScope& scope);

   // virtual void Update() = 0;
    void Clear();

    objects::CScope& GetScope() const;
    const TRows& GetData() const;
    size_t  GetRows(void) const;
    const SRow& GetRow(size_t row_idx) const;

    /// dump a text report of the comparisons
    void DumpTextReport(CNcbiOstream& ostr) const;

    /// register a listener to respond to background job completion
    /// events
    void RegisterListener(CEventHandler* listener);

    /// handle background job completion events
    void OnAJNotification(CEvent* evt);

    /// return progress indicator
    float GetProgress() const;

    void Update(){}

    /// access a given row's data
    //const SAlignment& GetData(size_t row) const;

    /// @name IwxTableModel implementation
    /// @{

    virtual int GetNumRows() const {return 0;};

    virtual int GetNumColumns() const {return 0;};

    virtual wxString GetColumnName( int /*aColIx*/ ) const {return wxString();}
    ///
    virtual wxString GetColumnType( int /*aColIx*/ ) const {return wxString();}
    ///
    virtual wxVariant GetValueAt( int /*i*/, int /*j*/ ) const {return wxVariant();}

protected:
    DECLARE_EVENT_MAP();

    mutable CRef<objects::CScope> m_Scope;

    TRows m_Rows; // row represents a pair of compared features

    /// control of background loading
    CAppJobDispatcher::TJobID m_ActiveJob;
    CEventHandler* m_Listener;

    void x_DeleteAllJobs();
    void x_BackgroundJob(IAppJob* job);

    virtual void x_OnAppJobNotification(CEvent* evt);
};


///////////////////////////////////////////////////////////////////////////////
/// CAnnotCompare_AlignDS
class NCBI_GUIWIDGETS_ALNTABLE_EXPORT CAnnotCompare_AlignDS
    : public CAnnotCompareDS
{
public:
    CAnnotCompare_AlignDS(objects::CScope& scope,
                          const objects::CSeq_annot& annot);
    CAnnotCompare_AlignDS(objects::CScope& scope,
                          const objects::CSeq_align& align);
    CAnnotCompare_AlignDS(objects::CScope& scope,
                          const objects::CSeq_align_set& aligns);
    CAnnotCompare_AlignDS(objects::CScope& scope,
                          const list< CConstRef<objects::CSeq_align> >& aligns);
    ~CAnnotCompare_AlignDS();

    void Update();

private:
    void x_Init();

private:
    /// our original alignments
    list< CConstRef<objects::CSeq_align> > m_Alignments;

    /// merged alignment from the above
    CConstRef<objects::CSeq_align> m_Align;

    /// forbidden
    CAnnotCompare_AlignDS(const CAnnotCompare_AlignDS&);
    CAnnotCompare_AlignDS& operator=(const CAnnotCompare_AlignDS&);
};


///////////////////////////////////////////////////////////////////////////////
/// CAnnotCompare_LocationDS
class NCBI_GUIWIDGETS_ALNTABLE_EXPORT CAnnotCompare_LocationDS
    : public CAnnotCompareDS
{
public:
    CAnnotCompare_LocationDS(objects::CScope& scope,
                             const objects::CSeq_loc& loc);
    ~CAnnotCompare_LocationDS();

    void Update();

private:
    CConstRef<objects::CSeq_loc> m_Loc; /// the set of locations we compare.

    /// forbidden
    CAnnotCompare_LocationDS(const CAnnotCompare_LocationDS&);
    CAnnotCompare_LocationDS& operator=(const CAnnotCompare_LocationDS&);
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_ALN_TABLE__ANNOT_COMPARE_DS__HPP
