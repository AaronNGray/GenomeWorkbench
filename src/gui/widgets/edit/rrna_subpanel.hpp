#ifndef GUI_WIDGETS_EDIT___RRNA_SUBPANEL__HPP
#define GUI_WIDGETS_EDIT___RRNA_SUBPANEL__HPP

/*  $Id: rrna_subpanel.hpp 29198 2013-11-13 20:34:31Z bollin $
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

////@begin includes
////@end includes
#include <gui/widgets/wx/auto_complete_text_ctrl.hpp>
#include <gui/widgets/wx/rna_name_completer.hpp>

#include "wx/panel.h"
#include <wx/combobox.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
class CAutoCompleteTextCtrl;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CRRNASUBPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CRRNASUBPANEL_TITLE _("rRNA SubPanel")
#define SYMBOL_CRRNASUBPANEL_IDNAME ID_CRRNASUBPANEL
#define SYMBOL_CRRNASUBPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CRRNASUBPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CrRNASubPanel class declaration
 */

class CrRNASubPanel: public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CrRNASubPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CrRNASubPanel();
    CrRNASubPanel( wxWindow* parent, wxWindowID id = SYMBOL_CRRNASUBPANEL_IDNAME, const wxPoint& pos = SYMBOL_CRRNASUBPANEL_POSITION, const wxSize& size = SYMBOL_CRRNASUBPANEL_SIZE, long style = SYMBOL_CRRNASUBPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CRRNASUBPANEL_IDNAME, const wxPoint& pos = SYMBOL_CRRNASUBPANEL_POSITION, const wxSize& size = SYMBOL_CRRNASUBPANEL_SIZE, long style = SYMBOL_CRRNASUBPANEL_STYLE );

    /// Destructor
    ~CrRNASubPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CrRNASubPanel event handler declarations

////@end CrRNASubPanel event handler declarations

////@begin CrRNASubPanel member function declarations

    wxString GetRnaName() const;
    void SetRnaName(wxString value);

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CrRNASubPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CrRNASubPanel member variables
    CAutoCompleteTextCtrl* m_NameCtrl;
private:
    /// Control identifiers
    enum {
        ID_CRRNASUBPANEL = 10026,
        ID_RRNA_NAME_CTRL = 10027
    };
////@end CrRNASubPanel member variables
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___MRNA_SUBPANEL__HPP
