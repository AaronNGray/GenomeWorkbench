#ifndef PKG_ALIGNMENT___BLAST_SEARCH_OPTIONS_PANEL__HPP
#define PKG_ALIGNMENT___BLAST_SEARCH_OPTIONS_PANEL__HPP

/*  $Id: blast_search_options_panel.hpp 36080 2016-08-04 19:41:09Z katargir $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>

#include <gui/core/algo_tool_manager_base.hpp>
#include <gui/objutils/objects.hpp>

#include <algo/blast/api/blast_types.hpp>

#include "wx/panel.h"
#include <wx/timer.h>

////@begin includes
#include "wx/listctrl.h"
////@end includes

class wxBoxSizer;
class wxComboBox;
class wxRadioButton;
class wxStaticText;
class wxButton;
class wxStaticBox;
class wxStaticBoxSizer;
class wxRadioBox;
class wxFlexGridSizer;
class wxBitmapButton;
class wxChoice;

////@begin control identifiers
#define SYMBOL_CBLASTSEARCHOPTIONSPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CBLASTSEARCHOPTIONSPANEL_TITLE _("BLAST Search Options Panel")
#define SYMBOL_CBLASTSEARCHOPTIONSPANEL_IDNAME ID_CBLASTSEARCHOPTIONSPANEL
#define SYMBOL_CBLASTSEARCHOPTIONSPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CBLASTSEARCHOPTIONSPANEL_POSITION wxDefaultPosition
////@end control identifiers

#include <wx/panel.h>


BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_ALIGNMENT
 *
 * @{
 */

class CObjectListWidget;
class CBLASTParams;
class CNetBLASTUIDataSource;


class CBLASTSearchOptionsPanel: public CAlgoToolManagerParamsPanel
{
    DECLARE_DYNAMIC_CLASS( CBLASTSearchOptionsPanel )
    DECLARE_EVENT_TABLE()

public:
    CBLASTSearchOptionsPanel();
    CBLASTSearchOptionsPanel( wxWindow* parent, wxWindowID id = SYMBOL_CBLASTSEARCHOPTIONSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CBLASTSEARCHOPTIONSPANEL_POSITION, const wxSize& size = wxSize(), long style = SYMBOL_CBLASTSEARCHOPTIONSPANEL_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CBLASTSEARCHOPTIONSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CBLASTSEARCHOPTIONSPANEL_POSITION, const wxSize& size = wxSize(), long style = SYMBOL_CBLASTSEARCHOPTIONSPANEL_STYLE );

    ~CBLASTSearchOptionsPanel();

    void Init();

    void CreateControls();

    bool TransferDataToWindow();

    bool TransferDataFromWindow();

////@begin CBLASTSearchOptionsPanel event handler declarations

    void OnNucRadioSelected( wxCommandEvent& event );

    void OnProtRadioSelected( wxCommandEvent& event );

    void OnQuerySequenceSetSelected( wxCommandEvent& event );

    void OnProgramComboSelected( wxCommandEvent& event );

    void OnSubjectSequencesSelected( wxCommandEvent& event );

    void OnSubjectNCBIDBSelected( wxCommandEvent& event );

    void OnSubjectLocalDBSelected( wxCommandEvent& event );

    void OnSubjectSequenceSetSelected( wxCommandEvent& event );

    void OnDatabaseComboSelected( wxCommandEvent& event );

    void OnBrowseDatabases( wxCommandEvent& event );

    void OnSelectLocalDB( wxCommandEvent& event );

    void OnLocalDBLoaderClick( wxCommandEvent& event );

////@end CBLASTSearchOptionsPanel event handler declarations

////@begin CBLASTSearchOptionsPanel member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CBLASTSearchOptionsPanel member function declarations

