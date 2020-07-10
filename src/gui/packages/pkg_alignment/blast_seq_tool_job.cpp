/*  $Id: blast_seq_tool_job.cpp 43837 2019-09-09 21:12:26Z katargir $
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
 * Authors:  Andrey Yazhuk
 *
 */

#include <ncbi_pch.hpp>

#include <connect/ncbi_conn_stream.hpp>
#include <corelib/ncbiexec.hpp>

#include <gui/packages/pkg_alignment/blast_seq_tool_job.hpp>

#include <gui/widgets/loaders/lblast_object_loader.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/objutils/label.hpp>

#include <algo/blast/api/bl2seq.hpp>
#include <algo/blast/api/windowmask_filter.hpp>
#include <algo/blast/blastinput/cmdline_flags.hpp>
#include <algo/align/util/genomic_compart.hpp>

#include <objmgr/feat_ci.hpp>
#include <objmgr/util/sequence.hpp>

#include <gui/widgets/wx/sys_path.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/filename.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);
USING_SCOPE(blast);

///////////////////////////////////////////////////////////////////////////////
/// CBLASTSeqToolJob

// utility function for translating between plugin arg loc-lists
// and BLAST loc vectors
static void x_ToBlastVec(const TConstScopedObjects& locs, bool mask_lc_regions,
                         bool mask_rep_feats, TSeqLocVector& vec)
{
    vec.reserve(locs.size());

    ITERATE(TConstScopedObjects, it, locs) {
        CObject* obj = const_cast<CObject*>(it->object.GetPointer());

        SSeqLoc loc;
        loc.seqloc = dynamic_cast<CSeq_loc*>(obj);
        loc.scope = it->scope;

        if (mask_lc_regions  || mask_rep_feats) {
            SAnnotSelector sel = CSeqUtils::GetAnnotSelector(CSeq_annot::TData::e_Ftable);

            CRef<CSeq_loc> mask_loc(new CSeq_loc);
            if (mask_lc_regions) {
                sel.SetFeatSubtype(CSeqFeatData::eSubtype_region);
                CFeat_CI feat_iter(*loc.scope, *loc.seqloc, sel);
                for ( ; feat_iter; ++feat_iter) {
                    if (feat_iter->GetData().GetRegion()
                            == "lowercase in FASTA file") {
                        mask_loc->Add(feat_iter->GetLocation());
                    }
                }
            }
            if (mask_rep_feats) {
                sel.SetFeatSubtype(CSeqFeatData::eSubtype_repeat_region);
                CFeat_CI feat_iter(*loc.scope, *loc.seqloc, sel);
                for ( ; feat_iter; ++feat_iter) {
                    mask_loc->Add(feat_iter->GetLocation());
                }
            }
            if (mask_loc->Which() != CSeq_loc::e_not_set) {
                mask_loc->ResetStrand();
                loc.mask = mask_loc;
            }
        }
        vec.push_back(loc);
    }
}


/// static callback for BLAST interruption
/// this must match the API in CBl2Seq and BLAST
Boolean CBLASTSeqToolJob::s_BlastInterrupt( SBlastProgress* prog )
{
    if( prog && prog->user_data ){
        CBLASTSeqToolJob* job =
            reinterpret_cast<CBLASTSeqToolJob*>(prog->user_data)
        ;
        if( job->IsCanceled() ){
            return TRUE;
        }
    }
    return FALSE;
}

static const TProcessHandle kInvalidProcHandle = 
    #if defined(NCBI_OS_UNIX)
    (pid_t)-1
    #elif defined(NCBI_OS_MSWIN)
    INVALID_HANDLE_VALUE
    #else
    -1
    #endif
;

CBLASTSeqToolJob::CBLASTSeqToolJob(
    const CBLASTParams& params
)
    : m_Params( params )
    , m_ProcHandle( kInvalidProcHandle )
{
    m_Descr = "BLAST Sequences"; // TODO
}

