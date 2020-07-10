/*  $Id: wx_phylo_edit_dlg.hpp 30858 2014-07-31 14:05:43Z ucko $
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
 * Authors:  Vladimir Tereshkov
 *
 * File Description:
 *
 */
#ifndef _WX_PHYLO_EDIT_DLG_H_
#define _WX_PHYLO_EDIT_DLG_H_


/*!
 * Includes
 */

#include <corelib/ncbistl.hpp>
#include <gui/utils/rgba_color.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_scheme.hpp>
#include <gui/widgets/phylo_tree/phylo_tree.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_ds.hpp>

// This header must (at least indirectly) precede any wxWidgets headers.
#include <gui/widgets/wx/fixed_base.hpp>

#include <wx/dialog.h>

////@begin includes
#include "wx/listctrl.h"
#include "wx/clrpicker.h"
#include "wx/statline.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxListCtrl;
class wxColourPickerCtrl;
////@end forward declarations

/*!
 * Control identifiers
 */
BEGIN_NCBI_SCOPE

////@begin control identifiers
#define ID_CWXPHYLOEDITDLG 10000
#define ID_LISTCTRL1 10002
#define ID_BUTTON4 10003
#define ID_BUTTON5 10010
#define ID_BUTTON 10001
#define ID_TEXTCTRL14 10011
#define ID_TEXTCTRL15 10012
#define ID_COLOURCTRL 10042
#define ID_STATICLINE1 10004
#define SYMBOL_CWXPHYLOEDITDLG_STYLE wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CWXPHYLOEDITDLG_TITLE _("Node Properties")
#define SYMBOL_CWXPHYLOEDITDLG_IDNAME ID_CWXPHYLOEDITDLG
#define SYMBOL_CWXPHYLOEDITDLG_SIZE wxSize(400, 300)
#define SYMBOL_CWXPHYLOEDITDLG_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * CwxPhyloEditDlg class declaration
 */

class CwxPhyloEditDlg: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CwxPhyloEditDlg )
    DECLARE_EVENT_TABLE()

public:
    CwxPhyloEditDlg();
    CwxPhyloEditDlg( wxWindow* parent, wxWindowID id = SYMBOL_CWXPHYLOEDITDLG_IDNAME, const wxString& caption = SYMBOL_CWXPHYLOEDITDLG_TITLE, const wxPoint& pos = SYMBOL_CWXPHYLOEDITDLG_POSITION, const wxSize& size = SYMBOL_CWXPHYLOEDITDLG_SIZE, long style = SYMBOL_CWXPHYLOEDITDLG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CWXPHYLOEDITDLG_IDNAME, const wxString& caption = SYMBOL_CWXPHYLOEDITDLG_TITLE, const wxPoint& pos = SYMBOL_CWXPHYLOEDITDLG_POSITION, const wxSize& size = SYMBOL_CWXPHYLOEDITDLG_SIZE, long style = SYMBOL_CWXPHYLOEDITDLG_STYLE );

    ~CwxPhyloEditDlg();

    void Init();

    void CreateControls();

    void SetParams(CPhyloTree* tree,
                   CPhyloTree::TTreeIdx node_idx,
                   CFeatureEdit* updated_feature);

////@begin CwxPhyloEditDlg event handler declarations

    void OnListctrl1Selected( wxListEvent& event );

    void OnListctrl1Deselected( wxListEvent& event );

    void OnButton4Click( wxCommandEvent& event );

    void OnButton5Click( wxCommandEvent& event );

    void OnButtonClick( wxCommandEvent& event );

    void OnColourctrlColourPickerChanged( wxColourPickerEvent& event );

    void OnOkClick( wxCommandEvent& event );

////@end CwxPhyloEditDlg event handler declarations

////@begin CwxPhyloEditDlg member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CwxPhyloEditDlg member function declarations

    static bool ShowToolTips();

////@begin CwxPhyloEditDlg member variables
    wxListCtrl* m_List;
    wxTextCtrl* m_Name;
    wxTextCtrl* m_Value;
    wxColourPickerCtrl* m_ColorPicker;
////@end CwxPhyloEditDlg member variables

    CPhyloTree* m_Tree;
    CPhyloTree::TTreeIdx m_NodeIdx;
    CFeatureEdit* m_UpdatedFeature;

protected:
    void x_UpdateSelectedRow();
};

END_NCBI_SCOPE

#endif // _WX_PHYLO_EDIT_DLG_H_
