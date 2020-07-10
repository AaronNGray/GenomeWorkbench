#ifndef GUI_WIDGETS_EDIT___BIOSOURCE_PANEL__HPP
#define GUI_WIDGETS_EDIT___BIOSOURCE_PANEL__HPP

/*  $Id: biosourcepanel.hpp 42407 2019-02-19 19:35:34Z asztalos $
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
 * Authors:  Vasuki Palanigobu
 */


#include <objects/seqfeat/BioSource.hpp>
#include "srcmod_list_panel.hpp"
#include <wx/checkbox.h>

/*!
 * Includes
 */

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

class wxListBox;
class wxCheckListBox;
class wxTreebook;

BEGIN_NCBI_SCOPE

////@begin forward declarations
class CAutoCompleteTextCtrl;
class IDescEditorPanel;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CBIOSOURCEPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CBIOSOURCEPANEL_TITLE _("Organism Name")
#define SYMBOL_CBIOSOURCEPANEL_IDNAME ID_ORGANISMNAMEPANEL
#define SYMBOL_CBIOSOURCEPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CBIOSOURCEPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CBioSourcePanel class declaration
 */

class CBioSourcePanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CBioSourcePanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CBioSourcePanel();
    CBioSourcePanel( wxWindow* parent, objects::CBioSource& source, 
                    IDescEditorPanel* desc_editor = nullptr,
                    wxWindowID id = SYMBOL_CBIOSOURCEPANEL_IDNAME, 
                    const wxPoint& pos = SYMBOL_CBIOSOURCEPANEL_POSITION, 
                    const wxSize& size = SYMBOL_CBIOSOURCEPANEL_SIZE, 
                    long style = SYMBOL_CBIOSOURCEPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
                wxWindowID id = SYMBOL_CBIOSOURCEPANEL_IDNAME, 
                const wxPoint& pos = SYMBOL_CBIOSOURCEPANEL_POSITION, 
                const wxSize& size = SYMBOL_CBIOSOURCEPANEL_SIZE, 
                long style = SYMBOL_CBIOSOURCEPANEL_STYLE );

    /// Destructor
    ~CBioSourcePanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();
    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();
    void PopulateBioSource(objects::CBioSource& source);

////@begin CBioSourcePanel event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_ORGPANEL_BTN
    void OnTaxonomyLookup( wxCommandEvent& event );

////@end CBioSourcePanel event handler declarations
    void OnUpdatedTaxname(wxMouseEvent& event);
    void OnEvtText(wxCommandEvent& event);

////@begin CBioSourcePanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CBioSourcePanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CBioSourcePanel member variables
    CAutoCompleteTextCtrl* m_ScientificNameCtrl;
    wxCheckBox* m_DisableStrainForwardingBtn;
    /// Control identifiers
    enum {
        ID_ORGANISMNAMEPANEL = 10500,
        ID_ORGPANEL_TXTCTRL = 10501,
        ID_ORGPANEL_BTN = 10502,
        ID_ORGPANEL_CHCKBOX = 10503,
        ID_ORGPANEL_SIZER = 10504
    };
////@end CBioSourcePanel member variables

private:
    CSrcModListPanel* m_SrcModList;
    objects::CBioSource& m_Source;
    IDescEditorPanel* m_ParentPanel{ nullptr };
    wxTreebook* m_ParentTreebook{ nullptr };

    wxTreebook* x_GetTree();
    void x_GetOrganismInfo();
    void x_SetOrganismInfo();
};

END_NCBI_SCOPE

#endif
    // GUI_WIDGETS_EDIT___BIOSOURCE_PANEL__HPP
