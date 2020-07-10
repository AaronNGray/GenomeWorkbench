#ifndef GUI_WIDGETS_WX___CHECKEDLISTCTRL_H
#define GUI_WIDGETS_WX___CHECKEDLISTCTRL_H

/*  $Id: checkedlistctrl.hpp 37741 2017-02-13 18:14:02Z katargir $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>
#include <gui/gui.hpp>

#include <wx/listctrl.h>

#if( wxMAJOR_VERSION == 3 && wxMINOR_VERSION == 0 )
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_LIST_ITEM_CHECKED, wxListEvent);
#define EVT_LIST_ITEM_CHECKED(id, fn) wx__DECLARE_LISTEVT(ITEM_CHECKED, id, fn)
#endif

class wxImageList;

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_WX_EXPORT wxCheckedListCtrl : public wxListCtrl
{
public:
    wxCheckedListCtrl();
    wxCheckedListCtrl(
        wxWindow *parent,
        wxWindowID winid = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxLC_REPORT,
        const wxValidator& validator = wxDefaultValidator,
        const wxString &name = wxListCtrlNameStr);

    ~wxCheckedListCtrl();

    bool Create(
            wxWindow *parent,
            wxWindowID winid = wxID_ANY,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = wxLC_REPORT,
            const wxValidator& validator = wxDefaultValidator,
            const wxString &name = wxListCtrlNameStr);

    void OnMouseEvent(wxMouseEvent& event);

    bool IsChecked(long item) const;
    void SetChecked(long item, bool checked);

protected:
    wxImageList* m_ImageList;

    wxDECLARE_DYNAMIC_CLASS_NO_COPY(wxCheckedListCtrl);
    DECLARE_EVENT_TABLE()
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS_WX___CHECKEDLISTCTRL_H
