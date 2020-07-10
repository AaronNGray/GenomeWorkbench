/*  $Id: add_deflinedlg.cpp 41933 2018-11-15 16:32:34Z asztalos $
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
 * Authors:  Andrea Asztalos
 */


#include <ncbi_pch.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/scope.hpp>

#include <objtools/edit/autodef_with_tax.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <gui/objutils/cmd_del_desc.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/packages/pkg_sequence_edit/seqtable_util.hpp>
#include <gui/packages/pkg_sequence_edit/add_deflinedlg.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <wx/msgdlg.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


IMPLEMENT_DYNAMIC_CLASS( CAddDefLineDlg, CBulkCmdDlg )


BEGIN_EVENT_TABLE( CAddDefLineDlg, CBulkCmdDlg )

END_EVENT_TABLE()



CAddDefLineDlg::CAddDefLineDlg()
{
    Init();
}

CAddDefLineDlg::CAddDefLineDlg( wxWindow* parent, IWorkbench* wb, wxWindowID id, 
                            const wxString& caption, const wxPoint& pos, 
                            const wxSize& size, long style ) 
: CBulkCmdDlg(wb)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


bool CAddDefLineDlg::Create( wxWindow* parent, wxWindowID id, 
                        const wxString& caption, const wxPoint& pos, 
                        const wxSize& size, long style )
{
////@begin CAddDefLineDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CBulkCmdDlg::Create(parent, id, caption, pos, size, style);

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CAddDefLineDlg creation
    return true;
}

CAddDefLineDlg::~CAddDefLineDlg()
{

}


/*!
 * Member initialisation
 */

void CAddDefLineDlg::Init()
{
    m_Title = NULL;
    m_OkCancel = NULL;
}


