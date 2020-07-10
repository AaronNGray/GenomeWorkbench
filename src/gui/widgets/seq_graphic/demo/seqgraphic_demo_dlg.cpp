/////////////////////////////////////////////////////////////////////////////
// Name:        seqgraphic_demo_dlg.cpp
// Purpose:     
// Author:      Vlad Lebedev
// Modified by: 
// Created:     Fri 15 Feb 14:23:52 2008
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////


////@begin includes
////@end includes

#include <ncbi_pch.hpp>
#include <corelib/ncbistd.hpp>
#include <corelib/ncbitime.hpp>

#include "seqgraphic_demo_dlg.hpp"

#include <gui/widgets/seq_graphic/seqgraphic_widget.hpp>

#include <objects/seq/Seq_annot.hpp>

#include <objtools/data_loaders/genbank/gbloader.hpp>

#include <serial/objistr.hpp>
#include <serial/serial.hpp>

#include <gui/widgets/wx/sys_path.hpp>

// Register commands
#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/commands.hpp>

#include <wx/msgdlg.h>
#include <wx/statline.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/bitmap.h>
#include <wx/icon.h>

////@begin XPM images
////@end XPM images


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

IMPLEMENT_DYNAMIC_CLASS( CSeqGraphicDemoDlg, wxDialog )
BEGIN_EVENT_TABLE( CSeqGraphicDemoDlg, wxDialog )

////@begin CSeqGraphicDemoDlg event table entries
    EVT_CLOSE( CSeqGraphicDemoDlg::OnCloseWindow )

    EVT_BUTTON( ID_ACC, CSeqGraphicDemoDlg::OnAccClick )

    EVT_BUTTON( ID_FILE, CSeqGraphicDemoDlg::OnFileClick )

    EVT_CHECKBOX( ID_CHECKBOX1, CSeqGraphicDemoDlg::OnFlipStrand )

    EVT_BUTTON( wxID_CLOSE, CSeqGraphicDemoDlg::OnCloseClick )

////@end CSeqGraphicDemoDlg event table entries

END_EVENT_TABLE()


CSeqGraphicDemoDlg::CSeqGraphicDemoDlg()
{
    Init();
}


CSeqGraphicDemoDlg::CSeqGraphicDemoDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


bool CSeqGraphicDemoDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSeqGraphicDemoDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    Centre();
////@end CSeqGraphicDemoDlg creation
    return true;
}


CSeqGraphicDemoDlg::~CSeqGraphicDemoDlg()
{
////@begin CSeqGraphicDemoDlg destruction
////@end CSeqGraphicDemoDlg destruction
}


void CSeqGraphicDemoDlg::Init()
{
////@begin CSeqGraphicDemoDlg member initialisation
    m_SeqWidget = NULL;
    m_AccInput = NULL;
    m_FileInput = NULL;
    m_FlipStrand = NULL;
////@end CSeqGraphicDemoDlg member initialisation
    
    if ( !m_ObjMgr ) {
        m_ObjMgr = CObjectManager::GetInstance();
        CGBDataLoader::RegisterInObjectManager(*m_ObjMgr);

        m_Scope.Reset(new CScope(*m_ObjMgr));
        m_Scope->AddDefaults();
    }

    wxFileArtProvider* provider = GetDefaultFileArtProvider();
    CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();
    WidgetsWx_RegisterCommands(cmd_reg, *provider);
    CSeqGraphicWidget::RegisterCommands(cmd_reg, *provider);
}


