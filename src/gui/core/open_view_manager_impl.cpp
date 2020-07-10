/*  $Id: open_view_manager_impl.cpp 43776 2019-08-29 19:04:06Z katargir $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software / database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software / database is freely available
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
 * Authors:  Liangshou Wu
 *
 * File Description:
 */


#include <ncbi_pch.hpp>

#include <gui/core/open_view_manager_impl.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/objutils/label.hpp>
#include <gui/objutils/obj_convert.hpp>
#include <gui/objutils/registry.hpp>

#include <objmgr/util/sequence.hpp>

#include <gui/widgets/wx/table_listctrl.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/async_call.hpp>

#include <objects/seqalign/Seq_align.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objmgr/util/sequence.hpp>
#include <serial/typeinfo.hpp>

#include <wx/statline.h>
#include <wx/valgen.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/msgdlg.h>
#include <wx/bitmap.h>
#include <wx/icon.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*
The main flow of opening view will be as follows.

User selects some data and opens OpenViewDialog. This dialog
is filled with project view factories. Factory is able to test
input objects against its view, and also it can provide
OpenViewManager (separately), that is responsible for further 
steps and pages for choosing particular objects to view.

Currently all OpenViewManagers are the same.

In the end of dialog there are "objects to show" and info about 
how to combine them together (i.e., one more array with ints, 
specifying what view to use; same numbers go to the same view).

After that there is(are) a call(s) of AddProjectView() - it is
not clear where should be this call. Now it is ProjectService
method, it seems that it is better be ViewService method.

It happens in COpenViewlDlgTask::x_Run().

As for merging vs combining, Combining should be preferred. 
So should the interface of OpenViewManager be changed.

For now, combining step should be done automatically.
*/


IMPLEMENT_DYNAMIC_CLASS( CParamsPanel, wxPanel )

BEGIN_EVENT_TABLE( CParamsPanel, wxPanel )

////@begin CParamsPanel event table entries
    EVT_CHECKBOX( ID_ONEVIEWPEROBJECT, CParamsPanel::OnOneViewPerObjectClick )

////@end CParamsPanel event table entries

END_EVENT_TABLE()


CParamsPanel::CParamsPanel()
    : m_OneViewPerObject( false )
{
}

bool CParamsPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    return wxPanel::Create( parent, id, pos, size, style );
}

bool CParamsPanel::ShowToolTips()
{
    return true;
}


wxBitmap CParamsPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CParamsPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CParamsPanel bitmap retrieval
}


wxIcon CParamsPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CParamsPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CParamsPanel icon retrieval
}


void CParamsPanel::OnOneViewPerObjectClick( wxCommandEvent& event )
{
    TransferDataFromWindow();
}

void CParamsPanel::SetObjects(const TObjectListMap& objs )
{
    m_ObjectMaps = objs;
}

bool CParamsPanel::IsCombineRequested() const
{
    return !m_OneViewPerObject;
}


void CParamsPanel::EnforceOneViewPerObject()
{
    m_OneViewPerObject = true;
    TransferDataToWindow();
}


void CParamsPanel::SetRegistryPath(const string& path)
{
    m_RegPath = path;
    //m_ListPanel->SetRegistryPath(m_RegPath + ".ObjectList");
}


static const char* kOneViewPerObject = "OneViewPerObject";

void CParamsPanel::SaveSettings() const
{
    if ( !m_RegPath.empty() ) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);
        view.Set(kOneViewPerObject, m_OneViewPerObject);
        //m_ListPanel->SaveSettings();
    }
}


void CParamsPanel::LoadSettings()
{
    if ( !m_RegPath.empty() ) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        m_OneViewPerObject = view.GetBool(kOneViewPerObject, m_OneViewPerObject);
        //m_ListPanel->LoadSettings();
        //TransferDataToWindow();
    }
}




IMPLEMENT_DYNAMIC_CLASS( CConversionResultPanel, CParamsPanel )


BEGIN_EVENT_TABLE( CConversionResultPanel, CParamsPanel )

////@begin CParamsPanel event table entries
EVT_CHECKBOX( ID_GROUPVIEW, CConversionResultPanel::OnGroupViewClick )

////@end CParamsPanel event table entries

END_EVENT_TABLE()

class CPVFactoryCombinator : public CObject, public IObjectCombinator
{
    /****** PUBLIC DEFINITION SECTION ******/
public:
    CPVFactoryCombinator( IProjectViewFactory* factory )
        : m_Factory( factory )
    {}

