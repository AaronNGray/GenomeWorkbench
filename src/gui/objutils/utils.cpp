/*  $Id: utils.cpp 44916 2020-04-17 21:39:25Z shkeda $
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
*    General utility classes for GUI projects.
*/

#include <ncbi_pch.hpp>

#include <corelib/ncbiapp.hpp>

#include <misc/xmlwrapp/xmlwrapp.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/objutils/tool_tip_info.hpp>
#include <gui/objutils/obj_convert.hpp>
#include <gui/objutils/gencoll_svc.hpp>
#include <gui/objutils/gui_eutils_client.hpp>

#include <objects/seqfeat/Feat_id.hpp>
#include <objects/seqfeat/SeqFeatXref.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/seq/Seq_descr.hpp>
#include <objects/seq/seqport_util.hpp>
#include <objects/seq/Annot_descr.hpp>
#include <objects/seq/Annotdesc.hpp>
#include <objects/seq/Pubdesc.hpp>
#include <objects/seqset/Seq_entry.hpp>
#include <objects/seqfeat/RNA_ref.hpp>
#include <objects/seqalign/Seq_align_set.hpp>
#include <objects/seqalign/Std_seg.hpp>
#include <objects/entrezgene/Entrezgene.hpp>
#include <objects/entrezgene/Entrezgene_Set.hpp>
#include <objects/submit/Seq_submit.hpp>
#include <objects/biotree/BioTreeContainer.hpp>

#include <objects/genomecoll/GC_Replicon.hpp>
#include <objects/gbproj/ProjectItem.hpp>

#include <objmgr/feat_ci.hpp>
#include <objmgr/util/sequence.hpp>
#include <objmgr/util/feature.hpp>
#include <objmgr/impl/handle_range_map.hpp>
#include <objmgr/impl/synonyms.hpp>
#include <objmgr/seq_vector.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seq_entry_ci.hpp>
#include <objmgr/align_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>

#include <serial/iterator.hpp>
#include <connect/ncbi_types.h>
#include <connect/ncbi_conn_stream.hpp>
#include <util/line_reader.hpp>
#include <algorithm>

#include <objtools/alnmgr/aln_generators.hpp>
#include <objtools/edit/apply_object.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

// cache results of top level seq-id checks
// key is seq-id normalized AsFastaString()
typedef map<string, bool> TTopLevels;
static TTopLevels m_TopLevels;
static CRWLock m_TopLevelsLock;

static CRWLock m_AssemblySeqIdLock;
typedef vector<CConstRef<CSeq_id>> TAssemblySeqIds;
static map<string, TAssemblySeqIds> s_AssemblySeqIdCache;

//
// functor for sorting features based on their length
//
struct SFeatLengthPredicate
{
    bool operator()(const CMappedFeat& feat0,
        const CMappedFeat& feat1) const
    {
        TSeqRange r0 = feat0.GetLocation().GetTotalRange();
        TSeqRange r1 = feat1.GetLocation().GetTotalRange();
        return (r0.GetLength() < r1.GetLength());
    }
};


//
// functor for sorting features based on the NCBI feature sort order
//
struct SFeatSortPredicate
{
    bool operator()(const CMappedFeat& feat0,
        const CMappedFeat& feat1) const
    {
        const CSeq_feat& f0 = feat0.GetOriginalFeature();
        const CSeq_feat& f1 = feat1.GetOriginalFeature();
        return (f0.Compare(f1, feat0.GetLocation(), feat1.GetLocation()) < 0);
    }
};



typedef SStaticPair<const char*, CSeqUtils::TAnnotNameType> TNameTypeStr;
static const TNameTypeStr s_NameTypeStrs[] = {
    { "",  CSeqUtils::eAnnot_All },
    { "All",  CSeqUtils::eAnnot_All },
    { "Named",  CSeqUtils::eAnnot_Named },
    { "Unnamed",   CSeqUtils::eAnnot_Unnamed },
};

typedef CStaticArrayMap<string, CSeqUtils::TAnnotNameType> TNameTypeMap;
DEFINE_STATIC_ARRAY_MAP(TNameTypeMap, sm_NameTypeMap, s_NameTypeStrs);


CSeqUtils::TAnnotNameType CSeqUtils::NameTypeStrToValue(const string& type)
{
    TNameTypeMap::const_iterator iter = sm_NameTypeMap.find(type);
    if (iter != sm_NameTypeMap.end()) {
        return iter->second;
    } else {
        return CSeqUtils::eAnnot_Other;
    }
}


const string&
CSeqUtils::NameTypeValueToStr(CSeqUtils::TAnnotNameType type)
{
    TNameTypeMap::const_iterator iter;
    for (iter = sm_NameTypeMap.begin();  iter != sm_NameTypeMap.end();  ++iter) {
        if (iter->second == type) {
            return iter->first;
        }
    }
    return kEmptyStr;
}


//
// retrieve an annot selector
//
SAnnotSelector CSeqUtils::GetAnnotSelector(TAnnotFlags flags)
{
    SAnnotSelector sel;
    sel
        // consider overlaps by total range...
        .SetOverlapTotalRange()
        // resolve all segments...
        .SetResolveAll()
        ;

    CGuiRegistry& reg = CGuiRegistry::GetInstance();

    CRegistryReadView view =
        reg.GetReadView("GBENCH.Utils.NamedAnnots");
    CRegistryReadView::TKeys naa_keys;
    view.GetKeys(naa_keys);
    ITERATE(CRegistryReadView::TKeys, iter, naa_keys) {
        sel.IncludeNamedAnnotAccession(view.GetString(iter->key));
    }

    view = reg.GetReadView("GBENCH.Utils.AnnotSelector");
    if ( !(flags & fAnnot_UnsetNamed) ) {
        if (view.GetBool("ExcludeExternal")) {
            sel.SetExcludeExternal(true);
        } else {
            sel.SetExcludeExternal(false);

            ///
            /// known external annotations
            ///

            static const char* named_annots[] = {
                "SNP",  /// SNPs = variation features
                "CDD",  /// CDD  = conserved domains
                "STS",  /// STS  = sequence tagged sites
                NULL
            };

            for (const char** p = named_annots;  p  &&  *p;  ++p) {
                bool incl = view.GetBool(*p, true);
                if ( !incl ) {
                    sel.ExcludeNamedAnnots(*p);
                }
            }
        }
    }

    if ( !(flags & fAnnot_UnsetDepth) ) {
        if (view.GetBool("AdaptiveDepth", true)) {
            sel.SetAdaptiveDepth(true);
            sel.SetResolveAll();
        }
    }
    return sel;
}


//
// retrieve an annot selector for our selected annotations
//
SAnnotSelector CSeqUtils::GetAnnotSelector(SAnnotSelector::TAnnotType c,
                                           TAnnotFlags flags)
{
    SAnnotSelector sel = GetAnnotSelector(flags);
    sel
        // limit by our annotation type
        .SetAnnotType(c);
    return sel;
}


//
// retrieve an annot selector for our selected annotations
//
SAnnotSelector CSeqUtils::GetAnnotSelector(SAnnotSelector::TFeatType  feat,
                                           TAnnotFlags flags)
{
    SAnnotSelector sel = GetAnnotSelector(CSeq_annot::TData::e_Ftable, flags);
    sel
        // retrieve feature type and subtype of interest
        .SetFeatType(feat);

    return sel;
}


SAnnotSelector CSeqUtils::GetAnnotSelector(SAnnotSelector::TFeatSubtype sub,
                                           TAnnotFlags flags)
{
    SAnnotSelector sel = GetAnnotSelector(CSeq_annot::TData::e_Ftable, flags);
    sel
        // retrieve feature type and subtype of interest
        .SetFeatSubtype(sub);

    return sel;
}


SAnnotSelector CSeqUtils::GetAnnotSelector(const vector<string>& annots)
{
    SAnnotSelector sel = GetAnnotSelector();
    sel.SetCollectNames();

    //CGuiRegistry& reg = CGuiRegistry::GetInstance();
    //CRegistryReadView view =
    //    reg.GetReadView("GBENCH.Utils.AnnotSelector");
    //bool include_naas = view.GetBool("IncludeNAAs", false);
    bool include_naas = false;

    if (include_naas  &&  annots.empty()) {
        sel.IncludeNamedAnnotAccession("NA*");
    }

    ITERATE (vector<string>, iter, annots) {
        const string& annot = *iter;
        switch (CSeqUtils::NameTypeStrToValue(annot)) {
        case CSeqUtils::eAnnot_Unnamed:
            sel.AddUnnamedAnnots();
            break;
        case CSeqUtils::eAnnot_Named:
            sel.ExcludeUnnamedAnnots();
            break;
        case CSeqUtils::eAnnot_All:
            if (include_naas) {
                sel.IncludeNamedAnnotAccession("NA*");
            }
            return sel;
        case CSeqUtils::eAnnot_Other:
        default:
            if (NStr::StartsWith(annot, "NA*")) {
                sel.IncludeNamedAnnotAccession("NA*");
            } else {
                sel.AddNamedAnnots(annot);
                if(IsNAA(annot) || IsExtendedNAA(annot)) {
                    sel.IncludeNamedAnnotAccession(annot);
                }
            }
            break;
        }
    }
    return sel;
}


SAnnotSelector CSeqUtils::GetAnnotSelector(const vector<string>& annots,
                                           bool adaptive, int depth)
{
    SAnnotSelector sel(GetAnnotSelector(annots));
    SetResolveDepth(sel, adaptive, depth);
    return sel;
}


void CSeqUtils::SetAnnot(objects::SAnnotSelector& sel, const string& annot)
{
    switch (CSeqUtils::NameTypeStrToValue(annot)) {
        case CSeqUtils::eAnnot_Unnamed:
            sel.AddUnnamedAnnots();
            break;
        case CSeqUtils::eAnnot_Named:
            sel.ExcludeUnnamedAnnots();
            break;
        case CSeqUtils::eAnnot_All:
            {{
                //CGuiRegistry& reg = CGuiRegistry::GetInstance();
                //CRegistryReadView view =
                //    reg.GetReadView("GBENCH.Utils.AnnotSelector");
                //bool include_naas = view.GetBool("IncludeNAAs", false);
                bool include_naas = false;
                if (include_naas) {
                    sel.IncludeNamedAnnotAccession("NA*");
                }
            }}
            break;
        case CSeqUtils::eAnnot_Other:
        default:
            sel.AddNamedAnnots(annot);
            if (IsNAA(annot) || IsExtendedNAA(annot)) {
                sel.IncludeNamedAnnotAccession(annot);
            }
            break;
    }
}



CRegistryReadView CSeqUtils::GetSelectorRegistry()
{
    CGuiRegistry& reg = CGuiRegistry::GetInstance();
    return reg.GetReadView("GBENCH.Utils.AnnotSelector");
}

int CSeqUtils::GetMaxSearchSegments(const CRegistryReadView& view)
{
    return view.GetInt("MaxSearchSegments", 0);
}

SAnnotSelector::EMaxSearchSegmentsAction CSeqUtils::GetMaxSearchSegmentsAction(const CRegistryReadView& view)
{
    string max_search_segs_action = view.GetString("MaxSearchSegmentsAction", "silent");
    SAnnotSelector::EMaxSearchSegmentsAction MaxSearchSegmentsAction{ SAnnotSelector::eMaxSearchSegmentsSilent };

    if(max_search_segs_action == "throw") {
        MaxSearchSegmentsAction = SAnnotSelector::eMaxSearchSegmentsThrow;
    } else if (max_search_segs_action == "log") {
        MaxSearchSegmentsAction = SAnnotSelector::eMaxSearchSegmentsLog;
    }

    return MaxSearchSegmentsAction;
}

