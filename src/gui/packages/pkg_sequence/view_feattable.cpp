/*  $Id: view_feattable.cpp 40880 2018-04-25 20:26:58Z katargir $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/packages/pkg_sequence/view_feattable.hpp>

#include <gui/framework/workbench.hpp>
#include <gui/core/project_service.hpp>
#include <gui/objutils/visible_range.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/objutils/label.hpp>
#include <gui/objutils/user_type.hpp>

#include <corelib/ncbistre.hpp>

#include <serial/typeinfo.hpp>

#include <gui/types.hpp>

#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <objects/seqloc/Seq_interval.hpp>
#include <objmgr/util/sequence.hpp>

#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/statusbr.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


CProjectViewTypeDescriptor s_FeatTableViewTypeDescr(
    "Feature Table View", // type name
    "feature_table_view", // icon alias TODO
    "Show tabular information about features",
    "The Feature Table View provides a tabular break-down of features for the sequence",
    "FEAT_TABLE_VIEW", // help ID
    "Sequence",   // category
    false,       // not a singleton
    "Seq-loc",
    eSimilarObjectsAccepted
);

/*
    info.AddApplicableUserType(CGUIUserType::sm_Tp_Annotation, CGUIUserType::sm_SbTp_Features);
    info.AddApplicableUserType(CGUIUserType::sm_Tp_Feature);
    info.AddApplicableUserType(CGUIUserType::sm_Tp_Location);
    info.AddApplicableUserType(CGUIUserType::sm_Tp_Sequence);
    info.AddApplicableUserType(CGUIUserType::sm_Tp_Sequence_ID);
    info.AddApplicableUserType(CGUIUserType::sm_Tp_Sequence_Set);
    info.AddApplicableUserType(CGUIUserType::sm_Tp_EntrezGeneRecord, "");
    // command info
    CPluginCommand& args = info.SetViewCommand(CPluginCommand::eCommand_new_view);
    args.AddArgument("loc", "Location to display",
                     CSeq_loc::GetTypeInfo());

    CPluginArg& arg =
        args.AddOptionalArgument("annot", "Limit to Annotation",
                                 CSeq_annot::GetTypeInfo());
    arg.SetHidden(true);
*/

enum    EFeatTableCommands {
    eCmdFind = 13000,
    eCmdSelectActiveColumns,
    eCmdVisibleRangeChange,
    eCmdFilter,
    //eCmdSettings,
    eCmdExportToCSV
};

/*
static
DEFINE_MENU(ViewMenu)
    SUBMENU("&View")
        SUBMENU(VIEW_UI_NAME)
            MENU_ITEM(eCmdFind, "Find...")
            MENU_ITEM(eCmdSelectActiveColumns, "Select Active Columns")
            MENU_ITEM(eCmdVisibleRangeChange, "Visible Range Change...")
            MENU_ITEM(eCmdFilter, "Filter...")
            MENU_ITEM(eCmdExportToCSV, "Export as Spreadsheet")
            //MENU_SEPARATOR()
            //MENU_ITEM(eCmdSettings, "Settings...")
        END_SUBMENU()
    END_SUBMENU()
END_MENU()


static
DEFINE_MENU(PopupMenu)
    MENU_ITEM(eCmdFind, "Find...")
    MENU_ITEM(eCmdSelectActiveColumns, "Select Active Columns")
    MENU_ITEM(eCmdVisibleRangeChange, "Visible Range Change...")
    MENU_ITEM(eCmdFilter, "Filter...")
    MENU_ITEM(eCmdExportToCSV, "Export as Spreadsheet")
    //MENU_SEPARATOR()
    //MENU_ITEM(eCmdSettings, "Settings...")
END_MENU()
*/

///////////////////////////////////////////////////////////////////////////////
/// CSelFeatHandle
class CSelFeatHandle : public ISelObjectHandle
{
public:
    CSelFeatHandle( const CFeatTableDS::SFeatureRow& feat_row ) : m_FeatRow( feat_row ) {}
    const CFeatTableDS::SFeatureRow& GetFeatureRow() const { return m_FeatRow; }

protected:
    const CFeatTableDS::SFeatureRow& m_FeatRow;
};