void CBLASTSeqToolJob::x_CreateProjectItems()
{
    x_SetStatusText("Preparing input sequences for BLAST ...");

    // get locations from parameters and convert them to TSeqLocVector
    TConstScopedObjects q_locs = m_Params.GetSeqLocs();
    TConstScopedObjects s_locs = m_Params.GetSubjSeqLocs();

    string localDB;
    CRef<CLoaderDescriptor> loader;

    CBLASTParams::ESubjectType subjectType = m_Params.GetSubjectType();
    if (subjectType == CBLASTParams::eSequences) {
        if (s_locs.empty())
            return;
    }
    else if (subjectType == CBLASTParams::eLocalDB) {
        bool nuc = m_Params.IsSubjNucInput();
        if (nuc)
            localDB = m_Params.GetLocalNucDB();
        else
            localDB = m_Params.GetLocalProtDB();
        if (localDB.empty())
            return;

        if (m_Params.GetLocalDBLoader())
            loader = CLBLASTObjectLoader::CreateLoader(localDB, !nuc);
    }

    // compose the query and target sequences into BLAST forms
    TSeqLocVector query_vec, subject_vec;

    const CBLASTParams::SProgParams&  prog_params = m_Params.GetCurrParams();
    bool mask_lc = prog_params.m_MaskLowercase;
    bool mask_rp = prog_params.m_MaskRepeats;

    x_ToBlastVec(q_locs, mask_lc, mask_rp, query_vec);
    if (subjectType == CBLASTParams::eSequences)
        x_ToBlastVec(s_locs, mask_lc, mask_rp, subject_vec);

    vector<string> arg_vec;
    CRef<CBlastOptionsHandle> blast_opts = m_Params.ToArgVecBlastOptions( arg_vec );

    if( IsCanceled() ){
        return;
    }

    auto_ptr<CAutoEnvironmentVariable>  p_wm_path;
    if( m_Params.IsNucInput() && prog_params.m_WM_TaxId != 0 ){
        const string& wm_dir = m_Params.GetWmDir();
        if( !wm_dir.empty() ){
            p_wm_path.reset(new CAutoEnvironmentVariable( "WINDOW_MASKER_PATH", wm_dir.c_str() ));
            blast::WindowMaskerPathInit( wm_dir );
        }
        else {
            ERR_POST(Error << "Directory for WINDOW_MASKER files is not defined wm_taxid=" << prog_params.m_WM_TaxId);
        }
    }

    if (subjectType == CBLASTParams::eLocalDB || m_Params.IsStandaloneRequested()){
        // run standalone BLAST

        //- determine proper executable (outside of "try")
        string prog_name = arg_vec.front();
        #ifdef NCBI_OS_MSWIN
        prog_name += ".exe";            
        #endif
        arg_vec.erase( arg_vec.begin() );

        vector< CRef<CSeq_annot> > res_annots;
        CRef<CMappingRanges> ranges( new CMappingRanges() );
        typedef map<CSeq_id_Handle, CSeq_loc_Mapper::ESeqType> TSeqTypes;
        TSeqTypes seq_types;

        string subjects_fname = CDirEntry::GetTmpName();
        string queries_fname = subjects_fname + "-q";
        string output_fname = subjects_fname + "-out";
        subjects_fname += "-s";
        try {

            //- export subjects as one FASTA file

            if (!s_locs.empty()) {
                CNcbiOfstream sub_ostr(subjects_fname.c_str());
                CFastaOstream fasta_sub_ostr(sub_ostr);

                int si = 0;
                ITERATE(TSeqLocVector, locit, subject_vec){
                    const SSeqLoc& sloc = *locit;
                    const CSeq_loc* loc = sloc.seqloc;
                    if (loc == NULL){
                        continue;
                    }

                    CScope* scope = const_cast<CScope*>(sloc.scope.GetPointer());

                    si++;
                    CBioseq_Handle handle = scope->GetBioseqHandle(*loc->GetId());
                    CSeq_loc_Mapper::ESeqType seqType = handle.IsNucleotide() ? CSeq_loc_Mapper::eSeq_nuc : CSeq_loc_Mapper::eSeq_prot;

                    if (loc->IsWhole()){
                        fasta_sub_ostr.Write(handle);

                    }
                    else {
                        fasta_sub_ostr.Write(handle, loc);
                    }

                    string seq_name = "Subject_" + NStr::IntToString(si);
                    CRef<CSeq_id> seq_id(new CSeq_id(CSeq_id::e_Local, seq_name));
                    CSeq_id_Handle local_sihd = CSeq_id_Handle::GetHandle(*seq_id);
                    seq_types[local_sihd] = seqType;

                    CSeq_id_Handle base_sihd = CSeq_id_Handle::GetHandle(*loc->GetId());
                    seq_types[base_sihd] = seqType;

                    int trcf = handle.IsNucleotide() ? 1 : 3;

                    // prepare for final mapping
                    ranges->AddConversion(
                        local_sihd,
                        0 * trcf, sequence::GetLength(*loc, scope) *trcf,
                        eNa_strand_plus,
                        base_sihd,
                        sequence::GetStart(*loc, scope) *trcf,
                        sequence::GetStrand(*loc, scope)
                        );
                }
                sub_ostr.close();
            }

            CNcbiOfstream que_ostr( queries_fname.c_str() );
            CFastaOstream fasta_que_ostr( que_ostr );

            int oi = 0;
            ITERATE( TSeqLocVector, locit, query_vec ){
                const SSeqLoc& sloc = *locit;
                const CSeq_loc* loc = sloc.seqloc;
                if( loc == NULL ){
                    continue;
                }

                CScope* scope = const_cast<CScope*>( sloc.scope.GetPointer() );

                oi++;
                CBioseq_Handle handle = scope->GetBioseqHandle(*loc->GetId());
                CSeq_loc_Mapper::ESeqType seqType = handle.IsNucleotide() ? CSeq_loc_Mapper::eSeq_nuc : CSeq_loc_Mapper::eSeq_prot;

                if( loc->IsWhole() ){
                    fasta_que_ostr.Write( handle );

                } else {
                    fasta_que_ostr.Write( handle, loc );
                }

                string seq_name = "Query_" + NStr::IntToString(oi);
				CRef<CSeq_id> seq_id( new CSeq_id( CSeq_id::e_Local, seq_name ) );
				CSeq_id_Handle local_sihd = CSeq_id_Handle::GetHandle( *seq_id );
                seq_types[local_sihd] = seqType;

				CSeq_id_Handle base_sihd = CSeq_id_Handle::GetHandle( *loc->GetId() );
                seq_types[base_sihd] = seqType;

				int trcf = handle.IsNucleotide() ? 1: 3;

                // prepare for final mapping
                ranges->AddConversion( 
                    local_sihd, 
					0 *trcf, sequence::GetLength( *loc, scope ) *trcf, 
					eNa_strand_plus,
                    base_sihd, 
                    sequence::GetStart( *loc, scope ) *trcf, 
					sequence::GetStrand( *loc, scope )
                );
            }
            que_ostr.close();


            wxString agent_path = CSysPath::GetStdPath();

            wxFileName fname( agent_path, wxEmptyString );
            #if !defined( NCBI_OS_DARWIN ) || defined( _DEBUG )
            fname.AppendDir(wxT("bin"));
            #endif
            fname.SetFullName( ToWxString( prog_name ) );

            agent_path = fname.GetFullPath();

            //- specify output format as text ASN.1
            arg_vec.push_back( "-" + string("outfmt") );
            arg_vec.push_back( "8" );

            //- specify subjects
            if (subjectType == CBLASTParams::eLocalDB) {
                arg_vec.push_back("-" + kArgDb);

                if (localDB.find(' ') != string::npos)
                    arg_vec.push_back('"' + localDB + '"');
                else
                    arg_vec.push_back(localDB);
            }
            else {
                arg_vec.push_back("-" + kArgSubject);
                arg_vec.push_back(subjects_fname);
            }

            //- specify queries
            arg_vec.push_back( "-" + kArgQuery );
            arg_vec.push_back( queries_fname );

            //- specify output
            arg_vec.push_back( "-" + kArgOutput );
            arg_vec.push_back( output_fname );

            if( IsCanceled() ){
                return;
            }

            string agent_args;
            ITERATE( vector<string>, argit, arg_vec ){
                agent_args += *argit + " ";
            }


            LOG_POST( Info << "We are about to start program on path '" << agent_path.ToUTF8() << "'" );
            LOG_POST( Info << "with parameters '" << agent_args << "'" );
			ERR_POST( Info << "Window masker path: " << blast::WindowMaskerPathGet() /*CNcbiApplication::Instance()->SetEnvironment().Get("WINDOW_MASKER_PATH")*/ );

            x_SetStatusText( string("Executing BLAST for: ") + agent_args );


            // preparing argv for SpawnV
            const char** arg_vec_ptr = new const char*[arg_vec.size() +2];
            size_t xi = 0;
            arg_vec_ptr[xi] = NULL; // it will be filled with cmdline inside SpawnV
            while( xi < arg_vec.size() ){
                const char* arg_ptr = arg_vec[xi].c_str();
                arg_vec_ptr[++xi] = arg_ptr;
            }
            arg_vec_ptr[++xi] = NULL;

            CExec::CResult proc_result = CExec::SpawnV( CExec::eNoWait, agent_path.utf8_str(), arg_vec_ptr );
            m_ProcHandle = proc_result.GetProcessHandle();

            delete[] arg_vec_ptr;

            CProcess::CExitInfo exit_info;
            CProcess( m_ProcHandle ).Wait( kInfiniteTimeoutMs, &exit_info );

            CNcbiIfstream result_istr( output_fname.c_str() );

            try {
                //- get output as ASN Seq-annot
                while( !result_istr.eof() ){
                    CRef<CSeq_annot> next_annot( new CSeq_annot() );

                    result_istr >> MSerial_AsnText >> *next_annot;

                    res_annots.push_back( next_annot );
				}
            } catch( CEofException& ){
                // it's ok

            } catch( CIOException& ioex ){
                ERR_POST(Warning 
                    << "IO error on reading BLAST output: " << ioex.GetErrCodeString()
                );
            }

            int exitcode;
            //EIO_Status status = ps.GetPipe().Close( &exitcode );
            exitcode = exit_info.GetExitCode();

			static string exitres[] = {
				"success",
				"input error",
				"database error",
				"engine error",
				"out of memory",
				"network error"
			};
			
			
			ERR_POST(
                Info << "Program " << prog_name << " completed with "
                //<< "status " << IO_StatusStr(status) << " and "
                << "exitcode " << exitcode
				<< " (" 
				<< (exitcode < sizeof(exitres)/sizeof(string) ? exitres[exitcode] : "unknown error" ) 
				<< ")"
            );

        } catch( CException& ex ){
            ERR_POST( "Failed to run standalone program " << prog_name << ": " << ex.GetMsg() );
        }
        m_ProcHandle = kInvalidProcHandle;

        try {
            //CRef<CSeq_annot> annot( new CSeq_annot() );
            //list< CRef<CSeq_align> >& aligns = annot->SetData().SetAlign();
            list< CRef<CSeq_align> > aligns;

            ITERATE(vector< CRef<CSeq_annot> >, annit, res_annots){
                CRef<CSeq_annot> next_annot( *annit );
                
                if( !next_annot.IsNull() && next_annot->IsAlign() ){
                    CSeq_loc_Mapper mapper(ranges);

                    for (const auto &i : seq_types)
                        mapper.SetSeqTypeById(i.first, i.second);

                    if (subjectType == CBLASTParams::eLocalDB) {
                        CSeq_loc_Mapper::ESeqType seqType = m_Params.IsSubjNucInput() ?
                            CSeq_loc_Mapper::eSeq_nuc : CSeq_loc_Mapper::eSeq_prot;

                        CTypeConstIterator<CSeq_id> id_iter(*next_annot);
                        for (; id_iter; ++id_iter) {
                            CSeq_id_Handle h = CSeq_id_Handle::GetHandle(*id_iter);
                            if (seq_types.find(h) == seq_types.end())
                                mapper.SetSeqTypeById(h, seqType);
                        }
                    }

                    const list< CRef<CSeq_align> > raw_aligns = next_annot->GetData().GetAlign();

                    //- translate Queries_ and Subjects_ to proper ids 
                    //- translate coords to proper offsets
                    ITERATE( list< CRef<CSeq_align> >, alnit, raw_aligns ){
                        CRef<CSeq_align> align = mapper.Map( **alnit );
                        aligns.push_back( align );
                    }
                }
            }

            if( !aligns.empty() ){
                x_SetStatusText( "Processing BLAST results..." );

                //- add results to project

                list< CRef<CSeq_align_set> > align_sets;

                if( m_Params.IsCompartmentsRequested() ){

                    FindCompartments( aligns, align_sets );
                } else {
                    CRef<CSeq_align_set> al_set( new CSeq_align_set() );
                    al_set->Set() = aligns;
                    align_sets.push_back( al_set );
                }

                NON_CONST_ITERATE( list< CRef<CSeq_align_set> >, as_itr, align_sets ){
                    // create Project items from the results
                    CRef<CSeq_annot> annot( new CSeq_annot() );
                    annot->SetData().SetAlign() = (*as_itr)->Set();

                    string label;
                    CRef<CScope> scope = m_Params.GetSeqLocs().front().scope;
                    CLabel::GetLabel( *annot, &label, CLabel::eDefault, scope );

                    label += " [" + EProgramToTaskName( m_Params.GetCurrProgram() ) + "]";

                    CRef<CProjectItem> item( new CProjectItem() );
                    item->SetItem().SetAnnot( *annot );
                    item->SetLabel( label );
                    annot->SetCreateDate( CTime(CTime::eCurrent) );
                    annot->SetTitleDesc( label );
                    annot->SetNameDesc( label );

                    if (loader)
                        AddProjectItem(*item, *loader);
                    else
                        AddProjectItem(*item);
                }

#				if _DEBUG
				string dumpname = CDirEntry::GetTmpName() + "-standalone-inproject";
				ERR_POST( Info << "Standalone in-project dumpname is: '" << dumpname << "'" );

				CNcbiOfstream dumpstream( dumpname.c_str() );
				auto_ptr<CObjectOStream> dumpasn(CObjectOStream::Open(eSerial_AsnText, dumpstream));

				ITERATE( list< CRef<CSeq_align_set> >, av_iter, align_sets ){
					CRef<objects::CSeq_align_set> aliset = *av_iter;

					*dumpasn << *aliset;
				}

				dumpasn->Close();
#				endif

            }
        } catch( CException& ex ){
            ERR_POST( "Failed to convert results of program " << prog_name << ": " << ex.GetMsg() );
        }

        CDirEntry subject_entry( subjects_fname );
        subject_entry.Remove();

        CDirEntry query_entry( queries_fname );
        query_entry.Remove();

        CDirEntry output_entry( output_fname );
        output_entry.Remove();

    } else {
        // run internal BLAST

#       ifdef _DEBUG
        blast_opts->GetOptions().DebugDumpText( cerr, "BlastOptions", 0 );
		ERR_POST( Info << "Window masker path: " << blast::WindowMaskerPathGet() /*CNcbiApplication::Instance()->SetEnvironment().Get("WINDOW_MASKER_PATH")*/ );
#       endif

        CBl2Seq  blaster( query_vec, subject_vec, *blast_opts );
        blaster.SetInterruptCallback( s_BlastInterrupt, this );

        if( IsCanceled() ){
            return;
        }
        x_SetStatusText( "Aligning sequences..." );

        TSeqAlignVector aligns = blaster.Run();


#		if _DEBUG
		string dumpname = CDirEntry::GetTmpName() + "-allapart";
		ERR_POST( Info << "All-apart dumpname is: '" << dumpname << "'" );

		CNcbiOfstream dumpstream( dumpname.c_str() );
		auto_ptr<CObjectOStream> dumpasn(CObjectOStream::Open(eSerial_AsnText, dumpstream));

		ITERATE( TSeqAlignVector, av_iter, aligns ){
			CRef<objects::CSeq_align_set> aliset = *av_iter;

			*dumpasn << *aliset;
		}

		dumpasn->Close();
#		endif

        if( !IsCanceled() && !aligns.empty() ){
            x_SetStatusText( "Processing BLAST results..." );

            x_CreateProjectItems( aligns );
        }
    }
}

