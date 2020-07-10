#ifndef GUI_CORE_VIEW_TABLE___FEAT_TABLE_DS__HPP
#define GUI_CORE_VIEW_TABLE___FEAT_TABLE_DS__HPP

/*  $Id: feat_table_ds.hpp 31826 2014-11-18 21:27:54Z katargir $
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
 *    CFeatTableDS - data model represinting features to show in CFeatTableWidget
 */

#include <gui/types.hpp>


#include <gui/widgets/wx/table_model.hpp>
#include <gui/widgets/wx/row_model.hpp>
#include <gui/objutils/objects.hpp>

#include <gui/utils/event_handler.hpp>
#include <gui/utils/app_job_dispatcher.hpp>

#include <objmgr/bioseq_handle.hpp>
#include <objmgr/feat_ci.hpp>

#include <objects/seqloc/Seq_loc.hpp>
#include <objmgr/scope.hpp>


BEGIN_NCBI_SCOPE

class CFeatTableEvent : public CEvent
{
public:
    enum EEventId {
        eStatusChange
    };

    CFeatTableEvent() : CEvent(eStatusChange) {}
};


///////////////////////////////////////////////////////////////////////////////
/// CFeatTableDS
/// Data source for Feature Table widget. Implements TableModel (for CTableControl)
/// and is responsible for loading and updating data asynchronously.
///
class NCBI_GUIWIDGETS_FEATTABLE_EXPORT CFeatTableDS
    : public CObjectEx
    , public CwxAbstractTableModel
    , public CEventHandler
{
/****** PUBLIC STRUCTURES SECTION ******/
public:
    /// a list of the available column types we can show
    enum EColumns {
        eNum,
        eLabel,
        eType,
        eFrom,
        eTo,
        eLength,
        eStrand,
        eProduct,
        eIntervals,

        /// this must be the last entry!
        eNumColumns
    };

    /// our actual features
    struct SFeatureRow {
        int row_idx;
        objects::CMappedFeat feat;
        bool filledIn;

        string label;
        string type;
        string strand;
        string product;
        int from;
        int to;
        int length;
        int intervals;
        bool partial;

        SFeatureRow() : filledIn( false ) {}

        SFeatureRow( int aRowIdx, const objects::CMappedFeat& aFeat )
            : row_idx( aRowIdx ), feat( aFeat ), filledIn( false )
        {
        }

        void FillIn( const objects::CMappedFeat& map_feat );
        void FillIn( const objects::CMappedFeat& map_feat, objects::CSeq_loc::TRange& range );
        void FillIn(){ FillIn( feat ); }
    };

/****** PUBLIC INTERFACE SECTION ******/
public:

    CFeatTableDS();
    virtual ~CFeatTableDS();

    /// @name Setting data methods
    /// @{
    void Init( const CSerialObject& obj, objects::CScope& scope, const objects::SAnnotSelector* sel = 0 );

    /// sets the DataSource-specific filter
    void SetSelector( const objects::SAnnotSelector& sel );
    objects::SAnnotSelector GetSelector() const { return m_Selector; }

    CRef<objects::CScope> GetScope() const { return m_Scope; }
    const SFeatureRow& GetFeatureRow( size_t row ) const;
    const objects::CSeq_feat& GetFeature( size_t row ) const;

    /// reloads data based on current settings
    void Update();
    /// @}

    /// @name ITableModel implementation
    /// @{
    virtual int GetNumRows() const;

    virtual int GetNumColumns() const;

    virtual wxVariant GetValueAt( int row, int col ) const;

    virtual wxString GetColumnName( int aColIx ) const;

    virtual wxString GetColumnType( int aColIx ) const;
    /// @}

    void GetTypeNames( vector<string>& names ) const;

    void OnJobNotification( CEvent* evt );
    void ClearCurrentJob();
    void Clear();

    void x_NotifyObserversNow();

    DECLARE_EVENT_MAP();

private:
    virtual void x_OnJobNotification( CEvent* evt );
    void x_UpdateTypeHash();


private:
    CConstRef<CSerialObject> m_Object;   // the source of the features
    CRef<objects::CScope> m_Scope;

    // the filter defining what features need to be loaded
    objects::SAnnotSelector m_Selector;

    vector<SFeatureRow> m_FeatureRows;

    /// app job notification and control
    CAppJobDispatcher::TJobID m_ActiveJob;

    typedef map<string, string> TTypeHash;
    TTypeHash m_TypeHash;
};

///////////////////////////////////////////////////////////////////////////////
/// SFeatureFilter
struct SFeatureFilter : public IRowFilter
{
    bool m_Disabled;

    wxString m_Label;
    wxArrayString m_Types;

    long m_FromLoc;
    long m_ToLoc;
    bool m_InclLoc;

    long m_LengthLoc;

    wxString m_Product;
    bool m_NoProduct;

    SFeatureFilter();

    //bool operator()( const CFeatTableDS::SFeatureRow& SF ) const;
    virtual bool operator()( const IRowModel& aRow ) const;
};

END_NCBI_SCOPE

#endif  /// GUI_CORE_VIEW_TABLE___FEAT_TABLE_DS__HPP
