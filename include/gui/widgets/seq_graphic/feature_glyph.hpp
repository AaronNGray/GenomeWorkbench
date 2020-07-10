#ifndef GUI_WIDGETS_SEQ_GRAPHIC___FEATURE_GLYPH__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___FEATURE_GLYPH__HPP

/*  $Id: feature_glyph.hpp 44708 2020-02-27 16:13:12Z rudnev $
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
 * Authors:  Mike DiCuccio, Liangshou Wu
 *
 * File Description:
 *    CFeatGlyph -- utility class to arrange CSeq_feat objects in hierarchical
 *                (tree) order.
 */

#include <gui/gui.hpp>
#include <gui/objutils/label.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/widgets/seq_graphic/alngraphic_ds.hpp>
#include <gui/widgets/seq_graphic/seq_glyph.hpp>
#include <gui/widgets/seq_graphic/object_based_glyph.hpp>
#include <gui/widgets/seq_graphic/feature_conf.hpp>
#include <gui/widgets/seq_graphic/feature_enums.hpp>

#include <corelib/ncbiobj.hpp>
#include <objmgr/feat_ci.hpp>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// Class for storing a set of nested features.
/// A feature describes the genetic location in true gene coordinates of any
/// sort of genetic object (gene / protein / RNA / etc).  Each feature has an
/// overall extent as well as a series of subregions
///
BEGIN_objects_SCOPE
class CUser_object;
END_objects_SCOPE

typedef struct SMappedSegment
{
    SMappedSegment() {}
    SMappedSegment(CRef<objects::CSeq_interval> product_int,
                   CRef<objects::CSeq_interval> mapped_int,
                   const TSeqRange& anchor_range = TSeqRange::GetEmpty(),
                   const TSeqRange& mapped_range = TSeqRange::GetEmpty()) :
                   m_ProductInt(product_int),
                   m_MappedInt(mapped_int),
                   m_AnchorRange(anchor_range),
                   m_MappedProdRange(mapped_range) // product range in genomic coordinats
    {
    }
    CRef<objects::CSeq_interval> m_ProductInt;
    CRef <objects::CSeq_interval> m_MappedInt;
    TSeqRange m_AnchorRange; // anchor's range (for projected feature)
    TSeqRange m_MappedProdRange; // product range in genomic cooordiants (for projected feature)
} TMappedSegment;

class  CProjectedMappingInfo :
    public vector<SMappedSegment>
{
public:
    void SetAlignmentDataSource(const IAlnGraphicDataSource& align_ds);
    const IAlnGraphicDataSource* GetAlignmentDataSource() const;
    TSeqRange& GetAnchorRange(const CSeqUtils::TMappingInfo::const_iterator& it) const; // anchor's range (for projected feature)
    TSeqRange GetMappedProdRange(const CSeqUtils::TMappingInfo::const_iterator& it); // product range in genomic cooordiants (for projected feature)
    void GetAnchorSequence(objects::CScope& scope, string& buffer) const;
protected:
    CConstRef< IAlnGraphicDataSource > m_AlnMgr;
};