void CSeqGraphicDemoDlg::CreateControls()
{    
////@begin CSeqGraphicDemoDlg content construction
    CSeqGraphicDemoDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    m_SeqWidget = new CSeqGraphicWidget( itemDialog1, ID_WIDGET, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(m_SeqWidget, 1, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxGROW|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer4->Add(itemFlexGridSizer5, 1, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_AccInput = new wxTextCtrl( itemDialog1, ID_ACC_TEXT, _("s"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AccInput->SetMaxLength(20);
    m_AccInput->SetHelpText(_("Enter Accession"));
    if (CSeqGraphicDemoDlg::ShowToolTips())
        m_AccInput->SetToolTip(_("Enter Accession"));
    itemFlexGridSizer5->Add(m_AccInput, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton7 = new wxButton( itemDialog1, ID_ACC, _("Load Accession"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer5->Add(itemButton7, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_FileInput = new wxTextCtrl( itemDialog1, ID_FILE_TEXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_FileInput->SetMaxLength(256);
    m_FileInput->SetHelpText(_("Enter asn file name"));
    if (CSeqGraphicDemoDlg::ShowToolTips())
        m_FileInput->SetToolTip(_("Enter asn file name"));
    itemFlexGridSizer5->Add(m_FileInput, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton9 = new wxButton( itemDialog1, ID_FILE, _("Load File"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer5->Add(itemButton9, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer5->AddGrowableCol(0);

    wxStaticLine* itemStaticLine10 = new wxStaticLine( itemDialog1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL );
    itemBoxSizer4->Add(itemStaticLine10, 0, wxGROW|wxALL, 5);

    m_FlipStrand = new wxCheckBox( itemDialog1, ID_CHECKBOX1, _("Flip Strand"), wxDefaultPosition, wxDefaultSize, 0 );
    m_FlipStrand->SetValue(false);
    itemBoxSizer4->Add(m_FlipStrand, 0, wxALIGN_TOP|wxALL, 5);

    wxButton* itemButton12 = new wxButton( itemDialog1, wxID_CLOSE, _("&Close"), wxDefaultPosition, wxDefaultSize, 0 );
    if (CSeqGraphicDemoDlg::ShowToolTips())
        itemButton12->SetToolTip(_("Close"));
    itemBoxSizer4->Add(itemButton12, 0, wxALIGN_BOTTOM|wxALL, 5);

////@end CSeqGraphicDemoDlg content construction
    m_SeqWidget->Create();
    m_AccInput->SetValue(wxT("nt_011515"));
}


void CSeqGraphicDemoDlg::OnCloseWindow( wxCloseEvent& event )
{
    Destroy();
}


void CSeqGraphicDemoDlg::OnAccClick( wxCommandEvent& event )
{
    m_SeqId.Reset(new CSeq_id());
    
    string acc = ToStdString(m_AccInput->GetValue());
    try {
        m_SeqId->Set(acc);
    }
    catch (CSeqIdException&) {
        string msg("Accession '");
        msg += acc + " not recognized as a valid accession";
        wxMessageBox(ToWxString(msg), _("Cannot load accesion"),
                     wxOK | wxICON_ERROR);
        return;
    }

    CGuiRegistry& reg = CGuiRegistry::GetInstance();
    //string file_path = "etc/plugin_config.asn";
    wxString file_path = CSysPath::ResolvePath(wxT("<std>/etc/plugin_config.asn"));
    //m_ThemeFile->value(file_path.c_str());
    /// set the global repository
    {{
        CNcbiIfstream istr(file_path.fn_str());
        if (istr) {
            reg.SetLocal(istr);
        }
    }}
    
    try {
        m_SeqWidget->UpdateConfig();
        LOG_POST(Info <<
            "CViewGraphical::InitView() - finish with loading configuration!");
    } catch (CException&) {
        wxMessageBox(_("Something is wrong with configuration!"),
            _("Failed to load configuration for Graphical view"),
            wxOK | wxICON_EXCLAMATION);
        return;
    }

    try {
        // retrieve our sequence
    
        SConstScopedObject obj(*m_SeqId, *m_Scope);
        m_SeqWidget->SetInputObject(obj);
        string id_desc;
    } catch (CException&) {
        string msg("Can't find sequence for accession '");
        msg += acc + "'";
        wxMessageBox(ToWxString(msg), _("Cannot load accesion"),
                     wxOK | wxICON_ERROR);
        return;
    }
}


void CSeqGraphicDemoDlg::OnFileClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_FILE in CSeqGraphicDemoDlg.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_FILE in CSeqGraphicDemoDlg. 
}


void CSeqGraphicDemoDlg::OnFlipStrand( wxCommandEvent& event )
{
    m_SeqWidget->SetHorizontal(true, m_FlipStrand->IsChecked());
}


void CSeqGraphicDemoDlg::OnCloseClick( wxCommandEvent& event )
{
    Close();
}


bool CSeqGraphicDemoDlg::ShowToolTips()
{
    return true;
}


wxBitmap CSeqGraphicDemoDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSeqGraphicDemoDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSeqGraphicDemoDlg bitmap retrieval
}


wxIcon CSeqGraphicDemoDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSeqGraphicDemoDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSeqGraphicDemoDlg icon retrieval
}

END_NCBI_SCOPE
