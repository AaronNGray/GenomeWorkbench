/*  $Id: seq_text_demo_dlg.cpp 38348 2017-04-27 14:54:37Z ivanov $
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
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbistd.hpp>
#include <corelib/ncbitime.hpp>

#include "seq_text_demo_dlg.hpp"

#include <gui/widgets/wx/sys_path.hpp>
#include <gui/widgets/seq_text/seq_text_widget.hpp>

// Register commands
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/commands.hpp>

#include <objects/seq/Seq_annot.hpp>

#include <objtools/data_loaders/genbank/gbloader.hpp>

#include <serial/objistr.hpp>
#include <serial/serial.hpp>


#include <wx/msgdlg.h>
#include <wx/statline.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/bitmap.h>
#include <wx/icon.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

IMPLEMENT_DYNAMIC_CLASS( CSeqTextDemoDlg, wxDialog )

BEGIN_EVENT_TABLE( CSeqTextDemoDlg, wxDialog )

////@begin CSeqTextDemoDlg event table entries
    EVT_CLOSE( CSeqTextDemoDlg::OnCloseWindow )
    EVT_BUTTON( ID_ACC, CSeqTextDemoDlg::OnAccClick )
    EVT_BUTTON( ID_FILE, CSeqTextDemoDlg::OnFileClick )
    EVT_BUTTON( wxID_CLOSE, CSeqTextDemoDlg::OnCloseClick )
////@end CSeqTextDemoDlg event table entries
END_EVENT_TABLE()


CSeqTextDemoDlg::CSeqTextDemoDlg()
{
    Init();
}


CSeqTextDemoDlg::CSeqTextDemoDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


bool CSeqTextDemoDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSeqTextDemoDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    Centre();
////@end CSeqTextDemoDlg creation
    return true;
}


CSeqTextDemoDlg::~CSeqTextDemoDlg()
{
////@begin CSeqTextDemoDlg destruction
////@end CSeqTextDemoDlg destruction
}


void CSeqTextDemoDlg::Init()
{
////@begin CSeqTextDemoDlg member initialisation
    m_SeqTextWidget = NULL;
    m_AccInput = NULL;
    m_FileInput = NULL;
////@end CSeqTextDemoDlg member initialisation

    if ( !m_ObjMgr ) {
        m_ObjMgr = CObjectManager::GetInstance();
        CGBDataLoader::RegisterInObjectManager(*m_ObjMgr);

        m_Scope.Reset(new CScope(*m_ObjMgr));
        m_Scope->AddDefaults();
    }

    // configure our GBench registry
    {{
         const CNcbiRegistry& reg = CNcbiApplication::Instance()->GetConfig();

         string gbench_cfg = reg.GetString("filesystem",
                                           "GBenchRegistry",
                                           "plugin_config.asn");
         CNcbiIfstream istr(gbench_cfg.c_str());
         if (istr) {
             CGuiRegistry::GetInstance().AddSite(istr, 0);
         }
    }}

    wxFileArtProvider* provider = GetDefaultFileArtProvider();
    CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();
    WidgetsWx_RegisterCommands(cmd_reg, *provider);
}


void CSeqTextDemoDlg::CreateControls()
{
////@begin CSeqTextDemoDlg content construction
    CSeqTextDemoDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    m_SeqTextWidget = new CSeqTextWidget( itemDialog1, ID_WIDGET, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(m_SeqTextWidget, 1, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxGROW|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer5 = new wxFlexGridSizer(2, 2, 0, 0);
    itemFlexGridSizer5->AddGrowableCol(0);
    itemBoxSizer4->Add(itemFlexGridSizer5, 1, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_AccInput = new wxTextCtrl( itemDialog1, ID_ACC_TEXT, _(""), wxDefaultPosition, wxDefaultSize, 0 );
    m_AccInput->SetMaxLength(20);
    m_AccInput->SetHelpText(_("Enter Accession"));
    if (CSeqTextDemoDlg::ShowToolTips())
        m_AccInput->SetToolTip(_("Enter Accession"));
    itemFlexGridSizer5->Add(m_AccInput, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton7 = new wxButton( itemDialog1, ID_ACC, _("Load Accession"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer5->Add(itemButton7, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_FileInput = new wxTextCtrl( itemDialog1, ID_FILE_TEXT, _(""), wxDefaultPosition, wxDefaultSize, 0 );
    m_FileInput->SetMaxLength(256);
    m_FileInput->SetHelpText(_("Enter asn file name"));
    if (CSeqTextDemoDlg::ShowToolTips())
        m_FileInput->SetToolTip(_("Enter asn file name"));
    itemFlexGridSizer5->Add(m_FileInput, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton9 = new wxButton( itemDialog1, ID_FILE, _("Load File"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer5->Add(itemButton9, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticLine* itemStaticLine10 = new wxStaticLine( itemDialog1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL );
    itemBoxSizer4->Add(itemStaticLine10, 0, wxGROW|wxALL, 5);

    wxButton* itemButton11 = new wxButton( itemDialog1, wxID_CLOSE, _("&Close"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton11, 0, wxALIGN_BOTTOM|wxALL, 5);

////@end CSeqTextDemoDlg content construction

    wxString path = CSysPath::ResolvePathExisting(wxT("<std>/gbench/etc/plugin_config.asn"));
    if (!path.empty()) {
        CNcbiIfstream istr(path.fn_str());
        if (istr)
             CGuiRegistry::GetInstance().AddSite(istr, 0);
    }

    m_SeqTextWidget->Create();
}


bool CSeqTextDemoDlg::ShowToolTips()
{
    return true;
}


wxBitmap CSeqTextDemoDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSeqTextDemoDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSeqTextDemoDlg bitmap retrieval
}


wxIcon CSeqTextDemoDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSeqTextDemoDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSeqTextDemoDlg icon retrieval
}



void CSeqTextDemoDlg::OnCloseWindow(wxCloseEvent& /*event*/)
{
    Destroy();
}



