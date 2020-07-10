/*  $Id: feature_glyph.cpp 45057 2020-05-19 16:57:25Z shkeda $
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

#include <ncbi_pch.hpp>
#include <gui/widgets/seq_graphic/feature_glyph.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_utils.hpp>
#include <gui/widgets/seq_graphic/layout_track_impl.hpp>
#include <gui/widgets/seq_graphic/layout_group.hpp>
#include <gui/widgets/seq_graphic/config_utils.hpp>

#include <gui/widgets/gl/ruler.hpp>
#include <gui/objutils/obj_fingerprint.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/objutils/gui_object_info_seq_feat.hpp>
#include <gui/objutils/tooltip.hpp>
#include <gui/objutils/snp_gui.hpp>
#include <gui/opengl/gltexturefont.hpp>
#include <gui/opengl/irender.hpp>

#include <objects/seqfeat/Variation_ref.hpp>
#include <objects/seqfeat/Variation_inst.hpp>
#include <objects/seqfeat/Delta_item.hpp>
#include <objects/seqfeat/Feat_id.hpp>
#include <objects/general/Dbtag.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/general/User_object.hpp>
#include <objects/seqloc/Seq_interval.hpp>
#include <objects/seq/Seq_literal.hpp>
#include <objects/seqloc/Giimport_id.hpp>

#include <objmgr/util/sequence.hpp>
#include <objtools/alnmgr/aln_converters.hpp>
#include <objtools/snputil/snp_utils.hpp>
#include <util/sequtil/sequtil_manip.hpp>

#include <util/sequtil/sequtil_manip.hpp>
#include <corelib/ncbi_stack.hpp>
#include <math.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/// feature ruler height.
static const int kRulerHeight = 16;

/// vertical space between elements.
static const int kVertSpace = 2;

/// Restriction sites marker height (relative to the height of the bar)
static const TModelUnit kRSiteMarkerHeight = 0.5;

/// Restriction sites marker width (absolute)
static const TModelUnit kRSiteMarkerWidth = 0.4;

/// Restriction sites marker visibility threshold (ie if the marker's width is smaller then the threshold, markers are not drawn)
static const TModelUnit kRSiteMarkerThreshold = 2;

/// Shoudl match feature_ds.cpp::kMinScaleForMapping 
/// i.e. what mapping is not created then do not show ruler labels
static const float kMinScaleForRulerLabels = 16.;


// #define DEBUG_INFO_ON_SCREEN 1

//
// simple functor for sorting CRange<> objects
//
template <class T>
struct SRangeSorter
{
    bool operator() (const CRange<T>& r0, const CRange<T>& r1) const
    {
        if (r0.GetFrom() < r1.GetFrom()) {
            return true;
        }
        if (r0.GetFrom() > r1.GetFrom()) {
            return false;
        }
        if (r0.GetTo() < r1.GetTo()) {
            return true;
        }
        return false;
    }
};


/* not used currently
static bool s_IsSV(const CVariation_ref& var)
{
    if (var.IsSetId()  &&  var.GetId().GetTag().IsStr()) {
        const string& id = var.GetId().GetTag().GetStr();
        if (id.length() > 3  &&  id[1] == 's'  &&  id[2] == 'v') {
            return true;
        }
    }
    return false;
}



static bool s_IsSSV(const CVariation_ref& var)
{
    if (var.IsSetId()  &&  var.GetId().GetTag().IsStr()) {
        const string& id = var.GetId().GetTag().GetStr();
        if (id.length() > 4  &&  id[1] == 's'  &&
            id[2] == 's'  &&  id[3] == 'v') {
            return true;
        }
    }
    return false;
}
*/

//
// CFeatGlyph::CFeatGlyph()
//
CFeatGlyph::CFeatGlyph(const CMappedFeat& f, ELinkedFeatDisplay LinkedFeatDisplay)
    : m_Feature(f)
    , m_Location(&f.GetLocation())
    , m_HideLabel(false)
    , m_ProjectedFeat(false)
    , m_RSite(false)
    , m_RulerType(eNoRuler)
    , m_LinkedFeat(LinkedFeatDisplay)
{
    if (m_Feature.IsSetData() && (CSeqFeatData::eSubtype_rsite == m_Feature.GetData().GetSubtype()))
        m_RSite = true;
}

CFeatGlyph::CFeatGlyph(const CMappedFeat& f, const CSeq_loc& loc, ELinkedFeatDisplay LinkedFeatDisplay)
    : m_Feature(f)
    , m_Location(&loc)
    , m_HideLabel(false)
    , m_ProjectedFeat(false)
    , m_RSite(false)
    , m_RulerType(eNoRuler)
    , m_LinkedFeat(LinkedFeatDisplay)
{
    if (m_Feature.IsSetData() && (CSeqFeatData::eSubtype_rsite == m_Feature.GetData().GetSubtype()))
        m_RSite = true;
}


bool CFeatGlyph::LessBySeqPos(const CSeqGlyph& obj) const
{
    const CFeatGlyph* feat = dynamic_cast<const CFeatGlyph*>(&obj);
    if (feat) {
        // two features - we sort in feature order
        try {
            SFeatByPos comp;
            return (comp(*this, *feat));
        } catch (CException&) {
            // guard against the case that two features have
            // differnt forms of seq-id.
        }
    }

    // otherwise, using default, compare by SeqRanges
    return CSeqGlyph::LessBySeqPos(obj);
}


bool CFeatGlyph::NeedTooltip(const TModelPoint& /*p*/, ITooltipFormatter& /*tt*/, string& t_title) const
{
    return true;
}


void CFeatGlyph::GetTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const
{
    CScope& scope = GetMappedFeature().GetAnnot().GetScope();
    SConstScopedObject scoped_obj(&GetFeature(), &scope);
    CIRef<IGuiObjectInfo> gui_info(
        CreateObjectInterface<IGuiObjectInfo>(scoped_obj, NULL));

    if ( !gui_info ) return;

    TSeqPos at_p = (TSeqPos)-1;
    if (p.X() >= 0) {
        at_p = (TSeqPos)p.X();
    }

    CGuiObjectInfoSeq_feat* gui_info_feat =
        dynamic_cast<CGuiObjectInfoSeq_feat*>(gui_info.GetPointer());
    if (gui_info_feat) {
        if (x_IsProjected()) {
            auto loc = Ref(new CSeq_loc);
            loc->Assign(GetMappedFeature().GetLocation());
            gui_info_feat->SetLocation(*loc);
            if (loc->GetId()) {
                CBioseq_Handle bsh = scope.GetBioseqHandle(*loc->GetId());
                if (bsh)
                    gui_info_feat->SetTaxId(sequence::GetTaxId(bsh));
            }
            if ((int)at_p != -1) {
                auto aln_mgr = m_ProjectedMappingInfo.GetAlignmentDataSource();
                int anchor = aln_mgr->GetAnchor();
                int aligned = aln_mgr->GetQuery();
                TSeqPos pos = at_p;
                if (aln_mgr->GetBaseWidth(anchor) == 3) {
                    pos *= aln_mgr->GetBaseWidth(anchor);
                    int fract_part = (p.X() - floor(p.X())) * 100.;
                    int off = min(2, fract_part / 33);
                    at_p = aln_mgr->GetSeqPosFromSeqPos(aligned, anchor, pos);
                    if (aln_mgr->IsNegativeStrand(aligned))
                        at_p -= off;
                    else
                        at_p += off;
                } else {
                    at_p = aln_mgr->GetSeqPosFromSeqPos(aligned, anchor, pos);
                }

                CSeq_id_Handle prod_id = GetMappedFeature().GetProductId();
                if (prod_id) {
                    CSeqUtils::TMappingInfo mapping_info;
                    CRef<CSeq_id> product_id(new CSeq_id);
                    product_id->Assign(*prod_id.GetSeqId());
                    for (const auto& mi : m_ProjectedMappingInfo) {
                        auto& prod_range = mi.m_MappedProdRange;
                        CRef<CSeq_interval> prod_int(new CSeq_interval(*product_id, prod_range.GetFrom(), prod_range.GetTo(), eNa_strand_plus));
                        mapping_info.push_back(make_pair(prod_int, mi.m_MappedInt));
                    }
                    gui_info_feat->SetMappingInfo(mapping_info);

                }
            }
        } else {
            gui_info_feat->SetLocation(*m_Location);
            const CRenderingContext* ctx = GetRenderingContext();
            if (ctx && ctx->GetSeqDS()) {
                CBioseq_Handle bsh = ctx->GetSeqDS()->GetBioseqHandle();
                if (bsh)
                    gui_info_feat->SetTaxId(sequence::GetTaxId(bsh));
            }
            gui_info_feat->SetMappingInfo(GetMappingInfo());
        }
    }
    gui_info_feat->SetMappedFeat(GetMappedFeature());

	bool isTooltipGeneratedBySvc(false);
	gui_info->GetToolTip(tt, t_title, at_p, &isTooltipGeneratedBySvc);
	if(!isTooltipGeneratedBySvc) {
		gui_info->GetLinks(tt, false);
	}
}


TSeqRange CFeatGlyph::GetRange(void) const
{
    TSeqRange SeqRange;
    try { // watch out for mix loc with multiple seq-ids
        SeqRange = m_Location->GetTotalRange();
    } catch (CException&) {
        // get SeqRange from intervals
        ITERATE(TIntervals,  it,  m_Intervals) {
            SeqRange.CombineWith(*it);
        }
    }

    if (IsDbVar(GetFeature())) {
        if (GetLocation().IsInt()) {
            if (GetLocation().GetInt().IsSetFuzz_from()  &&
                GetLocation().GetInt().GetFuzz_from().IsRange()) {
                const CInt_fuzz::C_Range& f_SeqRange =
                    GetLocation().GetInt().GetFuzz_from().GetRange();
                SeqRange.SetFrom(f_SeqRange.GetMin());
            }
            if (GetLocation().GetInt().IsSetFuzz_to()  &&
                GetLocation().GetInt().GetFuzz_to().IsRange()) {
                const CInt_fuzz::C_Range& t_SeqRange =
                    GetLocation().GetInt().GetFuzz_to().GetRange();
                SeqRange.SetTo(t_SeqRange.GetMax());
            }
        }
    }

    return SeqRange;
}


bool CFeatGlyph::GetCustomColor(CRgbaColor& color) const
{
    if(NSnpGui::isFromVcf(GetFeature())) {
        CGuiRegistry& registry(CGuiRegistry::GetInstance());
//        string sColorTheme(x_GetGlobalConfig()->GetColorTheme());
        string sColorTheme("Color");
        CRegistryReadView ColorView(CSGConfigUtils::GetColorReadView(registry, "GBPlugins.SnpTrack", "Default", sColorTheme));
    	string sColorKey("Default");

        switch(NSnpGui::GetVcfType(GetFeature())) {
            case CVariation_inst::eType_snv:
        		sColorKey = "SingleBase";
                break;
            case CVariation_inst::eType_mnp:
        		sColorKey = "MultiBase";
                break;
            case CVariation_inst::eType_ins:
        		sColorKey = "Insertion";
                break;
            case CVariation_inst::eType_del:
        		sColorKey = "Deletion";
                break;
            case CVariation_inst::eType_delins:
                sColorKey = "Dips";
                break;
        }
        if(sColorKey != "Default") {
    	    CSGConfigUtils::GetColor(ColorView, sColorKey, color);
            return true;
        } else {
            return false;
        }
    }

    size_t idx = GetCustomColorIdx(GetFeature());

    const CCustomFeatureColor::TColorCode& colors =
        m_Config->m_CustomColors->GetColorCode();
    if (idx > 0  &&  idx < colors.size())  {
        color = colors[idx];
        return true;
    }
    return false;
}



void CFeatGlyph::GetHTMLActiveAreas(TAreaVector* p_areas) const
{
//    string sLabel;
//    GetLabel(sLabel, CLabel::eContent);
//    LOG_POST(Trace << "<<<< CFeatGlyph::GetHTMLActiveAreas() " << this << " \"" << sLabel << " isDbVar: " << IsDbVar(GetFeature()));

    if(x_isDrawn()) {
        CHTMLActiveArea area;
        CSeqGlyph::x_InitHTMLActiveArea(area);
        area.m_PositiveStrand = sequence::GetStrand(GetLocation()) != eNa_strand_minus;
        bool is_db_xref = GetFeature().IsSetDbxref();

        switch (GetFeature().GetData().GetSubtype()) {
        case CSeqFeatData::eSubtype_cdregion:
        case CSeqFeatData::eSubtype_mRNA:
            if (GetFeature().IsSetProduct()) {
                const CSeq_loc& product = GetFeature().GetProduct();
                CSeq_id_Handle shdl = sequence::GetId(*product.GetId(),
                    m_Context->GetScope(), sequence::eGetId_ForceGi);
                if (shdl) {
                    area.m_DB_ID =  shdl.AsString();
                }
            } else {
                try {
                    const CSeq_id& id = sequence::GetId(GetLocation(),
                        &m_Context->GetScope());
                    id.GetLabel(&area.m_DB_ID, CSeq_id::eContent);
                } catch (CException&) {
                    /// ignore it
                }
            }
            if (!m_MappingInfo.empty())
                area.m_Flags |= CHTMLActiveArea::fNoCaching;
            break;

        case CSeqFeatData::eSubtype_gene:
            if (is_db_xref) {
                CConstRef<CDbtag> tag = GetFeature().GetNamedDbxref("GeneID");
                if (tag.NotEmpty()) {
                    area.m_DB_Name = "gene";
                    area.m_DB_ID = tag->GetUrl();
                }
            }
            area.m_Flags |= CHTMLActiveArea::fNoCaching;
            break;

        case CSeqFeatData::eSubtype_STS:
            if (is_db_xref) {
                CConstRef<CDbtag> tag = GetFeature().GetNamedDbxref("UniSTS");
                if (tag.NotEmpty()) {
                    area.m_DB_Name = "UniSTS";
                    area.m_DB_ID = tag->GetUrl();
                }
            }
            break;

        case CSeqFeatData::eSubtype_variation:
            if (is_db_xref) {
                CConstRef<CDbtag> tag = NSnp::GetTag(GetFeature());
                if (tag.NotEmpty()) {
                    area.m_DB_Name = "SNP";
                    area.m_DB_ID = tag->GetUrl();
                }
            }
            break;

        case CSeqFeatData::eSubtype_preRNA:
        case CSeqFeatData::eSubtype_tRNA:
        case CSeqFeatData::eSubtype_rRNA:
        case CSeqFeatData::eSubtype_snRNA:
        case CSeqFeatData::eSubtype_scRNA:
        case CSeqFeatData::eSubtype_snoRNA:
        case CSeqFeatData::eSubtype_otherRNA:
        case CSeqFeatData::eSubtype_ncRNA:
        case CSeqFeatData::eSubtype_tmRNA:
            if (!m_MappingInfo.empty())
                area.m_Flags |= CHTMLActiveArea::fNoCaching;
            break;
        case CSeqFeatData::eSubtype_C_region:
        case CSeqFeatData::eSubtype_V_segment:
        case CSeqFeatData::eSubtype_D_segment:
        case CSeqFeatData::eSubtype_J_segment:
            area.m_Flags |= CHTMLActiveArea::fNoCaching;
            break;

        default:
            break;
        }


        // for features with editable flag set, we also report feature-id
        if (GetFeature().IsSetExts()) {
            const CSeq_feat::TExts& exts = GetFeature().GetExts();
            ITERATE (CSeq_feat::TExts, iter, exts) {
                if ( (*iter)->GetType().IsStr()  &&
                    (*iter)->GetType().GetStr() == "Editing"  &&
                (*iter)->GetFieldRef("Editable")  &&
                (*iter)->GetFieldRef("Editable")->GetData().GetBool()  &&
                ( GetFeature().CanGetId()  ||
                    (GetFeature().CanGetIds()  &&
                        !GetFeature().GetIds().empty()) ) ) {
                area.m_Flags |= CHTMLActiveArea::fEditable;
                area.m_ID = x_GetFeatureId();
                if((*iter)->GetFieldRef("Ignorable")  &&
                    (*iter)->GetFieldRef("Ignorable")->GetData().GetBool()) {
                        area.m_Flags |= CHTMLActiveArea::fIgnorable;
                    }
                }
            }
        }
        area.m_Signature = GetSignature();
        
        // a tooltip should be generated for features created by a remote file pipeline to avoid an additional roundtrip
        if(isRmtBased()) {
            string s;
            string title;
            CIRef<ITooltipFormatter> tooltip = ITooltipFormatter::CreateTooltipFormatter(ITooltipFormatter::eTooltipFormatter_CSSTable);
            GetTooltip(TModelPoint(-1, -1), *tooltip, title);
            s = tooltip->Render();
            string text = NStr::Replace(s, "\n", "<br/>");
            area.m_Descr = text;
        }
        p_areas->push_back(area);
//        LOG_POST(Trace << ">>>> CFeatGlyph::GetHTMLActiveAreas() area with signature \"" << area.m_Signature << "\" generated for \"" << sLabel <<
//                            "\", rect: " << area.m_Bounds.Left() << ":" << area.m_Bounds.Right() << ":" << area.m_Bounds.Top() << ":" << area.m_Bounds.Bottom());
    } else {
//        LOG_POST(Trace << ">>>> CFeatGlyph::GetHTMLActiveAreas() no area generated for " << sLabel);
    }
}


