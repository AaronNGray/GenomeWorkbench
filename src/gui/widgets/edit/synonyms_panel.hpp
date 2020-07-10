#ifndef GUI_WIDGETS_EDIT___SYNONYMS_PANEL__HPP
#define GUI_WIDGETS_EDIT___SYNONYMS_PANEL__HPP

/*  $Id: synonyms_panel.hpp 25028 2012-01-12 17:26:13Z katargir $
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

#include <wx/panel.h>

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
    class CScope;
END_SCOPE(objects)

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
#define SYMBOL_CSYNONYMSPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSYNONYMSPANEL_TITLE _("Synonyms Panel")
#define SYMBOL_CSYNONYMSPANEL_IDNAME ID_CSYNONYMSPANEL
#define SYMBOL_CSYNONYMSPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CSYNONYMSPANEL_POSITION wxDefaultPosition
////@end control identifiers

class CSerialObject;

/*!
 * CSynonymsPanel class declaration
 */

class CSynonymsPanel: public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CSynonymsPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSynonymsPanel();
    CSynonymsPanel( wxWindow* parent, CSerialObject& object, objects::CScope& scope,
                    wxWindowID id = SYMBOL_CSYNONYMSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSYNONYMSPANEL_POSITION, const wxSize& size = SYMBOL_CSYNONYMSPANEL_SIZE, long style = SYMBOL_CSYNONYMSPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSYNONYMSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSYNONYMSPANEL_POSITION, const wxSize& size = SYMBOL_CSYNONYMSPANEL_SIZE, long style = SYMBOL_CSYNONYMSPANEL_STYLE );

    /// Destructor
    ~CSynonymsPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CSynonymsPanel event handler declarations

////@end CSynonymsPanel event handler declarations

////@begin CSynonymsPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSynonymsPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CSynonymsPanel member variables
    /// Control identifiers
    enum {
        ID_CSYNONYMSPANEL = 10019,
        ID_WINDOW = 10020
    };
////@end CSynonymsPanel member variables

private:
    CSerialObject* m_Object;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___SYNONYMS_PANEL__HPP
