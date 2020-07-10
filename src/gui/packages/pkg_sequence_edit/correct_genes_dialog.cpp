/*  $Id: correct_genes_dialog.cpp 45101 2020-05-29 20:53:24Z asztalos $
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
 * Authors:  Colleen Bollin
 */


#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include <objmgr/util/seq_loc_util.hpp>
#include <objmgr/feat_ci.hpp>
#include <gui/core/selection_service_impl.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/packages/pkg_sequence_edit/editing_actions.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_constraint.hpp>
#include <gui/packages/pkg_sequence_edit/constraint_panel.hpp>
#include <gui/packages/pkg_sequence_edit/aecr_frame.hpp>
#include <gui/packages/pkg_sequence_edit/correct_genes_dialog.hpp>
#include <gui/widgets/edit/generic_report_dlg.hpp>  
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <objtools/edit/text_object_description.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

USING_SCOPE(ncbi::objects);


/*!
 * CCorrectGenesDialog type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CCorrectGenesDialog, CBulkCmdDlg )


/*!
 * CCorrectGenesDialog event table definition
 */

BEGIN_EVENT_TABLE( CCorrectGenesDialog, CBulkCmdDlg )

////@begin CCorrectGenesDialog event table entries
////@end CCorrectGenesDialog event table entries

END_EVENT_TABLE()


/*!
 * CCorrectGenesDialog constructors
 */

CCorrectGenesDialog::CCorrectGenesDialog()
{
    Init();
}

CCorrectGenesDialog::CCorrectGenesDialog( wxWindow* parent, IWorkbench* wb, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
: CBulkCmdDlg(wb)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CCorrectGenesDialog creator
 */

bool CCorrectGenesDialog::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CCorrectGenesDialog creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CBulkCmdDlg::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CCorrectGenesDialog creation
    return true;
}


/*!
 * CCorrectGenesDialog destructor
 */

CCorrectGenesDialog::~CCorrectGenesDialog()
{
////@begin CCorrectGenesDialog destruction
////@end CCorrectGenesDialog destruction
}


/*!
 * Member initialisation
 */

void CCorrectGenesDialog::Init()
{
////@begin CCorrectGenesDialog member initialisation
    m_MatchInterval = NULL;
    m_MatchStrand = NULL;
    m_Log = NULL;
    m_FeatureChoice = NULL;
    m_OnlySelected = NULL;
    m_Constraint = NULL;
    m_OkCancel = NULL;
////@end CCorrectGenesDialog member initialisation
}


/*!
 * Control creation for CCorrectGenesDialog
 */

