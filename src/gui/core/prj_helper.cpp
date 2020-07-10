/*  $Id: prj_helper.cpp 40209 2018-01-08 20:35:14Z joukovv $
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
 * Authors:  Victor Joukov, Dmitry Rudnev
 *
 * File Description:  helper for adding items to a GBench project
 *
 */


#include <ncbi_pch.hpp>
#include <gui/core/prj_helper.hpp>
#include <gui/core/loading_app_job.hpp>
#include <gui/objutils/label.hpp>

#include <objects/seqalign/Seq_align.hpp>
#include <objects/seqalign/Seq_align_set.hpp>
#include <objects/seqalign/Dense_seg.hpp>
#include <objects/seqalign/Score.hpp>
#include <objects/general/Object_id.hpp>

#include <algo/sequence/util.hpp>
#include <algo/blast/blastinput/blast_scope_src.hpp>
#include <algo/sequence/align_cleanup.hpp>

BEGIN_NCBI_SCOPE



static void sPrepareSeqAlign(CSeq_align& align, list< CRef<CSeq_align> >& new_aligns)
{
    _ASSERT(align.IsSetScore());

    ITERATE (CSeq_align::TScore, it, align.GetScore()) {
        const CScore& score = **it;
        if (score.GetId().IsStr()  &&  score.GetId().GetStr() == "use_this_gi") {
            /// we perform a replacment from the gi indicated as the query
            /// NB: ASSUMPTIONS MADE
            ///  - BLAST encodes the identical gi using this score
            ///  - The gi to be replaced is the second
            TGi new_gi = GI_FROM(int, score.GetValue().GetInt());
            CRef<CSeq_align> new_align(new CSeq_align);
            new_align->Assign(align);

            switch (new_align->GetSegs().Which()) {
            case CSeq_align::TSegs::e_Denseg:   {{
                CSeq_id& id = *new_align->SetSegs().SetDenseg().SetIds()[1];
                id.SetGi(new_gi);
                break;
            }}
            default:
                /// we don't handle the others yet; fail silently as there is
                /// no real change, but place an assert to catch this when debugging
                _ASSERT(false);
                continue;
                break;
            }

            /// erase the use_this_gi scores from this new alignment
            CSeq_align::TScore::iterator i = new_align->SetScore().begin();
            for ( ;  i != new_align->SetScore().end();  ) {
                const CScore& s = **i;
                if (s.GetId().IsStr()  && s.GetId().GetStr() == "use_this_gi") {
                    i = new_align->SetScore().erase(i);
                } else {
                    ++i;
                }
            }
            /// save it in place and iterate
            new_aligns.push_back(new_align);
        }
    }
}


static const string kBLASTProjectItemTag = "blast_project_item";


