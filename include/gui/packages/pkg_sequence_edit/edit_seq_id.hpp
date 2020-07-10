/*  $Id: edit_seq_id.hpp 38638 2017-06-05 19:26:02Z asztalos $
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
#ifndef _EDIT_SEQ_ID_H_
#define _EDIT_SEQ_ID_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/scope.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <gui/packages/pkg_sequence_edit/string_constraint_panel.hpp>

/*!
 * Includes
 */

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif


/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

BEGIN_NCBI_SCOPE

class CParseTextOptionsDlg;
/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CEDIT_SEQ_ID 10420
#define SYMBOL_CEDIT_SEQ_ID_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CEDIT_SEQ_ID_TITLE _("Edit Sequence Id")
#define SYMBOL_CEDIT_SEQ_ID_IDNAME ID_CEDIT_SEQ_ID
#define SYMBOL_CEDIT_SEQ_ID_SIZE wxSize(400, 300)
#define SYMBOL_CEDIT_SEQ_ID_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CEditSeqId class declaration
 */

class CModifySeqId
{
public:
    CModifySeqId() {}
    CModifySeqId(objects::CSeq_entry_Handle seh, const string &name) : m_TopSeqEntry(seh), m_cmd_name(name) {}
    virtual ~CModifySeqId() {}
    virtual CRef<CCmdComposite> GetCommand(void);

protected:
    void x_ChangeId(objects::CSeq_id &id);
    virtual void x_UpdateBioseq(CRef<CBioseq> new_seq);
    void x_GetRemoveOriginalIdCommand(CRef<CCmdComposite> cmd);
    void x_UpdateSeqAnnot(objects::CSeq_annot &annot);
    void x_UpdateSeqAlign(objects::CSeq_align &align);
    void x_UpdateLoc(objects::CSeq_loc &loc);
    void x_UpdateFeat(objects::CSeq_feat &feat);
    void x_UpdateVariation(objects::CVariation_ref &var);

    objects::CSeq_entry_Handle m_TopSeqEntry;
    vector<pair<CConstRef<CSeq_id>, CRef<CSeq_id> > > m_old_to_new_id;
    string m_cmd_name;
    set<string> m_remove_ids;
};

class CEditSeqId : public CReportEditingDialog, public CModifySeqId
{    
    DECLARE_DYNAMIC_CLASS( CEditSeqId )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CEditSeqId();
    CEditSeqId( wxWindow* parent, objects::CSeq_entry_Handle seh, wxWindowID id = SYMBOL_CEDIT_SEQ_ID_IDNAME, const wxString& caption = SYMBOL_CEDIT_SEQ_ID_TITLE, const wxPoint& pos = SYMBOL_CEDIT_SEQ_ID_POSITION, const wxSize& size = SYMBOL_CEDIT_SEQ_ID_SIZE, long style = SYMBOL_CEDIT_SEQ_ID_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CEDIT_SEQ_ID_IDNAME, const wxString& caption = SYMBOL_CEDIT_SEQ_ID_TITLE, const wxPoint& pos = SYMBOL_CEDIT_SEQ_ID_POSITION, const wxSize& size = SYMBOL_CEDIT_SEQ_ID_SIZE, long style = SYMBOL_CEDIT_SEQ_ID_STYLE );

    /// Destructor
    virtual ~CEditSeqId();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CEditSeqId event handler declarations

////@end CEditSeqId event handler declarations

////@begin CEditSeqId member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CEditSeqId member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();
    virtual CRef<CCmdComposite> GetCommand(void);
    CRef<CCmdComposite> GetRemoveOriginalIdCommand(void);

private:
    void x_CollectChangedIds(void);

