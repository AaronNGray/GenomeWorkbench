/*  $Id: basic_converters.cpp 38509 2017-05-18 19:30:17Z katargir $
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

#include <ncbi_pch.hpp>
#include <corelib/ncbiobj.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/objutils/obj_convert.hpp>

#include <objects/seq/Bioseq.hpp>
#include <objects/seq/Seq_annot.hpp>
#include <objects/seq/seq_id_handle.hpp>
#include <objects/seq/Seq_descr.hpp>

#include <objects/seqalign/Seq_align.hpp>
#include <objects/seqalign/Dense_seg.hpp>
#include <objects/seqalign/Seq_align_set.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqset/Seq_entry.hpp>
#include <objects/seqset/Bioseq_set.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/Gene_ref.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/seqfeat/BioSource.hpp>

#include <objects/submit/Seq_submit.hpp>

#include <objects/entrezgene/Entrezgene.hpp>
#include <objects/entrezgene/Entrezgene_Set.hpp>
#include <objects/entrezgene/Gene_commentary.hpp>

#include <objects/genomecoll/GC_Replicon.hpp>
#include <objects/genomecoll/GC_AssemblyDesc.hpp>

#include <objects/biotree/BioTreeContainer.hpp>
#include <objects/biotree/FeatureDictSet.hpp>
#include <objects/biotree/FeatureDescr.hpp>
#include <objects/biotree/NodeSet.hpp>
#include <objects/biotree/Node.hpp>
#include <objects/biotree/NodeFeatureSet.hpp>
#include <objects/biotree/NodeFeature.hpp>

#include <objmgr/annot_selector.hpp>
#include <objmgr/annot_ci.hpp>
#include <serial/iterator.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/align_ci.hpp>
#include <objmgr/graph_ci.hpp>
#include <objmgr/util/sequence.hpp>
#include <objmgr/util/feature.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


#define CHECK_CANCELLED { if (cancel && cancel->IsCanceled()) return; }

/////////////////////////////////////////////////////////////////////////////
///
/// Comment formatters
///


string s_GetDescription(CSeq_id_Handle idh, CScope& scope)
{
    if ( !idh ) {
        return string();
    }
    idh = sequence::GetId(idh, scope, sequence::eGetId_Best);
    if ( !idh ) {
        return string();
    }

    string label;
    idh.GetSeqId()->GetLabel(&label, CSeq_id::eContent);
    NStr::ToUpper(label);

    switch (label[0]) {
    case 'N':
        switch (label[1]) {
        case 'C':
            return string("RefSeq chromosome");
        case 'T':
            return string("RefSeq genomic contig");
        case 'M':
            return string("RefSeq mRNA");
        case 'G':
            return string("RefSeq curated genomic region");
        case 'R':
            return string("RefSeq non-coding RNA");
        case 'P':
            return string("RefSeq protein");
        case 'W':
            return string("RefSeq WGS contig");
        default:
            return string("RefSeq sequence");
        }
        break;

    case 'X':
        switch (label[1]) {
        case 'M':
            return string("RefSeq mRNA (predicted)");
        case 'P':
            return string("RefSeq protein (predicted)");
        default:
            return string("RefSeq sequence (predicted)");
        }
        break;

    case 'A':
    case 'C':
        return string("RefSeq chromosome (alternate assembly)");

    default:
        break;
    }

    CSeq_id::EAccessionInfo info = idh.GetSeqId()->IdentifyAccession();
    label.erase();
    switch (idh.GetSeqId()->Which()) {
    case CSeq_id::e_Genbank:
        label = "GenBank";
        break;
    case CSeq_id::e_Embl:
        label = "EMBL";
        break;
    case CSeq_id::e_Ddbj:
        label = "DDBJ";
        break;
    case CSeq_id::e_Other:
        label = "RefSeq";
        break;
    case CSeq_id::e_Gpipe:
        label = "NCBI Genome Pipeline";
        break;

    default:
        break;
    }

    switch (info & CSeq_id::eAcc_division_mask) {
    case CSeq_id::eAcc_est:
        if ( !label.empty() ) {
            label += " ";
        }
        label += "EST";
        break;
    case CSeq_id::eAcc_htgs:
        if ( !label.empty() ) {
            label += " ";
        }
        label += "HTGS";
        break;
    case CSeq_id::eAcc_wgs:
        if ( !label.empty() ) {
            label += " ";
        }
        label += "WGS";
        break;

    default:
        break;
    }

    return label;
}


string s_GetDescription(const CSeq_loc& loc, CScope& scope)
{
    CSeq_id_Handle idh = sequence::GetIdHandle(loc, &scope);
    return s_GetDescription(idh, scope);
}

/////////////////////////////////////////////////////////////////////////////


/// Bioseq --> Seq-entry
static void Convert_Bioseq_Seq_entry(
                                     CScope& scope,
                                     const CObject& obj,
                                     CBasicRelation::TObjects& objs,
                                     CBasicRelation::TFlags /*flags*/,
                                     ICanceled* cancel)
{
    const CBioseq* bs = dynamic_cast<const CBioseq*> (&obj);
    if (bs) {
        CConstRef<CSeq_entry> e(bs->GetParentEntry());
        while (e.GetPointer()  &&  e->GetParentEntry()) {
            e = e->GetParentEntry();
        }

        CHECK_CANCELLED

        if ( !e ) {
            CRef<CSeq_entry> se(new CSeq_entry());
            se->SetSeq(const_cast<CBioseq&>(*bs));
            e.Reset(se.GetPointer());
        }
        objs.emplace_back(*e);
    }
}

/// Bioseq --> Seq-id
static void Convert_Bioseq_Seq_id(
                                  CScope& scope,
                                  const CObject& obj,
                                  CBasicRelation::TObjects& objs,
                                  CBasicRelation::TFlags /*flags*/,
                                  ICanceled*)
{
    const CBioseq* bs = dynamic_cast<const CBioseq*> (&obj);
    if (bs) {
        CConstRef<CSeq_id> id = FindBestChoice(bs->GetId(), CSeq_id::BestRank);
        if (id) {
            objs.emplace_back(*id);
        }
    }
}

/// Bioseq-set --> Bioseq
static void Convert_Bioseq_set_Bioseq(
                                      CScope& scope,
                                      const CObject& obj,
                                      CBasicRelation::TObjects& objs,
                                      CBasicRelation::TFlags /*flags*/,
                                      ICanceled* cancel)
{
    const CBioseq_set* bs = dynamic_cast<const CBioseq_set*> (&obj);
    if (bs) {
        CTypeConstIterator<CBioseq> iter(*bs);
        for ( ;  iter;  ++iter) {
            CHECK_CANCELLED
            const CBioseq& bioseq = *iter;
            objs.emplace_back(bioseq);
        }
    }
}

/// Bioseq-set --> Seq-entry
static void Convert_Bioseq_set_Seq_entry(
                                         CScope& scope,
                                         const CObject& obj,
                                         CBasicRelation::TObjects& objs,
                                         CBasicRelation::TFlags /*flags*/,
                                         ICanceled*)
{
    const CBioseq_set* bs = dynamic_cast<const CBioseq_set*> (&obj);
    if (bs) {
        CRef<CSeq_entry> entry(new CSeq_entry());
        entry->SetSet(const_cast<CBioseq_set&>(*bs));
        objs.emplace_back(*entry);
    }
}

/// Dense-seg --> Seq-align
static void Convert_Dense_seg_Seq_align(
                                        CScope& scope,
                                        const CObject& obj,
                                        CBasicRelation::TObjects& objs,
                                        CBasicRelation::TFlags /*flags*/,
                                        ICanceled*)
{
    const CDense_seg* ds = dynamic_cast<const CDense_seg*>(&obj);
    if (ds) {
        CRef<CSeq_align> aln(new CSeq_align);
        aln->SetSegs().SetDenseg(const_cast<CDense_seg&>(*ds));
        aln->SetType(CSeq_align::eType_not_set);
        objs.emplace_back(*aln);
    }
}

static CRef<CSeq_annot> s_Align2Annot(const CSeq_align& align)
{
    CRef<CSeq_annot> annot(new CSeq_annot());
    annot->SetData().SetAlign()
        .push_back(CRef<CSeq_align>(const_cast<CSeq_align*>(&align)));
    return annot;
}

/// Seq-align --> Seq-annot
static void Convert_Seq_align_Seq_annot(
                                        CScope& scope,
                                        const CObject& obj,
                                        CBasicRelation::TObjects& objs,
                                        CBasicRelation::TFlags /*flags*/,
                                        ICanceled*)
{
    const CSeq_align* align = dynamic_cast<const CSeq_align*>(&obj);
    if (align) {
        CRef<CSeq_annot> annot(s_Align2Annot(*align));
        objs.emplace_back(*annot);
    }
}

/// Seq-align -> Seq-id
static void Convert_Seq_align_Seq_id(
                                     CScope& scope,
                                     const CObject& obj,
                                     CBasicRelation::TObjects& objs,
                                     CBasicRelation::TFlags /*flags*/,
                                     ICanceled* cancel)
{
    const CSeq_align* align = dynamic_cast<const CSeq_align*> (&obj);
    if (align) {
        set<CSeq_id_Handle> id_set;
        CTypeConstIterator<CSeq_id> id_iter(*align);
        for ( ;  id_iter;  ++id_iter) {
            CHECK_CANCELLED
            id_set.insert(CSeq_id_Handle::GetHandle(*id_iter));
        }

        ITERATE (set<CSeq_id_Handle>, iter, id_set) {
            CHECK_CANCELLED
            objs.emplace_back(*iter->GetSeqId());
        }
    }
}

static void s_Seq_align_Seq_loc(const CSeq_align& align,
                                list< CConstRef<CSeq_loc> >& locs)
{
    switch (align.GetSegs().Which()) {
    case CSeq_align::TSegs::e_Denseg:
        {{
             const CDense_seg& ds = align.GetSegs().GetDenseg();
             for (int i = 0;  i < ds.GetDim();  ++i) {
                 CConstRef<CSeq_id> id = ds.GetIds()[i];

                 CRef<CSeq_loc> loc(new CSeq_loc());
                 loc->SetInt().SetFrom  (ds.GetSeqStart(i));
                 loc->SetInt().SetTo    (ds.GetSeqStop(i));
                 if (ds.IsSetStrands()) {
                     loc->SetInt().SetStrand(ds.GetSeqStrand(i));
                 }
                 loc->SetId(*id);
                 locs.push_back(loc);
             }
         }}
        break;

    case CSeq_align::TSegs::e_Disc:
        ITERATE (CSeq_align::TSegs::TDisc::Tdata, iter, align.GetSegs().GetDisc().Get()) {
            s_Seq_align_Seq_loc(**iter, locs);
        }
        break;

    default:
        {{
            int rows = 0;
            try {
                rows = align.CheckNumRows();
                for (CSeq_align::TDim row = 0; row < rows; ++row) {
                    // range specific to the alignment 
                    CRef<CSeq_loc> loc(new CSeq_loc());
                    CSeq_interval& ival = loc->SetInt();
                    ival.SetId().Assign(align.GetSeq_id(row));
                    ival.SetFrom(align.GetSeqRange(row).GetFrom());
                    ival.SetTo(align.GetSeqRange(row).GetTo());
                    locs.push_back(loc);

                    // range to the whole sequence 
                    loc.Reset(new CSeq_loc());
                    loc->SetWhole().Assign(align.GetSeq_id(row));
                    locs.push_back(loc);
                }
            } catch (std::exception&) {
                // check row is not supported, we need to do
                // something differently
            }
            if (rows == 0) {
                set<CSeq_id_Handle> id_set;
                CTypeConstIterator<CSeq_id> iter(align);
                for ( ;  iter;  ++iter) {
                    id_set.insert(CSeq_id_Handle::GetHandle(*iter));
                }
                ITERATE (set<CSeq_id_Handle>, id_iter, id_set) {
                    CRef<CSeq_loc> loc(new CSeq_loc());
                    loc->SetWhole().Assign(*id_iter->GetSeqId());
                    locs.push_back(loc);
                }
            }
        }}
        break;
    }
}

