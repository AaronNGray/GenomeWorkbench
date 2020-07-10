#ifndef GUI_WIDGETS_SEQ_TEXT___SEQ_TEXT_DS__HPP
#define GUI_WIDGETS_SEQ_TEXT___SEQ_TEXT_DS__HPP

/*  $Id: seq_text_ds.hpp 35180 2016-04-05 14:49:47Z asztalos $
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
 * Authors:  Colleen Bollin (adapted from a file by Andrey Yazhuk)
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>
#include <corelib/ncbimisc.hpp>

#include <gui/gui_export.h>
//#include <gui/opengl/glpane.hpp>

#include <objmgr/scope.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objmgr/seq_vector.hpp>
#include <objmgr/annot_selector.hpp>
#include <objmgr/feat_ci.hpp>

#include <objects/seq/Seq_annot.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqfeat/Seq_feat.hpp>

#include <serial/iterator.hpp>
#include <util/icanceled.hpp>

#include <gui/widgets/seq_text/seq_text_conf.hpp>
#include <gui/widgets/seq_text/seq_text_defs.hpp>
#include <gui/widgets/seq_text/seq_text_geometry.hpp>

BEGIN_NCBI_SCOPE

class CSeqTextDataSourceInterval
{
public:
    CSeqTextDataSourceInterval(const objects::CSeq_loc& loc, objects::CScope& scope, TSeqPos offset);
    ~CSeqTextDataSourceInterval();

    TSeqPos GetSourceStart() const { return m_Offset; };
    TSeqPos GetSourceStop() const { return m_Offset + m_Length - 1; };
    TSeqPos GetSequenceStart() const { return m_Loc->GetStart(objects::eExtreme_Positional); };
    TSeqPos GetSequenceStop() const { return m_Loc->GetStop(objects::eExtreme_Positional); };
    TSeqPos GetLength() const { return m_Loc->GetStop(objects::eExtreme_Positional) - m_Loc->GetStart(objects::eExtreme_Positional) + 1; };
    void GetSeqString(TSeqPos src_start, TSeqPos src_stop, string& buffer) const;
    void GetSeqData (TSeqPos src_start, TSeqPos src_stop, string& buffer, objects::SAnnotSelector* feat_sel/*objects::CSeqFeatData::ESubtype subtype*/) const;
    objects::ENa_strand GetStrand() const { return m_Loc->GetStrand(); };
    void GetToolTipForPosition(TSeqPos src_pos, string& tooltip_text);
    bool IntersectingSourceInterval(TSeqPos& src_start, TSeqPos& src_stop) const;
    bool SourcePosToIntervalPos(TSeqPos& pos) const;
    bool SequencePosToIntervalPos(TSeqPos& pos) const;
    void RenderFeatureExtras (ISeqTextGeometry* pParent, TSeqPos start_offset, TSeqPos stop_offset);
    void GetFeatureData
(TSeqPos start_offset,
 TSeqPos stop_offset,
 CSeqTextConfig *cfg,
 ISeqTextGeometry* pParent,
 CSeqTextDefs::TSubtypeVector &subtypes,
 CSeqTextDefs::TSpliceSiteVector &splice_sites,
 CSeqTextDefs::TVariationGraphVector &variations);

    void GetVariations (TSeqPos start_offset, TSeqPos stop_offset, CSeqTextDefs::TVariationGraphVector &variations);
    void GetSubtypesForAlternatingExons (TSeqPos start_offset, 
                                         TSeqPos stop_offset, 
                                         ISeqTextGeometry* pParent, 
                                         CSeqTextDefs::TSubtypeVector &subtypes, 
                                         bool& even);
     void GetFeaturesAtPosition(TSeqPos source_pos, vector<CConstRef<objects::CSeq_feat> >& features);
    objects::CBioseq_Handle GetSeq() {return m_Seq;}