    static bool ShowToolTips();

////@begin CBLASTSearchOptionsPanel member variables
    wxRadioButton* m_NucRadio;
    wxRadioButton* m_ProtRadio;
    wxStaticText* m_QuerySeqSetStatic;
    wxChoice* m_QuerySeqSetCombo;
    CObjectListWidget* m_QueryListCtrl;
    wxComboBox* m_ProgramCombo;
    wxStaticBoxSizer* m_LocalSizer;
    wxRadioButton* m_SubjectTypeSeqsCtrl;
    wxRadioButton* m_SubjectTypeNCBICtrl;
    wxRadioButton* m_SubjectTypeLDBCtrl;
    wxStaticText* m_SubjectSeqSetStatic;
    wxChoice* m_SubjectSeqSetCombo;
    wxBoxSizer* m_LocalSeqsSizer;
    CObjectListWidget* m_SubjectListCtrl;
    wxFlexGridSizer* m_NCBIDBSizer;
    wxBoxSizer* m_DBSizer;
    wxComboBox* m_DatabaseCombo;
    wxBitmapButton* m_BrowseDBBtn;
    wxComboBox* m_QueryCombo;
    wxBoxSizer* m_LocalDBSizer;
    wxTextCtrl* m_LocalDBCtrl;
    wxCheckBox* m_LocalDBLoaderCtrl;
    enum {
        ID_CBLASTSEARCHOPTIONSPANEL = 10000,
        ID_NUC_RADIO = 10001,
        ID_PROT_RADIO = 10002,
        ID_QUERY_SEQ_SET = 10042,
        ID_LIST = 10003,
        ID_PROGRAM_COMBO = 10004,
        ID_RADIOBUTTON = 10040,
        ID_RADIOBUTTON2 = 10010,
        ID_RADIOBUTTON1 = 10041,
        ID_SUBJECT_SEQ_SET = 10043,
        ID_SUBJ_LIST = 10012,
        ID_DB_COMBO = 10005,
        ID_SELECT_NCBI_DB = 10006,
        ID_ENTREZ_COMBO = 10007,
        ID_TEXTCTRL = 10011,
        ID_SELECT_LOCAL_DB_BTN = 10026,
        ID_CHECKBOX = 10039
    };
////@end CBLASTSearchOptionsPanel member variables

    enum {
        ID_DBS_LOADING_TEXT = 10030,
        ID_DBS_LOADING_PROGRESS = 10031,
        ID_DBS_FAILED = 10032,
        ID_DBS_TRY_AGAIN = 10033
    };

public:
    /// @name CAlgoToolManagerParamsPanel implementation
    virtual void    LoadSettings();
    virtual void    SaveSettings() const;
    void RestoreDefaults();
    /// @}

    void    Setup(CNetBLASTUIDataSource& ds, const string& tool_name);
    void    SetParams(CBLASTParams* params,
                      map<string, TConstScopedObjects>* nuc_objects,
                      map<string, TConstScopedObjects>* prot_objects);
    void    SelectInputType();

    bool    CanDoNext();

protected:
    void x_OnInputTypeChanged(bool nuc_input);
    void x_OnSubjInputTypeChanged();

    void x_UpdateDBCombo();
    void x_CheckDBLoaded();
    void x_UpdateSubjects();

    void OnTryAgain( wxCommandEvent& event );
    void OnTimer(wxTimerEvent& event);

    void x_UpdateSubjectType();

    void x_UpdateQuerySeqSet();
    void x_FillQueryList();

    void x_UpdateSubjectSeqSet();
    void x_FillSubjectList();

protected:
    CNetBLASTUIDataSource*  m_DataSource;
    string  m_ToolName;

    map<string, TConstScopedObjects>*  m_NucObjects;
    map<string, TConstScopedObjects>*  m_ProtObjects;

    CBLASTParams* m_Params;

    wxTimer m_Timer;

    int m_DBLoadState;

    string m_CurrentQuerySet;
    string m_CurrentSubjectSet;
};

/* @} */

END_NCBI_SCOPE

#endif
    // _BLAST_SEARCH_OPTIONS_PANEL_H_
