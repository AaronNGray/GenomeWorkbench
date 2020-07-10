/*  $Id: convert_set_type_dlg.hpp 38635 2017-06-05 18:51:04Z asztalos $
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
#ifndef _CONVERT_SET_TYPE_DLG_H_
#define _CONVERT_SET_TYPE_DLG_H_

#include <corelib/ncbistd.hpp>
#include <objects/seqset/Bioseq_set.hpp>
#include <objmgr/scope.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <gui/widgets/edit/set_class_panel.hpp>
/*!
 * Includes
 */

////@begin includes
#include "wx/treectrl.h"
////@end includes

#include <wx/icon.h>
#include <wx/bitmap.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxTreeCtrl;
class CSetClassPanel;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CCONVERTSETTYPEDLG 10420
#define ID_TREEBOOK 10421
#define ID_WINDOW8 10422
#define SYMBOL_CCONVERTSETTYPEDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CCONVERTSETTYPEDLG_TITLE _("Convert Set Type")
#define SYMBOL_CCONVERTSETTYPEDLG_IDNAME ID_CCONVERTSETTYPEDLG
#define SYMBOL_CCONVERTSETTYPEDLG_SIZE wxSize(400, 300)
#define SYMBOL_CCONVERTSETTYPEDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CConvertSetTypeDlg class declaration
 */

class CConvertSetTypeDlg : public CReportEditingDialog, public ISetClassNotify
{    
    DECLARE_DYNAMIC_CLASS( CConvertSetTypeDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CConvertSetTypeDlg();
    CConvertSetTypeDlg( wxWindow* parent, const objects::CBioseq_set& set, wxWindowID id = SYMBOL_CCONVERTSETTYPEDLG_IDNAME, const wxString& caption = SYMBOL_CCONVERTSETTYPEDLG_TITLE, const wxPoint& pos = SYMBOL_CCONVERTSETTYPEDLG_POSITION, const wxSize& size = SYMBOL_CCONVERTSETTYPEDLG_SIZE, long style = SYMBOL_CCONVERTSETTYPEDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CCONVERTSETTYPEDLG_IDNAME, const wxString& caption = SYMBOL_CCONVERTSETTYPEDLG_TITLE, const wxPoint& pos = SYMBOL_CCONVERTSETTYPEDLG_POSITION, const wxSize& size = SYMBOL_CCONVERTSETTYPEDLG_SIZE, long style = SYMBOL_CCONVERTSETTYPEDLG_STYLE );

    /// Destructor
    ~CConvertSetTypeDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CConvertSetTypeDlg event handler declarations

    /// wxEVT_COMMAND_TREE_SEL_CHANGED event handler for ID_TREEBOOK
    void OnTreebookSelChanged( wxTreeEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
    void OnOkClick( wxCommandEvent& event );

////@end CConvertSetTypeDlg event handler declarations

////@begin CConvertSetTypeDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CConvertSetTypeDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CConvertSetTypeDlg member variables
    wxTreeCtrl* m_Tree;
    CSetClassPanel* m_ClassChoice;
////@end CConvertSetTypeDlg member variables

    CRef<CCmdComposite> GetCommand(objects::CScope& scope, const objects::CBioseq_set& orig, bool& error);
    CRef<CCmdComposite> GetCommand(objects::CScope& scope, 
                                   const objects::CBioseq_set& orig, 
                                   const objects::CBioseq_set& new_set,
                                   bool& error);

    virtual void ChangeNotify();

protected:
    CRef<objects::CBioseq_set> m_Set;

    void x_AddSet(objects::CBioseq_set& set, wxTreeItemId parent);

    typedef pair<wxTreeItemId, objects::CBioseq_set* > TItemSet;

    vector<TItemSet> m_ItemSets;

    objects::CBioseq_set &x_GetSetFromId(wxTreeItemId id);
    wxTreeItemId x_GetIdFromSet( objects::CBioseq_set *set);
};

END_NCBI_SCOPE

#endif
    // _CONVERT_SET_TYPE_DLG_H_
