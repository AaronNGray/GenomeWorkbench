/*  $Id: growable_list.hpp 33758 2015-09-15 14:17:30Z filippov $
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

#ifndef GROWABLE_LIST_HPP
#define GROWABLE_LIST_HPP

#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/hyperlink.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

BEGIN_NCBI_SCOPE


/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CGROWABLELISTCTRL_STYLE wxSUNKEN_BORDER|wxTAB_TRAVERSAL
#define SYMBOL_CGROWABLELISTCTRL_TITLE _("Growable List Control")
#define SYMBOL_CGROWABLELISTCTRL_IDNAME ID_CGROWABLELISTCTRL
#define SYMBOL_CGROWABLELISTCTRL_SIZE wxSize(-1,200)
#define SYMBOL_CGROWABLELISTCTRL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CGrowableListCtrl class declaration
 */

class CGrowableListCtrl: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CGrowableListCtrl )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CGrowableListCtrl();
    CGrowableListCtrl( wxWindow* parent, const wxArrayString &items,
                       wxWindowID id = SYMBOL_CGROWABLELISTCTRL_IDNAME, const wxPoint& pos = SYMBOL_CGROWABLELISTCTRL_POSITION, const wxSize& size = SYMBOL_CGROWABLELISTCTRL_SIZE, long style = SYMBOL_CGROWABLELISTCTRL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CGROWABLELISTCTRL_IDNAME, const wxPoint& pos = SYMBOL_CGROWABLELISTCTRL_POSITION, const wxSize& size = SYMBOL_CGROWABLELISTCTRL_SIZE, long style = SYMBOL_CGROWABLELISTCTRL_STYLE );

    /// Destructor
    ~CGrowableListCtrl();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CGrowableListCtrl event handler declarations


////@end CGrowableListCtrl event handler declarations

////@begin CGrowableListCtrl member function declarations


    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CGrowableListCtrl member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CGrowableListCtrl member variables

    void OnDelete ( wxHyperlinkEvent& event );
    void OnSpinCtrl ( wxCommandEvent& evt );
    void AddRow(const  wxString& item);
    void AddEmptyRow();
    void ClearRows();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();
    void GetItems(vector<string> &items);
    void SetItems( const wxArrayString &items);
private:
    struct SIntData
    {  
        wxString m_Item;
        bool     m_Empty;

        bool operator==(const SIntData& intData) const
            { return m_Item == intData.m_Item; }
        bool operator<(const SIntData& intData) const
            {
                return (m_Item < intData.m_Item);
            }
    };
    /// Control identifiers
    enum {
        ID_CGROWABLELISTCTRL = 10032,
        ID_CGROWABLELISTCTRL_PANEL1 = 10039,
        ID_CGROWABLELISTCTRL_SCROLLEDWINDOW1 = 10035,
    };
////@end CGrowableListCtrl member variables

    void x_AddRow(const wxString& item, size_t rowPos = -1);
    void x_AddEmptyRow(size_t rowPos = -1);

    vector<SIntData> m_Data;
    wxArrayString m_Items;
    wxScrolledWindow* m_ScrolledWindow; 
    wxFlexGridSizer* m_Sizer;
};

END_NCBI_SCOPE

#endif  // GROWABLE_LIST_HPP
