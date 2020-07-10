/*  $Id: lowercase_qualifiers.hpp 39649 2017-10-24 15:22:12Z asztalos $
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
#ifndef _LOWERCASE_QUALS_H_
#define _LOWERCASE_QUALS_H_

#include <corelib/ncbistd.hpp>
#include <gui/objutils/convert_feat.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_dlg.hpp>
#include <gui/widgets/edit/feature_type_panel.hpp>
#include <gui/packages/pkg_sequence_edit/field_constraint_panel.hpp>
#include <gui/packages/pkg_sequence_edit/ok_cancel_panel.hpp>
#include <gui/packages/pkg_sequence_edit/cap_change_panel.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/radiobut.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
class CFeatureTypePanel;
class CFieldConstraintPanel;
class COkCancelPanel;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_LOWERCASE_QUALS 10147
#define SYMBOL_LOWERCASE_QUALS_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_LOWERCASE_QUALS_TITLE _("Lowercase Qualifiers")
#define SYMBOL_LOWERCASE_QUALS_IDNAME ID_LOWERCASE_QUALS
#define SYMBOL_LOWERCASE_QUALS_SIZE wxSize(400, 300)
#define SYMBOL_LOWERCASE_QUALS_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CConvertFeatDlg class declaration
 */

class CLowerQuals: public CBulkCmdDlg
{    
    DECLARE_DYNAMIC_CLASS( CLowerQuals )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CLowerQuals();
    CLowerQuals( wxWindow* parent, IWorkbench* wb, wxWindowID id = SYMBOL_LOWERCASE_QUALS_IDNAME, const wxString& caption = SYMBOL_LOWERCASE_QUALS_TITLE, const wxPoint& pos = SYMBOL_LOWERCASE_QUALS_POSITION, const wxSize& size = SYMBOL_LOWERCASE_QUALS_SIZE, long style = SYMBOL_LOWERCASE_QUALS_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_LOWERCASE_QUALS_IDNAME, const wxString& caption = SYMBOL_LOWERCASE_QUALS_TITLE, const wxPoint& pos = SYMBOL_LOWERCASE_QUALS_POSITION, const wxSize& size = SYMBOL_LOWERCASE_QUALS_SIZE, long style = SYMBOL_LOWERCASE_QUALS_STYLE );

    /// Destructor
    ~CLowerQuals();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CConvertFeatDlg event handler declarations

    void ProcessUpdateFeatEvent( wxCommandEvent& event );
////@end CConvertFeatDlg event handler declarations

////@begin CConvertFeatDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CConvertFeatDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    // for CBulkCmdDlg
    virtual CRef<CCmdComposite> GetCommand();
    virtual string GetErrorMessage();

private:
    CFieldChoicePanel* m_FieldType;
    COkCancelPanel* m_OkCancel;
    CCapChangePanel *m_CapChangeOptions;
    wxRadioButton* m_RadioDesc;
    wxRadioButton* m_RadioFeat;
};

END_NCBI_SCOPE

#endif
    // _LOWERCASE_QUALS_H_
