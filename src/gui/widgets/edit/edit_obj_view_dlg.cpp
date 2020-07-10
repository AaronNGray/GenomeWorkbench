/*  $Id: edit_obj_view_dlg.cpp 43609 2019-08-08 16:12:53Z filippov $
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
////@begin includes
////@end includes

#include <objmgr/bioseq_ci.hpp>
#include <gui/widgets/edit/edit_obj_view_dlg.hpp>
#include <gui/widgets/edit/edit_object_seq_feat.hpp>
#include <gui/widgets/edit/edit_object_seq_desc.hpp>
#include <gui/widgets/edit/edit_object_set.hpp>
#include <gui/widgets/edit/bioseq_editor.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/utils/command_processor.hpp>
#include <gui/widgets/wx/file_extensions.hpp>
#include <gui/widgets/edit/utilities.hpp>
#include <gui/widgets/edit/publicationtype_panel.hpp>
#include <gui/widgets/edit/authornames_panel.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <objects/biblio/Auth_list.hpp>
#include <objects/biblio/Affil.hpp>
#include <objects/pub/Pub_equiv.hpp>
#include <objects/pub/Pub.hpp>
#include <objects/biblio/Cit_art.hpp>
#include <objects/biblio/Cit_book.hpp>
#include <objects/biblio/Cit_jour.hpp>
#include <objects/biblio/Cit_let.hpp>
#include <objects/biblio/Cit_pat.hpp>
#include <objects/biblio/Cit_proc.hpp>
#include <objects/biblio/Cit_sub.hpp>
#include <serial/serial.hpp>
#include <serial/objistr.hpp>
#include <serial/serial.hpp>
#include <wx/button.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/app.h>
#include <wx/panel.h>
#include <wx/hyperlink.h>
#include "location_panel.hpp"
////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

USING_NCBI_SCOPE;
USING_SCOPE(objects);

/*!
 * CEditObjViewDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CEditObjViewDlg, wxFrame )


/*!
 * CEditObjViewDlg event table definition
 */

BEGIN_EVENT_TABLE( CEditObjViewDlg, wxFrame )

////@begin CEditObjViewDlg event table entries
    EVT_BUTTON( wxID_OK, CEditObjViewDlg::OnOkClick )
    EVT_BUTTON( wxID_CANCEL, CEditObjViewDlg::OnCancelClick )
    EVT_BUTTON( CEDITOBJVIEWDLG_BUTTON_EXPORT, CEditObjViewDlg::OnExport )
    EVT_BUTTON( CEDITOBJVIEWDLG_BUTTON_IMPORT, CEditObjViewDlg::OnImport )
////@end CEditObjViewDlg event table entries

END_EVENT_TABLE()

void CEditObjViewDlg::Activate(wxIdleEvent&)
{
    if (!IsShown())
        Show(true);
    Unbind(wxEVT_IDLE, &CEditObjViewDlg::Activate, this);
}

/*!
 * CEditObjViewDlg constructors
 */

CEditObjViewDlg::CEditObjViewDlg()
{
    Init();
}