void CAddDefLineDlg::CreateControls()
{    
////@begin CAddDefLineDlg content construction
    CAddDefLineDlg* itemCBulkCmdDlg1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCBulkCmdDlg1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText(itemCBulkCmdDlg1, wxID_STATIC, _("Title"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Title = new wxTextCtrl(itemCBulkCmdDlg1, ID_ADDDEFLINE_TEXTCTRL, wxEmptyString, wxDefaultPosition, wxSize(110, 55), wxTE_MULTILINE);
    itemBoxSizer2->Add(m_Title, 0, wxGROW|wxALL, 5);

    m_OkCancel = new COkCancelPanel(itemCBulkCmdDlg1, ID_ADDDEFLINE_OKCANCEL, wxDefaultPosition, wxSize(100, 100), 0);
    itemBoxSizer2->Add(m_OkCancel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CAddDefLineDlg content construction
}


/*!
 * Should we show tooltips?
 */

bool CAddDefLineDlg::ShowToolTips()
{
    return true;
}


wxBitmap CAddDefLineDlg::GetBitmapResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullBitmap;
}

wxIcon CAddDefLineDlg::GetIconResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullIcon;
}

CRef<CCmdComposite> CAddDefLineDlg::GetCommand()
{
    string new_title = ToStdString(m_Title->GetValue());
    NStr::TruncateSpacesInPlace(new_title);
    NStr::ReplaceInPlace(new_title, "\n", " "); 
    
    if (NStr::IsBlank(new_title)){
        m_ErrorMessage = "No text has been entered.";
        return CRef<CCmdComposite>(NULL);
    }
    else {
        string msg("You are manually editing a title. If you continue, the title will not be automatically\n"
            "updated if the taxonomy information changes. Do you want to continue?");
        if (wxNO == wxMessageBox(ToWxString(msg), wxT("Confirm"), wxYES_NO | wxICON_QUESTION)) {
            return CRef<CCmdComposite>(NULL);
        }
    }
    
    int num_conflicts = CountDefLineConflicts(m_TopSeqEntry, new_title); 
    edit::EExistingText existing_text = x_GetExistingTextHandling (num_conflicts);
    if (existing_text == edit::eExistingText_cancel || existing_text == edit::eExistingText_leave_old){
        return CRef<CCmdComposite>(NULL);
    }
    
    CRef<CCmdComposite> cmd(new CCmdComposite("Add definition line"));
    UpdateAllDefinitionLine(m_TopSeqEntry, cmd, new_title, existing_text);

    return (cmd) ? cmd : CRef<CCmdComposite>(NULL);
}

string CAddDefLineDlg::GetErrorMessage()
{
    return m_ErrorMessage;
}


int CountDefLineConflicts(CSeq_entry_Handle entry, const string& title)
{
    int count=0;
    CBioseq_CI b_iter(entry, CSeq_inst::eMol_na); 
    for ( ; b_iter ; ++b_iter){
        CSeqdesc_CI seqdesc(*b_iter, CSeqdesc::e_Title, 1);
        while (seqdesc){
            if (!NStr::IsBlank(seqdesc->GetTitle()) && !NStr::Equal(seqdesc->GetTitle(), title))
                count++;
            ++seqdesc;
        }
    }
    
    CSeq_entry_CI set_ci(entry, CSeq_entry_CI::fRecursive | CSeq_entry_CI::fIncludeGivenEntry, CSeq_entry::e_Set);
    for ( ; set_ci ; ++set_ci){
        if (set_ci->GetSet().IsSetClass() && CBioseq_set::NeedsDocsumTitle(set_ci->GetSet().GetClass())){
            CSeqdesc_CI seqdesc(*set_ci, CSeqdesc::e_Title, 1);
            while (seqdesc){
                if (!NStr::IsBlank(seqdesc->GetTitle()) && !NStr::Equal(seqdesc->GetTitle(), title))
                    count++;
                ++seqdesc;
            }
        }
    }
    
    return count;
}

bool UpdateAllDefinitionLine(CSeq_entry_Handle entry, CCmdComposite* composite, const string& title, edit::EExistingText existing_text)
{ 
    bool modified = false;
    CSeq_entry_CI entry_ci(entry, CSeq_entry_CI::fRecursive | CSeq_entry_CI::fIncludeGivenEntry);
    for ( ; entry_ci ; ++entry_ci){
        if (entry_ci->IsSeq() && entry_ci->GetSeq().IsNa()){
            UpdateOneDefLine(entry_ci, composite, title, existing_text, modified);
        }else if (entry_ci->IsSet() && entry_ci->GetSet().IsSetClass() && CBioseq_set::NeedsDocsumTitle(entry_ci->GetSet().GetClass())){
            UpdateOneDefLine(entry_ci, composite, title, existing_text, modified);
        }
    }
    return modified;
}

void UpdateOneDefLine(const CSeq_entry_CI& entry_ci, CCmdComposite* composite, const string& title, edit::EExistingText existing_text, bool& modified)
{
    CSeqdesc_CI seqdesc(*entry_ci, CSeqdesc::e_Title, 1);
    if (seqdesc){
        string new_title = seqdesc->GetTitle();
        if (AddValueToString(new_title, title, existing_text)){
            CRef<CSeqdesc> new_desc(new CSeqdesc());
            new_desc->SetTitle(new_title);
            CRef<CCmdChangeSeqdesc> cmd(new CCmdChangeSeqdesc(seqdesc.GetSeq_entry_Handle(), *seqdesc, *new_desc));
            composite->AddCommand(*cmd);
            modified = true;
        }
        ++seqdesc;
        while (seqdesc){ //multiple titles will be deleted
            composite->AddCommand(*CRef<CCmdDelDesc>(new CCmdDelDesc(*entry_ci, *seqdesc)) );
            modified = true;
            ++seqdesc;
        }
    }else{ // make a new title
        CRef<CSeqdesc> new_desc(new CSeqdesc());
        new_desc->SetTitle(title);
        composite->AddCommand(*CRef<CCmdCreateDesc>(new CCmdCreateDesc(*entry_ci,*new_desc)));
        modified = true;
    }

    if (modified) {
        for (CSeqdesc_CI desc_it(*entry_ci, CSeqdesc::e_User); desc_it; ++desc_it) {
            if (desc_it->GetUser().GetObjectType() == CUser_object::eObjectType_AutodefOptions) {
                CRef<CCmdDelDesc> cmddel(new CCmdDelDesc(*entry_ci, *desc_it));
                composite->AddCommand(*cmddel);
            }
        }
    }
    
}

void RemoveTitlesSeqEntry(CSeq_entry_Handle seh, const CSeq_entry& entry, CCmdComposite* composite)
{
    
    FOR_EACH_SEQDESC_ON_SEQENTRY (seqdesc, entry)
        if ((*seqdesc)->IsTitle()){
            CRef<CCmdDelDesc> cmddel(new CCmdDelDesc(seh.GetScope().GetSeq_entryHandle(entry), **seqdesc));
            composite->AddCommand(*cmddel);
        }
    
    if (entry.IsSet()){
        FOR_EACH_SEQENTRY_ON_SEQSET (it, entry.GetSet())
            RemoveTitlesSeqEntry(seh, **it, composite);
    }
}

END_NCBI_SCOPE