    wxRadioButton* m_Local;
    wxRadioButton* m_GeneralDb;
    wxRadioButton* m_GeneralTag;
    wxTextCtrl* m_Prefix;
    wxTextCtrl* m_Suffix;
    wxRadioButton* m_Any;
    wxRadioButton* m_Nuc;
    wxRadioButton* m_DNA;
    wxRadioButton* m_RNA;
    wxRadioButton* m_Proteins;
    CStringConstraintPanel* m_StringConstraintPanel;    
    CParseTextOptionsDlg* m_ParseOptions;
    wxCheckBox *m_RemoveOrig;
};

class CRemoveSeqId:  public CModifySeqId
{
public:
    CRemoveSeqId() {}
    CRemoveSeqId(objects::CSeq_entry_Handle seh, CSeq_id::E_Choice id_type, CSeq_inst::EMol filter = CSeq_inst::eMol_not_set);
    CRef<CCmdComposite> GetRemoveOriginalIdCommand();
protected:
    virtual void x_UpdateBioseq(CRef<CBioseq> new_seq);
};

class CLocalToGeneralId:  public CModifySeqId
{
public:
    CLocalToGeneralId(objects::CSeq_entry_Handle seh, CSeq_inst::EMol filter = CSeq_inst::eMol_not_set);
};

class CGeneralToLocalId:  public CModifySeqId
{
public:
    CGeneralToLocalId(objects::CSeq_entry_Handle seh, CSeq_inst::EMol filter = CSeq_inst::eMol_not_set);
};


class CRemoveName:  public CModifySeqId
{
public:
    CRemoveName(objects::CSeq_entry_Handle seh);

private:
    void x_GetReplacementIds(CConstRef<CSeq_id> id);
};

class CAccessionToLocalId:  public CModifySeqId
{
public:
    CAccessionToLocalId(objects::CSeq_entry_Handle seh, CSeq_inst::EMol filter, bool force_name);

private:
    void x_GetReplacementIds(CConstRef<CSeq_id> id, bool force_name);
};

////@begin control identifiers
#define ID_REMOVE_GENERAL_ID 10420
#define SYMBOL_REMOVE_GENERAL_ID_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_REMOVE_GENERAL_ID_TITLE _("Remove General Id")
#define SYMBOL_REMOVE_GENERAL_ID_IDNAME ID_REMOVE_GENERAL_ID
#define SYMBOL_REMOVE_GENERAL_ID_SIZE wxSize(400, 300)
#define SYMBOL_REMOVE_GENERAL_ID_POSITION wxDefaultPosition
////@end control identifiers

class CRemoveGeneralId : public CReportEditingDialog, public CRemoveSeqId
{    
    DECLARE_DYNAMIC_CLASS( CRemoveGeneralId )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CRemoveGeneralId();
    CRemoveGeneralId( wxWindow* parent, objects::CSeq_entry_Handle seh, wxWindowID id = SYMBOL_REMOVE_GENERAL_ID_IDNAME, const wxString& caption = SYMBOL_REMOVE_GENERAL_ID_TITLE, const wxPoint& pos = SYMBOL_REMOVE_GENERAL_ID_POSITION, const wxSize& size = SYMBOL_REMOVE_GENERAL_ID_SIZE, long style = SYMBOL_REMOVE_GENERAL_ID_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_REMOVE_GENERAL_ID_IDNAME, const wxString& caption = SYMBOL_REMOVE_GENERAL_ID_TITLE, const wxPoint& pos = SYMBOL_REMOVE_GENERAL_ID_POSITION, const wxSize& size = SYMBOL_REMOVE_GENERAL_ID_SIZE, long style = SYMBOL_REMOVE_GENERAL_ID_STYLE );

    /// Destructor
    virtual ~CRemoveGeneralId();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CEditSeqId event handler declarations

////@end CEditSeqId event handler declarations

////@begin CEditSeqId member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CEditSeqId member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();
    virtual CRef<CCmdComposite> GetCommand(void);

private:
    void x_CollectChangedIds(void);

    CStringConstraintPanel* m_StringConstraintPanel;    
};


END_NCBI_SCOPE

#endif
    // _EDIT_SEQ_ID_H_
