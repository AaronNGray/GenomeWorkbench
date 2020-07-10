#ifndef GUI_WIDGETS_ALN_MULTIPLE___WX_BUILD_OPTIONS_DLG__HPP
#define GUI_WIDGETS_ALN_MULTIPLE___WX_BUILD_OPTIONS_DLG__HPP
/*  $Id: wx_build_options_dlg.hpp 25493 2012-03-27 18:28:58Z kuznets $
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

#include <gui/widgets/wx/dialog.hpp>

#include <gui/widgets/aln_multiple/alnmulti_ds_builder.hpp>

////@begin includes
#include "wx/valgen.h"
////@end includes

////@begin forward declarations
////@end forward declarations

class wxListBox;
class wxTextCtrl;

////@begin control identifiers
#define SYMBOL_CBUILDOPTIONSDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CBUILDOPTIONSDLG_TITLE wxT("Build Alignment")
#define SYMBOL_CBUILDOPTIONSDLG_IDNAME ID_CWXBUILDOPTIONSDLG
#define SYMBOL_CBUILDOPTIONSDLG_SIZE wxSize(400, 300)
#define SYMBOL_CBUILDOPTIONSDLG_POSITION wxDefaultPosition
////@end control identifiers


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CwxBuildOptionsDlg
class NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT CBuildOptionsDlg: public CDialog
{
    DECLARE_DYNAMIC_CLASS( CBuildOptionsDlg )
    DECLARE_EVENT_TABLE()

public:
    CBuildOptionsDlg();
    CBuildOptionsDlg( wxWindow* parent, wxWindowID id = SYMBOL_CBUILDOPTIONSDLG_IDNAME, const wxString& caption = SYMBOL_CBUILDOPTIONSDLG_TITLE, const wxPoint& pos = SYMBOL_CBUILDOPTIONSDLG_POSITION, const wxSize& size = SYMBOL_CBUILDOPTIONSDLG_SIZE, long style = SYMBOL_CBUILDOPTIONSDLG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CBUILDOPTIONSDLG_IDNAME, const wxString& caption = SYMBOL_CBUILDOPTIONSDLG_TITLE, const wxPoint& pos = SYMBOL_CBUILDOPTIONSDLG_POSITION, const wxSize& size = SYMBOL_CBUILDOPTIONSDLG_SIZE, long style = SYMBOL_CBUILDOPTIONSDLG_STYLE );

    ~CBuildOptionsDlg();

    void Init();

    void CreateControls();

////@begin CBuildOptionsDlg event handler declarations

    void OnMultipleRadioSelected( wxCommandEvent& event );

    void OnQueryRadioSelected( wxCommandEvent& event );

    void OnOriginalRadioSelected( wxCommandEvent& event );

    void OnClipRadioSelected( wxCommandEvent& event );

    void OnExtendRadioSelected( wxCommandEvent& event );

    void OnShowFlankRadioSelected( wxCommandEvent& event );

    void OnShowUaRadioSelected( wxCommandEvent& event );

////@end CBuildOptionsDlg event handler declarations

////@begin CBuildOptionsDlg member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CBuildOptionsDlg member function declarations

    static bool ShowToolTips();

public:
    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    void SetParams(const CAlnMultiDSBuilder::TOptions& options, const vector<CBioseq_Handle>& handles, objects::CScope& scope);
    void GetOptions(CAlnMultiDSBuilder::TOptions& options);

protected:
////@begin CBuildOptionsDlg member variables
    wxListBox* m_AnchorIDList;
    wxTextCtrl* m_StartText;
    wxTextCtrl* m_EndText;
    wxTextCtrl* m_ExtentText;
    wxTextCtrl* m_FlankText;
protected:
    bool m_Multiple;
    bool m_CombineRows;
    bool m_SeparateStrands;
    bool m_Clip;
    bool m_Extend;
    wxArrayInt m_IDIndexes;
    bool m_Original;
    enum {
        ID_CWXBUILDOPTIONSDLG = 10002,
        ID_MULTIPLE_RADIO = 10004,
        ID_QUERY_RADIO = 10003,
        ID_ANCHOR_LIST = 10005,
        ID_ORIGINAL_RADIO = 10006,
        ID_CLIP_RADIO = 10007,
        ID_START_TEXT = 10017,
        ID_END_TEXT = 10009,
        ID_EXTEND_RADIO = 10008,
        ID_TEXTCTRL = 10010,
        ID_HIDE_UA_RADIO = 10018,
        ID_SHOW_FLANK_RADIO = 10020,
        ID_EXTENT_TEXT = 10021,
        ID_SHOW_UA_RADIO = 10018,
        ID_COMBINE_CHECK = 10012,
        ID_SEPARATE_STRANDS = 10011
    };
////@end CBuildOptionsDlg member variables

    vector<CBioseq_Handle> m_Handles;
    CRef<CScope> m_Scope;
    CAlnMultiDSBuilder::TOptions m_Options;
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS_ALN_MULTIPLE___WX_BUILD_OPTIONS_DLG__HPP
