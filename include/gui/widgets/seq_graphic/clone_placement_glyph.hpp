#ifndef GUI_WIDGETS_SEQ_GRAPHIC___CLONE_PLACEMENT_GLYPH__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___CLONE_PLACEMENT_GLYPH__HPP

/*  $Id: clone_placement_glyph.hpp 35413 2016-05-04 17:51:26Z evgeniev $
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
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>
#include <gui/gui.hpp>
#include <objmgr/feat_ci.hpp>

#include <gui/objutils/label.hpp>
#include <gui/widgets/seq_graphic/seq_glyph.hpp>
#include <gui/widgets/seq_graphic/object_based_glyph.hpp>
#include <gui/widgets/seq_graphic/feature_conf.hpp>

#include <list>
#include <set>
#include <vector>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CClonePlacementGlyph is similar to CFeatureGlyph, but with different 
/// rendering style and settings.
///

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CClonePlacementGlyph :
    public CSeqGlyph,
    public IObjectBasedGlyph
{
public:
    enum EConcordancy {
        eConcordant,
        eDiscordant,
        eCCNotSet
    };

    enum EUniqueness {
        eUnique,
        eMultiple,
        eVirtual,
        eUniqueNotSet
    };

    // Support type for all non-prototype Clone-seqs.
    enum ESupportType {
        eNoNonprototypeEnd = 0x0,
        eSupporting        = 0x1,
        eNonsupporting     = 0x2,
        eMixed             = 0x3,
        
    };
    
    enum EEndStrand {
        ePositive,
        eNegative,
        eStrandNotSet
    };

    struct SCloneEnd
    {
        SCloneEnd()
            : m_Uniqueness(eUniqueNotSet)
            , m_Strand(eStrandNotSet)
        {}

        EUniqueness m_Uniqueness;
        EEndStrand  m_Strand;
        TSeqRange   m_Range;
    };

    typedef vector<SCloneEnd> TCloneEnds;

    /// @name ctors.
    /// @{
    CClonePlacementGlyph(const objects::CMappedFeat& feat);
    CClonePlacementGlyph(const objects::CMappedFeat& feat, const objects::CSeq_loc& loc);
    /// @}

    /// @name CSeqGlyph virtual method implementation.
    /// @{
    virtual bool NeedTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const;
    virtual void GetTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const;
    virtual void GetHTMLActiveAreas(TAreaVector* p_areas) const;
    virtual TSeqRange GetRange(void) const;
    virtual bool IsClickable() const;
    virtual void SetHideLabel(bool b);
    virtual bool HasSideLabel() const;
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

    CConstRef<CClonePlacementParams> GetConfig() const;
    void SetConfig(CConstRef<CClonePlacementParams> conf);

    string GetLabel() const;

protected:
    typedef set<string> TStrSet;
    typedef vector<string> TStrVector;

    virtual void x_Draw() const;
    virtual void x_UpdateBoundingBox();

    void x_SetPolygonStipple(EUniqueness uniqueness) const;
    ESupportType x_GetSupportTypeForNonPrototypeEnds() const;
    bool x_MatchIds(const TStrSet &supportingIds, const TStrVector &nonSupportingIds) const;

private:
    bool x_ShowLabel() const;
    TSeqRange x_GetTotalRange() const;
    void x_GetCloneEnds(TCloneEnds& clone_ends, TCloneEnds& prototype_ends) const;
    void x_GetBarSize(TModelUnit& bar_h, TModelUnit& head_h) const;

protected:
    /// we store a mapped feature object which in turn holds the
    /// original feature.
    objects::CMappedFeat    m_Feature;

    /// Mapped location in top sequence coordinate.
    /// Not necessarily the sequence that this feature annoated on.
    /// For example, the features associated with an aligned sequence need
    /// to be mapped onto the top sequence to show them in proper positions.
    CConstRef<objects::CSeq_loc> m_Location;

    /// intervals (like for features or alignments).
    TIntervals  m_Intervals;

    /// Force to hide the label.
    /// There is another setting related to showing label in CFeatParams.
    /// That is global for configuring feature subtype.  But m_HideLabel
    /// is for configureing a single feature glyph.  The consquence is
    /// that we can show labels for all feautres (with a given feature
    /// subtype) other than those with m_HideLabel set to true.
    bool m_HideLabel;

    /// All the configs needed for rendering a feature.
    CConstRef<CClonePlacementParams>    m_Config;
};


///////////////////////////////////////////////////////////////////////////////
/// CClonePlacementGlyph inline implementations
///
inline
CClonePlacementGlyph::TIntervals& CClonePlacementGlyph::SetIntervals()
{
    return m_Intervals;
}

inline
const objects::CSeq_feat& CClonePlacementGlyph::GetFeature(void) const
{
    return m_Feature.GetOriginalFeature();
}

inline
const objects::CMappedFeat& CClonePlacementGlyph::GetMappedFeature(void) const
{
    return m_Feature;
}

inline
CConstRef<CClonePlacementParams> CClonePlacementGlyph::GetConfig() const
{
    return m_Config;
}

inline
void CClonePlacementGlyph::SetConfig(CConstRef<CClonePlacementParams> conf)
{
    m_Config = conf;
}


END_NCBI_SCOPE

/* @} */

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___CLONE_PLACEMENT_GLYPH__HPP
