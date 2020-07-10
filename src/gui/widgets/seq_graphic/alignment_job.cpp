/*  $Id: alignment_job.cpp 44887 2020-04-08 17:09:43Z shkeda $
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
#include <ncbi_pch.hpp>


#include <gui/widgets/seq_graphic/alignment_job.hpp>
#include <gui/widgets/seq_graphic/graph_cache.hpp>
#include <gui/widgets/seq_graphic/sparse_graph.hpp>
#include <gui/widgets/seq_graphic/alignment_glyph.hpp>
#include <gui/widgets/seq_graphic/gene_model_group.hpp>
#include <gui/widgets/seq_graphic/cds_glyph.hpp>
#include <gui/widgets/seq_graphic/mate_pair_glyph.hpp>
#include <gui/widgets/seq_graphic/alignment_smear_glyph.hpp>
#include <gui/widgets/seq_graphic/alnvec_graphic_ds.hpp>
#include <gui/widgets/seq_graphic/sparsealn_graphic_ds.hpp>
#include <gui/widgets/seq_graphic/denseg_graphic_ds.hpp>
#include <gui/widgets/seq_graphic/simple_graphic_ds.hpp>
#include <gui/widgets/seq_graphic/alignment_sorter_factory.hpp>
#include <gui/widgets/seq_graphic/named_group.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_utils.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/objutils/na_utils.hpp>

#include <objtools/alnmgr/aln_converters.hpp>
#include <objtools/alnmgr/aln_builders.hpp>
#include <objtools/alnmgr/aln_container.hpp>
#include <objmgr/align_ci.hpp>
#include <objmgr/util/sequence.hpp>
#include <objmgr/util/feature.hpp>
#include <objmgr/seq_loc_mapper.hpp>
#include <objmgr/graph_ci.hpp>
#include <objmgr/impl/synonyms.hpp>

#include <objmgr/impl/scope_info.hpp>

#include <objects/general/Dbtag.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/general/User_object.hpp>
#include <objects/seq/Annot_descr.hpp>
#include <objects/seq/Annotdesc.hpp>
#include <objects/seqres/Byte_graph.hpp>
#include <objects/seqres/Int_graph.hpp>

#include <corelib/ncbiutil.hpp>
#include <gui/widgets/seq_graphic/layout_policy.hpp>

#include <math.h>

#include <gui/widgets/seq_graphic/column_layout_policy.hpp>
#include <future>
#include <corelib/rwstream.hpp>
#include <util/checksum.hpp>
#include <connect/services/grid_worker_app.hpp>
#include <corelib/ncbiapp.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/// always show individual alignments when zoom level is below this threshold.
/// Unit: bases per screen pixel (BPP)
static const double kSequenceZoomLevel = 1.0/8.;

/// Scale at which align details are made visible 
/// Align details: unaligned tails, mate pairs
static const double kDetailsZoomLevel = 2;

/// maximal number of alignments allowed to load to avoid potential memery
/// and performance problem.
/// Unit: number of alignments
static const int kMaxAlignmentLoaded = 250000;



///////////////////////////////////////////////////////////////////////////////
/// CSGAlignmentJob
///////////////////////////////////////////////////////////////////////////////
typedef pair<CConstRef<IAlnGraphicDataSource>, int> TAlnDS_Anchor;
typedef vector<TAlnDS_Anchor> TAlnDataSources;

static void s_BuildAlignDataSource(CBioseq_Handle& handle,
                                   vector<CSeq_align::TDim>& anchors,
                                   const CSeq_align& align,
                                   bool sparseAln,
                                   TModelUnit window,
                                   TAlnDataSources& aln_datasources);

static void s_GetAnchors(CBioseq_Handle& handle,
                         const CSynonymsSet* synonyms,
                         const CSeq_align& align,
                         vector<CSeq_align::TDim>& anchors);

CSGAlignmentJob::CSGAlignmentJob(const string& desc,
                                 objects::CBioseq_Handle handle,
                                 const objects::SAnnotSelector& sel,
                                 const TSeqRange& range, TModelUnit window,
                                 TSignedSeqPos &maxStartTail, TSignedSeqPos &maxEndTail)
    : CSGAnnotJob(desc, handle, sel, range)
    , m_Window(window)
    , m_AlignLimit(-1)
    , m_SmearOverLimit(false)
    , m_LinkPair(false)
    , m_LoadCoverageGraph(true)
    , m_HideSra(CAlignmentConfig::eHide_None)
    , m_UnalignedTailsMode(CAlignmentConfig::eTails_ShowGlyph)
    , m_MaxStartTail(maxStartTail)
    , m_MaxEndTail(maxEndTail)
{
}


CSGAlignmentJob::CSGAlignmentJob(const string& desc,
                                 objects::CBioseq_Handle handle,
                                 const objects::SAnnotSelector& sel,
                                 const TSeqRange& range, TModelUnit window,
                                 int align_limit, bool smear_if_overlimit,
                                 bool link_pair,
                                 TSignedSeqPos &maxStartTail, TSignedSeqPos &maxEndTail)
    : CSGAnnotJob(desc, handle, sel, range)
    , m_Window(window)
    , m_AlignLimit(align_limit)
    , m_SmearOverLimit(smear_if_overlimit)
    , m_LinkPair(link_pair)
    , m_LoadCoverageGraph(false)
    , m_HideSra(CAlignmentConfig::eHide_None)
    , m_UnalignedTailsMode(CAlignmentConfig::eTails_ShowGlyph)
    , m_MaxStartTail(maxStartTail)
    , m_MaxEndTail(maxEndTail)
{}


CSGAlignmentJob::CSGAlignmentJob(const string& desc,
                                 objects::CBioseq_Handle handle,
                                 const objects::SAnnotSelector& sel,
                                 const TSeqRange& range,
                                 TModelUnit window,
                                 vector< CRef<CAlignGlyph> > aligns,
                                 TSignedSeqPos &maxStartTail, TSignedSeqPos &maxEndTail)
    : CSGAnnotJob(desc, handle, sel, range)
    , m_Aligns(aligns)
    , m_Window(window)
    , m_AlignLimit(-1)
    , m_SmearOverLimit(false)
    , m_LinkPair(false)
    , m_LoadCoverageGraph(false)
    , m_HideSra(CAlignmentConfig::eHide_None)
    , m_UnalignedTailsMode(CAlignmentConfig::eTails_ShowGlyph)
    , m_MaxStartTail(maxStartTail)
    , m_MaxEndTail(maxEndTail)
{}


void CSGAlignmentJob::GetAnnotNames(const objects::CBioseq_Handle& handle,
                                    const TSeqRange& range,
                                    objects::SAnnotSelector& sel,
                                    TAnnotNameTitleMap& names)
{
    sel.SetCollectNames();
    CAlign_CI aln_iter(handle, range, sel);
    ITERATE (CAlign_CI::TAnnotNames, iter, aln_iter.GetAnnotNames()) {
        if (iter->IsNamed()) {
            if (iter->GetName().find("@@") == string::npos) {
                names.insert(TAnnotNameTitleMap::value_type(iter->GetName(), ""));
            }
        } else {
            names.insert(TAnnotNameTitleMap::value_type(
                CSeqUtils::GetUnnamedAnnot(), ""));
        }
    }
}


void CSGAlignmentJob::SetSortBy(const string& sort_by)
{
    if (sort_by.empty()) return;

    size_t found = sort_by.find_first_of('|');
    string sorter_type = NStr::TruncateSpaces(sort_by.substr(0, found));
    transform(sorter_type.begin(), sorter_type.end(), sorter_type.begin(), ::tolower);
    string sort_str = kEmptyStr;
    if (found != string::npos) {
        sort_str = sort_by.substr(found + 1);
    }
  
    m_Sorter.Reset(CAlignSorterFactory::CreateAlignSorter(sorter_type, sort_str));
}

IAlnExplorer::EAlignType
CSGAlignmentJob::GetAlignType(const objects::CBioseq_Handle& handle,
                              objects::SAnnotSelector& sel,
                              bool& has_quality_map,
                              bool isFastConfig)
{
    // LOG_POST("<<<<");
    sel.SetMaxSize(1);
    sel.SetCollectNames(false);

    IAlnExplorer::EAlignType type = IAlnExplorer::fInvalid;

    //!! tmp measure! TMS will hopefully serve us alignment types later
    if(isFastConfig) {
        type = IAlnExplorer::fHomogenous;
    } else {
        // LOG_POST("Creating alignment iterator");
        CAlign_CI aln_iter(handle, TSeqRange::GetWhole(), sel);

        // LOG_POST("Creating alignment iterator done");
        if (aln_iter) {
            const CSeq_align& align = *aln_iter;
            int num_row = 0;
            try {
                num_row = align.CheckNumRows();
            } catch (CException&) {
            }

            if (num_row < 2) return type;

            CScope& scope = handle.GetScope();

            // check align type
            for (int row = 0;  row < num_row;  ++row) {
                // LOG_POST("Getting type from row " << row);
                IAlnExplorer::EAlignType this_type = IAlnExplorer::fMixed;
                CBioseq_Handle row_handle = scope.GetBioseqHandle(align.GetSeq_id(row));
                // LOG_POST("done");
                if ( !row_handle ) continue;

                switch (row_handle.GetBioseqCore()->GetInst().GetMol())
                {
                case objects::CSeq_inst::eMol_dna:
                case objects::CSeq_inst::eMol_rna:
                case objects::CSeq_inst::eMol_na:
                    this_type = IAlnExplorer::fDNA;
                    break;

                case objects::CSeq_inst::eMol_aa:
                    this_type = IAlnExplorer::fProtein;
                    break;

                default:
                    break;
                }

                if (this_type == IAlnExplorer::fMixed) {
                    type = this_type;
                    break;
                }

                if (row == 0) {
                    type = this_type;
                } else if (this_type != type) {
                    type = IAlnExplorer::fMixed;
                    break;
                }
            }
            // LOG_POST("Processing rows done");

            // check if there is quality graph
            if(!isFastConfig) {
                for (int row = num_row - 1;  row >= 0;  --row) {
                    // LOG_POST("Checking for quality graphs in row " << row);
                    // LOG_POST("Creating graph selector");
                    SAnnotSelector g_sel(CSeqUtils::GetAnnotSelector(CSeq_annot::TData::e_Graph));
                    // LOG_POST("Creating graph selector done");
                    g_sel.AddUnnamedAnnots();
                    g_sel.SetCollectNames();
                    sel.SetMaxSize(1);
                    CBioseq_Handle row_handle = scope.GetBioseqHandle(align.GetSeq_id(row));
                    if (row_handle) {
                        // LOG_POST("Creating graph iterator");
                        CGraph_CI graph_iter(row_handle, align.GetSeqRange(row), sel);
                        // LOG_POST("Creating graph iterator done");
                        if (graph_iter  &&  graph_iter.GetSize() == 1) {
                          has_quality_map = true;
                          break;
                        }
                    }
                }
                // LOG_POST("Checking for quality graphs done");
            }
        }
    }
    // LOG_POST(">>>>");

    return type;
}

bool CSGAlignmentJob::FilterDuplicatesAndBadReads(const CSeq_align& mapped_align)
{
    //    if (!mapped_align.IsSetExt() || (CAlignmentConfig::eHide_None == m_HideSra))
    //        return false;
    if (CAlignmentConfig::eHide_None == m_HideSra)
        return false;

    if (mapped_align.IsSetExt()) {
        const CSeq_align_Base::TExt& exts = mapped_align.GetExt();
        ITERATE (CSeq_align_Base::TExt, iter, exts) {
            if (!(*iter)->GetType().IsStr())
                continue;
            
            if ((CAlignmentConfig::eHide_Duplicates & m_HideSra) && ((*iter)->GetType().GetStr() == "PCR duplicate"))
                return true;
            
            if ((CAlignmentConfig::eHide_BadReads & m_HideSra) && ((*iter)->GetType().GetStr() == "Poor sequence quality"))
                return true;
        }
    }
    /*
    if (CAlignmentConfig::eHide_BadReads & m_HideSra) {
        auto align_ratio = mapped_align.AlignLengthRatio();
        if (align_ratio < 0.01)
            return true;
    }
    */
    return false;
}

