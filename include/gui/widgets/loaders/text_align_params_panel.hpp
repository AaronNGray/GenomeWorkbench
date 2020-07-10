#ifndef GUI_WIDGETS___LOADERS___TEXT_ALIGN_LOAD_PAGE__HPP
#define GUI_WIDGETS___LOADERS___TEXT_ALIGN_LOAD_PAGE__HPP

/*  $Id: text_align_params_panel.hpp 38443 2017-05-10 15:49:30Z katargir $
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
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>

/*!
 * Includes
 */

#include <gui/widgets/loaders/text_align_load_params.hpp>

#include <wx/panel.h>

////@begin includes
#include "text_align_load_params.hpp"
#include "wx/valtext.h"
#include "wx/valgen.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CTEXTALIGNPARAMSPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CTEXTALIGNPARAMSPANEL_TITLE _("Text Alignment Format Params Panel")
#define SYMBOL_CTEXTALIGNPARAMSPANEL_IDNAME ID_CTEXTALIGNPARAMSPANEL
#define SYMBOL_CTEXTALIGNPARAMSPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CTEXTALIGNPARAMSPANEL_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE

/*!
 * CTextAlignParamsPanel class declaration
 */

class NCBI_GUIWIDGETS_LOADERS_EXPORT CTextAlignParamsPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CTextAlignParamsPanel )
    DECLARE_EVENT_TABLE()

public:
    CTextAlignParamsPanel();
    CTextAlignParamsPanel( wxWindow* parent, wxWindowID id = SYMBOL_CTEXTALIGNPARAMSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CTEXTALIGNPARAMSPANEL_POSITION, const wxSize& size = SYMBOL_CTEXTALIGNPARAMSPANEL_SIZE, long style = SYMBOL_CTEXTALIGNPARAMSPANEL_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CTEXTALIGNPARAMSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CTEXTALIGNPARAMSPANEL_POSITION, const wxSize& size = SYMBOL_CTEXTALIGNPARAMSPANEL_SIZE, long style = SYMBOL_CTEXTALIGNPARAMSPANEL_STYLE );

    ~CTextAlignParamsPanel();

    void Init();

    void CreateControls();

    virtual bool TransferDataToWindow();

    virtual bool TransferDataFromWindow();

////@begin CTextAlignParamsPanel event handler declarations

////@end CTextAlignParamsPanel event handler declarations

////@begin CTextAlignParamsPanel member function declarations

    /// Data access
    CTextAlignParams& GetData() { return m_data; }
    const CTextAlignParams& GetData() const { return m_data; }
    void SetData(const CTextAlignParams& data) { m_data = data; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CTextAlignParamsPanel member function declarations

    static bool ShowToolTips();

////@begin CTextAlignParamsPanel member variables
    /// The data edited by this window
    CTextAlignParams m_data;
    /// Control identifiers
    enum {
        ID_CTEXTALIGNPARAMSPANEL = 10077,
        ID_TEXTCTRL14 = 10078,
        ID_TEXTCTRL15 = 10079,
        ID_TEXTCTRL16 = 10080,
        ID_TEXTCTRL17 = 10081,
        ID_TEXTCTRL18 = 10082,
        ID_CHOICE7 = 10083
    };
////@end CTextAlignParamsPanel member variables
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS___LOADERS___TEXT_ALIGN_LOAD_PAGE__HPP
