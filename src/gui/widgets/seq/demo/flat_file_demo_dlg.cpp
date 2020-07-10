/*  $Id: flat_file_demo_dlg.cpp 37314 2016-12-22 19:12:37Z katargir $
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
 * Authors:  Roman Katargin
 */


#include <ncbi_pch.hpp>
////@begin includes
////@end includes

#include "flat_file_demo_dlg.hpp"

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/dcclient.h>
#include <wx/settings.h>
#include <wx/icon.h>
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <wx/utils.h>

#include <wx/wfstream.h>
#include <wx/txtstrm.h>

#include <objtools/data_loaders/genbank/gbloader.hpp>

#include <gui/objutils/objects.hpp>
#include <gui/objutils/label.hpp>
#include <gui/widgets/seq/text_panel.hpp>
//#include <gui/widgets/text_widget/text_item.hpp>
#include <gui/widgets/text_widget/text_item_panel.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <serial/objistr.hpp>
#include <objects/general/User_object.hpp>

BEGIN_NCBI_SCOPE

////@begin XPM images
////@end XPM images


/*!
 * CFlatFileDemoDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CFlatFileDemoDlg, wxDialog )


/*!
 * CFlatFileDemoDlg event table definition
 */

BEGIN_EVENT_TABLE( CFlatFileDemoDlg, wxDialog )

////@begin CFlatFileDemoDlg event table entries
    EVT_CLOSE( CFlatFileDemoDlg::OnCloseWindow )

    EVT_BUTTON( ID_FILE, CFlatFileDemoDlg::OnLoadFileClick )

    EVT_BUTTON( ID_ACC, CFlatFileDemoDlg::OnAccClick )

    EVT_BUTTON( ID_BUTTON1, CFlatFileDemoDlg::OnSaveText )

    EVT_BUTTON( wxID_CANCEL, CFlatFileDemoDlg::OnCancelClick )

////@end CFlatFileDemoDlg event table entries

END_EVENT_TABLE()


/*!
 * CFlatFileDemoDlg constructors
 */

CFlatFileDemoDlg::CFlatFileDemoDlg()
{
    Init();
}

CFlatFileDemoDlg::CFlatFileDemoDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CFlatFileDemoDlg creator
 */

bool CFlatFileDemoDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CFlatFileDemoDlg creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CFlatFileDemoDlg creation
    return true;
}


/*!
 * CFlatFileDemoDlg destructor
 */

CFlatFileDemoDlg::~CFlatFileDemoDlg()
{
////@begin CFlatFileDemoDlg destruction
////@end CFlatFileDemoDlg destruction
}


/*!
 * Member initialisation
 */

void CFlatFileDemoDlg::Init()
{
////@begin CFlatFileDemoDlg member initialisation
    m_LoadedStatus = NULL;
    m_AccInput = NULL;
////@end CFlatFileDemoDlg member initialisation

    m_ObjMgr = CObjectManager::GetInstance();
    CGBDataLoader::RegisterInObjectManager(*m_ObjMgr);

    m_Scope.Reset(new CScope(*m_ObjMgr));
    m_Scope->AddDefaults();

    CUser_object::GetTypeInfo();
}


/*!
 * Control creation for CFlatFileDemoDlg
 */

