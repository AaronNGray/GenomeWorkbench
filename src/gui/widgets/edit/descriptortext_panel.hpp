/*  $Id: descriptortext_panel.hpp 36852 2016-11-04 14:21:29Z filippov $
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
 * Authors:  Roman Katargin
 */
#ifndef _CDESCRIPTORTEXTPANEL_H_
#define _CDESCRIPTORTEXTPANEL_H_

#include <objects/seq/Seqdesc.hpp>
#include <corelib/ncbistd.hpp>
#include <gui/widgets/edit/utilities.hpp>
#include <gui/widgets/edit/import_export_access.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/valtext.h>
#include <wx/toplevel.h>

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
#define SYMBOL_CDESCRIPTORTEXTPANEL_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CDESCRIPTORTEXTPANEL_TITLE _("Descriptor Text Panel")
#define SYMBOL_CDESCRIPTORTEXTPANEL_IDNAME ID_CDESCRIPTORTEXTPANEL
#define SYMBOL_CDESCRIPTORTEXTPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CDESCRIPTORTEXTPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CDescriptorTextPanel class declaration
 */

class CDescriptorTextPanel: public wxPanel, public IDescEditorPanel, public CImportExportAccess
{    
    DECLARE_DYNAMIC_CLASS( CDescriptorTextPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CDescriptorTextPanel();
    CDescriptorTextPanel( wxWindow* parent, wxWindowID id = SYMBOL_CDESCRIPTORTEXTPANEL_IDNAME, const wxString& caption = SYMBOL_CDESCRIPTORTEXTPANEL_TITLE, const wxPoint& pos = SYMBOL_CDESCRIPTORTEXTPANEL_POSITION, const wxSize& size = SYMBOL_CDESCRIPTORTEXTPANEL_SIZE, long style = SYMBOL_CDESCRIPTORTEXTPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CDESCRIPTORTEXTPANEL_IDNAME, const wxString& caption = SYMBOL_CDESCRIPTORTEXTPANEL_TITLE, const wxPoint& pos = SYMBOL_CDESCRIPTORTEXTPANEL_POSITION, const wxSize& size = SYMBOL_CDESCRIPTORTEXTPANEL_SIZE, long style = SYMBOL_CDESCRIPTORTEXTPANEL_STYLE );

    /// Destructor
    ~CDescriptorTextPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    void SetText( const wxString& value);
    const wxString GetText ();

////@begin CDescriptorTextPanel event handler declarations

////@end CDescriptorTextPanel event handler declarations

////@begin CDescriptorTextPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CDescriptorTextPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CDescriptorTextPanel member variables
    wxTextCtrl* m_Text;
    /// Control identifiers
    enum {
        ID_CDESCRIPTORTEXTPANEL = 10064,
        ID_TEXTCTRL23 = 10065
    };
////@end CDescriptorTextPanel member variables

    // IDescEditorPanel implementation
    virtual void ChangeSeqdesc(const objects::CSeqdesc& desc);
    virtual void UpdateSeqdesc(objects::CSeqdesc& desc);

    virtual bool IsImportEnabled(void);
    virtual bool IsExportEnabled(void);
    virtual CRef<CSerialObject> OnExport(void);
    virtual void OnImport( CNcbiIfstream &istr);
private:
    CRef<objects::CSeqdesc> m_Desc;

};

END_NCBI_SCOPE

#endif
    // _CDESCRIPTORTEXTPANEL_H_
