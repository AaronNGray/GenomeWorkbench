/*  $Id: fuse_features.hpp 39649 2017-10-24 15:22:12Z asztalos $
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
#ifndef _FUSE_FEATURES_DLG_H_
#define _FUSE_FEATURES_DLG_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objmgr/bioseq_set_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/cmd_del_seq_feat.hpp>
#include <gui/packages/pkg_sequence_edit/string_constraint_panel.hpp>
#include <gui/packages/pkg_sequence_edit/field_choice_panel.hpp>
#include <gui/widgets/edit/feature_field_name_panel.hpp>
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
using namespace objects;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CFUSE_FEATURES_DLG 11000
#define SYMBOL_CFUSE_FEATURES_DLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CFUSE_FEATURES_DLG_TITLE _("Fuse Features")
#define SYMBOL_CFUSE_FEATURES_DLG_IDNAME ID_CFUSE_FEATURES_DLG
#define SYMBOL_CFUSE_FEATURES_DLG_SIZE wxDefaultSize
#define SYMBOL_CFUSE_FEATURES_DLG_POSITION wxDefaultPosition
////@end control identifiers
#define ID_FUSE_FEATURES_OKCANCEL 11012

class CFuseFeaturesDlg: public CBulkCmdDlg
{    
    DECLARE_DYNAMIC_CLASS( CFuseFeaturesDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CFuseFeaturesDlg();
    CFuseFeaturesDlg( wxWindow* parent,  IWorkbench *wb, wxWindowID id = SYMBOL_CFUSE_FEATURES_DLG_IDNAME, const wxString& caption = SYMBOL_CFUSE_FEATURES_DLG_TITLE, const wxPoint& pos = SYMBOL_CFUSE_FEATURES_DLG_POSITION, const wxSize& size = SYMBOL_CFUSE_FEATURES_DLG_SIZE, long style = SYMBOL_CFUSE_FEATURES_DLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CFUSE_FEATURES_DLG_IDNAME, const wxString& caption = SYMBOL_CFUSE_FEATURES_DLG_TITLE, const wxPoint& pos = SYMBOL_CFUSE_FEATURES_DLG_POSITION, const wxSize& size = SYMBOL_CFUSE_FEATURES_DLG_SIZE, long style = SYMBOL_CFUSE_FEATURES_DLG_STYLE );

    /// Destructor
    ~CFuseFeaturesDlg();

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
    void ProcessUpdateFeatEvent( wxCommandEvent& event );

private:
    void UpdateChildrenFeaturePanels( wxSizer* sizer );

    CFeatureTypePanel*  m_FeatureType;
    CFeatureFieldNamePanel* m_FeatureConstraint;
    CStringConstraintPanel* m_StringConstraintPanel;

};

class CFuseJoinsInLocs
{
public:
    static CRef<CCmdComposite> apply(CSeq_entry_Handle seh);
};

END_NCBI_SCOPE

#endif
    // _FUSE_FEATURES_DLG_H_
