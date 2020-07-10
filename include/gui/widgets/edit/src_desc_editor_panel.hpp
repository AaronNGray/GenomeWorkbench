/*  $Id: src_desc_editor_panel.hpp 42408 2019-02-19 21:12:19Z asztalos $
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
#ifndef _SRC_DESC_EDITOR_PANEL_H_
#define _SRC_DESC_EDITOR_PANEL_H_

#include <corelib/ncbistd.hpp>

#include <objects/seqfeat/BioSource.hpp>
#include <gui/widgets/edit/utilities.hpp>
/*!
 * Includes
 */

////@begin includes
#include "wx/treebook.h"
////@end includes

#include "wx/panel.h"

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxTreebook;
////@end forward declarations

BEGIN_NCBI_SCOPE

class CLocAndGCodePanel;
class CBioSourcePanel;
class CSourceOtherPanel;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CSRCDESCEDITORPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSRCDESCEDITORPANEL_TITLE _("SourceDescriptorEditorPanel")
#define SYMBOL_CSRCDESCEDITORPANEL_IDNAME ID_CSRCDESCEDITORPANEL
#define SYMBOL_CSRCDESCEDITORPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CSRCDESCEDITORPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSrcDescEditorPanel class declaration
 */

class CSrcDescEditorPanel: public wxPanel, public IDescEditorPanel
{    
    DECLARE_DYNAMIC_CLASS( CSrcDescEditorPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSrcDescEditorPanel();
    CSrcDescEditorPanel( wxWindow* parent, wxWindowID id = SYMBOL_CSRCDESCEDITORPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSRCDESCEDITORPANEL_POSITION, const wxSize& size = SYMBOL_CSRCDESCEDITORPANEL_SIZE, long style = SYMBOL_CSRCDESCEDITORPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSRCDESCEDITORPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSRCDESCEDITORPANEL_POSITION, const wxSize& size = SYMBOL_CSRCDESCEDITORPANEL_SIZE, long style = SYMBOL_CSRCDESCEDITORPANEL_STYLE );

    /// Destructor
    ~CSrcDescEditorPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

////@begin CSrcDescEditorPanel event handler declarations

////@end CSrcDescEditorPanel event handler declarations

////@begin CSrcDescEditorPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSrcDescEditorPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CSrcDescEditorPanel member variables
    wxTreebook* m_SourceTreebook;
    /// Control identifiers
    enum {
        ID_CSRCDESCEDITORPANEL = 10176,
        ID_TREEBOOK1 = 10177
    };
////@end CSrcDescEditorPanel member variables

    // IDescEditorPanel implementation
    virtual void ChangeSeqdesc(const objects::CSeqdesc& desc);
    virtual void UpdateSeqdesc(objects::CSeqdesc& desc);
    virtual void UpdateTaxname();

protected:
    CRef<objects::CBioSource> m_BioSource;
    CBioSourcePanel* m_SrcPanel{ nullptr };
    CLocAndGCodePanel* m_LocAndGCodePanel{ nullptr };
    CSourceOtherPanel* m_SrcOtherPanel{ nullptr };

};

END_NCBI_SCOPE

#endif
    // _SRC_DESC_EDITOR_PANEL_H_
