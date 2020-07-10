#ifndef GUI_WIDGETS_ALN_SPAN___ALN_SPAN_WIDGET__HPP
#define GUI_WIDGETS_ALN_SPAN___ALN_SPAN_WIDGET__HPP

/*  $Id: alnspan_widget.hpp 23967 2011-06-24 22:33:37Z voronov $
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

#include <corelib/ncbiobj.hpp>
#include <gui/utils/command.hpp>
#include <gui/widgets/wx/table_listctrl.hpp>
#include <gui/widgets/aln_table/alnspan_vertmodel.hpp>

#include <gui/objutils/objects.hpp>

#include <objmgr/scope.hpp>

#include <objects/seqalign/Seq_align.hpp>


BEGIN_NCBI_SCOPE


class NCBI_GUIWIDGETS_ALNTABLE_EXPORT CAlnSpanWidget
    : public CwxTableListCtrl
{
public:
    CAlnSpanWidget() {}

    CAlnSpanWidget(
        wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxLC_REPORT | wxLC_VIRTUAL
    );

    virtual ~CAlnSpanWidget();


    // void Add(objects::CScope& scope, const objects::CSeq_align& aln);
    void SetAlnModel( CAlnSpanVertModel& model );

    void Update();

    // Convinience function
    int GetNumRowsSelected() const { return GetSelectedItemCount(); }

    typedef list< CConstRef<objects::CSeq_loc> > TLocations;
    void GetSelection( TLocations& locs ) const;

    /// @name ISelectionModel::ISMListener Interface
    /// @{
    //void SelectionChanged( const ISelectionModel::CSMNotice& msg );
    /// @}

    virtual void AppendMenuItems( wxMenu& aMenu );

    /// @name Command and Event handlers
    /// @{
    virtual void x_OnSelectionChanged( bool on, int start_row, int end_row = -1 );
    
    void OnExportToCSV( wxCommandEvent& event );
    void OnSettings( wxCommandEvent& event );
    void OnSettingsUpdateUI( wxUpdateUIEvent& event );
    ///@}

protected:
    void x_NotifyWidgetSelChanged();
    void x_NotifyWidgetRangeChanged();

private:
    /// prohibited
    CAlnSpanWidget( const CAlnSpanWidget& );
    CAlnSpanWidget& operator=( const CAlnSpanWidget& );

    DECLARE_EVENT_TABLE()
};




END_NCBI_SCOPE


#endif  // GUI_WIDGETS_ALN_SPAN___ALN_SPAN_WIDGET__HPP
