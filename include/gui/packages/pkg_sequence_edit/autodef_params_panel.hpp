/*  $Id: autodef_params_panel.hpp 34575 2016-01-27 13:20:56Z bollin $
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
#ifndef _AUTODEF_PARAMS_PANEL_H_
#define _AUTODEF_PARAMS_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <corelib/ncbistl.hpp>
#include <corelib/ncbiobj.hpp>

#include <gui/core/algo_tool_manager_base.hpp>

#include <gui/objutils/objects.hpp>

#include <gui/objutils/reg_settings.hpp>

#include <gui/packages/pkg_sequence_edit/autodef_params.hpp>
#include <wx/dialog.h>
#include <wx/choice.h>
#include <wx/listctrl.h>
#include <wx/radiobox.h>

/*!
 * Includes
 */

////@begin includes
#include "wx/listctrl.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxListCtrl;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CAUTODEFPARAMSPANEL 10025
#define ID_LISTCTRL 10000
#define ID_MODIFIER_LIST 10027
#define ID_MODIFIERS_PER_LINE 10028
#define ID_USE_LABELS 10029
#define ID_LEAVE_PARENTHETICAL 10030
#define ID_DO_NOT_APPLY_TO_SP 10031
#define ID_INCLUDE_COUNTRY_TEXT 10032
#define ID_DO_NOT_APPLY_TO_NR 10033
#define ID_DO_NOT_APPLY_TO_CF 10034
#define ID_DO_NOT_APPLY_TO_AFF 10035
#define ID_KEEP_AFTER_SEMICOLON 10036
#define ID_LIST_FEATURES 10026
#define ID_SUPPRESS_ALTERNATIVE_SPLICE 10042
#define ID_SUPPRESS_TRANSPOSON 10043
#define ID_SUPPRESS_GENE_CLUSTER 10044
#define ID_SUPPRESS_LOCUS_TAGS 10045
#define ID_INCLUDE_EXONS 10046
#define ID_INCLUDE_5UTRS 10049
#define ID_INCLUDE_INTRONS 10047
#define ID_INCLUDE_3UTRS 10050
#define ID_INCLUDE_REGULATORY_FEATURES 10048
#define ID_USE_FAKE_PROMOTERS 10058
#define ID_INCLUDE_MISC_FEATS 10052
#define ID_MISC_FEAT_RULE 10053
#define ID_SUPPRESS_FEATURES 10054
#define ID_PRODUCT_NAME 10055
#define ID_ADD_ALT_SPLICE 10056
#define ID_MODIFIER_CHECKBOX 10057
#define ID_USE_NCRNA_COMMENT 10059
#define SYMBOL_CAUTODEFPARAMSPANEL_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CAUTODEFPARAMSPANEL_TITLE _("Autodef Params Panel")
#define SYMBOL_CAUTODEFPARAMSPANEL_IDNAME ID_CAUTODEFPARAMSPANEL
#define SYMBOL_CAUTODEFPARAMSPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CAUTODEFPARAMSPANEL_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE

class CObjectListWidget;

/*!
 * CAutodefParamsPanel class declaration
 */

class CAutodefParamsPanel:public CAlgoToolManagerParamsPanel
{
    DECLARE_DYNAMIC_CLASS( CAutodefParamsPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAutodefParamsPanel();
    CAutodefParamsPanel( wxWindow* parent, wxWindowID id = SYMBOL_CAUTODEFPARAMSPANEL_IDNAME, const wxString& caption = SYMBOL_CAUTODEFPARAMSPANEL_TITLE, const wxPoint& pos = SYMBOL_CAUTODEFPARAMSPANEL_POSITION, const wxSize& size = SYMBOL_CAUTODEFPARAMSPANEL_SIZE, long style = SYMBOL_CAUTODEFPARAMSPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CAUTODEFPARAMSPANEL_IDNAME, const wxString& caption = SYMBOL_CAUTODEFPARAMSPANEL_TITLE, const wxPoint& pos = SYMBOL_CAUTODEFPARAMSPANEL_POSITION, const wxSize& size = SYMBOL_CAUTODEFPARAMSPANEL_SIZE, long style = SYMBOL_CAUTODEFPARAMSPANEL_STYLE );

    /// Destructor
    ~CAutodefParamsPanel();

    /// Initializes member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    bool TransferDataToWindow();

    bool TransferDataFromWindow();

    /// @name CAlgoToolManagerParamsPanel implementation
    void RestoreDefaults();
    /// @}

////@begin CAutodefParamsPanel event handler declarations
    void    OnChangeMiscFeat(wxCommandEvent& event);
    void    OnSuppressFeatures(wxCommandEvent& event);
    void    OnChangeModifierListChoice(wxCommandEvent& event);
    void    OnChangeModifierSelection(wxCommandEvent& event);
////@end CAutodefParamsPanel event handler declarations

////@begin CAutodefParamsPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAutodefParamsPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    void    SetParams(SAutodefParams* params, TConstScopedObjects* objects);

    virtual void    SetRegistryPath(const string& path);
    virtual void    LoadSettings();
    virtual void    SaveSettings() const;

////@begin CAutodefParamsPanel member variables
    CObjectListWidget* m_ObjectListCtrl;
    wxChoice* m_ModifierListChoice;
    wxChoice* m_MaxModifiers;
    vector<wxCheckBox *> m_ModifierButtons;
    wxRadioBox* m_HIVRule;
    wxCheckBox* m_UseLabels;
    wxCheckBox* m_LeaveParen;
    wxCheckBox* m_NoSp;
    wxCheckBox* m_NoNr;
    wxCheckBox* m_NoCf;
    wxCheckBox* m_NoAff;
    wxCheckBox* m_IncludeCountry;
    wxCheckBox* m_KeepAfterSemicolon;
    wxChoice* m_FeatureListChoice;
    wxCheckBox* m_SuppressAltSplice;
    wxCheckBox* m_SuppressTransposon;
    wxCheckBox* m_SupressGeneCluster;
    wxCheckBox* m_SuppressLocusTags;
    wxCheckBox* m_KeepExons;
    wxCheckBox* m_Keep5UTRs;
    wxCheckBox* m_KeepIntrons;
    wxCheckBox* m_Keep3UTRs;
    wxCheckBox* m_UseFakePromoters;
    wxCheckBox* m_KeepRegulatoryFeatures;
    wxCheckBox* m_KeepMiscFeats;
    wxChoice* m_MiscFeatParseRule;
    wxChoice* m_ProductClauseChoice;
    wxCheckBox* m_AppendAltSplice;
    wxCheckBox* m_UseNcRNAComment;
////@end CAutodefParamsPanel member variables

protected:
    SAutodefParams* m_Params;
    set<objects::CFeatListItem> m_SuppressedFeatures;

    TConstScopedObjects*    m_InputObjects;
    bool    m_InputListDirty;

    bool x_AllowHIVRule();


};

END_NCBI_SCOPE

#endif
    // _AUTODEF_PARAMS_PANEL_H_