class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CFeatGlyph :
    public CSeqGlyph,
    public IObjectBasedGlyph
{
public:
    typedef vector< CRef<CFeatGlyph> > TFeatList;
    typedef CSeqUtils::TMappingInfo TMappingInfo;

    enum EFeatureParts {
        eHead_No,
        eTail_No,
        eNeedHead,
        eNeedTail
    };

    enum EUndefinedBpType {
        eBp_Outer,
        eBp_Inner,
        eBp_Unknown
    };

    enum ERulerType {
        eNoRuler = 0,
        eNtRuler = 0x01,
        eAaRuler = 0x02
    };

    /// @name ctors.
    /// @{
    CFeatGlyph(const objects::CMappedFeat& feat,
               ELinkedFeatDisplay LinkedFeat = ELinkedFeatDisplay::eLFD_Default);
    CFeatGlyph(const objects::CMappedFeat& feat,
               const objects::CSeq_loc& loc,
               ELinkedFeatDisplay LinkedFeat = ELinkedFeatDisplay::eLFD_Default);
    /// @}

    /// @name CSeqGlyph virtual method implementation.
    /// @{
    virtual bool NeedTooltip(const TModelPoint& /*p*/, ITooltipFormatter& /*tt*/, string& /*t_title*/) const;
    virtual void GetTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const;
    virtual void GetHTMLActiveAreas(TAreaVector* p_areas) const;
    virtual TSeqRange GetRange(void) const;
    virtual bool SetSelected(bool f);
    virtual void SetHideLabel(bool b);
    virtual bool HasSideLabel() const;
    virtual bool IsClickable() const;
    virtual bool LessBySeqPos(const CSeqGlyph& obj) const;
    virtual bool HitTestHor(TSeqPos x, const CObject *obj);
    /// @}

    /// @name IObjectBasedGlyph interface implement
    /// @{
    /// Access the feature's remapped location.
    virtual const objects::CSeq_loc& GetLocation(void) const;
    /// Retrieve the feature as an object.
    /// For safety's sake, this returns the remapped feature.
    virtual CConstRef<CObject> GetObject(TSeqPos pos) const;
    virtual void  GetObjects(vector<CConstRef<CObject> >& objs) const;
    virtual bool HasObject(CConstRef<CObject> obj) const;
    virtual string GetSignature() const;
    virtual const TIntervals& GetIntervals(void) const;
    /// @}

    TIntervals& SetIntervals();

    /// Access the original feature.
    /// The location on this feature may not be correct, as it hasn't
    /// been remapped
    const objects::CSeq_feat&        GetFeature(void) const;

    /// Access a new, fully remapped feature.
    /// This is useful for passing exact features to plugins
    const objects::CMappedFeat&      GetMappedFeature(void) const;

    bool IsLabelHided() const;

    TModelUnit GetBarCenter() const;

    /// retrieve feature label for a given type
    void GetLabel(string& label, CLabel::ELabelType type) const;

    /// Some features may have an accessory label on top (e.g. introns)
    void GetAccessoryLabel(string& accessory_label) const;

    /// Some features may have additional info on the right (alleles for SNPs)
    void GetAdditionalInfo(string& sAdditionalInfo) const;

    /// Check if (intron) feature is consensus or not
    bool IsConsensus() const;

    // true if the feature is based on a remote file derived  annotation
    bool isRmtBased() const;

    CConstRef<CFeatureParams> GetConfig() const;
    void SetConfig(CConstRef<CFeatureParams> conf);

    /// sets the filter that was used for creating the track where this feature glyph resides
    void SetFilter(const string& sFilter) { m_sFilter = sFilter; }

    void SetRulerType(int ruler_types);

    void SetProjectedFeat(bool f);
    bool IsProjectedFeat() const;

    void SetMappingInfo(const TMappingInfo& info);
    const TMappingInfo& GetMappingInfo() const;

    void SetProjectedMappingInfo(const CProjectedMappingInfo& info);
    const CProjectedMappingInfo& GetProjectedMappingInfo() const;

    /// Get the customized color for a given feature.
    /// Special color code.
    /// For features with Var_type and identity qualifier set,
    /// we render them using special color code:
    ///	Color dbVar features based on qual::Var_type
    ///     - Deletions/Loss=red
    ///     - Indels=red
    ///     - Gains/Insertion=green
    ///     - Inversions=gray
    ///	Color segmental duplication feature based on qual::identity
    ///     - Gray: 90-98%
    ///     - Yellow: 98-99%
    ///     - Orange:>99%
    /// @param color the returned customized color
    /// @return true if custom color is needed
    bool GetCustomColor(CRgbaColor& color) const;
    void GetCustomColor(size_t idx, CRgbaColor& color) const;

    virtual string GetPName() const;

    void SetRelatedGlyphSelected(bool Selected);
    bool GetRelatedGlyphSelected() const;

    /// Set prefix to prepend to labels on top
    void SetTopLabelPrefix(const char* prefix);

    /// Get the customized color idx for a given feature.
    static size_t GetCustomColorIdx(const objects::CSeq_feat& feat);
    /// Utility to check if a feature is a structural variation
    static bool   IsDbVar(const objects::CSeq_feat& feat);
    
    /// determines whether the glyph should be drawn or not at all (currently children of unselected parents when m_LinkedFeat
    /// is set to "Expandable")
    bool isDrawn() const { return x_isDrawn(); }

    /// update the layout for everything encompassing this glyph
    void LayoutChanged();
protected:
    virtual void x_Draw() const;
    virtual void x_UpdateBoundingBox();

    /// Returns protein translation offset
    TSeqPos x_GetProtOffset() const;
    
    /// Adjusts a location to accommodate for the protein translation offset
    CRef<objects::CSeq_loc> x_AdjustFrame(const objects::CSeq_loc &loc, TSeqPos offset) const;

    // the following x_Draw*(base) methods must be called in listed order to provide for correct adjusment of base
    void x_DrawRuler(TModelUnit base, bool protein_scale = false) const;
    void x_DrawProjectedRuler(TModelUnit base, bool protein_scale = false) const;
    void x_MaybeDrawLabelAbove(TModelUnit& base) const;
    void x_DrawFeatureBar(TModelUnit& base) const;
    void x_DrawFeatureBar_sv(TModelUnit base) const;
    void x_DrawAdditionalInfo(TModelUnit base) const;
    void x_MaybeDrawLabelOthers(TModelUnit base) const;
    void x_DrawRNAProductSequence(TModelUnit base) const;


    bool x_RedundantLabelCheck(const string& label) const;
    void x_DrawInsertion(TModelUnit SeqPosTriangleMidPointX,
                            TModelUnit BoundaryYLow,
                            TModelUnit BoundaryYHigh,
                            TModelUnit YCenterLine) const;
    void x_DrawDeletion(TModelUnit SeqPosTriangleMidPointX,
                            TModelUnit BoundaryYLow,
                            TModelUnit BoundaryYHigh) const;
    void x_DrawRSites(const CRgbaColor& color, TModelUnit BoundaryYLow, TModelUnit BoundaryYHigh) const;
    void x_DrawLabelWithXPinned(TModelUnit& base) const;
    void x_DrawLabelWithYPinned(TModelUnit base) const;
    void x_DrawInnerLabels(TModelUnit base, vector<TModelRange> *labels_range = nullptr, TSeqRange* interval = nullptr) const;

    /// determines whether the glyph should be drawn or not at all (currently children of unselected parents when m_LinkedFeat
    /// is set to "Expandable")
    bool x_isDrawn() const;

    /// returns true if this glyph is expandable i.e.
    /// - it is not currently selected
    /// - it corresponds to a feature that has children
    /// - the current mode in m_LinkedFeat is "Expandable"
    /// when true, also sets nChildren to number of existing children
    bool x_isExpandable(size_t& nChildren) const;

    /// returns true if this glyph is collapsible i.e.
    /// - it or one of its children is currently selected
    /// - it is a master glyph in a group
    /// - the current mode in m_LinkedFeat is "Expandable"
    bool x_isCollapsible() const;

    bool x_IsProjected() const;

    bool x_CanShowStrand(objects::ENa_strand strand) const;

private:
    TModelUnit x_GetBarHeight() const;
    EUndefinedBpType x_GetUndefinedBp_from(const objects::CSeq_loc& loc) const;
    EUndefinedBpType x_GetUndefinedBp_to(const objects::CSeq_loc& loc) const;
    bool x_HasOuterOnly() const;
    bool x_ShowLabel() const;
    /// check to see if label needs to be drawn before feature bar.
    /// for side-placed labels, returns true on positive strand if left side of the glyph is before start of the whole sequence
    /// or on negative strand beyond the end (has no relation
    /// to the part of sequence being drawn) i.e. most of the time it is "false"
    bool x_LabelFirst() const;
    /// true if a label should be placed to the left of the feature bar (provided that it is ePos_Side)
    bool x_LabelOnLeft() const;
    bool x_ShowRuler() const;
    string x_GetFeatureId() const;
    void x_GetUserColor(const objects::CUser_object &display_settings, CRgbaColor &color) const;

protected:
    /// we store a mapped feature object which in turn holds the
    /// original feature.
    objects::CMappedFeat    m_Feature;

    /// Mapped location in top sequence coordinate.
    /// Not necessarily the sequence that this feature annotated on.
    /// For example, the features associated with an aligned sequence need
    /// to be mapped onto the top sequence to show them in proper positions.
    CConstRef<objects::CSeq_loc> m_Location;

    /// Feature product sequence mapping info.
    /// The mapping pair contains more accurate mapping information than
    /// the feature location. This will be used for features with product
    /// sequence set, such as mRNA feature and CDS features. The mapping info
    /// can be used to accurately map (translated) protein sequence to
    /// feature intervals and show more accurate feature ruler for some cases.
    TMappingInfo m_MappingInfo;

    CProjectedMappingInfo m_ProjectedMappingInfo;

    /// intervals (like for features or alignments).
    TIntervals  m_Intervals;

    /// All the configs needed for rendering a feature.
    CConstRef<CFeatureParams>    m_Config;

    /// Force to hide the label.
    /// There is another setting related to showing label in CFeatParams.
    /// That is global for configuring feature subtype.  But m_HideLabel
    /// is for configureing a single feature glyph.  The consquence is
    /// that we can show labels for all features (with a given feature
    /// subtype) other than those with m_HideLabel set to true.
    bool m_HideLabel;

    /// Projected features.
    /// flag indicating if this is a projected feature from other
    /// annotated sequences (CDSs, mRNAs, and alignments)
    bool m_ProjectedFeat;

    /// Flag indicating if this is a restriction site
    bool m_RSite;

    int m_RulerType;

    /// linkage mode for the track where this glyph is residing
    ELinkedFeatDisplay m_LinkedFeat;

    /// filter (if any) for the track where this glyph is residing
    string m_sFilter;

    /// Prefix to prepend to labels on top
    string m_sTopLabelPrefix;

    mutable map<CLabel::ELabelType, string> m_Labels;
};