static void s_ObjectIdToStr(const CObject_id& id, string& id_str)
{
    if (id.IsStr()) {
        id_str = id.GetStr();
    } else {
        id_str = NStr::NumericToString(id.GetId());
    }
}


static bool
s_CheckTraceMateAlign(bool match_id,
                      bool is_bam_align,
                      const CSeq_align& align,
                      string& ti,
                      string& mate_ti,
                      const string& expect_mate_ti = "")
{
    //
    // specific requirements for mate pair alignments:
    //

    // must be a pairwise dense-seg
    if ( !align.GetSegs().IsDenseg()  ||
        align.GetSegs().GetDenseg().GetIds().size() != 2) {
            return false;
    }
    ti = "";
    mate_ti = "";

    // there are two possible cases:
    //  1. trace assemblies that store mate pair info in seq-align::score
    //  2. cSRA/Bam short reads that store mate pair info in seq-align::ext

    // first, let's check short reads case
    if (is_bam_align) {
        if (align.IsSetExt()) {
            // exclude secondary alignments
            ITERATE (CSeq_align::TExt, iter, align.GetExt()) {
                if ((*iter)->GetType().IsStr()  &&
                    (*iter)->GetType().GetStr() == "Secondary") {
                    return false;
                }
            }
        }

        int i = 1;
        // assumming the second row is the short read
        // try second row first.
        while (ti.empty()  &&  i >=0) {
            const CSeq_id& seq_id = align.GetSeq_id(i);
            if (seq_id.IsLocal()) {
                s_ObjectIdToStr(seq_id.GetLocal(), ti);
            } else if (seq_id.IsGeneral()) {
                s_ObjectIdToStr(seq_id.GetGeneral().GetTag(), ti);
            }
            --i;
        }

        if (ti.empty()) {
            return false;
        }
    }

    if (match_id) {
        size_t len = ti.size();
        mate_ti = ti.substr(0, len - 1) + (ti[len-1] == '1' ? "2" : "1");
    } else if (align.IsSetExt()  &&  !ti.empty()) {
        ITERATE (CSeq_align::TExt, iter, align.GetExt()) {
            if ((*iter)->GetType().IsStr()  &&
                (*iter)->GetType().GetStr() == "Secondary") {
                return false;
            }
            if ((*iter)->GetType().IsStr()  &&
                (*iter)->GetType().GetStr() == "Mate read") {
                    CConstRef<CUser_field> user_field = (*iter)->GetFieldRef("lcl|");
                    if (user_field) {
                        if (user_field->GetData().IsStr()) {
                            mate_ti = user_field->GetData().GetStr();
                        } else if (user_field->GetData().IsInt()) {
                            mate_ti = NStr::NumericToString(user_field->GetData().GetInt());
                        }
                    }
            }
        }
    }

    if (is_bam_align) {
        if (mate_ti.empty()) {
            // check if it is possible to generate mate_ti based on ti
            // The ti naming pattern should be: xxxxx.[12]
            size_t len = ti.size();
            const char last_second_c = ti[len-2];
            const char last_c = ti[len-1];
            if (last_second_c == '.'  &&  (last_c == '1' ||  last_c == '2')) {
                mate_ti = ti.substr(0, len - 1) + (last_c == '1' ? "2" : "1");
            }
        }

        if (mate_ti.empty()) {
            return false;
        } else {
            return true;
        }
    }

    // then, check the IDs for a trace ID
    ITERATE (CDense_seg::TIds, iter,
        align.GetSegs().GetDenseg().GetIds()) {
            const CSeq_id& id = **iter;
            if (id.IsGeneral()  &&
                (id.GetGeneral().GetDb() == "ti"  ||
                id.GetGeneral().GetDb() == "TRACE")) {
                    if (id.GetGeneral().GetTag().IsId()) {
                        ti = NStr::IntToString(id.GetGeneral().GetTag().GetId());
                    } else {
                        ti = id.GetGeneral().GetTag().GetStr();
                    }
                    break;
            }
    }

    if (ti.empty()) {
        return false;
    }

    // must have a score field named 'matepair_ti'
    int mate_ti_int = 0;
    if ( !align.GetNamedScore("matepair ti", mate_ti_int)  &&
         !align.GetNamedScore("bad matepair ti", mate_ti_int)) {
            return false;
    }

    mate_ti = NStr::SizetToString((unsigned int)mate_ti_int);
    if (!expect_mate_ti.empty()  &&  mate_ti != expect_mate_ti) {
        return false;
    }

    return true;
}


IAppJob::EJobState CSGAlignmentJob::x_Execute()
{
    try {
        if ( !m_Aligns.empty() ) {
            CBatchJobResult* result = new CBatchJobResult();
            m_Result.Reset(result);
            result->m_Token = m_Token;
            NON_CONST_ITERATE (vector< CRef<CAlignGlyph> >, iter, m_Aligns) {
                if (IsCanceled()) {
                    return IAppJob::eCanceled;
                }
                CRef<CSGJobResult> single_res = x_LoadAlignmentFeats(**iter);
                if (single_res) {
                    result->m_Results.push_back(single_res);
                }
            }

            return eCompleted;
        } else if (m_LoadCoverageGraph) {
            return x_GetCoverageGraph();
        }

        return x_LoadAlignments();
    } catch (CException& ex) {
        m_Error.Reset(new CAppJobError(ex.GetMsg()));
        return eFailed;
    } catch (std::exception& ex) {
        m_Error.Reset(new CAppJobError(ex.what()));
        return eFailed;
    }

    return eCompleted;
}

void CSGAlignmentJob::x_UpdateMaxTails(const IAlnGraphicDataSource &aln_datasource)
{
    if (m_UnalignedTailsMode != CAlignmentConfig::eTails_ShowSequence)
        return;
    TSignedSeqPos start_tail(0);
    TSignedSeqPos end_tail(0);
    aln_datasource.GetUnalignedTails(start_tail, end_tail);
    if (start_tail > m_MaxStartTail)
        m_MaxStartTail = start_tail;
        if (end_tail > m_MaxEndTail)
            m_MaxEndTail = end_tail;
}

IAppJob::EJobState CSGAlignmentJob::x_LoadAlignments()
{
    CSGAlignJobResult* result(new CSGAlignJobResult());
    m_Result.Reset(result);
    result->m_Token = m_Token;
    IAppJob::EJobState status(eCompleted);

    // maximal number alignment allowed to load
    if (m_AlignLimit > kMaxAlignmentLoaded) {
        m_AlignLimit = kMaxAlignmentLoaded;
    }

    // upper limit used to control maximal number of alignments to load
    int upper_limit = m_AlignLimit;

    if (m_AlignLimit > -1 && m_Window <= kSequenceZoomLevel) {
        // If alignments won't be smeared even the number of alignments
        // is over the limit (!m_SmearOverLimit), and it is not in the
        // smear mode (m_AlignLimit != -1), and, and the zoom level
        // is at sequence level (m_Window <= kSequenceZoomLevel), we
        // shall load all alignments and show them.
        upper_limit = kMaxAlignmentLoaded;

        // increase the limit to accept any number of alignment
        // available in the given range.
        m_AlignLimit = upper_limit + 1;
    } else if (m_SmearOverLimit) {
        // In this case, we need to smear alignments if total number
        // is over the limit.  So we have to load all alignment. This
        // is for regular alignments only.
        upper_limit = kMaxAlignmentLoaded;
    }

    if (upper_limit <= 0) {
        return eCompleted;
    }

    CSeqGlyph::TObjects& objs = result->m_ObjectList;
    m_Sel.SetMaxSize(kMaxAlignmentLoaded);//upper_limit + 1);
    //CStopWatch sw(CStopWatch::eStart);
    CAlign_CI align_iter(m_Handle, m_Range, m_Sel);
    int obj_size = (int)align_iter.GetSize();
    if (obj_size)
        result->m_DataHandle = align_iter.GetAnnot().GetTSE_Handle();
    //ERR_POST(Error << "align_iter init  " << obj_size << " out of " << (upper_limit + 1) << " alignments in " << sw.AsSmartString(CTimeSpan::eSSP_Millisecond) << "\n");
    SetTaskTotal(obj_size);
    SetTaskCompleted(0);

    if (obj_size > m_AlignLimit) {
        if (m_SmearOverLimit) {
            // smear bar is requested when over limit
            return x_GetAlignSmear(objs, align_iter);
        }
        // see if x_GetAlignemnts can eliminate the number of objects 
        // by limiting the number of rows in icicles
        if (obj_size <= m_AlignLimit * 3) {
            x_GetAlignments(objs, align_iter, true);
            if ((int)objs.size() > m_AlignLimit) {
                objs.clear();
            }
        }
        // Otherwise, do thing. Maybe, pileup display is required in
        // this case which will be requested and done in other place
        // ELSE { RETURN NOTHING }
    } else {
        TSeqRange wholeRange(0, m_Handle.GetBioseqLength() - 1);
        bool account_for_tails = (m_UnalignedTailsMode == CAlignmentConfig::eTails_ShowSequence) && (wholeRange != m_Range)
            && ((m_MaxStartTail > 0) || (m_MaxEndTail > 0));
        if (account_for_tails) {
            // Extend the search range to accomodate the tails
            std::unique_ptr<CAlign_CI> extended_align_iter;
            TSignedSeqPos ext_from = m_Range.GetFrom() - m_MaxEndTail - 1;
            TSignedSeqPos ext_to = m_Range.GetTo() + m_MaxStartTail + 1;
            TSignedSeqPos bs_len = m_Handle.GetBioseqLength() - 1;
            TSeqRange extendedRange(ext_from > 0 ? ext_from : 0, ext_to < bs_len ? ext_to : bs_len);
            extended_align_iter.reset(new CAlign_CI(m_Handle, extendedRange, m_Sel));
            obj_size = (int)extended_align_iter->GetSize();
            status = x_GetAlignments(objs, *(extended_align_iter.get()), true, true);
        } else {
            // return all alignments, this includes the case when it
            // is at sequence level (m_AlignLimit has been adjusted)
            status = x_GetAlignments(objs, align_iter, true);
        }
    }
    SetTaskCompleted(obj_size);
    return status;
}

