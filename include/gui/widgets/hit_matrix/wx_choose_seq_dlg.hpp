#ifndef GUI_WIDGETS_HIT_MATRIX___WX_CHOOSE_SEQ_DLG__H
#define GUI_WIDGETS_HIT_MATRIX___WX_CHOOSE_SEQ_DLG__H

/*  $Id: wx_choose_seq_dlg.hpp 25478 2012-03-27 14:54:26Z kuznets $
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

#include <gui/widgets/hit_matrix/align_seq_table.hpp>

#include <objects/seqloc/Seq_id.hpp>
#include <objmgr/scope.hpp>

#include <wx/dialog.h>

////@begin includes
#include "wx/valgen.h"
#include "wx/statline.h"
#include "wx/listctrl.h"
////@end includes

////@begin forward declarations
class CwxTableListCtrl;
////@end forward declarations

class wxCheckBox;
class wxStaticBox;
class wxRadioButton;

////@begin control identifiers
#define ID_CWXCHOOSESEQDLG 10013
#define ID_SEP_CHECK 10006
#define ID_POS_NEG_RADIO 10003
#define ID_POS_RADIO 10004
#define ID_NEG_RADIO 10005
#define ID_SUBJECT_TABLE 10001
#define ID_QUERY_TABLE 10002
#define SYMBOL_CWXCHOOSESEQDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CWXCHOOSESEQDLG_TITLE wxT("Choose Sequences")
#define SYMBOL_CWXCHOOSESEQDLG_IDNAME ID_CWXCHOOSESEQDLG
#define SYMBOL_CWXCHOOSESEQDLG_SIZE wxDefaultSize
#define SYMBOL_CWXCHOOSESEQDLG_POSITION wxDefaultPosition
////@end control identifiers


BEGIN_NCBI_SCOPE


class CwxChooseSeqDlg: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CwxChooseSeqDlg )
    DECLARE_EVENT_TABLE()

public:
    typedef IHitMatrixDataSource::TIdVector    TIdVector;

    CwxChooseSeqDlg();
    CwxChooseSeqDlg( wxWindow* parent, wxWindowID id = SYMBOL_CWXCHOOSESEQDLG_IDNAME, const wxString& caption = SYMBOL_CWXCHOOSESEQDLG_TITLE, const wxPoint& pos = SYMBOL_CWXCHOOSESEQDLG_POSITION, const wxSize& size = SYMBOL_CWXCHOOSESEQDLG_SIZE, long style = SYMBOL_CWXCHOOSESEQDLG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CWXCHOOSESEQDLG_IDNAME, const wxString& caption = SYMBOL_CWXCHOOSESEQDLG_TITLE, const wxPoint& pos = SYMBOL_CWXCHOOSESEQDLG_POSITION, const wxSize& size = SYMBOL_CWXCHOOSESEQDLG_SIZE, long style = SYMBOL_CWXCHOOSESEQDLG_STYLE );

    ~CwxChooseSeqDlg();

    void Init();
    void CreateControls();

    void    SetSeqs(CIRef<IHitMatrixDataSource> ds);
    const IHitSeqId*    GetSubjectId();
    const IHitSeqId*    GetQueryId();
    void GetParams(IHitMatrixDataSource::SParams& params) const;

    bool TransferDataToWindow();

    bool TransferDataFromWindow();

////@begin CwxChooseSeqDlg event handler declarations

    void OnSepCheckClick( wxCommandEvent& event );

    void OnPosNegRadioSelected( wxCommandEvent& event );

    void OnPosRadioSelected( wxCommandEvent& event );

    void OnNegRadioSelected( wxCommandEvent& event );

    void OnSubjectSelected( wxListEvent& event );

    void OnSubjectTableDeselected( wxListEvent& event );

    void OnOkClick( wxCommandEvent& event );

    void OnCancelClick( wxCommandEvent& event );

////@end CwxChooseSeqDlg event handler declarations

////@begin CwxChooseSeqDlg member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CwxChooseSeqDlg member function declarations

    static bool ShowToolTips();

protected:
    void    x_InitTable(bool subject, IHitMatrixDataSource& ds, const TIdVector& ids, int sel_index);
    void    x_SetSelectedIds(const IHitSeqId* s_id, const IHitSeqId* q_id);
    void    x_ReloadIDLists();
    void    x_SaveSettings();
    void    x_LoadSettings();
    int     x_GetIdIndex(const IHitSeqId* id, const IHitMatrixDataSource::TIdVector& ids);
    bool    x_Validate();
    void    x_OnSelectionChanged();
    const IHitSeqId* x_GetSelectedId(bool subject) const;

protected:
////@begin CwxChooseSeqDlg member variables
    wxCheckBox* m_SepCheck;
    wxStaticBox* m_ShowRadios;
    wxRadioButton* m_OrientRadios;
    CwxTableListCtrl* m_SubjectTable;
    CwxTableListCtrl* m_QueryTable;
protected:
    wxArrayInt m_SubjectSelection;
    wxArrayInt m_QuerySelection;
    bool m_PosAndNeg;
    bool m_Pos;
    bool m_Neg;
////@end CwxChooseSeqDlg member variables

    CIRef<IHitMatrixDataSource> m_DataSource;
    CAlignSeqTableModel m_SubjectModel;
    CAlignSeqTableModel m_QueryModel;
    IHitMatrixDataSource::TIdVector m_AllIds;
    int m_Orient;
    bool m_SeparateAligns;
    auto_ptr<IHitSeqId> m_SubjectId;
    auto_ptr<IHitSeqId> m_QueryId;
    auto_ptr<IHitSeqId> m_SepQueryId;
    auto_ptr<IHitSeqId> m_SepSubjectId;
};

#endif
    // GUI_WIDGETS_HIT_MATRIX___WX_CHOOSE_SEQ_DLG__H

END_NCBI_SCOPE
