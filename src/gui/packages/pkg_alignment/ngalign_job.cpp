/*  $Id: ngalign_job.cpp 37341 2016-12-27 17:19:44Z katargir $
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
*  and reliability of the software and data,  the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties,  express or implied,  including
*  warranties of performance,  merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
* Authors:  Roman Katargin
*/


#include <ncbi_pch.hpp>

#include <corelib/ncbifile.hpp>
#include <corelib/ncbiexec.hpp>
#include <algo/phy_tree/phy_node.hpp>
#include <objmgr/seq_vector.hpp>
#include <objmgr/util/sequence.hpp>
#include <objtools/readers/fasta.hpp>

#include <serial/iterator.hpp>

#include <objects/seqalign/Seq_align.hpp>
#include <objects/biotree/BioTreeContainer.hpp>

#include <algo/phy_tree/dist_methods.hpp>

#include <gui/objutils/label.hpp>
#include <objects/gbproj/AbstractProjectItem.hpp>

#include <gui/packages/pkg_alignment/ngalign_job.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <algo/align/util/score_builder.hpp>
#include <algo/align/ngalign/ngalign.hpp>
#include <algo/align/ngalign/unordered_spliter.hpp>
#include <algo/align/ngalign/blast_aligner.hpp>
#include <algo/align/ngalign/banded_aligner.hpp>
#include <algo/align/ngalign/merge_aligner.hpp>
#include <algo/align/ngalign/inversion_merge_aligner.hpp>
#include <algo/align/ngalign/alignment_filterer.hpp>
#include <algo/align/ngalign/alignment_scorer.hpp>

#include <algo/blast/blastinput/blastn_args.hpp>
#include <algo/blast/api/windowmask_filter.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// CNGAlignJob
CNGAlignJob::CNGAlignJob(const CNGAlignParams& params) : m_Params(params)
{
    m_Descr = "NGAlign Job";
}

///////////////////////////////////////////////////////////////////////////////

enum EAlignScoreTypes {
    /// add BLAST-style 'num_ident' score
    fScore_Identities       = 0x001,

    /// add a 'mismatch' core with a count of mismatches
    fScore_Mismatches       = 0x002,

    /// add a 'gap_count' score
    fScore_GapCount         = 0x004,

    /// add scores for ungapped and gapped percent identity
    fScore_PercentIdentity  = 0x008,

    /// add a score for percent coverage of query (sequence 0)
    fScore_PercentCoverage  = 0x010,

    /// default flags: everything
    fScore_Default = 0xffffffff
};
typedef int TAlignScoreFlags;

void AddStandardAlignmentScores(CScope&          scope,
                                CSeq_align&      align,
                                TAlignScoreFlags flags = fScore_Default)
{
    CScoreBuilder sb;

    if (flags & (fScore_Identities |
                 fScore_Mismatches |
                 fScore_PercentIdentity)) {
        if (flags & fScore_PercentIdentity) {
            /// this automatically adds num_ident and num_mismatch
            sb.AddScore(scope, align,
                        CSeq_align::eScore_PercentIdentity_Gapped);
            sb.AddScore(scope, align,
                        CSeq_align::eScore_PercentIdentity_Ungapped);
        } else if (flags & fScore_Identities) {
            sb.AddScore(scope, align, CSeq_align::eScore_IdentityCount);
        } else if (flags & fScore_Mismatches) {
            sb.AddScore(scope, align, CSeq_align::eScore_MismatchCount);
        }
    }

    if (flags & fScore_GapCount) {
        /// FIXME: add eScore_GapCount to CSeq_align, CScoreBuilder
        //sb.AddScore(scope, align, CScoreBuilder::eScore_GapCount);
        int gap_count = sb.GetGapCount(align);
        align.SetNamedScore("gap_count", gap_count);
    }

    if (flags & fScore_PercentCoverage) {
        sb.AddScore(scope, align, CSeq_align::eScore_PercentCoverage);
    }

}

//////////////////////////////////////////////////////////////////////////////

