#ifndef GUI_WIDGETS_EDIT___MACRO_AUTHORS_PANEL__HPP
#define GUI_WIDGETS_EDIT___MACRO_AUTHORS_PANEL__HPP
/*  $Id: macro_authors_panel.hpp 41345 2018-07-12 18:04:13Z asztalos $
* ===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software / database is a "United States Government Work" under the
*  terms of the United States Copyright Act.It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.This software / database is freely available
*  to the public for use.The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data.The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
* Authors: Andrea Asztalos
*
*/

#include <corelib/ncbistd.hpp>
#include <gui/widgets/edit/author_names_container.hpp>
//#include <wx/hyperlink.h>

class wxScrolledWindow;
class wxFlexGridSizer;

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CMACROAUTHORNAMESPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CMACROAUTHORNAMESPANEL_TITLE _("Macro Add Authors Panel")
#define SYMBOL_CMACROAUTHORNAMESPANEL_IDNAME ID_CMACROAUTHORNAMESPANEL
#define SYMBOL_CMACROAUTHORNAMESPANEL_SIZE wxDefaultSize
#define SYMBOL_CMACROAUTHORNAMESPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CMacroAuthorNamesPanel class declaration
 */

class CMacroAuthorNamesPanel: public CAuthorNamesContainer
{    
    DECLARE_DYNAMIC_CLASS( CMacroAuthorNamesPanel )
    DECLARE_EVENT_TABLE()

public:
    // names:                last,   first,  m.I.,   suffix
    using TNameTuple = tuple<string, string, string, string >;
    using TAuthorsVec = vector<TNameTuple>;

    /// Constructors
    CMacroAuthorNamesPanel();
    CMacroAuthorNamesPanel( wxWindow* parent, 
        wxWindowID id = SYMBOL_CMACROAUTHORNAMESPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CMACROAUTHORNAMESPANEL_POSITION, 
        const wxSize& size = SYMBOL_CMACROAUTHORNAMESPANEL_SIZE, 
        long style = SYMBOL_CMACROAUTHORNAMESPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CMACROAUTHORNAMESPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CMACROAUTHORNAMESPANEL_POSITION, 
        const wxSize& size = SYMBOL_CMACROAUTHORNAMESPANEL_SIZE, 
        long style = SYMBOL_CMACROAUTHORNAMESPANEL_STYLE );

    /// Destructor
    ~CMacroAuthorNamesPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CMacroAuthorNamesPanel event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_IMPORTAUTHORSBTN
    void OnImportAuthorsClicked( wxCommandEvent& event );

    void OnDelete(wxHyperlinkEvent& event);

    /// @name CAuthorNamesContainer implementation
    /// @{
    virtual void AddLastAuthor(wxWindow* link);
    virtual void InsertAuthorBefore(wxWindow* link);
    virtual void InsertAuthorAfter(wxWindow* link);
    virtual void SetAuthors(const objects::CAuth_list& auth_list);
    /// @}

///@end CMacroAuthorNamesPanel event handler declarations

////@begin CMacroAuthorNamesPanel member function declarations

    TAuthorsVec GetAuthorNames() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CMacroAuthorNamesPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CMacroAuthorNamesPanel member variables
    /// Control identifiers
    enum {
        ID_CMACROAUTHORNAMESPANEL = 10035,
        ID_MACROAUTHORSSCROLLEDWND = 10036,
        ID_ADDAUTHORBTN = 10037,
        ID_IMPORTAUTHORSBTN = 10038,
        ID_MACROAUTHORDELETELINK = 10039
    };
////@end CMacroAuthorNamesPanel member variables

private:
    /// @name CAuthorNamesContainer implementation
    /// @{
    virtual wxSizerItem* x_FindSingleAuthor(wxWindow* wnd, wxSizerItemList& itemList);
    virtual void x_AddRowToWindow(wxWindow* row);
    virtual void x_InsertRowtoWindow(wxWindow* row, size_t index);
    virtual void x_MoveRowUp(int row_num);
    virtual int x_FindRow(wxWindow* wnd, wxSizerItemList& itemList);
    /// @}

    void x_AddEmptyRow();
    void x_SetAuthors(objects::CAuth_list& auth_list);

    size_t m_Rows{ 6 };
    int m_RowHeight{ 0 };
};

END_NCBI_SCOPE

#endif
    // GUI_WIDGETS_EDIT___MACRO_AUTHORS_PANEL__HPP
