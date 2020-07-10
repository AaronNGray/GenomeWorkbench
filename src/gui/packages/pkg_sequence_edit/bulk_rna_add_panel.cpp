/*  $Id: bulk_rna_add_panel.cpp 37942 2017-03-07 17:11:31Z bollin $
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
#include <objects/seqloc/Seq_id.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/RNA_gen.hpp>
#include <objects/seqfeat/RNA_qual.hpp>
#include <objects/seqfeat/RNA_qual_set.hpp>
#include <objects/seqfeat/Trna_ext.hpp>
#include <objmgr/bioseq_ci.hpp>

////@begin includes
////@end includes

#include <gui/packages/pkg_sequence_edit/bulk_rna_add_panel.hpp>
#include <gui/packages/pkg_sequence_edit/subprep_util.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/widgets/edit/utilities.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/button.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

USING_SCOPE(ncbi::objects);

/*!
 * CBulkRNAAddPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CBulkRNAAddPanel, CBulkCmdPanel )


/*!
 * CBulkRNAAddPanel event table definition
 */

BEGIN_EVENT_TABLE( CBulkRNAAddPanel, CBulkCmdPanel )

////@begin CBulkRNAAddPanel event table entries
    EVT_CHOICE( ID_CHOICE1, CBulkRNAAddPanel::OnSelectRNAType )

    EVT_BUTTON( ID_BUTTON11, CBulkRNAAddPanel::OnAdd18SToComment )

    EVT_BUTTON( ID_BUTTON12, CBulkRNAAddPanel::OnAdd16StoComment )

////@end CBulkRNAAddPanel event table entries

END_EVENT_TABLE()


/*!
 * CBulkRNAAddPanel constructors
 */

CBulkRNAAddPanel::CBulkRNAAddPanel()
{
    Init();
}

CBulkRNAAddPanel::CBulkRNAAddPanel( wxWindow* parent, CSeq_entry_Handle seh, bool add_location_panel, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
: m_TopSeqEntry(seh), m_tRNAname(NULL), m_RNAname(NULL), CBulkCmdPanel(add_location_panel)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CBulkRNAAddPanel creator
 */

bool CBulkRNAAddPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CBulkRNAAddPanel creation
    CBulkCmdPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CBulkRNAAddPanel creation
    return true;
}


/*!
 * CBulkRNAAddPanel destructor
 */

CBulkRNAAddPanel::~CBulkRNAAddPanel()
{
////@begin CBulkRNAAddPanel destruction
////@end CBulkRNAAddPanel destruction
}


/*!
 * Member initialisation
 */

void CBulkRNAAddPanel::Init()
{
////@begin CBulkRNAAddPanel member initialisation
    m_RNAType = NULL;
    m_ncRNAClass = NULL;
    m_FieldSizer = NULL;
    m_RNANameSizer = NULL;
    m_Comment = NULL;
    m_ButtonsSizer = NULL;
    m_Location = NULL;
////@end CBulkRNAAddPanel member initialisation
    m_Locus = NULL;
    m_GeneDesc = NULL;
    m_tRNAname = NULL;
    m_RNAname = NULL;
    m_rRNAname = NULL;
}


/*!
 * Control creation for CBulkRNAAddPanel
 */

