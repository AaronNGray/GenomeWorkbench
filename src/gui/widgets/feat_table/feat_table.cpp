/*  $Id: feat_table.cpp 38774 2017-06-16 15:57:03Z katargir $
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
 * Authors: Yury Voronov
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/feat_table/feat_table.hpp>
#include <gui/widgets/feat_table/feature_filter_dlg.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/objutils/label.hpp>
#include <gui/utils/view_event.hpp>

#include <objmgr/feat_ci.hpp>
#include <objects/seqloc/Seq_bond.hpp>
#include <objects/seqloc/Seq_interval.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqloc/Seq_loc_equiv.hpp>
#include <objects/seqloc/Seq_point.hpp>
#include <objmgr/util/feature.hpp>
#include <objmgr/util/sequence.hpp>

//#include <gui/widgets/feat_table/filter_feature.hpp>


BEGIN_NCBI_SCOPE

USING_SCOPE(objects);

enum EFTCommands   {
    eCmdFilter = 20000, 
    eEndOfCmds
};


BEGIN_EVENT_TABLE( CFeatTableWidget, CwxTableListCtrl )
    EVT_LIST_ITEM_SELECTED( wxID_ANY, CFeatTableWidget::OnSelectionChanged )
    EVT_LIST_ITEM_DESELECTED( wxID_ANY, CFeatTableWidget::OnSelectionChanged )

    EVT_MENU( eCmdFilter, CFeatTableWidget::OnFilter )
END_EVENT_TABLE()

/////////////////////////////////////////////////////////////////////////////
/// CFeatTableWidget

CFeatTableWidget::CFeatTableWidget(
    wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style
)
    : CwxTableListCtrl( parent, id, pos, size, style )
{
    m_DataSource.Reset( new CFeatTableDS() );

    SetModel( m_DataSource );
}

CFeatTableWidget::~CFeatTableWidget()
{
    IRowFilter* filter = RemoveFilter();
    if( filter ){
        delete filter;
    }


}

void CFeatTableWidget::SetDataSource( CFeatTableDS* new_ds )
{
    if( new_ds != m_DataSource.GetPointer() ){
        RemoveModel();

        m_DataSource.Reset( new_ds );
        m_DataSource->AddTMListener( *this );

        SetModel( new_ds );

        SFeatureFilter* filter = new SFeatureFilter();
        SetFilter( filter );
    }
}

void CFeatTableWidget::Update()
{
    m_DataSource->Update();
}

void CFeatTableWidget::AppendMenuItems( wxMenu& aMenu )
{
    CwxTableListCtrl::AppendMenuItems( aMenu );

    aMenu.Append( wxID_SEPARATOR, wxT("Actions") );
    aMenu.Append( eCmdFilter, wxT( "Filter..." ) );
}

void CFeatTableWidget::SetVisibleRangeMethod(EVisibleRangeMethod method)
{
    m_VisibleRangeMethod = method;
}

CFeatTableWidget::EVisibleRangeMethod CFeatTableWidget::GetVisibleRangeMethod() const
{
    return m_VisibleRangeMethod;
}


/// select a given feature
void CFeatTableWidget::SetVisibleRange( const CSeq_loc& loc )
{
    /*
    int start_pos = loc.GetStart( eExtreme_Positional );
    int stop_pos = loc.GetStop( eExtreme_Positional );

    int start_ix = 0;
    for( ; start_ix < m_Table -> GetNumRowsVisible(); start_ix++ ){
        int from = m_Table -> GetValueVisibleAt( start_ix, CFeatTableDS::eFrom ).GetValue<int>();
        if( from >= start_pos ){
            break;
        }
    }

    int stop_ix = m_Table -> GetNumRowsVisible() -1;
    for( ; stop_ix >= 0; stop_ix++ ){
        int to = m_Table -> GetValueVisibleAt( stop_ix, CFeatTableDS::eFrom ).GetValue<int>();
        if( to <= stop_pos ){
            break;
        }
    }

    m_Table -> ScrollRowsToVisible( start_ix, stop_ix );
    */
}


void CFeatTableWidget::GetSelection( TConstObjects& objs ) const
{
    wxArrayInt objIxs = GetDataRowsSelected();

    for( int i = 0; i < (int)objIxs.size(); ++i ){
        const CSeq_feat& feat = m_DataSource->GetFeature( objIxs[i] );
        objs.push_back( CConstRef<CObject>( &feat ) );
    }
}

/*
TSeqRange CFeatTableWidget::GetSelectionVisibleRange() const
{
    int minIx = GetFirstSelected();
    if( minIx >=0 ){
        minIx = RowVisibleToData( minIx );
    }

    int maxIx = GetLastSelected();
    if( maxIx >=0 ){
        maxIx = RowVisibleToData( maxIx );
    }

    if( minIx >= 0 && maxIx >= 0 ){
        IwxTableModel& model = *GetModel();
        int minFrom = model.GetValueAt( minIx, CFeatTableDS::eFrom ).GetInteger();
        int maxTo = model.GetValueAt( maxIx, CFeatTableDS::eTo ).GetInteger();

        return TSeqRange( TSeqPos( minFrom ), TSeqPos( maxTo ) );
    }

    return TSeqRange( TSeqPos(-1), TSeqPos(-1) );
}
*/

void CFeatTableWidget::OnFilter( wxCommandEvent& anEvent )
{
    CFeatureFilterDlg dlg( this );

    vector<string> type_names;
    m_DataSource->GetTypeNames( type_names );
    wxArrayString wx_type_names;
    ITERATE( vector<string>, type_itr, type_names ){
        wx_type_names.Add( ToWxString( *type_itr ) );
    }
    dlg.SetTypeNames( wx_type_names );

    SFeatureFilter* params = (SFeatureFilter*)GetFilter();
    if( params->m_Types.IsEmpty() ){
        params->m_Types = wx_type_names;
    }
    dlg.SetParams( *params );

    dlg.Fit();

    if( dlg.ShowModal() == wxID_OK ){
        params = (SFeatureFilter*)RemoveFilter();
        if( params ){
            delete params;
        }

        params = new SFeatureFilter();
        *params = dlg.GetParams();

        SetFilter( params );
        InvalidateSorting();

        ApplyFiltering();
        ApplySorting();
        wxListEvent evt;
        OnSelectionChanged( evt );
    } 
}

void CFeatTableWidget::OnSelectionChanged( wxListEvent& event )
{
    CEvent evt( CEvent::eEvent_Message, CViewEvent::eWidgetSelectionChanged );
    Send( &evt, ePool_Parent );
}

END_NCBI_SCOPE
