#ifndef GUI_WIDGETS_WX___TREECTRL__EX_H
#define GUI_WIDGETS_WX___TREECTRL__EX_H

/*  $Id: treectrl_ex.hpp 18397 2008-11-28 19:45:17Z dicuccio $
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
 * Authors:  Vladimir Tereshkov
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>
#include <gui/gui.hpp>
#include <wx/gdicmn.h>
#include <wx/treectrl.h>

class wxWindow;

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_WX_EXPORT CwxTreeCtrlEx : public wxTreeCtrl
{
public:
    typedef enum {
        TREE_EX_UNDEF       =-1,
        TREE_EX_UNCHECKED   = 0,
        TREE_EX_THREESTATE  = 1,
        TREE_EX_CHECKED     = 2
    } TTreeState;

public:
    CwxTreeCtrlEx(wxWindow *parent, wxWindowID id = -1,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               long style = wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT,
               const wxValidator& validator = wxDefaultValidator,
               const wxString& name = wxTreeCtrlNameStr);
    ~CwxTreeCtrlEx();

    void CheckAll(bool bcheck = true);
    void SetCheck(const wxTreeItemId& item, bool bcheck = true);
    bool GetCheck(const wxTreeItemId& item);
    

    size_t GetCheckedMultiple(wxArrayTreeItemIds& ids) const;
    void SetCheckMultiple(const wxArrayTreeItemIds& ids);   
    
protected:
    void OnLeftDown(wxMouseEvent &event);
    void OnKeyDown(wxKeyEvent &event);      
    void OnFocus(wxFocusEvent &event);

    void        x_SetState(const wxTreeItemId &id, TTreeState state = TREE_EX_UNDEF);
    TTreeState  x_ModifyState(const wxTreeItemId &id, TTreeState state = TREE_EX_UNDEF);
    TTreeState  x_ValidateState(const wxTreeItemId &id, TTreeState state = TREE_EX_UNDEF);
    TTreeState  x_ProcessDownstream(const wxTreeItemId &id, TTreeState state = TREE_EX_UNDEF);
    TTreeState  x_ProcessUpstream(const wxTreeItemId &id,  TTreeState state = TREE_EX_UNDEF);
    void        x_GetCheckedMultiple(wxArrayTreeItemIds& ids, wxTreeItemId& item) const;

    DECLARE_EVENT_TABLE();
};

END_NCBI_SCOPE

#endif