    virtual void operator()( const TConstScopedObjects& objects,
        TObjectsMap& objects_map ) const;

private:
    IProjectViewFactory* m_Factory;
};

void CPVFactoryCombinator::operator()(const TConstScopedObjects& objects,
                                      TObjectsMap& objects_map 
) const {
    if( m_Factory ){
        m_Factory->CombineObjects( objects, objects_map );
    }
}

CConversionResultPanel::CConversionResultPanel()
{
    Init();
}


CConversionResultPanel::CConversionResultPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


bool CConversionResultPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    ////@begin CConversionResultPanel creation
    CParamsPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
    ////@end CConversionResultPanel creation

    CIRef<IObjectCombinator> comber( new CPVFactoryCombinator( m_PVFactory ) );
    m_ListPanel->SetCombinator( comber );

    return true;
}

void CConversionResultPanel::Init()
{
    ////@begin CConversionResultPanel member initialization
    m_ListPanel = NULL;
    m_GroupCtrl = NULL;
    m_OneViewPerObjectCheckBox = NULL;
    ////@end CConversionResultPanel member initialization
}


void CConversionResultPanel::CreateControls()
{
    ////@begin CConversionResultPanel content construction
    CConversionResultPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_ListPanel = new CObjectListWidgetSel( itemPanel1, wxID_ANY, wxDefaultPosition, wxSize(100, 100), 0 );
    itemBoxSizer2->Add(m_ListPanel, 1, wxGROW|wxALL, 5);

    m_GroupCtrl = new CwxTableListCtrl( itemPanel1, wxID_ANY, wxDefaultPosition, wxSize(100, 100), 0 );
    itemBoxSizer2->Add(m_GroupCtrl, 1, wxGROW|wxALL, 5);

    m_GroupCheckBox = new wxCheckBox( itemPanel1, ID_GROUPVIEW, _("Group objects by identifying sequences"), wxDefaultPosition, wxDefaultSize, 0 );
    m_GroupCheckBox->SetValue(false);
    itemBoxSizer2->Add(m_GroupCheckBox, 0, wxALIGN_LEFT|wxLEFT|wxBOTTOM, 5);

    m_OneViewPerObjectCheckBox = new wxCheckBox( itemPanel1, ID_ONEVIEWPEROBJECT, _("Open each object in a separate view"), wxDefaultPosition, wxDefaultSize, 0 );
    m_OneViewPerObjectCheckBox->SetValue(true);
    itemBoxSizer2->Add(m_OneViewPerObjectCheckBox, 0, wxALIGN_LEFT|wxLEFT|wxBOTTOM, 5);

    // Set validators
    m_OneViewPerObjectCheckBox->SetValidator( wxGenericValidator(& m_OneViewPerObject) );
    ////@end CConversionResultPanel content construction

    m_ListPanel->SetAccessionValidator(NULL);
    m_ListPanel->SetTitle(wxT("Converted objects"));

    m_GroupCheckBox->Hide();
    m_GroupCtrl->Hide();
}

void CConversionResultPanel::OnGroupViewClick( wxCommandEvent& event )
{
    if( event.IsChecked() ){
        m_ListPanel->Hide();
        m_GroupCtrl->Show();
    } else {
        m_GroupCtrl->Hide();
        m_ListPanel->Show();
    }

    Layout();
    Refresh();
}


void CConversionResultPanel::SelectAll()
{
    m_ListPanel->SetDoSelectAll(true);
}

const char* kSelection = "Selection";