static void s_AddCoalescedLocations(const list< CConstRef<CSeq_loc> >& locs,
                                    CScope& scope,
                                    CBasicRelation::TObjects& objs,
                                    ICanceled* cancel)
{
    CScope::TSeq_id_Handles whole_ids;
    /// step 2: coalesce these where possible
    typedef map<CSeq_id_Handle, list<TSeqRange> > TIdRanges;
    TIdRanges id_ranges;
    ITERATE (list< CConstRef<CSeq_loc> >, iter, locs) {
        CHECK_CANCELLED
        try {
            const CSeq_loc& loc = **iter;
            CSeq_id_Handle idh = sequence::GetIdHandle(loc, &scope);

            TSeqRange range;
            switch (loc.Which()) {
            case CSeq_loc::e_Whole:
                whole_ids.push_back(idh);
                continue;
            default:
                range.SetFrom(sequence::GetStart(loc, &scope));
                range.SetTo(sequence::GetStop(loc, &scope));
                break;
            }
            id_ranges[idh].push_back(range);
        }
        catch (CException&) {
        }
    }
    auto&& id_lens = scope.GetSequenceLengths(whole_ids);
    _ASSERT(id_lens.size() == whole_ids.size());
    if (id_lens.size() == whole_ids.size()) {
        for (auto i = 0; i < whole_ids.size(); ++i) {
            id_ranges[whole_ids[i]].push_back(TSeqRange(0, id_lens[i] - 1));
        }
    }

    list< CConstRef<CSeq_loc> > overlocs;
    ITERATE (TIdRanges, iter, id_ranges) {
        CHECK_CANCELLED
        if (iter->second.size() > 1) {
            list<TSeqRange>::const_iterator it     = iter->second.begin();
            list<TSeqRange>::const_iterator it_end = iter->second.end();
            TSeqRange r = *it++;
            for ( ;  it != it_end;  ++it) {
                r += *it;
            }
            CRef<CSeq_loc> loc(new CSeq_loc());
            loc->SetInt().SetFrom(r.GetFrom());
            loc->SetInt().SetTo  (r.GetTo());
            loc->SetId(*iter->first.GetSeqId());
            overlocs.push_back(loc);
        }
    }

    ITERATE (list< CConstRef<CSeq_loc> >, iter, overlocs) {
        CHECK_CANCELLED
        objs.push_back(CRelation::SObject(**iter));
    }
}

/// Seq-align -> Seq-loc : Basic
static void Convert_Seq_align_Seq_loc_Basic(
                                            CScope& scope,
                                            const CObject& obj,
                                            CBasicRelation::TObjects& objs,
                                            CBasicRelation::TFlags /*flags*/,
                                            ICanceled* cancel)
{
    const CSeq_align* align = dynamic_cast<const CSeq_align*> (&obj);
    if (align) {
        list< CConstRef<CSeq_loc> > locs;
        s_Seq_align_Seq_loc(*align, locs);
        ITERATE (list< CConstRef<CSeq_loc> >, loc_iter, locs) {
            CHECK_CANCELLED
            objs.emplace_back(**loc_iter);
        }
    }
}

/// Seq-align -> Seq-loc : Coalesced
static void Convert_Seq_align_Seq_loc_Coalesced(
                                                CScope& scope,
                                                const CObject& obj,
                                                CBasicRelation::TObjects& objs,
                                                CBasicRelation::TFlags /*flags*/,
                                                ICanceled* cancel)
{
    const CSeq_align* align = dynamic_cast<const CSeq_align*> (&obj);
    if (align) {
        list< CConstRef<CSeq_loc> > locs;
        s_Seq_align_Seq_loc(*align, locs);
        s_AddCoalescedLocations(locs, scope, objs, cancel);
    }
}

/*
/// Seq-align -> Seq-loc
static void Convert_Seq_align_Seq_loc(
CScope& scope,
const CObject& obj,
CBasicRelation::TObjects& objs,
CBasicRelation::TFlags flags)
{
const CSeq_align* align = dynamic_cast<const CSeq_align*> (&obj);
if (align) {
list< CConstRef<CSeq_loc> > locs;
s_Seq_align_Seq_loc(*align, locs);
s_AddCoalescedLocations(locs, scope, objs, "total aligned range");

ITERATE (list< CConstRef<CSeq_loc> >, loc_iter, locs) {
objs.push_back(CRelation::SObject(**loc_iter));
}
}
}
 */

static CRef<CSeq_annot> s_AlignSet2Annot(const CSeq_align_set& align_set)
{
    CRef<CSeq_annot> annot(new CSeq_annot());
    ITERATE (CSeq_align_set::Tdata, iter, align_set.Get()) {
        CRef<CSeq_align> ref
            (const_cast<CSeq_align*>(iter->GetPointer()));
        annot->SetData().SetAlign().push_back(ref);
    }
    return annot;
}

/// Seq-align-set --> Seq-align
static void Convert_Seq_align_set_Seq_align(
                                            CScope& scope,
                                            const CObject& obj,
                                            CBasicRelation::TObjects& objs,
                                            CBasicRelation::TFlags /*flags*/,
                                            ICanceled* cancel)
{
    const CSeq_align_set* align =
        dynamic_cast<const CSeq_align_set*>(&obj);
    if (align) {
        ITERATE (CSeq_align_set::Tdata, iter, align->Get()) {
            CHECK_CANCELLED
            objs.emplace_back(**iter);
        }
    }
}

/// Seq-align-set -> Seq-loc
static void Convert_Seq_align_set_Seq_loc(
                                          CScope& scope,
                                          const CObject& obj,
                                          CBasicRelation::TObjects& objs,
                                          CBasicRelation::TFlags /*flags*/,
                                          ICanceled* cancel)
{
    const CSeq_align_set* align = dynamic_cast<const CSeq_align_set*> (&obj);
    if (align) {
        list< CConstRef<CSeq_loc> > locs;
        ITERATE (CSeq_align_set::Tdata, iter, align->Get()) {
            CHECK_CANCELLED
            s_Seq_align_Seq_loc(**iter, locs);
        }

        s_AddCoalescedLocations(locs, scope, objs, cancel);
        ITERATE (list< CConstRef<CSeq_loc> >, loc_iter, locs) {
            CHECK_CANCELLED
            objs.emplace_back(**loc_iter);
        }
    }
}

/// Seq-align-set --> Seq-annot
static void Convert_Seq_align_set_Seq_annot(
                                            CScope& scope,
                                            const CObject& obj,
                                            CBasicRelation::TObjects& objs,
                                            CBasicRelation::TFlags /*flags*/,
                                            ICanceled*)
{
    const CSeq_align_set* align =
        dynamic_cast<const CSeq_align_set*>(&obj);
    if (align) {
        CRef<CSeq_annot> annot(s_AlignSet2Annot(*align));
        objs.emplace_back(*annot);
    }
}

/// Seq-annot --> Seq-align
static void Convert_Seq_annot_Seq_align(
                                        CScope& scope,
                                        const CObject& obj,
                                        CBasicRelation::TObjects& objs,
                                        CBasicRelation::TFlags /*flags*/,
                                        ICanceled* cancel)
{
    const CSeq_annot* annot = dynamic_cast<const CSeq_annot*>(&obj);
    if (annot  &&  annot->IsSetData()  &&  annot->GetData().IsAlign()) {
        ITERATE (CSeq_annot::TData::TAlign, iter, annot->GetData().GetAlign()) {
            CHECK_CANCELLED
            objs.emplace_back(**iter);
        }
    }
}

/// Seq-annot --> Seq-entry
static void Convert_Seq_annot_Seq_entry(
                                        CScope& scope,
                                        const CObject& obj,
                                        CBasicRelation::TObjects& objs,
                                        CBasicRelation::TFlags /*flags*/,
                                        ICanceled*)
{
    const CSeq_annot* annot = dynamic_cast<const CSeq_annot*> (&obj);
    if (annot) {
        CRef<CSeq_entry> entry(new CSeq_entry());
        entry->SetSet().SetSeq_set();
        entry->SetSet().SetAnnot()
            .push_back(CRef<CSeq_annot>(const_cast<CSeq_annot*>(annot)));
        objs.emplace_back(*entry);
    }
}

/// Seq-annot --> Seq-feat
static void Convert_Seq_annot_Seq_feat(
                                       CScope& scope,
                                       const CObject& obj,
                                       CBasicRelation::TObjects& objs,
                                       CBasicRelation::TFlags /*flags*/,
                                       ICanceled* cancel)
{
    const CSeq_annot* annot = dynamic_cast<const CSeq_annot*>(&obj);
    if (annot  &&  annot->IsSetData()  &&  annot->GetData().IsFtable()) {
        ITERATE (CSeq_annot::TData::TFtable, iter, annot->GetData().GetFtable()) {
            CHECK_CANCELLED
            objs.emplace_back(**iter);
        }
    }
}

/// Seq-annot --> Seq-graph
static void Convert_Seq_annot_Seq_graph(
                                        CScope& scope,
                                        const CObject& obj,
                                        CBasicRelation::TObjects& objs,
                                        CBasicRelation::TFlags /*flags*/,
                                        ICanceled* cancel)
{
    const CSeq_annot* annot = dynamic_cast<const CSeq_annot*>(&obj);
    if (annot  &&  annot->IsSetData()  &&  annot->GetData().IsGraph()) {
        ITERATE (CSeq_annot::TData::TGraph, iter, annot->GetData().GetGraph()) {
            CHECK_CANCELLED
            objs.emplace_back(**iter);
        }
    }
}

