  /*  $Id: clear_keywords.cpp 39538 2017-10-06 14:50:51Z bollin $
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
 * Authors:  Igor Filippov
 */


#include <ncbi_pch.hpp>

////@begin includes
////@end includes
#include <sstream>

#include <objmgr/scope.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/seq_entry_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <objects/seq/seq_id_handle.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <gui/objutils/cmd_change_bioseq_set.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/clear_keywords.hpp>

#include <wx/button.h>
#include <wx/statbox.h>
#include <wx/stattext.h>

////@begin XPM images
////@end XPM images



BEGIN_NCBI_SCOPE



IMPLEMENT_DYNAMIC_CLASS( CClearKeywords, wxDialog )


/*!
 * CClearKeywords event table definition
 */

BEGIN_EVENT_TABLE( CClearKeywords, wxDialog )
EVT_CHECKBOX(wxID_ANY, CClearKeywords::OnAllKeywordsSelected)

END_EVENT_TABLE()


/*!
 * CClearKeywords constructors
 */

CClearKeywords::CClearKeywords()
{
    Init();
}

CClearKeywords::CClearKeywords( wxWindow* parent, objects::CSeq_entry_Handle seh, 
                          wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_TopSeqEntry(seh)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CVectorTrimDlg creator
 */

bool CClearKeywords::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CClearKeywords creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CClearKeywords creation

    return true;
}


/*!
 * CClearKeywords destructor
 */

CClearKeywords::~CClearKeywords()
{
////@begin CClearKeywords destruction
////@end CClearKeywords destruction
}


/*!
 * Member initialisation
 */

void CClearKeywords::Init()
{
}


/*!
 * Control creation for CClearKeywords
 */

