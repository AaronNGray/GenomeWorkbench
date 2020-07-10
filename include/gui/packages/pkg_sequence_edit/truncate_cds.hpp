/*  $Id: truncate_cds.hpp 38635 2017-06-05 18:51:04Z asztalos $
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
 * Authors: Igor Filippov
 */
#ifndef _TRUNCATE_CDS_H_
#define _TRUNCATE_CDS_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <wx/dialog.h>
#include <wx/checkbox.h>

/*!
 * Includes
 */

////@begin includes
////@end includes
#include <wx/sizer.h>

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
#define ID_TRUNCATE_CDS wxID_ANY
#define SYMBOL_CTRUNCATE_CDS_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CTRUNCATE_CDS_TITLE _("Truncate CDS")
#define SYMBOL_CTRUNCATE_CDS_IDNAME ID_TRUNCATE_CDS
#define SYMBOL_CTRUNCATE_CDS_SIZE wxSize(200,100)
#define SYMBOL_CTRUNCATE_CDS_POSITION wxDefaultPosition
////@end control identifiers


class CTruncateCDS : public CReportEditingDialog
{    
    DECLARE_DYNAMIC_CLASS( CTruncateCDS )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CTruncateCDS();
    CTruncateCDS( wxWindow* parent, 
                  objects::CSeq_entry_Handle seh,
                  wxWindowID id = SYMBOL_CTRUNCATE_CDS_IDNAME, 
                  const wxString& caption = SYMBOL_CTRUNCATE_CDS_TITLE, 
                  const wxPoint& pos = SYMBOL_CTRUNCATE_CDS_POSITION, 
                  const wxSize& size = SYMBOL_CTRUNCATE_CDS_SIZE, 
                  long style = SYMBOL_CTRUNCATE_CDS_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
		 wxWindowID id = SYMBOL_CTRUNCATE_CDS_IDNAME, 
		 const wxString& caption = SYMBOL_CTRUNCATE_CDS_TITLE, 
		 const wxPoint& pos = SYMBOL_CTRUNCATE_CDS_POSITION, 
		 const wxSize& size = SYMBOL_CTRUNCATE_CDS_SIZE, 
		 long style = SYMBOL_CTRUNCATE_CDS_STYLE );

    /// Destructor
    ~CTruncateCDS();

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
private:
    void TruncateProteinFeat(objects::CSeq_feat &prot_feat, const objects::CSeq_feat& cds, objects::CScope& scope);
    void AdjustMolinfo(const objects::CSeq_entry& se, CRef<CCmdComposite> composite, const objects::CSeq_feat& cds);
    objects::CSeq_entry_Handle m_TopSeqEntry;
    wxCheckBox *m_TrimProtFeats;
    wxCheckBox *m_Retranslate;
    wxCheckBox *m_TruncateMRNA;
    wxCheckBox *m_ResetGenes;
    wxCheckBox *m_PrependProtName;
};

END_NCBI_SCOPE

#endif
    // _TRUNCATE_CDS_H_
