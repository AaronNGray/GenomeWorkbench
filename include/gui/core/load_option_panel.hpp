/*  $Id: load_option_panel.hpp 25477 2012-03-27 14:53:29Z kuznets $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */
#ifndef _LOAD_OPTION_PANEL_H_
#define _LOAD_OPTION_PANEL_H_


/*!
 * Includes
 */

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

////@begin control identifiers
#define SYMBOL_CLOADOPTIONPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CLOADOPTIONPANEL_TITLE wxT("Dialog")
#define SYMBOL_CLOADOPTIONPANEL_IDNAME ID_CLOADOPTIONPANEL
#define SYMBOL_CLOADOPTIONPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CLOADOPTIONPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CLoadOptionPanel class declaration
 */

class CLoadOptionPanel: public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CLoadOptionPanel )
    DECLARE_EVENT_TABLE()

public:
    CLoadOptionPanel();
    CLoadOptionPanel( wxWindow* parent, wxWindowID id = SYMBOL_CLOADOPTIONPANEL_IDNAME, const wxPoint& pos = SYMBOL_CLOADOPTIONPANEL_POSITION, const wxSize& size = SYMBOL_CLOADOPTIONPANEL_SIZE, long style = SYMBOL_CLOADOPTIONPANEL_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CLOADOPTIONPANEL_IDNAME, const wxPoint& pos = SYMBOL_CLOADOPTIONPANEL_POSITION, const wxSize& size = SYMBOL_CLOADOPTIONPANEL_SIZE, long style = SYMBOL_CLOADOPTIONPANEL_STYLE );

    ~CLoadOptionPanel();

    void Init();

    void CreateControls();

////@begin CLoadOptionPanel event handler declarations
////@end CLoadOptionPanel event handler declarations

////@begin CLoadOptionPanel member function declarations
    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CLoadOptionPanel member function declarations

    static bool ShowToolTips();

////@begin CLoadOptionPanel member variables
    enum {
        ID_CLOADOPTIONPANEL = 10005
    };
////@end CLoadOptionPanel member variables
};

#endif
    // _LOAD_OPTION_PANEL_H_
