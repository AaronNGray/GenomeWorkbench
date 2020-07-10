#ifndef GUI_WIDGETS_EDIT___STRUCT_FIELDVALUE_PANEL__HPP
#define GUI_WIDGETS_EDIT___STRUCT_FIELDVALUE_PANEL__HPP
/*  $Id: struct_fieldvalue_panel.hpp 42432 2019-02-22 18:44:43Z filippov $
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
 * 
 *   Structured Comment panel used in Parse Text dialog
 */

#include <corelib/ncbistd.hpp>
#include <gui/widgets/edit/field_name_panel.hpp>

class wxChoice;

BEGIN_NCBI_SCOPE
/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CSTRUCTFIELDVALUEPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSTRUCTFIELDVALUEPANEL_TITLE _("StructuredCommentFieldValue")
#define SYMBOL_CSTRUCTFIELDVALUEPANEL_IDNAME ID_CSTRUCTFIELDVALUEPANEL
#define SYMBOL_CSTRUCTFIELDVALUEPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CSTRUCTFIELDVALUEPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CStructFieldValuePanel class declaration
 */

wxDEFINE_EVENT(wxEVT_COMMAND_UPDATE_STRUCTCOMMENT_FIELD, wxCommandEvent);

class NCBI_GUIWIDGETS_EDIT_EXPORT CStructFieldValuePanel : public CFieldNamePanel
{    
    DECLARE_DYNAMIC_CLASS( CStructFieldValuePanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CStructFieldValuePanel();
    CStructFieldValuePanel( wxWindow* parent, 
        wxWindowID id = SYMBOL_CSTRUCTFIELDVALUEPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CSTRUCTFIELDVALUEPANEL_POSITION, 
        const wxSize& size = SYMBOL_CSTRUCTFIELDVALUEPANEL_SIZE, 
        long style = SYMBOL_CSTRUCTFIELDVALUEPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CSTRUCTFIELDVALUEPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CSTRUCTFIELDVALUEPANEL_POSITION, 
        const wxSize& size = SYMBOL_CSTRUCTFIELDVALUEPANEL_SIZE, 
        long style = SYMBOL_CSTRUCTFIELDVALUEPANEL_STYLE );

    /// Destructor
    ~CStructFieldValuePanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CStructFieldValuePanel event handler declarations

////@end CStructFieldValuePanel event handler declarations

////@begin CStructFieldValuePanel member function declarations
    virtual string GetFieldName(const bool subfield);
    virtual bool SetFieldName(const string& field);

    void ListStructCommentFields(const set<string> &field_names);
    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CStructFieldValuePanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CStructFieldValuePanel member variables
    wxChoice* m_StructField;
////@end CStructFieldValuePanel member variables

    enum {
        ID_CSTRUCTFIELDVALUEPANEL = 6180,
        ID_STRCMNTFLD_CHOICE
    };
};

wxDECLARE_EVENT(wxEVT_COMMAND_UPDATE_STRUCTCOMMENT_FIELD, wxCommandEvent);

END_NCBI_SCOPE

#endif
    // GUI_WIDGETS_EDIT___STRUCT_FIELDVALUE_PANEL__HPP