void CFlatFileDemoDlg::CreateControls()
{
////@begin CFlatFileDemoDlg content construction
    CFlatFileDemoDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    CTextPanel* itemPanel3 = new CTextPanel( itemDialog1, ID_WINDOW, wxDefaultPosition, itemDialog1->ConvertDialogToPixels(wxSize(50, 50)), wxNO_BORDER );
    itemBoxSizer2->Add(itemPanel3, 1, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_LEFT|wxALL, 0);

    m_LoadedStatus = new wxStaticText( itemDialog1, wxID_STATIC, _("Not loaded..."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(m_LoadedStatus, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticLine* itemStaticLine6 = new wxStaticLine( itemDialog1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer2->Add(itemStaticLine6, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer7, 0, wxGROW|wxALL, 0);

    wxFlexGridSizer* itemFlexGridSizer8 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer7->Add(itemFlexGridSizer8, 1, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    itemFlexGridSizer8->Add(0, 0, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton10 = new wxButton( itemDialog1, ID_FILE, _("Load File..."), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer8->Add(itemButton10, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AccInput = new wxTextCtrl( itemDialog1, ID_ACC_TEXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_AccInput->SetMaxLength(20);
    m_AccInput->SetHelpText(_("Enter Accession"));
    if (CFlatFileDemoDlg::ShowToolTips())
        m_AccInput->SetToolTip(_("Enter Accession"));
    itemFlexGridSizer8->Add(m_AccInput, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton12 = new wxButton( itemDialog1, ID_ACC, _("Load Accession"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer8->Add(itemButton12, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer8->AddGrowableCol(0);

    wxStaticLine* itemStaticLine13 = new wxStaticLine( itemDialog1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL );
    itemBoxSizer7->Add(itemStaticLine13, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer14 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer7->Add(itemBoxSizer14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton15 = new wxButton( itemDialog1, ID_BUTTON1, _("Save Text..."), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton15->Enable(false);
    itemBoxSizer14->Add(itemButton15, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton16 = new wxButton( itemDialog1, wxID_CANCEL, _("&Close"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer14->Add(itemButton16, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CFlatFileDemoDlg content construction

    m_AccInput->SetValue(wxT("U49845"));

    FindWindow(ID_ACC)->SetFocus();
    FindWindow(ID_WINDOW)->Show();
}

/*!
 * Should we show tooltips?
 */

bool CFlatFileDemoDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CFlatFileDemoDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CFlatFileDemoDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CFlatFileDemoDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CFlatFileDemoDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CFlatFileDemoDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CFlatFileDemoDlg icon retrieval
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_ACC
 */

void CFlatFileDemoDlg::OnAccClick( wxCommandEvent& WXUNUSED(event) )
{
    wxBusyCursor wait;

    CRef<CSeq_id> id(new CSeq_id());

    string acc = ToStdString(m_AccInput->GetValue());
    try {
        id->Set(acc);
    }
    catch (CSeqIdException&) {
        string msg("Accession '");
        msg += acc + " not recognized as a valid accession";
        wxMessageBox(ToWxString(msg), _("Cannot load accesion"),
                     wxOK | wxICON_ERROR);
        return;
    }

    CTextPanel* textPanel = (CTextPanel*)FindWindow(ID_WINDOW);
    SConstScopedObject obj(id, m_Scope);
    TConstScopedObjects objects;
    objects.push_back(obj);
    textPanel->InitWidget(objects);
    FindWindow(ID_BUTTON1)->Enable();

    /*{{
        CBioseq_Handle handle = m_Scope->GetBioseqHandle(*id);
        if (handle) {
            CSeq_entry_Handle se = handle.GetTopLevelEntry();
            CConstRef<CSerialObject> so(se.GetSeq_entryCore());
            CNcbiOfstream("c:\\tmp\\test.xml") << MSerial_Xml << *so;
        }
     }}*/

    string label;
    CLabel::GetLabel(*id, &label, CLabel::eDefault, m_Scope.GetPointer());
    m_LoadedStatus->SetLabel(ToWxString("Accession: " + label));
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_FILE
 */

void CFlatFileDemoDlg::OnLoadFileClick( wxCommandEvent& WXUNUSED(event) )
{
    wxFileDialog dlg(this, _("Select a file"), _(""), _(""),
                     _("ASN files (*.asn)|*.asn|All files (*.*)|*.*"),
                     wxFD_OPEN);

    if (dlg.ShowModal() != wxID_OK)
        return;

    wxString fileName = dlg.GetPath();
    if (fileName.empty())
        return;

    CRef<CSerialObject> so;

    try {
        wxBusyCursor wait;
        CNcbiIfstream istr(fileName.fn_str());
        auto_ptr<CObjectIStream> os(CObjectIStream::Open(eSerial_AsnText, istr));
        string header = os->ReadFileHeader();
        TTypeInfo type_info = CClassTypeInfo::GetClassInfoByName(header);
        so.Reset((CSerialObject*)type_info->Create());
        os->ReadObject(so.GetPointer(), type_info);
    } catch(CException& e)  {
        LOG_POST(Error << e.ReportAll());
        string s = e.GetMsg();
        wxMessageBox(ToWxString(s), _("Error loading file."),
                     wxOK | wxICON_ERROR);
        return;
    }

    CSeq_entry* seq_entry = dynamic_cast<CSeq_entry*>(so.GetPointer());
    if (seq_entry)
        m_Scope->AddTopLevelSeqEntry(*seq_entry);
    CBioseq* bioseq = dynamic_cast<CBioseq*>(so.GetPointer());
    if (bioseq)
        m_Scope->AddBioseq(*bioseq);

    CTextPanel* textPanel = (CTextPanel*)FindWindow(ID_WINDOW);
    SConstScopedObject obj(so, m_Scope);
    TConstScopedObjects objects;
    objects.push_back(obj);
    textPanel->InitWidget(objects);
    FindWindow(ID_BUTTON1)->Enable();

    m_LoadedStatus->SetLabel(_("File: ") + dlg.GetFilename());
}

/*!
 * wxEVT_CLOSE_WINDOW event handler for ID_HIT_MATRIX_DEMO_DLG
 */

void CFlatFileDemoDlg::OnCancelClick( wxCommandEvent& WXUNUSED(event) )
{
    Destroy();
}


/*!
 * wxEVT_CLOSE_WINDOW event handler for ID_HIT_MATRIX_DEMO_DLG
 */

void CFlatFileDemoDlg::OnCloseWindow( wxCloseEvent& WXUNUSED(event) )
{
    Destroy();
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON1
 */

void CFlatFileDemoDlg::OnSaveText( wxCommandEvent& WXUNUSED(event) )
{
    static wxString filePath;
    wxFileDialog dlg(this, _("Select a file"), _(""), filePath,
                     _("All files (*.*)|*.*"),
                     wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

    if (dlg.ShowModal() != wxID_OK)
        return;

    filePath = dlg.GetPath();

    CTextPanel* textPanel = (CTextPanel*)FindWindow(ID_WINDOW);
    CTextItemPanel* textItemPanel = textPanel->GetWidget();
    ITextItem* textItem = textItemPanel->GetRootItem();
    if (!textItem) return;

    wxFFileOutputStream stream(filePath);
    wxTextOutputStream os(stream);
    textItem->GetText(os, textItem->GetItemBlock(), textItemPanel->GetContext());

    if (stream.GetLastError() != wxSTREAM_NO_ERROR)
        wxMessageBox(_("Failed to write file: \"") + filePath + _("\"."),
                     _("Write Error"), wxOK|wxICON_ERROR);
}

END_NCBI_SCOPE
