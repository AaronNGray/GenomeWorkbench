/*  $Id: prosplign_panel.cpp 36103 2016-08-10 21:15:00Z asztalos $
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
 * Authors: Andrea Asztalos
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <objects/seq/MolInfo.hpp>
#include <objects/seqfeat/Genetic_code.hpp>
#include <objects/seqfeat/Genetic_code_table.hpp>
#include <objmgr/seqdesc_ci.hpp>

#include <gui/objutils/label.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/number_validator.hpp>
#include <gui/widgets/object_list/object_list_widget_sel.hpp>
#include <gui/packages/pkg_alignment/blast_search_params.hpp>
#include <gui/packages/pkg_alignment/prosplign_panel.hpp>

#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/notebook.h>
#include <wx/valtext.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

IMPLEMENT_DYNAMIC_CLASS(CProSplignPanel, CAlgoToolManagerParamsPanel)

BEGIN_EVENT_TABLE(CProSplignPanel, CAlgoToolManagerParamsPanel)

    EVT_CHECKBOX(ID_PROSPLIGN_CHCKBX2, CProSplignPanel::OnRefineAlignmentClick)

END_EVENT_TABLE()

CProSplignPanel::CProSplignPanel()
    : m_ProteinListAccValidator(CMolTypeValidator::kProtein),
      m_NucleotideListAccValidator(CMolTypeValidator::kNucleotide)
{
    Init();
}

CProSplignPanel::CProSplignPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, bool visible )
    : m_ProteinListAccValidator(CMolTypeValidator::kProtein),
      m_NucleotideListAccValidator(CMolTypeValidator::kNucleotide)
{
    Init();
    Create(parent, id, pos, size, style, visible);
}

bool CProSplignPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, bool visible )
{
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);

    CAlgoToolManagerParamsPanel::Create(parent, id, pos, size, style);

    if (!visible) {
        Hide();
    }

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();

    return true;
}

CProSplignPanel::~CProSplignPanel()
{
////@begin CProSplignPanel destruction
////@end CProSplignPanel destruction
}

void CProSplignPanel::Init()
{
////@begin CProSplignPanel member initialisation
    m_ProteinList = NULL;
    m_NucleotideList = NULL;
////@end CProSplignPanel member initialisation
}

static string sGetWarningMsg(const string& start, int a, int b)
{
    CNcbiOstrstream oss;
    oss << start << ": please enter a number between " << a <<  " and " << b;
    return string(CNcbiOstrstreamToString(oss));
}

void CProSplignPanel::CreateControls()
{    
////@begin CProSplignPanel content construction
    CProSplignPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_ProteinList = new CObjectListWidgetSel(itemPanel1, ID_PROSPLIGN_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxLC_SINGLE_SEL);
    itemBoxSizer2->Add(m_ProteinList, 1, wxGROW | wxALL, wxDLG_UNIT(itemPanel1, wxSize(1, -1)).x);

    m_NucleotideList = new CObjectListWidgetSel(itemPanel1, ID_PROSPLIGN_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    itemBoxSizer2->Add(m_NucleotideList, 1, wxGROW | wxALL, wxDLG_UNIT(itemPanel1, wxSize(1, -1)).x);
    wxNotebook* itemNotebook5 = new wxNotebook( itemPanel1, ID_PROSPLIGN_NOTEBOOK, wxDefaultPosition, wxDefaultSize, wxBK_DEFAULT );

    wxPanel* itemPanel6 = new wxPanel( itemNotebook5, ID_PROSPLIGN_PANEL3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxVERTICAL);
    itemPanel6->SetSizer(itemBoxSizer7);

    wxFlexGridSizer* itemFlexGridSizer8 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer7->Add(itemFlexGridSizer8, 0, wxALIGN_CENTER_HORIZONTAL|wxTOP, 2);
    wxStaticText* itemStaticText9 = new wxStaticText( itemPanel6, wxID_STATIC, _("Strand"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer8->Add(itemStaticText9, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer8->Add(itemBoxSizer10, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    wxArrayString itemChoice11Strings;
    itemChoice11Strings.Add(_("Plus"));
    itemChoice11Strings.Add(_("Minus"));
    itemChoice11Strings.Add(_("Both"));
    wxChoice* itemChoice11 = new wxChoice( itemPanel6, ID_PROSPLIGN_CHOICE1, wxDefaultPosition, wxDefaultSize, itemChoice11Strings, 0 );
    itemBoxSizer10->Add(itemChoice11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxCheckBox* itemCheckBox12 = new wxCheckBox( itemPanel6, ID_PROSPLIGN_CHCKBX1, _("With introns"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox12->SetValue(true);
    itemBoxSizer10->Add(itemCheckBox12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText13 = new wxStaticText( itemPanel6, wxID_STATIC, _("Frameshift opening cost"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer8->Add(itemStaticText13, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl14 = new wxTextCtrl( itemPanel6, ID_PROSPLIGN_TXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer8->Add(itemTextCtrl14, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText15 = new wxStaticText( itemPanel6, wxID_STATIC, _("Gap opening cost"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer8->Add(itemStaticText15, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer8->Add(itemBoxSizer16, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    wxTextCtrl* itemTextCtrl17 = new wxTextCtrl( itemPanel6, ID_PROSPLIGN_TXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer16->Add(itemTextCtrl17, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText18 = new wxStaticText( itemPanel6, wxID_STATIC, _("Gap extension cost"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer16->Add(itemStaticText18, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl19 = new wxTextCtrl( itemPanel6, ID_PROSPLIGN_TXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer16->Add(itemTextCtrl19, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText20 = new wxStaticText( itemPanel6, wxID_STATIC, _("Genetic code"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer8->Add(itemStaticText20, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString itemChoice21Strings;
    wxChoice* itemChoice21 = new wxChoice(itemPanel6, ID_PROSPLIGN_CHOICE2, wxDefaultPosition, wxSize(340, -1), itemChoice21Strings, 0);
    itemFlexGridSizer8->Add(itemChoice21, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemNotebook5->AddPage(itemPanel6, _("General Options"));

    wxPanel* itemPanel22 = new wxPanel( itemNotebook5, ID_PROSPLIGN_PANEL4, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer23 = new wxBoxSizer(wxVERTICAL);
    itemPanel22->SetSizer(itemBoxSizer23);

    wxFlexGridSizer* itemFlexGridSizer24 = new wxFlexGridSizer(0, 1, 0, 0);
    itemBoxSizer23->Add(itemFlexGridSizer24, 0, wxALIGN_CENTER_HORIZONTAL|wxTOP, 2);
    wxCheckBox* itemCheckBox25 = new wxCheckBox(itemPanel22, ID_PROSPLIGN_CHCKBX2, _("Refine the alignment"), wxDefaultPosition, wxDefaultSize, 0);
    itemCheckBox25->SetValue(false);
    itemFlexGridSizer24->Add(itemCheckBox25, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

    wxBoxSizer* itemBoxSizer26 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer24->Add(itemBoxSizer26, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    wxCheckBox* itemCheckBox27 = new wxCheckBox( itemPanel22, ID_PROSPLIGN_CHCKBX3, _("Remove flank regions only"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox27->SetValue(false);
    itemBoxSizer26->Add(itemCheckBox27, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxCheckBox* itemCheckBox28 = new wxCheckBox( itemPanel22, ID_PROSPLIGN_CHCKBX4, _("Remove Ns from the end of good regions"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox28->SetValue(false);
    itemBoxSizer26->Add(itemCheckBox28, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer29 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer23->Add(itemBoxSizer29, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);
    wxFlexGridSizer* itemFlexGridSizer30 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer29->Add(itemFlexGridSizer30, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);
    wxStaticText* itemStaticText31 = new wxStaticText( itemPanel22, wxID_STATIC, _("Total positives"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer30->Add(itemStaticText31, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer32 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer30->Add(itemBoxSizer32, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    wxTextCtrl* itemTextCtrl33 = new wxTextCtrl( itemPanel22, ID_PROSPLIGN_TXTCTRL4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer32->Add(itemTextCtrl33, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText34 = new wxStaticText( itemPanel22, wxID_STATIC, _("%"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer32->Add(itemStaticText34, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText35 = new wxStaticText( itemPanel22, wxID_STATIC, _("Flank positives"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer30->Add(itemStaticText35, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer36 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer30->Add(itemBoxSizer36, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    wxTextCtrl* itemTextCtrl37 = new wxTextCtrl( itemPanel22, ID_PROSPLIGN_TXTCTRL5, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer36->Add(itemTextCtrl37, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText38 = new wxStaticText( itemPanel22, wxID_STATIC, _("%"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer36->Add(itemStaticText38, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText39 = new wxStaticText( itemPanel22, wxID_STATIC, _("Min length of good region"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer30->Add(itemStaticText39, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl40 = new wxTextCtrl( itemPanel22, ID_PROSPLIGN_TXTCTRL6, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer30->Add(itemTextCtrl40, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer41 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer29->Add(itemFlexGridSizer41, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);
    wxStaticText* itemStaticText42 = new wxStaticText( itemPanel22, wxID_STATIC, _("Min exon identity"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer41->Add(itemStaticText42, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer43 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer41->Add(itemBoxSizer43, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    wxTextCtrl* itemTextCtrl44 = new wxTextCtrl( itemPanel22, ID_PROSPLIGN_TXTCTRL7, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer43->Add(itemTextCtrl44, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText45 = new wxStaticText( itemPanel22, wxID_STATIC, _("%"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer43->Add(itemStaticText45, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText46 = new wxStaticText( itemPanel22, wxID_STATIC, _("Min exon positives"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer41->Add(itemStaticText46, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer47 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer41->Add(itemBoxSizer47, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    wxTextCtrl* itemTextCtrl48 = new wxTextCtrl( itemPanel22, ID_PROSPLIGN_TXTCTRL8, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer47->Add(itemTextCtrl48, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText49 = new wxStaticText( itemPanel22, wxID_STATIC, _("%"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer47->Add(itemStaticText49, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText50 = new wxStaticText( itemPanel22, wxID_STATIC, _("Min flanking exon length"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer41->Add(itemStaticText50, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl51 = new wxTextCtrl( itemPanel22, ID_PROSPLIGN_TXTCTRL9, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer41->Add(itemTextCtrl51, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemNotebook5->AddPage(itemPanel22, _("Refinement Options"));

    itemBoxSizer2->Add(itemNotebook5, 0, wxGROW|wxALL, 5);

    // Set validators
////@end CProSplignPanel content construction

    m_ProteinList->SetTitle(wxT("Select Protein Sequence"));
    m_ProteinList->SetAccessionValidator(&m_ProteinListAccValidator);

    m_NucleotideList->SetTitle(wxT("Select Genomic Ranges/Transcript Sequences"));
    m_NucleotideList->SetAccessionValidator(&m_NucleotideListAccValidator);

    x_ListGeneticCodes();

    itemChoice11->SetValidator(wxGenericValidator(&m_Data.m_Strand));
    itemCheckBox12->SetValidator(wxGenericValidator(&m_Data.m_WithIntrons));

    itemTextCtrl14->SetValidator(wxTextValidator(wxFILTER_DIGITS, &m_FrameshiftCostStr));
    itemTextCtrl17->SetValidator(wxTextValidator(wxFILTER_DIGITS, &m_GapOpenCostStr));
    itemTextCtrl19->SetValidator(wxTextValidator(wxFILTER_DIGITS, &m_GapExtendCostStr));
    itemChoice21->SetValidator(wxGenericValidator(&m_Data.m_GeneticCode));

    itemCheckBox25->SetValidator(wxGenericValidator(&m_Data.m_RefineAlignment));
    itemCheckBox27->SetValidator(wxGenericValidator(&m_Data.m_RemoveFlanks));
    itemCheckBox28->SetValidator(wxGenericValidator(&m_Data.m_RemoveNs));

    itemTextCtrl33->SetValidator(CNumberValidator(&m_TotalPosL, 0, 100, sGetWarningMsg("Total positives", 0, 100).c_str()));
    itemTextCtrl37->SetValidator(CNumberValidator(&m_FlankPosL, 0, 100, sGetWarningMsg("Flank positives", 0, 100).c_str()));
    itemTextCtrl40->SetValidator(CNumberValidator(&m_MinGoodLenL, 3, 10000, sGetWarningMsg("Min good length", 3, 10000).c_str()));

    itemTextCtrl44->SetValidator(CNumberValidator(&m_MinExonIdentL, 0, 100, sGetWarningMsg("Min exon identity", 0, 100).c_str()));
    itemTextCtrl48->SetValidator(CNumberValidator(&m_MinExonPosL, 0, 100, sGetWarningMsg("Min exon positives", 0, 100).c_str()));
    itemTextCtrl51->SetValidator(CNumberValidator(&m_MinFlankExonLenL, 3, 10000, sGetWarningMsg("Min flanking exon length", 3, 10000).c_str()));
}

void CProSplignPanel::x_ListGeneticCodes()
{
    wxChoice* gcode_choice = (wxChoice*)FindWindowById(ID_PROSPLIGN_CHOICE2);
    if (!gcode_choice) {
        return;
    }

    vector<string> labels;
    CBLASTParams::GetGeneticCodeLabels(labels);

    wxArrayString arraystr;
    arraystr.Alloc(labels.size() + 1);
    arraystr.Add("From organism");
    for (auto& it: labels) {
        arraystr.Add(ToWxString(it));
    }

    gcode_choice->Append(arraystr);
}

void CProSplignPanel::SetObjects(map<string, TConstScopedObjects>* protein,
    map<string, TConstScopedObjects>* nucleotide)
{
    m_ProteinList->SetObjects(protein);
    m_NucleotideList->SetObjects(nucleotide);
}


void CProSplignPanel::OnRefineAlignmentClick(wxCommandEvent& event)
{
    x_EnableRefinementOptions(event.IsChecked());
}

void CProSplignPanel::x_EnableRefinementOptions(bool val)
{
    wxCheckBox* removeflank_check = (wxCheckBox*)FindWindow(ID_PROSPLIGN_CHCKBX3);
    removeflank_check->Enable(val);
    wxCheckBox* removeNs_check = (wxCheckBox*)FindWindow(ID_PROSPLIGN_CHCKBX4);
    removeNs_check->Enable(val);

    FindWindowById(ID_PROSPLIGN_TXTCTRL4)->Enable(val);
    FindWindowById(ID_PROSPLIGN_TXTCTRL5)->Enable(val);
    FindWindowById(ID_PROSPLIGN_TXTCTRL6)->Enable(val);
    FindWindowById(ID_PROSPLIGN_TXTCTRL7)->Enable(val);
    FindWindowById(ID_PROSPLIGN_TXTCTRL8)->Enable(val);
    FindWindowById(ID_PROSPLIGN_TXTCTRL9)->Enable(val);
}

bool CProSplignPanel::TransferDataToWindow()
{
    m_FrameshiftCostStr = ToWxString(NStr::NumericToString(m_Data.m_FrameshiftCost));
    m_GapOpenCostStr = ToWxString(NStr::NumericToString(m_Data.m_GapOpenCost));
    m_GapExtendCostStr = ToWxString(NStr::NumericToString(m_Data.m_GapExtendCost));

    m_TotalPosL = m_Data.m_TotalPositives;
    m_FlankPosL = m_Data.m_FlankPositives;
    m_MinGoodLenL = m_Data.m_MinGoodLength;

    m_MinExonIdentL = m_Data.m_MinExonIdentity;
    m_MinExonPosL = m_Data.m_MinExonPositives;
    m_MinFlankExonLenL = m_Data.m_MinFlankingExonLength;

    x_EnableRefinementOptions(m_Data.m_RefineAlignment);

    return CAlgoToolManagerParamsPanel::TransferDataToWindow();
}

bool CProSplignPanel::TransferDataFromWindow()
{
    if (!CAlgoToolManagerParamsPanel::TransferDataFromWindow())
        return false;


    m_Data.m_FrameshiftCost = NStr::StringToInt(ToStdString(m_FrameshiftCostStr));
    m_Data.m_GapOpenCost = NStr::StringToInt(ToStdString(m_GapOpenCostStr));
    m_Data.m_GapExtendCost = NStr::StringToInt(ToStdString(m_GapExtendCostStr));

    m_Data.m_TotalPositives = m_TotalPosL;
    m_Data.m_FlankPositives = m_FlankPosL;
    m_Data.m_MinGoodLength = m_MinGoodLenL;

    m_Data.m_MinExonIdentity = m_MinExonIdentL;
    m_Data.m_MinExonPositives = m_MinExonPosL;
    m_Data.m_MinFlankingExonLength = m_MinFlankExonLenL;

    TConstScopedObjects selection = m_ProteinList->GetSelection();
    m_ProteinSeq = selection[0];
    m_NucleotideSeqs = m_NucleotideList->GetSelection();

    return true;
}

static const char* kProteinList = ".ProteinList";
static const char* kNucleotideList = ".NucleotideList";

void CProSplignPanel::SetRegistryPath(const string& path)
{
    m_RegPath = path;
    m_ProteinList->SetRegistryPath(m_RegPath + kProteinList);
    m_NucleotideList->SetRegistryPath(m_RegPath + kNucleotideList);
}

void CProSplignPanel::SaveSettings() const
{
    m_ProteinList->SaveSettings();
    m_NucleotideList->SaveSettings();
}

void CProSplignPanel::LoadSettings()
{
    m_ProteinList->LoadSettings();
    m_NucleotideList->LoadSettings();
}

void CProSplignPanel::RestoreDefaults()
{
    GetNCData().Init();
    TransferDataToWindow();
}

bool CProSplignPanel::ShowToolTips()
{
    return true;
}
wxBitmap CProSplignPanel::GetBitmapResource(const wxString& name)
{
    // Bitmap retrieval
    ////@begin CProSplignPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
    ////@end CProSplignPanel bitmap retrieval
}
wxIcon CProSplignPanel::GetIconResource(const wxString& name)
{
    // Icon retrieval
    ////@begin CProSplignPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
    ////@end CProSplignPanel icon retrieval
}

END_NCBI_SCOPE