void CCorrectGenesDialog::CreateControls()
{    
////@begin CCorrectGenesDialog content construction
    CCorrectGenesDialog* itemCBulkCmdDlg1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCBulkCmdDlg1->SetSizer(itemBoxSizer2);

    m_MatchInterval = new wxCheckBox( itemCBulkCmdDlg1, ID_CHECKBOX10, _("Set gene interval to match"), wxDefaultPosition, wxDefaultSize, 0 );
    m_MatchInterval->SetValue(true);
    itemBoxSizer2->Add(m_MatchInterval, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_MatchStrand = new wxCheckBox( itemCBulkCmdDlg1, ID_CHECKBOX11, _("Set gene strand to match"), wxDefaultPosition, wxDefaultSize, 0 );
    m_MatchStrand->SetValue(true);
    itemBoxSizer2->Add(m_MatchStrand, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Log = new wxCheckBox( itemCBulkCmdDlg1, ID_CHECKBOX12, _("Log gene changes"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Log->SetValue(false);
    itemBoxSizer2->Add(m_Log, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString m_FeatureChoiceStrings;
    m_FeatureChoiceStrings.Add(_("&CDS"));
    m_FeatureChoiceStrings.Add(_("&mRNA"));
    m_FeatureChoiceStrings.Add(_("&ncRNA"));
    m_FeatureChoiceStrings.Add(_("&rRNA"));
    m_FeatureChoiceStrings.Add(_("&tRNA"));
    m_FeatureChoice = new wxRadioBox( itemCBulkCmdDlg1, ID_RADIOBOX3, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_FeatureChoiceStrings, 1, wxRA_SPECIFY_ROWS );
    m_FeatureChoice->SetSelection(0);
    itemBoxSizer2->Add(m_FeatureChoice, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_OnlySelected = new wxCheckBox( itemCBulkCmdDlg1, ID_CHECKBOX13, _("Correct only selected feature-gene pair"), wxDefaultPosition, wxDefaultSize, 0 );
    m_OnlySelected->SetValue(false);
    itemBoxSizer2->Add(m_OnlySelected, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    GetTopLevelSeqEntryAndProcessor();
    m_Constraint = new CConstraintPanel( itemCBulkCmdDlg1, m_TopSeqEntry);
    itemBoxSizer2->Add(m_Constraint, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_OkCancel = new COkCancelPanel( itemCBulkCmdDlg1, ID_WINDOW14, wxDefaultPosition, wxSize(100, 100), 0 );
    itemBoxSizer2->Add(m_OkCancel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CCorrectGenesDialog content construction
    m_Constraint->SetSelection(CAECRFrame::eFieldType_CDSGeneProt);
}


/*!
 * Should we show tooltips?
 */

bool CCorrectGenesDialog::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CCorrectGenesDialog::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CCorrectGenesDialog bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CCorrectGenesDialog bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CCorrectGenesDialog::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CCorrectGenesDialog icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CCorrectGenesDialog icon retrieval
}


CRef<CCmdComposite> CCorrectGenesDialog::GetCommand()
{
    CRef<CCmdComposite> cmd(NULL);
    m_Error = "";

    CScope& scope = m_TopSeqEntry.GetScope();

    string report_msg = "";
    // which features to correct?
    if (m_OnlySelected->GetValue()) {
        vector<CConstRef<CSeq_feat> > list = x_GetSelectedPair();
        if (!list.empty()) {
            CBioseq_Handle bsh = scope.GetBioseqHandle(list[0]->GetLocation());
            string seq_id = "";
            bsh.GetCompleteBioseq()->GetLabel(&seq_id, CBioseq::eContent);
            CRef<CSeq_feat> new_gene(new CSeq_feat());
            new_gene->Assign(*list[0]);
            string this_msg = x_CorrectOnePair(*(list[1]), *new_gene, seq_id, scope);
            if (NStr::IsBlank(this_msg)) {
                m_Error = "No change!";
            } else {
                cmd.Reset(new CCmdComposite("Correct Genes"));   
                CSeq_feat_Handle orig_gene = scope.GetSeq_featHandle(*(list[0]));
                cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(orig_gene,*new_gene)));
                if (m_Log->GetValue()) {
                    report_msg = this_msg;
                }
            }
        }
    } else {
        vector<CConstRef<CSeq_feat> > constrained = x_GetConstrainedList();
        bool same_strand = !m_MatchStrand->GetValue();
        vector<CSeq_feat_Handle> feats;
        ITERATE(vector<CConstRef<CSeq_feat> >, it, constrained) 
        {
            CSeq_feat_Handle fh = scope.GetSeq_featHandle(**it, CScope::eMissing_Null);
            if (!fh)
                continue;
            feats.push_back(fh);
        }
        map<CSeq_feat_Handle, vector<CSeq_feat_Handle> > feat_to_genes; 
        set<CSeq_feat_Handle> used_genes;
        x_FindExactMatches(feats, scope, same_strand, feat_to_genes, used_genes);
        x_FindNonExactMatches(feats, scope, same_strand, feat_to_genes, used_genes);

        ITERATE(vector<CConstRef<CSeq_feat> >, it, constrained) {
            CSeq_feat_Handle fh = scope.GetSeq_featHandle(**it, CScope::eMissing_Null);
            if (!fh)
                continue;
            CBioseq_Handle bsh = scope.GetBioseqHandle((*it)->GetLocation());
            string seq_id;
            bsh.GetCompleteBioseq()->GetLabel(&seq_id, CBioseq::eContent);
            vector<CSeq_feat_Handle> &genes = feat_to_genes[fh];
            for (size_t ii = 0; ii < genes.size(); ii++)
            {
                CRef<CSeq_feat> new_gene(new CSeq_feat());
                new_gene->Assign(*(genes[ii].GetOriginalSeq_feat()));
                string this_msg = x_CorrectOnePair(**it, *new_gene, seq_id, scope);               
                if (NStr::IsBlank(this_msg)) {
                    m_Error = "No change!";
                } else {
                    if (!cmd) {
                        cmd.Reset(new CCmdComposite("Correct Genes"));   
                    }
                    cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(genes[ii],*new_gene)));
                    if (m_Log->GetValue()) {
                        report_msg += this_msg;
                    }
                }
            }
            if (genes.empty()) 
            {
                string cds_label = edit::GetTextObjectDescription(**it, scope);
                report_msg += "No gene found for " + cds_label + "\n";
            }
        }
    }

    if (m_Log->GetValue() && !NStr::IsBlank(report_msg)) {
        CGenericReportDlg* report = new CGenericReportDlg(NULL);
        report->SetTitle(wxT("Gene Change Log"));
        report->SetText(ToWxString(report_msg));
        report->Show(true);
    }

    return cmd;
}


string CCorrectGenesDialog::GetErrorMessage()
{
    return m_Error;
}


bool CCorrectGenesDialog::x_CorrectPairInterval(const CSeq_loc& cds, CSeq_feat& gene, CScope* scope)
{
    bool changed = false;
    if (cds.GetStrand() == eNa_strand_other)
        return changed;

    CRef<CSeq_loc> interval(new CSeq_loc);
    interval->SetInt().SetFrom(cds.GetStart(eExtreme_Positional));
    interval->SetInt().SetTo(cds.GetStop(eExtreme_Positional));
    interval->SetStrand(cds.GetStrand());
    CRef<CSeq_id> id(new CSeq_id);
    id->Assign(*cds.GetId());
    interval->SetInt().SetId(*id);  
    interval->SetPartialStart(cds.IsPartialStart(eExtreme_Biological),eExtreme_Biological);
    interval->SetPartialStop(cds.IsPartialStop(eExtreme_Biological),eExtreme_Biological);

  
    if (sequence::Compare(gene.GetLocation(), *interval, scope) != sequence::eSame ||
        gene.GetLocation().IsPartialStart(eExtreme_Biological) != interval->IsPartialStart(eExtreme_Biological) ||
        gene.GetLocation().IsPartialStop(eExtreme_Biological) != interval->IsPartialStop(eExtreme_Biological))
    {
        changed = true;
        gene.SetLocation().Assign(*interval);
        if (gene.GetLocation().IsPartialStart(eExtreme_Biological)
            || gene.GetLocation().IsPartialStop(eExtreme_Biological)) {
            gene.SetPartial(true);
        } else {
            gene.ResetPartial();
        }
    }
    return changed;
}


bool CCorrectGenesDialog::x_CorrectPairStrand(const CSeq_loc& cds, CSeq_loc& gene)
{
    bool changed = false;

    ENa_strand cds_strand = cds.GetStrand();
    ENa_strand gene_strand = gene.GetStrand();

    if (cds_strand == eNa_strand_minus && gene_strand != eNa_strand_minus) {
        gene.SetStrand(eNa_strand_minus);
        changed = true;
    } else if (cds_strand != eNa_strand_minus && gene_strand == eNa_strand_minus) {
        gene.ResetStrand();
        changed = true;
    }

    return changed;
}


string CCorrectGenesDialog::x_CorrectOnePair(const CSeq_feat& cds, CSeq_feat& gene, const string& seq_label, CScope& scope)
{
    string log_msg;
    string gene_label = edit::GetTextObjectDescription(gene, scope);
    string cds_label = edit::GetTextObjectDescription(cds, scope);

    if (gene.IsSetExcept() && gene.GetExcept() && gene.IsSetExcept_text() && gene.GetExcept_text() == "trans-splicing")
    {
        log_msg = "Sequence " + seq_label + ":  "+gene_label+" is a trans-spliced gene\n";
        return log_msg;
    }
    
    if (m_MatchInterval->GetValue()) {
        if (x_CorrectPairInterval(cds.GetLocation(), gene, &scope)) {
           
            
            log_msg = "Sequence " + seq_label + ":" +  gene_label + " reset to ";
            if (cds.GetData().IsCdregion()) {
                log_msg += "CDS";
            } else {
                log_msg += "mRNA";
            }
            log_msg += " interval (" + cds_label + ")\n";            
        }
    }
    if (m_MatchStrand->GetValue()) {
        if (x_CorrectPairStrand(cds.GetLocation(), gene.SetLocation())) {
            log_msg += "Sequence " + seq_label + ":" +  gene_label + " strand set to match ";
            if (cds.GetData().IsCdregion()) {
                log_msg += "CDS";
            } else {
                log_msg += "mRNA";
            }
            log_msg += " (" + cds_label + ")\n";            
        }
    }

    return log_msg;
}


vector<CConstRef<CSeq_feat> > CCorrectGenesDialog::x_GetSelectedPair()
{
    vector<CConstRef<CSeq_feat> > list;

    TConstScopedObjects objects;
    CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
    if (!sel_srv) {
        m_Error = "Can't find selection service!";
        return list;
    }
    sel_srv->GetActiveObjects(objects);
    if (objects.empty()) 
    {
        GetViewObjects(m_Workbench, objects);
    }
    if (objects.empty()) {
        m_Error = "No objects selected!";
        return list;
    }
    CConstRef<CSeq_feat> gene(NULL);
    CConstRef<CSeq_feat> other(NULL);
    NON_CONST_ITERATE (TConstScopedObjects, it, objects) {
        const CSeq_feat* f = dynamic_cast<const CSeq_feat*>(it->object.GetPointer());
        if (f) {
            if (f->GetData().IsGene()) {
                if (gene) {
                    m_Error = "More than one gene feature is selected!";
                    return list;
                } else {
                    gene.Reset(f);
                }
            } else {
                if (other) {
                    m_Error = "More than one non-gene feature is selected!";
                    return list;
                } else {
                    other.Reset(f);
                }
            }
        }
    }
    if (!gene || !other) {
        m_Error = "Must select one gene and one other feature!";
        return list;
    }
    list.push_back(gene);
    list.push_back(other);
    return list;
}


vector<CConstRef<CSeq_feat> > CCorrectGenesDialog::x_GetConstrainedList()
{
    vector<CConstRef<CSeq_feat> > constrained;
    // want to get a list of the features of the selected type that match the constraint
    CRef<CMiscSeqTableColumn> col(new CMiscSeqTableColumn("location"));

    CFieldNamePanel* constraint_field_panel = m_Constraint->GetFieldNamePanel();
    string constraint_field;
    string constraint_field_type = m_Constraint->GetFieldType();
    if (constraint_field_panel)
        constraint_field = constraint_field_panel->GetFieldName();

    if (NStr::IsBlank(constraint_field)) {
        constraint_field = constraint_field_type;
    } else {
        if (NStr::StartsWith(constraint_field_type, "RNA") && NStr::Find(constraint_field, "RNA") == NPOS ) {
            constraint_field = "RNA " + constraint_field;
        }
    }
    CRef<edit::CStringConstraint> string_constraint = m_Constraint->GetStringConstraint();

    col->SetConstraint(constraint_field, string_constraint);
    vector<CConstRef<CObject> > objs = col->GetObjects(m_TopSeqEntry, constraint_field, string_constraint);
    CSeqFeatData::ESubtype match = CSeqFeatData::eSubtype_cdregion;
    if (m_FeatureChoice->GetSelection() == 1) {
        match = CSeqFeatData::eSubtype_mRNA;
    } else if (m_FeatureChoice->GetSelection() == 2) {
        match = CSeqFeatData::eSubtype_ncRNA;
    } else if (m_FeatureChoice->GetSelection() == 3) {
        match = CSeqFeatData::eSubtype_rRNA;
    } else if (m_FeatureChoice->GetSelection() == 4) {
        match = CSeqFeatData::eSubtype_tRNA;
    }

    ITERATE(vector<CConstRef<CObject> >, it, objs) {
        const CSeq_feat* f = dynamic_cast<const CSeq_feat*>((*it).GetPointer());
        if (f && f->IsSetData() && f->GetData().GetSubtype() == match) {
            CConstRef<CSeq_feat> ref(f);
            constrained.push_back(ref);
        }
    }
    return constrained;
}


bool CCorrectGenesDialog::x_TestGeneForFeature(const CSeq_loc& gene_loc, const CSeq_loc& feat_loc, CScope& scope, bool same_strand, bool& exact, TSeqPos& diff)
{
    exact = false;
    // unless we plan to correct to same strand, only look at locations on same strand
    if (same_strand) {
        ENa_strand feat_strand = feat_loc.GetStrand();
        ENa_strand gene_strand = gene_loc.GetStrand();
        if (feat_strand == eNa_strand_minus && gene_strand != eNa_strand_minus) {
            return false;
        } else if (feat_strand != eNa_strand_minus && gene_strand == eNa_strand_minus) {
            return false;
        }
    }
    sequence::ECompare cmp = sequence::Compare(gene_loc, feat_loc, &scope);
    if (cmp == sequence::eSame) {
        exact = true;
        return true;
    } else if (cmp == sequence::eContained || cmp == sequence::eContains || cmp == sequence::eOverlap) {

        CRef<CSeq_loc> loc1 = sequence::Seq_loc_Subtract(gene_loc, feat_loc, CSeq_loc::fMerge_All|CSeq_loc::fSort, &scope);
        CRef<CSeq_loc> loc2 = sequence::Seq_loc_Subtract(feat_loc, gene_loc, CSeq_loc::fMerge_All|CSeq_loc::fSort, &scope);
        CRef<CSeq_loc> loc3 = sequence::Seq_loc_Add(*loc1, *loc2, CSeq_loc::fMerge_All|CSeq_loc::fSort, &scope);
        diff = sequence::GetCoverage(*loc3, &scope);
        return true;
    } else {
        return false;
    }
}


void CCorrectGenesDialog::x_FindExactMatches(const vector<CSeq_feat_Handle> &feats, CScope& scope, bool same_strand, 
                                             map<CSeq_feat_Handle, vector<CSeq_feat_Handle> > &feat_to_genes, set<CSeq_feat_Handle> &used_genes)
{
    for (const auto &feat : feats)
    {
        const CSeq_loc &loc = feat.GetLocation();
        CBioseq_Handle bsh = scope.GetBioseqHandle(loc);
        if (!bsh) 
        {
            continue;
        }
    
        for (CFeat_CI gene(bsh, CSeqFeatData::eSubtype_gene); gene; ++gene) 
        {
            if (used_genes.find(*gene) != used_genes.end())
                continue;
            bool exact = false;
            TSeqPos diff = INT_MAX;
            if (x_TestGeneForFeature(gene->GetLocation(), loc, scope, same_strand, exact, diff)) 
            {
                if (exact)
                {
                    feat_to_genes[feat].push_back(*gene);
                    used_genes.insert(*gene);
                }                      
            }
        }
    }
}

void CCorrectGenesDialog::x_FindNonExactMatches(const vector<CSeq_feat_Handle> &feats, CScope& scope, bool same_strand, 
                                                                    map<CSeq_feat_Handle, vector<CSeq_feat_Handle> > &feat_to_genes, set<CSeq_feat_Handle> &used_genes)
{
    map<TSeqPos, map<CSeq_feat_Handle, vector<CSeq_feat_Handle> > > dist_to_feat_to_genes;
    for (const auto &feat : feats)
    {
        if (feat_to_genes.find(feat) != feat_to_genes.end())
            continue;
        const CSeq_loc &loc = feat.GetLocation();
        CBioseq_Handle bsh = scope.GetBioseqHandle(loc);
        if (!bsh) 
        {
            continue;
        }
    
        for (CFeat_CI gene(bsh, CSeqFeatData::eSubtype_gene); gene; ++gene) 
        {
            if (used_genes.find(*gene) != used_genes.end())
                continue;
            bool exact = false;
            TSeqPos diff = INT_MAX;
            if (x_TestGeneForFeature(gene->GetLocation(), loc, scope, same_strand, exact, diff)) 
            {
                if (exact)
                {
                    diff = 0;
                }        
                dist_to_feat_to_genes[diff][feat].push_back(*gene);                
            }
        }
    }
    for (const auto &it : dist_to_feat_to_genes)
    {
        for (const auto &f_to_g : it.second)
        {
            const CSeq_feat_Handle feat = f_to_g.first;
            if (feat_to_genes.find(feat) != feat_to_genes.end())
                continue;
            for (const auto &gene : f_to_g.second)
            {
                if (used_genes.find(gene) != used_genes.end())
                    continue;
                feat_to_genes[feat].push_back(gene);
                used_genes.insert(gene);
            }
        }
    }    
}


END_NCBI_SCOPE


