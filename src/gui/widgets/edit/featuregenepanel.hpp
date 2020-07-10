/*  $Id: featuregenepanel.hpp 42681 2019-04-02 15:42:12Z filippov $
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
#ifndef _FEATUREGENEPANEL_H_
#define _FEATUREGENEPANEL_H_

#include <corelib/ncbistd.hpp>
#include <serial/serialbase.hpp>
#include <objmgr/scope.hpp>
#include <objects/seqfeat/Seq_feat.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes
#include <wx/panel.h>
#include <wx/choice.h>
#include <wx/listbox.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/radiobox.h>
#include <wx/button.h>
#include "gene_panel.hpp"

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
#define SYMBOL_CFEATUREGENEPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CFEATUREGENEPANEL_TITLE _("FeatureGenePanel")
#define SYMBOL_CFEATUREGENEPANEL_IDNAME ID_CFEATUREGENEPANEL
#define SYMBOL_CFEATUREGENEPANEL_SIZE wxDefaultSize
#define SYMBOL_CFEATUREGENEPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CFeatureGenePanel class declaration
 */

class CFeatureGenePanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CFeatureGenePanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CFeatureGenePanel();
    CFeatureGenePanel( wxWindow* parent, CSerialObject& object, objects::CScope& scope, wxWindowID id = SYMBOL_CFEATUREGENEPANEL_IDNAME, const wxPoint& pos = SYMBOL_CFEATUREGENEPANEL_POSITION, const wxSize& size = SYMBOL_CFEATUREGENEPANEL_SIZE, long style = SYMBOL_CFEATUREGENEPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CFEATUREGENEPANEL_IDNAME, const wxPoint& pos = SYMBOL_CFEATUREGENEPANEL_POSITION, const wxSize& size = SYMBOL_CFEATUREGENEPANEL_SIZE, long style = SYMBOL_CFEATUREGENEPANEL_STYLE );

    /// Destructor
    ~CFeatureGenePanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataFromWindow();
    CRef<objects::CSeq_loc> GetLocationForGene();
    CRef<objects::CSeq_feat> GetNewGene();
    CConstRef<objects::CSeq_feat> GetGeneToEdit();
    CRef<objects::CSeq_feat> GetUpdatedGene();
    void SetModifyPseudo(bool modify_pseudo, const string &pseudo_choice);

////@begin CFeatureGenePanel event handler declarations

    /// wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_CHOICE16
    void OnSelectGene( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX13
    void OnClickSuppress( wxCommandEvent& event );

    void OnEditGene(wxCommandEvent& event);

////@end CFeatureGenePanel event handler declarations

////@begin CFeatureGenePanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CFeatureGenePanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CFeatureGenePanel member variables
    wxListBox* m_GeneChoice;
    wxRadioBox* m_MapType;
    wxCheckBox* m_Suppress;
    /// Control identifiers
    enum {
        ID_CFEATUREGENEPANEL = 10007,
        ID_CHOICE16 = 10130,
        ID_RADIOBOX4 = 10131,
        ID_CHECKBOX13 = 10132,
        ID_BUTTON_EDIT_GENE = 10133
    };
////@end CFeatureGenePanel member variables
    CGenePanel* m_GeneRefPanel;
private:
    void AddReciprocicalXref(CRef<objects::CSeq_feat> gene_feat);

    CSerialObject* m_Object;
    CRef<objects::CScope> m_Scope;
    CRef<objects::CGene_ref> m_GeneRef;
    vector<CConstRef<objects::CGene_ref> > m_GeneList;
    vector<CConstRef<objects::CSeq_feat> > m_GeneFeatList;
    CConstRef<objects::CSeq_feat> m_OverlappingGene;
    wxButton *m_ButtonEditGene;
    bool m_modify_pseudo;
    string m_pseudo_choice;
};

END_NCBI_SCOPE

#endif
    // _FEATUREGENEPANEL_H_
