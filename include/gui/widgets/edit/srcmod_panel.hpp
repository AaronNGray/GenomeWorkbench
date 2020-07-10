#ifndef GUI_WIDGETS_EDIT___SRCMOD_PANEL__HPP
#define GUI_WIDGETS_EDIT___SRCMOD_PANEL__HPP

/*  $Id: srcmod_panel.hpp 43092 2019-05-14 15:50:00Z bollin $
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
 * Authors:  Vasuki Palanigobu
 */

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/sizer.h>
#include <wx/arrstr.h>

#include <objects/seqfeat/SubSource.hpp>
#include <objects/seqfeat/OrgMod.hpp>

#include <gui/widgets/edit/srcmod_edit_panel.hpp>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_SRCMODPANEL 10013
#define ID_MODIFIER_NAME 10010
#define SYMBOL_CSRCMODPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSRCMODPANEL_TITLE _("Modifier")
#define SYMBOL_CSRCMODPANEL_IDNAME ID_SRCMODPANEL
#define SYMBOL_CSRCMODPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CSRCMODPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSrcModPanel class declaration
 */

class NCBI_GUIWIDGETS_EDIT_EXPORT CSrcModPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CSrcModPanel )
    DECLARE_EVENT_TABLE()

public:
	struct SModData {
		wxString name;
		wxString value;
	};
    
    /// Constructors
    CSrcModPanel();
    CSrcModPanel( wxWindow* parent, SModData mod_data, wxWindowID id = SYMBOL_CSRCMODPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSRCMODPANEL_POSITION, const wxSize& size = SYMBOL_CSRCMODPANEL_SIZE, long style = SYMBOL_CSRCMODPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSRCMODPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSRCMODPANEL_POSITION, const wxSize& size = SYMBOL_CSRCMODPANEL_SIZE, long style = SYMBOL_CSRCMODPANEL_STYLE );

    /// Destructor
    ~CSrcModPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

	  void SetModifierData();
    const SModData&	 GetModifierData(); //CHECK!!!

    void SetModifierData(const string& mod_name, const string& mod_val);

////@begin CSrcModPanel event handler declarations

    /// wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_MODIFIER_NAME
    void OnModifierNameSelected( wxCommandEvent& event );
	void OnLeftDown( wxMouseEvent& event);
	void OnLeftUp( wxMouseEvent& event);

////@end CSrcModPanel event handler declarations

    void OnEditorChange( );

////@begin CSrcModPanel member function declarations

    wxArrayString s_GetModifierList();
    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSrcModPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CSrcModPanel member variables
    wxBoxSizer* m_Sizer;
    wxComboBox* m_SrcModName;
////@end CSrcModPanel member variables

    virtual bool DoNotListOrgMod(int st);
    virtual bool DoNotListSubSrc(int st);

private:
	  SModData	m_ModData;
      CSrcModEditPanel* m_Editor;
	  string m_Selected;

    void x_SetEditor(CSrcModEditPanel::EEditorType editor_type);
};


END_NCBI_SCOPE

#endif
    //GUI_WIDGETS_EDIT___SRCMOD_PANEL__HPP