bool CFeatGlyph::IsClickable() const
{
    return true;
}

bool CFeatGlyph::isRmtBased() const
{
    return m_Feature.GetAnnot().IsNamed() && CSeqUtils::isRmtAnnotName(m_Feature.GetAnnot().GetName());
}

bool CFeatGlyph::SetSelected(bool f)
{
//    LOG_POST(Trace << "==== CFeatGlyph::SetSelected() " << this << " from " << IsSelected() << " to " << f);
    if (f != IsSelected()) {
        CSeqGlyph::SetSelected(f);
        // The object size may be changed in the following cases:
        //  - For feature with ruler enabled, we always show
        //    feature ruler when it is selected
        //  - For feature with top labeling, but somehow the label
        //    is forced to hide, we will show the label when it is
        //    selected.
        //  - when a selection potentially causes expansion, so that the
        if(m_LinkedFeat == ELinkedFeatDisplay::eLFD_Expandable) {
            CLayoutGroup::PropagateRelatedSelection(f, this);
            x_OnLayoutChanged();
            return true;
        }
        if (m_RulerType != eNoRuler ||
            (m_HideLabel  &&
            m_Config->m_LabelPos == CFeatureParams::ePos_Above)) {
//            LOG_POST(Trace << "Calling x_OnLayoutChanged()");
                x_OnLayoutChanged();
        }
    }
    return false;
}

void CFeatGlyph::LayoutChanged()
{
    x_OnLayoutChanged();
}



void CFeatGlyph::SetHideLabel(bool b)
{
    m_HideLabel = b;
}


bool CFeatGlyph::HasSideLabel() const
{
    return !m_HideLabel  &&  m_Config->m_LabelPos == CFeatureParams::ePos_Side;
}


const objects::CSeq_loc& CFeatGlyph::GetLocation(void) const
{
    return *m_Location;
}


CConstRef<CObject> CFeatGlyph::GetObject(TSeqPos) const
{
    return CConstRef<CObject>(&m_Feature.GetOriginalFeature());
}


void  CFeatGlyph::GetObjects(vector<CConstRef<CObject> >& objs) const
{
    objs.push_back( CConstRef<CObject>(&m_Feature.GetOriginalFeature()) );
}


bool CFeatGlyph::HasObject(CConstRef<CObject> obj) const
{
    return &m_Feature.GetOriginalFeature() == obj.GetPointer();
}

bool CFeatGlyph::HitTestHor(TSeqPos x, const CObject *obj)
{
    return IsInHor(x) && (&m_Feature.GetOriginalFeature() == obj);
}

string CFeatGlyph::GetSignature() const
{
    string sig = kEmptyStr;
    if (m_ProjectedFeat) {
        try {
            sig = CObjFingerprint::GetFeatSignature(
                GetFeature(), GetFeature().GetLocation(),
                &m_Context->GetScope(), m_Feature.GetAnnot(), m_sFilter);
        } catch (CException& e) {
            // failed to generated the signature.
            // it is likely the feature location contains multiple seq-ids.
            LOG_POST(Warning <<
                "CFeatGlyph::GetSignature() failed to generate signature: " <<
                e.GetMsg());
        }
    } else {
        sig = CObjFingerprint::GetFeatSignature(
            GetMappedFeature().GetMappedFeature(), GetLocation(),
            &m_Context->GetScope(), m_Feature.GetAnnot(), m_sFilter);
    }
    return sig;
}


const CFeatGlyph::TIntervals& CFeatGlyph::GetIntervals(void) const
{
    return m_Intervals;
}


TModelUnit CFeatGlyph::GetBarCenter() const
{
    IRender& gl = GetGl();

    TModelUnit y = m_Config->GetBarHeight(
        m_Context->IsOverviewMode()  &&  m_HideLabel) * 0.5;
    if (x_ShowLabel()  &&  m_Config->m_LabelPos == CFeatureParams::ePos_Above) {
        y += gl.TextHeight(&(m_Config->m_LabelFont)) + kVertSpace + 1;
    }

    // Ruler is always above a feature bar
    if (x_ShowRuler()) {
        if (m_RulerType & eNtRuler)
            y += kRulerHeight + kVertSpace;
        if (m_RulerType & eAaRuler)
            y += kRulerHeight + kVertSpace;
    }

    return y;
}


string CFeatGlyph::GetPName() const
{
    string sLabel;
    CScope& scope = GetMappedFeature().GetAnnot().GetScope();
    CLabel::GetLabel(GetFeature(), &sLabel, CLabel::eContent, &scope);

    // for debugging
    // m_sPName = sLabel;

    return sLabel;
}

void CFeatGlyph::GetLabel(string& label, CLabel::ELabelType type) const
{
    auto it = m_Labels.find(type);
    if (it != m_Labels.end()) {
        label = it->second;
        return;
    }
    CScope& scope = GetMappedFeature().GetAnnot().GetScope();
    CLabel::GetLabel(GetFeature(), &label, type, &scope);
    if (type == CLabel::eContent) {
        NStr::ReplaceInPlace(label, "/standard_name=", "");
    }

    size_t nChildren(0);
    bool isExpandable(x_isExpandable(nChildren));
#ifdef DEBUG_INFO_ON_SCREEN
    label += " CFeatGlyph<" + NStr::NumericToString(GetLevel()) + ">/" + NStr::IntToString(x_isDrawn()) + "/" +
                (isExpandable ? string() : "[" + NStr:: NumericToString(nChildren) + "]") +
                " ";
#endif
    label += isExpandable ? " [+" + NStr:: NumericToString(nChildren) + "]" : string();
    label += x_isCollapsible() ? " [-]" : "";
    if(!GetTearlineText().empty()) {
        label += GetTearlineText();
    }
    m_Labels.emplace(type, label);
}

/// Some features may have an accessory label on top (e.g. introns)
/// Not sure if this code should be here - should be more general, but
/// maybe wait and see if other feature types need this too and at
/// that point we could maybe make it more like CLabel?
void CFeatGlyph::GetAccessoryLabel(string& accessory_label) const
{
    accessory_label = "";

    if (GetFeature().IsSetData() && GetFeature().GetData().GetSubtype() == CSeqFeatData::eSubtype_intron) {
        if (GetFeature().IsSetExts()) {
            const CSeq_feat::TExts& exts = GetFeature().GetExts();
            ITERATE (CSeq_feat::TExts, iter, exts) {
                if ((*iter)->GetType().IsStr()  &&
                    (*iter)->GetType().GetStr() == "Support"  &&
                    (*iter)->GetFieldRef("Total") &&
                    !(*iter)->GetFieldRef("Total").IsNull()) {
                    const auto& d = (*iter)->GetFieldRef("Total")->GetData();
                    if (d.IsInt())
                        accessory_label = NStr::IntToString(d.GetInt());
                    else if (d.IsReal()) 
                        accessory_label = NStr::NumericToString(d.GetReal());
                }
            }
        }
    }
}

/// some features may have additional info on the right (currently alleles for SNPs)
/// that will be shown at the same time as labels
void CFeatGlyph::GetAdditionalInfo(string& sAdditionalInfo) const
{
    sAdditionalInfo.clear();

    NSnp::TAlleles Alleles;
    NSnp::GetAlleles(GetFeature(), Alleles);

    sAdditionalInfo = NStr::Join(Alleles, "/");
}

bool CFeatGlyph::IsConsensus() const
{
    // return true if intron is consensus or if data to check consensus (splice sequence)
    // is unavailable.
    if (GetFeature().IsSetData() && GetFeature().GetData().GetSubtype() == CSeqFeatData::eSubtype_intron) {
        if (GetFeature().IsSetExts()) {
            const CSeq_feat::TExts& exts = GetFeature().GetExts();
            ITERATE (CSeq_feat::TExts, iter, exts) {
                if ((*iter)->GetType().IsStr()  &&
                    (*iter)->GetType().GetStr() == "Splice Sequence") {
                        CConstRef<CUser_field> donor = (*iter)->GetFieldRef("Donor");
                        CConstRef<CUser_field> acceptor = (*iter)->GetFieldRef("Acceptor");
                        if (!donor.IsNull() && donor->GetData().IsStr() &&
                            !acceptor.IsNull() && acceptor->GetData().IsStr()) {
                                string donor_str =  donor->GetData().GetStr();
                                string acceptor_str =  acceptor->GetData().GetStr();
                                if ((donor_str == "GT" && acceptor_str == "AG") ||
                                    (donor_str == "GC" && acceptor_str == "AG") ||
                                    (donor_str == "AT" && acceptor_str == "AC"))
                                        return true;
                                else
                                        return false;
                        }
                        else return true;
                }
            }
        }
    }

    return true;
}


static int s_GetLiteralLength(const CVariation_ref& var)
{
    int len = -1;
    if (var.GetData().IsInstance()  &&  var.GetData().GetInstance().IsSetDelta()) {
        const CVariation_inst::TDelta& delta = var.GetData().GetInstance().GetDelta();
        ITERATE (CVariation_inst::TDelta, iter, delta) {
            if ((*iter)->IsSetSeq()  &&  (*iter)->GetSeq().IsLiteral()) {
                len = (*iter)->GetSeq().GetLiteral().GetLength();
                break;
            }
        }
    }
    return len;
}

/*
static int s_GetChildNum(const objects::CSeq_feat& feat)
{
    string num_str = feat.GetNamedQual("Child Count");
    if ( !num_str.empty() ) {
        try {
            return NStr::StringToInt(num_str);
        } catch (CException&) {
            // ignore it
        }
    }
    return -1;
}
*/

size_t CFeatGlyph::GetCustomColorIdx(const CSeq_feat& feat)
{
    CCustomFeatureColor::EColorIdex idx = CCustomFeatureColor::eDefault;
    if (IsDbVar(feat)) {
        const CVariation_ref& var = feat.GetData().GetVariation();
        if (var.IsComplex()) {
            idx = CCustomFeatureColor::eComplex;
        } else if (var.IsInsertion()) {
            idx = CCustomFeatureColor::eInsertion;
        } else if (var.IsInversion()) {
            idx = CCustomFeatureColor::eInversion;
        } else if (var.IsEversion()) {
            idx = CCustomFeatureColor::eEverted;
        } else if (var.IsTranslocation()) {
            idx = CCustomFeatureColor::eTranschr;
        } else if (var.IsGain()) {
            idx = CCustomFeatureColor::eGain;
        } else if (var.IsLoss()  ||  var.IsDeletion()) {
            idx = CCustomFeatureColor::eLoss;
        } else if (var.IsCNV()) {
            idx = CCustomFeatureColor::eCNV;
        } else if (var.IsDeletionInsertion()) {
            idx = CCustomFeatureColor::eDeletionInsertion;
        } else {
            idx = CCustomFeatureColor::eUnknown;
        }
    } else if (feat.GetData().Which() == CSeqFeatData::e_Variation) {
        const string& var_type = feat.GetNamedQual("Var_type");
        if ( !var_type.empty() ) {
            if (NStr::EqualNocase(var_type, "Deletion")  ||
                NStr::EqualNocase(var_type, "Loss")) {
                    idx = CCustomFeatureColor::eLoss;
            } else if (NStr::EqualNocase(var_type, "Insertion")  ||
                NStr::EqualNocase(var_type, "Duplication")) {
                    idx = CCustomFeatureColor::eInsertion;
            } else if (NStr::EqualNocase(var_type, "Gain")) {
                idx = CCustomFeatureColor::eGain;
            } else if (NStr::EqualNocase(var_type, "Inversion")) {
                idx = CCustomFeatureColor::eInversion;
            }
        } else {
            const string& identity = feat.GetNamedQual("identity");
            if ( !identity.empty() ) {
                try {
                    double identity_num = NStr::StringToDouble(identity);
                    if (identity_num > 99.0) {
                        idx = CCustomFeatureColor::eHighIdentity;
                    } else if (identity_num > 98.0) {
                        idx = CCustomFeatureColor::eMidIdentity;
                    } else if (identity_num > 90.0) {
                        idx = CCustomFeatureColor::eLowIdentity;
                    }
                } catch (CException&) {
                    /// ignore
                }
            }
        }
    }
    return (size_t)idx;
}


bool CFeatGlyph::IsDbVar(const objects::CSeq_feat& feat)
{
    if (feat.GetData().Which() == CSeqFeatData::e_Variation) {
        const CVariation_ref& sv = feat.GetData().GetVariation();
        if (sv.CanGetId()  &&  sv.GetId().GetDb() == "dbVar") {
            return true;
        }
    }
    return false;
}


bool CFeatGlyph::x_isDrawn() const
{
//    LOG_POST(Trace << "<<<< CFeatGlyph::x_isDrawn() " << this << " <" << GetLevel() <<  "> \"" << GetPName() << "\"");
    if(m_LinkedFeat == ELinkedFeatDisplay::eLFD_Expandable)
    {
        // level 0 (topmost) are unconditionally shown
        // undefined levels are shown too (just in case)
        if(GetLevel() != -1 && GetLevel() != 0) {
            bool isRelatedGlyphSelected(GetRelatedGlyphSelected());
//            LOG_POST(Trace << ">>>> using GetRelatedGlyphSelected(): " << isRelatedGlyphSelected);
            return isRelatedGlyphSelected;
        }
    }
//    LOG_POST(Trace << ">>>> true");
    return true;
}

