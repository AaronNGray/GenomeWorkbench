/*  $Id: resolve_intersecting_feats.hpp 39649 2017-10-24 15:22:12Z asztalos $
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
#ifndef _RESOLVE_INTERSECTING_FEATS_H_
#define _RESOLVE_INTERSECTING_FEATS_H_

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
USING_SCOPE(objects);

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_RESOLVE_INTERSECTING_FEATS 11000
#define SYMBOL_RESOLVE_INTERSECTING_FEATS_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_RESOLVE_INTERSECTING_FEATS_TITLE _("Resolve Feature Overlaps")
#define SYMBOL_RESOLVE_INTERSECTING_FEATS_IDNAME ID_RESOLVE_INTERSECTING_FEATS
#define SYMBOL_RESOLVE_INTERSECTING_FEATS_SIZE wxDefaultSize
#define SYMBOL_RESOLVE_INTERSECTING_FEATS_POSITION wxDefaultPosition
////@end control identifiers
#define ID_RESOLVE_INTERSECTING_FEATS_OKCANCEL 11022

class CResolveIntersectingFeats: public CBulkCmdDlg
{    
    DECLARE_DYNAMIC_CLASS( CResolveIntersectingFeats )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CResolveIntersectingFeats();
    CResolveIntersectingFeats( wxWindow* parent,  IWorkbench *wb, wxWindowID id = SYMBOL_RESOLVE_INTERSECTING_FEATS_IDNAME, const wxString& caption = SYMBOL_RESOLVE_INTERSECTING_FEATS_TITLE, const wxPoint& pos = SYMBOL_RESOLVE_INTERSECTING_FEATS_POSITION, const wxSize& size = SYMBOL_RESOLVE_INTERSECTING_FEATS_SIZE, long style = SYMBOL_RESOLVE_INTERSECTING_FEATS_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_RESOLVE_INTERSECTING_FEATS_IDNAME, const wxString& caption = SYMBOL_RESOLVE_INTERSECTING_FEATS_TITLE, const wxPoint& pos = SYMBOL_RESOLVE_INTERSECTING_FEATS_POSITION, const wxSize& size = SYMBOL_RESOLVE_INTERSECTING_FEATS_SIZE, long style = SYMBOL_RESOLVE_INTERSECTING_FEATS_STYLE );

    /// Destructor
    ~CResolveIntersectingFeats();

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

    CFeatureTypePanel*  m_FeatureType1;
    CFeatureFieldNamePanel* m_FeatureConstraint1;
    CStringConstraintPanel* m_StringConstraintPanel1;
    CFeatureTypePanel*  m_FeatureType2;
    CFeatureFieldNamePanel* m_FeatureConstraint2;
    CStringConstraintPanel* m_StringConstraintPanel2;
};

END_NCBI_SCOPE

#endif
    // _RESOLVE_INTERSECTING_FEATS_H_
