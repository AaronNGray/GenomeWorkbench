
/*  $Id: align_span_view.cpp 40880 2018-04-25 20:26:58Z katargir $
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
 * Authors:  Mike DiCuccio, Yuri Voronov
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/packages/pkg_alignment/align_span_view.hpp>


#include <gui/widgets/aln_table/alnspan_widget.hpp>

#include <gui/core/project_service.hpp>
#include <gui/objutils/visible_range.hpp>

#include <gui/framework/workbench.hpp>

#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/aln_multiple/alnmulti_ds_builder.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/objutils/label.hpp>
#include <gui/objutils/registry.hpp>

#include <objmgr/util/seq_loc_util.hpp>
#include <objmgr/util/sequence.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objtools/alnmgr/alnvec.hpp>
#include <objtools/alnmgr/aln_container.hpp>
#include <objtools/alnmgr/aln_converters.hpp>
#include <objtools/alnmgr/aln_builders.hpp>

#include <serial/iterator.hpp>

#include <wx/menu.h>
#include <wx/sizer.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CProjectViewTypeDescriptor CAlignSpanView::m_TypeDescr(
    "Alignment Span View", // type name
    "align_span_view", // icon alias TODO
    "Show tabular information about matches and mismatches",
    "The Alignment Span View provides a tabular break-down of matches, "
       "mismatches and gaps in bounded regions within an alignment",
    "ALIGN_SPAN_VIEW", // help ID
    "Alignment",   // category
    false,       // not a singleton
    "Seq-align",
    eSimilarObjectsAccepted
);

/*
BEGIN_EVENT_MAP(CAlignSpanView, CProjectView)
    //ON_COMMAND(eCmdFilter,  &CViewAlnSpanTable::OnFilter)
END_EVENT_MAP()
*/


CAlignSpanView::CAlignSpanView()
    : m_Panel()
    , m_StatusBar()
    , m_AlnWidget()
{
}

wxWindow* CAlignSpanView::GetWindow()
{
    _ASSERT(m_Panel);
    return m_Panel;
}

void CAlignSpanView::x_InitMenuResources()
{
    static bool initialized = false;

    // this code should execute only once
    if( ! initialized ){
        // register commands
        /*
        CUICommandRegistry& cmd_reg = m_Workbench->GetUICommandRegistry();
        cmd_reg->RegisterCommand(
            eCmdExportToCSV, 
            "Export to CSV...", "Export to CSV", "", "Exports data to file in CSV format"
        );
        */

        // register icons
        //wxFileArtProvider& provider = m_Workbench->GetFileArtProvider();
        //provider.RegisterFileAlias( ... );

        initialized = true;
    }
}

void CAlignSpanView::x_CreateMenuBarMenu()
{
    x_InitMenuResources();

    wxMenu* view_menu = new wxMenu();

    //m_AlnWidget->AppendMenuItems( *view_menu );
    view_menu->Append( wxID_SEPARATOR, wxT("Actions") );
    view_menu->Append( CwxTableListCtrl::eCmdExportToCSV, wxT("Export to CSV...") );

    wxMenu* bar_menu = new wxMenu();
    bar_menu->Append( wxID_ANY, wxT("&View"), view_menu );

    m_MenuBarMenu.reset( bar_menu );
}

void CAlignSpanView::CreateViewWindow( wxWindow* parent )
{
    _ASSERT( !m_AlnWidget );

    m_Panel = new wxPanel( parent );

    // main sizer
    wxBoxSizer* main_sizer = new wxBoxSizer( wxVERTICAL );
    m_Panel->SetSizer( main_sizer );

    m_AlnWidget = new CAlnSpanWidget( m_Panel );
    m_Model.reset( new CAlnSpanVertModel() );
    m_AlnWidget->SetAlnModel( *m_Model );

    AddListener( m_AlnWidget, ePool_Child );
    m_AlnWidget->AddListener( this, ePool_Parent );

    main_sizer->Add( m_AlnWidget, 1, wxEXPAND );

    // wxStatusBar
    m_StatusBar = new wxStatusBar( m_Panel );
    main_sizer->Add( m_StatusBar, 0, wxEXPAND );

    //m_Panel->PushEventHandler( new CCommandTransferHandler( m_AlnWidget ) );
    m_Panel->PushEventHandler( new CCommandToFocusHandler( m_Panel ) );
}


