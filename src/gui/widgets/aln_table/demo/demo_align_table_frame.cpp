/////////////////////////////////////////////////////////////////////////////
// Name:        demo_align_table_frame.cpp
// Purpose:
// Author:      Yury Voronov
// Modified by:
// Created:     17/01/2008 21:42:34
// RCS-ID:
// Copyright:
// Licence:
/////////////////////////////////////////////////////////////////////////////

#include <ncbi_pch.hpp>

#include "demo_align_table_frame.hpp"
#include <gui/widgets/aln_table/aln_table.hpp>
#include <gui/widgets/aln_table/alnspan_widget.hpp>
#include <gui/widgets/aln_multiple/alnmulti_ds_builder.hpp>
#include <gui/objutils/utils.hpp>

#include <serial/serial.hpp>
#include <serial/objistr.hpp>
#include <serial/iterator.hpp>

#include <objects/seq/Seq_annot.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objtools/data_loaders/genbank/gbloader.hpp>
#include <objmgr/object_manager.hpp>
#include <objmgr/scope.hpp>

#include <objmgr/align_ci.hpp>
#include <objmgr/seq_vector.hpp>
#include <objects/seqalign/Seq_align_set.hpp>
#include <objtools/alnmgr/alnvec.hpp>

#include <algo/align/util/score_builder.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CwxDemoAlignTableFrame type definition
 */

IMPLEMENT_CLASS( CwxDemoAlignTableFrame, wxFrame )


/*!
 * CwxDemoAlignTableFrame event table definition
 */

BEGIN_EVENT_TABLE( CwxDemoAlignTableFrame, wxFrame )

////@begin CwxDemoAlignTableFrame event table entries
    EVT_MENU( wxID_OPEN, CwxDemoAlignTableFrame::OnOpenClick )

    EVT_MENU( wxID_LOAD, CwxDemoAlignTableFrame::OnLoadClick )

    EVT_MENU( ID_SEGMENTS, CwxDemoAlignTableFrame::OnSegmentsClick )

    EVT_MENU( ID_SPANS, CwxDemoAlignTableFrame::OnSpansClick )

////@end CwxDemoAlignTableFrame event table entries

END_EVENT_TABLE()


/*!
 * CwxDemoAlignTableFrame constructors
 */

CwxDemoAlignTableFrame::CwxDemoAlignTableFrame()
{
    Init();
}

CwxDemoAlignTableFrame::CwxDemoAlignTableFrame( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create( parent, id, caption, pos, size, style );
}


/*!
 * CwxDemoAlignTableFrame creator
 */

bool CwxDemoAlignTableFrame::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CwxDemoAlignTableFrame creation
    wxFrame::Create( parent, id, caption, pos, size, style );

    CreateControls();
    Centre();
////@end CwxDemoAlignTableFrame creation
    return true;
}


/*!
 * CwxDemoAlignTableFrame destructor
 */

CwxDemoAlignTableFrame::~CwxDemoAlignTableFrame()
{
////@begin CwxDemoAlignTableFrame destruction
////@end CwxDemoAlignTableFrame destruction
}


/*!
 * Member initialisation
 */

void CwxDemoAlignTableFrame::Init()
{
////@begin CwxDemoAlignTableFrame member initialisation
    m_ObjectManager = CObjectManager::GetInstance();
    m_Segments = true;
    m_AlignTableCtrl = NULL;
////@end CwxDemoAlignTableFrame member initialisation

    CGBDataLoader::RegisterInObjectManager(*m_ObjectManager);

    m_Scope.Reset( new CScope(*m_ObjectManager) );
    m_Scope->AddDefaults();
}


/*!
 * Control creation for CwxDemoAlignTableFrame
 */

