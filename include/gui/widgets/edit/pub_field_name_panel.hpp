#ifndef GUI_WIDGETS_EDIT___PUB_FIELD_NAME_PANEL__HPP
#define GUI_WIDGETS_EDIT___PUB_FIELD_NAME_PANEL__HPP
/*  $Id: pub_field_name_panel.hpp 41307 2018-07-03 18:34:58Z filippov $
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
 * Authors:  Colleen Bollin
 */

#include <corelib/ncbistd.hpp>
#include <gui/widgets/edit/field_name_panel.hpp>

#include <wx/panel.h>
#include <wx/listbox.h>


BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CFIELDHANDLERNAMEPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CFIELDHANDLERNAMEPANEL_TITLE _("PubFieldNamePanel")
#define SYMBOL_CFIELDHANDLERNAMEPANEL_IDNAME ID_PUBFIELDNAMEPANEL
#define SYMBOL_CFIELDHANDLERNAMEPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CFIELDHANDLERNAMEPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CFieldHandlerNamePanel class declaration
 */

class NCBI_GUIWIDGETS_EDIT_EXPORT CFieldHandlerNamePanel : public CFieldNamePanel
{    
    DECLARE_DYNAMIC_CLASS( CFieldHandlerNamePanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CFieldHandlerNamePanel();
    CFieldHandlerNamePanel( wxWindow* parent, 
        wxWindowID id = SYMBOL_CFIELDHANDLERNAMEPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CFIELDHANDLERNAMEPANEL_POSITION, 
        const wxSize& size = SYMBOL_CFIELDHANDLERNAMEPANEL_SIZE, 
        long style = SYMBOL_CFIELDHANDLERNAMEPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CFIELDHANDLERNAMEPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CFIELDHANDLERNAMEPANEL_POSITION, 
        const wxSize& size = SYMBOL_CFIELDHANDLERNAMEPANEL_SIZE, 
        long style = SYMBOL_CFIELDHANDLERNAMEPANEL_STYLE );

    /// Destructor
    ~CFieldHandlerNamePanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CFieldHandlerNamePanel event handler declarations

    /// wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_PUBFIELDNAME_FIELD
    void OnFieldSelected( wxCommandEvent& event );

////@end CFieldHandlerNamePanel event handler declarations

////@begin CFieldHandlerNamePanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CFieldHandlerNamePanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CFieldHandlerNamePanel member variables
    wxListBox* m_Field;
////@end CFieldHandlerNamePanel member variables

    virtual string GetFieldName(const bool subfield = false);
    virtual bool SetFieldName(const string& field);
    void SetFieldNames(const vector<string>& field_names);
    virtual vector<string> GetChoices(bool& allow_other);
    virtual vector<string> GetMacroValues();
    virtual void ClearValues(void);
    virtual string GetMacroFieldName(const string &target, const string& selected_field = kEmptyStr);
    virtual string GetMacroFieldLabel(const string& target, const string& selected_field = kEmptyStr);
    void SetMacroFieldNames(const vector<string>& field_names);
    void SetMacroSelf(const string& self) {m_self = self;}

    enum {
        ID_PUBFIELDNAMEPANEL = 6130,
        ID_PUBFIELDNAME_FIELD
    };
private:
    vector<string> m_MacroField;
    string m_self;
};

END_NCBI_SCOPE

#endif
    // GUI_WIDGETS_EDIT___PUB_FIELD_NAME_PANEL__HPP
