#ifndef GUI_CORE_VIEW_TABLE___FEAT_TABLE__HPP
#define GUI_CORE_VIEW_TABLE___FEAT_TABLE__HPP

/*  $Id: feat_table.hpp 22487 2010-11-03 18:10:34Z voronov $
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
 * Authors: Mike DiCuccio, Yury Voronov
 *
 * File Description:
 *    CFeatTableWidget - a widget representing features as a text table
 */

#include <gui/types.hpp>

#include <gui/widgets/wx/table_listctrl.hpp>

#include <gui/widgets/feat_table/feat_table_ds.hpp>

#include <gui/utils/command.hpp>
#include <gui/objutils/objects.hpp>
#include <gui/objutils/filter.hpp>
#include <gui/objutils/registry.hpp>

#include <objects/seqloc/Seq_loc.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objmgr/feat_ci.hpp>

#include <vector>

BEGIN_NCBI_SCOPE

class CTableControl;
class CScrollPanel;
class CBox;

///////////////////////////////////////////////////////////////////////////////
/// CFeatTableWidget
class NCBI_GUIWIDGETS_FEATTABLE_EXPORT CFeatTableWidget
    : public CwxTableListCtrl
{
public:
    enum EVisibleRangeMethod {
        eScrollTo,
        eEntirelyContained,
        eIntersection
    };

    CFeatTableWidget(
        wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxLC_REPORT | wxLC_VIRTUAL
    );
    virtual ~CFeatTableWidget();

    /// access the data source in a typesafe manner
    void SetDataSource( CFeatTableDS* ds );

    const CFeatTableDS* GetDataSource() const { return m_DataSource; }
    CFeatTableDS* GetDataSource() { return m_DataSource; }


    void Update();

    /// select a given feature
    void SelectDataRow( size_t row ){ Select( RowDataToVisible( row ) ); }

    /// Get the selected objects
    void GetSelection( TConstObjects& objs ) const;

    void OnSelectionChanged( wxListEvent& event );

    //TODO
    void SetVisibleRangeMethod(EVisibleRangeMethod method);
    EVisibleRangeMethod GetVisibleRangeMethod() const;

    /// set the visible range for the widget.
    /// this is merely a hint; the tableis free to ignore this
    void SetVisibleRange( const objects::CSeq_loc& loc );

    //virtual TSeqRange GetSelectionVisibleRange() const;

    virtual void AppendMenuItems( wxMenu& aMenu );

    void OnFilter( wxCommandEvent& anEvent );


    //DECLARE_CLASS( CFeatTableWidget )
    DECLARE_EVENT_TABLE()

private:
    CRef<CFeatTableDS> m_DataSource;

    /// what do we do if the visible range changes?
    EVisibleRangeMethod m_VisibleRangeMethod;

    /// forbidden
    CFeatTableWidget( const CFeatTableWidget& );
    CFeatTableWidget& operator=( const CFeatTableWidget& );
};


END_NCBI_SCOPE

#endif  /// GUI_CORE_VIEW_TABLE___FEAT_TABLE__HPP
