/*  $Id: align_multiple_demo_dlg.cpp 36426 2016-09-22 17:12:22Z shkeda $
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


#include "align_multiple_demo_dlg.hpp"

#include <gui/widgets/aln_multiple/wx_build_options_dlg.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/wx/commands.hpp>

#include <gui/widgets/wx/ui_tool_registry.hpp>

#include <objects/seq/Seq_annot.hpp>

#include <objtools/data_loaders/genbank/gbloader.hpp>

#include <serial/objistr.hpp>
#include <serial/serial.hpp>


#include <wx/sizer.h>
#include <wx/msgdlg.h>
#include <wx/statline.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/bitmap.h>
#include <wx/icon.h>



BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


IMPLEMENT_DYNAMIC_CLASS( CAlignMultipleDemoDlg, wxDialog )


BEGIN_EVENT_TABLE( CAlignMultipleDemoDlg, wxDialog )
////@begin CAlignMultipleDemoDlg event table entries
    EVT_CLOSE( CAlignMultipleDemoDlg::OnCloseWindow )

    EVT_BUTTON( ID_ACC, CAlignMultipleDemoDlg::OnAccClick )

    EVT_BUTTON( ID_FILE, CAlignMultipleDemoDlg::OnFileClick )

    EVT_BUTTON( wxID_CLOSE, CAlignMultipleDemoDlg::OnCloseClick )

////@end CAlignMultipleDemoDlg event table entries
END_EVENT_TABLE()


CAlignMultipleDemoDlg::CAlignMultipleDemoDlg()
{
    Init();
}


CAlignMultipleDemoDlg::CAlignMultipleDemoDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


bool CAlignMultipleDemoDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAlignMultipleDemoDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CAlignMultipleDemoDlg creation
    return true;
}


CAlignMultipleDemoDlg::~CAlignMultipleDemoDlg()
{
////@begin CAlignMultipleDemoDlg destruction
////@end CAlignMultipleDemoDlg destruction
}


void CAlignMultipleDemoDlg::Init()
{
////@begin CAlignMultipleDemoDlg member initialisation
    m_AlnWidget = NULL;
    m_AccInput = NULL;
    m_FileInput = NULL;
////@end CAlignMultipleDemoDlg member initialisation

    if ( !m_ObjMgr ) {
        m_ObjMgr = CObjectManager::GetInstance();
        CGBDataLoader::RegisterInObjectManager(*m_ObjMgr);

        m_Scope.Reset(new CScope(*m_ObjMgr));
        m_Scope->AddDefaults();
    }

    /// Create an empty data source
    vector< CConstRef<CSeq_align> > aligns;
    m_Builder.Init(*m_Scope, aligns);
    m_DataSource = m_Builder.CreateDataSource();

}

#define DEF_FILE "E:\\Users\\Yazhuk\\Gbench Data\\blast.asn"
#define DEF_ACC "19568015"


void CAlignMultipleDemoDlg::CreateControls()
{
////@begin CAlignMultipleDemoDlg content construction
    CAlignMultipleDemoDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    m_AlnWidget = new CAlnMultiWidget( itemDialog1, ID_WIDGET, wxDefaultPosition, wxSize(800, 400), wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(m_AlnWidget, 1, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxGROW|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer4->Add(itemFlexGridSizer5, 1, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_AccInput = new wxTextCtrl( itemDialog1, ID_ACC_TEXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_AccInput->SetMaxLength(20);
    m_AccInput->SetHelpText(_("Enter Accession"));
    if (CAlignMultipleDemoDlg::ShowToolTips())
        m_AccInput->SetToolTip(_("Enter Accession"));
    itemFlexGridSizer5->Add(m_AccInput, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton7 = new wxButton( itemDialog1, ID_ACC, _("Load Accession"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer5->Add(itemButton7, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_FileInput = new wxTextCtrl( itemDialog1, ID_FILE_TEXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_FileInput->SetMaxLength(256);
    m_FileInput->SetHelpText(_("Enter asn file name"));
    if (CAlignMultipleDemoDlg::ShowToolTips())
        m_FileInput->SetToolTip(_("Enter asn file name"));
    itemFlexGridSizer5->Add(m_FileInput, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton9 = new wxButton( itemDialog1, ID_FILE, _("Load File"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer5->Add(itemButton9, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer5->AddGrowableCol(0);

    wxStaticLine* itemStaticLine10 = new wxStaticLine( itemDialog1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL );
    itemBoxSizer4->Add(itemStaticLine10, 0, wxGROW|wxALL, 5);

    wxButton* itemButton11 = new wxButton( itemDialog1, wxID_CLOSE, _("&Close"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton11, 0, wxALIGN_BOTTOM|wxALL, 5);

////@end CAlignMultipleDemoDlg content construction

    /// register command
    m_FileInput->SetValue(ToWxString(DEF_FILE));
    m_AccInput->SetValue(ToWxString(DEF_ACC));

    m_AlnWidget->CreateWidget();

    CWidgetDisplayStyle* ws = const_cast<CWidgetDisplayStyle*>(m_AlnWidget->GetDisplayStyle());
    ws->m_TextFont.SetFontFace(CGlTextureFont::eFontFace_Helvetica);
    ws->m_TextFont.SetFontSize(12);
    ws->m_SeqFont.SetFontFace(CGlTextureFont::eFontFace_Fixed);
    ws->m_SeqFont.SetFontSize(8);


    m_StyleCatalog.SetDefaultStyle(new CRowDisplayStyle());
    m_StyleCatalog.SetWidgetStyle(ws);

    m_AlnWidget->SetStyleCatalog(&m_StyleCatalog);

    m_AlnWidget->SetDataSource(m_DataSource, &*m_Scope);

    // register commands
    CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();
    wxFileArtProvider* provider = GetDefaultFileArtProvider();

    WidgetsWx_RegisterCommands(cmd_reg, *provider);
    CAlnMultiWidget::RegisterCommands(cmd_reg, *provider);
}


bool CAlignMultipleDemoDlg::ShowToolTips()
{
    return true;
}


wxBitmap CAlignMultipleDemoDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAlignMultipleDemoDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CAlignMultipleDemoDlg bitmap retrieval
}

wxIcon CAlignMultipleDemoDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAlignMultipleDemoDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CAlignMultipleDemoDlg icon retrieval
}


void CAlignMultipleDemoDlg::OnCloseWindow(wxCloseEvent& event)
{
    Destroy();
}


void CAlignMultipleDemoDlg::OnCloseClick( wxCommandEvent& event )
{
    Close();
}


void CAlignMultipleDemoDlg::OnAccClick( wxCommandEvent& event )
{
    CSeq_id id;

    string acc = ToStdString(m_AccInput->GetValue());
    try {
        id.Set(acc);
    }
    catch (CSeqIdException&) {
        string msg("Accession '");
        msg += acc + " not recognized as a valid accession";
        wxMessageBox(ToWxString(msg), wxT("Cannot load accesion"),
                     wxOK | wxICON_ERROR);
        return;
    }

    // retrieve our sequence
    CBioseq_Handle handle = m_Scope->GetBioseqHandle(id);
    if(handle) {
        m_Builder.Init(*m_Scope, handle);
        OnRebuild();
    } else {
        string msg("Can't find sequence for accession '");
        msg += acc + "'";
        wxMessageBox(ToWxString(msg), wxT("Cannot load accesion"),
                     wxOK | wxICON_ERROR);
        return;
    }
}


typedef CAlnMultiDSBuilder::TAlignVector TAlignVector;


/// loads a serializable object of type T
template <class T>
    bool    LoadObject(const wxString& filename, CScope& scope, CAlnMultiDSBuilder& builder)
{
    try {
        CNcbiIfstream istr(filename.fn_str());
        auto_ptr<CObjectIStream> os(CObjectIStream::Open(eSerial_AsnText, istr));

        CRef<T> obj(new T());
        *os >> *obj;

        builder.Init(scope, *obj);
        return true;
    } catch(CException& /*e*/)  {
        return false;
    }
}


