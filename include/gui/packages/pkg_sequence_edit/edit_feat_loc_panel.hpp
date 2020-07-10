/*  $Id: edit_feat_loc_panel.hpp 34894 2016-02-26 19:03:19Z asztalos $
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
#ifndef _EDIT_FEAT_LOC_PANEL_H_
#define _EDIT_FEAT_LOC_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <objtools/edit/loc_edit.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/panel.h>

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
#define SYMBOL_CEDITFEATLOCPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CEDITFEATLOCPANEL_TITLE _("EditFeatLocPanel")
#define SYMBOL_CEDITFEATLOCPANEL_IDNAME ID_CEDITFEATLOCPANEL
#define SYMBOL_CEDITFEATLOCPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CEDITFEATLOCPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CEditFeatLocPanel class declaration
 */

class CEditFeatLocPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CEditFeatLocPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CEditFeatLocPanel();
    CEditFeatLocPanel( wxWindow* parent, wxWindowID id = SYMBOL_CEDITFEATLOCPANEL_IDNAME, const wxPoint& pos = SYMBOL_CEDITFEATLOCPANEL_POSITION, const wxSize& size = SYMBOL_CEDITFEATLOCPANEL_SIZE, long style = SYMBOL_CEDITFEATLOCPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CEDITFEATLOCPANEL_IDNAME, const wxPoint& pos = SYMBOL_CEDITFEATLOCPANEL_POSITION, const wxSize& size = SYMBOL_CEDITFEATLOCPANEL_SIZE, long style = SYMBOL_CEDITFEATLOCPANEL_STYLE );

    /// Destructor
    ~CEditFeatLocPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CEditFeatLocPanel event handler declarations

    /// wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_EFL_PARTIAL5
    void OnEflPartial5Selected( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_EFL_PARTIAL3
    void OnEflPartial3Selected( wxCommandEvent& event );

////@end CEditFeatLocPanel event handler declarations

////@begin CEditFeatLocPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CEditFeatLocPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CEditFeatLocPanel member variables
    wxChoice* m_Adjust5Partial;
    wxCheckBox* m_Extend5Btn;
    wxChoice* m_Adjust3Partial;
    wxCheckBox* m_Extend3Btn;
    wxChoice* m_JoinOrderMerge;
    wxCheckBox* m_RetranslateBtn;
    wxCheckBox* m_AdjustGeneBtn;
    /// Control identifiers
    enum {
        ID_CEDITFEATLOCPANEL = 6430,
        ID_EFL_PARTIAL5 = 6431,
        ID_EFL_EXTEND5 = 6432,
        ID_EFL_PARTIAL3 = 6433,
        ID_EFL_EXTEND3 = 6434,
        ID_EFL_JOM = 6435,
        ID_EFL_RETRANSLATE = 6436,
        ID_EFL_ADJUST_GENE = 6437
    };
////@end CEditFeatLocPanel member variables
    
    void SetPolicy(CRef<objects::edit::CLocationEditPolicy> policy);
    CRef<objects::edit::CLocationEditPolicy> GetPolicy();
    void SetRetranslate(bool val);
    bool GetRetranslate();
    void SetAdjustGene(bool val);
    bool GetAdjustGene();

private:
    bool m_Retranslate;
    bool m_AdjustGene;
    objects::edit::CLocationEditPolicy::EPartialPolicy x_GetPartial5Policy();
    objects::edit::CLocationEditPolicy::EPartialPolicy x_GetPartial3Policy();
    objects::edit::CLocationEditPolicy::EMergePolicy x_GetMergePolicy();
    void x_SetPartial5Policy(objects::edit::CLocationEditPolicy::EPartialPolicy partial5);
    void x_SetPartial3Policy(objects::edit::CLocationEditPolicy::EPartialPolicy partial3);
    void x_SetMergePolicy(objects::edit::CLocationEditPolicy::EMergePolicy merge);
    void x_EnableExtend5Btn();
    void x_EnableExtend3Btn();
};

END_NCBI_SCOPE

#endif
    // _EDIT_FEAT_LOC_PANEL_H_
