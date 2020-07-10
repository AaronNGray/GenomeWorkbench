/////////////////////////////////////////////////////////////////////////////
// Name:        demo_feat_table_frame.cpp
// Purpose:
// Author:      Yury Voronov
// Modified by:
// Created:     03/01/2008 15:26:43
// RCS-ID:
// Copyright:
// Licence:
/////////////////////////////////////////////////////////////////////////////

#include <ncbi_pch.hpp>

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
#include "wx/imaglist.h"
////@end includes

#include "demo_feat_table_frame.hpp"
#include <gui/widgets/feat_table/feat_table.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <serial/serial.hpp>
#include <serial/objistr.hpp>
#include <serial/iterator.hpp>

#include <objects/seq/Seq_annot.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objtools/data_loaders/genbank/gbloader.hpp>
#include <objmgr/object_manager.hpp>
#include <objmgr/scope.hpp>

#include <gui/objutils/utils.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CwxDemoFeatureTableFrame type definition
 */

IMPLEMENT_CLASS( CwxDemoFeatureTableFrame, wxFrame )


/*!
 * CwxDemoFeatureTableFrame event table definition
 */

BEGIN_EVENT_TABLE( CwxDemoFeatureTableFrame, wxFrame )

////@begin CwxDemoFeatureTableFrame event table entries
    EVT_MENU( wxID_OPEN, CwxDemoFeatureTableFrame::OnOpenClick )

    EVT_MENU( wxID_LOAD, CwxDemoFeatureTableFrame::OnLoadClick )

////@end CwxDemoFeatureTableFrame event table entries

END_EVENT_TABLE()


/*!
 * CwxDemoFeatureTableFrame constructors
 */

CwxDemoFeatureTableFrame::CwxDemoFeatureTableFrame()
{
    Init();
}

CwxDemoFeatureTableFrame::CwxDemoFeatureTableFrame( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create( parent, id, caption, pos, size, style );
}


/*!
 * CwxDemoObjectListFrame creator
 */

bool CwxDemoFeatureTableFrame::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CwxDemoFeatureTableFrame creation
    wxFrame::Create( parent, id, caption, pos, size, style );

    CreateControls();
    Centre();
////@end CwxDemoFeatureTableFrame creation
    return true;
}


/*!
 * CwxDemoFeatureTableFrame destructor
 */

CwxDemoFeatureTableFrame::~CwxDemoFeatureTableFrame()
{
////@begin CwxDemoFeatureTableFrame destruction
////@end CwxDemoFeatureTableFrame destruction
}


/*!
 * Member initialisation
 */

void CwxDemoFeatureTableFrame::Init()
{
////@begin CwxDemoFeatureTableFrame member initialisation
    m_ObjectManager = CObjectManager::GetInstance();
    m_FeatTableCtrl = NULL;
////@end CwxDemoFeatureTableFrame member initialisation

    CGBDataLoader::RegisterInObjectManager(*m_ObjectManager);
}


/*!
 * Control creation for CwxDemoObjectListFrame
 */

void CwxDemoFeatureTableFrame::CreateControls()
{
////@begin CwxDemoFeatureTableFrame content construction
    CwxDemoFeatureTableFrame* itemFrame1 = this;

    wxMenuBar* menuBar = new wxMenuBar;
    wxMenu* itemMenu3 = new wxMenu;
    itemMenu3->Append(wxID_OPEN, _("&Open...\tCtrl+O"), wxEmptyString, wxITEM_NORMAL);
    itemMenu3->Append(wxID_LOAD, _("&Load...\tCtrl+L"), wxEmptyString, wxITEM_NORMAL);
    itemMenu3->AppendSeparator();
    itemMenu3->Append(wxID_EXIT, _("E&xit\tAlt+F4"), wxEmptyString, wxITEM_NORMAL);
    menuBar->Append(itemMenu3, _("File"));
    wxMenu* itemMenu8 = new wxMenu;
    itemMenu8->Append(wxID_ABOUT, _("About..."), wxEmptyString, wxITEM_NORMAL);
    menuBar->Append(itemMenu8, _("Help"));
    itemFrame1->SetMenuBar(menuBar);

    m_FeatTableCtrl = new CFeatTableWidget( itemFrame1, ID_FEATTABLE, wxDefaultPosition, wxSize(100, 100), wxLC_REPORT|wxLC_VIRTUAL );

////@end CwxDemoFeatureTableFrame content construction
}