bool CSeqUtils::CheckMaxSearchSegments(int actual, int max, SAnnotSelector::EMaxSearchSegmentsAction action)
{
    if (max > 0 && actual > max) {
        switch(action) {
            case SAnnotSelector::eMaxSearchSegmentsThrow:
                NCBI_THROW(CAnnotSearchLimitException, eSegmentsLimitExceded,
                           "CSeqUtils::CheckMaxSearchSegments: search segments limit exceeded");
                break;
            case SAnnotSelector::eMaxSearchSegmentsSilent:
                break;
            case SAnnotSelector::eMaxSearchSegmentsLog:
            default:
                ERR_POST("CSeqUtils::CheckMaxSearchSegments: search segments limit exceeded");
                break;
        }
        return true;
    }
    return false;
}


int CSeqUtils::GetMaxSearchTime(const CRegistryReadView& view)
{
    return view.GetInt("MaxSearchTime", 0);
}


void CSeqUtils::SetResolveDepth(objects::SAnnotSelector& sel,
                                bool adaptive, int depth)
{
    if (adaptive) {
        sel.SetAdaptiveDepth(true);
        sel.SetExactDepth(false);
        // TODO: watch out
        // Maybe there is bug inside selector, we have call SetResolveAll() even
        // for cases where we only want to resolve up to a given depth.
        sel.SetResolveAll();

        CRegistryReadView view = GetSelectorRegistry();
        int max_search_segs = GetMaxSearchSegments(view);
//        LOG_POST(Trace << "MaxSearchSegments: " << max_search_segs);
        sel.SetMaxSearchSegments(max_search_segs);
        if(max_search_segs > 0) {
            sel.SetMaxSearchSegmentsAction(GetMaxSearchSegmentsAction(view));
        }
        sel.SetMaxSearchTime((float)GetMaxSearchTime(view));

        if (depth >=0) {
            sel.SetResolveDepth(depth);
        }
    } else if (depth >= 0) {
        sel.SetResolveDepth(depth);
        sel.SetExactDepth(true);
        sel.SetAdaptiveDepth(false);
    }
}


//
// LinkFeatures()
// This builds explicit links between features, creating a hierarchical tree of
// features.
//

template <class T, class U>
struct SPairBy1stLess
    : public binary_function< pair<T,U>, pair<T,U>, bool>
{
    bool operator()(const pair<T,U>& p1, const pair<T,U>& p2) const
    {
        return p1.first < p2.first;
    }
};


bool CSeqUtils::LinkFeatures(CLinkedFeature::TLinkedFeats& feats,
                             CSeqUtils::TFeatLinkingMode mode,
                             ISeqTaskProgressCallback* p_cb)
{
    if (p_cb) {
        p_cb->SetTaskName("Linking features...");
        p_cb->SetTaskTotal((int)feats.size());
        p_cb->SetTaskCompleted(0);
    }

    CLinkedFeature::TLinkedFeats out_feats;

    // using CFeatTree
    feature::CFeatTree tree;
    tree.SetFeatIdMode(feature::CFeatTree::EFeatIdMode(mode));
    map<CMappedFeat, CRef<CLinkedFeature> > fmap;
    NON_CONST_ITERATE (CLinkedFeature::TLinkedFeats, iter, feats) {
        if (p_cb  &&  p_cb->StopRequested()) {
            return  false;
        }
        CRef<CLinkedFeature> curr_feat = *iter;
        tree.AddFeature(curr_feat->GetMappedFeature());
        fmap[curr_feat->GetMappedFeature()] = curr_feat;
    }
    NON_CONST_ITERATE (CLinkedFeature::TLinkedFeats, iter, feats) {
        if (p_cb  &&  p_cb->StopRequested()) {
            return  false;
        }
        CRef<CLinkedFeature> curr_feat = *iter;
        CMappedFeat parent_feat = tree.GetParent(curr_feat->GetMappedFeature());
        if ( parent_feat ) {
            fmap[parent_feat]->AddChild(curr_feat);
        }
        else {
            out_feats.push_back(curr_feat);
        }
        if (p_cb) p_cb->AddTaskCompleted(1);
    }

    out_feats.swap(feats);

    return true;
}


// remap a child location to a parent
CRef<CSeq_loc> CSeqUtils::RemapChildToParent(const CSeq_loc& parent,
                                             const CSeq_loc& child,
                                             CScope* scope)
{
    CSeq_loc dummy_parent;
    dummy_parent.SetWhole(const_cast<CSeq_id&>(sequence::GetId(parent, 0)));
    SRelLoc converter(dummy_parent, child, scope);
    converter.m_ParentLoc = &parent;
    return converter.Resolve(scope);
}

bool CSeqUtils::Match(const CSeq_id& id1, const CSeq_id& id2, CScope* scope)
{
    return Match(CSeq_id_Handle::GetHandle(id1),
        CSeq_id_Handle::GetHandle(id2),
        scope);
}


bool CSeqUtils::Match(const CSeq_id_Handle& id1,
                      const CSeq_id_Handle& id2, CScope* scope)
{
    if (id1.MatchesTo(id2))  {
        return true;
    }

    if (id1.IsGi() && id2.IsGi() )
        return false;

    if (scope) {
        CConstRef<CSynonymsSet> syns;

        syns = scope->GetSynonyms(id1);
        if (syns) {
            ITERATE (CSynonymsSet, iter, *syns) {
                if (id2.MatchesTo(CSynonymsSet::GetSeq_id_Handle(iter))) {
//                    cerr << "matched to synonym of presented seq-id: " << endl;
//                    cerr << MSerial_AsnText << *CSynonymsSet::GetSeq_id_Handle(iter).GetSeqId();
                    return true;
                }
            }
        }

        syns = scope->GetSynonyms(id2);
        if (syns) {
            ITERATE (CSynonymsSet, iter, *syns) {
                if (id1.MatchesTo(CSynonymsSet::GetSeq_id_Handle(iter))) {
//                    cerr << "matched to synonym of tls-seq-id: " << endl;
//                    cerr << MSerial_AsnText << *CSynonymsSet::GetSeq_id_Handle(iter).GetSeqId();
                    return true;
                }
            }
        }
    }
    return false;
}

CRef<CSeq_loc>  CSeqUtils::CreateSeq_loc(const CSeq_id& id,
                                         const CRangeCollection<TSeqPos>& ranges)
{
    CRef<CSeq_loc>  seq_loc(new CSeq_loc());
    CSeq_loc::TPacked_int& p_int = seq_loc->SetPacked_int();

    ITERATE(CRangeCollection<TSeqPos>, it_r, ranges) { // for each range in mark
        if ( !it_r->Empty() ) {
            p_int.AddInterval(id, it_r->GetFrom(), it_r->GetTo());
        }
    }
    switch (p_int.Get().size()) {
    case 0:
        {{
        return CRef<CSeq_loc>();
        }}
    case 1:
        {{
            CRef<CSeq_interval> ival(p_int.Set().front());
            seq_loc->SetInt(*ival);
            /// p_int no longer valid!
        }}
        break;
    default:
        break;
    }
    return seq_loc;
}

bool   CSeqUtils::GetRangeCollection(const CSeq_id& id, const CHandleRangeMap& map,
                                     CRangeCollection<TSeqPos>& ranges)
{
    CSeq_id_Handle s_id = CSeq_id_Handle::GetHandle(id);

    // extract from the given map all segments corresponding to the given id
    const CHandleRangeMap::TLocMap& loc_map = map.GetMap();
    CHandleRangeMap::TLocMap::const_iterator it = loc_map.find(s_id);
    if(it != loc_map.end()) {
        ITERATE(CHandleRange, it_r, it->second) {
            ranges.CombineWith(it_r->first);
        }
        return true;
    } else return false;
}

CBioseq* CSeqUtils::SeqLocToBioseq(CScope& scope,
                                   const CSeq_loc& loc)
{
    // Build a Seq-entry for the query Seq-loc
    // A seq-loc may have multiple seq-ids, use the first successful seq-id.
    CBioseq_Handle handle;
    for ( CSeq_loc_CI citer (loc); citer; ++citer) {
        handle = scope.GetBioseqHandle(citer.GetSeq_id());
        if ( handle ) {
            break;
        }
    }

    if ( !handle ) {
        return CRef<CBioseq>();
    }

    /// easy out: if the bioseq is of type whole, just duplicate it
    if (loc.IsWhole()) {
        CRef<CBioseq> bioseq(new CBioseq());
        bioseq->Assign(*handle.GetCompleteBioseq());
        return bioseq.Release();
    }

    CSeqVector vec(loc, scope, CBioseq_Handle::eCoding_Iupac);
    string seq_string;
    vec.GetSeqData(0, vec.size(), seq_string);

    CRef<CBioseq> bioseq(new CBioseq());

    // curate our inst
    bioseq->SetInst().SetRepr(CSeq_inst::eRepr_raw);
    bioseq->SetInst().SetLength((int)seq_string.size());
    if (vec.IsProtein()) {
        bioseq->SetInst().SetMol(CSeq_inst::eMol_aa);
        bioseq->SetInst().SetSeq_data().SetIupacaa(*new CIUPACaa(seq_string));
    } else {
        bioseq->SetInst().SetMol(CSeq_inst::eMol_na);
        bioseq->SetInst().SetSeq_data().SetIupacna(*new CIUPACna(seq_string));
        CSeqportUtil::Pack(&bioseq->SetInst().SetSeq_data());
    }


    // add an ID for our sequence
    CRef<CSeq_id> id(new CSeq_id());
    id->Assign(*handle.GetSeqId());
    bioseq->SetId().push_back(id);

    // a title
    CRef<CSeqdesc> title(new CSeqdesc);
    string title_str;
    id->GetLabel(&title_str);
    title_str += ": ";
    loc.GetLabel(&title_str);
    title->SetTitle(title_str);
    bioseq->SetDescr().Set().push_back(title);

    return bioseq.Release();
}


CRef<CSeq_loc> CSeqUtils::MixLocToLoc(const CSeq_loc& mix_loc,
                                      const CBioseq_Handle& handle)
{
    CRef<CSeq_loc>  seq_loc(new CSeq_loc());
    CSeq_loc::TPacked_int& p_int = seq_loc->SetPacked_int();

    for (CSeq_loc_CI iter(mix_loc);  iter;  ++iter) {
        if (handle.IsSynonym(iter.GetSeq_id())) {
            CSeq_loc_CI::TRange range = iter.GetRange();
            if ( !range.Empty() ) {
                p_int.AddInterval(*handle.GetSeqId(), range.GetFrom(),
                                  range.GetTo(), iter.GetStrand());
            }
        }
    }
    switch (p_int.Get().size()) {
    case 0:
        {{
            return CRef<CSeq_loc>();
        }}
    case 1:
        {{
            CRef<CSeq_interval> ival(p_int.Set().front());
            seq_loc->SetInt(*ival);
            /// p_int no longer valid!
        }}
        break;
    default:
        break;
    }

    return seq_loc;
}


string CSeqUtils::GetAnnotName(const CSeq_annot_Handle& annot_handle)
{
    string name(GetUnnamedAnnot());
    CConstRef<CSeq_annot> annot = annot_handle.GetCompleteSeq_annot();
    if (annot) {
        name = GetAnnotName(*annot);
    } else if (annot_handle.IsNamed()) {
        name = annot_handle.GetName();
    }
    return name;
}


string CSeqUtils::GetAnnotName(const CSeq_annot& annot)
{
    string name(GetUnnamedAnnot());
    if (annot.IsSetDesc()) {
        ITERATE (objects::CAnnot_descr::Tdata, descrIter, annot.GetDesc().Get()) {
            if ((*descrIter)->IsTitle()) {
                name = (*descrIter)->GetTitle();
                break;
            } else if ((*descrIter)->IsName()) {
                name = (*descrIter)->GetName();
            }
        }
    }
    return name;
}


string CSeqUtils::GetAnnotComment(const CSeq_annot_Handle& annot_handle)
{
    string comment = kEmptyStr;
    CConstRef<CSeq_annot> annot = annot_handle.GetCompleteSeq_annot();
    if (annot) {
        comment = GetAnnotComment(*annot);
    }
    return comment;
}