CRef<CSGJobResult>
CSGAlignmentJob::x_LoadAlignmentFeats(CAlignGlyph& align)
{
    CRef<CSGJobResult> result;
    CSeqGlyph::TObjects tmp;

    const IAlnGraphicDataSource& aln_mgr = align.GetAlignMgr();
    // aligned seq-id
    const CSeq_id& aligned_seq =
        aln_mgr.GetSeqId(aln_mgr.GetAnchor() == 0 ? 1 : 0);
    // anchored seq-loc
    const CSeq_loc& loc     = align.GetLocation();

    try {
        // map visible range to product feature
        CSeq_loc_Mapper seq_range_mapper(align.GetAlignment(),
            (size_t)(1 - align.GetOrigAnchor()), &GetScope());
        CRef<CSeq_loc> tmp_loc(new CSeq_loc());
        tmp_loc->SetInt().SetFrom(m_Range.GetFrom());
        tmp_loc->SetInt().SetTo  (m_Range.GetTo());
        tmp_loc->SetId(*loc.GetId());

        // our new location for feature iterator
        CRef<CSeq_loc> seq_loc(new CSeq_loc());
        CRef<CSeq_loc> mapped_loc = seq_range_mapper.Map(tmp_loc.GetObject());

        CSeq_loc::TRange mapped_range = mapped_loc->GetTotalRange();
        seq_loc->SetInt().SetFrom(mapped_range.GetFrom());
        seq_loc->SetInt().SetTo(mapped_range.GetTo());
        seq_loc->SetId(aligned_seq);

        // use newly created location to get the features
        CFeat_CI feat_iter(GetScope(), seq_loc.GetObject(), m_Sel);

        if (feat_iter.GetSize() > 0) {
            //::wxWakeUpIdle();
            CSeq_loc_Mapper mapper(
                align.GetAlignment(), (size_t)align.GetOrigAnchor(), &GetScope());

            CLinkedFeature::TLinkedFeats main_features;
            CSeqGlyph::TObjects other_features;
            for ( ;  feat_iter;  ++feat_iter) {
                if (IsCanceled()) {
                    return result;
                }
                const CMappedFeat& feat = *feat_iter;
                int subtype = feat.GetFeatSubtype();
                int type = feat.GetFeatType();
                if (CDataTrackUtils::IsGeneModelFeature(type, subtype)) {
                    CRef<CLinkedFeature> fref( new CLinkedFeature(feat) );
                    main_features.push_back(fref);
                } else {
                    CRef<CSeqGlyph> g_glyph = x_CreateFeatGlyph(mapper, feat, align);
                    if (g_glyph) {
                        other_features.push_back(g_glyph);
                    }
                }
            }

            // linking the features
            if ( !CSeqUtils::LinkFeatures(main_features,
                feature::CFeatTree::eFeatId_by_type, this)  ||
                !x_CreateGeneModels(mapper, main_features, tmp, align)) {
                return result;
            }

            std::copy(other_features.begin(), other_features.end(),
                back_inserter(tmp));
        }
    }
    catch (CAnnotMapperException&) {
        /// ignore errors from location mapping
        return result;
    }

    if ( !tmp.empty() ) {
        result.Reset(new CSGJobResult());
        result->m_ObjectList.swap(tmp);
        result->m_Token = m_Token;
        result->m_Owner = CRef<CSeqGlyph>(&align);
    }
    return result;
}


CRef<CSeqGlyph>
CSGAlignmentJob::x_CreateFeatGlyph(CSeq_loc_Mapper& mapper,
                                   const CMappedFeat& mapped_feat,
                                   CAlignGlyph& align) const
{
    CRef<CSeqGlyph> glyph;
    // mapped_feat.GetLocation() -- location of the feature on the aligned sequence
    // mapped_loc -- location of the feature on the main sequence
    CConstRef<CSeq_loc> mapped_loc = mapper.Map(mapped_feat.GetLocation());
    if (!mapped_loc->IsNull() && !mapped_loc->IsEmpty()) {
        CFeatGlyph* feat;
        bool cds = false;
        if (mapped_feat.GetData().IsCdregion()) {
            feat = new CCdsGlyph(mapped_feat, *mapped_loc);
            cds = true;
        } else {
            feat = new CFeatGlyph(mapped_feat, *mapped_loc);
        }

        if (mapped_feat.GetOriginalSeq_feat()->IsSetProduct()) {

            const IAlnGraphicDataSource& aln_mgr = align.GetAlignMgr();

            int anchor = aln_mgr.GetAnchor();
            int aligned_seq = anchor == 0 ? 1 : 0;

            CProjectedMappingInfo projected_info;
            projected_info.SetAlignmentDataSource(aln_mgr);

            const CSeq_align& orig_aln = align.GetAlignment();
            CSeq_loc_Mapper aln_mapper_up(orig_aln, aln_mgr.GetSeqId(anchor), &GetScope());
            CSeq_loc_Mapper aln_mapper_down(orig_aln, aln_mgr.GetSeqId(aligned_seq), &GetScope());
            CSeq_loc_Mapper prod_mapper(mapped_feat.GetMappedFeature(), CSeq_loc_Mapper::eLocationToProduct, &GetScope());
            AutoPtr<CSeq_loc_Mapper> prod_mapper_up(0);

            try {
                auto id = Ref(new CSeq_id);
                id->Assign(aln_mgr.GetSeqId(aligned_seq));

                // aligned_loc - location of the feature on the aligned sequence mapped through the alignments
                CRef<CSeq_loc> aligned_loc = Ref(new CSeq_loc);
                for (CSeq_loc_CI lit(mapped_feat.GetLocation()); lit; ++lit) {
                    auto anchor_loc = aln_mapper_up.Map(*lit.GetRangeAsSeq_loc());
                    if (anchor_loc->IsNull())
                        continue;
                    auto mapped_loc = aln_mapper_down.Map(*anchor_loc);
                    if (mapped_loc->IsNull())
                        continue;
                    aligned_loc->Add(*mapped_loc);
                }
                aligned_loc = aligned_loc->Merge(CSeq_loc::fMerge_OverlappingOnly | CSeq_loc::fMerge_Contained | CSeq_loc::fSort, 0);

                for (CSeq_loc_CI lit(*aligned_loc); lit; ++lit) {

                    auto aligned_segment = lit.GetRangeAsSeq_loc();
                    auto anchor_loc = aln_mapper_up.Map(*aligned_segment);
                    if (anchor_loc->IsNull()) {
                        _ASSERT(true);
                        continue;
                    }
                    auto prod_loc = prod_mapper.Map(*aligned_segment);
                    if (prod_loc->IsNull()) {
                        _ASSERT(true);
                        continue;
                    }

                    auto prod_range = prod_loc->GetTotalRange();
                    auto prod_from = prod_range.GetFrom();
                    auto prod_to = prod_range.GetTo();

                    if (cds) {
                        if (aln_mgr.GetBaseWidth(anchor) == 3) {
                            TSeqPos offset_from = 0;
                            TSeqPos offset_to = 0;
                            auto gen_loc = aln_mapper_down.Map(*anchor_loc);
                            // calculate protein range in genomic coordinates
                            // adjust coordinates to account for ending codons
                            // that borrows a base from other segments
                            // the borrowed bases are caclulated via mapping
                            CSeq_loc_CI g_l(*gen_loc);
                            int i = 0;
                            while (g_l) {
                                auto len = g_l.GetRange().GetLength();
                                if (len < 3) {
                                    if (i == 0)
                                        offset_from = len;
                                    else
                                        offset_to = len;
                                }
                                ++i;
                                ++g_l;
                            }
                            prod_from *= 3;
                            prod_to *= 3;
                            bool neg = prod_loc->GetStrand() == eNa_strand_minus;

                            if (offset_from > 0) {
                                // offset == 2 means that one base was borrowed by another segemnt
                                // therefor product coordiante should be adjusted

                                if (offset_from == 2) {
                                    if (neg) {
                                        prod_to -= 1;
                                    } else {
                                        prod_from += 1;
                                    }
                                }
                            } else {
                                if (neg)
                                    prod_from -= 2;
                            }
                            if (offset_to > 0) {
                                // offset == 1 means that this segment borrowed one base from another segemnt
                                // therefore product coordiante should be adjusted
                                if (offset_to == 1) {
                                    if (neg) {
                                        prod_from -= 1;
                                    } else {
                                        prod_to += 1;
                                    }
                                }

                            } else {
                                // if there no offset
                                // prod_to is the last base of the codon i.e. codon start + 2
                                if (!neg)
                                    prod_to += 2;
                            }
                        } else {
                            if (!prod_mapper_up)
                                prod_mapper_up.reset(new CSeq_loc_Mapper(mapped_feat.GetMappedFeature(), CSeq_loc_Mapper::eProductToLocation, &GetScope()));
                            auto prod_mapped = prod_mapper_up->Map(*prod_loc);
                            if (prod_mapped->IsNull()) {
                                _ASSERT(true);
                                continue;
                            }
                            TSeqPos offset_from = 0;
                            TSeqPos offset_to = 0;
                            auto diff_loc = prod_mapped->Subtract(*aligned_segment, 0, nullptr, nullptr);
                            CSeq_loc_CI g_l(*diff_loc);
                            while (g_l) {
                                auto len = g_l.GetRange().GetLength();
                                if (aligned_segment->GetStrand() == eNa_strand_minus) {
                                    if (g_l.GetRange().GetFrom() > aligned_segment->GetTotalRange().GetTo())
                                        offset_from = len;
                                    else if (g_l.GetRange().GetTo() < aligned_segment->GetTotalRange().GetFrom())
                                        offset_to = len;
                                } else {
                                    if (g_l.GetRange().GetTo() < aligned_segment->GetTotalRange().GetFrom())
                                        offset_from = len;
                                    else if (g_l.GetRange().GetFrom() > aligned_segment->GetTotalRange().GetTo())
                                        offset_to = len;
                                }
                                ++g_l;
                            }

                            bool neg = prod_loc->GetStrand() == eNa_strand_minus;
                            prod_from *= 3;
                            prod_to *= 3;

                            if (offset_from > 0) {
                                // offset == 2 means that one base was borrowed by another segemnt
                                // therefor product coordiante should be adjusted

                                if (offset_from == 2) {
                                    if (neg) {
                                        prod_to -= offset_from;
                                        prod_range.SetTo(prod_range.GetTo() - 1);
                                    } else {
                                        prod_from += offset_from;
                                        prod_range.SetFrom(prod_range.GetFrom() + 1);
                                    }
                                } else {
                                    if (neg) {
                                        prod_to -= offset_from;
                                    } else {
                                        prod_from += offset_from;
                                    }

                                }
                            } else {
                                if (neg)
                                    prod_from -= 2;
                            }
                            if (offset_to > 0) {
                                if (neg) {
                                    prod_from -= offset_to;
                                } else {
                                    prod_to += offset_to;
                                }
                            } else {
                                // if there no offset
                                // prod_to is the last base of the codon i.e. codon start + 2
                                if (!neg)
                                    prod_to += 2;
                            }
/*
                            prod_from *= 3;
                            prod_to *= 3;
                            auto aligned_r = aligned_segment->GetTotalRange();
                            auto mapped_r = prod_mapped->GetTotalRange();
                            int off_from = aligned_r.GetFrom() - mapped_r.GetFrom();
                            if (off_from > 0 && off_from < 3) {
                                if (off_from > 1)
                                    prod_range.SetFrom(prod_range.GetFrom() + 1);
                                prod_from += off_from;
                                prod_to += off_from;
                            }
                            int off_to = mapped_r.GetTo() - aligned_r.GetTo();
                            if (off_to > 1) {
                                prod_range.SetTo(prod_range.GetTo() + 1);
                            }
                            prod_to += off_to;
                            // need to adjust to max to include the stop codon
                            prod_to = max<int>(prod_to, prod_from + aligned_r.GetLength() - 1);
*/
                        }
                    }
                    CRef<CSeq_interval> prod_int(new CSeq_interval);
                    prod_int->SetFrom(prod_range.GetFrom());
                    prod_int->SetTo(prod_range.GetTo());
                    prod_int->SetStrand(prod_loc->GetStrand());
                    prod_int->SetId().Assign(*prod_loc->GetId());
                    prod_int->SetPartialStart(prod_loc->IsPartialStart(eExtreme_Biological), eExtreme_Biological);
                    prod_int->SetPartialStop(prod_loc->IsPartialStop(eExtreme_Biological), eExtreme_Biological);

                    CRef<CSeq_interval> gen_int(new CSeq_interval);
                    gen_int->SetFrom(aligned_segment->GetTotalRange().GetFrom());
                    gen_int->SetTo(aligned_segment->GetTotalRange().GetTo());
                    gen_int->SetStrand(aligned_segment->GetStrand());
                    gen_int->SetPartialStart(aligned_segment->IsPartialStart(eExtreme_Biological), eExtreme_Biological);
                    gen_int->SetPartialStop(aligned_segment->IsPartialStop(eExtreme_Biological), eExtreme_Biological);
                    gen_int->SetId(*id);

                    projected_info.push_back(CProjectedMappingInfo::value_type(prod_int, gen_int,
                        anchor_loc->GetTotalRange(), TSeqRange(prod_from, prod_to)));
                }
            } catch (const CException&) {

            }
            feat->SetProjectedMappingInfo(projected_info);
        }
        glyph.Reset(feat);
        CFeatGlyph::TIntervals& intervals = feat->SetIntervals();
        const CSeq_loc& loc = feat->GetLocation();
        CSeq_loc_CI iter(loc);
        for ( ;  iter;  ++iter) {
            intervals.push_back(iter.GetRange());
        }
    }
    return glyph;
}