///////////////////////////////////////////////////////////////////////////////
///  CFeatTableView
CFeatTableView::CFeatTableView()
    : m_Panel()
    , m_StatusBar()
    , m_FTblWidget()
    , m_DataSource( new CFeatTableDS() )
{
}

wxWindow* CFeatTableView::GetWindow()
{
    _ASSERT(m_Panel);
    return m_Panel;
}

void CFeatTableView::x_InitMenuResources()
{
    static bool initialized = false;

    // this code should execute only once
    if( ! initialized ){
        // register commands
        //CUICommandRegistry& cmd_reg = m_Workbench->GetUICommandRegistry();
        //cmd_reg->RegisterCommand( ... );

        // register icons
        //wxFileArtProvider& provider = m_Workbench->GetFileArtProvider();
        //provider.RegisterFileAlias( ... );

        initialized = true;
    }
}

void CFeatTableView::x_CreateMenuBarMenu()
{
    x_InitMenuResources();

    //CUICommandRegistry& cmd_reg = m_Workbench->GetUICommandRegistry();

    //m_MenuBarMenu.reset( new wxMenu() );

    /* TODO commands
    wxMenu* view_menu = new wxMenu();
    wxMenu* dm_menu = new wxMenu();

    cmd_reg->AppendMenuItem(*dm_menu, eCmdChooseSeq);
    cmd_reg->AppendMenuItem(*dm_menu, eCmdColorByScore);

    view_menu->Append(wxID_ANY, wxT("Dot Matrix"), dm_menu);
    m_MenuBarMenu->Append(wxID_ANY, wxT("&View"), view_menu);

    // create menu
    m_RootItem.reset(CreateMenuItems(ViewMenu));

    // init view icon
    CRef<CResourceManager> mgr = CResourceManager::GetInstance();
    m_Icon = mgr->GetImage(VIEW_ICON_ALIAS);
    */
}


// override function to suppress Navigation toolbar
void CFeatTableView::GetCompatibleToolBars(vector<string>& names)
{
    names.push_back(CProjectViewToolBatFactory::sm_SelTBName);
}


void CFeatTableView::CreateViewWindow(wxWindow* parent)
{
    _ASSERT( !m_FTblWidget );

    m_Panel = new wxPanel( parent );

    // main sizer
    wxBoxSizer* main_sizer = new wxBoxSizer( wxVERTICAL );
    m_Panel->SetSizer( main_sizer );

    m_FTblWidget = new CFeatTableWidget( m_Panel );
    m_FTblWidget->SetDataSource( m_DataSource );

    m_DataSource->AddListener(this);

    AddListener( m_FTblWidget, ePool_Child );
    m_FTblWidget->AddListener( this, ePool_Parent );

    main_sizer->Add( m_FTblWidget, 1, wxEXPAND );

    // wxStatusBar
    m_StatusBar = new wxStatusBar( m_Panel );
    main_sizer->Add( m_StatusBar, 0, wxEXPAND );

    //m_Panel->PushEventHandler( new CCommandTransferHandler( m_AlnWidget ) );
    m_Panel->PushEventHandler( new CCommandToFocusHandler( m_Panel ) );
}


void CFeatTableView::DestroyViewWindow()
{
    if( GetWindow() ){
        m_Panel->PopEventHandler( true );

        m_DataSource->RemoveListener(this);

        m_FTblWidget->RemoveModel();

        GetWindow()->Destroy();
        m_Panel = NULL;
        m_FTblWidget = NULL;
        m_StatusBar = NULL;
    }
}


void CFeatTableView::LoadSettings()
{
    /// restore our table's settings
    CGuiRegistry& reg = CGuiRegistry::GetInstance();
    CRegistryReadView view = reg.GetReadView(m_RegPath);
    m_FTblWidget->LoadTableSettings(view);
}


