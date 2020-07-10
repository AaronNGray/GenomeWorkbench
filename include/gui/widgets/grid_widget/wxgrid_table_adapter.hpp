#ifndef GUI_WIDGETS_WX___WXGRID_TABLE_ADAPTER_HPP
#define GUI_WIDGETS_WX___WXGRID_TABLE_ADAPTER_HPP

/*  $Id: wxgrid_table_adapter.hpp 27372 2013-02-04 18:45:24Z katargir $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>

#include <wx/grid.h>

#include <gui/objutils/table_data.hpp>
#include <gui/objutils/grid_table_adapter.hpp>
#include <gui/objutils/reg_settings.hpp>

BEGIN_NCBI_SCOPE

class ICommandProccessor;

///////////////////////////////////////////////////////////////////////////////
/// CGridTableQuery

class NCBI_GUIWIDGETS_GRID_WIDGET_EXPORT CwxGridTableAdapter
    : public wxGridTableBase, public IGridTableAdapter, public IRegSettings
{
public:
    CwxGridTableAdapter(ITableData& tableData, ICommandProccessor* cmdProccessor);
    ~CwxGridTableAdapter();

    class CEventHandler : public wxEvtHandler
    {
        DECLARE_EVENT_TABLE()
    public:
        CEventHandler(CwxGridTableAdapter& adapter) : m_Adapter(adapter) {}
    private:
        void OnAssignColumnValue(wxCommandEvent& event);
        void OnUpdateAssignColumnValue(wxUpdateUIEvent& event);

        void OnCopyColumn(wxCommandEvent& event);
        void OnUpdateCopyColumn(wxUpdateUIEvent& event);

        CwxGridTableAdapter& m_Adapter;
    };

    /// @name wxGridTableBase interface implementation
    /// @{
    virtual int GetNumberRows();
    virtual int GetNumberCols();

    virtual wxString GetColLabelValue(int col);
    virtual wxString GetRowLabelValue(int row);
    virtual wxString GetValue(int row, int col);

    virtual void SetValue(int row, int col, const wxString& value);
    virtual bool IsEmptyCell(int , int) { return false; }

    virtual bool CanGetValueAs( int row, int col, const wxString& typeName );
    virtual long GetValueAsLong( int row, int col );
    virtual double GetValueAsDouble( int row, int col );

    virtual void SetValueAsLong( int row, int col, long value );
    virtual void SetValueAsDouble( int row, int col, double value );
    virtual bool CanSetValueAs( int row, int col, const wxString& typeName );

    wxString GetHtmlValue(int row, int col);


    /*
    virtual wxGridCellAttr* GetAttr( int row, int col, wxGridCellAttr::wxAttrKind  kind );
    */

    virtual void SetView( wxGrid *grid );
    /// @}

    /// @name IGridTableAdapter interface implementation
    /// @{
    virtual void ShowColumn(size_t col, bool show);
    virtual bool IsColumnShown(size_t col) const;

    virtual size_t GetOriginalRow(size_t queryRow) const;
    virtual size_t GetCurrentRow(size_t queryRow) const;

    virtual void SortByColumn(TSortColumn sortColumn);
    virtual TSortColumn GetSortColumn() const { return m_SortColumn; }

    virtual void SetSelection(const vector<size_t>& query_sel);
    virtual void IterateSelection(int dir); 
    /// @}

    /// @name IRegSettings interface implementation
    /// @{
    virtual void SetRegistryPath(const string& reg_path) { m_RegPath = reg_path; }
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    /// @}

    string GetRegPath() const;

    wxMenu* CreateMenu();

protected:

    /// maps from row position in display to table (ITableData) index
    vector<size_t>    m_RowToTable;
    /// maps from ITableData index to row position in display
    vector<size_t>    m_TableToRow;

    /// maps from index in m_Selected to table (ITableData) index
    vector<size_t>    m_SelectedRowToTable;
    /// maps from ITableData index to index in m_Selected array
    vector<size_t>    m_SelectedTableToRow;

    TSortColumn m_SortColumn;

    bool x_ValidateColumns(const vector<size_t>& colums) const;
    void x_UpdateSelectedIndices();
    void x_ResetRows();
    void x_UpdateRenderers();

    void x_SaveColumnWidth(const string& label, int width);
    int  x_LoadColumnWidth(const string& label);

protected:
    wxGridCellAttr*   x_GetDefaultRendererAttr(bool allowEdit);
    wxGridCellAttr*   x_GetIntRendererAttr(bool allowEdit);
    wxGridCellAttr*   x_GetFloatRendererAttr(bool allowEdit);
    wxGridCellAttr*   x_GetHtmlCellAttrL();
    wxGridCellAttr*   x_GetHtmlCellAttrR();

    bool x_CoordsToTableData(int& row, int& col);
    bool x_ColToTableData(int& col);
    bool x_RowToTableData(int& row);

    wxGridCellAttr*   m_DefaultCellAttr;
    wxGridCellAttr*   m_IntCellAttr;
    wxGridCellAttr*   m_FloatCellAttr;

    wxGridCellAttr*   m_HtmlCellAttrL;
    wxGridCellAttr*   m_HtmlCellAttrR;

    wxGridCellAttr*   m_TextCellAttrEdit;
    wxGridCellAttr*   m_IntCellAttrEdit;
    wxGridCellAttr*   m_FloatCellAttrEdit;

    string m_RegPath;

    ICommandProccessor* m_CmdProccessor;

    CEventHandler m_EventHandler;
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS_WX___WXGRID_TABLE_ADAPTER_HPP