void CConversionResultPanel::SetObjects( const TObjectListMap& objs )
{
    CParamsPanel::SetObjects( objs );

    m_ListPanel->SetObjects( objs );
    m_ListPanel->TransferDataToWindow();

    // No need to do grouping for object type in seq-loc or seq-id.
    // It is too slow for a large number of objects.
    if (objs.begin()->second->GetNumRows() > 0) {
        const CSerialObject* obj =
            dynamic_cast<const CSerialObject*>(objs.begin()->second->GetObject(0));
        if (obj  &&  (obj->GetThisTypeInfo() == CSeq_id::GetTypeInfo()  ||
            obj->GetThisTypeInfo() == CSeq_loc::GetTypeInfo())) {
                return;
        }
    }
    
    vector<wxString> columns;
    columns.push_back( wxT("Objects/groups") );
    columns.push_back( wxT("Type") );
    columns.push_back( wxT("Description") );

    CTextTableModel* model = new CTextTableModel();

    m_Groups.clear();
    m_GroupSize.clear();
    m_LocHandles.clear();
    m_AlnHandles.clear();

    TObjectList objects;
    if( m_ObjectMaps.size() == 1 ){
        objects = m_ObjectMaps.begin()->second;
    } else {
        objects = m_ObjectMaps[kSelection];
    }

    size_t num_objects = objects->GetNumRows();

    m_Groups.resize( num_objects, -1 );
    m_GroupSize.resize( 2*num_objects, 0 );

    vector<CScope*> loc_scopes, aln_scopes;

    map<CSeq_id_Handle, int> find_map;

    int num_simple = 0;
    for( size_t ix = 0; ix < num_objects; ix++ ){

        const CObject* object = objects->GetObject((int)ix);
        CScope* scope = objects->GetScope((int)ix);

        const CSeq_loc* loc = dynamic_cast<const CSeq_loc*>( object );
        if( loc ){
            CSeq_id_Handle idh = sequence::GetIdHandle( *loc, scope );
            idh = sequence::GetId( idh, *scope, sequence::eGetId_Canonical );


            map<CSeq_id_Handle, int>::const_iterator it = find_map.find(idh);
//          vector<CSeq_id_Handle>::iterator found = 
//              find( m_LocHandles.begin(), m_LocHandles.end(), idh )
//          ;
//          if( found != m_LocHandles.end() ){
//                m_Groups[ix] = found - m_LocHandles.begin();
            if( it != find_map.end() ){
                m_Groups[ix] = it->second;
            } else {
                find_map[idh] = (int)m_LocHandles.size();
                m_Groups[ix] = (int)m_LocHandles.size();
                m_LocHandles.push_back( idh );
                loc_scopes.push_back( scope );
            }
            m_GroupSize[ m_Groups[ix] ]++;

            continue;
        }

        const CSeq_align* aln = dynamic_cast<const CSeq_align*>( object );
        if( aln ){
            int num_seqs = aln->CheckNumRows();
            if( num_seqs <= 0 ){
                continue;
            }

            set<CSeq_id_Handle> idh_set;
            for( int q = 0; q < num_seqs; q++ ){
                const CSeq_id& seq_id = aln->GetSeq_id( q );
                CSeq_id_Handle idh = CSeq_id_Handle::GetHandle( seq_id );
                idh = sequence::GetId( idh, *scope, sequence::eGetId_Canonical );
                idh_set.insert( idh );
            }

            vector< set<CSeq_id_Handle> >::iterator found = 
                find( m_AlnHandles.begin(), m_AlnHandles.end(), idh_set )
            ;
            if( found != m_AlnHandles.end() ){
                m_Groups[ix] = (int)(found - m_AlnHandles.begin() + num_objects);
            } else {
                m_Groups[ix] = (int)(m_AlnHandles.size() + num_objects);
                m_AlnHandles.push_back( idh_set );
                aln_scopes.push_back( scope );
            }
            m_GroupSize[ m_Groups[ix] ]++;

            continue;
        }

        num_simple++;
    }

    model->Init( columns, (int)(m_LocHandles.size() + m_AlnHandles.size() + num_simple) );

    int row = 0;
    for( size_t ix = 0; ix < m_LocHandles.size(); ix++ ){
        CSeq_id_Handle idh = sequence::GetId( m_LocHandles[ix], *loc_scopes[ix], sequence::eGetId_Best );
        if ( !idh ) {
            idh = m_LocHandles[ix];
        }

        string label = idh.GetSeqId()->GetSeqIdString();
        label += " [" 
            + NStr::IntToString( m_GroupSize[ix], NStr::fWithCommas ) 
            + " objects]"
        ;

        model->SetStringValueAt( row++, 0, wxString::FromUTF8( label.c_str() ) );
    }

    for( size_t ix = 0; ix < m_AlnHandles.size(); ix++ ){
        const set<CSeq_id_Handle>& aln_set = m_AlnHandles[ix];

        string label;

        bool first = true;
        ITERATE( set<CSeq_id_Handle>, aln_ir, aln_set ){
            if( first ){
                first = false;
            } else {
                label += " x ";
            }

            CSeq_id_Handle idh = sequence::GetId( *aln_ir, *aln_scopes[ix], sequence::eGetId_Best );
            if (!idh)
                idh = *aln_ir;
            label += idh.GetSeqId()->GetSeqIdString();
        }
        label += " [" 
            + NStr::IntToString( m_GroupSize[ix+num_objects], NStr::fWithCommas ) 
            + " objects]"
        ;

        model->SetStringValueAt( row++, 0, wxString::FromUTF8( label.c_str() ) );
    }

    for( size_t ix = 0; ix < num_objects; ix++ ){
        if( m_Groups[ix] < 0 ){
            string label;

            CLabel::GetLabel( *objects->GetObject((int)ix), &label,
                CLabel::eDefault, objects->GetScope((int)ix));

            model->SetStringValueAt( row++, 0, wxString::FromUTF8( label.c_str() ) );
            //! Important: m_Groups contains -(row+1) index for objects alone.
            m_Groups[ix] = -row;
        }
    }

    m_GroupCtrl->SetModel( model, true );
    m_GroupCtrl->SetColumnWidth( 0, 250 );
    m_GroupCtrl->TransferDataToWindow();
}