class CGPipeAlignmentScorer : public IAlignmentScorer {
public:
    CGPipeAlignmentScorer() {}

    void ScoreAlignments(TAlignResultsRef results, CScope& scope)
    {
        NON_CONST_ITERATE (CAlignResultsSet::TQueryToSubjectSet,
                           result_iter, results->Get()) {
            NON_CONST_ITERATE (CQuerySet::TAssemblyToSubjectSet,
                               assm_iter, result_iter->second->Get()) {
                NON_CONST_ITERATE (CQuerySet::TSubjectToAlignSet,
                                   query_iter, assm_iter->second) {
                    NON_CONST_ITERATE (CSeq_align_set::Tdata, it,
                                       query_iter->second->Set()) {
                        CSeq_align& align = **it;
                        AddStandardAlignmentScores(scope, align);

                        /// additionally, add the gaponly version, used in gbDNA
                        CScoreBuilder sb;
                        sb.AddScore(scope, align,
                                    CSeq_align::eScore_PercentIdentity_GapOpeningOnly);
                    }
                }
            }
        }
    }
};

static Boolean s_BlastInterruptCallback( SBlastProgress* prog )
{
    if( prog && prog->user_data ){
        CJobCancelable* job =
            reinterpret_cast<CJobCancelable*>(prog->user_data)
        ;
        if( job->IsCanceled() ){
            return TRUE;
        }
    }
    return FALSE;
}

static CRef<CSeq_align_set> s_CallAligner(
    CNGAlignParams& params,
    CNgAligner& aligner,
    CJobCancelable* job,
    CUnorderedSplitter* aSplitter = NULL,
    bool is_splittable_sequences = false
){
    CRef<blast::CBlastNucleotideOptionsHandle> opts
        (new blast::CBlastNucleotideOptionsHandle);

    opts->SetTraditionalBlastnDefaults();
    blast::CBlastOptions& options = opts->SetOptions();

    options.SetWordSize (params.GetWordSize());
    options.SetEvalueThreshold (params.GetEValue());

    auto_ptr<CAutoEnvironmentVariable>  p_wm_path;

    if (params.GetWMTaxId() != 0) {
        if (!params.GetWMDir().empty()) {
            options.SetWindowMaskerTaxId(params.GetWMTaxId());
            //p_wm_path.reset(new CAutoEnvironmentVariable("WINDOW_MASKER_PATH", params.GetWMDir().c_str()));
            blast::WindowMaskerPathInit( params.GetWMDir() );
        } else {
            ERR_POST(Error << "BLAST window masker tax-id filtering ignored. WindowMasker DB directory not set.");
        }
    }

    if (params.GetBestHit()) {
        options.SetBestHitScoreEdge(0.1);
        options.SetBestHitOverhang(0.1);
    }

    string advParams = ToStdString(params.GetAdvParams());
    if (!advParams.empty()) {
		static const string s_whitespace(" \n\t\r");
		vector<string> argv;
		argv.push_back( EProgramToTaskName( blast::eBlastn ) );
		NStr::Split( advParams, s_whitespace, argv );

        CRef<blast::CBlastAppArgs> blast_args(new blast::CBlastnAppArgs());
        auto_ptr<CArgDescriptions> arg_descs( blast_args->SetCommandLine() );
		auto_ptr<CArgs> args( arg_descs->CreateArgs( argv.size(), argv ) );

         ///
         /// Standard processing for BLAST
         ///
         blast::CGenericSearchArgs search_args(false /* not protein */,
                                               false /* not RPS blast */,
                                               true /* show %identity */);
         search_args.ExtractAlgorithmOptions(*args, options);

         blast::CFilteringArgs filter_args(false /* not protein */);
         filter_args.ExtractAlgorithmOptions(*args, options);

         blast::CNuclArgs nucl_args;
         nucl_args.ExtractAlgorithmOptions(*args, options);

         blast::CGappedArgs gapped_args;
         gapped_args.ExtractAlgorithmOptions(*args, options);

         /// best hit args here
         blast::CHspFilteringArgs cull_args;
         cull_args.ExtractAlgorithmOptions(*args, options);

         /// Argments for window size
         blast::CWindowSizeArg window_args;
         window_args.ExtractAlgorithmOptions(*args, options);
    }

    auto_ptr<CBlastAligner> al(new CBlastAligner(*opts, 0));
    al->SetInterruptCallback( s_BlastInterruptCallback, job );
    aligner.AddAligner(al.release());

    aligner.AddAligner(new CMergeAligner(1));
    /*
	aligner.AddAligner(new CInstancedAligner(120,
                                             10,
                                             5,
                                             1));
    */
	if( aSplitter && is_splittable_sequences ){
        /// special split merger for phase 1 alignments
        aligner.AddAligner( new CSplitSeqAlignMerger(aSplitter) );
    } else {
        /// non-phase1 sequences: use the inversion merge aligner
        aligner.AddAligner( new CInversionMergeAligner(1) );
    }

    int filterMode = params.GetFilterMode();

    if (filterMode == 0 || filterMode == 1) {
        aligner.AddFilter(new CQueryFilter(0,
            "pct_identity_gapopen_only >= 99.5 AND pct_coverage >= 99"));
        aligner.AddFilter(new CQueryFilter(1,
            "pct_identity_gapopen_only >= 95 AND pct_coverage >= 95"));
        aligner.AddFilter(new CQueryFilter(2,
            "pct_identity_gapopen_only >= 95 AND pct_coverage >= 50"));
        aligner.AddFilter(new CQueryFilter(3,
            "pct_identity_gapopen_only >= 80 AND pct_coverage >= 25"));
    }
    else {
        aligner.AddFilter(new CQueryFilter(0,
            "pct_identity_gapopen_only >= 99.5 AND pct_coverage >= 99 AND align_length_ratio <= 2"));
        aligner.AddFilter(new CQueryFilter(1,
            "pct_identity_gapopen_only >= 95 AND pct_coverage >= 95"));
        aligner.AddFilter(new CQueryFilter(2,
            "pct_identity_gapopen_only >= 95 AND pct_coverage >= 50"));
        aligner.AddFilter(new CQueryFilter(3,
            "pct_identity_gapopen_only >= 80 AND pct_coverage >= 25"));
    }

    ///
    /// add scoring
    /// These add scores 
    aligner.AddScorer( new CBlastScorer( CBlastScorer::eSkipUnsupportedAlignments ) );
    aligner.AddScorer( new CGPipeAlignmentScorer() );
    aligner.AddScorer( new CCommonComponentScorer() );

    ///
    /// Run!
    return aligner.Align();
}

