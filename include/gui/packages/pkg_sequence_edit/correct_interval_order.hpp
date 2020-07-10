/*  $Id: correct_interval_order.hpp 37317 2016-12-22 19:28:12Z filippov $
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
#ifndef _CORRECT_INTERVAL_ORDER_H_
#define _CORRECT_INTERVAL_ORDER_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objmgr/bioseq_set_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>

#include <wx/dialog.h>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_dlg.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes
#include <wx/sizer.h>
#include <wx/listctrl.h>
#include <wx/radiobut.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <gui/widgets/wx/wx_utils.hpp>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
////@end forward declarations

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CCORRECT_INTERVAL_ORDER 11000
#define SYMBOL_CCORRECT_INTERVAL_ORDER_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CCORRECT_INTERVAL_ORDER_TITLE _("Correct Interval Order")
#define SYMBOL_CCORRECT_INTERVAL_ORDER_IDNAME ID_CCORRECT_INTERVAL_ORDER
#define SYMBOL_CCORRECT_INTERVAL_ORDER_SIZE wxDefaultSize
#define SYMBOL_CCORRECT_INTERVAL_ORDER_POSITION wxDefaultPosition
////@end control identifiers
#define ID_LISTCTRL_CORRECT_INTERVAL_ORDER 11001
#define ID_CORRECT_INTERVAL_ORDER_RADIOBUTTON 11002
#define ID_CORRECT_INTERVAL_ORDER_RADIOBUTTON1 11003
#define ID_CORRECT_INTERVAL_ORDER_TEXTCTRL 11004
#define ID_CORRECT_INTERVAL_ORDER_CHECKBOX 11005
#define ID_CORRECT_INTERVAL_ORDER_RADIOBUTTON2 11006
#define ID_CORRECT_INTERVAL_ORDER_RADIOBUTTON3 11007
#define ID_CORRECT_INTERVAL_ORDER_TEXTCTRL1 11008
#define ID_CORRECT_INTERVAL_ORDER_CHECKBOX1 11009
#define ID_CORRECT_INTERVAL_ORDER_CHOICE1 11010
#define ID_CORRECT_INTERVAL_ORDER_CHOICE2 11011
#define ID_CORRECT_INTERVAL_ORDER_OKCANCEL 11012

class CCorrectIntervalOrder: public CBulkCmdDlg
{    
    DECLARE_DYNAMIC_CLASS( CCorrectIntervalOrder )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CCorrectIntervalOrder();
    CCorrectIntervalOrder( wxWindow* parent, IWorkbench *wb, wxWindowID id = SYMBOL_CCORRECT_INTERVAL_ORDER_IDNAME, const wxString& caption = SYMBOL_CCORRECT_INTERVAL_ORDER_TITLE, const wxPoint& pos = SYMBOL_CCORRECT_INTERVAL_ORDER_POSITION, const wxSize& size = SYMBOL_CCORRECT_INTERVAL_ORDER_SIZE, long style = SYMBOL_CCORRECT_INTERVAL_ORDER_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CCORRECT_INTERVAL_ORDER_IDNAME, const wxString& caption = SYMBOL_CCORRECT_INTERVAL_ORDER_TITLE, const wxPoint& pos = SYMBOL_CCORRECT_INTERVAL_ORDER_POSITION, const wxSize& size = SYMBOL_CCORRECT_INTERVAL_ORDER_SIZE, long style = SYMBOL_CCORRECT_INTERVAL_ORDER_STYLE );

    /// Destructor
    ~CCorrectIntervalOrder();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    virtual CRef<CCmdComposite> GetCommand();
    virtual string GetErrorMessage(){ return m_ErrorMessage;}
private:
    void ApplyToCSeq_entry (objects::CSeq_entry_Handle tse, CRef<CCmdComposite> composite, const string &name, bool subtypeAnyFlag, bool &modified);

    map<pair<int,int>,string> m_Types;
    vector<string> m_Descriptions;
    vector<bool> m_SubtypeAnyFlag;
    wxListCtrl* m_ListCtrl;
    wxTextCtrl* m_TextCtrl;
    wxRadioButton* m_RadioButton;
    wxCheckBox* m_CheckBox;
    wxTextCtrl* m_TextCtrl_id;
    wxRadioButton* m_RadioButton_id;
    wxCheckBox* m_CheckBox_id;
    wxChoice* m_ChoiceStrand;
    wxChoice* m_ChoiceType;
};


END_NCBI_SCOPE

#endif
    // _CORRECT_INTERVAL_ORDER_H_
