/*  $Id: alnspan_widget.cpp 38774 2017-06-16 15:57:03Z katargir $
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
 * Authors:  Yury Voronov
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/aln_table/alnspan_widget.hpp>
#include <gui/widgets/aln_table/alnspan_settings_dlg.hpp>
#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/popup_event.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/utils/event_handler.hpp>

#include <gui/utils/view_event.hpp>
#include <gui/objutils/registry.hpp>

#include <serial/iterator.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/// Definitions for Align Span Widget commands
///
/*
enum EAlignSpanTableCommands {
    eCmdFind = eBaseCmdLast + 300,
    eCmdSelectActiveColumns,
    eCmdVisibleRangeChange,
    eCmdFilter,
    eCmdExportToCSV
};
*/


BEGIN_EVENT_TABLE(CAlnSpanWidget, CwxTableListCtrl)
    EVT_MENU( CwxTableListCtrl::eCmdExportToCSV, CAlnSpanWidget::OnExportToCSV )
    EVT_MENU( eCmdSettings, CAlnSpanWidget::OnSettings )
    EVT_UPDATE_UI( eCmdSettings, CAlnSpanWidget::OnSettingsUpdateUI )
    EVT_LIST_ITEM_SELECTED( wxID_ANY, CAlnSpanWidget::OnSelectionChanged )
    EVT_LIST_ITEM_DESELECTED( wxID_ANY, CAlnSpanWidget::OnSelectionChanged )
END_EVENT_TABLE()


CAlnSpanWidget::CAlnSpanWidget(
    wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style
)
    : CwxTableListCtrl( parent, id, pos, size, style )
{
}

CAlnSpanWidget::~CAlnSpanWidget()
{
}

void CAlnSpanWidget::SetAlnModel( CAlnSpanVertModel& model )
{
    CwxTableListCtrl::SetModel( &model );

    for( int i = 0; i < model.GetNumSequences(); i++ ){
        SetColumnWidth( i, 100 );
    }
}

void CAlnSpanWidget::Update()
{
    //((CAlnSpanVertModel*)GetModel())->Update();
}

void CAlnSpanWidget::GetSelection( TLocations& locs ) const
{
    wxArrayInt objIxs = GetDataRowsSelected();

    IwxTableModel* imodel = GetModel();
    const CAlnSpanVertModel* model = dynamic_cast<const CAlnSpanVertModel*>( imodel );
    _ASSERT( model != NULL );

    for( int i = 0; i < (int)objIxs.size(); ++i ){
        const CAlnSpanVertModel::SSpanRow& row = model->GetData( objIxs[i] );

        ///
        /// we need to extract a location from each row
        /// we have two locations to provide: one for the object
        /// and one for the component
        ///
        ITERATE( vector< CConstRef<CSeq_loc> >, iter, row.locs ){
            if( *iter ){
                locs.push_back(*iter);
            }
        }
    }
}

// send an event to parents notifing that selection has changed
void CAlnSpanWidget::x_NotifyWidgetSelChanged()
{
    CEvent evt(CEvent::eEvent_Message, CViewEvent::eWidgetSelectionChanged);
    Send(&evt, ePool_Parent);
}

// send an event to parents notifing that selected seq range has changed
void CAlnSpanWidget::x_NotifyWidgetRangeChanged()
{
    CEvent evt(CEvent::eEvent_Message, CViewEvent::eWidgetRangeChanged);
    Send(&evt, ePool_Parent);
}


void CAlnSpanWidget::AppendMenuItems( wxMenu& aMenu )
{
    CwxTableListCtrl::AppendMenuItems( aMenu );

    aMenu.Append( wxID_SEPARATOR, wxT("Actions") );
    aMenu.Append( CwxTableListCtrl::eCmdExportToCSV, wxT("Export to CSV...") );

    aMenu.Append( wxID_SEPARATOR, wxT("Settings") );
    aMenu.Append( eCmdSettings, wxT("Settings...") );
}


void CAlnSpanWidget::OnExportToCSV( wxCommandEvent& event )
{
    ExportTableControlToCSV();
}

void CAlnSpanWidget::OnSettings( wxCommandEvent& event )
{
    CAlnSpanVertModel* model = dynamic_cast<CAlnSpanVertModel*>( GetModel() );
    _ASSERT( model != NULL );

   CAlnSpanSettingsDlg dlg( this );
   dlg.SetThreshold( model->GetThreshold() );
   dlg.SetShowIndels( model->GetShowIndels() );
   if( dlg.ShowModal() == wxID_OK ){
       model->SetThreshold( dlg.GetThreshold() );
       model->SetShowIndels( dlg.GetShowIndels() );
       model->UpdateRows();

       //HACK 
       // This is to update status bar
       // We need a special x_NotifyWidgetDataChanged() call
       x_NotifyWidgetSelChanged();
   }
}

void CAlnSpanWidget::OnSettingsUpdateUI( wxUpdateUIEvent& event )
{   
    event.Enable( true );
}


void CAlnSpanWidget::x_OnSelectionChanged( bool on, int start_row, int end_row )
{
    x_NotifyWidgetSelChanged();
}

END_NCBI_SCOPE

/*
 * ===========================================================================
 * $Log$
 * ===========================================================================
 */