string CSeqUtils::GetAnnotComment(const CSeq_annot& annot)
{
    if (annot.IsSetDesc()) {
        ITERATE (objects::CAnnot_descr::Tdata, descrIter, annot.GetDesc().Get()) {
            if ((*descrIter)->IsComment()) {
                return (*descrIter)->GetComment();
            }
        }
    }
    return kEmptyStr;
}

static bool s_IsNAA(const string& annot, char div)
{
//    if(IsExtendedNAA(annot)) {
//        LOG_POST(Trace << "CSeqUtils::IsNAA() when in fact IsExtendedNA() " << annot);
//    }
    size_t acc_len = 11;
    bool is_naa = false;
    size_t len = annot.size();
    if (len >= acc_len && annot[0] == 'N' && annot[1] == 'A') {
        size_t i = 2;
        while (i < acc_len  &&  annot[i] >= '0' && annot[i] <= '9') {
            ++i;
        }

        if (i == acc_len) {
            if (len == acc_len) {
                is_naa = true;
            }
            else if (annot[i++] == div) {
                while (i < len  &&  annot[i] >= '0' && annot[i] <= '9') {
                    ++i;
                }
                if (i == len) {
                    is_naa = true;
                }
            }
        }
    }

    return is_naa;
}

bool CSeqUtils::IsNAA(const string& annot, bool isStrict)
{
    return isStrict ? s_IsNAA(annot, '.') : IsExtendedNAA(annot);
}

bool CSeqUtils::IsNAA_Name(const string& annot)
{
    return IsExtendedNAA_Name(annot);
}

/// create an annotation name for a remote file pipeline, appending sSuffix
string CSeqUtils::MakeRmtAnnotName(const string& sSuffix)
{
    return "rmt_pipleine_" + sSuffix;
}

/// check if a given annotation was created by a remote file pipeline
bool CSeqUtils::isRmtAnnotName(const string& sAnnotName)
{
    return NStr::StartsWith(sAnnotName, "rmt_pipleine_");
}

bool CSeqUtils::isRmtPipelineFileType(const string& sFileType)
{
    return sFileType == "bigBed" || sFileType == "bigWig" || sFileType == "vcfTabix";
}


static bool s_IsExtendedNAA(const string& sAnnotName, char div, bool isStrict)
{
    size_t posHashSign{sAnnotName.find('#')};

    if(posHashSign == NPOS) {
        if(isStrict) {
            return false;
        } else {
            return s_IsNAA(sAnnotName, div);
        }
    }
    if(!s_IsNAA(sAnnotName.substr(0, posHashSign), div))
    {
        return false;
    }
    if(posHashSign == sAnnotName.length() - 1) {
        return true;
    }
    for(size_t i = posHashSign+1; i<sAnnotName.length(); ++i) {
        if(sAnnotName[i] < '0' || sAnnotName[i] > '9') {
            return false;
        }
    }
    return true;
}

bool CSeqUtils::IsExtendedNAA(const string& sAnnotName, bool isStrict)
{
    return s_IsExtendedNAA(sAnnotName, '.', isStrict);
}

bool CSeqUtils::IsExtendedNAA_Name(const string& sAnnotName)
{
    return s_IsExtendedNAA(sAnnotName, '_', false);
}

// check if a given annotation is AlignDb (potentially suffixed with batch identication string after a '#')
bool CSeqUtils::IsAlignDb(const string& annot)
{
    return NStr::StartsWith(annot, "AlignDb", NStr::eNocase);
}

// get a batch string from  AlignDb annotation suffixed with batch identication string after a '#'
string CSeqUtils::GetAlignDbBatch(const string& annot)
{
    vector<string> parts;
    if(IsAlignDb(annot)) {
        NStr::Split(annot, "#", parts);
    }
    return parts.size() > 1 ? parts[1] : string();
}

    /// get a is_source_assembly_query string fro  AlignDb annotation suffixed after a second '#'
string CSeqUtils::GetAlignDbIsQuery(const string& annot)
{
    vector<string> parts;
    if(IsAlignDb(annot)) {
        NStr::Split(annot, "#", parts);
    }
    return parts.size() > 2 ? parts[2] : string("N");
}

// VDB accessions in scope are in ("SRA", "SRR", "DRR", "ERR")
bool CSeqUtils::IsVDBAccession(const string& acc) 
{
    if (acc.size() < 3 || acc[1] != 'R')
        return false;
    switch (acc[0]) {
    case 'S':
    case 'D':
    case 'E':
        break;
    default:
        return false;
    }
    switch (acc[2]) {
    case 'A':
    case 'R':
        break;
    default:
        return false;
    }
    return true;
}


bool CSeqUtils::IsPseudoFeature(const CSeq_feat& feat)
{
    if (feat.IsSetPseudo()) {
        return feat.GetPseudo();
    } else {
        const CSeq_feat::TData& data = feat.GetData();
        if (data.IsGene()  &&  data.GetGene().IsSetPseudo()) {
            return data.GetGene().GetPseudo();
        } else if (data.IsRna()  &&  data.GetRna().IsSetPseudo()) {
            return data.GetRna().GetPseudo();
        }
    }
    return false;
}


bool CSeqUtils::IsPartialFeature(const CSeq_feat& feat)
{
    if (feat.IsSetPartial()  &&  feat.GetPartial()  &&
        !IsPartialStart(feat.GetLocation())  &&
        !IsPartialStop(feat.GetLocation())) {
        return true;
    }
    return false;
}


bool CSeqUtils::IsPartialStart(const CSeq_loc& loc)
{
    return loc.IsPartialStart(objects::eExtreme_Biological);
}


bool CSeqUtils::IsPartialStop(const CSeq_loc& loc)
{
    return loc.IsPartialStop(objects::eExtreme_Biological);
}

bool CSeqUtils::IsSameStrands(const CSeq_loc& loc)
{
    CSeq_loc_CI it(loc);
    if (it) {
        auto strand = it.GetStrand();
        ++it;
        for (; it; ++it) {
            if (it.GetStrand() != strand) 
                return false;
        }
    }
    return true;
}

bool CSeqUtils::IsException(const CSeq_feat& feat)
{
    if (feat.IsSetExcept()) {
        return feat.GetExcept();
    }
    return false;
}


string CSeqUtils::GetNcbiBaseUrl()
{
    static string base_url = "https://www.ncbi.nlm.nih.gov";
    return base_url;
}


string CSeqUtils::CreateTableStart()
{
    return "<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\">";
}


string CSeqUtils::CreateTableEnd()
{
    return "</table>";
}


string CSeqUtils::CreateTableRow(const string& tag, const string& value)
{
    return "<tr><td align=\"right\" valign=\"top\" nowrap><b>" + tag + (tag.empty() ? "" : ":") + "&nbsp;" +
        "</b></td><td valign=\"top\" width=\"200\">" + value + "</td></tr>";
}


string CSeqUtils::CreateSectionRow(const string& tag)
{
    return "<tr><td align=\"right\" nowrap>[<i>" + tag + "</i>]&nbsp;&nbsp;</td><td></td></tr>";
}


string CSeqUtils::CreateLinkRow(const string& tag,
                                const string& label,
                                const string& url)
{
    return "<tr><td align=\"right\" valign=\"top\" nowrap><b>" + tag  + ":&nbsp;" +
        "</b></td><td width=\"200\"><a href=\"" + url + "\">" + label + "</a></td></tr>";
}

static CSeqUtils::TLocVec s_GetAlnMapplingLocs(const CSeq_align_set &align_set, TGi gi)
{
    CSeqUtils::TLocVec mapped_locs;

    typedef map<TGi, TSeqRange> TRangeMap;
    TRangeMap r_map;
    CTypeConstIterator<CSeq_align> aln_iter(align_set);
    for (; aln_iter; ++aln_iter) {
        const CSeq_align& aln = *aln_iter;
        if (aln.CheckNumRows() == 2 && aln.GetSegs().IsStd() &&
            aln.GetSegs().GetStd().size() == 1) {
            int target_row = 0;
            if (aln.GetSeq_id(0).IsGi() && aln.GetSeq_id(0).GetGi() == gi) {
                target_row = 1;
            }
            if (aln.GetSeq_id(target_row).IsGi()) {
                TGi target_gi = aln.GetSeq_id(target_row).GetGi();
                TSignedSeqRange range =
                    aln.GetSegs().GetStd().front()->GetSeqRange(target_row);
                TSeqPos from = (TSeqPos)range.GetFrom();
                TSeqPos to = (TSeqPos)range.GetTo();
                if (from > to) {
                    swap(from, to);
                }
                if (r_map.count(target_gi) == 0) {
                    r_map[target_gi] = TSeqRange(from, to);
                }
                else {
                    r_map[target_gi].CombineWith(TSeqRange(from, to));
                }
            }
        }
    }

    ITERATE(TRangeMap, iter, r_map) {
        CRef<CSeq_id> id(new CSeq_id);
        id->SetGi(iter->first);
        CRef<CSeq_loc> loc(new CSeq_loc(*id, iter->second.GetFrom(),
            iter->second.GetTo()));
        mapped_locs.push_back(loc);
    }
    return mapped_locs;
}

static const char* kLinksUrlDefault =
"https://www.ncbi.nlm.nih.gov/sviewer/links.fcgi?link_name=gi_placement&report=asn";

static const string& GetLinksURL()
{
    static string LinksUrl;
    if (LinksUrl.empty()) {
        const CNcbiRegistry& reg = CNcbiApplication::Instance()->GetConfig();
        LinksUrl = reg.GetString("links", "url", kLinksUrlDefault);
    }
    return LinksUrl;
}

CSeqUtils::TLocVec CSeqUtils::GetGiPlacements(TGi gi, int time_out_sec, THTTP_Flags flags)
{
    STimeout timeout;
    timeout.sec = time_out_sec;
    timeout.usec = 0;

    CConn_HttpStream stream(GetLinksURL() + string("&gi=") + NStr::NumericToString(gi), flags, &timeout);
    auto_ptr<CObjectIStream> obj_stream(CObjectIStream::Open(eSerial_AsnText, stream));
    CSeq_align_set align_set;
    try {
        *obj_stream >> align_set;
    } catch (const CException& e) {
        LOG_POST(Error << "Failed to retrieve gi placements for gi|"
                 << gi << ", error: " << e.GetMsg());
        return TLocVec();
    }

    return s_GetAlnMapplingLocs(align_set, gi);
}

bool CSeqUtils::CanHavePlacements(const objects::CSeq_id& seqid)
{
    // GenColl accessions without NC
    CSeq_id::EAccessionInfo info = seqid.IdentifyAccession();
    return info == CSeq_id::eAcc_refseq_contig //NT
        || info == CSeq_id::eAcc_refseq_genome //NS
        || info == CSeq_id::eAcc_refseq_genomic //NG
        || info == CSeq_id::eAcc_refseq_mrna //NM
        || info == CSeq_id::eAcc_refseq_mrna_predicted //XM
        || info == CSeq_id::eAcc_refseq_ncrna //NR
        || info == CSeq_id::eAcc_refseq_ncrna_predicted //XR
        || info == CSeq_id::eAcc_refseq_prot  //NP
        || info == CSeq_id::eAcc_refseq_prot_predicted //XP
        || info == CSeq_id::eAcc_refseq_unreserved //AA
        || info == CSeq_id::eAcc_refseq_wgs_intermed
        || info == CSeq_id::eAcc_refseq_wgs_nuc //NZ
        || info == CSeq_id::eAcc_refseq_wgs_prot // ZP
        || (info & CSeq_id::eAcc_type_mask) == CSeq_id::e_Genbank           // any GenBank
        || (info & CSeq_id::eAcc_type_mask) == CSeq_id::e_Gi;
}


CSeqUtils::TLocVec CSeqUtils::GetAccessionPlacements(const CSeq_id &id, CScope &scope, int time_out_sec, THTTP_Flags flags)
{
    return GetAccessionPlacementsMsec(id, scope, time_out_sec * 1000, flags);
}

