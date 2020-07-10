/*  $Id: pubstatus_panel.cpp 43202 2019-05-28 18:05:59Z filippov $
* ===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software / database is a "United States Government Work" under the
*  terms of the United States Copyright Act.It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.This software / database is freely available
*  to the public for use.The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data.The NLM and the U.S.
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
*/

#include <ncbi_pch.hpp>
#include <gui/widgets/edit/pubauthor_panel.hpp>
#include <gui/widgets/edit/unpublishedref_panel.hpp>
#include <gui/widgets/edit/publishedref_panel.hpp>
#include <gui/widgets/edit/pubstatus_panel.hpp>
#include <gui/widgets/edit/reference_panel.hpp>

#include <objects/pub/Pub_equiv.hpp>
#include <objects/pub/Pub.hpp>
#include <objects/biblio/Cit_gen_.hpp>
#include <objects/biblio/Cit_art.hpp>
#include <objects/biblio/Cit_jour.hpp>
#include <objects/biblio/Imprint.hpp>
#include <objects/general/Person_id.hpp>
#include <objects/general/Name_std.hpp>

#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <objmgr/seq_entry_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>

#include <wx/sizer.h>
#include <wx/radiobut.h>
#include <wx/stattext.h>
#include <wx/icon.h>
#include <wx/simplebook.h>

BEGIN_NCBI_SCOPE

/*
 * CPubStatusPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CPubStatusPanel, wxPanel )


/*
 * CPubStatusPanel event table definition
 */

BEGIN_EVENT_TABLE( CPubStatusPanel, wxPanel )

    EVT_RADIOBUTTON(ID_UNPUBLISHEDBTN, CPubStatusPanel::OnUnpublishedSelected)
    EVT_RADIOBUTTON(ID_INPRESSBTN, CPubStatusPanel::OnInpressSelected)
    EVT_RADIOBUTTON(ID_PUBLISHEDBTN, CPubStatusPanel::OnPublishedSelected)
    EVT_RADIOBUTTON(ID_REFSEQAUTHORSBTN, CPubStatusPanel::OnSameAuthorsSelected)
    EVT_RADIOBUTTON(ID_NEWAUTHORSBTN, CPubStatusPanel::OnNewAuthorsSelected)

END_EVENT_TABLE()


/*
 * CPubStatusPanel constructors
 */

CPubStatusPanel::CPubStatusPanel()
{
    Init();
}

CPubStatusPanel::CPubStatusPanel( wxWindow* parent, ICommandProccessor*  cmdproc, objects::CSeq_entry_Handle seh, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
: m_CmdProcessor(cmdproc), m_Seh(seh)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * CPubStatusPanel creator
 */

bool CPubStatusPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CPubStatusPanel creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CPubStatusPanel creation
    return true;
}


/*
 * CPubStatusPanel destructor
 */

CPubStatusPanel::~CPubStatusPanel()
{
////@begin CPubStatusPanel destruction
////@end CPubStatusPanel destruction
}


/*
 * Member initialisation
 */

void CPubStatusPanel::Init()
{
    m_AuthorsPanel = NULL;
    ////@begin CPubStatusPanel member initialisation
    m_Notebook = NULL;
    m_UnpublishedRef = NULL;
    m_InpressRef = NULL;
    m_PublishedRef = NULL;
////@end CPubStatusPanel member initialisation
    m_Unpublished = nullptr;
    m_InPress = nullptr;
    m_Published = nullptr;
    m_SameAuthors = nullptr;
    m_NewAuthors = nullptr;
}


/*
 * Control creation for CPubStatusPanel
 */

