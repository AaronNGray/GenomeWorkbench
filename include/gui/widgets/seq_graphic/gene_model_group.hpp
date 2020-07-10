#ifndef GUI_WIDGET_SEQ_GRAPHICS___GENE_MODEL_GROUP__HPP
#define GUI_WIDGET_SEQ_GRAPHICS___GENE_MODEL_GROUP__HPP

/*  $Id: gene_model_group.hpp 43230 2019-05-30 18:25:57Z shkeda $
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
 * Authors:  Liangshou Wu
 *
 */

#include <gui/widgets/seq_graphic/layout_group.hpp>
#include <gui/widgets/seq_graphic/feature_conf.hpp>
#include <gui/widgets/seq_graphic/object_based_glyph.hpp>

BEGIN_NCBI_SCOPE

class CFeatGlyph;

///////////////////////////////////////////////////////////////////////////////
/// CGeneGroup is a subclass of CLayoutGroup contains gene, RNA, cds, and Exon
/// features.  CGeneGroup is able to deal gene model rendering when merging
/// RNA-CDS pair, merging all RNAs and CDSs, and hiding gene features.
///

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CGeneGroup
    : public CLayoutGroup
    , public IObjectBasedGlyph
{
public:
    enum EGene {
        eGene_Shown,         ///< show gene bar
        eGene_ShowLabelOnly, ///< hide gene bar, but show label with bar space
        eGene_Hidden         ///< hide gene bar and gene label completely
    };

    CGeneGroup() : m_ShowGene(eGene_Shown) {}
    virtual ~CGeneGroup(void) {}

    /// @name CSeqGlyph interface implementation
    /// @{
    virtual void Update(bool layout_only);
    /// Get gene feature range instead.
    virtual TSeqRange GetRange(void) const;
    virtual bool Accept(IGlyphVisitor* visitor);
    virtual void GetHTMLActiveAreas(TAreaVector *p_areas) const;
    /// @}

    /// @name IObjectBasedGlyph interface implement
    /// @{
    virtual const objects::CSeq_loc& GetLocation() const;
    virtual CConstRef<CObject> GetObject(TSeqPos pos) const;
    virtual void  GetObjects(vector<CConstRef<CObject> >& objs) const;
    virtual bool HasObject(CConstRef<CObject> obj) const;
    virtual string GetSignature() const;
    virtual const TIntervals& GetIntervals() const;
    /// @}

    void SetShowGene(bool flag);
    void HideGeneAndLabel();

protected:
    /// @name CSeqGlyph virtual methods.
    /// @{
    virtual void x_Draw() const;
    /// @}

    bool x_ShowGeneLabel() const;
    /// Returns the height of the label with margin included
    TModelUnit x_GetLabelHeight() const;

private:
    const CFeatGlyph* x_GetFirstFeat() const;

private:
    EGene    m_ShowGene;
};


