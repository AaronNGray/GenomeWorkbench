#ifndef GUI_WIDGETS_EDIT___STRING_LIST_CTRL__HPP
#define GUI_WIDGETS_EDIT___STRING_LIST_CTRL__HPP

/*  $Id: string_list_ctrl.hpp 31203 2014-09-11 12:31:21Z bollin $
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
 * Authors:  Roman Katargin
 */

#include <corelib/ncbiobj.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/scrolwin.h>
#include <wx/textctrl.h>
#include <wx/hyperlink.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxFlexGridSizer;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CSTRINGLISTCTRL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSTRINGLISTCTRL_TITLE _("String List Control")
#define SYMBOL_CSTRINGLISTCTRL_IDNAME ID_CSTRINGSPANEL
#define SYMBOL_CSTRINGLISTCTRL_SIZE wxSize(400, 300)
#define SYMBOL_CSTRINGLISTCTRL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CStringListCtrl class declaration
 */

class CStringListCtrl: public wxScrolledWindow
{
    DECLARE_DYNAMIC_CLASS( CStringListCtrl )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CStringListCtrl();
    CStringListCtrl( wxWindow* parent, wxWindowID id = SYMBOL_CSTRINGLISTCTRL_IDNAME, const wxPoint& pos = SYMBOL_CSTRINGLISTCTRL_POSITION, const wxSize& size = SYMBOL_CSTRINGLISTCTRL_SIZE, long style = SYMBOL_CSTRINGLISTCTRL_STYLE );

    enum {
        kStringIdBase = 10100,
        kMaxStringsCount = 100
    };

    class CConstIterator
    {
    public:
        CConstIterator (CStringListCtrl& control);
        operator bool(void) const { return (m_TextCtrl != 0); }
        CConstIterator& operator++ ();
        string GetValue();

    private:
        CStringListCtrl& m_Control;
        int m_Id;
        wxTextCtrl* m_TextCtrl;
    };

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSTRINGLISTCTRL_IDNAME, const wxPoint& pos = SYMBOL_CSTRINGLISTCTRL_POSITION, const wxSize& size = SYMBOL_CSTRINGLISTCTRL_SIZE, long style = SYMBOL_CSTRINGLISTCTRL_STYLE );

    size_t GetMaxStringsCount() const { return kMaxStringsCount; }
    bool AddString(const string& value);
    void Clear();

    /// Destructor
    ~CStringListCtrl();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CStringListCtrl event handler declarations

////@end CStringListCtrl event handler declarations

////@begin CStringListCtrl member function declarations

    int GetCurrentStringId() const { return m_CurrentStringId ; }
    void SetCurrentStringId(int value) { m_CurrentStringId = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CStringListCtrl member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CStringListCtrl member variables
    wxFlexGridSizer* m_GridSizer;
private:
    int m_CurrentStringId;
    /// Control identifiers
    enum {
        ID_CSTRINGSPANEL = 10016
    };
////@end CStringListCtrl member variables

protected:
    void OnTextCtrlUpdated (wxCommandEvent& event);
    void OnDelete (wxHyperlinkEvent& event);
    int  FindSlot();
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___STRING_LIST_CTRL__HPP