// create  BLAST result project items from a given RID
// if the results contain any unresolvable/local sequences, 
// add the sequences to the project
// mostly based on code borrowed from CDLGbprjCreator::CreateProject(rid)
void CGBProjectHelper::AddProjectItemsFromRID(CRef<objects::CScope> scope,
                                       CSeq_align_set& results,
                                       bool isFindComp,
									   CRef<blast::CRemoteBlast> RemoteBlast,
                                       const set<string>& filter,
                                       CDataLoadingAppJob& job,
                                       SRIDStatInfo& RIDStatInfo)
{
    list<CConstRef<CSeq_id> > unresolved_subjects;
    const string& rid(RemoteBlast->GetRID());
    const string& rid_title(RemoteBlast->GetTitle());
    
    typedef CSeq_annot::TData::TAlign TAlign;

    TAlign& an_align = results.Set();
    string sProjectItemBaseName(rid_title.empty() ? "BLAST Results for: " + rid_title : "BLAST Results, RID: " + rid);

    bool is_db_search = RemoteBlast->IsDbSearch();

    bool write_local_seq = false;
    CRef<CBlast4_queries> qq = RemoteBlast->GetQueries();
    int num_queries = qq->GetNumQueries();
    if (num_queries == 1  &&  qq->IsSeq_loc_list()) {
        const list<CRef<CSeq_loc> > seq_loc_list = qq->GetSeq_loc_list();
        CRef<CSeq_loc> seq_loc = seq_loc_list.front();
        if (!seq_loc->IsWhole()) {
            CRange<TSeqPos> rng = seq_loc->GetTotalRange();
            RIDStatInfo.m_QueryRangeSet = true;
            RIDStatInfo.m_QueryBeg = rng.GetFrom();
            RIDStatInfo.m_QueryEnd = rng.GetTo();
        }
    }

    bool report_total = true;
    typedef map< string, CRef<CSeq_align_set> > TAlignSetMap;
    TAlignSetMap align_set_map;

    for ( TAlign::iterator it = an_align.begin();  it != an_align.end();  ++it) {
        CRef<CSeq_align> align = *it;
        const CSeq_id& seq_id = align->GetSeq_id(0);
        const CSeq_id& target_id = align->GetSeq_id(1);

        // if a filter is defined, use only targets present in the filter
        string target_id_str = target_id.AsFastaString();
        if (filter.size() && filter.find(target_id_str) == filter.end()) {
            continue;
        }
        // sort alignments according to query sequence
        string id_str;
        seq_id.GetLabel(&id_str);
        CRef<CSeq_align_set>& curr_set = align_set_map[id_str];
        if ( !curr_set ) {
            curr_set.Reset(new CSeq_align_set);
        }
        curr_set->Set().push_back(*it);
        if (align->IsSetScore()) {
            list< CRef<CSeq_align> > new_aligns;

            sPrepareSeqAlign(*align, new_aligns);

            if (new_aligns.size()) {
                an_align.insert(it, new_aligns.begin(), new_aligns.end());
            }
        }
        if (report_total) {
            TSeqPos beg = align->GetSeqStart(0);
            TSeqPos end = align->GetSeqStop(0);
            if (RIDStatInfo.m_QueryId.empty()) {
                RIDStatInfo.m_QueryId = seq_id.AsFastaString();
                RIDStatInfo.m_TotalBeg = beg;
                RIDStatInfo.m_TotalEnd = end;
                write_local_seq = write_local_seq  ||  (seq_id.Which() == CSeq_id::e_Local);
            } else if (RIDStatInfo.m_QueryId != seq_id.AsFastaString()) {
                // Alignment does not have a single query, can not report
                RIDStatInfo.m_QueryId = "";
                // write_local_seq = false;
                report_total = false;
            }
            if (beg < RIDStatInfo.m_TotalBeg) RIDStatInfo.m_TotalBeg = beg;
            if (end > RIDStatInfo.m_TotalEnd) RIDStatInfo.m_TotalEnd = end;
        }
        // If target_id is unresolvable through usual means, that is either
        // bl2seq or regular Scope can not get its handle, put it away for
        // later resolution attempt
        if(!is_db_search || !scope->GetBioseqHandle(target_id)) {
            unresolved_subjects.push_back(CConstRef<CSeq_id>(&target_id));
        }
        // check for source sequences being local
        write_local_seq = write_local_seq || (seq_id.Which() == CSeq_id::e_Local);
    }


    // Compartmentalize alignment for each query here
    TAlignSetMap cleanup_align_set_map;
    if (isFindComp) {
        // Use CAlignCleanup to clean up alignments (truncating and grouping)
        CAlignCleanup cleanup(*scope);
        cleanup.SortInputsByScore(true);
        cleanup.PreserveRows(false);
        cleanup.FillUnaligned(false);
        
        ITERATE(TAlignSetMap, aln_set_iter, align_set_map) {
            CAlignCleanup::TAligns aligns_out_tmp;
            cleanup.Cleanup(aln_set_iter->second->Get(), aligns_out_tmp,
                            CAlignCleanup::eAnchoredAlign);
            
            if ( !aligns_out_tmp.empty()  &&  aligns_out_tmp.size() != aln_set_iter->second->Get().size() )  {
                CRef<CSeq_align_set> aln_set(new CSeq_align_set);
                aln_set->Set() = aligns_out_tmp;
                cleanup_align_set_map.insert(
                    TAlignSetMap::value_type(aln_set_iter->first, aln_set));
            }
        }
    }

    NON_CONST_ITERATE(TAlignSetMap, aln_set_iter, align_set_map) {
        // cerr << "Entry for " << aln_set_iter->first << ":" << MSerial_AsnText << *(aln_set_iter->second) << endl;
        string annot_base_name(sProjectItemBaseName + ", Query: " + aln_set_iter->first);
        
        /// Check if there is a corresponding cleaned-up alignment set.
        /// If yes, put it before the original one.
        TAlignSetMap::iterator cleaned_iter = cleanup_align_set_map.find(aln_set_iter->first);
        if (cleaned_iter != cleanup_align_set_map.end()) {
            CRef<CSeq_annot> annot(new CSeq_annot);
            annot->SetData().SetAlign() = cleaned_iter->second->Set();
            annot->SetNameDesc("Cleaned Alignments - " + annot_base_name);
            annot->SetCreateDate(CTime(CTime::eCurrent));
            CRef<CProjectItem> item(new CProjectItem);
            item->SetObject(*annot);
            item->SetLabel(annot_base_name);
            item->AddTag(kBLASTProjectItemTag);
            job.AddProjectItem(*item);
            
        }
        CRef<CSeq_annot> annot(new CSeq_annot);
        annot->SetData().SetAlign() = aln_set_iter->second->Set();
        annot->SetCreateDate(CTime(CTime::eCurrent));
        annot->SetNameDesc(annot_base_name);

        CRef<CProjectItem> item(new CProjectItem);
        item->SetObject(*annot);
        item->SetLabel(annot_base_name);
        item->AddTag(kBLASTProjectItemTag);
        job.AddProjectItem(*item);
    }

    string label;
    // add local query sequences
    if (write_local_seq  &&  qq->IsBioseq_set()) {
        // TODO: do we need to exclude bioseq with non-local id
        CBioseq_set& bss = qq->SetBioseq_set();
        NON_CONST_ITERATE(CBioseq_set::TSeq_set, iter, bss.SetSeq_set()) {
            const CSeq_entry& entry = **iter;
            if (entry.IsSeq() && entry.GetSeq().IsSetId()) {
                auto ids = entry.GetSeq().GetId();
                bool found_in_scope = false;
                for (auto seq_id : ids) {
                    if (scope->GetBioseqHandle(*seq_id)) {
                        found_in_scope = true;
                        break;
                    }
                }
                if (found_in_scope) continue;
            }
            CRef<CProjectItem> new_item(new CProjectItem());
            (*iter)->GetLabel(&label, CSeq_entry::eBoth);
            new_item->SetLabel(label);
            new_item->AddTag(kBLASTProjectItemTag);
            label.clear();
            new_item->SetObject(*(*iter));
            job.AddProjectItem(*new_item);
        }
    }
    // resolve and create additional items for unresolved targets
    if (unresolved_subjects.size()) {
        if(is_db_search) {
            // Use Blast DB scope to get the sequences 
            // so that resulting Genome Workbench project is
            // self-contained.
            CRef<objects::CObjectManager> objMgr(CObjectManager::GetInstance());
            CRef<objects::CScope> blast_scope(new CScope(*objMgr));
            CRef<CBlast4_database> bdb(RemoteBlast->GetDatabases());
            blast::SDataLoaderConfig bdlc(bdb->GetName(), bdb->IsProtein());
            blast::CBlastScopeSource bss(bdlc, objMgr);
            bss.AddDataLoaders(blast_scope);
            ITERATE(list<CConstRef<CSeq_id> >, it, unresolved_subjects) {
                CBioseq_Handle bsh = blast_scope->GetBioseqHandle(**it);
                if (bsh) {
                    CConstRef<CBioseq> bioseq(bsh.GetCompleteBioseq());
                    CRef<CSeq_entry> entry(new CSeq_entry);
                    entry->SetSeq(static_cast<CBioseq&>(*(const_cast<CSerialObject*>(static_cast<const CSerialObject*>(&*bioseq)))));
                    CRef<CProjectItem> new_item(new CProjectItem());
                    entry->GetLabel(&label, CSeq_entry::eBoth);
                    new_item->SetLabel(label);
                    new_item->AddTag(kBLASTProjectItemTag);
                    label.clear();
                    new_item->SetObject(*entry);
                    job.AddProjectItem(*new_item);
                }
            }
        } else {
            // Add subject sequences from RID to project
            list<CRef<CBioseq> > subjects = RemoteBlast->GetSubjectSequences();
            NON_CONST_ITERATE(list<CRef<CBioseq> >, it, subjects) {
                CRef<CBioseq> bioseq_ref(*it);
                CRef<CSeq_entry> entry(new CSeq_entry);
                entry->SetSeq(*bioseq_ref);
                CRef<CProjectItem> new_item(new CProjectItem());
                entry->GetLabel(&label, CSeq_entry::eBoth);
                new_item->SetLabel(label);
                new_item->AddTag(kBLASTProjectItemTag);
                label.clear();
                new_item->SetObject(*entry);
                job.AddProjectItem(*new_item);
            }
        }
    }
}




END_NCBI_SCOPE