///////////////////////////////////////////////////////////////////////////////
/// CLinkedFeatsGroup is a container class contains related features.
/// CLinkedFeatsGroup can merge all the features it contains and show
/// the merged features into on single bar consisting of a set of segments.
/// Each segment may have a different intensity color to indicate the number
/// of features overlaping within that region.
///

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CLinkedFeatsGroup
    : public CSeqGlyph
    , public IObjectBasedGlyph
{
public:
    CLinkedFeatsGroup();
    virtual ~CLinkedFeatsGroup(void) {}

    enum FLabelType {
        fLabel_ParentLabel  = 1 << 0,
        fLabel_FeatNum      = 1 << 1,
        fLabel_Both         = fLabel_ParentLabel | fLabel_FeatNum
    };

    /// @name CSeqGlyph interface implementation
    /// @{
    virtual void Update(bool layout_only);
    virtual CRef<CSeqGlyph> HitTest(const TModelPoint& p);
    virtual bool Intersects(const TModelRect& rect, TConstObjects& objs) const;
    virtual bool NeedTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const;
    virtual void GetTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const;
    virtual void GetHTMLActiveAreas(TAreaVector* p_areas) const;
    virtual TSeqRange   GetRange(void) const;
    virtual bool SetSelected(bool f);
    virtual bool Accept(IGlyphVisitor* visitor);
    virtual bool IsClickable() const;
    /// @}

    /// @name IObjectBasedGlyph interface implement
    /// @{
    virtual const objects::CSeq_loc& GetLocation() const;
    virtual CConstRef<CObject> GetObject(TSeqPos pos) const;
    virtual void  GetObjects(vector<CConstRef<CObject> >& objs) const;
    virtual bool HasObject(CConstRef<CObject> obj) const;
    virtual string GetSignature() const;
    virtual const TIntervals& GetIntervals() const;
    /// @}

    /// @name delegate methods of CLayoutGroup
    /// @{
    const ILayoutPolicy::TObjectList& GetChildren() const;
    ILayoutPolicy::TObjectList& SetChildren();
    void PushBack(CSeqGlyph* obj);
    void SetLayoutPolicy(ILayoutPolicy* policy);
    /// @}

    void SetConfig(const CFeatureParams* conf);
    CLayoutGroup& SetGroup();
    void SetParentFeat(const CFeatGlyph* feat);
    CConstRef<CFeatGlyph> GetParentFeat() const;

    void SetLabelType(FLabelType type);
    void SetFirstIsParent();

    // true if some feature is in the group  based on a remote file derived  annotation
    bool isRmtBased() const;

    void SetShowLabel(bool show_label = true);

protected:
    /// @name CSeqGlyph virtual methods.
    /// @{
    virtual void x_Draw() const;
    virtual void x_UpdateBoundingBox();
    /// override x_DrawHighlight() to do nothing.
    virtual void x_DrawHighlight() const {}
    /// @}

private:
    /// Check if need to show label.
    bool x_ShowLabel() const;

    /// check to see if label needs to be drawn before (above) feature bar.
    bool x_LabelFirst() const;

    /// draw the labels for merged features.
    void x_DrawLabels(TModelUnit& base, bool draw_left=true, bool draw_right=true) const;

    /// draw the merged feature bar with children on top of parent.
    void x_DrawChildrenOnParent(TModelUnit& base) const;

    /// draw the merged feature bar.
    void x_DrawFeatureBar(TModelUnit& base) const;

    /// calculate the merged feature bar intervals and overlapping counts.
    void x_CalcIntervals();

    /// utility method for generating a unique label for the merged bar.
    void x_GetUniqueLabel(string& label) const;

    /// get the first feature glyph in the group.
    const CFeatGlyph* x_GetFirstFeat() const;

    /// get the last feature glyph in the group.
    const CFeatGlyph* x_GetLastFeat() const;

    /// Returns the height of the label with margin included
    TModelUnit x_GetLabelHeight() const;

    void x_AddFeatHTMLActiveArea(TAreaVector* p_areas, const CFeatGlyph* feat, unsigned merged_feats_count = 0) const;

    bool x_AreAllFeaturesOnSameStrand() const;

private:
    typedef vector<int> TCounts;

    /// just for satisfy IObjectBasedGlyph interface.
    CRef<objects::CSeq_loc>   m_Location;
    CLayoutGroup    m_Group;
    CConstRef<CFeatGlyph>  m_ParentFeat;
    FLabelType      m_LabelType;

    /// settings for rendering merged feature bar.
    CConstRef<CFeatureParams>    m_Config;
    TIntervals      m_Intervals;
    TCounts         m_IntCounts;    ///< Shall have the same size as m_Intervals.
    bool            m_FirstIsParent;///< The first feature is the parent feature.
    bool            m_ShowLabel = true;
};


///////////////////////////////////////////////////////////////////////////////
/// CGeneGroup inline methods
///
inline
void CGeneGroup::HideGeneAndLabel()
{
    m_ShowGene = eGene_Hidden;
}


///////////////////////////////////////////////////////////////////////////////
/// CLinkedFeatsGroup inline methods
///
inline
const ILayoutPolicy::TObjectList& CLinkedFeatsGroup::GetChildren() const
{ return m_Group.GetChildren(); }

inline
ILayoutPolicy::TObjectList& CLinkedFeatsGroup::SetChildren()
{ return m_Group.SetChildren(); }

inline
void CLinkedFeatsGroup::PushBack(CSeqGlyph* obj)
{ m_Group.PushBack(obj); }

inline
void CLinkedFeatsGroup::SetLayoutPolicy(ILayoutPolicy* policy)
{ m_Group.SetLayoutPolicy(policy); }

inline
void CLinkedFeatsGroup::SetConfig(const CFeatureParams* conf)
{ m_Config.Reset(conf); }

inline
CLayoutGroup& CLinkedFeatsGroup::SetGroup()
{ return m_Group; }

inline
void CLinkedFeatsGroup::SetParentFeat(const CFeatGlyph* feat)
{ m_ParentFeat.Reset(feat); }

inline
CConstRef<CFeatGlyph> CLinkedFeatsGroup::GetParentFeat() const
{ return m_ParentFeat; }

inline
void CLinkedFeatsGroup::SetLabelType(CLinkedFeatsGroup::FLabelType type)
{ m_LabelType = type; }

inline
void CLinkedFeatsGroup::SetFirstIsParent()
{ m_FirstIsParent = true; }

inline
void CLinkedFeatsGroup::SetShowLabel(bool show_label)
{
    m_ShowLabel = show_label;
}

END_NCBI_SCOPE


#endif  // GUI_WIDGET_SEQ_GRAPHICS___GENE_MODEL_GROUP__HPP