CSeqUtils::TLocVec CSeqUtils::GetAccessionPlacementsMsec(const CSeq_id &id, CScope &scope, unsigned long time_out_msec, THTTP_Flags flags)
{
    STimeout timeout;
    NcbiMsToTimeout(&timeout, time_out_msec);
    // filter out cases when the given id something unsuitable e.g. like a local id
    // generally what's bad for GenColl should be bad for getting placements
     if(!CanHavePlacements(id)) {
        return TLocVec();
    }

    do {
        CConn_HttpStream stream(GetLinksURL() + string("&id=") +  id.GetSeqIdString(true), flags, &timeout);
        auto_ptr<CObjectIStream> obj_stream(CObjectIStream::Open(eSerial_AsnText, stream));
        CSeq_align_set align_set;
        *obj_stream >> align_set;
 
        CSeq_id_Handle gi_idh = sequence::GetId(id, scope, sequence::eGetId_ForceGi);
        if (!gi_idh)
            break;
        TGi gi(gi_idh.GetGi());
        return s_GetAlnMapplingLocs(align_set, gi);
    } while (false);

    return TLocVec();
}

CSeqUtils::TLocVec CSeqUtils::GetLocPlacements(const objects::CSeq_loc& loc, int time_out_sec)
{
    TLocVec mapped_locs;
    STimeout timeout;
    timeout.sec = time_out_sec;
    timeout.usec = 0;
    TSignedSeqPos SourceFrom(-1);
    TSignedSeqPos SourceTo(-1);
    TGi SourceGi = INVALID_GI;

    if(loc.IsInt()) {
        SourceFrom = loc.GetInt().GetFrom();
        SourceTo = loc.GetInt().GetTo();
        if(loc.GetInt().GetId().IsGi()) {
            SourceGi = loc.GetInt().GetId().GetGi();
        } else {
            return mapped_locs;
        }
    } else if(loc.IsPnt()) {
        SourceFrom = loc.GetPnt().GetPoint();
        SourceTo = SourceFrom;
        if(loc.GetPnt().GetId().IsGi()) {
            SourceGi = loc.GetPnt().GetId().GetGi();
        } else {
            return mapped_locs;
        }
    } else {
        return mapped_locs;
    }
    if(SourceTo < SourceFrom)  {
        swap(SourceTo, SourceFrom);
    }
    CConn_HttpStream stream(GetLinksURL() + string("&gi=") + NStr::NumericToString(SourceGi) +
                                "&from=" + NStr::NumericToString(SourceFrom) +
                                "&to=" + NStr::NumericToString(SourceTo),
                                fHTTP_AutoReconnect, &timeout);
    auto_ptr<CObjectIStream> obj_stream(CObjectIStream::Open(eSerial_AsnText, stream));
    CSeq_align_set align_set;
    try {
        *obj_stream >> align_set;
    } catch (const CException& e) {
        LOG_POST(Error << "Failed to retrieve location placements for gi|"
                 << SourceGi << ", error: " << e.GetMsg());
        return mapped_locs;
    }

    typedef map<TGi, TSignedSeqRange> TRangeMap;
    TRangeMap range_map;
    CTypeConstIterator<CSeq_align> aln_iter(align_set);
    for(; aln_iter; ++aln_iter) {
        const CSeq_align& aln = *aln_iter;
        if (aln.CheckNumRows() == 2  &&  aln.GetSegs().IsStd()  &&
            aln.GetSegs().GetStd().size() == 1) {
            int target_row = 0;
            int source_row = 1;
            if (aln.GetSeq_id(target_row).IsGi()  &&  aln.GetSeq_id(target_row).GetGi() == SourceGi) {
                target_row = 1;
                source_row = 0;
            }
            if (aln.GetSeq_id(target_row).IsGi()) {
                // check that the source range falls within the source within this alignment
                TSignedSeqRange i_source_range =
                    aln.GetSegs().GetStd().front()->GetSeqRange(source_row);
                TSignedSeqPos i_source_from = i_source_range.GetFrom();
                TSignedSeqPos i_source_to = i_source_range.GetTo();
                if (i_source_from > i_source_to) {
                    swap(i_source_from, i_source_to);
                }
                if(i_source_from <= SourceFrom && SourceTo <= i_source_to) {
                    TGi target_gi = aln.GetSeq_id(target_row).GetGi();
                    TSignedSeqRange range =
                        aln.GetSegs().GetStd().front()->GetSeqRange(target_row);
                    TSeqPos from = (TSeqPos)range.GetFrom();
                    TSeqPos to = (TSeqPos)range.GetTo();
                    if (from > to) {
                        swap(from, to);
                    }
                    range_map[target_gi] = TSignedSeqRange(from + (SourceFrom - i_source_from), from + (SourceTo - i_source_from));
                }
            }
        }
    }

    ITERATE (TRangeMap, iter, range_map) {
        CRef<CSeq_id> id(new CSeq_id);
        id->SetGi(iter->first);
        CRef<CSeq_loc> loc(new CSeq_loc(*id, iter->second.GetFrom(), iter->second.GetTo()));
        mapped_locs.push_back(loc);
    }
    return mapped_locs;
}


bool CSeqUtils::StringToRange(const string& range_str,
                              long& from, long& to)
{
    // Any input range string that follows this pattern will be
    // consisdered as a valid input:
    // "^[ \t]*[1-9][0-9,]*[ \t]*[kKmM]?((([ \t]*([-:]|\\.\\.)[ \t]*)|([ \t]+))[1-9][0-9,]*[ \t]*[kKmM]?)?[ \t]*$"
    // Some valid range examples:
    //   - 1000
    //   - [space]1000 -[tab]2000[tab]
    //   - [space]1000[space]..[space]2000
    //   - 10,000:2,000,000
    //   - 100 k : 1m
    //   - 1000[space]2000
    //   - [space]1000[tab]2000

    from = to = 0;
    string str = NStr::TruncateSpaces(range_str);
    size_t len = str.length();

    if (len == 0) return false;

    // The loop tries to accomplish the followings:
    // - remove ','
    // - replace 'k' or 'K' with '000'
    // - replace 'm' or 'M' with '000000'
    // - remove white paces (and tabs) before ',', 'k', 'K', 'm', and 'M'
    // - remove white space (and tabs) around any separator (':', '..', and '-')
    // - replace any separator with '-'
    // - replace spaces between two numbers with '-'
    string out_str;
    bool space_before_this = false;
    bool separator_before_this = false;
    for (size_t i = 0; i < len; ++i) {
        switch (str[i])
        {
        case ' ':
        case '\t':
            // ignore space after a separator
            if ( !separator_before_this )
                space_before_this = true;
            break;
        case ',':
            if (separator_before_this)  return false; // invalid
            // ignore space before ','
            space_before_this = false;
            break;
        case 'k':
        case 'K':
            if (separator_before_this)  return false; // invalid
            // ignore space before 'k' and 'K'
            space_before_this = false;
            // replace it with "000'
            out_str.append("000");
            break;
        case 'm':
        case 'M':
            if (separator_before_this)  return false; // invalid
            // ignore space before 'm' and 'M'
            space_before_this = false;
            // replace it with "0000000'
            out_str.append("000000");
            break;
        case '.':
        case '-':
        case ':':
            // ignore space before a separator
            space_before_this = false;
            separator_before_this = true;
            break;
        default:
            if (separator_before_this) {
                out_str.append("-");
                separator_before_this = false;
            } else if (space_before_this) {
                out_str.append("-");
                space_before_this = false;
            }
            out_str.append(1, str[i]);
            break;
       }
    }

    typedef vector<string> TPositions;
    TPositions pos;
    NStr::Split(out_str, "-", pos);
    if (pos.size() < 3) {
        try {
            bool is_from = true;
            NON_CONST_ITERATE (TPositions, iter, pos) {
                NStr::TruncateSpaces(*iter);
                if (iter->empty()) continue;
                if (is_from) {
                    to = from = NStr::StringToLong(*iter);
                    is_from = false;
                } else {
                    to = NStr::StringToLong(*iter);
                }
            }
        } catch (const CException&) {
            return false;
        }
    }
    return true;
}

static const string kTaxDb = "taxonomy";
static const string kNucDb = "nucleotide";
static const string kAssmDb = "assembly";
static const int kRetMax = 5000;


void CSeqUtils::GetAssmIds_GIChr(TUids& gc_ids, TGi gi)
{
    gc_ids.clear();

    TGis uids_from;
    uids_from.push_back(gi);
    TUids uids_to;

    try {
        // prepare eLink request that will get entrez-id (not exactly the same as assembly ids)
        // chromosome is indicated by score of "2"
        // (magic string indicated in e-mail communication from Avi Kimchi on 04/01/2013)
        ELinkQuery(kNucDb, kAssmDb, uids_from, uids_to, "neighbor_score", "//Link[Score = \"2\"]/Id/text()");
    }
    catch (const CException& e) {
        LOG_POST(Error << "Failed to get assembly entrez ids for gi: " << gi << ". Error: " << e.GetMsg());
    }

    if (uids_to.empty())
        return;

    CGuiEutilsClient ecli;
    ecli.SetMaxReturn(kRetMax);
    xml::document docsums;

    try {
        // from Entrez ids, get true assembly ids
        ecli.Summary(kAssmDb, uids_to, docsums);
    } catch (const CException& e) {
        LOG_POST(Error << "Failed to get assembly ids from entrez ids: " << CreateIdStr(uids_to) << ". Error: " << e.GetMsg());
    }

    xml::node_set::const_iterator itNode;
    xml::node_set nodes ( docsums.get_root_node().run_xpath_query("//RsUid/text() | //GbUid/text()") );
    for (itNode = nodes.begin(); itNode != nodes.end(); ++itNode) {
        string id(itNode->get_content());
        if (id.empty())
            continue;
        gc_ids.push_back(NStr::StringToNonNegativeInt(id));
    }
}

string CSeqUtils::GetChrGI(TGi gi)
{
    TUids gc_ids;
    GetAssmIds_GIChr(gc_ids, gi);
    CRef<CGenomicCollectionsService> gcs(CGencollSvc::GetGenCollService());
    CSeq_id_Handle idh(CSeq_id_Handle::GetHandle(gi));

    ITERATE(TUids, iGCId, gc_ids) {
        try {
            CRef<CGC_Assembly> assm(gcs->GetAssembly(*iGCId, "Gbench_chrs"));

            CGC_Assembly::TSequenceList sequences;
            assm->Find(idh, sequences);

            ITERATE(CGC_Assembly::TSequenceList, iSequences, sequences) {
                CConstRef<CGC_Replicon> replicon((*iSequences)->GetReplicon());
                if(replicon->IsSetName()) {
                    return replicon->GetName();
                }
            }
        } catch(...) {
            LOG_POST(Error << "Call to GenColl timed out when getting assembly: " << *iGCId);
        }
    }
    return "";
}


// retrieve or cache list of ids for assembly accessions
static const TAssemblySeqIds& s_GetAssemblySeqIds(const string& assm_acc)
{
    {{
        CReadLockGuard lock(m_AssemblySeqIdLock);
        auto it = s_AssemblySeqIdCache.find(assm_acc);
        if (it != s_AssemblySeqIdCache.end()) {
            return it->second;
        }
    }}

    // Warning: this is potentially a very slow call
    // no caching because we are caching molecule list instaed as it's much smaller
    CRef<CGC_Assembly> assm = CGencollSvc::GetInstance()->GetGCAssembly(assm_acc, false, "Gbench");
    if(assm.IsNull()) 
        NCBI_THROW(CException, eUnknown, "Failed to retrieve Assembly for '" + assm_acc + "'");
    CGC_Assembly::TSequenceList top_level_seqs;
    assm->GetMolecules(top_level_seqs, CGC_Assembly::eTopLevel);
    TAssemblySeqIds ids;
    for (auto it : top_level_seqs) {
        const CSeq_id& tls_seq_id = it->GetSeq_id();
        ids.emplace_back(&tls_seq_id);
    }
    CWriteLockGuard lock(m_AssemblySeqIdLock);
    {{
        auto it = s_AssemblySeqIdCache.find(assm_acc);
        if (it != s_AssemblySeqIdCache.end()) 
            return it->second;
    }}
    s_AssemblySeqIdCache.emplace(assm_acc, ids);
    if (assm_acc != assm->GetAccession()) 
        s_AssemblySeqIdCache.emplace(assm->GetAccession(), ids);
    return s_AssemblySeqIdCache[assm_acc];
}