protected:
    CRef<objects::CSeq_loc> m_Loc;
    objects::CFeat_CI m_Feat;
    TSeqPos m_Offset;
    TSeqPos m_Length;
    objects::CBioseq_Handle m_Seq;  
    objects::CSeqVector m_Vect;

    void x_AddVariationsFromFeature(const objects::CSeq_feat &feat, const objects::CSeq_loc &loc, CSeqTextDefs::TVariationGraphVector &variations) const;
    void LookForSpliceJunctions (const objects::CMappedFeat& feat, CSeqTextDefs::TSpliceSiteVector &splice_sites, TSeqPos start_offset, TSeqPos stop_offset) const;
    CRef<objects::CSeq_loc> x_GetSeqLocForInterval(TSeqPos interval_start, TSeqPos interval_stop) const;
    bool x_IsSpliceSite(string splice_buffer, bool before_loc, bool is_minus) const;
};

/// CSeqTextDataSource implements Adapter design pattern. It adapts given
/// CSeq_annot ot CSeq_align_set object by providing standard interface
/// representing data as plain list of pair-wise alignments.

class NCBI_GUIWIDGETS_SEQTEXT_EXPORT CSeqTextDataSource
    : public CObject
{
public:
    typedef CConstRef<objects::CSeq_id>  TIdRef;
    typedef vector<bool> TAvailableSubtypeVector;

    static int ChooseBetterSubtype (int subtype1, int subtype2);

    CSeqTextDataSource(objects::CSeq_entry& sep, objects::CScope& scope);
    CSeqTextDataSource(objects::CBioseq_Handle handle, objects::CScope& scope);
    CSeqTextDataSource(objects::CSeq_loc &loc, objects::CScope& scope);

    virtual ~CSeqTextDataSource();

    void GetSeqData (TSeqPos start, TSeqPos stop, string& buffer, objects::SAnnotSelector *feat_sel = NULL, bool showFeatAsLower = false);
    void GetFeatureData (TSeqPos start_offset, TSeqPos stop_offset, CSeqTextConfig *cfg,  ISeqTextGeometry* pParent, CSeqTextDefs::TSubtypeVector &subtypes, CSeqTextDefs::TSpliceSiteVector &splice_sites, CSeqTextDefs::TVariationGraphVector &variations);
    void GetSubtypesForAlternatingExons (TSeqPos start_offset, TSeqPos stop_offset, ISeqTextGeometry* pParent, CSeqTextDefs::TSubtypeVector &subtypes);
    void GetIntervalBreaks (TSeqPos start_offset, TSeqPos stop_offset, CSeqTextDefs::TSeqPosVector &breaks);
    void GetVariations (TSeqPos start_offset, TSeqPos stop_offset, CSeqTextDefs::TVariationGraphVector &variations);
    void RenderFeatureExtras (ISeqTextGeometry* pParent, CGlPane &pane, TSeqPos seq_start, TSeqPos seq_stop);

    TSeqPos  GetDataLen ();
    string   GetTitle ();
    TIdRef   GetId()  const;

    TSeqPos SequencePosToSourcePos (TSeqPos sequence_pos, bool *found_in_source = NULL);
    TSeqPos SourcePosToSequencePos (TSeqPos source_pos);
    string  GetToolTipForSourcePos (TSeqPos source_pos);

    vector<CConstRef<objects::CSeq_feat> > GetFeaturesAtPosition(TSeqPos source_pos);
    int FindSequenceFragment (const string& fragment, TSeqPos start_search);
    void FindSequenceFragmentList (const string& fragment, CSeqTextDefs::TSeqPosVector &locations, ICanceled* cancel);

    bool IsmRNASequence ();

    const objects::CSeq_loc* GetLoc() const;

    objects::CScope&     GetScope();
    objects::CBioseq_Handle GetSeq() {return m_SubjectHandle;}
protected:

    void GetSeqString (TSeqPos start, TSeqPos stop, string &buffer);
    void x_PopulateFeatureIterators();

    CRef<objects::CSeq_entry>      m_Sep;
    CRef<objects::CScope>          m_Scope;
    CRef<objects::CSeq_loc>        m_Loc;
    objects::CBioseq_Handle        m_SubjectHandle;
    TIdRef                         m_ID;

    typedef vector<CSeqTextDataSourceInterval> TIntervalList;
    TIntervalList m_Intervals;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_TEXT___SEQ_TEXT_DS__HPP