void CSeqTextDemoDlg::OnCloseClick( wxCommandEvent& /*event*/ )
{
    Close();
}



void CSeqTextDemoDlg::OnAccClick( wxCommandEvent& /*event*/ )
{
    CSeq_id id;

    string acc = ToStdString(m_AccInput->GetValue());
    try {
        id.Set(acc);
    }
    catch (CSeqIdException&) {
        wxString msg(_("Accession '"));
        msg += ToWxString(acc) + _(" not recognized as a valid accession");
        wxMessageBox(msg, _("Cannot load accesion"),
                     wxOK | wxICON_ERROR);
        return;
    }

    // retrieve our sequence
    CBioseq_Handle handle = m_Scope->GetBioseqHandle(id);
    if(handle) {
        m_DataSource.Reset(new CSeqTextDataSource(handle, *m_Scope));
        m_SeqTextWidget->SetDataSource (*m_DataSource);

    } else {
        wxString msg = _("Can't find sequence for accession '");
        msg += ToWxString(acc) + _("'");
        wxMessageBox(msg, _("Cannot load accesion"),
                     wxOK | wxICON_ERROR);
        return;
    }
}


void CSeqTextDemoDlg::OnFileClick( wxCommandEvent& /*event*/ )
{
    wxString filename = m_FileInput->GetValue();
    CNcbiIfstream istr(filename.fn_str());
    auto_ptr<CObjectIStream> is(CObjectIStream::Open(eSerial_AsnText, istr));

    CRef<CSeq_entry> sep(new CSeq_entry());
    *is >> *sep;

    if ( !m_ObjMgr ) {
        m_ObjMgr = CObjectManager::GetInstance();
        CGBDataLoader::RegisterInObjectManager(*m_ObjMgr);

        m_Scope.Reset(new CScope(*m_ObjMgr));
        m_Scope->AddDefaults();
    }

    m_DataSource.Reset(new CSeqTextDataSource(*sep, *m_Scope));

    m_SeqTextWidget->SetDataSource (*m_DataSource);

}


END_NCBI_SCOPE
