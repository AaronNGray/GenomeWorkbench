/*  $Id: dblinkpanel.hpp 36852 2016-11-04 14:21:29Z filippov $
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
#ifndef _DBLINKPANEL_H_
#define _DBLINKPANEL_H_

#include <corelib/ncbistd.hpp>
#include <objects/general/User_object.hpp>
#include <gui/widgets/edit/utilities.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class CStringListCtrl;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CDBLINKPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CDBLINKPANEL_TITLE _("DBLinkPanel")
#define SYMBOL_CDBLINKPANEL_IDNAME ID_CDBLINKPANEL
#define SYMBOL_CDBLINKPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CDBLINKPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CDBLinkPanel class declaration
 */

class CDBLinkPanel: public wxPanel, public IDescEditorPanel
{    
    DECLARE_DYNAMIC_CLASS( CDBLinkPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CDBLinkPanel();
    CDBLinkPanel( wxWindow* parent, CRef<objects::CUser_object> user, wxWindowID id = SYMBOL_CDBLINKPANEL_IDNAME, const wxPoint& pos = SYMBOL_CDBLINKPANEL_POSITION, const wxSize& size = SYMBOL_CDBLINKPANEL_SIZE, long style = SYMBOL_CDBLINKPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CDBLINKPANEL_IDNAME, const wxPoint& pos = SYMBOL_CDBLINKPANEL_POSITION, const wxSize& size = SYMBOL_CDBLINKPANEL_SIZE, long style = SYMBOL_CDBLINKPANEL_STYLE );

    /// Destructor
    ~CDBLinkPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    CRef<objects::CUser_object> GetUser_object() const;

////@begin CDBLinkPanel event handler declarations

////@end CDBLinkPanel event handler declarations

////@begin CDBLinkPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CDBLinkPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CDBLinkPanel member variables
    CStringListCtrl* m_BioProject;
    CStringListCtrl* m_BioSample;
    CStringListCtrl* m_ProbeDB;
    CStringListCtrl* m_TraceAssembly;
    CStringListCtrl* m_SRA;
    /// Control identifiers
    enum {
        ID_CDBLINKPANEL = 10021,
        ID_BIOPROJECT = 10020,
        ID_BIOSAMPLE = 10020,
        ID_PROBEDB = 10020,
        ID_TRACEASSEMBLY = 10000,
        ID_SRA = 10020
    };
////@end CDBLinkPanel member variables

    // IDescEditorPanel implementation
    virtual void ChangeSeqdesc(const objects::CSeqdesc& desc);
    virtual void UpdateSeqdesc(objects::CSeqdesc& desc);

private:
    CRef<objects::CUser_object> m_User;

};

END_NCBI_SCOPE

#endif
    // _DBLINKPANEL_H_