bool CFeatGlyph::x_isExpandable(size_t& nChildren) const
{
    if(m_LinkedFeat == ELinkedFeatDisplay::eLFD_Expandable && !GetRelatedGlyphSelected())
    {
        const CSeqGlyph* pParent(GetParent());

        if(pParent) {
            const CLayoutGroup* pParentLayoutGroup(dynamic_cast<const CLayoutGroup*>(pParent));

            if(pParentLayoutGroup) {
                // the presented glyph is a master in the group and the group has other features and none of them are selected
                if(pParentLayoutGroup->IsMaster(this) && pParentLayoutGroup->GetChildrenNum() > 1) {
                    for(size_t iSeqGlyphs=0; iSeqGlyphs < pParentLayoutGroup->GetChildrenNum(); ++iSeqGlyphs) {
                        if(pParentLayoutGroup->GetChild(iSeqGlyphs)->IsSelected()) {
                            return false;
                        }
                    }
                    nChildren = pParentLayoutGroup->GetChildrenNum() - 1;
                    return true;
                }
            }
        }
    }
    return false;
}


bool CFeatGlyph::x_isCollapsible() const
{
    if(m_LinkedFeat == ELinkedFeatDisplay::eLFD_Expandable)
    {
        const CSeqGlyph* pParent(GetParent());

        if(pParent) {
            const CLayoutGroup* pParentLayoutGroup(dynamic_cast<const CLayoutGroup*>(pParent));

            if(pParentLayoutGroup) {
                // the presented glyph is a master in the group and the group has other features and at least one of them is selected
                if(pParentLayoutGroup->IsMaster(this) && pParentLayoutGroup->GetChildrenNum() > 1) {
                    for(size_t iSeqGlyphs=0; iSeqGlyphs < pParentLayoutGroup->GetChildrenNum(); ++iSeqGlyphs) {
                        if(pParentLayoutGroup->GetChild(iSeqGlyphs)->IsSelected()) {
                            return true;
                        }
                    }
                    return false;
                }
            }
        }
    }
    return false;
}



void CFeatGlyph::x_Draw() const
{
    _ASSERT(m_Context);

    IRender& gl = GetGl();
//    LOG_POST(Trace << "<<<<");

/*
// print drawing info
    {
        string sLabel;
        GetLabel(sLabel, CLabel::eContent);
        LOG_POST(Trace << "Attempting to draw " << this << " \"" << sLabel << "\" x_isDrawn: " << x_isDrawn() <<
                          ", m_Height: " << GetHeight() << ", m_Width: " << GetWidth() <<
                          ", m_Pos.m_X: " << GetLeft() << ", m_Pos.m_Y: " << GetTop());
    }
*/
    if (GetHeight() == 0  ||  m_Context->IntersectVisible(GetRange()).Empty() || !x_isDrawn()) {
//        LOG_POST(Trace << ">>>>");
        return;
    }
    // for debugging
    /*
    CScope& scope = GetMappedFeature().GetAnnot().GetScope();
    string sig = CObjFingerprint::GetFeatSignature(GetFeature(), &scope, m_sFilter);
    int subtype;
    CSeq_id_Handle id;
    TSeqPos f, t;
    CObjFingerprint::EObjectType type;
    Uint4   fingerprint;
    CObjFingerprint::ParseSignature(sig, id, f, t, type, subtype, fingerprint);
    cout << ", From: " << f
         << ", To: " << t
         << ", Subtype: " << subtype << endl;
    */

    TModelUnit base = GetTop();
    // Draw a shading background to indicate exception
    if (CSeqUtils::IsException(GetFeature())) {
        gl.Color4f(0.3f, 0.21f, 0.07f, 0.25f);
        TModelUnit padding_y = 1.0;
        TModelUnit padding_x = m_Context->ScreenToSeq(padding_y);
        m_Context->DrawQuad(GetLeft() - padding_x, GetTop() - padding_y,
            GetRight() + padding_x, GetBottom() + padding_y);
//        LOG_POST(Trace << " drawing exception feature with padding_x: " << padding_x << " at left: " << (GetLeft() - padding_x) <<
//                        " and right: " << (GetRight() + padding_x));
    }

    if (x_ShowRuler()) {
        try {
            if (m_RulerType & eNtRuler) {
                x_DrawRuler(base);
                base += kRulerHeight + kVertSpace;
            }
            if (m_RulerType & eAaRuler) {
                x_DrawRuler(base, true);
                base += kRulerHeight + kVertSpace;
            }
        } catch (CException&) {
            // we may have problems on setting up the required alignment
            // mapping for features with mix location (multip seq-ids or
            // mix strands from one location)
        }
    }

    TModelUnit bar_height = x_GetBarHeight();
    x_MaybeDrawLabelAbove(base);
    base += bar_height * 0.5;   // bar center

    if (IsDbVar(GetFeature())) {
        x_DrawFeatureBar_sv(base);
    } else {
        x_DrawFeatureBar(base);
    }
    if (m_Context->WillSeqLetterFit()) {
        if (GetFeature().IsSetProduct()
            && GetFeature().GetData().IsRna()
            && !GetMappingInfo().empty()) {
            x_DrawRNAProductSequence(base);
        }
    }

    x_MaybeDrawLabelOthers(base);
    x_DrawAdditionalInfo(base);

    if (IsSelected()) {
        m_Context->DrawSelection(GetModelRect());
    }
    // LOG_POST(Trace << ">>>>");
}

void CFeatGlyph::x_DrawRNAProductSequence(TModelUnit base) const
{
    // base - is center y-line of the feature
    IRender& gl = GetGl();

    CRef<CSGSequenceDS> seq_ds = m_Context->GetSeqDS();
    const CSeqVector& vec = seq_ds->GetSeqVector();

    string prod_seq;
    size_t prod_len = 0;
    try {
        const CSeq_loc& product = GetFeature().GetProduct();
        CScope& scope = GetMappedFeature().GetAnnot().GetScope();
        CBioseq_Handle bsh = scope.GetBioseqHandle(GetFeature().GetProduct());
        if (!bsh)  // undefined product
            return;
        CSeqVector prod_vec(product, scope, CBioseq_Handle::eCoding_Iupac);
        prod_len = prod_vec.size();
        prod_vec.GetSeqData(0, prod_len, prod_seq);
    } catch (CException&) {
        // ignore exceptions - these arise if the product doesn't resolve
    }

    // Get the mapped genomic sequence based the mapping intervals
    string gen_seq;
    gen_seq.reserve(prod_len);
    {
        TSeqPos pre_t_to = 0; // previos transcript to + 1 coordinate
        for (auto&& iter : GetMappingInfo()) {
            // iter->first interval - transcrit
            // iter->second interval - genomic
            auto& prod_int = *iter.first;
            auto& gen_int = *iter.second;
            // fill the gap on protein sequence with 'N'
            TSeqPos t_from = prod_int.GetFrom();
            if (t_from > pre_t_to)
                gen_seq.append(t_from - pre_t_to, 'N');
            string tmp_seq;
            vec.GetSeqData(gen_int.GetFrom(), gen_int.GetTo() + 1, tmp_seq);
            bool neg = gen_int.IsSetStrand() && gen_int.GetStrand() == eNa_strand_minus;
            if (neg) {
                string seq;
                CSeqManip::ReverseComplement(tmp_seq, CSeqUtil::e_Iupacna,
                                             0, tmp_seq.length(), seq);
                tmp_seq.swap(seq);
            }
            pre_t_to = prod_int.GetTo() + 1;
            gen_seq += tmp_seq;
        }
    }

    char prod_out[2];
    prod_out[1] = '\0';

    static const CRgbaColor c_white("white");
    gl.ColorC(c_white);

    const CGlTextureFont& font = m_Config->m_LabelFont;
    TModelUnit font_height = gl.TextHeight(&font);
    font_height *= 0.5;

    TModelUnit y = base + ceil(font_height);

    CConstRef<CSeq_interval> prev_gen(0);
    CConstRef<CSeq_interval> prev_prod(0);

    auto current_exon = m_Intervals.begin();
    for (auto&& map_it : GetMappingInfo()) {
        // iter->first interval - transcrit
        // iter->second interval - genomic
        auto& prod_int = *map_it.first;
        auto& gen_int = *map_it.second;

        bool neg = gen_int.IsSetStrand() && gen_int.GetStrand() == eNa_strand_minus;
        int step = neg ? -1 : 1;
        TSeqRange gen_range(gen_int.GetFrom(), gen_int.GetTo());
        if (neg) {
            while (current_exon != m_Intervals.end() && current_exon->GetFrom() > gen_range.GetTo()) {
                ++current_exon;
                prev_gen = 0;
            }
        } else {
            while (current_exon != m_Intervals.end() && current_exon->GetTo() < gen_range.GetFrom()) {
                ++current_exon;
                prev_gen = 0;
            }
        }

        // if this interval is outside of the visible range, skip it
        if (m_Context->IntersectVisible(gen_range).Empty())
            continue;
        if (current_exon == m_Intervals.end())
            prev_gen = 0;

        // c_pos - position on genomic sequence
        // prod_pos - position on transcript
        TSeqRange prod_range(prod_int.GetFrom(), prod_int.GetTo());
        TSeqPos c_pos = neg ? gen_range.GetTo() : gen_range.GetFrom();
        TSeqPos prod_pos = prod_range.GetFrom();
        TSeqPos prod_stop = min(prod_range.GetToOpen(), (TSeqPos)gen_seq.size());

        while (prod_pos < prod_stop) {
            bool mismatch = (prod_seq[prod_pos] != gen_seq[prod_pos]);
            if (mismatch) {
                prod_out[0] = prod_seq[prod_pos];
                m_Context->TextOut(&font, prod_out, c_pos + 0.5, y, true);
            }
            prod_pos += 1;
            c_pos += step;
        }
        if (prev_gen != 0) {

            if (prev_prod->GetTo() + 1 != prod_int.GetFrom()) {
                c_pos = neg ? gen_int.GetTo() + 1 : gen_int.GetFrom();
                x_DrawInsertion(c_pos, base - font_height, base + font_height, base);
            } else {
                int gap_length = 0;
                if (neg) {
                     if (prev_gen->GetFrom() - 1 != gen_int.GetTo()) {
                        c_pos = prev_gen->GetFrom() - 1;
                        gap_length = c_pos - gen_int.GetTo();
                    }
                } else {
                    if (prev_gen->GetTo() + 1 != gen_int.GetFrom()) {
                        c_pos = prev_gen->GetTo() + 1;
                        gap_length = gen_int.GetFrom() - c_pos;
                    }
                }
                while (gap_length > 0) {
                    m_Context->TextOut(&font, "-", c_pos + 0.5, y, true);
                    c_pos += step;
                    --gap_length;
                }
            }
        }
        prev_gen.Reset(&gen_int);
        prev_prod.Reset(&prod_int);
    }

}


void CFeatGlyph::x_UpdateBoundingBox()
{
    _ASSERT(m_Context);
    // LOG_POST(Trace << "<<<<");

    if(x_isDrawn()) {
        TSeqRange SeqRange = GetRange();
        TModelUnit bar_height = x_GetBarHeight();
        SetHeight(bar_height);
        SetWidth(SeqRange.GetLength());
        SetLeft(SeqRange.GetFrom());

        bool showLabel = x_ShowLabel();

        // Adjust height, if restrictions markers are present & visible
        if (m_RSite && (m_Context->SeqToScreen(kRSiteMarkerWidth) >= kRSiteMarkerThreshold)) {
            SetHeight(GetHeight() + bar_height*kRSiteMarkerHeight); // Add the height of the bottom cut marker
            if (!showLabel || (m_Config->m_LabelPos == CFeatureParams::ePos_Side))
                SetHeight(GetHeight() + bar_height*kRSiteMarkerHeight); // Add the height of the top cut marker
        }

        // preserve space for undefined breakpoint indicator
        // for structural variants
        if (IsDbVar(GetFeature())) {
            TModelUnit tri_width = m_Context->ScreenToSeq(GetHeight() * 0.5);
            TModelUnit extra = 0.0;
            if (x_GetUndefinedBp_from(GetLocation()) != eBp_Unknown) {
                SetLeft(GetLeft() - tri_width);
                extra += tri_width;
            }
            if (x_GetUndefinedBp_to(GetLocation()) != eBp_Unknown) {
                extra += tri_width;
            }
            SetWidth(GetWidth() + extra);
        }

        // we preserve label space if it fits either on side or above
        if (showLabel) {
            IRender& gl = GetGl();

            const CGlTextureFont& font = m_Config->m_LabelFont;
            if (m_Config->m_LabelPos == CFeatureParams::ePos_Side) {
                string label;
                GetLabel(label, CLabel::eContent);
                TModelUnit text_w_px = min(gl.TextWidth(&font, label.c_str()), m_Context->GetMaxLabelWidth(font));
                TModelUnit text_w_pos = m_Context->ScreenToSeq(text_w_px + CRenderingContext::kLabelSpacePx);
                SetWidth(GetWidth() + text_w_pos);
                TModelUnit min_text_w_pos = m_Context->GetMinLabelWidthPos(font);

                if (x_LabelOnLeft()) {
                    SetLeft(GetLeft() - text_w_pos);
                    TModelRange visible_range_pos = m_Context->IntersectVisible(this);

                    if (GetLeft() < 0) {
                        if (visible_range_pos.GetLength() < min_text_w_pos) {
                            // Can't even show the minimal length of label
                            SetWidth(GetWidth() + GetLeft());
                            SetLeft(0.0);
                        }
                    }
                }
                // also reserve some space for additional info
                string sAdditionalInfo;
                GetAdditionalInfo(sAdditionalInfo);
                if (!sAdditionalInfo.empty()) {
                    TModelUnit info_width_px = min(gl.TextWidth(&font, sAdditionalInfo.c_str()), m_Context->GetMaxLabelWidth(font));
                    TModelUnit info_width_pos = m_Context->ScreenToSeq(info_width_px + CRenderingContext::kLabelSpacePx);
                    SetWidth(GetWidth() + info_width_pos);
                    if(!x_LabelOnLeft()) {
                        SetLeft(GetLeft() - info_width_pos);
                        if (GetLeft() < 0) {
                            SetWidth(GetWidth() + GetLeft());
                            SetLeft(0.0);
                        }
                    }
                }
            } else if (m_Config->m_LabelPos == CFeatureParams::ePos_Above) {
                SetHeight(GetHeight() + gl.TextHeight(&font) + kVertSpace + 1);
            }
        }

        // Preserve space for feature ruler
        if (x_ShowRuler()) {

            if (GetFeature().IsSetExcept() && GetFeature().HasExceptionText("trans-splicing") && !CSeqUtils::IsSameStrands(GetLocation())) {
                m_RulerType = eNoRuler;
            } else {
                if (m_RulerType & eNtRuler)
                    SetHeight(GetHeight() + kRulerHeight + kVertSpace);
                if (m_RulerType & eAaRuler)
                    SetHeight(GetHeight() + kRulerHeight + kVertSpace);
            }
        }
    } else {
        SetHeight(0);
        SetWidth(0);
        SetLeft(0.0);
        SetTop(0.0);
    }
    // print bounding box info
//    {
//        string sLabel;
//        GetLabel(sLabel, CLabel::eContent);
//        LOG_POST(Trace << "Bounding box for " << this << " \"" << sLabel << "\" x_isDrawn: " << x_isDrawn() <<
//                          ", m_Height: " << GetHeight() << ", m_Width: " << GetWidth() <<
//                          ", m_Pos.m_X: " << GetLeft() << ", m_Pos.m_Y: " << GetTop());
//        LOG_POST(Trace << "Call stack for " << this << ": " << CStackTrace());

//        LOG_POST(Trace << ">>>>");
//    }
}