bool CConversionResultPanel::GetSelection( TConstScopedObjects& buf )
{
    if( !m_ListPanel->TransferDataFromWindow() ){
        return false;
    }

    buf.clear();

    if( m_GroupCheckBox->IsChecked() ){

        TObjectList objects;
        if( m_ObjectMaps.size() == 1 ){
            objects = m_ObjectMaps.begin()->second;
        } else {
            objects = m_ObjectMaps[kSelection];
        }

        size_t num_objects = objects->GetNumRows();

        vector<int> rows_selected;
        m_GroupCtrl->GetDataRowsSelected( rows_selected );

        for( size_t row = 0; row < rows_selected.size(); row++ ){
            int ix = rows_selected[row];

            if( ix < (int)m_LocHandles.size() ){
                for( size_t i = 0; i < num_objects; i++ ){
                    if( m_Groups[i] == ix ){
                        SConstScopedObject
                            s_obj(objects->GetObject((int)i), objects->GetScope((int)i));
                        buf.push_back(s_obj);
                    }
                }

                continue;
            }

            ix -= (int)m_LocHandles.size();
            
            if (ix < (int)m_AlnHandles.size()) {
                for (size_t i = 0; i < num_objects; i++) {
                    if (int(m_Groups[i] - num_objects) == ix) {
                        SConstScopedObject
                            s_obj(objects->GetObject((int)i), objects->GetScope((int)i));
                        buf.push_back(s_obj);
                    }
                }

                continue;
            }

            ix += (int)m_LocHandles.size();

            for( size_t i = 0; i < num_objects; i++ ){
                if( m_Groups[i] == -(ix+1) ){
                    SConstScopedObject
                        s_obj(objects->GetObject((int)i), objects->GetScope((int)i));
                    buf.push_back(s_obj);
                }
            }

        }

    } else {
        if( m_ListPanel ){
            buf = m_ListPanel->GetSelection();
        }
    }

    return ( ! buf.empty());
}

bool CConversionResultPanel::GetSelectedIndices( vector<int>& buf )
{
    buf.clear();

    if( m_GroupCheckBox->IsChecked() ){

        TObjectList objects;
        if( m_ObjectMaps.size() == 1 ){
            objects = m_ObjectMaps.begin()->second;
        } else {
            objects = m_ObjectMaps[kSelection];
        }

        size_t num_objects = objects->GetNumRows();

        vector<int> rows_selected;
        m_GroupCtrl->GetDataRowsSelected( rows_selected );

        for( size_t row = 0; row < rows_selected.size(); row++ ){
            int ix = rows_selected[row];

            if( ix < (int)m_LocHandles.size() ){
                for( size_t i = 0; i < num_objects; i++ ){
                    if( m_Groups[i] == ix ){
                        buf.push_back( m_Groups[i] );
                    }
                }

                continue;
            }

            ix -= (unsigned)m_LocHandles.size();
            
            if( ix < (int)m_AlnHandles.size() ){
                for( size_t i = 0; i < num_objects; i++ ){
                    if( int(m_Groups[i] - num_objects) == ix ){
                        buf.push_back( m_Groups[i] );
                    }
                }

                continue;
            }

            // No need to process single objects
        }
    }

    return !buf.empty();
}


void CConversionResultPanel::EnforceOneViewPerObject()
{
    CParamsPanel::EnforceOneViewPerObject();

    m_OneViewPerObjectCheckBox->Disable();
}