void CAlignSpanView::DestroyViewWindow()
{
    if( GetWindow() ){
        m_Panel->PopEventHandler( true );

        GetWindow()->Destroy();
        m_Panel = NULL;
        m_AlnWidget = NULL;
        m_StatusBar = NULL;
    }
}


void CAlignSpanView::SetRegistryPath(const string& reg_path)
{
    m_RegPath = reg_path;
}


void CAlignSpanView::LoadSettings()
{
    if( m_AlnWidget->GetModel() == NULL ){
        return;
    }

    /// restore our table's settings
    CGuiRegistry& reg = CGuiRegistry::GetInstance();
    CRegistryReadView reg_view = reg.GetReadView(m_RegPath);

    m_AlnWidget->LoadTableSettings(reg_view, true);
}

void CAlignSpanView::SaveSettings() const
{
    if( m_AlnWidget->GetModel() == NULL ){
        return;
    }

    /// save our table's settings
    CGuiRegistry& reg = CGuiRegistry::GetInstance();
    CRegistryWriteView reg_view = reg.GetWriteView(m_RegPath);

    m_AlnWidget->SaveTableSettings(reg_view);
}


const CViewTypeDescriptor& CAlignSpanView::GetTypeDescriptor() const
{
    return m_TypeDescr;
}

