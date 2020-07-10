/*  $Id: prosplign_tool.cpp 35788 2016-06-23 19:51:18Z asztalos $
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
* Authors:  Andrea Asztalos
*
*/

#include <ncbi_pch.hpp>
#include <objects/seq/MolInfo.hpp>
#include <objects/seq/Annot_descr.hpp>
#include <objects/seq/Annotdesc.hpp>
#include <objects/seq/Align_def.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/util/seq_loc_util.hpp>

#include <algo/blast/core/blast_def.h>
#include <algo/blast/api/bl2seq.hpp>
#include <algo/blast/api/tblastn_options.hpp>
#include <algo/blast/blastinput/blast_args.hpp>
#include <algo/blast/blastinput/tblastn_args.hpp>
#include <algo/align/nw/nw_aligner.hpp>

#include <algo/align/prosplign/prosplign.hpp>
#include <algo/align/prosplign/prosplign_exception.hpp>
#include <algo/align/prosplign/compartments.hpp>

#include <gui/objutils/label.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/packages/pkg_alignment/prosplign_tool.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


///////////////////////////////////////////////////////////////////////////////
/// CProSplignJob
class  CProSplignJob : public CDataLoadingAppJob
{
public:
    CProSplignJob(const CProSplignParams& params,
        const TConstScopedObjects& protein,
        const TConstScopedObjects& nucleotide);

    static bool s_ProSplignInterruptCallback(void* user_data);
    static Boolean s_BlastInterrupt(SBlastProgress* prog);

protected:
    virtual void    x_CreateProjectItems(); // overriding virtual function

    CRef<CSeq_align> x_RunProSplign(const CSeq_id& protein_id, const CSeq_loc& genomic_loc);

    prosplign::TCompartments x_GetCompartments(const CSeq_loc& protein_loc, const CSeq_loc& genomic_loc);
    
    blast::TSeqAlignVector x_SetupAndRuntBlastn(const CSeq_loc& protein_loc, const CSeq_loc& genomic_loc);

    prosplign::TCompartments x_GenerateCompartments(const blast::TSeqAlignVector& blast_results);

    void x_FilterCompartments(prosplign::TCompartments& comparts);

    CRef<CSeq_align> x_ProcessCompartment(CRef<CSeq_annot> compartment);

    void x_CreateOneProjectItem(const vector<CRef<CSeq_align>>& align_result, const CSeq_loc& protein_loc, const CSeq_loc& genomic_loc);

protected:
    CProSplignParams    m_Params;
    TConstScopedObjects m_Protein;
    TConstScopedObjects m_NA;
    CRef<CScope>        m_Scope;
};


///////////////////////////////////////////////////////////////////////////////
/// CProSplignTool
CProSplignTool::CProSplignTool()
:   CAlgoToolManagerBase("ProSPLIGN",
                         "",
                         "ProSPLIGN",
                         "Compute protein-to-genomic spliced alignments",
                         "ProSPLIGN",
                         "Alignment Creation"),
                         m_Panel()
{
}

void CProSplignTool::InitUI()
{
    CAlgoToolManagerBase::InitUI();

    m_Panel = NULL;
}

void CProSplignTool::CleanUI()
{
    m_Panel = NULL;
    CAlgoToolManagerBase::CleanUI();
}

wxPanel* CProSplignTool::GetMaxPanel()
{
    CSplignPanel* panel = new CSplignPanel();
    panel->Create(m_ParentWindow);
    return panel;
}

string CProSplignTool::GetExtensionIdentifier() const
{
    return "prosplign_tool";
}


string CProSplignTool::GetExtensionLabel() const
{
    return "ProSPLIGN Tool";
}

CAlgoToolManagerParamsPanel* CProSplignTool::x_GetParamsPanel()
{
    return m_Panel;
}

IRegSettings* CProSplignTool::x_GetParamsAsRegSetting()
{
    return &m_Params;
}

void CProSplignTool::x_CreateParamsPanelIfNeeded()
{
    if (m_Panel == NULL) {
        x_SelectCompatibleInputObjects();

        m_Panel = new CProSplignPanel(m_ParentWindow, wxID_ANY, wxDefaultPosition, 
                                    wxSize(0, 0), SYMBOL_CPROSPLIGNPANEL_STYLE, false);
        m_Panel->SetData(m_Params);
        m_Panel->SetObjects(&m_ProteinObjects, &m_NAObjects);

        m_Panel->SetRegistryPath(m_RegPath + ".ParamsPanel");
        m_Panel->LoadSettings();
    }
}

