/*  $Id: rearrange_list.hpp 38157 2017-04-04 13:32:45Z ucko $
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
 * Authors:  Igor Filippov, based on wxRearrangeCtrl
 */


#ifndef _GUI_WIDGETS_REARRANGECTRL_H_
#define _GUI_WIDGETS_REARRANGECTRL_H_


#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>


#include "wx/checklst.h"
#include "wx/panel.h"
#include "wx/arrstr.h"

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_EDIT_EXPORT CCheckListBoxListener
{
public:
    virtual void UpdateCheckedState(size_t checked) {}
    virtual ~CCheckListBoxListener() {}
};

class NCBI_GUIWIDGETS_EDIT_EXPORT CDoubleClickConsumer
{
public:
    virtual void DoubleClick(int n) {}
    virtual ~CDoubleClickConsumer() {}
};

class NCBI_GUIWIDGETS_EDIT_EXPORT  CRearrangeList : public wxCheckListBox
{
public:
    // ctors and such
    // --------------

    // default ctor, call Create() later
    CRearrangeList() { }

    // ctor creating the control, the arguments are the same as for
    // wxCheckListBox except for the extra order array which defines the
    // (initial) display order of the items as well as their statuses, see the
    // description above
    CRearrangeList(wxWindow *parent,
                    wxWindowID id,
                    const wxPoint& pos,
                    const wxSize& size,
                    const wxArrayInt& order,
                    const wxArrayString& items,
                    long style = 0,
                    const wxValidator& validator = wxDefaultValidator,
                    const wxString& name = wxListBoxNameStr )
    {
        Create(parent, id, pos, size, order, items, style, validator, name);
    }

    // Create() function takes the same parameters as the base class one and
    // the order array determining the initial display order
    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxPoint& pos,
                const wxSize& size,
                const wxArrayInt& order,
                const wxArrayString& items,
                long style = 0,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxListBoxNameStr );


    // items order
    // -----------

    // get the current items order; the returned array uses the same convention
    // as the one passed to the ctor
    const wxArrayInt& GetCurrentOrder() const { return m_order; }

    // return true if the current item can be moved up or down (i.e. just that
    // it's not the first or the last one)
    bool CanMoveCurrentUp() const;
    bool CanMoveCurrentDown() const;

    // move the current item one position up or down, return true if it was moved
    // or false if the current item was the first/last one and so nothing was done
    bool MoveCurrentUp();
    bool MoveCurrentDown();


    // Override this to keep our m_order array in sync with the real item state.
    virtual void Check(unsigned int item, bool check = true) override;
    virtual void Clear();
    virtual void Set(const wxArrayInt& order, const wxArrayString& items);
    void SetCheckListListener(CCheckListBoxListener *win) {m_Listener = win;}
    void SetDoubleClickConsumer(CDoubleClickConsumer *win) {m_DoubleClickConsumer = win;}
private:
    // swap two items at the given positions in the listbox
    void Swap(int pos1, int pos2);

    // event handler for item checking/unchecking
    void OnCheck(wxCommandEvent& event);

    void OnDoubleClick(wxCommandEvent& event);

    // the current order array
    wxArrayInt m_order;
    size_t m_checked;
    CCheckListBoxListener *m_Listener;
    CDoubleClickConsumer *m_DoubleClickConsumer;

    wxDECLARE_EVENT_TABLE();
    wxDECLARE_NO_COPY_CLASS(CRearrangeList);
};

// ----------------------------------------------------------------------------
// CRearrangeCtrl: composite control containing a CRearrangeList and buttons
// ----------------------------------------------------------------------------

class NCBI_GUIWIDGETS_EDIT_EXPORT CRearrangeCtrl : public wxPanel
{
public:
    // ctors/Create function are the same as for wxRearrangeList
    CRearrangeCtrl()
    {
        Init();
    }

    CRearrangeCtrl(wxWindow *parent,
                    wxWindowID id,
                    const wxPoint& pos,
                    const wxSize& size,
                    const wxArrayInt& order,
                    const wxArrayString& items,
                    long style = 0,
                    const wxValidator& validator = wxDefaultValidator,
                    const wxString& name = wxListBoxNameStr)
    {
        Init();

        Create(parent, id, pos, size, order, items, style, validator, name);
    }

    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxPoint& pos,
                const wxSize& size,
                const wxArrayInt& order,
                const wxArrayString& items,
                long style = 0,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxListBoxNameStr);

    // get the underlying listbox
    CRearrangeList *GetList() const { return m_list; }
   
private:
    // common part of all ctors
    void Init();

    // event handlers for the buttons
    void OnUpdateButtonUI(wxUpdateUIEvent& event);
    void OnButton(wxCommandEvent& event);


    CRearrangeList *m_list;

    wxDECLARE_EVENT_TABLE();
    wxDECLARE_NO_COPY_CLASS(CRearrangeCtrl);
};

END_NCBI_SCOPE
#endif // _GUI_WIDGETS_REARRANGECTRL_H_


