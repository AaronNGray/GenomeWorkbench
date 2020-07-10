/*  $Id: apply_edit_convert_panel.hpp 38698 2017-06-09 20:08:17Z filippov $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Igor Filippov
 */


// Generated by DialogBlocks (unregistered), 16/05/2014 10:25:31

#ifndef _APPLY_EDIT_CONVERT_PANEL_H_
#define _APPLY_EDIT_CONVERT_PANEL_H_


/*!
 * Includes
 */
#include <corelib/ncbistd.hpp>
#include <set>
#include <wx/grid.h>
#include <wx/listbox.h>
#include <wx/listbook.h>
#include <gui/packages/pkg_sequence_edit/cap_change_panel.hpp>
#include <gui/packages/pkg_sequence_edit/seqtable_util.hpp>
#include <gui/packages/pkg_sequence_edit/seq_table_grid.hpp>

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

BEGIN_NCBI_SCOPE

class CSubpanel;
class CParseTextOptionsDlg;


////@begin control identifiers
#define SYMBOL_CAPPLYEDITCONVERTPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CAPPLYEDITCONVERTPANEL_TITLE _("apply edit convert panel")
#define SYMBOL_CAPPLYEDITCONVERTPANEL_IDNAME ID_CAPPLYEDITCONVERTPANEL
#define SYMBOL_CAPPLYEDITCONVERTPANEL_SIZE  wxDefaultSize
#define SYMBOL_CAPPLYEDITCONVERTPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CApplyEditconvertPanel class declaration
 */

class CApplyEditconvertPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CApplyEditconvertPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CApplyEditconvertPanel();
    CApplyEditconvertPanel( wxWindow* parent, CSeqTableGridPanel* grid_panel, wxArrayString &choices, 
        int init_action = 0,  bool horizontal = true, wxWindowID id = SYMBOL_CAPPLYEDITCONVERTPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CAPPLYEDITCONVERTPANEL_POSITION, 
        const wxSize& size = SYMBOL_CAPPLYEDITCONVERTPANEL_SIZE, 
        long style = SYMBOL_CAPPLYEDITCONVERTPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent,
        wxWindowID id = SYMBOL_CAPPLYEDITCONVERTPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CAPPLYEDITCONVERTPANEL_POSITION, 
        const wxSize& size = SYMBOL_CAPPLYEDITCONVERTPANEL_SIZE, 
        long style = SYMBOL_CAPPLYEDITCONVERTPANEL_STYLE );

    /// Destructor
    ~CApplyEditconvertPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CApplyEditconvertPanel event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_AECR_BUTTON
    void OnApplyToAllButtonClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_AECR_BUTTON1
    void OnApplyToSelButtonClick( wxCommandEvent& event );

    void OnUndoButtonClick( wxCommandEvent& event );

////@end CApplyEditconvertPanel event handler declarations

////@begin CApplyEditconvertPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CApplyEditconvertPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    int GetColumn();
    wxString GetAutopopulateValue();
    void GetSelectedRows(std::set<int> &selected);
    void SetColumns(wxArrayString &columns);
    
    bool GetModified() {return m_Modified;}
    
    void AddUndo(int i, int j, const string &val) {m_undo_values[i][j] = val;}

    ////@begin CApplyEditconvertPanel member variables

    enum {
        ID_CAPPLYEDITCONVERTPANEL = 6350,
        ID_AECR_CHOICE,
        ID_AECR_LISTBOX,
        ID_AECR_BUTTON,
        ID_AECR_BUTTON1,
        ID_AECR_BUTTON2
    };
private:
    wxListBox* GetListBox();

    CSeqTableGridPanel* m_GridPanel;
    wxGrid *m_Grid;
    wxArrayString *m_Choices;
    wxListbook* m_Notebook;
    bool m_Modified;
    wxButton* m_UndoButton;
    map<int, map<int,string> > m_undo_values;
    int m_init_action;
    // flag to indicate the layout of the CParseTextOptionsDlg in CParseSubpanel
    bool m_Horizontal;
////@end CApplyEditconvertPanel member variables
};

