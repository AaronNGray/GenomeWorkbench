/*  $Id: prefix_deflinedlg.hpp 42821 2019-04-18 19:32:56Z joukovv $
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
#ifndef _PREFIX_DEFLINEDLG_H_
#define _PREFIX_DEFLINEDLG_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <objtools/edit/autodef_with_tax.hpp>
#include <gui/widgets/data/report_dialog.hpp>

#include <wx/dialog.h>
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

BEGIN_NCBI_SCOPE

class SAutodefParams;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CPREFIXDEFLINEDLG 10284
#define ID_COMBOBOX1 10285
#define ID_COMBOBOX2 10286
#define ID_COMBOBOX3 10287
#define ID_CHECKBOX15 10288
#define ID_CHECKBOX16 10289
#define SYMBOL_CPREFIXDEFLINEDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CPREFIXDEFLINEDLG_TITLE _("Definition Line Prefixes")
#define SYMBOL_CPREFIXDEFLINEDLG_IDNAME ID_CPREFIXDEFLINEDLG
#define SYMBOL_CPREFIXDEFLINEDLG_SIZE wxSize(400, 300)
#define SYMBOL_CPREFIXDEFLINEDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CPrefixDeflinedlg class declaration
 */

class CPrefixDeflinedlg : public CReportEditingDialog
{    
    DECLARE_DYNAMIC_CLASS( CPrefixDeflinedlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CPrefixDeflinedlg();
    CPrefixDeflinedlg( wxWindow* parent, 
		objects::CSeq_entry_Handle seh, unsigned int withcombo = 0,
		wxWindowID id = SYMBOL_CPREFIXDEFLINEDLG_IDNAME, 
		const wxString& caption = SYMBOL_CPREFIXDEFLINEDLG_TITLE, 
		const wxPoint& pos = SYMBOL_CPREFIXDEFLINEDLG_POSITION, 
		const wxSize& size = SYMBOL_CPREFIXDEFLINEDLG_SIZE, 
		long style = SYMBOL_CPREFIXDEFLINEDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CPREFIXDEFLINEDLG_IDNAME, 
		const wxString& caption = SYMBOL_CPREFIXDEFLINEDLG_TITLE, 
		const wxPoint& pos = SYMBOL_CPREFIXDEFLINEDLG_POSITION, 
		const wxSize& size = SYMBOL_CPREFIXDEFLINEDLG_SIZE, 
		long style = SYMBOL_CPREFIXDEFLINEDLG_STYLE );

    /// Destructor
    ~CPrefixDeflinedlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

	void SetParams(SAutodefParams *params) { m_Params = params; }
	
	void PopulateModifierCombo();
	
	CRef<CCmdComposite> GetCommand();

////@begin CPrefixDeflinedlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CPrefixDeflinedlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    wxComboBox* m_Firstmod;
    wxComboBox* m_Secondmod;
    wxComboBox* m_Thirdmod;
    wxCheckBox* m_PrefixTaxname;
    wxCheckBox* m_UseLabels;

private:
	objects::CSeq_entry_Handle m_TopSeqEntry;
	unsigned int m_WithComboboxes;
	SAutodefParams* m_Params;
	
	size_t x_FindModifierBasedOnLabel(const string& label);
};

bool NCBI_GUIPKG_SEQUENCE_EDIT_EXPORT FindModifierOnBioseq(const objects::CBioseq_CI& bioseq_ci, 
		const objects::CAutoDefAvailableModifier& mod, string& modifier, bool show_label);

bool NCBI_GUIPKG_SEQUENCE_EDIT_EXPORT AddOrgToDefline(objects::CSeq_entry_Handle seh, CCmdComposite* composite);
bool NCBI_GUIPKG_SEQUENCE_EDIT_EXPORT AddModToDefline(objects::CSeq_entry_Handle entry, CCmdComposite* composite, 
				const objects::CAutoDefAvailableModifier& mod, bool show_label);
bool NCBI_GUIPKG_SEQUENCE_EDIT_EXPORT AddModVectorToDefline(objects::CSeq_entry_Handle entry, 
		CCmdComposite* composite, const objects::CAutoDefSourceDescription::TAvailableModifierVector &mod_vector, 
		bool show_label, bool show_org);

END_NCBI_SCOPE

#endif
    // _PREFIX_DEFLINEDLG_H_