void CwxDemoAlignTableFrame::CreateControls()
{
////@begin CwxDemoAlignTableFrame content construction
    CwxDemoAlignTableFrame* itemFrame1 = this;

    wxMenuBar* menuBar = new wxMenuBar;
    wxMenu* itemMenu3 = new wxMenu;
    itemMenu3->Append(wxID_OPEN, _("&Open...\tCtrl+O"), wxEmptyString, wxITEM_NORMAL);
    itemMenu3->Append(wxID_LOAD, _("&Load...\tCtrl+L"), wxEmptyString, wxITEM_NORMAL);
    itemMenu3->AppendSeparator();
    itemMenu3->Append(wxID_EXIT, _("E&xit\tAlt+F4"), wxEmptyString, wxITEM_NORMAL);
    menuBar->Append(itemMenu3, _("File"));
    wxMenu* itemMenu8 = new wxMenu;
    itemMenu8->Append(ID_SEGMENTS, _("Segments"), wxEmptyString, wxITEM_RADIO);
    itemMenu8->Check(ID_SEGMENTS, true);
    itemMenu8->Append(ID_SPANS, _("Spans"), wxEmptyString, wxITEM_RADIO);
    menuBar->Append(itemMenu8, _("View"));
    wxMenu* itemMenu11 = new wxMenu;
    itemMenu11->Append(wxID_ABOUT, _("About..."), wxEmptyString, wxITEM_NORMAL);
    menuBar->Append(itemMenu11, _("Help"));
    itemFrame1->SetMenuBar(menuBar);

    m_AlignTableCtrl = new wxListCtrl( itemFrame1, ID_ALIGNTABLE, wxDefaultPosition, wxSize(100, 100), wxLC_REPORT|wxLC_VIRTUAL );

////@end CwxDemoAlignTableFrame content construction

    delete m_AlignTableCtrl;
    m_AlignTableCtrl = new CAlnTableWidget( itemFrame1, ID_ALIGNTABLE );
    m_Segments = true;
}

bool CwxDemoAlignTableFrame::LoadAccession( const wxString& anAccession ){

    CRef<CSeq_id> id;
    id.Reset( new CSeq_id() );

    string accession( anAccession.c_str() );

    try {
        id->Set( accession );

    } catch( CSeqIdException& ){
        string msg("Accession '");
        msg += anAccession;
        msg += "' not recognized as a valid accession";

        wxMessageDialog dlg(
            NULL, wxT("Unhandled Accession"),
            msg, wxOK | wxICON_ERROR
        );
        dlg.ShowModal();

        return false;
    }

    CRef<CScope> scope( new CScope(*m_ObjectManager) );
    scope->AddDefaults();

    // retrieve our sequence
    CBioseq_Handle handle = scope->GetBioseqHandle( *id );
    if( !handle ){
        string msg("Can't find sequence for accession '");
        msg += anAccession;
        msg += "'";
        wxMessageDialog dlg(
            NULL, wxT("Sequence Not Found"),
            msg, wxOK | wxICON_ERROR
        );
        dlg.ShowModal();

        return false;
    }

    CRef<CSeq_entry> seq( new CSeq_entry() );
    seq->SetSeq( const_cast<CBioseq&>( *handle.GetCompleteBioseq() ) );

    scope->AddTopLevelSeqEntry(*seq);

    m_Seq_id = id;
    m_Scope = scope;

    InitData();
    /*
    CRef<CSeq_loc> loc;
    loc.Reset( new CSeq_loc( CSeq_loc::e_Whole ) );
    loc->SetWhole( *id );

    if( m_Segments ){
        ((CAlnTableWidget*)m_AlignTableCtrl)->SetDataSource(*new CAlnTableDS(*scope, *loc));
    } else {
        vector< CConstRef<CSeq_align> > align_vec;
        CAlign_CI align_iter(*scope, *loc);
        for ( ;  align_iter;  ++align_iter) {
            const CSeq_align& align = align_iter.GetOriginalSeq_align();
            CConstRef<CSeq_align> ref( &align );
            align_vec.push_back( ref );
        }

        CAlnMultiDSBuilder builder;
        builder.Init( *scope, align_vec );
        builder.SetSyncCreate( true );
        builder.PreCreateDataSource( false );

        CRef<IAlnMultiDataSource> dataSource = builder.CreateDataSource();

        ((CAlnSpanWidget*)m_AlignTableCtrl)->SetModel( new CAlnSpanVertModel( *dataSource ), true );
    }
    */

    return true;
}

