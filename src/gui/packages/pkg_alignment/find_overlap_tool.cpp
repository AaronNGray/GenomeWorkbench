/*  $Id: find_overlap_tool.cpp 36594 2016-10-12 20:17:36Z evgeniev $
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
 * Authors:  Roman Katargin
 *
 */

#include <ncbi_pch.hpp>

#include <objects/seqloc/Seq_id.hpp>

#include <gui/packages/pkg_alignment/find_overlap_tool.hpp>
#include <gui/packages/pkg_alignment/find_overlap_panel.hpp>

#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <serial/typeinfo.hpp>

#include <objmgr/object_manager.hpp>

#include <gui/objutils/label.hpp>

#include <gui/objutils/interface_registry.hpp>
#include <gui/objutils/gui_seq_info.hpp>

#include <algo/align/contig_assembly/contig_assembly.hpp>

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
#include <algo/blast/api/blast_exception.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

//static void s_PrepareAligner( CNgAligner& aligner, CFindOverlapParams& params );

///////////////////////////////////////////////////////////////////////////////
/// CFindOverlapTool
CFindOverlapTool::CFindOverlapTool()
:   CAlgoToolManagerBase("Find Overlap between DNA Sequences",
                         "",
                         "Find overlap between closely related DNA Sequences",
                         "Generate an overlap alignment between "
                         "closely related DNA sequences, particularly for "
                         "exploring clone overlap relationships",
                         "FIND_OVERLAP",
                         "Alignment Creation"),
    m_Panel()
{
}

string CFindOverlapTool::GetExtensionIdentifier() const
{
    return "find_overlap_tool";
}


string CFindOverlapTool::GetExtensionLabel() const
{
    return "Find Overlap Tool";
}

void CFindOverlapTool::InitUI()
{
    CAlgoToolManagerBase::InitUI();

    m_Panel = NULL;
}

void CFindOverlapTool::CleanUI()
{
    m_Panel = NULL;
    CAlgoToolManagerBase::CleanUI();
}

void CFindOverlapTool::x_CreateParamsPanelIfNeeded()
{
    if (m_Panel == NULL) {
        x_SelectCompatibleInputObjects();

        m_Panel = new CFindOverlapPanel(m_ParentWindow, wxID_ANY,
                                        wxDefaultPosition, wxSize(0, 0), SYMBOL_CFINDOVERLAPPANEL_STYLE, false);
        m_Panel->SetData(m_Params);
        m_Panel->SetObjects(&m_SeqIds);

        m_Panel->SetRegistryPath(m_RegPath + ".ParamsPanel");
        m_Panel->LoadSettings();
    }
}

bool CFindOverlapTool::x_ValidateParams()
{
    return true;
}

/// select only Seq-ids
void CFindOverlapTool::x_SelectCompatibleInputObjects()
{
    TConstScopedObjects objects;
    x_ConvertInputObjects(CSeq_id::GetTypeInfo(), objects);

    m_SeqIds.clear();
    NON_CONST_ITERATE(TConstScopedObjects, it, objects) {
        CIRef<IGuiSeqInfo> gui_seq_info(CreateObjectInterface<IGuiSeqInfo>(*it, NULL));
        if( !gui_seq_info.IsNull() && gui_seq_info->IsDNA() ){
            m_SeqIds.push_back(*it);
        }
    }
}

CAlgoToolManagerParamsPanel* CFindOverlapTool::x_GetParamsPanel()
{
    return m_Panel;
}

IRegSettings* CFindOverlapTool::x_GetParamsAsRegSetting()
{
    return &m_Params;
}

///////////////////////////////////////////////////////////////////////////////
/// CFindOverlapJob
class  CFindOverlapJob  : public CDataLoadingAppJob
{
public:
    CFindOverlapJob (const CFindOverlapParams& params);

protected:
    virtual void    x_CreateProjectItems(); // overriding virtual function

private:
    static Boolean x_BlastInterruptCallback( SBlastProgress* prog );
    void x_PrepareAligner( CNgAligner& aligner, CFindOverlapParams& params );

protected:
    CFindOverlapParams m_Params;
};

