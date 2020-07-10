/*  $Id: bulk_cmd_dlg.hpp 42432 2019-02-22 18:44:43Z filippov $
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
#ifndef _BULKCMDDLG_H_
#define _BULKCMDDLG_H_

#include <corelib/ncbistd.hpp>
#include <objects/submit/Seq_submit.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/framework/workbench.hpp>
#include <gui/objutils/objects.hpp>
#include <gui/utils/command_processor.hpp>
#include <gui/packages/pkg_sequence_edit/seqtable_util.hpp>
#include <wx/frame.h>
#include <vector>

class wxSizer;

BEGIN_NCBI_SCOPE


/*!
 * CBulkCmdPanel class declaration
 */

class CBulkCmdDlg: public wxFrame
{    
	DECLARE_ABSTRACT_CLASS( CBulkCmdDlg )
    DECLARE_EVENT_TABLE()
public:
    /// Constructors
    CBulkCmdDlg () {};
    CBulkCmdDlg (IWorkbench* wb) : m_Workbench(wb) {};
   
    bool Create (wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize, long style=wxDEFAULT_FRAME_STYLE, const wxString &name=wxFrameNameStr);
    bool Create (wxWindow *parent, IWorkbench* wb, wxWindowID id, const wxString &title, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize, long style=wxDEFAULT_FRAME_STYLE, const wxString &name=wxFrameNameStr);
    /// Destructor
    ~CBulkCmdDlg () {};


    virtual CRef<CCmdComposite> GetCommand() = 0;
    virtual string GetErrorMessage() = 0;
    virtual void OnChildChange(wxWindow *wnd) {}
    virtual void ExecuteCmd(CRef<CCmdComposite> cmd);
    bool GetTopLevelSeqEntryAndProcessor();
    TConstScopedObjects GetSelectedObjects();
    void UpdateChildrenFeaturePanels( wxSizer* sizer );
    void onCancelPressed( wxCommandEvent& event );
protected:
    IWorkbench*     m_Workbench;
    ICommandProccessor* m_CmdProccessor;
    objects::CSeq_entry_Handle m_TopSeqEntry;
    CConstRef<objects::CSeq_submit> m_SeqSubmit;
    string m_ErrorMessage;

    edit::EExistingText x_GetExistingTextHandling (int num_conflicts, bool allow_multiple = false);
};


void AddSize(wxSize& orig, const wxSize& add, int borders);

END_NCBI_SCOPE

#endif
    // _BULKCMDDLG_H_
