#ifndef GUI_WIDGETS_EDIT___MACRO_CHOICETYPE_CONSTRAINT_PANEL__HPP
#define GUI_WIDGETS_EDIT___MACRO_CHOICETYPE_CONSTRAINT_PANEL__HPP
/*  $Id: macro_choicetype_constraint_panel.hpp 40627 2018-03-21 15:34:12Z asztalos $
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
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Igor Filippov
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>

#include <objects/macro/Location_constraint.hpp>
#include <gui/widgets/edit/field_name_panel.hpp>
#include <gui/widgets/edit/macro_item_constraint_panel.hpp>

#include <wx/panel.h>

class wxChoice;
class wxTextCtrl;

BEGIN_NCBI_SCOPE


/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_MACRO_CHOICETYPE wxID_ANY
#define SYMBOL_MACRO_CHOICETYPE_STYLE wxTAB_TRAVERSAL 
#define SYMBOL_MACRO_CHOICETYPE_IDNAME ID_MACRO_CHOICETYPE
#define SYMBOL_MACRO_CHOICETYPE_SIZE wxDefaultSize
#define SYMBOL_MACRO_CHOICETYPE_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CMacroChoiceTypePanel class declaration
 */

class CMacroChoiceTypePanel : public wxPanel, public CMacroMatcherPanelBase
{    
    DECLARE_CLASS(CMacroChoiceTypePanel)
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CMacroChoiceTypePanel();
  CMacroChoiceTypePanel(wxWindow* parent, const string& target,
			wxWindowID id = SYMBOL_MACRO_CHOICETYPE_IDNAME,
			const wxPoint& pos = SYMBOL_MACRO_CHOICETYPE_POSITION,
			const wxSize& size = SYMBOL_MACRO_CHOICETYPE_SIZE,
			long style = SYMBOL_MACRO_CHOICETYPE_STYLE);

    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_MACRO_CHOICETYPE_IDNAME,
        const wxPoint& pos = SYMBOL_MACRO_CHOICETYPE_POSITION,
        const wxSize& size = SYMBOL_MACRO_CHOICETYPE_SIZE,
        long style = SYMBOL_MACRO_CHOICETYPE_STYLE);

    /// Destructor
    ~CMacroChoiceTypePanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CMacroChoiceTypePanel event handler declarations
////@end CMacroChoiceTypePanel event handler declarations

////@begin CMacroChoiceTypePanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CMacroChoiceTypePanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    virtual pair<string, string> GetMatcher(const pair<string, string>& target, size_t num);
    virtual void SetMatcher(const string &item);

    string GetLocationConstraint(const string &feat);
    string GetDescription(const string& feat) const;
private:
  void x_PopulateList();
////@begin CMacroChoiceTypePanel member variables
  wxListBox* m_List;
  string m_target;
  string m_field;
////@end CMacroChoiceTypePanel member variables
};

END_NCBI_SCOPE
#endif
    // GUI_WIDGETS_EDIT___MACRO_CHOICETYPE_CONSTRAINT_PANEL__HPP