bool CFeatGlyph::x_RedundantLabelCheck(const string& label) const
{
    // If label is redundant with the title (is contained in the title) for the
    // track, and all glyphs in the track are the same type, do not display the label.
    // This only applies though if the label for the track starts with '[' and ends with ']'
    const CSeqGlyph* parent = m_Parent;
    bool redundant = false;
    bool all_same_type = false;

    if (label.length() > 0 && label[0] == '[' && label[label.size()-1] == ']') {
        while (parent != NULL) {
            const CLayoutGroup* lg = dynamic_cast<const CLayoutGroup*>(parent);
            if (lg != NULL) {
                all_same_type = lg->AllChildrenSameType();
            }

            const CLayoutTrack* lt = dynamic_cast<const CLayoutTrack*>(parent);
            if (lt != NULL) {
                // trim blanks, brackets etc from glyph display string - e.g. [intron] to intron
                string track_title = lt->GetFullTitle();
                size_t start_idx = label.find_first_not_of(" ()[]'\" <>,.@#$&*");
                size_t stop_idx = label.find_last_not_of(" ()[]'\" <>,.@#$&*");
                size_t len = stop_idx-start_idx;

                if (len > 0 &&
                    NStr::FindNoCase(track_title, label.substr(start_idx, len)) != NPOS) {
                        redundant = true;
                }
                break;
            }
            parent = parent->GetParent();
        }
    }

    return (all_same_type && redundant);
}


bool CFeatGlyph::x_LabelOnLeft() const
{
    return sequence::GetStrand(GetLocation()) != eNa_strand_minus;
}

void CFeatGlyph::x_MaybeDrawLabelAbove(TModelUnit& base) const
{
    if ( !x_ShowLabel() || m_Config->m_LabelPos != CFeatureParams::ePos_Above) {
        return;
    }
    x_DrawLabelWithXPinned(base);
}

void CFeatGlyph::x_MaybeDrawLabelOthers(TModelUnit base) const
{
    if ( !x_ShowLabel()) {
        return;
    }
    if (CFeatureParams::ePos_Side != m_Config->m_LabelPos)
            return;

    x_DrawLabelWithYPinned(base);
}



void CFeatGlyph::x_DrawLabelWithXPinned(TModelUnit& base) const
{
    if (m_Config->m_LabelPos != CFeatureParams::ePos_Above) {
        return;
    }
    IRender& gl = GetGl();

    // visible part of the whole glyph (feature bar + label + additional info)
    TModelRange visible_range = m_Context->IntersectVisible(this);

    const CGlTextureFont& font = m_Config->m_LabelFont;
    TModelUnit font_height = gl.TextHeight(&font);

    if (m_Context->WillLabelFit(visible_range)) {
        // label text that will be shown
        string sLabelTextOut;
        // label type text (used to estimate whether there is enough space to show both type and content)
        string sLabelTypeText;
        GetLabel(sLabelTypeText, CLabel::eType);
        TModelUnit LabelTypeWidth =
            m_Context->ScreenToSeq(gl.TextWidth(&font, sLabelTypeText.c_str()));

        // if visible part is wide enough, show both label type and content
        if (visible_range.GetLength() > LabelTypeWidth * 4) { // 4 widths of eType label width
            GetLabel(sLabelTextOut, CLabel::eUserTypeAndContent);
            if (!m_sTopLabelPrefix.empty()) {
                string label(m_sTopLabelPrefix);
                label += '/';
                label += sLabelTextOut;
                sLabelTextOut = label;
            }
        } else {
            // otherwise only content
            GetLabel(sLabelTextOut, CLabel::eContent);
        }
        // shorten the text if it is wider than visible range
        sLabelTextOut = font.Truncate(sLabelTextOut.c_str(), m_Context->SeqToScreen(visible_range.GetLength()));

        // both above and inside labels should be centered relatively to feature bar on x axis
        TModelUnit LabelX = visible_range.GetFrom() + visible_range.GetLength() * 0.5;
        TModelUnit LabelY = base;
        if (m_Config->m_LabelPos == CFeatureParams::ePos_Above) {
            // move the base to give space for the text above the feature bar
            base += font_height;
            LabelY = base;
            base += kVertSpace + 1;
        }
        if (IsSelected()) {
            gl.ColorC(m_Context->GetSelLabelColor());
        }  else if (m_Config->m_LabelPos == CFeatureParams::ePos_Inside) {
            gl.ColorC(m_Config->m_fgColor.ContrastingColor());
        } else {
            gl.ColorC(m_Config->m_LabelColor);
        }
        m_Context->TextOut(&font, sLabelTextOut.c_str(), LabelX, LabelY, true, true);
    }
}


void CFeatGlyph::x_DrawInnerLabels(TModelUnit base, vector<TModelRange> *labels_range, TSeqRange* interval) const
{
    if (m_HideLabel)
        return;
    if ((m_Config->m_LabelPos != CFeatureParams::ePos_Side) && (m_Config->m_LabelPos != CFeatureParams::ePos_Inside)) 
        return;
    TSeqRange exon_int;
    if (interval) {
        exon_int = interval->IntersectionWith(m_Context->GetVisSeqRange());
        if (exon_int.Empty())
            return;
    }

    // visible part of the whole glyph (feature bar + label + additional info)
    TModelRange visible_range = m_Context->IntersectVisible(this);

    const CGlTextureFont &font = m_Config->m_LabelFont;

    // location of feature bar, must not be overwritten by the label
    TSeqRange FeatureBarRange(GetRange());
    TModelUnit FeatureBarLeft(FeatureBarRange.GetFrom());
    TModelUnit FeatureBarRight(FeatureBarRange.GetToOpen());

    bool side_lbl{ false };
    if (CFeatureParams::ePos_Side == m_Config->m_LabelPos) {
        // how much of the space (in sequence coords) is available for the label
        TModelUnit LabelVisibleWidth{ 0 };
        if (x_LabelOnLeft()) {
            LabelVisibleWidth = (FeatureBarLeft < visible_range.GetFrom() ? 0 : FeatureBarLeft - visible_range.GetFrom());
        }
        else {
            LabelVisibleWidth = (FeatureBarRight > visible_range.GetTo() ? 0 : visible_range.GetTo() - FeatureBarRight);
        }
        side_lbl = (LabelVisibleWidth >= m_Context->GetMinLabelWidthPos());
    }

    string fl_content;
    GetLabel(fl_content, CLabel::eContent);

    CRgbaColor inner_color(m_Config->m_fgColor);
    GetCustomColor(inner_color);
    if ((GetCustomColorIdx(GetFeature()) == CCustomFeatureColor::eLoss) && (s_GetLiteralLength(GetFeature().GetData().GetVariation()) >= 0))
        inner_color = m_Config->m_bgColor;
    inner_color = inner_color.ContrastingColor();

    bool inside_only = m_Config->m_LabelPos == CFeatureParams::ePos_Inside;
    CSeqGlyph::x_DrawInnerLabels(base, fl_content, inner_color, font, side_lbl, inside_only, labels_range, exon_int.Empty() ? nullptr : &exon_int);
}

void CFeatGlyph::x_DrawLabelWithYPinned(TModelUnit base) const
{
    if(m_Config->m_LabelPos != CFeatureParams::ePos_Side) return;

    IRender& gl = GetGl();

    // visible part of the whole glyph (feature bar + label + additional info)
    TModelRange visible_range_pos = m_Context->IntersectVisible(this);

    const CGlTextureFont &font = m_Config->m_LabelFont;
    TModelUnit font_height = gl.TextHeight(&font);

    // location of feature bar, must not be overwritten by the label
    TSeqRange FeatureBarRange(GetRange());
    TModelUnit FeatureBarLeft(FeatureBarRange.GetFrom());
    // this is an open position i.e. +1 of the real end pos
    TModelUnit FeatureBarRight(FeatureBarRange.GetToOpen());

    // how much of the space (in sequence position coords) is available for the label
    TModelUnit AvailableLabelVisibleWidthPos(x_LabelOnLeft() ?
        (FeatureBarLeft < visible_range_pos.GetFrom() ? 0 : FeatureBarLeft - visible_range_pos.GetFrom())
        :
        (FeatureBarRight > visible_range_pos.GetToOpen() ? 0 :  visible_range_pos.GetToOpen() - FeatureBarRight));

    string fl_content;
    GetLabel(fl_content, CLabel::eContent);

    CRgbaColor inner_color(m_Config->m_fgColor);
    GetCustomColor(inner_color);
    if ((GetCustomColorIdx(GetFeature()) == CCustomFeatureColor::eLoss) && (s_GetLiteralLength(GetFeature().GetData().GetVariation()) >= 0))
        inner_color = m_Config->m_bgColor;
    inner_color = inner_color.ContrastingColor();


    if (AvailableLabelVisibleWidthPos < m_Context->GetMinLabelWidthPos(font)) {
        return;
    }

    // these widths are in screen pixels
    TModelUnit label_width_px = gl.TextWidth(&font, fl_content.c_str());
    TModelUnit max_width_px = min(m_Context->SeqToScreen(AvailableLabelVisibleWidthPos),
        m_Context->GetMaxLabelWidth(font));
    if (label_width_px > max_width_px) {
        label_width_px = max_width_px;
        fl_content = font.Truncate(fl_content.c_str(), label_width_px);
        if ((string::npos != fl_content.find("...")) && (fl_content.length() <= 5))
            return;
    }
    // convert back to sequence position coords
    TModelUnit label_width_pos = m_Context->ScreenToSeq(label_width_px);
    TModelUnit label_x_pos = 0.0;
    TModelUnit label_base = base;
    TModelUnit label_clear_base = base;
    TModelUnit label_clear_height = font_height;
    label_base += (font_height * 0.5);

    // draw background - larger of font height or
    // bar height (if label on same line, e.g. not label first)
    TModelUnit bar_height = m_Config->m_BarHeight;
    label_clear_base = label_base;
    if (bar_height > font_height) {
        label_clear_height = bar_height;
        label_clear_base = floor(base) + (label_clear_height * 0.5);
    }
    if (x_LabelOnLeft()) {
        label_x_pos = visible_range_pos.GetFrom();
        if (label_x_pos > GetLeft()) {
            gl.ColorC(m_Config->m_bgColor);
            m_Context->DrawBackground(
                TModelRect(label_x_pos, label_clear_base + 1, label_x_pos + label_width_pos,
                    label_clear_base - label_clear_height - 1), 0);
        }
    } else {
        label_x_pos = visible_range_pos.GetToOpen() - label_width_pos;
        if (visible_range_pos.GetTo() < GetRight()) {
            gl.ColorC(m_Config->m_bgColor);
            m_Context->DrawBackground(
                TModelRect(label_x_pos, label_clear_base + 1, visible_range_pos.GetToOpen(),
                    label_clear_base - label_clear_height - 1), 0);
        }
    }
    gl.ColorC(IsSelected() ?
        m_Context->GetSelLabelColor() : m_Config->m_LabelColor);

    m_Context->TextOut(&font, fl_content.c_str(), label_x_pos,
        label_base, false, true);
}

void CFeatGlyph::x_DrawAdditionalInfo(TModelUnit base) const
{
    if ( !x_ShowLabel() ) {
        return;
    }

    IRender& gl = GetGl();

    // visible part of the whole glyph (feature bar + label + additional info)
    TModelRange visible_range = m_Context->IntersectVisible(this);
    const CGlTextureFont& font = m_Config->m_LabelFont;
    TModelUnit font_height = gl.TextHeight(&font);

    if(m_Config->m_LabelPos == CFeatureParams::ePos_Side) {
        // location of feature bar, must not be overwritten by the additional info
        TSeqRange FeatureBarRange(GetRange());
        TModelUnit FeatureBarLeft(FeatureBarRange.GetFrom());
        TModelUnit FeatureBarRight(FeatureBarRange.GetToOpen());

        // how much of the space (in sequence coords) is available for the additional info
        // left/right location of additional info is complementary to that of the label
        TModelUnit AvailableAdditionalInfoVisibleWidthPos(!x_LabelOnLeft() ?
                        (FeatureBarLeft < visible_range.GetFrom() ? 0 : FeatureBarLeft - visible_range.GetFrom())
                        :
                        (FeatureBarRight > visible_range.GetToOpen() ? 0 :  visible_range.GetToOpen() - FeatureBarRight));

        string sAdditionalInfo;
        GetAdditionalInfo(sAdditionalInfo);
//        LOG_POST(Trace << "sAdditionalInfo: " << sAdditionalInfo);
//        LOG_POST(Trace << "AvailableAdditionalInfoVisibleWidthPos: " << AvailableAdditionalInfoVisibleWidthPos);
        // widths are in screen pixels
        TModelUnit info_width_px = gl.TextWidth(&font, sAdditionalInfo.c_str());
//        LOG_POST(Trace << "initial info_width_px: " << info_width_px);

        if(AvailableAdditionalInfoVisibleWidthPos < m_Context->GetMinLabelWidthPos()) {
            return;
        }
        TModelUnit max_width_px = min(m_Context->SeqToScreen(AvailableAdditionalInfoVisibleWidthPos),
                                   m_Context->GetMaxLabelWidth(font));
//        LOG_POST(Trace << "max_width_px: " << max_width_px);
        if (info_width_px > max_width_px) {
            info_width_px = max_width_px;
            sAdditionalInfo = font.Truncate(sAdditionalInfo.c_str(), info_width_px);
        }
//        LOG_POST(Trace << "modified sAdditionalInfo: " << sAdditionalInfo << " and info_width_px: " << info_width_px);
        // convert back to sequence coords
        TModelUnit info_width_pos = m_Context->ScreenToSeq(info_width_px);
        TModelUnit info_x_pos = 0.0;
        TModelUnit info_base = base;
        TModelUnit info_clear_base = base;
        TModelUnit info_clear_height = font_height;
        info_base += (font_height * 0.5);

//        LOG_POST(Trace << "info_width_pos: " << info_width_pos);
        // draw background - larger of font height or
        // bar height (if label on same line, e.g. not label first)
        TModelUnit bar_height = m_Config->m_BarHeight;
        info_clear_base = info_base;
        if (bar_height > font_height) {
            info_clear_height = bar_height;
            info_clear_base = floor(base) + (info_clear_height * 0.5);
        }
        if (!x_LabelOnLeft()) {
            info_x_pos = visible_range.GetFrom();
            if (info_x_pos > GetLeft()) {
//                LOG_POST(Trace << " drawing background at left: " << info_x_pos << " with offset: " << m_Context->GetOffset());
                gl.ColorC(m_Config->m_bgColor);
                m_Context->DrawBackground(
                    TModelRect(info_x_pos, info_clear_base + 1, info_x_pos + info_width_pos,
                        info_clear_base - info_clear_height - 1), 0);
            }
        } else {
            info_x_pos = visible_range.GetToOpen() - info_width_pos;
            if (visible_range.GetTo() < GetRight()) {
//                LOG_POST(Trace << " drawing background at right: " << info_x_pos << " with offset: " << m_Context->GetOffset());
                gl.ColorC(m_Config->m_bgColor);
                m_Context->DrawBackground(
                    TModelRect(info_x_pos, info_clear_base + 1, visible_range.GetToOpen(),
                        info_clear_base - info_clear_height - 1), 0);
            }
        }
        gl.ColorC(IsSelected() ?
            m_Context->GetSelLabelColor() : m_Config->m_LabelColor);

//        LOG_POST(Trace << "info_x_pos: " << info_x_pos);
        m_Context->TextOut(&font, sAdditionalInfo.c_str(), info_x_pos,
            info_base, false, true);
    }
}

