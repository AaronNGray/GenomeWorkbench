#ifndef GUI_WIDGETS_OBJECT_LIST_WIDGET___OBJECT_LIST_WIDGET__HPP
#define GUI_WIDGETS_OBJECT_LIST_WIDGET___OBJECT_LIST_WIDGET__HPP

/*  $Id: object_list_widget.hpp 39666 2017-10-25 16:01:13Z katargir $
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
 * Authors:  Andrey Yazhuk, Yury voronov
 *
 * File Description:
 *
 */

#include <gui/gui_export.h>

#include <gui/objutils/object_list.hpp>

#include <gui/widgets/wx/table_listctrl.hpp>

#include <gui/utils/iclipboard_handler.hpp>

#include <gui/objutils/registry.hpp>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CObjectListTableModel

class NCBI_GUIWIDGETS_OBJECT_LIST_EXPORT CObjectListTableModel
    : public CwxAbstractTableModel
{
public:
    CObjectListTableModel() : m_ObjectList( new CObjectList() ) {}
    CObjectListTableModel( CObjectList& list ) : m_ObjectList( &list ) {}

    virtual ~CObjectListTableModel();

	CObjectList* GetObjectList() { return m_ObjectList.GetPointer(); }
    void SetObjectList( CObjectList& list );
    void SetObjects( TConstScopedObjects& objects );

    CObject* GetObject( int row ){ return m_ObjectList->GetObject( row ); }
    objects::CScope* GetScope( int row ){ return m_ObjectList->GetScope( row ); }

    void Clear();

    /// @name IwxTableModel virtual functions implementation
    /// @{
    virtual int GetNumRows() const;

    virtual int GetNumColumns() const;
    virtual wxString GetColumnType( int col ) const;
    virtual wxString GetColumnName( int col ) const;

    virtual wxVariant GetValueAt( int row, int col ) const;
    /// @}

    /// @name Table model decorations from IwxDecoratedTableModel
    /// @{
    virtual CIRef<IwxStringFormat> GetColumnFormat( int col ) const;
    virtual wxString GetImageAliasAt( int row, int col ) const;
    /// @}

    virtual wxString GetImageAlias( int row ) const;

    virtual int GetNumExtraColumns() const;
    virtual wxString GetExtraColumnType( int col ) const;
    virtual wxString GetExtraColumnName( int col ) const;
    virtual wxVariant GetExtraValueAt( int row, int col ) const;

    virtual CIRef<IwxStringFormat> GetExtraColumnFormat( int col ) const;

protected:
    void x_Clear();

    inline void x_AssertRowValid( int row ) const;
	bool x_ColIsLabel( int col ) const;
	int x_AdjustColToData( int col ) const;

protected:
    CRef<CObjectList> m_ObjectList;

    struct CachedCell
    {
        CachedCell() : m_Initialized(false) {}
        CachedCell(const string& value) : m_Initialized(true), m_Value(value) {}

        bool   m_Initialized;
        string m_Value;
    };

    bool x_GetCachedLabel(int row, int col, string& value) const;
    void x_SetCachedLabel(int row, int col, const string& value) const;
    mutable vector<vector<CachedCell> > m_Cache;
};


///////////////////////////////////////////////////////////////////////////////
/// CObjectListWidget - mediator widget
class  NCBI_GUIWIDGETS_OBJECT_LIST_EXPORT CObjectListWidget
    : public CwxTableListCtrl
{
    // DECLARE_EVENT_TABLE()
public:
    CObjectListWidget() {}

    CObjectListWidget(
        wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = 0
    );

    //virtual ~CObjectListWidget();

    //NCBI_DEPRECATED void SetHandler( IObjectColumnHandler* handler );
    //NCBI_DEPRECATED void SetDefaultHandler();

	CObjectList* GetObjectList() { 
		return ((CObjectListTableModel*)GetModel())->GetObjectList();
	}
    void SetObjectList( CObjectList& list );
    void SetObjects( TConstScopedObjects& objects );

    //NCBI_DEPRECATED void InitModel( TConstScopedObjects& objects, IObjectColumnHandler* handler );
    //NCBI_DEPRECATED void InitModel( CObjectList& list, IObjectColumnHandler* handler );

    /// Inits widget with object list and optional model. If model is omitted,
    /// default implementation is used. If model explicitly stated as NULL,
    /// model is not replaced.
    void Init( CObjectList& list, CObjectListTableModel* model = new CObjectListTableModel() );

    void Clear();

    /// ISelection-style API
    void GetSelection( TConstScopedObjects& objects );
    void DisableMultipleSelection(bool disabled = true);

    void SaveSettings(CRegistryWriteView& view);
    void LoadSettings(CRegistryReadView& view);

    virtual void AppendMenuItems( wxMenu& aMenu );

protected:
    virtual void x_OnCopy();

    /// Debug & testing methods right now
    void OnSelected( wxListEvent& anEvt );
    void OnDeselected( wxListEvent& anEvt );

	inline CObjectListTableModel* GetOLTModel() const;
};

CObjectListTableModel* CObjectListWidget::GetOLTModel() const 
{ 
	return (CObjectListTableModel*)GetModel();
}

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_OBJECT_LIST_WIDGET___OBJECT_LIST_WIDGET__HPP