void CBulkRNAAddPanel::CreateControls()
{    
////@begin CBulkRNAAddPanel content construction
    CBulkRNAAddPanel* itemCBulkCmdPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemCBulkCmdPanel1->SetSizer(itemBoxSizer2);

    itemBoxSizer2->Add(0, 500, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer5Static = new wxStaticBox(itemCBulkCmdPanel1, wxID_ANY, _("RNA Type"));
    wxStaticBoxSizer* itemStaticBoxSizer5 = new wxStaticBoxSizer(itemStaticBoxSizer5Static, wxVERTICAL);
    itemBoxSizer4->Add(itemStaticBoxSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString m_RNATypeStrings;
    m_RNATypeStrings.Add(_("preRNA"));
    m_RNATypeStrings.Add(_("mRNA"));
    m_RNATypeStrings.Add(_("tRNA"));
    m_RNATypeStrings.Add(_("rRNA"));
    m_RNATypeStrings.Add(_("ncRNA"));
    m_RNATypeStrings.Add(_("tmRNA"));
    m_RNATypeStrings.Add(_("miscRNA"));
    m_RNAType = new wxChoice( itemStaticBoxSizer5->GetStaticBox(), ID_CHOICE1, wxDefaultPosition, wxDefaultSize, m_RNATypeStrings, 0 );
    m_RNAType->SetStringSelection(_("rRNA"));
    itemStaticBoxSizer5->Add(m_RNAType, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer5->Add(itemBoxSizer7, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemStaticBoxSizer5->GetStaticBox(), wxID_STATIC, _("ncRNA class"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemStaticText8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_ncRNAClassStrings;
    m_ncRNAClass = new wxComboBox( itemStaticBoxSizer5->GetStaticBox(), ID_COMBOBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_ncRNAClassStrings, wxCB_DROPDOWN );
    m_ncRNAClass->Enable(false);
    itemBoxSizer7->Add(m_ncRNAClass, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_FieldSizer = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer4->Add(m_FieldSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText11 = new wxStaticText( itemCBulkCmdPanel1, wxID_STATIC, _("RNA Name"), wxDefaultPosition, wxDefaultSize, 0 );
    m_FieldSizer->Add(itemStaticText11, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_RNANameSizer = new wxBoxSizer(wxHORIZONTAL);
    m_FieldSizer->Add(m_RNANameSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxStaticText* itemStaticText13 = new wxStaticText( itemCBulkCmdPanel1, wxID_STATIC, _("Comment"), wxDefaultPosition, wxDefaultSize, 0 );
    m_FieldSizer->Add(itemStaticText13, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Comment = new wxTextCtrl( itemCBulkCmdPanel1, ID_TEXTCTRL10, wxEmptyString, wxDefaultPosition, wxSize(300, 70), wxTE_MULTILINE );
    m_FieldSizer->Add(m_Comment, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ButtonsSizer = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer4->Add(m_ButtonsSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton16 = new wxButton( itemCBulkCmdPanel1, ID_BUTTON11, _("Add '18S-ITS-5.8S-ITS-28S' to comment"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ButtonsSizer->Add(itemButton16, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton17 = new wxButton( itemCBulkCmdPanel1, ID_BUTTON12, _("Add '16S-IGS-23S' to comment"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ButtonsSizer->Add(itemButton17, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    if (m_add_location_panel)
    {
        m_Location = new CBulkLocationPanel( itemCBulkCmdPanel1, ID_WINDOW1, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER );
        itemBoxSizer4->Add(m_Location, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    }

////@end CBulkRNAAddPanel content construction
    vector<string> class_vals = CRNA_gen::GetncRNAClassList();
    ITERATE(vector<string>, it, class_vals) {
        m_ncRNAClass->AppendString(ToWxString(*it));
    }
    
    // because we start with rRNA as the default:
    x_AddrRNAName();
    m_ButtonsSizer->ShowItems(false);
}


/*!
 * Should we show tooltips?
 */

bool CBulkRNAAddPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CBulkRNAAddPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CBulkRNAAddPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CBulkRNAAddPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CBulkRNAAddPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CBulkRNAAddPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CBulkRNAAddPanel icon retrieval
}


void CBulkRNAAddPanel::x_RemoveRNANames ()
{
    // hide feature details
    if (m_RNANameSizer->GetItemCount() > 0) {
        size_t pos = 0;
        m_RNANameSizer->GetItem(pos)->DeleteWindows();
        m_RNANameSizer->Remove(pos);
        m_RNAname = NULL;
        m_tRNAname = NULL;
        m_rRNAname = NULL;
    }
}


bool CBulkRNAAddPanel::x_AddGeneFields ()
{
    if (m_Locus == NULL && m_GeneDesc == NULL) {
        wxStaticText* itemStaticText11 = new wxStaticText( this, wxID_STATIC, _("Gene Symbol"), wxDefaultPosition, wxDefaultSize, 0 );
        m_FieldSizer->Add(itemStaticText11, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

        m_Locus = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(300, -1), 0 );
        m_FieldSizer->Add(m_Locus, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

        wxStaticText* itemStaticText13 = new wxStaticText( this, wxID_STATIC, _("Gene Description"), wxDefaultPosition, wxDefaultSize, 0 );
        m_FieldSizer->Add(itemStaticText13, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

        m_GeneDesc = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(300, -1), 0 );
        m_FieldSizer->Add(m_GeneDesc, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
        return true;
    } else {
        return false;
    }

}


bool CBulkRNAAddPanel::x_RemoveGeneFields ()
{
    bool need_layout = false;
    size_t num_items = m_FieldSizer->GetItemCount();
    while (num_items > 4) {
        size_t pos = num_items - 1;
        m_FieldSizer->GetItem(pos)->DeleteWindows();
        m_FieldSizer->Remove(pos);
        need_layout = true;
        num_items = m_FieldSizer->GetItemCount();
    }
    m_Locus = NULL;
    m_GeneDesc = NULL;
    return need_layout;
}


bool CBulkRNAAddPanel::x_AddrRNAName()
{
    bool need_layout = false;
    if (m_rRNAname == NULL) {
        x_RemoveRNANames();
        wxArrayString m_NameStrings;
        m_NameStrings.Add(_("16S ribosomal RNA"));
        m_NameStrings.Add(_("18S ribosomal RNA"));
        m_NameStrings.Add(_("23S ribosomal RNA"));
        m_NameStrings.Add(_("26S ribosomal RNA"));
        m_NameStrings.Add(_("28S ribosomal RNA"));
        m_NameStrings.Add(_("5.8S ribosomal RNA"));
        m_NameStrings.Add(_("large subunit ribosomal RNA"));
        m_NameStrings.Add(_("small subunit ribosomal RNA"));
        m_rRNAname = new wxComboBox( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_NameStrings, wxCB_DROPDOWN );
        m_RNANameSizer->Add(m_rRNAname, 0, wxALL, 5);
        need_layout = true;
    }
    return need_layout;
}


/*!
 * wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_CHOICE1
 */

void CBulkRNAAddPanel::OnSelectRNAType( wxCommandEvent& event )
{
    string rna_type = ToStdString(m_RNAType->GetStringSelection());
    bool need_layout = false;
    if (NStr::Equal(rna_type, "ncRNA")) {
        m_ncRNAClass->Enable(true);
    } else {
        m_ncRNAClass->Enable(false);
    }
    bool buttons_shown = m_ButtonsSizer->IsShown((size_t)0);
    if (NStr::Equal(rna_type, "miscRNA")) {
        if (!buttons_shown) {
            m_ButtonsSizer->ShowItems(true);
            need_layout = true;
        }
    } else {
        if (buttons_shown) {
            m_ButtonsSizer->ShowItems(false);
            need_layout = true;
        }
    }
    // create RNA Name control
    if (NStr::Equal(rna_type, "tRNA")) {
        if (m_tRNAname == NULL) {
            x_RemoveRNANames();
            wxArrayString m_tRNAStrings;
            m_tRNAStrings.Add(_("A Alanine"));
            m_tRNAStrings.Add(_("B Asp or Asn"));
            m_tRNAStrings.Add(_("C Cysteine"));
            m_tRNAStrings.Add(_("D Aspartic Acid"));
            m_tRNAStrings.Add(_("E Glutamic Acid"));
            m_tRNAStrings.Add(_("F Phenylalanine"));
            m_tRNAStrings.Add(_("G Glycine"));
            m_tRNAStrings.Add(_("H Histidine"));
            m_tRNAStrings.Add(_("I Isoleucine"));
            m_tRNAStrings.Add(_("J Leu or Ile"));
            m_tRNAStrings.Add(_("K Lysine"));
            m_tRNAStrings.Add(_("L Leucine"));
            m_tRNAStrings.Add(_("M Methionine"));
            m_tRNAStrings.Add(_("N Asparagine"));
            m_tRNAStrings.Add(_("O Pyrrolysine"));
            m_tRNAStrings.Add(_("P Proline"));
            m_tRNAStrings.Add(_("Q Glutamine"));
            m_tRNAStrings.Add(_("R Arginine"));
            m_tRNAStrings.Add(_("S Serine"));
            m_tRNAStrings.Add(_("T Threonine"));
            m_tRNAStrings.Add(_("U Selenocysteine"));
            m_tRNAStrings.Add(_("V Valine"));
            m_tRNAStrings.Add(_("W Tryptophan"));
            m_tRNAStrings.Add(_("X Undetermined or atypical"));
            m_tRNAStrings.Add(_("Y Tyrosine"));
            m_tRNAStrings.Add(_("Z Glu or Gln"));
            m_tRNAname = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_tRNAStrings, 0 );
            m_RNANameSizer->Add(m_tRNAname, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
            need_layout = true;
        }
        need_layout |= x_AddGeneFields();
    } else if (NStr::Equal(rna_type, "rRNA")) {
        need_layout |= x_RemoveGeneFields();
        need_layout |= x_AddrRNAName();

    } else {
        if (NStr::Equal(rna_type, "ncRNA") || NStr::Equal(rna_type, "mRNA")) {
            need_layout |= x_AddGeneFields();
        } else {
            need_layout |= x_RemoveGeneFields();
        }

        if (m_RNAname == NULL) {
            x_RemoveRNANames();
            m_RNAname = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(300, -1), 0 );
            m_RNANameSizer->Add(m_RNAname, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
            need_layout = true;
        }
    }
    if (need_layout) {
        Fit();        
        Layout();
    }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON11
 */

void CBulkRNAAddPanel::OnAdd18SToComment( wxCommandEvent& event )
{
    string comment = ToStdString(m_Comment->GetValue());
    if (!NStr::IsBlank(comment)) {
        comment += "; ";
    }
    comment += "contains 18S ribosomal RNA, internal transcribed spacer 1, 5.8S ribosomal RNA, internal transcribed spacer 2, and 28S ribosomal RNA";
    m_Comment->SetValue(ToWxString(comment));
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON12
 */

void CBulkRNAAddPanel::OnAdd16StoComment( wxCommandEvent& event )
{
    string comment = ToStdString(m_Comment->GetValue());
    if (!NStr::IsBlank(comment)) {
        comment += "; ";
    }
    comment += "contains 16S ribosomal RNA, 16S-23S ribosomal RNA intergenic spacer, and 23S ribosomal RNA";
    m_Comment->SetValue(ToWxString(comment));
}


static void s_SettRNAProduct(string aa, CRef<CRNA_ref> rna)
{
    rna->SetExt().SetTRNA();
    if (NStr::IsBlank(aa)) {
        return;
    }
    aa = NStr::ToUpper(aa);
    char a = aa.c_str()[0];
    rna->SetExt().SetTRNA().SetAa().SetNcbieaa(a);
}


CRNA_ref::TType CBulkRNAAddPanel::x_GetRnaType()
{
    CRNA_ref::TType rna_type = CRNA_ref::eType_miscRNA;
    switch (m_RNAType->GetSelection()) {
        case 0:
            rna_type = CRNA_ref::eType_premsg;
            break;
        case 1:
            rna_type = CRNA_ref::eType_mRNA;
            break;
        case 2:
            rna_type = CRNA_ref::eType_tRNA;
            break;
        case 3:
            rna_type = CRNA_ref::eType_rRNA;
            break;
        case 4:
            rna_type = CRNA_ref::eType_ncRNA;
            break;
        case 5:
            rna_type = CRNA_ref::eType_tmRNA;
            break;
        case 6:
            rna_type = CRNA_ref::eType_miscRNA;
            break;
        case 7:
            rna_type = CRNA_ref::eType_other;
            break;
    }
    return rna_type;
}


string CBulkRNAAddPanel::x_GetRnaName()
{
    string rna_name = "";
    if (m_RNAname != NULL) {
        rna_name = ToStdString(m_RNAname->GetValue());
    } else if (m_rRNAname != NULL) {
        rna_name = ToStdString(m_rRNAname->GetValue());
    }
    rna_name = GetAdjustedRnaName(rna_name);
    return rna_name;
}


CRef<CCmdComposite> CBulkRNAAddPanel::GetCommand()
{   
    CRNA_ref::TType rna_type = x_GetRnaType();
    string rna_name = x_GetRnaName();

    if (NStr::IsBlank(rna_name) && rna_type == CRNA_ref::eType_rRNA) {
        CRef<CCmdComposite> empty(NULL);
        return empty;
    }
 
    CRef<CCmdComposite> cmd (new CCmdComposite("Bulk Add RNA"));
    objects::CBioseq_CI b_iter(m_TopSeqEntry, objects::CSeq_inst::eMol_na);
    for ( ; b_iter ; ++b_iter ) {
        AddOneCommand(*b_iter,cmd);

    }

    return cmd;
}

void CBulkRNAAddPanel::AddOneCommand(const CBioseq_Handle& bsh, CRef<CCmdComposite> cmd)
{
    CRNA_ref::TType rna_type = x_GetRnaType();
    string rna_name = x_GetRnaName();
    string comment = ToStdString(m_Comment->GetValue());
    string ncrna_class = ToStdString (m_ncRNAClass->GetStringSelection());
    string gene_symbol = "";
    if (m_Locus) {
        gene_symbol = ToStdString(m_Locus->GetValue());
    }
    string gene_desc = "";
    if (m_GeneDesc) {
        gene_desc = ToStdString(m_GeneDesc->GetValue());
    }
    CRef<CRNA_ref> rna_ref(new CRNA_ref());
    rna_ref->SetType(rna_type);
    // set product
    switch (rna_type) {
        case CRNA_ref::eType_ncRNA:
        case CRNA_ref::eType_tmRNA:
        case CRNA_ref::eType_miscRNA:
            rna_ref->SetExt().SetGen();
            if (!NStr::IsBlank(rna_name)) {
                rna_ref->SetExt().SetGen().SetProduct(rna_name);
            }
            if (rna_type == CRNA_ref::eType_ncRNA && !NStr::IsBlank(ncrna_class)) {
                rna_ref->SetExt().SetGen().SetClass(ncrna_class);
            }
            break;
        case CRNA_ref::eType_tRNA:
            if (m_tRNAname != NULL) {
                s_SettRNAProduct(ToStdString(m_tRNAname->GetStringSelection()), rna_ref);
            }
            break;
        default:
            if (!NStr::IsBlank(rna_name)) {
                rna_ref->SetExt().SetName(rna_name);
            }
            break;
    }

    objects::CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
    CRef<objects::CSeq_feat> rna(new objects::CSeq_feat());
    rna->SetData().SetRna().Assign(*rna_ref);
    if (!ShouldAddToExisting() && AlreadyHasFeature(bsh, rna->GetData().GetSubtype())) {
        return;
    }
    
    if (!NStr::IsBlank(comment)) {
        rna->SetComment(comment);
    }
    CRef<objects::CSeq_loc> loc = GetLocForBioseq(*(bsh.GetCompleteBioseq()));
    rna->SetLocation().Assign(*loc);
    if (rna->GetLocation().IsPartialStart(objects::eExtreme_Biological)
        || rna->GetLocation().IsPartialStop(objects::eExtreme_Biological)) {
        rna->SetPartial(true);
    }
    cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *rna)));

    if (!NStr::IsBlank(gene_symbol) || !NStr::IsBlank(gene_desc)) {
        // create gene feature
        CRef<objects::CSeq_feat> new_gene(new objects::CSeq_feat());
        new_gene->SetData().SetGene().SetLocus(gene_symbol);
        new_gene->SetData().SetGene().SetDesc(gene_desc);
        new_gene->SetLocation().Assign(*loc);
        if (new_gene->GetLocation().IsPartialStart(objects::eExtreme_Biological)
            || new_gene->GetLocation().IsPartialStop(objects::eExtreme_Biological)) {
            new_gene->SetPartial(true);
        }
        cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *new_gene)));
    }
}

string CBulkRNAAddPanel::GetErrorMessage()
{
    CRNA_ref::TType rna_type = x_GetRnaType();
    string rna_name = x_GetRnaName();
    if (NStr::IsBlank(rna_name) && rna_type == CRNA_ref::eType_rRNA) {
        return "Must specify rRNA product name!";
    } else {
        return "Unknown error";
    }
}

END_NCBI_SCOPE
