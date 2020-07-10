/*  $Id: remove_desc_dlg.cpp 45081 2020-05-26 20:33:57Z asztalos $
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
#include <serial/objostr.hpp>
#include <serial/serial.hpp>
#include <objmgr/scope.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <gui/objutils/cmd_del_desc.hpp>
#include <objtools/format/flat_file_generator.hpp>
#include <gui/packages/pkg_sequence_edit/remove_desc_dlg.hpp>

#include <util/xregexp/regexp.hpp>

#include <wx/button.h>
#include <wx/statbox.h>
#include <wx/sizer.h>
#include <wx/listctrl.h>
#include <wx/radiobut.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CRemoveDescDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CRemoveDescDlg, wxDialog )


/*!
 * CRemoveDescDlg event table definition
 */

BEGIN_EVENT_TABLE( CRemoveDescDlg, wxDialog )

////@begin CRemoveDescDlg event table entries
////@end CRemoveDescDlg event table entries

END_EVENT_TABLE()


/*!
 * CRemoveDescDlg constructors
 */

CRemoveDescDlg::CRemoveDescDlg()
{
    Init();
}

CRemoveDescDlg::CRemoveDescDlg( wxWindow* parent, CSeq_entry_Handle seh, 
    wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_TopSeqEntry(seh)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CVectorTrimDlg creator
 */

bool CRemoveDescDlg::Create( wxWindow* parent, 
    wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CRemoveDescDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CRemoveDescDlg creation

    SetSize(wxSize(280, 750));
    return true;
}


/*!
 * CRemoveDescDlg destructor
 */

CRemoveDescDlg::~CRemoveDescDlg()
{
////@begin CRemoveDescDlg destruction
////@end CRemoveDescDlg destruction
}


/*!
 * Member initialisation
 */

void CRemoveDescDlg::Init()
{
    m_Types.clear();
    m_Types["Name"] = CSeqdesc::e_Name;
    m_Types["Title"] = CSeqdesc::e_Title;        
    m_Types["Comment"] = CSeqdesc::e_Comment;      
    m_Types["Numbering"] = CSeqdesc::e_Num;          
    m_Types["MapLoc"] = CSeqdesc::e_Maploc;       
    m_Types["PIR"] = CSeqdesc::e_Pir;          
    m_Types["GenBank"] = CSeqdesc::e_Genbank;
    m_Types["Publication"] = CSeqdesc::e_Pub;          
    m_Types["Region"] = CSeqdesc::e_Region;       
    m_Types["User"] = CSeqdesc::e_User;         
    m_Types["SWISS-PROT"] = CSeqdesc::e_Sp;
    m_Types["dbXREF"] = CSeqdesc::e_Dbxref;       
    m_Types["EMBL"] = CSeqdesc::e_Embl;         
    m_Types["Create Date"] = CSeqdesc::e_Create_date;
    m_Types["Update Date"] = CSeqdesc::e_Update_date;  
    m_Types["PRF"] = CSeqdesc::e_Prf;          
    m_Types["PDB"] = CSeqdesc::e_Pdb;          
    m_Types["Heterogen"] = CSeqdesc::e_Het;
    m_Types["BioSource"] = CSeqdesc::e_Source;       
    m_Types["MolInfo"] = CSeqdesc::e_Molinfo;      
    m_Types["StructuredComment"] = CSeqdesc::e_User;
    m_Types["DBLink"] = CSeqdesc::e_User;

    m_ListCtrl = NULL;
    m_TextCtrl = nullptr;
    m_RadioButton = nullptr;
    m_CheckBox = nullptr;
}


/*!
 * Control creation for CRemoveDescDlg
 */

void CRemoveDescDlg::CreateControls()
{    
    CRemoveDescDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_ListCtrl = new wxListCtrl( itemDialog1, ID_LISTCTRL_DESC_DLG, wxDefaultPosition, wxSize(240, 500), wxLC_REPORT );
    itemBoxSizer3->Add(m_ListCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ListCtrl->InsertColumn(0, "    Descriptor", wxLIST_FORMAT_LEFT, 237);
    int i = 0;
    for (map<string,CSeqdesc::E_Choice>::iterator e = m_Types.begin(); e != m_Types.end(); e++)
    {
        m_ListCtrl->InsertItem(i, ToWxString(e->first));
        m_ListCtrl->SetItemData(i,e->second);
        i++;
    }

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer5->Add(itemBoxSizer6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer7Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Optional String Constraint"));
    wxStaticBoxSizer* itemStaticBoxSizer7 = new wxStaticBoxSizer(itemStaticBoxSizer7Static, wxVERTICAL);
    itemBoxSizer6->Add(itemStaticBoxSizer7, 0, wxALIGN_LEFT|wxALL, 5);

    m_RadioButton = new wxRadioButton(itemDialog1, ID_RBTN_RMVDESC1, _("Remove When Text Is Present"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    m_RadioButton->SetValue(true);
    itemStaticBoxSizer7->Add(m_RadioButton, 0, wxALIGN_LEFT|wxALL, 5);

    wxRadioButton* itemRadioButton9 = new wxRadioButton(itemDialog1, ID_RBTN_RMVDESC2, _("Remove When Text Is Not Present"));
    itemRadioButton9->SetValue(false);
    itemStaticBoxSizer7->Add(itemRadioButton9, 0, wxALIGN_LEFT|wxALL, 5);

    m_TextCtrl = new wxTextCtrl(itemDialog1, ID_TXT_RMVDESC, wxEmptyString, wxDefaultPosition, wxSize(237, -1), 0);
    itemStaticBoxSizer7->Add(m_TextCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    m_CheckBox = new wxCheckBox(itemDialog1, ID_CHKBOX_RMVDESC, _("Case Insensitive"), wxDefaultPosition, wxDefaultSize, 0);
    m_CheckBox->SetValue(false);
    itemStaticBoxSizer7->Add(m_CheckBox, 0, wxALIGN_LEFT|wxALL, 5);

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

bool CRemoveDescDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CRemoveDescDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CVectorTrimDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CVectorTrimDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CRemoveDescDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CVectorTrimDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CVectorTrimDlg icon retrieval
}

CRef<CCmdComposite> CRemoveDescDlg::GetCommand()
{
    long item = -1;
    CRef<CCmdComposite> cmd( new CCmdComposite("Remove Descriptors") );
    if (m_ListCtrl && m_TopSeqEntry)
        for ( ;; )
        {
            item = m_ListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            if ( item == -1 )  break;
            CSeqdesc::E_Choice choice = static_cast<CSeqdesc::E_Choice>(m_ListCtrl->GetItemData(item)); 
            string name = ToStdString(m_ListCtrl->GetItemText(item));
            x_ApplyToSeq_entry(m_TopSeqEntry, *(m_TopSeqEntry.GetCompleteSeq_entry()), cmd, choice, name);
        }
    return cmd;
}

void CRemoveDescDlg::x_ApplyToSeq_entry(CSeq_entry_Handle tse, 
    const CSeq_entry& se, CCmdComposite* composite, CSeqdesc::E_Choice choice, const string& name)
{

    FOR_EACH_SEQDESC_ON_SEQENTRY (it, se) 
    {
        if ((*it)->Which() == choice) 
        {
            bool go = true;
            if (choice == CSeqdesc::e_User && name != "User" )
            {
                go = false;
                if ((*it)->GetUser().IsSetType() && 
                    (*it)->GetUser().GetType().IsStr() && 
                    (*it)->GetUser().GetType().GetStr() == name)
                    go = true;
            }
            if (go)
            {
                CSeq_entry_Handle seh = tse.GetScope().GetSeq_entryHandle(se);

                if (m_TextCtrl->GetLineLength(0) > 0)
                {
                    go = false;
                    CNcbiOstrstream ostr;

                    // this is for ASN.1 text format
                    auto_ptr<CObjectOStream>  out(CObjectOStream::Open(eSerial_AsnText, ostr));   
                    *out << **it;
                    string str = CNcbiOstrstreamToString(ostr);

                    // This is for Flat File generation
                    //CFlatFileGenerator ff;  
                    //ff.Generate(seh, ostr); 
                    //string str = CNcbiOstrstreamToString(ostr);

                    CRegexp pattern(ToStdString(m_TextCtrl->GetLineText(0))); 
                    if (m_CheckBox->GetValue())
                        pattern.Set(ToStdString(m_TextCtrl->GetLineText(0)),CRegexp::fCompile_ignore_case);
                    bool match = pattern.IsMatch(str);
                    if (match && m_RadioButton->GetValue()) go = true;
                    else if (!match && !m_RadioButton->GetValue()) go = true;
                }
                
                if (go)
                {
                    CIRef<IEditCommand> cmdDelDesc(new CCmdDelDesc(seh, **it));
                    composite->AddCommand(*cmdDelDesc);
                }
            }
        }
    }

    if (se.IsSet()) {
        FOR_EACH_SEQENTRY_ON_SEQSET (it, se.GetSet()) {
            x_ApplyToSeq_entry(tse, **it, composite, choice, name);
        }
    }
}


END_NCBI_SCOPE