bool CSeqUtils::isTopLevel(const CSeq_id& seq_id, const string& assm_acc, CScope* scope)
{
    CBioseq_Handle handle(scope->GetBioseqHandle(seq_id));
    // filter out cases when the given id something unsuitable e.g. like a local id
    if(assm_acc.empty() || !CGencollSvc::isGenCollSequence(handle)) {
        return false;
    }
    {
        CSeq_id::EAccessionInfo info = seq_id.IdentifyAccession();
        // NCs are top level
        if (info == CSeq_id::eAcc_refseq_chromosome)  //NC
            return true;
    }

    string sNormalizedSeqId(seq_id.AsFastaString() + ":" + assm_acc);
    {{
        CReadLockGuard lock(m_TopLevelsLock);
        if(m_TopLevels.find(sNormalizedSeqId) != m_TopLevels.end()) {
            //        cerr << "found " << sNormalizedSeqId << " in top level cache with value: " << m_TopLevels[sNormalizedSeqId] << endl;
            return m_TopLevels[sNormalizedSeqId];
        }
    }}
    try {
        auto assembly_ids = s_GetAssemblySeqIds(assm_acc);
        if (scope && !assembly_ids.empty() && assembly_ids.front()->IsGi()) {
            auto gi = sequence::GetGiForId(seq_id, *scope);
            if (gi > ZERO_GI) {
                bool all_checked = true;
                for (const auto& id : assembly_ids)  {
                    if (!id->IsGi()) {
                        // not expected to happen 
                        // but we'll make sure that we check all the cases
                        all_checked = false;
                        continue;
                    }
                    if (id->GetGi() == gi) {
                        CWriteLockGuard lock(m_TopLevelsLock);
                        m_TopLevels[sNormalizedSeqId] = true;
                        return true;
                    }
                }
                if (all_checked) {
                    // all molecules are gi and they don't match our gi
                    CWriteLockGuard lock(m_TopLevelsLock);
                    m_TopLevels[sNormalizedSeqId] = false;
                    return false;
                }
            }
        }        
        // Here if our id or some of the assembly seqeunces are gi-less
        for (const auto& id : assembly_ids)  {
            if(Match(seq_id, *id, scope)) {
                CWriteLockGuard lock(m_TopLevelsLock);
                m_TopLevels[sNormalizedSeqId] = true;
                return true;
            }
        }
        // nothing found
        CWriteLockGuard lock(m_TopLevelsLock);
        m_TopLevels[sNormalizedSeqId] = false;

    } catch (exception& e) {
        LOG_POST(Error << "Call to GenColl timed out when getting assembly: " << assm_acc << ", " << e.what());
    }
    return false;
}


string CSeqUtils::GetChrId(const string& id_str, objects::CScope& scope) 
{
    TGi gi(ZERO_GI);
    if(!id_str.empty() ) {
        CRef<objects::CSeq_id> seq_id(new objects::CSeq_id);
        seq_id->Set(id_str);
        objects::CBioseq_Handle bsh = scope.GetBioseqHandle(*seq_id);
        objects::CSeq_id_Handle shdl = bsh.GetAccessSeq_id_Handle();
        shdl = sequence::GetId(shdl, scope, sequence::eGetId_ForceGi);
        if (shdl) {
            gi = shdl.GetGi();
        }
    }
    return CSeqUtils::GetChrGI(gi);
}


void CSeqUtils::GetAssmAccs_Gi(TAccs& accs, TGi gi)
{
    accs.clear();
    TUids gc_ids;
    GetAssmIds_GI(gc_ids, gi);

    if(gc_ids.empty())
        return;

    xml::document docsums;
    CGuiEutilsClient ecli;
    ecli.SetMaxReturn(kRetMax);

    try {
        ecli.Summary(kAssmDb, gc_ids, docsums);
    }
    catch (const CException& e) {
        LOG_POST(Error << "Failed to get summary for the following assemblies: " << CreateIdStr(gc_ids) << ". Error: " << e.GetMsg());
        return;
    }

    // Using "gcassembly" Entrez, you will not get the the GB assemblies
    // for some cases (those paired to RS assemblies) as separate entries
    // from an Entrez search. But if you are looking at the Entrez Docsums,
    // there is a field "GbUid" that tells you the release id of the
    // corresponding GB, and fields <Synonym>/<Genbank> and <Synonym>/<RefSeq>
    // will tell you the accessions of both of them. (JIRA: GCOL-1493)
    xml::node_set doc_sums ( docsums.get_root_node().run_xpath_query("//DocumentSummary[contains(AssemblyAccession/text(),'GCF_') and GbUid/text()!=\"\"]/Synonym/Genbank/text()") );
    xml::node_set::const_iterator itAcc;
    for (itAcc = doc_sums.begin(); itAcc != doc_sums.end(); ++itAcc) {
        string acc(itAcc->get_content());
        if (acc.empty())
            continue;
        accs.insert(acc);
    }
}

string CSeqUtils::CreateIdStr(const TUids& uids)
{
    stringstream idstrm;
    size_t count = uids.size();
    if (count) {
        idstrm << uids[0];
        for (size_t i=1; i<count; ++i)
            idstrm << ',' << uids[i];
    }
    return idstrm.str();
}

template<class T1, class T2>
static void s_ELinkQuery(const string &db_from, const string &db_to, const vector<T1> &uids_from, vector<T2> &uids_to, const string &cmd, const string &xpath)
{
    xml::document xmldoc;
    CSeqUtils::ELinkQuery(db_from, db_to, uids_from, xmldoc, cmd);

    xml::node_set links ( xmldoc.get_root_node().run_xpath_query(xpath.c_str()) );
    xml::node_set::const_iterator itLink;
    for (itLink = links.begin(); itLink != links.end(); ++itLink) {
        string id(itLink->get_content());
        if (id.empty())
            continue;
        uids_to.push_back(NStr::StringToNumeric<T2>(id));
    }
}

void CSeqUtils::ELinkQuery(const string &db_from, const string &db_to, const TUids &uids_from, TUids &uids_to, const string &cmd, const string &xpath)
{
    s_ELinkQuery(db_from, db_to, uids_from, uids_to, cmd, xpath);
}

#ifdef NCBI_INT8_GI
void CSeqUtils::ELinkQuery(const string &db_from, const string &db_to, const TGis &uids_from, TGis &uids_to, const string &cmd, const string &xpath)
{
    s_ELinkQuery(db_from, db_to, uids_from, uids_to, cmd, xpath);
}

void CSeqUtils::ELinkQuery(const string &db_from, const string &db_to, const TGis &uids_from, TUids &uids_to, const string &cmd, const string &xpath)
{
    s_ELinkQuery(db_from, db_to, uids_from, uids_to, cmd, xpath);
}

void CSeqUtils::ELinkQuery(const string &db_from, const string &db_to, const TUids &uids_from, TGis &uids_to, const string &cmd, const string &xpath)
{
    s_ELinkQuery(db_from, db_to, uids_from, uids_to, cmd, xpath);
}

void CSeqUtils::ELinkQuery(const string &db_from, const string &db_to, const TSeqIdHandles &uids_from, TGis &uids_to, const string &cmd, const string &xpath)
{
    s_ELinkQuery(db_from, db_to, uids_from, uids_to, cmd, xpath);
}

#endif

void CSeqUtils::ELinkQuery(const string &db_from, const string &db_to, const TSeqIdHandles &uids_from, TUids &uids_to, const string &cmd, const string &xpath)
{
    s_ELinkQuery(db_from, db_to, uids_from, uids_to, cmd, xpath);
}

template<class T>
static void s_ELinkQuery(const string &db_from, const string &db_to, const vector<T> &uids_from, xml::document& linkset, const string &cmd)
{
    CGuiEutilsClient ecli;
    ecli.SetMaxReturn(kRetMax);
    CNcbiStrstream xml;

    ecli.Link(db_from, db_to, uids_from, xml, cmd);

    stringbuf sb;
    xml >> &sb;
    string docstr(sb.str());

/* DEBUG dump
    if (uids_from[0] == 56782) {
        CNcbiOfstream ofs;
        ofs.open("56782.xml", std::ofstream::out);
        ofs << docstr;
        ofs.close();
    }
 */
    xml::document doc(docstr.data(), docstr.size(), NULL);
    linkset.swap(doc);
}

void CSeqUtils::ELinkQuery(const string &db_from, const string &db_to, const TUids &uids_from, xml::document& linkset, const string &cmd)
{
    s_ELinkQuery(db_from, db_to, uids_from, linkset, cmd);
}

#ifdef NCBI_INT8_GI
void CSeqUtils::ELinkQuery(const string &db_from, const string &db_to, const TGis &uids_from, xml::document& linkset, const string &cmd)
{
#ifdef NCBI_STRICT_GI
    vector<TEntrezId> entrez_uids;
    for (TGi gi : uids_from) {
        entrez_uids.push_back(GI_TO(TEntrezId, gi));
    }
#else
    const TGis &entrez_uids = uids_from;
#endif
    s_ELinkQuery(db_from, db_to, entrez_uids, linkset, cmd);
}
#endif

void CSeqUtils::ELinkQuery(const string &db_from, const string &db_to, const TSeqIdHandles &uids_from, xml::document& linkset, const string &cmd)
{
    s_ELinkQuery(db_from, db_to, uids_from, linkset, cmd);
}


template<class T>
static void s_ESearchQuery(const string &db, const string &term, vector<T> &uids, size_t &count, const int ret_max, const string &xpath)
{
    CGuiEutilsClient ecli;
	ecli.SetMaxReturn(ret_max);
	CNcbiStrstream xml;

	ecli.Search(db, term, xml);

	stringbuf sb;
	xml >> &sb;
	string docstr(sb.str());

    xml::document xmldoc(docstr.data(), docstr.size(), NULL);
	xml::node_set links(xmldoc.get_root_node().run_xpath_query(xpath.c_str()));
	xml::node_set::const_iterator itLink;
	for (itLink = links.begin(); itLink != links.end(); ++itLink) {
		string id(itLink->get_content());
		if (id.empty())
			continue;
		uids.push_back(NStr::StringToNumeric<T>(id));
	}
	string countStr = CSeqUtils::GetXmlChildNodeValue(xmldoc.get_root_node(), "Count");
	if (!countStr.empty())
		count = NStr::StringToSizet(countStr);
	else
		count = uids.size();
}

void CSeqUtils::ESearchQuery(const string &db, const string &term, TUids &uids, size_t &count, const int ret_max, const string &xpath)
{
    s_ESearchQuery(db, term, uids, count, ret_max, xpath);
}

#ifdef NCBI_INT8_GI
void CSeqUtils::ESearchQuery(const string &db, const string &term, TGis &uids, size_t &count, const int ret_max, const string &xpath)
{
#ifdef NCBI_STRICT_GI
    vector<TEntrezId> entrez_uids;
#else
    TGis &entrez_uids = uids;
#endif
    s_ESearchQuery(db, term, entrez_uids, count, ret_max, xpath);
#ifdef NCBI_STRICT_GI
    for (TEntrezId id : entrez_uids) {
        uids.push_back(GI_FROM(TEntrezId, id));
    }
#endif
}
#endif


