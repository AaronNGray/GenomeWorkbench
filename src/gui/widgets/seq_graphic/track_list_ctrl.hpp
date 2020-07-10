#ifndef GUI_WIDGETS_SEQ_GRAPHIC___TRACK_LIST_CTRL__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___TRACK_LIST_CTRL__HPP

/*  $Id: track_list_ctrl.hpp 39733 2017-10-31 14:31:55Z katargir $
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
#include <wx/listctrl.h>

#include <gui/objutils/reg_view.hpp>

BEGIN_NCBI_SCOPE

class CTrackListCtrl : public wxListCtrl
{
public:
    class ITrackInfo
    {
    public:
        virtual ~ITrackInfo() {}
        virtual string GetDisplayName() const = 0;
        virtual string GetAccession() const = 0;
        virtual string GetCategory() const = 0;
        virtual string GetSubCategory() const = 0;
        virtual string GetComments() const = 0;
        virtual bool   GetVisible() const = 0;
        virtual void   SetVisible(bool visible) = 0;
    };
    typedef vector<ITrackInfo*> TTracks;

    CTrackListCtrl(
        wxWindow *parent,
        wxWindowID id = wxID_ANY,
        // All these parameters will be ignored, they were added for DialogBlocks use
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxLC_REPORT | wxLC_VIRTUAL | wxLC_SINGLE_SEL,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name = wxListCtrlNameStr);
    
    ~CTrackListCtrl();

    void SetTracks(const TTracks& tracks);
    void FilterValues(const string& filter);

    virtual wxString OnGetItemText(long item, long column) const;
    virtual int OnGetItemImage(long item) const;

    void OnLeftDown(wxMouseEvent& evt);
    void OnColumnClick(wxListEvent &evt);
    void OnSetVisibility(wxCommandEvent& evt);

    void LoadSettings(const CRegistryReadView& view);
    void SaveSettings(CRegistryWriteView view) const;

    DECLARE_EVENT_TABLE()

private:
    void x_UpdateHeader();
    void x_Sort();

    TTracks      m_Tracks;
    TTracks      m_TracksVisible;
    wxImageList* m_ImageList;
    int          m_SortColumn;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___TRACK_LIST_CTRL__HPP
