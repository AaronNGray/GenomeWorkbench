/*  $Id: splign_panel.cpp 37399 2017-01-05 17:42:22Z katargir $
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

#include <serial/typeinfo.hpp>
#include <gui/objutils/obj_convert.hpp>

#include <objects/seq/MolInfo.hpp>

#include <objmgr/seqdesc_ci.hpp>

#include <gui/packages/pkg_alignment/splign_panel.hpp>
#include <gui/widgets/object_list/object_list_widget.hpp>

#include <gui/widgets/wx/number_validator.hpp>
#include <gui/widgets/wx/float_text_validator.hpp>

#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/radiobox.h>
#include <wx/msgdlg.h>

#include <gui/widgets/object_list/object_list_widget_sel.hpp>

////@begin includes
////@end includes

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

IMPLEMENT_DYNAMIC_CLASS( CSplignPanel, CAlgoToolManagerParamsPanel )

BEGIN_EVENT_TABLE( CSplignPanel, CAlgoToolManagerParamsPanel )

////@begin CSplignPanel event table entries
////@end CSplignPanel event table entries

END_EVENT_TABLE()


CConstRef<CObject> CMolTypeValidator::Validate(const CSeq_id& id, CScope& scope, wxString& errMessage)
{
    CObjectConverter::TRelationVector relations;
    CObjectConverter::FindRelations(scope, id,
            CSeq_loc::GetTypeInfo()->GetName(), relations);

    if (relations.empty()) {
        errMessage = wxT("Object cannot be converted to CSeq_loc.");
        return CConstRef<CObject>();
    }

    CRelation::TObjects related;
    relations.front()->GetRelated(scope, id, related, CRelation::fConvert_NoExpensive);

    if (related.empty()) {
        errMessage = wxT("Object cannot be converted to CSeq_loc.");
        return CConstRef<CObject>();
    }

    try {
        ITERATE(CRelation::TObjects, it, related) {
            const CSeq_loc& loc = dynamic_cast<const CSeq_loc&>(it->GetObject());
            CBioseq_Handle handle = scope.GetBioseqHandle(*loc.GetId());

            if (m_MolType == kProtein && handle.IsAa()) {
                return CConstRef<CObject>(it->GetObjectPtr());
            } else if (m_MolType == kNucleotide && handle.IsNa()) {
                return CConstRef<CObject>(it->GetObjectPtr());
            }

            const CMolInfo* molInfo = 0;
            CSeqdesc_CI desc_iter(handle, CSeqdesc::e_Molinfo);
            if (desc_iter) molInfo = &desc_iter->GetMolinfo();
            if (molInfo) {
                CMolInfo::TBiomol biomol = molInfo->GetBiomol();
                switch (biomol) {
                case CMolInfo::eBiomol_genomic:
                    if (m_MolType == kGenomic)
                        return CConstRef<CObject>(it->GetObjectPtr());
                    break;
                case CMolInfo::eBiomol_unknown:
                    return CConstRef<CObject>(it->GetObjectPtr());
                case CMolInfo::eBiomol_peptide:
                    break;
                default:
                    if (m_MolType == kCDNA)
                        return CConstRef<CObject>(it->GetObjectPtr());
                    break;
                }
            }
        }
    }
    catch (const CException& e) {
        errMessage = e.GetMsg();
    }
    catch (const exception& e) {
        errMessage = e.what();
    }

    if (errMessage.empty()) {
        switch (m_MolType) {
        case (kGenomic) :
            errMessage = wxT("Object is not a Genomic sequence.");
            break;
        case (kCDNA) :
            errMessage = wxT("Object is not a cDNA/Transcript sequence.");
            break;
        case (kProtein):
            errMessage = wxT("Object is not a protein sequence.");
            break;
        case (kNucleotide):
            errMessage = wxT("Object is not a nucleotide sequence.");
            break;
        }
    }

    return CConstRef<CObject>();
}

CSplignPanel::CSplignPanel()
: m_GenomicListAccValidator(CMolTypeValidator::kGenomic),
  m_CDNAListAccValidator(CMolTypeValidator::kCDNA)
{
    Init();
}

CSplignPanel::CSplignPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, bool visible )
: m_GenomicListAccValidator(CMolTypeValidator::kGenomic),
  m_CDNAListAccValidator(CMolTypeValidator::kCDNA)
{
    Init();
    Create(parent, id, pos, size, style, visible);
}

bool CSplignPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, bool visible )
{
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);

    CAlgoToolManagerParamsPanel::Create( parent, id, pos, size, style );

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

CSplignPanel::~CSplignPanel()
{
////@begin CSplignPanel destruction
////@end CSplignPanel destruction
}

void CSplignPanel::Init()
{
////@begin CSplignPanel member initialisation
    m_GenomicList = NULL;
    m_cDNAList = NULL;
////@end CSplignPanel member initialisation
}

void CSplignPanel::CreateControls()
{
////@begin CSplignPanel content construction
    CSplignPanel* itemCAlgoToolManagerParamsPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCAlgoToolManagerParamsPanel1->SetSizer(itemBoxSizer2);

    m_GenomicList = new CObjectListWidgetSel( itemCAlgoToolManagerParamsPanel1, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxLC_SINGLE_SEL );
    itemBoxSizer2->Add(m_GenomicList, 1, wxGROW|wxALL, wxDLG_UNIT(itemCAlgoToolManagerParamsPanel1, wxSize(1, -1)).x);

    m_cDNAList = new CObjectListWidgetSel( itemCAlgoToolManagerParamsPanel1, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(m_cDNAList, 1, wxGROW|wxALL, wxDLG_UNIT(itemCAlgoToolManagerParamsPanel1, wxSize(1, -1)).x);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    wxStaticText* itemStaticText6 = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, _("cDNA strand"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemStaticText6, 0, wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemCAlgoToolManagerParamsPanel1, wxSize(5, -1)).x);

    wxArrayString itemChoice7Strings;
    itemChoice7Strings.Add(_("plus"));
    itemChoice7Strings.Add(_("minus"));
    itemChoice7Strings.Add(_("both"));
    wxChoice* itemChoice7 = new wxChoice( itemCAlgoToolManagerParamsPanel1, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, itemChoice7Strings, 0 );
    itemBoxSizer5->Add(itemChoice7, 0, wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemCAlgoToolManagerParamsPanel1, wxSize(5, -1)).x);

    wxArrayString itemChoice8Strings;
    itemChoice8Strings.Add(_("EST"));
    itemChoice8Strings.Add(_("mRNA"));
    wxChoice* itemChoice8 = new wxChoice( itemCAlgoToolManagerParamsPanel1, ID_CHOICE4, wxDefaultPosition, wxDefaultSize, itemChoice8Strings, 0 );
    itemBoxSizer5->Add(itemChoice8, 0, wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemCAlgoToolManagerParamsPanel1, wxSize(5, -1)).x);

    wxCheckBox* itemCheckBox9 = new wxCheckBox( itemCAlgoToolManagerParamsPanel1, ID_CHECKBOX3, _("EndGap Detection"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox9->SetValue(false);
    itemBoxSizer5->Add(itemCheckBox9, 0, wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemCAlgoToolManagerParamsPanel1, wxSize(5, -1)).x);

    wxCheckBox* itemCheckBox10 = new wxCheckBox( itemCAlgoToolManagerParamsPanel1, ID_CHECKBOX4, _("PolyA Detection"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox10->SetValue(false);
    itemBoxSizer5->Add(itemCheckBox10, 0, wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemCAlgoToolManagerParamsPanel1, wxSize(5, -1)).x);

    wxFlexGridSizer* itemFlexGridSizer11 = new wxFlexGridSizer(0, 4, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer11, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    wxStaticText* itemStaticText12 = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, _("Min compartment identity"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer11->Add(itemStaticText12, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemCAlgoToolManagerParamsPanel1, wxSize(2, -1)).x);

    wxTextCtrl* itemTextCtrl13 = new wxTextCtrl( itemCAlgoToolManagerParamsPanel1, ID_TEXTCTRL7, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer11->Add(itemTextCtrl13, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemCAlgoToolManagerParamsPanel1, wxSize(2, -1)).x);

    wxStaticText* itemStaticText14 = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, _("Maximum Genomic Extent"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer11->Add(itemStaticText14, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemCAlgoToolManagerParamsPanel1, wxSize(2, -1)).x);

    wxTextCtrl* itemTextCtrl15 = new wxTextCtrl( itemCAlgoToolManagerParamsPanel1, ID_TEXTCTRL6, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer11->Add(itemTextCtrl15, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemCAlgoToolManagerParamsPanel1, wxSize(2, -1)).x);

    wxStaticText* itemStaticText16 = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, _("Min exon identity"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer11->Add(itemStaticText16, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemCAlgoToolManagerParamsPanel1, wxSize(2, -1)).x);

    wxTextCtrl* itemTextCtrl17 = new wxTextCtrl( itemCAlgoToolManagerParamsPanel1, ID_TEXTCTRL8, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer11->Add(itemTextCtrl17, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemCAlgoToolManagerParamsPanel1, wxSize(2, -1)).x);

    wxStaticText* itemStaticText18 = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, _("Compartment penalty"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer11->Add(itemStaticText18, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemCAlgoToolManagerParamsPanel1, wxSize(2, -1)).x);

    wxTextCtrl* itemTextCtrl19 = new wxTextCtrl( itemCAlgoToolManagerParamsPanel1, ID_TEXTCTRL9, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer11->Add(itemTextCtrl19, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemCAlgoToolManagerParamsPanel1, wxSize(2, -1)).x);

    wxStaticText* itemStaticText20 = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, _("Maximum intron length"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer11->Add(itemStaticText20, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemCAlgoToolManagerParamsPanel1, wxSize(5, -1)).x);

    wxTextCtrl* itemTextCtrl21 = new wxTextCtrl( itemCAlgoToolManagerParamsPanel1, ID_TEXTCTRL13, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer11->Add(itemTextCtrl21, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemCAlgoToolManagerParamsPanel1, wxSize(5, -1)).x);

    // Set validators
    itemChoice7->SetValidator( wxGenericValidator(& GetData().m_cDNA_strand) );
    itemChoice8->SetValidator( wxGenericValidator(& GetData().m_DiscType) );
    itemCheckBox9->SetValidator( wxGenericValidator(& GetData().m_EndGapDetect) );
    itemCheckBox10->SetValidator( wxGenericValidator(& GetData().m_PolyADetect) );
    itemTextCtrl13->SetValidator( CFloatTextValidator(& GetData().m_MinCompIdentity,0.,1.) );
    itemTextCtrl15->SetValidator( CNumberValidator(& GetData().m_MaxGenomicExtent,0,2000000) );
    itemTextCtrl17->SetValidator( CFloatTextValidator(& GetData().m_MinExonIdentity,0.,1.) );
    itemTextCtrl19->SetValidator( CFloatTextValidator(& GetData().m_CompPenalty,0.,1.) );
    itemTextCtrl21->SetValidator( CNumberValidator(& GetData().m_MaxIntron,0,2000000) );
////@end CSplignPanel content construction

    m_GenomicList->SetTitle(wxT("Select Genomic Sequence"));
    m_GenomicList->SetAccessionValidator(&m_GenomicListAccValidator);

    m_cDNAList->SetDoSelectAll(true);
    m_cDNAList->SetTitle(wxT("Select cDNA/Transcript Sequences"));
    m_cDNAList->SetAccessionValidator(&m_CDNAListAccValidator);
}

void CSplignPanel::SetObjects(map<string, TConstScopedObjects>* genomic,
                              map<string, TConstScopedObjects>* na)
{
    m_GenomicList->SetObjects(genomic);
    m_cDNAList->SetObjects(na);
}

bool CSplignPanel::ShowToolTips()
{
    return true;
}
wxBitmap CSplignPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSplignPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSplignPanel bitmap retrieval
}
wxIcon CSplignPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSplignPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSplignPanel icon retrieval
}

bool CSplignPanel::TransferDataFromWindow()
{
    if (!CAlgoToolManagerParamsPanel::TransferDataFromWindow())
        return false;

    TConstScopedObjects selection = m_GenomicList->GetSelection();
    m_GenomicSeq = selection[0];
    m_cDNASeqs = m_cDNAList->GetSelection();

    return true;
}

static const char* kGenomicList = ".GenomicList";
static const char* kCDNAList = ".CDNAList";

void CSplignPanel::SetRegistryPath(const string& path)
{
    m_RegPath = path;
    m_GenomicList->SetRegistryPath(m_RegPath + kGenomicList);
    m_cDNAList->SetRegistryPath(m_RegPath + kCDNAList);
}

void CSplignPanel::SaveSettings() const
{
    m_GenomicList->SaveSettings();
    m_cDNAList->SaveSettings();
}

void CSplignPanel::LoadSettings()
{
    m_GenomicList->LoadSettings();
    m_cDNAList->LoadSettings();
}

void CSplignPanel::RestoreDefaults()
{
    GetData().Init();
    TransferDataToWindow();
}

END_NCBI_SCOPE
