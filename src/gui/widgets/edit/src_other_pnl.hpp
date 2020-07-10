/*  
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
 * Authors:  Vasuki Gobu
 */
#ifndef _SRC_OTHER_PNL_H_
#define _SRC_OTHER_PNL_H_

#include <corelib/ncbistd.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include "dbxref_panel.hpp"

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
#define SYMBOL_CSOURCEOTHERPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSOURCEOTHERPANEL_TITLE _("SourceOther")
#define SYMBOL_CSOURCEOTHERPANEL_IDNAME ID_CSOURCEOTHERPANEL
#define SYMBOL_CSOURCEOTHERPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CSOURCEOTHERPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSourceOtherPanel class declaration
 */

class CSourceOtherPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CSourceOtherPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSourceOtherPanel();
    CSourceOtherPanel( wxWindow* parent, wxWindowID id = SYMBOL_CSOURCEOTHERPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSOURCEOTHERPANEL_POSITION, const wxSize& size = SYMBOL_CSOURCEOTHERPANEL_SIZE, long style = SYMBOL_CSOURCEOTHERPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSOURCEOTHERPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSOURCEOTHERPANEL_POSITION, const wxSize& size = SYMBOL_CSOURCEOTHERPANEL_SIZE, long style = SYMBOL_CSOURCEOTHERPANEL_STYLE );

    /// Destructor
    ~CSourceOtherPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CSourceOtherPanel event handler declarations

////@end CSourceOtherPanel event handler declarations

////@begin CSourceOtherPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSourceOtherPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CSourceOtherPanel member variables
    wxTextCtrl* m_CommonNameCtrl;
    wxTextCtrl* m_LineageCtrl;
    wxTextCtrl* m_DivisionCtrl;
    /// Control identifiers
    enum {
        ID_CSOURCEOTHERPANEL = 10035,
        ID_TEXTCTRL1 = 10003,
        ID_TEXTCTRL2 = 10011,
        ID_TEXTCTRL3 = 10012
    };
////@end CSourceOtherPanel member variables

    void TransferFromOrgRef(const objects::COrg_ref& org);
    void TransferToOrgRef(objects::COrg_ref& org);
    void OnChangedTaxname(void);

protected:
    CDbxrefPanel* m_Dbxrefs;
    CRef<objects::COrg_ref> m_OrgRef; // used for dbxrefs panel
};

END_NCBI_SCOPE

#endif
    // _SRC_OTHER_PNL_H_
