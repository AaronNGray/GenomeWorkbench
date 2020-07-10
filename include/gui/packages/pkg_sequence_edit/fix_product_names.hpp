/*  $Id: fix_product_names.hpp 38635 2017-06-05 18:51:04Z asztalos $
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
 * Authors:  Igor Filippov
 */
#ifndef _FIX_PRODUCT_NAMES_H_
#define _FIX_PRODUCT_NAMES_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/widgets/data/report_dialog.hpp>

#include <wx/dialog.h>
#include <wx/checkbox.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

#define ID_FIX_PRODUCT_NAMES wxID_ANY
#define SYMBOL_FIX_PRODUCT_NAMES_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_FIX_PRODUCT_NAMES_TITLE _("Fix Product Names")
#define SYMBOL_FIX_PRODUCT_NAMES_IDNAME ID_FIX_PRODUCT_NAMES
#define SYMBOL_FIX_PRODUCT_NAMES_SIZE wxDefaultSize
#define SYMBOL_FIX_PRODUCT_NAMES_POSITION wxDefaultPosition

class CFixProductNames : public CReportEditingDialog
{    
    DECLARE_DYNAMIC_CLASS( CFixProductNames )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CFixProductNames();
    CFixProductNames( wxWindow* parent, objects::CSeq_entry_Handle seh, wxWindowID id = SYMBOL_FIX_PRODUCT_NAMES_IDNAME, const wxString& caption = SYMBOL_FIX_PRODUCT_NAMES_TITLE, const wxPoint& pos = SYMBOL_FIX_PRODUCT_NAMES_POSITION, const wxSize& size = SYMBOL_FIX_PRODUCT_NAMES_SIZE, long style = SYMBOL_FIX_PRODUCT_NAMES_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_FIX_PRODUCT_NAMES_IDNAME, const wxString& caption = SYMBOL_FIX_PRODUCT_NAMES_TITLE, const wxPoint& pos = SYMBOL_FIX_PRODUCT_NAMES_POSITION, const wxSize& size = SYMBOL_FIX_PRODUCT_NAMES_SIZE, long style = SYMBOL_FIX_PRODUCT_NAMES_STYLE );

    /// Destructor
    ~CFixProductNames();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    CRef<CCmdComposite> GetCommand();

    static CRef<CCmdComposite> apply(CSeq_entry_Handle tse, bool before_punct = false, bool before_cap = false, bool before_num = false, bool rm_commas = false, bool keep_last = false);
private:
    static void FixCaps(string &name, bool before_punct, bool before_cap, bool before_num, bool rm_commas, bool keep_last);
    CSeq_entry_Handle m_TopSeqEntry;
    wxCheckBox *m_BeforePunct;
    wxCheckBox *m_BeforeCap;
    wxCheckBox *m_BeforeNum;
    wxCheckBox *m_RmCommas;
    wxCheckBox *m_KeepLast;
};

END_NCBI_SCOPE

#endif
    // _FIX_PRODUCT_NAMES_H_