void CBLASTSeqToolJob::RequestCancel()
{
    if( m_ProcHandle != kInvalidProcHandle ){
        CProcess( m_ProcHandle ).Kill();
    }

    CDataLoadingAppJob::RequestCancel();
}


void CBLASTSeqToolJob::x_CreateProjectItems( vector< CRef<CSeq_align_set> >& aligns )
{
    CRef<CSeq_annot> annot( new CSeq_annot() );
    list< CRef< CSeq_align > >& align_list = annot->SetData().SetAlign();

    NON_CONST_ITERATE( TSeqAlignVector, it, aligns ){
        CRef<CSeq_align_set> results = *it;

        ITERATE (CSeq_align_set::Tdata, align_it, results->Get()) {
            CRef<CSeq_align> align = *align_it;
            if (align->GetSegs().IsDisc()) {
                CRef<CSeq_align_set> sas( new CSeq_align_set() );
                const list<CRef<CSeq_align> >& disc = align->GetSegs().GetDisc().Get();

                sas->Set().insert(sas->Set().end(), disc.begin(), disc.end());
                align_list.insert(align_list.end(), sas->Get().begin(), sas->Get().end());
            } else {
                align_list.push_back(align);
            }
        }
    }

    if( !align_list.empty() ){
        list< CRef<CSeq_align_set> > align_sets;

        if( m_Params.IsCompartmentsRequested() ){

            FindCompartments( align_list, align_sets );
        } else {
            CRef<CSeq_align_set> al_set( new CSeq_align_set() );
            al_set->Set() = align_list;
            align_sets.push_back( al_set );
        }

        NON_CONST_ITERATE( list< CRef<CSeq_align_set> >, as_itr, align_sets ){
            // create Project items from the results
            CRef<CSeq_annot> annot( new CSeq_annot() );
            annot->SetData().SetAlign() = (*as_itr)->Set();

            string label;
            CRef<CScope> scope = m_Params.GetSeqLocs().front().scope;
            CLabel::GetLabel( *annot, &label, CLabel::eDefault, scope );

            label += ": BLAST Sequences"; // TODO

            CRef<CProjectItem> item( new CProjectItem() );
            item->SetItem().SetAnnot( *annot );
            item->SetLabel( label );
            annot->SetCreateDate( CTime(CTime::eCurrent) );
            annot->SetTitleDesc( label );
            annot->SetNameDesc( label );

            AddProjectItem(*item);
        }

#		if _DEBUG
		string dumpname = CDirEntry::GetTmpName() + "-allapart-inproject";
		ERR_POST( Info << "All-apart in-project dumpname is: '" << dumpname << "'" );

		CNcbiOfstream dumpstream( dumpname.c_str() );
		auto_ptr<CObjectOStream> dumpasn(CObjectOStream::Open(eSerial_AsnText, dumpstream));

		ITERATE( list< CRef<CSeq_align_set> >, av_iter, align_sets ){
			CRef<objects::CSeq_align_set> aliset = *av_iter;

			*dumpasn << *aliset;
		}

		dumpasn->Close();
#		endif
    }
}

END_NCBI_SCOPE
