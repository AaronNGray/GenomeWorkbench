/*  $Id: edit_object_seq_feat.cpp 44319 2019-11-27 18:57:47Z filippov $
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
 * Authors: Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/edit/edit_object_seq_feat.hpp>

#include <gui/utils/command_processor.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_change_bioseq_inst.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/cmd_del_seq_feat.hpp>
#include <gui/objutils/cmd_del_bioseq.hpp>
#include <gui/objutils/cmd_add_seqentry.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/objutils/convert_raw_to_delta.hpp>


#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/SeqFeatXref.hpp>
#include <objects/seqfeat/RNA_ref.hpp>
#include <objects/seqfeat/Cdregion.hpp>
#include <objects/seq/Pubdesc.hpp>
#include <objects/misc/sequence_macros.hpp>

#include <objmgr/util/sequence.hpp>
#include <objmgr/feat_ci.hpp>

#include <objtools/edit/cds_fix.hpp>
#include <objtools/cleanup/cleanup.hpp>

#include <wx/treebook.h>
#include <wx/panel.h>
//#include <wx/sizer.h>
//#include <wx/bitmap.h>
//#include <wx/icon.h>
//#include <wx/choice.h>
//#include <wx/textctrl.h>
//#include <wx/stattext.h>
//#include <wx/toplevel.h>
#include <wx/hyperlink.h>
#include <wx/toplevel.h>

#include "generic_props_panel.hpp"
#include "comment_panel.hpp"
#include "location_panel.hpp"
#include "gene_panel.hpp"
#include "synonyms_panel.hpp"
#include "rna_panel.hpp"
#include "cds_exception_panel.hpp"
#include "cds_protein_panel.hpp"
#include "cds_translation_panel.hpp"
#include "protein_panel.hpp"
#include "ec_numbers_panel.hpp"
#include "protein_activities_panel.hpp"
#include "importfeature_panel.hpp"
#include <gui/widgets/edit/gbqual_panel.hpp>
#include "dbxref_panel.hpp"
#include "identifiers_panel.hpp"
#include "experiments_panel.hpp"
#include "inference_list_panel.hpp"
#include "citations_panel.hpp"
#include "loc_gcode_panel.hpp"
#include "biosourcepanel.hpp"
#include "srcmod_list_panel.hpp"
#include "src_other_pnl.hpp"
#include "featuregenepanel.hpp"
#include "region_panel.hpp"
#include "bond_panel.hpp"
#include "site_panel.hpp"
#include "experiment_list_panel.hpp"
#include "feat_comment.hpp"
#include "protein_properties.hpp"
#include <gui/widgets/edit/secondary_structure_panel.hpp>

// publication editor includes
#include <gui/widgets/edit/publicationtype_panel.hpp>
#include <gui/widgets/edit/non_ascii_replacement_dlg.hpp>

#ifdef _DEBUG
#include <serial/objostr.hpp>
#include <serial/serial.hpp>
#endif

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static const CBioseq* GetCDSProductSeq(const CSeq_feat* feat, CScope& scope)
{
    const CBioseq* protein = NULL;

    if (feat->IsSetProduct() && feat->GetProduct().GetId()) {
        CBioseq_Handle bsh = scope.GetBioseqHandle(*(feat->GetProduct().GetId()));
        if (bsh) {
            protein = bsh.GetCompleteBioseq().GetPointer();
        }
    }
    return protein;
}

    
static const CSeq_feat* GetFeatureProtein(const CSeq_feat* feat, CScope& scope)
{
    //const CProt_ref* protRef = feat->GetProtXref();
    const CSeq_feat* protein = NULL;

    if (feat->IsSetProduct()) {
        const CSeq_loc& product = feat->GetProduct();
        int dist = numeric_limits<int>::max();
        CFeat_CI it(scope, product, SAnnotSelector(CSeqFeatData::e_Prot));
        for (; it; ++it) {
            const CSeq_loc& loc = it->GetLocation();
            sequence::ECompare comp = sequence::Compare(product, loc, &scope,
                                                        sequence::fCompareOverlapping);
            if (comp == sequence::eSame) {
                protein = &it->GetOriginalFeature();
                break;
            }
            else if (comp == sequence::eContains) {
                int tmp = sequence::GetLength(product, &scope) -
                    sequence::GetLength(loc, &scope);
                if (tmp < dist) {
                    dist = tmp;
                    protein = &it->GetOriginalFeature();
                }
            }
        }
    }

    return protein;
}


void CEditObjectSeq_feat::x_AddCodingRegionProteinNamePanel(CProt_ref& prot)
{
    wxWindow* panel;

    panel = new CCDSProteinPanel(m_Window, prot, *m_Scope);
    m_Window->AddPage(panel, wxT("Protein Name and Description"));
}


void CEditObjectSeq_feat::x_AddCodingRegionProteinPanels(CProt_ref& prot, CSeq_feat* edited_protein)
{
    wxWindow* panel;

    panel = new CProteinPropertiesPanel(m_Window, prot, edited_protein);
    m_Window->AddPage(panel, wxT("Protein Properties"));
}


void ConvertRegulatoryFeature(CSeq_feat& feat)
{
    if (CSeqFeatData::IsRegulatory(feat.GetData().GetSubtype())) {
        string regulatory_class = CSeqFeatData::GetRegulatoryClass(feat.GetData().GetSubtype());
        if (!NStr::IsBlank(regulatory_class)) {
            feat.SetData().SetImp().SetKey("regulatory");
            bool has_class = false;
            NON_CONST_ITERATE(CSeq_feat::TQual, it, feat.SetQual()) {
                if ((*it)->IsSetQual() && NStr::EqualNocase((*it)->GetQual(), "regulatory_class")) {
                    (*it)->SetVal(regulatory_class);
                    has_class = true;
                    break;
                }
            }
            if (!has_class) {
                CRef<CGb_qual> q(new CGb_qual());
                q->SetQual("regulatory_class");
                q->SetVal(regulatory_class);
                feat.SetQual().push_back(q);
            }
        }
    }
}


CSeq_feat& CEditObjectSeq_feat::x_GetSeqFeatCopy()
{
    const CSeq_feat& seq_feat = dynamic_cast<const CSeq_feat&>(*m_Object);

    m_EditedFeature.Reset((CSerialObject*)CSeq_feat::GetTypeInfo()->Create());
    m_EditedFeature->Assign(seq_feat);

    CSeq_feat& edited_seq_feat = dynamic_cast<CSeq_feat&>(*m_EditedFeature);
    return edited_seq_feat;
}

wxWindow* CEditObjectSeq_feat::CreateWindow(wxWindow* parent)
{
    CSeq_feat& edited_seq_feat = x_GetSeqFeatCopy();
    return CreateWindowImpl(parent,edited_seq_feat);
}

wxWindow* CEditObjectSeq_feat::CreateWindowImpl(wxWindow* parent,CSeq_feat& edited_seq_feat)
{
    if (m_Window) {
        NCBI_THROW(CException, eUnknown, 
                   "CEditObjectSeq_feat::CreateWindow - Window already created!");
    }
    wxTopLevelWindow* gui_widget = dynamic_cast<wxTopLevelWindow*>(parent);
    IHelpUrlClient* help_client = nullptr;
    if (m_CreateMode)
        help_client = dynamic_cast<IHelpUrlClient*>(parent);

    m_Window = new wxTreebook( parent, ID_NOTEBOOK, wxDefaultPosition, wxDefaultSize, wxBK_DEFAULT );
    m_Window->Bind(wxEVT_TREEBOOK_PAGE_CHANGED, &CEditObjectSeq_feat::OnPageChange, this);

    CSeqFeatData& seq_feat_data = edited_seq_feat.SetData();
    ConvertRegulatoryFeature(edited_seq_feat);

    string sub_type = seq_feat_data.GetKey();
    if (sub_type.find("RNA") == sub_type.length()-3)
        sub_type = "RNA";

    wxWindow* panel;
    wxWindow* location = NULL;

    if (seq_feat_data.IsGene()) {
        CGene_ref& gene_ref = seq_feat_data.SetGene();

        panel = new CGenePanel(m_Window, gene_ref, *m_Scope);
        m_Window->AddPage(panel, wxT("Gene"));

        location = new CLocationPanel(m_Window, edited_seq_feat.SetLocation(), *m_Scope, true, false);
        m_Window->AddPage(location, wxT("Location"));

        panel = new CSynonymsPanel(m_Window, gene_ref, *m_Scope);
        m_Window->AddPage(panel, wxT("Synonyms"));
        if (gui_widget)
            gui_widget->SetTitle(_("Gene"));
        
        if (help_client)
            help_client->SetHelpUrl(_("https://www.ncbi.nlm.nih.gov/tools/gbench/manual10/#gene"));
    }
    else if (seq_feat_data.IsRna()) {
        panel = new CRNAPanel(m_Window, edited_seq_feat, *m_Scope);
        m_Window->AddPage(panel, wxT("RNA"));

        location = new CLocationPanel(m_Window, edited_seq_feat.SetLocation(), *m_Scope, true, false);
        m_Window->AddPage(location, wxT("Location"));
        if (gui_widget)
            gui_widget->SetTitle(_("RNA"));
        if (help_client)
        {
            if (seq_feat_data.GetSubtype() == CSeqFeatData::eSubtype_rRNA)
                help_client->SetHelpUrl(_("https://www.ncbi.nlm.nih.gov/tools/gbench/manual10/#rrna"));
            if (seq_feat_data.GetSubtype() == CSeqFeatData::eSubtype_tRNA)
                help_client->SetHelpUrl(_("https://www.ncbi.nlm.nih.gov/tools/gbench/manual10/#trna"));
            if (seq_feat_data.GetSubtype() == CSeqFeatData::eSubtype_ncRNA)
                help_client->SetHelpUrl(_("https://www.ncbi.nlm.nih.gov/tools/gbench/manual10/#ncrna"));
        }

    }
    else if (seq_feat_data.IsCdregion()) {
        if (m_CreateMode && edited_seq_feat.IsSetLocation()) {
            CBioseq_Handle bh = m_Scope->GetBioseqHandle(edited_seq_feat.GetLocation());
            CRef<CGenetic_code> code = edit::GetGeneticCodeForBioseq(bh);
            if (code) {
                seq_feat_data.SetCdregion().SetCode(*code);
            }
        }
        const CSeq_feat* protein = GetFeatureProtein(&edited_seq_feat, *m_Scope);
        CRef<CProt_ref> prot_feat;
        CSeq_feat* edited_protein = NULL;
        if (protein && protein->GetData().IsProt()) {
            m_EditedProtein.Reset(
                static_cast<CSeq_feat*>(protein->GetThisTypeInfo()->Create()));
            m_EditedProtein->Assign(*protein);

            edited_protein = dynamic_cast<CSeq_feat*>(m_EditedProtein.GetPointer());
            prot_feat = &edited_protein->SetData().SetProt();
        } else {
            bool has_xref = false;
            if (edited_seq_feat.IsSetXref()) {
                NON_CONST_ITERATE (CSeq_feat::TXref, it, edited_seq_feat.SetXref()) {
                    if ((*it)->IsSetData() && (*it)->GetData().IsProt()) {
                        prot_feat = &((*it)->SetData().SetProt());
                        has_xref = true;
                        break;
                    }
                }
            }
            if (!has_xref) {
                CRef<CSeqFeatXref> xref(new CSeqFeatXref());
                xref->SetData().SetProt();
                edited_seq_feat.SetXref().push_back(xref);
                prot_feat = &(xref->SetData().SetProt());
            }
        }
        panel = new CCDSTranslationPanel (m_Window, edited_seq_feat, *m_Scope);
        m_Window->AddPage(panel, wxT("Coding Region"));
        x_AddCodingRegionProteinNamePanel(*prot_feat);

        location = new CLocationPanel(m_Window, edited_seq_feat.SetLocation(), *m_Scope, true, false, true);
        m_Window->AddPage(location, wxT("Location"));

        x_AddCodingRegionProteinPanels(*prot_feat, edited_protein);
        panel = new CCDSExceptionPanel (m_Window, edited_seq_feat, *m_Scope);
        m_Window->AddPage(panel, wxT("Translation Exceptions"));

        panel = new CFeatCommentPanel(m_Window, _("Non-functional CDS"));
        m_Window->AddPage(panel, wxT("Misc"));

        if (gui_widget)
            gui_widget->SetTitle(_("CDS"));
        if (help_client)
            help_client->SetHelpUrl(_("https://www.ncbi.nlm.nih.gov/tools/gbench/manual10/#cds"));
    } else if (seq_feat_data.IsPub()) {
        CRef<CPubdesc> pubdesc(&(seq_feat_data.SetPub()));
        panel = new CPublicationTypePanel (m_Window, pubdesc, m_CreateMode);
        m_Window->AddPage (panel, wxT("Publication"));

        location = new CLocationPanel(m_Window, edited_seq_feat.SetLocation(), *m_Scope, true, false);
        m_Window->AddPage(location, wxT("Location"));
        if (gui_widget)
            gui_widget->SetTitle(_("Pub feature"));

    }
    else if (seq_feat_data.IsImp()) {
        panel = new CImportFeaturePanel(m_Window, edited_seq_feat);
        m_Window->AddPage(panel, wxT("Import Feature"));

        location = new CLocationPanel(m_Window, edited_seq_feat.SetLocation(), *m_Scope, true, false);
        m_Window->AddPage(location, wxT("Location"));
        if (gui_widget)
        {
            if (seq_feat_data.GetImp().IsSetKey())
                gui_widget->SetTitle(wxString(seq_feat_data.GetImp().GetKey()));
            else
                gui_widget->SetTitle(_("Imp Feature"));
        }
        if (help_client)
        {
            if (seq_feat_data.GetImp().IsSetKey() && seq_feat_data.GetImp().GetKey() == "misc_feature")
                help_client->SetHelpUrl(_("https://www.ncbi.nlm.nih.gov/tools/gbench/manual10/#misc-feature"));
        }

    }
    else if (seq_feat_data.IsProt()) {
        panel = new CProteinPanel(m_Window, edited_seq_feat.SetData().SetProt());
        m_Window->AddPage(panel, wxT("Protein Name and Description"));
        panel = new CECNumbersPanel(m_Window, edited_seq_feat.SetData().SetProt());
        m_Window->AddPage(panel, wxT("Enzyme Commission Numbers"));
        panel = new CProteinActivitiesPanel(m_Window, edited_seq_feat.SetData().SetProt());
        m_Window->AddPage(panel, wxT("Activity"));
        location = new CLocationPanel(m_Window, edited_seq_feat.SetLocation(), *m_Scope, false, true);
        m_Window->AddPage(location, wxT("Location"));
        if (gui_widget)
            gui_widget->SetTitle(_("Protein"));
    }
    else if (seq_feat_data.IsBiosrc()) {
        panel = new CBioSourcePanel(m_Window, edited_seq_feat.SetData().SetBiosrc());
        m_Window->AddPage(panel, wxT("Source"));
        
        panel = new CLocAndGCodePanel(m_Window, edited_seq_feat.SetData().SetBiosrc());
        m_Window->AddSubPage(panel, wxT("Genetic Code"));
        
//        panel = new CSrcModListPanel(m_Window, edited_seq_feat.SetData().SetBiosrc());
//        m_Window->AddSubPage(panel, wxT("Source Modifiers"));
        
//        panel = new CDbxrefPanel(m_Window, edited_seq_feat.SetData().SetBiosrc().SetOrg());
//        m_Window->AddSubPage(panel, wxT("Dbxrefs"));

        CSourceOtherPanel* other = new CSourceOtherPanel(m_Window);
        other->TransferFromOrgRef(edited_seq_feat.SetData().SetBiosrc().SetOrg());
        m_Window->AddSubPage(other, wxT("Other"));
        
        location = new CLocationPanel(m_Window, edited_seq_feat.SetLocation(), *m_Scope, true, false);
        m_Window->AddPage(location, wxT("Location"));
        if (gui_widget)
            gui_widget->SetTitle(_("BioSource"));
    }
    else if (seq_feat_data.IsRegion()) {
        panel = new CRegionPanel(m_Window, edited_seq_feat.SetData().SetRegion()); 
        m_Window->AddPage(panel, wxT("Region"));

        location = new CLocationPanel(m_Window, edited_seq_feat.SetLocation(), *m_Scope, true, false);
        m_Window->AddPage(location, wxT("Location"));
        if (gui_widget)
            gui_widget->SetTitle(_("Region"));
    }
    else if (seq_feat_data.IsBond()) {
        panel = new CBondPanel(m_Window, edited_seq_feat.SetData().SetBond()); 
        m_Window->AddPage(panel, wxT("Bond"));
        
        //panel = new CLocationPanel(m_Window, edited_seq_feat.SetLocation(), *m_Scope, true, false); // TODO - location can't seem to handle bond seq-loc
        //m_Window->AddPage(panel, wxT("Location"));
        if (gui_widget)
            gui_widget->SetTitle(_("Bond"));
    }
    else if (seq_feat_data.IsSite()) {
        panel = new CSitePanel(m_Window, edited_seq_feat.SetData().SetSite()); 
        m_Window->AddPage(panel, wxT("Site"));
        
        location = new CLocationPanel(m_Window, edited_seq_feat.SetLocation(), *m_Scope, true, false); 
        m_Window->AddPage(location, wxT("Location"));
        if (gui_widget)
            gui_widget->SetTitle(_("Site"));
    }
    else if (seq_feat_data.IsPsec_str()) {
        CSecondaryStructurePanel *sec_panel = new CSecondaryStructurePanel(m_Window);
        sec_panel->SetSecondaryStructure(seq_feat_data.GetPsec_str());
        m_Window->AddPage(sec_panel, wxT("Secondary Structure"));       
        if (gui_widget)
            gui_widget->SetTitle(_("Secondary Structure")); 
    }
    else if (seq_feat_data.IsComment()) {
        panel = new CFeatCommentPanel(m_Window, _("Enter comment in Comment page"));
        m_Window->AddPage(panel, wxT("Feature"));
        
        location = new CLocationPanel(m_Window, edited_seq_feat.SetLocation(), *m_Scope, true, false); 
        m_Window->AddPage(location, wxT("Location"));
        if (gui_widget)
            gui_widget->SetTitle(_("Comment"));
    }
    else {
        location = new CLocationPanel(m_Window, edited_seq_feat.SetLocation(), *m_Scope);
        m_Window->AddPage(location, wxT("Location"));
    }

    if (!seq_feat_data.IsGene() && !seq_feat_data.IsComment()) {
        panel = new CFeatureGenePanel(m_Window, edited_seq_feat, *m_Scope);
        m_Window->AddPage(panel, wxT("Gene"));
    }

    panel = new CCommentPanel(m_Window, edited_seq_feat);
    m_Window->AddPage(panel, wxT("Comment"));

    if (CGBQualPanel::PanelNeeded(edited_seq_feat) || seq_feat_data.IsRna()) {
        panel = new CGBQualPanel(m_Window, edited_seq_feat);
        m_Window->AddPage (panel, wxT("GenBank Qualifiers"));

        if (seq_feat_data.IsRna()) {
            CRNAPanel* rna_panel = 0;
            for (size_t i = 0; i < m_Window->GetPageCount(); ++i) {
                if ((rna_panel = dynamic_cast<CRNAPanel*>(m_Window->GetPage(i))) != 0) {
                    break;
                }
            }

            rna_panel->SetGBQualPanel(dynamic_cast<CGBQualPanel*>(panel));
        }
    }

    panel = new CGenericPropsPanel(m_Window, edited_seq_feat, *m_Scope);
    m_Window->AddPage(panel, wxT("General                         "));

    panel = new CDbxrefPanel (m_Window, edited_seq_feat);
    m_Window->AddSubPage (panel, wxT("Dbxrefs"));

    panel = new CIdentifiersPanel (m_Window, edited_seq_feat, m_Scope);
    m_Window->AddSubPage (panel, wxT("Feature Identifiers"));

#if 1
    panel = new CExperimentListPanel (m_Window, edited_seq_feat);
#else
    panel = new CExperimentsPanel (m_Window, edited_seq_feat);
#endif
    m_Window->AddSubPage (panel, wxT("Experiments"));

    panel = new CInferenceListPanel (m_Window, edited_seq_feat);
    m_Window->AddSubPage (panel, wxT("Inferences"));

    panel = new CCitationsPanel (m_Window, edited_seq_feat, *m_Scope);
    m_Window->AddSubPage (panel, wxT("Citations"));
    for (size_t i = 0; i < m_Window->GetPageCount(); ++i) {
        m_Window->GetPage(i)->TransferDataToWindow();
    }
    return m_Window;
}


bool s_ShouldSetFeaturePartial(const CSeq_loc& loc, CRef<CScope> scope)
{
    bool rval = false;
    unsigned int partial_loc  = sequence::SeqLocPartialCheck(loc, scope );
    if (( partial_loc & sequence::eSeqlocPartial_Start )
        || ( partial_loc & sequence::eSeqlocPartial_Stop )
        || ( partial_loc & sequence::eSeqlocPartial_Internal )
        || ( partial_loc & sequence::eSeqlocPartial_Other )
        || ( partial_loc & sequence::eSeqlocPartial_Nostart )
        || ( partial_loc & sequence::eSeqlocPartial_Nostop )
        || ( partial_loc & sequence::eSeqlocPartial_Nointernal)) {
        rval = true;
    }
    return rval;
}


bool s_ShouldSetGenePartialStart(const CSeq_loc& gene_loc, const CSeq_loc& cds_loc)
{
    bool rval = false;
    TSeqPos gene_start_pos = gene_loc.GetStart(eExtreme_Biological);
    TSeqPos cds_start_pos = cds_loc.GetStart(eExtreme_Biological);
    if (gene_start_pos == cds_start_pos) {
        if (!gene_loc.IsPartialStart(eExtreme_Biological)
            && cds_loc.IsPartialStart(eExtreme_Biological)) {
            rval = true;
        }
    }
    return rval;
}


bool s_ShouldSetGenePartialStop(const CSeq_loc& gene_loc, const CSeq_loc& cds_loc)
{
    bool rval = false;
    TSeqPos gene_stop_pos = gene_loc.GetStop(eExtreme_Biological);
    TSeqPos cds_stop_pos = cds_loc.GetStop(eExtreme_Biological);
    if (gene_stop_pos == cds_stop_pos) {
        if (!gene_loc.IsPartialStop(eExtreme_Biological)
            && cds_loc.IsPartialStop(eExtreme_Biological)) {
            rval = true;
        }
    }
    return rval;
}

bool CEditObjectSeq_feat::x_IsCircular(const CSeq_loc& loc)
{
    CBioseq_Handle bsh;
    try
    {
    bsh = m_Scope->GetBioseqHandle(loc);
    } catch (CObjMgrException&) {}
    if (bsh && bsh.IsSetInst_Topology() && bsh.GetInst_Topology() == CSeq_inst::eTopology_circular)
        return true;
    return false;
}

void CEditObjectSeq_feat::x_AdjustCircularGene( CRef<CSeq_feat> adjusted_gene, const CSeq_loc& new_loc)
{
    CBioseq_Handle bsh;
    try
    {
    bsh = m_Scope->GetBioseqHandle(new_loc);
    } catch (CObjMgrException&) {}
    if (!bsh)
        return;
    TSeqPos length = bsh.GetBioseqLength();
    bool origin_start = false;
    bool origin_stop = false;
    set<pair<TSeqPos, TSeqPos> > intervals;
    for (CSeq_loc_CI loc_iter(new_loc, CSeq_loc_CI::eEmpty_Skip);  loc_iter;  ++loc_iter)
    {
        CSeq_loc_CI::TRange range = loc_iter.GetRange();
        TSeqPos start = range.GetFrom();
        TSeqPos stop = range.GetTo(); 
        if (start == 0)
            origin_start = true;
        else if (stop == length - 1)
            origin_stop = true;

        if (start == 0)
        {
            start += length;
            stop += length;
        }
        intervals.insert(make_pair(start, stop));
    }
    if (origin_start && origin_stop && !intervals.empty()) // crossing origin
    {
        TSeqPos int_start = intervals.begin()->first;
        TSeqPos int_stop = intervals.rbegin()->second;
        if (int_stop > length)
        {
            int_stop %= length;
            int_start %= length;
            CRef<CSeq_loc> before_origin(new CSeq_loc);
            before_origin->SetInt().SetFrom(int_start);
            before_origin->SetInt().SetTo(length - 1);
            before_origin->SetInt().SetId().Assign(*new_loc.GetId());
            CRef<CSeq_loc> after_origin(new CSeq_loc);
            after_origin->SetInt().SetFrom(0);
            after_origin->SetInt().SetTo(int_stop);
            after_origin->SetInt().SetId().Assign(*new_loc.GetId());
            adjusted_gene->SetLocation().SetMix().Set().clear();
            adjusted_gene->SetLocation().SetMix().Set().push_back(before_origin);
            adjusted_gene->SetLocation().SetMix().Set().push_back(after_origin);
        }
        else
        {
            int_start %= length;
            adjusted_gene->SetLocation().SetInt().SetFrom(int_start);
            adjusted_gene->SetLocation().SetInt().SetTo(int_stop);
            adjusted_gene->SetLocation().SetInt().SetId().Assign(*new_loc.GetId());
        }        
    }
    else
    {
        set<ENa_strand> strands;
        for (CSeq_loc_CI loc_iter(new_loc);  loc_iter;  ++loc_iter) 
        {
            if (loc_iter.IsSetStrand())
                strands.insert(loc_iter.GetStrand());
            else
                strands.insert(eNa_strand_plus);
        }
        if (strands.size() > 1)
        {
            adjusted_gene->SetLocation().Assign(new_loc);
        }
        else
        {
            adjusted_gene->SetLocation().SetInt().SetFrom(new_loc.GetStart(eExtreme_Positional));
            adjusted_gene->SetLocation().SetInt().SetTo(new_loc.GetStop(eExtreme_Positional));
            adjusted_gene->SetLocation().SetInt().SetId().Assign(*new_loc.GetId());
        }
    }
}

CIRef<IEditCommand> CEditObjectSeq_feat::x_AdjustGene(const CSeq_feat& old_gene,
                                                      const CSeq_loc& old_loc,  const CSeq_loc& new_loc, CRef<CSeq_feat> updated_gene)
{
    bool update_gene_span = true;
    
    CCDSTranslationPanel* cdsProductPanel = NULL;
    for (size_t i = 0; i < m_Window->GetPageCount(); ++i) {
        cdsProductPanel = dynamic_cast<CCDSTranslationPanel*>(m_Window->GetPage(i));
        if (cdsProductPanel) {
            update_gene_span = cdsProductPanel->ShouldUpdateGeneLocation();
            break;
        }
    }
    if (!update_gene_span && updated_gene->Equals(old_gene))
        return CIRef<IEditCommand>(NULL);
    // may need to adjust location of overlapping gene
    sequence::ECompare before_comp = sequence::Compare(old_gene.GetLocation(), old_loc, m_Scope, sequence::fCompareOverlapping);
    sequence::ECompare after_comp = sequence::Compare(old_gene.GetLocation(), new_loc, m_Scope, sequence::fCompareOverlapping);
    sequence::ECompare new_comp = sequence::Compare(updated_gene->GetLocation(), new_loc, m_Scope, sequence::fCompareOverlapping);

    CRef<CSeq_feat> adjusted_gene(new CSeq_feat());
    adjusted_gene->Assign(*updated_gene);

    if ((!updated_gene->Equals(old_gene) && new_comp != sequence::eSame && new_comp != sequence::eContains) ||
        ((before_comp != sequence::eNoOverlap) &&
        ((before_comp == sequence::eSame && after_comp != sequence::eSame)
        || (after_comp != sequence::eSame && after_comp != sequence::eContains)
        || s_ShouldSetGenePartialStart(old_gene.GetLocation(), new_loc)
         || s_ShouldSetGenePartialStop(old_gene.GetLocation(), new_loc)))) {        

        if (x_IsCircular(old_gene.GetLocation()))
        {
            x_AdjustCircularGene(adjusted_gene, new_loc);
        }
        else
        {
            set<ENa_strand> strands;
        set<CSeq_id_Handle> idhs;
            for (CSeq_loc_CI loc_iter(new_loc);  loc_iter;  ++loc_iter) 
            {
                if (loc_iter.IsSetStrand())
                    strands.insert(loc_iter.GetStrand());
                else
                    strands.insert(eNa_strand_plus);
        idhs.insert(loc_iter.GetSeq_id_Handle());
            }
            if (strands.size() > 1 || idhs.size() > 1)
            {
                adjusted_gene->SetLocation().Assign(new_loc);
            }
            else
            {
        const CSeq_id* id = new_loc.GetId();
        if (!id)
            return CIRef<IEditCommand>(NULL);
                adjusted_gene->SetLocation().SetInt().SetFrom(new_loc.GetStart(eExtreme_Positional));
                adjusted_gene->SetLocation().SetInt().SetTo(new_loc.GetStop(eExtreme_Positional));
                adjusted_gene->SetLocation().SetInt().SetId().Assign(*id);
            }
        }
        if (s_ShouldSetGenePartialStart(adjusted_gene->GetLocation(), new_loc)) {
            adjusted_gene->SetLocation().SetPartialStart(true, eExtreme_Biological);
            adjusted_gene->SetPartial(true);
        }
        if (s_ShouldSetGenePartialStop(adjusted_gene->GetLocation(), new_loc)) {
            adjusted_gene->SetLocation().SetPartialStop(true, eExtreme_Biological);
            adjusted_gene->SetPartial(true);
        }
    }
                    
    if (!adjusted_gene->Equals(old_gene)) {
        CSeq_feat_Handle fh = m_Scope->GetSeq_featHandle(old_gene);
        CIRef<IEditCommand> chgFeat(new CCmdChangeSeq_feat(fh, *adjusted_gene));
        return chgFeat;
    }
    
    return CIRef<IEditCommand>(NULL);
}


const char* kTransSplicing = "trans-splicing";


void CEditObjectSeq_feat::x_AdjustLocation(CSeq_feat& edited_seq_feat)
{
    CLocationPanel* locPanel = 0;
    for (size_t i = 0; i < m_Window->GetPageCount(); ++i) {
        locPanel = dynamic_cast<CLocationPanel*>(m_Window->GetPage(i));
        if (locPanel)
            break;
    }
    if (locPanel) {
        CRef<CSeq_loc> loc = locPanel->GetSeq_loc();
        edited_seq_feat.SetLocation(*loc);
        // note - fields from generic properties panel have already been set
        // if feature was deliberately flagged as partial, will have already been set, otherwise will have been reset
        if (s_ShouldSetFeaturePartial(*loc, m_Scope)) {
            edited_seq_feat.SetPartial(true);
        }
    }
}

void CEditObjectSeq_feat::x_ConvertCommentToMiscFeat(CSeq_feat& edited_seq_feat)
{
    CFeatCommentPanel* panel = 0;
    for (size_t i = 0; i < m_Window->GetPageCount(); ++i) {
        panel = dynamic_cast<CFeatCommentPanel*>(m_Window->GetPage(i));
        if (panel)
            break;
    }
    if (panel && panel->GetConvertToMiscFeat() && edited_seq_feat.IsSetData())
    {
        if (edited_seq_feat.GetData().IsComment()) 
        {
            edited_seq_feat.ResetData();
            edited_seq_feat.SetData().SetImp().SetKey("misc_feature");        
        }       
    }
}


void CEditObjectSeq_feat::x_SetGbQuals(CSeq_feat& edited_seq_feat)
{
    // GenBank quals
    CGBQualPanel* qualPanel = 0;
    for (size_t i = 0; i < m_Window->GetPageCount(); ++i) {
        qualPanel = dynamic_cast<CGBQualPanel*>(m_Window->GetPage(i));
        if (qualPanel)
            break;
    }
    if (qualPanel) {
        qualPanel->PopulateGBQuals(edited_seq_feat);
    }
}


void CEditObjectSeq_feat::x_SetCrossRefs(CSeq_feat& edited_seq_feat)
{
    // Cross-refs
    CDbxrefPanel* xrefPanel = 0;
    for (size_t i = 0; i < m_Window->GetPageCount(); ++i) {
        xrefPanel = dynamic_cast<CDbxrefPanel*>(m_Window->GetPage(i));
        if (xrefPanel) {
            xrefPanel->PopulateDbxrefs(edited_seq_feat);
            break;
        }
    }
}


void CEditObjectSeq_feat::x_SetIdentifiers(CSeq_feat& edited_seq_feat)
{
    // identifiers
    CIdentifiersPanel* idPanel = nullptr;
    for (size_t i = 0; i < m_Window->GetPageCount(); ++i) {
        idPanel = dynamic_cast<CIdentifiersPanel*>(m_Window->GetPage(i));
        if (idPanel)
            break;
    }
    if (idPanel) {
        idPanel->PopulateIdentifiers(edited_seq_feat);
    }
}


void CEditObjectSeq_feat::x_SetInferences(CSeq_feat& edited_seq_feat)
{

    // inferences
    CInferenceListPanel* inferencePanel = 0;
    for (size_t i = 0; i < m_Window->GetPageCount(); ++i) {
        inferencePanel = dynamic_cast<CInferenceListPanel*>(m_Window->GetPage(i));
        if (inferencePanel)
            break;
    }
    if (inferencePanel) {
        inferencePanel->PopulateGBQuals(edited_seq_feat);
    }
}


void CEditObjectSeq_feat::x_SetExperiments(CSeq_feat& edited_seq_feat)
{

    // inferences
    CExperimentListPanel* experimentsPanel = 0;
    for (size_t i = 0; i < m_Window->GetPageCount(); ++i) {
        experimentsPanel = dynamic_cast<CExperimentListPanel*>(m_Window->GetPage(i));
        if (experimentsPanel)
            break;
    }
    if (experimentsPanel) {
        experimentsPanel->PopulateGBQuals(edited_seq_feat);
    }
}
    

void CEditObjectSeq_feat::x_SetCitations(CSeq_feat& edited_seq_feat)
{
    // citations
    CCitationsPanel* citationsPanel = 0;
    for (size_t i = 0; i < m_Window->GetPageCount(); ++i) {
        citationsPanel = dynamic_cast<CCitationsPanel*>(m_Window->GetPage(i));
        if (citationsPanel)
            break;
    }
    if (citationsPanel) {
        citationsPanel->SetCitations(edited_seq_feat);
    }
}


CConstRef<CSeq_feat> s_GetmRNAForCDS(const CSeq_feat& cds, const CSeq_feat& orig_cds, CScope& scope)
{
    // note - if the edited feature refers to an mRNA by feature ID, use that feature
    bool has_xref = false;
    CConstRef<CSeq_feat> mrna;
    if (cds.IsSetXref()) {
        CBioseq_Handle bsh = scope.GetBioseqHandle(*(cds.GetLocation().GetId()));
        CTSE_Handle tse = bsh.GetTSE_Handle();
        FOR_EACH_SEQFEATXREF_ON_SEQFEAT (it, cds) {
            if ((*it)->IsSetId() && (*it)->GetId().IsLocal() && (*it)->GetId().GetLocal().IsId()) {
                CSeq_feat_Handle mrna_h = tse.GetFeatureWithId(CSeqFeatData::eSubtype_mRNA, (*it)->GetId().GetLocal().GetId());
                if (mrna_h) {
                    mrna = mrna_h.GetSeq_feat();
                }
                has_xref = true;
            }
        }
    }

    if (!has_xref) {
        mrna = GetBestOverlappingFeat(
        orig_cds.GetLocation(), // note - using original location to find mRNA
        CSeqFeatData::eSubtype_mRNA,
        sequence::eOverlap_CheckIntRev,
        scope);
    }
    return mrna;
}


CIRef<IEditCommand> CEditObjectSeq_feat::x_UpdatemRNA(const CSeq_feat& cds)
{
    if (!cds.IsSetData() || !cds.GetData().IsCdregion() || !cds.IsSetLocation()) {
        return CIRef<IEditCommand>(NULL);
    }
    // find original overlapping mRNA, if editing a coding region
    bool update_mrna_span = false;
    bool update_mrna_product = false;

    CCDSProteinPanel* cdsProteinPanel = 0;
    CCDSTranslationPanel* cdsProductPanel = 0;
    CLocationPanel *locPanel(NULL);
    for (size_t i = 0; i < m_Window->GetPageCount(); ++i) {
        if (!cdsProteinPanel) {
            cdsProteinPanel = dynamic_cast<CCDSProteinPanel*>(m_Window->GetPage(i));
        }
        if (!cdsProductPanel) {
            cdsProductPanel = dynamic_cast<CCDSTranslationPanel*>(m_Window->GetPage(i));
        }
        if (!locPanel) {
            locPanel = dynamic_cast<CLocationPanel*>(m_Window->GetPage(i));
        }

        if (cdsProteinPanel && cdsProductPanel && locPanel) {
            break;
        }
    }
    if (cdsProteinPanel) {
        update_mrna_product = cdsProteinPanel->ShouldUpdatemRNAProduct();
    }
    if (cdsProductPanel) {
        update_mrna_span = cdsProductPanel->ShouldUpdatemRNALocation();
    }
    if (locPanel) {
        update_mrna_span |= locPanel->ShouldUpdatemRNALocation();
    }

    if (update_mrna_product || update_mrna_span) {
        const CSeq_feat& orig_cds = dynamic_cast<const CSeq_feat&>(*m_Object);
        CConstRef<CSeq_feat> mrna = s_GetmRNAForCDS(cds, orig_cds, *m_Scope);

        if ( mrna ) {
            CRef<CSeq_feat> new_mrna(new CSeq_feat());
            new_mrna->Assign(*mrna);
            bool any_change = false;

            if (update_mrna_product) {
                // update product
                string new_prot_product = "";
                if (m_EditedProtein) {
                    CSeq_feat& edited_protein = dynamic_cast<CSeq_feat&>(*m_EditedProtein);
                    if ( edited_protein.GetData().GetProt().IsSetName()
                            && edited_protein.GetData().GetProt().GetName().size() > 0) {
                        new_prot_product = edited_protein.GetData().GetProt().GetName().front();
                    }
                }              
                string orig_mrna_product = "";
                if (mrna->GetData().GetRna().IsSetExt() && mrna->GetData().GetRna().GetExt().IsName()) {
                    orig_mrna_product = mrna->GetData().GetRna().GetExt().GetName();
                }
                if (!NStr::Equal(new_prot_product, orig_mrna_product) 
                    || !mrna->GetData().GetRna().IsSetExt()
                    || !mrna->GetData().GetRna().GetExt().IsName()) {
                    new_mrna->SetData().SetRna().SetExt().SetName(new_prot_product);
                    any_change = true;
                }
            }

            if (update_mrna_span) {
                // adjust location
                sequence::ECompare orig_comp = sequence::Compare(orig_cds.GetLocation(), mrna->GetLocation(), m_Scope, sequence::fCompareOverlapping);
                sequence::ECompare new_comp = sequence::Compare(cds.GetLocation(), new_mrna->GetLocation(), m_Scope, sequence::fCompareOverlapping);
                if (orig_comp == sequence::eSame || new_comp != sequence::eContained) {
                    new_mrna->SetLocation().Assign(cds.GetLocation());
                    bool make_partial = s_ShouldSetFeaturePartial(new_mrna->GetLocation(), m_Scope);
                    if (make_partial) {
                        new_mrna->SetPartial(true);
                    } else {
                        new_mrna->ResetPartial();
                    }
                    any_change = true;
                }
            }

            if (any_change) {
                CSeq_feat_Handle fh = m_Scope->GetSeq_featHandle(*mrna);
                CIRef<IEditCommand> chgFeat(new CCmdChangeSeq_feat(fh, *new_mrna));
                return chgFeat;
            }
        }
    }
    return CIRef<IEditCommand>(NULL);            
}


void CEditObjectSeq_feat::x_SetTranslExcept(CSeq_feat& cds)
{
    if (!cds.IsSetData() || !cds.GetData().IsCdregion()) {
        return;
    }

    CCDSExceptionPanel *transl_except_panel = 0;
    for (size_t i = 0; i < m_Window->GetPageCount(); ++i) {
        transl_except_panel = dynamic_cast<CCDSExceptionPanel*>(m_Window->GetPage(i));
        if (transl_except_panel)
            break;
    }
    if (transl_except_panel) {
        transl_except_panel->SetExceptions(cds.SetData().SetCdregion());
    }

}

CRef<CCmdComposite> CEditObjectSeq_feat::x_HandleCdsProduct(const CSeq_feat &old_cds, CSeq_feat& cds)
{
    CRef<CCmdComposite> composite(new CCmdComposite("Handle CDS Product"));
    bool any_actions = false;
    const CBioseq * product_seq = GetCDSProductSeq(&cds, *m_Scope);
    const CSeq_feat* orig_protein = 0;
    if (product_seq) {
        orig_protein = GetFeatureProtein(&cds, *m_Scope);
    }

    CFeatCommentPanel* misc_panel = 0;
    for (size_t i = 0; i < m_Window->GetPageCount(); ++i) {
        misc_panel = dynamic_cast<CFeatCommentPanel*>(m_Window->GetPage(i));
        if (misc_panel)
            break;
    }
    if (misc_panel && misc_panel->GetConvertToMiscFeat())
    {
        cds.ResetProduct();
        cds.SetData().SetImp().SetKey("misc_feature");     
        if (orig_protein) 
        {
            CSeq_feat_Handle fh = m_Scope->GetSeq_featHandle(*orig_protein);
            CIRef<IEditCommand> delFeat(new CCmdDelSeq_feat(fh));
            composite->AddCommand(*delFeat);
            any_actions = true;
        }
        if (product_seq) 
        {
            CBioseq_Handle bh = m_Scope->GetBioseqHandle(*product_seq);
            CIRef<IEditCommand> delInst(new CCmdDelBioseqInst(bh));
            composite->AddCommand(*delInst);       
            any_actions = true;
        }
        if (!any_actions) {
            composite.Reset(NULL);
        }
        return composite;
    }

    CCDSTranslationPanel* transPanel = 0;
    for (size_t i = 0; i < m_Window->GetPageCount(); ++i) {
        transPanel = dynamic_cast<CCDSTranslationPanel*>(m_Window->GetPage(i));
        if (transPanel)
            break;
    }
    if (transPanel) { 
        bool ShouldCreateNewProductSeq = transPanel->ShouldCreateNewProductSeq();
        if (transPanel->ShouldRetranslate()) {
            transPanel->Retranslate();
        }

        if (product_seq && !ShouldCreateNewProductSeq) {
            // adjust existing product
            CBioseq_Handle bh = m_Scope->GetBioseqHandle(*product_seq);
            CRef<CBioseq> edited_product(new CBioseq());
            edited_product->Assign(*product_seq);
            transPanel->SetProtein(*edited_product);
            if (edited_product->IsSetInst() && edited_product->GetInst().IsSetRepr()
                && edited_product->GetInst().GetRepr() != CSeq_inst::eRepr_not_set)
            {
                CIRef<IEditCommand> chgInst(new CCmdChangeBioseqInst(bh, edited_product->GetInst()));
                composite->AddCommand(*chgInst);       
                CRef<CCmdComposite> chgPartials = GetSynchronizeProductMolInfoCommand(*m_Scope, cds);
                if (chgPartials) {
                    composite->AddCommand(*chgPartials);
                    any_actions = true;
                }
                try
                {
                    NRawToDeltaSeq::RemapOtherProtFeats(old_cds, cds, bh, composite, any_actions);
                } catch(const CUtilException&) {}
                if (orig_protein) {
                    // adjust existing protein feature
                    CSeq_feat& edited_protein = dynamic_cast<CSeq_feat&>(*m_EditedProtein);
                    edited_protein.SetLocation().SetInt().SetId().Assign(*(edited_product->GetId().front()));
                    edited_protein.SetLocation().SetInt().SetFrom(0);
                    edited_protein.SetLocation().SetInt().SetTo(edited_product->GetLength() - 1);
                    edit::AdjustProteinFeaturePartialsToMatchCDS(edited_protein, cds);
                    CCleanup cleanup;
                    cleanup.BasicCleanup(edited_protein);
                    
                    CSeq_feat_Handle fh = m_Scope->GetSeq_featHandle(*orig_protein);
                    CIRef<IEditCommand> chgFeat(new CCmdChangeSeq_feat(fh, edited_protein));
                    composite->AddCommand(*chgFeat);
                    any_actions = true;
                } else {
                    // create new protein feature
                    bool cds_change = false;
                    CRef<CCmdComposite> create_prot = CreateOrAdjustProteinFeature(bh, cds, cds_change);
                    if (create_prot) {
                        composite->AddCommand(*create_prot);
                        any_actions = true;
                    }
                }
            }
            else
            {
                // delete product
                if (orig_protein) 
                {
                    CSeq_feat_Handle fh = m_Scope->GetSeq_featHandle(*orig_protein);
                    CIRef<IEditCommand> delFeat(new CCmdDelSeq_feat(fh));
                    composite->AddCommand(*delFeat);
                }
                CIRef<IEditCommand> delInst(new CCmdDelBioseqInst(bh));
                composite->AddCommand(*delInst);    
                // adjust or add prot-xref
                CSeq_feat& edited_protein = dynamic_cast<CSeq_feat&>(*m_EditedProtein);
                bool found_existing = false;
                if (cds.IsSetXref()) {
                    NON_CONST_ITERATE(CSeq_feat::TXref, it, cds.SetXref()) {
                        if ((*it)->IsSetData() && (*it)->GetData().IsProt()) {
                            (*it)->SetData().SetProt().Assign(edited_protein.GetData().GetProt());
                            found_existing = true;
                            break;
                        }
                    }
                }
                if (!found_existing) {
                    CRef<CSeqFeatXref> prot_xref(new CSeqFeatXref());
                    prot_xref->SetData().SetProt().Assign(edited_protein.GetData().GetProt());
                    cds.SetXref().push_back(prot_xref);
                }
                any_actions = true;
            }
        } else if (ShouldCreateNewProductSeq) {
            // create new product sequence
            CRef<CBioseq> new_product(new CBioseq());
            transPanel->SetProtein(*new_product);
            if (new_product->IsSetInst() && new_product->GetInst().IsSetRepr()
                && new_product->GetInst().GetRepr() != CSeq_inst::eRepr_not_set) {
                CBioseq_Handle bsh;
                for (CSeq_loc_CI loc_iter(cds.GetLocation(), CSeq_loc_CI::eEmpty_Skip);  loc_iter;  ++loc_iter)
                {
                    try {
                        bsh = m_Scope->GetBioseqHandle(loc_iter.GetSeq_id());
                    } catch (CObjMgrException&) {}
                    if (bsh)
                        break;
                }
                CRef<CSeq_id> product_id;
                if (!new_product->IsSetId() || new_product->GetId().empty()) {                   
                    CSeq_entry_Handle seh = bsh.GetTopLevelEntry();
                    product_id = transPanel->GetProductId();
                    if (!product_id) {
                        int offset = 1;
                        string id_label;
                        bool create_general_only = objects::edit::IsGeneralIdProtPresent(m_SEH.GetTopLevelEntry());
                        product_id = edit::GetNewProtId(bsh, offset, id_label, create_general_only);
                    }
                    new_product->SetId().push_back(product_id);
                }
                else {
                    product_id = new_product->SetId().front();
                }

                string label;
                product_id->GetLabel(&label, CSeq_id::eFasta);
                transPanel->SetProductId(label);
                transPanel->TransferDataFromWindow();

                CRef<CSeq_entry> prot_entry (new CSeq_entry());
                prot_entry->SetSeq(*new_product);
                bool partial5 = cds.GetLocation().IsPartialStart(eExtreme_Biological);
                bool partial3 = cds.GetLocation().IsPartialStop(eExtreme_Biological);
                SetMolinfoForProtein (prot_entry, partial5, partial3);
                CRef<CSeq_feat> prot = AddEmptyProteinFeatureToProtein(prot_entry, partial5, partial3);
                if (orig_protein) {
                    CRef<CSeq_loc> keep_loc(new CSeq_loc());
                    keep_loc->Assign(prot->GetLocation());
                    prot->Assign(*orig_protein);
                    prot->SetLocation().Assign(*keep_loc);
                    if (partial5 || partial3) {
                        prot->SetPartial(true);
                    }
                } else if (cds.IsSetXref()) {
                    NON_CONST_ITERATE (CSeq_feat::TXref, it, cds.SetXref()) {
                        if ((*it)->IsSetData() && (*it)->GetData().IsProt()) {
                            prot->SetData().SetProt().Assign((*it)->GetData().GetProt());
                            cds.SetXref().erase(it);
                            break;
                        }
                    }
                    if (cds.GetXref().empty()) {
                        cds.ResetXref();
                    }
                }

                CCleanup cleanup;
                cleanup.BasicCleanup(*prot);    

                if (product_seq) {
                    CBioseq_Handle old_prot = m_Scope->GetBioseqHandle(*product_seq);
                    CRef<CCmdDelBioseqInst> subcmd(new CCmdDelBioseqInst(old_prot));
                    composite->AddCommand(*subcmd);
                }

                CRef<CCmdAddSeqEntry> subcmd(new CCmdAddSeqEntry(prot_entry, bsh.GetParentEntry()));
                composite->AddCommand(*subcmd);                

                any_actions = true;
            }            
        }
        transPanel->SetProduct(cds);
    }
     

    if (!any_actions) {
        composite.Reset(NULL);
    }

    return composite;
}

bool CEditObjectSeq_feat::CanClose()
 {
     //if (m_CreateMode)
     {
         CSeq_feat& edited_feat = dynamic_cast<CSeq_feat&>(*m_EditedFeature);
         CSeqFeatData::TQualifiers mandatory_vec = CSeqFeatData::GetMandatoryQualifiers(edited_feat.GetData().GetSubtype());
         set<CSeqFeatData::EQualifier> mandatory(mandatory_vec.begin(), mandatory_vec.end());
         if (edited_feat.IsSetQual()) 
         {
             for (CSeq_feat::TQual::iterator qual = edited_feat.SetQual().begin();  qual != edited_feat.SetQual().end(); ++qual) 
             {
                 if ((*qual)->IsSetQual() && !(*qual)->GetQual().empty() && (*qual)->IsSetVal() && !(*qual)->GetVal().empty())
                     mandatory.erase(CSeqFeatData::GetQualifierType((*qual)->GetQual())); 
             }
         }

         // some "qualifiers" are represented elsewhere
         if (edited_feat.GetData().IsRna() &&
             edited_feat.GetData().GetRna().IsSetExt() &&
             edited_feat.GetData().GetRna().GetExt().IsGen() &&
             edited_feat.GetData().GetRna().GetExt().GetGen().IsSetClass() &&
             !NStr::IsBlank(edited_feat.GetData().GetRna().GetExt().GetGen().GetClass())) {
             mandatory.erase(CSeqFeatData::eQual_ncRNA_class);
         }
     
         if (!mandatory.empty())
         {
             vector<string> txt;
             for (auto q : mandatory)
             {
                 txt.push_back("\"" + CSeqFeatData::GetQualifierAsString(q) + "\"");
             }
             wxString msg;
             msg << "This feature is missing mandatory qualifier";
             if (txt.size() > 1)
                 msg << "s";
             msg << " ";
             if (txt.size() > 2)
                 msg << NStr::Join(txt, ", ");
             else
                 msg << NStr::Join(txt, " and ");
             msg << ". Would you like to continue?";
             int answer = wxMessageBox(msg, ToWxString("Warning"), wxYES_NO | wxICON_QUESTION);
             if (answer != wxYES)
             {
                 if (m_Window)
                 {
                     m_Window->ChangeSelection(0);                    
                 }
                 return false;
             }
         }
     }
     return true;
 }

IEditCommand* CEditObjectSeq_feat::GetEditCommand()
{
    if (!m_Window)
        return 0;

    if (!m_EditAction) {

        TestForNonAsciiText(m_Window);

        for (size_t i = 0; i < m_Window->GetPageCount(); ++i) {
            if (!m_Window->GetPage(i)->TransferDataFromWindow()) {
                return 0;
            }
        }
        CRef<CCmdComposite> composite(new CCmdComposite("Edit Feature"));

        const CSeq_feat& seq_feat = dynamic_cast<const CSeq_feat&>(*m_Object);
        CSeq_feat& edited_seq_feat = dynamic_cast<CSeq_feat&>(*m_EditedFeature);

        x_AdjustLocation(edited_seq_feat);       
        x_ConvertCommentToMiscFeat(edited_seq_feat);

        if (!m_CreateMode) {
            CSeq_feat_Handle fh;
            try {
                fh = m_Scope->GetSeq_featHandle(seq_feat);
            } catch (const CException&) {
                fh = GetSeqFeatHandleForBadLocFeature(seq_feat, *m_Scope);
            }
            CIRef<IEditCommand> chgFeat(new CCmdChangeSeq_feat(fh, edited_seq_feat));
            composite->AddCommand(*chgFeat);
        }

        // set data for publication feature
        CPublicationTypePanel* pubPanel = 0;
        for (size_t i = 0; i < m_Window->GetPageCount(); ++i) {
            pubPanel = dynamic_cast<CPublicationTypePanel*>(m_Window->GetPage(i));
            if (pubPanel)
                break;
        }

        if (pubPanel) {
            // copy to data
            CRef<CPubdesc> pub = pubPanel->GetPubdesc();
            edited_seq_feat.SetData().SetPub(*pub);
        }

        // import feature
        CImportFeaturePanel* impPanel = 0;
        for (size_t i = 0; i < m_Window->GetPageCount(); ++i) {
            impPanel = dynamic_cast<CImportFeaturePanel*>(m_Window->GetPage(i));
            if (impPanel)
                break;
        }

        if (impPanel) {
            impPanel->PopulateImpFeat (edited_seq_feat);
        }
        //biosource
        bool is_biosrc = false;
        CBioSourcePanel* biosource_panel = 0;
        for (size_t i = 0; i < m_Window->GetPageCount(); ++i) {
            biosource_panel = dynamic_cast<CBioSourcePanel*>(m_Window->GetPage(i));
            if (biosource_panel)
                break;
        }
        if (biosource_panel) {
                  biosource_panel->PopulateBioSource(edited_seq_feat.SetData().SetBiosrc());
            is_biosrc = true;
        }
        if (is_biosrc) {
            CSourceOtherPanel* other_panel = 0;
            for (size_t i = 0; i < m_Window->GetPageCount(); ++i) {
                other_panel = dynamic_cast<CSourceOtherPanel*>(m_Window->GetPage(i));
                if (other_panel) {
                    break;
                }
            }
            if (other_panel) {
                other_panel->TransferToOrgRef(edited_seq_feat.SetData().SetBiosrc().SetOrg());
            }

            CLocAndGCodePanel* loc_gc_panel = 0;
            for (size_t i = 0; i < m_Window->GetPageCount(); ++i) {
                loc_gc_panel = dynamic_cast<CLocAndGCodePanel*>(m_Window->GetPage(i));
                if (loc_gc_panel)
                    break;
            }
            if (loc_gc_panel) {
                loc_gc_panel->PopulateLocAndGC(edited_seq_feat.SetData().SetBiosrc());
            }
            CCleanup cleanup;
            cleanup.BasicCleanup(edited_seq_feat.SetData().SetBiosrc());
        }

        x_SetGbQuals(edited_seq_feat);
        x_SetCrossRefs(edited_seq_feat);
        x_SetIdentifiers(edited_seq_feat);
        x_SetInferences(edited_seq_feat);
        x_SetExperiments(edited_seq_feat); // TODO restores gb-quals edited before
        x_SetCitations(edited_seq_feat);

        CRegionPanel* regionPanel = 0;
        for (size_t i = 0; i < m_Window->GetPageCount(); ++i) {
            regionPanel = dynamic_cast<CRegionPanel*>(m_Window->GetPage(i));
            if (regionPanel)
                break;
        }

        if (regionPanel) {
            // copy to data
            string region = regionPanel->GetRegion();
            if (regionPanel->GetConvertToMiscFeat())
            {
                edited_seq_feat.ResetData();
                edited_seq_feat.SetData().SetImp().SetKey("misc_feature");
                if (edited_seq_feat.IsSetComment())
                {
                    if (!region.empty())
                        region += "; ";
                    region += edited_seq_feat.GetComment();
                }
                edited_seq_feat.SetComment(region);
            }       
            else
            {
                edited_seq_feat.SetData().SetRegion(region);
            }
        }

        CBondPanel* bondPanel = 0;
        for (size_t i = 0; i < m_Window->GetPageCount(); ++i) {
            bondPanel = dynamic_cast<CBondPanel*>(m_Window->GetPage(i));
            if (bondPanel)
                break;
        }

        if (bondPanel) {
            // copy to data
            CSeqFeatData::EBond bond = bondPanel->GetBond();
            edited_seq_feat.SetData().SetBond(bond);
            }
        
        CSitePanel* sitePanel = 0;
        for (size_t i = 0; i < m_Window->GetPageCount(); ++i) {
            sitePanel = dynamic_cast<CSitePanel*>(m_Window->GetPage(i));
            if (sitePanel)
                break;
        }
        
        if (sitePanel) {
            // copy to data
            CSeqFeatData::ESite site = sitePanel->GetSite();
            edited_seq_feat.SetData().SetSite(site);
        }

        // Secondary Structure features
        CSecondaryStructurePanel* secondary_struc_panel = 0;
        for (size_t i = 0; i < m_Window->GetPageCount(); ++i) {
            secondary_struc_panel = dynamic_cast<CSecondaryStructurePanel*>(m_Window->GetPage(i));
            if (secondary_struc_panel)
                break;
        }
        
        if (secondary_struc_panel) {
            // copy to data
            CSeqFeatData::EPsec_str struc = secondary_struc_panel->GetSecondaryStructure();
            edited_seq_feat.SetData().SetPsec_str(struc);
        }

        CGenericPropsPanel* generic_props_panel = nullptr;
        for (size_t i = 0; i < m_Window->GetPageCount(); ++i) {
            generic_props_panel = dynamic_cast<CGenericPropsPanel*>(m_Window->GetPage(i));
            if (generic_props_panel)
                break;
        }

        bool modify_pseudo = false;
        string pseudo_choice;
        if (generic_props_panel && generic_props_panel->PseudoSelectionChanged()) {
            modify_pseudo = true;
            pseudo_choice = generic_props_panel->GetPseudoChoice();
        }

        // some generic cleanup
        if (edited_seq_feat.IsSetQual() && edited_seq_feat.GetQual().empty()) {
            edited_seq_feat.ResetQual();
        }

        CCleanup cleanup;
        cleanup.BasicCleanup(edited_seq_feat);

        // find original overlapping mRNA, if editing a coding region
        CIRef<IEditCommand> chgmRNA = x_UpdatemRNA(edited_seq_feat);
        if (chgmRNA) {
            composite->AddCommand(*chgmRNA);
        }

        CFeatureGenePanel* addedGenePanel = 0;
        CRef<CSeq_feat> new_gene;
        CConstRef<CSeq_feat> old_gene;
        CRef<CSeq_feat> updated_gene;
        for (size_t i = 0; i < m_Window->GetPageCount(); ++i) {
            addedGenePanel = dynamic_cast<CFeatureGenePanel*>(m_Window->GetPage(i));
            if (addedGenePanel) {
                addedGenePanel->SetModifyPseudo(modify_pseudo, pseudo_choice);
                new_gene = addedGenePanel->GetNewGene();
                old_gene = addedGenePanel->GetGeneToEdit();
                updated_gene = addedGenePanel->GetUpdatedGene();
                break;
            }
        }

        // copy trans-splicing exception from coding region to gene if present
        if (new_gene && edited_seq_feat.GetData().IsCdregion()
            && edited_seq_feat.IsSetExcept_text()
            && NStr::Find(edited_seq_feat.GetExcept_text(), kTransSplicing) != string::npos) {
            new_gene->SetExcept(true);
            new_gene->SetExcept_text(kTransSplicing);
        }

        try {
            if (!new_gene && !m_CreateMode && old_gene) {
                // may need to adjust location of overlapping gene
                CIRef<IEditCommand> change_gene = x_AdjustGene(*old_gene, seq_feat.GetLocation(), edited_seq_feat.GetLocation(), updated_gene);
                if (change_gene) {
                    composite->AddCommand(*change_gene);
                }
            } else if (new_gene) {
                CBioseq_Handle bsh;
                for (CSeq_loc_CI loc_iter(new_gene->GetLocation(), CSeq_loc_CI::eEmpty_Skip);  loc_iter;  ++loc_iter)
                {
                    try
                    {
                        bsh = m_SEH.GetScope().GetBioseqHandle(loc_iter.GetSeq_id());
                    } catch (CObjMgrException&) {}
                    if (bsh)
                        break;
                }
                CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
                CIRef<IEditCommand> addFeat(new CCmdCreateFeat(seh, *new_gene));
                composite->AddCommand(*addFeat);
            }

            if (edited_seq_feat.IsSetData() && edited_seq_feat.GetData().IsCdregion()) {
                x_SetTranslExcept(edited_seq_feat);
                CRef<CCmdComposite> handle_product = x_HandleCdsProduct(seq_feat, edited_seq_feat);
                if (handle_product) {
                    composite->AddCommand(*handle_product);
                }
            }
            
            if (m_CreateMode) {
                CIRef<IEditCommand> addFeat(new CCmdCreateFeat(m_SEH, edited_seq_feat));
                composite->AddCommand(*addFeat);
            }

            m_EditAction.Reset(composite);
        }
        catch (CException& e) {
            LOG_POST(Error << "CEditObjectSeq_feat::GetEditAction(): " << e.GetMsg());
        }

 

#if 0
#ifdef _DEBUG
    CNcbiOstrstream ostr;
    {{
        auto_ptr<CObjectOStream>
            out(CObjectOStream::Open(eSerial_AsnText, ostr));
        *out << edited_seq_feat;
    }}
    _TRACE(string(CNcbiOstrstreamToString(ostr)));
    //LOG_POST(Info << string(CNcbiOstrstreamToString(ostr)));
#endif
#endif

    }
    return m_EditAction.GetPointer();
}

void CEditObjectSeq_feat::OnPageChange(wxBookCtrlEvent& event)
{
    CImportExportContainer *parent = nullptr;

    wxPanel* panel_parent = dynamic_cast<wxPanel*>(m_Window->GetParent());
    if (panel_parent) {
        parent = dynamic_cast<CImportExportContainer*>(panel_parent->GetParent());
    }
    else {
        parent = dynamic_cast<CImportExportContainer*>(m_Window->GetParent());
    }

    _ASSERT(parent);
    CImportExportAccess *child = dynamic_cast<CImportExportAccess*>(m_Window->GetCurrentPage());
    bool enable_import = false;
    bool enable_export = false;
    if (child)
    {
        enable_import = child->IsImportEnabled();
        enable_export = child->IsExportEnabled();
    }
    parent->EnableImport(enable_import);
    parent->EnableExport(enable_export);
}


END_NCBI_SCOPE