/// Seq-annot --> Seq-id
static void Convert_Seq_annot_Seq_id(
                                     CScope& scope,
                                     const CObject& obj,
                                     CBasicRelation::TObjects& objs,
                                     CBasicRelation::TFlags /*flags*/,
                                     ICanceled* cancel)
{
    const CSeq_annot* annot = dynamic_cast<const CSeq_annot*> (&obj);
    if (annot) {
        set<CSeq_id_Handle> id_set;
        switch (annot->GetData().Which()) {
        case CSeq_annot::TData::e_Ids:
            ITERATE (CSeq_annot::TData::TIds, iter, annot->GetData().GetIds()) {
                CHECK_CANCELLED
                const CSeq_id& id = **iter;
                CSeq_id_Handle idh = CSeq_id_Handle::GetHandle(id);
                if ( !id_set.insert(idh).second ) {
                    /// already in our set
                    continue;
                }
                objs.push_back(CRelation::SObject(id));
            }
            break;

        case CSeq_annot::TData::e_Locs:
            ITERATE (CSeq_annot::TData::TLocs, iter, annot->GetData().GetLocs()) {
                CHECK_CANCELLED
                const CSeq_loc& loc = **iter;
                CSeq_id_Handle idh = sequence::GetIdHandle(loc, &scope);
                if ( !id_set.insert(idh).second ) {
                    /// already in our set
                    continue;
                }
                objs.push_back(CRelation::SObject(*idh.GetSeqId()));
            }
            break;

        default:
            {{
                 CTypeConstIterator<CSeq_id> id_iter(*annot);
                 for ( ;  id_iter;  ++id_iter) {
                     CHECK_CANCELLED
                     CSeq_id_Handle idh = CSeq_id_Handle::GetHandle(*id_iter);
                     if ( !id_set.insert(idh).second ) {
                         /// already in our list
                         continue;
                     }
                     objs.push_back(CRelation::SObject(*id_iter));
                 }
             }}
            break;
        }
    }
}

/// Seq-annot --> Seq-loc (total range)
static void Convert_Seq_annot_Seq_loc_TotalRange(
                                                 CScope& scope,
                                                 const CObject& obj,
                                                 CBasicRelation::TObjects& objs,
                                                 CBasicRelation::TFlags /*flags*/,
                                                 ICanceled* cancel)
{
    const CSeq_annot* annot = dynamic_cast<const CSeq_annot*> (&obj);
    if (annot) {
        ///
        /// step 1: collect all locations
        ///
        typedef map<CSeq_id_Handle, TSeqRange> TRanges;
        TRanges ranges;
        switch (annot->GetData().Which()) {
        case CSeq_annot::TData::e_Ftable:
            ITERATE (CSeq_annot::TData::TFtable, iter, annot->GetData().GetFtable()) {
                CHECK_CANCELLED

                const CSeq_loc& loc = (*iter)->GetLocation();
                CSeq_id_Handle idh = sequence::GetIdHandle(loc, &scope);
                if (idh) {
                    TSeqRange range = loc.GetTotalRange();
                    ranges[idh] += range;
                }
            }
            break;

        case CSeq_annot::TData::e_Locs:
            ITERATE (CSeq_annot::TData::TLocs, iter, annot->GetData().GetLocs()) {
                CHECK_CANCELLED
                const CSeq_loc& loc = **iter;
                CSeq_id_Handle idh = sequence::GetIdHandle(loc, &scope);
                if ( !idh ) {
                    continue;
                }

                TSeqRange range;
                switch (loc.Which()) {
                case CSeq_loc::e_Whole:
                    ///
                    /// special case: skip these
                    /// total range doesn't make sense here
                    ///
                    continue;
                    /**
                      {{
                      CBioseq_Handle bsh = scope.GetBioseqHandle(idh);
                      range.SetTo(bsh.GetBioseqLength());
                      }}
                      break;
                     **/

                default:
                    range = loc.GetTotalRange();
                    break;
                }
                ranges[idh] += range;
            }
            break;

        case CSeq_annot::TData::e_Ids:
            ///
            /// special case:
            /// don't return these; total range doesn't make sense here
            ///
            /**
              {{
              set<CSeq_id_Handle> ids;
              ITERATE (CSeq_annot::TData::TIds, iter, annot->GetData().GetIds()) {
              const CSeq_id& id = **iter;
              CSeq_id_Handle idh = CSeq_id_Handle::GetHandle(id);
              if (ids.insert(idh).second == false) {
            /// already in our set
            continue;
            }

            CRef<CSeq_loc> loc(new CSeq_loc);
            loc->SetWhole().Assign(id);

            string s = s_GetDescription(*loc, scope);
            objs.push_back(CRelation::SObject(*loc, s, "total range"));
            }

            }}
            /// return, not break...
             **/
            return;

        case CSeq_annot::TData::e_Align:
            {{
                 ITERATE (CSeq_annot::TData::TAlign, align_iter, annot->GetData().GetAlign()) {
                     CHECK_CANCELLED
                     list< CConstRef<CSeq_loc> > locs;
                     s_Seq_align_Seq_loc(**align_iter, locs);
                     ITERATE (list< CConstRef<CSeq_loc> >, iter, locs) {
                         const CSeq_loc& loc = **iter;
                         CSeq_id_Handle idh = sequence::GetIdHandle(loc, &scope);
                         TSeqRange range = loc.GetTotalRange();
                         ranges[idh] += range;
                     }
                 }
             }}
            break;

        case CSeq_annot::TData::e_Graph:
            {{
                ITERATE (CSeq_annot::TData::TGraph, graph_iter, annot->GetData().GetGraph()) {
                    CHECK_CANCELLED
                    const CSeq_loc& loc = (*graph_iter)->GetLoc();
                    CSeq_id_Handle idh = sequence::GetIdHandle(loc, &scope);
                    TSeqRange range = loc.GetTotalRange();
                    ranges[idh] += range;
                 }
            }}
            break;
        default:
            {{
                CTypeConstIterator<CSeq_id> id_iter(*annot);
                for ( ;  id_iter;  ++id_iter) {
                    CHECK_CANCELLED
                    CSeq_id_Handle idh = CSeq_id_Handle::GetHandle(*id_iter);
                    if (idh) {
                        ranges[idh] = TSeqRange::GetWhole();
                    }
                }
            }}
            break;
        }

        // step 2: create new locations
        //
        string s;
        ITERATE (TRanges, iter, ranges) {
            CHECK_CANCELLED
            CRef<CSeq_loc> loc(new CSeq_loc);
            if (iter->second.IsWhole()) {
                loc->SetWhole().Assign(*iter->first.GetSeqId());
            } else {
                loc->SetInt().SetFrom(iter->second.GetFrom());
                loc->SetInt().SetTo  (iter->second.GetTo());
                loc->SetId(*iter->first.GetSeqId());
            }
            
            objs.emplace_back(*loc, "total range");
        }
    }
}

/// Seq-annot --> Seq-loc (all locs)
static void Convert_Seq_annot_Seq_loc_All(
                                          CScope& scope,
                                          const CObject& obj,
                                          CBasicRelation::TObjects& objs,
                                          CBasicRelation::TFlags /*flags*/,
                                          ICanceled* cancel)
{
    const CSeq_annot* annot = dynamic_cast<const CSeq_annot*> (&obj);
    if (annot) {
        ///
        /// step 1: collect all locations
        ///
        switch (annot->GetData().Which()) {
        case CSeq_annot::TData::e_Ftable:
            ITERATE (CSeq_annot::TData::TFtable, iter, annot->GetData().GetFtable()) {
                CHECK_CANCELLED
                const CSeq_loc& loc = (*iter)->GetLocation();

                string comm = s_GetDescription(loc, scope);
                objs.emplace_back(loc, comm);
            }
            break;

        case CSeq_annot::TData::e_Ids:
            ITERATE (CSeq_annot::TData::TIds, iter, annot->GetData().GetIds()) {
                CHECK_CANCELLED
                const CSeq_id& id = **iter;
                CSeq_id_Handle idh = CSeq_id_Handle::GetHandle(id);

                CRef<CSeq_loc> loc(new CSeq_loc);
                loc->SetWhole().Assign(id);

                objs.push_back(CRelation::SObject(*loc));
            }
            break;

        case CSeq_annot::TData::e_Locs:
            ITERATE (CSeq_annot::TData::TLocs, iter, annot->GetData().GetLocs()) {
                CHECK_CANCELLED
                const CSeq_loc& loc = **iter;
                objs.push_back(CRelation::SObject(loc));
            }
            break;

        case CSeq_annot::TData::e_Align:
            {{
                 ITERATE (CSeq_annot::TData::TAlign, align_iter, annot->GetData().GetAlign()) {
                     CHECK_CANCELLED
                     list< CConstRef<CSeq_loc> > locs;
                     s_Seq_align_Seq_loc(**align_iter, locs);
                     ITERATE (list< CConstRef<CSeq_loc> >, iter, locs) {
                         CHECK_CANCELLED
                         objs.push_back(CRelation::SObject(**iter));
                     }
                 }
             }}
            break;

        default:
            {{
                 set<CSeq_id_Handle> id_set;
                 CTypeConstIterator<CSeq_id> id_iter(*annot);
                 for ( ;  id_iter;  ++id_iter) {
                     CHECK_CANCELLED
                     CSeq_id_Handle idh = CSeq_id_Handle::GetHandle(*id_iter);
                     CConstRef<CSeq_id> seq_id = idh.GetSeqIdOrNull();
                     if (!seq_id.IsNull() && id_set.insert(idh).second) {
                         CRef<CSeq_loc> loc(new CSeq_loc());
                         loc->SetWhole().Assign(*idh.GetSeqId());
                         objs.push_back(CRelation::SObject(*loc));
                     }
                 }
             }}
            break;
        }
    }
}

/// Seq-entry --> Bioseq
static void Convert_Seq_entry_Bioseq(
                                     CScope& scope,
                                     const CObject& obj,
                                     CBasicRelation::TObjects& objs,
                                     CBasicRelation::TFlags /*flags*/,
                                     ICanceled* cancel)
{
    const CSeq_entry* entry = dynamic_cast<const CSeq_entry*> (&obj);
    if (entry) {
        CTypeConstIterator<CBioseq> iter(*entry);
        for ( ;  iter;  ++iter) {
            CHECK_CANCELLED
            objs.emplace_back(*iter);
        }
    }
}

/// Seq-entry --> Bioseq-set
static void Convert_Seq_entry_Bioseq_set(
                                         CScope& scope,
                                         const CObject& obj,
                                         CBasicRelation::TObjects& objs,
                                         CBasicRelation::TFlags /*flags*/,
                                         ICanceled*)
{
    const CSeq_entry* entry = dynamic_cast<const CSeq_entry*> (&obj);
    if (entry) {
        if (entry->IsSet()) {
            objs.emplace_back(entry->GetSet());
        }
        else {
            CRef<CBioseq_set> bs_set(new CBioseq_set());
            CRef<CSeq_entry> ref(const_cast<CSeq_entry*>(entry));
            bs_set->SetSeq_set().push_back(ref);
            objs.emplace_back(*bs_set);
        }
    }
}

