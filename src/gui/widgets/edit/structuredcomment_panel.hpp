/*  $Id: structuredcomment_panel.hpp 42583 2019-03-22 14:42:04Z bollin $
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
#ifndef _STRUCTUREDCOMMENTPANEL_H_
#define _STRUCTUREDCOMMENTPANEL_H_

#include <corelib/ncbistd.hpp>
#include <objects/general/User_object.hpp>
#include <objects/valid/Comment_set.hpp>
#include <gui/widgets/edit/utilities.hpp>
#include <gui/widgets/edit/import_export_access.hpp>

#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/hyperlink.h>

class wxBoxSizer;
class wxCheckBox;
class wxStaticText;
class wxScrolledWindow;
class wxTextCtrl;
class wxChoice;


BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CSTRUCTUREDCOMMENTPANEL_STYLE wxCAPTION|wxTAB_TRAVERSAL
#define SYMBOL_CSTRUCTUREDCOMMENTPANEL_TITLE _("StructuredCommentPanel")
#define SYMBOL_CSTRUCTUREDCOMMENTPANEL_IDNAME ID_CSTRUCTUREDCOMMENTPANEL
#define SYMBOL_CSTRUCTUREDCOMMENTPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CSTRUCTUREDCOMMENTPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CStructuredCommentPanel class declaration
 */

class CStructuredCommentPanel: public wxPanel, public IDescEditorPanel, public CImportExportAccess
{    
    DECLARE_DYNAMIC_CLASS( CStructuredCommentPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CStructuredCommentPanel();
    CStructuredCommentPanel( wxWindow* parent, CRef<objects::CUser_object> user, 
        wxWindowID id = SYMBOL_CSTRUCTUREDCOMMENTPANEL_IDNAME, 
        const wxString& caption = SYMBOL_CSTRUCTUREDCOMMENTPANEL_TITLE, 
        const wxPoint& pos = SYMBOL_CSTRUCTUREDCOMMENTPANEL_POSITION, 
        const wxSize& size = SYMBOL_CSTRUCTUREDCOMMENTPANEL_SIZE, 
        long style = SYMBOL_CSTRUCTUREDCOMMENTPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CSTRUCTUREDCOMMENTPANEL_IDNAME, 
        const wxString& caption = SYMBOL_CSTRUCTUREDCOMMENTPANEL_TITLE, 
        const wxPoint& pos = SYMBOL_CSTRUCTUREDCOMMENTPANEL_POSITION, 
        const wxSize& size = SYMBOL_CSTRUCTUREDCOMMENTPANEL_SIZE, 
        long style = SYMBOL_CSTRUCTUREDCOMMENTPANEL_STYLE );

    /// Destructor
    ~CStructuredCommentPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    CRef<objects::CUser_object> GetUser_object() const;


////@begin CStructuredCommentPanel event handler declarations

    /// wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_CHOICE11
    void OnChoice11Selected( wxCommandEvent& event );

    /// wxEVT_COMMAND_HYPERLINK event handler for ID_HYPERLINKCTRL
    void OnHyperlinkctrlHyperlinkClicked( wxHyperlinkEvent& event );

    void OnTextEntered(wxCommandEvent& event);

////@end CStructuredCommentPanel event handler declarations

////@begin CStructuredCommentPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CStructuredCommentPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CStructuredCommentPanel member variables
    wxBoxSizer* m_StandardCommentSizer;
    wxStaticText* m_StandardCommentTypeLabel;
    wxChoice* m_StandardCommentCtrl;
    wxScrolledWindow* m_ScrolledWindow;
    wxBoxSizer* m_Sizer;
    /// Control identifiers
    enum {
        ID_CSTRUCTUREDCOMMENTPANEL = 10073,
        ID_CHOICE11 = 10075,
        ID_HYPERLINKCTRL = 10074,
        ID_SCROLLEDWINDOW = 10045
    };
////@end CStructuredCommentPanel member variables

    // IDescEditorPanel implementation
    virtual void ChangeSeqdesc(const objects::CSeqdesc& desc);
    virtual void UpdateSeqdesc(objects::CSeqdesc& desc);

    virtual bool IsImportEnabled(void);
    virtual bool IsExportEnabled(void);
    virtual CRef<CSerialObject> OnExport(void);
    virtual void OnImport( CNcbiIfstream &istr);

    static void s_RemovePoundSigns (string& str);
    static void s_AddPoundSigns (string& str);
private:
    CRef<objects::CUser_object> m_User;
    int m_TotalHeight;
    int m_ScrollRate;
    vector <wxControl*> m_FieldNameCtrls;
    vector <wxTextCtrl*> m_FieldValueCtrls;

    CConstRef<objects::CComment_set> m_StructuredCommentRules;

    void s_AddRow (string field_name, string field_value, bool static_fieldname = false, bool is_required = false);
    bool x_GetFields (bool keep_blanks = false);
    int  x_FindRow(wxWindow* wnd, wxSizerItemList& itemList);
    void OnDelete (wxHyperlinkEvent& event);
};

END_NCBI_SCOPE

#endif
    // _STRUCTUREDCOMMENTPANEL_H_
