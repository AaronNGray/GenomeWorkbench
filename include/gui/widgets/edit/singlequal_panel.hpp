#ifndef GUI_PKG_SEQ_EDIT___SINGLEQUAL_PANEL__HPP
#define GUI_PKG_SEQ_EDIT___SINGLEQUAL_PANEL__HPP
/*  $Id: singlequal_panel.hpp 44582 2020-01-24 21:10:25Z asztalos $
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
 * Authors:  Andrea Asztalos
 */

#include <corelib/ncbistd.hpp>
#include <gui/widgets/edit/macro_editor_context.hpp>
#include <wx/panel.h>
#include <wx/event.h>
#include <wx/hyperlink.h>
#include <wx/checkbox.h>

class wxChoice;
class wxComboBox;
class wxTextCtrl;

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CSINGLEQUALPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSINGLEQUALPANEL_TITLE _("Single Qualifier Panel")
#define SYMBOL_CSINGLEQUALPANEL_IDNAME ID_CSINGLEQUALPANEL
#define SYMBOL_CSINGLEQUALPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CSINGLEQUALPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSingleQualPanel class declaration
 */

class CSingleQualPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CSingleQualPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSingleQualPanel();
    CSingleQualPanel( wxWindow* parent, EMacroFieldType type,
        wxWindowID id = SYMBOL_CSINGLEQUALPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CSINGLEQUALPANEL_POSITION, 
        const wxSize& size = SYMBOL_CSINGLEQUALPANEL_SIZE, 
        long style = SYMBOL_CSINGLEQUALPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CSINGLEQUALPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CSINGLEQUALPANEL_POSITION, 
        const wxSize& size = SYMBOL_CSINGLEQUALPANEL_SIZE, 
        long style = SYMBOL_CSINGLEQUALPANEL_STYLE );

    /// Destructor
    ~CSingleQualPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CSingleQualPanel event handler declarations
    void OnSelectClicked(wxHyperlinkEvent& event);
    void OnFieldSelected(wxCommandEvent& event);
////@end CSingleQualPanel event handler declarations

////@begin CSingleQualPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSingleQualPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    void SetField(const string& first_field, int col);
    bool IsMatchingRow() const { return m_MatchCheck->IsChecked();  }
    SFieldFromTable GetField();

    wxComboBox* m_FieldFirst;
    wxTextCtrl* m_Column;
    /// Control identifiers
    enum {
        ID_CSINGLEQUALPANEL = 10590,
        ID_SNGLQUALCH1,
        ID_SNGLQUALCOL
    };

private:
    ////@begin CSingleQualPanel member variables
    wxCheckBox* m_MatchCheck;

    void x_LoadChoices();
    EMacroFieldType m_FieldType{ EMacroFieldType::eNotSet };  // the type of fields loaded into m_FieldFirst
    EMacroFieldType m_MatchType{ EMacroFieldType::eNotSet };
    list<SFieldFromTable> m_AdditionalFields; // fields that are added to the list of fields by the user
////@end CSingleQualPanel member variables
};

END_NCBI_SCOPE

#endif
    // GUI_PKG_SEQ_EDIT___SINGLEQUAL_PANEL__HPP