CEditObjViewDlg::CEditObjViewDlg( wxWindow* parent, bool create, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_Create(create)
{
    Init();
    Create(parent, id, caption, pos, size, style);
    NEditingStats::ReportUsage(caption);
}


/*!
 * CEditObjViewDlg creator
 */

bool CEditObjViewDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{

    if ( !parent )
    {
        parent = wxTheApp->GetTopWindow();
    }
////@begin CEditObjViewDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxFrame::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CEditObjViewDlg creation

    Bind(wxEVT_IDLE, &CEditObjViewDlg::Activate, this);
    return true;
}


/*!
 * CEditObjViewDlg destructor
 */

CEditObjViewDlg::~CEditObjViewDlg()
{
////@begin CEditObjViewDlg destruction
////@end CEditObjViewDlg destruction
}


/*!
 * Member initialisation
 */

void CEditObjViewDlg::Init()
{
////@begin CEditObjViewDlg member initialisation
    m_EditWindowSizer = NULL;
    m_ContainerPanel = NULL;
////@end CEditObjViewDlg member initialisation
    m_Editor = NULL;
    m_EditorWindow = NULL;
    m_ButtonImport = NULL;
    m_ButtonExport = NULL;
    m_ButtonHelp = NULL;
}


/*!
 * Control creation for CEditObjViewDlg
 */

void CEditObjViewDlg::CreateControls()
{    
////@begin CEditObjViewDlg content construction

    CEditObjViewDlg* itemCGuiWidgetDlg1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCGuiWidgetDlg1->SetSizer(itemBoxSizer2);

    m_ContainerPanel = new wxPanel(itemCGuiWidgetDlg1, wxID_ANY);
    itemBoxSizer2->Add(m_ContainerPanel, 1, wxGROW);

    wxBoxSizer* panel_sizer = new wxBoxSizer(wxVERTICAL);
    m_ContainerPanel->SetSizer(panel_sizer);

    m_EditWindowSizer = new wxBoxSizer(wxHORIZONTAL);
    panel_sizer->Add(m_EditWindowSizer, 1, wxGROW | wxALL, 5);

    wxStaticLine* itemStaticLine4 = new wxStaticLine(m_ContainerPanel, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
    panel_sizer->Add(itemStaticLine4, 0, wxGROW | wxALL, 5);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    panel_sizer->Add(itemBoxSizer3, 0, wxGROW | wxALL, 5);

    m_ButtonImport = new wxButton(m_ContainerPanel, CEDITOBJVIEWDLG_BUTTON_IMPORT, _("&Import"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer3->Add(m_ButtonImport, 0, wxALIGN_LEFT|wxALL|wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5); 
    m_ButtonImport->Hide();

    m_ButtonExport = new wxButton(m_ContainerPanel, CEDITOBJVIEWDLG_BUTTON_EXPORT, _("&Export"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer3->Add(m_ButtonExport, 0, wxALIGN_LEFT|wxALL|wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5); 
    m_ButtonExport->Hide();

    wxStdDialogButtonSizer* itemStdDialogButtonSizer5 = new wxStdDialogButtonSizer;
    itemBoxSizer3->Add(itemStdDialogButtonSizer5, 1, wxALL, 5);

    wxButton* itemButton6 = new wxButton(m_ContainerPanel, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0);
    itemStdDialogButtonSizer5->AddButton(itemButton6);

    wxButton* itemButton7 = new wxButton(m_ContainerPanel, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0);
    itemStdDialogButtonSizer5->AddButton(itemButton7);

    m_ButtonHelp = new wxHyperlinkCtrl( m_ContainerPanel, wxID_HELP, _("Help"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    m_ButtonHelp->SetForegroundColour(wxColour(192, 192, 192));
    itemStdDialogButtonSizer5->Add(m_ButtonHelp, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    m_ButtonHelp->Hide();

    itemStdDialogButtonSizer5->Realize();

////@end CEditObjViewDlg content construction
}

void CEditObjViewDlg::SetHelpUrl(const wxString &url)
{
    m_ButtonHelp->SetURL(url);
    m_ButtonHelp->Show(!url.IsEmpty());
}


/*!
 * Should we show tooltips?
 */

bool CEditObjViewDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CEditObjViewDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CEditObjViewDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CEditObjViewDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CEditObjViewDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CEditObjViewDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CEditObjViewDlg icon retrieval
}


void CEditObjViewDlg::OnCancelClick( wxCommandEvent& event )
{
    Close();
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void CEditObjViewDlg::OnOkClick( wxCommandEvent& event )
{
    wxWindowList &children = m_EditorWindow->GetChildren();
    for (wxWindowList::iterator child = children.begin(); child != children.end(); ++child)
    {
        CPublicationTypePanel* win = dynamic_cast<CPublicationTypePanel*>(*child);
        if (win)
        {
            win->SetFinalValidation();
            break;
        }
    }

    if (!m_EditorWindow->TransferDataFromWindow())
        return;
    CIRef<IEditCommand> cmd(m_Editor->GetEditCommand());
    if (!m_Editor->CanClose())
        return;

    if (cmd) m_CmdProccessor->Execute(cmd);
    Destroy();
}



void CEditObjViewDlg::SetEditorWindow(wxWindow* editorWindow)
{
    editorWindow->Reparent(m_ContainerPanel);
    m_EditWindowSizer->Add(editorWindow, 1, wxGROW|wxALL, 5);    
    GetSizer()->SetSizeHints(this);
    Centre(wxBOTH|wxCENTRE_ON_SCREEN);

    m_EditorWindow = editorWindow;
    m_EditorWindow->SetFocus();

    CImportExportAccess* import_export = GetImportExportWindow();
    if (!import_export)
        return;   
    EnableImport(import_export->IsImportEnabled());
    EnableExport(import_export->IsExportEnabled());    
}


void CEditObjViewDlg::OnExport( wxCommandEvent& event )
{
    CRef<CSerialObject> so;
    CImportExportAccess* import_export = GetImportExportWindow();
    if (import_export)
        so = import_export->OnExport();
    if (so)
    {
        wxFileDialog asn_save_file(this, wxT("Export object to file"), m_WorkDir, wxEmptyString,
                                   CFileExtensions::GetDialogFilter(CFileExtensions::kASN) + wxT("|") +
                                   CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles),
                                   wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    
        if (asn_save_file.ShowModal() == wxID_OK) 
        {
            wxString path = asn_save_file.GetPath();
            if( !path.IsEmpty())
            {
                ios::openmode mode = ios::out;
                CNcbiOfstream os(path.fn_str(), mode); 
                os << MSerial_AsnText;
                os << *so;        
            }
        }
    }
    else
    {
        wxMessageBox(wxT("No export function found for object of this type"), wxT("Error"), wxOK|wxICON_ERROR);
    }
}


void CEditObjViewDlg::OnImport( wxCommandEvent& event )
{
    CImportExportAccess* import_export = GetImportExportWindow();
    if (!import_export)
        return;
    wxFileDialog asn_open_file(this, wxT("Import object from file"), wxEmptyString, wxEmptyString,
                               CFileExtensions::GetDialogFilter(CFileExtensions::kASN) + wxT("|") +
                               CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles),
                               wxFD_OPEN|wxFD_FILE_MUST_EXIST);
  
    if (asn_open_file.ShowModal() == wxID_OK) 
    {
        wxString path = asn_open_file.GetPath();
        if( !path.IsEmpty() )
        {
            CNcbiIfstream istr( path.fn_str() );
            try
            {
                import_export->OnImport(istr);
            }
            catch(const CException&)
            {
                wxMessageBox(wxT("Need Seq-feat object for import"), wxT("Error"), wxOK|wxICON_ERROR);
                return;
            }  
            catch (const exception&)             
            {
                wxMessageBox(wxT("Need Seq-feat object for import"), wxT("Error"), wxOK|wxICON_ERROR);
                return;
            }  
        }
    }
}

void CEditObjViewDlg::EnableImport(bool enable)
{
    if (enable)
        m_ButtonImport->Show();
    else
        m_ButtonImport->Hide();
    Refresh();
}

void CEditObjViewDlg::EnableExport(bool enable)
{
    if (enable)
        m_ButtonExport->Show();
    else
        m_ButtonExport->Hide();
    Refresh();
}

CImportExportAccess*  CEditObjViewDlg::GetImportExportWindow()
{
    CImportExportAccess* win = dynamic_cast<CImportExportAccess*>(m_EditorWindow);
    if (!win)
    {
        wxTreebook *book = dynamic_cast<wxTreebook*>(m_EditorWindow);
        if (book)
        {
            win = dynamic_cast<CImportExportAccess*>(book->GetCurrentPage());
            return win;
        }
    }
    if (!win)
    {
        wxWindowList &children = m_EditorWindow->GetChildren();
        for (wxWindowList::iterator child = children.begin(); child != children.end(); ++child)
        {
            win = dynamic_cast<CImportExportAccess*>(*child);
            if (win)
                break;
        }
    }
    return win;
}



/*!
 * CEditObjViewDlgModal type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CEditObjViewDlgModal, CGuiWidgetDlg )


/*!
 * CEditObjViewDlgModal event table definition
 */

BEGIN_EVENT_TABLE( CEditObjViewDlgModal, CGuiWidgetDlg )

////@begin CEditObjViewDlgModal event table entries
    EVT_BUTTON( wxID_OK, CEditObjViewDlgModal::OnOkClick )
    EVT_CLOSE( CEditObjViewDlgModal::OnClose )   
    EVT_BUTTON( CEDITOBJVIEWDLG_BUTTON_EXPORT, CEditObjViewDlgModal::OnExport )
    EVT_BUTTON( CEDITOBJVIEWDLG_BUTTON_IMPORT, CEditObjViewDlgModal::OnImport )
////@end CEditObjViewDlgModal event table entries

END_EVENT_TABLE()


/*!
 * CEditObjViewDlgModal constructors
 */

CEditObjViewDlgModal::CEditObjViewDlgModal()
{
    Init();
}

CEditObjViewDlgModal::CEditObjViewDlgModal( wxWindow* parent, bool create, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_Create(create)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CEditObjViewDlgModal creator
 */

bool CEditObjViewDlgModal::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CEditObjViewDlgModal creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CGuiWidgetDlg::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CEditObjViewDlgModal creation
    return true;
}


/*!
 * CEditObjViewDlgModal destructor
 */

CEditObjViewDlgModal::~CEditObjViewDlgModal()
{
////@begin CEditObjViewDlgModal destruction
////@end CEditObjViewDlgModal destruction
}


/*!
 * Member initialisation
 */

void CEditObjViewDlgModal::Init()
{
////@begin CEditObjViewDlgModal member initialisation
    m_EditWindowSizer = NULL;
////@end CEditObjViewDlgModal member initialisation
    m_Editor = NULL;
    m_EditorWindow = NULL;
    m_ButtonImport = NULL;
    m_ButtonExport = NULL;
    m_ButtonHelp = NULL;
}


/*!
 * Control creation for CEditObjViewDlgModal
 */

void CEditObjViewDlgModal::CreateControls()
{    
////@begin CEditObjViewDlgModal content construction
    CEditObjViewDlgModal* itemCGuiWidgetDlg1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCGuiWidgetDlg1->SetSizer(itemBoxSizer2);

    m_EditWindowSizer = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(m_EditWindowSizer, 1, wxGROW|wxALL, 5);

    wxStaticLine* itemStaticLine4 = new wxStaticLine( itemCGuiWidgetDlg1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer2->Add(itemStaticLine4, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxGROW|wxALL, 5);

    m_ButtonImport = new wxButton( itemCGuiWidgetDlg1, CEDITOBJVIEWDLG_BUTTON_IMPORT, _("&Import"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_ButtonImport, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5); 
    m_ButtonImport->Hide();

    m_ButtonExport = new wxButton( itemCGuiWidgetDlg1, CEDITOBJVIEWDLG_BUTTON_EXPORT, _("&Export"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_ButtonExport, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5); 
    m_ButtonExport->Hide();

    wxStdDialogButtonSizer* itemStdDialogButtonSizer5 = new wxStdDialogButtonSizer;
    itemBoxSizer3->Add(itemStdDialogButtonSizer5, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);    

    wxButton* itemButton6 = new wxButton( itemCGuiWidgetDlg1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer5->AddButton(itemButton6);

    wxButton* itemButton7 = new wxButton( itemCGuiWidgetDlg1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer5->AddButton(itemButton7);

    m_ButtonHelp = new wxHyperlinkCtrl( itemCGuiWidgetDlg1, wxID_HELP, _("Help"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    m_ButtonHelp->SetForegroundColour(wxColour(192, 192, 192));
    itemStdDialogButtonSizer5->Add(m_ButtonHelp, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    m_ButtonHelp->Hide();

    itemStdDialogButtonSizer5->Realize();

////@end CEditObjViewDlgModal content construction
}

void CEditObjViewDlgModal::SetHelpUrl(const wxString &url)
{
    m_ButtonHelp->SetURL(url);
    m_ButtonHelp->Show(!url.IsEmpty());
}

/*!
 * Should we show tooltips?
 */

bool CEditObjViewDlgModal::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CEditObjViewDlgModal::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CEditObjViewDlgModal bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CEditObjViewDlgModal bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CEditObjViewDlgModal::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CEditObjViewDlgModal icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CEditObjViewDlgModal icon retrieval
}

bool CEditObjViewDlgModal::Show(bool show)
{
    if (m_WidgetHost)
        return CGuiWidgetDlg::Show(show);

    return wxDialog::Show(show);
}

void CEditObjViewDlgModal::OnClose(wxCloseEvent& event)
{
    if (IsModal())
    {
        EndModal(wxID_CANCEL);
    }
    else
    {
        CGuiWidgetDlg::OnClose(event);
    }
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void CEditObjViewDlgModal::OnOkClick( wxCommandEvent& event )
{
    wxWindowList &children = m_EditorWindow->GetChildren();
    for (wxWindowList::iterator child = children.begin(); child != children.end(); ++child)
    {
        CPublicationTypePanel* win = dynamic_cast<CPublicationTypePanel*>(*child);
        if (win)
        {
            win->SetFinalValidation();
            break;
        }
    }

    if (!m_EditorWindow->TransferDataFromWindow())
        return;
    CIRef<IEditCommand> cmd(m_Editor->GetEditCommand());
    if (!m_Editor->CanClose())
        return;

    if (IsModal())
    {
        EndModal(wxID_OK);
    }
    else if (m_CmdProccessor)
    {
        if (cmd) m_CmdProccessor->Execute(cmd);
        Destroy();
    }
}


wxWindow* CEditObjViewDlgModal::CreateDialogNew(wxWindow* parent)
{
    return new CEditObjViewDlgModal(parent, true);
}


wxWindow* CEditObjViewDlgModal::CreateDialogEdit(wxWindow* parent)
{
    return new CEditObjViewDlgModal(parent, false);
}


void CEditObjViewDlgModal::SetEditorWindow(wxWindow* editorWindow)
{
    editorWindow->Reparent(this);
    m_EditWindowSizer->Add(editorWindow, 1, wxGROW|wxALL, 5);
    
    //Fit();
    GetSizer()->SetSizeHints(this);
    Centre(wxBOTH|wxCENTRE_ON_SCREEN);

    m_EditorWindow = editorWindow;
    m_EditorWindow->SetFocus();

    CImportExportAccess* import_export = GetImportExportWindow();
    if (!import_export)
        return;   
    EnableImport(import_export->IsImportEnabled());
    EnableExport(import_export->IsExportEnabled());    
}

/// IGuiWidgetHost implementation

void CEditObjViewDlgModal::SetHost(IGuiWidgetHost* host)
{
    CGuiWidgetDlg::SetHost(host);
}


bool CEditObjViewDlgModal::InitWidget(TConstScopedObjects& objects)
{
    m_Orig_Objects = objects;

    CSeq_entry_Handle seh = GetTopSeqEntryFromScopedObject(m_Orig_Objects.front());
    if (!seh) {
        // probably doesn't exist yet, just get the first top level seq entry from the scope
        CScope::TTSE_Handles tses;
        m_Orig_Objects.front().scope->GetAllTSEs(tses, CScope::eAllTSEs);
        ITERATE (CScope::TTSE_Handles, handle, tses) {
            CSeq_entry_CI entry_ci(*handle, CSeq_entry_CI::fRecursive | CSeq_entry_CI::fIncludeGivenEntry);
            if (entry_ci) {
                seh = *entry_ci;
            }
        }
    }        

   
    m_Editor = CreateEditorForObject(m_Orig_Objects.front().object, seh, m_Create);
    if (m_Editor) {
        m_EditorWindow = m_Editor->CreateWindow(this);
        m_EditorWindow->TransferDataToWindow();
        SetEditorWindow(m_EditorWindow);
//        m_EditorWindow->GetParent()->CenterOnParent();
//        m_EditorWindow->GetParent()->Center(wxBOTH|wxCENTRE_ON_SCREEN);
        return true;
    } else {
        return false;
    }
}


const CObject* CEditObjViewDlgModal::GetOrigObject() const
{
    return m_Orig_Objects.front().object.GetPointer();
}


void CEditObjViewDlgModal :: GetSelectedObjects(TConstObjects& objects) const
{
   // GetSelectedObjects of your dialog should return objects selected 
   // in your view/dialog.
   // For example, you have a list control (of objects?) in your dialog.
   // You get selection from the list and convert it to objects.
   // Then you return these objects from GetSelectedObjects.
   //

}

void CEditObjViewDlgModal::SetSelectedObjects(const TConstObjects& objects)
{
   // Synchronization of selected objects between views works with 
   // function SetSelectedObjects.
   // When selection in the active view changes Gbench calls 
   // SetSelectedObjects for all other views.
   // In your SetSelectedObjects  you can check if your view/dialog shows 
   // some of these objects and if yes, select them in your view.
   //
   // nothing needed at this moment

}


void CEditObjViewDlgModal::DataChanging()
{
}

void CEditObjViewDlgModal::DataChanged()
{
}

void CEditObjViewDlgModal::GetActiveObjects(vector<TConstScopedObjects>& objects)
{
}

void CEditObjViewDlgModal::OnExport( wxCommandEvent& event )
{
    CRef<CSerialObject> so;
    CImportExportAccess* import_export = GetImportExportWindow();
    if (import_export)
        so = import_export->OnExport();
    if (so)
    {
        wxFileDialog asn_save_file(this, wxT("Export object to file"), wxEmptyString, wxEmptyString,
                                   CFileExtensions::GetDialogFilter(CFileExtensions::kASN) + wxT("|") +
                                   CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles),
                                   wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    
        if (asn_save_file.ShowModal() == wxID_OK) 
        {
            wxString path = asn_save_file.GetPath();
            if( !path.IsEmpty())
            {
                ios::openmode mode = ios::out;
                CNcbiOfstream os(path.fn_str(), mode); 
                os << MSerial_AsnText;
                os << *so;        
            }
        }
    }
    else
    {
        wxMessageBox(wxT("No export function found for object of this type"), wxT("Error"), wxOK|wxICON_ERROR);
    }
}


void CEditObjViewDlgModal::OnImport( wxCommandEvent& event )
{
    CImportExportAccess* import_export = GetImportExportWindow();
    if (!import_export)
        return;
    wxFileDialog asn_open_file(this, wxT("Import object from file"), wxEmptyString, wxEmptyString,
                               CFileExtensions::GetDialogFilter(CFileExtensions::kASN) + wxT("|") +
                               CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles),
                               wxFD_OPEN|wxFD_FILE_MUST_EXIST);
  
    if (asn_open_file.ShowModal() == wxID_OK) 
    {
        wxString path = asn_open_file.GetPath();
        if( !path.IsEmpty() )
        {
            CNcbiIfstream istr( path.fn_str() );
            try
            {
                import_export->OnImport(istr);
            }
            catch(const CException&)
            {
                wxMessageBox(wxT("Unable to import object"), wxT("Error"), wxOK|wxICON_ERROR);
                return;
            }               
            catch (const exception&) 
            {
                wxMessageBox(wxT("Unable to import object"), wxT("Error"), wxOK|wxICON_ERROR);
                return;
            }        
        }
    }
}

void CEditObjViewDlgModal::EnableImport(bool enable)
{
    if (enable)
        m_ButtonImport->Show();
    else
        m_ButtonImport->Hide();
    Refresh();
}

void CEditObjViewDlgModal::EnableExport(bool enable)
{
    if (enable)
        m_ButtonExport->Show();
    else
        m_ButtonExport->Hide();
    Refresh();
}

CImportExportAccess*  CEditObjViewDlgModal::GetImportExportWindow()
{
    CImportExportAccess* win = dynamic_cast<CImportExportAccess*>(m_EditorWindow);
    if (!win)
    {
        wxTreebook *book = dynamic_cast<wxTreebook*>(m_EditorWindow);
        if (book)
        {
            win = dynamic_cast<CImportExportAccess*>(book->GetCurrentPage());
            return win;
        }
    }
    if (!win)
    {
        wxWindowList &children = m_EditorWindow->GetChildren();
        for (wxWindowList::iterator child = children.begin(); child != children.end(); ++child)
        {
            win = dynamic_cast<CImportExportAccess*>(*child);
            if (win)
                break;
        }
    }
    return win;
}

END_NCBI_SCOPE
