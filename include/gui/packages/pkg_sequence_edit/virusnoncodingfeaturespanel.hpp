/*  $Id: virusnoncodingfeaturespanel.hpp 28329 2013-06-19 11:58:38Z bollin $
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
#ifndef _VIRUSNONCODINGFEATURESPANEL_H_
#define _VIRUSNONCODINGFEATURESPANEL_H_

#include <corelib/ncbistd.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_panel.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/radiobox.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/statbox.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/radiobox.h>
#include <wx/icon.h>


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
#define ID_CVIRUSNONCODINGFEATURESPANEL 10031
#define ID_RADIOBOX 10032
#define ID_NONCODINGVIRUSFEATURETYPE 10033
#define ID_PARTIAL5 10025
#define ID_PARTIAL3 10026
#define SYMBOL_CVIRUSNONCODINGFEATURESPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CVIRUSNONCODINGFEATURESPANEL_TITLE _("VirusNonCodingFeaturesPanel")
#define SYMBOL_CVIRUSNONCODINGFEATURESPANEL_IDNAME ID_CVIRUSNONCODINGFEATURESPANEL
#define SYMBOL_CVIRUSNONCODINGFEATURESPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CVIRUSNONCODINGFEATURESPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CVirusNonCodingFeaturesPanel class declaration
 */

class CVirusNonCodingFeaturesPanel: public CBulkCmdPanel
{    
    DECLARE_DYNAMIC_CLASS( CVirusNonCodingFeaturesPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CVirusNonCodingFeaturesPanel();
    CVirusNonCodingFeaturesPanel( wxWindow* parent, objects::CSeq_entry_Handle seh, wxWindowID id = SYMBOL_CVIRUSNONCODINGFEATURESPANEL_IDNAME, const wxPoint& pos = SYMBOL_CVIRUSNONCODINGFEATURESPANEL_POSITION, const wxSize& size = SYMBOL_CVIRUSNONCODINGFEATURESPANEL_SIZE, long style = SYMBOL_CVIRUSNONCODINGFEATURESPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CVIRUSNONCODINGFEATURESPANEL_IDNAME, const wxPoint& pos = SYMBOL_CVIRUSNONCODINGFEATURESPANEL_POSITION, const wxSize& size = SYMBOL_CVIRUSNONCODINGFEATURESPANEL_SIZE, long style = SYMBOL_CVIRUSNONCODINGFEATURESPANEL_STYLE );

    /// Destructor
    ~CVirusNonCodingFeaturesPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CVirusNonCodingFeaturesPanel event handler declarations

    /// wxEVT_COMMAND_RADIOBOX_SELECTED event handler for ID_RADIOBOX
    void OnFeatureTypeSelected( wxCommandEvent& event );

////@end CVirusNonCodingFeaturesPanel event handler declarations

////@begin CVirusNonCodingFeaturesPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CVirusNonCodingFeaturesPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CVirusNonCodingFeaturesPanel member variables
    wxRadioBox* m_FeatureType;
    wxTextCtrl* m_ExtraText;
    wxStaticBox* m_PartialBox;
    wxCheckBox* m_Partial5;
    wxCheckBox* m_Partial3;
////@end CVirusNonCodingFeaturesPanel member variables
    virtual CRef<CCmdComposite> GetCommand();
    virtual string GetErrorMessage();

private:
    objects::CSeq_entry_Handle m_TopSeqEntry;

    void x_HidePartial();
    void x_ShowPartial();
};

END_NCBI_SCOPE

#endif
    // _VIRUSNONCODINGFEATURESPANEL_H_