//
// Predicates for sorting gene info
//

//
// SFeatByPos()
// arrange features by position.  in essence, this sorts features as a
// CSeq_feat is sorted
//
struct NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT SFeatByPos
{
    bool operator() (const CFeatGlyph& g0, const CFeatGlyph& g1) const {
        return (g0.GetFeature().Compare(g1.GetFeature(),
                                        g0.GetLocation(),
                                        g1.GetLocation()) < 0);
    }
    bool operator() (const CFeatGlyph* g0, const CFeatGlyph* g1) const {
        return (*this)(*g0, *g1);
    }
};

///////////////////////////////////////////////////////////////////////////////
/// CFeatGlyph inline implementations

inline
CFeatGlyph::TIntervals& CFeatGlyph::SetIntervals()
{
    return m_Intervals;
}

inline
const objects::CSeq_feat& CFeatGlyph::GetFeature(void) const
{
    return m_Feature.GetOriginalFeature();
}

inline
const objects::CMappedFeat& CFeatGlyph::GetMappedFeature(void) const
{
    return m_Feature;
}

inline
bool CFeatGlyph::IsLabelHided(void) const
{
    return m_HideLabel;
}

inline
CConstRef<CFeatureParams> CFeatGlyph::GetConfig() const
{
    return m_Config;
}