bool CFeatGlyph::x_IsProjected() const
{
    return m_ProjectedMappingInfo.GetAlignmentDataSource() != nullptr;
}

// This is exactly as it done in Sequin.
TSeqPos CFeatGlyph::x_GetProtOffset() const
{
    _ASSERT(GetFeature().GetData().IsCdregion());

    TSeqPos offset = 1; // translation offset
    
    const CCdregion& cdr = GetFeature().GetData().GetCdregion();
    if (cdr.IsSetFrame()) {
        switch (cdr.GetFrame()) {
        case CCdregion::eFrame_two:
            offset = 2;
            break;
        case CCdregion::eFrame_three:
            offset = 3;
            break;
        default:
            break;
        }
    }
    return offset;
}

CRef<CSeq_loc> CFeatGlyph::x_AdjustFrame(const CSeq_loc &loc, TSeqPos offset) const
{
    CRef<CSeq_loc> new_loc(new CSeq_loc);
    new_loc->Assign(loc);

    CSeq_loc_I loc_it(*new_loc); 
    if (loc_it || loc_it.IsEmpty()) {
        if (loc_it.IsSetStrand() && (eNa_strand_minus == loc_it.GetStrand())) {
            loc_it.SetTo(loc_it.GetRange().GetTo() - offset);
        }
        else {
            loc_it.SetFrom(loc_it.GetRange().GetFrom() + offset);
        }
    }
    CRef<CSeq_loc> result = loc_it.MakeSeq_loc();

    return result;
}

void CFeatGlyph::x_DrawRuler(TModelUnit base, bool protein_scale) const
{
    bool has_product = GetFeature().IsSetProduct();
    if (x_IsProjected() && has_product) {
        x_DrawProjectedRuler(base, protein_scale);
        return;
    }

    bool horz = m_Context->IsHorizontal();
    bool flip_strand = m_Context->IsFlippedStrand();

    IRender& gl = GetGl();

    CRuler ruler_panel;
    ruler_panel.SetFont(CGlTextureFont::eFontFace_Helvetica_Bold, 8);
    ruler_panel.SetGeometryParam (CRuler::eMinorTickHeight, 1);
    ruler_panel.SetGeometryParam (CRuler::eMajorTickHeight, 3);
    ruler_panel.SetGeometryParam (CRuler::eLabelTickHeight, 3);
    ruler_panel.SetGeometryParam (CRuler::eOppMinorTickHeight, 0);
    ruler_panel.SetGeometryParam (CRuler::eOppMajorTickHeight, 0);
    ruler_panel.SetGeometryParam (CRuler::eOppLabelTickHeight, 2);

    ruler_panel.SetHorizontal(horz, horz ? CRuler::eTop : CRuler::eRight);
    int display_options = 0; // don't show background

	TSeqPos offset = 0; // translation offset
    // offset is ignored for CDS features withput product (SV-4266)
    // their mapping info already reflects this offset
    if (protein_scale && GetFeature().GetData().IsCdregion() && has_product) {
        offset = x_GetProtOffset();
        offset -= 1;
    }

    CRef<CPairwiseAln> aln;
    if (!GetMappingInfo().empty() || has_product) {
        const CSeq_loc& loc1 = GetLocation();
        const CSeq_loc& loc2 = has_product ? GetFeature().GetProduct() : GetLocation();
        TAlnSeqIdIRef id1(new CAlnSeqId(*loc1.GetId()));
        TAlnSeqIdIRef id2(new CAlnSeqId(*loc2.GetId()));

        // Sequence type is hardcoded, needs to be fixed:
        id1->SetBaseWidth(1); // location is on a nucleotide
        id2->SetBaseWidth(1); // product is on a protein
        if (m_RulerType & eAaRuler) {
            if (protein_scale) {
                ruler_panel.SetBaseWidth(3);
                ruler_panel.SetTextLabel("aa");
            } else if (m_RulerType & eNtRuler) {
                ruler_panel.SetTextLabel("nt");
            }
            if (!m_Config->m_CgiMode)
                display_options |= CRuler::fFirstLabelHasText;
        }

        // Do we really need to allow overlap? or data are incorrect?
        // Example feature: NP_057849.4 on NC_001802.1
        aln.Reset(new CPairwiseAln(id1, id2,
            CPairwiseAln::fAllowOverlap | CPairwiseAln::fKeepNormalized));
        if (!GetMappingInfo().empty()) {
            CRef<CSeq_loc> gen_loc(new CSeq_loc);
            CRef<CSeq_loc> prod_loc(new CSeq_loc);
            for (auto&& iter : GetMappingInfo()) {
                prod_loc->SetPacked_int().Set().push_back(iter.first);
                gen_loc->SetPacked_int().Set().push_back(iter.second);
            }

            if (offset > 0) {
                CRef<CSeq_loc> adj_gen_loc = x_AdjustFrame(*gen_loc, offset);
                CRef<CSeq_loc> adj_prod_loc = x_AdjustFrame(*prod_loc, offset);
                ConvertSeqLocsToPairwiseAln(*aln, *adj_gen_loc, *adj_prod_loc, CAlnUserOptions::eBothDirections);
            }
            else
                ConvertSeqLocsToPairwiseAln(*aln, *gen_loc, *prod_loc, CAlnUserOptions::eBothDirections);
        } else { 
            if (m_Context->GetScale() > kMinScaleForRulerLabels) // the view is zoomed out and mapping info is not accurate 
                display_options |= CRuler::fHideLabels;
            if (offset > 0) {
                CRef<CSeq_loc> adj_gen_loc = x_AdjustFrame(loc1, offset);
                ConvertSeqLocsToPairwiseAln(*aln, *adj_gen_loc, loc2, CAlnUserOptions::eBothDirections);
            }
            else
                ConvertSeqLocsToPairwiseAln(*aln, loc1, loc2, CAlnUserOptions::eBothDirections);
        }
    } else {
        const CSeq_loc& loc1 = GetLocation();
        if ( !loc1.GetId()  ||  loc1.GetStrand() == eNa_strand_other) {
            return;
        }

        TAlnSeqIdIRef id1(new CAlnSeqId(*loc1.GetId()));

        CSeq_loc loc2;
        loc2.SetWhole();
        loc2.SetId(*loc1.GetId());

        // Sequence type is hardcoded, needs to be fixed:
        id1->SetBaseWidth(1); // location is on a nucleotide

        aln.Reset(new CPairwiseAln(id1, id1,
            CPairwiseAln::fAllowOverlap | CPairwiseAln::fKeepNormalized));
        ConvertSeqLocsToPairwiseAln(*aln, loc1, loc2, CAlnUserOptions::eBothDirections);
    }

    // revesre collention for filled strand
    CRef<CSGSequenceDS> seq_ds = m_Context->GetSeqDS();
    TSeqPos seq_length = seq_ds->GetSequenceLength();
    if (flip_strand) {
        ITERATE(CPairwiseAln, it, *aln)  {
            const CPairwiseAln::TAlnRng& r = *it;
            CPairwiseAln::TAlnRng* r1 = const_cast<CPairwiseAln::TAlnRng*>(&r);
            r1->SetFirstFrom(seq_length-r.GetFirstToOpen());
            r1->SetReversed( !r.IsReversed() );
        }
        aln->Sort();
    }
    ruler_panel.SetMapping(*aln);
    ruler_panel.SetDisplayOptions(display_options);

    CGlPane* pane = m_Context->GetGlPane();
    gl.PushMatrix();
    gl.LoadIdentity();
    pane->Close();
    CGlPane RP(*pane);

    TModelUnit ruler_y = base - GetTop(), ruler_x = 0.0;
    x_Local2World(ruler_x, ruler_y);
    TModelUnit base_top = pane->ProjectY(ruler_y);
    TVPRect vpt = pane->GetViewport();
    vpt.SetTop((int)floor(base_top));
    vpt.SetBottom((int)ceil(base_top - kRulerHeight));
    RP.SetViewport(vpt);

    TModelRect rcV = pane->GetVisibleRect();
    if (flip_strand) {
        rcV.SetLeft(seq_length - rcV.Left());
        rcV.SetRight(seq_length - rcV.Right());
    }
    rcV.SetBottom(0);
    rcV.SetTop(kRulerHeight);
    RP.SetVisibleRect(rcV);

    ruler_panel.Render(RP);
    pane->OpenOrtho();
    gl.PopMatrix();
}

TVPUnit s_AdjustScreenCoordinate(TModelUnit x)
{
    return (TVPUnit)((x > 0.0) ? floor(x + 0.5) : ceil(x - 0.5));
}

void CFeatGlyph::x_DrawProjectedRuler(TModelUnit base, bool protein_scale) const
{

    bool horz = m_Context->IsHorizontal();
    bool flip_strand = m_Context->IsFlippedStrand();

    IRender& gl = GetGl();

    CRuler ruler_panel;
    ruler_panel.SetFont(CGlTextureFont::eFontFace_Helvetica_Bold, 8);
    ruler_panel.SetGeometryParam(CRuler::eMinorTickHeight, 1);
    ruler_panel.SetGeometryParam(CRuler::eMajorTickHeight, 3);
    ruler_panel.SetGeometryParam(CRuler::eLabelTickHeight, 3);
    ruler_panel.SetGeometryParam(CRuler::eOppMinorTickHeight, 0);
    ruler_panel.SetGeometryParam(CRuler::eOppMajorTickHeight, 0);
    ruler_panel.SetGeometryParam(CRuler::eOppLabelTickHeight, 2);

    ruler_panel.SetHorizontal(horz, horz ? CRuler::eTop : CRuler::eRight);
    int display_options = 0; // don't show background
    CRef<CPairwiseAln> aln;

    const CSeq_loc& loc1 = GetLocation();
    const CSeq_loc& loc2 = GetFeature().GetProduct();
    TAlnSeqIdIRef id1(new CAlnSeqId(*loc1.GetId()));
    TAlnSeqIdIRef id2(new CAlnSeqId(*loc2.GetId()));

    auto aln_mgr = m_ProjectedMappingInfo.GetAlignmentDataSource();

    int anchor = aln_mgr->GetAnchor();
    int aligned_seq = aln_mgr->GetQuery();
    // revesre collention for filled strand
    TSeqPos seq_length = aln_mgr->GetSeqLength(aligned_seq);
    int base_width = aln_mgr->GetBaseWidth(anchor);

    // Sequence type is hardcoded, needs to be fixed:
    id1->SetBaseWidth(1); // location is on a nucleotide
    id2->SetBaseWidth(1); // location is on a nucleotide
    if (m_RulerType & eAaRuler) {
        if (protein_scale) {
            ruler_panel.SetBaseWidth(3);
            ruler_panel.SetTextLabel("aa");
        } else if (m_RulerType & eNtRuler) {
            ruler_panel.SetTextLabel("nt");
        }
        if (!m_Config->m_CgiMode)
            display_options |= CRuler::fFirstLabelHasText;
    }
    if (m_Context->GetScale() > kMinScaleForRulerLabels) // the view is zoomed out and mapping info is not accurate 
        display_options |= CRuler::fHideLabels;

    ruler_panel.SetDisplayOptions(display_options);

    CGlPane* pane = m_Context->GetGlPane();
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    gl.PushMatrix();
    gl.LoadIdentity();
    pane->Close();

    TModelUnit ruler_y = base - GetTop(), ruler_x = 0.0;
    x_Local2World(ruler_x, ruler_y);
    TModelUnit base_top = pane->ProjectY(ruler_y);

    bool anchor_neg = aln_mgr->IsNegativeStrand(anchor);
    bool aligned_neg = aln_mgr->IsNegativeStrand(aligned_seq);

    ITERATE(CProjectedMappingInfo, iter, m_ProjectedMappingInfo)
    {

        auto& anchor_range = iter->m_AnchorRange;
        if (m_Context->IntersectVisible(anchor_range).Empty())
            continue;
        auto& gen_int = *iter->m_MappedInt;
        TSignedSeqPos gen_from = gen_int.GetFrom();
        TSignedSeqPos gen_to = gen_int.GetTo();
        auto& prod_int = *iter->m_ProductInt;
        auto& prod_range = iter->m_MappedProdRange;

        bool flip = (flip_strand != (anchor_neg || aligned_neg));

        CRef<CSeq_loc> prod_loc(new CSeq_loc);
        auto prod_interval = Ref(new CSeq_interval());
        prod_interval->SetId().Assign(prod_int.GetId());
        prod_interval->SetFrom(prod_range.GetFrom());
        prod_interval->SetTo(prod_range.GetTo());
        prod_interval->SetStrand(prod_int.GetStrand());
        prod_loc->SetPacked_int().Set().push_back(prod_interval);

        CRef<CSeq_loc> gen_loc(new CSeq_loc);
        auto mapped_interval = Ref(new CSeq_interval());
        mapped_interval->SetId().Assign(gen_int.GetId());
        mapped_interval->SetFrom(gen_from);
        mapped_interval->SetTo(gen_to);
        mapped_interval->SetStrand(gen_int.GetStrand());

        gen_loc->SetPacked_int().Set().push_back(mapped_interval);

        aln.Reset(new CPairwiseAln(id1, id2,
            CPairwiseAln::fAllowOverlap | CPairwiseAln::fKeepNormalized));
        ConvertSeqLocsToPairwiseAln(*aln, *gen_loc, *prod_loc,
                                    CAlnUserOptions::eBothDirections);
        if (flip) {
            ITERATE(CPairwiseAln, it, *aln)
            {
                const CPairwiseAln::TAlnRng& r = *it;
                CPairwiseAln::TAlnRng* r1 = const_cast<CPairwiseAln::TAlnRng*>(&r);
                r1->SetFirstFrom(seq_length - r.GetFirstToOpen());
                r1->SetReversed(!r.IsReversed());
            }
            aln->Sort();
        }
        ruler_panel.SetMapping(*aln);
        TModelUnit seq_from = anchor_range.GetFrom();
        TModelUnit seq_to = min<int>(anchor_range.GetTo(), seq_from + (prod_range.GetLength() / base_width) - 1);

        if (m_Context->IsFlippedStrand()) {
            swap(seq_from, seq_to);
            seq_from = m_Context->GetOffset() - seq_from - 1;
            seq_to = m_Context->GetOffset() - seq_to- 1;
        } else {
            seq_from -= m_Context->GetOffset();
            seq_to -= m_Context->GetOffset();
        }

        TVPUnit view_from = s_AdjustScreenCoordinate(m_Context->SeqToScreen(seq_from));
        TVPUnit view_to = s_AdjustScreenCoordinate(m_Context->SeqToScreen(seq_to + 1));

        TModelRect rcV = pane->GetVisibleRect();
        rcV.SetBottom(0);
        rcV.SetTop(kRulerHeight);

        rcV.SetHorz(gen_from, gen_to + 1);
        if (flip) {
            rcV.SetHorz(rcV.Right(), rcV.Left());
            rcV.SetLeft(seq_length - rcV.Left());
            rcV.SetRight(seq_length - rcV.Right());
        }
        CGlPane RP(*pane);

        RP.SetModelLimitsRect(rcV);
        RP.SetVisibleRect(rcV);

        TVPRect vpt = pane->GetViewport();
        vpt.SetTop((int)floor(base_top));
        vpt.SetBottom((int)ceil(base_top - kRulerHeight));
        vpt.SetLeft(view_from);
        vpt.SetRight(view_to);
        RP.SetViewport(vpt);
        ruler_panel.Render(RP);
    }
    pane->OpenOrtho();
    gl.PopMatrix();
    glPopAttrib();
}