void CConversionResultPanel::SetRegistryPath(const string& path)
{
    CParamsPanel::SetRegistryPath( path );

    m_ListPanel->SetRegistryPath(m_RegPath + ".ObjectList");
}


void CConversionResultPanel::SaveSettings() const
{
    CParamsPanel::SaveSettings();

    if (!m_RegPath.empty()) {
        m_ListPanel->SaveSettings();
    }
}


void CConversionResultPanel::LoadSettings()
{
    CParamsPanel::LoadSettings();

    if (!m_RegPath.empty()) {
        m_ListPanel->LoadSettings();
    }
}


COpenViewManager::COpenViewManager()
    : m_Factory( NULL )
    , m_Descriptor( NULL )
{
}


COpenViewManager::COpenViewManager( IProjectViewFactory& factory )
    : m_Factory( &factory )
    , m_Descriptor( &factory.GetProjectViewTypeDescriptor() )
{
}

void COpenViewManager::SetProjectViewFactory( IProjectViewFactory* factory )
{
    m_Factory = factory;
    m_Descriptor = factory ? &factory->GetProjectViewTypeDescriptor() : NULL;
}

void COpenViewManager::SetParentWindow(wxWindow* parent)
{
    m_ParentWindow = parent;
}

void COpenViewManager::InitUI()
{
    m_State = eInvalid;
    m_ParamPanel = NULL;
}

void COpenViewManager::CleanUI()
{
    // clear the panels
    m_State = eInvalid;
    m_ParamPanel = NULL;

    // clear the essential objects
    m_InputObjects.clear();
    m_OutputObjects.clear();
    m_OutputIndices.clear();
}

void COpenViewManager::SetInputObjects( const TConstScopedObjects& input )
{
    m_InputObjects = input;

    m_OutputObjects.clear();
    m_OutputIndices.clear();
}


const TConstScopedObjects& COpenViewManager::GetOutputObjects() const
{
    return m_OutputObjects;
}

const vector<int> COpenViewManager::GetOutputIndices() const
{
    _ASSERT(m_Factory);
    if( !m_Factory ) return vector<int>();

    if( !m_OutputIndices.empty() ){
        return m_OutputIndices;
    }

    if( mf_IsCombineRequested ){
       m_OutputIndices = m_Factory->CombineInputObjects( m_OutputObjects );
    }

    return m_OutputIndices;
}


wxPanel* COpenViewManager::GetCurrentPanel()
{
    if (m_State == eParams) {
        return m_ParamPanel;
    }
    return NULL;
}


bool COpenViewManager::AtFinalStep()
{
    return (m_State == eParams);
}


bool COpenViewManager::IsCompletedState()
{
    return (m_State == eCompleted);
}


bool COpenViewManager::CanDo(EAction action)
{
    switch (m_State) {
    case eInvalid:
        return action == eNext;
    case eParams:
        return action == eNext ||  action == eBack;
    case eCompleted:
        return false; // nothing left to do
    default:
        _ASSERT(false);
        return false;
    }
}

bool COpenViewManager::NeedParamsPanel( const TObjectListMap& objects )
{
    TObjectListMap::const_iterator it = objects.find(kSelection);
    if (it != objects.end() && it->second->GetNumRows() == 1) return false;

    if( objects.size() > 1 ||
        (objects.size() == 1 && objects.begin()->second->GetNumRows() > 1)) {
        return true;

    } else if( objects.begin()->second->GetNumRows() == 1 ){
        return false;

    } else {    
        NCBI_THROW( COVMException, eGroupErr, "Input objects are not suitable to be seen" );
    }
}

CParamsPanel* COpenViewManager::CreateParamsPanel()
{
    CConversionResultPanel* panel = new CConversionResultPanel();
    panel->SetProjectViewFactory( m_Factory );

    return panel;
}

void COpenViewManager::FinalizeParams()
{
    m_State = eCompleted;
}


void COpenViewManager::x_LoadParamPanelSettings()
{
    if (m_ParamPanel) {
        m_ParamPanel->SetRegistryPath(m_RegPath + "." +
                                      m_Descriptor->GetLabel() + ".ParamPanel");
        m_ParamPanel->LoadSettings();
    }
}

#define MAX_NUM_VIEWS_SAFE 10