/// Seq-entry --> Seq-loc
static void Convert_Seq_entry_Seq_loc(
                                      CScope& scope,
                                      const CObject& obj,
                                      CBasicRelation::TObjects& objs,
                                      CBasicRelation::TFlags /*flags*/,
                                      ICanceled* cancel)
{
    const CSeq_entry* entry = dynamic_cast<const CSeq_entry*> (&obj);
    if (entry) {
        CTypeConstIterator<CBioseq> iter(*entry);
        for ( ;  iter;  ++iter) {
            CHECK_CANCELLED;
            CConstRef<CSeq_id> best_id =
                FindBestChoice(iter->GetId(), CSeq_id::BestRank);
            if (best_id) {
                CRef<CSeq_loc> loc(new CSeq_loc());
                loc->SetWhole().Assign(*best_id);

                objs.emplace_back(*loc);
            }
        }
    }
}

/// Seq-entry --> Seq-id
static void Convert_Seq_entry_Seq_id(
                                     CScope& scope,
                                     const CObject& obj,
                                     CBasicRelation::TObjects& objs,
                                     CBasicRelation::TFlags /*flags*/,
                                     ICanceled* cancel)
{
    const CSeq_entry* entry = dynamic_cast<const CSeq_entry*> (&obj);
    if (entry) {
        CTypeConstIterator<CBioseq> iter(*entry);
        for ( ;  iter;  ++iter) {
            CHECK_CANCELLED;
            CConstRef<CSeq_id> best_id =
                FindBestChoice(iter->GetId(), CSeq_id::BestRank);
            if (best_id) {
                CSeq_id_Handle idh = CSeq_id_Handle::GetHandle(*best_id);
                objs.emplace_back(*best_id);
            }
        }
    }
}

/// Seq-entry --> Seq-submit
static void Convert_Seq_entry_SeqSubmit(
                                        CScope& scope,
                                        const CObject& obj,
                                        CBasicRelation::TObjects& objs,
                                        CBasicRelation::TFlags /*flags*/,
                                        ICanceled*)
{
    const CSeq_entry* entry = dynamic_cast<const CSeq_entry*> (&obj);
    if (entry) {
        CRef<CSeq_submit> sub(new CSeq_submit());
        sub->SetData().SetEntrys().push_back
            (CRef<CSeq_entry>(const_cast<CSeq_entry*>(entry)));
        objs.emplace_back(*sub);
    }
}

/// Seq-entry --> Seq-annot
static void Convert_Seq_entry_Seq_annot(
                                        CScope& scope,
                                        const CObject& obj,
                                        CBasicRelation::TObjects& objs,
                                        CBasicRelation::TFlags /*flags*/,
                                        ICanceled* cancel)
{
    const CSeq_entry* entry = dynamic_cast<const CSeq_entry*> (&obj);
    if (entry) {
        CTypeConstIterator<CBioseq> iter(*entry);
        for ( ;  iter;  ++iter) {
            CHECK_CANCELLED
            CBioseq_Handle handle = scope.GetBioseqHandle(*iter);
            if (handle) {
                CSeq_annot_CI annot_iter(handle);
                for ( ;  annot_iter;  ++annot_iter) {
                    CConstRef<CSeq_annot> annot = annot_iter->GetCompleteSeq_annot();
                    CHECK_CANCELLED
                    objs.emplace_back(*annot);
                }
            }
        }
    }
}

/// Seq-entry --> Seq-align
static void Convert_Seq_entry_Seq_align(
                                        CScope& scope,
                                        const CObject& obj,
                                        CBasicRelation::TObjects& objs,
                                        CBasicRelation::TFlags /*flags*/,
                                        ICanceled* cancel)
{
    const CSeq_entry* entry = dynamic_cast<const CSeq_entry*> (&obj);
    if (entry) {
        CTypeConstIterator<CBioseq> iter(*entry);
        for ( ;  iter;  ++iter) {
            CHECK_CANCELLED
            CBioseq_Handle handle = scope.GetBioseqHandle(*iter);
            if (handle) {
                CAlign_CI align_iter(handle);
                for ( ;  align_iter;  ++align_iter) {
                    CConstRef<CSeq_align> align(&*align_iter);
                   CHECK_CANCELLED
                   objs.emplace_back(*align);
                }
            }
        }
    }
}

/**
/// Seq-feat --> Seq-id
static void Convert_Seq_feat_Seq_id(
CScope& scope,
const CObject& obj,
CBasicRelation::TObjects& objs,
CBasicRelation::TFlags flags)
{
const CSeq_feat* feat = dynamic_cast<const CSeq_feat*> (&obj);
if (feat) {
try {
const CSeq_id& id = sequence::GetId(feat->GetLocation(),
&scope);
objs.push_back(CRelation::SObject(id));
}
catch (...) {
}

if (feat->IsSetProduct()) {
try {
const CSeq_id& id = sequence::GetId(feat->GetProduct(),
&scope);
objs.push_back(CRelation::SObject(id));
}
catch (...) {
}
}

// deal with dbxrefs
if (feat->IsSetDbxref()) {
ITERATE (CSeq_feat::TDbxref, iter, feat->GetDbxref()) {
const CDbtag& dbtag = **iter;

CRef<CSeq_id> id(new CSeq_id);
try {
id->Set(dbtag, false);
objs.push_back(CRelation::SObject(*id));
}
catch (CSeqIdException&) {
}
}
}
}
}
 **/

/// Seq-feat --> Seq-loc : Genomic Location
static void Convert_Seq_feat_Seq_loc_Gen_Loc(
                                             CScope& scope,
                                             const CObject& obj,
                                             CBasicRelation::TObjects& objs,
                                             CBasicRelation::TFlags /*flags*/,
                                             ICanceled*)
{
    const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(&obj);
    if ( !feat ) {
        return;
    }

    string comm = s_GetDescription(feat->GetLocation(), scope);
    objs.emplace_back(feat->GetLocation(), comm);
}

/// Seq-feat --> Seq-loc : Genomic Location (total range)
static void Convert_Seq_feat_Seq_loc_Gen_Loc_TotalRange(
                                                        CScope& scope,
                                                        const CObject& obj,
                                                        CBasicRelation::TObjects& objs,
                                                        CBasicRelation::TFlags /*flags*/,
                                                        ICanceled*)
{
    const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(&obj);
    if ( !feat ) {
        return;
    }

    const CSeq_loc& orig_loc = feat->GetLocation();
    if ( !orig_loc.IsInt()  &&  !orig_loc.IsWhole()  &&
         (!orig_loc.IsPacked_int()  ||  orig_loc.GetPacked_int().Get().size() > 1)) {
        /// compose a location for the total range
        /// do this only for locations that are not already just a single interval
        try {
            CRef<CSeq_loc> loc(new CSeq_loc);
            loc->SetInt().SetFrom(feat->GetLocation().GetTotalRange().GetFrom());
            loc->SetInt().SetTo  (feat->GetLocation().GetTotalRange().GetTo());
            ENa_strand strand = sequence::GetStrand(feat->GetLocation(), &scope);
            if (strand == eNa_strand_plus  ||  strand == eNa_strand_minus) {
                loc->SetStrand(strand);
            }
            CSeq_id_Handle idh = sequence::GetIdHandle(feat->GetLocation(), &scope);
            loc->SetId(*idh.GetSeqId());

            string comm = s_GetDescription(*loc, scope);
            comm += " (total range)";
            objs.emplace_back(*loc, comm);
        } catch (std::exception&) {
            // it is very likely we got multiple seq-ids loc, discard this one.
        }
    }
}

/// Seq-feat --> Seq-loc : Product Location
static void Convert_Seq_feat_Seq_loc_Prod_Loc(
                                              CScope& scope,
                                              const CObject& obj,
                                              CBasicRelation::TObjects& objs,
                                              CBasicRelation::TFlags /*flags*/,
                                              ICanceled*)
{
    const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(&obj);
    if ( !feat ) {
        return;
    }

    if (feat->IsSetProduct()) {
        string comm = s_GetDescription(feat->GetProduct(), scope);
        comm += " (product)";
        objs.emplace_back(feat->GetProduct(), comm);
    }
}

/// Seq-feat --> Seq-loc : dbxrefs Location
static void Convert_Seq_feat_Seq_loc_dbxref_Loc(
                                                CScope& scope,
                                                const CObject& obj,
                                                CBasicRelation::TObjects& objs,
                                                CBasicRelation::TFlags flags,
                                                ICanceled*)
{
    const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(&obj);
    if ( !feat ) {
        return;
    }

    if (flags & CBasicRelation::fConvert_All && feat->IsSetDbxref()) {
        ITERATE (CSeq_feat::TDbxref, iter, feat->GetDbxref()) {
            try {
                const CDbtag& dbtag = **iter;
                CRef<CSeq_id> id(new CSeq_id);
                id->Set(dbtag, false);
                CRef<CSeq_loc> loc(new CSeq_loc());
                loc->SetWhole(*id);

                objs.emplace_back(*loc);
            }
            catch (CSeqIdException&) {
            }
        }
    }
}


/// Seq-id --> Bioseq: use CBioseq_Handle
static void Convert_Seq_id_Bioseq(
                                  CScope& scope,
                                  const CObject& obj,
                                  CBasicRelation::TObjects& objs,
                                  CBasicRelation::TFlags flags,
                                  ICanceled*)
{
    const CSeq_id* id = dynamic_cast<const CSeq_id*>(&obj);
    if (id) {
        // retrieve our handle from the scope and proceed
        CBioseq_Handle handle = scope.GetBioseqHandle(*id);
        if (handle) {
            CConstRef<CBioseq> bioseq;
            if (flags & CBasicRelation::fConvert_NoExpensive) {
                bioseq = handle.GetBioseqCore();
            } else {
                bioseq = handle.GetCompleteBioseq();
            }
            objs.emplace_back(*bioseq);
        }
    }
}

/// Seq-id --> Bioseq-set: use CBioseq_Handle
static void Convert_Seq_id_Bioseq_set(
                                      CScope& scope,
                                      const CObject& obj,
                                      CBasicRelation::TObjects& objs,
                                      CBasicRelation::TFlags /*flags*/,
                                      ICanceled*)
{
    const CSeq_id* id = dynamic_cast<const CSeq_id*>(&obj);
    if (id) {
        // retrieve our handle from the scope and proceed
        CBioseq_Handle handle = scope.GetBioseqHandle(*id);
        if (handle) {
            CConstRef<CSeq_entry> entry =
                handle.GetTopLevelEntry().GetCompleteSeq_entry();
            if (entry.GetPointer()  &&  entry->IsSet()) {
                objs.push_back(CRelation::SObject(entry->GetSet()));
            }
        }
    }
}