bool CFeatGlyph::x_CanShowStrand(ENa_strand strand) const
{
    return m_Config->m_ShowStrandIndicator && (!m_Context->IsOverviewMode() || !m_HideLabel)
    && strand != eNa_strand_both && strand != eNa_strand_both_rev;
}

void CFeatGlyph::x_DrawFeatureBar(TModelUnit& base) const
{
    IRender& gl = GetGl();

    TModelUnit bar_height = m_Config->m_BarHeight;
    if (m_Context->IsOverviewMode()  &&  m_HideLabel) {
        bar_height = floor(bar_height * m_Config->m_OverviewFactor);
    }
    TModelUnit head_height = bar_height * m_Config->m_HeadHeight;
    TModelUnit tail_height = bar_height * m_Config->m_TailHeight;
    TModelUnit head_size = m_Context->ScreenToSeq(head_height);
    TModelUnit tail_size = m_Context->ScreenToSeq(tail_height);

    if (m_RSite && (m_Context->SeqToScreen(kRSiteMarkerWidth) >= kRSiteMarkerThreshold) && (!x_ShowLabel() || (m_Config->m_LabelPos == CFeatureParams::ePos_Side)))
        base += bar_height*kRSiteMarkerHeight; // Add the height of the top cut marker

    TModelUnit YCenterLine = base;
    TModelUnit BoundaryYLow = YCenterLine - bar_height * 0.5f;
    TModelUnit BoundaryYHigh = YCenterLine + bar_height * 0.5f;

    auto strand = sequence::GetStrand(GetLocation());
    bool neg_strand = (strand == eNa_strand_minus);

    EFeatureParts head_style = eHead_No;
    EFeatureParts tail_style = eTail_No;

    CRgbaColor color;
    if ( !GetCustomColor(color) ) {
        color = m_Config->m_fgColor;

        // For introns (line-style) may have special color:
        if (m_Config->m_BoxStyle == CFeatureParams::eBox_Line && !IsConsensus())
            color = m_Config->m_NonConsensus;
    }
    // Set user defined color
    if (GetFeature().GetData().IsRegion()) {
        CConstRef< CUser_object > display_settings = GetFeature().FindExt("DisplaySettings");
        if (!display_settings.Empty()) {
            x_GetUserColor(*display_settings, color);
        }
    }

    bool is_pseudo = CSeqUtils::IsPseudoFeature(GetFeature());
    if (is_pseudo) {
        color.SetAlpha(0.8f);
    }
    CRgbaColor color_lite = color;
    color_lite.Lighten(0.2f);

    // distance among strand indicators
    TModelUnit pix_size = m_Context->ScreenToSeq(1.0);
    TModelUnit apart = 100.0 * pix_size;    // 100 pixels on screen

    // Draw feature bar
    TSeqRange SeqRange = GetRange();
    TSeqPos from = SeqRange.GetFrom();
    TSeqPos to = SeqRange.GetTo();

    if (SeqRange.GetLength() > pix_size * 2) {  // At least 2 pixel
        TModelUnit prev_to_x = 0.0f;
        TModelUnit prev_from_x = 0.0f;

        bool first_pass = true;
        // draw exons and introns

        // Ranges, occupied by labels
        vector<TModelRange> labels_range;
        // Previous exon range
        TModelRange prev_exon_rng;
        // Range to draw strands on
        TModelRange strand_rng;

        bool has_accessory_label = false;
        CRgbaColor strand_indicator_color = color;

        // Determine strand indicator color
        switch (m_Config->m_BoxStyle) {
        case CFeatureParams::eBox_Filled:
        case CFeatureParams::eBox_Hollow:
            strand_indicator_color = m_Config->m_fgColor.ContrastingColor();
            break;
        case CFeatureParams::eBox_Insertion:
        case CFeatureParams::eBox_Deletion:
            break;
        default:
            strand_indicator_color = color.ContrastingColor(false);
            // Line style means arrows are on a white background so should be darker.
            strand_indicator_color.Darken(0.1f);
            break;
        }

        vector<CConstRef<CSeq_loc>> locs;
        CSeq_loc_CI iter(GetLocation());
        for (; iter; ++iter) {
            locs.push_back(iter.GetRangeAsSeq_loc());
        }
        if (!m_RSite) {
            sort(locs.begin(), locs.end(), [](CConstRef<CSeq_loc>& a, CConstRef<CSeq_loc>& b) {
                return a->GetTotalRange().GetFrom() < b->GetTotalRange().GetFrom();
            });
        }

        for (const auto& curr_loc : locs) {
            TSeqRange curr = curr_loc->GetTotalRange();

            // Restriction sites have location mix, consisting of two points and an interval
            strand = sequence::GetStrand(*curr_loc);
            neg_strand = (strand == eNa_strand_minus);

            TModelUnit f = curr.GetFrom();
            TModelUnit t = curr.GetTo();

            bool rsiteCutPnt = false;
            if (m_RSite) {
                if (curr_loc->IsPnt()) {
                    const CSeq_point& pnt = curr_loc->GetPnt();
                    if (pnt.CanGetFuzz()) {
                        const CSeq_point_Base::TFuzz &fuzz = pnt.GetFuzz();
                        if (fuzz.IsLim() && (CInt_fuzz_Base::eLim_tl == fuzz.GetLim())) {
                            rsiteCutPnt = true;
                        }
                    }
                }
            }

            // adjust the size of exons to include style elements
            // such as head and tail pieces. Left (from)  or right (to) sides
            // are adjected according to strand
            // 1) adjust head
            bool fit      = head_size * 1.5 < fabs(t - f);
            bool adj_to   = !neg_strand  &&  t == to  &&  fit;
            bool adj_from = neg_strand  &&  f == from  &&  fit;

            if (adj_from || adj_to) {
                head_style = eNeedHead;
            }

            switch (m_Config->m_HeadStyle) {
            case CFeatureParams::eHead_Arrow:
            case CFeatureParams::eHead_NeckedTriangle:
                if (adj_to) {
                    t = to - head_size * 1.5;
                }
                if (adj_from) {
                    f = from + head_size * 1.5;
                }
                break;
            case CFeatureParams::eHead_Triangle:
                if (adj_to) {
                    t = to - head_size;
                }
                if (adj_from) {
                    f = from + head_size;
                }
                break;

            default:
                break;
            }

            // 2) adjust tail
            fit      = tail_size < fabs(t - f);
            adj_to   = neg_strand  &&  t == to  &&  fit;
            adj_from = !neg_strand  &&  f == from  &&  fit;

            if (adj_from || adj_to) {
                tail_style = eNeedTail;
            }

            switch (m_Config->m_TailStyle) {
            case CFeatureParams::eTail_Circle:
                if (adj_from) {
                    f = from + tail_size * 0.5f;
                }
                if (adj_to) {
                    t = to - tail_size * 0.5f;
                }
                break;
            case CFeatureParams::eTail_Square:
                if (adj_from) {
                    f = from + tail_size;
                }
                if (adj_to) {
                    t = to - tail_size;
                }
                break;
            case CFeatureParams::eTail_Fletched:
                if (adj_from) {
                    f = from + tail_size; //-1
                }
                if (adj_to) {
                    t = to - tail_size;
                }
                break;

            default:
                break;
            }

            // draw introns first

            TModelUnit from_x = curr.GetFrom();
            TModelUnit to_x = curr.GetTo() + 1;

            if (!first_pass) {
                //TModelUnit off = (m_Config->m_LineThickness) * 0.5f;
                TModelUnit intron_f;
                TModelUnit intron_t;
                if (!m_RSite) {
                    intron_f = prev_to_x;
                    intron_t = from_x;
                    /*
                    // In some cases, the intervals are not correctly ordered
                    // for the feature. Maybe, it is a circular sequence, we
                    // just can't handle it correctly.
                    bool correctly_ordered = (neg_strand == (curr.GetFrom() < (prev_to_x - 1)));
                    if (correctly_ordered) {
                        intron_f = prev_to_x;
                        intron_t = from_x;
                    } else {
                        intron_t = prev_from_x;
                        intron_f = to_x;
                    }
                    if (neg_strand)
                        swap(intron_f, intron_t);
                    */
                } else { // The workaround above corrupts restriction sites
                    intron_f = prev_from_x;
                    intron_t = from_x;
                    if (intron_f > intron_t)
                        swap(intron_f, intron_t);
                }
                
                if (intron_f < intron_t) {
                    glPushAttrib(GL_LINE_BIT);
                    switch (m_Config->m_LineStyle) {
                    case CFeatureParams::eLine_Dashed:
                        gl.LineStipple(1, 0x0F0F);
                        gl.Enable(GL_LINE_STIPPLE);
                        break;
                    case CFeatureParams::eLine_Dotted:
                        gl.LineStipple(1, 0x0202);
                        gl.Enable(GL_LINE_STIPPLE);
                        break;
                    case CFeatureParams::eLine_DashDotted:
                        gl.LineStipple(1, 0x1C47);
                        gl.Enable(GL_LINE_STIPPLE);
                        break;
                    case CFeatureParams::eLine_ShortDashed:  //Repeating pairs: --  -- --
                        gl.LineStipple(2, 0xAAAA);
                        gl.Enable(GL_LINE_STIPPLE);
                        break;
                    case CFeatureParams::eLine_Solid:
                        break;
                    }  // m_Config->m_LineStyle


                    gl.LineWidth((float)(m_Config->m_LineWidth));
                    gl.ColorC(color_lite);
                    switch (m_Config->m_Connections) {
                    case CFeatureParams::eCantedLine:
                    {{
                            TModelUnit middle_x =
                                intron_f + (intron_t - intron_f) * 0.5f;
                            m_Context->DrawLine(intron_f, YCenterLine, middle_x, BoundaryYLow);
                            m_Context->DrawLine(middle_x, BoundaryYLow, intron_t, YCenterLine);
                        }}
                    break;
                    case CFeatureParams::eBox:
                    {{
                            // if short, may look like we are drawing on one side or not at all
                            if (m_Context->SeqToScreen(fabs(intron_t - intron_f)) < TModelUnit(6.0))
                                gl.Disable(GL_LINE_STIPPLE);

                            m_Context->DrawLine(intron_f, BoundaryYLow, intron_t, BoundaryYLow);
                            m_Context->DrawLine(intron_f, BoundaryYHigh, intron_t, BoundaryYHigh);
                        }}
                    break;
                    case CFeatureParams::eStraightLine:
                        m_Context->DrawLine(intron_f, YCenterLine, intron_t, YCenterLine);
                        break;
                    case CFeatureParams::eFilledBox:
                        gl.Disable(GL_LINE_STIPPLE);
                        m_Context->Draw3DQuad(intron_f, BoundaryYLow, intron_t,
                                              BoundaryYHigh, color_lite);
                        break;
                    }  // m_Config->m_Connections
                    gl.Disable(GL_LINE_STIPPLE);
                    gl.LineWidth(1.0f);
                    glPopAttrib();

                    if (x_CanShowStrand(strand) && !rsiteCutPnt) {
                        m_Context->DrawStrandIndicators(
                            TModelPoint(intron_f, BoundaryYLow),
                            intron_t - intron_f, apart, bar_height,
                            CRgbaColor(0.7f, 0.7f, 0.7f), neg_strand);
                    }
                }

            }  // connection lines (introns)

            if (is_pseudo) {
                CRgbaColor lcolor = color;
                lcolor.Lighten(0.4f);
                CRgbaColor dcolor = color;
                dcolor.Darken(0.7f);

                m_Context->DrawPseudoBar(f, BoundaryYLow, t + 1, BoundaryYHigh, lcolor, dcolor);
            }

            // draw features as quads, hollow rectangles or just lines as configured

            if (!rsiteCutPnt) { // Don't draw bar if this is RSite cut point
                if(m_Config->m_BoxStyle == CFeatureParams::eBox_Filled) {
                    m_Context->Draw3DQuad(f, BoundaryYLow, t, BoundaryYHigh, color);
//                    LOG_POST(Trace << " drawing eBox_Filled feature at left: " << f <<
//                                    " and right: " << t);

                    // Draw SNPs separators
                    gl.ColorC(m_Config->m_bgColor);
                    if (m_Neighbours & eNghbrs_Left)
                        m_Context->DrawLine(f, BoundaryYLow, f, BoundaryYHigh);

                    if (m_Neighbours & eNghbrs_Right)
                        m_Context->DrawLine(t + 1, BoundaryYLow, t + 1, BoundaryYHigh);

                } else if(m_Config->m_BoxStyle == CFeatureParams::eBox_Hollow) {
                    gl.ColorC(color);
                    m_Context->DrawRect(f, BoundaryYLow, t+1, BoundaryYHigh);
                } else if(m_Config->m_BoxStyle == CFeatureParams::eBox_Insertion) {
                    m_Context->Draw3DQuad(f, BoundaryYLow, t, BoundaryYHigh, color);
                    CRgbaColor color_insert("black");
                    gl.ColorC(color_insert);
                    x_DrawInsertion((f + t) * 0.5 + 0.5, BoundaryYLow, BoundaryYHigh, YCenterLine);
                } else if(m_Config->m_BoxStyle == CFeatureParams::eBox_Deletion) {
                    m_Context->Draw3DQuad(f, BoundaryYLow, t, BoundaryYHigh, color);
                    CRgbaColor color_delete("black");
                    gl.ColorC(color_delete);
                    x_DrawDeletion((f + t) * 0.5 + 0.5, BoundaryYLow, BoundaryYHigh);
                } else {
                    gl.LineWidth(1.0f);
                    gl.Disable(GL_LINE_SMOOTH);
                    gl.ColorC(color);
                    // draw line slightly off center to make room for (possible)label
                    m_Context->DrawLine(f, YCenterLine+1, t+1, YCenterLine+1);

                    // Draw simple line tick marks.
                    m_Context->DrawLine(f, BoundaryYLow, f, BoundaryYHigh);
                    m_Context->DrawLine(t+1, BoundaryYLow, t+1, BoundaryYHigh);
                    gl.Enable(GL_LINE_SMOOTH);
                }

                TModelRange exon_rng(f, t);
                if (prev_exon_rng.Empty())
                    prev_exon_rng = exon_rng;
                else {
                    if (prev_exon_rng.IntersectingWith(exon_rng) || prev_exon_rng.AbuttingWith(exon_rng)) // Merge the ranges, if they overlap
                        prev_exon_rng.CombineWith(exon_rng);
                    else {
                        TSeqRange rng(prev_exon_rng.GetFrom(), prev_exon_rng.GetTo());
                        if (rng.GetLength() > 1) {
                            x_DrawInnerLabels(base, &labels_range, &rng);
                        }
                        strand_rng = prev_exon_rng;
                        prev_exon_rng = exon_rng;
                    }
                }
            }

            // Check for accessory label but do not draw it until after the strand
            // indicators (strand indicators on top of label make it hard to read).
            string accessory_label;
            TModelUnit clipped_from = std::min(t,f);
            TModelUnit clipped_to = std::max(t,f);
            const CGlTextureFont& font = m_Config->m_LabelFont;
            TModelUnit w = TModelUnit(0);

            if (first_pass) {
                GetAccessoryLabel(accessory_label);
                if (accessory_label != "") {
                    TModelRect r = m_Context->GetGlPane()->GetVisibleRect();
                    w = m_Context->ScreenToSeq(font.TextWidth(accessory_label.c_str()));

                    // Find SeqRange that is visible inside current view. Rect can be flipped (left>right)
                    TModelUnit clipped_from = std::min(t,f);
                    TModelUnit clipped_to = std::max(t,f);
                    if (r.Right() > r.Left()) {
                        clipped_from = std::max(r.Left(), clipped_from);
                        clipped_to = std::min(r.Right(), clipped_to);
                    }
                    else {
                        clipped_to = std::min(r.Left(), clipped_to);
                        clipped_from = std::max(r.Right(), clipped_from);
                    }

                    if (clipped_to-clipped_from > TModelUnit(1.5)*w) {
                        has_accessory_label = true;
                    }
                }
            }

            // draw strand indicators
            if (x_CanShowStrand(strand) && !strand_rng.Empty()) {
                m_Context->DrawStrandIndicators(
                    TModelPoint(strand_rng.GetFrom(), BoundaryYLow), fabs(strand_rng.GetToOpen() - strand_rng.GetFrom()), apart, bar_height,
                    strand_indicator_color, neg_strand,
                    m_Config->m_LabelPos == CFeatureParams::ePos_Inside  &&
                    !m_HideLabel, has_accessory_label, &labels_range);
                strand_rng.Set(0, 0);
            }

            if (first_pass && has_accessory_label) {
                gl.ColorC(color);
                m_Context->TextOut(&font, accessory_label.c_str(),
                    ((clipped_from+clipped_to)/TModelUnit(2)) - w/TModelUnit(2),
                    YCenterLine, false, true);
            }

            // draw partial loc indicators
            bool p_start = curr_loc->IsPartialStart(objects::eExtreme_Positional);
            bool p_stop = curr_loc->IsPartialStop(objects::eExtreme_Positional);
            if (p_start  ||  p_stop) {
                m_Context->DrawPartialBar(curr.GetFrom(), BoundaryYLow,
                    curr.GetToOpen(), BoundaryYHigh, p_start, p_stop, true);
            }

            prev_to_x  = to_x;
            prev_from_x = m_RSite ? from_x : from_x - 1;
            first_pass = false;
        }
        if (x_CanShowStrand(strand)) {

            // Draw head  (line-style boxes already have tick marks at end -don't add another.)
            if (head_style == eNeedHead && m_Config->m_BoxStyle != CFeatureParams::eBox_Line) {
                gl.LineWidth((float)(m_Config->m_LineWidth + 0.1));

                switch (m_Config->m_HeadStyle) {
                case CFeatureParams::eHead_NeckedTriangle:
                    m_Context->Draw3DTriangle(
                        TModelPoint(neg_strand ? from : to + 1, YCenterLine),
                        head_height, color, neg_strand, true); // with neck
                    break;
                case CFeatureParams::eHead_Triangle:
                    m_Context->Draw3DTriangle(
                        TModelPoint(neg_strand ? from : to + 1, YCenterLine),
                        head_height, color, neg_strand, false);
                    break;
                case CFeatureParams::eHead_Arrow:
                    gl.ColorC(color);
                    m_Context->Draw3DArrow(
                        TModelPoint(neg_strand ? from : to + 1, YCenterLine),
                        head_height, neg_strand);
                    break;

                default:
                    break;
                }  // m_Config->m_HeadStyle
                gl.LineWidth(1.0f);
            }  // eNeedHead

            // Draw tail (line-style boxes already have tick marks at end -don't add another.)
            if (tail_style == eNeedTail && m_Config->m_BoxStyle != CFeatureParams::eBox_Line) {
                gl.LineWidth((float)(m_Config->m_LineWidth + 0.1f));
                switch (m_Config->m_TailStyle) {
                case CFeatureParams::eTail_Square:
                    m_Context->DrawSquare(
                        TModelPoint(neg_strand ? to + 1.0 : from, YCenterLine),
                        tail_height, color, neg_strand);
                    break;
                case CFeatureParams::eTail_Circle:
                    m_Context->DrawDisk(
                        TModelPoint(neg_strand ? to + 1 : from, YCenterLine),
                        tail_height, color, neg_strand);
                    break;
                case CFeatureParams::eTail_Fletched:
                    gl.LineWidth(1.5f);
                    m_Context->Draw3DFletch(
                        neg_strand ? to + 1 : from, BoundaryYLow, BoundaryYHigh,
                        tail_height, color, neg_strand);
                    break;

                default:
                    break;
                }  // m_Config->m_TailStyle
                gl.LineWidth(1.0f);
            }  // eNeedTail
        }

        // draw partial feature indicators
        if (CSeqUtils::IsPartialFeature(GetFeature())) {
            m_Context->DrawPartialBar(from, BoundaryYLow, to + 1,
                BoundaryYHigh, true, true, false);
        } else {
            bool p_start = CSeqUtils::IsPartialStart(GetFeature().GetLocation());
            bool p_stop = CSeqUtils::IsPartialStop(GetFeature().GetLocation());
            if (p_start || p_stop) {
                if (neg_strand) swap(p_start, p_stop);
                m_Context->DrawPartialBar(from, BoundaryYLow, to + 1,
                    BoundaryYHigh, p_start, p_stop, true);
            }
        }
        if (m_RSite)
            x_DrawRSites(m_Config->m_fgColor, BoundaryYLow, BoundaryYHigh);

        if (!prev_exon_rng.Empty()) {
            TSeqRange rng(prev_exon_rng.GetFrom(), prev_exon_rng.GetToOpen());
            x_DrawInnerLabels(base, &labels_range, &rng);
            if (x_CanShowStrand(strand)) {
                m_Context->DrawStrandIndicators(
                    TModelPoint(prev_exon_rng.GetFrom(), BoundaryYLow), fabs(prev_exon_rng.GetToOpen() - prev_exon_rng.GetFrom()), apart, bar_height,
                    strand_indicator_color, neg_strand,
                    m_Config->m_LabelPos == CFeatureParams::ePos_Inside  &&
                    !m_HideLabel, has_accessory_label, &labels_range);
            }

        }
    } else {
        // bar is less then 2 pixel. Do not draw intervals.
        m_Context->Draw3DQuad(from, BoundaryYLow, to, BoundaryYHigh, color, true);
    }
}