inline
void CFeatGlyph::SetConfig(CConstRef<CFeatureParams> conf)
{
    m_Config = conf;
}

inline
void CFeatGlyph::SetRulerType(int ruler_types)
{
    m_RulerType = ruler_types;
}

inline
void CFeatGlyph::SetProjectedFeat(bool f)
{
    m_ProjectedFeat = f;
}

inline
bool CFeatGlyph::IsProjectedFeat() const
{
    return m_ProjectedFeat;
}

inline
void CFeatGlyph::SetMappingInfo(const TMappingInfo& info)
{
    m_MappingInfo = info;
}

inline
const CFeatGlyph::TMappingInfo& CFeatGlyph::GetMappingInfo() const
{
    return m_MappingInfo;
}

inline
void CFeatGlyph::SetProjectedMappingInfo(const CProjectedMappingInfo& info)
{
    m_ProjectedMappingInfo = info;
}

inline
const CProjectedMappingInfo& CFeatGlyph::GetProjectedMappingInfo() const
{
    return m_ProjectedMappingInfo;
}

inline
void CFeatGlyph::GetCustomColor(size_t idx, CRgbaColor& color) const
{
    const CCustomFeatureColor::TColorCode& colors =
        m_Config->m_CustomColors->GetColorCode();
    if (idx == 0) {
        color = m_Config->m_fgColor;
    } else if (idx < colors.size())  {
        color = colors[idx];
    }
}

inline
void CFeatGlyph::SetTopLabelPrefix(const char* prefix)
{
    _ASSERT(prefix);
    m_sTopLabelPrefix = prefix;
}


///////////////////////////////////////////////////////////////////////////////
/// CProjectedMappingInfo inline methods
///

/*
inline
bool CProjectedMappingInfo::IsProjected() const
{
    return m_AlnMgr != nullptr;
}

*/
inline
void CProjectedMappingInfo::SetAlignmentDataSource(const IAlnGraphicDataSource& align_ds)
{
    m_AlnMgr.Reset(&align_ds);
}

inline
const IAlnGraphicDataSource* CProjectedMappingInfo::GetAlignmentDataSource() const
{
    return m_AlnMgr.GetPointer();
}


END_NCBI_SCOPE

/* @} */

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___FEATURE_GLYPH__HPP