/// Seq-id --> Seq-entry: use CBioseq_Handle
static void Convert_Seq_id_Seq_entry(
                                     CScope& scope,
                                     const CObject& obj,
                                     CBasicRelation::TObjects& objs,
                                     CBasicRelation::TFlags /*flags*/,
                                     ICanceled*)
{
    const CSeq_id* id = dynamic_cast<const CSeq_id*>(&obj);
    if (id) {
        // retrieve our handle from the scope and proceed
        CBioseq_Handle handle = scope.GetBioseqHandle(*id);
        if (handle) {
            CConstRef<CSeq_entry> entry =
                handle.GetTopLevelEntry().GetCompleteSeq_entry();
            objs.push_back(CRelation::SObject(*entry));
        }
    }
}

/// Seq-id --> Seq-loc: use Seq-loc.whole representation
static void Convert_Seq_id_Seq_loc(
                                   CScope& scope,
                                   const CObject& obj,
                                   CBasicRelation::TObjects& objs,
                                   CBasicRelation::TFlags /*flags*/,
                                   ICanceled*)
{
    const CSeq_id* id = dynamic_cast<const CSeq_id*>(&obj);
    if (id) {
        CRef<CSeq_loc> loc(new CSeq_loc());
        loc->SetWhole().Assign(*id);
        objs.push_back(CRelation::SObject(*loc, "whole sequence"));
    }
}

/// Seq-loc --> Seq-align: use CFeat_CI
static void Convert_Seq_loc_Seq_align(
                                      CScope& scope,
                                      const CObject& obj,
                                      CBasicRelation::TObjects& objs,
                                      CBasicRelation::TFlags /*flags*/,
                                      ICanceled*)
{
    const CSeq_loc* loc = dynamic_cast<const CSeq_loc*>(&obj);
    if (loc) {
        SAnnotSelector sel = CSeqUtils::GetAnnotSelector();
        CAlign_CI iter(scope, *loc, sel);
        for (;  iter;  ++iter) {
            const CSeq_align& align = iter.GetOriginalSeq_align();
            objs.push_back(CRelation::SObject(align));
        }
    }
}

/// Seq-loc --> Seq-annot
static void Convert_Seq_loc_Seq_annot(
                                      CScope& scope,
                                      const CObject& obj,
                                      CBasicRelation::TObjects& objs,
                                      CBasicRelation::TFlags /*flags*/,
                                      ICanceled*)
{
    const CSeq_loc* loc = dynamic_cast<const CSeq_loc*>(&obj);
    if (loc) {
        SAnnotSelector sel = CSeqUtils::GetAnnotSelector();
        CAnnot_CI iter(scope, *loc, sel);
        for (;  iter;  ++iter) {
            const CSeq_annot& annot = *iter->GetCompleteSeq_annot();
            objs.push_back(CRelation::SObject(annot));
        }
    }
}

/// Seq-loc --> Seq-feat: use CFeat_CI
static void Convert_Seq_loc_Seq_feat(
                                     CScope& scope,
                                     const CObject& obj,
                                     CBasicRelation::TObjects& objs,
                                     CBasicRelation::TFlags /*flags*/,
                                     ICanceled*)
{
    const CSeq_loc* loc = dynamic_cast<const CSeq_loc*>(&obj);
    if (loc) {
        SAnnotSelector sel = CSeqUtils::GetAnnotSelector();
        CFeat_CI iter(scope, *loc, sel);
        for (;  iter;  ++iter) {
            const CSeq_feat& feat = iter->GetOriginalFeature();
            objs.push_back(CRelation::SObject(feat));
        }
    }
}

/// Seq-loc --> Seq-geaph: use CGraph_CI
static void Convert_Seq_loc_Seq_graph(
                                      CScope& scope,
                                      const CObject& obj,
                                      CBasicRelation::TObjects& objs,
                                      CBasicRelation::TFlags /*flags*/,
                                      ICanceled*)
{
    const CSeq_loc* loc = dynamic_cast<const CSeq_loc*>(&obj);
    if (loc) {
        SAnnotSelector sel = CSeqUtils::GetAnnotSelector();
        CGraph_CI iter(scope, *loc, sel);
        for (;  iter;  ++iter) {
            const CSeq_graph& graph = iter->GetOriginalGraph();
            objs.push_back(CRelation::SObject(graph));
        }
    }
}

/// Seq-loc --> Seq-id: use sequence::GetId()
static void Convert_Seq_loc_Seq_id(
                                   CScope& scope,
                                   const CObject& obj,
                                   CBasicRelation::TObjects& objs,
                                   CBasicRelation::TFlags flags,
                                   ICanceled*)
{
    const CSeq_loc* loc = dynamic_cast<const CSeq_loc*>(&obj);
    if (loc) {
        try {
            CScope* s = &scope;
            if (flags & CBasicRelation::fConvert_NoExpensive) {
                s = NULL;
            }
            CSeq_id_Handle id = sequence::GetIdHandle(*loc, s);
            objs.push_back(CRelation::SObject(*id.GetSeqId()));
        }
        catch (std::exception&) {
        }
    }
}

/// Seq-submit --> Seq-annot
static void Convert_SeqSubmit_Seq_annot(
                                        CScope& scope,
                                        const CObject& obj,
                                        CBasicRelation::TObjects& objs,
                                        CBasicRelation::TFlags /*flags*/,
                                        ICanceled*)
{
    const CSeq_submit* sub = dynamic_cast<const CSeq_submit*> (&obj);
    if (sub) {
        CTypeConstIterator<CSeq_annot> annot_iter(*sub);
        for ( ;  annot_iter;  ++annot_iter) {
            objs.push_back(CRelation::SObject(*annot_iter));
        }
    }
}

/// Seq-submit --> Seq-entry
static void Convert_SeqSubmit_Seq_entry(
                                        CScope& scope,
                                        const CObject& obj,
                                        CBasicRelation::TObjects& objs,
                                        CBasicRelation::TFlags /*flags*/,
                                        ICanceled*)
{
    const CSeq_submit* sub = dynamic_cast<const CSeq_submit*> (&obj);
    if (sub  &&  sub->GetData().IsEntrys()) {
        ITERATE (CSeq_submit::TData::TEntrys, iter,
                 sub->GetData().GetEntrys()) {
            objs.push_back(CRelation::SObject(**iter));
        }
    }
}

/// Seq-submit --> Seq-id
static void Convert_SeqSubmit_Seq_id(
                                     CScope& scope,
                                     const CObject& obj,
                                     CBasicRelation::TObjects& objs,
                                     CBasicRelation::TFlags /*flags*/,
                                     ICanceled* cancel)
{
    const CSeq_submit* sub = dynamic_cast<const CSeq_submit*> (&obj);
    if (sub  &&  sub->GetData().IsEntrys()) {
        CTypeConstIterator<CBioseq> iter(*sub);
        for ( ;  iter;  ++iter) {
            CHECK_CANCELLED;
            const CRef<CSeq_id>& id =
                FindBestChoice(iter->GetId(), CSeq_id::BestRank);

            objs.emplace_back(*id);
        }
    }
}

/// Object --> SerialObject
static void Convert_Object_SerialObject(
                                        CScope& scope,
                                        const CObject& obj,
                                        CBasicRelation::TObjects& objs,
                                        CBasicRelation::TFlags /*flags*/,
                                        ICanceled*)
{
    const CSerialObject* cso = dynamic_cast<const CSerialObject*> (&obj);
    if (cso) {
        objs.emplace_back(*cso);
    }
}

/// *******************************************************************
/// Moved from plugins/algo/init
/// *******************************************************************

enum EGCFlags {
    fGC_Seqs          = 0x01,
    fGC_GenomicCoords = 0x02,
    fGC_Recursive     = 0x04,
    fGC_Defaults = fGC_Seqs | fGC_GenomicCoords
};
typedef int TGCFlags;

static void s_GeneCommentaryToLocations(const CGene_commentary& gc,
                                        CBasicRelation::TObjects& objs,
                                        TGCFlags flags = fGC_Defaults)
{
    string comment;
    switch (gc.GetType()) {
    case CGene_commentary::eType_genomic:
        comment = "Genomic location";
        break;

    case CGene_commentary::eType_mRNA:
        comment = "mRNA sequence";
        break;

    case CGene_commentary::eType_rRNA:
        comment = "rRNA sequence";
        break;

    case CGene_commentary::eType_tRNA:
        comment = "tRNA sequence";
        break;

    case CGene_commentary::eType_peptide:
        comment = "Protein sequence";
        break;

    default:
        break;
    }

    if (gc.IsSetGenomic_coords()  &&  (flags & fGC_GenomicCoords)) {
        ITERATE (CGene_commentary::TGenomic_coords, iter, gc.GetGenomic_coords()) {
            objs.emplace_back(**iter, comment);
        }
    }

    if (gc.IsSetSeqs()  &&  (flags & fGC_Seqs)) {
        ITERATE (CGene_commentary::TSeqs, iter, gc.GetSeqs()) {
            objs.emplace_back(**iter, comment);
        }
    }

    if (flags & fGC_Recursive) {
        if (gc.IsSetProducts()) {
            ITERATE (CGene_commentary::TProducts, iter, gc.GetProducts()) {
                s_GeneCommentaryToLocations(**iter, objs, flags);
            }
        }
        if (gc.IsSetProperties()) {
            ITERATE (CGene_commentary::TProperties, iter, gc.GetProperties()) {
                s_GeneCommentaryToLocations(**iter, objs, flags);
            }
        }
        if (gc.IsSetComment()) {
            ITERATE (CGene_commentary::TComment, iter, gc.GetComment()) {
                s_GeneCommentaryToLocations(**iter, objs, flags);
            }
        }
    }
}


/// Entrezgene --> Seq-loc
void Convert_Entrezgene_Seq_loc_Genomic(CScope& /*scope*/,
                                        const CObject& obj,
                                        CBasicRelation::TObjects& objs,
                                        CBasicRelation::TFlags /*flags*/,
                                        ICanceled* cancel)
{
    const CEntrezgene* egene = dynamic_cast<const CEntrezgene*> (&obj);
    if (egene  &&  egene->IsSetLocus()) {
        ITERATE (CEntrezgene::TLocus, loc_iter, egene->GetLocus()) {
            CHECK_CANCELLED
            const CGene_commentary& gc = **loc_iter;
            s_GeneCommentaryToLocations(gc, objs);
        }
    }
}