void CFeatGlyph::x_DrawInsertion(TModelUnit SeqPosTriangleMidPointX,
                                 TModelUnit BoundaryYLow,
                                 TModelUnit BoundaryYHigh,
                                 TModelUnit YCenterLine) const
{
    // half the insertion triangle width in sequence coordinates
    TModelUnit SeqTriangleHalf = m_Context->ScreenToSeq(4.0);

    TModelUnit TriangleOffsetY = 1.0;
    m_Context->DrawLine(SeqPosTriangleMidPointX, BoundaryYLow, SeqPosTriangleMidPointX, BoundaryYHigh);
    m_Context->DrawTriangle(SeqPosTriangleMidPointX, YCenterLine,
        SeqPosTriangleMidPointX + SeqTriangleHalf, BoundaryYLow - TriangleOffsetY,
        SeqPosTriangleMidPointX - SeqTriangleHalf, BoundaryYLow - TriangleOffsetY);
    m_Context->DrawTriangle(SeqPosTriangleMidPointX, YCenterLine,
        SeqPosTriangleMidPointX + SeqTriangleHalf, BoundaryYHigh + TriangleOffsetY,
        SeqPosTriangleMidPointX - SeqTriangleHalf, BoundaryYHigh + TriangleOffsetY);
}

void CFeatGlyph::x_DrawDeletion(TModelUnit SeqPosTriangleMidPointX,
                                 TModelUnit BoundaryYLow,
                                 TModelUnit BoundaryYHigh) const
{
    // half the insertion triangle width in sequence coordinates
    TModelUnit SeqTriangleHalf = m_Context->ScreenToSeq(4.0);

    TModelUnit TriangleOffsetY = 1.0;
    m_Context->DrawTriangle(SeqPosTriangleMidPointX, BoundaryYHigh + TriangleOffsetY,
        SeqPosTriangleMidPointX + SeqTriangleHalf, BoundaryYLow - TriangleOffsetY,
        SeqPosTriangleMidPointX - SeqTriangleHalf, BoundaryYLow - TriangleOffsetY);
}



void CFeatGlyph::x_DrawFeatureBar_sv(TModelUnit base) const
{
    IRender& gl = GetGl();

    TModelUnit bar_height = m_Config->m_BarHeight;
    if (m_Context->IsOverviewMode()  &&  m_HideLabel) {
        bar_height = floor(bar_height * m_Config->m_OverviewFactor);
    }
    TModelUnit YCenterLine = base;
    TModelUnit BoundaryYLow = YCenterLine - bar_height * 0.5f;
    TModelUnit BoundaryYHigh = YCenterLine + bar_height * 0.5f;

    CRgbaColor color;
    if ( !GetCustomColor(color) ) {
        color = m_Config->m_fgColor;
    }
    // a lighter version of color
    CRgbaColor color_t = color;
    color_t.SetAlpha(0.35f);

    // Draw feature bar
    TSeqRange SeqRange = GetRange();
    TSeqPos SeqPosFrom = SeqRange.GetFrom();
    TSeqPos SeqPosTo = SeqRange.GetTo();

    if (SeqRange.GetLength() > m_Context->ScreenToSeq(2.0)) {  // At least 2 pixel
        size_t idx = GetCustomColorIdx(GetFeature());
        int literal_len =
            s_GetLiteralLength(GetFeature().GetData().GetVariation());

        TModelUnit prev_to_x = 0.0f;
        TModelUnit prev_from_x = 0.0f;

        bool first_pass = true;
        for (CSeq_loc_CI iter(GetLocation());  iter;  ++iter) {
            CConstRef<CSeq_loc> loc = iter.GetRangeAsSeq_loc();
            SeqRange = iter.GetRange();
            SeqPosFrom = SeqRange.GetFrom();
            SeqPosTo = SeqRange.GetTo();
            TSeqPos SeqPosFromIn = SeqPosFrom;
            TSeqPos SeqPosToIn = SeqPosTo;

            // initialize defined breakpoint SeqRanges
            // inner start/stop, and outer start/stop
            if (loc->IsInt()) {
                if (loc->GetInt().IsSetFuzz_from()  &&
                    loc->GetInt().GetFuzz_from().IsRange()) {
                        const CInt_fuzz::C_Range& f_SeqRange =
                            loc->GetInt().GetFuzz_from().GetRange();
                        SeqPosFrom = f_SeqRange.GetMin();
                        SeqPosFromIn = f_SeqRange.GetMax();
                }
                if (loc->GetInt().IsSetFuzz_to()  &&
                    loc->GetInt().GetFuzz_to().IsRange()) {
                        const CInt_fuzz::C_Range& t_SeqRange =
                            loc->GetInt().GetFuzz_to().GetRange();
                        SeqPosTo = t_SeqRange.GetMax();
                        SeqPosToIn = t_SeqRange.GetMin();
                }
            }

            // draw the connecting line between intervals
            if ( !first_pass ) {
                // use a lighter color
                CRgbaColor color_lite = color;
                color_lite.Lighten(0.2f);
                TModelUnit intron_f = SeqPosFrom > prev_to_x ? prev_to_x + 1 : SeqPosTo + 1;
                TModelUnit intron_t = SeqPosFrom > prev_to_x ? SeqPosFrom : prev_from_x;
                gl.LineWidth((float)(m_Config->m_LineWidth));
                gl.ColorC(color_lite);
                m_Context->DrawLine(intron_f, YCenterLine, intron_t, YCenterLine);
                gl.LineWidth(1.0f);
            }

            EUndefinedBpType UndefinedBreakpointFrom = x_GetUndefinedBp_from(*loc);
            EUndefinedBpType UndefinedBreakpointTo = x_GetUndefinedBp_to(*loc);

            if (idx == CCustomFeatureColor::eLoss  &&  literal_len >= 0) {
                if (literal_len >= (int)(SeqPosToIn - SeqPosFromIn)) {
                    literal_len = SeqPosToIn - SeqPosFromIn - 2;
                }
                TModelUnit xm = (SeqPosFromIn + SeqPosToIn) * 0.5;
                TModelUnit m = (SeqPosFromIn + SeqPosToIn - literal_len) * 0.5;
                TModelUnit n = (SeqPosFromIn + SeqPosToIn + literal_len) * 0.5;
                gl.ColorC(color_t);
                m_Context->DrawQuad(SeqPosFromIn, BoundaryYLow, m + 1.0, BoundaryYHigh);
                m_Context->DrawQuad(n, BoundaryYLow, SeqPosToIn + 1.0, BoundaryYHigh);

                m += 1.0;
                TModelUnit half_w = m_Context->ScreenToSeq(3.0);
                if (literal_len < half_w * 2.0) {
                    half_w = literal_len * 0.5;
                }

                vector<TModelPoint> points;
                points.push_back(TModelPoint(m, YCenterLine));
                points.push_back(TModelPoint(xm - half_w, YCenterLine));
                points.push_back(TModelPoint(xm - half_w * 0.5, BoundaryYLow - 1.0));
                points.push_back(TModelPoint(xm + half_w * 0.5, BoundaryYHigh + 1.0));
                points.push_back(TModelPoint(xm + half_w, YCenterLine));
                points.push_back(TModelPoint(n, YCenterLine));
                gl.Color3f(0.0f, 0.0f, 0.0f);
                m_Context->DrawLineStrip(points);

            } else if (idx == CCustomFeatureColor::eInsertion ||
                (idx == CCustomFeatureColor::eGain  &&  literal_len >= 0)) {

                gl.ColorC(color_t);
                m_Context->DrawQuad(SeqPosFromIn, BoundaryYLow, SeqPosToIn + 1.0, BoundaryYHigh);
                // draw insertion
                CRgbaColor color_insert;
                GetCustomColor(CCustomFeatureColor::eInsertion, color_insert);
                gl.ColorC(color_insert);
                x_DrawInsertion((SeqPosFromIn + SeqPosToIn) * 0.5 + 0.5, BoundaryYLow, BoundaryYHigh, YCenterLine);
            } else if (idx == CCustomFeatureColor::eDeletionInsertion) {
                // draw deletion
                gl.ColorC(color);
                m_Context->DrawQuad(SeqPosFromIn, BoundaryYLow, SeqPosToIn + 1.0, BoundaryYHigh);
                // draw insertion
                CRgbaColor color_insert;
                GetCustomColor(CCustomFeatureColor::eInsertion, color_insert);
                gl.ColorC(color_insert);
                x_DrawInsertion(SeqPosToIn + 1.0, BoundaryYLow, BoundaryYHigh, YCenterLine);
            } else if (idx == CCustomFeatureColor::eTranschr) {
                m_Context->Draw3DQuad_HorzLines(SeqPosFromIn, BoundaryYLow, SeqPosToIn + 1.0, BoundaryYHigh, color, true);
            } else if (UndefinedBreakpointFrom == eBp_Outer || UndefinedBreakpointFrom == eBp_Outer) {
                gl.ColorC(color_t);
                m_Context->DrawQuad(SeqPosFromIn, BoundaryYLow, SeqPosToIn + 1.0, BoundaryYHigh);
            } else {
                m_Context->Draw3DQuad(SeqPosFromIn, BoundaryYLow, SeqPosToIn, BoundaryYHigh, color);
            }

            // render defined breakpoint SeqRanges
            if (SeqPosFromIn != SeqPosFrom) {
                gl.ColorC(color_t);
                m_Context->DrawQuad(SeqPosFrom, BoundaryYLow, SeqPosFromIn, BoundaryYHigh);
            }
            if (SeqPosToIn != SeqPosTo) {
                gl.ColorC(color_t);
                m_Context->DrawQuad(SeqPosToIn + 1, BoundaryYLow, SeqPosTo, BoundaryYHigh);
            }

            // render undefined breakpoint SeqRanges indicators
            TModelUnit tri_width = m_Context->ScreenToSeq(bar_height * 0.5);
            TModelUnit TriangleOffsetX = m_Context->ScreenToSeq(1.0);

            gl.ColorC(color_t);
            if (UndefinedBreakpointFrom == eBp_Outer) {
                m_Context->DrawTriangle(SeqPosFromIn, YCenterLine, SeqPosFromIn - tri_width, BoundaryYHigh,
                    SeqPosFromIn - tri_width, BoundaryYLow, true);
            } else if (UndefinedBreakpointFrom == eBp_Inner) {
                m_Context->DrawTriangle(SeqPosFromIn - TriangleOffsetX, BoundaryYLow, SeqPosFromIn - TriangleOffsetX, BoundaryYHigh,
                    SeqPosFromIn - TriangleOffsetX - tri_width, YCenterLine, true);
            }

            gl.ColorC(color_t);
            if (UndefinedBreakpointTo == eBp_Outer) {
                m_Context->DrawTriangle(SeqPosToIn + 1.0, YCenterLine, SeqPosToIn + 1.0 + tri_width, BoundaryYHigh,
                    SeqPosToIn + 1.0 + tri_width, BoundaryYLow, true);
            } else if (UndefinedBreakpointTo == eBp_Inner) {
                m_Context->DrawTriangle(SeqPosToIn + TriangleOffsetX + 1.0, BoundaryYLow, SeqPosToIn + TriangleOffsetX + 1.0, BoundaryYHigh,
                    SeqPosToIn + TriangleOffsetX + 1.0 + tri_width, YCenterLine, true);
            }

            first_pass = false;
            prev_to_x = SeqPosTo;
            prev_from_x = SeqPosFrom;
        }

        // render Transchr ssv, only render it once per feature, not per interval
        if (idx == CCustomFeatureColor::eTranschr) {
            // use the total SeqRange for screen size comparison
            SeqRange = GetRange();
            if (m_Context->SeqToScreen(SeqRange.GetLength()) > bar_height * 2.0) {
                gl.Color3f(0.0f, 0.0f, 0.0f);
                TModelUnit x1 = SeqPosTo - m_Context->ScreenToSeq(bar_height) + 1.0;

                // first clear background
                gl.Disable(GL_BLEND);
                gl.Color3f(1.0f, 1.0f, 1.0f);
                m_Context->DrawQuad(x1, YCenterLine - bar_height, SeqPosTo + 1.0, YCenterLine + bar_height);

                // draw quad with lines
                m_Context->Draw3DQuad_HorzLines(
                    x1, YCenterLine - bar_height, SeqPosTo + 1.0, YCenterLine + bar_height, color, true);
            }
        }

        x_DrawInnerLabels(base);
    } else {
        // bar is less then 2 pixel. Do not draw intervals
        m_Context->Draw3DQuad(SeqPosFrom, BoundaryYLow,
            SeqPosTo, BoundaryYHigh, color, true);
    }
}