bool
CSGAlignmentJob::x_CreateGeneModels(CSeq_loc_Mapper& mapper,
                                    CLinkedFeature::TLinkedFeats& feats,
                                    CSeqGlyph::TObjects& objs,
                                    CAlignGlyph& align) const
{
    // Create glyph for each feature, and put the linked features
    // into the right group

    CRef<CLayoutGroup> global_exon_group;
    NON_CONST_ITERATE (CLinkedFeature::TLinkedFeats, iter, feats) {
        if (IsCanceled()) return false;

        const CMappedFeat& mapped_feat = (*iter)->GetMappedFeature();
        CRef<CSeqGlyph> glyph = x_CreateFeatGlyph(mapper, mapped_feat, align);
        if ( !glyph ) continue;

        CSeqFeatData::ESubtype subtype =
            (*iter)->GetFeature().GetData().GetSubtype();
        if ((*iter)->GetChildren().empty()) {
            if (subtype == CSeqFeatData::eSubtype_exon) {
                if ( !global_exon_group ) {
                    global_exon_group.Reset(new CLayoutGroup);
                }
                global_exon_group->PushBack(glyph);
            } else {
                objs.push_back(glyph);
            }
            continue;
        }

        CRef<CLayoutGroup> exon_group;
        CRef<CLayoutGroup> gene_group;
        gene_group.Reset(new CGeneGroup());
        objs.push_back(CRef<CSeqGlyph>(gene_group.GetPointer()));
        gene_group->PushBack(glyph);
        NON_CONST_ITERATE (CLinkedFeature::TLinkedFeats, c_iter, (*iter)->GetChildren()) {
            const CMappedFeat& c_mapped_feat = (*c_iter)->GetMappedFeature();
            CRef<CSeqGlyph> c_glyph = x_CreateFeatGlyph(mapper, c_mapped_feat, align);
            if ( !c_glyph ) continue;

            subtype = (*c_iter)->GetFeature().GetData().GetSubtype();
            if (subtype == CSeqFeatData::eSubtype_exon) {
                if ( !exon_group ) {
                    exon_group.Reset(new CLayoutGroup);
                }
                exon_group->PushBack(c_glyph);
            } else {
                gene_group->PushBack(c_glyph);
                NON_CONST_ITERATE (CLinkedFeature::TLinkedFeats, g_iter, (*c_iter)->GetChildren()) {
                    const CMappedFeat& g_mapped_feat = (*g_iter)->GetMappedFeature();
                    CRef<CSeqGlyph> g_glyph = x_CreateFeatGlyph(mapper, g_mapped_feat, align);
                    if (g_glyph) {
                        gene_group->PushBack(g_glyph);
                    }
                }
            }
        }
        if (exon_group) {
            if (exon_group->GetChildren().size() == 1) {
                gene_group->Append(exon_group->SetChildren());
            } else {
                gene_group->PushBack(exon_group.GetPointer());
            }
        }
    }
    if (global_exon_group) {
        if (objs.size() == 1  &&  dynamic_cast<CGeneGroup*>(objs.front().GetPointer())) {
            CGeneGroup* group = dynamic_cast<CGeneGroup*>(objs.front().GetPointer());
            if (global_exon_group->GetChildren().size() == 1) {
                group->Append(global_exon_group->SetChildren());
            } else {
                group->PushBack(global_exon_group.GetPointer());
            }
        } else {
            if (global_exon_group->GetChildren().size() == 1) {
                objs.push_back(global_exon_group->SetChildren().front());
            } else {
                objs.push_back(CRef<CSeqGlyph>(global_exon_group.GetPointer()));
            }
        }
    }

    return true;
}


IAppJob::EJobState
CSGAlignmentJob::x_GetAlignSmear(CSeqGlyph::TObjects& objs,
                                 CAlign_CI& align_iter)
{
    if (align_iter.GetSize() == 0) return eCompleted;

    CRef<CAlignSmearGlyph> smear_glyph;

    const CSeq_annot& seq_annot_first =
        *align_iter.GetSeq_align_Handle().GetAnnot().GetCompleteSeq_annot();
    if (CAlignmentSmear::SeparateStrands(seq_annot_first)) {
        // positive strand
        smear_glyph.Reset(new CAlignSmearGlyph(m_Handle, m_Range.GetFrom(),
        m_Range.GetTo(), m_Window, CAlignmentSmear::eSmearStrand_Pos));
        if (smear_glyph) {
            CAlignmentSmear& smear = smear_glyph->GetAlignSmear();
            if ( !smear.AddAlignments(align_iter, this) ) {
                return eCanceled;
            }
            objs.emplace_back(smear_glyph.GetPointer());
        }

        // negative strand
        smear_glyph.Reset(new CAlignSmearGlyph(m_Handle, m_Range.GetFrom(),
        m_Range.GetTo(), m_Window, CAlignmentSmear::eSmearStrand_Neg));
        if (smear_glyph) {
            CAlignmentSmear& smear = smear_glyph->GetAlignSmear();
            if ( !smear.AddAlignments(align_iter, this)) {
                return eCanceled;
            }
            objs.push_back(CRef<CSeqGlyph>(smear_glyph.GetPointer()));
        }
    } else {
        smear_glyph.Reset(new CAlignSmearGlyph(m_Handle, m_Range.GetFrom(),
        m_Range.GetTo(), m_Window, CAlignmentSmear::eSmearStrand_Both));
        if (smear_glyph) {
            CAlignmentSmear& smear = smear_glyph->GetAlignSmear();
            if ( !smear.AddAlignments(align_iter, this) ) {
                return eCanceled;
            }
            objs.emplace_back(smear_glyph.GetPointer());
        }
    }

    return eCompleted;
}

static bool s_IsMatchById(CConstRef<CSeq_annot> aln_annot)
{
    bool match_id = false;

    if (aln_annot  &&  aln_annot->CanGetDesc()) {
        ITERATE(objects::CAnnot_descr::Tdata, descrIter, aln_annot->GetDesc().Get())
            if ((*descrIter)->IsUser() &&
                (*descrIter)->GetUser().GetType().IsStr() &&
                (*descrIter)->GetUser().GetType().GetStr() == "Mate read") {
                CConstRef<CUser_field> user_field =
                    (*descrIter)->GetUser().GetFieldRef("Match by local Seq-id");
                if (user_field  &&  user_field->GetData().IsBool() &&
                    user_field->GetData().GetBool()) {
                    match_id = true;
                    break;
                }
            }
    }
    return match_id;
}


