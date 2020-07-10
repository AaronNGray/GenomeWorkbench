/*  $Id: dbxref_panel.hpp 31212 2014-09-12 12:45:01Z bollin $
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
#ifndef _DBXREF_PANEL_H_
#define _DBXREF_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/general/Dbtag.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/panel.h>
#include <wx/frame.h>
#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/valtext.h>
#include <wx/scrolwin.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CDBXREFPANEL_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CDBXREFPANEL_TITLE _("DbxrefPanel")
#define SYMBOL_CDBXREFPANEL_IDNAME ID_CDBXREFPANEL
#define SYMBOL_CDBXREFPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CDBXREFPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CDbxrefPanel class declaration
 */

class CDbxrefPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CDbxrefPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CDbxrefPanel();
    CDbxrefPanel( wxWindow* parent, CSerialObject& object,
                  wxWindowID id = SYMBOL_CDBXREFPANEL_IDNAME, const wxString& caption = SYMBOL_CDBXREFPANEL_TITLE, const wxPoint& pos = SYMBOL_CDBXREFPANEL_POSITION, const wxSize& size = SYMBOL_CDBXREFPANEL_SIZE, long style = SYMBOL_CDBXREFPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CDBXREFPANEL_IDNAME, const wxString& caption = SYMBOL_CDBXREFPANEL_TITLE, const wxPoint& pos = SYMBOL_CDBXREFPANEL_POSITION, const wxSize& size = SYMBOL_CDBXREFPANEL_SIZE, long style = SYMBOL_CDBXREFPANEL_STYLE );

    /// Destructor
    ~CDbxrefPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    void PopulateDbxrefs(objects::CSeq_feat& seq_feat);
    void PopulateDbxrefs(objects::COrg_ref& org);

    void AddLastDbxref (wxWindow* link);
	void DeleteTaxonRef();

////@begin CDbxrefPanel event handler declarations

////@end CDbxrefPanel event handler declarations

////@begin CDbxrefPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CDbxrefPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CDbxrefPanel member variables
    wxScrolledWindow* m_ScrolledWindow;
    wxBoxSizer* m_Sizer;
    /// Control identifiers
    enum {
        ID_CDBXREFPANEL = 10091,
        ID_SCROLLEDWINDOW2 = 10092
    };
////@end CDbxrefPanel member variables

private:
    CSerialObject* m_Object;

    vector< CRef< objects::CDbtag > > m_Xrefs;

    int m_NumRows;
    int m_TotalHeight;
    int m_TotalWidth;
    int m_ScrollRate;
    int m_MaxRowsDisplayed;

    void x_UpdateXrefs();
    wxWindow* x_AddRow(CRef<objects::CDbtag> tag);
};

END_NCBI_SCOPE

#endif
    // _DBXREF_PANEL_H_
