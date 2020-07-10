#ifndef GUI_WIDGETS___LOADERS___AGP_LOAD_PAGE__HPP
#define GUI_WIDGETS___LOADERS___AGP_LOAD_PAGE__HPP

/*  $Id: agp_load_page.hpp 33803 2015-09-17 16:18:33Z katargir $
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

#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */

#include <wx/panel.h>

#include <gui/widgets/loaders/agp_load_params.hpp>

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CAGPLOADPAGE_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CAGPLOADPAGE_TITLE _("AGP Load Params Panel")
#define SYMBOL_CAGPLOADPAGE_IDNAME ID_CAGPLOADPAGE
#define SYMBOL_CAGPLOADPAGE_SIZE wxSize(400, 300)
#define SYMBOL_CAGPLOADPAGE_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE

/*!
 * CAgpLoadPage class declaration
 */

class NCBI_GUIWIDGETS_LOADERS_EXPORT CAgpLoadPage: public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CAgpLoadPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAgpLoadPage();
    CAgpLoadPage( wxWindow* parent, wxWindowID id = SYMBOL_CAGPLOADPAGE_IDNAME, const wxPoint& pos = SYMBOL_CAGPLOADPAGE_POSITION, const wxSize& size = SYMBOL_CAGPLOADPAGE_SIZE, long style = SYMBOL_CAGPLOADPAGE_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CAGPLOADPAGE_IDNAME, const wxPoint& pos = SYMBOL_CAGPLOADPAGE_POSITION, const wxSize& size = SYMBOL_CAGPLOADPAGE_SIZE, long style = SYMBOL_CAGPLOADPAGE_STYLE );

    /// Destructor
    ~CAgpLoadPage();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// Transfer data to the window
    virtual bool TransferDataToWindow();

    /// Transfer data from the window
    virtual bool TransferDataFromWindow();

////@begin CAgpLoadPage event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BITMAPBUTTON1
    void OnFASTASeqsBrowse( wxCommandEvent& event );

////@end CAgpLoadPage event handler declarations

////@begin CAgpLoadPage member function declarations

    /// Data access
    CAgpLoadParams& GetData() { return m_data; }
    const CAgpLoadParams& GetData() const { return m_data; }
    void SetData(const CAgpLoadParams& data) { m_data = data; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAgpLoadPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CAgpLoadPage member variables
    /// The data edited by this window
    CAgpLoadParams m_data;
    /// Control identifiers
    enum {
        ID_CAGPLOADPAGE = 10041,
        ID_RADIOBOX2 = 10042,
        ID_CHECKBOX16 = 10043,
        ID_TEXTCTRL6 = 10044,
        ID_BITMAPBUTTON1 = 10017
    };
////@end CAgpLoadPage member variables
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS___LOADERS___AGP_LOAD_PAGE__HPP