IAppJob::EJobState
CSGAlignmentJob::x_GetAlignments(CSeqGlyph::TObjects& objs,
CAlign_CI& align_iter,
bool sparseAln,
bool filterResults)
{
    if (align_iter.GetSize() == 0) return eCompleted;

    SetTaskName("Creating alignment glyphs...");

    typedef pair<CRef<CAlignGlyph>, string> TMatedAlign;
    typedef map<string, TMatedAlign> TMatedAlignments;
    TMatedAlignments mated_aligns;
    bool match_id = false;

    CSGAlignmentJob::EAlignFlags flag = CSGAlignmentJob::fAlign_Default;
    if (m_LinkPair && m_Window < kDetailsZoomLevel) {
        flag = CSGAlignmentJob::fAlign_LinkMatePairs;
        CConstRef<CSeq_annot> aln_annot =
            align_iter.GetSeq_align_Handle().GetAnnot().GetCompleteSeq_annot();
        match_id = s_IsMatchById(aln_annot);
    }
    auto synonyms = m_Handle.GetSynonyms();

    for (; align_iter; ++align_iter) {
        if (IsCanceled())
            return eCanceled;
        const CSeq_align_Handle& align_handle =
            align_iter.GetSeq_align_Handle();
        const CSeq_align& align = *align_iter;

        if (FilterDuplicatesAndBadReads(align)) {
            AddTaskCompleted(1);
            continue;
        }
        vector<CSeq_align::TDim> anchors;
        s_GetAnchors(m_Handle, synonyms.GetPointer(), align, anchors);

        TAlnDataSources aln_datasources;
        bool retry = false;
        try {
            // try using both direction
            s_BuildAlignDataSource(m_Handle, anchors, align, sparseAln, m_Window, aln_datasources);
        } catch (CAlignRangeCollException& e) {
            if (sparseAln) {
                // try to build without sparseAln flag
                sparseAln = false;
                retry = true;
            } else {
                LOG_POST(Warning << "error in GetAlignments(): " << e.GetMsg());
            }
        } catch (CException& e) {
            // log errors
            LOG_POST(Warning << "error in GetAlignments(): " << e.GetMsg());
        }
        if (retry) {
            try {
                // try using both direction
                s_BuildAlignDataSource(m_Handle, anchors, align, false, m_Window, aln_datasources);
            } catch (CException& e) {
                // log errors
                LOG_POST(Warning << "error in GetAlignments(): " << e.GetMsg());
            }
        }

        for (size_t i = 0; i < aln_datasources.size(); ++i) {
            int anchor = aln_datasources[i].second;
            CConstRef<IAlnGraphicDataSource>& aln_datasource =
                aln_datasources[i].first;

            if (filterResults) { // Filter results in case extended range is used
                TSeqRange aln_range = aln_datasource->GetAlnRangeWithTails();
                if (!aln_range.IntersectingWith(m_Range)) {
                    continue;
                }
            }
            x_UpdateMaxTails(*aln_datasource);
            //
            // check to see if this is a mate pair alignment
            //
            auto align_glyph = Ref(new CAlignGlyph(aln_datasource, align_handle, align, anchor));
            objs.emplace_back(align_glyph.GetPointer());
            if (flag & fAlign_LinkMatePairs) {
                string ti;
                string mate_ti;
                if (s_CheckTraceMateAlign(match_id, match_id, align, ti, mate_ti)) {
                    // we will process mate mair alignments in a separate pass
                    // after we complete all other alignments
                    //TMatedAlign mp(objs.pwal, mate_ti);
                    mated_aligns.emplace(ti, make_pair(align_glyph, mate_ti));
                }
            }
        }
        AddTaskCompleted(1);
    }
    set<CSeqGlyph*> objs_to_remove;
    CSeqGlyph::TObjects mated_objs;
    //
    // final pass - connect our mate pair alignments, if we can
    //
    NON_CONST_ITERATE(TMatedAlignments, iter, mated_aligns)
    {
        if (IsCanceled()) {
            return eCanceled;
        }

        if (!iter->second.first) {
            continue;
        }

        CAlignGlyph& first_mate = *iter->second.first;
        const CSeq_align& align = first_mate.GetAlignment();
        const CSeq_align_Handle& orig_align = first_mate.GetOrigAlignment();
        const string& ti = iter->first;
        string mate_ti = iter->second.second;

        //
        // verify that we have a mated alignment for this trace alignment
        //
        CRef<CAlignGlyph> second_mate;
        TMatedAlignments::iterator mate_iter = mated_aligns.find(mate_ti);
        if (mate_iter == mated_aligns.end()) {
            //
            // NOT FOUND
            // try iterating. We do this only for trace assembly, not for
            // other cases like cSRA/BAM short reads.
            //
            CSeq_id id("gnl|ti|" + mate_ti);
            CBioseq_Handle h = m_Handle.GetScope().GetBioseqHandle(id);
            if (h) {
                SAnnotSelector sel =
                    CSeqUtils::GetAnnotSelector(CSeq_annot::TData::e_Align);
                CAlign_CI second_mate_iter(h, sel);
                for (; second_mate_iter; ++second_mate_iter) {
                    string temp = "";
                    if (!s_CheckTraceMateAlign(false, false, *second_mate_iter,
                        temp, mate_ti, ti)) {
                        continue;
                    }

                    try {
                        // create CAlnGraphicDataSource
                        TAlnDataSources aln_datasources;
                        vector<CSeq_align::TDim> anchors;
                        s_GetAnchors(m_Handle, synonyms.GetPointer(), align, anchors);

                        s_BuildAlignDataSource(m_Handle, anchors, align, sparseAln, m_Window, aln_datasources);
                        if (!aln_datasources.empty()) {
                            x_UpdateMaxTails(*(aln_datasources.front().first));
                            second_mate.Reset(new CAlignGlyph(
                                aln_datasources.front().first, orig_align, align,
                                first_mate.GetOrigAnchor()));
                        }
                    } catch (CException& e) {
                        LOG_POST(Warning << "error in GetAlignments(): " << e.GetMsg());
                    }
                    break;
                }
            }

            if (!second_mate) {
                //LOG_POST(Warning << "failed to find mate pair for ti = " << ti);
                // no mate available, so treat it as a normal alignment
                //                objs.emplace_back(&first_mate);
                continue;
            }
        } else if (!mate_iter->second.first) {
            // already processed
            continue;
        } else {
            second_mate = mate_iter->second.first;
            mate_iter->second.first.Reset();
        }

        CMatePairGlyph::TAlignList als;
        als.emplace_back(&first_mate);
        als.emplace_back(second_mate.GetPointer());
        objs_to_remove.insert({ &first_mate, second_mate.GetPointer() });
        mated_objs.emplace_back(new CMatePairGlyph(als));
    }
    if (!objs_to_remove.empty()) {
        auto remove_begin = remove_if(objs.begin(), objs.end(), [&objs_to_remove](CRef<CSeqGlyph>& glyph) {
            return objs_to_remove.count(glyph.GetPointer()) > 0;
        });
        objs.erase(remove_begin, objs.end());
        move(mated_objs.begin(), mated_objs.end(), back_inserter(objs));
    }

    if (m_Sorter  &&  mated_aligns.empty()) {
        // we don't sort alignments if they are paired and linked together
        typedef map<int, CSeqGlyph::TObjects> TAlignGroupMap;
        TAlignGroupMap sorted_aligns;
        NON_CONST_ITERATE(CSeqGlyph::TObjects, iter, objs)
        {
            CAlignGlyph* align_glyph = dynamic_cast<CAlignGlyph*>(iter->GetPointer());
            _ASSERT(align_glyph);
            int idx = m_Sorter->GetGroupIdx(align_glyph->GetAlignment());
            if (idx > -1) {
                sorted_aligns[idx].push_back(*iter);
            }
        }
        if (sorted_aligns.size() > 1) {
            objs.clear();
            map<string, int> sorted_groups;
            for (auto const& group : sorted_aligns) {
                sorted_groups[m_Sorter->GroupIdxToName(group.first)] = group.first;
            }
            for (auto const& group : sorted_groups) {
                CRef<CNamedGroup> named_group(new CNamedGroup(group.first));
                named_group->Set(sorted_aligns[group.second]);
                objs.emplace_back(named_group.GetPointer());
            }
        }
    }

    if (m_LayoutPolicy != 0) {
        for (auto& o : objs) {
            TSeqRange range = o->GetRange();
            o->SetWidth(range.GetLength());
            o->SetLeft(range.GetFrom());
            o->SetHeight(1);
        }
        CLayoutGroup group;
        group.SetChildren().swap(objs);
        group.SetTearline(eMaxRowLimit);
        ILayoutPolicy::SBoundingBox bound;
        m_LayoutPolicy->BuildLayout(group, bound);
        group.SetChildren().swap(objs);
        auto remove_begin = remove_if(objs.begin(), objs.end(), [](CRef<CSeqGlyph>& glyph) {
            return !glyph->GetVisible();
        });
        objs.erase(remove_begin, objs.end());
    }
    SetTaskCompleted(objs.size());
    return eCompleted;
}


IAppJob::EJobState
CSGAlignmentJob::x_GetCoverageGraph()
{
    CSGJobResult* result = new CSGJobResult();
    m_Result.Reset(result);
    SetTaskName("Loading coverate graph...");

    CSeqGlyph::TObjects glyphs;
    // Limit to just one coverage graph GB-7336
    // in case both generated by object manager are added by the user
    // are in the scope
    auto sel = m_Sel;
    sel.SetMaxSize(1);
    EJobState state = x_CreateHistFromGraph(glyphs, sel, m_Window, true, TAxisLimits());
    if (state == eCompleted) {
        result->m_ObjectList.swap(glyphs);
    }

    result->m_Token = m_Token;

    return state;
}

///////////////////////////////////////////////////////////////////////////////
/// CSGAlignScoringJob
///////////////////////////////////////////////////////////////////////////////
CSGAlignScoringJob::CSGAlignScoringJob(const string& desc,
                                       const TAlnScoreMap& scores)
    : CSeqGraphicJob(desc)
    , m_AlnScoreMap(scores)
{
    SetTaskName("Calculating alignment scores...");
}


IAppJob::EJobState CSGAlignScoringJob::x_Execute()
{
    CJobResultBase* result = new CJobResultBase();
    m_Result.Reset(result);

    SetTaskTotal((int)m_AlnScoreMap.size());
    SetTaskCompleted(0);

    NON_CONST_ITERATE (TAlnScoreMap, iter, m_AlnScoreMap) {
        if (IsCanceled()) {
            return IAppJob::eCanceled;
        }
        CIRef<ISGAlnScore> score = iter->second;
        if ( !score->HasScores() ) {
            score->CalculateScores();
        }
        AddTaskCompleted(1);
    }
    result->m_Token = m_Token;
    return IAppJob::eCompleted;
}


///////////////////////////////////////////////////////////////////////////////
/// CSGAlignStatJob
CSGAlignStatJob::CSGAlignStatJob(const string& desc,
                                 objects::CBioseq_Handle handle,
                                 const objects::SAnnotSelector& aln_sel,
                                 const objects::SAnnotSelector& graph_sel,
                                 const TSeqRange& range,
                                 const TAlnMgrVec& aligns,
                                 TModelUnit scale,
                                 const string& cache_key)
    : CSGAnnotJob(desc, handle, aln_sel, range)
    , m_GraphSel(graph_sel)
    , m_Aligns(aligns)
    , m_Window(scale)
    , m_CacheKey(cache_key)
{
    SetTaskName("Loading alignment pileup ...");
}


/// layout style to layout display name
typedef SStaticPair<const char*, CAlnStatConfig::EStatType> TGraphTitle;
static const TGraphTitle s_GraphTitles[] = {
    {"Number of A bases",  CAlnStatConfig::eStat_A},
    {"Number of C bases",  CAlnStatConfig::eStat_C},
    {"Number of G bases",  CAlnStatConfig::eStat_G},
    {"Number of T bases",  CAlnStatConfig::eStat_T},
    {"Number of inserts",  CAlnStatConfig::eStat_Gap},
    {"Number of introns",  CAlnStatConfig::eStat_Intron },
    {"Number of matches",  CAlnStatConfig::eStat_Match}
};

typedef CStaticArrayMap<string, CAlnStatConfig::EStatType> TGraphTitleMap;
DEFINE_STATIC_ARRAY_MAP(TGraphTitleMap, sm_GraphTitleMap, s_GraphTitles);

size_t s_GraphTitleToIndex(const string& title)
{
    TGraphTitleMap::const_iterator iter = sm_GraphTitleMap.find(title);
    if (iter != sm_GraphTitleMap.end()) {
        return (size_t)iter->second;
    }
    return -1;
}


const string& s_GraphIndexToTitle(size_t idx)
{
    TGraphTitleMap::const_iterator iter;
    for (iter = sm_GraphTitleMap.begin();  iter != sm_GraphTitleMap.end();  ++iter) {
        if ((size_t)iter->second == idx) {
            return iter->first;
        }
    }
    _ASSERT(false); // shouldn't get to here
    return kEmptyStr;
}