class CSubpanel: public wxPanel
{
 DECLARE_DYNAMIC_CLASS( CSubpanel )
 DECLARE_EVENT_TABLE()

public:
    CSubpanel() : wxPanel() {  Init(); }
    CSubpanel( wxWindow* parent,  wxGrid *grid, wxArrayString &choices, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) : wxPanel (parent, id, pos, size, style), m_Grid(grid), m_Choices(&choices)
        { 
          Init(); 
          Create(parent, id, pos, size, style);
        }
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL )
        {
            wxPanel::Create( parent, id, pos, size, style );
            CreateControls();
            if (GetSizer())
            {
                GetSizer()->SetSizeHints(this);
            }
            Centre();
            return true;
        }
    virtual ~CSubpanel() {}
    bool ShowToolTips()  { return true; }
    wxBitmap GetBitmapResource( const wxString& name ) {   wxUnusedVar(name);   return wxNullBitmap; }
    wxIcon GetIconResource( const wxString& name ) {  wxUnusedVar(name);   return wxNullIcon; }
    objects::edit::EExistingText GetExistingTextHandling(const string &field);
    virtual void Init() {m_ListBox = NULL; m_ListBox2 = NULL;}
    virtual void CreateControls() {}
    virtual void ApplyToAll(int col) {}
    virtual void ApplyToSelected(int col, const std::set<int> &selected) {}
    virtual void SetColumns(wxArrayString &columns) {m_ListBox->Set(columns);m_ListBox->SetSelection(0);}
    void ChangeValue(const string &new_value, int row, int col);
    CApplyEditconvertPanel *GetBaseFrame();
    wxListBox* GetListBox() {return m_ListBox;}
    int GetColumn2();

protected:
    wxGrid *m_Grid;
    wxListBox* m_ListBox;
    wxListBox* m_ListBox2;
    wxArrayString *m_Choices;
};


/*!
 * CApplySubpanel class declaration
 */