void CNGAlignJob::x_CreateProjectItems()
{
    ///
    /// Create the NG aligner
    ///

    m_Params.LogDump();

    CRef<CScope> scope  = m_Params.GetSubject().scope;

    /// subject sequence
    list< CRef<CSeq_loc> > subject_locs;
    const CSeq_id* seq_id = dynamic_cast<const CSeq_id*>(m_Params.GetSubject().object.GetPointer());
    if( seq_id != NULL ){
        CRef<CSeq_loc> loc( new CSeq_loc( CSeq_loc::e_Whole ) );
        loc->SetId( *seq_id );

        subject_locs.push_back( loc );
    } else {
        const CSeq_loc* seq_loc = dynamic_cast<const CSeq_loc*>(m_Params.GetSubject().object.GetPointer());
        if( seq_loc != NULL ){

            subject_locs.push_back( CRef<CSeq_loc>(const_cast<CSeq_loc*>(seq_loc)) );
        } else {
            LOG_POST(Error << "CNGAlignJob::x_CreateProjectItems(): invalid (non Seq-id/Seq-loc) subject sequence!");
            return;
        }
    }

    CRef<CSeqLocListSet> subject( new CSeqLocListSet() );
    subject->SetLocList().insert( subject->SetLocList().end(), subject_locs.begin(), subject_locs.end() );

    /// query sequences
    list< CRef<CSeq_loc> > query_locs;

    NON_CONST_ITERATE( TConstScopedObjects, iter, m_Params.SetQueries() ){
        const CSeq_id* seq_id = dynamic_cast<const CSeq_id*>( iter->object.GetPointer() );
        if( seq_id != NULL ){
            CRef<CSeq_loc> loc( new CSeq_loc( CSeq_loc::e_Whole ) );
            loc->SetId( *seq_id );

            query_locs.push_back( loc );
        } else {
            const CSeq_loc* seq_loc = dynamic_cast<const CSeq_loc*>( iter->object.GetPointer() );
            if( seq_loc != NULL ){

                query_locs.push_back( CRef<CSeq_loc>(const_cast<CSeq_loc*>(seq_loc)) );
            } else {
                LOG_POST(Error << "CNGAlignJob::x_CreateProjectItems(): invalid (non Seq-id/Seq-loc) query sequence!");
                return;
            }
        }
    }


    TAlignSetRef alignments;
    auto_ptr<CUnorderedSplitter> splitter;

    if (m_Params.GetFilterMode() == 0) {
        ///
        /// this pass takes care of splittable (i.e., delta-seq) phase 1
        /// sequences
        /// not all phase 1 sequences pass through here!
        ///
        
        splitter.reset( new CUnorderedSplitter( *scope ) );
        CRef<CSplitSeqLocListSet> queries( new CSplitSeqLocListSet( splitter.get() ) );

        ITERATE(list< CRef<CSeq_loc> >, it, query_locs ){
            queries->AddSeqLoc(*it);
        }
        
        /*---
        CRef<CSeqLocListSet> queries( new CSeqLocListSet() );
        queries->SetLocList().insert( queries->SetLocList().end(), query_locs.begin(), query_locs.end() );
        */

        CNgAligner aligner(*scope);
        aligner.SetQuery(queries);
        aligner.SetSubject(subject);

        CRef<CSeq_align_set> sas = s_CallAligner(m_Params, aligner, this, splitter.get(), true);
        //---CRef<CSeq_align_set> sas = s_CallAligner( m_Params, aligner );

        if( sas && sas->IsSet() ){
            if( !alignments ){
                alignments = sas;
            } else {
                alignments->Set().insert(
                    alignments->Set().end(),
                    sas->Set().begin(),
                    sas->Set().end()
                );
            }
        }

    } else {
        ///
        /// all non-splittable sequences are handled differently with different
        /// aligners
        ///
        CRef<CSeqLocListSet> queries( new CSeqLocListSet() );
        queries->SetLocList().insert( queries->SetLocList().end(), query_locs.begin(), query_locs.end() );

        CNgAligner aligner(*scope);
        aligner.SetQuery(queries);
        aligner.SetSubject(subject);

        CRef<CSeq_align_set> sas = s_CallAligner( m_Params, aligner, this );

        if( sas && sas->IsSet() ){
            if( !alignments ){
                alignments = sas;
            } else {
                alignments->Set().insert(
                    alignments->Set().end(),
                    sas->Set().begin(),
                    sas->Set().end()
                );
            }
        }
    }

    if (alignments && ! alignments->Get().empty()) {
        // pack the alignment in a Seq-annot and label it appropriately
        CRef<CSeq_annot> annot(new CSeq_annot());

        typedef CSeq_annot::TData::TAlign TAlign;
        TAlign& seqannot_align = annot->SetData().SetAlign();

        string title;

        ITERATE( CSeq_align_set::Tdata, align, alignments->Get() ){
            if (title.empty()) {
                CLabel::GetLabel(**align, &title, CLabel::eDefault, scope);
            }
            seqannot_align.push_back(*align);
        }

        title += ": Genomic Alignment";

        annot->SetCreateDate(CurrentTime());
        annot->SetTitleDesc(title);
        annot->SetNameDesc(title);

        /// now create a Project Item for the data
        CRef<CProjectItem> item(new CProjectItem());
        item->SetItem().SetAnnot(*annot);

        item->SetLabel(title);
        AddProjectItem(*item);
    }
}

END_NCBI_SCOPE