void Convert_Entrezgene_Seq_loc_Product(CScope& scope,
                                        const CObject& obj,
                                        CBasicRelation::TObjects& objs,
                                        CBasicRelation::TFlags /*flags*/,
                                        ICanceled* cancel)
{
    const CEntrezgene* egene = dynamic_cast<const CEntrezgene*> (&obj);
    if (egene  &&  egene->IsSetLocus()) {
        /// retrieve all possible locations
        CBasicRelation::TObjects tmp;
        ITERATE (CEntrezgene::TLocus, loc_iter, egene->GetLocus()) {
            CHECK_CANCELLED
            const CGene_commentary& gc = **loc_iter;

            if (gc.IsSetProducts()) {
                ITERATE (CGene_commentary::TProducts, iter, gc.GetProducts()) {
                    s_GeneCommentaryToLocations(**iter, tmp,
                                                fGC_Seqs);
                    if ((*iter)->IsSetProducts()) {
                        ITERATE (CGene_commentary::TProducts, it, (*iter)->GetProducts()) {
                            s_GeneCommentaryToLocations(**it, tmp, fGC_Seqs);
                        }
                    }
                }
            }
        }

        /// now, uniquify these
        set<CSeq_id_Handle> ids;
        ITERATE (CBasicRelation::TObjects, iter, tmp) {
            CHECK_CANCELLED
            const CSeq_loc& loc = dynamic_cast<const CSeq_loc&>(iter->GetObject());
            CSeq_id_Handle sih = sequence::GetIdHandle(loc, &scope);
            if (ids.insert(sih).second) {
                objs.push_back(*iter);
            }
        }
    }
}


void Convert_Entrezgene_Seq_loc_Related(CScope& /*scope*/,
                                        const CObject& obj,
                                        CBasicRelation::TObjects& objs,
                                        CBasicRelation::TFlags /*flags*/,
                                        ICanceled* cancel)
{
    const CEntrezgene* egene = dynamic_cast<const CEntrezgene*> (&obj);
    if (egene  &&  egene->IsSetComments()) {
        ITERATE (CEntrezgene::TComments, iter, egene->GetComments()) {
            CHECK_CANCELLED
            const CGene_commentary& gc = **iter;
            if ( !gc.IsSetHeading()  ||  gc.GetHeading() != "Related Sequences" ) {
                continue;
            }
            s_GeneCommentaryToLocations(gc, objs);

            if (gc.IsSetProducts()) {
                ITERATE (CGene_commentary::TProducts, pit, gc.GetProducts()) {
                    s_GeneCommentaryToLocations(**pit, objs);
                }
            }
        }
    }
}


void Convert_Entrezgene_Seq_loc_RefSeq(CScope& /*scope*/,
                                       const CObject& obj,
                                       CBasicRelation::TObjects& objs,
                                       CBasicRelation::TFlags /*flags*/,
                                       ICanceled* cancel)
{
    const CEntrezgene* egene = dynamic_cast<const CEntrezgene*> (&obj);
    if (egene  &&  egene->IsSetComments()) {
        ITERATE (CEntrezgene::TComments, iter, egene->GetComments()) {
            CHECK_CANCELLED
            const CGene_commentary& gc = **iter;
            string heading;
            if (gc.IsSetHeading()) {
                heading = gc.GetHeading();
            }

            if ( !gc.IsSetHeading()  ||  gc.GetHeading().find("RefSeq") == string::npos ) {
                continue;
            }
            s_GeneCommentaryToLocations(gc, objs, fGC_Defaults | fGC_Recursive);
        }
    }
}


/// Entrezgene --> Seq-loc
void Convert_Entrezgene_Set_Entrezgene(CScope& scope,
                                       const CObject& obj,
                                       CBasicRelation::TObjects& objs,
                                       CBasicRelation::TFlags /*flags*/,
                                       ICanceled* cancel)
{
    const CEntrezgene_Set* gene_set = dynamic_cast<const CEntrezgene_Set*> (&obj);
    if (gene_set) {
        ITERATE (CEntrezgene_Set::Tdata, iter, gene_set->Get()) {
            CHECK_CANCELLED
            objs.emplace_back(**iter);
        }
    }
}


/// GC-Assembly --> Seq-id
static void Convert_GC_Assembly_Seq_id(CScope& scope,
                                       const CObject& obj,
                                       CBasicRelation::TObjects& objs,
                                       CBasicRelation::TFlags /*flags*/,
                                       ICanceled* cancel)
{
    const CGC_Assembly* assm = dynamic_cast<const CGC_Assembly*>(&obj);
    if (assm) {
        vector< CConstRef<CGC_Sequence> >  seqs;

        CHECK_CANCELLED

        CGC_Assembly::TSequenceList chromosomes;
        assm->GetMolecules(chromosomes, CGC_Assembly::eChromosome);

        CHECK_CANCELLED

        CGC_Assembly::TSequenceList top_level_seqs;
        assm->GetMolecules(top_level_seqs, CGC_Assembly::eTopLevel);

        CHECK_CANCELLED

        set< CConstRef<CGC_Sequence> > unique_seqs;
        ITERATE(CGC_Assembly::TSequenceList, it, chromosomes) {
            if (unique_seqs.insert(*it).second) {
                seqs.push_back(*it);
            }
        }
        CHECK_CANCELLED

        ITERATE(CGC_Assembly::TSequenceList, it, top_level_seqs) {
            if (unique_seqs.insert(*it).second) {
                seqs.push_back(*it);
            }
        }
        CHECK_CANCELLED

        const CGC_AssemblyDesc& desc = assm->GetDesc();
        string organism;
        if (desc.IsSetDescr()) {
            ITERATE (CGC_AssemblyDesc::TDescr::Tdata, it, desc.GetDescr().Get()) {
                if ((*it)->IsSource()  &&  (*it)->GetSource().IsSetTaxname()) {
                    organism = (*it)->GetSource().GetTaxname();
                    break;
                }
            }
        }

        string assm_name = assm->GetName();

        CBasicRelation::TObjects gb_objs;
        ITERATE (vector< CConstRef<CGC_Sequence> >, iter, seqs) {
            CHECK_CANCELLED
            CConstRef<CSeq_id> best_id =
                (*iter)->GetSynonymSeq_id(CGC_TypedSeqId::e_Refseq, CGC_SeqIdAlias::e_Public);

            string chr_num = kEmptyStr;
            if ((*iter)->GetReplicon()  &&  (*iter)->GetReplicon()->CanGetName()) {
                chr_num = (*iter)->GetReplicon()->GetName();
            }
            string chr_prefix;
            if ( !chr_num.empty()  &&  chr_num.find("chr") == string::npos) {
                chr_prefix = "chr";
            }


            string extra_info;
            bool patch_type = false; 
            if ((*iter)->CanGetPatch_type()) {
                if ((*iter)->GetPatch_type() == CGC_Sequence::ePatch_type_fix) {
                    extra_info = "fix patch";
                    patch_type = true;
                }  else if ((*iter)->GetPatch_type() == CGC_Sequence::ePatch_type_novel) {
                    extra_info = "novel patch";
                    patch_type = true;
                }
            }
            if ((*iter)->GetParentRelation() == CGC_TaggedSequences::eState_aligned) {
                if ( !extra_info.empty() ) {
                    extra_info += ", ";
                }
                if (patch_type) {
                    extra_info += "aligned";
                }
                else {
                    extra_info += "alt Locus";
                }
            }

            if ( !extra_info.empty() ) {
                if ( !chr_num.empty() ) {
                    extra_info = " (" + extra_info + ")";
                }
            }

            string status;
            switch ((*iter)->GetParentRelation()) {
                case CGC_TaggedSequences::eState_unlocalized:
                    status = "unlocalized";
                    break;
                case CGC_TaggedSequences::eState_unplaced:
                    status = "unplaced";
                    break;
                case CGC_TaggedSequences::eState_aligned:
                    status = "aligned";
                    break;
                default:
                    break;
            }

            string role;
            if ((*iter)->CanGetRoles()) {
                ITERATE(CGC_Sequence::TRoles, r_iter, (*iter)->GetRoles()) {
                    switch(*r_iter) {
                        case eGC_SequenceRole_chromosome:
                            role = "chromosome";
                            break;
                        case eGC_SequenceRole_component:
                            role = "component";
                            break;
                        case eGC_SequenceRole_pseudo_scaffold:
                        case eGC_SequenceRole_submitter_pseudo_scaffold:
                        case eGC_SequenceRole_scaffold:
                            role = "scaffold";
                            break;
                        default:
                            break;
                    }
                    if ( !role.empty() ) {
                        break;
                    }
                }
            }

            string descr;
            if ( !organism.empty() ) {
                descr += organism + " ";
            }
            if (NStr::EqualNocase(chr_num, "mt")) {
                descr += "mitochondrion";
            } else if (NStr::EqualNocase(chr_num, "pltd")) {
                descr += "chloroplast";
            } else if ( !chr_num.empty() ) {
                descr += "chromosome " + chr_num;
            }
            if (role == "scaffold") {
                if (status == "unplaced" || status == "unlocalized") {
                    descr += " " + status;
                }
                descr += " " + role + ", ";
            } else {
                descr += ", ";
            }

            descr += assm_name;
            if (status == "aligned") {
                descr += " alternate locus";
            }

            if (best_id) {
                CConstRef<CSeq_id> id_gi =
                    (*iter)->GetSynonymSeq_id(CGC_TypedSeqId::e_Refseq, CGC_SeqIdAlias::e_Gi);
                objs.emplace_back(*best_id, chr_prefix + chr_num + extra_info);
            }

            CConstRef<CSeq_id> gb_id =
                (*iter)->GetSynonymSeq_id(CGC_TypedSeqId::e_Genbank, CGC_SeqIdAlias::e_Public);
            if (gb_id) {
                CConstRef<CSeq_id> id_gi =
                    (*iter)->GetSynonymSeq_id(CGC_TypedSeqId::e_Genbank, CGC_SeqIdAlias::e_Gi);
                gb_objs.emplace_back(*gb_id, chr_prefix + chr_num + extra_info);
            }
        }
        std::move(gb_objs.begin(), gb_objs.end(), std::back_inserter(objs));
    }
}


/// GC-Assembly --> Seq-loc

static void s_GCSeqsToSeqLocs(const CGC_Assembly& assm, const vector< CConstRef<CGC_Sequence> >&  gcSeqs, CBasicRelation::TObjects& seqLocs, ICanceled* cancel)
{
    bool is_refseq = assm.IsRefSeq();
    ITERATE(vector< CConstRef<CGC_Sequence> >, iter, gcSeqs) {
        CHECK_CANCELLED
            CConstRef<CSeq_id> seq_id;
        if (is_refseq) {
            seq_id = (*iter)->GetSynonymSeq_id(CGC_TypedSeqId::e_Refseq, CGC_SeqIdAlias::e_Public);
        }
        else {
            seq_id = (*iter)->GetSynonymSeq_id(CGC_TypedSeqId::e_Genbank, CGC_SeqIdAlias::e_Public);
        }

        string chr_num = kEmptyStr;
        if ((*iter)->GetReplicon() && (*iter)->GetReplicon()->CanGetName()) {
            chr_num = (*iter)->GetReplicon()->GetName();
        }
        if (!chr_num.empty() && chr_num.find("chr") == string::npos) {
            chr_num = "chr" + chr_num;
        }

        string extra_info;
        bool patch_type = false;
        if ((*iter)->CanGetPatch_type()) {
            if ((*iter)->GetPatch_type() == CGC_Sequence::ePatch_type_fix) {
                extra_info = "fix patch";
                patch_type = true;
            }
            else if ((*iter)->GetPatch_type() == CGC_Sequence::ePatch_type_novel) {
                extra_info = "novel patch";
                patch_type = true;
            }
        }
        if ((*iter)->GetParentRelation() == CGC_TaggedSequences::eState_aligned) {
            if (!extra_info.empty()) {
                extra_info += ", ";
            }
            if (patch_type) {
                extra_info += "aligned";
            }
            else {
                extra_info += "alt Locus";
            }
        }

        if (!extra_info.empty()) {
            if (!chr_num.empty()) {
                chr_num += " (" + extra_info + ")";
            }
            else {
                chr_num = extra_info;
            }
        }

        if (seq_id) {
            CRef<CSeq_loc> loc(new CSeq_loc());
            loc->SetWhole().Assign(*seq_id);
            seqLocs.push_back(CRelation::SObject(*loc, chr_num));
        }
    }
}