void CAlignSpanView::x_InitWidget()
{
    CRef<CScope> scope = GetScope();
    _ASSERT(scope.NotNull());

    m_AlnWidget->RemoveModel();

    if( true ){
        /*
        bool spliced = false;

        ITERATE( vector< CConstRef<CSeq_align> >, ait, m_InputAligns ){
            if( (*ait)->GetSegs().Which() == CSeq_align::C_Segs::e_Spliced ){

                m_Model.reset( 
                    new CAlnSpanVertModel( (*ait)->GetSegs().GetSpliced(), *GetScope() )
                );
                spliced = true;
                break;
            }
        }
        
        if( !spliced ){
            CAlnContainer container;

            if( m_OrigAlign ){
                container.insert( *m_OrigAlign );

            } else if( m_OrigAnnot ){
                CTypeConstIterator<CSeq_align> it( *m_OrigAnnot );
                while( it ){
                    container.insert( *it );
                    ++it;
                }

            } else if( m_OrigBioseq ){
                CTypeConstIterator<CSeq_align> it( *m_OrigAnnot );
                while( it ){
                    container.insert( *it );
                    ++it;
                }

            if( m_InputAligns.size() > 0 ){
                ITERATE( vector< CConstRef<CSeq_align> >, ait, m_InputAligns ){
                    container.insert( **ait );
                }

            } else {
                _ASSERT(false);
            }

            /// Create a vector of seq-ids per seq-align
            TIdExtract id_extract;
            TAlnIdMap aln_id_map( id_extract, container.size() );
            ITERATE( CAlnContainer, aln_it, container ){
                try {
                    aln_id_map.push_back( **aln_it );

                } catch( CAlnException e ){
                    LOG_POST( Error << "Skipping this alignment: " << e.what() );
                }
            }

            /// Create align statistics object
            TAlnStats aln_stats( aln_id_map );

            /// Can the alignments be anchored?
            if ( !aln_stats.CanBeAnchored() ) {
                LOG_POST( Error << "Input alignments cannot be anchored because they don't share at least one common sequence." );
                //! THROW
                return;
            }

            /// Create user options
            CAlnUserOptions aln_user_options;

            /// Construct a vector of anchored alignments
            TAnchoredAlnVec anchored_aln_vec;
            CreateAnchoredAlnVec( aln_stats, anchored_aln_vec, aln_user_options );

            BuildAln( anchored_aln_vec, m_AnchoredAln, aln_user_options );

            if( m_AnchoredAln.GetDim() == 1 ){
                m_AnchoredAln = CAnchoredAln();

                aln_user_options.m_MergeAlgo = CAlnUserOptions::ePreserveRows;

                BuildAln( anchored_aln_vec, m_AnchoredAln, aln_user_options );
            }

            /// Get sequence:
            CRef<CSparseAln> sparse_aln;
            sparse_aln.Reset( new CSparseAln( m_AnchoredAln, *GetScope() ) );

            m_Model.reset( new CAlnSpanVertModel( *sparse_aln ) );
        }
        */
        m_Model.reset( new CAlnSpanVertModel( m_InputAligns, *GetScope() ) );

        int aln_type = CAlnSpanVertModel::fGap;
        ITERATE( vector< CConstRef<CSeq_align> >, ait, m_InputAligns ){
            if( (*ait)->GetSegs().Which() == CSeq_align::C_Segs::e_Spliced ){
                aln_type = CAlnSpanVertModel::fIntron;

            } else if( 
                (*ait)->GetSegs().Which() == CSeq_align::C_Segs::e_Denseg 
                || (*ait)->GetSegs().Which() == CSeq_align::C_Segs::e_Disc 
            ){
                aln_type = CAlnSpanVertModel::fDiscontig;

            } else {
                continue;
            }

            break;
        }
        m_Model->SetMarkIndels( aln_type );
        m_Model->SetThreshold( 1000000 );
        m_Model->UpdateRows();

    } else {
        CAlnMultiDSBuilder builder;
        if( m_OrigAnnot ){
            builder.Init( *scope, *m_OrigAnnot );
        } else if( m_OrigAlign ){
            builder.Init( *scope, *m_OrigAlign );
        } else if( m_OrigBioseq ){
            builder.Init( *scope, *m_OrigBioseq );
        } else {
            _ASSERT(false);
        }

        builder.SetSyncCreate( true );
        builder.PreCreateDataSource( false );
        m_DataSource = builder.CreateDataSource();

        /* TODO clarification
        if(m_DataSource->SelectDefaultIds())    {
        m_AlnWidget->SetDataSource(m_DataSource.GetPointer());
        }
        */
        m_Model.reset( new CAlnSpanVertModel( *m_DataSource ) );
    }
    m_AlnWidget->SetAlnModel( *m_Model );
}

objects::CScope* CAlignSpanView::x_PreAttachToProject( TConstScopedObjects& objects )
{
    if( objects.size() == 0 ){
        return NULL;
    }

    const CObject* object = objects[0].object.GetPointer();
    CScope* scope = objects[0].scope.GetPointer();

    for(;;){
        const CSeq_annot* annot = dynamic_cast<const CSeq_annot*>(object);
        if( annot ){
            m_OrigAnnot.Reset( annot );
            break;
        }

        const CSeq_align* align = dynamic_cast<const CSeq_align*>(object);
        if( align ){
            m_OrigAlign.Reset( align );
            break;
        }

        const CBioseq* bioseq = dynamic_cast<const CBioseq*>(object);
        if( bioseq ){
            m_OrigBioseq.Reset( bioseq );
            break;
        }

        break;
    }

    ITERATE( TConstScopedObjects, cso_it, objects ){
        const CObject* object = cso_it->object.GetPointer();
        CConstRef<CSeq_align> ref;

        const CSeq_annot* annot = dynamic_cast<const CSeq_annot*>(object);
        if( annot ){
            CTypeConstIterator<CSeq_align> it( *annot );
            while( it ){
                ref.Reset( &*it );
                m_InputAligns.push_back( ref );
                ++it;
            }
        }

        const CSeq_align* align = dynamic_cast<const CSeq_align*>(object);
        if( align ){
            ref.Reset( align );
            m_InputAligns.push_back( ref );
        }

        const CBioseq* bioseq = dynamic_cast<const CBioseq*>(object);
        if( bioseq ){
            CTypeConstIterator<CSeq_align> it( *bioseq );
            while( it ){
                ref.Reset( &*it );
                m_InputAligns.push_back( ref );
                ++it;
            }
        }
    }

    if( m_OrigAnnot || m_OrigAlign || m_OrigBioseq ){
        if( m_InputAligns.size() > 0 ){
            return scope;
        }
    }

    return NULL;
}

