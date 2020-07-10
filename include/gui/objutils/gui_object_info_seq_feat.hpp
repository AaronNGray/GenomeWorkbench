#ifndef GUI_UTILS___GUI_OBJECT_INFO_SEQ_FEAT__HPP
#define GUI_UTILS___GUI_OBJECT_INFO_SEQ_FEAT__HPP

/*  $Id: gui_object_info_seq_feat.hpp 44145 2019-11-06 18:15:32Z shkeda $
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

#include <corelib/ncbistr.hpp>
#include <gui/objutils/interface_registry.hpp>
#include <gui/objutils/gui_object_info.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <gui/objutils/utils.hpp>
#include <objmgr/mapped_feat.hpp>

/** @addtogroup GUI_UTILS
 *
 * @{
 */

BEGIN_NCBI_SCOPE

class NCBI_GUIOBJUTILS_EXPORT CGuiObjectInfoSeq_feat
    : public CObject
    , public IGuiObjectInfo
{
public:
    using TSeqFeatMapInfoPair = std::pair<CConstRef<objects::CSeq_feat>, CSeqUtils::TMappingInfo>;
    using TSeqFeatVector = vector<TSeqFeatMapInfoPair>;

public:
    static CGuiObjectInfoSeq_feat* CreateObject(SConstScopedObject& object, ICreateParams* params);

    virtual string GetType() const { return "Feature"; }
    virtual string GetSubtype() const;
    virtual string GetLabel() const;
    virtual void GetToolTip(ITooltipFormatter& tooltip, string& t_title, TSeqPos at_p = (TSeqPos)-1, bool* isGeneratedBySvc = NULL) const;
    virtual void GetLinks(ITooltipFormatter& links, bool no_ncbi_base) const;
    virtual string GetIcon() const { return "symbol::feature"; }
    virtual string GetViewCategory() const { return "Sequence"; }
    void SetLocation(const objects::CSeq_loc& loc);
    void SetTaxId(int tax_id);
    void SetMappingInfo(const CSeqUtils::TMappingInfo& mapping_info);
    void SetMappedFeat(const objects::CMappedFeat& mapped_feat);

protected:
    using TLenPair = pair<string, string>;
    using TLens = vector<TLenPair>;
    string x_CreateLink(const string& label, const string& link) const;

    /// Add feature location-related information.
    void x_AddLocationRows(ITooltipFormatter& tooltip) const;
    void x_AddGroupLocationRows(ITooltipFormatter& tooltip) const;

    void x_GetFeatureToolTip(ITooltipFormatter& tooltip, string& t_title, TSeqPos at_p = (TSeqPos)-1) const;
    void x_GetGroupToolTip(ITooltipFormatter& tooltip, string& t_title, TSeqPos at_p = (TSeqPos)-1) const;

    /// Add position-specific (current cursor position) information.
    void x_AddPositionRows(ITooltipFormatter& tooltip, TSeqPos at_p) const;

    void x_AddExtraInfo(ITooltipFormatter& tooltip, TSeqPos at_p) const;

    void x_GetLocLengthRow(const objects::CSeq_loc &loc, const objects::CSeq_feat &feat, TLens &lens, const string &len_suffix) const;

    void x_GetProductLengthRow(const objects::CSeq_feat &feat, TLens &lens) const;

    CConstRef<objects::CSeq_loc> x_GetFeatLocation(const objects::CSeq_feat& feat, const objects::CBioseq_Handle& handle) const;
    
private:
    /// Data structure for holding a feature segment.
    /// It could be an exon, intron or out of bound
    struct SFeatSeg
    {
        SFeatSeg()
            : m_Type(eType_OutOfBound)
            , m_CrossOrigin(false)
            , m_Range(1, 0)
            , m_ProdRange(1, 0)
            , m_ExonDirForward(true)
        {}

        enum EType
        {
            eType_Exon,
            eType_Intron,
            eType_OutOfBound
        };

        EType       m_Type;
        bool        m_CrossOrigin;
        TSeqRange   m_Range;       ///< sequence 
        TSeqRange   m_ProdRange;   ///< in nucleotide coord.

        /// Previous segment's strand of an intron or
        /// this segment's strand if it is an exon.
        bool        m_ExonDirForward;
    };

    SFeatSeg x_GetHitSeg(TSeqPos pos) const;

    TSignedSeqPos x_ToFeature(TSeqPos pos) const;

    TSignedSeqPos x_ToProduct(TSeqPos pos) const;

    TSeqPos x_GetProtOffset() const;

    void x_GetSplicedSeqTitle(const SFeatSeg& seg, TSeqPos at_p, ITooltipFormatter& tooltip) const;

    void x_GetProductSequence(const SFeatSeg& seg, TSeqPos at_p, ITooltipFormatter& tooltip) const;
    
    string x_GetProductLabel() const;

    string x_GetAccession() const;

protected:
    CConstRef<objects::CSeq_feat> m_Feat;
    mutable CRef<objects::CScope> m_Scope;

    /// Mapped location in top sequence coordinate (can be set externally).
    /// Not necessarily the sequence that this feature annoated on.
    /// A unique seq-id is guaranteed.
    CConstRef<objects::CSeq_loc> m_Location;

    // CDbTag needs tax_id to create correct dbxref links 
    int m_TaxId;

    CSeqUtils::TMappingInfo m_MappingInfo;

    TSeqFeatVector  m_FeaturesGroup;

    objects::CMappedFeat m_MappedFeat;

};

inline
void CGuiObjectInfoSeq_feat::SetLocation(const objects::CSeq_loc& loc)
{
    m_Location.Reset(&loc);
}

inline
void CGuiObjectInfoSeq_feat::SetTaxId(int tax_id)
{
    m_TaxId = tax_id; 
}


inline
string CGuiObjectInfoSeq_feat::x_CreateLink(const string& label,
                                            const string& link) const
{
    return "<a href=\"" + link + "\">" + label + "</a>";
}

inline
void CGuiObjectInfoSeq_feat::SetMappingInfo(const CSeqUtils::TMappingInfo& mapping_info)
{
    m_MappingInfo = mapping_info;
}

inline
void CGuiObjectInfoSeq_feat::SetMappedFeat(const objects::CMappedFeat& mapped_feat)
{
    m_MappedFeat = mapped_feat;
}

END_NCBI_SCOPE

/* @} */

#endif  // GUI_UTILS___GUI_OBJECT_INFO_SEQ_FEAT__HPP