static void Convert_GC_Assembly_Seq_loc_Chromosomes(CScope& scope,
    const CObject& obj,
    CBasicRelation::TObjects& objs,
    CBasicRelation::TFlags /*flags*/,
    ICanceled* cancel)
{
    const CGC_Assembly* assm = dynamic_cast<const CGC_Assembly*>(&obj);
    if (assm) {
        CGC_Assembly::TSequenceList chromosomes;
        assm->GetMolecules(chromosomes, CGC_Assembly::eChromosome);
        vector< CConstRef<CGC_Sequence> > seqs(chromosomes.begin(), chromosomes.end());
        CHECK_CANCELLED;
        s_GCSeqsToSeqLocs(*assm, seqs, objs, cancel);
    }
}

static void Convert_GC_Assembly_Seq_loc_TopLevel(CScope& scope,
    const CObject& obj,
    CBasicRelation::TObjects& objs,
    CBasicRelation::TFlags /*flags*/,
    ICanceled* cancel)
{
    const CGC_Assembly* assm = dynamic_cast<const CGC_Assembly*>(&obj);
    if (assm) {
        CGC_Assembly::TSequenceList chromosomes;
        assm->GetMolecules(chromosomes, CGC_Assembly::eChromosome);
        CHECK_CANCELLED;

        CGC_Assembly::TSequenceList top_level_seqs;
        assm->GetMolecules(top_level_seqs, CGC_Assembly::eTopLevel);
        CHECK_CANCELLED;

        set< CConstRef<CGC_Sequence> > unique_seqs(chromosomes.begin(), chromosomes.end());
        vector<CConstRef<CGC_Sequence> > seqs;
        for (auto i : top_level_seqs) {
            if (unique_seqs.insert(i).second)
                seqs.push_back(i);
        }

        CHECK_CANCELLED;
        s_GCSeqsToSeqLocs(*assm, seqs, objs, cancel);
    }
}

/// BioTreeContainer --> Seq-id
static void Convert_BioTreeContainer_Seq_id(CScope& scope,
                                            const CObject& obj,
                                            CBasicRelation::TObjects& objs,
                                            CBasicRelation::TFlags /*flags*/,
                                            ICanceled* cancel)
{
    const CBioTreeContainer* biotree = dynamic_cast<const CBioTreeContainer*>(&obj);
    if (biotree) {
        CHECK_CANCELLED;

        int seq_id_idx = -1;
        const CFeatureDictSet::Tdata& fdict = biotree->GetFdict().Get();
        ITERATE(CFeatureDictSet::Tdata, it, fdict) {
            if (NStr::EqualNocase((*it)->GetName(), "seq-id")) {
                seq_id_idx = (*it)->GetId();
                break;
            }
        }

        if (seq_id_idx < 0) return;

        set<string> ids;
        const CNodeSet::Tdata& nodes = biotree->GetNodes().Get();
        ITERATE(CNodeSet::Tdata, it, nodes) {
            CHECK_CANCELLED;
            const CNode& node = **it;
            if (node.CanGetFeatures()) {
                const CNodeFeatureSet::Tdata& features = node.GetFeatures().Get();
                ITERATE(CNodeFeatureSet::Tdata, it, features) {
                    if ((*it)->GetFeatureid() == seq_id_idx) {
                        ids.insert((*it)->GetValue());
                        break;
                    }
                }
            }
        }


        ITERATE (set<string>, iter, ids) {
            CHECK_CANCELLED;
            CConstRef<CSeq_id> seq_id;
            try {
                seq_id.Reset(new CSeq_id(*iter));
            } catch (CException&) {
            }

            if (seq_id) {
                objs.push_back(CRelation::SObject(*seq_id));
            }
        }
    }
}



struct SConverterEntry
{
    const char*      m_Name;
    CBasicRelation::TFNConvert m_pFnCvt;
    const CTypeInfo* m_FromType;
    const CTypeInfo* m_ToType;
    const CBasicRelation::SPropPair* m_Properties;
};

BEGIN_RELATION_PROPERTY_LIST(Seq_feat_Seq_loc_Gen_Loc_Prop)
RELATION_PROPERTY_ENTRY(CSeq_loc::GetTypeInfo()->GetName(), "Genomic Location")
END_RELATION_PROPERTY_LIST();

BEGIN_RELATION_PROPERTY_LIST(Seq_feat_Seq_loc_Gen_Loc_TotalRange_Prop)
RELATION_PROPERTY_ENTRY(CSeq_loc::GetTypeInfo()->GetName(), "Genomic Location (total range)")
END_RELATION_PROPERTY_LIST();

BEGIN_RELATION_PROPERTY_LIST(Seq_feat_Seq_loc_Prod_Loc_Prop)
RELATION_PROPERTY_ENTRY(CSeq_loc::GetTypeInfo()->GetName(), "Product Location")
END_RELATION_PROPERTY_LIST();

BEGIN_RELATION_PROPERTY_LIST(Seq_feat_Seq_loc_dbxref_Loc_Prop)
RELATION_PROPERTY_ENTRY(CSeq_loc::GetTypeInfo()->GetName(), "dbxref Location")
END_RELATION_PROPERTY_LIST();


BEGIN_RELATION_PROPERTY_LIST(Convert_Seq_align_Seq_loc_Basic_Prop)
RELATION_PROPERTY_ENTRY(CSeq_loc::GetTypeInfo()->GetName(), "Basic")
END_RELATION_PROPERTY_LIST();

BEGIN_RELATION_PROPERTY_LIST(Convert_Seq_align_Seq_loc_Coalesced_Prop)
RELATION_PROPERTY_ENTRY(CSeq_loc::GetTypeInfo()->GetName(), "Total Range")
END_RELATION_PROPERTY_LIST();

BEGIN_RELATION_PROPERTY_LIST(Seq_annot_Seq_loc_All_Prop)
RELATION_PROPERTY_ENTRY(CSeq_loc::GetTypeInfo()->GetName(), "Genomic Location")
END_RELATION_PROPERTY_LIST();

BEGIN_RELATION_PROPERTY_LIST(Seq_annot_Seq_loc_TotalRange_Prop)
RELATION_PROPERTY_ENTRY(CSeq_loc::GetTypeInfo()->GetName(), "Genomic Location (total range)")
END_RELATION_PROPERTY_LIST();

/// entrezgene: moved from plugins/algo/init
BEGIN_RELATION_PROPERTY_LIST(Convert_Entrezgene_Seq_loc_Genomic_Prop)
RELATION_PROPERTY_ENTRY(CSeq_loc::GetTypeInfo()->GetName(), "Genomic Location")
END_RELATION_PROPERTY_LIST();

BEGIN_RELATION_PROPERTY_LIST(Convert_Entrezgene_Seq_loc_Product_Prop)
RELATION_PROPERTY_ENTRY(CSeq_loc::GetTypeInfo()->GetName(), "Product Location")
END_RELATION_PROPERTY_LIST();

BEGIN_RELATION_PROPERTY_LIST(Convert_Entrezgene_Seq_loc_Related_Prop)
RELATION_PROPERTY_ENTRY(CSeq_loc::GetTypeInfo()->GetName(), "Related Sequences")
END_RELATION_PROPERTY_LIST();

BEGIN_RELATION_PROPERTY_LIST(Convert_Entrezgene_Seq_loc_RefSeq_Prop)
RELATION_PROPERTY_ENTRY(CSeq_loc::GetTypeInfo()->GetName(), "RefSeq Sequences")
END_RELATION_PROPERTY_LIST();

BEGIN_RELATION_PROPERTY_LIST(Convert_GC_Assembly_Seq_loc_Chromosomes_Prop)
RELATION_PROPERTY_ENTRY(CSeq_loc::GetTypeInfo()->GetName(), "Assembly Chromosomes")
END_RELATION_PROPERTY_LIST();

BEGIN_RELATION_PROPERTY_LIST(Convert_GC_Assembly_Seq_loc_TopLevel_Prop)
RELATION_PROPERTY_ENTRY(CSeq_loc::GetTypeInfo()->GetName(), "Assembly Top Level sequences")
END_RELATION_PROPERTY_LIST();

