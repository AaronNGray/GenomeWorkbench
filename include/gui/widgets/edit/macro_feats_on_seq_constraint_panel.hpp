#ifndef GUI_WIDGETS_EDIT___MACRO_FEATS_ON_SEQ_CONSTRAINT_PANEL__HPP
#define GUI_WIDGETS_EDIT___MACRO_FEATS_ON_SEQ_CONSTRAINT_PANEL__HPP
/*  $Id: macro_feats_on_seq_constraint_panel.hpp 43627 2019-08-09 18:55:07Z asztalos $
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
#include <gui/widgets/edit/feature_field_name_panel.hpp>

#include <wx/panel.h>

class wxChoice;
class wxTextCtrl;

BEGIN_NCBI_SCOPE


/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_MACRO_FEATS_ON_SEQ wxID_ANY
#define ID_MACRO_FEATS_CHOICE 10089
#define ID_MACRO_FEATS_LENGTHTEXT 10090
#define ID_MACRO_FEATS_DISTCHOICE1 10091
#define ID_MACRO_FEATS_DISTCHOICE2 10092
#define SYMBOL_MACRO_FEATS_ON_SEQ_STYLE wxTAB_TRAVERSAL 
#define SYMBOL_MACRO_FEATS_ON_SEQ_IDNAME ID_MACRO_FEATS_ON_SEQ
#define SYMBOL_MACRO_FEATS_ON_SEQ_SIZE wxDefaultSize
#define SYMBOL_MACRO_FEATS_ON_SEQ_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CMacroFeatsOnSeqPanel class declaration
 */

class CMacroFeatsOnSeqPanel : public wxPanel, public CMacroMatcherPanelBase
{    
    DECLARE_CLASS(CMacroFeatsOnSeqPanel)
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CMacroFeatsOnSeqPanel();
    CMacroFeatsOnSeqPanel(wxWindow* parent, 
        wxWindowID id = SYMBOL_MACRO_FEATS_ON_SEQ_IDNAME,
        const wxPoint& pos = SYMBOL_MACRO_FEATS_ON_SEQ_POSITION,
        const wxSize& size = SYMBOL_MACRO_FEATS_ON_SEQ_SIZE,
        long style = SYMBOL_MACRO_FEATS_ON_SEQ_STYLE);

    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_MACRO_FEATS_ON_SEQ_IDNAME,
        const wxPoint& pos = SYMBOL_MACRO_FEATS_ON_SEQ_POSITION,
        const wxSize& size = SYMBOL_MACRO_FEATS_ON_SEQ_SIZE,
        long style = SYMBOL_MACRO_FEATS_ON_SEQ_STYLE);

    /// Destructor
    ~CMacroFeatsOnSeqPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CMacroFeatsOnSeqPanel event handler declarations
    void OnLengthTextCtrlUpdate(wxUpdateUIEvent &event);
////@end CMacroFeatsOnSeqPanel event handler declarations

////@begin CMacroFeatsOnSeqPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CMacroFeatsOnSeqPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    void OnDistanceSelected(wxCommandEvent& event);

    virtual pair<string, string> GetMatcher(const pair<string, string>& target, size_t num);
    virtual void SetMatcher(const string &item);

    string GetLocationConstraint(const string &feat, const string& target);
    string GetDescription(const string& feat) const;
private:

    wxArrayString x_GetFeatureChoices();
////@begin CMacroFeatsOnSeqPanel member variables
    CFeatureTypePanel* m_FeatureType;
    wxChoice* m_FeatChoice;
    wxTextCtrl* m_FeatCount;

    wxChoice* m_LengthChoice;
    wxTextCtrl* m_Length;

    wxChoice* m_SeqReprChoice;
    wxChoice* m_Dist5EndChoice;
    wxChoice* m_Dist3EndChoice;
    wxTextCtrl* m_5EndDistance;
    wxTextCtrl* m_3EndDistance;
    wxChoice* m_FirstFeatureChoice;
    wxChoice* m_LastFeatureChoice;

////@end CMacroFeatsOnSeqPanel member variables
};

END_NCBI_SCOPE
#endif
    // GUI_WIDGETS_EDIT___MACRO_FEATS_ON_SEQ_CONSTRAINT_PANEL__HPP
