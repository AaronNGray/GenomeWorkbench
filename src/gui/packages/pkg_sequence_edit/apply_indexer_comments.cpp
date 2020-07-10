/*  $Id: apply_indexer_comments.cpp 40461 2018-02-20 18:48:16Z filippov $
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

#include <objmgr/scope.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/packages/pkg_sequence_edit/ok_cancel_panel.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/packages/pkg_sequence_edit/apply_indexer_comments.hpp>

#include <util/xregexp/regexp.hpp>

#include <wx/button.h>
#include <wx/statbox.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE



IMPLEMENT_DYNAMIC_CLASS( CApplyIndexerComments, CBulkCmdDlg )


BEGIN_EVENT_TABLE( CApplyIndexerComments, CBulkCmdDlg )


END_EVENT_TABLE()

CApplyIndexerComments::CApplyIndexerComments()
{
    Init();
}

CApplyIndexerComments::CApplyIndexerComments( wxWindow* parent, IWorkbench *wb, 
                                        wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : CBulkCmdDlg(wb)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


bool CApplyIndexerComments::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CBulkCmdDlg::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();

    return true;
}


CApplyIndexerComments::~CApplyIndexerComments()
{
}


/*!
 * Member initialisation
 */

void CApplyIndexerComments::Init()
{
   
}




