/*  $Id: rnanamepanel.hpp 26514 2012-10-01 12:16:28Z bollin $
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
#ifndef _RNANAMEPANEL_H_
#define _RNANAMEPANEL_H_

#include <corelib/ncbistd.hpp>
#include <gui/packages/pkg_sequence_edit/srcedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_panel.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes
#include <wx/sizer.h>
#include <wx/radiobut.h>
#include <wx/textctrl.h>
#include <wx/statbox.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CRNANAMEPANEL 10034
#define SYMBOL_CRNANAMEPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CRNANAMEPANEL_TITLE _("RNANamePanel")
#define SYMBOL_CRNANAMEPANEL_IDNAME ID_CRNANAMEPANEL
#define SYMBOL_CRNANAMEPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CRNANAMEPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CRNANamePanel class declaration
 */

class CRNANamePanel: public CBulkCmdPanel
{    
    DECLARE_DYNAMIC_CLASS( CRNANamePanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CRNANamePanel();
    CRNANamePanel( wxWindow* parent, objects::CSeq_entry_Handle seh, 
                   CSourceRequirements::EWizardType wizard_type = CSourceRequirements::eWizardType_standard,
                   CSourceRequirements::EWizardSrcType src_type = CSourceRequirements::eWizardSrcType_any,  
                   bool multi = false,
                   wxWindowID id = SYMBOL_CRNANAMEPANEL_IDNAME, const wxPoint& pos = SYMBOL_CRNANAMEPANEL_POSITION, const wxSize& size = SYMBOL_CRNANAMEPANEL_SIZE, long style = SYMBOL_CRNANAMEPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CRNANAMEPANEL_IDNAME, const wxPoint& pos = SYMBOL_CRNANAMEPANEL_POSITION, const wxSize& size = SYMBOL_CRNANAMEPANEL_SIZE, long style = SYMBOL_CRNANAMEPANEL_STYLE );

    /// Destructor
    ~CRNANamePanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CRNANamePanel event handler declarations

////@end CRNANamePanel event handler declarations
    void OnRNATypeSelected( wxCommandEvent& event );

////@begin CRNANamePanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CRNANamePanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CRNANamePanel member variables
    wxStaticBoxSizer* m_RNANameSizer;
////@end CRNANamePanel member variables

    wxTextCtrl* m_OtherLabel;

    virtual CRef<CCmdComposite> GetCommand();
    virtual string GetErrorMessage();

private:
    objects::CSeq_entry_Handle m_TopSeqEntry;
    CSourceRequirements::EWizardType m_WizardType;
    CSourceRequirements::EWizardSrcType m_SrcType;
    bool m_IsMulti;

    string x_GetMultiElementName();
    string x_GetSingleElementName();
};

END_NCBI_SCOPE

#endif
    // _RNANAMEPANEL_H_