void CAlignSpanView::x_PostAttachToProject()
{
    if( x_HasProject() ){

        x_InitWidget();

        LoadSettings();
    }

    x_UpdateContentLabel();

    x_UpdateStatusMessage();
}

void CAlignSpanView::OnProjectChanged( CProjectViewEvent& evt )
{
    if( 
        evt.GetType() == CViewEvent::eProjectChanged
        && (evt.GetSubtype() & CProjectViewEvent::eProperties)
    ){
        x_UpdateContentLabel();

        x_UpdateStatusMessage();
    }
}

const CObject* CAlignSpanView::x_GetOrigObject() const
{
    const CObject* obj = NULL;

    if( m_OrigAnnot ){
        obj = (const CObject*) m_OrigAnnot.GetPointer();
    } else if( m_OrigAlign ){
        obj = (const CObject*) m_OrigAlign.GetPointer();
    } else if( m_OrigBioseq ){
        obj = (const CObject*) m_OrigBioseq.GetPointer();
    }
    return obj;
}

void CAlignSpanView::GetSelection( TConstScopedObjects& objs ) const
{
    if(m_AlnWidget  &&  x_HasProject())  {
        CRef<CScope> scope = GetScope();

        _ASSERT(!scope.IsNull());

        CAlnSpanWidget::TLocations locs;
        m_AlnWidget->GetSelection(locs);

        /// we pass everything back as range selections
        typedef map<CSeq_id_Handle, CSelectionEvent::TRangeColl> TRangeMap;
        TRangeMap ranges;
        ITERATE (CAlnSpanWidget::TLocations, iter, locs) {
            CSeq_id_Handle idh = sequence::GetIdHandle(**iter, NULL);
            TSeqRange range = (*iter)->GetTotalRange();

            ranges[idh] += range;
        }

        ITERATE (TRangeMap, iter, ranges) {
            CRef<CSeq_loc> loc(new CSeq_loc);
            loc->SetInt().SetFrom(iter->second.GetFrom());
            loc->SetInt().SetTo  (iter->second.GetTo());
            loc->SetId(*iter->first.GetSeqId());
            objs.push_back(SConstScopedObject(loc, scope));
        }
    }
}

void CAlignSpanView::GetSelection( CSelectionEvent& evt ) const
{
    if( x_HasProject() ){
        CAlnSpanWidget::TLocations locs;
        m_AlnWidget->GetSelection(locs);

        /// we pass everything back as range selections
        typedef map<CSeq_id_Handle, CSelectionEvent::TRangeColl> TRangeMap;
        TRangeMap ranges;
        ITERATE (CAlnSpanWidget::TLocations, iter, locs) {
            CSeq_id_Handle idh = sequence::GetIdHandle(**iter, NULL);
            TSeqRange range = (*iter)->GetTotalRange();

            ranges[idh] += range;
        }

        ITERATE (TRangeMap, iter, ranges) {
            evt.AddRangeSelection(*iter->first.GetSeqId(), iter->second);
        }
    }
}


// handles incoming selection broadcast (overriding CView virtual function)
void CAlignSpanView::x_OnSetSelection( CSelectionEvent& evt )
{
    // TODO It is not clear what to select here

    if( evt.HasObjectSelection() ){ 

        x_UpdateStatusMessage();
    }
}