bool COpenViewManager::DoTransition( EAction action )
{
    _ASSERT(m_Factory);
    if(!m_Factory) return false;

    wxBusyCursor wait;

    if( m_State == eInvalid  &&  action == eNext ){
        // do conversion
        TObjectListMap object_map;
        x_DoConversion( object_map );

        if( object_map.empty() || object_map.begin()->second->GetNumRows() == 0 ){
            wxMessageBox(
                wxT("The selections can't be opened in the selected view!"),
                wxT("Open View"), wxOK | wxICON_EXCLAMATION
            );

            return false;
        }

        try {
            if( NeedParamsPanel( object_map ) ){
                m_ParamPanel = CreateParamsPanel();
                m_ParamPanel->Hide(); // to reduce flicker
                m_ParamPanel->Create( m_ParentWindow );
                x_LoadParamPanelSettings();

                if( m_Descriptor->GetObjectsAccepted() <= eOneObjectAccepted ){
                    m_ParamPanel->EnforceOneViewPerObject();
                } else if( m_Descriptor->GetObjectsAccepted() == eAnyObjectsAccepted ){
                    m_ParamPanel->SelectAll();
                }

                m_ParamPanel->SetObjects( object_map );

                m_State = eParams;

            } else {
                TObjectListMap::const_iterator it = object_map.find(kSelection);
                if (it == object_map.end()) it = object_map.begin();

                TObjectList objects = it->second;
                int num_objects = objects->GetNumRows();
                for( int idx = 0; idx < num_objects; ++idx ){
                    SConstScopedObject s_obj(objects->GetObject(idx), objects->GetScope(idx));
                    m_OutputObjects.push_back( s_obj );
                }
                mf_IsCombineRequested = true;

                FinalizeParams();
            }
        
        } catch( COVMException& ){
            wxMessageBox(
                wxT("The input object is not compatible with the selected view."),
                wxT("Open View"), wxOK | wxICON_EXCLAMATION
            );
            return false;

        } catch( CException& ex ){
            wxMessageBox(
                wxT("Particular kind of this input object is not supported:\n")
                + ToWxString( ex.GetMsg() ),
                wxT("Open View"), wxOK | wxICON_EXCLAMATION
            );
            return false;
        }

        return true;

    } else if (m_State == eParams) {
        if( action == eNext ){
            if( !m_ParamPanel->GetSelectedObjects( m_OutputObjects ) ){
                wxMessageBox(
                    wxT("Please select at least one object."),
                    wxT("Open View"), wxOK | wxICON_EXCLAMATION
                );
                return false;
            }

            mf_IsCombineRequested = m_ParamPanel->IsCombineRequested();

            int num_groups = 0;
            if( mf_IsCombineRequested ){
                //vector<int> indices = GetOutputIndices();
                m_ParamPanel->GetSelectedIndices( m_OutputIndices );
                NON_CONST_ITERATE( vector<int>, ix_trv, m_OutputIndices ){
                    vector<int>::iterator found = find( m_OutputIndices.begin(), ix_trv, *ix_trv );
                        
                    if( found == ix_trv ){
                        num_groups++;
                    }
                }
            } else {
                num_groups = (unsigned)GetOutputObjects().size();
            }

            if( num_groups > MAX_NUM_VIEWS_SAFE ){ 
                string message = "You are about to open more than ";
                message += NStr::IntToString( MAX_NUM_VIEWS_SAFE );
                message += " views at a time.\nAre you sure?";
                if(
                    NcbiMessageBox( message, eDialog_OkCancel, eIcon_Exclamation, "Attention!" )
                    != eOK
                ){
                    return false;
                }
            } 

            m_ParamPanel->SaveSettings();
            FinalizeParams();

            return true;

        } else if( action == eBack ){
            m_ParamPanel->SaveSettings();
            m_State = eInvalid;

            return true;
        }
    }

    _ASSERT(false);
    return false;
}

void COpenViewManager::x_DoConversion( TObjectListMap& objects )
{
    try {
        GUI_AsyncExec([this, &objects](ICanceled& cancel) { this->DoAsyncConversion(objects, cancel); },
                       wxT("Doing object conversion..."));
    }
    NCBI_CATCH("COpenViewManager::x_DoConversion");
}


struct SCompareSeqLoc {
    bool operator()(const CSeq_loc* loc1, const CSeq_loc* loc2)
    {
        return loc1->Compare(*loc2) < 0;
    }
};