void CPubStatusPanel::CreateControls()
{    
////@begin CPubStatusPanel content construction
    CPubStatusPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    m_Unpublished = new wxRadioButton( itemPanel1, ID_UNPUBLISHEDBTN, _("Unpublished"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_Unpublished->SetValue(true);
    itemBoxSizer4->Add(m_Unpublished, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_InPress = new wxRadioButton( itemPanel1, ID_INPRESSBTN, _("In-press"), wxDefaultPosition, wxDefaultSize, 0 );
    m_InPress->SetValue(false);
    itemBoxSizer4->Add(m_InPress, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Published = new wxRadioButton( itemPanel1, ID_PUBLISHEDBTN, _("Published"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Published->SetValue(false);
    itemBoxSizer4->Add(m_Published, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Notebook = new wxSimplebook(itemPanel1, wxID_ANY);
    itemBoxSizer2->Add(m_Notebook, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);

    m_UnpublishedRef = new CUnpublishedRefPanel(m_Notebook);
    m_Notebook->AddPage(m_UnpublishedRef, wxEmptyString, true);

    m_InpressRef = new CPublishedRefPanel(m_Notebook, true);
    m_Notebook->AddPage(m_InpressRef, wxEmptyString, false);

    m_PublishedRef = new CPublishedRefPanel(m_Notebook, false);
    m_Notebook->AddPage(m_PublishedRef, wxEmptyString, false);

    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer1, 0, wxALIGN_CENTER_HORIZONTAL|wxLEFT, 5);

    wxStaticText* itemStaticText2 = new wxStaticText( itemPanel1, wxID_STATIC, _("Reference authors:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer1->Add(itemStaticText2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_SameAuthors = new wxRadioButton( itemPanel1, ID_REFSEQAUTHORSBTN, _("Same as sequence authors"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_SameAuthors->SetValue(true);
    itemBoxSizer1->Add( m_SameAuthors, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_NewAuthors = new wxRadioButton( itemPanel1, ID_NEWAUTHORSBTN, _("Specify new authors"), wxDefaultPosition, wxDefaultSize, 0 );
    m_NewAuthors->SetValue(false);
    itemBoxSizer1->Add(m_NewAuthors, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CPubStatusPanel content construction
    CRef<objects::CAuth_list> authlist(new objects::CAuth_list());
    m_AuthorsPanel = new CAuthorNamesPanel(itemPanel1, *authlist, false);
    itemBoxSizer2->Add(m_AuthorsPanel, 1, wxGROW|wxRESERVE_SPACE_EVEN_IF_HIDDEN |wxALL, 5);
    m_AuthorsPanel->Hide();

}

void CPubStatusPanel::OnUnpublishedSelected(wxCommandEvent& event)
{
    m_Notebook->ChangeSelection(0);
    CReferencePanel* reference_panel = x_GetReferencePanel();
    auto& gen = m_Pubdesc->SetPub().Set().front()->SetGen();
    m_Unpublished->SetValue(true);
    m_UnpublishedRef->SetCitGen(gen);
    bool match_authors = true;
    if (reference_panel && gen.IsSetAuthors() && !reference_panel->MatchesCitSubAuthors(gen.GetAuthors())) {
        match_authors = false;
        m_AuthorsPanel->SetAuthors(gen.GetAuthors());
    }
    if (match_authors) {
        m_SameAuthors->SetValue(true);
        m_AuthorsPanel->Show(false);
    } else {
        m_NewAuthors->SetValue(true);
        m_AuthorsPanel->Show(true);
    }
}
void CPubStatusPanel::OnInpressSelected(wxCommandEvent& event)
{
    m_Notebook->ChangeSelection(1);
    CReferencePanel* reference_panel = x_GetReferencePanel();
    auto& art = m_Pubdesc->SetPub().Set().front()->SetArticle();
    art.SetFrom().SetJournal().SetImp().SetPrepub(objects::CImprint::ePrepub_in_press);
    m_InPress->SetValue(true);
    m_InpressRef->SetCitArt(art);
    bool match_authors = true;
    if (reference_panel && art.IsSetAuthors() && !reference_panel->MatchesCitSubAuthors(art.GetAuthors())) {
        match_authors = false;
        m_AuthorsPanel->SetAuthors(art.GetAuthors());
    }
    if (match_authors) {
        m_SameAuthors->SetValue(true);
        m_AuthorsPanel->Show(false);
    } else {
        m_NewAuthors->SetValue(true);
        m_AuthorsPanel->Show(true);
    }
}

void CPubStatusPanel::OnPublishedSelected(wxCommandEvent& event)
{
    m_Notebook->ChangeSelection(2);
    CReferencePanel* reference_panel = x_GetReferencePanel();
    auto& art = m_Pubdesc->SetPub().Set().front()->SetArticle();
    art.SetFrom().SetJournal().SetImp().ResetPrepub();
    m_Published->SetValue(true);
    m_PublishedRef->SetCitArt(art);
    bool match_authors = true;
    if (reference_panel && art.IsSetAuthors() && !reference_panel->MatchesCitSubAuthors(art.GetAuthors())) {
        match_authors = false;
        m_AuthorsPanel->SetAuthors(art.GetAuthors());
    }
    if (match_authors) {
        m_SameAuthors->SetValue(true);
        m_AuthorsPanel->Show(false);
    } else {
        m_NewAuthors->SetValue(true);
        m_AuthorsPanel->Show(true);
    }
}

void CPubStatusPanel::OnSameAuthorsSelected(wxCommandEvent& event)
{
    m_AuthorsPanel->Show(false);
    CReferencePanel* reference_panel = x_GetReferencePanel();
    if (reference_panel) {
        if (m_Pubdesc->SetPub().Set().front()->IsGen())
        {
            auto& gen = m_Pubdesc->SetPub().Set().front()->SetGen();
            reference_panel->CopyAuthors(gen.SetAuthors());
        }
        if (m_Pubdesc->SetPub().Set().front()->IsArticle())
        {
            auto& art = m_Pubdesc->SetPub().Set().front()->SetArticle();
            reference_panel->CopyAuthors(art.SetAuthors());
        }
    }
}

void CPubStatusPanel::OnNewAuthorsSelected(wxCommandEvent& event)
{
    m_AuthorsPanel->Show(true);
    if (m_Pubdesc->SetPub().Set().front()->IsGen())
    {
        auto& gen = m_Pubdesc->SetPub().Set().front()->SetGen();
        m_AuthorsPanel->PopulateAuthors(gen.SetAuthors());
    }
    if (m_Pubdesc->SetPub().Set().front()->IsArticle())
    {
        auto& art = m_Pubdesc->SetPub().Set().front()->SetArticle();
        m_AuthorsPanel->PopulateAuthors(art.SetAuthors());
    }
}

/*
 * Should we show tooltips?
 */

bool CPubStatusPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CPubStatusPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CPubStatusPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CPubStatusPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CPubStatusPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CPubStatusPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CPubStatusPanel icon retrieval
}


CReferencePanel* CPubStatusPanel::x_GetReferencePanel()
{
    CReferencePanel* reference_panel = nullptr;
    wxWindow* parent = this->GetParent();
    reference_panel = dynamic_cast<CReferencePanel*>(parent);
    while (parent && !reference_panel) {
        parent = parent->GetParent();
        reference_panel = dynamic_cast<CReferencePanel*>(parent);
    }
    return reference_panel;
}

bool CPubStatusPanel::TransferDataToWindow()
{
    // need to ask parent reference panel if authors of pub matches sequence authors
    CReferencePanel* reference_panel = x_GetReferencePanel();
 
    bool match_authors = true;
    bool in_press = false;

    if (m_Pubdesc && m_Pubdesc->IsSetPub() && m_Pubdesc->GetPub().IsSet() &&
        !m_Pubdesc->GetPub().Get().empty()) {
        bool found_pmid = false;
        bool found_pub = false;
        for (auto pub : m_Pubdesc->GetPub().Get()) {            
            switch (pub->Which()) {
            case objects::CPub::e_Gen:
                if (!found_pub) {
                    m_Notebook->ChangeSelection(0);
                    m_Unpublished->SetValue(true);
                    m_UnpublishedRef->SetCitGen(pub->GetGen());
                    if (reference_panel && pub->GetGen().IsSetAuthors() && !reference_panel->MatchesCitSubAuthors(pub->GetGen().GetAuthors())) {
                        match_authors = false;
                        m_AuthorsPanel->SetAuthors(pub->GetGen().GetAuthors());
                    }
                    found_pub = true;
                }
                break;
            case objects::CPub::e_Article:
                if (!found_pub) {
                    if (!pub->GetArticle().IsSetFrom() || !pub->GetArticle().GetFrom().IsJournal() ||
                        !pub->GetArticle().GetFrom().GetJournal().IsSetImp() ||
                        pub->GetArticle().GetFrom().GetJournal().GetImp().IsSetPrepub()) {
                        in_press = true;
                    }
                    if (in_press) {
                        m_Notebook->ChangeSelection(1);
                        m_InPress->SetValue(true);
                        m_InpressRef->SetCitArt(pub->SetArticle());
                    }
                    else {
                        m_Notebook->ChangeSelection(2);
                        m_Published->SetValue(true);
                        m_PublishedRef->SetCitArt(pub->SetArticle());
                    }
                    if (reference_panel && pub->GetArticle().IsSetAuthors() && !reference_panel->MatchesCitSubAuthors(pub->GetArticle().GetAuthors())) {
                        match_authors = false;
                        m_AuthorsPanel->SetAuthors(pub->GetArticle().GetAuthors());
                    }
                    found_pub = true;
                }
                break;
            case objects::CPub::e_Pmid:
                if (!found_pmid) {
                    m_PublishedRef->SetPmid(pub->GetPmid().Get());
                    found_pmid = true;
                }
                break;
            default:
                break;
            }
            if (found_pub && found_pmid) {
                break;
            }
        }
    }


    if (match_authors) {
        m_SameAuthors->SetValue(true);
        m_AuthorsPanel->Show(false);
    } else {
        m_NewAuthors->SetValue(true);
        m_AuthorsPanel->Show(true);
    }

    return true;
}

bool CPubStatusPanel::TransferDataFromWindow()
{
    if (!m_Pubdesc || !m_Pubdesc->IsSetPub() || !m_Pubdesc->GetPub().IsSet() || m_Pubdesc->GetPub().Get().empty()) {
        return false;
    }

    bool use_same = m_SameAuthors->GetValue();
    CReferencePanel* reference_panel = x_GetReferencePanel();
    if (!reference_panel)
        return false;
    switch (m_Notebook->GetSelection()) {
    case 0:
    {
        auto& gen = m_Pubdesc->SetPub().Set().front()->SetGen();
        m_UnpublishedRef->UpdateCitGen(gen);
        // collect authors
        if (use_same) {
            reference_panel->CopyAuthors(gen.SetAuthors());
        }
        else {
            m_AuthorsPanel->PopulateAuthors(gen.SetAuthors());        
            if (!gen.GetAuthors().IsSetAffil()) {
                reference_panel->CopyAffil(gen.SetAuthors());
            }
        }
        break;
    }
    case 1:
    {
        auto& art = m_Pubdesc->SetPub().Set().front()->SetArticle();
        art.SetFrom().SetJournal().SetImp().SetPrepub(objects::CImprint::ePrepub_in_press);
        m_InpressRef->TransferDataFromWindow();
        // collect authors
        if (use_same) {
            reference_panel->CopyAuthors(art.SetAuthors());
        } else {
            m_AuthorsPanel->PopulateAuthors(art.SetAuthors());
            if (!art.GetAuthors().IsSetAffil()) {
                reference_panel->CopyAffil(art.SetAuthors());
            }
        }
        break;
    }
    case 2:
    {
        auto& art = m_Pubdesc->SetPub().Set().front()->SetArticle();
        art.SetFrom().SetJournal().SetImp().ResetPrepub();
        m_PublishedRef->TransferDataFromWindow();       
        // collect authors
        if (use_same) {
            reference_panel->CopyAuthors(art.SetAuthors());
        } else {
            m_AuthorsPanel->PopulateAuthors(art.SetAuthors());
            if (!art.GetAuthors().IsSetAffil()) {
                reference_panel->CopyAffil(art.SetAuthors());
            }
        }

        // get PMID
        int pmid = m_PublishedRef->GetPmid();
        if (pmid > 0) {
            if (m_Pubdesc->GetPub().Get().size() > 1) {
                m_Pubdesc->SetPub().Set().back()->SetPmid().Set(pmid);
            } else {
                CRef<objects::CPub> newpub(new objects::CPub());
                newpub->SetPmid().Set(pmid);
                m_Pubdesc->SetPub().Set().push_back(newpub);
            }
        }
        break;
    }
    default:
        break;
    }

    return true;
}

void CPubStatusPanel::ApplyCitSub(objects::CCit_sub& sub)
{
    m_Sub.Reset(&sub);
    TransferDataToWindow();
}

void CPubStatusPanel::ApplyPub(objects::CPubdesc& pubdesc)
{
    m_Pubdesc.Reset(&pubdesc);
    TransferDataToWindow();
}

void CPubStatusPanel::ApplyCommand()
{
    if (!TransferDataFromWindow())
        return;

    CRef<objects::CSeqdesc> empty(new objects::CSeqdesc());
    CRef<objects::CPub> citgen(new objects::CPub());
    citgen->SetGen().SetCit("unpublished");
    empty->SetPub().SetPub().Set().push_back(citgen);

    if (m_Pubdesc->Equals(empty->GetPub()))
        return;

    CRef<CCmdComposite> cmd(new CCmdComposite("update publication"));
    bool any_changes = false;
    bool found = false;

    for ( objects::CSeq_entry_CI entry_it(m_Seh, objects::CSeq_entry_CI::fRecursive|objects::CSeq_entry_CI::fIncludeGivenEntry); entry_it; ++entry_it ) 
    {
        for ( objects::CSeqdesc_CI di(*entry_it, objects::CSeqdesc::e_Pub, 1); di; ++di) 
        {
            // edit existing descriptor
            CRef<objects::CSeqdesc> cpy(new objects::CSeqdesc());
            cpy->SetPub(*m_Pubdesc);
            if (!di->Equals(*cpy)) 
            {
                CRef<CCmdChangeSeqdesc> chg(new CCmdChangeSeqdesc(di.GetSeq_entry_Handle(), *di, *cpy));
                cmd->AddCommand(*chg);
                any_changes = true;
            }
            found = true;             
        }
    }
    
    if (!found) 
    {
        CRef<objects::CSeqdesc> new_desc(new objects::CSeqdesc());
        new_desc->SetPub(*m_Pubdesc);
        CIRef<IEditCommand> cmdAddDesc(new CCmdCreateDesc(m_Seh, *new_desc));
        cmd->AddCommand(*cmdAddDesc);
        any_changes = true;
    }
    
    
    if (any_changes) 
    {
        m_CmdProcessor->Execute(cmd);
    }
}

void CPubStatusPanel::ReportMissingFields(string &text)
{
    switch (m_Notebook->GetSelection()) 
    {
    case 0:
    {
        m_UnpublishedRef->ReportMissingFields(text);
        break;
    }
    case 1:
    {
        m_InpressRef->ReportMissingFields(text);
        break;
    }
    case 2:
    {
        m_PublishedRef->ReportMissingFields(text);       
        break;
    }
    default:
        break;
    }

    if (! m_SameAuthors->GetValue())
    {
        CRef<objects::CAuth_list> authlist(new objects::CAuth_list());
        m_AuthorsPanel->PopulateAuthors(*authlist);
        if (!authlist->IsSetNames() ||
            (authlist->GetNames().IsStd() && authlist->GetNames().GetStd().empty()) ||
            !authlist->GetNames().GetStd().front()->IsSetName() ||
            (authlist->GetNames().GetStd().front()->GetName().IsName() && authlist->GetNames().GetStd().front()->GetName().IsName() && 
             authlist->GetNames().GetStd().front()->GetName().GetName().IsSetLast() && authlist->GetNames().GetStd().front()->GetName().GetName().GetLast() == "?"))
            text += "Publication Authors\n";
    }
}

END_NCBI_SCOPE

