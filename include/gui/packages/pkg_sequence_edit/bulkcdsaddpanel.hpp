/*  $Id: bulkcdsaddpanel.hpp 42185 2019-01-09 18:49:13Z filippov $
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
#ifndef _BULKCDSADDPANEL_H_
#define _BULKCDSADDPANEL_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/packages/pkg_sequence_edit/srcedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_panel.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_location_panel.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>


/*!
 * Forward declarations
 */

////@begin forward declarations
class CBulkLocationPanel;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CBULKCDSADDPANEL 10030
#define ID_BULKCDSADD_PROTEIN_NAME 10021
#define ID_BULKCDSADD_PROTEIN_DESC 10022
#define ID_BULKCDSADD_GENE 10023
#define ID_BULKCDSADD_COMMENT 10024
#define ID_WINDOW 10027
#define SYMBOL_CBULKCDSADDPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CBULKCDSADDPANEL_TITLE _("BulkCDSAddPanel")
#define SYMBOL_CBULKCDSADDPANEL_IDNAME ID_CBULKCDSADDPANEL
#define SYMBOL_CBULKCDSADDPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CBULKCDSADDPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CBulkCDSAddPanel class declaration
 */

class CBulkCDSAddPanel: public CBulkCmdPanel
{    
    DECLARE_DYNAMIC_CLASS( CBulkCDSAddPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CBulkCDSAddPanel();
    CBulkCDSAddPanel( wxWindow* parent, objects::CSeq_entry_Handle seh, bool add_location_panel = true, wxWindowID id = SYMBOL_CBULKCDSADDPANEL_IDNAME, const wxPoint& pos = SYMBOL_CBULKCDSADDPANEL_POSITION, const wxSize& size = SYMBOL_CBULKCDSADDPANEL_SIZE, long style = SYMBOL_CBULKCDSADDPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CBULKCDSADDPANEL_IDNAME, const wxPoint& pos = SYMBOL_CBULKCDSADDPANEL_POSITION, const wxSize& size = SYMBOL_CBULKCDSADDPANEL_SIZE, long style = SYMBOL_CBULKCDSADDPANEL_STYLE );

    /// Destructor
    ~CBulkCDSAddPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CBulkCDSAddPanel event handler declarations

////@end CBulkCDSAddPanel event handler declarations

////@begin CBulkCDSAddPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CBulkCDSAddPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CBulkCDSAddPanel member variables
    wxTextCtrl* m_ProteinName;
    wxTextCtrl* m_ProteinDescription;
    wxTextCtrl* m_GeneSymbol;
    wxTextCtrl* m_Comment;
////@end CBulkCDSAddPanel member variables

    virtual CRef<CCmdComposite> GetCommand();
    virtual string GetErrorMessage();
    virtual void AddOneCommand(const objects::CBioseq_Handle& bsh, CRef<CCmdComposite> cmd, bool &ambiguous);
private:
    objects::CSeq_entry_Handle m_TopSeqEntry;
    wxChoice *m_CodonStart;
    wxCheckBox *m_AddmRNA;
    bool m_create_general_only;
};

END_NCBI_SCOPE

#endif
    // _BULKCDSADDPANEL_H_
