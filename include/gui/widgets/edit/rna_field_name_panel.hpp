#ifndef GUI_WIDGETS_EDIT___RNA_FIELD_NAME_PANEL__HPP
#define GUI_WIDGETS_EDIT___RNA_FIELD_NAME_PANEL__HPP
/*  $Id: rna_field_name_panel.hpp 44700 2020-02-25 19:02:59Z asztalos $
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

/*!
 * Includes
 */

////@begin includes
////@end includes
#include <wx/choice.h>
#include <wx/combobox.h>
#include <wx/stattext.h>
#include <wx/listbox.h>

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
#define SYMBOL_CRNAFIELDNAMEPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CRNAFIELDNAMEPANEL_TITLE _("RNAFieldNamePanel")
#define SYMBOL_CRNAFIELDNAMEPANEL_IDNAME ID_CRNAFIELDNAMEPANEL
#define SYMBOL_CRNAFIELDNAMEPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CRNAFIELDNAMEPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CRNAFieldNamePanel class declaration
 */

class NCBI_GUIWIDGETS_EDIT_EXPORT CRNAFieldNamePanel : public CFieldNamePanel
{    
    DECLARE_DYNAMIC_CLASS( CRNAFieldNamePanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CRNAFieldNamePanel();
    CRNAFieldNamePanel( wxWindow* parent, CRNAFieldNamePanel* sibling = NULL, 
        wxWindowID id = SYMBOL_CRNAFIELDNAMEPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CRNAFIELDNAMEPANEL_POSITION, 
        const wxSize& size = SYMBOL_CRNAFIELDNAMEPANEL_SIZE, 
        long style = SYMBOL_CRNAFIELDNAMEPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CRNAFIELDNAMEPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CRNAFIELDNAMEPANEL_POSITION, 
        const wxSize& size = SYMBOL_CRNAFIELDNAMEPANEL_SIZE, 
        long style = SYMBOL_CRNAFIELDNAMEPANEL_STYLE );

    /// Destructor
    ~CRNAFieldNamePanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CRNAFieldNamePanel event handler declarations

    /// wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_RNA_TYPE
    void OnRnaTypeSelected( wxCommandEvent& event );

    void OnRnaFieldSelected( wxCommandEvent& event );
////@end CRNAFieldNamePanel event handler declarations

////@begin CRNAFieldNamePanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CRNAFieldNamePanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CRNAFieldNamePanel member variables
    enum {
        ID_CRNAFIELDNAMEPANEL = 6060,
        ID_CRNAFIELDNAME_RNA_TYPE,
        ID_CRNAFIELDNAME_NCRNA_CLASS,
        ID_CRNAFIELDNAME_RNA_FIELD
    };
    wxStaticText* m_RnaTypeLabel;
    wxChoice* m_RNAType;
    wxComboBox* m_NcrnaClass;
    wxListBox* m_RnaField;
////@end CRNAFieldNamePanel member variables
    virtual string GetFieldName(const bool subfield = false);
    virtual bool SetFieldName(const string& field);

    string GetRnaType();
    bool SetRnaType(const string& rna_type);
    static vector<string> GetRNATypes();
    static vector<string> GetRNAFields();

    string GetRnaTypeOnly(void);
    string GetNcrnaType(void);
    string GetRnaField(void);
    virtual void ClearValues(void);
    virtual string GetMacroFieldName(const string &target, const string& selected_field = kEmptyStr);
private:
    CRNAFieldNamePanel* m_Sibling;

    void x_EnableNcRnaClass();
};


/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CDUALRNAFIELDNAMEPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CDUALRNAFIELDNAMEPANEL_TITLE _("DualRNAFieldNamePanel")
#define SYMBOL_CDUALRNAFIELDNAMEPANEL_IDNAME ID_CDUALRNAFIELDNAMEPANEL
#define SYMBOL_CDUALRNAFIELDNAMEPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CDUALRNAFIELDNAMEPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CDualRNAFieldNamePanel class declaration
 */

class NCBI_GUIWIDGETS_EDIT_EXPORT CDualRNAFieldNamePanel : public CFieldNamePanel
{    
    DECLARE_DYNAMIC_CLASS( CDualRNAFieldNamePanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CDualRNAFieldNamePanel();
    CDualRNAFieldNamePanel( wxWindow* parent, 
        wxWindowID id = SYMBOL_CDUALRNAFIELDNAMEPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CDUALRNAFIELDNAMEPANEL_POSITION, 
        const wxSize& size = SYMBOL_CDUALRNAFIELDNAMEPANEL_SIZE, 
        long style = SYMBOL_CDUALRNAFIELDNAMEPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CDUALRNAFIELDNAMEPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CDUALRNAFIELDNAMEPANEL_POSITION, 
        const wxSize& size = SYMBOL_CDUALRNAFIELDNAMEPANEL_SIZE, 
        long style = SYMBOL_CDUALRNAFIELDNAMEPANEL_STYLE );

    /// Destructor
    ~CDualRNAFieldNamePanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CDualRNAFieldNamePanel event handler declarations

    /// wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_RNA_TYPE
    void OnRnaTypeSelected( wxCommandEvent& event );

    void OnRnaFieldSelected( wxCommandEvent& event );
////@end CDualRNAFieldNamePanel event handler declarations

////@begin CDualRNAFieldNamePanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CDualRNAFieldNamePanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CDualRNAFieldNamePanel member variables
    enum {
        ID_CDUALRNAFIELDNAMEPANEL = 6070,
        ID_CRNAFIELDNAME_RNA_TYPE,
        ID_CRNAFIELDNAME_NCRNA_CLASS,
        ID_CRNAFIELDNAME_RNA_FIELD,
        ID_CRNAFIELDNAME_RNA_FIELD2
    };
    wxStaticText* m_RnaTypeLabel;
    wxChoice* m_RNAType;
    wxComboBox* m_NcrnaClass;
    wxListBox* m_RnaField;
    wxListBox* m_RnaField2;
////@end CDualRNAFieldNamePanel member variables
    virtual string GetFieldName(const bool subfield = false);
    virtual bool SetFieldName(const string& field);
    string GetFieldName2(const bool subfield = false);
    bool SetFieldName2(const string& field);

    string GetRnaType();
    bool SetRnaType(const string& rna_type);

    string GetRnaTypeOnly(void);
    string GetNcrnaType(void);
    string GetRnaField(void);
    string GetRnaField2(void);
    virtual void ClearValues(void);
private:
    void x_EnableNcRnaClass();
};

END_NCBI_SCOPE

#endif
    // _RNA_FIELD_NAME_PANEL_H_
