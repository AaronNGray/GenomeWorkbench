/* $Id: find_asn1_dlg.cpp 42186 2019-01-09 19:34:50Z asztalos $
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
#include <util/xregexp/regexp.hpp>
#include <objects/seqfeat/seqfeat_macros.hpp>
#include <objects/general/Dbtag.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/submit/Seq_submit.hpp>
#include <objects/submit/Submit_block.hpp>
#include <objects/submit/Contact_info.hpp>
#include <objects/seqblock/GB_block.hpp>
#include <objects/seq/Pubdesc.hpp>
#include <objects/pub/Pub_equiv.hpp>
#include <objects/pub/Pub.hpp>
#include <objects/biblio/Cit_gen.hpp>
#include <objects/biblio/Cit_sub.hpp>
#include <objects/biblio/Cit_art.hpp>
#include <objects/biblio/Cit_book.hpp>
#include <objects/biblio/Cit_jour.hpp>
#include <objects/biblio/Cit_let.hpp>
#include <objects/biblio/Cit_proc.hpp>
#include <objects/biblio/Cit_pat.hpp>
#include <objects/biblio/Auth_list.hpp>
#include <objects/biblio/Affil.hpp>
#include <objects/biblio/Author.hpp>
#include <objects/biblio/Imprint.hpp>
#include <objects/biblio/Meeting.hpp>
#include <objects/biblio/Id_pat.hpp>
#include <objects/biblio/CitRetract.hpp>
#include <objects/general/Person_id.hpp>
#include <objects/general/Name_std.hpp>
#include <objects/medline/Medline_entry.hpp>
#include <objects/medline/Medline_mesh.hpp>
#include <objects/medline/Medline_si.hpp>
#include <objects/medline/Medline_field.hpp>
#include <objects/medline/Medline_rn.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/seq_entry_ci.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/cmd_change_seq_entry.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/packages/pkg_sequence_edit/find_asn1_dlg.hpp>

#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/msgdlg.h> 
#include <wx/display.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

IMPLEMENT_DYNAMIC_CLASS( CFindASN1Dlg, wxDialog )



BEGIN_EVENT_TABLE( CFindASN1Dlg, wxDialog )

////@begin CFindASN1Dlg event table entries
EVT_BUTTON( ID_COPY_BUTTON, CFindASN1Dlg::OnCopyButton )
EVT_BUTTON( ID_CLEAR_BUTTON, CFindASN1Dlg::OnClearButton )
EVT_TEXT( ID_FIND_TEXT, CFindASN1Dlg::OnFindText )
EVT_BUTTON( ID_REPLACE_BUTTON, CFindASN1Dlg::OnReplaceButton )
EVT_BUTTON( ID_FIND_ASN1_CANCEL_BUTTON, CFindASN1Dlg::OnCancelButton )
EVT_CLOSE(CFindASN1Dlg::OnClose)
////@end CFindASN1Dlg event table entries

END_EVENT_TABLE()


/*!
 * CFindASN1Dlg constructors
 */

CFindASN1Dlg::CFindASN1Dlg()
{
    Init();
}

CFindASN1Dlg::CFindASN1Dlg( wxWindow* parent, CSeq_entry_Handle seh, ICommandProccessor* cmdProcessor, CConstRef<objects::CSeq_submit> submit,
				wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_TopSeqEntry(seh), m_CmdProcessor(cmdProcessor), m_SeqSubmit(submit)
{
    Init();
    Create(parent, id, caption, pos, size, style);
    SetRegistryPath("Dialogs.Edit.FindASN1Dialog");
    LoadSettings();
}

static const char* kAutoCopy = "AutoCopy";
static const char* kFrameWidth = "Frame Width";
static const char* kFrameHeight = "Frame Height";
static const char* kFramePosX = "Frame Position X";
static const char* kFramePosY = "Frame Position Y";

void CFindASN1Dlg::SetRegistryPath(const string& reg_path)
{
    m_RegPath = reg_path;
}

void CFindASN1Dlg::SaveSettings() const
{
    if (m_RegPath.empty())
        return;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);
    if (m_AutoCopy) 
        view.Set(kAutoCopy, m_AutoCopy->GetValue());

    view.Set(kFrameWidth,GetScreenRect().GetWidth());
    view.Set(kFrameHeight,GetScreenRect().GetHeight());
    view.Set(kFramePosX,GetScreenPosition().x);
    view.Set(kFramePosY,GetScreenPosition().y);
}


void CFindASN1Dlg::LoadSettings()
{
    if (m_RegPath.empty())
        return;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryReadView view = gui_reg.GetReadView(m_RegPath);
    if (m_AutoCopy) 
        m_AutoCopy->SetValue(view.GetBool(kAutoCopy));

    int width = view.GetInt(kFrameWidth, -1);
    int height = view.GetInt(kFrameHeight, -1);
    if (width >= 0  && height >= 0)
        SetSize(wxSize(width,height));

    int pos_x = view.GetInt(kFramePosX, -1);
    int pos_y = view.GetInt(kFramePosY, -1);

   if (pos_x >= 0  && pos_y >= 0)
   {
       int max_x = 0;
       for (size_t i = 0; i < wxDisplay::GetCount(); i++) // also see gui/widgets/wx/wx_utils.cpp:CorrectWindowRect() for alternative window position validation
       {
        wxDisplay display(i);
        max_x += display.GetGeometry().GetWidth();
       }
       if (pos_x + width > max_x) pos_x = wxGetDisplaySize().GetWidth()-width-5;
       if (pos_y + height > wxGetDisplaySize().GetHeight()) pos_y = wxGetDisplaySize().GetHeight()-height-5;
       
       SetPosition(wxPoint(pos_x,pos_y));
   }
}

/*!
 * CFindASN1Dlg creator
 */

bool CFindASN1Dlg::Create( wxWindow* parent, 
			     wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CFindASN1Dlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CFindASN1Dlg creation

    return true;
}


/*!
 * CFindASN1Dlg destructor
 */

CFindASN1Dlg::~CFindASN1Dlg()
{
////@begin CFindASN1Dlg destruction
    SaveSettings();
////@end CFindASN1Dlg destruction
}


/*!
 * Member initialisation
 */

void CFindASN1Dlg::Init()
{
////@begin CFindASN1Dlg member initialisation
    m_Find  = NULL;
    m_Replace = NULL;
    m_AutoCopy = NULL;
    m_EntireWord = NULL;
    m_CaseSensitive = NULL;
////@end CFindASN1Dlg member initialisation
}


/*!
 * Control creation for CFindASN1Dlg
 */