bool CwxDemoFeatureTableFrame::LoadAccession( const wxString& anAccession ){

    CRef<CSeq_id> id;
    id.Reset( new CSeq_id() );

    string accession(ToStdString(anAccession));

    try {
        id->Set( accession );

    } catch( CSeqIdException& ){
        string msg("Accession '");
        msg += accession;
        msg += "' not recognized as a valid accession";

        wxMessageDialog dlg(NULL, wxT("Unhandled Accession"),
                            ToWxString(msg), wxOK | wxICON_ERROR);
        dlg.ShowModal();

        return false;
    }

    CRef<CScope> scope;
    scope.Reset( new CScope(*m_ObjectManager) );
    scope->AddDefaults();

    // retrieve our sequence
    CBioseq_Handle handle = scope->GetBioseqHandle( *id );
    if( !handle ){
        string msg("Can't find sequence for accession '");
        msg += accession;
        msg += "'";
        wxMessageDialog dlg(NULL, wxT("Sequence Not Found"),
                            ToWxString(msg), wxOK | wxICON_ERROR);
        dlg.ShowModal();

        return false;
    }

    CRef<CSeq_entry> seq( new CSeq_entry() );
    seq->SetSeq( const_cast<CBioseq&>( *handle.GetCompleteBioseq() ) );

    scope->AddTopLevelSeqEntry(*seq);

    CRef<CSeq_loc> loc;
    loc.Reset( new CSeq_loc( CSeq_loc::e_Whole ) );
    loc->SetWhole( *id );

    /*
    TConstScopedObjects objects;
    objects.push_back( SConstScopedObject( *id, *scope ) );
    objects.push_back( SConstScopedObject( *loc, *scope ) );
    objects.push_back( SConstScopedObject( *seq, *scope ) );

    objects::SAnnotSelector sel = CSeqUtils::GetAnnotSelector( CSeq_annot::TData::e_Ftable );

    CFeat_CI feat_iter( handle );

    for( int i = 0; feat_iter; ++feat_iter, i++ ){
        const CSeq_feat& feat = feat_iter->GetOriginalFeature();
        objects.push_back( SConstScopedObject( feat, *scope ) );
    }
    */

    m_FeatTableCtrl->GetDataSource()->Init( *loc, *scope );

    return true;
}

bool CwxDemoFeatureTableFrame::LoadFile( const wxString& aFileName )
{
    CRef<CSeq_entry> seq( new CSeq_entry() );

    try {
        CNcbiIfstream istr(aFileName.fn_str());
        auto_ptr<CObjectIStream> os( CObjectIStream::Open( eSerial_AsnText, istr ) );

        *os >> *seq;

    } catch( CException& /*e*/) {

        string msg("This type of file is not supported");
        wxMessageDialog dlg(NULL, wxT("Error loading file"),
                            ToWxString(msg), wxOK | wxICON_ERROR);
        dlg.ShowModal();

        return false;
    }

    CRef<CSeq_id> id;
    id.Reset( new CSeq_id() );

    if( seq->IsSeq() ){
        CBioseq& bioseq = seq->SetSeq();
        if( bioseq.CanGetId() ){
            list< CRef< CSeq_id > >& id_list = bioseq.SetId();
            if( !id_list.empty() ){
                id.Reset( id_list.front() );
            }
        }
    }

    CRef<CScope> scope;
    scope.Reset( new CScope(*m_ObjectManager) );
    scope->AddTopLevelSeqEntry(*seq);
    scope->AddDefaults();

    CRef<CSeq_loc> loc;
    loc.Reset( new CSeq_loc( CSeq_loc::e_Whole ) );
    loc->SetWhole( *id );


    /*
    TConstScopedObjects objects;
    objects.push_back( SConstScopedObject( *id, *scope ) );
    objects.push_back( SConstScopedObject( *loc, *scope ) );
    objects.push_back( SConstScopedObject( *seq, *scope ) );

    objects::SAnnotSelector sel = CSeqUtils::GetAnnotSelector( CSeq_annot::TData::e_Ftable );

    CFeat_CI feat_iter( *scope, *loc, sel );

    for( int i = 0; feat_iter; ++feat_iter, i++ ){
        const CSeq_feat& feat = feat_iter->GetOriginalFeature();
        objects.push_back( SConstScopedObject( feat, *scope ) );
    }
    */

    m_FeatTableCtrl->GetDataSource()->Init( *loc, *scope );

    return true;
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for wxID_OPEN
 */

void CwxDemoFeatureTableFrame::OnOpenClick( wxCommandEvent& event )
{
    wxFileDialog dlg(
        this,
        wxT("Choose a data file"), wxEmptyString, wxEmptyString,
        wxT("ASN files (*.asn)|*.asn"), wxFD_OPEN | wxFD_FILE_MUST_EXIST
    );

    if( dlg.ShowModal() == wxID_OK ){
        LoadFile( dlg.GetPath() );
    }
}


/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for wxID_LOAD
 */

void CwxDemoFeatureTableFrame::OnLoadClick( wxCommandEvent& event )
{
    // Show Choose/Accession dialog
    wxTextEntryDialog dlg(
        this,
        wxT("Please enter an accession from GenBank"), wxT("Enter accession"),
        wxEmptyString,
        wxOK | wxCANCEL
    );

    if( dlg.ShowModal() == wxID_OK ){
        LoadAccession( dlg.GetValue() );
    }
}


/*!
 * Should we show tooltips?
 */

bool CwxDemoFeatureTableFrame::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CwxDemoFeatureTableFrame::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CwxDemoFeatureTableFrame bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CwxDemoFeatureTableFrame bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CwxDemoFeatureTableFrame::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CwxDemoFeatureTableFrame icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CwxDemoFeatureTableFrame icon retrieval
}

END_NCBI_SCOPE
