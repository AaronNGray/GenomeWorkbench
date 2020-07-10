/*  $Id: splign_tool.cpp 33712 2015-09-09 16:18:24Z katargir $
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

#include <gui/packages/pkg_alignment/splign_tool.hpp>
#include <gui/packages/pkg_alignment/splign_panel.hpp>


#include <gui/widgets/wx/message_box.hpp>

#include <serial/typeinfo.hpp>

#include <objmgr/object_manager.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objects/seq/MolInfo.hpp>

#include <gui/objutils/user_type.hpp>
#include <gui/objutils/label.hpp>

#include <algo/align/splign/splign.hpp>
#include <algo/align/splign/splign_formatter.hpp>
#include <algo/blast/api/bl2seq.hpp>

#include <wx/msgdlg.h>

//#include <algo/align/splign/splign_simple.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);
USING_SCOPE(blast);


///////////////////////////////////////////////////////////////////////////////
/// CSplignTool
CSplignTool::CSplignTool()
:   CAlgoToolManagerBase("SPLIGN (SPliced aLIGNment)",
                         "",
                         "SPLIGN (SPliced aLIGNment)",
                         "Compute cDNA-to-genomic spliced alignments",
                         "SPLIGN",
                         "Alignment Creation"),
                         m_Panel()
{
}

string CSplignTool::GetExtensionIdentifier() const
{
    return "splign_tool";
}


string CSplignTool::GetExtensionLabel() const
{
    return "SPLIGN Tool";
}

void CSplignTool::InitUI()
{
    CAlgoToolManagerBase::InitUI();

    m_Panel = NULL;
}

void CSplignTool::CleanUI()
{
    m_Panel = NULL;
    CAlgoToolManagerBase::CleanUI();
}

void CSplignTool::x_CreateParamsPanelIfNeeded()
{
    if (m_Panel == NULL) {
        x_SelectCompatibleInputObjects();

        m_Panel = new CSplignPanel(m_ParentWindow, wxID_ANY,
                                   wxDefaultPosition, wxSize(0, 0), SYMBOL_CSPLIGNPANEL_STYLE, false);
        m_Panel->SetData(m_Params);
        m_Panel->SetObjects(&m_GenomicObjects, &m_NAObjects);

        m_Panel->SetRegistryPath(m_RegPath + ".ParamsPanel");
        m_Panel->LoadSettings();
    }
}

bool CSplignTool::x_ValidateParams()
{
    return true;
}

void CSplignTool::x_SelectCompatibleInputObjects()
{
    m_GenomicObjects.clear();
    m_NAObjects.clear();

    TObjects results;
    x_ConvertInputObjects (CSeq_loc::GetTypeInfo(), results);

    NON_CONST_ITERATE(TObjects, it, results) {
        TConstScopedObjects genomic, na;
        NON_CONST_ITERATE(TConstScopedObjects, it2, it->second) {
            const CSeq_loc* loc = dynamic_cast<const CSeq_loc*>(it2->object.GetPointer());
            if( !loc ) continue;

			if( !loc->IsInt() && !loc->IsWhole() ) continue;

            CBioseq_Handle handle = it2->scope->GetBioseqHandle(*loc->GetId());

            const CMolInfo* molInfo = 0;
            CSeqdesc_CI desc_iter(handle, CSeqdesc::e_Molinfo);
            if (desc_iter) molInfo = &desc_iter->GetMolinfo();
            if (molInfo) {
                CMolInfo::TBiomol biomol = molInfo->GetBiomol();
                switch(biomol) {
                case CMolInfo::eBiomol_genomic :
                    genomic.push_back(*it2);
                    break;
                case CMolInfo::eBiomol_unknown :
                    genomic.push_back(*it2);
                    na.push_back(*it2);
                    break;
                case CMolInfo::eBiomol_peptide :
                    break;
                default :
                    na.push_back(*it2);
                    break;
                }
            }
            else {
                genomic.push_back(*it2);
                na.push_back(*it2);
            }
        }

        if (!genomic.empty()) {
            TConstScopedObjects& vec = m_GenomicObjects[it->first];
            copy(genomic.begin(), genomic.end(), back_inserter(vec));
        }
        if (!na.empty()) {
            TConstScopedObjects& vec = m_NAObjects[it->first];
            copy(na.begin(), na.end(), back_inserter(vec));
        }
    }
}

wxPanel* CSplignTool::GetMaxPanel()
{
    CSplignPanel* panel = new CSplignPanel();
    panel->Create(m_ParentWindow);
    return panel;
}

CAlgoToolManagerParamsPanel* CSplignTool::x_GetParamsPanel()
{
    return m_Panel;
}

IRegSettings* CSplignTool::x_GetParamsAsRegSetting()
{
    return &m_Params;
}

///////////////////////////////////////////////////////////////////////////////
/// CSplignJob
class  CSplignJob  : public CDataLoadingAppJob
{
public:
    CSplignJob (const CSplignParams& params,
                const TConstScopedObjects& genomic,
                const TConstScopedObjects& cdna);
    static bool s_SplignInterruptCallback(CNWAligner::SProgressInfo* prog);
    static Boolean s_BlastInterruptCallback(SBlastProgress* prog);

protected:
    virtual void    x_CreateProjectItems(); // overriding virtual function

protected:
    CSplignParams       m_Params;
    TConstScopedObjects m_Genomic;
    TConstScopedObjects m_cDNA;
};

bool CSplignJob::s_SplignInterruptCallback(CNWAligner::SProgressInfo* prog)
{
    if (prog  &&  prog->m_data) {
        CSplignJob* job =
            reinterpret_cast<CSplignJob*>(prog->m_data);
        if(job->IsCanceled())    {
            return true;
        }
    }
    return false;
}

Boolean CSplignJob::s_BlastInterruptCallback(SBlastProgress* prog)
{
    if (prog  &&  prog->user_data) {
        CSplignJob* job =
            reinterpret_cast<CSplignJob*>(prog->user_data);
        if(job->IsCanceled())    {
            return TRUE;
        }
    }
    return FALSE;
}


CDataLoadingAppJob* CSplignTool::x_CreateLoadingJob()
{
    TConstScopedObjects genomic, cdna;

    if (m_Panel) {
        m_Params = m_Panel->GetData();
        cdna = m_Panel->GetCDNASeqs();;
        genomic.push_back(m_Panel->GetGenomicSeq());
    }
    else {
        NON_CONST_ITERATE(TObjects, it, m_GenomicObjects) {
            TConstScopedObjects& vec = m_GenomicObjects[it->first];
            copy(vec.begin(), vec.end(), back_inserter(genomic));
        }
        NON_CONST_ITERATE(TObjects, it, m_NAObjects) {
            TConstScopedObjects& vec = m_NAObjects[it->first];
            copy(vec.begin(), vec.end(), back_inserter(cdna));
        }
    }

	#ifdef _DEBUG
	m_Params.DebugDumpText( cerr, "Splign Options", 0 );
	#endif

    CSplignJob* job = new CSplignJob(m_Params, genomic, cdna);
    return job;
}

IAppTask* CSplignTool::QuickLaunch()
{
    x_SelectCompatibleInputObjects();

    if (m_GenomicObjects.empty()) {
        wxMessageBox(wxT("No genomic objects sequences can be proccessed were selected."),
                     wxT("SPLIGN Tool Error"), wxOK | wxICON_ERROR);
        return 0;
    }

    if (m_NAObjects.empty()) {
        wxMessageBox(wxT("No cDNA/Transcript sequences that can be proccessed were selected."),
                     wxT("SPLIGN Tool Error"), wxOK | wxICON_ERROR);
        return 0;
    }

    CRef<CDataLoadingAppJob> job(x_CreateLoadingJob());

    vector<int> projects;

    CProjectService* prj_srv =
        m_SrvLocator->GetServiceByType<CProjectService>();
    {{
        TConstScopedObjects objects;
        NON_CONST_ITERATE(TObjects, it, m_GenomicObjects) {
            TConstScopedObjects& vec = m_GenomicObjects[it->first];
            copy(vec.begin(), vec.end(), back_inserter(objects));
        }
        NON_CONST_ITERATE(TObjects, it, m_NAObjects) {
            TConstScopedObjects& vec = m_NAObjects[it->first];
            copy(vec.begin(), vec.end(), back_inserter(objects));
        }

        prj_srv->GetObjProjects(objects, projects);
    }}

    CSelectProjectOptions options;
    if (projects.empty())
        options.Set_CreateNewProject();
    else
        options.Set_AddToExistingProject(projects.front());

    CRef<CDataLoadingAppTask> task(new CDataLoadingAppTask(prj_srv, options, *job));
    return task.Release();
}

///////////////////////////////////////////////////////////////////////////////
/// CSplignJob
CSplignJob::CSplignJob(const CSplignParams& params,
                       const TConstScopedObjects& genomic,
                       const TConstScopedObjects& cdna)
    : m_Params(params), m_Genomic(genomic), m_cDNA(cdna)
{
    m_Descr = "SPLIGN Job"; //TODO
}

void CSplignJob::x_CreateProjectItems()
{
    ITERATE(TConstScopedObjects, it, m_Genomic) {
        const CSeq_loc& seqloc_genomic =
            dynamic_cast<const CSeq_loc&>(it->object.GetObject());

        vector<string> not_found_labels;

        NON_CONST_ITERATE(TConstScopedObjects, iter, m_cDNA) {
            if (IsCanceled()) return;

            const CSeq_loc& seqloc_cdna = dynamic_cast<const CSeq_loc&>(iter->object.GetObject());
            CScope& scope = *iter->scope;

		    CBl2Seq blast(
			    SSeqLoc(seqloc_cdna, scope),
			    SSeqLoc(seqloc_genomic, scope),
			    eMegablast
		    );

		    CConstRef<objects::CSeq_id> transcript_id( &sequence::GetId( seqloc_cdna, &scope ) );
		    CConstRef<objects::CSeq_id> genomic_id( &sequence::GetId( seqloc_genomic, &scope ) );

		    //true for EST, false for mrna
		    const bool query_low_quality = ( m_Params.GetDiscType() == 0 );

		    CSplign splign;
		    splign.SetAligner() = CSplign::s_CreateDefaultAligner( query_low_quality );
		    splign.SetAligner()->SetSpaceLimit( numeric_limits<Uint4>::max() );
		    splign.SetScope().Reset(&scope);
		    splign.PreserveScope();

            splign.SetEndGapDetection( m_Params.GetEndGapDetect() );
            splign.SetPolyaDetection( m_Params.GetPolyADetect() );
            splign.SetStrand( m_Params.GetCDNAStrand() != 1 );
            splign.SetMaxGenomicExtent( m_Params.GetMaxGenomicExtent() );
            splign.SetCompartmentPenalty( m_Params.GetCompPenalty() );
            splign.SetMinExonIdentity( m_Params.GetMinExonIdentity() );
            splign.SetMaxIntron( m_Params.GetMaxIntron() );

            /// NB: these two values should be the same, if min signleton identity is not
            /// exposed for user modification
            splign.SetMinCompartmentIdentity( m_Params.GetMinCompIdentity() );
            splign.SetMinSingletonIdentity  ( m_Params.GetMinCompIdentity() );

            // set progress interrupt callbacks for Blast and NWAligner
            {
            splign.SetAligner()->SetProgressCallback( s_SplignInterruptCallback, this );
            blast.SetInterruptCallback(s_BlastInterruptCallback, this);
            }

            //splign_simple.Run();

		    TSeqAlignVector blres( blast.Run() );

		    CSplign::THitRefs hitrefs;
		    ITERATE( TSeqAlignVector, ii, blres ){
			    if( (*ii)->IsSet() ){
				    const CSeq_align_set::Tdata &sas0 = (*ii)->Get();
				    ITERATE( CSeq_align_set::Tdata, sa_iter, sas0 ){
					    CSplign::THitRef hitref( new CSplign::THit(**sa_iter) );
					    if( hitref->GetQueryStrand() == false ){
						    hitref->FlipStrands();
					    }
					    hitrefs.push_back( hitref );
				    }
			    }
		    }

		    if( hitrefs.size() ){
			    splign.Run( &hitrefs );
		    }

		    CSplignFormatter sf( splign );
		    sf.SetSeqIds( transcript_id, genomic_id );
		    CRef<CSeq_align_set> seqaln_set( 
			    sf.AsSeqAlignSet( 0, CSplignFormatter::eAF_SplicedSegWithParts ) 
		    );

		    if( m_Params.GetCDNAStrand() == 2 ){
			    splign.SetStrand( false );

			    if( hitrefs.size() ){
				    splign.Run( &hitrefs );
			    }

			    CSplignFormatter sf( splign );
			    sf.SetSeqIds( transcript_id, genomic_id );
			    CRef<CSeq_align_set> seqaln_set_neg( 
				    sf.AsSeqAlignSet( 0, CSplignFormatter::eAF_SplicedSegWithParts ) 
			    );

			    if( !seqaln_set_neg->IsEmpty() ){
				    seqaln_set->Set().merge( seqaln_set_neg->Set() );
			    }
		    }

            typedef CSeq_annot::TData::TAlign TAlign;

            if( !seqaln_set->Get().empty() ){
                // pack the alignment in a Seq-annot and label it appropriately
                CRef<CSeq_annot> annot(new CSeq_annot());
                TAlign& seqannot_align = annot->SetData().SetAlign();
                ITERATE( CSeq_align_set::Tdata, align, seqaln_set->Get() ){
                    seqannot_align.push_back(*align);
                }

                // prepare a title
                string str;
                CLabel::GetLabel(seqloc_genomic, &str, CLabel::eDefault, &scope);
                str += " x ";
                CLabel::GetLabel(seqloc_cdna, &str, CLabel::eDefault, &scope);
                str += ": SPLIGN alignment";
                annot->SetNameDesc(str);

                annot->SetCreateDate(CurrentTime());

                /// now create a Project Item for the data
                CRef<CProjectItem> item(new CProjectItem());
                item->SetItem().SetAnnot(*annot);

                // TODO we need to generate title properly
                item->SetLabel(str);
                AddProjectItem(*item);
            }
            else {
                // no alignments - save cDNA label for info message
                string label;
                CLabel::GetLabel(seqloc_cdna, &label, CLabel::eDefault, &scope);
                not_found_labels.push_back(label);
            }
        }
    }
}


END_NCBI_SCOPE