void CClearKeywords::CreateControls()
{    
    CClearKeywords* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);
    
    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* stattext2 = new wxStaticText( itemDialog1, wxID_STATIC, _("Remove Keyword"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(stattext2, 0, wxALIGN_TOP|wxALL, 5);
    m_Keyword = new wxTextCtrl( itemDialog1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(100, -1), 0 );
    itemBoxSizer3->Add(m_Keyword, 0, wxALIGN_TOP|wxLEFT|wxRIGHT|wxBOTTOM, 5);
    m_AllKeywords = new wxCheckBox(itemDialog1, wxID_ANY, _("All Keywords"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer3->Add(m_AllKeywords, 0, wxALIGN_TOP | wxLEFT | wxRIGHT | wxBOTTOM, 5);

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Constraints"));
    wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer3->Add(itemBoxSizer7, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Any = new wxRadioButton( itemDialog1, wxID_ANY, _("Any sequence"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_Any->SetValue(true);
    itemBoxSizer7->Add(m_Any, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Nuc = new wxRadioButton( itemDialog1, wxID_ANY, _("Nucleotides"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Nuc->SetValue(false);
    itemBoxSizer7->Add(m_Nuc, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_DNA = new wxRadioButton( itemDialog1, wxID_ANY, _("DNA"), wxDefaultPosition, wxDefaultSize, 0 );
    m_DNA->SetValue(false);
    itemBoxSizer7->Add(m_DNA, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_RNA = new wxRadioButton( itemDialog1, wxID_ANY, _("RNA"), wxDefaultPosition, wxDefaultSize, 0 );
    m_RNA->SetValue(false);
    itemBoxSizer7->Add(m_RNA, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Proteins = new wxRadioButton( itemDialog1, wxID_ANY, _("Proteins"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Proteins->SetValue(false);
    itemBoxSizer7->Add(m_Proteins, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
  
    wxArrayString choices;
    set<string> str_choices;
    for ( CSeq_entry_CI entry_it(m_TopSeqEntry, CSeq_entry_CI::fRecursive|CSeq_entry_CI::fIncludeGivenEntry); entry_it; ++entry_it ) 
    {
        for ( CSeqdesc_CI biosource_it(*entry_it, CSeqdesc::e_Source, 1); biosource_it; ++biosource_it) 
        {
            if (biosource_it->GetSource().IsSetGenome())
            {
                string line = CBioSource::GetOrganelleByGenome(biosource_it->GetSource().GetGenome());
                str_choices.insert(line);
            }
        }
    }
    for (set<string>::const_iterator str_it = str_choices.begin(); str_it != str_choices.end(); ++str_it)
    {
        choices.Add(wxString(*str_it));
    }

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer3->Add(itemBoxSizer9, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText12 = new wxStaticText( itemDialog1, wxID_STATIC, _("Where source location is "), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer9->Add(itemStaticText12, 0, wxALIGN_TOP|wxALL, 5);

    m_SourceLoc = new wxListBox( itemDialog1, wxID_ANY, wxDefaultPosition, wxDefaultSize, choices, wxLB_SINGLE );
    itemBoxSizer9->Add(m_SourceLoc, 0, wxALIGN_TOP|wxALL, 5);

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer3->Add(itemBoxSizer10, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText11 = new wxStaticText( itemDialog1, wxID_STATIC, _("Where Sequence ID"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer10->Add(itemStaticText11, 0, wxALIGN_TOP|wxALL, 5);

    m_StringConstraintPanel = new CStringConstraintPanel( itemDialog1, false, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer10->Add(m_StringConstraintPanel, 0, wxALIGN_TOP|wxALL|wxFIXED_MINSIZE, 0);
    m_StringConstraintPanel->SetStringSelection(_("Is one of"));

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton13 = new wxButton( itemDialog1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton14 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
}



/*!
 * Should we show tooltips?
 */

bool CClearKeywords::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CClearKeywords::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon CClearKeywords::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
}

CRef<CCmdComposite> CClearKeywords::GetCommand()
{
    if (m_Keyword->GetValue().IsEmpty() && !m_AllKeywords->GetValue())
        return CRef<CCmdComposite>(NULL);
    CRef<CCmdComposite> cmd( new CCmdComposite("Remove Keyword") );
    bool pass_genome = true;
    if (m_SourceLoc->GetSelection() != wxNOT_FOUND)
        pass_genome = false;
    ApplyToCSeq_entry(*(m_TopSeqEntry.GetCompleteSeq_entry()), cmd, pass_genome);
    return cmd;
}

void CClearKeywords::ApplyToCSeq_entry(const CSeq_entry& se, CCmdComposite* composite, bool pass_genome)  
{

    if (!pass_genome)
    {
        CBioSource::EGenome organelle = CBioSource::GetGenomeByOrganelle(m_SourceLoc->GetString(m_SourceLoc->GetSelection()).ToStdString()); 

        FOR_EACH_SEQDESC_ON_SEQENTRY (it, se) 
        {
            if ((*it)->IsSource() && (*it)->GetSource().IsSetGenome() && (*it)->GetSource().GetGenome() == organelle)
            {
                pass_genome = true;
            }
        }
    }

    if (se.IsSet()) {
        FOR_EACH_SEQENTRY_ON_SEQSET (it, se.GetSet()) {
            ApplyToCSeq_entry (**it, composite, pass_genome);
        }
    }

    if (!pass_genome)
        return;

    bool pass = false;
    if (m_Any->GetValue())
        pass = true;
    if (m_Nuc->GetValue() && se.IsSeq() && se.GetSeq().IsNa() )
        pass = true;
    if (m_DNA->GetValue() && se.IsSeq() && se.GetSeq().IsSetInst() && se.GetSeq().GetInst().IsSetMol() &&          
        se.GetSeq().GetInst().GetMol() == CSeq_inst::eMol_dna )
        pass = true;
    if (m_RNA->GetValue() && se.IsSeq() && se.GetSeq().IsSetInst() && se.GetSeq().GetInst().IsSetMol() &&          
        se.GetSeq().GetInst().GetMol() == CSeq_inst::eMol_rna)
        pass = true;
    if (m_Proteins->GetValue() && se.IsSeq() && se.GetSeq().IsAa() )
        pass = true;

    if (!pass)
        return;

    CRef<edit::CStringConstraint> constraint(m_StringConstraintPanel->GetStringConstraint());

    if (constraint && se.IsSeq())
    {
        pass = false;
        FOR_EACH_SEQID_ON_BIOSEQ(id, se.GetSeq())
        {
            string label;
            (*id)->GetLabel(&label,CSeq_id::eContent);
            if (constraint->DoesTextMatch(label))
                pass = true;
        }
        if (!pass)
            return;
    }


    string remove = m_Keyword->GetValue().ToStdString();
    bool remove_all = m_AllKeywords->GetValue();
    FOR_EACH_SEQDESC_ON_SEQENTRY (it, se) 
    {
        if ((*it)->IsGenbank())
        {
            CRef<CSerialObject> edited_object;
            edited_object.Reset((CSerialObject*)CGB_block::GetTypeInfo()->Create());
            edited_object->Assign((*it)->GetGenbank());
            CGB_block& edited = dynamic_cast<CGB_block&>(*edited_object);

            EDIT_EACH_KEYWORD_ON_GENBANKBLOCK(keyword, edited)
            {
                if (remove_all || *keyword == remove)
                    ERASE_KEYWORD_ON_GENBANKBLOCK(keyword, edited);
            }
            if (edited.IsSetKeywords() && edited.GetKeywords().empty())
                edited.ResetKeywords();
            CChangeGenbankBlockCommand* cmd = new CChangeGenbankBlockCommand();
            CObject* actual = (CObject*) &((*it)->GetGenbank());
            cmd->Add(actual, CConstRef<CObject>(edited_object));
            composite->AddCommand(*cmd);            
        }
    }
   
}


void CClearKeywords::OnAllKeywordsSelected(wxCommandEvent& event)
{
    if (m_AllKeywords->GetValue()) {
        m_Keyword->Enable(false);
    } else {
        m_Keyword->Enable(true);
    }
}



END_NCBI_SCOPE
