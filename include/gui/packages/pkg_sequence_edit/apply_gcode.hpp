/*  $Id: apply_gcode.hpp 42311 2019-01-31 14:37:04Z asztalos $
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
#ifndef _APPLY_GCODE_H_
#define _APPLY_GCODE_H_

#include <corelib/ncbistd.hpp>
#include <objects/seqfeat/BioSource.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <wx/dialog.h>
#include <wx/choice.h>

BEGIN_NCBI_SCOPE

#define ID_APPLY_GCODE wxID_ANY
#define SYMBOL_APPLY_GCODE_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_APPLY_GCODE_TITLE _("Batch Apply Genetic Code")
#define SYMBOL_APPLY_GCODE_IDNAME ID_APPLY_GCODE
#define SYMBOL_APPLY_GCODE_SIZE wxDefaultSize
#define SYMBOL_APPLY_GCODE_POSITION wxDefaultPosition

class CApplyGCode : public CReportEditingDialog
{    
    DECLARE_DYNAMIC_CLASS( CApplyGCode )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CApplyGCode();
    CApplyGCode( wxWindow* parent, objects::CSeq_entry_Handle seh, wxWindowID id = SYMBOL_APPLY_GCODE_IDNAME, const wxString& caption = SYMBOL_APPLY_GCODE_TITLE, const wxPoint& pos = SYMBOL_APPLY_GCODE_POSITION, const wxSize& size = SYMBOL_APPLY_GCODE_SIZE, long style = SYMBOL_APPLY_GCODE_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_APPLY_GCODE_IDNAME, const wxString& caption = SYMBOL_APPLY_GCODE_TITLE, const wxPoint& pos = SYMBOL_APPLY_GCODE_POSITION, const wxSize& size = SYMBOL_APPLY_GCODE_SIZE, long style = SYMBOL_APPLY_GCODE_STYLE );

    /// Destructor
    ~CApplyGCode();

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
    void x_FindBioSource(CRef<CCmdComposite> composite);
    void x_ApplyToDescriptors(const objects::CSeq_entry& se, CCmdComposite* composite);
    void x_ApplyToBioSource(objects::CBioSource& biosource);

    objects::CSeq_entry_Handle m_TopSeqEntry;
    vector<pair<string,int> > m_GenCodes;
    wxChoice *m_Code;

};

END_NCBI_SCOPE

#endif
    // _APPLY_GCODE_H_