void COpenViewManager::DoAsyncConversion( TObjectListMap& objects, ICanceled& cancel )
{
    const string target_type = m_Descriptor->GetPrimaryInputType();
    set<const CObject*> obj_ptrs;
    // special case for seq-loc
    // make sure no two identical location are added
    typedef set<const CSeq_loc*, SCompareSeqLoc> TLocPtrs;
    typedef map<CSeq_id_Handle, TLocPtrs> TLocMap;
    TLocMap loc_map;

    bool need_extra_col = m_InputObjects.size() == 1;
    if (need_extra_col) {
        const CSerialObject* obj =
            dynamic_cast<const CSerialObject*>(m_InputObjects.front().object.GetPointer());
        if (!obj  ||  obj->GetThisTypeInfo() != CGC_Assembly::GetTypeInfo()){
            need_extra_col = false;
        }
    }

    for (auto& i : m_InputObjects) {
        if (cancel.IsCanceled()) return;

        const CObject* obj = i.object;
        CScope* scope = const_cast<CScope*>(i.scope.GetPointer());
        if (!scope || !obj) continue;

        if (m_Factory->IsCompatibleWith(*obj, *scope)) {
            TObjectList& obj_list = objects[kSelection];
            if (!obj_list)  obj_list.Reset(new CObjectList);
            obj_list->AddRow(const_cast<CObject*>(obj), scope);
            obj_ptrs.insert(obj);

            const CSeq_loc* loc = dynamic_cast<const CSeq_loc*>(obj);
            if (loc) {
                CSeq_id_Handle idh = sequence::GetIdHandle(*loc, scope);
                loc_map[idh].insert(loc);
            }
        }
    }


    for (auto& i : m_InputObjects) {
        if (cancel.IsCanceled()) return;

        const CObject* obj = i.object;
        CScope* scope = const_cast<CScope*>(i.scope.GetPointer());
        if (!scope || !obj) continue;

        CObjectConverter::TRelationVector relations;
        CObjectConverter::FindRelations(*scope, *obj, target_type, relations);

        NON_CONST_ITERATE(CObjectConverter::TRelationVector, itr, relations) {
            if (cancel.IsCanceled()) return;

            /// If there are more than one relation, the first one contains
            /// all the conversions, and the rest contain categorized 
            /// conversions.
            if (relations.size() > 1 && itr == relations.begin()) continue;

            CRelation::TObjects converts;
            CRelation& rel = **itr;
            string relName = rel.GetProperty(target_type);

            CRelation::TObjects related;
            rel.GetRelated(*scope, *obj, related, CRelation::fConvert_NoExpensive, &cancel);

            converts.reserve(related.size());
            ITERATE( CRelation::TObjects, obj_iter, related ){
                if (cancel.IsCanceled()) return;

                const CObject* converted_obj = &obj_iter->GetObject();

                if( obj_ptrs.insert( converted_obj ).second ){
                    bool valid = true;
                    const CSeq_loc* loc = dynamic_cast<const CSeq_loc*>(converted_obj);
                    if (loc) {
                        CSeq_id_Handle idh = sequence::GetIdHandle(*loc, scope);
                        if ( !loc_map[idh].insert(loc).second ) {
                            valid = false;
                        }
                    }

                    if(valid  &&  m_Factory->IsCompatibleWith(*converted_obj, *scope)){
                        converts.push_back(*obj_iter);
                    } 
                }
            }

            if( !converts.empty() ){
                TObjectList& obj_list = objects[relName];
                if ( !obj_list ) {
                    obj_list.Reset(new CObjectList);
                    if (need_extra_col) {
                        obj_list->AddColumn( CObjectList::eString, "Chromosome" );
                    }
                }
                ITERATE (CRelation::TObjects, obj_iter, converts) {
                    int row_num = obj_list->AddRow(const_cast<CObject*>(obj_iter->GetObjectPtr()), scope);
                    if (need_extra_col) {
                        obj_list->SetString( 0, row_num, obj_iter->GetComment() );
                    }
                }
            }
        }
    }
}


void COpenViewManager::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use
}


void COpenViewManager::SaveSettings() const
{
    if( ! m_RegPath.empty())   {
        /// save conversion parameters
        if(m_ParamPanel)  {
            m_ParamPanel->SaveSettings();
        }
    }
}


void COpenViewManager::LoadSettings()
{
    if( ! m_RegPath.empty())   {
        /// save conversion parameters
        if(m_ParamPanel)  {
            x_LoadParamPanelSettings();
        }
    }
}

END_NCBI_SCOPE