void CApplyIndexerComments::CreateControls()
{    
    CApplyIndexerComments* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);


    wxArrayString comment;
    comment.Add(wxT("Unverified Features"));
    comment.Add(wxT("Unverified Organism"));
    comment.Add(wxT("Unverified Features and Organism"));
    comment.Add(wxT("Unverified Misassembled"));
    comment.Add(wxT("Unverified Contamination"));
    comment.Add(wxT("Sequences were screened for chimeras by the submitter using "));
    comment.Add(wxT("PGAAP modified"));
    comment.Add(wxT("GeneBank staff are still waiting for submitters to provide appropriate coding region information"));
    comment.Add(wxT("GeneBank staff are still waiting for submitters to provide appropriate feature information"));
    comment.Add(wxT("Free Text"));
    comment.Add(wxT("FileTrackURL"));

    m_Choice = new wxChoice( itemDialog1, wxID_ANY, wxDefaultPosition, wxDefaultSize,comment,0);
    itemBoxSizer3->Add(m_Choice, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_Choice->Bind(wxEVT_CHOICE, &CApplyIndexerComments::OnSelect, this);
    m_Choice->SetSelection(0);

    m_TextCtrl = new wxTextCtrl( itemDialog1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(500, -1), 0 );
    itemBoxSizer2->Add(m_TextCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    m_TextCtrl->Disable();
  
    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer5->Add(itemBoxSizer6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer15Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Optional Seq-id Constraint"));
    wxStaticBoxSizer* itemStaticBoxSizer15 = new wxStaticBoxSizer(itemStaticBoxSizer15Static, wxVERTICAL);
    itemBoxSizer6->Add(itemStaticBoxSizer15, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    m_StringConstraintPanel = new CStringConstraintPanel(itemStaticBoxSizer15->GetStaticBox(), false, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    itemStaticBoxSizer15->Add(m_StringConstraintPanel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxFIXED_MINSIZE, 0);
    m_StringConstraintPanel->SetStringSelection(_("Is one of"));

    wxStaticBox* itemStaticBoxSizer17Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Optional Location Constraint"));
    wxStaticBoxSizer* itemStaticBoxSizer17 = new wxStaticBoxSizer(itemStaticBoxSizer17Static, wxVERTICAL);
    itemBoxSizer6->Add(itemStaticBoxSizer17, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString types;
    types.Add(wxT("Nucleotide and protein sequences"));
    types.Add(wxT("Nucleotide sequences only"));
    types.Add(wxT("Protein sequences only"));

    m_ChoiceType = new wxChoice(itemDialog1,wxID_ANY, wxDefaultPosition, wxDefaultSize, types,0,wxDefaultValidator,"Type");
    m_ChoiceType->SetSelection(1);
    itemStaticBoxSizer17->Add(m_ChoiceType, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    COkCancelPanel *OkCancel = new COkCancelPanel( itemDialog1, wxID_ANY, wxDefaultPosition, wxSize(100, 100), 0 );
    itemBoxSizer2->Add(OkCancel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
}

void CApplyIndexerComments::OnSelect(wxCommandEvent& event)
{
    int sel = m_Choice->GetSelection();
    if (sel == wxNOT_FOUND)
    {
        m_TextCtrl->Disable();
        return;
    }
    wxString str = m_Choice->GetString(sel);
    if (str == wxT("Sequences were screened for chimeras by the submitter using ") ||
        str == wxT("Free Text"))
    {
        m_TextCtrl->Enable();
        m_TextCtrl->SetValue(wxEmptyString);
    }
    else if (str == wxT("FileTrackURL"))
    {
        m_TextCtrl->Enable();
        m_TextCtrl->SetValue(_("https://submit.ncbi.nlm.nih.gov/ft/byid/"));
    }
    else
        m_TextCtrl->Disable();
}
/*!
 * Should we show tooltips?
 */

bool CApplyIndexerComments::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CApplyIndexerComments::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon CApplyIndexerComments::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
}

CRef<CCmdComposite> CApplyIndexerComments::GetCommand()
{

    CRef<CCmdComposite> composite(new CCmdComposite("Apply Indexer Comments")); 
    ApplyToCSeq_entry (m_TopSeqEntry, composite);   
    return composite;  
}

string CApplyIndexerComments::GetTextComment(const string &str)
{
    string comment(str);
    
    if (str == "Sequences were screened for chimeras by the submitter using ")
    {
        comment = str + m_TextCtrl->GetValue().ToStdString();
    }
    else if ( str == "Free Text")
    {
        comment =  m_TextCtrl->GetValue().ToStdString();
    }
    else if (str == "PGAAP modified")
    {
        comment = "Annotation modified by submitter";
    }

    return comment;
}

CRef<CUser_object> CApplyIndexerComments::GetUserObjectComment(const string &str)
{
    CRef<CUser_object> user(new CUser_object());
    user->SetObjectType(CUser_object::eObjectType_Unverified);

    if (str == "Unverified Organism" || str == "Unverified Features and Organism")
    {
        user->AddUnverifiedOrganism();
    }
    
    if (str == "Unverified Features" || str == "Unverified Features and Organism")
    {
        user->AddUnverifiedFeature();
    }
    if (str == "Unverified Misassembled")
    {
        user->AddUnverifiedMisassembled();
    }
    if (str == "Unverified Contamination")
    {
        user->AddUnverifiedContaminant();
    }
    if (str == "FileTrackURL")
    {
        user->SetObjectType(CUser_object::eObjectType_FileTrack);
        string url = m_TextCtrl->GetValue().ToStdString();
        user->SetFileTrackURL(url);
    }
    return user;
}

CRef<objects::CSeqdesc> CApplyIndexerComments::CreateNewComment()
{
    CRef<CSeqdesc> null_desc;
    CRef<objects::CSeqdesc> new_desc( new objects::CSeqdesc );
    int sel = m_Choice->GetSelection();
    if (sel == wxNOT_FOUND)
    {
        return null_desc;
    }
    string str = m_Choice->GetString(sel).ToStdString();

    if (str == "Unverified Features" || str == "Unverified Organism" || str == "Unverified Features and Organism" || str == "Unverified Misassembled"
	|| str == "Unverified Contamination" || str == "FileTrackURL")
    {
        CRef<CUser_object> user = GetUserObjectComment(str);
        new_desc->SetUser(*user);
    }
    else
    {
        string new_comment = GetTextComment(str);
        new_desc->SetComment(new_comment);
    }

    return new_desc;
}

void CApplyIndexerComments::ApplyToCSeq_entry (objects::CSeq_entry_Handle tse, CCmdComposite* composite)  
{
    CRef<objects::edit::CStringConstraint> constraint(m_StringConstraintPanel->GetStringConstraint());

    objects::CBioseq_CI b_iter(tse);
    for ( ; b_iter ; ++b_iter ) 
    {
	bool go = true;
	if (go && constraint)
	{
	    go = false;
	    for (auto handle : b_iter->GetId())
	    {
		string str;
		handle.GetSeqId()->GetLabel(&str,CSeq_id::eContent);
		if (!str.empty() && constraint->DoesTextMatch(str))    
		{                 
		    go = true;
		    break;
		}   
		str.clear();
		int version;
		handle.GetSeqId()->GetLabel(&str, &version, CSeq_id::eContent);
		if (!str.empty() && constraint->DoesTextMatch(str))    
		{                 
		    go = true;
		    break;
		}   
	    }   
	}

         if (go && m_ChoiceType->GetSelection()>0)
         {
             go = false;
             int n = m_ChoiceType->GetSelection();
             wxString type = m_ChoiceType->GetString(n);
             if (type == wxT("Nucleotide sequences only") && b_iter->IsNa()) go = true;
             if (type == wxT("Protein sequences only") &&  b_iter->IsAa()) go = true;
         }
         if (go)
         {          
             CRef<objects::CSeqdesc> new_desc = CreateNewComment();
             if (new_desc)
                 composite->AddCommand( *CRef<CCmdCreateDesc>(new CCmdCreateDesc(b_iter->GetParentEntry(), *new_desc)) );
         }
        
    }

}


END_NCBI_SCOPE