void CFeatTableView::SaveSettings() const
{
    /// save our table's settings
    CGuiRegistry& reg = CGuiRegistry::GetInstance();
    CRegistryWriteView view = reg.GetWriteView(m_RegPath);
    m_FTblWidget->SaveTableSettings(view);
}


const CViewTypeDescriptor& CFeatTableView::GetTypeDescriptor() const
{
    return s_FeatTableViewTypeDescr;
}

CScope* CFeatTableView::x_PreAttachToProject( TConstScopedObjects& objects )
{
    if( objects.size() != 1 ){
        return NULL;
    }

    const CObject* object = objects[0].object.GetPointer();
    CScope* scope = objects[0].scope.GetPointer();

    for(;;){
        const CSeq_annot* annot = dynamic_cast<const CSeq_annot*>(object);
        if( annot ){
            //! We can fetch CSeq_annot_Handle here.
            m_OrigObject.Reset( annot );
            break;
        }

        const CSeq_id* seq_id = dynamic_cast<const CSeq_id*>(object);
        if( seq_id ){
            CRef<CSeq_loc> loc( new CSeq_loc( CSeq_loc::e_Whole ) );
            loc->SetId( *seq_id );

            m_OrigObject.Reset( loc );
            break;
        }

        const CSeq_loc* seq_loc = dynamic_cast<const CSeq_loc*>(object);
        if( seq_loc ){
            m_OrigObject.Reset( seq_loc );
            break;
        }

        break;
    }

    m_Scope.Reset( scope );

    return m_OrigObject.IsNull() ? NULL : scope;
}

void CFeatTableView::x_PostAttachToProject()
{    
    if( x_HasProject() ){
        m_DataSource->Init( *m_OrigObject, *m_Scope );

        //m_Sel = CSeqUtils::GetAnnotSelector( CSeq_annot::TData::e_Ftable );
        //m_DataSource->SetSelector( m_Sel );

        // LoadSettings();
    }

    OnProjectChanged();
}




bool CFeatTableView::InitView(TConstScopedObjects& objects, const objects::CUser_object* params)
{
    x_MergeObjects( objects );

    return CProjectView::InitView( objects, params );
}

void CFeatTableView::OnProjectChanged( CProjectViewEvent& evt )
{
    if( evt.GetSubtype() & CProjectViewEvent::eData ){
        OnProjectChanged();
    }
}   


void CFeatTableView::OnProjectChanged()
{
    x_Update();
    x_UpdateContentLabel();
}


const CObject* CFeatTableView::x_GetOrigObject() const
{
    return m_OrigObject.GetPointer();
}

BEGIN_EVENT_MAP(CFeatTableView, CProjectView)
    ON_EVENT(CFeatTableEvent, CFeatTableEvent::eStatusChange, &CFeatTableView::x_OnFeatTableEvent)
END_EVENT_MAP()

// handles incoming selection broadcast (overriding CView virtual function)
void CFeatTableView::x_OnSetSelection( CSelectionEvent& evt )
{
    //m_StatusBar->SetMessage( "No features." );

    if( evt.HasObjectSelection() ){ // set Object Selection
        m_FTblWidget->ClearSelection();

        TConstObjects sel_objs;
        evt.GetAllObjects( sel_objs );

        CScope& scope = *m_DataSource->GetScope();

        ITERATE( TConstObjects, it_obj, sel_objs ){
            const CObject& obj = **it_obj;

            CObjectIndex::TResults results;
            m_ObjectIndex.GetMatches( obj, scope, results );

            ITERATE( CObjectIndex::TResults, it_r, results ){
                const CSelFeatHandle* fh = (const CSelFeatHandle*)(*it_r);
                m_FTblWidget->SelectDataRow( fh->GetFeatureRow().row_idx );
            }
        }

        //m_FTblWidget->ScrollSelectionToVisible();

        x_UpdateStatusMessage(); // prepare our status bar message
    }
}

void CFeatTableView::OnWidgetSelChanged()
{
    CProjectView::OnWidgetSelChanged();
    x_UpdateStatusMessage();
}

