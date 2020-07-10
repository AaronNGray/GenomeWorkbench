#ifndef GUI_WIDGETS_EDIT___STRUCT_COMM_NAME_PANEL__HPP
#define GUI_WIDGETS_EDIT___STRUCT_COMM_NAME_PANEL__HPP
/*  $Id: struct_comm_field_panel.hpp 42434 2019-02-25 16:57:07Z filippov $
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
 *   Structured Comment panel used in the AECR dialog
 */

#include <corelib/ncbistd.hpp>
#include <gui/widgets/edit/field_name_panel.hpp>
#include <wx/string.h>

class wxChoice;

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
    class CSeq_entry_Handle;
END_SCOPE(objects)

class CAutoCompleteTextCtrl;
/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CSTRUCTCOMMENTFIELDPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSTRUCTCOMMENTFIELDPANEL_TITLE _("Structured Comment Field Panel")
#define SYMBOL_CSTRUCTCOMMENTFIELDPANEL_IDNAME ID_CSTRUCTCOMMENTFIELDPANEL
#define SYMBOL_CSTRUCTCOMMENTFIELDPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CSTRUCTCOMMENTFIELDPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CStructCommentFieldPanel class declaration
 */

class NCBI_GUIWIDGETS_EDIT_EXPORT CStructCommentFieldPanel : public CFieldNamePanel
{    
    DECLARE_DYNAMIC_CLASS( CStructCommentFieldPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CStructCommentFieldPanel();
    CStructCommentFieldPanel( wxWindow* parent, 
        wxWindowID id = SYMBOL_CSTRUCTCOMMENTFIELDPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CSTRUCTCOMMENTFIELDPANEL_POSITION, 
        const wxSize& size = SYMBOL_CSTRUCTCOMMENTFIELDPANEL_SIZE, 
        long style = SYMBOL_CSTRUCTCOMMENTFIELDPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CSTRUCTCOMMENTFIELDPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CSTRUCTCOMMENTFIELDPANEL_POSITION, 
        const wxSize& size = SYMBOL_CSTRUCTCOMMENTFIELDPANEL_SIZE, 
        long style = SYMBOL_CSTRUCTCOMMENTFIELDPANEL_STYLE );

    /// Destructor
    ~CStructCommentFieldPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CStructCommentFieldPanel event handler declarations
    void OnStrCmntChoiceSelected( wxCommandEvent& event);

    void OnFieldNameEnter( wxCommandEvent& event);

    void CompleteStructCommentFields(const set<string> &fields);
////@end CStructCommentFieldPanel event handler declarations

////@begin CStructCommentFieldPanel member function declarations
    virtual string GetFieldName(const bool subfield);
    virtual bool SetFieldName(const string& field);
    virtual void ClearValues(void);

    virtual string GetMacroFieldName(const string &target, const string& selected_field = kEmptyStr);

    static void GetStructCommentFields(const objects::CSeq_entry_Handle& seh, set<string> &fields, size_t max = numeric_limits<size_t>::max());
    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CStructCommentFieldPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CStructCommentFieldPanel member variables
    wxChoice* m_StructCommFieldType;
    CAutoCompleteTextCtrl* m_FieldName;
////@end CStructCommentFieldPanel member variables

    enum {
        ID_CSTRUCTCOMMENTFIELDPANEL = 6160,
        ID_STRCMNT_CHOICE,
        ID_STRCMNT_TXTCTRL,
    };
private:
    void x_UpdatePanel();
    // container to hold the unique structured comment field names
    set<wxString> m_Fields;
};

END_NCBI_SCOPE

#endif
    // _GUI_PACKAGES_STRUCT_COMM_FIELD_PANEL_H_