template<class T>
static void s_ESearchQuery(const string &db, const string &term, const string &web_env, const string &query_key, vector<T> &uids, size_t &count, int retstart, const string &xpath)
{
    xml::document xmldoc;
    CSeqUtils::ESearchQuery(db, term, web_env, query_key, xmldoc, retstart);

    xml::node_set links ( xmldoc.get_root_node().run_xpath_query(xpath.c_str()) );
    xml::node_set::const_iterator itLink;
    for (itLink = links.begin(); itLink != links.end(); ++itLink) {
        string id(itLink->get_content());
        if (id.empty())
            continue;
        uids.push_back(NStr::StringToNumeric<T>(id));
    }
	string countStr = CSeqUtils::GetXmlChildNodeValue(xmldoc.get_root_node(), "Count");
	if (!countStr.empty())
		count = NStr::StringToSizet(countStr);
	else
		count = uids.size();
}

void CSeqUtils::ESearchQuery(const string &db, const string &term, const string &web_env, const string &query_key, TUids &uids, size_t &count, int retstart, const string &xpath)
{
    s_ESearchQuery(db, term, web_env, query_key, uids, count, retstart, xpath);
}

#ifdef NCBI_INT8_GI
void CSeqUtils::ESearchQuery(const string &db, const string &term, const string &web_env, const string &query_key, TGis &uids, size_t &count, int retstart, const string &xpath)
{
    s_ESearchQuery(db, term, web_env, query_key, uids, count, retstart, xpath);
}
#endif

void CSeqUtils::ESearchQuery(const string &db, const string &term, const string &web_env, const string &query_key, xml::document &searchset, int retstart)
{
    CGuiEutilsClient ecli;
    ecli.SetMaxReturn(kRetMax);
    CNcbiStrstream xml;

    ecli.SearchHistory(db, term, web_env, NStr::StringToNumeric<Int8>(query_key), retstart, xml);

    stringbuf sb;
    xml >> &sb;
    string docstr(sb.str());

    xml::document doc(docstr.data(), docstr.size(), NULL);
    searchset.swap(doc);
}

std::string CSeqUtils::GetXmlChildNodeValue(const xml::node& parent, const std::string& name)
{
    xml::node::const_iterator itNode = parent.find(name.c_str());
    do {
        if (parent.end() == itNode)
            break;

        const char* value = itNode->get_content();

        if (!value)
            break;

        return string(value);
    }
    while(false);
    return string();
}

void CSeqUtils::GetAssmIds_GI(TUids& gc_ids, TGi gi)
{
    gc_ids.clear();

    TGis uids_from;
    uids_from.push_back(gi);

    CGuiEutilsClient ecli;
    ecli.SetMaxReturn(kRetMax);

    try {
        ELinkQuery(kNucDb, kAssmDb, uids_from, gc_ids);
    }
    catch (const CException& e) {
        LOG_POST(Error << "Failed to get assembly ids for gi: " << gi << ". Error: " << e.GetMsg());
    }
}

/// For CDS and RNA feature mapping information
typedef vector< CRef<CSeq_loc> > TMappedLocs;

static TMappedLocs s_GetRnaMappingLocs(
    const CSeq_loc& feat_loc,
    const CMappedFeat& feat,
    const CBioseq_Handle& handle)
{
    TMappedLocs locs;
    CConstRef<CSeq_align> align_ref;
    CScope& scope = handle.GetScope();
    TSeqRange range = feat_loc.GetTotalRange();

    if (!feat.IsSetProduct()) return locs;
    const CSeq_id& product_id = *feat.GetProduct().GetId();

    // check if there is any alignment associated with the
    // product sequence
    SAnnotSelector sel;
    sel.SetAdaptiveDepth(true);
    sel.SetExactDepth(false);
    sel.SetResolveAll();
    sel.SetResolveDepth(1);
    sel.ExcludeNamedAnnots("SNP");
    sel.ExcludeNamedAnnots("STS");
    sel.ExcludeNamedAnnots("CDD");  
    CSeq_annot_Handle annot = feat.GetAnnot();
    if (annot && annot.IsNamed()) {
        const string& annot_str = annot.GetName();
        sel.AddNamedAnnots(annot_str);
        if (NStr::StartsWith(annot_str, "NA0")) {
            sel.IncludeNamedAnnotAccession(annot_str);
        }
    }

    CConstRef<CSeq_loc> aln_loc(
        handle.GetRangeSeq_loc(range.GetFrom(), range.GetTo()) );
    CAlign_CI align_iter(scope, *aln_loc, sel);

    while (align_iter  &&  !align_ref) {
        // find the first seq-align that matches the product sequence
        const CSeq_align& align = *align_iter;
        CSeq_align::TDim num_row = align.CheckNumRows();
        if (num_row != 2) continue;
        for (CSeq_align::TDim row = 0;  row < num_row;  ++row) {
            if (product_id.Match(align.GetSeq_id(row))) {
                if (align.GetSegs().IsSpliced()) {
                    // CSeq_loc_Mapper doesn't map location correctly for spliced-seg.
                    // This is a work-around to convert spliced-seg to denseg.
                    align_ref = ConvertSeq_align(align, CSeq_align::TSegs::e_Denseg);
                } else {
                    align_ref.Reset(&align);
                }
                break;
            }
        }
        ++align_iter;
    }

    CRef<CSeq_loc_Mapper> mapper;
    CRef<CSeq_loc_Mapper> back_mapper;

    // No alignment found
    if (align_ref) {
        mapper.Reset(new CSeq_loc_Mapper(*align_ref, product_id, &scope));
        back_mapper.Reset(new CSeq_loc_Mapper(*align_ref, *feat_loc.GetId(), &scope));
    } else {
        const CSeq_feat& mapped_feat = feat.GetMappedFeature();
        mapper.Reset(new CSeq_loc_Mapper(mapped_feat, CSeq_loc_Mapper::eLocationToProduct, &scope));
        back_mapper.Reset(new CSeq_loc_Mapper(mapped_feat, CSeq_loc_Mapper::eProductToLocation, &scope));
    }

    CRef<CSeq_loc> prod_loc = mapper->Map(feat_loc);
    CRef<CSeq_loc> gen_loc = back_mapper->Map(*prod_loc);
    locs.push_back(prod_loc);
    locs.push_back(gen_loc);

    return locs;
}



/// Helper function to convert two mapped locations into
/// an interval-to-interval mapping structure
static void s_CreateMappingInfo(
    const CSeq_loc& prod_loc,
    const CSeq_loc& gen_loc,
    CSeqUtils::TMappingInfo& info)
{
    // We assume both contains exactly the same number of intervals, and
    // each has the same length
    CSeq_loc_CI it1(prod_loc);
    CSeq_loc_CI it2(gen_loc);
    CRef<CSeq_id> gen_id(new CSeq_id);
    gen_id->Assign(*gen_loc.GetId());
    CRef<CSeq_id> prod_id(new CSeq_id);
    prod_id->Assign(*prod_loc.GetId());
    while (it1  &&  it2) {
        TSeqRange r1 = it1.GetRange();
        TSeqRange r2 = it2.GetRange();
        if (it1.IsEmpty()  ||  it2.IsEmpty()  ||
            r1.GetLength() != r2.GetLength()) {
            info.clear();
            return;
        }

        CRef<CSeq_interval> int1;
        CRef<CSeq_interval> int2;
        int1.Reset(new CSeq_interval);
        int1->SetFrom(r1.GetFrom());
        int1->SetTo(r1.GetTo());
        int1->SetId(*prod_id);
        if (it1.IsSetStrand()) {
            int1->SetStrand(it1.GetStrand());
        }
        int2.Reset(new CSeq_interval);
        int2->SetFrom(r2.GetFrom());
        int2->SetTo(r2.GetTo());
        int2->SetId(*gen_id);
        if (it2.IsSetStrand()) {
            int2->SetStrand(it2.GetStrand());
        }
        info.push_back(CSeqUtils::TMappingInfo::value_type(int1, int2));
        ++it1; ++it2;
    }

    // Two locations don't match
    if (it1  ||  it2) {
        info.clear();
    }
}


CSeqUtils::TMappingInfo CSeqUtils::GetRnaMappingInfo(
    const objects::CSeq_loc& feat_loc,
    const CMappedFeat&       feat,
    const objects::CBioseq_Handle& handle)
{
    TMappingInfo info;
    TMappedLocs locs = s_GetRnaMappingLocs(feat_loc, feat, handle);
    if (locs.size() == 2) {
        s_CreateMappingInfo(*locs[0], *locs[1], info);
    }
    return info;
}


CSeqUtils::TMappingInfo CSeqUtils::GetCdsMappingInfoFromRna(
    const TMappingInfo& mapping_info,
    const CSeq_id& rna_product_id,
    const CSeq_loc& feat_loc,
    const CSeq_loc& product,
    CScope& scope)
{
    const CSeq_id& product_id = *product.GetId();

    /// The start offset between CDS product sequence
    /// and its parent product sequence
    int cds_offset = -1;
    {
        // try if the parent RNA product sequence contain a
        // CDS feature with the same product sequence as 'product_id'
        CBioseq_Handle rna_bsh = scope.GetBioseqHandle(rna_product_id);
        if (rna_bsh) {
            SAnnotSelector sel;
            sel.SetAdaptiveDepth(true);
            sel.SetExactDepth(false);
            sel.SetResolveAll();
            sel.SetResolveDepth(1);
            sel.ExcludeNamedAnnots("SNP");
            sel.ExcludeNamedAnnots("STS");
            sel.IncludeFeatSubtype(CSeqFeatData::eSubtype_cdregion);
            CFeat_CI feat_iter(rna_bsh, TSeqRange::GetWhole(), sel);
            for (; feat_iter; ++feat_iter) {
                const CSeq_feat& cds_feat = feat_iter->GetMappedFeature();
                if (cds_feat.CanGetProduct()  &&
                    product_id.Match(*cds_feat.GetProduct().GetId())) {
                        cds_offset = (int)cds_feat.GetLocation().GetTotalRange().GetFrom();
                        break;
                }
            }
        }
    }

    CRef<CSeq_id> gen_id(new CSeq_id);
    gen_id->Assign(*feat_loc.GetId());
    CRef<CSeq_id> prod_id(new CSeq_id);
    prod_id->Assign(product_id);

    /// CDS biological range on the genomic sequence
    TSeqPos bio_start = feat_loc.GetStart(eExtreme_Biological);
    TSeqPos bio_stop = feat_loc.GetStop(eExtreme_Biological);

    /// truncate the rna mapping info using the CDS
    /// feature biological start and stop, and create
    /// the mapping info for the CDS feature by applying
    /// the cds-to-rna shift.
    TMappingInfo cds_map_info;
    TMappingInfo::const_iterator iter = mapping_info.begin();
    bool done = false;
    while (iter != mapping_info.end()  &&  !done) {
        auto& gen_int = iter->second;
        auto& prod_int = iter->first;

        TSeqPos gen_from = gen_int->GetFrom();
        TSeqPos gen_to = gen_int->GetTo();
        TSeqPos prod_from = prod_int->GetFrom();
        TSeqPos prod_to = prod_int->GetTo();
        bool reverse = gen_int->CanGetStrand() && gen_int->GetStrand() == eNa_strand_minus;

        TSeqRange r1, r2;
        if (cds_map_info.empty()) {
            if (bio_start >= gen_from  &&  bio_start <= gen_to) {
                // find the first interval intersecting with CDS location
                TSeqPos off1 = reverse ? gen_to - bio_start : bio_start - gen_from;
                r1.Set(prod_from + off1, prod_to);
                if (reverse) {
                    r2.Set(gen_from, gen_to - off1);
                } else {
                    r2.Set(gen_from + off1, gen_to);
                }
                if (cds_offset < 0  ||  (TSeqPos)cds_offset > r1.GetFrom()) {
                    cds_offset = int(r1.GetFrom());
                }
            } // else, it is outside of the CDS range, skip it
        } else {
            r1.Set(prod_from, prod_to);
            r2.Set(gen_from, gen_to);
        }

        // check if it intersects with the biological stop position
        if (!r1.Empty()  &&  !r2.Empty()) {
            if (bio_stop >= gen_from  &&  bio_stop <= gen_to) {
                // find the last interval intersection with CDS location
                TSeqPos off2 = reverse ? bio_stop - gen_from : gen_to - bio_stop;
                r1.SetTo(r1.GetTo() - off2);
                if (reverse) r2.SetFrom(r2.GetFrom() + off2);
                else r2.SetTo(r2.GetTo() - off2);

                // set termination flag
                done = true;
            }

            CRef<CSeq_interval> int1(new CSeq_interval);
            int1->SetFrom(r1.GetFrom() - cds_offset);
            int1->SetTo(r1.GetTo() - cds_offset);
            int1->SetId(*prod_id);
            int1->SetStrand(eNa_strand_plus);

            CRef<CSeq_interval> int2(new CSeq_interval);
            int2->SetFrom(r2.GetFrom());
            int2->SetTo(r2.GetTo());
            int2->SetId(*gen_id);
            int2->SetStrand(reverse ? eNa_strand_minus : eNa_strand_plus);

            // create intervals and push them to the info map
            cds_map_info.push_back(TMappingInfo::value_type(int1, int2));
        }

        ++iter;
    }

    // Final step to determine if the mapping info is necessary for
    // the given cds feature.
    if ( !cds_map_info.empty() ) {
        // Create the mapped seq-loc on genomic and product sequence
        CRef<CSeq_loc> gen_loc(new CSeq_loc);
        CRef<CSeq_loc> prod_loc(new CSeq_loc);
        ITERATE (TMappingInfo, iter, cds_map_info) {
            prod_loc->SetPacked_int().Set().push_back(iter->first);
            gen_loc->SetPacked_int().Set().push_back(iter->second);
        }
        prod_loc->Merge(CSeq_loc::fMerge_AbuttingOnly, NULL);
        if (prod_loc->GetStart(eExtreme_Biological) == 0  &&
            prod_loc->GetPacked_int().Get().size() == 1  &&
            gen_loc->CompareSubLoc(feat_loc, eNa_strand_plus) == 0) {
            // All of above confidtions are met. the mapping info
            // is not necessary
            cds_map_info.clear();
        }
    }

    return cds_map_info;
}


