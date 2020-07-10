/*  $Id: rawseqtodeltabyn_panel.hpp 43364 2019-06-20 14:46:17Z asztalos $
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
 * Authors:  Igor Filippov
 */

#ifndef _RAWSEQTODELTABYN_PANEL_H_
#define _RAWSEQTODELTABYN_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>
#include <wx/panel.h>

class wxTextCtrl;
class wxStaticText;
class wxChoice;
class wxCheckBox;
class wxRadioButton;

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CRAWSEQTODELTABYNPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CRAWSEQTODELTABYNPANEL_TITLE _("Add Assembly Gaps To Sequence")
#define SYMBOL_CRAWSEQTODELTABYNPANEL_IDNAME ID_CRAWSEQTODELTABYNPANEL
#define SYMBOL_CRAWSEQTODELTABYNPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CRAWSEQTODELTABYNPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CRawSeqToDeltaByNPanel class declaration
 */

class NCBI_GUIWIDGETS_EDIT_EXPORT CRawSeqToDeltaByNPanel : public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CRawSeqToDeltaByNPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CRawSeqToDeltaByNPanel();
    CRawSeqToDeltaByNPanel( wxWindow* parent, 
                            bool adjust_cds = true, bool add_linkage = false, bool keep_gap_length = false,
                            wxWindowID id = SYMBOL_CRAWSEQTODELTABYNPANEL_IDNAME,
                            const wxPoint& pos = SYMBOL_CRAWSEQTODELTABYNPANEL_POSITION, 
                            const wxSize& size = SYMBOL_CRAWSEQTODELTABYNPANEL_SIZE, 
                            long style = SYMBOL_CRAWSEQTODELTABYNPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
                 wxWindowID id = SYMBOL_CRAWSEQTODELTABYNPANEL_IDNAME, 
                 const wxPoint& pos = SYMBOL_CRAWSEQTODELTABYNPANEL_POSITION, 
                 const wxSize& size = SYMBOL_CRAWSEQTODELTABYNPANEL_SIZE, 
                 long style = SYMBOL_CRAWSEQTODELTABYNPANEL_STYLE );

    /// Destructor
    ~CRawSeqToDeltaByNPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CRawSeqToDeltaByNPanel event handler declarations

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RAW_SEQ_TO_DELTA_BY_N_RB1
    void GetExplanation( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_RAW_SEQ_TO_DELTA_BY_N_ADD_LINKAGE
    void OnAddLinkage( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_RAW_SEQ_TO_DELTA_BY_N_GAP_TYPE
    void OnGapType( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_RAW_SEQ_TO_DELTA_BY_N_LINKAGE
    void OnLinkage( wxCommandEvent& event );

////@end CRawSeqToDeltaByNPanel event handler declarations

////@begin CRawSeqToDeltaByNPanel member function declarations
    void GetMinMax(long &min_known, int &max_known, long &min_unknown, int &max_unknown);

    bool GetAdjustCDS() const;
    bool GetKeepGapLength() const;

    void GetAssemblyParams(bool& is_assembly_gap, int& gap_type, int& linkage, int& linkage_evidence);
    void GetAssemblyParams(bool& is_assembly_gap, string& gap_type, string& linkage, string& linkage_evidence);

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CRawSeqToDeltaByNPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

private:
    void x_SetDescription();
    void x_OnAddLinkage();

////@begin CRawSeqToDeltaByNPanel member variables
    wxRadioButton* m_RadioButton_unknown;
    wxTextCtrl* m_TextCtrl_unknown;
    wxRadioButton* m_RadioButton_known;
    wxTextCtrl* m_TextCtrl_known;
    wxStaticText* m_Description;
    wxCheckBox* m_AdjustCDS;
    wxCheckBox* m_AddLinkage;
    wxChoice* m_GapType;
    wxChoice* m_Linkage;
    wxChoice* m_LinkageEvidence;
    wxCheckBox* m_KeepGapLength;
    /// Control identifiers
    enum {
        ID_CRAWSEQTODELTABYNPANEL = 6600,
        ID_RAW_SEQ_TO_DELTA_BY_N_RB1 = 6601,
        ID_RAW_SEQ_TO_DELTA_BY_N_RB2 = 6602,
        ID_RAW_SEQ_TO_DELTA_BY_N_TC1 = 6603,
        ID_RAW_SEQ_TO_DELTA_BY_N_RB3 = 6604,
        ID_RAW_SEQ_TO_DELTA_BY_N_RB4 = 6605,
        ID_RAW_SEQ_TO_DELTA_BY_N_TC2 = 6606,
        ID_CHECKBOX = 6607,
        ID_RAW_SEQ_TO_DELTA_BY_N_ADD_LINKAGE = 6608,
        ID_RAW_SEQ_TO_DELTA_BY_N_GAP_TYPE = 6609,
        ID_RAW_SEQ_TO_DELTA_BY_N_LINKAGE = 6610,
        ID_RAW_SEQ_TO_DELTA_BY_N_LINKEVIDENCE = 6611
    };

    bool m_AdjustCDSValue{ true };
    bool m_AddLinkageValue{ false };
    bool m_KeepGapLengthValue{ false };
////@end CRawSeqToDeltaByNPanel member variables
};

END_NCBI_SCOPE

#endif
    // _RAWSEQTODELTABYN_PANEL_H_
