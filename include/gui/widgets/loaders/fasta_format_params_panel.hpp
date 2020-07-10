#ifndef GUI_WIDGETS___LOADERS___FASTA_FORMAT_PARAMS__HPP
#define GUI_WIDGETS___LOADERS___FASTA_FORMAT_PARAMS__HPP

/*  $Id: fasta_format_params_panel.hpp 44796 2020-03-17 22:37:42Z evgeniev $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */
#include <corelib/ncbistl.hpp>
#include <gui/gui_export.h>

#include <gui/objutils/registry.hpp>

#include <wx/panel.h>
#include "wx/valgen.h"

#include <gui/widgets/loaders/fasta_load_params.hpp>

////@begin control identifiers
#define SYMBOL_CFASTAFORMATPARAMSPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CFASTAFORMATPARAMSPANEL_TITLE _("Dialog")
#define SYMBOL_CFASTAFORMATPARAMSPANEL_IDNAME ID_CFASTAFORMATPARAMSPANEL
#define SYMBOL_CFASTAFORMATPARAMSPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CFASTAFORMATPARAMSPANEL_POSITION wxDefaultPosition
////@end control identifiers


BEGIN_NCBI_SCOPE


/** @addtogroup GUI_PKG_SEQUENCE
 *
 * @{
 */

///////////////////////////////////////////////////////////////////////////////
/// CFASTAFormatParamsPanel

class NCBI_GUIWIDGETS_LOADERS_EXPORT CFASTAFormatParamsPanel: public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CFASTAFormatParamsPanel )
    DECLARE_EVENT_TABLE()

public:
    CFASTAFormatParamsPanel();
    CFASTAFormatParamsPanel( wxWindow* parent, wxWindowID id = SYMBOL_CFASTAFORMATPARAMSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CFASTAFORMATPARAMSPANEL_POSITION, const wxSize& size = SYMBOL_CFASTAFORMATPARAMSPANEL_SIZE, long style = SYMBOL_CFASTAFORMATPARAMSPANEL_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CFASTAFORMATPARAMSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CFASTAFORMATPARAMSPANEL_POSITION, const wxSize& size = SYMBOL_CFASTAFORMATPARAMSPANEL_SIZE, long style = SYMBOL_CFASTAFORMATPARAMSPANEL_STYLE );

    ~CFASTAFormatParamsPanel();

    void Init();

    void CreateControls();

    virtual bool TransferDataToWindow();

    virtual bool TransferDataFromWindow();

////@end CFASTAFormatParamsPanel event handler declarations

////@begin CFASTAFormatParamsPanel member function declarations

    /// Data access
    CFastaLoadParams& GetData() { return m_data; }
    const CFastaLoadParams& GetData() const { return m_data; }
    void SetData(const CFastaLoadParams& data) { m_data = data; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CFASTAFormatParamsPanel member function declarations

    static bool ShowToolTips();

////@begin CFASTAFormatParamsPanel member variables
    /// The data edited by this window
    CFastaLoadParams m_data;
    /// Control identifiers
    enum {
        ID_CFASTAFORMATPARAMSPANEL = 10033,
        ID_SEQ_TYPE = 10040,
        ID_LOWERCASE = 10039,
        ID_CHECKBOX1 = 10034,
        ID_CHECKBOX2 = 10035,
        ID_CHECKBOX8 = 10021,
        ID_CHECKBOX4 = 10037,
        ID_CHECKBOX5 = 10038,
        ID_CHECKBOX = 10009
    };
////@end CFASTAFormatParamsPanel member variables
};

/* @} */

END_NCBI_SCOPE

#endif  // GUI_WIDGETS___LOADERS___FASTA_FORMAT_PARAMS__HPP