bool CwxDemoAlignTableFrame::LoadFile( const wxString& aFileName )
{
    CRef<CSeq_entry> seq( new CSeq_entry() );

    try {
        CNcbiIfstream istr(aFileName.fn_str());
        auto_ptr<CObjectIStream> os( CObjectIStream::Open( eSerial_AsnText, istr ) );

        *os >> *seq;

    } catch( CException& /*e*/) {

        string msg("This type of file is not supported");
        wxMessageDialog dlg(
            NULL, wxT("Error loading file"),
            msg, wxOK | wxICON_ERROR
        );
        dlg.ShowModal();

        return false;
    }

    CRef<CScope> scope( new CScope(*m_ObjectManager) );
    scope->AddDefaults();
    scope->AddTopLevelSeqEntry(*seq);

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

    m_Seq_id = id;
    m_Scope = scope;

    InitData();

    return true;
}

void CwxDemoAlignTableFrame::InitData() {


    CRef<CSeq_loc> loc;
    loc.Reset( new CSeq_loc( CSeq_loc::e_Whole ) );
    loc->SetWhole( *m_Seq_id );

    CRef<CScope> scope = m_Scope;

    if( m_Segments ){
        ((CAlnTableWidget*)m_AlignTableCtrl)->SetDataSource(*new CAlnTableDS(*scope, *loc));
    } else {
        vector< CConstRef<CSeq_align> > align_vec;
        CAlign_CI align_iter(*scope, *loc);
        for ( ;  align_iter;  ++align_iter) {
            const CSeq_align& align = align_iter.GetOriginalSeq_align();
            CConstRef<CSeq_align> ref( &align );
            align_vec.push_back( ref );
        }

        CAlnMultiDSBuilder builder;
        builder.Init( *scope, align_vec );
        builder.SetSyncCreate( true );
        builder.PreCreateDataSource( false );

        CRef<IAlnMultiDataSource> dataSource = builder.CreateDataSource();

        IwxTableModel* model = ((CAlnSpanWidget*)m_AlignTableCtrl)->GetModel();
        ((CAlnSpanWidget*)m_AlignTableCtrl)->RemoveModel();
        delete model;

        ((CAlnSpanWidget*)m_AlignTableCtrl)->SetAlnModel( *(new CAlnSpanVertModel( *dataSource )) );
    }
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for wxID_OPEN
 */

void CwxDemoAlignTableFrame::OnOpenClick( wxCommandEvent& event )
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

void CwxDemoAlignTableFrame::OnLoadClick( wxCommandEvent& event )
{
    // Show Choose/Accession dialog
    wxTextEntryDialog dlg(
        this,
        _("Please enter an accession from GenBank"), _("Enter accession"),
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

bool CwxDemoAlignTableFrame::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CwxDemoAlignTableFrame::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CwxDemoAlignTableFrame bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CwxDemoAlignTableFrame bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CwxDemoAlignTableFrame::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CwxDemoAlignTableFrame icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CwxDemoAlignTableFrame icon retrieval
}


/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_SEGMENTS
 */

void CwxDemoAlignTableFrame::OnSegmentsClick( wxCommandEvent& event )
{
    RemoveChild( m_AlignTableCtrl );
    delete m_AlignTableCtrl;
    m_AlignTableCtrl = new CAlnTableWidget( this, ID_ALIGNTABLE );
    m_Segments = true;

    InitData();
    m_AlignTableCtrl->SetSize( GetClientSize() );
    Refresh();
}


/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_SPANS
 */

void CwxDemoAlignTableFrame::OnSpansClick( wxCommandEvent& event )
{
    RemoveChild( m_AlignTableCtrl );
    delete m_AlignTableCtrl;
    m_AlignTableCtrl = new CAlnSpanWidget( this, ID_ALIGNTABLE );
    m_Segments = false;

    InitData();
    m_AlignTableCtrl->SetSize( GetClientSize() );
    Refresh();
}

END_NCBI_SCOPE