void CAlignMultipleDemoDlg::OnFileClick( wxCommandEvent& event )
{
    m_AlnWidget->SetDataSource(nullptr, nullptr);

    wxString filename = m_FileInput->GetValue();
    try {
        bool ok = LoadObject<CSeq_align>(filename, *m_Scope, m_Builder);
        if( ! ok)   {
            ok = LoadObject<CSeq_annot>(filename, *m_Scope, m_Builder);
        }
        if( ! ok)   {
            ok = LoadObject<CBioseq>(filename, *m_Scope, m_Builder);
        }
        if(ok)  {
            OnRebuild();
        } else {
            wxMessageBox(wxT("This type of file is not supported"),
                         wxT("Error loading file."),
                         wxOK | wxICON_ERROR);
        }
    } catch(CException& e)  {
        e.ReportAll();
        wxString s = ToWxString(e.GetMsg());
        wxMessageBox(s, wxT("Error loading file."), wxOK | wxICON_ERROR);
    }
}



void CAlignMultipleDemoDlg::OnRebuild()
{
  // reset old data
    m_AlnWidget->SetDataSource(nullptr, nullptr);

    vector<CBioseq_Handle> handles;

    m_Builder.PreCreateDataSource(false);
    m_Builder.GetBioseqHandles(handles);

    CBuildOptionsDlg dlg(this);
    dlg.SetParams(m_Builder.SetOptions(), handles, *m_Scope);

    //if(dlg.ShowModal() == wxID_OK) {
        //CAlnMultiDSBuilder::TOptions new_options;
        //dlg.GetOptions(new_options);
        //m_Builder.SetOptions() = new_options;

        m_DataSource = m_Builder.CreateDataSource();
    //}

    m_AlnWidget->SetDataSource(m_DataSource, &*m_Scope);
    if(m_DataSource)    {
        m_DataSource->SetListener(m_AlnWidget);
    }
}


END_NCBI_SCOPE