class CApplySubpanel: public CSubpanel
{    
    DECLARE_DYNAMIC_CLASS( CApplySubpanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CApplySubpanel(){  Init(); }
    CApplySubpanel( wxWindow* parent, wxGrid *grid, wxArrayString &choices, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL )  
        {  
            this->m_Grid = grid;
            this->m_Choices = &choices;
            Init(); 
            Create(parent, id, pos, size, style);
        }

    /// Creates the controls and sizers
    virtual void CreateControls();

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_APPLYSUB_AUTOPOPULATE
    void OnApplysubAutopopulateClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_APPLYSUB_CLEAR
    void OnApplysubClearClick( wxCommandEvent& event );

    virtual void ApplyToAll(int col);
    virtual void ApplyToSelected(int col, const std::set<int> &selected);

////@begin CApplySubpanel member variables
////@end CApplySubpanel member variables

    enum {
        ID_APPLYSUB_TEXTCTRL = 6370,
        ID_APPLYSUB_AUTOPOPULATE,
        ID_APPLYSUB_CLEAR
    };
private:
    wxTextCtrl *m_TextCtrl; 
};


/*!
 * CEditSubpanel class declaration
 */

class CEditSubpanel: public CSubpanel
{    
    DECLARE_DYNAMIC_CLASS( CEditSubpanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CEditSubpanel(){  Init(); }
    CEditSubpanel( wxWindow* parent, wxGrid *grid, wxArrayString &choices, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL )  
        {  
            this->m_Grid = grid;
            this->m_Choices = &choices;
            Init(); 
            Create(parent, id, pos, size, style);
        }
    /// Creates the controls and sizers
    virtual void CreateControls();

////@begin CEditSubpanel event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_EDITSUB_COPY
    void OnEditsubCopyClick( wxCommandEvent& event );

////@end CEditSubpanel event handler declarations


  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_EDITSUB_AUTOPOPULATE
    void OnEditsubAutopopulateClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_EDITSUB_CLEAR
    void OnEditsubClearClick( wxCommandEvent& event );

    virtual void ApplyToAll(int col);
    virtual void ApplyToSelected(int col, const std::set<int> &selected);

////@begin CEditSubpanel member variables
////@end CEditSubpanel member variables
    enum {
        ID_EDITSUB_FIND = 6390,
        ID_EDITSUB_REPLACE,
        ID_EDITSUB_COPY,
        ID_EDITSUB_ANY,
        ID_EDITSUB_BEG,
        ID_EDITSUB_END,
        ID_EDITSUB_AUTOPOPULATE,
        ID_EDITSUB_CLEAR
    };
private:
    wxTextCtrl* m_Find;
    wxTextCtrl* m_Replace;
    wxRadioButton *m_Anywhere, *m_AtBegin, *m_AtEnd;
};


/*!
 * CRemoveSubpanel class declaration
 */

class CRemoveSubpanel: public CSubpanel
{    
    DECLARE_DYNAMIC_CLASS( CRemoveSubpanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CRemoveSubpanel() {  Init(); }
    CRemoveSubpanel( wxWindow* parent, wxGrid *grid, wxArrayString &choices, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL ) 
        {  
            this->m_Grid = grid;
            this->m_Choices = &choices;
            Init(); 
            Create(parent, id, pos, size, style);
        }
    /// Creates the controls and sizers
    virtual void CreateControls();
    virtual void ApplyToAll(int col);
    virtual void ApplyToSelected(int col, const std::set<int> &selected);
};


/*!
 * CSwapSubpanel class declaration
 */

class CSwapSubpanel: public CSubpanel
{    
    DECLARE_DYNAMIC_CLASS( CSwapSubpanel )
    DECLARE_EVENT_TABLE()

public:
    CSwapSubpanel() {  Init(); }
    CSwapSubpanel( wxWindow* parent, wxGrid *grid, wxArrayString &choices, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL )  
        {  
            this->m_Grid = grid;
            this->m_Choices = &choices;
            Init(); 
            Create(parent, id, pos, size, style);
        }
    /// Creates the controls and sizers
    virtual void CreateControls();
    virtual void SetColumns(wxArrayString &columns) {CSubpanel::SetColumns(columns); m_ListBox2->Set(columns);m_ListBox2->SetSelection(0);}
    virtual void ApplyToAll(int col);
    virtual void ApplyToSelected(int col, const std::set<int> &selected);
};



/*!
 * CConvertSubpanel class declaration
 */

class CConvertSubpanel: public CSubpanel
{    
    DECLARE_DYNAMIC_CLASS( CConvertSubpanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CConvertSubpanel() {  Init(); }
    CConvertSubpanel( wxWindow* parent, wxGrid *grid, wxArrayString &choices, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL ) 
        {  
            this->m_Grid = grid;
            this->m_Choices = &choices;
            Init(); 
            Create(parent, id, pos, size, style);
        }
    /// Creates the controls and sizers
    virtual void CreateControls();
    virtual void SetColumns(wxArrayString &columns) {CSubpanel::SetColumns(columns); m_ListBox2->Set(columns);m_ListBox2->SetSelection(0);}
    virtual void ApplyToAll(int col);
    virtual void ApplyToSelected(int col, const std::set<int> &selected);
private:
    wxCheckBox *m_CheckBox;
};

/*!
 * CParseSubpanel class declaration
 */

class CParseSubpanel: public CSubpanel
{    
    DECLARE_DYNAMIC_CLASS( CParseSubpanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CParseSubpanel() {  Init(); }
    CParseSubpanel( wxWindow* parent, wxGrid *grid, wxArrayString &choices,  wxWindowID id = wxID_ANY,  const wxPoint& pos = wxDefaultPosition,  const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL ) 
        {  
            this->m_Grid = grid;
            this->m_Choices = &choices;
            Init(); 
            Create(parent, id, pos, size, style);
        }

    
    /// Creates the controls and sizers
    virtual void CreateControls();
    virtual void SetColumns(wxArrayString &columns) {CSubpanel::SetColumns(columns); m_ListBox2->Set(columns);m_ListBox2->SetSelection(0);}
    virtual void ApplyToAll(int col);
    virtual void ApplyToSelected(int col, const std::set<int> &selected);
private:
    CParseTextOptionsDlg* m_ParseOptions;   
};

/*!
 * CChangeCaseSubpanel class declaration
 */

class CChangeCaseSubpanel: public CSubpanel
{    
    DECLARE_DYNAMIC_CLASS( CChangeCaseSubpanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CChangeCaseSubpanel() {  Init(); }
    CChangeCaseSubpanel( wxWindow* parent, wxGrid *grid, wxArrayString &choices, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL ) 
        {  
            this->m_Grid = grid;
            this->m_Choices = &choices;
            Init(); 
            Create(parent, id, pos, size, style);
        }
    /// Creates the controls and sizers
    virtual void CreateControls();
    virtual void ApplyToAll(int col);
    virtual void ApplyToSelected(int col, const std::set<int> &selected);
private:
    CCapChangePanel *m_CapChangeOptions;
};

END_NCBI_SCOPE
#endif
    // _APPLY_EDIT_CONVERT_PANEL_H_