void CAlignSpanView::OnWidgetSelChanged()
{
    CProjectView::OnWidgetSelChanged();
    x_UpdateStatusMessage();
}

void CAlignSpanView::GetVisibleRanges(CVisibleRange& vrange) const
{
    if( x_HasProject() ){
        CAlnSpanWidget::TLocations locs;
        m_AlnWidget->GetSelection( locs );

        ITERATE ( CAlnSpanWidget::TLocations, iter, locs ){
            vrange.AddLocation( **iter );
        }
    }
}

void CAlignSpanView::OnWidgetRangeChanged()
{
    //CProjectView::OnWidgetRangeChanged();
    x_UpdateStatusMessage();
}

/*
void CAlignSpanView::OnFilter()
{
    //m_AlnWidget->OnFilter();
}

*/

void CAlignSpanView::x_Update() {

    if( m_AlnWidget && x_HasProject() ){
        m_AlnWidget->Update();

        x_UpdateStatusMessage();
    }
}

void CAlignSpanView::x_UpdateStatusMessage()
{
    // prepare our status bar message
    string status_msg;

    int total_rows = m_Model->GetNumRows();
    status_msg = NStr::IntToString( total_rows ) + " span";
    if( total_rows != 1 ){
        status_msg += "s";
    }

    int rows_shown = m_AlnWidget->GetNumRowsVisible();
    if( rows_shown != total_rows ){
        status_msg += " total, " + NStr::IntToString( rows_shown );
        status_msg += " shown";
    }

    int rows_selected = m_AlnWidget->GetNumRowsSelected();
    if( rows_selected > 0 ){
        status_msg += ", " + NStr::IntToString( rows_selected );
        status_msg += " selected";
    }

    // TODO describe the Filter - ideally the filter itself 
    // shall provide its description (better encapsulation)

    m_StatusBar->SetStatusText( ToWxString( status_msg ) );

    // TODO - get StatusBar service for update
}


///////////////////////////////////////////////////////////////////////////////
// CAlignSpanViewFactory
//
string  CAlignSpanViewFactory::GetExtensionIdentifier() const
{
    static string sid("align_span_view_factory");
    return sid;
}


string CAlignSpanViewFactory::GetExtensionLabel() const
{
    static string slabel("Align Span View Factory");
    return slabel;
}


void CAlignSpanViewFactory::RegisterIconAliases( wxFileArtProvider& provider )
{
    string alias = GetViewTypeDescriptor().GetIconAlias();
    provider.RegisterFileAlias(ToWxString(alias), wxT("alignment_span_view.png"));
}


const CProjectViewTypeDescriptor& CAlignSpanViewFactory::GetProjectViewTypeDescriptor() const
{
    return CAlignSpanView::m_TypeDescr;
}

IView* CAlignSpanViewFactory::CreateInstance() const
{
    return new CAlignSpanView();
}


IView* CAlignSpanViewFactory::CreateInstanceByFingerprint( const TFingerprint& fingerprint ) const
{
    /*
    TFingerprint print(CAlignSpanView::m_TypeDescr.GetLabel(), false);
    if(print == fingerprint)   {
        return new CAlignSpanView();
    }
    */
    return NULL;
}


int CAlignSpanViewFactory::TestInputObjects( TConstScopedObjects& objects )
{
    bool found_good = false;
    bool found_bad = false;
    for( size_t i = 0; i < objects.size(); i++ ){
        const CObject* obj = objects[i].object;
        const type_info& type = typeid(*obj);

        found_good = false 
            //|| typeid(CSeq_annot) == type
            || typeid(CSeq_align) == type
            || typeid(CBioseq) == type
        ;
    }

    if( found_good ){
        return fCanShowSeparated | (found_bad ? fCanShowSome : fCanShowAll);
    }
    return 0; // can show nothing
}


END_NCBI_SCOPE

