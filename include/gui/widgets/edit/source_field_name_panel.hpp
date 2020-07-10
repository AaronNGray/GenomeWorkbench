#ifndef GUI_WIDGETS_EDIT___SOURCE_FIELD_NAME_PANEL__HPP
#define GUI_WIDGETS_EDIT___SOURCE_FIELD_NAME_PANEL__HPP
/*  $Id: source_field_name_panel.hpp 40627 2018-03-21 15:34:12Z asztalos $
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
#include <wx/listbox.h>
#include <wx/sizer.h>
#include <wx/radiobut.h>

class wxRadioBox;
BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CSOURCEFIELDNAMEPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSOURCEFIELDNAMEPANEL_TITLE _("SourceFieldNamePanel")
#define SYMBOL_CSOURCEFIELDNAMEPANEL_IDNAME ID_CSOURCEFIELDNAMEPANEL
#define SYMBOL_CSOURCEFIELDNAMEPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CSOURCEFIELDNAMEPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSourceFieldNamePanel class declaration
 */

class NCBI_GUIWIDGETS_EDIT_EXPORT CSourceFieldNamePanel : public CFieldNamePanel
{    
    DECLARE_DYNAMIC_CLASS( CSourceFieldNamePanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSourceFieldNamePanel();
    CSourceFieldNamePanel( wxWindow* parent, 
        bool additional_items = false,
        wxWindowID id = SYMBOL_CSOURCEFIELDNAMEPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CSOURCEFIELDNAMEPANEL_POSITION, 
        const wxSize& size = SYMBOL_CSOURCEFIELDNAMEPANEL_SIZE, 
        long style = SYMBOL_CSOURCEFIELDNAMEPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CSOURCEFIELDNAMEPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CSOURCEFIELDNAMEPANEL_POSITION, 
        const wxSize& size = SYMBOL_CSOURCEFIELDNAMEPANEL_SIZE, 
        long style = SYMBOL_CSOURCEFIELDNAMEPANEL_STYLE );

    /// Destructor
    ~CSourceFieldNamePanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CSourceFieldNamePanel event handler declarations

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_TEXT_QUAL_BTN
    void OnTextQualBtnSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_TAXONOMY_BTN
    void OnTaxonomyBtnSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_LOCATION_BTN
    void OnLocationBtnSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_ORIGIN_BTN
    void OnOriginBtnSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_SRC_SUBLIST
    void OnSrcSublistSelected( wxCommandEvent& event );

////@end CSourceFieldNamePanel event handler declarations

////@begin CSourceFieldNamePanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSourceFieldNamePanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    void HideSourceType();
    void DoNotShowSubList(bool val) {m_DoNotShowSubList = val;}
////@begin CSourceFieldNamePanel member variables
    enum {
        ID_CSOURCEFIELDNAMEPANEL = 6040,
        ID_CSOURCEFIELD_TEXT_QUAL_BTN,
        ID_CSOURCEFIELD_TAXONOMY_BTN,
        ID_CSOURCEFIELD_LOCATION_BTN,
        ID_CSOURCEFIELD_ORIGIN_BTN,
        ID_CSOURCEFIELD_SRC_SUBLIST,
        ID_CSOURCEFIELD_SRCTYPES
    };
    wxRadioButton* m_TextQualifier;
    wxRadioButton* m_Taxonomy;
    wxRadioButton* m_Location;
    wxRadioButton* m_Origin;
    wxListBox* m_SourceSubList;
    wxRadioBox* m_SourceType;
////@end CSourceFieldNamePanel member variables
    virtual string GetFieldName(const bool subfield = false);
    virtual bool SetFieldName(const string& field);
    virtual vector<string> GetChoices(bool &allow_other);

    virtual string GetMacroFieldName(const string &target, const string& selected_field = kEmptyStr);

    void SetSourceSubpanelSelection(const string &sel);
    void AddAdditionalItems(void) {m_AdditionalItems = true; x_ChooseTextQualifier();}
    virtual void ClearValues(void);

    static vector<string> s_GetSourceOriginOptions();
    static vector<string> s_GetSourceLocationOptions();
private:
    void x_ChooseTextQualifier();
    void x_ChooseTaxonomy();
    void x_ChooseLocation();
    bool m_AdditionalItems;
    bool m_DoNotShowSubList;
    bool m_is_taxonomy_panel;
    bool m_first_run;
};

END_NCBI_SCOPE

#endif
    // GUI_WIDGETS_EDIT___SOURCE_FIELD_NAME_PANEL__HPP