void CSGAlignStatJob::x_CreatePileUpGraphs(TSeqRange& range, TGraphs& graphs_out)
{
    graphs_out.clear();
    if (m_Aligns.empty())
        return;
    SetTaskName("Creating pileup graphs ...");
    SetTaskTotal((int)m_Aligns.size());
    SetTaskCompleted(0);

    // initialize the top sequence to generate statistics for every base
    vector<unsigned char> ref_bases;
    size_t ref_len = 0;
    TSeqPos start = m_Range.GetFrom();
    TSeqPos stop = m_Range.GetToOpen();
    // graphs holder created from the loaded seq-aligns.
    {
        string seq_str;
        CSeqVector seq_vector =
            m_Handle.GetSeqVector(CBioseq_Handle::eCoding_Iupac);
        seq_vector.GetSeqData(start, stop, seq_str);
        ref_len = seq_str.length();
        if (ref_len < stop - start) {
            stop = start + ref_len;
        }
        ref_bases.resize(ref_len);
        for (size_t i = 0; i < ref_len; ++i) {
            ref_bases[i] = (unsigned char)CAlnStatConfig::GetRefSeqIdex(seq_str[i]);
        }
    }

    CRef<CSeq_loc> loc(new CSeq_loc());
    loc->SetInt().SetFrom(start);
    loc->SetInt().SetTo(stop - 1);
    range.Set(start, stop - 1);
    CConstRef<CSeq_id> seq_id = m_Handle.GetSeqId();
    loc->SetId(*seq_id);

    vector<CRef<CSeq_graph>> graphs;
    graphs.resize(CAlnStatConfig::eStat_Match + 1);

    for (int i = 0; i <= CAlnStatConfig::eStat_Match; ++i) {
        CRef<CSeq_graph> g(new CSeq_graph);
        g->SetTitle(s_GraphIndexToTitle((size_t)i));
        g->SetLoc(*loc);
        g->SetGraph().SetInt().SetValues().resize(ref_bases.size(), 0);
        graphs[i] = g;
    }

    string aln_seq; // variable used in nested loops down here, brought up decl. for better heap alloc.performance
    ITERATE(TAlnMgrVec, iter, m_Aligns)
    {
        if (IsCanceled())
            return;
        AddTaskCompleted(1);

        const IAlnGraphicDataSource& align = **iter;

        if (align.GetAlignType() != IAlnExplorer::fDNA) {
            // If it is not a DNA-to-DNA alignment, skip it
            continue;
        }

        auto row_n = align.GetNumRows();
        auto anchor = align.GetAnchor();
        TSeqPos anchor_start = align.GetSeqStart(anchor);
        TSeqPos anchor_stop = align.GetSeqStop(anchor);
        auto base_width_anchor = align.GetBaseWidth(anchor);
        anchor_start = max(anchor_start, start);
        anchor_stop = min(anchor_stop, stop - 1);
        if ((*iter)->IsSimplified()) {
            // No need to look at match/mismatch.
            // Only count overall coverage.
            vector<int>& match_vals =
                graphs[CAlnStatConfig::eStat_Match]->SetGraph().SetInt().SetValues();

            if (anchor_start < start)  anchor_start = start;
            if (anchor_stop > stop) anchor_stop = stop;
            while (anchor_start <= anchor_stop) {
                size_t curr_pos = anchor_start - start;
                vector<int>& ref_vals =
                    graphs[ref_bases[curr_pos]]->SetGraph().SetInt().SetValues();
                ref_vals[curr_pos]++;
                match_vals[curr_pos]++;
                anchor_start++;
            }
            continue;
        }
        TSignedSeqPos aln_r_s = align.GetAlnPosFromSeqPos(anchor, anchor_start * base_width_anchor, IAlnExplorer::eRight);
        TSignedSeqPos aln_r_e = align.GetAlnPosFromSeqPos(anchor, anchor_stop * base_width_anchor, IAlnExplorer::eLeft);
        if (aln_r_e < aln_r_s)
            swap(aln_r_s, aln_r_e);
        TSignedSeqRange aln_range(aln_r_s, aln_r_e);
        //TSignedSeqRange aln_range(align.GetAlnPosFromSeqPos(anchor, anchor_start, IAlnExplorer::eRight),
          //  align.GetAlnPosFromSeqPos(anchor, anchor_stop, IAlnExplorer::eLeft));

        for (TNumrow r = 0; r < row_n; r++) {
            if (r == anchor) 
                continue;

            auto base_width_row = align.GetBaseWidth(r);

            auto_ptr<IAlnSegmentIterator> p_it(
                align.CreateSegmentIterator(r,
                aln_range, IAlnSegmentIterator::eAllSegments));

            for (IAlnSegmentIterator& it = *p_it; it; ++it) {
                const IAlnSegment& seg = *it;
                if ((seg.GetType() & IAlnSegment::fIndel  &&
                    !seg.GetRange().Empty()) ||
                    seg.GetType() & IAlnSegment::fUnaligned ||
                    seg.GetType() & IAlnSegment::fGap) {
                    // ignore the inserts
                    continue;
                }
                const IAlnSegment::TSignedRange& curr_aln_r = seg.GetAlnRange();
                if (curr_aln_r.Empty())
                    continue;
//              TSignedSeqPos curr_aln_start = curr_aln_r.GetFrom();
                TSeqPos seg_len = (TSeqPos)curr_aln_r.GetLength();
                TModelUnit curr_seq_from =
                    align.GetSeqPosFromAlnPos(anchor, curr_aln_r.GetFrom()) / base_width_anchor;
                TModelUnit curr_seq_to =
                    align.GetSeqPosFromAlnPos(anchor, curr_aln_r.GetTo()) / base_width_anchor;
                if (curr_seq_from > curr_seq_to) 
                    swap(curr_seq_from, curr_seq_to);
//                TSeqPos curr_seq_from =
  //                  (TSeqPos)align.GetSeqPosFromAlnPos(anchor, curr_aln_start, IAlnExplorer::eRight);
                TSeqPos off = 0;
                size_t pos = 0;
                if (curr_seq_from < start) {
                    pos = start - curr_seq_from;
                } else {
                    off = curr_seq_from - start;
                }
                size_t curr_pos = off;
                if (seg.GetType() & IAlnSegment::fAligned) {
                    aln_seq.clear();

                    TModelUnit r_seq_from =
                        align.GetSeqPosFromAlnPos(r, curr_aln_r.GetFrom()) / base_width_row;
                    TModelUnit r_seq_to =
                        align.GetSeqPosFromAlnPos(r, curr_aln_r.GetTo()) / base_width_row;
                        
                    align.GetSeqString(aln_seq, r, IAlnSegment::TSignedRange(r_seq_from, r_seq_to), curr_aln_r, false);
                        
                    bool reverse = align.IsPositiveStrand(anchor) != align.IsPositiveStrand(r);
                    if (reverse) {
                        string tmp_seq;
                        CSeqManip::ReverseComplement(aln_seq, CSeqUtil::e_Iupacna,
                            0, aln_seq.length(), tmp_seq);
                        swap(tmp_seq, aln_seq);
                    }
                }
                while (pos < seg_len  &&  curr_pos < ref_len) {
                    if (IsCanceled())
                        return;

                    if (seg.GetType() & IAlnSegment::fAligned) {
                        if (pos < aln_seq.size()) {
                            int base_type = CAlnStatConfig::GetRefSeqIdex(aln_seq[pos]);
                            vector<int>& vals =
                                graphs[base_type]->SetGraph().SetInt().SetValues();
                            vals[curr_pos] += 1;
                            if ((CAlnStatConfig::eStat_Gap != base_type) &&
                                (CAlnStatConfig::eStat_Intron != base_type) &&
                                (base_type == ref_bases[curr_pos])) {
                                vector<int>& m_vals =
                                    graphs[CAlnStatConfig::eStat_Match]->SetGraph().SetInt().SetValues();
                                m_vals[curr_pos] += 1;
                            }
                        } else {
                            // It shouldn't get to here
                            _ASSERT(false);
                        }
                    } else {
                        if (pos < aln_seq.size()) {
                            int base_type = CAlnStatConfig::GetRefSeqIdex(aln_seq[pos]);
                            if (CAlnStatConfig::eStat_Gap == base_type) {
                                // gap
                                vector<int>& vals = graphs[CAlnStatConfig::eStat_Gap]->SetGraph().SetInt().SetValues();
                                vals[curr_pos] += 1;
                            }
                            else if (CAlnStatConfig::eStat_Intron == base_type) {
                                // intron
                                vector<int>& vals = graphs[CAlnStatConfig::eStat_Intron]->SetGraph().SetInt().SetValues();
                                vals[curr_pos] += 1;
                            }
                        }
                    }
                    ++pos;
                    ++curr_pos;
                }
            }
        }
    }
    for (const auto& gr : graphs) {
        graphs_out.emplace_back(gr.GetPointer());
    }
}

class CGraphStatCollector_Graph : public CGraphStatCollector
{
public:    
    CGraphStatCollector_Graph(double window, CPileUpGraph& data)
        : CGraphStatCollector(window)
        , m_Data(data)
    {
    }
    virtual void NewGroup(int batch_size) override
    {
        matches.clear();
        matches.reserve(batch_size);
        mismatches.clear();
        mismatches.reserve(batch_size);
        gaps.clear();
        gaps.reserve(batch_size);
        introns.clear();
        introns.reserve(batch_size);
    }
    virtual void AddStat(size_t index, CAlnStatGlyph::SStatStruct& stat) override
    {
        auto& data = stat.m_Data;
        int agtc = data[CAlnStatConfig::eStat_A]
            + data[CAlnStatConfig::eStat_G]
            + data[CAlnStatConfig::eStat_T]
            + data[CAlnStatConfig::eStat_C];
        data[CAlnStatConfig::eStat_Mismatch] = agtc - data[CAlnStatConfig::eStat_Match];
        matches.push_back(data[CAlnStatConfig::eStat_Match]);
        mismatches.push_back(data[CAlnStatConfig::eStat_Mismatch]);
        gaps.push_back(data[CAlnStatConfig::eStat_Gap]);
        introns.push_back(data[CAlnStatConfig::eStat_Intron]);
    }
    virtual void UpdateGroup(size_t offset) override {
        CPileUpGraph::TUpdateMap update = {
            { "m", &matches },
            { "mm", &mismatches },
            { "g", &gaps },
            { "n", &introns }
        };
        m_Data.Update(TSeqRange(offset, offset + matches.size() - 1), update);
    }
    virtual void ClearRange(const TSeqRange& range) override {
        matches.clear();
        matches.resize(range.GetLength(), 0);
        mismatches.clear();
        mismatches.resize(range.GetLength(), 0);
        gaps.clear();
        gaps.resize(range.GetLength(), 0);
        introns.clear();
        introns.resize(range.GetLength(), 0);
        UpdateGroup(range.GetFrom());
    }
    virtual bool ComputePileUp() override { return false; }

private:
    CPileUpGraph& m_Data;
    vector<CPileUpGraph::TValType> matches;
    vector<CPileUpGraph::TValType> mismatches;
    vector<CPileUpGraph::TValType> gaps;
    vector<CPileUpGraph::TValType> introns;
};

class CGraphStatCollector_Glyph : public CGraphStatCollector
{
public:
    CGraphStatCollector_Glyph(double window, CAlnStatGlyph::TStatVec& stats)
        : CGraphStatCollector(window)
        , m_Stats(stats)
        
    {
    }
    virtual void AddStat(size_t index, CAlnStatGlyph::SStatStruct& stat) override
    {
        if (m_LastIndex != (int)index) {
            m_LastIndex = index;
            m_MaxCount = -1;
        }
        //        stat.m_Data[CAlnStatConfig::eStat_Gap] = 0; //.. if we want to exclude gaps
        int agtc = stat.m_Data[CAlnStatConfig::eStat_A] +
            stat.m_Data[CAlnStatConfig::eStat_G] +
            stat.m_Data[CAlnStatConfig::eStat_T] +
            stat.m_Data[CAlnStatConfig::eStat_C];
        stat.m_Data[CAlnStatConfig::eStat_Mismatch] =
            agtc - stat.m_Data[CAlnStatConfig::eStat_Match];
        stat.m_Data[CAlnStatConfig::eStat_Total] =
            stat.m_Data[CAlnStatConfig::eStat_Match]
            + stat.m_Data[CAlnStatConfig::eStat_Mismatch]
            + stat.m_Data[CAlnStatConfig::eStat_Gap]
            + stat.m_Data[CAlnStatConfig::eStat_Intron];

        int curr_max = max(stat.m_Data[CAlnStatConfig::eStat_Gap],
                           stat.m_Data[CAlnStatConfig::eStat_Mismatch]);
        if (curr_max < stat.m_Data[CAlnStatConfig::eStat_Intron]) {
            curr_max = stat.m_Data[CAlnStatConfig::eStat_Intron];
        }
        if (m_MaxCount < curr_max) {
            m_Stats[index] = stat;
            m_MaxCount = curr_max;
        }
    }

private:
    CAlnStatGlyph::TStatVec& m_Stats;
    int m_MaxCount = -1;
    int m_LastIndex = -1;
};

