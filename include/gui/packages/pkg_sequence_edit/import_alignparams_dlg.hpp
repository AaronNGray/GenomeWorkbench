/*  $Id: import_alignparams_dlg.hpp 36312 2016-09-12 17:50:16Z asztalos $
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
 * Authors:  Andrea Asztalos
 */
#ifndef _IMPORT_ALIGNPARAMS_DLG_H_
#define _IMPORT_ALIGNPARAMS_DLG_H_

#include <corelib/ncbistd.hpp>
#include <wx/dialog.h>

BEGIN_NCBI_SCOPE

class CTextAlignParamsPanel;
class CTextAlignParams;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CIMPORTALIGNPARAMSDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CIMPORTALIGNPARAMSDLG_TITLE _("Import Alignment")
#define SYMBOL_CIMPORTALIGNPARAMSDLG_IDNAME ID_CIMPORTALIGNPARAMSDLG
#define SYMBOL_CIMPORTALIGNPARAMSDLG_SIZE wxSize(400, 300)
#define SYMBOL_CIMPORTALIGNPARAMSDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CImportAlignParamsDlg class declaration
 */

class CImportAlignParamsDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CImportAlignParamsDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CImportAlignParamsDlg();
    CImportAlignParamsDlg( wxWindow* parent, 
        wxWindowID id = SYMBOL_CIMPORTALIGNPARAMSDLG_IDNAME, 
        const wxString& caption = SYMBOL_CIMPORTALIGNPARAMSDLG_TITLE, 
        const wxPoint& pos = SYMBOL_CIMPORTALIGNPARAMSDLG_POSITION, 
        const wxSize& size = SYMBOL_CIMPORTALIGNPARAMSDLG_SIZE, 
        long style = SYMBOL_CIMPORTALIGNPARAMSDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CIMPORTALIGNPARAMSDLG_IDNAME, 
        const wxString& caption = SYMBOL_CIMPORTALIGNPARAMSDLG_TITLE, 
        const wxPoint& pos = SYMBOL_CIMPORTALIGNPARAMSDLG_POSITION, 
        const wxSize& size = SYMBOL_CIMPORTALIGNPARAMSDLG_SIZE, 
        long style = SYMBOL_CIMPORTALIGNPARAMSDLG_STYLE );

    /// Destructor
    ~CImportAlignParamsDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();
////@begin CImportAlignParamsDlg event handler declarations

////@end CImportAlignParamsDlg event handler declarations

////@begin CImportAlignParamsDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CImportAlignParamsDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    const CTextAlignParams& GetData() const;
    void SetData(const CTextAlignParams& data);
private:
////@begin CImportAlignParamsDlg member variables
    /// Control identifiers
    enum {
        ID_CIMPORTALIGNPARAMSDLG = 10000,
        ID_FOREIGN = 10001
    };

    CTextAlignParamsPanel* m_AlignParamsPanel;
////@end CImportAlignParamsDlg member variables
};

END_NCBI_SCOPE

#endif
    // _IMPORT_ALIGNPARAMS_DLG_H_