void CFeatTableView::OnWidgetRangeChanged()
{
    //CProjectView::OnWidgetRangeChanged();
    x_UpdateStatusMessage();
}

void CFeatTableView::x_SetVisibleRange(const CSeq_loc& loc)
{
    m_FTblWidget->SetVisibleRange(loc);
}

void CFeatTableView::x_Update()
{
    if(m_FTblWidget  &&  x_HasProject())    {
        m_StatusBar->SetStatusText( wxT("Searching for features...") );

        m_FTblWidget->Update();

        // prepare our status bar message
        //x_UpdateStatusMessage();
    }
}


void CFeatTableView::x_UpdateStatusMessage()
{
    // prepare our status bar message
    string status_msg;

    int total_feats = m_DataSource->GetNumRows();
    status_msg = NStr::IntToString( total_feats ) + " feature";
    if( total_feats != 1 ){
        status_msg += "s";
    }

    int visible_feats = m_FTblWidget->GetNumRowsVisible();
    if( visible_feats != total_feats ){
        status_msg += ", " + NStr::IntToString( visible_feats );
        status_msg += " filtered";
    }

    int sel_num = m_FTblWidget->GetSelectedItemCount();
    if( sel_num > 0 ){
        status_msg += ", " + NStr::IntToString(sel_num);
        status_msg += " selected";
    }

    m_StatusBar->SetStatusText( ToWxString( status_msg ) );
}

void CFeatTableView::x_OnFeatTableEvent(CEvent*)
{
    x_UpdateStatusMessage();
}

void CFeatTableView::GetSelection( TConstScopedObjects& objs ) const
{
    if( m_FTblWidget  &&  x_HasProject() ){
        CScope* scope = x_GetScope();

        // add Object selections
        TConstObjects sel_objs;
        m_FTblWidget->GetSelection( sel_objs );

        ITERATE( TConstObjects, it, sel_objs ){
            objs.push_back( SConstScopedObject(*it, scope) );
        }
    }
}


void CFeatTableView::GetSelection( CSelectionEvent& evt ) const
{
    if (x_HasProject()) {
        // Object Selection
        TConstObjects sel_objs;
        m_FTblWidget->GetSelection( sel_objs );
        evt.AddObjectSelection( sel_objs );
    }
}


//rebuild index  that is used for selection handling

void CFeatTableView::x_RebuildObjectIndex()
{
    if( m_DataSource ) {
        CScope& scope = *m_DataSource->GetScope();
        m_ObjectIndex.Clear( &scope );

        for( int row = 0; row < m_DataSource->GetNumRows(); ++row ){
            const CFeatTableDS::SFeatureRow& sfeat = m_DataSource -> GetFeatureRow( row );
            const CSeq_feat& feat = m_DataSource -> GetFeature( row );
            m_ObjectIndex.Add( new CSelFeatHandle( sfeat ), feat );
        }
    }
}

void CFeatTableView::GetVisibleRanges( CVisibleRange& vrange ) const
{
    //TSeqRange range = m_FTblWidget->GetSelectionVisibleRange();

    /* !TBD
    CRef<CSeq_loc> loc( new CSeq_loc() );
    loc->SetInt().SetFrom( range.GetFrom() );
    loc->SetInt().SetTo( range.GetTo() );
    loc->SetId( * m_OrigLoc -> GetId() );
    vrange.AddLocation( *loc );
    */
}


void CFeatTableView::SetRegistryPath(const string& reg_path)
{
    m_RegPath = reg_path;
}


