/*  $Id: autodef_opt_feat_panel.hpp 36069 2016-08-03 17:23:21Z bollin $
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
// Generated by DialogBlocks (unregistered), 21/06/2013 14:54:07

#ifndef _AUTODEF_OPT_FEAT_PANEL_H_
#define _AUTODEF_OPT_FEAT_PANEL_H_

#include <corelib/ncbistd.hpp>

#include <gui/packages/pkg_sequence_edit/autodef_params.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/panel.h>
#include <wx/checkbox.h>
#include <wx/radiobut.h>
#include <wx/choice.h>
#include <wx/statbox.h>
#include <wx/icon.h>

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
#define SYMBOL_CAUTODEFOPTFEATPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CAUTODEFOPTFEATPANEL_TITLE _("AutodefOptFeatPanel")
#define SYMBOL_CAUTODEFOPTFEATPANEL_IDNAME ID_CAUTODEFOPTFEATPANEL
#define SYMBOL_CAUTODEFOPTFEATPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CAUTODEFOPTFEATPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CAutodefOptFeatPanel class declaration
 */

class CAutodefOptFeatPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CAutodefOptFeatPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAutodefOptFeatPanel();
    CAutodefOptFeatPanel( wxWindow* parent, wxWindowID id = SYMBOL_CAUTODEFOPTFEATPANEL_IDNAME, const wxPoint& pos = SYMBOL_CAUTODEFOPTFEATPANEL_POSITION, const wxSize& size = SYMBOL_CAUTODEFOPTFEATPANEL_SIZE, long style = SYMBOL_CAUTODEFOPTFEATPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CAUTODEFOPTFEATPANEL_IDNAME, const wxPoint& pos = SYMBOL_CAUTODEFOPTFEATPANEL_POSITION, const wxSize& size = SYMBOL_CAUTODEFOPTFEATPANEL_SIZE, long style = SYMBOL_CAUTODEFOPTFEATPANEL_STYLE );

    /// Destructor
    ~CAutodefOptFeatPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAutodefOptFeatPanel event handler declarations

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_KEEP_MISC_FEATS
    void OnKeepMiscFeatsClick( wxCommandEvent& event );

////@end CAutodefOptFeatPanel event handler declarations

////@begin CAutodefOptFeatPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAutodefOptFeatPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CAutodefOptFeatPanel member variables
    wxCheckBox* m_KeepExons;
    wxCheckBox* m_KeepIntrons;
    wxCheckBox* m_KeepMiscRecomb;
    wxCheckBox* m_KeepRepeatRegion;
    wxCheckBox* m_Keep5UTRs;
    wxCheckBox* m_Keep3UTRs;
    wxCheckBox* m_KeepMiscFeats;
    wxChoice* m_MiscFeatParseRule;
    wxCheckBox* m_RegulatoryFeatures;
    wxCheckBox* m_UseFakePromoters;
    /// Control identifiers
    enum {
        ID_CAUTODEFOPTFEATPANEL = 10214,
        ID_AD_FEAT_EXONS = 10208,
        ID_AD_FEAT_INTRONS = 10209,
        ID_AD_FEAT_PRERNA = 10221,
        ID_AD_FEAT_NCRNA = 10222,
        ID_CHECKBOX2 = 10060,
        ID_AD_FEAT_UORF = 10212,
        ID_AD_FEAT_MOBILE_ELEMENT = 10220,
        ID_AD_FEAT_REPEAT_REGION = 10223,
        ID_AD_FEAT_5UTR = 10210,
        ID_AD_FEAT_3UTR = 10211,
        ID_KEEP_MISC_FEATS = 10218,
        ID_AD_FEAT_MISC_FEAT_PARSE_RULE = 10219,
        ID_REGULATORY_FEATURES = 10297,
        ID_AD_USE_FAKE_PROMOTERS = 10213
    };
////@end CAutodefOptFeatPanel member variables
    void SetParams(SAutodefParams* params);
    void PopulateParams(SAutodefParams& params);

    void Clear();
};

END_NCBI_SCOPE

#endif
    // _AUTODEF_OPT_FEAT_PANEL_H_