bool CProSplignTool::x_ValidateParams()
{
    return true;
}

CDataLoadingAppJob* CProSplignTool::x_CreateLoadingJob()
{
    TConstScopedObjects protein, na;

    _ASSERT(m_Panel);
    if (m_Panel) {
        m_Params = m_Panel->GetData();
        na = m_Panel->GetNucleotideSeqs();
        protein.push_back(m_Panel->GetProteinSeq());
    }
    else {
        NON_CONST_ITERATE(TObjects, it, m_ProteinObjects) {
            TConstScopedObjects& vec = m_ProteinObjects[it->first];
            copy(vec.begin(), vec.end(), back_inserter(protein));
        }
        NON_CONST_ITERATE(TObjects, it, m_NAObjects) {
            TConstScopedObjects& vec = m_NAObjects[it->first];
            copy(vec.begin(), vec.end(), back_inserter(na));
        }
    }

#ifdef _DEBUG
    m_Params.DebugDumpText(cerr, "ProSplign options", 0);
#endif

    CProSplignJob* job = new CProSplignJob(m_Params, protein, na);
    return job;
}

void CProSplignTool::x_SelectCompatibleInputObjects()
{
    m_ProteinObjects.clear();
    m_NAObjects.clear();

    TObjects results;
    x_ConvertInputObjects (CSeq_loc::GetTypeInfo(), results);

    NON_CONST_ITERATE(TObjects, it, results) {
        TConstScopedObjects protein, na;
        NON_CONST_ITERATE(TConstScopedObjects, it2, it->second) {
            const CSeq_loc* loc = dynamic_cast<const CSeq_loc*>(it2->object.GetPointer());
            if( !loc ) continue;

            if( !loc->IsInt() && !loc->IsWhole() ) continue;

            CBioseq_Handle bsh = it2->scope->GetBioseqHandle(*loc->GetId());
            if (bsh.IsAa()) {
                protein.push_back(*it2);
            }
            else if (bsh.IsNa()) {
                na.push_back(*it2);
            }
        }

        if (!protein.empty()) {
            TConstScopedObjects& vec = m_ProteinObjects[it->first];
            copy(protein.begin(), protein.end(), back_inserter(vec));
        }
        if (!na.empty()) {
            TConstScopedObjects& vec = m_NAObjects[it->first];
            copy(na.begin(), na.end(), back_inserter(vec));
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
/// CProSplignJob
CProSplignJob::CProSplignJob(const CProSplignParams& params,
    const TConstScopedObjects& protein,
    const TConstScopedObjects& na)
    : m_Params(params), m_Protein(protein), m_NA(na)
{
    m_Descr = "ProSPLIGN Job";
    if (!m_Protein.empty()) {
        m_Scope = m_Protein[0].scope;
    }
}

bool CProSplignJob::s_ProSplignInterruptCallback(void* user_data)
{
    if (user_data) {
        CProSplignJob* job =
            reinterpret_cast<CProSplignJob*>(user_data);
        if(job->IsCanceled())    {
            return TRUE;
        }
    }
    return FALSE;
}

/// static callback for BLAST interruption
/// this must match the API in CBl2Seq and BLAST
Boolean CProSplignJob::s_BlastInterrupt(SBlastProgress* prog)
{
    if (prog && prog->user_data){
        CProSplignJob* job =
            reinterpret_cast<CProSplignJob*>(prog->user_data)
            ;
        if (job->IsCanceled()){
            return TRUE;
        }
    }
    return FALSE;
}

void CProSplignJob::x_CreateProjectItems()
{
    // obtain the protein ID
    const CSeq_loc& protein_loc = dynamic_cast<const CSeq_loc&>(m_Protein[0].object.GetObject());
    CConstRef<CSeq_id> protein_id(&sequence::GetId(protein_loc, m_Scope));
    if (!protein_id) {
        return;
    }

    const int kCutoff = 9;

    ITERATE(TConstScopedObjects, it, m_NA) {
        if (IsCanceled()) return;

        const CSeq_loc& na_loc = dynamic_cast<const CSeq_loc&>(it->object.GetObject());

        CSeq_loc orig_genomic_loc;
        if (na_loc.IsWhole()) {
            CRef<CSeq_id> seqid(new CSeq_id);
            seqid->Assign(*na_loc.GetId());
            orig_genomic_loc.SetInt().SetId(*seqid);
            orig_genomic_loc.SetInt().SetFrom(0);
            orig_genomic_loc.SetInt().SetTo(sequence::GetLength(*seqid, m_Scope) - 1);
        }
        else {
            _ASSERT(na_loc.IsInt());
            SerialAssign(orig_genomic_loc, na_loc);
        }

        if (m_Params.m_Strand == 0) {
            orig_genomic_loc.SetStrand(eNa_strand_plus);
        }
        else if (m_Params.m_Strand == 1) {
            orig_genomic_loc.SetStrand(eNa_strand_minus);
        }
        else if (m_Params.m_Strand == 2) {
            orig_genomic_loc.SetStrand(eNa_strand_unknown);
        }

        if (IsCanceled()) return;

        vector<CRef<CSeq_align>> align_result;
        LOG_POST(Info << "Generating compartments");

        prosplign::TCompartments comparts = x_GetCompartments(protein_loc, orig_genomic_loc);
        if (IsCanceled() || comparts.empty()) return;

        for (auto&& it : comparts) {
#ifdef _DEBUG
            LOG_POST(Info << "Compartment:\n" << MSerial_AsnText << it);
#endif
            CRef<CSeq_align> align = x_ProcessCompartment(it);
            if (align) {
                align_result.push_back(align);
            }
        }
        
        x_CreateOneProjectItem(align_result, protein_loc, na_loc);
    }
}

namespace {

    const int kMaxCompartCount = 25;

    struct SKey
    {
        CSeq_id_Handle protein;
        double bit_score;
    };

    typedef pair<SKey, CRef<CSeq_annot>> TItem;
    typedef deque<TItem> TAlignInfo;

    struct SKey_Less
    {
        bool operator() (const TItem& item1, const TItem& item2) const
        {
            if (item1.first.protein < item2.first.protein) {
                return true;
            }
            if (item2.first.protein < item1.first.protein) {
                return false;
            }
            return (item1.first.bit_score > item2.first.bit_score);
        }
    };

    struct SExtractor
    {
        SKey operator() (const CSeq_annot& compartment) const
        {
            SKey key;

            bool found_protein = false;
            bool found_score = false;
            ITERATE(CSeq_annot::TDesc::Tdata, desc_it, compartment.GetDesc().Get()) {
                const CAnnotdesc& desc = **desc_it;
                switch (desc.Which()) {
                case CAnnotdesc::e_Align:
                    // protein id
                    if (desc.GetAlign().GetIds().size() != 1) {
                        NCBI_THROW(CException, eUnknown, "Unexpected number of IDs in align-ref");
                    }

                    key.protein = CSeq_id_Handle::GetHandle(*desc.GetAlign().GetIds().front());
                    found_protein = true;
                    break;
                case CAnnotdesc::e_User:
                    // get the scores
                {{
                    const char* kBitScore = "bit_score";

                    if (NStr::EqualNocase(desc.GetUser().GetType().GetStr(), "Compart Scores")) {
                        const CUser_field& field = desc.GetUser().GetField(kBitScore);
                        if (field.IsSetData() && field.GetData().IsReal()) {
                            key.bit_score = field.GetData().GetReal();
                            found_score = true;
                        }
                    }
                }}
                break;
                default:
                    break;
                }
            }

            if (!found_score) {
                LOG_POST(Error << "Compartment without bit_score\n " << MSerial_AsnText << compartment);
                NCBI_THROW(CException, eUnknown, "Failed to find scores in compartment");
            }

            if (!found_protein) {
                LOG_POST(Error << "Compartment without protein\n " << MSerial_AsnText << compartment);
                NCBI_THROW(CException, eUnknown, "Failed to find protein in compartment");
            }

            return key;
        }
    };

}  // namespace

prosplign::TCompartments CProSplignJob::x_GetCompartments(const CSeq_loc& protein_loc, const CSeq_loc& genomic_loc)
{
    
    blast::TSeqAlignVector blast_results = x_SetupAndRuntBlastn(protein_loc, genomic_loc);
    if (IsCanceled() || blast_results.empty()) return prosplign::TCompartments();

    x_SetStatusText("Processing BLAST results...");

    prosplign::TCompartments comparts = x_GenerateCompartments(blast_results);
    if (IsCanceled() || comparts.empty()) return prosplign::TCompartments();

    // filter compartment hits
    if (comparts.size() > kMaxCompartCount) {
        x_FilterCompartments(comparts);
    }

    return comparts;
}

blast::TSeqAlignVector CProSplignJob::x_SetupAndRuntBlastn(const CSeq_loc& protein_loc, const CSeq_loc& genomic_loc)
{
    USING_SCOPE(blast);

    SSeqLoc query(protein_loc, *m_Scope);
    SSeqLoc subject(genomic_loc, *m_Scope);
    EProgram prg = eTblastn;

    //Running: tblastn -comp_based_stats 0 -db_soft_mask 100 -evalue 0.001 -matrix BLOSUM80 -seg yes -soft_masking true -threshold 12

    CRef<CTBlastnOptionsHandle> opts_hndl(new CTBlastnOptionsHandle);
    opts_hndl->SetCompositionBasedStats(eNoCompositionBasedStats);      // -comp_basded_stats 0
    opts_hndl->SetEvalueThreshold(0.001);                               // -evalue 0.001
    opts_hndl->SetMatrixName("BLOSUM80");                               // -matrix BLOSUM80
    opts_hndl->SetSegFiltering(true);                                   // -seg yes
    opts_hndl->SetMaskAtHash(true);                                     // -soft_masking true
    opts_hndl->SetWordThreshold(12);                                    // -threshold 12

    // option -db_soft_mask 100 -will be ignored as it only applies when one is using a BLAST db that contains masking

#ifdef _DEBUG
    opts_hndl->GetOptions().DebugDumpText(NcbiCout, "tBLASTn Options", 5);
#endif

    TSeqAlignVector results;
    try {

        opts_hndl->Validate();
        CBl2Seq blaster(query, subject, *opts_hndl);
        blaster.SetInterruptCallback(s_BlastInterrupt, this);

        x_SetStatusText("Aligning sequences...");
        results = blaster.Run();
    }
    catch (const CBlastException& e) {
        LOG_POST(Error << e.GetMsg());
    }

    return results;
}

prosplign::TCompartments CProSplignJob::x_GenerateCompartments(const blast::TSeqAlignVector& blast_results)
{
    vector<CRef<CSeq_align>> align_list;

    ITERATE(blast::TSeqAlignVector, it, blast_results){
        CRef<CSeq_align_set> results = *it;
        ITERATE(CSeq_align_set::Tdata, align_it, results->Get()) {

            CRef<CSeq_align> align = *align_it;
            if (align->GetSegs().IsStd()) {
                align_list.push_back(align);
            }
            else {
                LOG_POST(Error << "Result from tBlastn is not a Seq-align.segs.std alignment");
                LOG_POST(Error << "Source alignment is:\n" << MSerial_AsnText << *align);
                return prosplign::TCompartments();
            }
        }
    }

    if (align_list.empty()) {
        return prosplign::TCompartments();
    }

    CSplign::THitRefs hit_refs;
    NON_CONST_ITERATE(vector<CRef<CSeq_align>>, iter, align_list) {
        CSeq_align& align = **iter;
        try {
            CRef<CBlastTabular> hitref(new CBlastTabular(align, false));
            hit_refs.push_back(hitref);
        }
        catch (const CException& e) {
            LOG_POST(Error << "Error producing compartments: " << e.what());
            LOG_POST(Error << "Source alignment is:\n" << MSerial_AsnText << align);
            return prosplign::TCompartments();
        }
    }

    prosplign::CCompartOptions compart_opts;
    prosplign::TCompartments comparts = prosplign::SelectCompartmentsHits(hit_refs, compart_opts);
    return comparts;
}

void CProSplignJob::x_FilterCompartments(prosplign::TCompartments& comparts)
{
    // extract relevant information from each compartment
    function<SKey(const CSeq_annot&)> extractor = SExtractor();

    TAlignInfo alignment_info;
    ITERATE(prosplign::TCompartments, compart_it, comparts) {
        SKey key = extractor(**compart_it);
        alignment_info.emplace_back(key, *compart_it);
    }

    // sort them based on the stored information in structure SKey

    function<bool(const TItem&, const TItem&)> sorter = SKey_Less();
    sort(alignment_info.begin(), alignment_info.end(), sorter);


    // take the top N from this sorted container
    prosplign::TCompartments sorted_comparts;
    for (size_t index = 0; index < alignment_info.size() && index < kMaxCompartCount; ++index) {
        sorted_comparts.emplace_back(alignment_info[index].second);
    }

    comparts.swap(sorted_comparts);
}

CRef<CSeq_align> CProSplignJob::x_ProcessCompartment(CRef<CSeq_annot> compartment)
{
    CSeq_id_Handle protein;
    CConstRef<CSeq_loc> new_genomic_loc;
    ITERATE(CSeq_annot::TDesc::Tdata, desc_it, compartment->GetDesc().Get()) {
        const CAnnotdesc& desc = **desc_it;
        switch (desc.Which()) {
        case CAnnotdesc::e_Align:
            // protein id
            if (desc.GetAlign().GetIds().size() != 1) {
                NCBI_THROW(CException, eUnknown, "Unexpected number of IDs in align-ref");
            }

            protein = CSeq_id_Handle::GetHandle(*desc.GetAlign().GetIds().front());
            break;
        case CAnnotdesc::e_Region:
            // genomic region
            new_genomic_loc.Reset(&desc.GetRegion());
            break;
        default:
            break;
        }
    }

    if (!protein) {
        NCBI_THROW(CException, eUnknown, "Failed to find protein id");
    }
    if (!new_genomic_loc) {
        NCBI_THROW(CException, eUnknown, "Failed to find genomic location");
    }

    // run ProSplign on this compartment
    return x_RunProSplign(*protein.GetSeqId(), *new_genomic_loc);
}

CRef<CSeq_align> CProSplignJob::x_RunProSplign(const CSeq_id& protein_id, const CSeq_loc& genomic_loc)
{
    CProSplign proSplign(*m_Params.GetScoringParams(), !m_Params.m_WithIntrons);
    // by default ProSplign looks for introns (intronless = false)

    if (m_Params.m_GeneticCode > -1 &&
        m_Params.m_GeneticCode != 0 &&
        m_Params.m_GeneticCode < 50) {
        proSplign.AssignGeneticCode(*m_Scope, *genomic_loc.GetId(), m_Params.m_GeneticCode);
    }

    // set progress interrupt callback for ProSplign
    proSplign.SetInterruptCallback(s_ProSplignInterruptCallback, this);

    CRef<CSeq_align> align;
    try {
        align = proSplign.FindAlignment(*m_Scope, protein_id, genomic_loc, *m_Params.GetOutputOptions());
    }
    catch (const CProSplignException& e) {
        if (e.GetErrCode() == CProSplignException::eUserInterrupt) {
            return align;
        }
        else {
            ERR_POST(e.GetMsg());
            return align;
        }
    }
    catch (const CException& e) {
        ERR_POST(e.GetMsg());
        return align;
    }
    catch (const exception& e) {
        ERR_POST(e.what());
        return align;
    }

    return align;
}

void CProSplignJob::x_CreateOneProjectItem(const vector<CRef<CSeq_align>>& align_result,
    const CSeq_loc& protein_loc, const CSeq_loc& na_loc)
{
    if (align_result.empty()) return;

    CRef<CSeq_annot> annot(new CSeq_annot);
    CSeq_annot::TData::TAlign& seqannot_align = annot->SetData().SetAlign();
    for (auto&& align_it : align_result) {
        if (align_it->GetSegs().IsSpliced() &&
            align_it->GetSegs().GetSpliced().IsSetExons() &&
            !align_it->GetSegs().GetSpliced().GetExons().empty()) {
            seqannot_align.push_back(align_it);
        }
    }

    if (seqannot_align.empty()) return;

    // prepare a title
    string str;
    CLabel::GetLabel(na_loc, &str, CLabel::eDefault, m_Scope);
    str += " x ";
    CLabel::GetLabel(protein_loc, &str, CLabel::eDefault, m_Scope);
    str += ": ProSPLIGN alignment";
    annot->SetNameDesc(str);

    annot->SetCreateDate(CurrentTime());

    // create a Project Item for the data
    CRef<CProjectItem> item(new CProjectItem());
    item->SetItem().SetAnnot(*annot);

    item->SetLabel(str);
    AddProjectItem(*item);
}

END_NCBI_SCOPE