CDataLoadingAppJob* CFindOverlapTool::x_CreateLoadingJob()
{
    if (m_Panel) m_Params = m_Panel->GetData();
    CFindOverlapJob* job = new CFindOverlapJob(m_Params);
    return job;
}

///////////////////////////////////////////////////////////////////////////////
/// CFindOverlapJob
CFindOverlapJob::CFindOverlapJob(const CFindOverlapParams& params)
:   m_Params(params)
{
    CFastMutexGuard lock(m_Mutex);

    m_Descr = "Find Overlap Job"; //TODO
}

void CFindOverlapJob::x_CreateProjectItems()
{
    SConstScopedObject seq1 = m_Params.GetSeq1();
    SConstScopedObject seq2 = m_Params.GetSeq2();

    const CSeq_id& id1 = dynamic_cast<const CSeq_id&>(*seq1.object);
    const CSeq_id& id2 = dynamic_cast<const CSeq_id&>(*seq2.object);
    CScope& scope1 = *seq1.scope;
    CScope& scope2 = *seq2.scope;

    CRef<CScope> scope(&scope1);
    if (&scope1 != &scope2) {
        LOG_POST(Error << "Inconsistent scopes - results may not be viewable");
        CRef<CObjectManager> obj_mgr = CObjectManager::GetInstance();
        scope.Reset(new CScope(*obj_mgr));
        scope->AddScope(scope1);
        scope->AddScope(scope2);
    }

    const string blast_params = ToStdString(m_Params.GetBlastParams());

    int max_slop = NStr::StringToInt(ToStdString(m_Params.GetMaxSlop()));
    
    // logging
    {{
        string name1, name2;
        CLabel::GetLabel(id1, &name1, CLabel::eDefault, scope);
        CLabel::GetLabel(id2, &name2, CLabel::eDefault, scope);

        LOG_POST(Info << "Find Overlap Alignments: \n" << "  " << name1 << " x " << name2 
            << "\n " 
            << " blast_params='" << blast_params << "'" << " " 
            << " max-slope=" << m_Params.GetMaxSlop()
            << " filter-quality=" << m_Params.GetFilterQuality()
        );
    }}

    vector<CRef<CSeq_align> > alns;
    /*
    alns = CContigAssembly::Align(id1, id2, blast_params,
        min_ident, max_slop, *scope, NULL,
        half_widths
    );
    */

    CRef<CSeqIdListSet> subject( new CSeqIdListSet() );
    subject->SetIdList().push_back( CRef<CSeq_id>( (CSeq_id*)&id1 ) );

    CRef<CSeqIdListSet> query( new CSeqIdListSet() );
    query->SetIdList().push_back( CRef<CSeq_id>( (CSeq_id*)&id2 ) );


    CNgAligner aligner(*scope);

    aligner.SetQuery(query);
    aligner.SetSubject(subject);

    x_PrepareAligner( aligner, m_Params );

    CRef<CSeq_align_set> sas = aligner.Align(); //s_CallAligner( m_Params, aligner );

    if( sas && sas->IsSet() ){
        alns.insert( alns.end(), sas->Set().begin(), sas->Set().end() );
    }



    // make an annotation
    if (alns.size()) {
        CRef<CSeq_annot> annot(new CSeq_annot());
        ITERATE (vector<CRef<CSeq_align> >, iter, alns) {
            annot->SetData().SetAlign().push_back(*iter);
        }

        string name;
        CLabel::GetLabel(id1, &name, CLabel::eDefault, scope);
        name += " x ";
        CLabel::GetLabel(id2, &name, CLabel::eDefault, scope);
        name += ": Overlap Alignment";

        annot->SetNameDesc(name);
        annot->SetCreateDate(CTime(CTime::eCurrent));

        /// now create a Project Item for the data
        CRef<CProjectItem> item(new CProjectItem());
        item->SetItem().SetAnnot(*annot);

        // TODO we need to generate title properly
        item->SetLabel(name);
        AddProjectItem(*item);
    
    } else {
        LOG_POST(Info << "No acceptable overlaps are found.");

        x_SetTextResult( "No overlaps are found according to the settings desired." );
    }

}