/*
void CFeatTableView::OnSettings()
{
    CFeatTablePreferenceDialog dlg;
    dlg.CenterOnActive();

    CFeatTableWidget::EVisibleRangeMethod method;

    // set dialog with current setting - Default is Scroll To
    method = m_FTblWidget->GetVisibleRangeMethod();
    if( method == CFeatTableWidget::eEntirelyContained ){
        dlg.SetEntirelyContained();
    } else if( method == CFeatTableWidget::eIntersection ){
        dlg.SetIntersection();
    } else {
        dlg.SetScrollTo();
    }

    if( dlg.ShowModal() ){
        // visible range method
        if (dlg.EntirelyContainedMethod()) {
            method = CFeatTableWidget::eEntirelyContained;
        } else if (dlg.ScrollToMethod()) {
            method = CFeatTableWidget::eScrollTo;
        } else if (dlg.IntersectionMethod()) {
            method = CFeatTableWidget::eIntersection;
        }

        m_FTblWidget->SetVisibleRangeMethod( method );

        //? m_DataSource->SetSelector(m_Sel);
    }
}
*/

///////////////////////////////////////////////////////////////////////////////
/// CFeatTableViewFactory
string  CFeatTableViewFactory::GetExtensionIdentifier() const
{
    static string sid("feature_table_view_factory");
    return sid;
}

string CFeatTableViewFactory::GetExtensionLabel() const
{
    static string slabel("Feature Table View Factory");
    return slabel;
}


void CFeatTableViewFactory::RegisterIconAliases(wxFileArtProvider& provider)
{
    string alias = GetViewTypeDescriptor().GetIconAlias();
    provider.RegisterFileAlias(
        ToWxString(alias), wxT("feature_table_view.png")
    );
}


const CProjectViewTypeDescriptor& CFeatTableViewFactory::GetProjectViewTypeDescriptor() const
{
    return s_FeatTableViewTypeDescr;
}


IView* CFeatTableViewFactory::CreateInstance() const
{
    return new CFeatTableView();
}


IView* CFeatTableViewFactory::CreateInstanceByFingerprint(const TFingerprint& fingerprint) const
{
    return NULL;
}

bool CFeatTableViewFactory::IsCompatibleWith( const CObject& object, objects::CScope& )
{
    const type_info& type = typeid(object);

    if( typeid(CSeq_loc) == type ) return true;
    if( typeid(CSeq_id) == type ) return true;

    if( typeid(CSeq_annot) == type ){
        const CSeq_annot& annot = dynamic_cast<const CSeq_annot&>(object);
        return annot.IsFtable();
    }

    return false;
}


int CFeatTableViewFactory::TestInputObjects( TConstScopedObjects& objects )
{
    bool found_good = false;
    bool found_bad = false;
    for( size_t i = 0; i < objects.size(); i++ ){
        if( IsCompatibleWith( *objects[i].object, *objects[i].scope ) ){
            found_good = true;
        } else {
            found_bad = true;
        }
    }

    if( found_good ){
        return fCanShowSeparated | (found_bad ? fCanShowSome : fCanShowAll);
    }
    return 0; // can show nothing
}


bool CFeatTableView::x_MergeObjects( TConstScopedObjects& objects )
{
    if( objects.size() < 2 ){
        return true;
    }
    // verify that all the objects has the same seq-id
    typedef map<CSeq_id_Handle, TSeqRange> TRanges;
    TRanges ranges;
    ITERATE( TConstScopedObjects, iter, objects ){
        const CSeq_loc* loc = dynamic_cast<const CSeq_loc*> (iter->object.GetPointer());
        if (loc) {
            CScope* scope = const_cast<CScope*>(iter->scope.GetPointer());
            CSeq_id_Handle idh = sequence::GetIdHandle(*loc, scope);
            TSeqRange range = loc->GetTotalRange();
            ranges[idh] += range;
        }
    }
    if (ranges.size() == 1) {
        CRef<objects::CScope> scope = objects[0].scope;
        CRef<CSeq_loc> loc(new CSeq_loc);
        TRanges::iterator iter = ranges.begin();
        loc->SetInt().SetFrom(iter->second.GetFrom());
        loc->SetInt().SetTo  (iter->second.GetTo());
        loc->SetId(*iter->first.GetSeqId());

        objects.clear();
        objects.push_back(
            SConstScopedObject(loc, scope)
        );
        return true;
    } else {
        return false;
    }
}


END_NCBI_SCOPE
