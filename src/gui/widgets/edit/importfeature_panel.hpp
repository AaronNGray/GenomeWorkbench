#ifndef GUI_WIDGETS_EDIT___IMPORTFEATUREPANEL__HPP
#define GUI_WIDGETS_EDIT___IMPORTFEATUREPANEL__HPP

/*  $Id: importfeature_panel.hpp 33303 2015-07-06 13:01:12Z asztalos $
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

#include <corelib/ncbistd.hpp>
#include <gui/widgets/edit/gbqual_panel.hpp>

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

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects) // namespace ncbi::objects::
class CSeq_feat;
END_SCOPE(objects)

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CIMPORTFEATUREPANEL_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CIMPORTFEATUREPANEL_TITLE _("ImportFeature SubPanel")
#define SYMBOL_CIMPORTFEATUREPANEL_IDNAME ID_CIMPORTFEATURESUBPANEL
#define SYMBOL_CIMPORTFEATUREPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CIMPORTFEATUREPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CImportFeaturePanel class declaration
 */

class CImportFeaturePanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CImportFeaturePanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CImportFeaturePanel();
    CImportFeaturePanel( wxWindow* parent, CSerialObject& object, wxWindowID id = SYMBOL_CIMPORTFEATUREPANEL_IDNAME, const wxString& caption = SYMBOL_CIMPORTFEATUREPANEL_TITLE, const wxPoint& pos = SYMBOL_CIMPORTFEATUREPANEL_POSITION, const wxSize& size = SYMBOL_CIMPORTFEATUREPANEL_SIZE, long style = SYMBOL_CIMPORTFEATUREPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CIMPORTFEATUREPANEL_IDNAME, const wxString& caption = SYMBOL_CIMPORTFEATUREPANEL_TITLE, const wxPoint& pos = SYMBOL_CIMPORTFEATUREPANEL_POSITION, const wxSize& size = SYMBOL_CIMPORTFEATUREPANEL_SIZE, long style = SYMBOL_CIMPORTFEATUREPANEL_STYLE );

    /// Destructor
    ~CImportFeaturePanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    bool TransferDataFromWindow();
    bool TransferDataToWindow();

    void PopulateImpFeat(objects::CSeq_feat& seq_feat);


////@begin CImportFeaturePanel event handler declarations

    /// wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_CHOICE3
    void OnChoice3Selected( wxCommandEvent& event );

////@end CImportFeaturePanel event handler declarations

////@begin CImportFeaturePanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CImportFeaturePanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CImportFeaturePanel member variables
    wxChoice* m_KeyCtrl;
    /// Control identifiers
    enum {
        ID_CIMPORTFEATURESUBPANEL = 10015,
        ID_CHOICE3 = 10042
    };
////@end CImportFeaturePanel member variables

private:
    CSerialObject* m_Object;
    CRef<CSerialObject> m_EditedFeat;

    CGBQualPanel* m_GBQualPanel;
    wxSizer* m_GBQualSizer;

};

END_NCBI_SCOPE

#endif
    // GUI_WIDGETS_EDIT___IMPORTFEATUREPANEL__HPP