void CFeatGlyph::x_DrawRSites(const CRgbaColor& color, TModelUnit BoundaryYLow, TModelUnit BoundaryYHigh) const
{
    _ASSERT(m_Context);

    TSeqPos from = m_Context->GetVisSeqFrom();
    TSeqPos to = m_Context->GetVisSeqTo();

    IRender& gl = GetGl();
    TModelUnit markerHeight = BoundaryYHigh - BoundaryYLow;
    markerHeight *= kRSiteMarkerHeight;
    TModelUnit halfMarkerWidth = kRSiteMarkerWidth / 2.0;


    // Check if the marker is visible
    if (m_Context->SeqToScreen(kRSiteMarkerWidth) < kRSiteMarkerThreshold)
        return;

    gl.ColorC(color);

    CSeq_loc_CI iter(GetLocation());
    for ( ;  iter;  ++iter) {
        CConstRef<CSeq_loc> curr_loc = iter.GetRangeAsSeq_loc();
        if (!curr_loc->IsPnt())
            continue;

        const CSeq_point& pnt = curr_loc->GetPnt();
        if (!pnt.CanGetFuzz() || !pnt.CanGetPoint())
            continue;

        if ((from > pnt.GetPoint()) || (to < pnt.GetPoint()))
            continue;

        const CSeq_point_Base::TFuzz &fuzz = pnt.GetFuzz();
        if (fuzz.IsLim() && (CInt_fuzz_Base::eLim_tl == fuzz.GetLim())) {
            TSeqPos pos = pnt.GetPoint();
            if (pnt.CanGetStrand() && (eNa_strand_minus == pnt.GetStrand()))
                m_Context->DrawTriangle(pos, BoundaryYHigh, pos + halfMarkerWidth, BoundaryYHigh + markerHeight, pos - halfMarkerWidth, BoundaryYHigh + markerHeight, true);
            else
                m_Context->DrawTriangle(pos, BoundaryYLow, pos + halfMarkerWidth, BoundaryYLow - markerHeight, pos - halfMarkerWidth, BoundaryYLow - markerHeight, true);
        }
    }
}

TModelUnit CFeatGlyph::x_GetBarHeight() const
{
    TModelUnit height = m_Config->GetBarHeight(
        m_Context->IsOverviewMode()  &&  m_HideLabel);

    if (IsDbVar(GetFeature())  &&
        m_Context->SeqToScreen(GetRange().GetLength()) > height * 2.0  &&
        GetCustomColorIdx(GetFeature()) == CCustomFeatureColor::eTranschr) {
            height += height;
    }

    return height;
}


CFeatGlyph::EUndefinedBpType
CFeatGlyph::x_GetUndefinedBp_from(const CSeq_loc& loc) const
{
    // currently only support location with type e_Int or e_Mix of e_Int.
    EUndefinedBpType type = eBp_Unknown;
    if (IsDbVar(GetFeature())  &&  GetFeature().GetData().GetVariation().IsInsertion()) {
        type = eBp_Outer;
    } else if (loc.IsInt()  &&
        loc.GetInt().IsSetFuzz_from()  &&
        loc.GetInt().GetFuzz_from().IsLim()) {
        CInt_fuzz::TLim lim = loc.GetInt().GetFuzz_from().GetLim();
        if (lim == CInt_fuzz::eLim_gt) {
            type = eBp_Outer;
        } else if (lim == CInt_fuzz::eLim_lt) {
            type = eBp_Inner;
        }
    } else if (loc.IsMix()) {
         const CSeq_loc* start_loc = loc.IsReverseStrand() ?
             loc.GetMix().Get().front() : loc.GetMix().Get().back();
         type = x_GetUndefinedBp_from(*start_loc);
    } else if (loc.IsPacked_int()) {
        const CSeq_interval* starSeqPosToInt = loc.IsReverseStrand() ?
             loc.GetPacked_int().Get().front() : loc.GetPacked_int().Get().back();
        if (starSeqPosToInt->IsSetFuzz_from()  && starSeqPosToInt->GetFuzz_from().IsLim()) {
            CInt_fuzz::TLim lim = starSeqPosToInt->GetFuzz_from().GetLim();
            if (lim == CInt_fuzz::eLim_gt) {
                type = eBp_Outer;
            } else if (lim == CInt_fuzz::eLim_lt) {
                type = eBp_Inner;
            }
        }
    }

    return type;
}


CFeatGlyph::EUndefinedBpType
CFeatGlyph::x_GetUndefinedBp_to(const CSeq_loc& loc) const
{
    // currently only support location with type e_Int or e_Mix of e_Int.
    EUndefinedBpType type = eBp_Unknown;
    if (IsDbVar(GetFeature())  &&  GetFeature().GetData().GetVariation().IsInsertion()) {
        type = eBp_Outer;
    } else if (loc.IsInt()  &&
        loc.GetInt().IsSetFuzz_to()  &&
        loc.GetInt().GetFuzz_to().IsLim()) {
        CInt_fuzz::TLim lim = loc.GetInt().GetFuzz_to().GetLim();
        if (lim == CInt_fuzz::eLim_gt) {
            type = eBp_Inner;
        } else if (lim == CInt_fuzz::eLim_lt) {
            type = eBp_Outer;
        }
    } else if (loc.IsMix()) {
         const CSeq_loc* stop_loc = loc.IsReverseStrand() ?
             loc.GetMix().Get().back() : loc.GetMix().Get().front();
         type = x_GetUndefinedBp_from(*stop_loc);
    } else if (loc.IsPacked_int()) {
         const CSeq_interval* stop_int = loc.IsReverseStrand() ?
             loc.GetPacked_int().Get().back() : loc.GetPacked_int().Get().front();
        if (stop_int->IsSetFuzz_to() && stop_int->GetFuzz_to().IsLim()) {
            CInt_fuzz::TLim lim = stop_int->GetFuzz_to().GetLim();
            if (lim == CInt_fuzz::eLim_gt) {
                type = eBp_Inner;
            } else if (lim == CInt_fuzz::eLim_lt) {
                type = eBp_Outer;
            }
        }
    }

    return type;
}


bool CFeatGlyph::x_ShowLabel() const
{
    bool shown = false;
    if (m_Config->m_LabelPos != CFeatureParams::ePos_NoLabel) {
        if (m_Config->m_LabelPos == CFeatureParams::ePos_Side) {
            // in MSA context, the whole range can be very small and label can be skipped altogether
            //if (m_Context->WillLabelFit(m_Context->GetVisibleRange())) {
                shown = !m_HideLabel;
            //} 
        } else { // can be either ePos_Above or ePos_Inside
            TModelRange r = m_Context->IntersectVisible(this);
            if (m_Context->WillLabelFit(r)) {
                shown = !m_HideLabel  ||  IsSelected();
            }
            if (shown) {
                string fl_content;
                GetLabel(fl_content, CLabel::eContent);
                IRender& gl = GetGl();
                TModelUnit available_width = m_Context->SeqToScreen(r.GetLength());
                TModelUnit label_width = gl.TextWidth(&(m_Config->m_LabelFont), fl_content.c_str());
                if (label_width > available_width) {
                    fl_content = m_Config->m_LabelFont.Truncate(fl_content.c_str(), available_width);
                    if ((string::npos != fl_content.find("...")) && (fl_content.length() <= 5))
                        shown = false;
                }
            }
        }

        // Check if label is redudant with track name and if so, hide:
        if (shown) {
            string fl_content;
            GetLabel(fl_content, CLabel::eContent);
            if (x_RedundantLabelCheck(fl_content))
                shown = false;
        }
    }
    return shown;
}


bool CFeatGlyph::x_ShowRuler() const
{
    // Show ruler if a feature has ruler enabled and
    // is selected in details mode
    return IsSelected() && (m_RulerType != eNoRuler)  &&
        !m_Context->IsOverviewMode();
}


string CFeatGlyph::x_GetFeatureId() const
{
    string id;
    const CSeq_feat& feat = GetFeature();
    const CSeq_feat::TId* feat_id = NULL;
    if (feat.IsSetId()) {
        feat_id = &feat.GetId();
    } else if (feat.IsSetIds()  &&  !feat.GetIds().empty()) {
        feat_id = feat.GetIds().front().GetPointer();
    }

    if (feat_id) {
        switch(feat_id->Which()) {
            case CFeat_id::e_Gibb:
                id = NStr::IntToString(feat_id->GetGibb());
                break;
            case CFeat_id::e_Giim:
                id = NStr::IntToString(feat_id->GetGiim().GetId());
                break;
            case CFeat_id::e_Local:
                if (feat_id->GetLocal().IsStr()) {
                    id = feat_id->GetLocal().GetStr();
                } else if (feat_id->GetLocal().IsId()) {
                    id = NStr::IntToString(feat_id->GetLocal().GetId());
                }
                break;
            case CFeat_id::e_General:
                id = feat_id->GetGeneral().GetDb() + ":";
                if (feat_id->GetGeneral().GetTag().IsStr()) {
                    id += feat_id->GetGeneral().GetTag().GetStr();
                } else if (feat_id->GetGeneral().GetTag().IsId()) {
                    id += NStr::IntToString(feat_id->GetGeneral().GetTag().GetId());
                }
                break;
            default:
                break;
        }
    }

    return id;
}

void CFeatGlyph::x_GetUserColor(const objects::CUser_object &display_settings, CRgbaColor &color) const
{
    if (!display_settings.HasField("color", ".", NStr::eNocase))
        return;
    CRgbaColor c;
    try {
        c.FromString(display_settings.GetField("color", ".", NStr::eNocase).GetString());
        bool is_white = c.GetRed() == 1.f && c.GetGreen() == 1.f && c.GetBlue() == 1.f;
        if (!is_white) 
            color = c;
    }
    catch (const CException &err)
    {
        LOG_POST(Error << "Invalid color found in Exts.DisplaySettings.color" << err);
    }
}


void CFeatGlyph::SetRelatedGlyphSelected(bool Selected) {
    m_Context->SetIsDrawn(GetPName(), Selected);
    // LOG_POST(Trace << "==== CLayoutGroup::SetRelatedGlyphSelected(" << Selected << "): " << this << ": " << CStackTrace());
}

bool CFeatGlyph::GetRelatedGlyphSelected() const
{
    return m_Context && m_Context->GetIsDrawn(GetPName());
}


void CProjectedMappingInfo::GetAnchorSequence(CScope& scope, string& buffer) const
{
    buffer.clear();
    if (empty())
        return;
    auto& id = front().m_MappedInt->GetId();
    CBioseq_Handle bsh = scope.GetBioseqHandle(id);
    if (!bsh)
        return;
    CSeqVector vec(bsh, CBioseq_Handle::eCoding_Iupac);
    TSignedSeqPos pre_gen = -1;
    TSignedSeqPos pre_to = 0;
    TSignedSeqPos prev_anchor = -1;
    bool neg = m_AlnMgr->IsNegativeStrand(m_AlnMgr->GetAnchor());

    for (auto&& map_it : *this) {
        auto& gen_int = *map_it.m_MappedInt;
        auto& prod_range = map_it.m_MappedProdRange;
        auto& anchor_range = map_it.m_AnchorRange;
        bool gen_neg = gen_int.GetStrand() == eNa_strand_minus;

        int num2add = prod_range.GetFrom() - pre_to;
        if (num2add >= 0)
            buffer.append(num2add, 'N');
        else if (prev_anchor > 0 && pre_gen > 0) {
            auto d = neg ?
                prev_anchor - anchor_range.GetTo() : anchor_range.GetFrom() - prev_anchor;
            bool is_insert = d == 0;
            if (is_insert) {
                num2add = gen_neg ? pre_gen - gen_int.GetTo() : gen_int.GetFrom() - pre_gen;
                if (num2add > 0)
                    buffer.append(num2add, 'N');
            }
        }

        string tmp_seq;
        vec.GetSeqData(gen_int.GetFrom(), gen_int.GetTo() + 1, tmp_seq);
        if (gen_neg) {
            string seq;
            CSeqManip::ReverseComplement(tmp_seq, CSeqUtil::e_Iupacna,
                                         0, tmp_seq.length(), seq);
            tmp_seq.swap(seq);
        }
        pre_to = prod_range.GetTo() + 1;
        pre_gen = gen_neg ? gen_int.GetFrom() - 1 : gen_int.GetTo() + 1;
        prev_anchor = neg ? anchor_range.GetFrom() - 1: anchor_range.GetTo() + 1;
        buffer += tmp_seq;
    }
}


END_NCBI_SCOPE