void CFindOverlapJob::x_PrepareAligner( CNgAligner& aligner, CFindOverlapParams& params )
{
    CRef<blast::CBlastNucleotideOptionsHandle> opts( new blast::CBlastNucleotideOptionsHandle() );

    opts->SetTraditionalBlastnDefaults();
    blast::CBlastOptions& options = opts->SetOptions();

    //options.SetWordSize (params.GetWordSize());
    //options.SetEvalueThreshold (params.GetEValue());

    /*
    if (params.GetBestHit()) {
        options.SetBestHitScoreEdge(0.1);
        options.SetBestHitOverhang(0.1);
    }
    */

    const string blast_params = ToStdString( params.GetBlastParams() );

    if( !blast_params.empty() ){
		static const string s_whitespace(" \n\t\r");
		vector<string> argv;
		argv.push_back( EProgramToTaskName( blast::eBlastn ) );
		NStr::Split( blast_params, s_whitespace, argv );

        // This is a hack against old-fasioned parameters
        for( unsigned int i = 0;  i < argv.size();  i++ ){
            string& name = argv[i];

            if (name == "-W") {
                //options.SetWordSize(NStr::StringToInt(value));
                name = "-word_size";
            } else if (name == "-r") {
                //options.SetMatchReward(NStr::StringToInt(value));
                name = "-reward";
            } else if (name == "-q") {
                //options.SetMismatchPenalty(NStr::StringToInt(value));
                name = "-penalty";
            } else if (name == "-e") {
                //options.SetEvalueThreshold(NStr::StringToDouble(value));
                name = "-evalue";
            } else if (name == "-Z") {
                //options.SetGapXDropoffFinal(NStr::StringToInt(value));
                name = "-xdrop_gap_final";
            } else if (name == "-F") {
                //options.SetFilterString(value.c_str());
                name = "-dust";

                string& value = argv[i + 1];
                value = ( value == "T" ) ? "yes" : "no";

            } else if (name == "-G") {
                //options.SetGapOpeningCost(NStr::StringToInt(value));
                name = "-gapopen";
            } else if (name == "-E") {
                //options.SetGapExtensionCost(NStr::StringToInt(value));
                name = "-gapextend";
            }
        }


        /*
        for (unsigned int i = 0;  i < argv.size();  i += 2) {
            const string& name = argv[i];

            if (i + 1 >= argv.size()) {
                throw runtime_error("no value given for " + name);
            }
            const string& value = argv[i + 1];
            if (name == "-W") {
                options.SetWordSize(NStr::StringToInt(value));
            } else if (name == "-r") {
                options.SetMatchReward(NStr::StringToInt(value));
            } else if (name == "-q") {
                options.SetMismatchPenalty(NStr::StringToInt(value));
            } else if (name == "-e") {
                options.SetEvalueThreshold(NStr::StringToDouble(value));
            } else if (name == "-Z") {
                options.SetGapXDropoffFinal(NStr::StringToInt(value));
            } else if (name == "-F") {
                options.SetFilterString(value.c_str());
            } else if (name == "-G") {
                options.SetGapOpeningCost(NStr::StringToInt(value));
            } else if (name == "-E") {
                options.SetGapExtensionCost(NStr::StringToInt(value));
            } else {
                throw runtime_error("invalid option: " + name);
            }
        }
        */


        try {
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

        } catch( CException& ex ){
            NCBI_THROW( CException, 
                eUnknown, 
                "Cannot parse BLAST params, please edit them:\n" + ex.GetMsg() 
            );
        }

    }

    int max_slop = NStr::StringToInt(ToStdString(params.GetMaxSlop()));

    CBlastAligner* blast_aligner = new CBlastAligner( *opts, 0 );
    blast_aligner->SetInterruptCallback( x_BlastInterruptCallback, this );

    aligner.AddAligner( blast_aligner );

    aligner.AddAligner(new CMergeAligner(1));


    // filters

    string perfect_dovetail      = " pct_identity_ungap >= 100  AND longest_gap < 50 AND full_dovetail = 1 ";
    string near_perfect_dovetail = " pct_identity_ungap >= 99.8 AND longest_gap < 50 AND full_dovetail = 1 ";
    string super_green_contained = " pct_identity_ungap >= 99.8 AND contained > -1 ";

    string green_dovetail  = " pct_identity_ungap >= 99.6 AND longest_gap < 50 AND full_dovetail = 1 ";
    string green_contained = " pct_identity_ungap >= 99.6 AND contained > -1 ";

    string yellow_dovetail = " pct_identity_ungap >= 98.0 AND longest_gap < 500 AND full_dovetail = 1 ";
    string yellow_contained = " pct_identity_ungap >= 98.0 AND contained > -1 ";
    string yellow_half_dovetail = " pct_identity_ungap >= 98.0 AND longest_gap < 500 AND half_dovetail = 1 AND tail_length <= 50 ";

    string red_dovetail = " pct_identity_ungap >= 95.0 AND full_dovetail = 1 ";
    string red_half_dovetail = " pct_identity_ungap >= 95.0 AND half_dovetail = 1 AND tail_length <= 2000 AND tail_length <= align_length_ungap  ";
    string red_contained = " pct_identity_ungap >= 95.0 AND contained > -1 ";

    string super_red_half_dovetail = " pct_identity_ungap >= 92.0 AND half_dovetail = 1 AND tail_length <= 5000 AND tail_length <= align_length_ungap  ";
    string mega_red_half_dovetail  = " pct_identity_ungap >= 92.0 AND half_dovetail = 1 AND tail_length <= 10000  ";
    string ultra_red_half_dovetail = " pct_identity_ungap >= 92.0 AND half_dovetail = 1 AND tail_length <= MUL(5, align_length_ungap) ";
    string omega_red_half_dovetail = " pct_identity_ungap >= 92.0 AND half_dovetail = 1  ";

    int filter_qty = params.GetFilterQuality();

    if( filter_qty >= 0 ){
        aligner.AddFilter(new CQueryFilter( 0, perfect_dovetail      ) );
        aligner.AddFilter(new CQueryFilter( 1, near_perfect_dovetail ) );
        aligner.AddFilter(new CQueryFilter( 2, super_green_contained ) );
    }
    if( filter_qty >= 1 ){
        aligner.AddFilter(new CQueryFilter( 3, green_dovetail  ) );
        aligner.AddFilter(new CQueryFilter( 4, green_contained ) );
    }
    if( filter_qty >= 2 ){
        aligner.AddFilter(new CQueryFilter( 5, yellow_dovetail      ) );
        aligner.AddFilter(new CQueryFilter( 6, yellow_contained     ) );
        aligner.AddFilter(new CQueryFilter( 7, yellow_half_dovetail ) );
    }
    if( filter_qty >= 3 ){
        aligner.AddFilter(new CQueryFilter( 8, red_dovetail      ) );
        aligner.AddFilter(new CQueryFilter( 9, red_half_dovetail ) );
        aligner.AddFilter(new CQueryFilter( 10, red_contained     ) );
    }
    if( filter_qty >= 4 ){
        aligner.AddFilter(new CQueryFilter( 11, super_red_half_dovetail ) );
        aligner.AddFilter(new CQueryFilter( 12, mega_red_half_dovetail  ) );
        aligner.AddFilter(new CQueryFilter( 13, ultra_red_half_dovetail ) );
        aligner.AddFilter(new CQueryFilter( 14, omega_red_half_dovetail ) );
    }

	aligner.AddScorer( new CBlastScorer() );
    aligner.AddScorer( new CPctIdentScorer() );
    aligner.AddScorer( new COverlapScorer( max_slop ) );
}

Boolean CFindOverlapJob::x_BlastInterruptCallback( SBlastProgress* prog )
{
    if( prog && prog->user_data ){
        CFindOverlapJob* job =
            reinterpret_cast<CFindOverlapJob*>(prog->user_data)
        ;
        if( job->IsCanceled() ){
            return TRUE;
        }
    }
    return FALSE;
}


END_NCBI_SCOPE
