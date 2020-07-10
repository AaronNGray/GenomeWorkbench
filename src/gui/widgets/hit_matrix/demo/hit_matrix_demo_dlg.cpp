/*  $Id: hit_matrix_demo_dlg.cpp 25806 2012-05-11 15:55:59Z katargir $
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

#include "hit_matrix_demo_dlg.hpp"

#include <gui/widgets/hit_matrix/hit_matrix_widget.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

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


IMPLEMENT_DYNAMIC_CLASS( CHitMatrixDemoDlg, wxDialog )

BEGIN_EVENT_TABLE( CHitMatrixDemoDlg, wxDialog )

////@begin CHitMatrixDemoDlg event table entries
    EVT_CLOSE( CHitMatrixDemoDlg::OnCloseWindow )

    EVT_BUTTON( ID_ACC, CHitMatrixDemoDlg::OnAccClick )

    EVT_BUTTON( ID_FILE, CHitMatrixDemoDlg::OnFileClick )

    EVT_BUTTON( wxID_CLOSE, CHitMatrixDemoDlg::OnCloseClick )

////@end CHitMatrixDemoDlg event table entries
END_EVENT_TABLE()


CHitMatrixDemoDlg::CHitMatrixDemoDlg()
{
    Init();
}


CHitMatrixDemoDlg::CHitMatrixDemoDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


bool CHitMatrixDemoDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CHitMatrixDemoDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    Centre();
////@end CHitMatrixDemoDlg creation
    return true;
}


CHitMatrixDemoDlg::~CHitMatrixDemoDlg()
{
////@begin CHitMatrixDemoDlg destruction
////@end CHitMatrixDemoDlg destruction
}


void CHitMatrixDemoDlg::Init()
{
////@begin CHitMatrixDemoDlg member initialisation
    m_MatrixWidget = NULL;
    m_AccInput = NULL;
    m_FileInput = NULL;
////@end CHitMatrixDemoDlg member initialisation

    if ( !m_ObjMgr ) {
        m_ObjMgr = CObjectManager::GetInstance();
        CGBDataLoader::RegisterInObjectManager(*m_ObjMgr);

        m_Scope.Reset(new CScope(*m_ObjMgr));
        m_Scope->AddDefaults();
    }
}

#define DEF_FILE "E:\\Users\\Yazhuk\\Gbench Data\\blast.asn"


void CHitMatrixDemoDlg::CreateControls()
{
////@begin CHitMatrixDemoDlg content construction
    CHitMatrixDemoDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    m_MatrixWidget = new CHitMatrixWidget( itemDialog1, ID_WIDGET, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(m_MatrixWidget, 1, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxGROW|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer4->Add(itemFlexGridSizer5, 1, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_AccInput = new wxTextCtrl( itemDialog1, ID_ACC_TEXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_AccInput->SetMaxLength(20);
    m_AccInput->SetHelpText(_("Enter Accession"));
    if (CHitMatrixDemoDlg::ShowToolTips())
        m_AccInput->SetToolTip(_("Enter Accession"));
    itemFlexGridSizer5->Add(m_AccInput, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton7 = new wxButton( itemDialog1, ID_ACC, _("Load Accession"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer5->Add(itemButton7, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_FileInput = new wxTextCtrl( itemDialog1, ID_FILE_TEXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_FileInput->SetMaxLength(256);
    m_FileInput->SetHelpText(_("Enter asn file name"));
    if (CHitMatrixDemoDlg::ShowToolTips())
        m_FileInput->SetToolTip(_("Enter asn file name"));
    itemFlexGridSizer5->Add(m_FileInput, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton9 = new wxButton( itemDialog1, ID_FILE, _("Load File"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer5->Add(itemButton9, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer5->AddGrowableCol(0);

    wxStaticLine* itemStaticLine10 = new wxStaticLine( itemDialog1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL );
    itemBoxSizer4->Add(itemStaticLine10, 0, wxGROW|wxALL, 5);

    wxButton* itemButton11 = new wxButton( itemDialog1, wxID_CLOSE, _("&Close"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton11, 0, wxALIGN_BOTTOM|wxALL, 5);

////@end CHitMatrixDemoDlg content construction
    m_MatrixWidget->Create();
    m_FileInput->SetValue(ToWxString(DEF_FILE));
}


bool CHitMatrixDemoDlg::ShowToolTips()
{
    return true;
}


wxBitmap CHitMatrixDemoDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CHitMatrixDemoDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CHitMatrixDemoDlg bitmap retrieval
}


wxIcon CHitMatrixDemoDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CHitMatrixDemoDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CHitMatrixDemoDlg icon retrieval
}



void CHitMatrixDemoDlg::OnCloseWindow(wxCloseEvent& event)
{
    Destroy();
}



void CHitMatrixDemoDlg::OnCloseClick( wxCommandEvent& event )
{
    Close();
}



void CHitMatrixDemoDlg::OnAccClick( wxCommandEvent& event )
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
        m_MatrixWidget->SetDataSource(NULL);

        m_Builder.Init(*m_Scope, handle);
        m_DataSource = m_Builder.CreateDataSource();

        if(m_DataSource->SelectDefaultIds())    {
            m_MatrixWidget->SetDataSource(m_DataSource.GetPointer());
        }
    } else {
        string msg("Can't find sequence for accession '");
        msg += acc + "'";
        wxMessageBox(ToWxString(msg), wxT("Cannot load accesion"),
                     wxOK | wxICON_ERROR);
        return;
    }
}


typedef IHitMatrixDataSource::TAlignVector TAlignVector;


/// loads a serializable object of type T
template <class T>
    bool    LoadObject(const wxString& filename, CScope& scope, CHitMatrixDSBuilder& builder)
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



void CHitMatrixDemoDlg::OnFileClick( wxCommandEvent& event )
{
    m_MatrixWidget->SetDataSource(NULL);

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
            m_DataSource = m_Builder.CreateDataSource();

            if(m_DataSource->SelectDefaultIds())    {
                m_MatrixWidget->SetDataSource(m_DataSource.GetPointer());
            }
        } else {
            wxMessageBox(wxT("This type of file is not supported"),
                         wxT("Error loading file."),
                         wxOK | wxICON_ERROR);
        }
    } catch(CException& e)  {
        LOG_POST(Error << e.ReportAll());
        wxString s = ToWxString(e.GetMsg());
        wxMessageBox(s, wxT("Error loading file."), wxOK | wxICON_ERROR);
    }
}


END_NCBI_SCOPE