void CFindASN1Dlg::CreateControls()
{    
////@begin CFindASN1Dlg content construction
    CFindASN1Dlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(itemBoxSizer4, 0, wxGROW|wxALL, 5);

    wxStaticText* text1 = new wxStaticText(itemDialog1, wxID_STATIC, _("Find"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer4->Add(text1, 1, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* text2 = new wxStaticText(itemDialog1, wxID_STATIC, _("Replace"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer4->Add(text2, 1, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(itemBoxSizer5, 0, wxGROW|wxALL, 5);

    m_Find = new wxTextCtrl( itemDialog1, ID_FIND_TEXT, wxEmptyString, wxDefaultPosition, wxSize(400,-1), 0 );
    itemBoxSizer5->Add(m_Find, 1, wxALIGN_LEFT|wxALL, 5);

    m_Replace = new wxTextCtrl( itemDialog1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(400,-1), 0 );
    itemBoxSizer5->Add(m_Replace, 1, wxALIGN_LEFT|wxALL, 5);

    wxButton* itemButton1 = new wxButton( itemDialog1, ID_COPY_BUTTON, _("Copy"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    itemBoxSizer3->Add(itemButton1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AutoCopy = new wxCheckBox( itemDialog1, ID_AUTOCOPY_CHECKBOX, _("Auto-copy"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_AutoCopy, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer6, 0, wxALIGN_LEFT|wxALL, 5);

    m_CaseSensitive = new wxCheckBox( itemDialog1, wxID_ANY, _("Case Sensitive"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(m_CaseSensitive, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_EntireWord = new wxCheckBox( itemDialog1, wxID_ANY, _("Entire Word"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(m_EntireWord, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer7, 0, wxEXPAND|wxALL, 5);

    wxButton* itemButton5 = new wxButton( itemDialog1, ID_REPLACE_BUTTON, _("Replace All"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemButton5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton6 = new wxButton( itemDialog1, ID_CLEAR_BUTTON, _("Clear"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemButton6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton7 = new wxButton( itemDialog1, ID_FIND_ASN1_CANCEL_BUTTON, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemButton7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CFindASN1Dlg content construction
    
}


/*!
 * Should we show tooltips?
 */

bool CFindASN1Dlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CFindASN1Dlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CFindASN1Dlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CFindASN1Dlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CFindASN1Dlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CFindASN1Dlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CFindASN1Dlg icon retrieval
}

void CFindASN1Dlg::OnCopyButton(wxCommandEvent& event )
{
    m_Replace->SetValue(m_Find->GetValue());
}

void CFindASN1Dlg::OnClearButton(wxCommandEvent& event )
{
    m_Find->SetValue(wxEmptyString);
    m_Replace->SetValue(wxEmptyString);
}

void CFindASN1Dlg::OnFindText(wxCommandEvent& event )
{
    if (m_AutoCopy->GetValue())
        m_Replace->SetValue(m_Find->GetValue());
}

void CFindASN1Dlg::OnReplaceButton(wxCommandEvent& event )
{
    string find = m_Find->GetValue().ToStdString();
    string replace = m_Replace->GetValue().ToStdString();

    NStr::ReplaceInPlace(find, "\r", "");
    NStr::ReplaceInPlace(replace, "\r", "");
    NStr::ReplaceInPlace(find, "\n", "");
    NStr::ReplaceInPlace(replace, "\n", "");

    if (NStr::Find(find,"\"") != NPOS || NStr::Find(replace,"\"") != NPOS)
    {
        wxMessageBox(wxT("Quotes are not allowed in find and replace strings"), wxT("Error"), wxOK | wxICON_ERROR);
        return;
    }
    if (m_EntireWord->GetValue())
    {
        find = "\\b" + CRegexp::Escape(find) + "\\b";
    }
    else
    {
        find = CRegexp::Escape(find);
    }

    CRegexp::TCompile options = CRegexp::fCompile_ignore_case;
    if (m_CaseSensitive->GetValue())
    {
        options = CRegexp::fCompile_default;
    }
    try
    {
        CRef<CCmdComposite> cmd(new CCmdComposite("Find and Replace ASN.1 action"));
        for (CFeat_CI feat_ci(m_TopSeqEntry); feat_ci; ++feat_ci)
        {
            CSeq_feat_Handle fh = feat_ci->GetSeq_feat_Handle();
            CRef<CSeq_feat> new_feat(new CSeq_feat);
            new_feat->Assign(*fh.GetOriginalSeq_feat());
            bool modified = ReplaceInFeature(new_feat, find, replace, options);
            if (modified)
            {
                CIRef<IEditCommand> chgFeat(new CCmdChangeSeq_feat(fh, *new_feat));
                if (chgFeat)
                    cmd->AddCommand(*chgFeat);
            }
        }   
        for ( CSeq_entry_CI entry_it(m_TopSeqEntry, CSeq_entry_CI::fRecursive|CSeq_entry_CI::fIncludeGivenEntry); entry_it; ++entry_it ) 
        {
            for ( CSeqdesc_CI desc_iter(*entry_it, CSeqdesc::e_not_set, 1); desc_iter; ++desc_iter) 
            {
                if (desc_iter->IsMolinfo())
                    continue;

                CRef<CSeqdesc> new_desc(new CSeqdesc);
                new_desc->Assign(*desc_iter);
                bool modified = ReplaceInDesc(new_desc, find, replace, options);

                if (modified)
                {
                    CRef<CCmdChangeSeqdesc> ecmd(new CCmdChangeSeqdesc(desc_iter.GetSeq_entry_Handle(), *desc_iter, *new_desc));
                    cmd->AddCommand (*ecmd);
                }
            }
        }
        if (m_SeqSubmit && m_SeqSubmit->IsSetSub())
        {
            const CSubmit_block &submit = m_SeqSubmit->GetSub();
            CRef<CSubmit_block> new_submit(new CSubmit_block);
            new_submit->Assign(submit);
            bool modified = ReplaceInSubmit(new_submit, find, replace, options);
            if (modified)
            {
                CChangeSubmitBlockCommand* cmd = new CChangeSubmitBlockCommand();
                const CObject* actual = dynamic_cast<const CObject*>(&submit);
                cmd->Add(const_cast<CObject*>(actual), CConstRef<CObject>(new_submit));
            }
        }
        m_CmdProcessor->Execute(cmd);
    }
    catch(const exception&)
    {
        wxMessageBox(wxT("Unable to perform requested substitution"), wxT("Error"), wxOK | wxICON_ERROR);
    }
}

void CFindASN1Dlg::ReplaceLinebreaks(string &input)
{
    string result;
    result.reserve(input.size());
    size_t quotes = 0;
    for (size_t i = 0; i < input.size(); i++)
    {
        char c = input[i];
        if (c == '"')
            quotes++;
        if ((c == '\r' || c == '\n') && quotes % 2 == 1)
        {
            continue;
        }
        result.push_back(c);
    }
    swap(result, input);
}


void CFindASN1Dlg::OnClose(wxCloseEvent& event)
{
    SaveSettings();
    event.Skip();
}

void CFindASN1Dlg::OnCancelButton(wxCommandEvent& event )
{
    Close();
}

bool CFindASN1Dlg::ReplaceInFeature(CRef<CSeq_feat> new_feat, const string &find, const string &replace, CRegexp::TCompile options)
{
    bool modified = false;
    if (new_feat->IsSetComment())
    {
        const string &old_value = new_feat->GetComment();
        string new_value =  ReplaceValue(old_value, find, replace, options);
        modified |= new_value != old_value;
        new_feat->SetComment(new_value);
    }
    if (new_feat->IsSetTitle())
    {
        const string &old_value = new_feat->GetTitle();
        string new_value =  ReplaceValue(old_value, find, replace, options);
        modified |= new_value != old_value;
        new_feat->SetTitle(new_value);
    }
    if (new_feat->IsSetExcept_text())
    {
        const string &old_value = new_feat->GetExcept_text();
        string new_value =  ReplaceValue(old_value, find, replace, options);
        modified |= new_value != old_value;
        new_feat->SetExcept_text(new_value);
    }
    EDIT_EACH_GBQUAL_ON_SEQFEAT(qual, *new_feat)
    {
        if ((*qual)->IsSetQual())
        {
            const string &old_value = (*qual)->GetQual();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            (*qual)->SetQual(new_value);
        }
        if ((*qual)->IsSetVal())
        {
            const string &old_value = (*qual)->GetVal();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            (*qual)->SetVal(new_value);
        }
    }
    EDIT_EACH_DBXREF_ON_SEQFEAT(dbtag, *new_feat)
    {
        modified |= ReplaceInDbxref(**dbtag, find, replace, options);       
    }
    if (new_feat->IsSetData())
    {
        if (new_feat->GetData().IsGene())
        {
            if (new_feat->GetData().GetGene().IsSetLocus())
            {
                const string &old_value = new_feat->GetData().GetGene().GetLocus();
                string new_value =  ReplaceValue(old_value, find, replace, options);
                modified |= new_value != old_value;
                new_feat->SetData().SetGene().SetLocus(new_value);
            }
            if (new_feat->GetData().GetGene().IsSetAllele())
            {
                const string &old_value = new_feat->GetData().GetGene().GetAllele();
                string new_value =  ReplaceValue(old_value, find, replace, options);
                modified |= new_value != old_value;
                new_feat->SetData().SetGene().SetAllele(new_value);
            }
            if (new_feat->GetData().GetGene().IsSetDesc())
            {
                const string &old_value = new_feat->GetData().GetGene().GetDesc();
                string new_value =  ReplaceValue(old_value, find, replace, options);
                modified |= new_value != old_value;
                new_feat->SetData().SetGene().SetDesc(new_value);
            }
            if (new_feat->GetData().GetGene().IsSetMaploc())
            {
                const string &old_value = new_feat->GetData().GetGene().GetMaploc();
                string new_value =  ReplaceValue(old_value, find, replace, options);
                modified |= new_value != old_value;
                new_feat->SetData().SetGene().SetMaploc(new_value);
            }
            if (new_feat->GetData().GetGene().IsSetLocus_tag())
            {
                const string &old_value = new_feat->GetData().GetGene().GetLocus_tag();
                string new_value =  ReplaceValue(old_value, find, replace, options);
                modified |= new_value != old_value;
                new_feat->SetData().SetGene().SetLocus_tag(new_value);
            }
            if (new_feat->GetData().GetGene().IsSetDb())
            {
                for (auto dbtag : new_feat->SetData().SetGene().SetDb())
                {
                    modified |= ReplaceInDbxref(*dbtag, find, replace, options);                       
                }
            }
            if (new_feat->GetData().GetGene().IsSetSyn())
            {
                for (auto &syn : new_feat->SetData().SetGene().SetSyn())
                {
                    const string &old_value = syn;
                    string new_value =  ReplaceValue(old_value, find, replace, options);
                    modified |= new_value != old_value;
                    syn = new_value;
                }
            }
        }
        if (new_feat->GetData().IsProt())
        {
            if (new_feat->GetData().GetProt().IsSetDesc())
            {
                const string &old_value = new_feat->GetData().GetProt().GetDesc();
                string new_value =  ReplaceValue(old_value, find, replace, options);
                modified |= new_value != old_value;
                new_feat->SetData().SetProt().SetDesc(new_value);
            }
            if (new_feat->GetData().GetProt().IsSetName())
            {
                for (auto &str : new_feat->SetData().SetProt().SetName())
                {
                    const string &old_value = str;
                    string new_value =  ReplaceValue(old_value, find, replace, options);
                    modified |= new_value != old_value;
                    str = new_value;
                }
            }
            if (new_feat->GetData().GetProt().IsSetEc())
            {
                for (auto &str : new_feat->SetData().SetProt().SetEc())
                {
                    const string &old_value = str;
                    string new_value =  ReplaceValue(old_value, find, replace, options);
                    modified |= new_value != old_value;
                    str = new_value;
                }
            }
            if (new_feat->GetData().GetProt().IsSetActivity())
            {
                for (auto &str : new_feat->SetData().SetProt().SetActivity())
                {
                    const string &old_value = str;
                    string new_value =  ReplaceValue(old_value, find, replace, options);
                    modified |= new_value != old_value;
                    str = new_value;
                }
            }
            if (new_feat->GetData().GetProt().IsSetDb())
            {
                for (auto dbtag : new_feat->SetData().SetProt().SetDb())
                {
                    modified |= ReplaceInDbxref(*dbtag, find, replace, options);                       
                }
            }
        }
        if (new_feat->GetData().IsRna() && new_feat->GetData().GetRna().IsSetExt())
        {
            if (new_feat->GetData().GetRna().GetExt().IsName())
            {
                const string &old_value = new_feat->GetData().GetRna().GetExt().GetName();
                string new_value =  ReplaceValue(old_value, find, replace, options);
                modified |= new_value != old_value;
                new_feat->SetData().SetRna().SetExt().SetName(new_value);
            }
            if (new_feat->GetData().GetRna().GetExt().IsGen())
            {
                if (new_feat->GetData().GetRna().GetExt().GetGen().IsSetClass())
                {
                    const string &old_value = new_feat->GetData().GetRna().GetExt().GetGen().GetClass();
                    string new_value =  ReplaceValue(old_value, find, replace, options);
                    modified |= new_value != old_value;
                    new_feat->SetData().SetRna().SetExt().SetGen().SetClass(new_value);
                }
                if (new_feat->GetData().GetRna().GetExt().GetGen().IsSetProduct())
                {
                    const string &old_value = new_feat->GetData().GetRna().GetExt().GetGen().GetProduct();
                    string new_value =  ReplaceValue(old_value, find, replace, options);
                    modified |= new_value != old_value;
                    new_feat->SetData().SetRna().SetExt().SetGen().SetProduct(new_value);
                }
                if (new_feat->GetData().GetRna().GetExt().GetGen().IsSetQuals() && new_feat->GetData().GetRna().GetExt().GetGen().GetQuals().IsSet())
                {
                    for (auto qual : new_feat->SetData().SetRna().SetExt().SetGen().SetQuals().Set())
                    {
                        if (qual->IsSetVal())
                        {
                            const string &old_value = qual->GetVal();
                            string new_value =  ReplaceValue(old_value, find, replace, options);
                            modified |= new_value != old_value;
                            qual->SetVal(new_value);
                        }
                    }
                }
            }
        }
        if (new_feat->GetData().IsImp())
        {
            if (new_feat->GetData().GetImp().IsSetLoc())
            {
                const string &old_value = new_feat->GetData().GetImp().GetLoc();
                string new_value =  ReplaceValue(old_value, find, replace, options);
                modified |= new_value != old_value;
                new_feat->SetData().SetImp().SetLoc(new_value);
            }
            if (new_feat->GetData().GetImp().IsSetDescr())
            {
                const string &old_value = new_feat->GetData().GetImp().GetDescr();
                string new_value =  ReplaceValue(old_value, find, replace, options);
                modified |= new_value != old_value;
                new_feat->SetData().SetImp().SetDescr(new_value);
            }
        }
        if (new_feat->GetData().IsRegion())
        {
            const string &old_value = new_feat->GetData().GetRegion();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            new_feat->SetData().SetRegion(new_value);
        }
        if (new_feat->GetData().IsOrg())
        {
            modified |= ReplaceInOrg(new_feat->SetData().SetOrg(), find, replace, options);      
        }
        if (new_feat->GetData().IsBiosrc())
        {
            modified |= ReplaceInBiosource(new_feat->SetData().SetBiosrc(), find, replace, options);      
        }
        if (new_feat->GetData().IsPub())
        {
            modified |= ReplaceInPubdesc(new_feat->SetData().SetPub(), find, replace, options); 
        }
    }
    return modified;
}

string CFindASN1Dlg::ReplaceValue(const string &input, const string &find, const string &replace, CRegexp::TCompile options)
{
    CRegexpUtil replacer(input);
    replacer.Replace( find, replace, options, CRegexp::fMatch_default, 0);
    return replacer.GetResult();
}

bool CFindASN1Dlg::ReplaceInDesc(CRef<CSeqdesc> new_desc, const string &find, const string &replace, CRegexp::TCompile options)
{
 bool modified = false;
    if (new_desc->IsName())
    {
        const string &old_value = new_desc->GetName();
        string new_value =  ReplaceValue(old_value, find, replace, options);
        modified |= new_value != old_value;
        new_desc->SetName(new_value);
    }
    if (new_desc->IsTitle())
    {
        const string &old_value = new_desc->GetTitle();
        string new_value =  ReplaceValue(old_value, find, replace, options);
        modified |= new_value != old_value;
        new_desc->SetTitle(new_value);
    }
    if (new_desc->IsComment())
    {
        const string &old_value = new_desc->GetComment();
        string new_value =  ReplaceValue(old_value, find, replace, options);
        modified |= new_value != old_value;
        new_desc->SetComment(new_value);
    }
    if (new_desc->IsRegion())
    {
        const string &old_value = new_desc->GetRegion();
        string new_value =  ReplaceValue(old_value, find, replace, options);
        modified |= new_value != old_value;
        new_desc->SetRegion(new_value);
    }
    if (new_desc->IsOrg())
    {
        modified |= ReplaceInOrg(new_desc->SetOrg(), find, replace, options);      
    }
    if (new_desc->IsSource())
    {
        modified |= ReplaceInBiosource(new_desc->SetSource(), find, replace, options);      
    }
    if (new_desc->IsGenbank())
    {
        if (new_desc->GetGenbank().IsSetSource())
        {
            const string &old_value = new_desc->GetGenbank().GetSource();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            new_desc->SetGenbank().SetSource(new_value);
        }
        if (new_desc->GetGenbank().IsSetOrigin())
        {
            const string &old_value = new_desc->GetGenbank().GetOrigin();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            new_desc->SetGenbank().SetOrigin(new_value);
        }
        if (new_desc->GetGenbank().IsSetDate())
        {
            const string &old_value = new_desc->GetGenbank().GetDate();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            new_desc->SetGenbank().SetDate(new_value);
        }
        if (new_desc->GetGenbank().IsSetDiv())
        {
            const string &old_value = new_desc->GetGenbank().GetDiv();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            new_desc->SetGenbank().SetDiv(new_value);
        }
        if (new_desc->GetGenbank().IsSetTaxonomy())
        {
            const string &old_value = new_desc->GetGenbank().GetTaxonomy();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            new_desc->SetGenbank().SetTaxonomy(new_value);
        }
        if (new_desc->GetGenbank().IsSetExtra_accessions())
            for (auto &str : new_desc->SetGenbank().SetExtra_accessions())
            {
                const string &old_value = str;
                string new_value =  ReplaceValue(old_value, find, replace, options);
                modified |= new_value != old_value;
                str = new_value;
            }
        if (new_desc->GetGenbank().IsSetKeywords())
            for (auto &str : new_desc->SetGenbank().SetKeywords())
            {
                const string &old_value = str;
                string new_value =  ReplaceValue(old_value, find, replace, options);
                modified |= new_value != old_value;
                str = new_value;
            }
    }
    if (new_desc->IsPub())
    {
        modified |= ReplaceInPubdesc(new_desc->SetPub(), find, replace, options); 
    }
    return modified;
}

bool CFindASN1Dlg::ReplaceInSubmit(CRef<CSubmit_block> new_submit, const string &find, const string &replace, CRegexp::TCompile options)
{
 bool modified = false;
    if (new_submit->IsSetTool())
    {
        const string &old_value = new_submit->GetTool();
        string new_value =  ReplaceValue(old_value, find, replace, options);
        modified |= new_value != old_value;
        new_submit->SetTool(new_value);
    }
    if (new_submit->IsSetUser_tag())
    {
        const string &old_value = new_submit->GetUser_tag();
        string new_value =  ReplaceValue(old_value, find, replace, options);
        modified |= new_value != old_value;
        new_submit->SetUser_tag(new_value);
    }
    if (new_submit->IsSetComment())
    {
        const string &old_value = new_submit->GetComment();
        string new_value =  ReplaceValue(old_value, find, replace, options);
        modified |= new_value != old_value;
        new_submit->SetComment(new_value);
    }   
    if (new_submit->IsSetContact())
    {
        if (new_submit->GetContact().IsSetName())
        {
            const string &old_value = new_submit->GetContact().GetName();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            new_submit->SetContact().SetName(new_value);
        }   
        if (new_submit->GetContact().IsSetPhone())
        {
            const string &old_value = new_submit->GetContact().GetPhone();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            new_submit->SetContact().SetPhone(new_value);
        }   
        if (new_submit->GetContact().IsSetFax())
        {
            const string &old_value = new_submit->GetContact().GetFax();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            new_submit->SetContact().SetFax(new_value);
        }   
        if (new_submit->GetContact().IsSetEmail())
        {
            const string &old_value = new_submit->GetContact().GetEmail();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            new_submit->SetContact().SetEmail(new_value);
        }   
        if (new_submit->GetContact().IsSetTelex())
        {
            const string &old_value = new_submit->GetContact().GetTelex();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            new_submit->SetContact().SetTelex(new_value);
        }   
        if (new_submit->GetContact().IsSetLast_name())
        {
            const string &old_value = new_submit->GetContact().GetLast_name();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            new_submit->SetContact().SetLast_name(new_value);
        }   
        if (new_submit->GetContact().IsSetFirst_name())
        {
            const string &old_value = new_submit->GetContact().GetFirst_name();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            new_submit->SetContact().SetFirst_name(new_value);
        }   
        if (new_submit->GetContact().IsSetMiddle_initial())
        {
            const string &old_value = new_submit->GetContact().GetMiddle_initial();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            new_submit->SetContact().SetMiddle_initial(new_value);
        }   
        if (new_submit->GetContact().IsSetAddress())
        {
            for (auto &str : new_submit->SetContact().SetAddress())
            {
                const string &old_value = str;
                string new_value =  ReplaceValue(old_value, find, replace, options);
                modified |= new_value != old_value;
                str = new_value;
            }
        }
        if (new_submit->GetContact().IsSetOwner_id() && new_submit->GetContact().GetOwner_id().IsStr())
        {   
            const string &old_value = new_submit->GetContact().GetOwner_id().GetStr();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            new_submit->SetContact().SetOwner_id().SetStr(new_value);
        }
        if (new_submit->GetContact().IsSetContact())
        {
            modified |= ReplaceInAuthor(new_submit->SetContact().SetContact(), find, replace, options); 
        }
    }
    if (new_submit->IsSetCit())
    {
        if (new_submit->GetCit().IsSetDescr())
        {
            const string &old_value = new_submit->GetCit().GetDescr();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            new_submit->SetCit().SetDescr(new_value);
        }   
        if (new_submit->GetCit().IsSetAuthors())
        {
            modified |= ReplaceInAuthList(new_submit->SetCit().SetAuthors(), find, replace, options); 
        }
    }
    return modified;
}

bool CFindASN1Dlg::ReplaceInDbxref(CDbtag &dbtag, const string &find, const string &replace, CRegexp::TCompile options)
{
    bool modified = false;
    if (dbtag.IsSetDb())
        {
            const string &old_value = dbtag.GetDb();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            dbtag.SetDb(new_value);
        }
    if (dbtag.IsSetTag() && dbtag.GetTag().IsStr())
    {
        const string &old_value = dbtag.GetTag().GetStr();
        string new_value =  ReplaceValue(old_value, find, replace, options);
        modified |= new_value != old_value;
        dbtag.SetTag().SetStr(new_value);
    }
    return modified;
}

bool CFindASN1Dlg::ReplaceInOrg(COrg_ref &org, const string &find, const string &replace, CRegexp::TCompile options)
{
    bool modified = false;
    if (org.IsSetTaxname())
    {
        const string &old_value = org.GetTaxname();
        string new_value =  ReplaceValue(old_value, find, replace, options);
        modified |= new_value != old_value;
        if (old_value != new_value)
        {
            org.ResetCommon();
            if (org.IsSetDb())
            {
                EDIT_EACH_DBXREF_ON_ORGREF(dbtag, org)
                {
                    if ((*dbtag)->IsSetDb() && NStr::EqualNocase((*dbtag)->GetDb(), "taxon"))
                        ERASE_DBXREF_ON_ORGREF(dbtag, org);
                }
            }
        }
        org.SetTaxname(new_value);
    }
    if (org.IsSetCommon())
    {
        const string &old_value = org.GetCommon();
        string new_value =  ReplaceValue(old_value, find, replace, options);
        modified |= new_value != old_value;
        org.SetCommon(new_value);
    }
    if (org.IsSetMod())
    {
        for (auto &str : org.SetMod())
        {
            const string &old_value = str;
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            str = new_value;
        }
    }
    if (org.IsSetSyn())
    {
        for (auto &syn : org.SetSyn())
        {
            const string &old_value = syn;
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            syn = new_value;
        }
    }
    if (org.IsSetDb())
    {
        EDIT_EACH_DBXREF_ON_ORGREF(dbtag, org)
        {
            modified |= ReplaceInDbxref(**dbtag, find, replace, options);
        }
    }
    if (org.IsSetOrgname())
    {
        if (org.GetOrgname().IsSetAttrib())
        {
            const string &old_value = org.GetOrgname().GetAttrib();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            org.SetOrgname().SetAttrib(new_value);
        }
        if (org.GetOrgname().IsSetLineage())
        {
            const string &old_value = org.GetOrgname().GetLineage();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            org.SetOrgname().SetLineage(new_value);
        }
        if (org.GetOrgname().IsSetDiv())
        {
            const string &old_value = org.GetOrgname().GetDiv();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            org.SetOrgname().SetDiv(new_value);
        }
        if (org.GetOrgname().IsSetMod())
        {
            for (auto mod : org.SetOrgname().SetMod())
            {
                if (mod->IsSetSubname())
                {
                    const string &old_value = mod->GetSubname();
                    string new_value =  ReplaceValue(old_value, find, replace, options);
                    modified |= new_value != old_value;
                    mod->SetSubname(new_value);
                }
                if (mod->IsSetAttrib())
                {
                    const string &old_value = mod->GetAttrib();
                    string new_value =  ReplaceValue(old_value, find, replace, options);
                    modified |= new_value != old_value;
                    mod->SetAttrib(new_value);
                }
            }
        }
    }
    return modified;
}

bool CFindASN1Dlg::ReplaceInBiosource(CBioSource &biosource, const string &find, const string &replace, CRegexp::TCompile options)
{
    bool modified = false;
    if (biosource.IsSetSubtype())
    {
        for (auto subsource : biosource.SetSubtype())
        {
            if (subsource->IsSetAttrib())
            {
                const string &old_value = subsource->GetAttrib();
                string new_value =  ReplaceValue(old_value, find, replace, options);
                modified |= new_value != old_value;
                subsource->SetAttrib(new_value);
            }
            if (subsource->IsSetName() && subsource->IsSetSubtype() 
                && subsource->GetSubtype() != CSubSource::eSubtype_germline
                && subsource->GetSubtype() != CSubSource::eSubtype_rearranged
                && subsource->GetSubtype() != CSubSource::eSubtype_transgenic
                && subsource->GetSubtype() != CSubSource::eSubtype_environmental_sample
                && subsource->GetSubtype() != CSubSource::eSubtype_metagenomic)
            {
                const string &old_value = subsource->GetName();
                string new_value =  ReplaceValue(old_value, find, replace, options);
                modified |= new_value != old_value;
                subsource->SetName(new_value);
            }
        }
    }
    if (biosource.IsSetOrg())
    {
        modified |= ReplaceInOrg(biosource.SetOrg(), find, replace, options);  
    }
    
    return modified;
}

bool CFindASN1Dlg::ReplaceInPubdesc(CPubdesc &pubdesc, const string &find, const string &replace, CRegexp::TCompile options)
{
    bool modified = false;
    if (pubdesc.IsSetComment())
    {
        const string &old_value = pubdesc.GetComment();
        string new_value =  ReplaceValue(old_value, find, replace, options);
        modified |= new_value != old_value;
        pubdesc.SetComment(new_value);
    }
    if (pubdesc.IsSetPub() && pubdesc.GetPub().IsSet())
    {
        for (auto pub : pubdesc.SetPub().Set())
        {
            modified |= ReplaceInPub(pub, find, replace, options);  
        }
    }
    return modified;
}

bool CFindASN1Dlg::ReplaceInPub(CRef<CPub> pub, const string &find, const string &replace, CRegexp::TCompile options)
{
    bool modified = false;
    if (pub->IsGen())
    {
        if (pub->GetGen().IsSetCit())
        {
            const string &old_value = pub->GetGen().GetCit();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            pub->SetGen().SetCit(new_value);
        }
        if (pub->GetGen().IsSetVolume())
        {
            const string &old_value = pub->GetGen().GetVolume();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            pub->SetGen().SetVolume(new_value);
        }
        if (pub->GetGen().IsSetIssue())
        {
            const string &old_value = pub->GetGen().GetIssue();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            pub->SetGen().SetIssue(new_value);
        }
        if (pub->GetGen().IsSetPages())
        {
            const string &old_value = pub->GetGen().GetPages();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            pub->SetGen().SetPages(new_value);
        }
        if (pub->GetGen().IsSetTitle())
        {
            const string &old_value = pub->GetGen().GetTitle();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            pub->SetGen().SetTitle(new_value);
        }
        if (pub->GetGen().IsSetAuthors())
        {
            modified |= ReplaceInAuthList(pub->SetGen().SetAuthors(), find, replace, options);  
        }
        if (pub->GetGen().IsSetJournal() && pub->GetGen().GetJournal().IsSet())
        {
            for (auto journal : pub->SetGen().SetJournal().Set())
            {
                modified |= ReplaceInJournal(journal, find, replace, options);  
            }
        }
    }
    if (pub->IsSub())
    {
        if (pub->GetSub().IsSetAuthors())
        {
            modified |= ReplaceInAuthList(pub->SetSub().SetAuthors(), find, replace, options);  
        }
        if (pub->GetSub().IsSetDescr())
        {
            const string &old_value = pub->GetSub().GetDescr();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            pub->SetSub().SetDescr(new_value);
        }
    }
    if (pub->IsMedline())
    {
        if (pub->GetMedline().IsSetCit())
        {
            modified |= ReplaceInArticle(pub->SetMedline().SetCit(), find, replace, options);  
        }
        if (pub->GetMedline().IsSetAbstract())
        {
            const string &old_value = pub->GetMedline().GetAbstract();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            pub->SetMedline().SetAbstract(new_value);
        }
        if (pub->GetMedline().IsSetSubstance())
        {
            for (auto substance : pub->SetMedline().SetSubstance())
            {
                if (substance->IsSetCit())
                {
                    const string &old_value = substance->GetCit();
                    string new_value =  ReplaceValue(old_value, find, replace, options);
                    modified |= new_value != old_value;
                    substance->SetCit(new_value);
                }
                if (substance->IsSetName())
                {
                    const string &old_value = substance->GetName();
                    string new_value =  ReplaceValue(old_value, find, replace, options);
                    modified |= new_value != old_value;
                    substance->SetName(new_value);
                }
            }
        }
        if (pub->GetMedline().IsSetMesh())
        {
            for (auto mesh : pub->SetMedline().SetMesh())
            {
                if (mesh->IsSetTerm())
                {
                    const string &old_value = mesh->GetTerm();
                    string new_value =  ReplaceValue(old_value, find, replace, options);
                    modified |= new_value != old_value;
                    mesh->SetTerm(new_value);
                }
            }
        }
        if (pub->GetMedline().IsSetXref())
        {
            for (auto xref : pub->SetMedline().SetXref())
            {
                if (xref->IsSetCit())
                {
                    const string &old_value = xref->GetCit();
                    string new_value =  ReplaceValue(old_value, find, replace, options);
                    modified |= new_value != old_value;
                    xref->SetCit(new_value);
                }
            }
        }
        if (pub->GetMedline().IsSetIdnum())
        {
            for (auto &str : pub->SetMedline().SetIdnum())
            {
                const string &old_value = str;
                string new_value =  ReplaceValue(old_value, find, replace, options);
                modified |= new_value != old_value;
                str = new_value;
            }
        }
        if (pub->GetMedline().IsSetGene())
        {
            for (auto &str : pub->SetMedline().SetGene())
            {
                const string &old_value = str;
                string new_value =  ReplaceValue(old_value, find, replace, options);
                modified |= new_value != old_value;
                str = new_value;
            }
        }
        if (pub->GetMedline().IsSetPub_type())
        {
            for (auto &str : pub->SetMedline().SetPub_type())
            {
                const string &old_value = str;
                string new_value =  ReplaceValue(old_value, find, replace, options);
                modified |= new_value != old_value;
                str = new_value;
            }
        }
        if (pub->GetMedline().IsSetMlfield())
        {
            for (auto field : pub->SetMedline().SetMlfield())
            {
                if (field->IsSetStr())
                {
                    const string &old_value = field->GetStr();
                    string new_value =  ReplaceValue(old_value, find, replace, options);
                    modified |= new_value != old_value;
                    field->SetStr(new_value);
                }
            }
        }
    }
    if (pub->IsArticle())
    {
        modified |= ReplaceInArticle(pub->SetArticle(), find, replace, options); 
    }
    if (pub->IsJournal())
    {
        if (pub->GetJournal().IsSetTitle() && pub->GetJournal().GetTitle().IsSet())
        {
            for (auto journal : pub->SetJournal().SetTitle().Set())
            {
                modified |= ReplaceInJournal(journal, find, replace, options);  
            }
        }
        if (pub->GetJournal().IsSetImp())
        {
            modified |= ReplaceInImp(pub->SetJournal().SetImp(), find, replace, options);  
        }
    }
    if (pub->IsBook())
    {
        modified |= ReplaceInBook(pub->SetBook(), find, replace, options);  
    }
    if (pub->IsProc())
    {
        if (pub->GetProc().IsSetBook())
        {
            modified |= ReplaceInBook(pub->SetProc().SetBook(), find, replace, options);  
        }
        if (pub->GetProc().IsSetMeet())
        {
            if (pub->GetProc().GetMeet().IsSetNumber())
            {
                const string &old_value = pub->GetProc().GetMeet().GetNumber();
                string new_value =  ReplaceValue(old_value, find, replace, options);
                modified |= new_value != old_value;
                pub->SetProc().SetMeet().SetNumber(new_value);
            }
            if (pub->GetProc().GetMeet().IsSetPlace())
            {
                modified |= ReplaceInAffil(pub->SetProc().SetMeet().SetPlace(), find, replace, options);  
            }
        }
    }
    if (pub->IsPatent())
    {
        if (pub->GetPatent().IsSetAuthors())
        {
            modified |= ReplaceInAuthList(pub->SetPatent().SetAuthors(), find, replace, options);
        }
        if (pub->GetPatent().IsSetApplicants())
        {
            modified |= ReplaceInAuthList(pub->SetPatent().SetApplicants(), find, replace, options);
        }
        if (pub->GetPatent().IsSetAuthors())
        {
            modified |= ReplaceInAuthList(pub->SetPatent().SetAuthors(), find, replace, options);
        }
        if (pub->GetPatent().IsSetCountry())
        {
            const string &old_value = pub->GetPatent().GetCountry();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            pub->SetPatent().SetCountry(new_value);
        }
        if (pub->GetPatent().IsSetTitle())
        {
            const string &old_value = pub->GetPatent().GetTitle();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            pub->SetPatent().SetTitle(new_value);
        }
        if (pub->GetPatent().IsSetDoc_type())
        {
            const string &old_value = pub->GetPatent().GetDoc_type();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            pub->SetPatent().SetDoc_type(new_value);
        }
        if (pub->GetPatent().IsSetNumber())
        {
            const string &old_value = pub->GetPatent().GetNumber();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            pub->SetPatent().SetNumber(new_value);
        }
        if (pub->GetPatent().IsSetApp_number())
        {
            const string &old_value = pub->GetPatent().GetApp_number();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            pub->SetPatent().SetApp_number(new_value);
        }
        if (pub->GetPatent().IsSetAbstract())
        {
            const string &old_value = pub->GetPatent().GetAbstract();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            pub->SetPatent().SetAbstract(new_value);
        }

    }
    if (pub->IsPat_id())
    {
        if (pub->GetPat_id().IsSetCountry())
        {
            const string &old_value = pub->GetPat_id().GetCountry();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            pub->SetPat_id().SetCountry(new_value);
        }
        if (pub->GetPat_id().IsSetDoc_type())
        {
            const string &old_value = pub->GetPat_id().GetDoc_type();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            pub->SetPat_id().SetDoc_type(new_value);
        }
        if (pub->GetPat_id().IsSetId() && pub->GetPat_id().GetId().IsNumber())
        {
            const string &old_value = pub->GetPat_id().GetId().GetNumber();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            pub->SetPat_id().SetId().SetNumber(new_value);
        }
        if (pub->GetPat_id().IsSetId() && pub->GetPat_id().GetId().IsApp_number())
        {
            const string &old_value = pub->GetPat_id().GetId().GetApp_number();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            pub->SetPat_id().SetId().SetApp_number(new_value);
        }
    }
    if (pub->IsMan() && pub->GetMan().IsSetCit())
    {
        modified |= ReplaceInBook(pub->SetMan().SetCit(), find, replace, options); 
    }
    if (pub->IsEquiv() && pub->GetEquiv().IsSet())
    {
         for (auto pub_equiv : pub->SetEquiv().Set())
         {
             modified |= ReplaceInPub(pub_equiv, find, replace, options);  
         }
    }
    return modified;
}

bool CFindASN1Dlg::ReplaceInAuthList(CAuth_list &auth, const string &find, const string &replace, CRegexp::TCompile options)
{
    bool modified = false;
    if (auth.IsSetAffil())
    {
        modified |= ReplaceInAffil(auth.SetAffil(), find, replace, options);  
    }
    if (auth.IsSetNames())
    {
        if (auth.GetNames().IsStd())
        {
            for (auto author : auth.SetNames().SetStd())
            {
                modified |= ReplaceInAuthor(*author, find, replace, options);  
            }
        }
        if (auth.GetNames().IsMl())
        {
            for (auto &str : auth.SetNames().SetMl())
            {
                const string &old_value = str;
                string new_value =  ReplaceValue(old_value, find, replace, options);
                modified |= new_value != old_value;
                str = new_value;
            }
        }
        if (auth.GetNames().IsStr())
        {
            for (auto &str : auth.SetNames().SetStr())
            {
                const string &old_value = str;
                string new_value =  ReplaceValue(old_value, find, replace, options);
                modified |= new_value != old_value;
                str = new_value;
            }
        }
    }

    return modified;
}

bool CFindASN1Dlg::ReplaceInJournal(CRef<CTitle::C_E> journal, const string &find, const string &replace, CRegexp::TCompile options)
{
    bool modified = false;
    string *str = NULL;
    switch(journal->Which())
    {
    case CTitle::C_E::e_Name : str = &journal->SetName(); break;
    case CTitle::C_E::e_Tsub : str = &journal->SetTsub(); break;
    case CTitle::C_E::e_Trans : str = &journal->SetTrans(); break;
    case CTitle::C_E::e_Jta : str = &journal->SetJta(); break;
    case CTitle::C_E::e_Iso_jta : str = &journal->SetIso_jta(); break;
    case CTitle::C_E::e_Ml_jta : str = &journal->SetMl_jta(); break;
    case CTitle::C_E::e_Coden : str = &journal->SetCoden(); break;
    case CTitle::C_E::e_Issn : str = &journal->SetIssn(); break;
    case CTitle::C_E::e_Abr : str = &journal->SetAbr(); break;
    case CTitle::C_E::e_Isbn : str = &journal->SetIsbn(); break;       
    default: break;
    }

    if (str)
    {
        const string &old_value = *str;
        string new_value =  ReplaceValue(old_value, find, replace, options);
        modified |= new_value != old_value;
        *str = new_value;
    }
    return modified;
}

bool CFindASN1Dlg::ReplaceInAffil(CAffil &affil, const string &find, const string &replace, CRegexp::TCompile options)
{
    bool modified = false;
    if (affil.IsStr())
    {
            const string &old_value = affil.GetStr();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            affil.SetStr(new_value);
    }
    if (affil.IsStd())
    {
        if (affil.GetStd().IsSetAffil())
        {
            const string &old_value = affil.GetStd().GetAffil();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            affil.SetStd().SetAffil(new_value);
        }
        if (affil.GetStd().IsSetDiv())
        {
            const string &old_value = affil.GetStd().GetDiv();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            affil.SetStd().SetDiv(new_value);
        }
        if (affil.GetStd().IsSetCity())
        {
            const string &old_value = affil.GetStd().GetCity();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            affil.SetStd().SetCity(new_value);
        }
        if (affil.GetStd().IsSetSub())
        {
            const string &old_value = affil.GetStd().GetSub();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            affil.SetStd().SetSub(new_value);
        }
        if (affil.GetStd().IsSetCountry())
        {
            const string &old_value = affil.GetStd().GetCountry();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            affil.SetStd().SetCountry(new_value);
        }
        if (affil.GetStd().IsSetStreet())
        {
            const string &old_value = affil.GetStd().GetStreet();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            affil.SetStd().SetStreet(new_value);
        }
        if (affil.GetStd().IsSetEmail())
        {
            const string &old_value = affil.GetStd().GetEmail();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            affil.SetStd().SetEmail(new_value);
        }
        if (affil.GetStd().IsSetFax())
        {
            const string &old_value = affil.GetStd().GetFax();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            affil.SetStd().SetFax(new_value);
        }
        if (affil.GetStd().IsSetPhone())
        {
            const string &old_value = affil.GetStd().GetPhone();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            affil.SetStd().SetPhone(new_value);
        }
        if (affil.GetStd().IsSetPostal_code())
        {
            const string &old_value = affil.GetStd().GetPostal_code();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            affil.SetStd().SetPostal_code(new_value);
        }
    }
    return modified;
}

bool CFindASN1Dlg::ReplaceInAuthor(CAuthor &author, const string &find, const string &replace, CRegexp::TCompile options)
{
    bool modified = false;
    if (author.IsSetAffil())
    {
        modified |= ReplaceInAffil(author.SetAffil(), find, replace, options);  
    }
    if (author.IsSetName())
    {
        if (author.GetName().IsDbtag())
        {
            modified |= ReplaceInDbxref(author.SetName().SetDbtag(), find, replace, options);       
        }
        if (author.GetName().IsMl())
        {
            const string &old_value = author.GetName().GetMl();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            author.SetName().SetMl(new_value);
        }
        if (author.GetName().IsStr())
        {
            const string &old_value = author.GetName().GetStr();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            author.SetName().SetStr(new_value);
        }
        if (author.GetName().IsConsortium())
        {
            const string &old_value = author.GetName().GetConsortium();
            string new_value =  ReplaceValue(old_value, find, replace, options);
            modified |= new_value != old_value;
            author.SetName().SetConsortium(new_value);
        }
        if (author.GetName().IsName())
        {
            if (author.GetName().GetName().IsSetLast())
            {
                const string &old_value = author.GetName().GetName().GetLast();
                string new_value =  ReplaceValue(old_value, find, replace, options);
                modified |= new_value != old_value;
                author.SetName().SetName().SetLast(new_value);
            }
            if (author.GetName().GetName().IsSetFirst())
            {
                const string &old_value = author.GetName().GetName().GetFirst();
                string new_value =  ReplaceValue(old_value, find, replace, options);
                modified |= new_value != old_value;
                author.SetName().SetName().SetFirst(new_value);
            }
            if (author.GetName().GetName().IsSetMiddle())
            {
                const string &old_value = author.GetName().GetName().GetMiddle();
                string new_value =  ReplaceValue(old_value, find, replace, options);
                modified |= new_value != old_value;
                author.SetName().SetName().SetMiddle(new_value);
            }
            if (author.GetName().GetName().IsSetFull())
            {
                const string &old_value = author.GetName().GetName().GetFull();
                string new_value =  ReplaceValue(old_value, find, replace, options);
                modified |= new_value != old_value;
                author.SetName().SetName().SetFull(new_value);
            }
            if (author.GetName().GetName().IsSetInitials())
            {
                const string &old_value = author.GetName().GetName().GetInitials();
                string new_value =  ReplaceValue(old_value, find, replace, options);
                modified |= new_value != old_value;
                author.SetName().SetName().SetInitials(new_value);
            }
            if (author.GetName().GetName().IsSetSuffix())
            {
                const string &old_value = author.GetName().GetName().GetSuffix();
                string new_value =  ReplaceValue(old_value, find, replace, options);
                modified |= new_value != old_value;
                author.SetName().SetName().SetSuffix(new_value);
            }
            if (author.GetName().GetName().IsSetTitle())
            {
                const string &old_value = author.GetName().GetName().GetTitle();
                string new_value =  ReplaceValue(old_value, find, replace, options);
                modified |= new_value != old_value;
                author.SetName().SetName().SetTitle(new_value);
            }
        }
    }

    return modified;
}

bool CFindASN1Dlg::ReplaceInArticle(CCit_art &article, const string &find, const string &replace, CRegexp::TCompile options)
{
    bool modified = false;
    if (article.IsSetAuthors())
    {
        modified |= ReplaceInAuthList(article.SetAuthors(), find, replace, options); 
    }
    if (article.IsSetTitle() && article.GetTitle().IsSet())
    {
        for (auto journal : article.SetTitle().Set())
        {
            modified |= ReplaceInJournal(journal, find, replace, options);  
        }
    }
    if (article.IsSetFrom())
    {
        if (article.GetFrom().IsJournal())
        {
            if (article.GetFrom().GetJournal().IsSetTitle() && article.GetFrom().GetJournal().GetTitle().IsSet())
            {
                for (auto journal : article.SetFrom().SetJournal().SetTitle().Set())
                {
                    modified |= ReplaceInJournal(journal, find, replace, options);  
                }
            }
            if (article.GetFrom().GetJournal().IsSetImp())
            {
                modified |= ReplaceInImp(article.SetFrom().SetJournal().SetImp(), find, replace, options);
            }            
        }
        if (article.GetFrom().IsBook())
        {
            modified |= ReplaceInBook(article.SetFrom().SetBook(), find, replace, options);
        }         
        if (article.GetFrom().IsProc())
        {
            if (article.GetFrom().GetProc().IsSetBook())
            {
                modified |= ReplaceInBook(article.SetFrom().SetProc().SetBook(), find, replace, options);
            }        
        }   
    }
    return modified;
}

bool CFindASN1Dlg::ReplaceInImp(CImprint &imp, const string &find, const string &replace, CRegexp::TCompile options)
{
    bool modified = false;
    if (imp.IsSetVolume())
    {
        const string &old_value = imp.GetVolume();
        string new_value =  ReplaceValue(old_value, find, replace, options);
        modified |= new_value != old_value;
        imp.SetVolume(new_value);
    }
    if (imp.IsSetIssue())
    {
        const string &old_value = imp.GetIssue();
        string new_value =  ReplaceValue(old_value, find, replace, options);
        modified |= new_value != old_value;
        imp.SetIssue(new_value);
    }
    if (imp.IsSetPages())
    {
        const string &old_value = imp.GetPages();
        string new_value =  ReplaceValue(old_value, find, replace, options);
        modified |= new_value != old_value;
        imp.SetPages(new_value);
    }
    if (imp.IsSetSection())
    {
        const string &old_value = imp.GetSection();
        string new_value =  ReplaceValue(old_value, find, replace, options);
        modified |= new_value != old_value;
        imp.SetSection(new_value);
    }
    if (imp.IsSetLanguage())
    {
        const string &old_value = imp.GetLanguage();
        string new_value =  ReplaceValue(old_value, find, replace, options);
        modified |= new_value != old_value;
        imp.SetLanguage(new_value);
    }
    if (imp.IsSetPart_sup())
    {
        const string &old_value = imp.GetPart_sup();
        string new_value =  ReplaceValue(old_value, find, replace, options);
        modified |= new_value != old_value;
        imp.SetPart_sup(new_value);
    }
    if (imp.IsSetPart_supi())
    {
        const string &old_value = imp.GetPart_supi();
        string new_value =  ReplaceValue(old_value, find, replace, options);
        modified |= new_value != old_value;
        imp.SetPart_supi(new_value);
    }
    if (imp.IsSetPub())
    {
        modified |= ReplaceInAffil(imp.SetPub(), find, replace, options); 
    }
    if (imp.IsSetRetract() && imp.GetRetract().IsSetExp())
    {
        const string &old_value = imp.GetRetract().GetExp();
        string new_value =  ReplaceValue(old_value, find, replace, options);
        modified |= new_value != old_value;
        imp.SetRetract().SetExp(new_value);
    }
    
    return modified;
}

bool CFindASN1Dlg::ReplaceInBook(CCit_book &book, const string &find, const string &replace, CRegexp::TCompile options)
{
    bool modified = false;
    if (book.IsSetTitle() && book.GetTitle().IsSet())
    {
        for (auto journal : book.SetTitle().Set())
        {
            modified |= ReplaceInJournal(journal, find, replace, options);  
        }
    }
    if (book.IsSetColl() && book.GetColl().IsSet())
    {
        for (auto journal : book.SetColl().Set())
        {
            modified |= ReplaceInJournal(journal, find, replace, options);  
        }
    }
    if (book.IsSetAuthors())
    {
        modified |= ReplaceInAuthList(book.SetAuthors(), find, replace, options); 
    }
    if (book.IsSetImp())
    {
        modified |= ReplaceInImp(book.SetImp(), find, replace, options);
    }            
    return modified;
}

END_NCBI_SCOPE