CSeqUtils::TMappingInfo CSeqUtils::GetMappingInfoFromLocation(
        const objects::CSeq_loc &feat_loc,
        const objects::CSeq_id  &product_id,
        int feat_offset
        )
{
    TMappingInfo info;
    CRef<CSeq_id> gen_id(new CSeq_id);
    gen_id->Assign(*feat_loc.GetId());
    CRef<CSeq_id> prod_id(new CSeq_id);
    prod_id->Assign(product_id);
    TSeqPos start = 0;
    CSeq_loc_CI it(feat_loc);
    while (it) {
        TSeqRange r = it.GetRange();
        auto int1 = Ref(new CSeq_interval(*prod_id, start, start + r.GetLength() - 1, eNa_strand_plus));
        start += r.GetLength();
        auto int2 = Ref(new CSeq_interval(*gen_id, r.GetFrom() + feat_offset, r.GetTo(), it.IsSetStrand() ?
                                              it.GetStrand() : eNa_strand_plus));
        info.emplace_back(int1, int2);
        feat_offset = 0;
        ++it;
    }
    return info;
}


CConstRef<CSeq_loc> CSeqUtils::GetFeatLocation(const CSeq_feat& feat, const CBioseq_Handle& bsh)
{
    const CSeq_feat_Base::TLocation& loc_obj = feat.GetLocation();
    CConstRef<CSeq_loc> loc;
    /// guard against the cases where the feature contains location
    /// with multiple seq-ids
    if ( !loc_obj.GetId() ) {
        loc = CSeqUtils::MixLocToLoc(loc_obj, bsh);
    }
    if ( !loc ) {
        loc.Reset(&loc_obj);
    }
    return loc;
}

CMappedFeat CSeqUtils::GetMrnaForCds(const CMappedFeat &cds_feat, const string &named_acc)
{
    try {
        if (!named_acc.empty()) {
            SAnnotSelector sel;
            sel.SetAdaptiveDepth(true);
            sel.SetExactDepth(false);
            sel.SetResolveAll();
            sel.SetResolveDepth(1);
            sel.ExcludeNamedAnnots("SNP");
            sel.ExcludeNamedAnnots("STS");
            sel.IncludeFeatSubtype(CSeqFeatData::eSubtype_mRNA);
            sel.ExcludeUnnamedAnnots();
            sel.IncludeNamedAnnotAccession(named_acc);
            return feature::GetBestMrnaForCds(cds_feat, nullptr, &sel);
        }
        else {
            return feature::GetBestMrnaForCds(cds_feat);
        }
    }
    catch(const CException&)
    {
    }

    return CMappedFeat();
}

void CSeqUtils::GetMappingInfo(const CMappedFeat &mapped_feat, const CBioseq_Handle& bsh, CSeqUtils::TMappingInfo &info, const string &annot)
{
    const CSeq_feat& feat = mapped_feat.GetMappedFeature();
    CSeqFeatData::E_Choice type = feat.GetData().Which();
    CSeqFeatData::ESubtype subtype = feat.GetData().GetSubtype();
    CConstRef<CSeq_loc> loc = CSeqUtils::GetFeatLocation(feat, bsh);
    if (feat.IsSetProduct()) {
        if (feat.GetData().IsRna()) {
            info = CSeqUtils::GetRnaMappingInfo(*loc, mapped_feat, bsh);
        }
        else if (type == CSeqFeatData::e_Cdregion) {
            do {
                const CSeq_loc& product = feat.GetProduct();
                CMappedFeat mapped_mrna = CSeqUtils::GetMrnaForCds(mapped_feat, annot);
                if (!mapped_mrna || !mapped_mrna.GetOriginalFeature().IsSetProduct()) {
                    info = CSeqUtils::GetMappingInfoFromLocation(*loc,*product.GetId());
                    break;
                }
                CConstRef<CSeq_loc> mrna_loc = CSeqUtils::GetFeatLocation(mapped_mrna.GetMappedFeature(), bsh);
                CSeqUtils::TMappingInfo mrna_info = CSeqUtils::GetRnaMappingInfo(*mrna_loc, mapped_mrna, bsh);
                if (mrna_info.empty())
                    break;
                info = CSeqUtils::GetCdsMappingInfoFromRna(mrna_info, *(mapped_mrna.GetOriginalFeature().GetProduct().GetId()), *loc, product, bsh.GetScope());
            } while (false);
        }
    }
    else {
        if ((type == CSeqFeatData::e_Cdregion) || (subtype == CSeqFeatData::eSubtype_V_segment) || feat.GetData().IsRna()) {
            // We assume both contains exactly the same number of intervals, and
            // each has the same length
            static unsigned id_num{ 0 };
            CRef<CSeq_id> prod_id(new CSeq_id("lcl|pseudo" + NStr::IntToString(++id_num)));
            info = CSeqUtils::GetMappingInfoFromLocation(*loc,*prod_id);
        }
    }
}


bool CSeqUtils::GetGIString(const string& sid, string* gi_str)
{
    bool match = false;

    if (!gi_str) return match;
    gi_str->clear();

    SIZE_TYPE pos = NStr::FindNoCase(sid, "gi|");
    if (pos != NPOS) {
        match = true;
        *gi_str = "gi|";
        for (size_t i = pos + 3; i < sid.length(); ++i) {
            char c = sid.at(i);
            if (isdigit(c)) {
                gi_str->append(1, c);
            } else {
                break;
            }
        }
    }
    return match;
}

int CSeqUtils::GetGenCode(const objects::CBioseq_Handle& handle)
{
    // get an appropriate translation table. For the full list of tables,
    // please refer to https://www.ncbi.nlm.nih.gov/Taxonomy/Utils/wprintgc.cgi
    int gencode = 1;
    try {
        CSeqdesc_CI desc_it(handle, CSeqdesc::e_Source);
        if (desc_it) {
            const CBioSource& src = desc_it->GetSource();
            gencode = src.GetGenCode();
        }
    } catch (CException&) {
        // ignore it, will try other approach
    }
    return gencode;
}


CConstRef<CBioseq> GetBioseqForSeqdesc (CRef<CScope> scope, const CSeqdesc& seq_desc)
{
    CConstRef<CBioseq> bioseq;

    if (!scope) {
        return bioseq;
    }

    CScope::TTSE_Handles tses;
    scope->GetAllTSEs(tses, CScope::eAllTSEs);
    ITERATE (CScope::TTSE_Handles, handle, tses) {
        for (CBioseq_CI bioseq_it(*handle);  bioseq_it;  ++bioseq_it) {
            // Is Seqdesc on this Bioseq?
            if (bioseq_it->IsSetDescr()) {
                CConstRef<CBioseq> r_bioseq = bioseq_it->GetCompleteBioseq();
                ITERATE (CBioseq::TDescr::Tdata, dit, r_bioseq->GetDescr().Get()) {
                    if (dit->GetPointer() == &seq_desc) {
                        return r_bioseq;
                    }
                }
            }
        }
    }
    return bioseq;
}


CSeq_entry_Handle GetSeqEntryForPubdesc (CRef<CScope> scope, const CPubdesc& pubdesc)
{
    CSeq_entry_Handle seh;

    if (!scope) {
        return seh;
    }

    CScope::TTSE_Handles tses;
    scope->GetAllTSEs(tses, CScope::eAllTSEs);
    ITERATE (CScope::TTSE_Handles, handle, tses) {
        for (CSeq_entry_CI entry_ci(*handle, CSeq_entry_CI::fRecursive | CSeq_entry_CI::fIncludeGivenEntry); entry_ci; ++entry_ci) {
            if (entry_ci->IsSetDescr()) {
                ITERATE (CBioseq::TDescr::Tdata, dit, entry_ci->GetDescr().Get()) {
                    if ((*dit)->IsPub()) {
                        const CPubdesc& desc_pub = (*dit)->GetPub();
                        if (&desc_pub == &pubdesc) {
                            return *entry_ci;
                        }
                    }
                }
            }
            if (entry_ci->IsSeq()) {
                for (CFeat_CI fi(entry_ci->GetSeq(), SAnnotSelector(CSeqFeatData::e_Pub)); fi; ++fi) {
                    if (&(fi->GetData().GetPub()) == &pubdesc) {
                        return *entry_ci;
                    }
                }
            }
        }
    }
    return seh;
}



CSeq_entry_Handle GetDefaultTopLevelSeqEntry(CScope& scope)
{
    CSeq_entry_Handle seh;

    CScope::TTSE_Handles handles;
    scope.GetAllTSEs(handles);
    if (handles.size() > 0) {
        seh = handles.front().GetTopLevelEntry();
    }
    return seh;
}


CSeq_feat_Handle GetSeqFeatHandleForBadLocFeature(const CSeq_feat& feat, CScope& scope)
{
    CScope::TTSE_Handles tse_list;
    scope.GetAllTSEs(tse_list);
    ITERATE(CScope::TTSE_Handles, tse, tse_list) {
        CFeat_CI f(*tse);
        while (f) {
            if (f->GetSeq_feat() == &feat) {
                return *f;
            }
            ++f;
        }
    }
    CSeq_feat_Handle fh;
    return fh;
}


CBioseq_Handle GetBioseqForSeqFeat(const CSeq_feat& f, CScope& scope)
{
    CBioseq_Handle bsh;
    if (f.IsSetLocation()) {
        CSeq_loc_CI subloc(f.GetLocation());
        if (subloc && !subloc.IsEmpty()) {
            bsh = scope.GetBioseqHandle(subloc.GetSeq_id());
        }
    }
    if (!bsh) {
        CSeq_feat_Handle fh = GetSeqFeatHandleForBadLocFeature(f, scope);
        if (fh) {
            CSeq_entry_Handle seh = fh.GetAnnot().GetParentEntry();
            if (seh.IsSeq()) {
                bsh = seh.GetSeq();
            }
        }
    }
    return bsh;
}