CSGAlignStatJob::EJobState CSGAlignStatJob::x_CollectAlignStats(const TSeqRange& main_range, CTSE_Handle& tse_handle, CGraphStatCollector& stat_collector)
{
    TSeqPos start = main_range.GetFrom();
    TSeqPos stop = main_range.GetToOpen();
    size_t val_num = (size_t)(stop - start);
    bool match_graph = false;
    // try to see if there is pileup graphs available
    SetTaskName("Loading pileup graphs ...");

    map<TSeqRange, TGraphs> groups;
    //CStopWatch sw(CStopWatch::eStart);
    CGraph_CI graph_iter(m_Handle, main_range, m_GraphSel);
    //ERR_POST(Error << "graph_iter init  in " << sw.AsSmartString(CTimeSpan::eSSP_Millisecond) << "\n");
    if (graph_iter.GetSize()) {
        tse_handle = graph_iter.GetAnnot().GetTSE_Handle();
        // The graphs are stored in the order of CAlnStatConfig::EStatType
        // which is A, G, T, C, Gap, and Match
        //TGraphVec graphs;

        for (; graph_iter; ++graph_iter) {
            size_t g_idx = s_GraphTitleToIndex(graph_iter->GetTitle());
            if (g_idx == (size_t)-1)
                continue;
            const auto& gr = graph_iter->GetMappedGraph();
            const auto& range = gr.GetLoc().GetTotalRange();
            auto it = groups.find(range);
            if (it == groups.end()) 
                it = groups.emplace(range, TGraphs(CAlnStatConfig::eStat_Match + 1)).first;
            it->second[g_idx] = &gr;
            if (CAlnStatConfig::eStat_Match == g_idx) 
                match_graph = true;
        }
    }
    stat_collector.ClearRange(main_range);
    if (groups.empty()) {
        if (!stat_collector.ComputePileUp()) 
            return eCompleted;

        // no pileup graph is available, try computing pileup on the fly
        if (m_Aligns.empty()) {
            // make sure the alignments are loaded
            SetTaskName("Loading alignments ...");
            EJobState status = x_LoadAlignments(tse_handle);
            if ( status != eCompleted)
                return status;
        }
        if (m_Aligns.empty())
            return eCompleted;
        TSeqRange range;
        TGraphs graphs;
        x_CreatePileUpGraphs(range, graphs);
        if (!graphs.empty())
            groups.emplace(range, graphs);
        match_graph = true;
    }
    if (groups.empty())
        return eCompleted;
    if (IsCanceled())
        return eCanceled;
    // We don't store more than one set of statistics for each screen pixel.
    // If the zoom scale is larger than one nucleotide per pixel, then we
    // need to squeeze several sets of statistics (one per base) into one (pixel)

    double window = stat_collector.GetWindow();
    // resize the stats to the actual size needed
    if (window < 1.0) window = 1.0;
    size_t pix_num = (size_t)ceil(val_num / window);
    auto l_GetVal = [](const CSeq_graph::TGraph& graph, int curr_idx) {
        int val = 0;
        if (graph.IsInt()) {
            const vector<int>& vals = graph.GetInt().GetValues();
            val = vals[curr_idx];
        } else { // must be byte
            const vector<char>& vals = graph.GetByte().GetValues();
            val = (int)(unsigned char)(vals[curr_idx]);
        }
        return val;
    };
    CSeqVector seq_vector = m_Handle.GetSeqVector(CBioseq_Handle::eCoding_Iupac);

    size_t curr_start, curr_idx, num_vals, ref_idx, stat_idx, end_idx;
    int graph_off, offset;
    double curr_pos;
    string ref_seq_str;
    for (const auto& group : groups) {
        const auto& range = group.first;
        const auto& graphs = group.second;
        curr_start = max<int>(main_range.GetFrom(), range.GetFrom());
        curr_idx = curr_start - range.GetFrom();
        num_vals = range.GetLength();
        stat_collector.NewGroup(num_vals);
        // reference sequence
        seq_vector.GetSeqData(curr_start, curr_start + num_vals, ref_seq_str);
        ref_idx = 0;
        graph_off = (int)range.GetFrom() - (int)main_range.GetFrom();
        curr_pos = (double)(curr_start - main_range.GetFrom());
        stat_idx = (size_t)floor(curr_pos / window);
        offset = main_range.GetFrom() + stat_idx;

        for (; stat_idx < pix_num && curr_idx < num_vals; ++stat_idx) {

            if (IsCanceled())
                return eCanceled;
            curr_pos += window;
            end_idx = min<int>(floor(curr_pos + 0.5) - graph_off, num_vals);

            // choose one set of stats from [start_idx, end_idx) to
            // represent current pixel (i).  The criteria are
            // like this: max(max(gap(n), mismatch(n))), where n belongs
            // to one of [start_idx, end_idx). In case all of them have
            // the same max(gap, mismatch), then the first one is used.

            for (; curr_idx < end_idx; ++curr_idx, ++ref_idx) {
                // collect pileup at base start_idx
                CAlnStatGlyph::SStatStruct tmp_stat;
                for (size_t g_idx = 0; g_idx <= (size_t)CAlnStatConfig::eStat_Match; ++g_idx) {
                    if (g_idx == (size_t)CAlnStatConfig::eStat_Match) {
                        auto matches = (match_graph && graphs[g_idx]) ?
                            l_GetVal(graphs[g_idx]->GetGraph(), curr_idx) : 0;
                        if (ref_idx < ref_seq_str.size()) {
                            int base_type = CAlnStatConfig::GetRefSeqIdex(ref_seq_str[ref_idx]);
                            if ((CAlnStatConfig::eStat_Gap != base_type) && (CAlnStatConfig::eStat_Intron != base_type)) {
                                if (matches == 0)
                                    matches = tmp_stat.m_Data[base_type];
                                else if (tmp_stat.m_Data[base_type] == 0) {
                                    tmp_stat.m_Data[base_type] = matches;
                                }
                            }
                        }
                        tmp_stat.m_Data[CAlnStatConfig::eStat_Match] = matches;
                    } else if (graphs[g_idx]) {
                        tmp_stat.m_Data[g_idx] = l_GetVal(graphs[g_idx]->GetGraph(), curr_idx);
                    }
                }
                stat_collector.AddStat(stat_idx, tmp_stat);
            }
        }
        stat_collector.UpdateGroup(offset);
    }

    return eCompleted;
}


IAppJob::EJobState CSGAlignStatJob::x_Execute()
{
    // prepare the job result object
    CSGAlignJobResult* result = new CSGAlignJobResult();
    m_Result.Reset(result);
    result->m_Token = m_Token;
    
    if (!m_CacheKey.empty()) {
        // BAM/cSAR alignment collect pileup into CAlignmentGraph and stored in ICache
        try {
            auto data = CGraphCache<CPileUpGraph>::GetInstance().GetData(m_CacheKey);
            vector<TSeqRange> missing_ranges;
            data->GetMissingRegions(m_Range, missing_ranges);
            bool update_data = !missing_ranges.empty();
            if (update_data) {
                if (!data->IsCacheGood()) {
                    CGraphCache<CPileUpGraph>::GetInstance().RemoveData(m_CacheKey);
                    /// Restrart Worker Node since remote file was updated 
                    /// and BAM/cSRA objmgr cached data got invalidated
                    ERR_POST(Fatal << "Restart due to expired cache data");

                    /// Clearing object manager cache for the remote file
                    CGraph_CI graph_iter(m_Handle, m_Range, m_GraphSel);
                    if (graph_iter) {
                        m_Handle.GetScope().RemoveFromHistory(graph_iter.GetAnnot().GetTSE_Handle());
                    }
                }
                vector<future<void>> results;
                mutex data_handle_lock;
                for (auto& range : missing_ranges) {
                    results.emplace_back(async(launch::async, [&](){
                                CGraphStatCollector_Graph stat_collector(1., *data);
                                CTSE_Handle tse_handle;
                                x_CollectAlignStats(range, tse_handle, stat_collector);
                                if (tse_handle) {
                                    lock_guard<mutex> guard(data_handle_lock);
                                    result->m_DataHandle = tse_handle;
                                }
                            }));
                }
                
                for (auto& f : results) 
                    f.get();
            } else {
                int num_reads = data->GetNumberOfReads(m_Range);
                if (num_reads == 0) {
                    CGraphStatCollector_Graph stat_collector(1., *data);
                    CTSE_Handle tse_handle;
                    x_CollectAlignStats(m_Range, tse_handle, stat_collector);
                    if (tse_handle) 
                        result->m_DataHandle = tse_handle;
                    num_reads = data->GetNumberOfReads(m_Range);
                    if (num_reads != 0) {
                        auto diag = GetDiagContext().Extra();
                        string err_msg = "failure at "
                            + NStr::NumericToString(m_Range.GetFrom())
                            + ".."
                            + NStr::NumericToString(m_Range.GetTo());
                        diag.Print("graph_cache_integrity", err_msg);
                        ERR_POST(Error << "GraphCache integrity check: " << err_msg);
                    } 
                }
                
            }
            CRef<CAlnStatGlyph> stat_glyph(new CAlnStatGlyph(m_Range.GetFrom(), m_Window));
            CAlnStatGlyph::TStatVec& stats = stat_glyph->GetStatVec();
            data->UpdateAlignStats(m_Range, m_Window, stats);
            stat_glyph->ShowIntrons();
            result->m_ObjectList.emplace_back(stat_glyph.GetPointer());
            if (update_data) 
                CGraphCache<CPileUpGraph>::GetInstance().SaveData(data);
            return eCompleted;
        } catch (exception& e) {
            m_Error.Reset(new CAppJobError(string(e.what())));
        }
        return eFailed;
    }

    // regular alignments
    // collect pileup directly into CAlnStatGlyph glyph
    CRef<CAlnStatGlyph> stat_glyph(new CAlnStatGlyph(m_Range.GetFrom(), m_Window));
    CAlnStatGlyph::TStatVec& stats = stat_glyph->GetStatVec();
    size_t pix_num = (size_t)ceil(m_Range.GetLength() / max(1.,m_Window));
    stats.resize(pix_num);
    CGraphStatCollector_Glyph stat_collector(m_Window, stats);
    CTSE_Handle tse_handle;
    x_CollectAlignStats(m_Range, tse_handle, stat_collector);
    stat_glyph->ShowIntrons();
    if (tse_handle)
        result->m_DataHandle = tse_handle;
    result->m_ObjectList.emplace_back(stat_glyph.GetPointer());
    return eCompleted;
}


IAppJob::EJobState CSGAlignStatJob::x_LoadAlignments(CTSE_Handle& tse_handle)
{
    SetTaskName("Load alignments ...");
    m_Sel.SetMaxSize(kMaxAlignmentLoaded);
    CAlign_CI align_iter(m_Handle, m_Range, m_Sel);
    if (align_iter.GetSize() == 0) 
        return eCompleted;
    tse_handle = align_iter.GetAnnot().GetTSE_Handle();

    SetTaskName("Create alignment data source ...");
    SetTaskTotal((int)align_iter.GetSize());
    SetTaskCompleted(0);
    auto synonyms = m_Handle.GetSynonyms();
    for ( ;  align_iter;  ++align_iter) {
        if (IsCanceled()) {
            return eCanceled;
        }
        vector<CSeq_align::TDim> anchors;
        s_GetAnchors(m_Handle, synonyms.GetPointer(), *align_iter, anchors);

        TAlnDataSources aln_datasources;
        try {
            // try using both direction
            s_BuildAlignDataSource(m_Handle, anchors, *align_iter, true, 0.0, aln_datasources);
        } catch (CException& e) {
            // log errors
            LOG_POST(Warning << "error in GetAlignments(): " << e.GetMsg());
        }

        for (size_t i = 0; i < aln_datasources.size(); ++i) {
            CConstRef<IAlnGraphicDataSource>& aln_datasource =
                aln_datasources[i].first;
            m_Aligns.push_back(aln_datasource);
        }
        AddTaskCompleted(1);
    }

    SetTaskCompleted((int)align_iter.GetSize());
    return eCompleted;
}

