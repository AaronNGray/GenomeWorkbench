/*  $Id: igspanel.hpp 26514 2012-10-01 12:16:28Z bollin $
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
#ifndef _IGSPANEL_H_
#define _IGSPANEL_H_

#include <corelib/ncbistd.hpp>
#include <gui/packages/pkg_sequence_edit/srcedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_panel.hpp>
#include <gui/packages/pkg_sequence_edit/igsflankpanel.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

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
#define ID_CIGSPANEL 10043
#define SYMBOL_CIGSPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CIGSPANEL_TITLE _("IGSPanel")
#define SYMBOL_CIGSPANEL_IDNAME ID_CIGSPANEL
#define SYMBOL_CIGSPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CIGSPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CIGSPanel class declaration
 */

class CIGSPanel: public CBulkCmdPanel
{    
    DECLARE_DYNAMIC_CLASS( CIGSPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CIGSPanel();
    CIGSPanel( wxWindow* parent, objects::CSeq_entry_Handle seh, 
                   CSourceRequirements::EWizardType wizard_type = CSourceRequirements::eWizardType_standard,
                   CSourceRequirements::EWizardSrcType src_type = CSourceRequirements::eWizardSrcType_any,  
                   wxWindowID id = SYMBOL_CIGSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CIGSPANEL_POSITION, const wxSize& size = SYMBOL_CIGSPANEL_SIZE, long style = SYMBOL_CIGSPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CIGSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CIGSPANEL_POSITION, const wxSize& size = SYMBOL_CIGSPANEL_SIZE, long style = SYMBOL_CIGSPANEL_STYLE );

    /// Destructor
    ~CIGSPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CIGSPanel event handler declarations

////@end CIGSPanel event handler declarations

////@begin CIGSPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CIGSPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CIGSPanel member variables
////@end CIGSPanel member variables

    virtual CRef<CCmdComposite> GetCommand();
    virtual string GetErrorMessage();

private:
    objects::CSeq_entry_Handle m_TopSeqEntry;
    CSourceRequirements::EWizardType m_WizardType;
    CSourceRequirements::EWizardSrcType m_SrcType;

    CIGSFlankPanel* m_Flank5;
    CIGSFlankPanel* m_Flank3;
};

END_NCBI_SCOPE

#endif
    // _IGSPANEL_H_