CSeq_entry_Handle GetTopSeqEntryFromScopedObject (SConstScopedObject& obj)
{
    const CObject* ptr = obj.object.GetPointer();

    /// CSeq_entry
    const objects::CSeq_entry* seqEntry = dynamic_cast<const objects::CSeq_entry*>(ptr);
    const objects::CBioseq* bioseq = dynamic_cast<const objects::CBioseq*>(ptr);
    const objects::CBioseq_set* bioseq_set = dynamic_cast<const objects::CBioseq_set*>(ptr);
    const objects::CSeq_annot* seqannot = dynamic_cast<const objects::CSeq_annot*>(ptr);
    const objects::CSeq_feat* seqfeat = dynamic_cast<const objects::CSeq_feat*>(ptr);
    const objects::CSeqdesc* seqdesc = dynamic_cast<const objects::CSeqdesc*>(ptr);
    const objects::CSeq_submit* seqsubmit = dynamic_cast<const objects::CSeq_submit*>(ptr);
    const objects::CPubdesc* pubdesc = dynamic_cast<const objects::CPubdesc*>(ptr);
    const objects::CSeq_loc* loc = dynamic_cast<const objects::CSeq_loc*>(ptr);
    const objects::CSeq_id* seq_id = dynamic_cast<const objects::CSeq_id*>(ptr);

    objects::CSeq_entry_Handle seh;
    if (!obj.scope)
        return seh;

    if (seqEntry) {
        seh = obj.scope->GetObjectHandle (*seqEntry, CScope::eMissing_Null);
        if (seh)
            seh = seh.GetTopLevelEntry();
    } else if(bioseq) {
        CBioseq_Handle bsh = obj.scope->GetObjectHandle (*bioseq, CScope::eMissing_Null);
        if (bsh)
            seh = bsh.GetTopLevelEntry();
    } else if(bioseq_set) {
        CBioseq_set_Handle bssh = obj.scope->GetObjectHandle(*bioseq_set, CScope::eMissing_Null);
        if (bssh) {
            seh = bssh.GetTopLevelEntry();
        }
    } else if(seqannot) {
        auto sah = obj.scope->GetObjectHandle (*seqannot, CScope::eMissing_Null);
        if (sah)
            seh = sah.GetTopLevelEntry();
    } else if(seqfeat) {
        CBioseq_Handle bsh = GetBioseqForSeqFeat(*seqfeat, *(obj.scope));
        if (bsh) {
            seh = bsh.GetTopLevelEntry();
        }
    } else if (seqdesc) {
        seh = edit::GetSeqEntryForSeqdesc(obj.scope, *seqdesc);
        if (seh) {
            seh = seh.GetTopLevelEntry();
        } else {
            seh = GetDefaultTopLevelSeqEntry(*obj.scope);
        }
    } else if (pubdesc) {
        seh = GetSeqEntryForPubdesc(obj.scope, *pubdesc);
        if (seh) {
            seh = seh.GetTopLevelEntry(); // GB-3727
        } else {
            seh = GetDefaultTopLevelSeqEntry(*obj.scope);
        }
    } else if (seqsubmit) {
        if (seqsubmit->IsEntrys() && seqsubmit->GetData().GetEntrys().front()) {
            seh = obj.scope->GetSeq_entryHandle(*(seqsubmit->GetData().GetEntrys().front()), CScope::eMissing_Null);
        }
    } else if (loc) {
        CBioseq_Handle bsh = obj.scope->GetBioseqHandle(*loc);
        if (bsh) {
            seh = bsh.GetTopLevelEntry();
        }
    } else if (seq_id) {
        CBioseq_Handle bsh = obj.scope->GetBioseqHandle(*seq_id);
        if (bsh) {
            seh = bsh.GetTopLevelEntry();
        }
    }
    return seh;
}


static bool s_CompareDescriptions (
    const CFeatListItem * p1,
    const CFeatListItem * p2
)

{
    string str1 = p1->GetDescription();
    string str2 = p2->GetDescription();

    char ch1 = str1.c_str()[0];
    char ch2 = str2.c_str()[0];
    // starts with a number -> goes at the end of the list
    bool num1 = isdigit(ch1);
    bool num2 = isdigit(ch2);
    if (num1 && num2) {
        return NStr::Compare(str1, str2, NStr::eNocase) < 0;
    } else if (num1) {
        return false;
    } else if (num2) {
        return true;
    }

    // starts with a tilde or dash - sort with other tildes,
    // put before numbers after alphas
    if (ch1 == '~' && ch2 == '~') {
        return NStr::Compare(str1, str2, NStr::eNocase) < 0;
    } else if (ch1 == '~') {
        return false;
    } else if (ch2 == '~') {
        return true;
    }
    if (ch1 == '-' && ch2 == '-') {
        return NStr::Compare(str1, str2, NStr::eNocase) < 0;
    } else if (ch1 == '-') {
        return false;
    } else if (ch2 == '-') {
        return true;
    }

    return NStr::Compare(p1->GetDescription(), p2->GetDescription(), NStr::eNocase) < 0;
}


vector<const CFeatListItem * > GetSortedFeatList(CSeq_entry_Handle seh, size_t max)
{
    vector<const CFeatListItem * > r_list;

    vector<bool> present(CSeqFeatData::eSubtype_max, false);
    size_t count = 0;
    if (seh) {
        CFeat_CI fi(seh);
        while (fi) {
            present[fi->GetData().GetSubtype()] = true;
            ++fi;
            ++count;
            if (count > max)
                break;
        }
    }

    set<string> existing;

    vector<const CFeatListItem * > used;
    vector<const CFeatListItem * > popular;
    vector<const CFeatListItem * > import_feats;
    vector<const CFeatListItem * > least_liked;
    vector<const CFeatListItem * > unused;
    const CFeatListItem * all = NULL;

    const CFeatList* feat_list = CSeqFeatData::GetFeatList();
    ITERATE(CFeatList, ft_it, *feat_list) {
        const CFeatListItem * f = &(*ft_it);
        int subtype = f->GetSubtype();
        if (subtype == CSeqFeatData::eSubtype_any && f->GetType() == 0) {
            all = f;
        } else if (subtype != CSeqFeatData::eSubtype_bad
            && subtype != CSeqFeatData::eSubtype_any
            && subtype != CSeqFeatData::eSubtype_Imp_CDS
            && subtype != CSeqFeatData::eSubtype_source
            && subtype != CSeqFeatData::eSubtype_org) {
            string desc = f->GetDescription();
            if (existing.find(desc) == existing.end())
            {
                existing.insert(desc);

                if (!present[subtype]) {
                    unused.push_back(f);
                } else {
                    switch (subtype) {
                        case CSeqFeatData::eSubtype_misc_RNA:
                        case CSeqFeatData::eSubtype_precursor_RNA:
                        case CSeqFeatData::eSubtype_mat_peptide:
                        case CSeqFeatData::eSubtype_sig_peptide:
                        case CSeqFeatData::eSubtype_transit_peptide:
                            import_feats.push_back(f);
                            break;
                        case CSeqFeatData::eSubtype_otherRNA:
                            popular.push_back(f);
                            break;
                        case CSeqFeatData::eSubtype_cdregion:
                        case CSeqFeatData::eSubtype_exon:
                        case CSeqFeatData::eSubtype_gene:
                        case CSeqFeatData::eSubtype_intron:
                        case CSeqFeatData::eSubtype_mRNA:
                        case CSeqFeatData::eSubtype_rRNA:
                        case CSeqFeatData::eSubtype_prot:
                            popular.push_back(f);
                            break;
                        case CSeqFeatData::eSubtype_txinit:
                            least_liked.push_back(f);
                            break;
                        default:
                            used.push_back(f);
                            break;
                    }
                }
            }
        }
    }

    sort(popular.begin(), popular.end(),s_CompareDescriptions);
    sort(used.begin(), used.end(), s_CompareDescriptions);
    sort(import_feats.begin(), import_feats.end(), s_CompareDescriptions);
    sort(unused.begin(), unused.end(), s_CompareDescriptions);

    r_list.insert(r_list.begin(), popular.begin(), popular.end());
    r_list.insert(r_list.end(), used.begin(), used.end());
    r_list.insert(r_list.end(), least_liked.begin(), least_liked.end());
    r_list.insert(r_list.end(), import_feats.begin(), import_feats.end());
    r_list.insert(r_list.end(), unused.begin(), unused.end());

    if (all) {
        r_list.insert(r_list.begin(), all);
    }

    return r_list;
}

void CSeqUtils::ParseRanges(const string& r_str, CSeqUtils::TRanges& ranges)
{
    vector<string> range_pairs;

    NStr::Split(r_str, ",", range_pairs);
    ITERATE (vector<string>, iter, range_pairs) {
        vector<string> pos;
        NStr::Split(*iter, "-", pos);
        if (pos.size() != 2) continue;
        string f_str = NStr::TruncateSpaces(pos[0]);
        string t_str = NStr::TruncateSpaces(pos[1]);
        if ( !f_str.empty()  &&  !t_str.empty() ) {
            try {
                TSeqPos from = NStr::StringToUInt(f_str);
                TSeqPos to = NStr::StringToUInt(t_str);

                //!! verify end of range inclusion!
                ranges.push_back(TSeqRange(from, to));
            } catch (CException&) {
                LOG_POST(Error << "Invalid range: " << *iter);
            }
        }
    }
}


/// --------------------------------
/// CGencollIdMapperAdapter 
/// --------------------------------

CGencollIdMapperAdapter::CGencollIdMapperAdapter(CRef<CGencollIdMapper> mapper, const CGencollIdMapper::SIdSpec& spec)
    : m_Mapper(mapper)
    , m_Spec(spec) {
    if (!m_Mapper)
        NCBI_THROW(CException, eInvalid, "IIdMapperAdapter: Invalid initialization");
}

objects::CSeq_id_Handle CGencollIdMapperAdapter::Map(const objects::CSeq_id_Handle& idh) {
    auto id = Ref(new CSeq_id);
    id->Assign(*idh.GetSeqId());

    auto loc = Ref(new CSeq_loc);
    loc->SetWhole(*id);
    auto mapped_loc = Map(*loc);

    CSeq_id_Handle out_sih;
    if (mapped_loc && !mapped_loc->IsNull() && !mapped_loc->IsEmpty() && mapped_loc->GetId())
        out_sih = CSeq_id_Handle::GetHandle(*mapped_loc->GetId());
    return out_sih;
}

CRef<objects::CSeq_loc> CGencollIdMapperAdapter::Map(const objects::CSeq_loc& loc) {
    return m_Mapper->Map(loc, m_Spec);
}

void CGencollIdMapperAdapter::MapObject(CSerialObject& obj) {
    set< CRef<CSeq_id> > ids;
    CTypeIterator<CSeq_id> idit(obj);
    for (; idit; ++idit) {
        CSeq_id& id = *idit;
        if (ids.emplace(&id).second == false)
            continue;
        auto loc = Ref(new CSeq_loc);
        loc->SetWhole(id);
        auto mapped_loc = Map(*loc);
        if (!mapped_loc || !mapped_loc->GetId())
            continue;
        id.Assign(*mapped_loc->GetId());
    }
}

IIdMapper* CGencollIdMapperAdapter::GetIdMapper(CRef<CGC_Assembly> assm) 
{
    CGencollIdMapper::SIdSpec MapSpec;
    MapSpec.TypedChoice = assm->IsRefSeq() ?
        CGC_TypedSeqId::e_Refseq : CGC_TypedSeqId::e_Genbank;
    MapSpec.Alias = CGC_SeqIdAlias::e_Public;
    MapSpec.Role = eGC_SequenceRole_top_level;
    CRef<CGencollIdMapper> mapper(new CGencollIdMapper(assm));
    return new CGencollIdMapperAdapter(mapper, MapSpec);
}



END_NCBI_SCOPE