static void s_GetAnchors(CBioseq_Handle& handle, const CSynonymsSet* synonyms, const CSeq_align& align, vector<CSeq_align::TDim>& anchors)
{
    CSeq_align::TDim num_row = align.CheckNumRows();
    if (num_row == 0)
        NCBI_THROW(CException, eUnknown, "Get empty alignment!");

    auto& ids = handle.GetId();
    CSeq_align::TDim row = 0;
    for (row = 0; row < num_row; ++row) {
        const auto& row_id = align.GetSeq_id(row);
        for (auto& id : ids) {
            if (id.GetSeqId()->Match(row_id)) {
                anchors.push_back(row);
            }
        }
    }
    if (anchors.empty() && synonyms) {
        CScope& scope = handle.GetScope();
        /// try a more aggressive matching approach
        for (size_t level = 0; level <= 5 && anchors.empty(); ++level) {
            for (row = 0; row < num_row; ++row) {
                const auto& row_id = align.GetSeq_id(row);
                CSeq_id_Handle idh = sequence::GetId(row_id, scope, sequence::eGetId_Best);
                if (!idh)
                    idh = sequence::GetId(row_id, scope, sequence::eGetId_Canonical);
                if (handle.ContainsSegment(idh, level)) {
                    anchors.push_back(row);
                }
            }
        }
    }
}


void s_BuildAlignDataSource(CBioseq_Handle& handle,
                            vector<CSeq_align::TDim>& anchors,
                            const CSeq_align& align,
                            bool sparseAln,
                            TModelUnit window,
                            TAlnDataSources& data_sources)
{
    data_sources.clear();
    // get the anchor rows which are the referent sequence
    _ASSERT(!anchors.empty());
    if (anchors.empty()) {
        NCBI_THROW(CException, eUnknown,
                   "Can find the anchor sequence in the alignment!");
    }

    CSeq_align::TDim num_row = align.CheckNumRows();
    if (num_row == 0) {
        // empty alignment
        NCBI_THROW(CException, eUnknown, "Get empty alignment!");
    }
    bool self_alignment = false;
    if (num_row  ==  (int)anchors.size()) {
        // This is a self-aligned alignment (same sequence for all rows).
        // Request no merge (ePreserveRows) when creating CAnchoredAln
        self_alignment = true;

        // check if all aligned rows match with the exactly same range
        bool exact_self_aligned = true;
        CRange<TSeqPos> aligned_r = align.GetSeqRange(0);
        for (CSeq_align::TDim row = 1; row < num_row; ++row) {
            if (aligned_r != align.GetSeqRange(row)) {
                exact_self_aligned = false;
                break;
            }
        }

        // Use the first row only to avoid duplication if it is an exact
        // self-aligned alignment (JIRA GB-1289). Otherwise show the alignment
        // multiple times, once for each row as the anchor sequence range
        // (JIRA GB-2349)
        if (exact_self_aligned) {
            anchors.clear();
            anchors.push_back(0);
        }
    }

    if (align.CheckNumRows() == 2 && (anchors.size() == 1 || self_alignment)) {
        ITERATE (vector<CSeq_align::TDim>, row_iter, anchors) {
            CSeq_align::TDim anchor_row = *row_iter;
            try {
                if (window > 0.0  &&  align.GetSeqRange(anchor_row).GetLength() / window < 5.0) {
                    CConstRef<IAlnGraphicDataSource>
                        ds(new CSimpleGraphicDataSource(align, handle.GetScope(), anchor_row));
                    data_sources.push_back(TAlnDS_Anchor(ds, anchor_row));
                } else if (align.CheckNumRows() == 2) {
                    if (align.GetSegs().IsDenseg()  &&
                        align.GetSegs().GetDenseg().CheckNumSegs() > 0  &&
                        !align.GetSegs().GetDenseg().IsSetWidths()) { // we don't deal with mixed denseg

                            CConstRef<IAlnGraphicDataSource>
                                ds(new CDensegGraphicDataSource(align, handle.GetScope(), anchor_row));
                            //TAlnDS_Anchor(ds, anchor_row)
                            data_sources.emplace_back(ds, anchor_row);

                    } else if (align.GetSegs().IsStd()  &&  align.GetSegs().GetStd().size() == 1) {
                        const vector< CRef< CSeq_loc > >& locs =
                            align.GetSegs().GetStd().front()->GetLoc();
                        _ASSERT(locs.size() == 2);
                        if (locs[0]->IsInt()  &&  locs[1]->IsInt()) {
                            TSeqPos len1 = locs[0]->GetTotalRange().GetLength();
                            TSeqPos len2 =  locs[1]->GetTotalRange().GetLength();
                            if (len1 != len2  &&  len1 != 3 * len2  &&  len2 != 3 * len1) {
                                // For std-seg with two uneven aligned segments, if it is not
                                // a protein-to-genomic alignment, use a simplified alignemnt
                                // manager to handle this special case.
                                CSimpleGraphicDataSource* simple_ds =
                                    new CSimpleGraphicDataSource(align, handle.GetScope(), anchor_row);
                                CConstRef<IAlnGraphicDataSource> ds(simple_ds);
                                simple_ds->SetRegularity(false);
                                data_sources.emplace_back(ds, anchor_row);
                            }
                        }
                    }
                }
            } catch (CException&) {
                // ignore
            }
        }
    }

    if ( !data_sources.empty() )
        return;

    bool report_overlaps = false;

    ITERATE (vector<CSeq_align::TDim>, row_iter, anchors) {
        CSeq_align::TDim anchor = *row_iter;
        // generate an alignment manager for this alignment (either CAlnVec or CSparseAln)
        if (sparseAln) {
            typedef vector<const objects::CSeq_align*> TAlnVector;
            typedef CAlnIdMap<TAlnVector, TIdExtract> TAlnIdMap;
            typedef CAlnStats<TAlnIdMap> TAlnStats;

            CAlnContainer aln_container;
            aln_container.insert(align);
            CAlnSeqIdsExtract<CAlnSeqId> id_extract;
            TAlnIdMap aln_id_map(id_extract, aln_container.size());
            int num_align = 0;
            ITERATE(CAlnContainer, aln_it, aln_container) {
                try {
                    // Create a vector of seq-ids for the seq-align
                    aln_id_map.push_back(**aln_it);
                    ++num_align;
                } catch (CAlnException e) {
                    // Skipping this alignment
                }
            }
            // Create align statistics object
            TAlnStats aln_stats(aln_id_map);
            // Create user options
            CAlnUserOptions aln_user_options;
            aln_user_options.m_Direction = CAlnUserOptions::eBothDirections;
            TAnchoredAlnVec anchored_aln_vec;

            // Explicitly specify anchor sequence seq_id
            //aln_user_options.SetAnchorId(aln_id_map[0][anchor]);
            //CreateAnchoredAlnVec(aln_stats, anchored_aln_vec, aln_user_options);

            // Use anchor sequence row number stored in seq-align
            for (size_t aln_idx = 0; aln_idx < aln_stats.GetAlnCount(); ++aln_idx) {
                CRef<CAnchoredAln> anchored_aln =
                    CreateAnchoredAlnFromAln(aln_stats, aln_idx, aln_user_options, anchor);
                if ( anchored_aln ) {
                    anchored_aln_vec.push_back(anchored_aln);
                    // Calc scores
                    for (CSeq_align::TDim row = 0; row < anchored_aln->GetDim(); ++row) {
                        ITERATE(CPairwiseAln, rng_it, *anchored_aln->GetPairwiseAlns()[row]) {
                            anchored_aln->SetScore() += rng_it->GetLength();
                        }
                    }
                    anchored_aln->SetScore() /= anchored_aln->GetDim();
                }
            }

            if (self_alignment) {
                aln_user_options.m_MergeAlgo = CAlnUserOptions::ePreserveRows;
            } else {
                aln_user_options.m_MergeAlgo = CAlnUserOptions::eDefaultMergeAlgo;
            }
            CRef<CAnchoredAln> out_aln(new CAnchoredAln);
            BuildAln(anchored_aln_vec, *out_aln, aln_user_options);
            anchored_aln_vec.clear();
            anchored_aln_vec.push_back(out_aln);

            NON_CONST_ITERATE (TAnchoredAlnVec, a_iter, anchored_aln_vec) {
                bool has_overlaps = false;
                auto& anchored_aln = **a_iter;
                for (CAnchoredAln::TDim row = 0; has_overlaps == false && row < anchored_aln.GetDim(); ++row) {
                    const CPairwiseAln& pw = *anchored_aln.GetPairwiseAlns()[row];
                    has_overlaps = pw.IsSet(CPairwiseAln::fOverlap);
                }

                _ASSERT(anchored_aln.GetDim() > 1);
                if (anchored_aln.GetDim() < 2) {
                    // something is wrong, skip this one
                    LOG_POST(Warning << "CSGAlignmentJob::s_BuildAlignDataSource(): "
                        << "Something is wrong with the alignment. It has only one row.");
                    continue;
                }
                if (has_overlaps)
                    report_overlaps = true;

                if (has_overlaps || anchored_aln.GetPairwiseAlns()[0]->GetSecondBaseWidth() == -1) {
                    // use a simplified alignemnt manager to handle this special case.
                    CSimpleGraphicDataSource* simple_ds =
                        new CSimpleGraphicDataSource(align, handle.GetScope(), anchor);
                    CConstRef<IAlnGraphicDataSource> ds(simple_ds);
                    simple_ds->SetRegularity(false);
                    data_sources.emplace_back(ds, anchor);
                } else {
                    // Build a sparse align
                    CConstRef<CSparseAln> sparse_aln(
                        new CSparseAln(**a_iter, handle.GetScope()));
                    _ASSERT(sparse_aln->GetDim() > 1);
                    if (sparse_aln->GetDim() < 2) {
                        // something is wrong, skip this one
                        LOG_POST(Warning << "CSGAlignmentJob::s_BuildAlignDataSource(): "
                                 << "Something is wrong with the alignment. Create CSparseAln has only one row.");
                        continue;
                    }

                    // create CSparseGraphicDataSource
                    CConstRef<IAlnGraphicDataSource>
                        ds(new CSparseAlnGraphicDataSource(sparse_aln));
                    data_sources.emplace_back(ds, anchor);
                }
            }
        } else {
            CRef<CAlnVec> aln_mgr;
            if (align.GetSegs().IsDenseg()) {
                aln_mgr.Reset(new CAlnVec(align.GetSegs().GetDenseg(),
                    handle.GetScope()));
            } else {
                CAlnMix mix(handle.GetScope());
                mix.Add(align);
                mix.Merge(CAlnMix::fGapJoin);

                aln_mgr.Reset(new CAlnVec(mix.GetDenseg(), handle.GetScope()));
            }
            // anchor the alignment on the referent sequence
            aln_mgr->SetAnchor(anchor);

            // create CAlnVecGraphicDataSource
            CConstRef<IAlnGraphicDataSource>
                ds(new CAlnVecGraphicDataSource(*aln_mgr));
            data_sources.emplace_back(ds, anchor);
        }
    }
    if (report_overlaps) {
        LOG_POST(Warning << "Coloring is not supported for alignment with overlapping segments");
    }

    //// preload aligned sequences
    //try {
    //    for (int row = 0; row < aln_datasource->GetNumRows(); ++row) {
    //        aln_datasource->GetBioseqHandle(row);
    //    }
    //} catch (CException& e) {
    //    // ignore the error on resolving the seq-id
    //    LOG_POST(Error << e.GetMsg());
    //}

    return;
}



END_NCBI_SCOPE