static SConverterEntry m_ConverterEntries[] = {
        // Bioseq
        { "Bioseq --> Seq-entry", Convert_Bioseq_Seq_entry,
            CBioseq::GetTypeInfo(), CSeq_entry::GetTypeInfo() },
        { "Bioseq --> Seq-id", Convert_Bioseq_Seq_id,
            CBioseq::GetTypeInfo(), CSeq_id::GetTypeInfo() },
        // Bioseq-set
        { "Bioseq-set --> Bioseq", Convert_Bioseq_set_Bioseq,
            CBioseq_set::GetTypeInfo(), CBioseq::GetTypeInfo() },
        { "Bioseq-set --> Seq-entry", Convert_Bioseq_set_Seq_entry,
            CBioseq_set::GetTypeInfo(), CSeq_entry::GetTypeInfo() },
        // Dense-seg
        { "Dense-seg --> Seq-align", Convert_Dense_seg_Seq_align,
            CDense_seg::GetTypeInfo(), CSeq_align::GetTypeInfo() },
        { "Seq-align --> Seq-annot", Convert_Seq_align_Seq_annot,
            CSeq_align::GetTypeInfo(), CSeq_annot::GetTypeInfo() },
        // Seq-align
        { "Seq-align -> Seq-id", Convert_Seq_align_Seq_id,
            CSeq_align::GetTypeInfo(), CSeq_id::GetTypeInfo() },
        { "Seq-align -> Seq-loc : Basic", Convert_Seq_align_Seq_loc_Basic,
            CSeq_align::GetTypeInfo(), CSeq_loc::GetTypeInfo(),
            Convert_Seq_align_Seq_loc_Basic_Prop },
        { "Seq-align -> Seq-loc : Coalesced", Convert_Seq_align_Seq_loc_Coalesced,
            CSeq_align::GetTypeInfo(), CSeq_loc::GetTypeInfo(),
            Convert_Seq_align_Seq_loc_Coalesced_Prop },
        /*
           { "Seq-align -> Seq-loc", Convert_Seq_align_Seq_loc,
           CSeq_align::GetTypeInfo(), CSeq_loc::GetTypeInfo() },
         */
        // Seq-align-set
        { "Seq-align-set --> Seq-align", Convert_Seq_align_set_Seq_align,
            CSeq_align_set::GetTypeInfo(), CSeq_align::GetTypeInfo() },
        { "Seq-align-set -> Seq-loc", Convert_Seq_align_set_Seq_loc,
            CSeq_align_set::GetTypeInfo(), CSeq_loc::GetTypeInfo() },
        { "Seq-align-set --> Seq-annot", Convert_Seq_align_set_Seq_annot,
            CSeq_align_set::GetTypeInfo(), CSeq_annot::GetTypeInfo() },
        // Seq-annot
        { "Seq-annot --> Seq-align", Convert_Seq_annot_Seq_align,
            CSeq_annot::GetTypeInfo(), CSeq_align::GetTypeInfo() },
        { "Seq-annot --> Seq-entry", Convert_Seq_annot_Seq_entry,
            CSeq_annot::GetTypeInfo(), CSeq_entry::GetTypeInfo() },
        { "Seq-annot --> Seq-feat", Convert_Seq_annot_Seq_feat,
            CSeq_annot::GetTypeInfo(), CSeq_feat::GetTypeInfo() },
        { "Seq-annot --> Seq-graph", Convert_Seq_annot_Seq_graph,
            CSeq_annot::GetTypeInfo(), CSeq_graph::GetTypeInfo() },
        { "Seq-annot --> Seq-id", Convert_Seq_annot_Seq_id,
            CSeq_annot::GetTypeInfo(), CSeq_id::GetTypeInfo() },

        { "Seq-annot --> Seq-loc : All", Convert_Seq_annot_Seq_loc_All,
            CSeq_annot::GetTypeInfo(), CSeq_loc::GetTypeInfo(),
            Seq_annot_Seq_loc_All_Prop },
        { "Seq-annot --> Seq-loc : Total range", Convert_Seq_annot_Seq_loc_TotalRange,
            CSeq_annot::GetTypeInfo(), CSeq_loc::GetTypeInfo(),
            Seq_annot_Seq_loc_TotalRange_Prop },

        // Seq-entry
        { "Seq-entry --> Bioseq", Convert_Seq_entry_Bioseq,
            CSeq_entry::GetTypeInfo(), CBioseq::GetTypeInfo() },
        { "Seq-entry --> Bioseq-set", Convert_Seq_entry_Bioseq_set,
            CSeq_entry::GetTypeInfo(), CBioseq_set::GetTypeInfo() },
        { "Seq-entry --> Seq-loc", Convert_Seq_entry_Seq_loc,
            CSeq_entry::GetTypeInfo(), CSeq_loc::GetTypeInfo() },
        { "Seq-entry --> Seq-id", Convert_Seq_entry_Seq_id,
            CSeq_entry::GetTypeInfo(), CSeq_id::GetTypeInfo() },
        { "Seq-entry --> Seq-submit", Convert_Seq_entry_SeqSubmit,
            CSeq_entry::GetTypeInfo(), CSeq_submit::GetTypeInfo() },
        { "Seq-entry --> Seq-annot", Convert_Seq_entry_Seq_annot,
            CSeq_entry::GetTypeInfo(), CSeq_annot::GetTypeInfo() },
        { "Seq-entry --> Seq-align", Convert_Seq_entry_Seq_align,
            CSeq_entry::GetTypeInfo(), CSeq_align::GetTypeInfo() },
        // Seq-feat
        /**
          { "Seq-feat --> Seq-id", Convert_Seq_feat_Seq_id,
          CSeq_feat::GetTypeInfo(), CSeq_id::GetTypeInfo() },
         **/
        /*
           { "Seq-feat --> Seq-loc", Convert_Seq_feat_Seq_loc,
           CSeq_feat::GetTypeInfo(), CSeq_loc::GetTypeInfo() },
         */
        { "Seq-feat --> Seq-loc : Genomic Location", Convert_Seq_feat_Seq_loc_Gen_Loc,
            CSeq_feat::GetTypeInfo(), CSeq_loc::GetTypeInfo(),
            Seq_feat_Seq_loc_Gen_Loc_Prop },
        { "Seq-feat --> Seq-loc : Genomic Location (total range)", Convert_Seq_feat_Seq_loc_Gen_Loc_TotalRange,
            CSeq_feat::GetTypeInfo(), CSeq_loc::GetTypeInfo(),
            Seq_feat_Seq_loc_Gen_Loc_TotalRange_Prop },
        { "Seq-feat --> Seq-loc : Product Location", Convert_Seq_feat_Seq_loc_Prod_Loc,
            CSeq_feat::GetTypeInfo(), CSeq_loc::GetTypeInfo(),
            Seq_feat_Seq_loc_Prod_Loc_Prop },
        { "Seq-feat --> Seq-loc : dbxref Location", Convert_Seq_feat_Seq_loc_dbxref_Loc,
            CSeq_feat::GetTypeInfo(), CSeq_loc::GetTypeInfo(),
            Seq_feat_Seq_loc_dbxref_Loc_Prop },
        // Seq-id
        { "Seq-id --> Bioseq: use CBioseq_Handle", Convert_Seq_id_Bioseq,
            CSeq_id::GetTypeInfo(), CBioseq::GetTypeInfo() },
        { "Seq-id --> Bioseq-set: use CBioseq_Handle", Convert_Seq_id_Bioseq_set,
            CSeq_id::GetTypeInfo(), CBioseq_set::GetTypeInfo() },
        { "Seq-id --> Seq-entry: use CBioseq_Handle", Convert_Seq_id_Seq_entry,
            CSeq_id::GetTypeInfo(), CSeq_entry::GetTypeInfo() },
        { "Seq-id --> Seq-loc: use Seq-loc.whole representation", Convert_Seq_id_Seq_loc,
            CSeq_id::GetTypeInfo(), CSeq_loc::GetTypeInfo() },
        // Seq-loc
        { "Seq-loc --> Seq-align: use CFeat_CI", Convert_Seq_loc_Seq_align,
            CSeq_loc::GetTypeInfo(), CSeq_align::GetTypeInfo() },
        { "Seq-loc --> Seq-annot", Convert_Seq_loc_Seq_annot,
            CSeq_loc::GetTypeInfo(), CSeq_annot::GetTypeInfo() },
        { "Seq-loc --> Seq-feat: use CFeat_CI", Convert_Seq_loc_Seq_feat,
            CSeq_loc::GetTypeInfo(), CSeq_feat::GetTypeInfo() },
        { "Seq-loc --> Seq-graph: use CGraph_CI", Convert_Seq_loc_Seq_graph,
            CSeq_loc::GetTypeInfo(), CSeq_graph::GetTypeInfo() },
        { "Seq-loc --> Seq-id: use sequence::GetId()", Convert_Seq_loc_Seq_id,
            CSeq_loc::GetTypeInfo(), CSeq_id::GetTypeInfo() },
        // Seq-submit
        { "Seq-submit --> Seq-annot", Convert_SeqSubmit_Seq_annot,
            CSeq_submit::GetTypeInfo(), CSeq_annot::GetTypeInfo() },
        { "Seq-submit --> Seq-entry", Convert_SeqSubmit_Seq_entry,
            CSeq_submit::GetTypeInfo(), CSeq_entry::GetTypeInfo() },
        { "Seq-submit --> Seq-id", Convert_SeqSubmit_Seq_id,
            CSeq_submit::GetTypeInfo(), CSeq_id::GetTypeInfo() },
        // entrezgene
        { "Entrezgene --> Seq-loc : Genomic", Convert_Entrezgene_Seq_loc_Genomic,
            CEntrezgene::GetTypeInfo(), CSeq_loc::GetTypeInfo(),
            Convert_Entrezgene_Seq_loc_Genomic_Prop },
        { "Entrezgene --> Seq-loc : Product", Convert_Entrezgene_Seq_loc_Product,
            CEntrezgene::GetTypeInfo(), CSeq_loc::GetTypeInfo(),
            Convert_Entrezgene_Seq_loc_Product_Prop },
        { "Entrezgene --> Seq-loc : Related", Convert_Entrezgene_Seq_loc_Related,
            CEntrezgene::GetTypeInfo(), CSeq_loc::GetTypeInfo(),
            Convert_Entrezgene_Seq_loc_Related_Prop },
        { "Entrezgene --> Seq-loc : RefSeq", Convert_Entrezgene_Seq_loc_RefSeq,
            CEntrezgene::GetTypeInfo(), CSeq_loc::GetTypeInfo(),
            Convert_Entrezgene_Seq_loc_RefSeq_Prop },
        { "Entrezgene-Set --> Entrezgene", Convert_Entrezgene_Set_Entrezgene,
            CEntrezgene_Set::GetTypeInfo(), CEntrezgene::GetTypeInfo(), NULL },
        // BioTreeContainer
        { "BioTreeContainer --> Seq-id", Convert_BioTreeContainer_Seq_id,
            CBioTreeContainer::GetTypeInfo(), CSeq_id::GetTypeInfo() },
        // GC-Assembly
        { 
            "GC-Assembly --> Seq-loc : Chromosomes",
            Convert_GC_Assembly_Seq_loc_Chromosomes,
            CGC_Assembly::GetTypeInfo(),
            CSeq_loc::GetTypeInfo(),
            Convert_GC_Assembly_Seq_loc_Chromosomes_Prop
       },
       {
           "GC-Assembly --> Seq-loc : TopLevel",
            Convert_GC_Assembly_Seq_loc_TopLevel,
            CGC_Assembly::GetTypeInfo(),
            CSeq_loc::GetTypeInfo(),
            Convert_GC_Assembly_Seq_loc_TopLevel_Prop
       },
       {
           "GC-Assembly --> Seq-id",
           Convert_GC_Assembly_Seq_id,
           CGC_Assembly::GetTypeInfo(),
           CSeq_id::GetTypeInfo()
       }

    };

void init_basic_converters()
{
    CObjectConverter::Register(new CIdentityRelation());

    for (size_t i = 0;
         i < sizeof(m_ConverterEntries)/sizeof(m_ConverterEntries[0]); ++i) {
        CObjectConverter::Register
            (new CBasicRelation(m_ConverterEntries[i].m_Name,
                                m_ConverterEntries[i].m_FromType->GetName(),
                                m_ConverterEntries[i].m_ToType->GetName(),
                                m_ConverterEntries[i].m_pFnCvt,
                                m_ConverterEntries[i].m_Properties));
    }

    CObjectConverter::Register
        (new CBasicRelation("Object --> SerialObject",
                            "Object", "SerialObject",
                            Convert_Object_SerialObject));
}

END_NCBI_SCOPE
