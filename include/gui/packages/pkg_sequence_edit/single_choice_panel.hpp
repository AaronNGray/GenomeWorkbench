#ifndef PKG_SEQUENCE_EDIT___SINGLE_CHOICE_PANEL__HPP
#define PKG_SEQUENCE_EDIT___SINGLE_CHOICE_PANEL__HPP
/*  $Id: single_choice_panel.hpp 39649 2017-10-24 15:22:12Z asztalos $
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
 * Authors:  Colleen Bollin, Andrea Asztalos
 * 
 * This panel contains one single wxChoice control. It can be used as a base class
 * for defining panels that handle various types of fields, e.g., misc_field, gene_field.
 */

#include <corelib/ncbistd.hpp>
#include <gui/widgets/edit/field_name_panel.hpp>

#include <wx/choice.h>

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CSINGLECHOICEPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSINGLECHOICEPANEL_TITLE _("SingleChoicePanel")
#define SYMBOL_CSINGLECHOICEPANEL_IDNAME ID_CSINGLECHOICEPANEL
#define SYMBOL_CSINGLECHOICEPANEL_SIZE wxDefaultSize
#define SYMBOL_CSINGLECHOICEPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSingleChoicePanel class declaration
 */

class CSingleChoicePanel : public CFieldNamePanel
{    
    DECLARE_ABSTRACT_CLASS(CSingleChoicePanel)
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSingleChoicePanel();
    CSingleChoicePanel(wxWindow* parent, const vector<string>& choices, 
        wxWindowID id = SYMBOL_CSINGLECHOICEPANEL_IDNAME,
        const wxPoint& pos = SYMBOL_CSINGLECHOICEPANEL_POSITION,
        const wxSize& size = SYMBOL_CSINGLECHOICEPANEL_SIZE,
        long style = SYMBOL_CSINGLECHOICEPANEL_STYLE);

    bool Create(wxWindow* parent, 
        wxWindowID id = SYMBOL_CSINGLECHOICEPANEL_IDNAME,
        const wxPoint& pos = SYMBOL_CSINGLECHOICEPANEL_POSITION,
        const wxSize& size = SYMBOL_CSINGLECHOICEPANEL_SIZE,
        long style = SYMBOL_CSINGLECHOICEPANEL_STYLE);

    /// Destructor
    ~CSingleChoicePanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CSingleChoicePanel event handler declarations

    /// wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_FIELD_CHOICE
    void OnFieldChoiceSelected( wxCommandEvent& event );

////@end CSingleChoicePanel event handler declarations

////@begin CSingleChoicePanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSingleChoicePanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    virtual vector<string> GetChoices(bool& allow_other);
    enum {
        ID_CSINGLECHOICEPANEL = 6100,
        ID_CSINGLECHOICE_FIELD
    };
protected:
    ////@begin CSingleChoicePanel member variables
    wxChoice* m_FieldChoice;
    vector<string> m_Choices;
    ////@end CSingleChoicePanel member variables
};

END_NCBI_SCOPE

#endif
    // PKG_SEQUENCE_EDIT___SINGLE_CHOICE_PANEL__HPP
